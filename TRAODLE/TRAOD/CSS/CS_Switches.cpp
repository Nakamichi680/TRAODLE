#include "stdafx.h"
#include "Classes.h"
#include "hash_Functions.h"
#include "TRAOD/CSS/CSS_Functions.h"


/*------------------------------------------------------------------------------------------------------------------
Lettura di un file <FILMATO>.TXT (vedi CSS_Functions.h). Come CS_Initialise del gioco: righe di intestazione
"#MAP <nome> <stato>" o "#CHAR <attore> <stato>", poi il numero di voci, poi una voce per riga (sscanf "%s": spazi e
tabulazioni ignorati). La lettura si ferma al primo carattere nullo o non ASCII (dopo il testo il file contiene dati
residui) o alla prima intestazione non valida.
------------------------------------------------------------------------------------------------------------------*/
static bool CS_ParseSwitches (const vector <char> &data, CS_Switches &out)
{
	string text;
	for (char c : data)
	{
		if (c == 0 || (unsigned char)c > 126 || ((unsigned char)c < 32 && c != '\r' && c != '\n' && c != '\t'))
			break;
		text.push_back(c);
	}
	vector <vector <string>> lines;							// Parole di ogni riga non vuota
	stringstream ss(text);
	string line;
	while (getline(ss, line))
	{
		stringstream ls(line);
		vector <string> words;
		string w;
		while (ls >> w)
			words.push_back(w);
		if (!words.empty())
			lines.push_back(words);
	}
	size_t i = 0;
	while (i < lines.size())
	{
		string head = lines[i][0];
		for (char &c : head)
			c = (char)toupper((unsigned char)c);
		if ((head != "#MAP" && head != "#CHAR") || lines[i].size() < 2 || i + 1 >= lines.size())
			break;
		CS_SwitchBlock block;
		block.map = head == "#MAP";
		block.name = lines[i][1];
		block.state = lines[i].size() > 2 ? lines[i][2] : "";
		int count = atoi(lines[i + 1][0].c_str());
		i += 2;
		for (int k = 0; k < count && i < lines.size(); k++, i++)
			block.items.push_back(lines[i][0]);
		out.blocks.push_back(block);
	}
	for (const CS_SwitchBlock &b : out.blocks)				// Testo pulito (riscritto con lo stesso formato)
	{
		out.text += (b.map ? "#MAP " : "#CHAR ") + b.name + (b.state.empty() ? "" : " " + b.state) + "\r\n";
		out.text += to_string(b.items.size()) + "\r\n";
		for (const string &item : b.items)
			out.text += item + "\r\n";
	}
	return !out.blocks.empty();
}


/*------------------------------------------------------------------------------------------------------------------
Switch di tutti i filmati del livello (file TXT), letti una sola volta dalla cartella \NOMELIVELLO.
------------------------------------------------------------------------------------------------------------------*/
const vector <CS_Switches> &CS_GetSwitches ()
{
	static vector <CS_Switches> switches;
	static string level;
	if (level == AOD_IO.levelname && !switches.empty())
		return switches;
	level = AOD_IO.levelname;
	switches.clear();
	for (unsigned int f = 0; f < AOD_IO.gmxfiles.size(); f++)
	{
		if (AOD_IO.gmxfiles[f].type != AoDFileType::TXT)
			continue;
		string path = AOD_IO.folder_level + AOD_IO.gmxfiles[f].name;
		ifstream file(path, ios::binary);
		vector <char> data((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
		CS_Switches s;
		s.cutscene = AOD_IO.gmxfiles[f].name.substr(0, AOD_IO.gmxfiles[f].name.find('.'));
		if (CS_ParseSwitches(data, s))
			switches.push_back(s);
	}
	return switches;
}


/*------------------------------------------------------------------------------------------------------------------
Oggetti agganciati da nascondere all'inizio dell'animazione di un attore di un filmato: l'animazione <ATTORE>_<FILMATO>
(nome del CAL_ANIMATION) corrisponde al blocco "#CHAR <ATTORE> OFF" del file <FILMATO>.TXT. "_NO_CULL_" non e' un
oggetto (flag CHARFLAGS_NOCULL del personaggio) e viene escluso.
------------------------------------------------------------------------------------------------------------------*/
vector <string> CS_HiddenAttachments (string animation)
{
	vector <string> hidden;
	for (char &c : animation)
		c = (char)toupper((unsigned char)c);
	for (const CS_Switches &s : CS_GetSwitches())
		for (const CS_SwitchBlock &b : s.blocks)
		{
			if (b.map)
				continue;
			string full = b.name + "_" + s.cutscene;
			for (char &c : full)
				c = (char)toupper((unsigned char)c);
			if (full != animation)
				continue;
			for (const string &item : b.items)
				if (item != "_NO_CULL_")
					hidden.push_back(item);
		}
	return hidden;
}


/*------------------------------------------------------------------------------------------------------------------
Filmati che nascondono il nodo della mappa con l'hash indicato ("#MAP WORLD OFF": il gioco imposta il bit 0x2000000 di
ObjFlags dell'oggetto RMX con quel nome), separati da spazi.
------------------------------------------------------------------------------------------------------------------*/
string CS_CutscenesHidingNode (uint32_t hash)
{
	string out;
	for (const CS_Switches &s : CS_GetSwitches())
		for (const CS_SwitchBlock &b : s.blocks)
			if (b.map)
				for (const string &item : b.items)
					if ((uint32_t)GetHashValue(item.c_str()) == hash && out.find(s.cutscene) == string::npos)
						out += (out.empty() ? "" : " ") + s.cutscene;
	return out;
}


/*------------------------------------------------------------------------------------------------------------------
Esporta gli switch di un filmato come testo pulito in \NOMELIVELLO\Cameras\<FILMATO>_SWITCHES.txt.
------------------------------------------------------------------------------------------------------------------*/
bool Export_TXT (string filename)
{
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Reading " << filename;
	string cutscene = filename.substr(0, filename.find('.'));
	for (const CS_Switches &s : CS_GetSwitches())
	{
		if (s.cutscene != cutscene)
			continue;
		CreateDirectory(AOD_IO.folder_cameras_lpwstr, NULL);
		string path = AOD_IO.folder_cameras + cutscene + "_SWITCHES.txt";
		ofstream out(path, ios::binary);
		out << "// Oggetti nascosti durante il filmato " << cutscene << " (file " << filename << ", letto all'avvio da CS_Initialise)\r\n";
		out << "// #MAP: nodi oggetto della mappa (RMX). #CHAR <attore>: oggetti agganciati del personaggio __<attore>; _NO_CULL_ = sempre disegnato\r\n";
		out << s.text;
		msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Output filename: " << path;
		return true;
	}
	msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << filename << " contains no cutscene switches.";
	return true;
}
