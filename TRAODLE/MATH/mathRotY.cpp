/*------------------------------------------------------------------------------------------------------------------
Ruota la matrice in ingresso attorno l'asse Y
INPUT: MATRIX matrix, float angle (in gradi)
OUTPUT: MATRIX
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Classes.h"
#include "MATH/math.h"


MATRIX mathRotY (MATRIX matrix, float angle_deg)
{
	float angle_rad = angle_deg * 3.141593f / 180;			// Conversione da gradi a radianti
	MATRIX rotMatrix;
	rotMatrix.m00 = cos(angle_rad);		rotMatrix.m01 = 0;			rotMatrix.m02 = -sin(angle_rad);	rotMatrix.m03 = 0;
	rotMatrix.m10 = 0;					rotMatrix.m11 = 1;			rotMatrix.m12 = 0;					rotMatrix.m13 = 0;
	rotMatrix.m20 = sin(angle_rad);		rotMatrix.m21 = 0;			rotMatrix.m22 = cos(angle_rad);		rotMatrix.m23 = 0;
	rotMatrix.m30 = 0;					rotMatrix.m31 = 0;			rotMatrix.m32 = 0;					rotMatrix.m33 = 1;
	return mathMulMatrices(rotMatrix, matrix);
}