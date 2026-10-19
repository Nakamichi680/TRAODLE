#include "stdafx.h"
#include <map>
#include "Classes.h"
#include "resource.h"
#include "hash_Functions.h"
#include "AOD_Names.h"


/*------------------------------------------------------------------------------------------------------------------
Nomi originali degli hash di AoD. I file del gioco contengono solo gli hash dei nomi (GetHashValue) di stanze, nodi,
mesh e gruppi di eventi; i nomi sono stati ricavati dai log dell'"Export manager" di WorldEdit (2003) e salvati nella
risorsa IDR_AOD_NAMES (Resources/AoD_Names.txt, formato descritto nell'intestazione del file). L'hash viene ricalcolato
dal nome al caricamento, quindi un nome viene usato solo se il suo hash coincide con quello del file del gioco: vale anche
per versioni diverse dei livelli e per i livelli senza log, dove i nomi mancanti restano hash.
I template degli effetti (righe F, dal file effects.fsc di WorldEdit) sono indicizzati per hash come i nodi.
I nomi degli Object Template e degli script non hanno hash: sono per livello e per indice, e vengono usati solo se il
numero di template / script del livello coincide con quello del log.
------------------------------------------------------------------------------------------------------------------*/
class AOD_NameDictionary {
public:
	map <uint32_t, string> nodes, meshes, events;
	map <string, vector <string>> templates, scripts;				// Livello -> nomi in ordine di indice
	map <uint32_t, AOD_Effect> effects;								// Template degli effetti (RMX_EMITTER.EffectHash)
};


static string AOD_Upper (string s)
{
	for (char &c : s)
		c = (char)toupper((unsigned char)c);
	return s;
}


static const AOD_NameDictionary &AOD_Names ()
{
	static AOD_NameDictionary dictionary;
	static bool loaded = false;
	if (loaded)
		return dictionary;
	loaded = true;
	HRSRC resource = FindResource(NULL, MAKEINTRESOURCE(IDR_AOD_NAMES), RT_RCDATA);
	HGLOBAL handle = resource ? LoadResource(NULL, resource) : NULL;
	const char *text = handle ? static_cast<const char*>(LockResource(handle)) : nullptr;
	if (!text)
	{
		msg(msg::TGT::FILE, msg::TYP::WARN) << "Names dictionary (IDR_AOD_NAMES) not found: hashes are exported without names.";
		return dictionary;
	}
	stringstream in(string(text, SizeofResource(NULL, resource)));
	string line;
	while (getline(in, line))
	{
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		vector <string> field;
		size_t start = 0, tab;
		while ((tab = line.find('\t', start)) != string::npos)
		{
			field.push_back(line.substr(start, tab - start));
			start = tab + 1;
		}
		field.push_back(line.substr(start));
		if (field.size() == 2 && field[0] == "N")
			dictionary.nodes[(uint32_t)GetHashValue(field[1].c_str())] = field[1];
		else if (field.size() == 2 && field[0] == "M")
			dictionary.meshes[(uint32_t)GetHashValue(AOD_Upper(field[1]).c_str())] = field[1];
		else if (field.size() == 2 && field[0] == "E")
			dictionary.events[(uint32_t)GetHashValue(AOD_Upper(field[1]).c_str())] = field[1];
		else if (field.size() == 4 && field[0] == "F")
		{
			AOD_Effect effect;
			effect.name = field[1];
			effect.shape = field[2];
			effect.oneshot = field[3] == "EF_ONESHOT";
			dictionary.effects[(uint32_t)GetHashValue(AOD_Upper(field[1]).c_str())] = effect;
		}
		else if (field.size() == 4 && (field[0] == "T" || field[0] == "S"))
		{
			vector <string> &list = (field[0] == "T" ? dictionary.templates : dictionary.scripts)[field[1]];
			unsigned int index = atoi(field[2].c_str());
			if (list.size() <= index)
				list.resize(index + 1);
			list[index] = field[3];
		}
	}
	msg(msg::TGT::FILE, msg::TYP::LOG) << "Names dictionary: " << dictionary.nodes.size() << " nodes, " << dictionary.meshes.size() << " meshes, " << dictionary.events.size() << " event groups.";
	return dictionary;
}


static string AOD_Find (const map <uint32_t, string> &names, uint32_t hash)
{
	map <uint32_t, string>::const_iterator it = names.find(hash);
	return it == names.end() ? "" : it->second;
}


string AOD_NodeName (uint32_t hash)
{
	return AOD_Find(AOD_Names().nodes, hash);
}


string AOD_MeshName (uint32_t hash)
{
	return AOD_Find(AOD_Names().meshes, hash);
}


string AOD_EventName (uint32_t hash)
{
	return AOD_Find(AOD_Names().events, hash);
}


static string AOD_LevelName (const map <string, vector <string>> &lists, unsigned int index, unsigned int count)
{
	map <string, vector <string>>::const_iterator it = lists.find(AOD_Upper(AOD_IO.levelname));
	if (it == lists.end() || it->second.size() != count || index >= count)
		return "";
	return it->second[index];
}


string AOD_TemplateName (unsigned int index, unsigned int nTemplates)
{
	return AOD_LevelName(AOD_Names().templates, index, nTemplates);
}


string AOD_ScriptName (unsigned int index, unsigned int nScripts)
{
	return AOD_LevelName(AOD_Names().scripts, index, nScripts);
}


const map <uint32_t, string> &AOD_NodeNames ()
{
	return AOD_Names().nodes;
}


const map <uint32_t, string> &AOD_EventNames ()
{
	return AOD_Names().events;
}


const AOD_Effect *AOD_EffectTemplate (uint32_t hash)
{
	map <uint32_t, AOD_Effect>::const_iterator it = AOD_Names().effects.find(hash);
	return it == AOD_Names().effects.end() ? nullptr : &it->second;
}


string AOD_MayaName (string name)
{
	for (char &c : name)
		if (!isalnum((unsigned char)c) && c != '_')
			c = '_';
	if (!name.empty() && isdigit((unsigned char)name[0]))
		name = "_" + name;
	return name;
}
