#pragma once


struct TRAE_GEOM_HEADER				// Size: 52 bytes
{
	uint32_t ID;					// "SIM"
	uint32_t FileVersion;			// 55
	uint32_t nVertices;				// Number of vertices
	uint32_t nTriangles;			// Number of triangles
	uint32_t nElements;				// Number of elements (each element is a group of triangles with the same material)
	uint32_t Unknown1;
	uint32_t Unknown2;
	uint32_t Unknown3;
	uint32_t Unknown4;
	uint32_t Unknown5;
	uint32_t Unknown6;
	uint32_t Unknown7;
	uint32_t Unknown8;
};


struct TRAE_GEOM_VERTEX				// Size: 400 bytes
{
	float Unknown1;
	float Unknown2;
	float Unknown3;
	float Unknown4;
	float Unknown5;
	float Unknown6;
	float R;						// Vertex color, red
	float G;						// Vertex color, green
	float B;						// Vertex color, blue
	float A;						// Vertex color, alpha, unused? Always 3F800000h

	uint32_t Padding1;				// Always 0
	uint32_t Padding2;				// Always FFFFFFFFh
	float Unknown7;					// Always 3F800000h
	float X1;						// Vertex x coordinate (set 1)
	float Y1;						// Vertex y coordinate (set 1)
	float Z1;						// Vertex z coordinate (set 1)
	float Xn1;						// Vertex normal, x coordinate (set 1)
	float Yn1;						// Vertex normal, y coordinate (set 1)
	float Zn1;						// Vertex normal, z coordinate (set 1)

	uint32_t Padding3;				// Always 0
	uint32_t Padding4;				// Always FFFFFFFFh
	float Unknown8;					// Always 0
	float X2;						// Vertex x coordinate (set 2)
	float Y2;						// Vertex y coordinate (set 2)
	float Z2;						// Vertex z coordinate (set 2)
	float Xn2;						// Vertex normal, x coordinate (set 2)
	float Yn2;						// Vertex normal, y coordinate (set 2)
	float Zn2;						// Vertex normal, z coordinate (set 2)

	uint32_t Padding5;				// Always 0
	uint32_t Padding6;				// Always FFFFFFFFh
	float Unknown9;					// Always 0
	float X3;						// Vertex x coordinate (set 3)
	float Y3;						// Vertex y coordinate (set 3)
	float Z3;						// Vertex z coordinate (set 3)
	float Xn3;						// Vertex normal, x coordinate (set 3)
	float Yn3;						// Vertex normal, y coordinate (set 3)
	float Zn3;						// Vertex normal, z coordinate (set 3)

	uint32_t Padding7;				// Always 0
	uint32_t Padding8;				// Always FFFFFFFFh
	float Unknown10;				// Always 0
	float X4;						// Vertex x coordinate (set 4)
	float Y4;						// Vertex y coordinate (set 4)
	float Z4;						// Vertex z coordinate (set 4)
	float Xn4;						// Vertex normal, x coordinate (set 4)
	float Yn4;						// Vertex normal, y coordinate (set 4)
	float Zn4;						// Vertex normal, z coordinate (set 4)

	uint32_t Padding9;				// Always 0
	uint32_t Padding10;				// Always FFFFFFFFh
	float Unknown11;				// Always 0
	float X5;						// Vertex x coordinate (set 5)
	float Y5;						// Vertex y coordinate (set 5)
	float Z5;						// Vertex z coordinate (set 5)
	float Xn5;						// Vertex normal, x coordinate (set 5)
	float Yn5;						// Vertex normal, y coordinate (set 5)
	float Zn5;						// Vertex normal, z coordinate (set 5)

	uint32_t Padding11;				// Always 0
	uint32_t Padding12;				// Always FFFFFFFFh
	float Unknown12;				// Always 0
	float X6;						// Vertex x coordinate (set 6)
	float Y6;						// Vertex y coordinate (set 6)
	float Z6;						// Vertex z coordinate (set 6)
	float Xn6;						// Vertex normal, x coordinate (set 6)
	float Yn6;						// Vertex normal, y coordinate (set 6)
	float Zn6;						// Vertex normal, z coordinate (set 6)

	uint32_t Padding13;				// Always 0
	uint32_t Padding14;				// Always FFFFFFFFh
	float Unknown13;				// Always 0
	float X7;						// Vertex x coordinate (set 7)
	float Y7;						// Vertex y coordinate (set 7)
	float Z7;						// Vertex z coordinate (set 7)
	float Xn7;						// Vertex normal, x coordinate (set 7)
	float Yn7;						// Vertex normal, y coordinate (set 7)
	float Zn7;						// Vertex normal, z coordinate (set 7)

	uint32_t Padding15;				// Always 0
	uint32_t Padding16;				// Always FFFFFFFFh
	float Unknown14;				// Always 0
	float X8;						// Vertex x coordinate (set 8)
	float Y8;						// Vertex y coordinate (set 8)
	float Z8;						// Vertex z coordinate (set 8)
	float Xn8;						// Vertex normal, x coordinate (set 8)
	float Yn8;						// Vertex normal, y coordinate (set 8)
	float Zn8;						// Vertex normal, z coordinate (set 8)
	char Padding17[64];				// Always 0
	uint32_t hash;					// Hash of something, maybe texture filename
	uint32_t index;					// Index of something, maybe material/texture
};


struct TRAE_GEOM_TRIANGLE			// Size: 16 bytes
{
	uint32_t Element_index;			// Index of the parent element of the triangle
	uint32_t v1_index;				// Index of the first vertex of the triangle
	uint32_t v2_index;				// Index of the second vertex of the triangle
	uint32_t v3_index;				// Index of the third vertex of the triangle
};