#include "stdafx.h"
#include "Classes.h"
#include "MATH/math.h"


VECTOR mathRotTransPers2(MATRIX matrix, VECTOR vec)
{
	VECTOR out;
	float v1, v2, v3, v4;

	v1 = matrix.m00 * vec.x + matrix.m10 * vec.y + matrix.m20 * vec.z + matrix.m30;
	v2 = matrix.m01 * vec.x + matrix.m11 * vec.y + matrix.m21 * vec.z + matrix.m31;
	v3 = matrix.m02 * vec.x + matrix.m12 * vec.y + matrix.m22 * vec.z + matrix.m32;
	v4 = matrix.m03 * vec.x + matrix.m13 * vec.y + matrix.m23 * vec.z + matrix.m33;
	
	out.x = v1 / v4;
	out.y = v2 / v4;
	out.z = v3 / v4;
	out.w = 1 / v4;
	return out;
}