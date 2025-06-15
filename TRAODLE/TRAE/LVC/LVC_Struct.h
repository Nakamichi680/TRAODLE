#pragma once


struct LVC_HEADER				// Size: 96 bytes
{
	uint16_t Magic1;				// 8Fh/93h/96h
	uint16_t TargetPlatform;		// 100h = PC, 200h = PS, 1000h = PSP
	uint32_t Magic2;				// "LVC"
	uint32_t Unknown1;
	uint32_t nGeomPointers1;
	uint32_t Unknown2;
	uint32_t Unknown3;
	uint32_t nGeomPointers2;
	char Name[8];					// Usually "all"
	uint32_t PtrToGeomPointers;		// Pointer to the beginning of geometry pointers list
	uint32_t PtrUnknown1;
	uint32_t PtrUnknown2;
	uint32_t PtrUnknown3;
	uint32_t PtrToTEXSET;
	uint32_t PtrToUNKNOWN1;
	uint32_t PtrUnknown6;
	uint32_t PtrUnknown7;
	uint32_t PtrUnknown8;
	uint32_t PtrUnknown9;
	uint32_t PtrUnknown10;
	uint32_t PtrUnknown11;
	uint32_t PtrUnknown12;
	uint32_t PtrUnknown13;
	uint32_t PtrUnknown14;
};


struct LVC_GEOMETRY_POINTER
{
	uint32_t Pointer;
};


struct LVC_GEOMETRY_HEADER
{
	char Unknown1[44];
	uint32_t nMeshes;
	uint32_t Unknown2;
	uint32_t PtrToIndices;			// Pointer to the beginning of the indices list
	uint32_t PtrToEndOfGeometry;	// Pointer to the end of the current geometry block
};


struct LVC_GEOMETRY_MESH_POINTER
{
	uint32_t Pointer;
};


struct LVC_GEOMETRY_INDICES_LIST
{
	uint32_t nIndices;
};


struct LVC_GEOMETRY_INDEX
{
	uint32_t Unknown1;
	uint32_t Unknown2;
};


struct LVC_GEOMETRY_MESH_HEADER
{
	char Unknown1[100];
	uint32_t Material_hash;			// Hash of the linked material
	char Unknown2[192];
	uint32_t nVertices;
	uint32_t Unknown3;
};


struct LVC_GEOMETRY_MESH_VERTEX		// Size: 36 bytes
{
	float X;
	float Y;
	float Z;
	uint8_t VC_red;					// Vertex color R 0-255
    uint8_t VC_green;				// Vertex color G 0-255
    uint8_t VC_blue;				// Vertex color B 0-255
	uint8_t VC_alpha;				// Vertex color A 0-255
	float U;
	float V;
	float Padding1;					// -666
	float Padding2;					// -666
	float Padding3;					// -666
};


struct LVC_UNKNOWN1_HEADER
{
	uint32_t PtrToVerts1;
	uint32_t PtrToVerts2;
	uint32_t PtrToIndices;
	uint32_t PtrToVerts3;
	uint32_t PtrToOffsets;
	uint32_t Unknown1;
	uint32_t Unknown2;
	uint32_t nVertices;
	uint32_t nOffsets;
};


struct LVC_UNKNOWN1_VERTS
{
	float X;
	float Y;
	float Z;
	float W;
};