/*------------------------------------------------------------------------------------------------------------------
Restituisce gli angoli euleriani partendo da una matrice
INPUT: MATRIX matrix
OUTPUT: XYZ
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Classes.h"


XYZ mathMatrixToEuler (MATRIX matrix)
{
    XYZ output;
    output.x = atan2(matrix.m12, matrix.m22);
    float temp = sqrt(pow(matrix.m00, 2) + pow(matrix.m01, 2));
    output.y = atan2(-matrix.m02, temp);
    output.z = atan2(matrix.m01, matrix.m00);
    return output;
}