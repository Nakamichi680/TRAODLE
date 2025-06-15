#pragma once


struct GMX_HEADER				// SIZE: 8 bytes
{
	float GMX_Version;
	uint16_t nFiles;			// Number of files
	uint16_t Unknown;
};


struct GMX_LIST					// SIZE: 2040 bytes
{
	uint32_t Filename;			// Hashed filename
	uint32_t Offset;			// Offset from begin of the file + 2048 bytes
	uint32_t Size;				// Size of the file in bytes
};