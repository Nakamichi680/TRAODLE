/*------------------------------------------------------------------------------------------------------------------
Restituisce una matrice di rotazione dalla camera al target
INPUT: VECTOR camera, VECTOR target, float twist
OUTPUT: MATRIX
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Classes.h"
#include "MATH/math.h"


MATRIX mathMatrixLookAt (VECTOR camera, VECTOR target, float roll)
{
	VECTOR delta;
	delta.x = -(camera.x - target.x);
	delta.y = -(camera.y - target.y);
	delta.z = -(camera.z - target.z);
	XYZ angle = mathVectorGetAngles(delta);
	mathRadToDeg(&angle);
	MATRIX unitmatrix, rotmat = mathLoadUnitMatrix();
	//unitmatrix.m00 = -1;
	//unitmatrix.m11 = 1;
	//unitmatrix.m22 = -1;
	//float rotazione = ((90.0) / 180.0) * 3.14;
	//cout << rotazione << " ";
	//unitmatrix = mathRotAxis(unitmatrix, delta, rotazione);
	roll = atan(sin(roll) / (cos(angle.z) * cos(roll) * cos(angle.x)));
	//unitmatrix = mathRotAxis(unitmatrix, delta, roll);
	unitmatrix = mathRotZ(unitmatrix, angle.z);
	
	unitmatrix = mathRotX(unitmatrix, angle.x);
	
	//unitmatrix = mathRotY(unitmatrix, roll);
	//rotmat.m00 = -1;
	//rotmat.m11 = 1;
	//rotmat.m22 = -1;
	//unitmatrix = mathMulMatrices(unitmatrix, rotmat);
	return mathTranslatef(unitmatrix, -camera.x, -camera.y, -camera.z);
}