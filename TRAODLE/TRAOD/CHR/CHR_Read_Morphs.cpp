#include "stdafx.h"
#include <map>
#include <set>
#include "Classes.h"
#include "TRAOD/CHR/CHR_Functions.h"
#include "TRAOD/CHR/TMS_Struct.h"
#include "TRAOD/CAL/CAL_Functions.h"
#include "hash_Functions.h"


/*------------------------------------------------------------------------------------------------------------------
Legge i blend shapes del volto (file TMT estratti dal GMX del livello, vedi TMT_Struct.h) e li indicizza con l'ID della
mesh base (BaseId = ID della MESH2 del volto nel CHR).
Se piu' TMT hanno lo stesso BaseId (es. LARA.TMT, LARA_IG_1_24.TMT, LARA_CS_1_22D.TMT) ne viene usato uno, in ordine di
priorita': quello indicato da preferred (filmati: TMT con il nome del filmato, es. LARA_CS_1_22D), quello con il nome del
personaggio, quello di cui il personaggio e' una variante (LARA per LARAD, LARAC1, ...), quello il cui nome inizia con il
nome del personaggio, altrimenti il primo.
Gli altri TMT dello stesso volto con un Id diverso (es. LARA_CS_1_22D.TMT) contengono gli stessi target in un ordine
diverso: per ognuno viene calcolata la corrispondenza dei target (CHR_Morph.remap), usata dalle animazioni facciali.
INPUT: string chrname, string preferred (nome del TMT da preferire, vuoto se nessuno)
OUTPUT: map <uint32_t, CHR_Morph>
------------------------------------------------------------------------------------------------------------------*/
map <uint32_t, CHR_Morph> CHR_Read_Morphs (string chrname, string preferred)
{
	map <uint32_t, CHR_Morph> morphs;
	vector <CHR_Morph> others;								// TMT non scelti (per la corrispondenza dei target)
	vector <uint32_t> others_base;
	map <uint32_t, int> priority;							// Priorita' del TMT scelto per ogni volto (vedi sopra)
	for (const auto &f : AOD_IO.gmxfiles)
	{
		if (f.type != AoDFileType::TMT)
			continue;
		ifstream tmtfile(f.name, std::ios::binary);
		TMT_HEADER header;
		if (!tmtfile.read(reinterpret_cast<char*>(&header), sizeof(header)) || header.Magic != TMT_MAGIC || header.nTargets == 0 || header.nTargets > 64 || header.nVertices == 0 || header.nVertices > 65535)
		{
			msg(msg::TGT::FILE, msg::TYP::WARN) << f.name << " is not a valid TMT file. Skipped.";
			continue;
		}
		string name = f.name.substr(0, f.name.find('.'));
		int p = (!preferred.empty() && name == preferred) ? 4 : (name == chrname) ? 3 :
			(chrname.compare(0, name.size(), name) == 0) ? 2 : (name.compare(0, chrname.size() + 1, chrname + "_") == 0) ? 1 : 0;

		CHR_Morph morph;
		morph.name = name;
		morph.Id = header.Id;
		morph.nTargets = header.nTargets;
		morph.nVertices = header.nVertices;
		vector <TMT_VERTEX> records((size_t)header.nVertices * (1 + header.nTargets));
		tmtfile.seekg(8 + header.VERTICES_PTR);
		if (!tmtfile.read(reinterpret_cast<char*>(records.data()), records.size() * sizeof(TMT_VERTEX)))
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Error reading " << f.name << ". Skipped.";
			continue;
		}
		morph.base.resize(header.nVertices);
		morph.target.assign(header.nTargets, vector <TMT_VERTEX> (header.nVertices));
		for (uint32_t v = 0; v < header.nVertices; v++)
		{
			morph.base[v] = records[(size_t)v * (1 + header.nTargets)];
			for (uint32_t t = 0; t < header.nTargets; t++)
				morph.target[t][v] = records[(size_t)v * (1 + header.nTargets) + 1 + t];
		}
		if (morphs.count(header.BaseId) && priority[header.BaseId] >= p)
		{
			others.push_back(morph);
			others_base.push_back(header.BaseId);
			continue;
		}
		if (morphs.count(header.BaseId))
		{
			others.push_back(morphs[header.BaseId]);
			others_base.push_back(header.BaseId);
		}
		morphs[header.BaseId] = morph;
		priority[header.BaseId] = p;
	}

	// Corrispondenza dei target degli altri TMT dello stesso volto: target con gli stessi offset delle posizioni
	for (unsigned int o = 0; o < others.size(); o++)
	{
		CHR_Morph &morph = morphs[others_base[o]];
		const CHR_Morph &other = others[o];
		if (other.Id == morph.Id || morph.remap.count(other.Id) || other.nVertices != morph.nVertices)
			continue;
		vector <int> mapping(other.nTargets, -1);
		set <int> used;
		for (unsigned int t = 0; t < other.nTargets; t++)
		{
			float best = 0.01f;										// Scarto massimo ammesso
			for (unsigned int u = 0; u < morph.nTargets; u++)
			{
				float diff = 0;
				for (unsigned int v = 0; v < morph.nVertices && diff < best; v++)
				{
					const TMT_VERTEX &a = other.target[t][v], &b = morph.target[u][v];
					diff = max(diff, max(fabsf(a.X - b.X), max(fabsf(a.Y - b.Y), fabsf(a.Z - b.Z))));
				}
				if (diff < best && !used.count(u))
				{
					best = diff;
					mapping[t] = u;
				}
			}
			if (mapping[t] >= 0)
				used.insert(mapping[t]);
		}
		if (find(mapping.begin(), mapping.end(), -1) == mapping.end())
		{
			morph.remap[other.Id] = mapping;
			msg(msg::TGT::FILE, msg::TYP::LOG) << other.name << ".TMT: same targets as " << morph.name << ".TMT in a different order.";
		}
		else
			msg(msg::TGT::FILE, msg::TYP::WARN) << other.name << ".TMT: targets different from " << morph.name << ".TMT. Its facial animations are not exported.";
	}
	return morphs;
}


/*------------------------------------------------------------------------------------------------------------------
Legge le animazioni facciali (sequenze MPHS dei file TMS e ".3" del livello, vedi TMS_Struct.h) che animano i target
del TMT indicato (TargetId = TMT_HEADER.Id) o di un altro TMT dello stesso volto con i target riordinati (CHR_Morph.remap).
Peso di ogni target per ogni frame: traccia / 8192 * inviluppo / 8192.
Nomi: file .TMS -> nome del file; battute di dialogo dei file ".3" -> nome il cui hash e' l'Id della sequenza
(<2 lettere minuscole><numero>, es. "pa211"), altrimenti l'Id in esadecimale.
INPUT: const CHR_Morph &morph, string chrname
OUTPUT: vector <BlendShapeAnimation> (meshes e targets gia' compilati)
------------------------------------------------------------------------------------------------------------------*/
vector <BlendShapeAnimation> CHR_Read_FacialAnimations (const CHR_Morph &morph, string chrname)
{
	vector <BlendShapeAnimation> anims;
	vector <uint32_t> ids;										// Id delle sequenze da nominare (battute di dialogo)
	for (const auto &f : AOD_IO.gmxfiles)
	{
		if (f.type != AoDFileType::TMS)
			continue;
		ifstream file(f.name, std::ios::binary | std::ios::ate);
		if (!file.is_open())
			continue;
		vector <uint8_t> data((size_t)file.tellg());
		file.seekg(0);
		file.read(reinterpret_cast<char*>(data.data()), data.size());
		bool single = f.name.size() > 4 && f.name.substr(f.name.size() - 4) == ".TMS";
		size_t p = 0;
		while (p + sizeof(MPHS_HEADER) <= data.size())
		{
			MPHS_HEADER h;
			memcpy(&h, data.data() + p, sizeof(h));
			if (h.Magic != MPHS_MAGIC || h.Size < sizeof(MPHS_HEADER) || p + h.Size > data.size())
				break;
			map <uint32_t, vector <int>>::const_iterator remap = morph.remap.find(h.TargetId);	// TMT dello stesso volto con i target riordinati
			if ((h.TargetId == morph.Id || remap != morph.remap.end()) && h.NumFrames > 0 && h.NumFrames < 100000)
			{
				const uint8_t *base = data.data() + p + 8;				// Gli offset sono relativi all'offset 8 del blocco
				size_t available = h.Size - 8;
				auto Track = [&](uint32_t ptr) -> vector <float>			// Valori di una traccia (MPHS_TRACK all'offset ptr)
				{
					MPHS_TRACK track;
					if (ptr + sizeof(track) > available)
						return vector <float> (h.NumFrames, 0);
					memcpy(&track, base + ptr, sizeof(track));
					if (track.BLOCKS_PTR >= available)
						return vector <float> (h.NumFrames, 0);
					return CAL_DecodeTrack(base + track.BLOCKS_PTR, available - track.BLOCKS_PTR, h.NumFrames);
				};
				BlendShapeAnimation anim;
				anim.name = single ? f.name.substr(0, f.name.size() - 4) : "";
				anim.nFrames = h.NumFrames;
				anim.id = h.Id;
				anim.meshes = morph.meshes;
				anim.targets = morph.targets;
				vector <float> envelope = Track(h.ENVELOPE_PTR);
				anim.weight.assign(morph.targets.size(), vector <float> (h.NumFrames, 0));
				for (uint32_t t = 0; t < h.NumTracks; t++)
				{
					int target = (h.TargetId == morph.Id) ? (int)t : (t < remap->second.size() ? remap->second[t] : -1);
					if (target < 0 || target >= (int)morph.targets.size())
						continue;
					vector <float> w = Track(h.TRACKS_PTR + t * sizeof(MPHS_TRACK));
					for (unsigned int k = 0; k < h.NumFrames; k++)
						w[k] = (w[k] / MPHS_WEIGHT_SCALE) * (envelope[k] / MPHS_WEIGHT_SCALE);
					anim.weight[target] = w;
				}
				if (anim.name.empty())
				{
					stringstream ss;
					ss << hex << uppercase << setw(8) << setfill('0') << h.Id;
					anim.name = ss.str();									// Nome provvisorio, sostituito sotto se l'hash viene risolto
					ids.push_back(h.Id);
				}
				anims.push_back(anim);
			}
			p += h.Size;
		}
	}

	// Nomi delle battute di dialogo: hash di <2 lettere minuscole><numero>
	if (!ids.empty())
	{
		map <uint32_t, string> names;
		set <uint32_t> wanted(ids.begin(), ids.end());
		char name[16];
		for (char a = 'a'; a <= 'z'; a++)
			for (char b = 'a'; b <= 'z'; b++)
				for (int n = 0; n < 10000; n++)
				{
					snprintf(name, sizeof(name), "%c%c%d", a, b, n);
					uint32_t h = (uint32_t)GetHashValue(name);
					if (wanted.count(h))
						names[h] = name;
				}
		for (BlendShapeAnimation &anim : anims)
		{
			uint32_t h = (uint32_t)strtoul(anim.name.c_str(), NULL, 16);
			if (anim.name.size() == 8 && names.count(h))
				anim.name = names[h];
		}
	}

	// Nomi univoci e preceduti dal nome del personaggio (es. CARVIER_pa211, KURTIS_CS_10_14)
	set <string> used;
	for (BlendShapeAnimation &anim : anims)
	{
		if (anim.name.compare(0, chrname.size(), chrname) != 0)
			anim.name = chrname + "_" + anim.name;
		string unique = anim.name;
		for (int n = 2; used.count(unique); n++)
			unique = anim.name + "_" + to_string(n);
		anim.name = unique;
		used.insert(unique);
	}
	if (!anims.empty())
		msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Facial animations: " << anims.size() << " for " << morph.name << ".TMT";
	return anims;
}
