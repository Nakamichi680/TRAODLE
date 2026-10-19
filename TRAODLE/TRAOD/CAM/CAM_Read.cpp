#include "stdafx.h"
#include "MATH/math.h"
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "TRAOD/CAM/CAM_Struct.h"
#include "TRAOD/CAM/CAM_Functions.h"


bool CAM_Read (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	CAM_HEADER cam_header;
	CAM_POINT_FROM camera_point_from;
	CAM_LOOK_AT camera_look_at;
	CAM_ROLL cam_roll;
	CAM_FOV cam_fov;

	string name = filename.substr(0, filename.find(".CAM"));

	ifstream camfile(filename, std::ios::binary);
	if (!camfile.is_open())
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " not found.";
		return false;
	}

	///////////////////    LETTURA HEADER FILE CAM
	camfile.read(reinterpret_cast<char*>(&cam_header.MAGIC1), sizeof(cam_header.MAGIC1));				// ID1 file CAM ("HDER")
	camfile.seekg(8, ios_base::cur);
	camfile.read(reinterpret_cast<char*>(&cam_header.nFrames), sizeof(cam_header.nFrames));				// Numero di frames dell'animazione della telecamera
	camfile.read(reinterpret_cast<char*>(&cam_header.MAGIC2), sizeof(cam_header.MAGIC2));				// ID2 file CAM ("TRAX")
	camfile.read(reinterpret_cast<char*>(&cam_header.Cuts_offset), sizeof(cam_header.Cuts_offset));		// Puntatore ai marcatori dei cuts

	if (cam_header.MAGIC1 != 1380271176 || cam_header.MAGIC2 != 1480675924)		// Se il file CAM non è valido
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " is not a valid CAM file.";
		return false;
	}

	msg(msg::TGT::FILE, msg::TYP::LOG) << "Camera " << name << " contains " << cam_header.nFrames << " frames.";

	vector <float> Xfrom(cam_header.nFrames), Yfrom(cam_header.nFrames), Zfrom(cam_header.nFrames);
	vector <float> Xat(cam_header.nFrames), Yat(cam_header.nFrames), Zat(cam_header.nFrames);
	vector <float> Roll(cam_header.nFrames);
	vector <float> Fov(cam_header.nFrames);

	///////////////////    LETTURA VALORI TELECAMERA DAL FILE CAM
	for (unsigned int f = 0; f < cam_header.nFrames; f++)
		camfile.read(reinterpret_cast<char*>(&Xfrom[f]), sizeof(camera_point_from.Data));
	for (unsigned int f = 0; f < cam_header.nFrames; f++)
		camfile.read(reinterpret_cast<char*>(&Yfrom[f]), sizeof(camera_point_from.Data));
	for (unsigned int f = 0; f < cam_header.nFrames; f++)
		camfile.read(reinterpret_cast<char*>(&Zfrom[f]), sizeof(camera_point_from.Data));
	for (unsigned int f = 0; f < cam_header.nFrames; f++)
		camfile.read(reinterpret_cast<char*>(&Xat[f]), sizeof(camera_look_at.Data));
	for (unsigned int f = 0; f < cam_header.nFrames; f++)
		camfile.read(reinterpret_cast<char*>(&Yat[f]), sizeof(camera_look_at.Data));
	for (unsigned int f = 0; f < cam_header.nFrames; f++)
		camfile.read(reinterpret_cast<char*>(&Zat[f]), sizeof(camera_look_at.Data));
	for (unsigned int f = 0; f < cam_header.nFrames; f++)
		camfile.read(reinterpret_cast<char*>(&Roll[f]), sizeof(cam_roll.Data));
	for (unsigned int f = 0; f < cam_header.nFrames; f++)
		camfile.read(reinterpret_cast<char*>(&Fov[f]), sizeof(cam_fov.Data));

	return CAM_BuildAnimatedCamera(name, Xfrom, Yfrom, Zfrom, Xat, Yat, Zat, Roll, Fov, FBX, MA);
}


/*------------------------------------------------------------------------------------------------------------------
Telecamera animata "Camera_<name>" (un key per frame, 30 fps, dal frame 0) a partire dai valori del gioco per frame:
posizione (Xfrom..), target (Xat..), roll (gradi) e FOV (gradi). Usata per i file CAM dei filmati e per le sequenze
animate contenute negli script (Camera_SetScriptCamera, vedi Export_SCX). La rotazione riproduce la vista del gioco
(CAM_GameCameraRotation).
------------------------------------------------------------------------------------------------------------------*/
bool CAM_BuildAnimatedCamera (string name, const vector <float> &Xfrom, const vector <float> &Yfrom, const vector <float> &Zfrom,
							  const vector <float> &Xat, const vector <float> &Yat, const vector <float> &Zat,
							  const vector <float> &Roll, const vector <float> &Fov, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	unsigned int nFrames = (unsigned int)Xfrom.size();
	if (nFrames == 0)
		return false;
	Camera CAM;
	Animation FBX_anim;
	AnimationCurveNode FBX_animCurveNode;
	MA_animCurve MA_animCurve;
	vector <float> Xrot(nFrames), Yrot(nFrames), Zrot(nFrames);
	vector <float> Focal_lenght(nFrames);
	vector <float> COI(nFrames);
	for (unsigned int f = 0; f < nFrames; f++)
		Focal_lenght[f] = 60 / (2 * tan(Fov[f] * 3.141593f / 360));				// Conversione da FOV a lunghezza focale

	///////////////////    CALCOLO VALORI DI ROTAZIONE E SALVATAGGIO ANIMAZIONE
	for (unsigned int f = 0; f < nFrames; f++)
	{
		// Vista del gioco (mathLookAt, vedi CBH_Struct.h): guarda esattamente il target, con l'alto ricavato dall'asse Z del
		// mondo ruotato di Roll gradi attorno all'asse Y del mondo. La vecchia formula Yrot = atan(tan(Roll) * cos(Zrot)),
		// con Xrot = 90 + elevazione e Zrot = direzione, era esatta solo per le telecamere orizzontali
		Vec3 eye(Xfrom[f], Yfrom[f], Zfrom[f]), target(Xat[f], Yat[f], Zat[f]);
		Vec3 prev(f ? Xrot[f - 1] : 0, f ? Yrot[f - 1] : 0, f ? Zrot[f - 1] : 0);
		Vec3 rot = CAM_GameCameraRotation(eye, target, Roll[f], f ? &prev : nullptr);	// Angoli continui tra i frame
		Xrot[f] = rot.x;
		Yrot[f] = rot.y;
		Zrot[f] = rot.z;
		COI[f] = sqrt((Xat[f] - Xfrom[f]) * (Xat[f] - Xfrom[f]) + (Yat[f] - Yfrom[f]) * (Yat[f] - Yfrom[f]) + (Zat[f] - Zfrom[f]) * (Zat[f] - Zfrom[f]));
	

		//////// FBX
		// Scrittura traslazione X
		FBX_animCurveNode.tX.KeyTime.push_back((unsigned long long)f * FBXframe1);			// Tempo
		FBX_animCurveNode.tX.KeyValueFloat.push_back(Xfrom[f]);
		FBX_animCurveNode.tX.KeyAttrFlags.push_back(24840);												// Cubic|TangeantAuto|GenericTimeIndependent|GenericClampProgressive
        FBX_animCurveNode.tX.KeyAttrDataFloat.push_back(0);												// RightSlope:0
        FBX_animCurveNode.tX.KeyAttrDataFloat.push_back(0);												// NextLeftSlope:0
        FBX_animCurveNode.tX.KeyAttrDataFloat.push_back(218434821);										// RightWeight:0.333333, NextLeftWeight:0.333333
        FBX_animCurveNode.tX.KeyAttrDataFloat.push_back(0);												// RightVelocity:0, NextLeftVelocity:0		
		// Scrittura traslazione Y
		FBX_animCurveNode.tY.KeyTime.push_back((unsigned long long)f * FBXframe1);			// Tempo
		FBX_animCurveNode.tY.KeyValueFloat.push_back(Yfrom[f]);
		FBX_animCurveNode.tY.KeyAttrFlags.push_back(24840);												// Cubic|TangeantAuto|GenericTimeIndependent|GenericClampProgressive
        FBX_animCurveNode.tY.KeyAttrDataFloat.push_back(0);												// RightSlope:0
        FBX_animCurveNode.tY.KeyAttrDataFloat.push_back(0);												// NextLeftSlope:0
        FBX_animCurveNode.tY.KeyAttrDataFloat.push_back(218434821);										// RightWeight:0.333333, NextLeftWeight:0.333333
        FBX_animCurveNode.tY.KeyAttrDataFloat.push_back(0);												// RightVelocity:0, NextLeftVelocity:0
		// Scrittura traslazione Z
		FBX_animCurveNode.tZ.KeyTime.push_back((unsigned long long)f * FBXframe1);			// Tempo
		FBX_animCurveNode.tZ.KeyValueFloat.push_back(Zfrom[f]);
		FBX_animCurveNode.tZ.KeyAttrFlags.push_back(24840);												// Cubic|TangeantAuto|GenericTimeIndependent|GenericClampProgressive
        FBX_animCurveNode.tZ.KeyAttrDataFloat.push_back(0);												// RightSlope:0
        FBX_animCurveNode.tZ.KeyAttrDataFloat.push_back(0);												// NextLeftSlope:0
        FBX_animCurveNode.tZ.KeyAttrDataFloat.push_back(218434821);										// RightWeight:0.333333, NextLeftWeight:0.333333
        FBX_animCurveNode.tZ.KeyAttrDataFloat.push_back(0);												// RightVelocity:0, NextLeftVelocity:0
		// Scrittura rotazione X
		FBX_animCurveNode.rX.KeyTime.push_back((unsigned long long)f * FBXframe1);			// Tempo
		FBX_animCurveNode.rX.KeyValueFloat.push_back(Xrot[f]);
		FBX_animCurveNode.rX.KeyAttrFlags.push_back(24840);												// Cubic|TangeantAuto|GenericTimeIndependent|GenericClampProgressive
        FBX_animCurveNode.rX.KeyAttrDataFloat.push_back(0);												// RightSlope:0
        FBX_animCurveNode.rX.KeyAttrDataFloat.push_back(0);												// NextLeftSlope:0
        FBX_animCurveNode.rX.KeyAttrDataFloat.push_back(218434821);										// RightWeight:0.333333, NextLeftWeight:0.333333
        FBX_animCurveNode.rX.KeyAttrDataFloat.push_back(0);												// RightVelocity:0, NextLeftVelocity:0		
		// Scrittura rotazione Y
		FBX_animCurveNode.rY.KeyTime.push_back((unsigned long long)f * FBXframe1);			// Tempo
		FBX_animCurveNode.rY.KeyValueFloat.push_back(Yrot[f]);
		FBX_animCurveNode.rY.KeyAttrFlags.push_back(24840);												// Cubic|TangeantAuto|GenericTimeIndependent|GenericClampProgressive
        FBX_animCurveNode.rY.KeyAttrDataFloat.push_back(0);												// RightSlope:0
        FBX_animCurveNode.rY.KeyAttrDataFloat.push_back(0);												// NextLeftSlope:0
        FBX_animCurveNode.rY.KeyAttrDataFloat.push_back(218434821);										// RightWeight:0.333333, NextLeftWeight:0.333333
        FBX_animCurveNode.rY.KeyAttrDataFloat.push_back(0);												// RightVelocity:0, NextLeftVelocity:0
		// Scrittura rotazione Z
		FBX_animCurveNode.rZ.KeyTime.push_back((unsigned long long)f * FBXframe1);			// Tempo
		FBX_animCurveNode.rZ.KeyValueFloat.push_back(Zrot[f]);
		FBX_animCurveNode.rZ.KeyAttrFlags.push_back(24840);												// Cubic|TangeantAuto|GenericTimeIndependent|GenericClampProgressive
        FBX_animCurveNode.rZ.KeyAttrDataFloat.push_back(0);												// RightSlope:0
        FBX_animCurveNode.rZ.KeyAttrDataFloat.push_back(0);												// NextLeftSlope:0
        FBX_animCurveNode.rZ.KeyAttrDataFloat.push_back(218434821);										// RightWeight:0.333333, NextLeftWeight:0.333333
        FBX_animCurveNode.rZ.KeyAttrDataFloat.push_back(0);												// RightVelocity:0, NextLeftVelocity:0


		//////// MA
		// Scrittura traslazioni
		MA_animCurve.tX.Time.push_back(f);
		MA_animCurve.tX.Value.push_back(Xfrom[f]);
		MA_animCurve.tY.Time.push_back(f);
		MA_animCurve.tY.Value.push_back(Yfrom[f]);
		MA_animCurve.tZ.Time.push_back(f);
		MA_animCurve.tZ.Value.push_back(Zfrom[f]);
		// Scrittura rotazioni
		MA_animCurve.rX.Time.push_back(f);
		MA_animCurve.rX.Value.push_back(Xrot[f]);
		MA_animCurve.rY.Time.push_back(f);
		MA_animCurve.rY.Value.push_back(Yrot[f]);
		MA_animCurve.rZ.Time.push_back(f);
		MA_animCurve.rZ.Value.push_back(Zrot[f]);
		// Scrittura lunghezza focale
		MA_animCurve.fl.Time.push_back(f);
		MA_animCurve.fl.Value.push_back(Focal_lenght[f]);
		// Scrittura center of interest
		MA_animCurve.coi.Time.push_back(f);
		MA_animCurve.coi.Value.push_back(COI[f]);
	}
	
	stringstream temp;
	temp << "Camera_" << name;
	CAM.name = temp.str();
	CAM.translate_flag = true;
	CAM.rotate_flag = true;
	CAM.scale_flag = true;
	CAM.tX = Xfrom[0];
	CAM.tY = Yfrom[0];
	CAM.tZ = Zfrom[0];
	CAM.rX = Xrot[0];
	CAM.rY = Yrot[0];
	CAM.rZ = Zrot[0];
	CAM.sX = 120;
	CAM.sY = 120;
	CAM.sZ = 120;
	CAM.fl = Focal_lenght[0];
	CAM.coi = COI[0];

	// Porzione file FBX
	FBX.Camera.push_back(CAM);
	FBX_animCurveNode.name = "Joint_0";
	FBX_animCurveNode.parent = CAM.name;
	FBX_animCurveNode.FBX_parent = hashID(CAM.name, "Camera");
	FBX_animCurveNode.tX_flag = true;
	FBX_animCurveNode.tY_flag = true;
	FBX_animCurveNode.tZ_flag = true;
	FBX_animCurveNode.rX_flag = true;
	FBX_animCurveNode.rY_flag = true;
	FBX_animCurveNode.rZ_flag = true;

	FBX_anim.name = CAM.name;
	FBX_anim.nJoints = 1;
	FBX_anim.nFrames = nFrames;
	FBX_anim.Curvenode.push_back(FBX_animCurveNode);

	FBX.Animation.push_back(FBX_anim);

	// Porzione file MA
	MA_animCurve.name = temp.str();
	MA_animCurve.parent = temp.str();
	MA_animCurve.nFrames = nFrames;
	MA_animCurve.translateX_flag = true;
	MA_animCurve.translateY_flag = true;
	MA_animCurve.translateZ_flag = true;
	MA_animCurve.rotateX_flag = true;
	MA_animCurve.rotateY_flag = true;
	MA_animCurve.rotateZ_flag = true;
	MA_animCurve.focalLength_flag = true;
	MA_animCurve.centerOfInterest_flag = true;

	MA.Camera.push_back(CAM);
	MA.Animation.resize(MA.Animation.size() + 1);
	MA.Animation[MA.Animation.size() - 1].push_back(MA_animCurve);
	
	return true;
}