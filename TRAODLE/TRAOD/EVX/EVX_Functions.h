#pragma once
#include "TRAOD/EVX/EVX_Struct.h"


bool Export_EVX (string filename);
const vector <EVX_OBJTEMPLATE> &EVX_GetTemplates ();	// Object Template del livello (file <LIVELLO>.EVX), per RMX_OBJECT.ObjectIndex
string EVX_MessageName (uint32_t id);				// Nome del messaggio NM_* (map_msg.h), MESSAGE_<id> se sconosciuto					// Esportazione dei gruppi di eventi in un file di testo (cartella Scripts)
