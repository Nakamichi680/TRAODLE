/*------------------------------------------------------------------------------------------------------------------
Scambia le righe con le colonne di una matrice
INPUT: MATRIX input
OUTPUT: MATRIX
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Classes.h"


MATRIX mathTransposeAMatrix (MATRIX input)
{
	MATRIX output;
	output.m00 = input.m00;		output.m01 = input.m10;		output.m02 = input.m20;		output.m03 = input.m30;
	output.m10 = input.m01;		output.m11 = input.m11;		output.m12 = input.m21;		output.m13 = input.m31;
	output.m20 = input.m02;		output.m21 = input.m12;		output.m22 = input.m22;		output.m23 = input.m32;
	output.m30 = input.m03;		output.m31 = input.m13;		output.m32 = input.m23;		output.m33 = input.m33;
	return output;
}