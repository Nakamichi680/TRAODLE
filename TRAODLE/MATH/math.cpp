#include "stdafx.h"
#include "Classes.h"



/*------------------------------------------------------------------------------------------------------------------
Restituisce gli angoli in radianti partendo da un vettore
INPUT: VECTOR input
OUTPUT: XYZ angles
------------------------------------------------------------------------------------------------------------------*/
XYZ mathVectorGetAngles(VECTOR v)
{
	XYZ output;
	output.x = atan2(sqrt(v.x * v.x + v.y * v.y), -v.z);
	output.y = 0;
	output.z = atan2(-v.x, -v.y);
	return output;
}



/*------------------------------------------------------------------------------------------------------------------
Effettua l'addizione tra un vettore "a" ed un vettore "b"
INPUT: VECTOR a, VECTOR b
OUTPUT: VECTOR c
------------------------------------------------------------------------------------------------------------------*/
VECTOR mathVectorAdd(VECTOR a, VECTOR b)
{
	VECTOR output;
	output.x = a.x + b.x;
	output.y = a.y + b.y;
	output.z = a.z + b.z;
	output.w = a.w + b.w;
	return output;
}



/*------------------------------------------------------------------------------------------------------------------
Effettua la sottrazione tra un vettore "a" ed un vettore "b"
INPUT: VECTOR a, VECTOR b
OUTPUT: VECTOR c
------------------------------------------------------------------------------------------------------------------*/
VECTOR mathVectorSub(VECTOR a, VECTOR b)
{
	VECTOR output;
	output.x = a.x - b.x;
	output.y = a.y - b.y;
	output.z = a.z - b.z;
	output.w = a.w - b.w;
	return output;
}



/*------------------------------------------------------------------------------------------------------------------
Effettua il prodotto scalare tra un vettore "a" ed un vettore "b"
INPUT: VECTOR a, VECTOR b
OUTPUT: float c
------------------------------------------------------------------------------------------------------------------*/
float mathVectorDot(VECTOR a, VECTOR b)
{
	return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z));
}



/*------------------------------------------------------------------------------------------------------------------
Effettua il prodotto vettoriale tra un vettore "a" ed un vettore "b"
INPUT: VECTOR a, VECTOR b
OUTPUT: VECTOR c
------------------------------------------------------------------------------------------------------------------*/
VECTOR mathVectorCross(VECTOR a, VECTOR b)
{
	VECTOR output;
	output.x = (a.y * b.z - a.z * b.y);
	output.y = (a.z * b.x - a.x * b.z);
	output.z = (a.x * b.y - a.y * b.x);
	return output;
}



/*------------------------------------------------------------------------------------------------------------------
Normalizza un vettore "v"
INPUT: VECTOR v
OUTPUT: VECTOR v
------------------------------------------------------------------------------------------------------------------*/
VECTOR mathVectorNormalise(VECTOR v)
{
	VECTOR output;
	float invmag = 1.0f / sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
	output.x = v.x * invmag;
	output.y = v.y * invmag;
	output.z = v.z * invmag;
	return output;
}