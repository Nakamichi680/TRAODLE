#pragma once


struct MDC_HEADER				// Size: 56 bytes
{
	uint16_t Version;				// 5Ch
	uint16_t TargetPlatform;		// 100h = PC, 200h = PS, 1000h = PSP
	uint32_t Magic;					// "MDC"
	uint32_t Unknown1;
	char Name[8];					// Name of the model
	uint32_t Unknown2;				// 0
	uint32_t Unknown3;				// 38h
	uint32_t Unknown4;				// 38h
	uint32_t Unknown5;				// 38h
	uint32_t Unknown6;				// 38h
	uint32_t PtrSkeleton;			// 40h
	uint32_t PtrGeometry;
	uint32_t PtrBlock3;
	uint32_t PtrBlock4;
};


struct MDC_SKELETON_HEADER
{
	uint32_t nBones;
	uint32_t Unknown1;				// Always 0?
	uint32_t PtrList1;				// Pointer to List1
	uint32_t PtrBoneOffsets;		// Pointer to bone offsets
	uint32_t PtrSkeletonEnd;		// Pointer to the end of the skeleton
};


struct MDC_SKELETON_LIST1_HEADER
{
	uint32_t nBones;
};


struct MDC_SKELETON_LIST1_ITEM
{
	uint32_t hash;
	uint16_t index1;
	uint16_t index2;
};


struct MDC_SKELETON_BONE_OFFSETS
{
	uint32_t BoneOffset;			// Pointer to MDC_SKELETON_BONE
};


struct MDC_SKELETON_BONE		// Size: 152 bytes
{
	uint32_t Unknown1;
	uint32_t Unknown2;
	int16_t Parent;
	uint8_t Unknown4;
	uint8_t Unknown5;
	float X1;
	float Y1;
	float Z1;
	float W1;
	float X2;
	float Y2;
	float Z2;
	float W2;
	float Xpos;
	float Ypos;
	float Zpos;
	float Padding1;
	float Xrot;
	float Yrot;
	float Zrot;
	float Wrot;
	float Xscal;
	float Yscal;
	float Zscal;
	float Padding2;
	float a1;						// a1 = a2 = a3
	float a2;						// a1 = a2 = a3
	float a3;						// a1 = a2 = a3
	float b1;						// b1 = b2 = b3
	float b2;						// b1 = b2 = b3
	float b3;						// b1 = b2 = b3
	uint32_t Unknown6;				// Always 40h ?
	float X6;
	float Y6;
	float Z6;
	float W6;
	float X7;
	float Y7;
	float Z7;
	float W7;
};
