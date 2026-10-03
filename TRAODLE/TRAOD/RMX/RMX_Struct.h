#pragma once


/*==================================================================================================================
	FORMATO RMX (versione 3.6) - Tomb Raider: The Angel of Darkness

	Le informazioni sono state ricavate analizzando tutti i 57 file RMX disponibili (1701 stanze, ~38000 nodi).
	Legenda dei commenti:
		[OK]		verificato su tutti i file
		[PROB]		molto probabile, verificato su un sottoinsieme o dedotto da piu' indizi
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
struct RMX_NODE_HEADER					// SIZE: 112 bytes
{
	uint32_t BegPrev;					// [OK] Offset (dall'inizio della stanza) del nodo precedente della stessa lista, 0 se primo
	uint32_t BegNext;					// [OK] Offset (dall'inizio della stanza) del nodo successivo della stessa lista, 0 se ultimo
	char null1[24];						// [OK] Sempre 0
	float Xpos;							// [OK] Posizione (coordinate del mondo)
	float Ypos;
	float Zpos;
	float Wpos;							// [OK] Sempre 1.0
	float Xrot;							// [OK] Rotazione in gradi. Quasi sempre solo Zrot e' diversa da 0
	float Yrot;
	float Zrot;
	float Wrot;							// [OK] Sempre 0
	char null2[16];						// [OK] Sempre 0
	float Xscal;						// [OK] Scala. Per trigger, acqua ed emitter e' la semi-dimensione del volume (vedi strutture)
	float Yscal;
	float Zscal;
	uint32_t null3;						// [OK] Sempre 0
	uint32_t null4;						// [OK] Sempre 0
	uint32_t Hash;						// [OK] Hash del nome del nodo. Usato per i riferimenti tra nodi (trigger, waypoint, ...) e dagli
										//		script AMX (es. mapFindHashedNodeTypedAMX). Nodi con nomi simili hanno hash che differiscono
										//		solo nei bit bassi (es. LIGHT01, LIGHT02, ...)
	uint32_t Type;						// [OK] Tipo del nodo (RMX_NODE_TYPE), coincide con la lista che lo contiene
	uint32_t Flags;						// [PROB] Flag del nodo: luci sempre 4, PS2_ROOM_OBJ 4 o 12, CHARLOC sempre 1,
										//		trigger/emitter 0 o 1, audio 0 o 1, altri 0. Il bit 4 indica probabilmente un nodo statico
};


/*------------------------------------------------------------------------------------------------------------------
Messaggio/evento associato ad un nodo (code di oggetti, emitter e acqua). Gli ID dei messaggi sono gli stessi usati
nelle azioni dei trigger (RMX_TRIGGER_ACTION.MsgID). Valori di MsgID piu' frequenti: 57, 37, 46, 25, 9, 68, 3, 4, 0, 67.
------------------------------------------------------------------------------------------------------------------*/
struct RMX_MSG							// SIZE: 8 bytes
{
	uint32_t MsgID;						// [PROB] Identificativo del messaggio/evento
	uint32_t Param;						// [PROB] Parametro (di solito un intero piccolo, a volte un hash)
};


struct RMX_ROOM			// SIZE: 368 bytes. Ha la stessa struttura di RMX_NODE_HEADER nei primi 112 bytes (Type = 12)
{
	char null1[100];					// [OK] Always 0 (header di nodo non usato)
	uint32_t Room_ID;					// [OK] Hash del nome della stanza (RMX_NODE_HEADER.Hash)
	uint32_t Unknown26;					// [OK] Tipo di nodo: sempre 12 (RMX_NODE_ROOM)
	uint32_t Unknown27;					// [OK] Flags: sempre 0
	float Room_Xt;						// [OK] Room X position
	float Room_Yt;						// [OK] Room Y position
	float Room_Zt;						// [OK] Room Z position
	float pad1;							// [OK] Sempre 0
	float BB_Xmin;						// [OK] Bounding box X min (coordinate del mondo)
	float BB_Ymin;						// [OK] Bounding box Y min
	float BB_Zmin;						// [OK] Bounding box Z min
	float pad2;							// [OK] Sempre 0
	float BB_Xmax;						// [OK] Bounding box X max
	float BB_Ymax;						// [OK] Bounding box Y max
	float BB_Zmax;						// [OK] Bounding box Z max
	float pad3;							// [OK] Sempre 0
	uint32_t null5;						// [OK] Always 0x00000000
	uint32_t null6;						// [OK] Always 0x00000000
	uint32_t Room_ID2;					// [OK] Copia di Room_ID
	uint32_t null7;						// [OK] Always 0x00000000
	uint32_t null8;						// [OK] Always 0x00000000
	uint32_t null9;						// [OK] Always 0x00000000

	// Liste di nodi: offset (dall'inizio della stanza) del primo e dell'ultimo nodo, 0 se la lista e' vuota [OK]
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

	uint32_t P15_Portal_First;			// [OK] Offset del primo RMX_PORTAL (0 se nessuno)
	uint32_t nPortals;					// [OK] Numero di portali (0-15)
	uint32_t Offset28;					// [OK] ZoneList: offset della lista delle zone a cui appartiene la stanza (uint32_t x Offset29)
										//		I valori sono indici delle zone (file .Z00, .Z01, ...) e sono sempre < numero di zone
	uint32_t Offset29;					// [OK] Numero di elementi della ZoneList (0-3)
	uint32_t Offset30;					// [?] Flag 0/1 (1 in 95 stanze). Non e' un offset
	uint16_t ReverbID;					// [OK] Preset di riverbero della stanza (0-16)
	uint8_t Collision_set;				// [OK] Set di collisioni (0-21, diverso da 0 solo in poche stanze)
	uint8_t Collision_set_current;		// [OK] Sempre 0 nel file: il gioco copia "Collision_set" qui all'avvio del livello
	uint32_t Offset32;					// [OK] RoomMask_Begin: offset di una maschera di bit con un bit per stanza (ceil(nRooms/32) uint32_t)
										//		Il bit della stanza stessa e' sempre a 1, i bit oltre nRooms sempre a 0. La relazione e'
										//		simmetrica nel 96% delle coppie: probabilmente stanze potenzialmente visibili (PVS) o da
										//		mantenere caricate. Segue padding fino a 16 bytes [GARBAGE]
	uint32_t Offset33;					// [OK] RoomMask_End: Offset32 + ceil(nRooms/32) * 4
	uint32_t Unknown1;					// [?] Quasi sempre 0xFFFFFFFF. In un livello contiene valori progressivi (74-80) e 0 nelle 17 stanze con Unknown5 = 1
	uint32_t Unknown2;					// [?] Quasi sempre 0xFFFFFFFF. In un livello contiene valori progressivi (81-87)
	uint32_t Unknown3;					// [?] 0xFFFFFFFF (0 nelle 17 stanze con Unknown5 = 1)
	uint32_t Unknown4;					// [?] 0xFFFFFFFF (0 nelle 17 stanze con Unknown5 = 1)
	uint32_t Unknown5;					// [?] Flag 0/1 (1 in 17 stanze, che sono anche le uniche senza maschera delle stanze)
	uint32_t Unknown6;					// [?] 0-2 (diverso da 0 in 10 stanze)
	uint32_t Unknown7;					// [OK] Sempre 0
	uint32_t Unknown8;					// [OK] Sempre 0
	uint32_t Unknown9;					// [OK] Sempre 0
	uint32_t Unknown10;					// [OK] Sempre 0
};


/*------------------------------------------------------------------------------------------------------------------
Oggetto interattivo (porte, leve, ascensori, casse, ...). 2963 nodi.
Dimensione: 528 + 8 * nMessages arrotondata a 16 [OK su 2958/2963]
------------------------------------------------------------------------------------------------------------------*/
struct RMX_OBJECT		// SIZE: 528 bytes + coda
{
	RMX_NODE_HEADER Node;				// Type = 0, Flags = 0
	char null1[12];						// [OK] Sempre 0
	uint32_t LinkedHash;				// [?] Hash esterno (non e' l'hash di un nodo RMX), presente in 112 oggetti
	char null2[232];					// [OK] Sempre 0
	uint32_t ObjFlags;					// [PROB] Campo di bit (valori osservati: 0x40, 0x10, 0x2, 0x200, 0x1000, 0x20 << 16, 0x402 << 16, ...)
										//		Nella vecchia versione della struttura era chiamato AnimationID
	uint32_t null3;						// [OK] Sempre 0
	uint16_t Unknown1;					// [?] 0-1023 (byte basso 0-255, byte alto 0-4). Era ObjectID
	uint16_t ObjectIndex;				// [?] Numero progressivo piccolo (1-129), ripetuto in piu' oggetti dello stesso livello.
										//		Non e' l'indice dello slot della ZONE
	uint32_t nMessages;					// [OK] Numero di RMX_MSG in coda al nodo (0-10)
	char null4[24];						// [OK] Sempre 0
	uint16_t null5;						// [OK] Sempre 0
	uint16_t Unknown2;					// [?] 0-606 (in 725 oggetti 0, valore piu' frequente 160)
	uint32_t null6;						// [OK] Sempre 0
	uint32_t null7;						// [OK] Sempre 0
	uint32_t Unknown3;					// [?] Intero 1-37 (1 nel 51% dei casi)

	// Illuminazione dell'oggetto: 3 luci direzionali + luce ambientale (stile PS2) [PROB]
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
struct RMX_CHARLOC		// SIZE: 416 bytes
{
	RMX_NODE_HEADER Node;				// Type = 2, Flags = 1. Usa Zrot per l'orientamento, scala sempre 1 (tranne 1 caso)
	char null1[12];						// [OK] Sempre 0
	uint32_t CharHash;					// [OK] Se diverso da 0 (257 nodi su 433) e' spesso GetHashValue() del nome "di famiglia" del personaggio:
										//		es. hash("GUARD") per GUARD/GUARDB/GUARDC, hash("LARA") per LARAC1/LARAD. 132 valori non corrispondono
										//		a nessun nome di ACTOR.DB (e non sono hash degli script SCX)
	char null2[240];					// [OK] Sempre 0
	uint32_t ActorID;					// [OK] ID del personaggio nel database ACTOR.DB (campo ID): corrisponde in tutti i 433 nodi.
										//		Identifica la variante esatta (es. LARAD, GUARDB, STRNMSKA). Vedi RMX_GetActor()
	uint32_t Flags1;					// [?] Potenza di 2: 256, 128, 64, 32
	uint32_t Flags2;					// [?] 768, 64, 512, 76
	uint32_t null3;						// [OK] Sempre 0
	uint32_t null4;						// [OK] Sempre 0
	uint32_t Unknown2;					// [?] Valori misti: 1, 65, 5, 0x00400057, ...
	float Unknown3;						// [?] 50.0 nel 94% dei casi, altrimenti 64, 32, 1
	uint32_t null5[3];					// [OK] Sempre 0
	uint32_t LinkedObject;				// [OK] Hash di un RMX_OBJECT (37 casi), altrimenti 0
	uint32_t null6;						// [OK] Sempre 0
};


/*------------------------------------------------------------------------------------------------------------------
Volume trigger. Quando viene attivato invia una serie di messaggi a nodi bersaglio. 2738 nodi.
Il volume e' un parallelepipedo centrato in Node.pos, ruotato di Node.rot, con semi-dimensioni Node.scal.
Dimensione: 416 + 12 * nActions + 8 * nConditions arrotondata a 16 [OK su 2737/2738]
------------------------------------------------------------------------------------------------------------------*/
struct RMX_TRIGGER_ACTION				// SIZE: 12 bytes
{
	uint32_t MsgID;						// [OK] Messaggio da inviare (stessi ID di RMX_MSG). Piu' frequenti: 5, 67, 134, 8, 44, 14, 56, 82, 70
	uint32_t TargetHash;				// [OK] Hash del nodo bersaglio. Puo' essere un nodo di qualsiasi tipo (oggetti, personaggi, trigger,
										//		emitter, acqua, audio, stanze) oppure un hash esterno (es. 0x348BD399, usato con MsgID 8)
	uint32_t Param;						// [OK] Parametro: di solito 0, altrimenti un valore (1, 120, 180, 300, ...) o l'hash di un waypoint
};


struct RMX_TRIGGER		// SIZE: 416 bytes + coda
{
	RMX_NODE_HEADER Node;				// Type = 3, Flags = 0/1. Xscal/Yscal/Zscal = semi-dimensioni del volume
	char null1[12];						// [OK] Sempre 0
	uint32_t LinkedHash;				// [?] Hash esterno, presente in 4 trigger
	char null2[224];					// [OK] Sempre 0
	float BoxMin[3];					// [OK] Box locale: -Xscal, -Yscal, -Zscal
	uint32_t Unknown1;					// [?] Quasi sempre 0
	float BoxMax[3];					// [OK] Box locale: +Xscal, +Yscal, +Zscal
	uint32_t TriggerFlags;				// [PROB] Campo di bit (1, 33, 5, 2209, ...): probabilmente chi/cosa attiva il trigger
	uint32_t null3;						// [OK] Sempre 0
	uint32_t nActions;					// [OK] Numero di RMX_TRIGGER_ACTION in coda (0-30)
	uint32_t Unknown2;					// [?] 65 nel 96% dei casi, altrimenti hash (a volte di un RMX_OBJECT)
	uint32_t LinkedObject1;				// [?] 0 oppure hash (a volte di un RMX_OBJECT)
	uint32_t LinkedObject2;				// [?] 0 oppure hash
	uint32_t nConditions;				// [OK] Numero di RMX_MSG che seguono le azioni (0-3). Esempi: (37, 23), (136, 12)
	uint32_t Unknown3;					// [?] 0-14, diverso da 0 in 40 trigger
	uint32_t null4;						// [OK] Sempre 0
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


struct RMX_PS2_ROOM_OBJ		// SIZE: 144 bytes. Istanza di un oggetto statico della ZONE. 9804 nodi
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
	uint16_t Unknown6;					// [OK] Sempre 0
	uint16_t ZoneSlot;					// [OK] Indice nella lista PS2_OBJ_SLOTS della ZONE (ZONE_PS2_OBJ_SLOTS): l'oggetto della ZONE in quello
										//		slot viene istanziato con posizione/rotazione/scala di questo nodo. Verificato: sempre < nSlots
										//		della ZONE e mai uno slot vuoto. Lo stesso slot puo' essere istanziato piu' volte
	float BB_Xmax;						// [OK] Bounding box X max
	float BB_Ymax;						// [OK] Bounding box Y max
	float BB_Zmax;						// [OK] Bounding box Z max
	uint32_t Unknown7;					// [GARBAGE] Contiene spesso offset di altri nodi (residui)
};


/*------------------------------------------------------------------------------------------------------------------
Volume d'acqua. 327 nodi. Dimensione: 432 + 16 * nMessages (0 o 1) [OK]
------------------------------------------------------------------------------------------------------------------*/
struct RMX_WATER			// SIZE: 432 bytes + coda
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
	char null4[248];					// [OK] Flags (sempre 0) + 244 bytes a 0
	float Xmin;							// [OK] Coordinata box acqua. Punto base per waterpatch (la dimensione viene poi arrotondata per difetto dal lato opposto)
	float Xmax;							// [OK] Coordinata box acqua. La coordinata viene arrotondata per difetto in base alle dimensioni della Waterpatch.
	float Ymin;							// [OK] Coordinata box acqua. Punto base per waterpatch (la dimensione viene poi arrotondata per difetto dal lato opposto)
	float Ymax;							// [OK] Coordinata box acqua. La coordinata viene arrotondata per difetto in base alle dimensioni della Waterpatch.
	float Zmin;							// [OK] Coordinata box acqua. La coordinata viene arrotondata per difetto in base alle dimensioni della Waterpatch.
	float Zmax;							// [OK] Coordinata box acqua. Punto base per waterpatch sull'asse Z
	uint32_t ARGB_PC;					// [OK] Colore dell'acqua sul PC (0x00RRGGBB)
	uint32_t ARGB_PS2;					// [OK] Colore dell'acqua sulla PS2 (0x00RRGGBB)
	uint16_t Unknown5;					// [OK] Sempre 0 nei file noti (a volte 0200h secondo vecchie note, ignorato dal gioco)
	uint16_t Water_roughness;			// [OK] Intensita' dell'agitazione dell'acqua (4, 16, 8, 25, ...)
	float Waterpatch_size;				// [OK] Dimensione della patch dell'acqua (valori standard 1-0.9, consigliabile non scendere sotto 0.5)
	char null5[4];						// [OK] Always 0
	uint32_t Unknown7;					// [?] Intero 0-11 (piu' frequenti 7, 5, 3, 4)
	uint32_t Unknown8;					// [?] Valore variabile (es. 36256, 0x00038740): forse un offset o residuo
	uint32_t Unknown9;					// [?] 4 bytes di flag (es. 0x00010101, 0x0210FF01, 0x02010101)
	uint32_t null7;						// [OK] Sempre 0
	uint32_t nMessages;					// [OK] Numero di messaggi in coda (0 o 1). Ogni messaggio occupa 16 bytes: RMX_MSG + 8 bytes a 0
	char null6[12];						// [OK] Always 0
	// RMX_MSG Messages[nMessages] (16 bytes ciascuno)
};


struct RMX_AUDIO_LOCATOR	// SIZE: 368 bytes. 551 nodi
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
	uint32_t AudioTrack;				// [OK] Il numero della traccia audio (es. 142-145)
	uint32_t Unknown8;					// [?] 0, 3 (piu' frequente), 2, 7
	char null6[8];						// [OK] Always 0
};


/*------------------------------------------------------------------------------------------------------------------
Emettitore di effetti (nebbia, particelle, ...). 3070 nodi. La nebbia e' un tipo di emettitore: la vecchia struttura
RMX_FOG descriveva lo stesso nodo.
Dimensione: 416 + 8 * nMessages arrotondata a 16 [OK su tutti i 3070 nodi]
------------------------------------------------------------------------------------------------------------------*/
struct RMX_EMITTER		// SIZE: 416 bytes + coda
{
	RMX_NODE_HEADER Node;				// Type = 8, Flags = 0/1. Xscal/Yscal/Zscal = semi-dimensioni del volume (spesso 1.0 o 64.0)
	char null1[12];						// [OK] Sempre 0
	uint32_t LinkedHash;				// [?] Hash esterno, presente in 85 emitter (sempre lo stesso: 0x9E058E4E)
	char null2[224];					// [OK] Sempre 0
	float BoxMin[3];					// [OK] Box del volume nel mondo: pos - scal
	uint8_t R;							// [PROB] Colore dell'effetto (per la nebbia: colore della nebbia). Nella vecchia RMX_FOG: R, G, B, A
	uint8_t G;
	uint8_t B;
	uint8_t A;							// [?] Di solito 0
	float BoxMax[3];					// [OK] Box del volume nel mondo: pos + scal
	uint8_t Color2[4];					// [?] Secondo colore o parametri a byte. Nella vecchia RMX_FOG il primo byte era l'intensita' della nebbia
	uint32_t EffectHash;				// [PROB] Hash del tipo di effetto (pochi valori ricorrenti: 0xAE7F819F, 0xD11C4BD1, 0x78180820, ...)
	float Unknown1;						// [?] 1.0, 8.0, 0.6, 6.0, ...
	float Unknown2;						// [?] 1.0, 0.1, 0.005, 0.2, ...
	uint32_t null3;						// [OK] Sempre 0
	uint32_t Unknown3;					// [?] Valore variabile (es. 13520, 32720)
	uint32_t null4;						// [OK] Sempre 0
	uint32_t nMessages;					// [OK] Numero di RMX_MSG in coda (0-3)
	uint32_t Unknown4;					// [?] 0x0003518B nel 62% dei casi, 0, oppure hash di un RMX_OBJECT (9 casi)
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
