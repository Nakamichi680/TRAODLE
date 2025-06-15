#pragma once
#include "MA/MA_Classes.h"

void MA_Export (string output_filename, MA_EXPORT &MA);

void MA_Write_Header (MA_EXPORT &MA);

void MA_Write_Animation (unsigned int a, MA_EXPORT &MA);

string MA_Write_Animation_Subroutine (MA_KeyTimeValue *Data);

void MA_Write_PointLight (unsigned int l, MA_EXPORT &MA);

void MA_Write_Camera (unsigned int c, MA_EXPORT &MA);

void MA_Write_Locator (unsigned int l, MA_EXPORT &MA);

void MA_Write_DefaultNodes (MA_EXPORT &MA);

void MA_Write_Transform (unsigned int t, MA_EXPORT &MA);

void MA_Write_Mesh (Mesh mesh, MA_EXPORT &MA);

void MA_Write_DisplayLayer (unsigned int l, MA_EXPORT &MA);

void MA_Write_NurbsSurface (unsigned int n, MA_EXPORT &MA);

void MA_Write_Texture (unsigned int t, MA_EXPORT &MA);

void MA_Write_Material (unsigned int m, MA_EXPORT &MA);

void MA_Write_Joint (unsigned int j, MA_EXPORT &MA);

void MA_Write_PolyPlane (unsigned int p, MA_EXPORT &MA);

void MA_Write_BossWave (unsigned int b, MA_EXPORT &MA);