/*------------------------------------------------------------------------------------------------------------------
Riduzione delle chiavi di una curva campionata ad ogni frame: restituisce i frames da usare come chiavi con
interpolazione lineare, in modo che la curva ricostruita non si discosti da nessun valore originale piu' di tolerance.
Le chiavi mantengono il valore esatto del frame. Algoritmo a cono di pendenze, lineare nel numero di frames.
INPUT: const vector <float> &v (un valore per frame), float tolerance
OUTPUT: vector <unsigned int> (frames chiave, sempre compresi il primo e l'ultimo)
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "MATH/math.h"


vector <unsigned int> mathCurveKeys (const vector <float> &v, float tolerance)
{
	vector <unsigned int> keys;
	if (v.empty())
		return keys;
	keys.push_back(0);
	unsigned int k = 0;									// Ultima chiave
	double lo = -DBL_MAX, hi = DBL_MAX;					// Pendenze ammesse dai frames intermedi
	for (unsigned int i = 1; i < v.size(); i++)
	{
		double slope = ((double)v[i] - v[k]) / (i - k);
		if (slope < lo || slope > hi)					// La retta fino ad i non passa per tutti i frames intermedi
		{
			k = i - 1;
			keys.push_back(k);
			lo = -DBL_MAX;
			hi = DBL_MAX;
		}
		double d = i - k;
		lo = max(lo, ((double)v[i] - tolerance - v[k]) / d);
		hi = min(hi, ((double)v[i] + tolerance - v[k]) / d);
	}
	if (v.size() > 1)
		keys.push_back((unsigned int)v.size() - 1);
	return keys;
}
