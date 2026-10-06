#pragma once


/*------------------------------------------------------------------------------------------------------------------
Formato GMX (archivio dei file di un livello). Nomi originali tra parentesi quadre, dal "GMX Masher"
(TRAOD\CHR\Reference_TRAODAE\Tools\GMX Masher\GMX MasherDlg.h): l'archivio e' un WAD_DIRECTORY di 2048 bytes
seguito dai file, ognuno allineato a 2048 bytes. Il GMX Masher aggiunge in coda l'audio del livello (file .AWD).
------------------------------------------------------------------------------------------------------------------*/


struct GMX_HEADER				// SIZE: 8 bytes [WAD_DIRECTORY]
{
	float GMX_Version;			// Versione [Ver]
	uint16_t nFiles;			// Number of files [nFiles] (al massimo 170)
	uint16_t Unknown;			// [nResident] Numero di file residenti in memoria
};


struct GMX_LIST					// SIZE: 12 bytes, ripetuto nFiles volte [WAD_DIRENTRY]
{
	uint32_t Filename;			// Hashed filename [HashID]: GetHashValue del nome in maiuscolo (es. "PARIS1.AWD")
	uint32_t Offset;			// Offset from begin of the file + 2048 bytes [Offset] (multiplo di 2048)
	uint32_t Size;				// Size of the file in bytes [Length] (arrotondata a 2048 per i file aggiunti dal GMX Masher)
};
