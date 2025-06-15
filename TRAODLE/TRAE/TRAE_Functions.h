#pragma once
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"


bool Export_TRAE (string filename);

bool TRAE_GEOMRAW_Read (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA);