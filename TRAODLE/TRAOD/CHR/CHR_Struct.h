#pragma once


/*==================================================================================================================
	FORMATO CHR - Tomb Raider: The Angel of Darkness (personaggi: scheletro, mesh, materiali e textures)

	Informazioni verificate su tutti i 217 file CHR di personaggi disponibili (9846 bones, 4266 MESH2).
	Legenda: [OK] verificato su tutti i file, [?] ipotesi / significato sconosciuto,
			 [GARBAGE] memoria non inizializzata scritta dal builder (non va interpretata)

	ESISTONO DUE FORMATI CON ESTENSIONE CHR
		- Personaggi (Magic = 0): descritti in questo file
		- Oggetti animati (porte, leve, cassetti, interruttori, ...): iniziano con la stringa "NODE" e hanno una
		  struttura diversa (684 file su 901), descritta in fondo a questo file (CHR_NODE_HEADER, CHR_NODE_DATA): solo
		  nodi con pivot, senza geometria. Ogni nodo e' una mesh degli oggetti del file ZONE del livello
		  (ZONE_OBJECT_INFO.Hash = GetHashValue(nome in maiuscolo), 3251 nodi su 3497), animata dal CAL omonimo

	OGGETTI AGGANCIATI [OK su tutti i 217 CHR][SRC: SKIN.Hierachy, AO_AnimateItems, CMeshFile::ExportToGame]
		Sequenza di gruppi fino a SKELETON_PTR (il resto e' riempito di 0; i gruppi con nNodes = 0 vanno saltati):
			CHR_ATTACH_GROUP (8 bytes): numero di nodi, hash della bone di aggancio [HIERACHY.flags]
			per ogni nodo: nome (terminato da 0, allineato a 4 bytes), uint32_t GetHashValue(nome) [boneID], uint32_t
			flags (CHR_BONEFLAG_MATRIXPUSH / POP come per le bones), padding fino a 16 bytes (offset nel file),
			CHR_ATTACH_NODE (176 bytes)
		Ogni nodo e' un transform di Maya con pivot (stessa matrice dei nodi dei CHR "NODE"), figlio della bone di aggancio
		(o del nodo precedente, secondo i flags). Le MESH2 con MESHFLAG_ANIMATED (CHR_MESH2_HEADER.Unknown2 = 5) sono le
		mesh dei nodi: Unknown3 [animidx] e' l'indice del nodo (contando tutti i gruppi in ordine) e i loro vertici sono
		nello spazio del nodo (verificato: fondina e pistola dei poliziotti sul fianco, fucile nella mano di Lara).
		Nei CAL le ParentTracks con l'hash del nome del nodo lo animano (traslazione e scala assolute, ANIM_SET*SCALE);
		senza traccia il nodo resta nella posa del CHR.

	STRUTTURA DEL FILE (personaggi)
		CHR_HEADER (48 bytes)
		Blocco degli oggetti agganciati (da HeaderSize a SKELETON_PTR)	[OK] Gerarchie di nodi agganciate alle bones (armi,
																	fondine, borse, ...), vedi OGGETTI AGGANCIATI
		CHR_BONE x nBONES (da SKELETON_PTR)							SKELETON_PTR + nBONES * 448 = MESH_PTR [OK]
		Blocco mesh (da MESH_PTR):
			uint32_t SizeMESH12										Dimensione del blocco mesh, esclusa questa cella [OK]
			MESH1 (mesh deformata da due bones per vertice)
			CHR_MESH2_LIST + MESH2 x nMeshes2 (mesh rigide, ognuna collegata ad una sola bone)
			padding fino a 16 bytes									MESH_PTR + 4 + SizeMESH12 arrotondato a 16 = TEXTURE_PTR [OK]
		Materiali e textures (da TEXTURE_PTR):
			CHR_MATERIALS_HEADER + CHR_MATERIALS_LIST x nMaterials
			CHR_TEXTURES_HEADER + (CHR_TEXTURES_LIST + dati raw) x nTextures
		padding fino a multiplo di 2048 bytes [OK]

	MESH (MESH1 e MESH2)
		header, vertici, triangle strip (CHR_MESH_STRIP_HEADER + uint16_t x nIndices), elementi
		(CHR_MESH_ELEMENT_HEADER + CHR_MESH_ELEMENT x nElements). Ogni elemento e' una porzione dello strip con un
		proprio materiale. Le coordinate dei vertici sono int16 / 16, le normali int8 / 127, le UV int16 / 32767.

	SCHELETRO E SKINNING [OK]
		La gerarchia si ricava dai 2 bit bassi di CHR_BONE.Hierarchy, letti in sequenza dalla bone 1 (la bone 0 e'
		la radice) con uno stack di bones:
			0 = figlia della bone precedente
			1 = figlia della bone precedente, che viene messa nello stack (inizio di una ramificazione)
			2 = figlia della bone in cima allo stack, che viene tolta dallo stack (ultimo ramo)
			3 = figlia della bone in cima allo stack, che resta nello stack
		[SRC] Il bit 0 e' BONEFLAG_MATRIXPUSH ed il bit 1 BONEFLAG_MATRIXPOP: l'algoritmo e' equivalente a quello del
		runtime (APB_InitBoneFlags in Tools\Libs\AnimLib\AnimAPI.c). CHR_BONE e' la struttura BONE della libreria di
		animazione (AnimAPI.h), salvata cosi' com'e' in memoria.
		Le animazioni (CAL) hanno una traccia per ogni bone SENZA BONEFLAG_NOTRACK (le bones "*_DYNAMIC").
		LocalMatrix e' la trasformazione della bone rispetto alla bone padre (convenzione a vettore riga: traslazione
		nell'ultima riga). La matrice globale si ottiene come Local * Global(padre).
		I vertici sono espressi nello spazio locale della bone: posizione nel mondo = vertice * Global(bone).
		Verificato sui 79135 vertici MESH1 con due bones diverse: le due posizioni coincidono (entro 0.5 unita' nel
		98.7% dei casi). Applicare solo la traslazione locale della bone (come faceva il vecchio Animation Exporter)
		da' errori medi di 166 unita'.
==================================================================================================================*/


struct CHR_HEADER				// SIZE: 48 bytes
{
	uint32_t CHR_MAGIC;					// [OK] 0 per i personaggi ("NODE" per gli oggetti animati, formato diverso)
	uint32_t TEXTURE_PTR;				// [OK] Offset del blocco materiali/textures
	uint32_t SKELETON_PTR;				// [OK] Offset della prima bone
	uint32_t HeaderSize;				// [OK] Sempre 48: offset del blocco dei punti di aggancio (era UNKNOWN2)
	uint32_t MESH1_PTR;					// [OK] Offset del blocco mesh (SizeMESH12, MESH1, MESH2)
	uint32_t nBONES;					// [OK] Numero di bones
	uint32_t Unknown1;					// [?] Intero 0-27 (non e' il numero di punti di aggancio)
	uint32_t Unknown2;					// [OK] Sempre 1
	uint32_t Garbage[4];				// [GARBAGE] In 175 file contiene 0x0012DE94 (stack), 0x77C73187 (DLL di sistema), 0, 0x1B0
};


enum CHR_BONE_HIERARCHY					// Valori di CHR_BONE.Hierarchy [SRC: BONEFLAG_* in AnimAPI.h]
{
	CHR_HIER_STACK_MASK = 0x003,		// Operazione di stack per la ricostruzione della gerarchia (vedi sopra)
	CHR_BONEFLAG_MATRIXPUSH = 0x001,	// [SRC] Inizio di una ramificazione
	CHR_BONEFLAG_MATRIXPOP = 0x002,		// [SRC] Fine di una ramificazione
	CHR_BONEFLAG_OVERRIDEANIM = 0x004,	// [SRC] (non presente nei file)
	CHR_BONEFLAG_NOTRACK = 0x008,		// [SRC][OK] Bone senza traccia nelle animazioni: tutte le bones "*_DYNAMIC"
	CHR_BONEFLAG_BEGINDYNAMIC = 0x010,	// [SRC] Prima bone di una catena dinamica ("*_DYNAMICBEGIN")
	CHR_BONEFLAG_DYNAMIC = 0x020,		// [SRC][OK] Bone simulata dalla fisica (code di cavallo, ...). Valore tipico 0x28
	CHR_BONEFLAG_HASBOUND = 0x040,		// [SRC] La bone ha una sfera di collisione (CHR_BONE.Vector / Unknown3)
	CHR_BONEFLAG_BOUNDSPHERE = 0x080,	// [SRC] Valore tipico 0xC0 su anca, gambe, braccia, torace, testa
	CHR_BONEFLAG_REMOVABLE = 0x400		// [SRC][OK] Bone che puo' essere ignorata nelle animazioni a basso dettaglio (dita)
};


struct CHR_BONE					// SIZE: 448 bytes
{
	uint32_t Hierarchy;					// [OK] Bit 0-1: operazione di stack per la gerarchia. Altri bit: categoria della bone (CHR_BONE_HIERARCHY)
	char Bone_name[64];					// [OK] Nome della bone (ASCII). [GARBAGE] dopo il terminatore nel 40% delle bones
	uint32_t Bone_name_hashed;			// [OK] GetHashValue(Bone_name) [SRC: boneID]
	uint32_t NULL1;						// [OK] Sempre 0 [SRC: boneidx, calcolato a runtime]
	uint32_t NULL2;						// [OK] Sempre 0 [SRC: matrixIdx]
	float LocalMatrix[16];				// [OK] Trasformazione rispetto alla bone padre (righe 0-2 rotazione/scala, riga 3 traslazione). Era X_scale...W1_trasl
	float InverseBindMatrix[16];		// [OK] Inversa della matrice globale in posa di bind nel 95% delle bones (nelle altre e' diversa:
										//		va usata la matrice globale calcolata dalle LocalMatrix). Era Ragdoll_X...X2_trasl
	char null1[128];					// [OK] Sempre 0 [SRC: matrici pre e post, calcolate a runtime]
	float Vector[4];					// [SRC] Centro della sfera di collisione della bone (x, y, z, 1) [sphere.origin], presente
										//       nelle bones con BONEFLAG_HASBOUND (negli altri casi 0, 0, 0, 1)
	float Unknown3;						// [SRC] Raggio della sfera di collisione [sphere.radius] (es. 70.92, 54.12)
	char null2[92];						// [OK] Sempre 0 [SRC: sphere.pad, physics_flags, uParentBone, pad, vpad]
};


struct CHR_MESH12_HEADER
{
	uint32_t SizeMESH12;				// [OK] Dimensione del blocco mesh (MESH1 + MESH2), esclusa questa cella
};


struct CHR_MESH1_HEADER			// SIZE: 12 bytes (segue CHR_MESH12_HEADER)
{
	uint32_t Unknown1;					// [OK] Sempre 0
	uint32_t ID;						// [OK] Hash del nome della mesh
	uint32_t nVertices;					// [OK] Numero di vertici
};


#pragma pack(push, 1)
struct CHR_MESH1_VERTEX			// SIZE: 38 bytes. Ogni vertice ha due posizioni, una nello spazio di ciascuna delle due bones
{
	int16_t X1;							// [OK] Posizione nello spazio della bone 1 (/ 16)
	int16_t Y1;
	int16_t Z1;
	int16_t X2;							// [OK] Posizione nello spazio della bone 2 (/ 16)
	int16_t Y2;
	int16_t Z2;
	int8_t X1n;							// [OK] Normale nello spazio della bone 1 (/ 127)
	int8_t Y1n;
	int8_t Z1n;
	int8_t X2n;							// [OK] Normale nello spazio della bone 2 (/ 127)
	int8_t Y2n;
	int8_t Z2n;
	int8_t X1tg;						// Tangente (bone 1)
	int8_t Y1tg;
	int8_t Z1tg;
	int8_t X1bn;						// Binormale (bone 1)
	int8_t Y1bn;
	int8_t Z1bn;
	int8_t X2tg;						// Tangente (bone 2)
	int8_t Y2tg;
	int8_t Z2tg;
	int8_t X2bn;						// Binormale (bone 2)
	int8_t Y2bn;
	int8_t Z2bn;
	int16_t Weight;						// [OK] Peso della bone 1 (/ 32767). Il peso della bone 2 e' 1 - peso 1
	uint8_t Bone1;						// [OK] Indice della bone 1. Unica eccezione: COP_A.CHR contiene 170 vertici con Bone1 = Bone2 = 0xFF,
										//		posizione e normali nulle, usati da triangoli dello strip (asset probabilmente incompleto)
	uint8_t Bone2;						// [OK] Indice della bone 2 (uguale a Bone1 se il vertice dipende da una sola bone)
	int16_t U;							// [OK] Coordinata U (/ 32767)
	int16_t V;							// [OK] Coordinata V (/ 32767)
};
#pragma pack(pop)


struct CHR_MESH_STRIP_HEADER
{
	uint32_t nIndices;					// [OK] Numero di indici del triangle strip (seguono nIndices uint16_t)
};


struct CHR_MESH_STRIP
{
	uint16_t Index;
};


struct CHR_MESH_ELEMENT_HEADER
{
	uint32_t nElements;					// [OK] Numero di elementi
};


struct CHR_MESH_ELEMENT			// SIZE: 12 bytes
{
	int16_t nElement_Triangles;			// [OK] Numero di triangoli. Sempre uguale a nElement_Indices - 2
	int16_t nElement_Indices;			// [OK] Numero di indici dello strip usati dall'elemento
	int16_t Offset;						// [OK] Posizione del primo indice nello strip (la parita' determina l'orientamento delle facce)
	uint16_t Material_Ref;				// [OK] Indice del materiale
	uint16_t Unknown1;					// [OK] Sempre 0xFFFF
	uint16_t Draw_mode;					// [OK] 5 = triangle strip, 4 = lista di triangoli (vedi Calculate_Faces)
};


struct CHR_MESH2_LIST
{
	uint32_t nMeshes2;					// [OK] Numero di mesh di tipo 2
};


struct CHR_MESH2_HEADER			// SIZE: 24 bytes
{
	uint32_t Unknown1;					// [OK] Sempre 0
	uint32_t Unknown2;					// [OK] 1 (3972 mesh) oppure 5 (294 mesh) [SRC: MESH.flags = MESHFLAG_VISIBLE (1) |
										//      MESHFLAG_ANIMATED (4): mesh animata da una traccia propria nei CAL]
	uint32_t ID;						// [OK] Hash del nome della mesh [SRC: meshID] (lo stesso hash usato dalle chiavi
										//      MAKE_VISIBLE / MAKE_INVISIBLE e da ParentTracks nei CAL)
	uint32_t Bone_ref;					// [OK] Bone a cui la mesh e' collegata (sempre < nBONES) [SRC: bone]
	int32_t Unknown3;					// [OK] -1 se Unknown2 = 1, indice 0-6 se Unknown2 = 5 [SRC: animidx] [?] probabilmente l'indice della mesh
										//      animata tra le ParentTracks delle animazioni]
	uint32_t nVertices;					// [OK] Numero di vertici
};


#pragma pack(push, 1)
struct CHR_MESH2_VERTEX			// SIZE: 19 bytes (vertici nello spazio della bone Bone_ref)
{
	int16_t X;							// [OK] / 16
	int16_t Y;
	int16_t Z;
	int8_t Xn;							// [OK] / 127
	int8_t Yn;
	int8_t Zn;
	int8_t Xtg;
	int8_t Ytg;
	int8_t Ztg;
	int8_t Xbn;
	int8_t Ybn;
	int8_t Zbn;
	int16_t U;							// [OK] / 32767
	int16_t V;							// [OK] / 32767
};
#pragma pack(pop)


struct CHR_MATERIALS_HEADER
{
	uint32_t nMaterials;				// [OK] Numero materiali
};


struct CHR_MATERIALS_LIST		// SIZE: 24 bytes. Stesso formato di ZONE_MATERIALS_LIST
{
	uint16_t TextureMode;				// Tipologia di materiale
	uint16_t DoubleSided;				// Texture a doppia faccia. Inutile nei CHR perche' hanno il backface culling disattivato
	int32_t Unknown1;					// Sembrerebbe sempre pari a 00000004h, forse indica il numero di slot per textures a seguire
	int32_t DiffuseID;					// Slot 1: texture principale
	int32_t ShadowMapID;				// Slot 2: shadow map
	int32_t BumpSpecID;					// Slot 3: bump map e riflessi
	int32_t FurID;						// Slot 4: texture ARGB usata per fur
};


struct CHR_TEXTURES_HEADER
{
	uint32_t nTextures;					// [OK] Numero textures
};


struct CHR_TEXTURES_LIST		// SIZE: 28 bytes. Seguono subito RAWsize bytes di dati della texture
{
	uint32_t DXT;						// [OK] Formato: 'DXT1' (827611204), 'DXT3' (861165636) o 21 (ARGB 8888). DXT5 non usato
	uint32_t ColourBumpShadow;			// [OK] 2 = diffuse map, 4 = bump map, 7 = ARGB (osservati nei CHR)
	uint32_t Unknown1;					// [OK] Sempre 1
	uint32_t Mips;						// Numero di mipmaps
	uint32_t Xsize;						// Dimensione asse X immagine in pixels
	uint32_t Ysize;						// Dimensione asse Y immagine in pixels
	uint32_t RAWsize;					// Dimensione texture in bytes
};


// Verifica delle dimensioni delle strutture
static_assert(sizeof(CHR_HEADER) == 48, "CHR_HEADER");
static_assert(sizeof(CHR_BONE) == 448, "CHR_BONE");
static_assert(sizeof(CHR_MESH1_HEADER) == 12, "CHR_MESH1_HEADER");
static_assert(sizeof(CHR_MESH1_VERTEX) == 38, "CHR_MESH1_VERTEX");
static_assert(sizeof(CHR_MESH_ELEMENT) == 12, "CHR_MESH_ELEMENT");
static_assert(sizeof(CHR_MESH2_HEADER) == 24, "CHR_MESH2_HEADER");
static_assert(sizeof(CHR_MESH2_VERTEX) == 19, "CHR_MESH2_VERTEX");
static_assert(sizeof(CHR_MATERIALS_LIST) == 24, "CHR_MATERIALS_LIST");
static_assert(sizeof(CHR_TEXTURES_LIST) == 28, "CHR_TEXTURES_LIST");


/*==================================================================================================================
	FORMATO CHR DEGLI OGGETTI ANIMATI ("NODE") [OK] (684 file, 3497 nodi)

	Fonti: [SRC] runtime Libs\AnimLib\AnimAPI.c (struttura NODE, APBFLAG_ANIMATEDSET: calcolo delle matrici dei nodi in
	AO_ProcessHierachy, AT_GetSetFrameData2) e AnimEdit\EditorToGame.cpp (CPropHierachy::ExportToGame).

	STRUTTURA DEL FILE
		CHR_NODE_HEADER (12 bytes)
		Per ogni nodo: uint32_t lunghezza del nome (multiplo di 4, NUL compreso), nome, CHR_NODE_DATA (120 bytes)
		Il resto del file fino a multiplo di 2048 bytes e' [GARBAGE] (memoria del builder)

	Ogni nodo e' un transform di Maya: nessuna geometria nel CHR, le mesh sono quelle degli oggetti del file ZONE del
	livello con lo stesso nome (ZONE_OBJECT_INFO.Hash = GetHashValue(nome del nodo in maiuscolo)), con i vertici nello
	spazio dell'oggetto. Matrice di un nodo (vettore riga, come Maya):
		Tr(-SP) * S * Tr(SP) * Tr(ST) * Tr(-RP) * RO * R * Tr(RP) * Tr(RT) * Tr(T) * matrice del padre
	con T, R (quaternione) e S dalla traccia del CAL di indice uguale al nodo (ANIM_SETTSCALE 1, ANIM_SETRSCALE 16383,
	ANIM_SETSSCALE 255: traslazione e scala assolute) e i pivot dal CHR. Gerarchia: come per le bones (BONEFLAG_MATRIXPOP
	ripristina la matrice salvata, BONEFLAG_MATRIXPUSH salva la matrice corrente prima di applicare il nodo).
==================================================================================================================*/
struct CHR_NODE_HEADER			// SIZE: 12 bytes
{
	uint32_t Magic;						// [OK] "NODE" (0x45444F4E)
	uint32_t Size;						// [OK] Dimensione dei nodi (header escluso: 12 + Size = fine dell'ultimo nodo)
	uint32_t nNodes;					// [OK] Numero di nodi (= tracce delle animazioni del CAL omonimo)
};


struct CHR_NODE_DATA			// SIZE: 120 bytes, dopo la lunghezza ed il nome del nodo
{
	uint32_t Hash;						// [OK] GetHashValue(nome) [SRC: boneID]
	uint32_t Flags;						// [OK] CHR_NODEFLAG_PUSH / CHR_NODEFLAG_POP [SRC: BONEFLAG_MATRIXPUSH / BONEFLAG_MATRIXPOP]
	float RotatePivot[3];				// [SRC] rotatePivot di Maya
	float ScalePivot[3];				// [SRC] scalePivot (uguale a RotatePivot in 3486 nodi su 3497)
	float RotatePivotTranslate[3];		// [SRC] rotateTranslate (diverso da 0 in 210 nodi)
	float ScalePivotTranslate[3];		// [SRC] scaleTranslate (sempre 0)
	float RotateOrient[16];				// [SRC] rotateOrient: matrice 4x4 (rotate axis di Maya, identita' tranne 14 nodi)
};


#define CHR_NODE_MAGIC			0x45444F4E
#define CHR_NODEFLAG_PUSH		1
#define CHR_NODEFLAG_POP		2

static_assert(sizeof(CHR_NODE_HEADER) == 12, "CHR_NODE_HEADER");
static_assert(sizeof(CHR_NODE_DATA) == 120, "CHR_NODE_DATA");

struct CHR_ATTACH_GROUP			// SIZE: 8 bytes [HIERACHY]: gruppo di nodi agganciati ad una bone (vedi OGGETTI AGGANCIATI)
{
	uint32_t nNodes;					// [OK] Numero di nodi del gruppo [nNodes] (0 = voce vuota da saltare)
	uint32_t BoneHash;					// [OK] GetHashValue del nome della bone di aggancio (es. PALM_R, HIP) [flags]
};


struct CHR_ATTACH_NODE			// SIZE: 176 bytes, dopo nome, hash, flags e padding [NODE]
{
	float RotatePivot[4];				// [OK] rotatePivot di Maya (x, y, z, 1)
	float ScalePivot[4];				// [OK] scalePivot
	float RotatePivotTranslate[4];		// [SRC] rotateTranslate
	float ScalePivotTranslate[4];		// [SRC] scaleTranslate
	float RotateOrient[16];				// [SRC] rotate axis (matrice 4x4 a vettore riga)
	float Rotate[4];					// [OK] Rotazione di riposo: quaternione x, y, z, w
	float Translate[4];					// [OK] Traslazione di riposo
	float Scale[4];						// [OK] Scala di riposo
};
static_assert(sizeof(CHR_ATTACH_GROUP) == 8, "CHR_ATTACH_GROUP");
static_assert(sizeof(CHR_ATTACH_NODE) == 176, "CHR_ATTACH_NODE");
