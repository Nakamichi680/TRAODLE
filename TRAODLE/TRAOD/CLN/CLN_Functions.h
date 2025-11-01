#pragma once
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "Classes.h"

bool Export_CLN (string filename);

bool CLN_Read (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA);

void CLN_Get_Triangle(ifstream &clnfile, Vec3 &v1, Vec3 &v2, Vec3 &v3, unsigned int &MissingAxis, unsigned int &Unknown);

string CLN_GetAttributeName (unsigned int composition, unsigned int mapping, unsigned int bitattrib);

int CLN_GetResource (string attribute_string);