#pragma once
#include "TRAOD/CSS/CSS_Struct.h"


bool Export_CSS (string filename);					// Esportazione dei sottotitoli di un filmato in un file di testo (cartella Subtitles)


/*------------------------------------------------------------------------------------------------------------------
File <FILMATO>.TXT dei filmati (CS_* e IG_*): oggetti nascosti durante il filmato [SRC: Debug.elf, CS_Initialise].
	#MAP WORLD OFF				nodi oggetto della mappa: il gioco imposta il bit 0x2000000 di RMX_OBJECT.ObjFlags
	<n>
	<nome del nodo RMX> x n
	#CHAR <ATTORE> OFF			oggetti agganciati del personaggio della mappa "__<attore>" (minuscolo):
	<n>							AO_SetParentedObjectVisibility(oggetto, hash, 0); "_NO_CULL_" imposta invece
	<nome> x n					CHARFLAGS_NOCULL del personaggio
Dopo il testo il file contiene dati residui (ignorati).
------------------------------------------------------------------------------------------------------------------*/
class CS_SwitchBlock {
public:
	bool map = false;									// #MAP (nodi della mappa) o #CHAR (oggetti di un attore)
	string name;										// "WORLD" o nome dell'attore (es. STRAHOVA_1)
	string state;										// "OFF" (o vuoto)
	vector <string> items;
};

class CS_Switches {
public:
	string cutscene;									// Nome del filmato (nome del file TXT)
	vector <CS_SwitchBlock> blocks;
	string text;										// Testo pulito
};

const vector <CS_Switches> &CS_GetSwitches ();			// Switch di tutti i filmati del livello
vector <string> CS_HiddenAttachments (string animation);	// Oggetti nascosti all'inizio dell'animazione <ATTORE>_<FILMATO>
string CS_CutscenesHidingNode (uint32_t hash);			// Filmati che nascondono il nodo della mappa (separati da spazi)
bool Export_TXT (string filename);						// \Cameras\<FILMATO>_SWITCHES.txt
