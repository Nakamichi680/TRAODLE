#pragma once
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "Classes.h"

bool Export_CLN (string filename);

bool CLN_Read (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA);

void CLN_Get_Triangle(ifstream &clnfile, XYZ &v0, XYZ &v1, XYZ &v2, unsigned int &MissingAxis, unsigned int &Unknown);