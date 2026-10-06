#pragma once


/*==================================================================================================================
	FORMATO CAL - Tomb Raider: The Angel of Darkness (animazioni scheletriche di personaggi e oggetti)

	Fonti:
		[SRC]	sorgenti originali della libreria di animazione del gioco e dell'editor delle animazioni
				(TRAOD\CHR\Reference_TRAODAE\Tools\Libs\AnimLib\AnimAPI.h / AnimAPI.c e Tools\AnimEdit\EditorToGame.cpp).
				Tra parentesi quadre il nome originale della struttura o del campo, es. [ANIMATION.altanimID]
		[OK]	verificato su tutti i 955 file CAL dei livelli (31658 animazioni, 6.5 milioni di canali)
		[MAYA]	verificato confrontando LARA.CAL e KURTIS.CAL con le scene Maya originali
				(TRAOD\CHR\Reference_TRAODAE\ANIMS e \Kurtis): errore 0 su traslazioni e rotazioni
		[?]		ipotesi / significato sconosciuto
		[GARBAGE] memoria non inizializzata scritta dal builder (non va interpretata)
	Il file e' l'immagine in memoria di un ANIMATION_LIST in cui i puntatori sono offset dall'inizio del file.
	Gli hash sono STRING_GetHashValue() del nome, identica a GetHashValue() di TRAODLE.

	STRUTTURA DEL FILE
		CAL_HEADER (24 bytes)										[ANIMATION_LIST]
		uint32_t AnimPtr[nAnims]									Offset degli header delle animazioni (da 0x18) [OK]
		CAL_LINKPATH x nLinkPaths									Tabella dei percorsi, subito dopo la lista [OK]
		uint32_t LinkHash[...]										Animazioni intermedie dei percorsi [OK]
		CAL_ANIMATION x nAnims (132 bytes)							Di norma contigui dopo i percorsi, ma vanno sempre letti
																	tramite la lista (in 3707 casi sono sparsi tra i dati)
		Dati di ogni animazione, allineati a 16 bytes e separati da padding [GARBAGE]:
			CAL_ANIMATION_DATA, CAL_TRACK x nTracks, liste di blocchi e blocchi,
			CAL_BLENDRANGE x nBlendRanges + CAL_BLENDCLIP, CAL_LINK x nLinkRanges, CAL_TRACK_KEY x nKeys,
			ParentTracks / ParentIdx, CAL_FXNODE x nFXNodes
		padding fino a multiplo di 2048 bytes [OK] (nel 98% dei file contiene dati di altri file [GARBAGE])

	PUNTATORI CON CONTATORE A ZERO [OK]
		Quando un contatore vale 0 il puntatore corrispondente non va usato: contiene il valore di un'altra animazione
		o valori ricorrenti anche oltre la fine del file (0x1D73E8, 0x2CC22C, 0xD54, ...) [GARBAGE].
		Vale anche per CAL_HEADER.LINKPATHS_PTR quando nLinkPaths == 0.

	DATI DI UN'ANIMAZIONE
		CAL_ANIMATION.DATA_PTR -> CAL_ANIMATION_DATA								[ANIMATION_DATA]
			.TRACKS_PTR  -> CAL_TRACK x nTracks									Sempre subito dopo [OK]
			.vtrack      -> traccia della velocita' (root motion), stessa forma di CAL_TRACK
		CAL_TRACK.BLOCKS_PTR -> uint32_t BlockPtr[popcount(Flags & 0x3FF)]		[ANIMATION_TRACK.animblock]
			I puntatori sono nell'ordine dei bit di CAL_TRACK_FLAGS, dal bit 0 al bit 9 [OK][SRC: AT_GetFrameData]
		BlockPtr -> sequenza di blocchi (CAL_ANIMBLOCK_HEADER + valori int16)

	CORRISPONDENZA TRACCE - BONES DEL CHR [OK][SRC]
		Le tracce seguono l'ordine delle bones del CHR SALTANDO le bones con BONEFLAG_NOTRACK (0x08, le bones
		"*_DYNAMIC" simulate dalla fisica: code di cavallo, ...). In 3597 animazioni di personaggi con bones dinamiche
		il numero di tracce e' esattamente quello delle bones senza NOTRACK. [SRC: AO_AnimateHierachy, trackidx]
		Le ultime nParentTracks tracce non sono bones ma oggetti agganciati (armi, fondine, occhi, accessori):
		ParentTracks contiene l'hash del nome della mesh animata, ParentIdx l'indice della sua traccia [SRC]. Nomi
		trovati: LARA_EYE_LEFT, LARA_EYE_RIGHT, GUNHAND_LEFT, GUNHAND_RIGHT, GUNHOLSTER_LEFT, GUNHOLSTER_RIGHT, HOLSTER,
		POUCH, RADIO, SHOTGUN_PUMP, PISTOL_L, PISTOL_R, CANISTERA, TONFA, SHIELD, VISOR2, ... La loro visibilita' e'
		gestita dalle chiavi TRACK_KEY_CODE_MAKE_VISIBLE / MAKE_INVISIBLE con lo stesso hash.

	DECODIFICA DI UN BLOCCO [OK][SRC: APB_GetTrackValue]
		La traccia copre i frames da 0 a nFrames-1 ed e' divisa in blocchi. Ogni blocco copre i frames dall'ultimo frame
		del blocco precedente (0 per il primo) fino al proprio LastFrame compreso: il frame di confine e' condiviso.
		L'ultimo blocco ha LastFrame == nFrames-1 ed il bit ANIM_LASTBLOCK. Size comprende l'header: il blocco
		successivo inizia a BlockPtr + Size.
			ANIM_CONST  (2):	Size 6. Un valore per tutti i frames del blocco.
			ANIM_KEYS   (4):	Size 4 + 2*n. Un valore per ogni frame.
			ANIM_BEZIER (1):	Curva di Bezier cubica A,B,C,D valutata con t = (frame - inizio) / (LastFrame - inizio)
								(algoritmo di de Casteljau, APB_BezierIdx).
								Size 12: A,B,C,D presenti. Size 10: solo B,C,D, A e' l'int16 che precede l'header, cioe'
								l'ultimo valore del blocco precedente (APB_BezierIdx(d, t, -3, 0, 1, 2)).

	VALORI (valore reale = int16 / scala) [SRC][OK][MAYA]
		Traslazioni (bit 0-2):		/ ANIM_TSCALE  (4)		DELTA rispetto alla traslazione di bind del CHR: il runtime
															somma bind.m30..m32 (mathVectorInterpAdd). Tracce assenti: 0
		Rotazioni (bit 3-5, 9):		/ ANIM_RSCALE  (16383)	Quaternione ASSOLUTO della rotazione locale (il runtime lo
															converte in matrice e non usa la rotazione di bind). [MAYA] E'
															la rotazione locale Maya RotateAxis * Rotate * JointOrient
		Scalature (bit 6-8):		/ ANIM_SSCALE  (255)	Valore assoluto (default 1.0)
		Root motion traslazioni:	/ ANIM_VTSCALE (32)		Incrementi per frame, unita' del mondo
		Root motion rotazione Z:	/ ANIM_VRSCALE (512)	Incrementi per frame in GRADI (il runtime usa DTOR)
		Le scene Maya sono in unita' 1/100 di quelle del gioco (HIP: 4.53 in Maya, 453 nel CHR).

	QUATERNIONE [SRC: AT_GetFrameData]
		Valori di default prima della lettura: X = Y = Z = 0, W = 1. Se il bit ANIMTRACK_HASRW manca, W resta 1
		(le 23655 tracce senza W hanno sempre |XYZ|^2 < 0.1, quindi rotazioni piccole). Il quaternione (X, Y, Z, W)
		viene interpolato con slerp e convertito in matrice.
		Se mancano tutti i bit di rotazione la rotazione e' l'identita' (le bones di queste 9081 tracce hanno rotazione
		di bind identita' in 9073 casi). Il runtime usa la matrice di bind solo per le bones senza animazione e per
		le bones dinamiche (BONEFLAG_DYNAMIC).

	ROOT MOTION (traccia della velocita') [SRC: APB_GetVelocityOffset][OK]
		I valori sono incrementi per frame. Posizione e rotazione al frame f:
			r = t = 0;  per i = 0 .. f-1:  r.z += rz[i];  t += RotZ(r.z) * (tx[i], ty[i], tz[i])
		(la traslazione del frame i viene ruotata con la rotazione gia' aggiornata con l'incremento dello stesso frame).
		Il personaggio avanza verso -Y. [MAYA] Il root motion e' il movimento dell'HIP estratto sugli assi scelti
		nell'esportatore e tolto dalla traccia dell'HIP: HIP in Maya = HIP del CAL + root motion.
		Nei CAL dei filmati (CS_*) il root motion dei personaggi e' invece contenuto nei file POS.

	ANIMAZIONI SPECCHIATE (APBFLAG_MIRROR) [SRC][OK]
		3956 animazioni *_MIRROR su 3993 hanno lo stesso DATA_PTR dell'animazione originale: lo specchiamento e' fatto
		a runtime. Per ottenere l'animazione specchiata:
			- la traccia della bone i e' quella della bone speculare (nomi che finiscono con _L / _R, BONE_CreateMirrorMap)
			- traslazione X negata, quaternione con X e W negati
			- root motion: incremento X negato e rotazione Z negata

	TABELLA DEI PERCORSI (CAL_LINKPATH) [SRC][OK]
		Per ogni coppia (partenza, arrivo) elenca le animazioni intermedie da eseguire (escluse partenza e arrivo).
		Esempio (COP_A): COPC_RIOT_LOOKAROUND -> COPA_DRAWHANDGUN: COPC_RIOT_STAND, COPC_RIOT_PUTBATONAWAY, COPA_STANCE
		Ordinata per (sourceID, destID) per la ricerca binaria (AL_SortLinkPathList). Assente in 842 file su 955.

	TEMPI
		Il runtime chiama "ms" i tempi delle animazioni, ma ANIM_ONESECOND = ANIM_FPS = 30, quindi 1 "ms" = 1 frame:
		i tempi dei BLENDCLIP e delle CAL_TRACK_KEY sono in frames.
==================================================================================================================*/


#define CAL_GAME_ANIMATION_VERSION	17		// [SRC: GAME_ANIMATION_VERSION]

#define CAL_TSCALE		4.0f				// [SRC: ANIM_TSCALE]
#define CAL_RSCALE		16383.0f			// [SRC: ANIM_RSCALE]
#define CAL_SSCALE		255.0f				// [SRC: ANIM_SSCALE]
#define CAL_VTSCALE		32.0f				// [SRC: ANIM_VTSCALE]
#define CAL_VRSCALE		512.0f				// [SRC: ANIM_VRSCALE]


struct CAL_HEADER				// SIZE: 24 bytes [ANIMATION_LIST]
{
	uint32_t nAnims;					// [OK] Numero di animazioni [nEntries]
	uint32_t Version;					// [OK] Sempre 17 = GAME_ANIMATION_VERSION (in memoria e' nAllocEntries)
	uint32_t ANIM_LIST_PTR;				// [OK] Sempre 0x18: offset della lista di puntatori alle animazioni [animation]
	uint32_t nLinkPaths;				// [OK] Numero di CAL_LINKPATH [linktable.nEntries]
	uint32_t LINKPATHS_PTR;				// [OK] Offset della tabella CAL_LINKPATH, 0x18 + 4 * nAnims [linktable.animlink]
										//      ([GARBAGE] se nLinkPaths = 0)
	uint32_t uData;						// [SRC] ID dell'oggetto audio del personaggio impostato nell'editor delle animazioni
										//       (uguale per tutti i CAL dello stesso personaggio, 0 nei DLG)
};
static_assert(sizeof(CAL_HEADER) == 24, "CAL_HEADER deve essere 24 bytes");


struct CAL_LINKPATH				// SIZE: 16 bytes [ANIMATION_LINKPATH]
{
	uint32_t sourceID;					// [OK] Hash dell'animazione di partenza
	uint32_t destID;					// [OK] Hash dell'animazione di arrivo
	uint32_t nLinks;					// [OK] Numero di animazioni intermedie
	uint32_t LINKS_PTR;					// [OK] Offset di uint32_t LinkHash[nLinks] (hash di animazioni dello stesso file)
};
static_assert(sizeof(CAL_LINKPATH) == 16, "CAL_LINKPATH deve essere 16 bytes");


struct CAL_ANIMATION			// SIZE: 132 bytes [ANIMATION]
{
	char name[64];						// [OK] Nome dell'animazione ("Debug Only"; in 197 casi garbage dopo il NUL)
	uint32_t DATA_PTR;					// [OK] Offset di CAL_ANIMATION_DATA [data]
	uint32_t BLENDRANGES_PTR;			// [OK] Offset di CAL_BLENDRANGE x nBlendRanges, seguiti dai CAL_BLENDCLIP [BlendRanges]
	uint32_t LINKS_PTR;					// [OK] Offset di CAL_LINK x nLinkRanges [Links]
	uint32_t KEYS_PTR;					// [OK] Offset di CAL_TRACK_KEY x nKeys [keys]
	uint32_t PARENTTRACKS_PTR;			// [OK] Offset di uint32_t MeshID[nParentTracks]: hash delle mesh agganciate [ParentTracks]
	uint32_t PARENTIDX_PTR;				// [OK] Offset di uint32_t TrackIdx[nParentTracks]: indice della traccia di ogni mesh,
										//      sempre nTracks-nParentTracks .. nTracks-1 [ParentIdx]
	uint32_t FXNODES_PTR;				// [OK] Offset di CAL_FXNODE x nFXNodes [FXNodes]
	uint32_t flags;						// [OK] CAL_ANIM_FLAGS [flags]
	uint32_t animID;					// [OK] Hash del nome [animID]; in 4003 animazioni (copie rinominate, nomi che finiscono
										//      con cifre) e' l'hash del nome originale
	uint32_t altanimID;					// [OK] Animazione alternativa ("ie Mirror") [altanimID]: nel 94% dei casi la versione
										//      _MIRROR / non _MIRROR, negli altri la variante opposta (_LEFT/_RIGHT, ...)
	uint32_t nFrames;					// [OK] Numero di frames [nFrames]
	uint32_t nTracks;					// [OK] Numero di tracce: bones senza NOTRACK + nParentTracks [nTracks]
	uint32_t nLinkRanges;				// [OK] Numero di CAL_LINK [nLinkRanges]
	uint32_t nBlendRanges;				// [OK] Numero di CAL_BLENDRANGE [nBlendRanges]
	uint32_t nKeys;						// [OK] Numero di CAL_TRACK_KEY [nKeys]
	uint32_t nParentTracks;				// [OK] Numero di tracce di mesh agganciate (sempre le ultime) [nParentTracks]
	uint32_t nFXNodes;					// [OK] Numero di CAL_FXNODE [nFXNodes]
};
static_assert(sizeof(CAL_ANIMATION) == 132, "CAL_ANIMATION deve essere 132 bytes");


enum CAL_ANIM_FLAGS : uint32_t	// [SRC: APBFLAG_*] Unici bit presenti nei file
{
	CAL_APBFLAG_MIRROR = 0x2,			// [OK] Animazione specchiata a runtime (dati condivisi con l'originale)
	CAL_APBFLAG_LOOP = 0x20,			// [OK][MAYA] Animazione ciclica (opzione "loop" dell'esportatore Maya)
	CAL_APBFLAG_LINK = 0x200,			// [OK][MAYA] Animazione di raccordo (opzione "link": nomi *_TO_*, *_START)
	CAL_APBFLAG_TERMINATOR = 0x400000	// [SRC] Animazione terminale (soprattutto colpi subiti, morti, cadute, atterraggi)
};


struct CAL_ANIMATION_DATA		// SIZE: 12 bytes [ANIMATION_DATA]
{
	uint32_t TRACKS_PTR;				// [OK] Offset di CAL_TRACK x nTracks (sempre DATA_PTR + 12) [tracks]
	uint32_t vtrackFlags;				// [OK] Canali del root motion (CAL_VTRACK_FLAGS), 0 = nessuno [vtrack.flags]
	uint32_t VTRACK_PTR;				// [OK] Offset di uint32_t BlockPtr[popcount(vtrackFlags)] [vtrack.animblock]
};
static_assert(sizeof(CAL_ANIMATION_DATA) == 12, "CAL_ANIMATION_DATA deve essere 12 bytes");


struct CAL_TRACK				// SIZE: 8 bytes [ANIMATION_TRACK]
{
	uint32_t flags;						// [OK] Canali presenti (CAL_TRACK_FLAGS)
	uint32_t BLOCKS_PTR;				// [OK] Offset di uint32_t BlockPtr[popcount(flags & 0x3FF)] [animblock]
};
static_assert(sizeof(CAL_TRACK) == 8, "CAL_TRACK deve essere 8 bytes");


enum CAL_TRACK_FLAGS : uint32_t	// [SRC: ANIMTRACK_HAS*] Ordine dei blocchi = ordine dei bit
{
	CAL_HASTX = 0x001,					// Traslazione X (delta rispetto al bind, / 4)
	CAL_HASTY = 0x002,					// Traslazione Y
	CAL_HASTZ = 0x004,					// Traslazione Z
	CAL_HASRX = 0x008,					// Quaternione X (/ 16383)
	CAL_HASRY = 0x010,					// Quaternione Y
	CAL_HASRZ = 0x020,					// Quaternione Z
	CAL_HASSX = 0x040,					// Scala X (/ 255)
	CAL_HASSY = 0x080,					// Scala Y
	CAL_HASSZ = 0x100,					// Scala Z
	CAL_HASRW = 0x200,					// Quaternione W (se assente W = 1)
	CAL_HASSSC = 0x400,					// Segment Scale Compensate: la scala del padre non si applica (nessun blocco). Presente
										// sulle bones dei personaggi, mai sulle mesh agganciate e sugli oggetti NODE [OK]
	CAL_CHANNEL_MASK = 0x3FF
};


enum CAL_VTRACK_FLAGS : uint32_t	// Canali della traccia della velocita' (root motion), nell'ordine dei bit
{
	CAL_VTRACK_TX = 0x01,				// [OK] Traslazione X (incrementi, / 32)
	CAL_VTRACK_TY = 0x02,				// [OK] Traslazione Y (incrementi, / 32; avanti = -Y)
	CAL_VTRACK_TZ = 0x04,				// [OK] Traslazione Z (incrementi, / 32)
	CAL_VTRACK_RZ = 0x20				// [OK] Rotazione attorno a Z (incrementi in gradi, / 512)
};										// Combinazioni trovate: 0, 1, 2, 3, 4, 5, 6, 7, 0x20, 0x23, 0x27


#pragma pack(push, 1)
struct CAL_ANIMBLOCK_HEADER		// SIZE: 4 bytes, seguito da (size - 4) / 2 valori int16 [ANIMBLOCK_HEADER]
{
	uint16_t frame;						// [OK] Ultimo frame coperto dal blocco (compreso)
	uint8_t word;						// [OK] CAL_ANIMBLOCK_TYPE (+ ANIM_LASTBLOCK nell'ultimo blocco)
	uint8_t size;						// [OK] Dimensione del blocco compreso l'header
};
#pragma pack(pop)
static_assert(sizeof(CAL_ANIMBLOCK_HEADER) == 4, "CAL_ANIMBLOCK_HEADER deve essere 4 bytes");


enum CAL_ANIMBLOCK_TYPE : uint8_t	// [SRC: ANIM_*] Valori trovati: 1, 2, 4, 9, 0xA, 0xC
{
	CAL_ANIM_BEZIER = 1,				// Bezier cubica: 4 valori (size 12) o 3 valori (size 10, A = valore precedente)
	CAL_ANIM_CONST = 2,					// 1 valore per tutti i frames (size 6)
	CAL_ANIM_KEYS = 4,					// 1 valore per frame (size 4 + 2 * n)
	CAL_ANIM_LASTBLOCK = 8				// Ultimo blocco della traccia
};


struct CAL_BLENDRANGE			// SIZE: 12 bytes [ANIMATION_BLENDRANGE]
{
	uint32_t destID;					// [OK] Hash dell'animazione in cui si puo' passare con un blend (sempre nello stesso file)
	uint32_t nClips;					// [OK] Numero di CAL_BLENDCLIP (1 nel 96% dei casi, al massimo 3)
	uint32_t BLENDCLIPS_PTR;			// [OK] Offset del primo CAL_BLENDCLIP (blocchi contigui dopo la lista dei range)
};
static_assert(sizeof(CAL_BLENDRANGE) == 12, "CAL_BLENDRANGE deve essere 12 bytes");


struct CAL_BLENDCLIP			// SIZE: 24 bytes [BLENDCLIP]. Tempi in frames (vedi TEMPI)
{
	uint32_t flags;						// [SRC] Flag del range nell'editor (0x8000 in tutti i file salvo 24 casi con 0)
	float sourceStart;					// [OK] Primo frame dell'animazione corrente da cui si puo' partire [source.msStart]
	float sourceEnd;					// [OK] Ultimo frame (se < sourceStart la finestra passa per la fine del ciclo) [source.msEnd]
	float destStart;					// [OK] Primo frame di ingresso nell'animazione di destinazione [dest.msStart]
	float destEnd;						// [OK] Ultimo frame di ingresso [dest.msEnd]
	float blendTime;					// [SRC] Durata del blend in frames [msBlendTime]
};
static_assert(sizeof(CAL_BLENDCLIP) == 24, "CAL_BLENDCLIP deve essere 24 bytes");


struct CAL_LINK					// SIZE: 12 bytes [ANIMATION_LINK]
{
	uint32_t destID;					// [OK] Animazione da raggiungere...
	uint32_t LLinkID;					// [OK] ...passando prima per questa animazione (lato sinistro)...
	uint32_t RLinkID;					// [OK] ...o per questa (lato destro; spesso uguale a LLinkID)
};										// Es. in LARA_STANCE: LARA_RUN via LARA_STANCE_TO_RUN / LARA_STANCE_TO_RUN_MIRROR
static_assert(sizeof(CAL_LINK) == 12, "CAL_LINK deve essere 12 bytes");


struct CAL_TRACK_KEY			// SIZE: 12 bytes [TRACK_KEY]
{
	uint32_t time;						// [OK] Bit 0-23: frame. Bit 24-31: CAL_AUDIO_EVENT_FLAGS (solo per i suoni)
	uint32_t type;						// [OK] CAL_TRACK_KEY_CODE
	uint32_t data;						// [OK] Dipende dal tipo
};
static_assert(sizeof(CAL_TRACK_KEY) == 12, "CAL_TRACK_KEY deve essere 12 bytes");


enum CAL_TRACK_KEY_CODE : uint32_t	// [SRC: TRACK_KEY_CODE_*]
{
	CAL_KEY_MAKE_INVISIBLE = 0,			// Nasconde la mesh data (hash: mesh agganciata, mesh o shape, es. TONFA, BAT_BODY1SHAPE)
	CAL_KEY_MAKE_VISIBLE = 1,			// Mostra la mesh data. Le mesh agganciate hanno quasi sempre una chiave al frame 0
	CAL_KEY_FIRE_AUDIO_EVENT = 2,		// Suono: data = ID dell'evento audio (0x582B nei passi di Lara)
	CAL_KEY_FIRE_ANIM_TRIGGER = 3,		// Animazione sovrapposta: data = hash (GENERIC_FEMALE_NOD, ... nei dialoghi *_DLG)
	CAL_KEY_FIRE_MORPH_TRIGGER = 4,		// Sequenza di morph (animazione facciale): data = hash del nome della sequenza
										// (in minuscolo, es. CARV_PA184 -> "pa184")
	CAL_KEY_FIRE_FXEVENT_ON = 5,		// Attiva l'effetto del CAL_FXNODE di indice data
	CAL_KEY_FIRE_FXEVENT_OFF = 6,		// Disattiva l'effetto del CAL_FXNODE di indice data
	CAL_KEY_FIRE_FXEVENT_ONESHOT = 7	// Effetto istantaneo del CAL_FXNODE di indice data (es. fiammate delle armi)
};


enum CAL_AUDIO_EVENT_FLAGS : uint32_t	// [SRC: AEFLAG_*] Bit alti di CAL_TRACK_KEY.time negli eventi audio
{
	CAL_AEFLAG_ISFOOTSTEP = 1 << 24,	// Passo (il suono dipende dal materiale della collisione)
	CAL_AEFLAG_ISLEFTRIGHT = 1 << 25,	// Piede sinistro / destro
	CAL_AEFLAG_ISFRONTBACK = 1 << 26	// Punta / tallone
};										// Valori trovati nel byte alto: 1, 2, 3, 5, 6, 7, 0x80, 0xC0


struct CAL_FXNODE				// SIZE: 160 bytes [FXNODE]. Effetto agganciato ad una bone (nodi "FXNode" di Maya)
{
	float rotate[4];					// [OK] Rotazione euleriana XYZ in radianti, W = 1
	float translate[4];					// [OK] Traslazione rispetto alla bone, W = 1
	float scale[4];						// [OK] Scala, W = 1 ((1, 1, 1) nell'89% dei casi)
	uint32_t flags;						// [SRC] FXNODE_MIRROR (1), FXNODE_ACTIVE (2): stato a runtime, nei file [GARBAGE]
										//       (0, 0xCDCDCDCD o valori casuali)
	char name[64];						// [OK] Nome del nodo Maya: TRANSFORMn, SMOKELOCATOR_TRANSFORM1, ... ([GARBAGE] dopo il NUL)
	uint16_t bone;						// [SRC] Indice della bone di aggancio [boneID & 0xFFFF] (es. PALM_L / PALM_R per le fiammate)
	uint16_t mirrorTrack;				// [SRC] bonemap[bone] << 16: indice di traccia speculare, calcolato dall'editor usando per
										//       errore l'indice della bone come indice di traccia [boneID >> 16]
	uint32_t argb;						// [SRC] Colore dell'effetto (0 nell'86% dei casi)
	uint32_t type;						// [SRC] Tipo di effetto ([MAYA] fxtype dell'FXNode + 1: fiammate 2, 3, 8, oggetti posati 4)
	float speed;						// [SRC] Parametri dell'effetto (0 nell'82% dei casi)
	float emit;							// [SRC]
	float random;						// [SRC]
	float u0;							// [SRC]
	float u1;							// [SRC]
	float u2;							// [SRC]
	float u3;							// [SRC] (spesso [GARBAGE])
	uint32_t Message;					// [SRC] "Message + 1 (so 0 is no message)" ([MAYA] animmsg 104 nelle prese di oggetti)
};
static_assert(sizeof(CAL_FXNODE) == 160, "CAL_FXNODE deve essere 160 bytes");
