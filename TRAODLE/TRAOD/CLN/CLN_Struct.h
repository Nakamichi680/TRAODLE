#pragma once


struct CLN_OCTREE		// Size: 80 bytes
{
	uint32_t PtrToParent;			// Pointer to the address of the parent octree. First element is always FFFFFFFFh
	uint32_t Ptr_TList;				// Pointer to TLIST element. The pointer is relative to CLN_TRIANGLE beginning
	uint32_t Unknown2;				// Always 00000000h?
	uint16_t nChildren;				// Number of children
	uint16_t ChildrenIndices;		// The index(indices) of child(ren) used. Ex. FFh (11111111, all octants), AAh (10101010, 1st, 3rd, 5th, 7th octants used)
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
	uint32_t attribute;				// Composition + mapping + bitattrib (tipo di materiale, tipo di superficie, tipo di interazione)
};


/*------------------------------------------------------------------------------------------------------------------
Attributo di collisione. Definizioni originali (Tools\CollisionProducer\CollisionSpec.h e Tools\worldedit\Sys\collision.csc):
	bits 0-7	composition [CF_GETSURFACETYPE 0xFF]
	bits 8-9	mapping     [CF_GETMAPPINGTYPE 0x300, CF_MAPPINGSHIFT 8]: 0 NONE (parete), 1 FLOOR, 2 CEILING
	bits 10-21	bitattrib   [CF_BITATTRSTART 10]: DEADLY, HARMFUL, CLIMBABLE, MONKEY_SWING, SHOT_PERMEABLE, VIEW_PERMEABLE,
				WALL_CLIMB, STAIRS, SLIDE, NO_SLIDE, CAMERA_PERMEABLE, NO_GRAB
	composition (indice nella lista [COMPOSITION] di collision.csc):
		 0- 2 STONE1-3			 3- 5 MARBLE1-3			 6- 8 GRAVEL1-3			 9-11 SAND1-3
		12-14 MUD1-3			15-17 GRASS1-3			18-20 TALL_GRASS1-3		21-23 CARPET1-3
		24-26 WOOD1-3			27-29 CREAKY_WOOD1-3	30-32 METAL1-3			33-35 CREAKY_METAL1-3
		36-38 ROCK1-3			39-41 UNSAFE_ROCK1-3	42-44 WATER_PUDDLE1-3	45 WET_WOOD
		46 WET_METAL			47 WET_STONE			48-50 SNOW1-3			51 RUBBER
		52 SKIP					53 GLASS_PANEL			54 ICE					55 WET_GRAVEL
		56 WET_CARPET			57 WET_GRASS			58 WET_SAND				59 WET_RUBBER
		60 WET_SKIP
	CLN_GetAttributeName usa la texture "stone" per 51-54 e "wetstone" per 59-60 (non esistono texture dedicate).
------------------------------------------------------------------------------------------------------------------*/


struct CLN_TLIST		// Size: variable
{
	uint32_t attribute;
	uint16_t padding;
	uint16_t nIndices;
	uint16_t Index;
};