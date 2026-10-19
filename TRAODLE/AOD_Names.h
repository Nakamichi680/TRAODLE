#pragma once
#include <map>


// Nomi originali degli hash di AoD (dizionario Resources/AoD_Names.txt, vedi AOD_Names.cpp). "" se il nome non e' noto.
string AOD_NodeName (uint32_t hash);								// Nodo RMX o stanza (hash = GetHashValue(nome))
string AOD_MeshName (uint32_t hash);								// Mesh degli oggetti della ZONE (GetHashValue del nome in maiuscolo)
string AOD_EventName (uint32_t hash);								// Gruppo di eventi EVX (GetHashValue del nome in maiuscolo)
string AOD_TemplateName (unsigned int index, unsigned int nTemplates);	// Object Template del livello corrente, se il livello ne ha nTemplates
string AOD_ScriptName (unsigned int index, unsigned int nScripts);	// Script del livello corrente, se il livello ne ha nScripts
const map <uint32_t, string> &AOD_NodeNames ();						// Tutti i nomi dei nodi e delle stanze (hash -> nome)
const map <uint32_t, string> &AOD_EventNames ();					// Tutti i nomi dei gruppi di eventi (hash -> nome)
string AOD_MayaName (string name);

class AOD_Effect {														// Template di un effetto di WorldEdit (effects.fsc)
public:
	string name;														// Nome del template (es. "Flame Torch 1")
	string shape;														// Forma dell'emitter: ST_BOX, ST_LINE, ST_POINT
	bool oneshot = false;												// EF_ONESHOT (altrimenti EF_CONST, effetto continuo)
};
const AOD_Effect *AOD_EffectTemplate (uint32_t hash);					// Template con hash = GetHashValue(nome in maiuscolo), nullptr se sconosciuto									// Nome valido per un nodo di Maya (caratteri diversi da A-Z a-z 0-9 _ -> _)
