#pragma once


struct CLN_OCTREE		// Size: 80 bytes
{
	uint32_t PtrToParent;			// Pointer to the address of the parent octree. First element is always FFFFFFFFh
	uint32_t Ptr_TList;				// Pointer to TLIST element. The pointer is relative to CLN_TRIANGLE beginning
	uint32_t Unknown2;				// Always 00000000h?
	uint16_t nChildren;				// Number of children
	uint16_t Unknown3;				// 0000h if the octree has no children, otherwise variable
	float Xmin;						// Multiply by 1024 to match world coordinates
	float Ymin;						// Multiply by 1024 to match world coordinates
	float Zmin;						// Multiply by 1024 to match world coordinates
	uint32_t nDescendants;			// Total number of descendants (children, grandchildren, great-grandchildren...)
	float Xmax;						// Multiply by 1024 to match world coordinates
	float Ymax;						// Multiply by 1024 to match world coordinates
	float Zmax;						// Multiply by 1024 to match world coordinates
	uint32_t nTriangles;			// Number of collision triangles inside the octant
	int32_t Ptr_Child1;				// Pointer to the address of the child inside CLN_OCTREE block. FFFFFFFFh if the octant has no child
	int32_t Ptr_Child2;				// Pointer to the address of the child inside CLN_OCTREE block. FFFFFFFFh if the octant has no child
	int32_t Ptr_Child3;				// Pointer to the address of the child inside CLN_OCTREE block. FFFFFFFFh if the octant has no child
	int32_t Ptr_Child4;				// Pointer to the address of the child inside CLN_OCTREE block. FFFFFFFFh if the octant has no child
	int32_t Ptr_Child5;				// Pointer to the address of the child inside CLN_OCTREE block. FFFFFFFFh if the octant has no child
	int32_t Ptr_Child6;				// Pointer to the address of the child inside CLN_OCTREE block. FFFFFFFFh if the octant has no child
	int32_t Ptr_Child7;				// Pointer to the address of the child inside CLN_OCTREE block. FFFFFFFFh if the octant has no child
	int32_t Ptr_Child8;				// Pointer to the address of the child inside CLN_OCTREE block. FFFFFFFFh if the octant has no child
};


struct CLN_TRIANGLE		// Size: 48 bytes
{
	float vecX;						// A value between -1 and 1
	float vecY;						// A value between -1 and 1
	float vecZ;						// A value between -1 and 1
	float ConstValue;				// Multiply by 1024 to match world coordinates
	float v1a;						// Can be either X or Y or Z depending on MissingAxis value. Multiply by 1024 to match world coordinates
	float v2a;						// Can be either X or Y or Z depending on MissingAxis value. Multiply by 1024 to match world coordinates
	float v3a;						// Can be either X or Y or Z depending on MissingAxis value. Multiply by 1024 to match world coordinates
	uint32_t MissingAxis;			// 1 = X, 2 = Y, 3 = Z
	float v1b;						// Can be either X or Y or Z depending on MissingAxis value. Multiply by 1024 to match world coordinates
	float v2b;						// Can be either X or Y or Z depending on MissingAxis value. Multiply by 1024 to match world coordinates
	float v3b;						// Can be either X or Y or Z depending on MissingAxis value. Multiply by 1024 to match world coordinates
	uint32_t Unknown;				// Viene letto dagli indirizzi 0x56F6EE, 0x5464F5, ecc.
};


struct CLN_TLIST		// Size: variable
{
	uint16_t Unknown1;
	uint16_t Unknown2;
	uint16_t Unknown3;
	uint16_t nIndices;
	uint16_t Index;
};