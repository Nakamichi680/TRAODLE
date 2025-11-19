#include "stdafx.h"


bool CheckShadowMap (unsigned short TextureMode)
{
	short special = TextureMode & 0b10000000;			// Contiene solo il flag per materiale speciale
	
	if (TextureMode & 0b10000100)						// Se il materiale è FUR
		return true;

	if (TextureMode & 0b10000 && !special)				// Se il materiale è normale ed il flag per la shadowmap è attivo
		return true;

	return false;
}