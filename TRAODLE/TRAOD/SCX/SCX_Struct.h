#pragma once


struct SCX_HEADER				// Size: 8 bytes
{
	uint32_t MAGIC;				// Sempre 0x00000102
	uint32_t nScriptFiles;		// Numero di file script contenuti nel file SCX
};


struct SCX_ENTRY				// Size: 8 bytes. Ripetuto nScriptFiles volte. Ogni entry inizia ad un offset allineato a 4 bytes
{
	uint32_t Hash;				// Hash dello script (probabilmente del nome)
	uint32_t Size;				// Dimensione del blocco dati. Il blocco inizia al primo offset allineato a 16 bytes dopo l'entry e contiene
								// un file AMX completo (AMX_HEADER.size bytes) seguito da AMX_HEADER.stp bytes di dati spazzatura: il builder
								// originale scriveva anche il buffer di memoria della VM senza azzerarlo (contiene residui di altri file).
								// Size e' sempre uguale a AMX_HEADER.size + AMX_HEADER.stp. Anche la coda del file SCX contiene spazzatura
};


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
