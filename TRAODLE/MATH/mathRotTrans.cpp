#include "stdafx.h"
#include "Classes.h"
#include "MATH/math.h"


VECTOR mathRotTrans(MATRIX matrix, VECTOR vec)
{
	VECTOR output;
	output.x = (matrix.m00 * vec.x) + (matrix.m10 * vec.y) + (matrix.m20 * vec.z) + matrix.m30;
	output.y = (matrix.m01 * vec.x) + (matrix.m11 * vec.y) + (matrix.m21 * vec.z) + matrix.m31;
	output.z = (matrix.m02 * vec.x) + (matrix.m12 * vec.y) + (matrix.m22 * vec.z) + matrix.m32;
	output.w = (matrix.m03 * vec.x) + (matrix.m13 * vec.y) + (matrix.m23 * vec.z) + matrix.m33;
	return output;
}