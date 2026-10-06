#pragma once


/*==================================================================================================================
	FORMATO POS - Tomb Raider: The Angel of Darkness (posizioni degli attori nei filmati CS_* / IG_*)

	Fonti:
		[SRC]	esportatore Maya originale (TRAOD\CHR\Reference_TRAODAE\Tools\Exporter\Export_Animation.cpp:
				chunk "CPOS", ExportCharCutscenePositions, SampleCutCurve1)
		[OK]	verificato su tutti i file POS di PARIS1B

	Chunk unico "CPOS" (POS_HEADER), seguito da nActors blocchi a lunghezza variabile:
		uint32_t nameLen						Lunghezza della stringa arrotondata a multiplo di 4 (ChunkWriteString)
		char name[nameLen]						"__" + nome del nodo ORIGIN_<ATTORE> in minuscolo (es. "__lara_1")
		uint32_t nameHash						STRING_GetHashValue(name)
		uint32_t animHash						STRING_GetHashValue("<ATTORE>_<CUTSCENE>") in maiuscolo: e' CAL_ANIMATION.animID
												dell'animazione dell'attore nel CAL <PERSONAGGIO>_<CUTSCENE>.CAL [OK]
		uint32_t tmsHash						Hash della sequenza di morph facciale ("<PERSONAGGIO>_<CUTSCENE>"), 0 se assente
		uint32_t tmsLen							Lunghezza (arrotondata a 4) del nome del file TMS, 0 se assente
		char tms[tmsLen]						Nome del file TMS facciale (es. "LARA_IG_1_24.TMS")
		uint32_t nFrames						Ultimo frame (ef - bf della scena Maya)
		POS_FRAME frame[nFrames + 1]			Un record per frame, da bf a ef compreso (= CAL_ANIMATION.nFrames) [OK]

	Ogni record e' la posizione nel mondo del joint radice (HIP) dell'attore: traslazione dell'HIP ruotata con la
	rotazione Z del nodo ORIGIN dell'attore, piu' la traslazione del nodo ORIGIN e l'offset del nodo ORIGIN del filmato.
	Nei CAL dei filmati la traslazione dell'HIP e' nulla (bind + delta ~ 0): il gioco usa quella del POS [OK].
	La rotazione dei nodi ORIGIN non viene salvata (la rotazione dell'HIP nel CAL e' relativa al nodo ORIGIN).
==================================================================================================================*/


#define POS_MAGIC	0x534F5043				// "CPOS"


struct POS_HEADER				// SIZE: 12 bytes
{
	uint32_t Magic;						// [SRC] "CPOS"
	uint32_t Size;						// [OK] Dimensione dei dati che seguono questo campo
	uint32_t nActors;					// [SRC] Numero di attori
};
static_assert(sizeof(POS_HEADER) == 12, "POS_HEADER deve essere 12 bytes");


struct POS_FRAME				// SIZE: 16 bytes
{
	float x, y, z;						// [SRC] Posizione nel mondo del joint radice (HIP)
	float visibility;					// [SRC] Visibilita' del nodo ORIGIN dell'attore (1 se non animata)
};
static_assert(sizeof(POS_FRAME) == 16, "POS_FRAME deve essere 16 bytes");
