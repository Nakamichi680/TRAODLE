#pragma once


/*==================================================================================================================
	FORMATO TMS (e ".3") - Tomb Raider: The Angel of Darkness (animazioni facciali: sequenze di morph)

	Fonti: [SRC] struttura MORPH_SEQUENCE / MORPH_SEQUENCE_TRACK e funzione MPH_FrameTick della libreria di animazione
		   (Tools\Libs\AnimLib\AnimAPI.h / AnimAPI.c), CMorphSequence::ExportToGame (Tools\AnimEdit\Morph.cpp);
		   [OK] verificato sui 78 file del tipo TMS dei livelli.

	STRUTTURA DEL FILE
		Uno o piu' blocchi MPHS consecutivi: il blocco successivo inizia a offset blocco + MPHS_HEADER.Size.
			- file .TMS dei filmati (es. LARA_CS_10_8.TMS): un solo blocco, lungo quanto il filmato
			- file ".3" (es. LARA.3, CARVIER.3): un blocco per ogni battuta di dialogo
		padding fino a multiplo di 2048 bytes
	Tutti gli offset (_PTR) sono relativi all'offset 8 del blocco (dopo Magic e Size).

	BLOCCO MPHS
		MPHS_HEADER (40 bytes)
		MPHS_TRACK envelope (a ENVELOPE_PTR)
		MPHS_TRACK x NumTracks (a TRACKS_PTR)
		Tracce compresse a blocchi (CAL_ANIMBLOCK_HEADER, stesso formato delle tracce dei CAL, vedi CAL_Struct.h)

	PESI DEI TARGET [SRC: MPH_FrameTick]
		peso del target t al frame f = traccia[t](f) / 8192 * envelope(f) / 8192
		I target sono quelli del TMT il cui Id e' TargetId (stesso ordine: traccia t = target t del TMT).
		Le animazioni partono dalle chiavi TRACK_KEY_CODE_FIRE_MORPH_TRIGGER dei CAL: il valore della chiave e' l'Id della
		sequenza (MPH_GetMorphSequence). 30 frames al secondo.

	NOMI [OK]
		Le battute di dialogo hanno nomi minuscoli del tipo <2 lettere><numero> (es. "pa211"): l'Id e' il loro hash
		(52 sequenze su 56 risolte). Le sequenze dei filmati prendono il nome dal file.
==================================================================================================================*/


struct MPHS_HEADER				// SIZE: 40 bytes [MORPH_SEQUENCE]
{
	uint32_t Magic;						// [OK] "MPHS"
	uint32_t Size;						// [OK] Dimensione del blocco (header compreso)
	uint32_t Id;						// [SRC][OK] Hash del nome della sequenza (usato da FIRE_MORPH_TRIGGER)
	uint32_t Unknown;					// [?] Uguale a Id nei TMS, numero progressivo della battuta nei file ".3" (0x121, 0x122, ...)
	uint32_t TargetId;					// [SRC][OK] MORPH_MESH.Id del TMT animato (TMT_HEADER.Id)
	uint32_t MorphMesh;					// [SRC] Puntatore runtime, sempre 0
	uint32_t NumFrames;					// [SRC][OK] Numero di frames
	uint32_t ENVELOPE_PTR;				// [SRC][OK] Offset di MPHS_TRACK dell'inviluppo (sempre 32)
	uint32_t NumTracks;					// [SRC][OK] Numero di tracce (= target del TMT, sempre 7)
	uint32_t TRACKS_PTR;				// [SRC][OK] Offset di MPHS_TRACK x NumTracks (sempre 48)
};
static_assert(sizeof(MPHS_HEADER) == 40, "MPHS_HEADER deve essere 40 bytes");


struct MPHS_TRACK				// SIZE: 8 bytes [MORPH_SEQUENCE_TRACK]
{
	uint32_t BLOCKS_PTR;				// [SRC][OK] Offset della traccia compressa [AnimBlock]
	uint32_t Size;						// [SRC][OK] Dimensione della traccia in bytes [AnimSize]
};
static_assert(sizeof(MPHS_TRACK) == 8, "MPHS_TRACK deve essere 8 bytes");


#define MPHS_MAGIC 0x5348504D			// "MPHS"
#define MPHS_WEIGHT_SCALE 8192.0f		// [SRC] Scala dei valori delle tracce e dell'inviluppo
