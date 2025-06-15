/*------------------------------------------------------------------------------------------------------------------
Ruota una matrice attorno ad un asse
INPUT: MATRIX matrix, VECTOR axis, float ang
OUTPUT: MATRIX
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Classes.h"
#include "MATH/math.h"

MATRIX mathRotAxis (MATRIX matrix, VECTOR axis, float ang_rad)
{
	float fCos = cos(ang_rad);
	float fSin = sin(ang_rad);
	float fX, fY, fZ;
	MATRIX MRotate;

	float norm = sqrt(pow(axis.x, 2) + pow(axis.y, 2) + pow(axis.z, 2));
	if (norm == 0)
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Division by zero.";
		norm = 1;
	}
	fX = axis.x / norm;
	fY = axis.y / norm;
	fZ = axis.z / norm;

	MRotate.m00 = (fX * fX) * (1.0f - fCos) + fCos;
	MRotate.m01 = (fX * fY) * (1.0f - fCos) - (fZ * fSin);
	MRotate.m02 = (fX * fZ) * (1.0f - fCos) + (fY * fSin);

	MRotate.m10 = (fY * fX) * (1.0f - fCos) + (fZ * fSin);
	MRotate.m11 = (fY * fY) * (1.0f - fCos) + fCos;
	MRotate.m12 = (fY * fZ) * (1.0f - fCos) - (fX * fSin);

	MRotate.m20 = (fZ * fX) * (1.0f - fCos) - (fY * fSin);
	MRotate.m21 = (fZ * fY) * (1.0f - fCos) + (fX * fSin);
	MRotate.m22 = (fZ * fZ) * (1.0f - fCos) + fCos;

	MRotate.m03 = MRotate.m13 = MRotate.m23 = 0.0f;
	MRotate.m30 = MRotate.m31 = MRotate.m32 = 0.0f;
	MRotate.m33 = 1.0f;

	return mathMulMatrices(MRotate, matrix);
}