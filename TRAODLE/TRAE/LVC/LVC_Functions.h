#pragma once
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"

bool Export_LVC (string filename);

bool LVC_Read_Geometry (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA);