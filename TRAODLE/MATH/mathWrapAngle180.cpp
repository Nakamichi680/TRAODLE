/*------------------------------------------------------------------------------------------------------------------
Blocca l'angolo euleriano nel range -180/+180 gradi
INPUT: float *angle
OUTPUT: float *angle
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"


void mathWrapAngle180 (float *angle)
{
	*angle = (float)fmod(*angle + 180, 360);
	if (*angle < 0)
		*angle += 360;
	*angle -= 180;
}