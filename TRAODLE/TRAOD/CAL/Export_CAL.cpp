#include "stdafx.h"
#include <set>
#include "Classes.h"
#include "MATH/math.h"
#include "MA/MA_Functions.h"
#include "FBX/FBX_Functions.h"
#include "TRAOD/CHR/CHR_Functions.h"
#include "TRAOD/CAL/CAL_Functions.h"


class CAL_Character {						// Personaggio (CHR) a cui si applicano le animazioni di un CAL
public:
	string name;							// Nome del CHR senza estensione (= gruppo del personaggio e file MA referenziato)
	CHR_Skeleton skeleton;					// Joints, gerarchia e matrici locali di bind
	vector <Joint> joints;					// Joints esportati (per i file FBX)
	vector <int> trackbone;					// Bone di ogni traccia (bones senza BONEFLAG_NOTRACK, nell'ordine del CHR)
	vector <int> mirror;					// Traccia della bone speculare di ogni traccia (BONE_CreateMirrorMap)
};


template <class T> static bool CAL_Get (const vector <uint8_t> &data, size_t offset, T &value)
{
	if (offset + sizeof(T) > data.size())
		return false;
	memcpy(&value, &data[offset], sizeof(T));
	return true;
}


/*------------------------------------------------------------------------------------------------------------------
Legge un CHR e verifica che il numero di bones animate (senza BONEFLAG_NOTRACK) sia nBoneTracks.
Calcola anche la mappa delle bones speculari come BONE_CreateMirrorMap del runtime: il nome e' diviso in token
separati da "_" (NOME_FLAG_LR); se il terzo token (o il secondo se manca) e' "L" o "R" l'ultima lettera viene scambiata.
------------------------------------------------------------------------------------------------------------------*/
static bool CAL_LoadCharacter (string chrfilename, unsigned int nBoneTracks, CAL_Character &character)
{
	ifstream chrfile(chrfilename, std::ios::binary);
	if (!chrfile.is_open())
		return false;
	CHR_HEADER chr_header;
	chrfile.read(reinterpret_cast<char*>(&chr_header), sizeof(chr_header));
	if (!chrfile || chr_header.CHR_MAGIC != 0 || chr_header.HeaderSize != sizeof(CHR_HEADER) || chr_header.nBONES == 0 || chr_header.nBONES > 255)
		return false;										// Non e' un personaggio (es. oggetti animati "NODE")
	vector <CHR_BONE> bones(chr_header.nBONES);
	chrfile.seekg(chr_header.SKELETON_PTR);
	chrfile.read(reinterpret_cast<char*>(bones.data()), sizeof(CHR_BONE) * bones.size());
	if (!chrfile)
		return false;
	vector <string> names;
	character.trackbone.clear();
	for (unsigned int b = 0; b < bones.size(); b++)
		if (!(bones[b].Hierarchy & CHR_BONEFLAG_NOTRACK))
		{
			character.trackbone.push_back(b);
			names.push_back(string(bones[b].Bone_name, strnlen(bones[b].Bone_name, sizeof(bones[b].Bone_name))));
		}
	if (character.trackbone.size() != nBoneTracks)
		return false;

	character.mirror.resize(names.size());
	for (unsigned int t = 0; t < names.size(); t++)
	{
		character.mirror[t] = t;
		vector <string> token;
		size_t start = 0, end;
		while ((end = names[t].find('_', start)) != string::npos)
		{
			if (end > start)
				token.push_back(names[t].substr(start, end - start));
			start = end + 1;
		}
		if (start < names[t].size())
			token.push_back(names[t].substr(start));
		if (token.size() < 2)
			continue;
		const string &lr = (token.size() >= 3) ? token[2] : token[1];
		if (lr != "L" && lr != "R")
			continue;
		string other = names[t];
		other.back() = (lr == "L") ? 'R' : 'L';
		for (unsigned int u = 0; u < names.size(); u++)
			if (names[u] == other)
			{
				character.mirror[t] = u;
				break;
			}
	}

	// Scheletro (stessi nomi dei joints del file MA/FBX del personaggio)
	character.name = chrfilename.substr(0, chrfilename.find(".CHR"));
	FBX_EXPORT FBX;
	MA_EXPORT MA;
	if (!CHR_Read_Skeleton(chrfile, chr_header, character.name, character.skeleton, FBX, MA))
		return false;
	character.joints = FBX.Joint;
	return true;
}


/*------------------------------------------------------------------------------------------------------------------
Sceglie l'angolo euleriano equivalente (x+180, 180-y, z+180 e multipli di 360 gradi) piu' vicino al frame precedente,
per evitare salti nelle curve di rotazione.
------------------------------------------------------------------------------------------------------------------*/
static Vec3 CAL_UnwrapEuler (Vec3 r, const Vec3 &prev)
{
	auto Near = [](float a, float p) { return a + 360.0f * roundf((p - a) / 360.0f); };
	Vec3 c1(Near(r.x, prev.x), Near(r.y, prev.y), Near(r.z, prev.z));
	Vec3 c2(Near(r.x + 180, prev.x), Near(180 - r.y, prev.y), Near(r.z + 180, prev.z));
	float d1 = fabsf(c1.x - prev.x) + fabsf(c1.y - prev.y) + fabsf(c1.z - prev.z);
	float d2 = fabsf(c2.x - prev.x) + fabsf(c2.y - prev.y) + fabsf(c2.z - prev.z);
	return (d2 < d1) ? c2 : c1;
}


static bool CAL_Differs (const vector <float> &v, float value)		// Vero se la curva si discosta dal valore dato
{
	for (float x : v)
		if (fabsf(x - value) > 1e-4f)
			return true;
	return false;
}


/*------------------------------------------------------------------------------------------------------------------
Decodifica un'animazione del CAL in curve per frame dei joints del personaggio (vedi CAL_Struct.h):
- traslazione = traslazione di bind + delta / 4; rotazione = quaternione assoluto (identita' se assente, W = 1 se
  manca); scala assoluta / 255. La matrice locale (vettore riga) viene scomposta in traslazione, rotazione euleriana
  XYZ e scala come i joints del personaggio;
- animazioni APBFLAG_MIRROR: traccia della bone speculare, traslazione X, quaternione X e W negati;
- root motion (traccia della velocita'): accumulato e combinato con la trasformazione del joint radice (HIP).
- filmati (animazione con un attore in un file POS): la traslazione del joint radice e' la posizione nel mondo del POS
  (la traslazione dell'HIP nei CAL dei filmati e' nulla) e la traccia della velocita' non viene usata.
  Il file MA dell'animazione contiene anche l'animazione facciale indicata nel POS (TMS dell'attore).
Vengono scritti solo i gruppi di canali (T, R, S) che si discostano dalla posa di bind.
------------------------------------------------------------------------------------------------------------------*/
static bool CAL_ReadAnimation (const vector <uint8_t> &data, const CAL_ANIMATION &anim, const CAL_Character &character, const CAL_ActorPositions *positions, SkeletalAnimation &out)
{
	CAL_ANIMATION_DATA anim_data;
	if (!CAL_Get(data, anim.DATA_PTR, anim_data))
		return false;
	unsigned int nFrames = anim.nFrames;
	bool mirror = (anim.flags & CAL_APBFLAG_MIRROR) != 0;
	out.character = character.name;
	out.nFrames = nFrames;

	// Canali di una traccia: blocchi nell'ordine dei bit (valori grezzi, vuoto = canale assente)
	auto ReadChannels = [&](uint32_t flags, uint32_t blocks_ptr, vector <vector <float>> &channels) -> bool
	{
		channels.assign(10, vector <float>());
		unsigned int idx = 0;
		for (unsigned int bit = 0; bit < 10; bit++)
			if (flags & (1u << bit))
			{
				uint32_t ptr;
				if (!CAL_Get(data, blocks_ptr + 4 * idx++, ptr) || ptr >= data.size())
					return false;
				channels[bit] = CAL_DecodeTrack(&data[ptr], data.size() - ptr, nFrames);
			}
		return true;
	};

	// Root motion: incrementi per frame accumulati come APB_GetVelocityOffset (posizione al frame f = somma dei frames < f).
	// Viene applicato al joint radice (come nelle scene Maya originali, dove l'HIP contiene anche il root motion) e non al
	// gruppo del personaggio: le mesh skinnate sono figlie del gruppo e verrebbero spostate due volte.
	vector <MATRIX> root(nFrames);								// Matrice (vettore riga) RotZ * traslazione di ogni frame
	uint32_t vflags = positions ? 0 : anim_data.vtrackFlags & (CAL_VTRACK_TX | CAL_VTRACK_TY | CAL_VTRACK_TZ | CAL_VTRACK_RZ);
	if (vflags)
	{
		vector <vector <float>> ch;
		if (!ReadChannels(vflags, anim_data.VTRACK_PTR, ch))
			return false;
		float px = 0, py = 0, pz = 0, angle = 0;
		for (unsigned int f = 0; f < nFrames; f++)
		{
			float c = cosf(angle * (float)M_PI / 180), s = sinf(angle * (float)M_PI / 180);
			root[f].m00 = c;	root[f].m01 = s;
			root[f].m10 = -s;	root[f].m11 = c;
			root[f].m30 = px;	root[f].m31 = py;	root[f].m32 = pz;
			float vx = ch[0].empty() ? 0 : ch[0][f] / CAL_VTSCALE;
			float vy = ch[1].empty() ? 0 : ch[1][f] / CAL_VTSCALE;
			float vz = ch[2].empty() ? 0 : ch[2][f] / CAL_VTSCALE;
			float vr = ch[5].empty() ? 0 : ch[5][f] / CAL_VRSCALE;
			if (mirror)
			{
				vx = -vx;
				vr = -vr;
			}
			angle += vr;
			c = cosf(angle * (float)M_PI / 180);
			s = sinf(angle * (float)M_PI / 180);
			px += vx * c - vy * s;
			py += vx * s + vy * c;
			pz += vz;
		}
	}
	bool root_applied = false;

	for (unsigned int t = 0; t < character.trackbone.size(); t++)
	{
		unsigned int bone = character.trackbone[t];
		unsigned int src = mirror ? character.mirror[t] : t;
		CAL_TRACK track;
		vector <vector <float>> ch;
		if (!CAL_Get(data, anim_data.TRACKS_PTR + sizeof(CAL_TRACK) * src, track) || !ReadChannels(track.flags & CAL_CHANNEL_MASK, track.BLOCKS_PTR, ch))
			return false;
		auto Value = [&](unsigned int bit, unsigned int f, float def, float scale) { return ch[bit].empty() ? def : ch[bit][f] / scale; };
		bool rotation = (track.flags & (CAL_HASRX | CAL_HASRY | CAL_HASRZ | CAL_HASRW)) != 0;

		const MATRIX &bind = character.skeleton.local[bone];
		Vec3 bind_t, bind_r, bind_s;
		mathMatrixDecompose(bind, &bind_t, &bind_r, &bind_s);
		NodeAnimation node;
		node.node = character.skeleton.joint_name[bone];
		node.joint = true;
		vector <float> tx(nFrames), ty(nFrames), tz(nFrames), rx(nFrames), ry(nFrames), rz(nFrames), sx(nFrames), sy(nFrames), sz(nFrames);
		Vec3 prev = bind_r;
		for (unsigned int f = 0; f < nFrames; f++)
		{
			float dx = Value(0, f, 0, CAL_TSCALE), dy = Value(1, f, 0, CAL_TSCALE), dz = Value(2, f, 0, CAL_TSCALE);
			float qx = Value(3, f, 0, CAL_RSCALE), qy = Value(4, f, 0, CAL_RSCALE), qz = Value(5, f, 0, CAL_RSCALE), qw = Value(9, f, 1, CAL_RSCALE);
			float scx = Value(6, f, 1, CAL_SSCALE), scy = Value(7, f, 1, CAL_SSCALE), scz = Value(8, f, 1, CAL_SSCALE);
			if (mirror)
			{
				dx = -dx;
				qx = -qx;
				qw = -qw;
			}
			if (!rotation)
			{
				qx = qy = qz = 0;
				qw = 1;
			}
			float len = sqrtf(qx * qx + qy * qy + qz * qz + qw * qw);
			if (len > 0)
			{
				qx /= len;	qy /= len;	qz /= len;	qw /= len;
			}
			else
				qw = 1;

			// Matrice locale a vettore riga: righe 0-2 = scala * rotazione (trasposta della matrice "colonna" del quaternione)
			MATRIX m;
			m.m00 = scx * (1 - 2 * (qy * qy + qz * qz));	m.m01 = scx * 2 * (qx * qy + qz * qw);		m.m02 = scx * 2 * (qx * qz - qy * qw);
			m.m10 = scy * 2 * (qx * qy - qz * qw);		m.m11 = scy * (1 - 2 * (qx * qx + qz * qz));	m.m12 = scy * 2 * (qy * qz + qx * qw);
			m.m20 = scz * 2 * (qx * qz + qy * qw);		m.m21 = scz * 2 * (qy * qz - qx * qw);		m.m22 = scz * (1 - 2 * (qx * qx + qy * qy));
			m.m30 = bind.m30 + dx;	m.m31 = bind.m31 + dy;	m.m32 = bind.m32 + dz;
			if (positions && !positions->frames.empty() && character.skeleton.parent[bone] < 0)	// Filmati: posizione del POS
			{
				const POS_FRAME &p = positions->frames[min<size_t>(f, positions->frames.size() - 1)];
				m.m30 = p.x;	m.m31 = p.y;	m.m32 = p.z;
			}
			if (vflags && character.skeleton.parent[bone] < 0)		// Root motion sul joint radice
				m = mathMulMatrices(m, root[f]);
			Vec3 tr, rot, sc;
			mathMatrixDecompose(m, &tr, &rot, &sc);
			rot = CAL_UnwrapEuler(rot, prev);
			prev = rot;
			tx[f] = tr.x;	ty[f] = tr.y;	tz[f] = tr.z;
			rx[f] = rot.x;	ry[f] = rot.y;	rz[f] = rot.z;
			sx[f] = sc.x;	sy[f] = sc.y;	sz[f] = sc.z;
		}
		if (CAL_Differs(tx, bind_t.x) || CAL_Differs(ty, bind_t.y) || CAL_Differs(tz, bind_t.z))
		{
			node.tX = tx;	node.tY = ty;	node.tZ = tz;
		}
		if (CAL_Differs(rx, bind_r.x) || CAL_Differs(ry, bind_r.y) || CAL_Differs(rz, bind_r.z))
		{
			node.rX = rx;	node.rY = ry;	node.rZ = rz;
		}
		if (CAL_Differs(sx, bind_s.x) || CAL_Differs(sy, bind_s.y) || CAL_Differs(sz, bind_s.z))
		{
			node.sX = sx;	node.sY = sy;	node.sZ = sz;
		}
		if (character.skeleton.parent[bone] < 0)
			root_applied = true;
		if (!node.tX.empty() || !node.rX.empty() || !node.sX.empty())
			out.nodes.push_back(node);
	}

	if (vflags && !root_applied)								// Joint radice senza traccia: root motion sul gruppo
	{
		NodeAnimation group;
		group.node = character.name;
		group.joint = false;
		for (unsigned int f = 0; f < nFrames; f++)
		{
			group.tX.push_back(root[f].m30);	group.tY.push_back(root[f].m31);	group.tZ.push_back(root[f].m32);
			group.rX.push_back(0);	group.rY.push_back(0);	group.rZ.push_back(atan2f(root[f].m01, root[f].m00) * 180 / (float)M_PI);
		}
		out.nodes.push_back(group);
	}
	return true;
}


class CAL_Facial {							// Blend shapes del volto del personaggio e sequenze facciali che li animano
public:
	string tmt;								// TMT applicato al volto (vuoto se nessuno)
	vector <Mesh> meshes;					// Mesh con il blend shape (target del TMT applicato)
	vector <BlendShapeAnimation> anims;		// Sequenze facciali (file TMS e ".3") del TMT applicato
};


/*------------------------------------------------------------------------------------------------------------------
Blend shapes e animazioni facciali del personaggio come in Export_CHR (CHR_Read_Morphs, CHR_Read_Meshes,
CHR_Read_FacialAnimations), usando il TMT preferred se presente (filmati: TMT con il nome del filmato). Le mesh del CHR
vengono rilette per conoscere i nomi delle mesh con il blend shape e i loro target (senza messaggi: sono gia' stati
scritti da Export_CHR). Va chiamata con la cartella corrente \NOMELIVELLO.
------------------------------------------------------------------------------------------------------------------*/
static CAL_Facial CAL_ReadFacial (const CAL_Character &character, string preferred)
{
	CAL_Facial facial;
	ifstream chrfile(character.name + ".CHR", std::ios::binary);
	CHR_HEADER chr_header;
	if (!chrfile.read(reinterpret_cast<char*>(&chr_header), sizeof(chr_header)))
		return facial;
	msg::Quiet() = true;
	map <uint32_t, CHR_Morph> morphs = CHR_Read_Morphs(character.name, preferred);
	FBX_EXPORT FBX;
	MA_EXPORT MA;
	if (!morphs.empty() && CHR_Read_Meshes(chrfile, chr_header, character.name, character.skeleton, morphs, FBX, MA))
	{
		for (const Mesh &mesh : FBX.Geometry)
			if (!mesh.BlendShape.empty())
				facial.meshes.push_back(mesh);
		for (const pair <const uint32_t, CHR_Morph> &m : morphs)
			if (m.second.used)
			{
				if (facial.tmt.empty() || m.second.name == preferred)
					facial.tmt = m.second.name;
				vector <BlendShapeAnimation> anims = CHR_Read_FacialAnimations(m.second, character.name);
				facial.anims.insert(facial.anims.end(), anims.begin(), anims.end());
			}
	}
	msg::Quiet() = false;
	return facial;
}


/*------------------------------------------------------------------------------------------------------------------
Sequenze facciali avviate da un'animazione: chiavi TRACK_KEY di tipo FIRE_MORPH_TRIGGER (frame, hash della sequenza).
------------------------------------------------------------------------------------------------------------------*/
static vector <pair <unsigned int, uint32_t>> CAL_MorphTriggers (const vector <uint8_t> &data, const CAL_ANIMATION &anim)
{
	vector <pair <unsigned int, uint32_t>> triggers;
	for (uint32_t k = 0; k < anim.nKeys; k++)
	{
		CAL_TRACK_KEY key;
		if (!CAL_Get(data, anim.KEYS_PTR + sizeof(CAL_TRACK_KEY) * k, key))
			break;
		if (key.type == CAL_KEY_FIRE_MORPH_TRIGGER)
			triggers.push_back(make_pair(key.time & 0xFFFFFF, key.data));
	}
	return triggers;
}


/*------------------------------------------------------------------------------------------------------------------
Esporta le animazioni scheletriche di un file CAL nella cartella \NOMELIVELLO\Animations\<NOMECAL>: per ogni animazione
un file MA che carica il personaggio come riferimento
(..\..\Characters\<CHR>.MA, esportato da Export_CHR) e ne anima i joints; inoltre un unico file <NOMECAL>.FBX con lo
scheletro del personaggio e tutte le animazioni come takes.
Struttura dei file di AoD:
- livelli: <PERSONAGGIO>.CAL contiene le animazioni di gioco, comuni a tutti i modelli del personaggio (LARA.CAL per
  LARAD, LARAC1, LARAC2, ...); <PERSONAGGIO>_DLG.CAL le animazioni dei dialoghi, che avviano le sequenze facciali dei file
  ".3" (TRACK_KEY FIRE_MORPH_TRIGGER) sui blend shapes del TMT base (<PERSONAGGIO>.TMT);
- filmati (livelli CS_* e filmati IG_* dentro i livelli): <PERSONAGGIO>_<FILMATO>.CAL / .TMT / .TMS sono l'animazione,
  i blend shapes del volto e l'animazione facciale dell'attore; <FILMATO>.POS contiene le posizioni dell'HIP di tutti gli
  attori (vedi POS_Struct.h). Il TMT del filmato sostituisce i target del blend shape del personaggio nel file MA.
Il personaggio (CHR di riferimento) viene cercato tra i CHR del livello: stesso nome del CAL, nome senza i suffissi _DLG /
_CS_* / _IG_* (es. LARA_IG_1_24 -> LARA), CHR il cui nome inizia con quel nome (es. LARA -> LARAD). Viene scelto il primo
con lo stesso numero di bones animate delle tracce del CAL; in mancanza, un CHR che contiene quel nome (es. COP_A -> CS_COP_A).
Le tracce delle mesh agganciate (ParentTracks), gli eventi diversi da FIRE_MORPH_TRIGGER e gli FXNODE non vengono esportati.
------------------------------------------------------------------------------------------------------------------*/
bool Export_CAL (string filename)
{
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Reading " << filename;
	SetCurrentDirectory(AOD_IO.folder_level_lpwstr);			// \NOMELIVELLO
	vector <uint8_t> data;
	{
		ifstream calfile(filename, std::ios::binary);
		if (!calfile.is_open())
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " not found.";
			return false;
		}
		data.assign(istreambuf_iterator<char>(calfile), istreambuf_iterator<char>());
	}
	CAL_HEADER header;
	if (!CAL_Get(data, 0, header) || header.Version != CAL_GAME_ANIMATION_VERSION || header.nAnims == 0 || header.nAnims > 100000 ||
		(size_t)header.ANIM_LIST_PTR + 4 * (size_t)header.nAnims > data.size())
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " is not a valid CAL file.";
		return false;
	}
	vector <CAL_ANIMATION> anims(header.nAnims);
	for (unsigned int a = 0; a < header.nAnims; a++)
	{
		uint32_t ptr;
		if (!CAL_Get(data, header.ANIM_LIST_PTR + 4 * a, ptr) || !CAL_Get(data, ptr, anims[a]))
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << ": animation " << a << " out of file.";
			return false;
		}
	}
	unsigned int nBoneTracks = anims[0].nTracks - anims[0].nParentTracks;
	map <uint32_t, CAL_ActorPositions> positions = CAL_Read_Positions();	// Filmati: posizioni degli attori (file POS)

	// Ricerca del personaggio
	string calname = filename.substr(0, filename.find(".CAL"));
	string base = calname;
	for (const char *suffix : {"_DLG", "_CS_", "_IG_"})
	{
		size_t pos = base.find(suffix);
		if (pos != string::npos && pos > 0)
			base = base.substr(0, pos);
	}
	vector <string> candidates = {calname + ".CHR", base + ".CHR"};
	for (unsigned int i = 0; i < AOD_IO.gmxfiles.size(); i++)
		if (AOD_IO.gmxfiles[i].type == AoDFileType::CHR && AOD_IO.gmxfiles[i].name.compare(0, base.size(), base) == 0)
			candidates.push_back(AOD_IO.gmxfiles[i].name);
	for (unsigned int i = 0; i < AOD_IO.gmxfiles.size(); i++)		// Poi i CHR che contengono il nome (es. COP_A -> CS_COP_A)
		if (AOD_IO.gmxfiles[i].type == AoDFileType::CHR && AOD_IO.gmxfiles[i].name.find(base) != string::npos)
			candidates.push_back(AOD_IO.gmxfiles[i].name);
	CAL_Character character;
	bool found = false;
	set <string> tried;
	for (const string &c : candidates)
	{
		if (tried.count(c))
			continue;
		tried.insert(c);
		if (CAL_LoadCharacter(c, nBoneTracks, character))
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << "No character (CHR) with " << nBoneTracks << " animated bones found for " << filename << ". Animations not exported.";
		return true;
	}
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Character: " << character.name << ".CHR, animations: " << header.nAnims;

	// Animazioni facciali (blend shapes del volto), aggiunte ai file MA delle animazioni:
	// - filmati (<PERSONAGGIO>_<FILMATO>.CAL): sequenza del TMS indicato nel POS o con il nome del CAL, sui target del TMT
	//   con il nome del CAL (TMT del filmato) se presente;
	// - altre animazioni (es. dialoghi *_DLG): sequenze avviate dalle chiavi FIRE_MORPH_TRIGGER (file ".3"), sui target del
	//   TMT base del personaggio.
	CAL_Facial facial_base, facial_cutscene;
	bool cutscene = false;
	for (const auto &f : AOD_IO.gmxfiles)
		if ((f.type == AoDFileType::TMS || f.type == AoDFileType::TMT) && f.name.substr(0, f.name.find('.')) == calname)
			cutscene = true;
	for (const CAL_ANIMATION &anim : anims)
	{
		map <uint32_t, CAL_ActorPositions>::const_iterator pos = positions.find(anim.animID);
		if (pos != positions.end() && !pos->second.tms.empty())
			cutscene = true;
	}
	if (cutscene)
		facial_cutscene = CAL_ReadFacial(character, calname);
	for (const CAL_ANIMATION &anim : anims)
		if (!CAL_MorphTriggers(data, anim).empty())
		{
			facial_base = CAL_ReadFacial(character, "");
			break;
		}

	// Cartella \NOMELIVELLO\Animations\NOMECAL
	string folder = AOD_IO.folder_animations + calname;
	CreateDirectoryA(AOD_IO.folder_animations.c_str(), NULL);
	CreateDirectoryA(folder.c_str(), NULL);
	if (!SetCurrentDirectoryA(folder.c_str()))
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Unable to access folder " << folder << ". Animations not exported.";
		return false;
	}

	Transform group;
	group.name = character.name;
	group.translate_flag = group.rotate_flag = group.scale_flag = true;		// Proprieta' Lcl necessarie per animare il gruppo nell'FBX
	FBX_EXPORT FBX;												// Scheletro (scritto una volta) + tutte le animazioni come takes
	FBX.Group.push_back(group);
	FBX.Joint = character.joints;
	set <string> used_names;
	unsigned int exported = 0;
	bool ok = true;
	for (unsigned int a = 0; a < anims.size(); a++)
	{
		const CAL_ANIMATION &anim = anims[a];
		string name(anim.name, strnlen(anim.name, sizeof(anim.name)));
		for (char &c : name)										// Nome valido per file e nodi
			if (!isalnum((unsigned char)c) && c != '_')
				c = '_';
		if (name.empty())
			name = calname + "_" + to_string(a);
		if (used_names.count(name))
			name += "_" + to_string(a);
		used_names.insert(name);

		if (anim.nFrames == 0 || anim.nTracks - anim.nParentTracks != nBoneTracks)
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << name << ": " << anim.nFrames << " frames, " << anim.nTracks - anim.nParentTracks << " bone tracks. Skipped.";
			continue;
		}
		SkeletalAnimation skanim;
		skanim.name = name;
		auto pos = positions.find(anim.animID);					// Filmati: posizioni del joint radice dal file POS
		const CAL_ActorPositions *actor = (pos != positions.end()) ? &pos->second : nullptr;
		if (actor && actor->frames.size() != anim.nFrames)
			msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << name << ": " << anim.nFrames << " frames, " << actor->frames.size() << " positions in " << actor->posfile << ".";
		if (actor)
			msg(msg::TGT::FILE, msg::TYP::LOG) << name << ": root positions from " << actor->posfile << " (" << actor->actor << ")";
		if (!CAL_ReadAnimation(data, anim, character, actor, skanim))
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << name << ": invalid animation data. Skipped.";
			ok = false;
			continue;
		}

		BlendShapeAnimation face;									// Animazione facciale (pesi per frame dell'animazione)
		const vector <Mesh> *face_targets = nullptr;				// Target del TMT del filmato (sostituiscono quelli del personaggio)
		string tms = (actor && !actor->tms.empty()) ? actor->tms.substr(0, actor->tms.rfind('.')) : calname;
		for (const BlendShapeAnimation &b : facial_cutscene.anims)	// Filmati: sequenza dell'attore
			if (b.name == tms || b.name == character.name + "_" + tms)
			{
				face = b;
				if (facial_cutscene.tmt == calname)					// TMT del filmato applicato
					face_targets = &facial_cutscene.meshes;
				msg(msg::TGT::FILE, msg::TYP::LOG) << name << ": facial animation " << tms << ".TMS, blend shapes " << facial_cutscene.tmt << ".TMT";
				break;
			}
		if (face.meshes.empty() && actor && !actor->tms.empty())
			msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << name << ": facial animation " << actor->tms << " not found.";
		if (face.meshes.empty())									// Sequenze avviate dalle chiavi FIRE_MORPH_TRIGGER
			for (const pair <unsigned int, uint32_t> &trigger : CAL_MorphTriggers(data, anim))
			{
				const BlendShapeAnimation *seq = nullptr;
				for (const BlendShapeAnimation &b : facial_base.anims)
					if (b.id == trigger.second)
						seq = &b;
				if (!seq)
				{
					msg(msg::TGT::FILE, msg::TYP::LOG) << name << ": facial sequence " << hex << trigger.second << dec << " not in this level.";
					continue;
				}
				if (face.meshes.empty())
				{
					face.name = seq->name;
					face.meshes = seq->meshes;
					face.targets = seq->targets;
					face.weight.assign(seq->targets.size(), vector <float>());
				}
				face.nFrames = max(face.nFrames, max(anim.nFrames, trigger.first + seq->nFrames));
				for (unsigned int t = 0; t < face.weight.size() && t < seq->weight.size(); t++)
				{
					face.weight[t].resize(face.nFrames, 0);
					for (unsigned int k = 0; k < seq->weight[t].size(); k++)		// Una sequenza successiva sostituisce la precedente
						face.weight[t][trigger.first + k] = seq->weight[t][k];
				}
				msg(msg::TGT::FILE, msg::TYP::LOG) << name << ": facial sequence " << seq->name << " at frame " << trigger.first;
			}
		for (vector <float> &w : face.weight)
			w.resize(face.nFrames, w.empty() ? 0 : w.back());
		MA_Write_SkeletalAnimationFile(skanim, "../../Characters/" + character.name + ".MA", face.meshes.empty() ? nullptr : &face, face_targets);	// Stesso nome scritto da MA_Export
		FBX.SkeletalAnimation.push_back(move(skanim));
		exported++;
	}
	if (exported > 0)
		FBX_Export(calname, FBX);								// Un solo FBX: scheletro + una take per animazione
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Output: " << exported << " animations in " << folder;
	return ok;
}
