#include "stdafx.h"


bool CheckShadowMap (unsigned short TextureMode)
{
	short special = TextureMode & 0b10000000;			// Contiene solo il flag per materiale speciale
	 
	if (TextureMode & 0b10000 && !special)
		return true;

	return false;
}