#pragma once


/*==================================================================================================================
	FORMATO RMX (versione 3.6) - Tomb Raider: The Angel of Darkness

	Le informazioni sono state ricavate analizzando tutti i 57 file RMX disponibili (1701 stanze, ~38000 nodi).
	Legenda dei commenti:
		[OK]		verificato su tutti i file
		[PROB]		molto probabile, verificato su un sottoinsieme o dedotto da piu' indizi
		[LOG]		verificato con i log dell'"Export manager" di WorldEdit (Release\*.log, 2003): 1393 stanze e 32952 nodi
					di 34 livelli. Il log elenca per ogni stanza i nodi di ogni lista ("Begin Path": objects = lista 0,
					lights = 1, character locators = 2, trigger locators = 3, waypoints = 5, water locators = 7,
					FX emitters = 8, scenery = 10, Audio locators = 11) con offset ("Buffer offset") e hash: coincidono
					tutti (tranne PARIS1, versione diversa del livello, e un trigger in piu' nel GMX di PARIS2H)
		[?]			ipotesi / significato sconosciuto (sono indicati i valori osservati)
		[GARBAGE]	memoria non inizializzata scritta dal builder originale: il contenuto varia da file a file e
					contiene puntatori di stack (0x0012xxxx), dell'eseguibile (0x004xxxxx-0x005xxxxx), dell'heap e
					residui di altri dati. NON va interpretato.

	STRUTTURA GENERALE DEL FILE
		RMX_HEADER
		uint32_t Offset[nRooms]						Offset assoluto di ogni stanza (RMX_OFFSETS)
		padding fino a 16 bytes						La prima stanza e' sempre allineata a 16 bytes
		Stanza 0, stanza 1, ...						Ogni stanza e' un blocco contiguo che termina all'inizio della successiva
		padding fino a multiplo di 2048 bytes		[GARBAGE] in 41 file su 57 (il file ha sempre dimensione multipla di 2048)

	STRUTTURA DI UNA STANZA (tutti gli offset interni sono relativi all'inizio della stanza)
		RMX_ROOM (368 bytes)
		Nodi, raggruppati per lista. Ogni lista e' una lista doppiamente concatenata (BegPrev/BegNext) di nodi
			dello stesso tipo; il tipo del nodo coincide con l'indice della lista nella stanza (RMX_NODE_TYPE).
			Le liste sono memorizzate in ordine: OBJ, LIGHT, CHARLOC, TRIGGER, WAYPOINT, WATER, EMITTER,
			PS2_ROOM_OBJ, AUDIO_LOCATOR (le liste vuote non occupano spazio)
		Maschera delle stanze (RoomMask_Begin .. RoomMask_End) + padding fino a 16 bytes [GARBAGE]
		Portali (RMX_PORTAL x nPortals)
		Lista delle zone (ZoneList: uint32_t x nZones) + padding fino a 16 bytes
		Verificato: queste strutture coprono interamente ogni stanza, senza buchi e senza sovrapposizioni.

	DIMENSIONE DEI NODI
		Ogni nodo e' composto da RMX_NODE_HEADER (112 bytes) + dati specifici del tipo + eventuale coda variabile
		(messaggi, azioni, collegamenti). La dimensione totale e' sempre arrotondata a un multiplo di 16 bytes.
		La dimensione di un nodo si ricava come BegNext - offset (per l'ultimo nodo della lista dalla formula
		indicata per ogni tipo, oppure dall'inizio del blocco successivo).

	COORDINATE E UNITA'
		Le rotazioni sono in gradi. L'asse verticale e' Z (la rotazione usata di gran lunga piu' spesso e' Zrot e
		i portali orizzontali, normale +-Z, sono i meno numerosi).
		I colori delle luci sono float nella scala PS2 0-128 (128 = intensita' piena): valore = 128 * byte / 255.
		Alcuni colori sono memorizzati come float "biased" 2^23 + n (0x4B0000nn), il trucco usato dalla VU della PS2
		per tenere interi in registri float: il valore intero e' (bits & 0xFF).
==================================================================================================================*/


enum RMX_NODE_TYPE						// RMX_NODE_HEADER.Type = indice della lista nella stanza [OK]
{
	RMX_NODE_OBJ = 0,					// Oggetti interattivi (porte, leve, casse, ...)
	RMX_NODE_LIGHT = 1,					// Luci statiche (punto con raggio interno/esterno)
	RMX_NODE_CHARLOC = 2,				// Posizioni di partenza dei personaggi (Lara, nemici, NPC)
	RMX_NODE_TRIGGER = 3,				// Volumi trigger con lista di azioni
	RMX_NODE_PORTAL = 4,				// Mai usato come nodo (i portali sono un blocco separato, RMX_PORTAL)
	RMX_NODE_WAYPOINT = 5,				// Punti del grafo di navigazione dell'IA
	RMX_NODE_ACTOR = 6,					// Mai usato
	RMX_NODE_WATER = 7,					// Volumi d'acqua
	RMX_NODE_EMITTER = 8,				// Emettitori di effetti (nebbia, particelle, ...)
	RMX_NODE_CAMERA = 9,				// Mai usato
	RMX_NODE_PS2_ROOM_OBJ = 10,			// Istanze degli oggetti statici della ZONE (slot PS2_OBJ_SLOTS) posizionate nella stanza
	RMX_NODE_AUDIO_LOCATOR = 11,		// Sorgenti audio ambientali
	RMX_NODE_ROOM = 12					// La stanza stessa (RMX_ROOM.Type)
};


struct RMX_HEADER						// SIZE: 20 bytes
{
	float Version;						// [OK] Sempre 3.6
	uint32_t DefaultZone;				// [OK] Zona caricata di default all'avvio del livello (sempre 0 nei file noti)
	uint32_t DefaultReverb;				// [OK] Preset di riverbero di default (0-18, 0xFFFFFFFF in 2 file)
	uint32_t null;						// [GARBAGE] 0 in 26 file su 57, negli altri contiene puntatori di heap
	uint32_t nRooms;					// [OK] Numero di stanze contenute nel livello
};


struct RMX_OFFSETS
{
	uint32_t Offset;					// [OK] Offset assoluto dell'inizio di ogni stanza (ripetuto nRooms volte)
};


/*------------------------------------------------------------------------------------------------------------------
Header comune a tutti i nodi (e alla stanza stessa). I campi di posizione/rotazione/scala hanno lo stesso significato
per tutti i tipi, salvo dove indicato nella struttura specifica.
I campi "sempre 0" sono [GARBAGE] nei waypoint (RMX_NODE_WAYPOINT), che contengono memoria non inizializzata ovunque
tranne in posizione, hash, tipo e collegamenti.
------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------
[SRC: Debug.elf] I nodi del file RMX sono le strutture del runtime del gioco (copiate in memoria e "fissate"), descritte
dalle informazioni di debug (stabs) di TRAOD/Reference/ProView/Debug.elf, l'eseguibile PS2 di sviluppo:
	tagMAP_BASENODE_LITE (112 bytes)	luci (MAP_LIGHTNODE_LITE), istanze (tagMAP_SCENERY), stanze (ROOM_HEADER_TAG)
	tagMAP_BASENODE (352 bytes)			tutti gli altri nodi: ai 112 bytes seguono
		+112 pEventCurrent, +116 nEventRemain, +120 uint16 Room, +122 uint16 SrcRoom (runtime, 0 nel file)
		+124 hScript					hash dello script AMX (SCX) collegato al nodo ("LinkedHash" nelle strutture sotto)
		+128 pMsgHandlers[8], +160 userVector[8], +288 userValue[16] (runtime, 0 nel file)
	tagMAP_OBJNODE (528), tagMAP_CHARNODE (416), tagMAP_TRIGNODE (416), tagMAP_WAYPOINT (368), tagMAP_WATERNODE (432),
	tagMAP_FXNODE (416), tagMAP_AUDIONODE (368): campi specifici da +352
I nomi originali dei campi sono indicati con [SRC: nome]. I puntatori (p*) e i campi del runtime sono 0 o residui nel file.
------------------------------------------------------------------------------------------------------------------*/
struct RMX_NODE_HEADER					// SIZE: 112 bytes
{
	uint32_t BegPrev;					// [OK] Offset (dall'inizio della stanza) del nodo precedente della stessa lista, 0 se primo
	uint32_t BegNext;					// [OK] Offset (dall'inizio della stanza) del nodo successivo della stessa lista, 0 se ultimo
	char null1[24];						// [OK] Sempre 0 [SRC: ctrl[16] (puntatori della lista), UserPad, pListBase].
										//		Nei Fakes della ZONE UserPad e' l'indice del Fake (vedi ZONE_FAKES_ELEMENT)
	float Xpos;							// [OK] Posizione (coordinate del mondo) [SRC: pos]
	float Ypos;
	float Zpos;
	float Wpos;							// [OK] Sempre 1.0
	float Xrot;							// [OK] Rotazione in gradi [SRC: rot]. Quasi sempre solo Zrot e' diversa da 0
	float Yrot;
	float Zrot;
	float Wrot;							// [OK] Sempre 0
	char null2[16];						// [OK] Sempre 0 [SRC: vel]
	float Xscal;						// [OK] Scala [SRC: scl]. Per trigger, acqua ed emitter e' la semi-dimensione del volume (vedi strutture)
	float Yscal;
	float Zscal;
	uint32_t null3;						// [OK] Sempre 0
	uint32_t null4;						// [OK] Sempre 0 [SRC: fTimer (tagMAP_BASENODE) / type (tagMAP_BASENODE_LITE)]
	uint32_t Hash;						// [LOG] GetHashValue del nome del nodo (es. "herod__chocolate1", maiuscole e minuscole come nell'editor).
										//		Usato per i riferimenti tra nodi (trigger, waypoint, ...) e dagli
										//		script AMX (es. mapFindHashedNodeTypedAMX). Nodi con nomi simili hanno hash che differiscono
										//		solo nei bit bassi (es. LIGHT01, LIGHT02, ...)
	uint32_t Type;						// [OK] Tipo del nodo (RMX_NODE_TYPE), coincide con la lista che lo contiene [SRC: NodeType]
	uint32_t Flags;						// [SRC: NodeFlags] Flag del nodo: luci sempre 4, PS2_ROOM_OBJ 4 o 12, CHARLOC sempre 1,
										//		trigger/emitter 0 o 1, audio 0 o 1, altri 0.
										//		Bit 0 (1): nodo attivo (trigger ed emitter disattivati finche' un messaggio non li attiva;
										//		mapUpdateTrigger ignora i trigger senza questo bit). Bit 2 (4): nodo statico.
										//		Bit 3 (8): Fake orientato verso la telecamera (mapDrawFakes: posizione relativa alla telecamera)
};


/*------------------------------------------------------------------------------------------------------------------
Messaggio/evento associato ad un nodo (code di oggetti, emitter e acqua). Gli ID dei messaggi sono gli stessi usati
nelle azioni dei trigger (RMX_TRIGGER_ACTION.MsgID). Valori di MsgID piu' frequenti: 57, 37, 46, 25, 9, 68, 3, 4, 0, 67.
------------------------------------------------------------------------------------------------------------------*/
struct RMX_MSG							// SIZE: 8 bytes
{
	uint32_t MsgID;						// [LOG] Messaggio NM_* ricevuto dal nodo (stessa numerazione di map_msg.h usata da Export_EVX)
	uint32_t Param;						// [LOG] Indice del gruppo di eventi del file EVX avviato quando il nodo riceve il messaggio
										//		(log: "Msg [64 : NM_TAKEFINISH] , Event [3 : give_diary]"). 5660 code verificate
};


struct RMX_ROOM			// SIZE: 368 bytes [SRC: ROOM_HEADER_TAG]. I primi 112 bytes sono un RMX_NODE_HEADER (Type = 12)
{
	char null1[100];					// [OK] Always 0 (header di nodo non usato)
	uint32_t Room_ID;					// [LOG] GetHashValue del nome della stanza (es. "carviers__room2") = RMX_NODE_HEADER.Hash
	uint32_t Unknown26;					// [OK] Tipo di nodo: sempre 12 (RMX_NODE_ROOM)
	uint32_t Unknown27;					// [OK] Flags: sempre 0
	float Room_Xt;						// [OK] Room X position
	float Room_Yt;						// [OK] Room Y position
	float Room_Zt;						// [OK] Room Z position
	float pad1;							// [OK] Sempre 0
	float BB_Xmin;						// [OK] Bounding box X min (coordinate del mondo) [SRC: Min]
	float BB_Ymin;						// [OK] Bounding box Y min
	float BB_Zmin;						// [OK] Bounding box Z min
	float pad2;							// [OK] Sempre 0 [SRC: StripCount, runtime]
	float BB_Xmax;						// [OK] Bounding box X max
	float BB_Ymax;						// [OK] Bounding box Y max
	float BB_Zmax;						// [OK] Bounding box Z max
	float pad3;							// [OK] Sempre 0 [SRC: StripIndices, runtime]
	uint32_t null5;						// [OK] Always 0x00000000 [SRC: pOctreeHdr, runtime]
	uint32_t null6;						// [OK] Always 0x00000000 [SRC: pOctree, runtime]
	uint32_t Room_ID2;					// [OK] Copia di Room_ID [SRC: BindHash]
	uint32_t null7;						// [OK] Always 0x00000000 [SRC: BindFlag]
	uint32_t null8;						// [OK] Always 0x00000000 [SRC: pPCData_Dummy]
	uint32_t null9;						// [OK] Always 0x00000000 [SRC: CollStripCount]

	// Liste di nodi [SRC: NodeLists[14]]: offset (dall'inizio della stanza) del primo e dell'ultimo nodo, 0 se la lista e' vuota [OK]
	// L'indice della coppia coincide con il tipo dei nodi della lista (RMX_NODE_TYPE)
	uint32_t NL_OBJ_First;				// Lista 0: RMX_OBJECT
	uint32_t NL_OBJ_Last;
	uint32_t NL_LIGHT_First;			// Lista 1: RMX_LIGHT
	uint32_t NL_LIGHT_Last;
	uint32_t NL_CHARLOC_First;			// Lista 2: RMX_CHARLOC (personaggi: Lara e nemici). Dimensione 416 bytes
	uint32_t NL_CHARLOC_Last;
	uint32_t NL_TRIGGER_First;			// Lista 3: RMX_TRIGGER
	uint32_t NL_TRIGGER_Last;
	uint32_t NL_PORTAL_First;			// Lista 4: mai usata (sempre 0)
	uint32_t NL_PORTAL_Last;
	uint32_t NL_WAYPOINT_First;			// Lista 5: RMX_WAYPOINT
	uint32_t NL_WAYPOINT_Last;
	uint32_t NL_ACTOR_First;			// Lista 6: mai usata (sempre 0)
	uint32_t NL_ACTOR_Last;
	uint32_t NL_WATER_First;			// Lista 7: RMX_WATER
	uint32_t NL_WATER_Last;
	uint32_t NL_EMITTER_First;			// Lista 8: RMX_EMITTER (nebbia, particelle, ...)
	uint32_t NL_EMITTER_Last;
	uint32_t NL_CAMERA_First;			// Lista 9: mai usata (sempre 0)
	uint32_t NL_CAMERA_Last;
	uint32_t P11_PS2_Room_Obj_First;	// Lista 10: RMX_PS2_ROOM_OBJ
	uint32_t P11_PS2_Room_Obj_Last;
	uint32_t P12_Audio_Locator_First;	// Lista 11: RMX_AUDIO_LOCATOR
	uint32_t P12_Audio_Locator_Last;
	uint32_t Offset22;					// Lista 12: mai usata (sempre 0)
	uint32_t Offset23;
	uint32_t Offset24;					// Lista 13: mai usata (sempre 0)
	uint32_t Offset25;

	uint32_t P15_Portal_First;			// [OK] Offset del primo RMX_PORTAL (0 se nessuno) [SRC: pPortals]
	uint32_t nPortals;					// [OK] Numero di portali (0-15)
	uint32_t Offset28;					// [OK] ZoneList [SRC: pZones]: offset della lista delle zone a cui appartiene la stanza (uint32_t x Offset29)
										//		I valori sono indici delle zone (file .Z00, .Z01, ...) e sono sempre < numero di zone
	uint32_t Offset29;					// [OK] Numero di elementi della ZoneList (0-3) [SRC: nZones]
	uint32_t Indoor;					// [SRC: Indoor] Stanza al chiuso (bool, 1 in 95 stanze)
	uint16_t ReverbID;					// [OK] Preset di riverbero della stanza (0-16) [SRC: Reverb]
	uint8_t Collision_set;				// [OK] Set di collisioni (0-21, diverso da 0 solo in poche stanze) [SRC: CollisionFlipIndex]
	uint8_t Collision_set_current;		// [OK] Sempre 0 nel file: il gioco copia "Collision_set" qui all'avvio del livello
										//		[SRC: OriginalCollisionFlipIndex]
	uint32_t Offset32;					// [OK] RoomMask_Begin [SRC: RoomVisibility]: offset di una maschera di bit con un bit per stanza
										//		(ceil(nRooms/32) uint32_t). Il bit della stanza stessa e' sempre a 1, i bit oltre nRooms sempre
										//		a 0. La relazione e' simmetrica nel 96% delle coppie: stanze visibili dalla stanza
	uint32_t Offset33;					// [SRC: FakeVisibility] Offset della maschera di visibilita' dei Fakes (sempre subito dopo quella delle
										//		stanze, che termina quindi qui; prosegue fino ai portali). Il Fake con indice i
										//		(ZONE_FAKES_ELEMENT.Index) viene disegnato solo se il bit i della maschera della stanza
										//		corrente e' a 1 (mapDrawFakes). Di solito tutti i bit dei Fakes del livello sono a 1
	int32_t iFlipRooms[4];				// [SRC: iFlipRooms] Stanze "flip" (versioni alternative della stanza, scambiate da un messaggio
										//		NM_*FLIP*, mapFlipRoom): indici delle stanze, -1 se non usato. 0 nelle stanze flip stesse
										//		(es. PARIS3: stanze 66-72 -> 74-80 e 81-87)
	uint32_t bIsFlipRoom;				// [SRC: bIsFlipRoom] 1 se la stanza e' la versione alternativa di un'altra (17 stanze, le uniche
										//		senza maschere di visibilita')
	uint32_t nFlipRooms;				// [SRC: nFlipRooms] Numero di elementi validi di iFlipRooms (0-2)
	uint32_t FlipIndex;					// [OK] Sempre 0 [SRC: FlipIndex] Versione attiva (runtime)
	uint32_t OriginalStripCount;		// [OK] Sempre 0 [SRC: OriginalStripCount] (runtime)
	uint32_t OriginalStripIndices;		// [OK] Sempre 0 [SRC: OriginalStripIndices] (runtime)
	uint32_t RoomFlags;					// [OK] Sempre 0 [SRC: RoomFlags]
};


/*------------------------------------------------------------------------------------------------------------------
Oggetto interattivo (porte, leve, ascensori, casse, ...). 2963 nodi.
Dimensione: 528 + 8 * nMessages arrotondata a 16 [OK su 2958/2963]
------------------------------------------------------------------------------------------------------------------*/
struct RMX_OBJECT		// SIZE: 528 bytes + coda [SRC: tagMAP_OBJNODE]
{
	RMX_NODE_HEADER Node;				// Type = 0, Flags = 0
	char null1[12];						// [OK] Sempre 0
	uint32_t LinkedHash;				// [SRC: hScript] Hash dello script AMX collegato al nodo, presente in 112 oggetti
	char null2[232];					// [OK] Sempre 0 (campi runtime di tagMAP_BASENODE + pOctPrev, pOctNext)
	uint32_t ObjFlags;					// [SRC: ObjFlags] Campo di bit (valori osservati: 0x40, 0x10, 0x2, 0x200, 0x1000, 0x20 << 16,
										//		0x402 << 16, ...). Bit noti dal codice: 0x10 oggetto che oscilla (RockX/RockY, gli emitter
										//		collegati lo seguono), 0x10000 senza fisica delle collisioni, 0x20000 apribile,
										//		0x40000 aperto, 0x1000000 interruttore acceso. Molti bit sono stato del runtime
	uint32_t null3;						// [OK] Sempre 0 [SRC: AnimActionFlags]
	uint16_t ZoneSlot;					// [OK] Slot (ZONE_PS2_OBJ_SLOTS) dell'oggetto della ZONE istanziato da questo nodo, con
										//		posizione/rotazione/scala del nodo. Era Unknown1 / ObjectID. Verificato: i valori dei 2963
										//		oggetti sono tutti diversi (2 ripetizioni), coprono 2879 dei 3291 slot non istanziati da
										//		RMX_PS2_ROOM_OBJ e solo 82 puntano a slot vuoti in tutte le zone del livello [SRC: MeshN]
	uint16_t ObjectIndex;				// [LOG] Indice (da 0) dell'"Object Template" dell'oggetto nella lista dei template del livello
										//		(log: "Exporting Object Template : Lara Drawer", ...; 34 livelli su 34). I nomi dei
										//		template non sono nel GMX (solo residui di memoria nel padding del file EVX) [SRC: TemplateKey]
	uint32_t nMessages;					// [OK] Numero di RMX_MSG in coda al nodo (0-10) [SRC: uint16 nEventBindings + uint16 Strength (0)]
	char null4[24];						// [OK] Sempre 0 [SRC: InitialStrength, Weight, PuzzleCount, RockX, RockY, VelScale, pAO,
										//		pTrig, drawOverride: copiati dal template o runtime]
	uint16_t null5;						// [OK] Sempre 0 [SRC: frameTick, charPad]
	uint16_t absDBKey;					// [SRC: absDBKey] Chiave nel database degli oggetti (0-606, 0 in 725 oggetti, piu' frequente 160)
	uint32_t null6;						// [OK] Sempre 0 [SRC: collPhysics]
	uint32_t null7;						// [OK] Sempre 0 [SRC: pMatrixStart]
	uint32_t nElements;					// [SRC: nElements] Numero di elementi (matrici) dell'oggetto, 1-37 (1 nel 51% dei casi)

	// Illuminazione dell'oggetto: 3 luci direzionali + luce ambientale (stile PS2) [SRC: LightDir[3], ColourMatrix]
	float Light1_dir[4];				// Direzione della luce 1 (vettore unitario x, y, z) + 1.0. Tutto 0 se la luce non e' usata
	float Light2_dir[4];				// Direzione della luce 2
	float Light3_dir[4];				// Direzione della luce 3
	float Light1_RGBA[4];				// Colore della luce 1 (R, G, B in scala 0-128), A = 128 se la luce e' usata, altrimenti 0
	float Light2_RGBA[4];				// Colore della luce 2
	float Light3_RGBA[4];				// Colore della luce 3
	uint32_t Ambient_RGBA[4];			// Colore ambientale come float "biased" 0x4B0000nn: R, G, B (0-255) e A (sempre 0x4B000080 = 128)
										//		Nella vecchia versione era indicato come "Always 0x0000004B" (byte letti al contrario)
	// RMX_MSG Messages[nMessages];
};


struct RMX_LIGHT		// SIZE: 144 bytes. 14277 nodi
{
	uint32_t BegPrev;					// [OK] RMX_NODE_HEADER.BegPrev
	uint32_t BegNext;					// [OK] RMX_NODE_HEADER.BegNext
	char null1[24];						// [OK] Always 0
	float Xpos;							// [OK] X position of the center of the light
	float Ypos;							// [OK] Y position of the center of the light
	float Zpos;							// [OK] Z position of the center of the light
	float Wpos;							// [OK] Always 1.0
	char null2[32];						// [OK] Rotazione (quasi sempre 0, raramente Yrot = 0.5 o Zrot) + 16 bytes a 0
	char null3[12];						// [OK] Scala: always 1.0
	char null4[8];						// [OK] Always 0
	uint32_t Light_ID;					// [OK] Hashed name of the light (RMX_NODE_HEADER.Hash)
	uint32_t null5;						// [OK] Tipo di nodo: always 1 (RMX_NODE_LIGHT)
	uint32_t Static_flag;				// [OK] Flags: always 4 (4 means it is a static node and it's not linked to any script)
	float R;							// [OK] Red light intensity, scala PS2 0-128 (valore = 128 * byte / 255)
	float G;							// [OK] Green light intensity (0-128)
	float B;							// [OK] Blue light intensity (0-128)
	uint32_t null6;						// [OK] Always 0
	float intRadius;					// [OK] Internal sphere radius
	float extRadius;					// [OK] External sphere radius
	char null7[8];						// [OK] Always 0
};


/*------------------------------------------------------------------------------------------------------------------
Posizione di partenza di un personaggio (Lara, nemici, NPC). 433 nodi.
------------------------------------------------------------------------------------------------------------------*/
struct RMX_CHARLOC		// SIZE: 416 bytes [SRC: tagMAP_CHARNODE]
{
	RMX_NODE_HEADER Node;				// Type = 2, Flags = 1. Usa Zrot per l'orientamento, scala sempre 1 (tranne 1 caso)
	char null1[12];						// [OK] Sempre 0
	uint32_t CharHash;					// [SRC: hScript] Hash dello script AMX del personaggio.
										//		[OK] Se diverso da 0 (257 nodi su 433) e' spesso GetHashValue() del nome "di famiglia" del personaggio:
										//		es. hash("GUARD") per GUARD/GUARDB/GUARDC, hash("LARA") per LARAC1/LARAD. 132 valori non corrispondono
										//		a nessun nome di ACTOR.DB (e non sono hash degli script SCX)
	char null2[240];					// [OK] Sempre 0 (campi runtime di tagMAP_BASENODE + boneTargetPos)
	uint32_t ActorID;					// [OK] ID del personaggio nel database ACTOR.DB (campo ID) [SRC: nKeyCharacter]: corrisponde in tutti i 433 nodi.
										//		Identifica la variante esatta (es. LARAD, GUARDB, STRNMSKA). Vedi RMX_GetActor()
	uint32_t Flags1;					// [SRC: w] Potenza di 2: 256, 128, 64, 32
	uint32_t Flags2;					// [SRC: h] 768, 64, 512, 76
	uint32_t null3;						// [OK] Sempre 0 [SRC: pActor]
	uint32_t null4;						// [OK] Sempre 0 [SRC: hScript del personaggio, runtime]
	uint32_t CharFlags;					// [SRC: nFlags] Flag CHARFLAGS_* (enum in Debug.elf): 1 COLLISION, 2 RESERVED2, 4 ENABLE,
										//		0x10 TRIGGER (attiva i trigger con il bit 0x2), 0x20 ACTIVATE, 0x40 TALK, 0x80 RAGDOLL,
										//		0x400 RAGDOLL_TRACK, 0x800 RAGDOLL_BLEND, 0x8000 STATIC, 0x10000 ONSCREEN, 0x20000 ANIMATED,
										//		0x40000 TARGETABLE, 0x80000 TARGETED, 0x100000 DORMANT, 0x200000 SNOOZE, 0x400000 CUTSCENE,
										//		0x800000 DROPPEDITEM, 0x1000000 INVISIBLE, 0x2000000 NOTARGET, 0x4000000 BLOCKER,
										//		0x8000000 BOSS, 0x10000000 NOCULL. Valori: 1, 65, 5, 0x00400057, ...
	float Weight;						// [SRC: Weight] Peso: 50.0 nel 94% dei casi, altrimenti 64, 32, 1
	uint32_t null5[3];					// [OK] Sempre 0 [SRC: aoObject, animList, iLifeDefault]
	uint32_t LinkedObject;				// [SRC: dropTarget] Hash di un RMX_OBJECT (37 casi), altrimenti 0: oggetto lasciato/raccolto
	uint32_t null6;						// [OK] Sempre 0 [SRC: boneTarget]
};


/*------------------------------------------------------------------------------------------------------------------
Volume trigger. Quando viene attivato invia una serie di messaggi a nodi bersaglio. 2738 nodi. [SRC: tagMAP_TRIGNODE]
Il volume e' un parallelepipedo centrato in Node.pos, ruotato di Node.rot, con semi-dimensioni Node.scal.
Il trigger e' attivo solo con il bit 0 di Node.Flags (vedi RMX_NODE_HEADER); quando scatta invia a se stesso NM_ACTORCOLLIDE (4)
e i messaggi delle azioni partono verso i bersagli (mapUpdateTrigger, mapTriggerDispatchMessages).
Dimensione: 416 + 12 * nActions + 8 * nConditions arrotondata a 16 [OK su 2737/2738]
------------------------------------------------------------------------------------------------------------------*/
struct RMX_TRIGGER_ACTION				// SIZE: 12 bytes
{
	uint32_t MsgID;						// [OK] Messaggio da inviare (stessi ID di RMX_MSG). Piu' frequenti: 5, 67, 134, 8, 44, 14, 56, 82, 70
	uint32_t TargetHash;				// [OK] Hash del nodo bersaglio. Puo' essere un nodo di qualsiasi tipo (oggetti, personaggi, trigger,
										//		emitter, acqua, audio, stanze) oppure un hash esterno (es. 0x348BD399, usato con MsgID 8)
	uint32_t Param;						// [OK] Parametro: di solito 0, altrimenti un valore (1, 120, 180, 300, ...) o l'hash di un waypoint
};


#define RMX_TRIGFLAG_PLAYER			0x0001		// [SRC] Attivato da Lara (giocatore) dentro il volume (2615 trigger)
#define RMX_TRIGFLAG_CHARACTERS		0x0002		// [SRC] Attivato dai personaggi con CHARFLAGS_TRIGGER (0x10) (14)
#define RMX_TRIGFLAG_ONESHOT		0x0004		// [SRC] Si disattiva (azzera il bit 0 di Node.Flags) dopo essere scattato (829)
#define RMX_TRIGFLAG_TRACKED		0x0020		// [SRC] Tiene la lista di chi e' dentro: scatta solo all'ingresso; all'uscita invia
												//		NM_CHAREXIT (9) o NM_CAMERAEXIT (27) (mapUpdateTrackedTrigger) (1304)
#define RMX_TRIGFLAG_BOUND			0x0040		// [SRC] Segue il nodo BindNodeHash (posizione del nodo + offset) (66)
#define RMX_TRIGFLAG_ACTION			0x0080		// [SRC] Con PLAYER: serve il tasto azione (icona dell'azione) (532)
#define RMX_TRIGFLAG_FACING			0x0100		// [SRC] Lara deve essere rivolta come il trigger (prodotto scalare delle direzioni)
#define RMX_TRIGFLAG_CAMERA			0x0200		// [SRC] Attivato dalla telecamera (sfera di raggio 256) (69)
#define RMX_TRIGFLAG_ACTUATOR		0x0400		// [SRC] Attivato dall'oggetto ActuatorHash che tocca il volume (box-box) (40)
#define RMX_TRIGFLAG_REPEAT			0x0800		// [SRC] Con TRACKED e ACTION: dopo l'azione puo' scattare di nuovo restando dentro (445)
#define RMX_TRIGFLAG_NOACTION		0x1000		// [SRC] Tasto azione disabilitato (impostato a runtime da un messaggio)
#define RMX_TRIGFLAG_BONE			0x2000		// [SRC] Con BOUND: segue l'osso BindShapeHash del nodo (indice risolto al primo
												//		aggiornamento, poi il bit viene azzerato) (60)

struct RMX_TRIGGER		// SIZE: 416 bytes + coda
{
	RMX_NODE_HEADER Node;				// Type = 3, Flags = 0/1. Xscal/Yscal/Zscal = semi-dimensioni del volume
	char null1[12];						// [OK] Sempre 0
	uint32_t LinkedHash;				// [SRC: hScript] Hash dello script AMX collegato, presente in 4 trigger
	char null2[224];					// [OK] Sempre 0
	float BoxMin[3];					// [OK] Box locale: -Xscal, -Yscal, -Zscal [SRC: Min]
	uint32_t MinorAxis;					// [SRC: MinorAxis] Quasi sempre 0 (6 trigger con valori float)
	float BoxMax[3];					// [OK] Box locale: +Xscal, +Yscal, +Zscal [SRC: Max]
	uint32_t TriggerFlags;				// [SRC: TriggerFlags] Bit RMX_TRIGFLAG_* (es. 1, 33, 5, 2209 = 0x8A1)
	uint32_t TriggerShape;				// [SRC: TriggerShape] Sempre 0 (box): con altri valori i personaggi non vengono mai rilevati
	uint32_t nActions;					// [OK] Numero di RMX_TRIGGER_ACTION in coda (0-30) [SRC: NumTargets]
	uint32_t BindNodeHash;				// [SRC: BindNodeHash] Nodo seguito con RMX_TRIGFLAG_BOUND (72 hash, spesso un RMX_OBJECT).
										//		Negli altri trigger 65 (96%) o 0: valore predefinito dell'editor, non e' un hash
	uint32_t ActuatorHash;				// [SRC: ActuatorHash] Oggetto che attiva il trigger con RMX_TRIGFLAG_ACTUATOR (44 trigger)
	uint32_t BindShapeHash;				// [SRC: BindShapeHash] Osso del nodo BindNodeHash seguito con RMX_TRIGFLAG_BONE (66 trigger)
	uint32_t nConditions;				// [LOG] Numero di RMX_MSG che seguono le azioni (0-3): messaggi ricevuti dal trigger -> gruppo di
										//		eventi (vedi RMX_MSG). Esempi: (37 NM_TIMEREVENT, evento 23) [SRC: nEventBindings]
	uint32_t SecondaryID;				// [SRC: SecondaryID] 0-14, diverso da 0 in 40 trigger
	uint32_t null4;						// [OK] Sempre 0 [SRC: pBind, runtime]
	// RMX_TRIGGER_ACTION Actions[nActions];
	// RMX_MSG Conditions[nConditions];
};


/*------------------------------------------------------------------------------------------------------------------
Punto del grafo di navigazione dell'IA. 4297 nodi.
ATTENZIONE: tutti i campi tranne posizione, Hash, Type e collegamenti contengono memoria non inizializzata [GARBAGE],
compresi i campi dell'header che negli altri nodi sono sempre 0.
Dimensione: 356 + 4 * (nLinks + 1) arrotondata a 16 [OK su tutti i 4297 nodi]
------------------------------------------------------------------------------------------------------------------*/
struct RMX_WAYPOINT		// SIZE: 356 bytes + coda (minimo 368 bytes su file)
{
	RMX_NODE_HEADER Node;				// Type = 5. Validi solo pos, Hash, Type (Flags = 0)
	char garbage[240];					// [GARBAGE]
	uint32_t nLinks;					// [OK] Numero di waypoint collegati (1-14)
	// uint32_t Links[nLinks];			// [OK] Hash degli RMX_WAYPOINT collegati (archi del grafo)
	// uint32_t Garbage;				// [GARBAGE] Una cella in piu' dopo l'ultimo collegamento
};


struct RMX_PS2_ROOM_OBJ		// SIZE: 144 bytes. Istanza di un oggetto statico della ZONE. 9804 nodi [SRC: tagMAP_SCENERY]
{
	uint32_t BegPrev;					// [OK] RMX_NODE_HEADER.BegPrev
	uint32_t BegNext;					// [OK] RMX_NODE_HEADER.BegNext
	char null1[24];						// [OK] Always 0
	float Xpos;							// [OK] X position of the center of the object (mondo)
	float Ypos;							// [OK] Y position
	float Zpos;							// [OK] Z position
	float Wpos;							// [OK] Always 1.0
	float Xrot;							// [OK] Rotazione in gradi (spesso -0.0 = 0x80000000)
	float Yrot;
	float Zrot;
	char null2[20];						// [OK] Always 0
	float Xscal;						// [OK] Scala (1.0 nel 89% dei casi)
	float Yscal;
	float Zscal;
	char null3[8];						// [OK] Always 0
	uint32_t Unknown3;					// [OK] Hash del nome del nodo (RMX_NODE_HEADER.Hash)
	uint32_t Unknown4;					// [OK] Tipo di nodo: sempre 10 (RMX_NODE_PS2_ROOM_OBJ)
	uint32_t Static_flag;				// [OK] Flags: 4 (9535 nodi) o 12 (269 nodi)
	float BB_Xmin;						// [OK] Bounding box X min (mondo)
	float BB_Ymin;						// [OK] Bounding box Y min
	float BB_Zmin;						// [OK] Bounding box Z min
	uint16_t Unknown6;					// [OK] Sempre 0 [SRC: Validate]
	uint16_t ZoneSlot;					// [OK] [SRC: MeshN] Indice nella lista PS2_OBJ_SLOTS della ZONE (ZONE_PS2_OBJ_SLOTS): l'oggetto della ZONE in quello
										//		slot viene istanziato con posizione/rotazione/scala di questo nodo. Verificato: sempre < nSlots
										//		della ZONE e mai uno slot vuoto. Lo stesso slot puo' essere istanziato piu' volte
	float BB_Xmax;						// [OK] Bounding box X max
	float BB_Ymax;						// [OK] Bounding box Y max
	float BB_Zmax;						// [OK] Bounding box Z max
	uint32_t Unknown7;					// [GARBAGE] Contiene spesso offset di altri nodi (residui) [SRC: pRef, puntatore runtime]
};


/*------------------------------------------------------------------------------------------------------------------
Volume d'acqua. 327 nodi. Dimensione: 432 + 16 * nMessages (0 o 1) [OK]
------------------------------------------------------------------------------------------------------------------*/
struct RMX_WATER			// SIZE: 432 bytes + coda [SRC: tagMAP_WATERNODE]
{
	uint32_t BegPrev;					// [OK] RMX_NODE_HEADER.BegPrev
	uint32_t BegNext;					// [OK] RMX_NODE_HEADER.BegNext
	char null1[24];						// [OK] Always 0
	float X1;							// [OK] Posizione del centro del volume
	float Y1;
	float Z1;
	float W1;							// [OK] Always 1.0
	char null2[32];						// [OK] Rotazione (0 tranne 2 casi con Zrot = 360.5) + 16 bytes a 0
	float X2;							// [OK] Semi-dimensioni del volume (scala del nodo)
	float Y2;
	float Z2;
	char null3[8];						// [OK] Always 0
	uint32_t Unknown1;					// [OK] Hash del nome del nodo (RMX_NODE_HEADER.Hash)
	uint32_t Unknown2;					// [OK] Tipo di nodo: sempre 7 (RMX_NODE_WATER)
	char null4[248];					// [OK] Flags (sempre 0) + 244 bytes a 0 (campi runtime di tagMAP_BASENODE + Hash a +352)
	float Xmin;							// [OK] [SRC: Box[6]] Coordinata box acqua. Punto base per waterpatch (la dimensione viene poi arrotondata per difetto dal lato opposto)
	float Xmax;							// [OK] Coordinata box acqua. La coordinata viene arrotondata per difetto in base alle dimensioni della Waterpatch.
	float Ymin;							// [OK] Coordinata box acqua. Punto base per waterpatch (la dimensione viene poi arrotondata per difetto dal lato opposto)
	float Ymax;							// [OK] Coordinata box acqua. La coordinata viene arrotondata per difetto in base alle dimensioni della Waterpatch.
	float Zmin;							// [OK] Coordinata box acqua. La coordinata viene arrotondata per difetto in base alle dimensioni della Waterpatch.
	float Zmax;							// [OK] Coordinata box acqua. Punto base per waterpatch sull'asse Z
	uint32_t ARGB_PC;					// [OK] Colore dell'acqua sul PC (0x00RRGGBB) [SRC: rgb]
	uint32_t ARGB_PS2;					// [OK] Colore dell'acqua sulla PS2 (0x00RRGGBB) [SRC: envrgb, colore dell'ambiente riflesso]
	uint16_t Unknown5;					// [OK] Sempre 0 nei file noti (a volte 0200h secondo vecchie note, ignorato dal gioco) [SRC: WaterType]
	uint16_t Water_roughness;			// [OK] Intensita' dell'agitazione dell'acqua (4, 16, 8, 25, ...) [SRC: Viscosity]
	float Waterpatch_size;				// [OK] Dimensione della patch dell'acqua (valori standard 1-0.9, consigliabile non scendere sotto 0.5)
										//		[SRC: BlockSize]
	char null5[4];						// [OK] Always 0 [SRC: pMem]
	uint32_t Unknown7;					// [SRC: uint16 WaterFlags + uint16 Validate] Flag dell'acqua 0-11 (piu' frequenti 7, 5, 3, 4)
	uint32_t Unknown8;					// [GARBAGE] Valore variabile (es. 36256, 0x00038740) [SRC: pRef, puntatore runtime]
	uint32_t Unknown9;					// [SRC: uint8 Shade, Highlight, Envlight, PerturbType] (es. 0x00010101, 0x0210FF01, 0x02010101)
	uint32_t null7;						// [OK] Sempre 0 [SRC: pPCData]
	uint32_t nMessages;					// [OK] Numero di messaggi in coda (0 o 1) [SRC: nEventBindings]. Ogni messaggio occupa 16 bytes:
										//		RMX_MSG + 8 bytes a 0
	char null6[12];						// [OK] Always 0 [SRC: sAmp, sFreq, sAng]
	// RMX_MSG Messages[nMessages] (16 bytes ciascuno)
};


struct RMX_AUDIO_LOCATOR	// SIZE: 368 bytes. 551 nodi [SRC: tagMAP_AUDIONODE]
{
	uint32_t BegPrev;					// [OK] RMX_NODE_HEADER.BegPrev
	uint32_t BegNext;					// [OK] RMX_NODE_HEADER.BegNext
	char null1[24];						// [OK] Always 0
	float Xpos;							// [OK] X position of the center of the audio locator
	float Ypos;							// [OK] Y position
	float Zpos;							// [OK] Z position
	float Wpos;							// [OK] Always 1.0
	char null2[32];						// [OK] Rotazione (sempre 0) + 16 bytes a 0
	float Unknown1;						// [OK] Scala: always 64.0
	float Unknown2;						// [OK] Always 64.0
	float Unknown3;						// [OK] Always 64.0
	char null4[8];						// [OK] Always 0
	uint32_t Unknown4;					// [OK] Hash del nome del nodo (RMX_NODE_HEADER.Hash)
	uint32_t Unknown5;					// [OK] Tipo di nodo: sempre 11 (RMX_NODE_AUDIO_LOCATOR)
	uint32_t Unknown6;					// [OK] Flags: 0 o 1 (circa meta' e meta')
	char null5[240];					// [OK] Always 0
	uint32_t AudioTrack;				// [OK] Il numero della traccia audio (es. 142-145) [SRC: AudioID]
	uint32_t Unknown8;					// [SRC: AudioFlags] Flag dell'audio: 0, 3 (piu' frequente), 2, 7
	char null6[8];						// [OK] Always 0
};


/*------------------------------------------------------------------------------------------------------------------
Emettitore di effetti (nebbia, particelle, ...). 3070 nodi. La nebbia e' un tipo di emettitore: la vecchia struttura
RMX_FOG descriveva lo stesso nodo. [SRC: tagMAP_FXNODE] L'effetto e' attivo con il bit 0 di Node.Flags (2149 nodi).
Dimensione: 416 + 8 * nMessages arrotondata a 16 [OK su tutti i 3070 nodi]
------------------------------------------------------------------------------------------------------------------*/
struct RMX_EMITTER		// SIZE: 416 bytes + coda
{
	RMX_NODE_HEADER Node;				// Type = 8, Flags = 0/1. Xscal/Yscal/Zscal = semi-dimensioni del volume (spesso 1.0 o 64.0)
	char null1[12];						// [OK] Sempre 0
	uint32_t LinkedHash;				// [SRC: hScript] Hash dello script AMX, presente in 85 emitter (sempre lo stesso: 0x9E058E4E)
	char null2[224];					// [OK] Sempre 0
	float BoxMin[3];					// [OK] [SRC: Min] Emitter ST_BOX / ST_POINT: box del volume nel mondo, pos - scal.
										//		ST_LINE: punto iniziale della linea (= pos, 986/986)
	uint8_t R;							// [SRC: rgb0] Colore dell'effetto (per la nebbia: colore della nebbia). Nella vecchia RMX_FOG: R, G, B, A
	uint8_t G;
	uint8_t B;
	uint8_t A;							// [?] Di solito 0
	float BoxMax[3];					// [OK] [SRC: Max] ST_BOX / ST_POINT: pos + scal. ST_LINE: punto finale della linea (lunghezza > 0 in 986/986)
	uint8_t Color2[4];					// [SRC: rgb1] Secondo colore dell'effetto (R, G, B, A); il significato dipende dal template.
										//		Nella vecchia RMX_FOG il primo byte era l'intensita' della nebbia
	uint32_t EffectHash;				// [OK] GetHashValue del nome del template dell'effetto in maiuscolo (3068/3070), dalla lista
										//		Tools\worldedit\Sys\effects.fsc (155 template: nome, forma ST_BOX / ST_LINE / ST_POINT,
										//		EF_CONST / EF_ONESHOT). Piu' usati: FOG FADE, LAVA TRAIL, RAINSPLASH NODETECT, LIGHT GLOW
										//		[SRC: TemplateHash]
	float Scale;						// [SRC: Scale] Scala dell'effetto: 1.0, 8.0, 0.6, 6.0, ...
	float Rate;							// [SRC: Rate] Frequenza di emissione: 1.0, 0.1, 0.005, 0.2, ...
	uint16_t FxNo;						// [OK] Sempre 0 [SRC: FxNo] Indice dell'effetto, calcolato al caricamento dal template
	uint16_t Validate;					// [OK] Sempre 0 [SRC: Validate]
	uint32_t Unknown3;					// [GARBAGE] Offset del nodo stesso dall'inizio della prima stanza (2380/3070) [SRC: pRef,
										//		puntatore del runtime], non e' un parametro dell'effetto
	uint32_t CameraFlags;				// [OK] Sempre 0 [SRC: CameraFlags]
	uint32_t nMessages;					// [OK] Numero di RMX_MSG in coda (0-3) [SRC: nEventBindings]
	uint32_t LinkedObject;				// [SRC: BindNodeHash] Nodo seguito dall'effetto (RMX_OBJECT, 9 casi: l'emitter si muove con
										//		l'oggetto, mapEmitterNodeUpdate); 0x0003518B = GetHashValue("pad") (nessuno, 62%) oppure 0
	// RMX_MSG Messages[nMessages];
};


/*------------------------------------------------------------------------------------------------------------------
Portale tra due stanze: rettangolo allineato agli assi. 4939 portali. Ogni portale compare in entrambe le stanze che
collega, con la normale in verso opposto.
------------------------------------------------------------------------------------------------------------------*/
struct RMX_PORTAL			// SIZE: 112 bytes
{
	uint32_t Unknown1;					// [OK] Sempre 3
	uint32_t Unknown2;					// [OK] Sempre 1
	uint32_t DestRoom;					// [OK] Indice della stanza di destinazione
	float null1;						// [GARBAGE] Spesso 1.0 ma anche puntatori di stack (0x0012F470) e 0xFFFFFFFF
	char null2[28];						// [GARBAGE] Puntatori di stack/eseguibile e float residui di calcoli
	uint32_t Unknown4;					// [OK] Direzione della normale del portale: 0/1 = +-X, 2/3 = +-Y, 4/5 = +-Z (orizzontale).
										//		I valori compaiono sempre in coppie di pari frequenza (le due facce dello stesso portale)
	float v0_X;							// [OK] Coordinata X del vertice 0 del rettangolo del portale
	float v0_Y;							// [OK] Coordinata Y del vertice 0 del rettangolo del portale
	float v0_Z;							// [OK] Coordinata Z del vertice 0 del rettangolo del portale
	uint32_t Unknown5;					// [GARBAGE] Quarto componente del vertice: non inizializzato
	float v1_X;							// [OK] Coordinata X del vertice 1 del rettangolo del portale
	float v1_Y;							// [OK] Coordinata Y del vertice 1 del rettangolo del portale
	float v1_Z;							// [OK] Coordinata Z del vertice 1 del rettangolo del portale
	uint32_t Unknown6;					// [GARBAGE] Quarto componente del vertice
	float v2_X;							// [OK] Coordinata X del vertice 2 del rettangolo del portale
	float v2_Y;							// [OK] Coordinata Y del vertice 2 del rettangolo del portale
	float v2_Z;							// [OK] Coordinata Z del vertice 2 del rettangolo del portale
	uint32_t Unknown7;					// [GARBAGE] Quarto componente del vertice
	float v3_X;							// [OK] Coordinata X del vertice 3 del rettangolo del portale
	float v3_Y;							// [OK] Coordinata Y del vertice 3 del rettangolo del portale
	float v3_Z;							// [OK] Coordinata Z del vertice 3 del rettangolo del portale
	uint32_t Unknown8;					// [GARBAGE] Quarto componente del vertice
};


// Verifica delle dimensioni delle strutture (parte fissa, senza code variabili)
static_assert(sizeof(RMX_HEADER) == 20, "RMX_HEADER");
static_assert(sizeof(RMX_NODE_HEADER) == 112, "RMX_NODE_HEADER");
static_assert(sizeof(RMX_ROOM) == 368, "RMX_ROOM");
static_assert(sizeof(RMX_OBJECT) == 528, "RMX_OBJECT");
static_assert(sizeof(RMX_LIGHT) == 144, "RMX_LIGHT");
static_assert(sizeof(RMX_CHARLOC) == 416, "RMX_CHARLOC");
static_assert(sizeof(RMX_TRIGGER) == 416, "RMX_TRIGGER");
static_assert(sizeof(RMX_TRIGGER_ACTION) == 12, "RMX_TRIGGER_ACTION");
static_assert(sizeof(RMX_MSG) == 8, "RMX_MSG");
static_assert(sizeof(RMX_WAYPOINT) == 356, "RMX_WAYPOINT");
static_assert(sizeof(RMX_PS2_ROOM_OBJ) == 144, "RMX_PS2_ROOM_OBJ");
static_assert(sizeof(RMX_WATER) == 432, "RMX_WATER");
static_assert(sizeof(RMX_AUDIO_LOCATOR) == 368, "RMX_AUDIO_LOCATOR");
static_assert(sizeof(RMX_EMITTER) == 416, "RMX_EMITTER");
static_assert(sizeof(RMX_PORTAL) == 112, "RMX_PORTAL");
