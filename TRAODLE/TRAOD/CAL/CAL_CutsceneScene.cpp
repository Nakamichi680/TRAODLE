#include "stdafx.h"
#include <set>
#include "Classes.h"
#include "TRAOD/CAL/CAL_Functions.h"


/*------------------------------------------------------------------------------------------------------------------
Scena unica di ogni filmato del livello: \NOMELIVELLO\Animations\<FILMATO>.MA, da scrivere dopo l'esportazione di CAL
e CAM. I filmati sono quelli che hanno un file CAM o POS (CS_* e IG_*). La scena carica come riferimenti:
	- tutte le animazioni <NOME>_<FILMATO>.ma della cartella Animations (attori e oggetti animati: ogni file carica a
	  sua volta il proprio personaggio o oggetto con un namespace unico, vedi MA_Write_SkeletalAnimationFile);
	  namespace = <NOME> (es. STRAHOVA_1, OBJECT01);
	- la telecamera ..\Cameras\<FILMATO>.MA (namespace CAMERA).
La timeline va dal frame 0 all'ultimo frame del piu' lungo tra telecamera e animazioni (30 fps).
------------------------------------------------------------------------------------------------------------------*/
static string CAL_Upper (string s)
{
	for (char &c : s)
		c = (char)toupper((unsigned char)c);
	return s;
}


/*------------------------------------------------------------------------------------------------------------------
Riferimento contenuto in un file di animazione (personaggio o oggetto): riga "file -r -ns <ns> -dr 1 -rfn <rn> ... <path>".
Il file del filmato deve ripeterlo con "-rdi 2", altrimenti Maya lo lascia scaricato (i riferimenti annidati sono
caricati secondo le informazioni del file principale).
------------------------------------------------------------------------------------------------------------------*/
static bool CAL_NestedReference (const string &path, string &ns, string &rn, string &file)
{
	ifstream in(path);
	string line;
	auto Quoted = [](const string &l, size_t from) -> string
	{
		size_t a = l.find('"', from), b = a == string::npos ? string::npos : l.find('"', a + 1);
		return b == string::npos ? string() : l.substr(a + 1, b - a - 1);
	};
	while (getline(in, line))
	{
		if (line.compare(0, 8, "file -r ") != 0)
			continue;
		size_t pns = line.find("-ns "), prn = line.find("-rfn ");
		if (pns == string::npos || prn == string::npos)
			return false;
		ns = Quoted(line, pns);
		rn = Quoted(line, prn);
		size_t last = line.rfind('"'), first = last == string::npos ? string::npos : line.rfind('"', last - 1);
		file = first == string::npos ? string() : line.substr(first + 1, last - first - 1);
		return !ns.empty() && !rn.empty() && !file.empty();
	}
	return false;
}


static int CAL_LastFrameOfFile (const string &path)				// Ultimo frame della timeline scritta in un file MA
{
	ifstream file(path);
	string line;
	while (getline(file, line))
	{
		size_t p = line.find("playbackOptions -min 0 -max ");
		if (p != string::npos)
			return atoi(line.c_str() + p + 28);
	}
	return -1;
}


bool Export_CutsceneScenes ()
{
	set <string> cutscenes;
	for (unsigned int i = 0; i < AOD_IO.gmxfiles.size(); i++)
		if (AOD_IO.gmxfiles[i].type == AoDFileType::CAM || AOD_IO.gmxfiles[i].type == AoDFileType::POS)
			cutscenes.insert(AOD_IO.gmxfiles[i].name.substr(0, AOD_IO.gmxfiles[i].name.find('.')));
	if (cutscenes.empty())
		return true;

	// File di animazione esportati: Animations\<CAL>\<animazione>.ma
	vector <pair <string, string>> files;							// (sottocartella, nome senza estensione)
	WIN32_FIND_DATAA dir;
	HANDLE hdir = FindFirstFileA((AOD_IO.folder_animations + "*").c_str(), &dir);
	if (hdir != INVALID_HANDLE_VALUE)
	{
		do
		{
			string sub = dir.cFileName;
			if (!(dir.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || sub == "." || sub == "..")
				continue;
			WIN32_FIND_DATAA f;
			HANDLE hf = FindFirstFileA((AOD_IO.folder_animations + sub + "\\*.ma").c_str(), &f);
			if (hf == INVALID_HANDLE_VALUE)
				continue;
			do
			{
				string name = f.cFileName;
				files.push_back(make_pair(sub, name.substr(0, name.size() - 3)));
			} while (FindNextFileA(hf, &f));
			FindClose(hf);
		} while (FindNextFileA(hdir, &dir));
		FindClose(hdir);
	}

	bool ok = true;
	for (const string &cs : cutscenes)
	{
		struct Ref { string ns, path; };
		vector <Ref> refs;
		int last = -1;
		string suffix = "_" + CAL_Upper(cs);
		for (const pair <string, string> &f : files)
		{
			string upper = CAL_Upper(f.second);
			if (upper.size() <= suffix.size() || upper.compare(upper.size() - suffix.size(), suffix.size(), suffix) != 0)
				continue;
			refs.push_back({f.second.substr(0, f.second.size() - suffix.size()), f.first + "/" + f.second + ".ma"});
			last = max(last, CAL_LastFrameOfFile(AOD_IO.folder_animations + f.first + "\\" + f.second + ".ma"));
		}
		if (refs.empty())
			continue;
		sort(refs.begin(), refs.end(), [](const Ref &a, const Ref &b) { return a.ns < b.ns; });
		ifstream cam(AOD_IO.folder_cameras + cs + ".MA");
		if (cam.is_open())
		{
			refs.push_back({"CAMERA", "../Cameras/" + cs + ".MA"});
			ifstream camfile(AOD_IO.folder_level + cs + ".CAM", ios::binary);	// Numero di frames della telecamera
			uint32_t header[4] = {0, 0, 0, 0};
			if (camfile.read(reinterpret_cast<char*>(header), sizeof(header)) && header[3] > 0)
				last = max(last, (int)header[3] - 1);
		}
		if (last < 0)
			last = 0;

		string path = AOD_IO.folder_animations + cs + ".MA";
		ofstream out(path);
		if (!out.is_open())
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Unable to create " << path;
			ok = false;
			continue;
		}
		out << "//Maya ASCII 2016 scene\n";
		out << "//Name: " << cs << ".MA\n";
		out << "//Cutscene " << cs << ": actors, animated objects and camera, exported by TRAODLE\n";
		out << "requires maya \"2016\";\n";
		out << "currentUnit -l centimeter -a degree -t ntsc;\n";
		for (const Ref &r : refs)
		{
			out << "file -rdi 1 -ns \"" << r.ns << "\" -rfn \"" << r.ns << "RN\" -typ \"mayaAscii\" \"" << r.path << "\";\n";
			string ns, rn, file;										// Personaggio / oggetto caricato dal file dell'animazione
			string full = AOD_IO.folder_animations + r.path;
			replace(full.begin(), full.end(), '/', '\\');
			if (CAL_NestedReference(full, ns, rn, file))
				out << "file -rdi 2 -ns \"" << ns << "\" -rfn \"" << r.ns << ":" << rn << "\" -typ \"mayaAscii\" \"" << file << "\";\n";
		}
		for (const Ref &r : refs)
			out << "file -r -ns \"" << r.ns << "\" -dr 1 -rfn \"" << r.ns << "RN\" -typ \"mayaAscii\" \"" << r.path << "\";\n";
		for (const Ref &r : refs)
		{
			out << "createNode reference -n \"" << r.ns << "RN\";\n";
			out << "	setAttr \".ed\" -type \"dataReferenceEdits\" \n";
			out << "		\"" << r.ns << "RN\";\n";
		}
		out << "createNode script -n \"sceneConfigurationScriptNode\";\n";
		out << "	setAttr \".b\" -type \"string\" \"playbackOptions -min 0 -max " << last << " -ast 0 -aet " << last << " \";\n";
		out << "	setAttr \".st\" 6;\n";
		out << "// End of " << cs << ".MA\n";
		msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Cutscene " << cs << ": " << refs.size() << " references, " << last + 1 << " frames, output filename: " << path;
	}
	return ok;
}
