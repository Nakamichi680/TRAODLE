/*------------------------------------------------------------------------------------------------------------------
Restituisce un quaternione dati un vettore e l'angolo di rotazione attorno ad esso
INPUT: XYZ axis, float angle (in gradi)
OUTPUT: VECTOR
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Classes.h"


VECTOR mathAxisAngleToQuaternion (XYZ axis, float angle_deg)
{
	float angle_rad = angle_deg * 3.141593f / 180;			// Conversione da gradi a radianti
	VECTOR output;
	output.x = axis.x * sinf(angle_rad / 2);
	output.y = axis.y * sinf(angle_rad / 2);
	output.z = axis.z * sinf(angle_rad / 2);
    output.z = cosf(angle_rad / 2);
    return output;
}