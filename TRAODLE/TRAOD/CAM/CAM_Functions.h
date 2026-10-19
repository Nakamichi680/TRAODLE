#pragma once
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"

bool Export_CAM (string filename);

bool CAM_Read (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA);

bool CAM_BuildAnimatedCamera (string name, const vector <float> &Xfrom, const vector <float> &Yfrom, const vector <float> &Zfrom,
							  const vector <float> &Xat, const vector <float> &Yat, const vector <float> &Zat,
							  const vector <float> &Roll, const vector <float> &Fov, FBX_EXPORT &FBX, MA_EXPORT &MA);	// Telecamera animata (un key per frame)

bool Export_CBH (string filename);								// Telecamere di gioco del livello (CBH_Struct.h)

Vec3 CAM_GameCameraRotation (Vec3 eye, Vec3 target, float roll, const Vec3 *prev);	// Rotazione Maya (XYZ) della vista del gioco