#pragma once
#include "TRAOD/EVX/EVX_Struct.h"


bool Export_EVX (string filename);
string EVX_MessageName (uint32_t id);				// Nome del messaggio NM_* (map_msg.h), MESSAGE_<id> se sconosciuto					// Esportazione dei gruppi di eventi in un file di testo (cartella Scripts)
