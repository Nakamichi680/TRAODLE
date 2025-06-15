#pragma once


struct ZONE_HEADER
{
    uint32_t ZONE_ID;				// Sempre 20h
    uint32_t TEXTURE_PTR;
    uint32_t PS2_OBJ_PTR;
    uint32_t MESH_PTR;
    uint32_t EOF_PTR;
};


struct ZONE_PS2_OBJ_HEADER
{
	uint32_t nObjects;				// Numero di oggetti contenuti nel file ZONE
	uint32_t nSlots;				// Numero di slot per oggetti (gli slot contengono un pointer per l'oggetto, se lo slot è vuoto il pointer è sostituito da FFFFFFFFh)
};


struct ZONE_PS2_OBJ_SLOTS			// Questo blocco è ripetuto ZONE_PS2_OBJ_HEADER.nSlots volte
{
	uint32_t P1_Object;				// Puntatore all'header dell'oggetto nel blocco PS2
};


struct ZONE_PS2_OBJ_UNKNOWN		// Lunghezza: 80 bytes
{
	float Unknown1;
	float Unknown2;
	float Unknown3;
	uint32_t Unknown4;		// Numeri piccoli variabili
	float Unknown5;
	float Unknown6;
	float Unknown7;
	uint32_t Pointer;		// Puntatore ad un altro puntatore in un blocco di dati ancora sconosciuti. PARE SIA L'UNICO PUNTATORE DI QUESTA STRUCT!!!
	uint32_t Unknown8;		// Numero grande variabile (con alcune ripetizioni). Che sia un hash?
	uint32_t null1;					// Sempre 0? PARE DI SI!!!
	uint32_t Unknown10;		// Numero grande (forse è un hash, di sicuro non è un float). Costante all'interno del file, diverso tra file e file
	uint32_t Unknown11;		// Spesso 0, a volte con numeri piccoli
	uint32_t Unknown12;		// A volte è un numero piccolo, altre volte è FFFFFFFFh
	uint32_t Unknown13;		// Spesso 0 ma non sempre, a volte con numeri grandi (non sembrano puntatori)
	uint32_t Unknown14;		// Spesso 1 ma non sempre, a volte con numeri grandi (non sembrano puntatori, spesso questi numeri sono ripetuti molte volte)
	uint32_t Unknown15;		// A volte è un numero piccolo, altre volte è FFFFFFFFh
	uint32_t null2;					// Sempre 0? PARE DI SI!!!
	uint32_t Unknown16;				// Sempre 22CE4D00h? PARE DI SI!!!
	uint32_t Unknown17;				// Sempre C884BD00h? PARE DI SI!!!
	uint32_t Unknown18;		// Sempre uguale a Unknown10
};


struct ZONE_PS2_OBJ			// Lunghezza variabile (minimo 64 bytes)
{
	float Unknown1x;					// Inutilizzato?
	float Unknown2y;					// Inutilizzato?
	float Unknown3z;					// Inutilizzato?
	float Unknown4w;					// Inutilizzato?
	float Unknown5x;					// Inutilizzato?
	float Unknown6y;					// Inutilizzato?
	float Unknown7z;					// Inutilizzato?
	float Unknown8w;					// Inutilizzato?
	int Obj_ID;							// Se >0 è l'ID dell'oggetto nel blocco ZONE_MESH_OBJECT, se <0 ha funzione differente a seconda del valore
	float Xrot;
	float Yrot;
	float Zrot;
};


struct ZONE_MATERIALS_HEADER
{
    uint32_t nMaterials;			// Numero materiali
    uint32_t Unknown1;
    uint32_t nTextures;				// Numero textures
    uint32_t Unknown2;
};


struct ZONE_MATERIALS_LIST
{
    uint16_t TextureMode;			// Tipologia di materiale
	uint16_t DoubleSided;			// Texture a doppia faccia
    uint32_t Unknown1;				// Sembrerebbe sempre pari a 00000004h, forse indica il numero di slot per textures a seguire
    int32_t DiffuseID;				// Slot 1: texture principale
    int32_t ShadowMapID;			// Slot 2: shadow map
    int32_t BumpSpecID;				// Slot 3: bump map e riflessi
    int32_t FurID;					// Slot 4: texture ARGB usata per fur (principalmente ciuffi di erba)
};


struct ZONE_TEXTURE_LIST
{
    char32_t DXT;					// DXT1, DXT3, DXT5 o RGBA
    uint32_t ColourBumpShadow;		// 01000000h per shadow map, 02000000h per diffuse map, 04000000h per bump map, 05000000h per fur
    uint32_t Unknown1;				// Sempre 01000000h
    uint32_t Unknown2;				// Sempre 1E000000h
    uint32_t Mips;					// Numero di mipmaps
    uint32_t Xsize;					// Dimensione asse X immagine in pixels
    uint32_t Ysize;					// Dimensione asse Y immagine in pixels
    uint32_t RAWsize;				// Dimensione texture in bytes
    uint32_t Unknown3;				// Sempre 00000000h
    uint32_t Unknown4;				// Sempre 00000000h
};


struct ZONE_MESH_HEADER1
{
    uint32_t nRooms;				// Numero stanze contenute nel file ZONE
};


struct ZONE_MESH_ROOM_HEADER
{
    uint32_t RoomID;				// ID univoco della stanza
    uint32_t Room_size;				// Dimensione stanza in bytes
    uint32_t Unknown1;
    uint32_t Unknown2;
    uint32_t nVertices;				// Numero di vertici della stanza
    uint32_t Unknown3;
    uint32_t nIndices;				// Numero di indici contenuti nel triangle strip
    uint32_t Unknown4;
    uint32_t nElements;				// Numero di elementi nella stanza
    uint32_t Unknown5;
    uint32_t Unknown6;
    uint32_t Unknown7;
    uint32_t Unknown8;
    uint32_t Unknown9;
};


struct ZONE_MESH_HEADER2
{
    uint32_t nObjects;				// Numero oggetti contenuti nel file ZONE
};


struct ZONE_MESH_OBJECT_HEADER
{
    uint32_t Object_size;			// Dimensione oggetto in bytes
    uint32_t Unknown1;
    uint32_t Unknown2;
    uint32_t nVertices;				// Numero di vertici dell'oggetto
    uint32_t Unknown3;
    uint32_t nIndices;				// Numero di indici contenuti nel triangle strip
    uint32_t Unknown4;
    uint32_t nElements;				// Numero di elementi nell'oggetto
    uint32_t Unknown5;
    uint32_t Unknown6;
    uint32_t Unknown7;
    uint32_t Unknown8;
    uint32_t Unknown9;
};


struct ZONE_MESH_VERTEX
{
    float X;						// Coordinata x vertice
    float Y;						// Coordinata y vertice
    float Z;						// Coordinata z vertice
    float U1;						// Coordinata U texture principale
    float V1;						// Coordinata V texture principale
    float U2;						// Coordinata U texture secondaria (shadow map)
    float V2;						// Coordinata V texture secondaria (shadow map)
    uint8_t Xn;						// Coordinata x vertex normal. 1h=-1, 80h=0, FFh=1. Il range di valori è 1-255. I valori vanno riportati da -1 a +1, nel mezzo (80h, cioè 128) c'è lo 0.
    uint8_t Yn;						// Coordinata y vertex normal. 1h=-1, 80h=0, FFh=1. Il range di valori è 1-255. I valori vanno riportati da -1 a +1, nel mezzo (80h, cioè 128) c'è lo 0.
    uint8_t Zn;						// Coordinata z vertex normal. 1h=-1, 80h=0, FFh=1. Il range di valori è 1-255. I valori vanno riportati da -1 a +1, nel mezzo (80h, cioè 128) c'è lo 0.
    uint8_t Xtg;
    uint8_t Ytg;
    uint8_t Ztg;
    uint8_t Xbn;
    uint8_t Ybn;
    uint8_t Zbn;
    uint8_t VC_red;					// Vertex color R 0-255
    uint8_t VC_green;				// Vertex color G 0-255
    uint8_t VC_blue;				// Vertex color B 0-255
};


struct ZONE_MESH_STRIP
{
    uint16_t Index;
};


struct ZONE_MESH_ELEMENT
{
    uint32_t nElement_Triangles;	// Numero di triangoli. Sempre uguale a numero indici triangle strip dell'elemento - 2
    uint32_t nElement_Indices;		// Numero di indici di vertici. Sempre uguale a nElement_Triangles + 2
    uint32_t Offset;				// Il primo è 00000000h, i successivi sono Offset + nElement_Indices dell'elemento precedente
    uint32_t Material_Ref;			// Numero del materiale
    uint32_t Unknown1;				// Sempre FFFFFFFFh ?
    uint32_t Vbuffer_min;			// Indice del vertice più piccolo dell'elemento
    uint32_t Vbuffer_max;			// Indice del vertice più grande dell'elemento
    uint32_t Draw_mode;				// 00000000h e 01000000h vista vertici. 02000000h e 03000000h wireframe rispettivamente senza e con flip poligoni pari. 04000000h e 05000000h vista normale rispettivamente senza e con flip poligoni pari.
    float BB_Xmin;					// Bounding box coordinata X min
    float BB_Ymin;					// Bounding box coordinata Y min
    float BB_Zmin;					// Bounding box coordinata Z min
    uint32_t Unknown2;
    float BB_Xmax;					// Bounding box coordinata X max
    float BB_Ymax;					// Bounding box coordinata Y max
    float BB_Zmax;					// Bounding box coordinata Z max
    uint32_t Unknown3;
};


struct ZONE_FAKES_HEADER			// SIZE: 16 bytes
{
	uint32_t P1_Fake_First;			// Se è uguale a 0 non ci sono fakes
	uint32_t P1_Fake_Last;
	uint32_t null1;
	uint32_t null2;
};


struct ZONE_FAKES_ELEMENT			// SIZE: 144 bytes
{
	uint32_t BegPrev;
	uint32_t BegNext;
	uint32_t null1;
    uint32_t null2;
    uint32_t null3;
    uint32_t null4;
	uint32_t Unknown1;								// Numero progressivamente crescente, forse indica lo slot dove viene salvato il Fake
	uint32_t null5;
	float Xpos;												// X position of the center of the fake
    float Ypos;												// Y position of the center of the fake
    float Zpos;												// Z position of the center of the fake
	float Wpos;						// Always 0x0000803F
	uint32_t Unknown2;				// Always 0
	uint32_t Unknown3;				// Variable 0 or 0x00000080
	char null6[24];					// Always 0
	float Xscal;
    float Yscal;
    float Zscal;
	char null7[8];					// Always 0
	uint32_t Unknown4;				// Forse un hash
	uint32_t Unknown5;				// Sempre 0A000000h
	uint32_t Static_FollowCam;		// Se 4 oggetto statico, se 8 l'oggetto si sposta seguendo la telecamera
	float BB_Xmin;						// Bounding box X min
    float BB_Ymin;						// Bounding box Y min
    float BB_Zmin;						// Bounding box Z min
	uint16_t Unknown6;
	uint16_t Fake_List_Index;			// Indice dell'oggetto nella lista PS2_OBJ_SLOTS
	float BB_Xmax;						// Bounding box X max
    float BB_Ymax;						// Bounding box Y max
    float BB_Zmax;						// Bounding box Z max
	uint32_t BegPrev2;
};