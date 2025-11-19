#include "stdafx.h"


bool CheckFur (unsigned short TextureMode)
{
	short mtype = TextureMode & 0b1111111111111100;		// Contiene esclusivamente le informazioni sul tipo di materiale

	if (mtype == 0b10000100)
		return true;

	return false;
}