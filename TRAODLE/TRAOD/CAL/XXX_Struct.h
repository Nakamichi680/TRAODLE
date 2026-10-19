#pragma once
#include "TRAOD/CAL/POS_Struct.h"


/*==================================================================================================================
	FORMATO XXX ("PPOS") - Tomb Raider: The Angel of Darkness (posizioni degli oggetti nei filmati CS_* / IG_*)
	[OK] verificato su tutti i 56 file XXX dei livelli (193 oggetti)

	E' l'equivalente del file POS per gli oggetti animati dei filmati (CHR "NODE" <OGGETTO>_<FILMATO>, es.
	OBJECT01_IG_1_24): per ogni frame la posizione nel mondo dell'origine dell'oggetto e la sua visibilita'. Il CAL
	dell'oggetto anima i suoi nodi rispetto a questa origine.

	STRUTTURA DEL FILE
		XXX_HEADER (12 bytes)
		Per ogni oggetto:
			uint32_t nameLen, char name[nameLen]	Nome dell'oggetto (es. "OBJECT01"), lunghezza arrotondata a 4
			uint32_t nodeLen, char node[nodeLen]	Nome del nodo Maya (es. "objects__closed_journal1")
			uint32_t nodeHash						GetHashValue(node) (191 oggetti su 193)
			uint32_t animHash						CAL_ANIMATION.animID dell'animazione dell'oggetto nel CAL
													<OGGETTO>_<FILMATO>.CAL (191 su 193)
			uint32_t nFrames						Ultimo frame
			POS_FRAME frame[nFrames + 1]			Posizione nel mondo (x, y, z) e visibilita' (0 / 1) per ogni frame
		padding fino a multiplo di 2048 bytes
==================================================================================================================*/


#define XXX_MAGIC	0x534F5050				// "PPOS"

struct XXX_HEADER				// SIZE: 12 bytes
{
	uint32_t Magic;						// [OK] "PPOS"
	uint32_t Size;						// [OK] Dimensione dei dati (header compreso)
	uint32_t nObjects;					// [OK] Numero di oggetti
};
static_assert(sizeof(XXX_HEADER) == 12, "XXX_HEADER deve essere 12 bytes");
