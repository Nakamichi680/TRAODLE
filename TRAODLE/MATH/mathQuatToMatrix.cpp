/*------------------------------------------------------------------------------------------------------------------
Converte il quaternione in matrice
INPUT: VECTOR quat
OUTPUT: MATRIX
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Classes.h"


MATRIX mathQuatToMatrix (VECTOR quat)
{
	MATRIX output;
    output.m00 = 1 - 2 * pow(quat.x, 2) - 2 * pow(quat.z, 2);
    output.m01 = 2 * quat.x * quat.y - 2 * quat.w * quat.z;
    output.m02 = 2 * quat.x * quat.z + 2 * quat.w * quat.y;
    output.m03 = 0;
    output.m10 = 2 * quat.x * quat.y + 2 * quat.w * quat.z;
    output.m11 = 1 - 2 * pow(quat.x, 2) - 2 * pow(quat.z, 2);
    output.m12 = 2 * quat.x * quat.y - 2 * quat.w * quat.x;
    output.m13 = 0;
    output.m20 = 2 * quat.x * quat.z - 2 * quat.w * quat.y;
    output.m21 = 2 * quat.y * quat.z + 2 * quat.w * quat.x;
    output.m22 = 1 - 2 * pow(quat.x, 2) - 2 * pow(quat.y, 2);
    output.m23 = 0;
    output.m30 = 0;
    output.m31 = 0;
    output.m32 = 0;
    output.m33 = 1;
    return output;

	//MATRIX output;
	//output.m00 = 1 - (2 * pow(quat.y, 2) + 2 * pow(quat.z, 2));
	//output.m01 = 2 * quat.z * quat.w + 2 * quat.x * quat.y;
	//output.m02 = 2 * quat.x * quat.z - 2 * quat.y * quat.w;
	//output.m03 = 0;
	//output.m10 = 2 * quat.x * quat.y - 2 * quat.z * quat.w;
	//output.m11 = 1 - (2 * pow(quat.z, 2) + 2 * pow(quat.x, 2));
	//output.m12 = 2 * quat.x * quat.w + 2 * quat.y * quat.z;
	//output.m13 = 0;
	//output.m20 = 2 * quat.y * quat.w + 2 * quat.x * quat.z;
	//output.m21 = 2 * quat.y * quat.z - 2 * quat.x * quat.w;
	//output.m22 = 1 - (2 * pow(quat.x, 2) + 2 * pow(quat.y, 2));
	//output.m23 = 0;
	//output.m30 = 0;
	//output.m31 = 0;
	//output.m32 = 0;
	//output.m33 = 1;
	//return output;
}