#include "stdafx.h"
#include "Misc_Functions.h"
#include "MA/MA_Functions.h"
#include "FBX/FBX_Functions.h"
#include "TRAOD/CAM/CAM_Functions.h"
#include "TRAOD/CAM/CBH_Struct.h"
#include "TRAOD/RMX/RMX_Functions.h"


/*------------------------------------------------------------------------------------------------------------------
Rotazione (gradi, ordine XYZ di Maya) di una telecamera Maya che riproduce la vista del gioco (mathLookAt, vedi
CBH_Struct.h): guarda esattamente il target e ha come alto la componente perpendicolare alla vista del vettore
(sin roll, 0, cos roll), cioe' l'asse Z del mondo ruotato di roll gradi attorno all'asse Y del mondo.
Le telecamere Maya guardano lungo -Z locale con l'alto lungo +Y locale. Con prev diverso da nullptr gli angoli vengono
scelti vicini a quelli del frame precedente (curve di animazione continue).
------------------------------------------------------------------------------------------------------------------*/
Vec3 CAM_GameCameraRotation (Vec3 eye, Vec3 target, float roll, const Vec3 *prev)
{
	auto Cross = [](const Vec3 &a, const Vec3 &b) { return Vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); };
	auto Normalise = [](const Vec3 &a) { float l = sqrt(a.x * a.x + a.y * a.y + a.z * a.z); return l > 1e-12f ? a / l : Vec3(); };
	auto Length = [](const Vec3 &a) { return sqrt(a.x * a.x + a.y * a.y + a.z * a.z); };
	const double DEG = 180.0 / M_PI;
	double r = roll / DEG;
	Vec3 up((float)sin(r), 0, (float)cos(r));
	Vec3 z = Normalise(eye - target);							// -Z locale = direzione di vista
	if (Length(z) == 0)
		z = Vec3(0, -1, 0);
	Vec3 x = Cross(up, z);
	if (Length(x) < 1e-6f)										// Vista parallela al vettore alto (nel gioco: matrice degenere)
		x = Cross(Vec3(0, 1, 0), z);
	x = Normalise(x);
	Vec3 y = Cross(z, x);
	// Matrice di rotazione con colonne x, y, z = Rz * Ry * Rx (ordine XYZ di Maya)
	Vec3 out;
	double sy = -x.z;											// R[riga][colonna]: R[i][0] = x[i]; R[2][0] = -sin(ry)
	if (fabs(sy) < 0.999999)
	{
		out.x = (float)(atan2(y.z, z.z) * DEG);
		out.y = (float)(asin(sy) * DEG);
		out.z = (float)(atan2(x.y, x.x) * DEG);
	}
	else
	{
		out.x = 0;
		out.y = (float)(sy > 0 ? 90.0 : -90.0);
		out.z = (float)(atan2(-y.x, y.y) * DEG);
	}
	if (prev)
	{
		Vec3 alt(out.x + 180, 180 - out.y, out.z + 180);		// Stessa rotazione con angoli diversi
		auto Near = [](float a, float ref) { while (a - ref > 180) a -= 360; while (a - ref < -180) a += 360; return a; };
		Vec3 a(Near(out.x, prev->x), Near(out.y, prev->y), Near(out.z, prev->z));
		Vec3 b(Near(alt.x, prev->x), Near(alt.y, prev->y), Near(alt.z, prev->z));
		float da = fabs(a.x - prev->x) + fabs(a.y - prev->y) + fabs(a.z - prev->z);
		float db = fabs(b.x - prev->x) + fabs(b.y - prev->y) + fabs(b.z - prev->z);
		out = da <= db ? a : b;
	}
	return out;
}


/*------------------------------------------------------------------------------------------------------------------
Esporta le telecamere di gioco del livello (file <LIVELLO>.CBH, vedi CBH_Struct.h) nella cartella \NOMELIVELLO\Cameras:
<LIVELLO>_GAME_CAMERAS.MA / .FBX. Ogni telecamera e' un gruppo <LIVELLO>_GCAM_<indice>[_<trigger>] nella posizione e
con la rotazione della vista del gioco (CAM_GameCameraRotation), con la telecamera come figlio e gli attributi
cameraIndex, trackingMode (0 = guarda Lara, 1 = fissa), trigger, roll, fov, target, railTarget e nextCamera; il target
e' anche un locator. Le rotaie sono gruppi <LIVELLO>_RAIL_<indice> con la spezzata delle posizioni delle loro
telecamere e gli attributi railIndex, railType, trigger e cameras.
------------------------------------------------------------------------------------------------------------------*/
bool Export_CBH (string filename)
{
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Reading " << filename;
	SetCurrentDirectory(AOD_IO.folder_level_lpwstr);			// \NOMELIVELLO
	vector <uint8_t> data;
	{
		ifstream cbhfile(filename, std::ios::binary);
		if (!cbhfile.is_open())
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " not found.";
			return false;
		}
		data.assign(istreambuf_iterator<char>(cbhfile), istreambuf_iterator<char>());
	}
	CBH_HEADER header;
	if (data.size() < CBH_CAMERAS_PTR + sizeof(CBH_CAMERA) * CBH_MAX_CAMERAS)
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " is not a valid CBH file (" << data.size() << " bytes).";
		return false;
	}
	memcpy(&header, data.data(), sizeof(header));
	if (header.NumCameras > CBH_MAX_CAMERAS || header.NumRails > CBH_MAX_RAILS)
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " is not a valid CBH file (" << header.NumCameras << " cameras, " << header.NumRails << " rails).";
		return false;
	}
	if (header.Version != 2)
		msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << filename << ": version " << header.Version << " (expected 2).";
	vector <CBH_CAMERA> cameras(header.NumCameras);
	vector <CBH_RAIL> rails(header.NumRails);
	if (!cameras.empty())
		memcpy(cameras.data(), &data[CBH_CAMERAS_PTR], sizeof(CBH_CAMERA) * cameras.size());
	if (!rails.empty())
		memcpy(rails.data(), &data[CBH_RAILS_PTR], sizeof(CBH_RAIL) * rails.size());

	FBX_EXPORT FBX;
	MA_EXPORT MA;
	string root = AOD_IO.levelname + "_GAME_CAMERAS";
	Layer layer;
	layer.name = AOD_IO.levelname + "_game_cameras";
	layer.Label_ARGB = 0xFF4080FF;
	MA.Layer.push_back(layer);
	Transform group;
	group.name = root;
	FBX.Group.push_back(group);
	MA.Transform.push_back(group);

	auto Label = [](uint32_t hash) { return hash ? RMX_NodeLabel(hash) : string(); };
	auto MayaName = [](string s) { for (char &c : s) if (!isalnum((unsigned char)c) && c != '_') c = '_'; return s; };
	vector <string> camera_names(cameras.size());
	unsigned int exported = 0;
	for (unsigned int i = 0; i < cameras.size(); i++)
	{
		const CBH_CAMERA &c = cameras[i];
		Vec3 eye(c.EyePos[0], c.EyePos[1], c.EyePos[2]), target(c.Target[0], c.Target[1], c.Target[2]);
		if (eye.x == 0 && eye.y == 0 && eye.z == 0 && target.x == 0 && target.y == 0 && target.z == 0)
			continue;											// Voce vuota
		string trigger = Label(c.TriggerHashID);
		string name = AOD_IO.levelname + "_GCAM_" + to_string(i);
		if (!trigger.empty() && trigger.compare(0, 2, "0x") != 0)
			name += "_" + MayaName(trigger);
		camera_names[i] = name;

		Vec3 rot = CAM_GameCameraRotation(eye, target, c.Roll, nullptr);
		Transform cam_group;
		cam_group.name = name;
		cam_group.parent = root;
		cam_group.FBX_parent = hashID(root, "Group");
		cam_group.layer = layer.name;
		cam_group.translate_flag = cam_group.rotate_flag = true;
		cam_group.tX = eye.x;	cam_group.tY = eye.y;	cam_group.tZ = eye.z;
		cam_group.rX = rot.x;	cam_group.rY = rot.y;	cam_group.rZ = rot.z;
		cam_group.int_attributes.push_back(make_pair(string("cameraIndex"), (int)i));
		cam_group.int_attributes.push_back(make_pair(string("trackingMode"), (int)c.TrackingMode));
		if (!trigger.empty())
			cam_group.string_attributes.push_back(make_pair(string("trigger"), trigger));
		cam_group.float_attributes.push_back(make_pair(string("roll"), c.Roll));
		cam_group.float_attributes.push_back(make_pair(string("fov"), c.FOV));
		cam_group.float_attributes.push_back(make_pair(string("targetX"), target.x));
		cam_group.float_attributes.push_back(make_pair(string("targetY"), target.y));
		cam_group.float_attributes.push_back(make_pair(string("targetZ"), target.z));
		bool on_rail = c.RailTarget[0] != 0 || c.RailTarget[1] != 0 || c.RailTarget[2] != 0 || c.NextRailIndex < 0;
		if (on_rail)
		{
			cam_group.float_attributes.push_back(make_pair(string("railTargetX"), c.RailTarget[0]));
			cam_group.float_attributes.push_back(make_pair(string("railTargetY"), c.RailTarget[1]));
			cam_group.float_attributes.push_back(make_pair(string("railTargetZ"), c.RailTarget[2]));
			cam_group.int_attributes.push_back(make_pair(string("nextCamera"), (int)c.NextRailIndex));
		}
		FBX.Group.push_back(cam_group);
		MA.Transform.push_back(cam_group);

		Camera cam;
		cam.name = name + "_Camera";
		cam.parent = name;
		cam.FBX_parent = hashID(name, "Group");
		cam.layer = layer.name;
		cam.translate_flag = cam.rotate_flag = true;			// Nessuno spostamento rispetto al gruppo
		cam.fl = 60 / (2 * tan(c.FOV * 3.141593f / 360));		// Stessa conversione FOV -> lunghezza focale dei file CAM
		Vec3 d = target - eye;
		cam.coi = sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
		FBX.Camera.push_back(cam);
		MA.Camera.push_back(cam);

		Locator loc;											// Punto inquadrato
		loc.name = name + "_Target";
		loc.parent = root;
		loc.FBX_parent = hashID(root, "Group");
		loc.layer = layer.name;
		loc.translate_flag = true;
		loc.tX = target.x;	loc.tY = target.y;	loc.tZ = target.z;
		FBX.Locator.push_back(loc);
		MA.Locator.push_back(loc);
		exported++;
	}

	for (unsigned int k = 0; k < rails.size(); k++)
	{
		const CBH_RAIL &rail = rails[k];
		vector <int> chain;										// Telecamere della rotaia: HeadCamera e le successive
		for (int c = rail.HeadCamera; c >= 0 && c < (int)cameras.size() && chain.size() < rail.CameraCount; c = cameras[c].NextRailIndex)
		{
			if (find(chain.begin(), chain.end(), c) != chain.end())
				break;
			chain.push_back(c);
		}
		if (chain.size() != rail.CameraCount)
			msg(msg::TGT::FILE, msg::TYP::WARN) << filename << ": rail " << k << " has " << chain.size() << " cameras instead of " << rail.CameraCount << ".";
		Transform rail_group;
		rail_group.name = AOD_IO.levelname + "_RAIL_" + to_string(k);
		rail_group.parent = root;
		rail_group.FBX_parent = hashID(root, "Group");
		rail_group.layer = layer.name;
		rail_group.int_attributes.push_back(make_pair(string("railIndex"), (int)k));
		rail_group.int_attributes.push_back(make_pair(string("railType"), (int)rail.RailType));
		if (rail.TriggerHashID)
			rail_group.string_attributes.push_back(make_pair(string("trigger"), Label(rail.TriggerHashID)));
		string list;
		for (int c : chain)
			list += (list.empty() ? "" : " ") + to_string(c);
		rail_group.string_attributes.push_back(make_pair(string("cameras"), list));
		FBX.Group.push_back(rail_group);
		MA.Transform.push_back(rail_group);
		if (chain.size() >= 2)
		{
			NurbsCurve curve;
			curve.name = rail_group.name + "_Path";
			curve.parent = rail_group.name;
			curve.layer = layer.name;
			for (int c : chain)
				curve.Points.push_back(Vec3(cameras[c].EyePos[0], cameras[c].EyePos[1], cameras[c].EyePos[2]));
			MA.NurbsCurve.push_back(curve);
			for (size_t p = 0; p + 1 < curve.Points.size(); p++)	// FBX: un triangolo sottile per segmento
				FBX.Geometry.push_back(DrawTriangle(curve.name + "_" + to_string(p), rail_group.name, layer.name, curve.Points[p], curve.Points[p + 1], curve.Points[p + 1] + Vec3(0, 0, 1), 0xFF4080FF));
		}
	}

	SetCurrentDirectory(AOD_IO.folder_cameras_lpwstr);			// \NOMELIVELLO\Cameras
	string outname = AOD_IO.levelname + "_GAME_CAMERAS";
	FBX_Export(outname, FBX);
	MA_Export(outname, MA);
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Output: " << exported << " game cameras, " << rails.size() << " rails in " << outname;
	return true;
}
