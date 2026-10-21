#pragma once


/*==================================================================================================================
	FORMATO EVX - Tomb Raider: The Angel of Darkness (gruppi di eventi del livello)

	Fonti:
		[SRC]	WorldEdit (Tools\WorldEdit.exe: "EVXHeaderStart", "EventPointerStart"), documento "Map Messaging Tools In
				WorldEdit.doc" e lista dei messaggi Tools\Exporter\map_msg.h (enum NM_*, condivisa tra gioco e script)
		[OK]	verificato su tutti i 57 file EVX dei livelli (3802 messaggi)

	Un gruppo di eventi (event group) e' una sequenza di messaggi che il gioco invia, dal primo all'ultimo, quando il
	gruppo viene attivato: i gruppi vengono legati nell'editor ai messaggi ricevuti dagli oggetti della mappa (es. un
	trigger che riceve NM_ACTORCOLLIDE invia i messaggi del gruppo) o avviati con NM_DISPATCHEVENT.
	I nomi dei gruppi e dei nodi non sono salvati: sono hash (STRING_GetHashValue = GetHashValue di TRAODLE).

	STRUTTURA DEL FILE
		EVX_HEADER (12 bytes)
		EVX_GROUP x nGroups					Tabella dei gruppi
		Per ogni gruppo: uint32_t nMessages + EVX_MESSAGE x nMessages, all'offset indicato nella tabella
		All'offset EVX_HEADER.Size: EVX_TEMPLATES_HEADER + EVX_OBJTEMPLATE x nTemplates (Object Template degli oggetti
		interattivi, indicizzati da RMX_OBJECT.ObjectIndex) [SRC: tagMAP_OBJTEMPLATE][OK: 57 file, nTemplates = massimo
		ObjectIndex + 1 in tutti i livelli], seguiti da altri dati (template delle porte, non decodificati)
		Padding fino a multiplo di 2048 bytes

	Gruppi presenti in tutti i livelli [OK]: 0x7582915E (NM_OPEN), 0xC1617B88 (NM_CLOSE), 0xB9863743 (NM_UNLOCK), con un
	solo messaggio inviato al nodo stesso: i gruppi standard di porte e oggetti apribili.
==================================================================================================================*/


struct EVX_HEADER				// SIZE: 12 bytes
{
	uint32_t Version;					// [OK] Sempre 8
	uint32_t Size;						// [OK] Dimensione dei dati (header compreso; in 5 file 12 bytes oltre l'ultimo gruppo)
	uint32_t nGroups;					// [OK] Numero di gruppi di eventi
};
static_assert(sizeof(EVX_HEADER) == 12, "EVX_HEADER deve essere 12 bytes");


struct EVX_GROUP				// SIZE: 8 bytes
{
	uint32_t Offset;					// [OK] Offset dei messaggi del gruppo dall'inizio del file
	uint32_t Hash;						// [LOG] GetHashValue del nome del gruppo di eventi in maiuscolo (es. "GIVE_DIARY"). Il gruppo
										//		e' avviato dai nodi RMX con RMX_MSG.Param = indice del gruppo in questa tabella
};
static_assert(sizeof(EVX_GROUP) == 8, "EVX_GROUP deve essere 8 bytes");


struct EVX_MESSAGE				// SIZE: 12 bytes
{
	uint32_t Message;					// [SRC] Messaggio NM_* (map_msg.h) + EVX_MSG_SELF
	uint32_t Target;					// [SRC] Hash del nome del nodo destinatario (0 con EVX_MSG_SELF)
	uint32_t Param;						// [SRC] Parametro del messaggio (uParam): numero, hash (oggetto, filmato, gruppo di eventi,
										//       ...) o valori impacchettati (2 x 16 bit o 4 x 8 bit)
};
static_assert(sizeof(EVX_MESSAGE) == 12, "EVX_MESSAGE deve essere 12 bytes");


struct EVX_TEMPLATES_HEADER		// SIZE: 16 bytes
{
	uint32_t nTemplates;				// [OK] Numero di EVX_OBJTEMPLATE
	uint32_t Unknown1;					// [?] Minore o uguale a nTemplates (forse il numero di template delle porte, MAP_DOORTEMPLATE)
	uint32_t pad[2];					// [OK] Sempre 0
};
static_assert(sizeof(EVX_TEMPLATES_HEADER) == 16, "EVX_TEMPLATES_HEADER deve essere 16 bytes");


struct EVX_OBJTEMPLATE			// SIZE: 96 bytes [SRC: tagMAP_OBJTEMPLATE]. Object Template di un oggetto interattivo
{
	uint32_t Weight;					// [SRC] Peso
	uint32_t Strength;					// [SRC] Resistenza
	uint32_t Flags;						// [SRC] Flag dell'oggetto
	uint32_t pAO;						// [SRC] Puntatore runtime all'oggetto animato (0 nel file)
	uint32_t pAL;						// [SRC] Puntatore runtime alla lista di animazioni (0 nel file)
	uint32_t AnimCloned;				// [SRC]
	uint32_t DoorClass;					// [SRC] Classe della porta
	uint32_t PuzzleCount;				// [SRC] Numero di oggetti puzzle
	int32_t PuzzleItem[4];				// [SRC] Oggetti puzzle (A-D), -1 se non usati
	uint32_t UserValue;					// [SRC]
	uint32_t SubClass;					// [SRC]
	uint32_t DisconnectRule;			// [SRC]
	uint32_t Pad;						// [SRC]
	char AnimFileName[32];				// [SRC][OK] Nome del CHR "NODE" / CAL dell'oggetto animato (es. "DOOR_LEFT", "weed"),
										//		"__NULL__" se l'oggetto non e' animato. Le mesh sono quelle dello slot del RMX_OBJECT
};
static_assert(sizeof(EVX_OBJTEMPLATE) == 96, "EVX_OBJTEMPLATE deve essere 96 bytes");


#define EVX_MSG_SELF	0x80000000		// [OK] Messaggio inviato al nodo a cui e' legato il gruppo (sempre con Target = 0)
#define EVX_MSG_MASK	0x7FFFFFFF
