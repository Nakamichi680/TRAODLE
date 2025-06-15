#pragma once


struct RMX_HEADER
{
    float Version;					// 3.6
    uint32_t DefaultZone;			// Zona caricata di default all'avvio del livello
    uint32_t DefaultReverb;
    uint32_t null;					// Non utilizzato
    uint32_t nRooms;				// Numero di stanze contenute nel livello
};


struct RMX_OFFSETS
{
    uint32_t Offset;				// Puntatori all'inizio di ogni stanza
};


struct RMX_ROOM			// SIZE: 368 bytes
{
	char null1[100];			// Always 0
    uint32_t Room_ID;					// Room ID 1
    uint32_t Unknown26;
    uint32_t Unknown27;
    float Room_Xt;						// Room X position
    float Room_Yt;						// Room Y position
    float Room_Zt;						// Room Z position
    float null2;
    float BB_Xmin;						// Bounding box X min
    float BB_Ymin;						// Bounding box Y min
    float BB_Zmin;						// Bounding box Z min
    float null3;
    float BB_Xmax;						// Bounding box X max
    float BB_Ymax;						// Bounding box Y max
    float BB_Zmax;						// Bounding box Z max
    float null4;
    uint32_t null5;				// Always 0x00000000
    uint32_t null6;				// Always 0x00000000
    uint32_t Room_ID2;					// Room ID 2
    uint32_t null7;				// Always 0x00000000
    uint32_t null8;				// Always 0x00000000
    uint32_t null9;				// Always 0x00000000
	
	uint32_t P1_Object_First;				// RMX_OBJECT offset from RMX_OFFSET of that room
    uint32_t P1_Object_Last;
    uint32_t P2_Light_First;	// RMX_LIGHT offset from RMX_OFFSET of that room
	uint32_t P2_Light_Last;
	uint32_t Offset2;
	uint32_t Offset3;
	uint32_t P4_Trigger_First;
	uint32_t P4_Trigger_Last;
	uint32_t Offset6;
	uint32_t Offset7;
	uint32_t Offset8;
	uint32_t Offset9;
	uint32_t Offset10;
	uint32_t Offset11;
	uint32_t P8_Water_First;
	uint32_t P8_Water_Last;
	uint32_t P9_Fog_First;
	uint32_t P9_Fog_Last;
	uint32_t Offset16;
	uint32_t Offset17;
	uint32_t P11_PS2_Room_Obj_First;
	uint32_t P11_PS2_Room_Obj_Last;
	uint32_t P12_Audio_Locator_First;
	uint32_t P12_Audio_Locator_Last;
	uint32_t Offset22;
	uint32_t Offset23;
	uint32_t Offset24;
	uint32_t Offset25;
	uint32_t P15_Portal_First;
	uint32_t nPortals;
	uint32_t Offset28;
	uint32_t Offset29;
	uint32_t Offset30;
	uint16_t ReverbID;
	uint8_t Collision_set;
	uint8_t Collision_set_current;		// Il gioco copia "Collision_set" qui all'avvio del livello
	uint32_t Offset32;
	uint32_t Offset33;
	uint32_t Unknown1;	//FFFFFFFF
	uint32_t Unknown2;	//FFFFFFFF
	uint32_t Unknown3;	//FFFFFFFF
	uint32_t Unknown4;	//FFFFFFFF
	uint32_t Unknown5;
	uint32_t Unknown6;
	uint32_t Unknown7;
	uint32_t Unknown8;
	uint32_t Unknown9;
	uint32_t Unknown10;
};




struct RMX_OBJECT		// SIZE: 528 bytes
{
    uint32_t BegPrev;										// Pointer to the beginning of the previous object from RMX_OFFSET of that room
    uint32_t BegNext;										// Pointer to the beginning of the following object from RMX_OFFSET of that room
    uint32_t Unknown1;			// Always 0x00000000
    uint32_t Unknown2;			// Always 0x00000000
    uint32_t Unknown3;			// Always 0x00000000
    uint32_t Unknown4;			// Always 0x00000000
    uint32_t Unknown5;			// Always 0x00000000
    uint32_t Unknown6;			// Always 0x00000000
    float Xpos;												// X position of the center of the object
    float Ypos;												// Y position of the center of the object
    float Zpos;												// Z position of the center of the object
    float Unknown7;				// Always 0x0000803F
    float Xrot;
    float Yrot;
    float Zrot;
    uint32_t Unknown8;			// Always 0x00000000
    uint32_t Xtrasl;										// Usually 0x00000000. It traslate the object
    uint32_t Ytrasl;										// Usually 0x00000000. It traslate the object
    uint32_t Ztrasl;										// Usually 0x00000000. It traslate the object
    uint32_t Unknown12;			// Always 0x00000000
    float Unknown13;			// Always 0x0000803F
    float Unknown14;			// Always 0x0000803F
    float Unknown15;			// Always 0x0000803F
    uint32_t Unknown16;			// Always 0x00000000
    uint32_t Unknown17;			// Always 0x00000000
    uint32_t Trigger1;										// Used for something related to triggers
    char Unknown18[256];		// Always 0x00000000
    uint32_t AnimationID;									// Slot for object animation
    uint32_t Unknown19;			// Always 0x00000000
    uint16_t ObjectID;										// ID of the object
    uint16_t Unknown20;										// Unknown. Used for interaction between Lara and the object
    uint32_t Trigger2;										// Trigger to a certain event inside the RMX file itself
    char Unknown21[24];			// Always 0x00000000
    uint32_t Unknown22;			// Often 0x0000A000 but variable (sometimes 0x00001500)
    uint32_t Unknown23;			// Always 0x00000000
    uint32_t Unknown24;			// Always 0x00000000
    uint32_t Unknown25;			// Often 0x01000000 but variable (sometimes 0x04000000)
    float Light1_x;											// X position of light 1 of the object
    float Light1_y;											// Y position of light 1 of the object
    float Light1_z;											// Z position of light 1 of the object
    float Unknown26;			// Always 0x0000803F
    float Light2_x;											// X position of light 2 of the object
    float Light2_y;											// Y position of light 2 of the object
    float Light2_z;											// Z position of light 2 of the object
    float Unknown27;			// Always 0x0000803F
    float Light3_x;											// X position of light 3 of the object
    float Light3_y;											// Y position of light 3 of the object
    float Light3_z;											// Z position of light 3 of the object
    float Unknown28;			// Always 0x0000803F
    float Light1_R;											// Red intensity (0-128) of light 1
    float Light1_G;											// Green intensity (0-128) of light 1
    float Light1_B;											// Blue intensity (0-128) of light 1
    float Unknown29;			// 0x00000043 if light is used, otherwise 0x00000000
    float Light2_R;											// Red intensity (0-128) of light 2
    float Light2_G;											// Green intensity (0-128) of light 2
    float Light2_B;											// Blue intensity (0-128) of light 2
    float Unknown30;			// 0x00000043 if light is used, otherwise 0x00000000
    float Light3_R;											// Red intensity (0-128) of light 3
    float Light3_G;											// Green intensity (0-128) of light 3
    float Light3_B;											// Blue intensity (0-128) of light 3
    float Unknown31;			// 0x00000043 if light is used, otherwise 0x00000000
    uint32_t Unknown32;			// Always 0x0000004B
    uint32_t Unknown33;			// Always 0x0000004B
    uint32_t Unknown34;			// Always 0x0000004B
    uint32_t Unknown35;			// Always 0x8000004B
};

struct RMX_LIGHT		// SIZE: 144 bytes
{
    uint32_t BegPrev;										// Pointer to the beginning of the previous light from RMX_OFFSET of that room
    uint32_t BegNext;										// Pointer to the beginning of the following light from RMX_OFFSET of that room
    char null1[24];				// Always 0
	float Xpos;												// X position of the center of the light
    float Ypos;												// Y position of the center of the light
    float Zpos;												// Z position of the center of the light
    float Wpos;					// Always 0x0000803F
	char null2[32];				// Always 0
    char null3[12];				// Always 0x0000803F
	char null4[8];				// Always 0
    uint32_t Light_ID;										// Hashed name of the light
    uint32_t null5;				// Always 1
    uint32_t Static_flag;		// Always 4 (4 means it is a static node and it's not linked to any script)
    float R;												// Red light intensity (0-255)
    float G;												// Green light intensity (0-255)
    float B;												// Blue light intensity (0-255)
    uint32_t null6;				// Always 0
    float intRadius;										// Internal sphere radius
    float extRadius;										// External sphere radius
    char null7[8];				// Always 0
};

struct RMX_PS2_ROOM_OBJ		// SIZE: 144 bytes
{
	uint32_t BegPrev;
    uint32_t BegNext;
	char null1[24];				// Always 0
    float Xpos;					// X position of the center of the object
    float Ypos;					// Y position of the center of the object
    float Zpos;					// Z position of the center of the object
	float Wpos;					// Always 0x0000803F
	float Xrot;
    float Yrot;
    float Zrot;
	char null2[20];				// Always 0
	float Xscal;
    float Yscal;
    float Zscal;
	char null3[8];				// Always 0
	uint32_t Unknown3;			// Forse un hash
	uint32_t Unknown4;			// Sempre 0A000000h
	uint32_t Static_flag;		// Always 4
	float BB_Xmin;						// Bounding box X min
    float BB_Ymin;						// Bounding box Y min
    float BB_Zmin;						// Bounding box Z min
	uint32_t Unknown6;
	float BB_Xmax;						// Bounding box X max
    float BB_Ymax;						// Bounding box Y max
    float BB_Zmax;						// Bounding box Z max
	uint32_t Unknown7;
};

struct RMX_WATER			// SIZE: 432 bytes
{
	uint32_t BegPrev;
    uint32_t BegNext;
	char null1[24];				// Always 0
    float X1;					// X? unk
    float Y1;					// Y? unk
    float Z1;					// Z? unk
	float W1;					// Always 0x0000803F
	char null2[32];				// Always 0
	float X2;					// X? unk
    float Y2;					// Y? unk
    float Z2;					// Z? unk
	char null3[8];				// Always 0
	uint32_t Unknown1;			// Forse un hash
	uint32_t Unknown2;			// Sempre 07000000h
	char null4[248];			// Always 0
	float Xmin;					// Coordinata box acqua. Punto base per waterpatch (la dimensione viene poi arrotondata per difetto dal lato opposto)
    float Xmax;					// Coordinata box acqua. La coordinata viene arrotondata per difetto in base alle dimensioni della Waterpatch.
    float Ymin;					// Coordinata box acqua. Punto base per waterpatch (la dimensione viene poi arrotondata per difetto dal lato opposto)
	float Ymax;					// Coordinata box acqua. La coordinata viene arrotondata per difetto in base alle dimensioni della Waterpatch.
    float Zmin;					// Coordinata box acqua. La coordinata viene arrotondata per difetto in base alle dimensioni della Waterpatch.
    float Zmax;					// Coordinata box acqua. Punto base per waterpatch sull'asse Z
	uint32_t ARGB_PC;			// Colore dell'acqua sul PC. Da approfondire
	uint32_t ARGB_PS2;			// Colore dell'acqua sulla PS2. Da approfondire
	uint16_t Unknown5;			// A volte ha valore 0200h ma sembra che il gioco lo ignori
	uint16_t Water_roughness;	// Intensità dell'agitazione dell'acqua. 
	float Waterpatch_size;		// Dimensione della patch dell'acqua (valori standard 1-0.9, consigliabile non scendere sotto 0.5)
	char null5[4];				// Always 0
	uint32_t Unknown7;			// A volte 3, a volte 1
	uint32_t Unknown8;			// numero variabile
	uint32_t Unknown9;			// numero variabile ma a volte si ripete
	char null6[20];				// Always 0
};

struct RMX_AUDIO_LOCATOR	// SIZE: 368 bytes
{
	uint32_t BegPrev;
    uint32_t BegNext;
	char null1[24];				// Always 0
    float Xpos;					// X position of the center of the audio locator
    float Ypos;					// Y position of the center of the audio locator
    float Zpos;					// Z position of the center of the audio locator
	float Wpos;					// Always 0x0000803F
	char null2[32];				// Always 0
	float Unknown1;				// Always 0x00008042
	float Unknown2;				// Always 0x00008042
	float Unknown3;				// Always 0x00008042
	char null4[8];				// Always 0
	uint32_t Unknown4;			// Forse un hash
	uint32_t Unknown5;			// Sempre 0B000000h
	uint32_t Unknown6;			// Variable (sometimes 0, sometimes 1)
	char null5[240];				// Always 0
	uint32_t AudioTrack;		// Il numero della traccia audio
	uint32_t Unknown8;			// Variable (sometimes 0, sometimes 3)
	char null6[8];				// Always 0
};

struct RMX_FOG				// SIZE: 416 bytes NON COSTANTE!!!!!!!!!!!
{
	uint32_t BegPrev;
    uint32_t BegNext;
	char null1[24];				// Always 0
    float Xpos;					// X position of the center of the fog?
    float Ypos;					// Y position of the center of the fog?
    float Zpos;					// Z position of the center of the fog?
	float Wpos;					// Always 0x0000803F
	char null2[32];				// Always 0
	float Unknown1;
	float Unknown2;
	float Unknown3;
	char null4[8];				// Always 0
	uint32_t Unknown4;			// Forse un hash
	uint32_t Unknown5;			// Sempre 08000000h ?
	uint32_t Unknown6;			// Sempre 01000000h ?
	char null5[240];			// Always 0
	float Unknown7;
	float Unknown8;
	float Unknown9;
	uint8_t R;					// Intensità canale rosso della nebbia
	uint8_t G;					// Intensità canale verde della nebbia
	uint8_t B;					// Intensità canale blu della nebbia
	uint8_t A;					// Inutilizzato
	float Unknown10;
	float Unknown11;
	float Unknown12;
	uint8_t Intensity;			// Intensità dell'effetto nebbia
	uint8_t Unknown13;			// Always 02h ?
	uint16_t Unknown14;			// Always 0 ?
	uint32_t Unknown15;			// Forse un altro hash
	float Unknown16;			// Sempre 0.6 ?
	float Unknown17;			// Sempre 1 ?
	uint32_t Unknown18;			// Sempre 0 ?
	uint32_t Unknown19;
	char null6[12];				// Always 0 ?
};

struct RMX_PORTAL			// SIZE: 112 bytes (da controllare)
{
	uint32_t Unknown1;
	uint32_t Unknown2;
	uint32_t DestRoom;
	float null1;			// Sempre 1
	char null2[28];			// Always 0
	uint32_t Unknown4;		// A volte 3, a volte 2, a volte 1
	float v0_X;				// Coordinata X del vertice 0 del rettangolo del portale
	float v0_Y;				// Coordinata Y del vertice 0 del rettangolo del portale
	float v0_Z;				// Coordinata Z del vertice 0 del rettangolo del portale
	uint32_t Unknown5;		// Questo valore è utilizzato ma non si sa per cosa. Sembra essere costante all'interno del file
	float v1_X;				// Coordinata X del vertice 1 del rettangolo del portale
	float v1_Y;				// Coordinata Y del vertice 1 del rettangolo del portale
	float v1_Z;				// Coordinata Z del vertice 1 del rettangolo del portale
	uint32_t Unknown6;		// Questo valore è utilizzato ma non si sa per cosa. Sembra essere costante all'interno del file. Diverso da Unknown5
	float v2_X;				// Coordinata X del vertice 2 del rettangolo del portale
	float v2_Y;				// Coordinata Y del vertice 2 del rettangolo del portale
	float v2_Z;				// Coordinata Z del vertice 2 del rettangolo del portale
	uint32_t Unknown7;		// Sempre 4?
	float v3_X;				// Coordinata X del vertice 3 del rettangolo del portale
	float v3_Y;				// Coordinata Y del vertice 3 del rettangolo del portale
	float v3_Z;				// Coordinata Z del vertice 3 del rettangolo del portale
	uint32_t Unknown8;		// Sempre 0?
};