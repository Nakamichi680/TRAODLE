#include "stdafx.h"
#include "Classes.h"

vector <Face> Calculate_Faces (vector <unsigned int> strip, unsigned int Begin_position, unsigned int Draw_mode)
{
	vector <Face> out;
	Face temp;
	switch (Draw_mode)
    {
	case(4):
		for (unsigned int i = 0; i < strip.size();)
        {
			temp.v1 = strip[i+2];
			temp.v2 = strip[i+1];
			temp.v3 = strip[i];
            out.push_back(temp);
            i += 3;
        }
		break;
	case(5):
		if (Begin_position % 2 == 0)				// Verifica se il punto d'inizio del frammento di strip è pari o dispari
        {
            for (unsigned int i = 0; i < (strip.size() - 2); i++)
                if (strip[i] != strip[i+1] && strip[i] != strip[i+2] && strip[i+1] != strip[i+2])		// Esclude le facce nulle
                {
                    if (i % 2 == 0)		// Se pari
                    {
						temp.v1 = strip[i+2];
						temp.v2 = strip[i+1];
						temp.v3 = strip[i];
                        out.push_back(temp);
                    }
                    else				// Se dispari
                    {
						temp.v1 = strip[i];
						temp.v2 = strip[i+1];
						temp.v3 = strip[i+2];
                        out.push_back(temp);
                    }
                }
        }
        else
        {
            for (unsigned int i = 0; i < (strip.size() - 2); i++)
                if (strip[i] != strip[i+1] && strip[i] != strip[i+2] && strip[i+1] != strip[i+2])		// Esclude le facce nulle
                {
                    if (i % 2 == 0)		// Se pari
                    {
						temp.v1 = strip[i];
						temp.v2 = strip[i+1];
						temp.v3 = strip[i+2];
                        out.push_back(temp);
                    }
                    else				// Se dispari
                    {
						temp.v1 = strip[i+2];
						temp.v2 = strip[i+1];
						temp.v3 = strip[i];
                        out.push_back(temp);
                    }
                }
        }
        break;
	}
	return out;
}