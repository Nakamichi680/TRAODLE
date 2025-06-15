#include "stdafx.h"


bool CheckDoubleSided (unsigned short TextureMode, unsigned int DoubleSided)
{
	short mtype = TextureMode & 0b1111111111111100;		// Contiene esclusivamente le informazioni sul tipo di materiale

	if (mtype == 0x9C)									// Glass è sempre double sided
		return true;

	if (mtype == 0x84 || (TextureMode - mtype == 0 && !DoubleSided))
		return false;

	return true;
}