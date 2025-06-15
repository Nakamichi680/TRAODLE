#pragma once


struct TEXSET_HEADER			// Size: 36 bytes
{
	uint16_t Magic1;				// 9 = TXS
	uint16_t TargetPlatform;		// 100h = PC, 200h = PS, 1000h = PSP
	uint32_t Magic2;				// "TXS"
	uint32_t PtrToMaterialsHeader;	// Pointer to materials header
	uint32_t PtrToTextureData;		// Pointer to the beginning of the first texture
	uint32_t PtrToMaterialsIndices;	// This block is at the beginning of the file
	uint32_t PtrToTextureInfo;		// This block contains the following information: x size, y size, texture name and texture offset
	uint32_t PtrToIndices2;			// This block is towards the end of the file and it's identical to Indices1 block
	uint32_t PtrToSomething4;
	uint32_t nTextures;				// Number of textures inside the file
};


struct TEXSET_OFFSET
{
	uint32_t hash;					// Hash of texture's name
	uint32_t Texture_offset;		// Offset from PtrToTextureData
};


struct TEXSET_INDICES1_LIST
{
	uint32_t nIndices1;				// Number of indices inside the file
};


struct TEXSET_INDEX1
{
	uint32_t hash;					// Hash of texture's name
	uint32_t Texture_ID;			// Texture's index
};



struct TEXSET_TEXTURE_DATA
{
	uint16_t Xsize;					// 2 ^ Xsize = texture X size in pixels
	uint16_t Ysize;					// 2 ^ Ysize = texture Y size in pixels
	uint32_t MaterialType;			// Possible values: 41h, 51h, 4441h, 4041h, 451h
	uint32_t Texture_hash;			// Maybe hash but different from other hashes in the file
	uint32_t DEADBEAF;				// Hexspeak "DEADBEAF", useless
	char Texture_name[16];			// ASCII name of the texture
};


struct TEXSET_TEXTURE_INFO
{
	uint16_t Xsize;					// 2 ^ Xsize = texture X size in pixels
	uint16_t Ysize;					// 2 ^ Ysize = texture Y size in pixels
	uint32_t Padding;				// Always 0
	uint16_t Texture_ID;
	uint16_t MaterialType;			// Possible values: 41h, 51h, 4441h, 4041h, 451h
	uint32_t Texture_hash;			// Maybe hash but different from other hashes in the file
	char Texture_name[16];			// ASCII name of the texture
	uint32_t Texture_offset;		// Offset from PtrToRawData
};

struct TEXSET_INDICES2
{
	uint32_t nIndices2;				// Number of indices inside the file
};


struct TEXSET_INDICES2_LIST
{
	uint32_t hash;					// Hash of texture's name
	uint32_t Texture_ID;			// Texture's index
};