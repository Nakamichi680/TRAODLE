#pragma once


struct CLUSTER_HEADER			// Size: 32 bytes
{
	uint16_t Magic1;				// 1 = CLU, 3 = FNT, Eh/13h = SEQ, 24h = AST, 5Ah/5Ch = MDC, 8Fh/93h/96h = LVC
	uint16_t TargetPlatform;		// 100h = PC, 200h = PS, 1000h = PSP
	uint32_t Magic2;				// "CLU"
	uint32_t HeaderSize;
	uint32_t nFiles;				// Number of files inside the cluster file
	uint32_t FilenameHash;
	char Description[12];			// Usually "global clu"
};


struct CLUSTER_FILE_LIST		// Size: 16 bytes
{
	uint32_t FileListEnd;			// Pointer to the end of the the file list
	uint32_t Filesize;				// Size of the file in bytes
	uint32_t Offset;				// Offset of the file inside the cluster file
	uint32_t Hash;					// Hash of the file name
};