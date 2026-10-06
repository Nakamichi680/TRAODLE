#pragma once


/*------------------------------------------------------------------------------------------------------------------
Formato SCX (archivio degli script di un livello). Nomi originali tra parentesi quadre, dallo ScriptProducer
(TRAOD\CHR\Reference_TRAODAE\Tools\ScriptProducer\ScriptProducerDlg.cpp, CScriptProducerDlg::Archive):
	SCX_HEADER
	SCX_ENTRY + script AMX, x nScriptFiles	[scriptRelEntry::Archive]: entry, padding fino a 16 bytes, dati, padding
											fino a 4 bytes. Il padding e' riempito con 0xDEADBEEF
	uint32_t nSymbols						[m_nSymScript]
	SCX_SYMBOL x nSymbols					[scriptSymEntry::Archive]: associa ogni oggetto del livello al suo script
------------------------------------------------------------------------------------------------------------------*/


struct SCX_HEADER				// Size: 8 bytes
{
	uint32_t MAGIC;				// Sempre 0x00000102 [SCRIPT_FILE_FORMAT_VERSION]
	uint32_t nScriptFiles;		// Numero di file script contenuti nel file SCX [m_nRelScript]
};


struct SCX_ENTRY				// Size: 8 bytes. Ripetuto nScriptFiles volte. Ogni entry inizia ad un offset allineato a 4 bytes
{
	uint32_t Hash;				// Hash dello script [scriptRelEntry::Hash], usato da SCX_SYMBOL.Link
	uint32_t Size;				// Dimensione del blocco dati. Il blocco inizia al primo offset allineato a 16 bytes dopo l'entry e contiene
								// un file AMX completo (AMX_HEADER.size bytes) seguito da AMX_HEADER.stp bytes di dati spazzatura: il builder
								// originale scriveva anche il buffer di memoria della VM senza azzerarlo (contiene residui di altri file).
								// Size e' sempre uguale a AMX_HEADER.size + AMX_HEADER.stp.
};


struct SCX_SYMBOL				// Size: 8 bytes. Ripetuto nSymbols volte dopo l'ultimo script [scriptSymEntry]
{
	uint32_t Hash;				// Hash del nome dell'oggetto del livello che esegue lo script (es. LARA, CARVIER)
	uint32_t Link;				// SCX_ENTRY.Hash dello script associato (verificato su tutti i 180 simboli dei livelli)
};								// Dopo la tabella il file e' riempito fino a 2048 bytes (spesso con dati di altri file)


#pragma pack(push, 1)
struct AMX_HEADER				// Size: 52 bytes. Formato Small 2.x (file_version 6). Tutti gli offset sono relativi all'inizio dell'header
{
	int32_t size;				// Dimensione del file AMX (codice e dati compressi se flags contiene AMX_FLAG_COMPACT)
	uint16_t magic;				// Sempre 0xF1E0
	uint8_t file_version;		// Sempre 6
	uint8_t amx_version;		// Sempre 6 (versione minima della macchina virtuale)
	uint16_t flags;				// Sempre 0x0004 (AMX_FLAG_COMPACT: codice e dati compressi)
	uint16_t defsize;			// Dimensione di ogni voce delle tabelle publics/natives/... (sempre 64: indirizzo + nome di 60 caratteri)
	int32_t cod;				// Offset inizio codice
	int32_t dat;				// Offset inizio dati (non compressi)
	int32_t hea;				// Offset inizio heap (= fine dati non compressi)
	int32_t stp;				// Offset cima dello stack (dimensione totale della memoria)
	int32_t cip;				// Indirizzo della funzione main (-1 se assente)
	int32_t publics;			// Offset tabella funzioni pubbliche
	int32_t natives;			// Offset tabella funzioni native
	int32_t libraries;			// Offset tabella librerie
	int32_t pubvars;			// Offset tabella variabili pubbliche
	int32_t tags;				// Offset tabella tags
};
#pragma pack(pop)
