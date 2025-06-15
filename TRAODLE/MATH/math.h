#pragma once
#include "Classes.h"


VECTOR mathAxisAngleToQuaternion(XYZ axis, float angle_deg);				// Restituisce un quaternione dati un vettore e l'angolo di rotazione attorno ad esso
MATRIX mathLoadUnitMatrix();												// Crea una matrice unitaria
XYZ mathMatrixGetAngles(MATRIX matrix);										// Restituisce gli angoli in radianti partendo da una matrice
MATRIX mathMatrixLookAt(VECTOR camera, VECTOR target, float roll);			// Restituisce una matrice di rotazione dalla camera al target
XYZ mathMatrixToEuler(MATRIX matrix);										// Restituisce gli angoli euleriani partendo da una matrice
MATRIX mathMulMatrices(MATRIX a, MATRIX b);									// Moltiplica la matrice a per la matrice b
VECTOR mathMulQuaternions(VECTOR a, VECTOR b);								// Moltiplica il quaternione a per il quaternione b
void mathQuatNormalise(VECTOR *quat);										// Normalizza il quaternione
MATRIX mathQuatToMatrix(VECTOR quat);										// Converte il quaternione in matrice
void mathRadToDeg(XYZ *rad);												// Converti i radianti in gradi
VECTOR mathRot(VECTOR input, MATRIX rotMatrix);								// Ruota un vettore secondo una matrice di rotazione
MATRIX mathRotAxis(MATRIX matrix, VECTOR axis, float ang_rad);				// Ruota una matrice attorno ad un asse
MATRIX mathRotX(MATRIX matrix, float angle_deg);							// Ruota la matrice in ingresso attorno l'asse X
MATRIX mathRotY(MATRIX matrix, float angle_deg);							// Ruota la matrice in ingresso attorno l'asse Y
MATRIX mathRotZ(MATRIX matrix, float angle_deg);							// Ruota la matrice in ingresso attorno l'asse Z
VECTOR mathRotTrans(MATRIX matrix, VECTOR vec);
MATRIX mathTranslatef(MATRIX matrix, float x, float y, float z);			// Trasla una matrice
MATRIX mathTransposeAMatrix(MATRIX input);									// Scambia le righe con le colonne di una matrice
XYZ mathVectorGetAngles(VECTOR v);											// Restituisce gli angoli in radianti partendo da un vettore
VECTOR mathVectorAdd(VECTOR a, VECTOR b);									// Effettua l'addizione tra un vettore "a" ed un vettore "b"
VECTOR mathVectorSub(VECTOR a, VECTOR b);									// Effettua la sottrazione tra un vettore "a" ed un vettore "b"
float mathVectorDot(VECTOR a, VECTOR b);									// Effettua il prodotto scalare tra un vettore "a" ed un vettore "b"
VECTOR mathVectorCross(VECTOR a, VECTOR b);									// Effettua il prodotto vettoriale tra un vettore "a" ed un vettore "b"
VECTOR mathVectorNormalise(VECTOR v);										// Normalizza un vettore "v"
void mathWrapAngle180(float *angle);										// Blocca l'angolo euleriano nel range -180/+180 gradi