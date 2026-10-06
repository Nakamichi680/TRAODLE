#pragma once
#include <map>
#include "TRAOD/CAL/CAL_Struct.h"
#include "TRAOD/CAL/POS_Struct.h"


class CAL_ActorPositions {					// Posizioni del joint radice di un attore in un filmato (file POS)
public:
	string posfile;							// File POS di provenienza
	string actor;							// Nome dell'attore ("__lara_1")
	string tms;								// File TMS della sequenza facciale dell'attore (vuoto se assente)
	vector <POS_FRAME> frames;				// Un record per frame dell'animazione
};


vector <float> CAL_DecodeTrack (const uint8_t *data, size_t available, unsigned int nFrames);	// Valori grezzi di una traccia a blocchi
map <uint32_t, CAL_ActorPositions> CAL_Read_Positions ();										// File POS del livello, per hash dell'animazione
bool Export_CAL (string filename);																// Esportazione delle animazioni scheletriche
