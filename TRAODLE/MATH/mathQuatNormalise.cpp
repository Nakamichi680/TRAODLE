/*------------------------------------------------------------------------------------------------------------------
Normalizza il quaternione
INPUT: VECTOR *quat
OUTPUT: VECTOR *quat
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Classes.h"


void mathQuatNormalise (VECTOR *quat)
{
    float norm = sqrt(pow(quat->x, 2) + pow(quat->y, 2) + pow(quat->z, 2) + pow(quat->w, 2));	// Calcola la norma del quaternione
	if (norm == 0)
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Division by zero.";
		return;
	}
    quat->x /= norm;																			// Normalizza la X
    quat->y /= norm;																			// Normalizza la Y
    quat->z /= norm;																			// Normalizza la Z
    quat->w /= norm;																			// Normalizza la W
}