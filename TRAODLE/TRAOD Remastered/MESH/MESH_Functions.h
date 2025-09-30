#pragma once
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"

bool Export_MESH (string filename);

bool MESH_Read_Object (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA);

vector <Face> Calculate_Faces (vector <unsigned int> strip, unsigned int Begin_position, unsigned int Draw_mode);