/*------------------------------------------------------------------------------------------------------------------
Restituisce l'hash della stringa in ingresso
INPUT: const char *String
OUTPUT: int Hash
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"


int GetHashValue(const char *String)
{
	const char *v1, *i;
	int Hash, j;
	int v2 = 0;
	v1 = String;
	for (i = String; *i; v2++)
		i++;
	Hash = v2;
	for (j = (v2 >> 6) + 1; v2 > 0; v1++)
	{
		Hash ^= (Hash >> 2) + 32 * Hash + *v1;
		v2 -= j;
	}
	return Hash;
}