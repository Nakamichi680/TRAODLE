/*------------------------------------------------------------------------------------------------------------------
Scompone una matrice di trasformazione (convenzione a vettore riga: righe 0-2 = assi trasformati, riga 3 =
traslazione) in traslazione, rotazione e scala. La rotazione e' restituita in gradi nell'ordine XYZ usato da Maya e
dall'FBX (rotazione attorno a X, poi Y, poi Z: matrice = Rx * Ry * Rz). Le eventuali deformazioni di taglio vengono
ignorate.
INPUT: MATRIX m
OUTPUT: Vec3 *translation, Vec3 *rotation_deg, Vec3 *scale
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Classes.h"


void mathMatrixDecompose (MATRIX m, Vec3 *translation, Vec3 *rotation_deg, Vec3 *scale)
{
	translation->x = m.m30;
	translation->y = m.m31;
	translation->z = m.m32;

	// Scala: lunghezza di ciascun asse
	scale->x = sqrt(m.m00 * m.m00 + m.m01 * m.m01 + m.m02 * m.m02);
	scale->y = sqrt(m.m10 * m.m10 + m.m11 * m.m11 + m.m12 * m.m12);
	scale->z = sqrt(m.m20 * m.m20 + m.m21 * m.m21 + m.m22 * m.m22);
	float r00 = m.m00 / scale->x, r01 = m.m01 / scale->x, r02 = m.m02 / scale->x;
	float r12 = m.m12 / scale->y, r22 = m.m22 / scale->z, r10 = m.m10 / scale->y, r11 = m.m11 / scale->y;

	// Rotazione: R = Rx(a) * Ry(b) * Rz(c)  ->  r02 = -sin(b), r00 = cos(b)cos(c), r01 = cos(b)sin(c), r12 = sin(a)cos(b), r22 = cos(a)cos(b)
	const float rad2deg = 57.29577951308232f;
	float sb = -r02;
	sb = sb > 1 ? 1 : (sb < -1 ? -1 : sb);
	float b = asin(sb), a, c;
	if (fabs(sb) < 0.99999f)
	{
		a = atan2(r12, r22);
		c = atan2(r01, r00);
	}
	else											// Gimbal lock (b = +-90): rotazioni attorno a X e Z non distinguibili, si azzera Z. Riga 1 di Rx*Ry = (sin(a)sin(b), cos(a), 0)
	{
		c = 0;
		a = atan2(r10 * sb, r11);
	}
	rotation_deg->x = a * rad2deg;
	rotation_deg->y = b * rad2deg;
	rotation_deg->z = c * rad2deg;
}
