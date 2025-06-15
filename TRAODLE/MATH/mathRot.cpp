/*------------------------------------------------------------------------------------------------------------------
Ruota un vettore secondo una matrice di rotazione
INPUT: VECTOR input, MATRIX rotMatrix
OUTPUT: VECTOR
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Classes.h"


VECTOR mathRot (VECTOR input, MATRIX rotMatrix)
{
	VECTOR output;
	output.x = rotMatrix.m00 * input.x + rotMatrix.m10 * input.y + rotMatrix.m20 * input.z + rotMatrix.m30;
	output.y = rotMatrix.m01 * input.x + rotMatrix.m11 * input.y + rotMatrix.m21 * input.z + rotMatrix.m31;
	output.z = rotMatrix.m02 * input.x + rotMatrix.m12 * input.y + rotMatrix.m22 * input.z + rotMatrix.m32;
	output.w = rotMatrix.m03 * input.x + rotMatrix.m13 * input.y + rotMatrix.m23 * input.z + rotMatrix.m33;
	return output;
}