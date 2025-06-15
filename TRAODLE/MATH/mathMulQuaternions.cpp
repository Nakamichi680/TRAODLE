/*------------------------------------------------------------------------------------------------------------------
Moltiplica due quaternioni
INPUT: Quaternion a, Quaternion b
OUTPUT: Quaternion
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Classes.h"


VECTOR mathMulQuaternions (VECTOR a, VECTOR b)
{
	VECTOR c;
	c.x = b.w * a.x + b.x * a.w - b.y * a.z + b.z * a.y;
	c.y = b.w * a.y + b.x * a.z + b.y * a.w - b.z * a.x;
	c.z = b.w * a.z - b.x * a.y + b.y * a.x + b.z * a.w;
	c.w = b.w * a.w - b.x * a.x - b.y * a.y - b.z * a.z;
	return c;
}