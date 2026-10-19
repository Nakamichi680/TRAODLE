#pragma once


/*==================================================================================================================
	FORMATO CSS - Tomb Raider: The Angel of Darkness (sottotitoli dei filmati CS_* / IG_*)

	Fonti:
		[SRC]	tool originale Vegas EDL Parser (Tools\Vegas EDL Parser\Vegas EDL ParserDlg.cpp, OnButtonSaveAsCsc), che
				legge la lista di montaggio (EDL) di Sony Vegas dell'audio del filmato e salva il "CutScene Subtitle File"
		[OK]	verificato su tutti i 22 file CSS dei livelli (169 sottotitoli)

	STRUTTURA DEL FILE
		CSS_HEADER (16 bytes)
		CSS_LINE x nLines							In ordine di frame di inizio, mai sovrapposti [OK]
		Padding fino a multiplo di 2048 bytes

	TEMPI [SRC]
		Frame a 30 fps dall'inizio del filmato (frame 0 = inizio del filmato, come CAL, POS e camera):
		frame = tempo della timeline di Vegas in ms / 1000 * 30, meno lo sfasamento iniziale (StartOffset) della timeline.
==================================================================================================================*/


#define CSS_MAGIC	0x46435343				// "CSCF"


struct CSS_HEADER				// SIZE: 16 bytes
{
	uint32_t Magic;						// [SRC] "CSCF"
	uint32_t nLines;					// [SRC] Numero di sottotitoli [outputLineCount]
	uint32_t Flags1;					// [SRC][OK] Flag del file, sempre 0 [fileFlags1]
	uint32_t Flags2;					// [SRC][OK] Flag del file, sempre 0 [fileFlags2]
};
static_assert(sizeof(CSS_HEADER) == 16, "CSS_HEADER deve essere 16 bytes");


struct CSS_LINE					// SIZE: 16 bytes
{
	uint32_t StartFrame;				// [SRC][OK] Frame di inizio del sottotitolo (inizio della clip audio della battuta) [frameNo]
	uint32_t nFrames;					// [SRC][OK] Durata in frames (durata della clip audio della battuta) [numFrames]
	uint32_t TextID;					// [SRC] Chiave del testo nel database dei testi del gioco [m_DBID = GDBSentence.m_nKeyText].
										//       0 se il tool non ha trovato la battuta nel database (1 caso: IG_2_1)
	uint32_t Flags;						// [SRC][OK] Flag della riga, mai impostati dal tool: sempre 0 [m_Flags]
};
static_assert(sizeof(CSS_LINE) == 16, "CSS_LINE deve essere 16 bytes");
