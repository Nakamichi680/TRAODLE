/*------------------------------------------------------------------------------------------------------------------
Crea una matrice unitaria
INPUT: 
OUTPUT: MATRIX
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Classes.h"


MATRIX mathLoadUnitMatrix()
{
	MATRIX output;
	output.m00 = 1;
	output.m01 = 0;
	output.m02 = 0;
	output.m03 = 0;
	output.m10 = 0;
	output.m11 = 1;
	output.m12 = 0;
	output.m13 = 0;
	output.m20 = 0;
	output.m21 = 0;
	output.m22 = 1;
	output.m23 = 0;
	output.m30 = 0;
	output.m31 = 0;
	output.m32 = 0;
	output.m33 = 1;
	return output;
}