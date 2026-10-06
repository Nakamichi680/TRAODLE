#include "stdafx.h"
#include "TRAOD/CAL/CAL_Functions.h"


/*------------------------------------------------------------------------------------------------------------------
Decodifica una traccia compressa a blocchi (CAL_ANIMBLOCK_HEADER, vedi CAL_Struct.h), usata dalle animazioni CAL e
dalle sequenze di morph (TMS): restituisce il valore grezzo (int16, prima della divisione per la scala) di ogni frame.
Stesso algoritmo di APB_GetTrackValue del runtime originale. In caso di dati non validi i frames mancanti ripetono
l'ultimo valore letto.
INPUT: const uint8_t *data (inizio della traccia), size_t available (bytes leggibili da data), unsigned int nFrames
OUTPUT: vector <float> (nFrames valori)
------------------------------------------------------------------------------------------------------------------*/
vector <float> CAL_DecodeTrack (const uint8_t *data, size_t available, unsigned int nFrames)
{
	vector <float> out;
	out.reserve(nFrames);
	size_t p = 0;
	unsigned int begin = 0;
	float last = 0;
	auto Value = [&](size_t offset) -> float		// int16 all'offset indicato (relativo all'inizio della traccia)
	{
		int16_t v = 0;
		if (offset + 2 <= available)
			memcpy(&v, data + offset, 2);
		return v;
	};
	while (out.size() < nFrames && p + sizeof(CAL_ANIMBLOCK_HEADER) <= available)
	{
		CAL_ANIMBLOCK_HEADER block;
		memcpy(&block, data + p, sizeof(block));
		if (block.size < sizeof(CAL_ANIMBLOCK_HEADER) || block.frame < begin)
			break;
		unsigned int end = block.frame;
		size_t d = p + sizeof(CAL_ANIMBLOCK_HEADER);
		for (unsigned int f = (out.empty() ? begin : begin + 1); f <= end && out.size() < nFrames; f++)
		{
			float v;
			if (block.word & CAL_ANIM_CONST)
				v = Value(d);
			else if (block.word & CAL_ANIM_KEYS)
				v = Value(d + 2 * (f - begin));
			else if (block.word & CAL_ANIM_BEZIER)
			{
				float t = (end > begin) ? (float)(f - begin) / (float)(end - begin) : 0;
				float a, b, c, e;
				if (block.size == 10)						// A = ultimo valore del blocco precedente
				{
					a = (p >= 2) ? Value(p - 2) : 0;
					b = Value(d);	c = Value(d + 2);	e = Value(d + 4);
				}
				else
				{
					a = Value(d);	b = Value(d + 2);	c = Value(d + 4);	e = Value(d + 6);
				}
				float ab = a + (b - a) * t, bc = b + (c - b) * t, ce = c + (e - c) * t;		// de Casteljau (APB_BezierIdx)
				float abbc = ab + (bc - ab) * t, bcce = bc + (ce - bc) * t;
				v = abbc + (bcce - abbc) * t;
			}
			else
				v = last;
			out.push_back(v);
			last = v;
		}
		if (block.word & CAL_ANIM_LASTBLOCK)
			break;
		p += block.size;
		begin = end;
	}
	while (out.size() < nFrames)
		out.push_back(last);
	return out;
}
