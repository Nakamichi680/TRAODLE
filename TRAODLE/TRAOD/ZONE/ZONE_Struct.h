#pragma once


/*==================================================================================================================
	FORMATO ZONE (.Z00, .Z01, ...) - Tomb Raider: The Angel of Darkness

	Legenda: [OK] verificato su tutti i 99 file ZONE dei livelli estratti, [?] ipotesi, [GARBAGE] non interpretabile,
			 [LOG] verificato con i log dell'"Export manager" di WorldEdit (Release\*.log, 2003: 74 zone di 34 livelli,
			 15003 mesh; il log elenca per ogni zona le mesh in ordine di indice con nome, copie, collisioni e flag)

	STRUTTURA GENERALE DEL FILE [OK]
		ZONE_HEADER (32 bytes)
		0x20 .. PS2_OBJ_PTR			Geometria delle stanze in formato PS2 (ZONE_PS2_ROOM_*): pacchetti DMA/VIF per la VU1
									della PlayStation 2, non usati dalla versione PC (che usa ZONE_MESH)
		PS2_OBJ_PTR .. TEXTURE_PTR	Oggetti (ZONE_PS2_OBJ_HEADER): tabella degli slot, ZONE_OBJECT_INFO x nObjects, liste di
									comandi degli oggetti, poi i dati puntati da ZONE_OBJECT_INFO (geometria PS2 degli
									oggetti e collisioni degli oggetti)
		TEXTURE_PTR .. MESH_PTR		Materiali e textures (ZONE_MATERIALS_HEADER)
		MESH_PTR .. EOF_PTR			Geometria PC delle stanze e degli oggetti (ZONE_MESH_*)
		EOF_PTR ..					Fakes (ZONE_FAKES_HEADER)
	Verificato: le strutture PS2 e degli oggetti coprono le rispettive sezioni (227.6 MB di geometria PS2 delle stanze con
	10 KB di soli allineamenti; nella sezione oggetti restano non spiegati 1.5 MB su 76 MB: catene DMA che proseguono
	oltre il primo tag "ret").

	GEOMETRIA PS2 (stanze e oggetti) [OK]
		Non e' usata dalla versione PC del gioco, che disegna solo la geometria ZONE_MESH: non viene esportata.
		Ogni lotto (batch) di geometria e' un blocco ZONE_PS2_BATCH: bounding box + catena di tag DMA della PS2, ognuno
		seguito dai comandi VIF (UNPACK V4-32 / V4-16 / V3-8 / V4-5, STCYCL, STMASK, ...). Il primo UNPACK V4-32 contiene
		bounding box e GIF tag (NLOOP = numero di vertici, registri ST, RGBAQ, XYZ2), seguito dalle posizioni (V4-32).
		Tutte le catene di tutti i file si decodificano senza errori.
==================================================================================================================*/


struct ZONE_HEADER
{
    uint32_t ZONE_ID;				// Sempre 20h (dimensione dell'header)
    uint32_t TEXTURE_PTR;
    uint32_t PS2_OBJ_PTR;
    uint32_t MESH_PTR;
    uint32_t EOF_PTR;
};


/*------------------------------------------------------------------------------------------------------------------
Geometria delle stanze in formato PS2 (offset 0x20) [OK]:
	uint32_t nRooms
	ZONE_PS2_ROOM_ENTRY x nRooms
	Per ogni stanza, all'offset indicato: ZONE_PS2_ROOM_HEADER, poi i lotti ZONE_PS2_BATCH e la lista dei puntatori
------------------------------------------------------------------------------------------------------------------*/
struct ZONE_PS2_ROOM_ENTRY		// SIZE: 8 bytes
{
	uint32_t RoomID;				// [OK] Hash del nome della stanza (= RMX_ROOM.Room_ID)
	uint32_t Offset;				// [OK] Offset dei dati PS2 della stanza
};


struct ZONE_PS2_ROOM_HEADER		// SIZE: 16 bytes
{
	uint32_t nBatches;				// [OK] Numero di lotti di geometria (ZONE_PS2_BATCH)
	uint32_t null1;					// [OK] Sempre 0
	uint32_t BATCHES_PTR;			// [OK] Offset della lista di puntatori ai lotti (uint32_t x nBatches)
	uint32_t BATCHES_PTR2;			// [OK] Sempre uguale a BATCHES_PTR
};


struct ZONE_PS2_BATCH			// SIZE: 32 bytes + catena DMA/VIF (vedi sopra)
{
	float Xmin, Ymin, Zmin;			// [OK] Bounding box del lotto
	uint32_t Info1;					// [?] 16:16, es. 0x00020029: nei 16 bit bassi spesso il numero di vertici del GIF tag
	float Xmax, Ymax, Zmax;
	uint32_t Info2;					// [?] 16:16
	// Segue la catena DMA: tag di 16 bytes (QWC nei bit 0-15, ID nei bit 28-30: 3 ref, 6 ret, ...) con 2 comandi VIF nei
	// bytes 8-15, seguiti da QWC quadword di dati VIF (tranne che per i tag "ref", i cui dati sono altrove)
};


struct ZONE_PS2_OBJ_HEADER
{
	uint32_t nObjects;				// [OK] Numero di ZONE_OBJECT_INFO = numero di mesh degli oggetti (ZONE_MESH_HEADER2.nObjects)
	uint32_t nSlots;				// [LOG] Numero di slot (oggetti composti, referenziati da RMX_PS2_ROOM_OBJ, RMX_OBJECT e dai Fakes):
									//		"Num groups (global)", uguale in tutte le zone del livello (numerazione condivisa)
};


struct ZONE_PS2_OBJ_SLOTS			// Ripetuto ZONE_PS2_OBJ_HEADER.nSlots volte
{
	uint32_t P1_Object;				// [OK] Offset della lista di comandi dell'oggetto (ZONE_PS2_OBJ), FFFFFFFFh se lo slot e' vuoto
};


/*------------------------------------------------------------------------------------------------------------------
Informazioni di una mesh degli oggetti (era ZONE_PS2_OBJ_UNKNOWN). Array di nObjects elementi subito dopo la tabella
degli slot; l'elemento i descrive la mesh i del blocco ZONE_MESH_OBJECT (stesso indice usato dai comandi degli
oggetti). Tre tipi [OK] (20342 elementi):
	- mesh visibile (13192): mesh PC + nBatches lotti di geometria PS2. Bounding box = bounding box dei vertici PC
	- copia di un'altra mesh (5582): nessuna mesh PC (0 vertici), lotti PS2 con tag DMA "ref" ai dati della mesh base:
	  la stessa geometria riusata in un altro oggetto (3692 istanze RMX usano slot fatti solo di copie, solo 90 nella
	  stessa posizione dello slot base). BaseIndex = indice della mesh base, che ha lo stesso bounding box (5582/5582)
	- collisione (1568): nBatches = 0, Flags bit 0 = 1, nessuna mesh PC; DATA_PTR punta a ZONE_OBJECT_COLLISION
------------------------------------------------------------------------------------------------------------------*/
#define ZONE_MESHFLAG_COLLISION		0x01	// [SRC] Mesh di collisione: ObjectCollision punta a ZONE_OBJECT_COLLISION (mapBindMesh)
#define ZONE_MESHFLAG_ALIAS			0x08	// [SRC] Mesh trovata per AliasHash tra le mesh dello slot (mapFindAliasMesh)
#define ZONE_MESHFLAG_SOLID			0x10	// [SRC] Mesh solida per le collisioni con gli oggetti (mapObjectHasCollision,
											//		mapAddDrawMesh: fisica delle collisioni dell'oggetto)
#define ZONE_MESHFLAG_CAPSULE		0x20	// [SRC] Mesh che collide con le capsule dei personaggi animati (mapCollideAnimSetToCapsule)
#define ZONE_MESHFLAG_LIT			0x40	// [SRC] Mesh illuminata dalle luci dell'oggetto (RMX_OBJECT.Light*, mapSetObjectLights)
#define ZONE_MESHFLAG_BOUND			0x80	// [SRC] Puntatori gia' fissati (solo runtime)

struct ZONE_OBJECT_INFO			// SIZE: 80 bytes [SRC: tagMESH_HEADER]
{
	float Xmin, Ymin, Zmin;			// [OK] Bounding box della mesh (coordinate locali dell'oggetto) [SRC: Min]
	uint32_t nBatches;				// [OK] Numero di lotti ZONE_PS2_BATCH (0 per le collisioni) [SRC: StripCount]
	float Xmax, Ymax, Zmax;
	uint32_t DATA_PTR;				// [OK] [SRC: StripIndices] Mesh: offset della lista di puntatori ai lotti (uint32_t x nBatches, terminata da 0).
									//		 Collisione: offset di ZONE_OBJECT_COLLISION
	uint32_t Hash;					// [LOG] [SRC: ShapeHash] GetHashValue del nome della mesh in maiuscolo (nome del nodo shape di Maya, es. "DWR",
									//		"D_L__COLLISION"). 9287 valori distinti su 20342: le copie hanno lo stesso nome
	uint32_t null1;					// [OK] Sempre 0 (tranne 1 caso) [SRC: nLOD]
	uint32_t ZoneHash;				// [OK] Quasi sempre costante nel file e uguale a ZoneHash2 (95%) [SRC: pPCData, puntatore ai dati PC]
	uint32_t Flags;					// [LOG] [SRC: MeshFlags] Bit ZONE_MESHFLAG_*. Bit 0: collisione ("[Collision]"; coincide con
									//		nBatches = 0 in 20319 casi su 20342). Bit 6 (0x40): presente in tutte le 8741 mesh "dynamic"
									//		(oggetti) e in 149 altre; 0 nelle 6113 mesh statiche restanti. Combinazioni osservate dei
									//		bit 3-5: 0x48, 0x50, 0x58, 0x60. "UNLIT" non e' nei flags
	uint32_t BaseIndex;				// [LOG] [SRC: InstanceBinding] Copie ("Mesh X(i) instance of j"): indice della mesh base, di cui
									//		usano la geometria. Altrimenti FFFFFFFFh
	uint32_t CollisionPtr;			// [OK] [SRC: ObjectCollision] Collisioni: uguale a DATA_PTR (0 in 23 casi). Altrimenti 0
	uint32_t ParentHash;			// [SRC: AliasHash] Nome alternativo della mesh (con ZONE_MESHFLAG_ALIAS): 1 = nessuno. Altrimenti
									//		un hash: in 3501 casi su 8222 e' l'Hash di un'altra mesh
	uint32_t CollisionIndex;		// [LOG] [SRC: CollideMeshIdx] Indice della mesh di collisione di questa mesh ("Mesh X bound to
									//		collision X__COLLISION"), FFFFFFFFh se non ne ha. Le collisioni seguono la mesh (es. porte animate)
	uint32_t Unknown2;				// [SRC: nBroken] Numero di versioni "rotte" della mesh (oggetti distruttibili): 0, a volte 1 o 2
	uint32_t Unknown3;				// [GARBAGE] [SRC: pad[3]] Pochi valori ricorrenti (0x004DC832, 0x004DCE42, ...)
	uint32_t Unknown4;				// [GARBAGE] [SRC: pad[3]] Pochi valori ricorrenti (0x00BD7168, 0x00BD84C8, 0x00BD6F28)
	uint32_t ZoneHash2;				// [OK] Vedi ZoneHash [SRC: pad[3]]
};


/*------------------------------------------------------------------------------------------------------------------
Lista di comandi di un oggetto composto (slot). Inizia con il bounding box e prosegue con comandi interi [OK] (11157
oggetti, nessun comando sconosciuto, push/pop sempre bilanciati, profondita' massima 14):
	-1				fine della lista
	-2				apre un nodo figlio (push della trasformazione corrente)
	-3				chiude il nodo (pop)
	-4				trasformazione del nodo: 6 float, traslazione X Y Z + rotazione X Y Z in gradi (-180..180)
	n >= 0			mesh n (ZONE_OBJECT_INFO / ZONE_MESH_OBJECT di indice n) + 3 float sempre 0 (padding)
	-4 compare sempre subito dopo -2: e' la trasformazione del nodo appena aperto, relativa al nodo padre (pila di
	matrici). Mai a profondita' 0, mai ripetuto, mai dopo una mesh dello stesso nodo [OK]. Rotazione nell'ordine XYZ.
	Istanze: i nodi RMX_PS2_ROOM_OBJ istanziano lo slot ZoneSlot con posizione, rotazione XYZ e scala nel mondo (matrice
	S * Rx * Ry * Rz * T): la bounding box risultante coincide con quella del nodo RMX in 7639 istanze su 7639 [OK].
	Le zone di uno stesso livello condividono la numerazione degli slot; slot vuoti in tutte le zone del livello sono
	referenziati da 1917 istanze (geometria non presente nel livello).
Le mesh di collisione fanno parte della gerarchia come quelle visibili (1568/1568 referenziate).
------------------------------------------------------------------------------------------------------------------*/
struct ZONE_PS2_OBJ			// Lunghezza variabile
{
	float Unknown1x;				// [OK] Bounding box dell'oggetto: X min
	float Unknown2y;				// [OK] Y min
	float Unknown3z;				// [OK] Z min
	float Unknown4w;				// [?] Intero 0
	float Unknown5x;				// [OK] X max
	float Unknown6y;				// [OK] Y max
	float Unknown7z;				// [OK] Z max
	float Unknown8w;				// [?] Intero 1
	int Obj_ID;						// [OK] Comando (vedi sopra)
	float Xrot;						// [OK] Mesh: sempre 0 (padding, NON e' una rotazione)
	float Yrot;
	float Zrot;
};


/*------------------------------------------------------------------------------------------------------------------
Collisione di una mesh degli oggetti [OK]: stesso formato dei file CLN (CLN_TRIANGLE, CLN_TLIST, coordinate / 1024,
attributi di collisione), come un octree di una sola foglia, in coordinate locali dell'oggetto.
	ZONE_OBJECT_COLLISION
	CLN_TRIANGLE x nTriangles (all'offset TRIANGLES_PTR, subito dopo il nodo)
	CLN_TLIST (all'offset TLIST_PTR, subito dopo i triangoli): attributo e indici di tutti i triangoli (0 .. n-1)
Verificato su 1546 collisioni (28812 triangoli: normali unitarie, asse 1-3, composizione 0-60 o 0xFF = non assegnata).
22 elementi con nBatches = 0 puntano invece ad un lotto PS2 (es. PARIS2B mesh 55-57).
Le collisioni compaiono solo negli slot che non sono istanziati ne' da RMX_PS2_ROOM_OBJ ne' dai Fakes (977 slot su 977, in 922
insieme a geometria visibile): sono gli oggetti interattivi/mobili, piazzati dai nodi RMX_OBJECT (slot nel campo
RMX_OBJECT.ZoneSlot, posizione/rotazione/scala del nodo). Le collisioni degli oggetti statici istanziati sono nella mappa
delle collisioni del livello (file CLN).
Gli oggetti interattivi/animati sono anche i nodi dei file CHR in formato "NODE" (porte, cassetti, macchine, ...), che
hanno come nome la mesh del ZONE (ZONE_OBJECT_INFO.Hash = GetHashValue(nome del nodo in maiuscolo), 3251 nodi su 3497):
tutte le mesh trovate (467 CHR) appartengono a slot piazzati da RMX_OBJECT, di solito uno solo per CHR. La lista di
comandi dello slot contiene la posa dell'oggetto nel livello (es. porta chiusa), le animazioni del CAL omonimo muovono i
nodi (vedi CHR_Struct.h e CHR_Nodes.cpp).
------------------------------------------------------------------------------------------------------------------*/
struct ZONE_OBJECT_COLLISION		// SIZE: 48 bytes
{
	float Xmin, Ymin, Zmin;			// [OK] Bounding box / 1024 (come CLN_OCTREE)
	uint32_t TLIST_PTR;				// [OK] Offset della CLN_TLIST = TRIANGLES_PTR + 48 * nTriangles
	float Xmax, Ymax, Zmax;
	uint32_t TRIANGLES_PTR;			// [OK] Offset dei triangoli (CLN_TRIANGLE)
	uint32_t nTriangles;			// [OK] Numero di triangoli
	uint32_t null[3];				// [OK] Sempre 0
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
    int32_t Material_Ref;			// Numero del materiale
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


/*------------------------------------------------------------------------------------------------------------------
Fake: oggetto di sfondo lontano (skyline, ...) [SRC: tagMAP_SCENERY, come RMX_PS2_ROOM_OBJ]. Viene disegnato solo se il
bit Index e' a 1 nella maschera dei Fakes della stanza corrente (RMX_ROOM.Offset33, FakeVisibility; mapDrawFakes).
------------------------------------------------------------------------------------------------------------------*/
struct ZONE_FAKES_ELEMENT			// SIZE: 144 bytes
{
	uint32_t BegPrev;
	uint32_t BegNext;
	uint32_t null1;
    uint32_t null2;
    uint32_t null3;
    uint32_t null4;
	uint32_t Unknown1;								// [SRC: UserPad] Indice del Fake nel livello (bit della maschera FakeVisibility delle stanze)
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
	uint32_t Unknown4;				// [SRC: Hash] Hash del nome del nodo
	uint32_t Unknown5;				// Sempre 0A000000h [SRC: NodeType = 10]
	uint32_t Static_FollowCam;		// [SRC: NodeFlags] Se 4 oggetto statico, se 8 l'oggetto si sposta seguendo la telecamera
									//		(mapDrawFakes: posizione relativa alla telecamera)
	float BB_Xmin;						// Bounding box X min
    float BB_Ymin;						// Bounding box Y min
    float BB_Zmin;						// Bounding box Z min
	uint16_t Unknown6;					// [SRC: Validate]
	uint16_t Fake_List_Index;			// Indice dell'oggetto nella lista PS2_OBJ_SLOTS [SRC: MeshN]
	float BB_Xmax;						// Bounding box X max
    float BB_Ymax;						// Bounding box Y max
    float BB_Zmax;						// Bounding box Z max
	uint32_t BegPrev2;					// [SRC: pRef, puntatore runtime]
};