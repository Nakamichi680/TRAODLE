#pragma once
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"

bool Export_CAM (string filename);

bool CAM_Read (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA);