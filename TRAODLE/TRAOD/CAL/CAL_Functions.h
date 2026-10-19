#pragma once
#include <map>
#include "TRAOD/CAL/CAL_Struct.h"
#include "TRAOD/CAL/POS_Struct.h"
#include "TRAOD/CAL/XXX_Struct.h"


class CAL_ActorPositions {					// Posizioni del joint radice di un attore in un filmato (file POS)
public:
	string posfile;							// File POS di provenienza
	string actor;							// Nome dell'attore ("__lara_1")
	string tms;								// File TMS della sequenza facciale dell'attore (vuoto se assente)
	vector <POS_FRAME> frames;				// Un record per frame dell'animazione
};


class CAL_PropPositions {					// Posizioni dell'origine di un oggetto animato in un filmato (file XXX "PPOS")
public:
	string xxxfile;							// File XXX di provenienza
	string object;							// Nome dell'oggetto ("OBJECT01")
	string node;							// Nome del nodo Maya ("objects__closed_journal1")
	vector <POS_FRAME> frames;				// Un record per frame: posizione nel mondo e visibilita'
};


vector <float> CAL_DecodeTrack (const uint8_t *data, size_t available, unsigned int nFrames);	// Valori grezzi di una traccia a blocchi
map <uint32_t, CAL_ActorPositions> CAL_Read_Positions ();
map <uint32_t, CAL_PropPositions> CAL_Read_PropPositions ();							// File XXX del livello, per hash dell'animazione										// File POS del livello, per hash dell'animazione
bool Export_CutsceneScenes ();																	// Animations\<FILMATO>.MA: scena unica di ogni filmato (dopo CAL e CAM)
bool Export_CAL (string filename);																// Esportazione delle animazioni scheletriche
