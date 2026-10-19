#pragma once


/*==================================================================================================================
	FORMATO CBH - Tomb Raider: The Angel of Darkness (telecamere di gioco dei livelli, <LIVELLO>.CBH)
	[SRC: Debug.elf, _tag_CAMERA_FILE / _tag_RailSequence / _tag_Cine_Camera; Camera_LoadCameras_CurrentVersion]
	[OK] verificato su tutti i 32 file CBH dei livelli (sempre 81920 bytes, Version 2)

	Sono le telecamere che il gioco usa al posto della telecamera che segue Lara: inquadrature fisse attivate da un
	trigger e "rotaie" su cui la telecamera scorre seguendo Lara. Il file e' l'immagine della struttura CAMERA_FILE
	(dimensione fissa, le voci oltre NumCameras / NumRails sono vuote o residui):

		CBH_HEADER							16 bytes
		CBH_RAIL Rails[75]					75 x 20 bytes (offset 16)
		(padding fino a 16 bytes)
		CBH_CAMERA Cameras[1000]			1000 x 80 bytes (offset 1520)
		padding							3200 bytes (offset 81520)

	VISTA DELLA TELECAMERA [SRC: mathLookAt, chiamata a ogni frame dal ciclo principale con posizione, target e roll
	della telecamera corrente; vale anche per le telecamere dei filmati, file CAM]
		- il roll e' in gradi;
		- il vettore "alto" e' l'asse Z del mondo ruotato di roll attorno all'asse Y del mondo: (sin roll, 0, cos roll);
		- la telecamera guarda esattamente il target; l'alto dello schermo e' la componente del vettore alto
		  perpendicolare alla direzione di vista.
		Il roll quindi agisce come un vero roll solo quando la telecamera guarda lungo +-Y; guardando lungo +-X non ha
		effetto. Vedi CAM_GameCameraRotation.
	Il FOV e' l'angolo di campo in gradi (mathLoadPerspMatrix: FOV * PI / 360 = semiangolo).
==================================================================================================================*/


#define CBH_MAX_RAILS	75
#define CBH_MAX_CAMERAS	1000
#define CBH_RAILS_PTR	16
#define CBH_CAMERAS_PTR	1520


struct CBH_HEADER				// SIZE: 16 bytes
{
	uint32_t Version;					// [OK] Sempre 2
	uint32_t NumCameras;				// [OK] Numero di telecamere usate (CBH_CAMERA)
	uint32_t NumRails;					// [OK] Numero di rotaie (CBH_RAIL)
	uint32_t pad;						// [GARBAGE]
};
static_assert(sizeof(CBH_HEADER) == 16, "CBH_HEADER deve essere 16 bytes");


struct CBH_RAIL					// SIZE: 20 bytes [SRC: _tag_RailSequence]
{
	uint32_t CameraCount;				// [OK] Numero di telecamere della rotaia (0-2): la prima e' HeadCamera, le altre seguono
										//		CBH_CAMERA.NextRailIndex. La telecamera scorre lungo la spezzata delle loro posizioni
	uint32_t RailType;					// [SRC] Tipo di rotaia (0 o 1)
	int32_t HeadCamera;					// [OK] Indice della prima telecamera, -1 se la rotaia e' vuota
	uint32_t TriggerHashID;				// [OK] Hash del trigger (RMX_TRIGGER) che attiva la rotaia, 0 se nessuno
	uint32_t next;						// [GARBAGE] Puntatore del runtime
};
static_assert(sizeof(CBH_RAIL) == 20, "CBH_RAIL deve essere 20 bytes");


struct CBH_CAMERA				// SIZE: 80 bytes [SRC: _tag_Cine_Camera]
{
	float EyePos[4];					// [OK] Posizione della telecamera (W [GARBAGE])
	float Target[4];					// [OK] Punto inquadrato (W [GARBAGE])
	float Roll;							// [OK] Roll in gradi (vedi VISTA DELLA TELECAMERA)
	float FOV;							// [OK] Angolo di campo in gradi
	float pad[2];						// [GARBAGE]
	float RailTarget[4];				// [OK] Telecamere delle rotaie: secondo punto di mira (di solito il target 650 unita' piu' in
										//		basso); 0 nelle telecamere fisse
	int32_t NextRailIndex;				// [OK] Telecamere delle rotaie: indice della telecamera successiva, -1 per l'ultima (0 nelle fisse)
	uint32_t TrackingMode;				// [SRC] 0 = la telecamera guarda la testa di Lara invece del target (Camera_UpdateActiveCamera:
										//		Camera_TargetPlayerHead), 1 = inquadratura fissa sul target
	uint32_t TriggerHashID;				// [OK] Hash del trigger (RMX_TRIGGER) che attiva la telecamera, 0 se nessuno
	uint32_t next;						// [GARBAGE] Puntatore del runtime
};
static_assert(sizeof(CBH_CAMERA) == 80, "CBH_CAMERA deve essere 80 bytes");
