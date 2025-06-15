/*------------------------------------------------------------------------------------------------------------------
Restituisce gli angoli in radianti partendo da una matrice
INPUT: MATRIX matrix
OUTPUT: XYZ
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Classes.h"


XYZ mathMatrixGetAngles (MATRIX pMatrix)
{
	XYZ pAngle;
	//float *mat = *pMatrix;
	float angle_x, angle_y, angle_z;
	float tr_x, tr_y;
	float C;
	float D;

	D = asin(pMatrix.m02);        /* Calculate Y-axis angle */
	C = cos(D);
	angle_y = D;

	if (fabs(C) > FLT_EPSILON)             /* Gimball lock? */
	{
		tr_x = pMatrix.m22 / C;           /* No, so get X-axis angle */
		tr_y = pMatrix.m12 / C;

		angle_x = atan2(tr_y, tr_x);

		tr_x = pMatrix.m00 / C;            /* Get Z-axis angle */
		tr_y = pMatrix.m01 / C;

		angle_z = atan2(tr_y, tr_x);
	}
	else                                 /* Gimball lock has occurred */
	{
		angle_x = 0;                      /* Set X-axis angle to zero */

		tr_x = pMatrix.m11;                 /* And calculate Z-axis angle */
		tr_y = pMatrix.m10;

		angle_z = atan2(tr_y, tr_x);
	}

	pAngle.x = angle_x;
	pAngle.y = angle_y;
	pAngle.z = angle_z;
	return pAngle;
}


/*
XYZ mathMatrixGetAngles (MATRIX matrix)
{
	float xmm0, xmm1, xmm2;
	float esp, esp18, esp28;
	esp = esp28 = asin(-matrix.m02);
	float esp20 = cos(esp);
	if (fabs(esp20) <= 1.192093e-07)
		esp18 = xmm0 = atan2(matrix.m10, matrix.m11);
	else
	{
		xmm0 = 1 / esp20;
		xmm2 = matrix.m22;
		xmm1 = matrix.m12;
		xmm2 *= xmm0;
		xmm1 *= xmm0;
		esp18 = atan2(xmm1, xmm2);
		xmm0 = 1 / esp20;
		xmm2 = matrix.m00;
		xmm1 = matrix.m01;
		xmm2 *= xmm0;
		xmm1 *= xmm0;
		xmm0 = atan2(xmm1, xmm2);
	}
	XYZ output;
	output.x = esp18;
	output.y = esp28;
	output.z = xmm0;


   // output.x = atan2(matrix.m12, matrix.m22);
   // float temp = sqrt(pow(matrix.m00, 2) + pow(matrix.m01, 2));
    //output.y = atan2(-matrix.m02, temp);
   // output.z = atan2(matrix.m01, matrix.m00);
    return output;
}*/