/*------------------------------------------------------------------------------------------------------------------
Trasla una matrice
INPUT: MATRIX matrix, float x, float y, float z
OUTPUT: MATRIX
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Classes.h"
#include "MATH/math.h"


MATRIX mathTranslatef(MATRIX matrix, float x, float y, float z)
{
	MATRIX TranslationM;

	TranslationM.m30 = x;
	TranslationM.m31 = y;
	TranslationM.m32 = z;

	return mathMulMatrices(TranslationM, matrix);
}