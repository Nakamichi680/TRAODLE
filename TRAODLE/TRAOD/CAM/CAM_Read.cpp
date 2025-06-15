#include "stdafx.h"
#include "MATH/math.h"
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "TRAOD/CAM/CAM_Struct.h"


bool CAM_Read (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	CAM_HEADER cam_header;
	CAM_POINT_FROM camera_point_from;
	CAM_LOOK_AT camera_look_at;
	CAM_ROLL cam_roll;
	CAM_FOV cam_fov;

	Camera CAM;
	Animation FBX_anim;
	AnimationCurveNode FBX_animCurveNode;
	MA_animCurve MA_animCurve;
	

	//ofstream debug;
	//debug.open("camera_debug.txt");


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
	vector <float> Xrot(cam_header.nFrames), Yrot(cam_header.nFrames), Zrot(cam_header.nFrames);
	vector <float> Roll(cam_header.nFrames);
	vector <float> Focal_lenght(cam_header.nFrames);
	vector <float> COI(cam_header.nFrames);

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
	{
		camfile.read(reinterpret_cast<char*>(&cam_fov.Data), sizeof(cam_fov.Data));
		Focal_lenght[f] = 60 / (2 * tan(cam_fov.Data * 3.141593f / 360));				// Conversione da FOV a lunghezza focale
	}

	//debug << name << " camera debug\n\n";

	///////////////////    CALCOLO VALORI DI ROTAZIONE E SALVATAGGIO ANIMAZIONE
	for (unsigned int f = 0; f < cam_header.nFrames; f++)
	{
		COI[f] = sqrt(pow(Zat[f] - Zfrom[f], 2) + pow(Yat[f] - Yfrom[f], 2) + pow(Xat[f] - Xfrom[f], 2));


		msg(msg::TGT::FILE, msg::TYP::DBG) << f << ") FROM: " << Xfrom[f] << " " << Yfrom[f] << " " << Zfrom[f] << "   AT: " << Xat[f] << " " << Yat[f] << " " << Zat[f];
		msg(msg::TGT::FILE, msg::TYP::DBG) << "   ROT: " << Xrot[f] << " " << Yrot[f] << " " << Zrot[f] << "   F.L.: " << Focal_lenght[f] << "   COI: " << COI[f];

		VECTOR vec_cam;
		vec_cam.x = Xat[f] - Xfrom[f];
		vec_cam.y = Yat[f] - Yfrom[f];
		vec_cam.z = Zat[f] - Zfrom[f];
		vec_cam = mathVectorNormalise(vec_cam);
		VECTOR a, b;
		a.x = vec_cam.x;
		b.y = vec_cam.y;

		if (mathVectorDot(a, b) == 0)
			cout << "è uguale a zero" << endl;
		else
			cout << "fslkfncldnfjlksd" << endl;


		XYZ angles = mathVectorGetAngles(vec_cam);
		mathRadToDeg(&angles);
		Xrot[f] = angles.x;
		Yrot[f] = angles.y;
		Zrot[f] = -angles.z + 180;


		/*VECTOR camera, target;
		Xfrom[f] = 51583.77;
		Yfrom[f] = 61876.94;
		Zfrom[f] = 9211.82;
		Xat[f] = 51585.77;
		Yat[f] = 61877.94;
		Zat[f] = 9215.82;

		camera.x = Xfrom[f];
		camera.y = Yfrom[f];
		camera.z = Zfrom[f];
		camera.w = 1;
		target.x = Xat[f];
		target.y = Yat[f];
		target.z = Zat[f];
		target.w = 1;

		//float twist = Yrot[f];
		float twist = 0;
		VECTOR forward = mathVectorSub(camera, target);
		cout << endl << "forward: " << forward.x << " " << forward.y << " " << forward.z << " " << forward.w << endl;
		forward = mathVectorNormalise(forward);
		cout << "forward normalised: " << forward.x << " " << forward.y << " " << forward.z << " " << forward.w << endl;
		VECTOR temp_up = { 0, 0, 1, 1 };
		VECTOR right = mathVectorCross(temp_up, forward);
		cout << "right: " << right.x << " " << right.y << " " << right.z << " " << right.w << endl;
		VECTOR up = mathVectorCross(forward, right);
		cout << "up: " << up.x << " " << up.y << " " << up.z << " " << up.w << endl;
		MATRIX test;
		test.m00 = right.x;		test.m01 = right.y;		test.m02 = right.z;		test.m03 = right.w;
		test.m10 = forward.x;	test.m11 = forward.y;	test.m12 = forward.z;	test.m13 = forward.w;
		test.m20 = up.x;		test.m21 = up.y;		test.m22 = up.z;		test.m23 = up.w;
		test.m30 = camera.x;	test.m31 = camera.y;	test.m32 = camera.z;	test.m33 = camera.w;
		//XYZ angles = mathMatrixGetAngles(test);
		XYZ angles;
		float yaw, roll;
		float pitch = asin(-test.m20);
		if (cos(pitch) > FLT_EPSILON) {
			// Calcolo di roll (rotazione attorno all'asse x) e yaw (rotazione attorno all'asse z)
			roll = atan2(test.m21, test.m22);
			yaw = atan2(test.m10, test.m00);
		}
		else {
			// Caso di gimbal lock: assegnazione fissa di roll e yaw
			roll = 0.0f;
			yaw = atan2(-test.m01, test.m11);
		}

		angles.x = pitch;
		angles.y = roll;
		angles.z = yaw;
		
		
		mathRadToDeg(&angles);
		Xrot[f] = angles.x;
		Yrot[f] = angles.y;
		Zrot[f] = angles.z;

		cout << "angles: " << angles.x << " " << angles.y << " " << angles.z << endl;*/

		

		/*
		VECTOR i, j, k, l;
		VECTOR up = {0, 0, -1, 1};
		MATRIX a = mathLoadUnitMatrix();
		MATRIX b = mathRotY(a, twist);
		VECTOR c = mathRotTrans(b, up);
		k = mathVectorSub(target, camera);
		i = mathVectorCross(up, k);
		i= mathVectorNormalise(i);

		j = mathVectorCross(k, i);
		j = mathVectorNormalise(j);

		k= mathVectorCross(i, j);
		k= mathVectorNormalise(k);

		MATRIX test;
		test.m00 = i.x;		test.m01 = i.y;		test.m02 = i.z;		test.m03 = i.w;
		test.m10 = j.x;		test.m11 = j.y;		test.m12 = j.z;		test.m13 = j.w;
		test.m20 = k.x;		test.m21 = k.y;		test.m22 = k.z;		test.m23 = k.w;
		test.m30 = l.x;		test.m31 = l.y;		test.m32 = l.z;		test.m33 = l.w;

		//test = mathTransposeAMatrix(test);

		XYZ angles = mathMatrixGetAngles(test);
		mathRadToDeg(&angles);
		Xrot[f] = angles.x;
		Yrot[f] = angles.y;
		Zrot[f] = angles.z;
		*/

		/*
		MATRIX test = mathMatrixLookAt(camera, target, Yrot[f]);
		XYZ euler = mathMatrixGetAngles(test);
		mathRadToDeg(&euler);
		Xrot[f] = -euler.x;
		Yrot[f] = euler.y;
		Zrot[f] = 180.0 + euler.z;
		*/


		/*VECTOR delta;
		delta.x = -(camera.x - target.x);
		delta.y = (camera.y - target.y);
		delta.z = -(camera.z - target.z);
		XYZ angle = mathVectorGetAngles(delta);
		mathRadToDeg(&angle);
		Xrot[f] = angle.x ;
		Yrot[f] = angle.y;
		Zrot[f] = angle.z ;*/





		/*MATRIX Yrotation, cameramatrix;
		Yrotation.m00 = 1;
		Yrotation.m01 = 0;
		Yrotation.m02 = 0;
		Yrotation.m03 = 0;
		Yrotation.m10 = 0;
		Yrotation.m11 = 1;
		Yrotation.m12 = 0;
		Yrotation.m13 = 0;
		Yrotation.m20 = 0;
		Yrotation.m21 = 0;
		Yrotation.m22 = 1;
		Yrotation.m23 = 0;
		Yrotation.m30 = 0;
		Yrotation.m31 = 0;
		Yrotation.m32 = 0;
		Yrotation.m33 = 1;
		VECTOR diff;
		diff.x = Xat[f] - Xfrom[f];
		diff.y = Yat[f] - Yfrom[f];
		diff.z = Zat[f] - Zfrom[f];
		diff.w = 0;
		float Yrad = Yrot[f] * 3.14 / 180;
		mathRotY(Yrotation, Yrad);
		VECTOR rotvec;
		rotvec.x = 0;
		rotvec.y = 0;
		rotvec.z = -1;
		rotvec.w = 1;
		rotvec = mathRot(rotvec, Yrotation);
		VECTOR vec2;
		vec2.x = (rotvec.y * diff.z) - (rotvec.z * diff.y);
		vec2.y = (rotvec.z * diff.x) - (rotvec.x * diff.z);
		vec2.z = (rotvec.x * diff.y) - (rotvec.y * diff.x);
		vec2.w = 0;

		cameramatrix.m20 = diff.x;
		cameramatrix.m21 = diff.y;
		cameramatrix.m22 = diff.z;
		cameramatrix.m23 = diff.w;

		cameramatrix.m00 = vec2.x;
		cameramatrix.m01 = vec2.y;
		cameramatrix.m02 = vec2.z;
		cameramatrix.m03 = vec2.w;

		float xmm2, xmm1, xmm0;

		xmm0 = sqrt(vec2.x * vec2.x + vec2.y * vec2.y + vec2.z * vec2.z);
		xmm1 = 0;
		if (xmm0 == 0)
			xmm1 = 1 / xmm0;*/












		/*VECTOR dir;
		VECTOR dir2;
		int frame = 609;
		dir.x = Xat[f] - Xfrom[f];
		dir.z = Yat[f] - Yfrom[f];
		dir.y = -Zat[f] + Zfrom[f];

		dir2.x = Xat[f] - Xfrom[f];
		dir2.y = Yat[f] - Yfrom[f];
		dir2.z = Zat[f] - Zfrom[f];
		XYZ angles = mathVectorGetAngles(dir2);
		mathRadToDeg(&angles);

		MATRIX testrot;
		testrot = mathRotX(testrot, 180);
		testrot = mathRotY(testrot, Yrot[f]);
		testrot = mathRotX(testrot, angles.x);
		testrot = mathRotZ(testrot, angles.z);
		
		
		
		angles = mathMatrixGetAngles(testrot);*/
		//dir2.x = 1;
		//dir2.y = 0;
		//dir2.z = 0;
		/*if (f == frame)
			msg(msg::TGT::CONS, msg::TYP::DBG) << "dir2: " << dir2.x << "  " << dir2.y << "  " << dir2.z;

		VECTOR quat = mathAxisAngleToQuaternion(dir2, Yrot[f]);
		if (f == frame)
			msg(msg::TGT::CONS, msg::TYP::DBG) << "quat pre:  " << quat.x << "  " << quat.y << "  " << quat.z << " " << quat.w;
		mathQuatNormalise(&quat);
		if (f == frame)
			msg(msg::TGT::CONS, msg::TYP::DBG) << "quat post: " << quat.x << "  " << quat.y << "  " << quat.z << " " << quat.w;

		MATRIX temp = mathQuatToMatrix(quat);*/
		//temp = mathRotZ(temp, -90);
		//MATRIX xrot;
		//xrot = mathRotZ(xrot, 90);
		//temp = mathMulMatrices(xrot, temp);
	//	temp = mathRotY(temp, Yrot[f]);
		
		//XYZ angles = mathMatrixToEuler(temp);
		

		//Xrot[f] = atan2(Zat[f] - Zfrom[f], sqrt(pow(Xat[f] - Xfrom[f],2) + pow(Yat[f] - Yfrom[f],2))) / 3.141593f * 180 + 90;	// Calcolo rotazione X
		//Zrot[f] = -atan2(Xat[f] - Xfrom[f], Yat[f] - Yfrom[f]) / 3.141593f * 180;												// Calcolo rotazione Z
		
		//MATRIX rotMatrix;
		//VECTOR vec {0, 0, -1, 1};
		//rotMatrix = mathQuatToMatrix(vec_pre);
		//rotMatrix.m30 = Xat[f] - Xfrom[f];
		//rotMatrix.m31 = Yat[f] - Yfrom[f];
		//rotMatrix.m32 = Zat[f] - Zfrom[f];
		/*rotMatrix.m11 = 0;
		rotMatrix.m12 = -1;
		rotMatrix.m21 = 1;
		rotMatrix.m22 = 0;*/
				//rotMatrix = mathRotX(rotMatrix, 180);
										/*rotMatrix = mathRotY(rotMatrix, Yrot[f]);
										vec = mathRot(vec, rotMatrix);
										VECTOR vec2, vec3, vec4;
										vec2.x = vec.y * dir.z - vec.z * dir.y;
										vec2.y = vec.z * dir.x - vec.x * dir.z;
										vec2.z = vec.x * dir.y - vec.y * dir.x;
										mathQuatNormalise(&vec2);

										vec3.x = vec2.z * dir.y - vec2.y * dir.z;
										vec3.y = vec2.x * dir.z - vec2.z * dir.x;
										vec3.z = vec2.y * dir.x - vec2.x * dir.y;
										mathQuatNormalise(&vec3);

										vec4.x = vec2.y * vec3.z - vec2.z * vec3.y;
										vec4.y = vec2.z * vec3.x - vec2.x * vec3.z;
										vec4.z = vec2.x * vec3.y - vec2.y * vec3.x;
										mathQuatNormalise(&vec4);
										MATRIX test;
										test.m00 = vec2.x;		test.m01 = vec2.y;		test.m02 = vec2.z;
										test.m10 = vec3.x;		test.m11 = vec3.y;		test.m12 = vec3.z;
										test.m20 = vec4.x;		test.m21 = vec4.y;		test.m22 = vec4.z;
*/
										//test = mathRotX(test, -90);
										//test = mathTransposeAMatrix(test);

		//vec_pre.x = Xat[f] - Xfrom[f];
		//vec_pre.y = Yat[f] - Yfrom[f];
		//vec_pre.z = Zat[f] - Zfrom[f];

		//XYZ angles = mathVectorGetAngles(dir2);
		//angles = mathMatrixToEuler(test);
		//mathRadToDeg(&angles);
		//Xrot[f] = angles.x;
		//Yrot[f] = angles.y;
		//Zrot[f] = angles.z;
		

		//////// FBX
		// Scrittura traslazione X
		FBX_animCurveNode.tX.KeyTime.push_back((unsigned long long)f * FBXframe1 + FBXframe1);			// Tempo
		FBX_animCurveNode.tX.KeyValueFloat.push_back(Xfrom[f]);
		FBX_animCurveNode.tX.KeyAttrFlags.push_back(24840);												// Cubic|TangeantAuto|GenericTimeIndependent|GenericClampProgressive
        FBX_animCurveNode.tX.KeyAttrDataFloat.push_back(0);												// RightSlope:0
        FBX_animCurveNode.tX.KeyAttrDataFloat.push_back(0);												// NextLeftSlope:0
        FBX_animCurveNode.tX.KeyAttrDataFloat.push_back(218434821);										// RightWeight:0.333333, NextLeftWeight:0.333333
        FBX_animCurveNode.tX.KeyAttrDataFloat.push_back(0);												// RightVelocity:0, NextLeftVelocity:0		
		// Scrittura traslazione Y
		FBX_animCurveNode.tY.KeyTime.push_back((unsigned long long)f * FBXframe1 + FBXframe1);			// Tempo
		FBX_animCurveNode.tY.KeyValueFloat.push_back(Yfrom[f]);
		FBX_animCurveNode.tY.KeyAttrFlags.push_back(24840);												// Cubic|TangeantAuto|GenericTimeIndependent|GenericClampProgressive
        FBX_animCurveNode.tY.KeyAttrDataFloat.push_back(0);												// RightSlope:0
        FBX_animCurveNode.tY.KeyAttrDataFloat.push_back(0);												// NextLeftSlope:0
        FBX_animCurveNode.tY.KeyAttrDataFloat.push_back(218434821);										// RightWeight:0.333333, NextLeftWeight:0.333333
        FBX_animCurveNode.tY.KeyAttrDataFloat.push_back(0);												// RightVelocity:0, NextLeftVelocity:0
		// Scrittura traslazione Z
		FBX_animCurveNode.tZ.KeyTime.push_back((unsigned long long)f * FBXframe1 + FBXframe1);			// Tempo
		FBX_animCurveNode.tZ.KeyValueFloat.push_back(Zfrom[f]);
		FBX_animCurveNode.tZ.KeyAttrFlags.push_back(24840);												// Cubic|TangeantAuto|GenericTimeIndependent|GenericClampProgressive
        FBX_animCurveNode.tZ.KeyAttrDataFloat.push_back(0);												// RightSlope:0
        FBX_animCurveNode.tZ.KeyAttrDataFloat.push_back(0);												// NextLeftSlope:0
        FBX_animCurveNode.tZ.KeyAttrDataFloat.push_back(218434821);										// RightWeight:0.333333, NextLeftWeight:0.333333
        FBX_animCurveNode.tZ.KeyAttrDataFloat.push_back(0);												// RightVelocity:0, NextLeftVelocity:0
		// Scrittura rotazione X
		FBX_animCurveNode.rX.KeyTime.push_back((unsigned long long)f * FBXframe1 + FBXframe1);			// Tempo
		FBX_animCurveNode.rX.KeyValueFloat.push_back(Xrot[f]);
		FBX_animCurveNode.rX.KeyAttrFlags.push_back(24840);												// Cubic|TangeantAuto|GenericTimeIndependent|GenericClampProgressive
        FBX_animCurveNode.rX.KeyAttrDataFloat.push_back(0);												// RightSlope:0
        FBX_animCurveNode.rX.KeyAttrDataFloat.push_back(0);												// NextLeftSlope:0
        FBX_animCurveNode.rX.KeyAttrDataFloat.push_back(218434821);										// RightWeight:0.333333, NextLeftWeight:0.333333
        FBX_animCurveNode.rX.KeyAttrDataFloat.push_back(0);												// RightVelocity:0, NextLeftVelocity:0		
		// Scrittura rotazione Y
		FBX_animCurveNode.rY.KeyTime.push_back((unsigned long long)f * FBXframe1 + FBXframe1);			// Tempo
		FBX_animCurveNode.rY.KeyValueFloat.push_back(-Yrot[f]);
		FBX_animCurveNode.rY.KeyAttrFlags.push_back(24840);												// Cubic|TangeantAuto|GenericTimeIndependent|GenericClampProgressive
        FBX_animCurveNode.rY.KeyAttrDataFloat.push_back(0);												// RightSlope:0
        FBX_animCurveNode.rY.KeyAttrDataFloat.push_back(0);												// NextLeftSlope:0
        FBX_animCurveNode.rY.KeyAttrDataFloat.push_back(218434821);										// RightWeight:0.333333, NextLeftWeight:0.333333
        FBX_animCurveNode.rY.KeyAttrDataFloat.push_back(0);												// RightVelocity:0, NextLeftVelocity:0
		// Scrittura rotazione Z
		FBX_animCurveNode.rZ.KeyTime.push_back((unsigned long long)f * FBXframe1 + FBXframe1);			// Tempo
		FBX_animCurveNode.rZ.KeyValueFloat.push_back(Zrot[f]);
		FBX_animCurveNode.rZ.KeyAttrFlags.push_back(24840);												// Cubic|TangeantAuto|GenericTimeIndependent|GenericClampProgressive
        FBX_animCurveNode.rZ.KeyAttrDataFloat.push_back(0);												// RightSlope:0
        FBX_animCurveNode.rZ.KeyAttrDataFloat.push_back(0);												// NextLeftSlope:0
        FBX_animCurveNode.rZ.KeyAttrDataFloat.push_back(218434821);										// RightWeight:0.333333, NextLeftWeight:0.333333
        FBX_animCurveNode.rZ.KeyAttrDataFloat.push_back(0);												// RightVelocity:0, NextLeftVelocity:0


		//////// MA
		// Scrittura traslazioni
		MA_animCurve.tX.Time.push_back(f + 1);
		MA_animCurve.tX.Value.push_back(Xfrom[f]);
		MA_animCurve.tY.Time.push_back(f + 1);
		MA_animCurve.tY.Value.push_back(Yfrom[f]);
		MA_animCurve.tZ.Time.push_back(f + 1);
		MA_animCurve.tZ.Value.push_back(Zfrom[f]);
		// Scrittura rotazioni
		MA_animCurve.rX.Time.push_back(f + 1);
		MA_animCurve.rX.Value.push_back(Xrot[f]);
		MA_animCurve.rY.Time.push_back(f + 1);
		MA_animCurve.rY.Value.push_back(Yrot[f]);
		MA_animCurve.rZ.Time.push_back(f + 1);
		MA_animCurve.rZ.Value.push_back(Zrot[f]);
		// Scrittura lunghezza focale
		MA_animCurve.fl.Time.push_back(f + 1);
		MA_animCurve.fl.Value.push_back(Focal_lenght[f]);
		// Scrittura center of interest
		MA_animCurve.coi.Time.push_back(f + 1);
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
	FBX_anim.nFrames = cam_header.nFrames;
	FBX_anim.Curvenode.push_back(FBX_animCurveNode);

	FBX.Animation.push_back(FBX_anim);

	// Porzione file MA
	MA_animCurve.name = temp.str();
	MA_animCurve.parent = temp.str();
	MA_animCurve.nFrames = cam_header.nFrames;
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