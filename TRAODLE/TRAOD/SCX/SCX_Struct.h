#pragma once


struct SCX_HEADER				// Size: ?? bytes
{
	uint32_t MAGIC;				// 0x02010000
	uint32_t nScriptFiles;		// Numero di file script contenuti nel file SCX
};


struct AMX_HEADER
{
	uint32_t Unknown1;
	uint32_t Unknown2;
	uint32_t Unknown3;
	uint32_t Const1;			// Sempre 0xE0F10606
	uint32_t Const2;			// Sempre 0x04004000
	uint32_t Unknown4;
	uint32_t Unknown5;
	uint32_t Unknown6;
	uint32_t Unknown7;
	uint32_t Const3;			// Sempre 0xFFFFFFFF
	uint32_t Const4;			// Sempre 0x34000000






};