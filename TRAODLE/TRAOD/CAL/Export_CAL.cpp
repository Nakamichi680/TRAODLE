#include "stdafx.h"
#include <set>
#include "Classes.h"
#include "MATH/math.h"
#include "MA/MA_Functions.h"
#include "FBX/FBX_Functions.h"
#include "TRAOD/CHR/CHR_Functions.h"
#include "TRAOD/CAL/CAL_Functions.h"
#include "TRAOD/CSS/CSS_Functions.h"
#include "hash_Functions.h"
#include "AOD_Names.h"


class CAL_Character {						// Personaggio (CHR) a cui si applicano le animazioni di un CAL
public:
	string name;							// Nome del CHR senza estensione (= gruppo del personaggio e file MA referenziato)
	CHR_Skeleton skeleton;					// Joints, gerarchia e matrici locali di bind
	vector <Joint> joints;					// Joints esportati (per i file FBX)
	vector <int> trackbone;					// Bone di ogni traccia (bones senza BONEFLAG_NOTRACK, nell'ordine del CHR)
	vector <int> mirror;					// Traccia della bone speculare di ogni traccia (BONE_CreateMirrorMap)
	vector <Transform> groups;				// Gruppi degli oggetti agganciati (per i file FBX)
	map <uint32_t, vector <string>> mesh_names;	// Mesh esportate per ID delle MESH2 (chiavi di visibilita')
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
	msg::Quiet() = true;
	bool attached = CHR_Read_Attachments(chrfile, chr_header, character.name, character.skeleton, FBX, MA);	// Oggetti agganciati
	if (attached)												// Nomi delle mesh esportate (come Export_CHR)
	{
		FBX_EXPORT FBX_meshes;
		MA_EXPORT MA_meshes;
		map <uint32_t, CHR_Morph> no_morphs;
		CHR_Read_Meshes(chrfile, chr_header, character.name, character.skeleton, no_morphs, FBX_meshes, MA_meshes, &character.mesh_names);
	}
	msg::Quiet() = false;
	if (!attached)
		return false;
	character.joints = FBX.Joint;
	character.groups = FBX.Group;
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


// Canali di una traccia: blocchi nell'ordine dei bit (valori grezzi, vuoto = canale assente)
static bool CAL_ReadChannels (const vector <uint8_t> &data, uint32_t flags, uint32_t blocks_ptr, unsigned int nFrames, vector <vector <float>> &channels)
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
}


static string CAL_AnimationName (const CAL_ANIMATION &anim, string calname, unsigned int a, set <string> &used_names)	// Nome valido per file e nodi
{
	string name(anim.name, strnlen(anim.name, sizeof(anim.name)));
	for (char &c : name)
		if (!isalnum((unsigned char)c) && c != '_')
			c = '_';
	if (name.empty())
		name = calname + "_" + to_string(a);
	if (used_names.count(name))
		name += "_" + to_string(a);
	used_names.insert(name);
	return name;
}


static bool CAL_Differs (const vector <float> &v, float value)		// Vero se la curva si discosta dal valore dato
{
	for (float x : v)
		if (fabsf(x - value) > 1e-4f)
			return true;
	return false;
}


/*------------------------------------------------------------------------------------------------------------------
Decodifica una traccia di un oggetto (nodi dei CHR "NODE" e oggetti agganciati ai personaggi, AT_GetSetFrameData del
runtime): traslazione assoluta (/ ANIM_SETTSCALE), quaternione (/ 16383, W = 1 se manca), scala (/ 255), canali assenti
nulli / identita' / 1. Riempie i gruppi di canali (T, R, S) di node che si discostano dalla posa pose.
------------------------------------------------------------------------------------------------------------------*/
static bool CAL_ReadSetTrack (const vector <uint8_t> &data, uint32_t track_offset, unsigned int nFrames, const Transform &pose, NodeAnimation &node)
{
	CAL_TRACK track;
	vector <vector <float>> ch;
	if (!CAL_Get(data, track_offset, track) || !CAL_ReadChannels(data, track.flags & CAL_CHANNEL_MASK, track.BLOCKS_PTR, nFrames, ch))
		return false;
	auto Value = [&](unsigned int bit, unsigned int f, float def, float scale) { return ch[bit].empty() ? def : ch[bit][f] / scale; };
	bool rotation = (track.flags & (CAL_HASRX | CAL_HASRY | CAL_HASRZ | CAL_HASRW)) != 0;
	vector <float> tx(nFrames), ty(nFrames), tz(nFrames), rx(nFrames), ry(nFrames), rz(nFrames), sx(nFrames), sy(nFrames), sz(nFrames);
	Vec3 prev(pose.rX, pose.rY, pose.rZ);
	for (unsigned int f = 0; f < nFrames; f++)
	{
		float qx = Value(3, f, 0, CAL_SETRSCALE), qy = Value(4, f, 0, CAL_SETRSCALE), qz = Value(5, f, 0, CAL_SETRSCALE), qw = Value(9, f, 1, CAL_SETRSCALE);
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
		MATRIX m;												// Rotazione a vettore riga (come CAL_ReadAnimation)
		m.m00 = 1 - 2 * (qy * qy + qz * qz);	m.m01 = 2 * (qx * qy + qz * qw);		m.m02 = 2 * (qx * qz - qy * qw);
		m.m10 = 2 * (qx * qy - qz * qw);		m.m11 = 1 - 2 * (qx * qx + qz * qz);	m.m12 = 2 * (qy * qz + qx * qw);
		m.m20 = 2 * (qx * qz + qy * qw);		m.m21 = 2 * (qy * qz - qx * qw);		m.m22 = 1 - 2 * (qx * qx + qy * qy);
		Vec3 tr, rot, sc;
		mathMatrixDecompose(m, &tr, &rot, &sc);
		rot = CAL_UnwrapEuler(rot, prev);
		prev = rot;
		tx[f] = Value(0, f, 0, CAL_SETTSCALE);	ty[f] = Value(1, f, 0, CAL_SETTSCALE);	tz[f] = Value(2, f, 0, CAL_SETTSCALE);
		rx[f] = rot.x;	ry[f] = rot.y;	rz[f] = rot.z;
		sx[f] = Value(6, f, 1, CAL_SETSSCALE);	sy[f] = Value(7, f, 1, CAL_SETSSCALE);	sz[f] = Value(8, f, 1, CAL_SETSSCALE);
	}
	if (CAL_Differs(tx, pose.tX) || CAL_Differs(ty, pose.tY) || CAL_Differs(tz, pose.tZ))
	{
		node.tX = tx;	node.tY = ty;	node.tZ = tz;
	}
	if (CAL_Differs(rx, pose.rX) || CAL_Differs(ry, pose.rY) || CAL_Differs(rz, pose.rZ))
	{
		node.rX = rx;	node.rY = ry;	node.rZ = rz;
	}
	if (CAL_Differs(sx, pose.sX) || CAL_Differs(sy, pose.sY) || CAL_Differs(sz, pose.sZ))
	{
		node.sX = sx;	node.sY = sy;	node.sZ = sz;
	}
	return true;
}


/*------------------------------------------------------------------------------------------------------------------
Decodifica un'animazione del CAL in curve per frame dei joints del personaggio (vedi CAL_Struct.h):
- traslazione = traslazione di bind + delta / 4; rotazione = quaternione assoluto (identita' se assente, W = 1 se
  manca); scala assoluta / 255. La matrice locale (vettore riga) viene scomposta in traslazione, rotazione euleriana
  XYZ e scala come i joints del personaggio;
- animazioni APBFLAG_MIRROR: traccia della bone speculare, traslazione X, quaternione X e W negati;
- root motion (traccia della velocita'): accumulato e combinato con la trasformazione del joint radice (HIP);
- oggetti agganciati (armi, fondine, ...): ParentTracks e chiavi di visibilita' sui gruppi dei nodi agganciati.
- filmati (animazione con un attore in un file POS): la traslazione del joint radice e' la posizione nel mondo del POS
  (la traslazione dell'HIP nei CAL dei filmati e' nulla) e la traccia della velocita' non viene usata.
  Il file MA dell'animazione contiene anche l'animazione facciale indicata nel POS (TMS dell'attore).
Vengono scritti solo i gruppi di canali (T, R, S) che si discostano dalla posa di bind.
------------------------------------------------------------------------------------------------------------------*/
static bool CAL_ReadAnimation (const vector <uint8_t> &data, const CAL_ANIMATION &anim, const CAL_Character &character, const CAL_ActorPositions *positions, SkeletalAnimation &out,
							   const vector <string> &hidden = vector <string> ())
{
	CAL_ANIMATION_DATA anim_data;
	if (!CAL_Get(data, anim.DATA_PTR, anim_data))
		return false;
	unsigned int nFrames = anim.nFrames;
	bool mirror = (anim.flags & CAL_APBFLAG_MIRROR) != 0;
	out.character = character.name;
	out.nFrames = nFrames;

	auto ReadChannels = [&](uint32_t flags, uint32_t blocks_ptr, vector <vector <float>> &channels) { return CAL_ReadChannels(data, flags, blocks_ptr, nFrames, channels); };

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

	// Oggetti agganciati (CHR_Read_Attachments): le ParentTracks animano i nodi con lo stesso hash (traccia ParentIdx[k]).
	// Le chiavi MAKE_VISIBLE / MAKE_INVISIBLE cambiano la visibilita' (solo file MA) dei nodi agganciati o delle mesh del
	// personaggio con quell'ID (es. la pistola in mano che compare quando quella nella fondina sparisce)
	const vector <CHR_Attachment> &attachments = character.skeleton.attachments;
	{
		map <uint32_t, unsigned int> attachment_of_hash;
		for (unsigned int i = 0; i < attachments.size(); i++)
			attachment_of_hash[attachments[i].hash] = i;
		map <unsigned int, NodeAnimation> animated;
		for (uint32_t k = 0; k < anim.nParentTracks; k++)
		{
			uint32_t hash = 0, index = 0;
			if (!CAL_Get(data, anim.PARENTTRACKS_PTR + 4 * k, hash) || !CAL_Get(data, anim.PARENTIDX_PTR + 4 * k, index))
				break;
			map <uint32_t, unsigned int>::const_iterator a = attachment_of_hash.find(hash);
			if (a == attachment_of_hash.end() || index >= anim.nTracks)
				continue;												// Mesh non presente in questo personaggio
			NodeAnimation node;
			node.node = attachments[a->second].group;
			node.joint = false;
			if (!CAL_ReadSetTrack(data, anim_data.TRACKS_PTR + sizeof(CAL_TRACK) * index, nFrames, attachments[a->second].transform, node))
				return false;
			animated[a->second] = node;
		}
		map <string, vector <pair <unsigned int, float>>> visibility;		// Nodo -> chiavi (frame, valore)
		for (const string &item : hidden)							// Filmati: oggetti nascosti all'avvio (file <FILMATO>.TXT), prima delle chiavi
		{
			uint32_t h = (uint32_t)GetHashValue(item.c_str());
			map <uint32_t, unsigned int>::const_iterator a = attachment_of_hash.find(h);
			map <uint32_t, vector <string>>::const_iterator m = character.mesh_names.find(h);
			if (a != attachment_of_hash.end())
				visibility[attachments[a->second].group].push_back(make_pair(0u, 0.0f));
			else if (m != character.mesh_names.end())
				for (const string &mesh : m->second)
					visibility[mesh].push_back(make_pair(0u, 0.0f));
			else
				msg(msg::TGT::FILE, msg::TYP::WARN) << out.name << ": cutscene switch " << item << " not found in " << character.name << ".CHR.";
		}
		for (uint32_t k = 0; k < anim.nKeys; k++)
		{
			CAL_TRACK_KEY key;
			if (!CAL_Get(data, anim.KEYS_PTR + sizeof(CAL_TRACK_KEY) * k, key))
				break;
			if (key.type != CAL_KEY_MAKE_VISIBLE && key.type != CAL_KEY_MAKE_INVISIBLE)
				continue;
			pair <unsigned int, float> value(key.time & 0xFFFF, key.type == CAL_KEY_MAKE_VISIBLE ? 1.0f : 0.0f);
			map <uint32_t, unsigned int>::const_iterator a = attachment_of_hash.find(key.data);
			map <uint32_t, vector <string>>::const_iterator m = character.mesh_names.find(key.data);
			if (a != attachment_of_hash.end())
				visibility[attachments[a->second].group].push_back(value);
			else if (m != character.mesh_names.end())
				for (const string &mesh : m->second)
					visibility[mesh].push_back(value);
		}
		map <string, NodeAnimation> nodes;
		for (pair <const unsigned int, NodeAnimation> &n : animated)
			nodes[n.second.node] = n.second;
		for (pair <const string, vector <pair <unsigned int, float>>> &v : visibility)
		{
			stable_sort(v.second.begin(), v.second.end(), [](const pair <unsigned int, float> &a, const pair <unsigned int, float> &b) { return a.first < b.first; });
			NodeAnimation &node = nodes[v.first];
			node.node = v.first;
			node.joint = false;
			node.vis.assign(nFrames, 1);
			for (const pair <unsigned int, float> &key : v.second)
				for (unsigned int f = key.first; f < nFrames; f++)
					node.vis[f] = key.second;
		}
		for (pair <const string, NodeAnimation> &n : nodes)
			if (!n.second.tX.empty() || !n.second.rX.empty() || !n.second.sX.empty() || !n.second.vis.empty())
				out.nodes.push_back(n.second);
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
			triggers.push_back(make_pair(key.time & 0xFFFF, key.data));
	}
	return triggers;
}


/*------------------------------------------------------------------------------------------------------------------
Esporta le animazioni di un oggetto animato (CHR "NODE", vedi CHR_Struct.h e CHR_Nodes.cpp) nella cartella
\NOMELIVELLO\Animations\<NOMECAL>: un file MA per animazione che referenzia il modello dell'oggetto
(..\..\Characters\<CHR>.MA, esportato da Export_CHR_Node) e un unico <NOMECAL>.FBX con il modello e tutte le
animazioni come takes. La traccia i anima il nodo i: traslazione assoluta (/ ANIM_SETTSCALE), quaternione (/ 16383,
W = 1 se manca), scala (/ 255); i pivot e il rotate axis dei nodi sono attributi fissi del modello. Vengono scritti solo
i gruppi di canali (T, R, S) che si discostano dalla posa del modello (posa dell'oggetto nel livello).
Filmati: se un file XXX ("PPOS", vedi XXX_Struct.h) contiene l'animazione (stesso animID), il gruppo radice dell'oggetto
viene spostato per ogni frame nella posizione nel mondo indicata, con la visibilita' (solo file MA).
------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------
Nomi delle animazioni di tutti i CAL del livello (animID -> nome), letti una volta: servono per i FIRE_ANIM_TRIGGER, che
avviano animazioni di altri CAL (es. GENERIC_FEMALE_NOD nei dialoghi).
------------------------------------------------------------------------------------------------------------------*/
static const map <uint32_t, string> &CAL_LevelAnimationNames ()
{
	static map <uint32_t, string> names;
	static bool loaded = false;
	if (loaded)
		return names;
	loaded = true;
	for (const auto &f : AOD_IO.gmxfiles)
	{
		if (f.type != AoDFileType::CAL)
			continue;
		ifstream calfile(AOD_IO.folder_level + f.name, std::ios::binary);
		vector <uint8_t> data((istreambuf_iterator<char>(calfile)), istreambuf_iterator<char>());
		CAL_HEADER header;
		if (!CAL_Get(data, 0, header) || header.Version != CAL_GAME_ANIMATION_VERSION || header.nAnims > 100000)
			continue;
		for (unsigned int a = 0; a < header.nAnims; a++)
		{
			uint32_t ptr;
			CAL_ANIMATION anim;
			if (CAL_Get(data, header.ANIM_LIST_PTR + 4 * a, ptr) && CAL_Get(data, ptr, anim))
				names.insert(make_pair(anim.animID, string(anim.name, strnlen(anim.name, sizeof(anim.name)))));
		}
	}
	return names;
}


/*------------------------------------------------------------------------------------------------------------------
Nomi dei nodi e delle mesh che le chiavi MAKE_VISIBLE / INVISIBLE possono nominare anche fuori dal CHR del CAL (hash ->
nome): i nomi dei nodi agganciati e delle mesh "cloth" cercati dal codice del gioco ([SRC] stringhe di TRAOD_P4.exe e
Debug.elf, map_charnode.c) e le stringhe (identificatori) di tutti i CHR del livello (nodi agganciati degli altri
modelli dello stesso personaggio, es. LARAC1 per LARA.CAL applicato a LARAD). Hash del nome e del nome in maiuscolo.
------------------------------------------------------------------------------------------------------------------*/
static const map <uint32_t, string> &CAL_LevelNodeNames ()
{
	static map <uint32_t, string> names;
	static bool loaded = false;
	if (loaded)
		return names;
	loaded = true;
	static const char *game_names[] = {"GUNHOLSTER_RIGHT", "LARA_K2_IMPACTOR_HOLSTER", "LARA_SIGHT_K2_IMPACTOR_HOLSTER", "LARA_RIGG09_HOLSTER",
		"LARA_MV9_HOLSTER", "LARA_SILENCER_MV9_HOLSTER", "LARA_SIGHT_MV9_HOLSTER", "LARA_DESERT_RANGER_HOLSTER", "LARA_SIGHT_DESERT_RANGER_HOLSTER",
		"LARA_DART_SS_HOLSTER", "LARA_SIGHT_DART_SS_HOLSTER", "LARA_SCORPION_X_HOLSTER", "LARA_SILENCER_SCORPION_X_HOLSTER", "GUNHOLSTER_LEFT",
		"SHOTGUN_BACKPACK", "LARA_VIPER_SMG_HOLSTER", "LARA_SIGHT_VIPER_SMG_HOLSTER", "LARA_MAGVEGA_HOLSTER", "LARA_SILENCER_MAGVEGA_HOLSTER",
		"LARA_SIGHT_MAGVEGA_HOLSTER", "LARA_GLASSES", "LARA_REBREATHER", "LARA_DIVINGMASK", "LARA_RESPIRATOR", "GUNHAND_LEFT", "LARA_CROWBAR",
		"SHOTGUN", "SHOTGUN_PUMP", "GUNHAND_RIGHT", "LARA_SHARD", "LARA_K2_IMPACTOR", "LARA_SIGHT_K2_IMPACTOR", "LARA_RIGG09", "LARA_MV10",
		"LARA_SILENCER_MV9", "LARA_SIGHT_MV9", "LARA_DESERT_RANGER", "LARA_SIGHT_DESERT_RANGER", "LARA_DART_SS", "LARA_SIGHT_DART_SS",
		"LARA_SCORPION_X", "LARA_SILENCER_SCORPION_X", "LARA_VIPER_SMG", "LARA_SIGHT_VIPER_SMG", "LARA_MAGVEGA", "LARA_SILENCER_MAGVEGA",
		"LARA_SIGHT_MAGVEGA", "LARA_HAMMER", "HARPOON_GUN", "HARPOON_GUN_BACKPACK", "LARA_SCORPION_X_LEFT", "LARA_SILENCER_SCORPION_X_LEFT",
		"LARA_SCORPION_X_LEFTHOLSTER", "LARA_SILENCER_SCORPION_X_LEFTHOLSTER", "HIP_CHATKA", "PISTOL_HAND", "GLAVE_SPIKE", "GLAVE", "SHARD",
		"LASER", "SIGHTS", "SILENCER", "BOAZFLY_CLOTH_H", "BOAZFLY_CLOTH_S", "KURTIS_CLOTH_H", "KURTIS_CLOTH_S", "LAB2_CLOTH_H", "LAB2_CLOTH_S",
		"COAT_CLOTH_H", "COAT_CLOTH_S", "KAREL_JACKET_CLOTH_H", "KAREL_JACKET_CLOTH_S", "TENDRILD_CLOTH_H", "TENDRILD_CLOTH_S",
		"TENDRILA_CLOTH_H", "TENDRILA_CLOTH_S", "TENDRILB_CLOTH_H", "TENDRILB_CLOTH_S", "BELLY_DYNAMIC", "STABBY", "CLAWMOUTH"};
	for (const char *n : game_names)
		names.insert(make_pair((uint32_t)GetHashValue(n), string(n)));
	for (const auto &f : AOD_IO.gmxfiles)							// Identificatori nei CHR del livello
	{
		if (f.type != AoDFileType::CHR)
			continue;
		ifstream chrfile(AOD_IO.folder_level + f.name, std::ios::binary);
		string data((istreambuf_iterator<char>(chrfile)), istreambuf_iterator<char>());
		size_t start = 0;
		for (size_t i = 0; i <= data.size(); i++)
		{
			char c = i < data.size() ? data[i] : 0;
			if (isalnum((unsigned char)c) || c == '_')
				continue;
			if (c == 0 && i - start >= 3 && i - start < 64 && (isalpha((unsigned char)data[start]) || data[start] == '_'))
			{
				string word = data.substr(start, i - start), upper = word;
				for (char &u : upper)
					u = (char)toupper((unsigned char)u);
				names.insert(make_pair((uint32_t)GetHashValue(word.c_str()), word));
				names.insert(make_pair((uint32_t)GetHashValue(upper.c_str()), upper));
			}
			start = i + 1;
		}
	}
	return names;
}


/*------------------------------------------------------------------------------------------------------------------
Nome dell'hash di una chiave (MAKE_VISIBLE / INVISIBLE: oggetto agganciato o mesh; FIRE_ANIM_TRIGGER: animazione;
FIRE_MORPH_TRIGGER: sequenza facciale): dizionario del CAL, animazioni del livello, nomi originali (log di WorldEdit).
Vuoto se sconosciuto. Gli ID dei FIRE_AUDIO_EVENT sono numeri degli eventi audio (banchi audio non presenti nel GMX).
------------------------------------------------------------------------------------------------------------------*/
static string CAL_HashName (uint32_t type, uint32_t hash, const map <uint32_t, string> &names)
{
	if (type != CAL_KEY_MAKE_INVISIBLE && type != CAL_KEY_MAKE_VISIBLE && type != CAL_KEY_FIRE_ANIM_TRIGGER && type != CAL_KEY_FIRE_MORPH_TRIGGER)
		return string();
	map <uint32_t, string>::const_iterator n = names.find(hash);
	if (n != names.end())
		return n->second;
	if (type == CAL_KEY_FIRE_ANIM_TRIGGER)
	{
		n = CAL_LevelAnimationNames().find(hash);
		if (n != CAL_LevelAnimationNames().end())
			return n->second;
	}
	if (type == CAL_KEY_MAKE_INVISIBLE || type == CAL_KEY_MAKE_VISIBLE)
	{
		n = CAL_LevelNodeNames().find(hash);
		if (n != CAL_LevelNodeNames().end())
			return n->second;
	}
	string name = AOD_MeshName(hash);
	return name.empty() ? AOD_NodeName(hash) : name;
}


static const char *CAL_KeyName (uint32_t type)					// Nome del tipo di chiave (TRACK_KEY_CODE)
{
	static const char *names[] = {"MAKE_INVISIBLE", "MAKE_VISIBLE", "FIRE_AUDIO_EVENT", "FIRE_ANIM_TRIGGER", "FIRE_MORPH_TRIGGER",
								  "FIRE_FXEVENT_ON", "FIRE_FXEVENT_OFF", "FIRE_FXEVENT_ONESHOT"};
	return type < 8 ? names[type] : "UNKNOWN";
}


/*------------------------------------------------------------------------------------------------------------------
Eventi di un'animazione per il file MA (nodo <animazione>_EVENTS, vedi MA_Write_SkeletalAnimationFile): un attributo
per tipo di chiave con un impulso a gradini (valore al frame della chiave, -1 altrimenti; con piu' chiavi dello stesso
tipo nello stesso frame le successive vanno negli attributi <nome>2, <nome>3, ...):
	audioEvent		FIRE_AUDIO_EVENT: ID dell'evento audio (i flag dei passi sono nel file _EVENTS.txt)
	animTrigger		FIRE_ANIM_TRIGGER: hash dell'animazione sovrapposta (intero con segno)
	morphTrigger	FIRE_MORPH_TRIGGER: hash della sequenza facciale (intero con segno)
e un attributo fx<i> per ogni effetto (FXNODE i) con lo stato dato dalle chiavi FIRE_FXEVENT_*: 0 spento (iniziale),
1 acceso (ON), 2 per un frame (ONESHOT, poi torna allo stato precedente).
Le chiavi MAKE_VISIBLE / INVISIBLE sono gia' curve di visibilita' degli oggetti. Gli effetti agganciati (FXNODE) sono
descritti nell'attributo di testo fxNodes (una riga per effetto), i nomi degli hash delle chiavi nell'attributo
eventNames (una riga "frame TIPO nome" per chiave con nome noto). names: nome di ogni bone / nodo (indice FXNODE.bone),
hash_names: nomi degli hash (vedi CAL_HashName).
------------------------------------------------------------------------------------------------------------------*/
static void CAL_AnimationEvents (const vector <uint8_t> &data, const CAL_ANIMATION &anim, const vector <string> &names, const map <uint32_t, string> &hash_names,
								 SkeletalAnimation &out)
{
	string event_names;
	const char *attr[5] = {nullptr, nullptr, "audioEvent", "animTrigger", "morphTrigger"};
	vector <pair <string, map <unsigned int, double>>> pulses;			// Attributo -> (frame -> valore)
	map <uint32_t, vector <pair <unsigned int, uint32_t>>> fx_keys;		// Effetto -> (frame, tipo) in ordine
	for (uint32_t k = 0; k < anim.nKeys && k < 100000; k++)
	{
		CAL_TRACK_KEY key;
		if (!CAL_Get(data, anim.KEYS_PTR + sizeof(CAL_TRACK_KEY) * (size_t)k, key))
			break;
		unsigned int frame = key.time & 0xFFFF;
		string hash_name = CAL_HashName(key.type, key.data, hash_names);
		if (!hash_name.empty())
			event_names += to_string(frame) + " " + CAL_KeyName(key.type) + " " + hash_name + "\n";
		if (key.type >= CAL_KEY_FIRE_FXEVENT_ON && key.type <= CAL_KEY_FIRE_FXEVENT_ONESHOT)
		{
			fx_keys[key.data].push_back(make_pair(frame, key.type));
			continue;
		}
		if (key.type >= 5 || !attr[key.type])
			continue;
		for (unsigned int n = 1; ; n++)									// Primo attributo libero in questo frame
		{
			string name = string(attr[key.type]) + (n > 1 ? to_string(n) : string());
			unsigned int p = 0;
			while (p < pulses.size() && pulses[p].first != name)
				p++;
			if (p == pulses.size())
				pulses.push_back(make_pair(name, map <unsigned int, double> ()));
			if (!pulses[p].second.count(frame))
			{
				pulses[p].second[frame] = (double)(int32_t)key.data;
				break;
			}
		}
	}
	for (const pair <string, map <unsigned int, double>> &p : pulses)
	{
		vector <pair <unsigned int, double>> keys;
		if (p.second.begin()->first > 0)
			keys.push_back(make_pair(0u, -1.0));
		for (map <unsigned int, double>::const_iterator it = p.second.begin(); it != p.second.end(); ++it)
		{
			keys.push_back(make_pair(it->first, it->second));
			map <unsigned int, double>::const_iterator next = it;
			++next;
			if (next == p.second.end() || next->first > it->first + 1)
				keys.push_back(make_pair(it->first + 1, -1.0));			// Fine dell'impulso
		}
		out.events.push_back(make_pair(p.first, keys));
	}
	for (const pair <const uint32_t, vector <pair <unsigned int, uint32_t>>> &f : fx_keys)
	{
		map <unsigned int, double> state;								// Frame -> stato (l'ultima chiave del frame vince)
		map <unsigned int, double> oneshot;								// Frame dei ONESHOT -> stato successivo
		double current = 0.0;
		for (const pair <unsigned int, uint32_t> &k : f.second)			// Le chiavi sono in ordine di frame
		{
			if (k.second == CAL_KEY_FIRE_FXEVENT_ONESHOT)
				oneshot[k.first] = current;
			else
			{
				current = k.second == CAL_KEY_FIRE_FXEVENT_ON ? 1.0 : 0.0;
				state[k.first] = current;
				oneshot.erase(k.first);
			}
		}
		for (const pair <const unsigned int, double> &o : oneshot)
		{
			state[o.first] = 2.0;
			if (!state.count(o.first + 1))
				state[o.first + 1] = o.second;
		}
		vector <pair <unsigned int, double>> keys;
		if (state.empty() || state.begin()->first > 0)
			keys.push_back(make_pair(0u, 0.0));
		keys.insert(keys.end(), state.begin(), state.end());
		out.events.push_back(make_pair("fx" + to_string(f.first), keys));
	}
	string fx;
	for (uint32_t f = 0; f < anim.nFXNodes && f < 1000; f++)
	{
		CAL_FXNODE node;
		if (!CAL_Get(data, anim.FXNODES_PTR + sizeof(CAL_FXNODE) * (size_t)f, node))
			break;
		char buf[512];
		string bone = node.bone < names.size() ? names[node.bone] : to_string(node.bone);
		snprintf(buf, sizeof(buf), "%u %s bone=%s t=%g %g %g r=%g %g %g s=%g %g %g type=%u message=%u\n", f,
				 string(node.name, strnlen(node.name, sizeof(node.name))).c_str(), bone.c_str(), node.translate[0], node.translate[1], node.translate[2],
				 node.rotate[0] * 57.2957795, node.rotate[1] * 57.2957795, node.rotate[2] * 57.2957795, node.scale[0], node.scale[1], node.scale[2], node.type, node.Message);
		fx += buf;
	}
	if (!fx.empty())
		out.event_text.push_back(make_pair(string("fxNodes"), fx));
	if (!event_names.empty())
		out.event_text.push_back(make_pair(string("eventNames"), event_names));
}


/*------------------------------------------------------------------------------------------------------------------
Scrive \NOMELIVELLO\Animations\<NOMECAL>\<NOMECAL>_EVENTS.txt: per ogni animazione gli effetti agganciati (CAL_FXNODE, tutti
i campi) e tutte le chiavi (CAL_TRACK_KEY: frame, flag dei suoni, tipo, dato), con i valori esatti (float con 9 cifre
significative, interi in esadecimale) per poterli riscrivere. names: nome di ogni bone / nodo (FXNODE.bone), se noto;
hash_names: nomi degli hash delle chiavi (CAL_HashName), scritti dopo il dato.
Formato (una voce per riga):
	ANIM <indice> NOME(0xanimID) frames=N fxnodes=N keys=N
	  FXNODE <i> name=<nome> bone=<nome>(<indice>) mirrorTrack=<n> t=x y z r=x y z (radianti) s=x y z flags=0x.. argb=0x..
	         type=<n> speed= emit= random= u=u0 u1 u2 u3 message=<n>
	  KEY frame=<n> flags=0x.. type=<NOME>(<n>) data=0x........ [nome dell'hash, se noto]
Al termine la cartella corrente torna \NOMELIVELLO.
------------------------------------------------------------------------------------------------------------------*/
static void CAL_ExportEvents (const vector <uint8_t> &data, const vector <CAL_ANIMATION> &anims, string calname, const vector <string> &names,
							  const map <uint32_t, string> &hash_names)
{
	auto Hex = [](uint32_t v) { char b[16]; snprintf(b, sizeof(b), "0x%08X", v); return string(b); };
	stringstream out;
	out << setprecision(9);
	out << "# TRAODLE - eventi delle animazioni di " << calname << ".CAL (effetti agganciati CAL_FXNODE e chiavi CAL_TRACK_KEY)\n";
	out << "# KEY: frame, flags = 16 bit alti del tempo (suoni: 0x100 passo, 0x200 sinistro/destro, 0x400 punta/tallone,\n";
	out << "#      0x4000 non avviato, 0x8000 aggiornato ad ogni frame), tipo, dato\n";
	unsigned int nKeys = 0, nFx = 0;
	for (unsigned int a = 0; a < anims.size(); a++)
	{
		const CAL_ANIMATION &anim = anims[a];
		out << "\nANIM " << a << " " << string(anim.name, strnlen(anim.name, sizeof(anim.name))) << "(" << Hex(anim.animID) << ") frames=" << anim.nFrames
			<< " fxnodes=" << anim.nFXNodes << " keys=" << anim.nKeys << "\n";
		for (uint32_t f = 0; f < anim.nFXNodes && f < 1000; f++)
		{
			CAL_FXNODE node;
			if (!CAL_Get(data, anim.FXNODES_PTR + sizeof(CAL_FXNODE) * (size_t)f, node))
				break;
			out << "  FXNODE " << f << " name=" << string(node.name, strnlen(node.name, sizeof(node.name))) << " bone="
				<< (node.bone < names.size() ? names[node.bone] : string("?")) << "(" << node.bone << ") mirrorTrack=" << node.mirrorTrack
				<< " t=" << node.translate[0] << " " << node.translate[1] << " " << node.translate[2]
				<< " r=" << node.rotate[0] << " " << node.rotate[1] << " " << node.rotate[2]
				<< " s=" << node.scale[0] << " " << node.scale[1] << " " << node.scale[2]
				<< " flags=" << Hex(node.flags) << " argb=" << Hex(node.argb) << " type=" << node.type << " speed=" << node.speed << " emit=" << node.emit
				<< " random=" << node.random << " u=" << node.u0 << " " << node.u1 << " " << node.u2 << " " << node.u3 << " message=" << node.Message << "\n";
			nFx++;
		}
		for (uint32_t k = 0; k < anim.nKeys && k < 100000; k++)
		{
			CAL_TRACK_KEY key;
			if (!CAL_Get(data, anim.KEYS_PTR + sizeof(CAL_TRACK_KEY) * (size_t)k, key))
				break;
			out << "  KEY frame=" << (key.time & 0xFFFF) << " flags=0x" << hex << uppercase << (key.time >> 16) << dec << nouppercase
				<< " type=" << CAL_KeyName(key.type) << "(" << key.type << ") data=" << Hex(key.data);
			string hash_name = CAL_HashName(key.type, key.data, hash_names);
			if (!hash_name.empty())
				out << " " << hash_name;
			out << "\n";
			nKeys++;
		}
	}
	string folder = AOD_IO.folder_animations + calname;
	CreateDirectoryA(AOD_IO.folder_animations.c_str(), NULL);
	CreateDirectoryA(folder.c_str(), NULL);
	string path = folder + "\\" + calname + "_EVENTS.txt";
	ofstream file(path, ios::binary);
	file << out.str();
	SetCurrentDirectory(AOD_IO.folder_level_lpwstr);
	msg(msg::TGT::FILE, msg::TYP::LOG) << "Events: " << nFx << " FX nodes, " << nKeys << " keys in " << path;
}


static bool CAL_ExportObject (const vector <uint8_t> &data, const vector <CAL_ANIMATION> &anims, string calname, const CHR_NodeObject &object)
{
	vector <string> node_names;									// Nomi dei nodi (indici FXNODE.bone)
	map <uint32_t, string> hash_names;							// Nomi degli hash delle chiavi: nodi (nome e mesh)
	for (const CHR_Node &n : object.nodes)
	{
		node_names.push_back(n.group);
		hash_names.insert(make_pair((uint32_t)GetHashValue(n.name.c_str()), n.name));
		hash_names.insert(make_pair(n.mesh_hash, n.name));
	}
	CAL_ExportEvents(data, anims, calname, node_names, hash_names);
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Animated object: " << object.name << ".CHR, animations: " << anims.size();
	FBX_EXPORT FBX;												// Modello + tutte le animazioni come takes
	MA_EXPORT MA;
	bool ok = CHR_Build_NodeObject(object, FBX, MA);
	map <string, Transform> rest;								// Posa del modello
	for (const Transform &g : FBX.Group)
		rest[g.name] = g;
	map <uint32_t, CAL_PropPositions> props = CAL_Read_PropPositions();	// Filmati: posizioni degli oggetti (file XXX)

	string folder = AOD_IO.folder_animations + calname;			// Cartella \NOMELIVELLO\Animations\NOMECAL
	CreateDirectoryA(AOD_IO.folder_animations.c_str(), NULL);
	CreateDirectoryA(folder.c_str(), NULL);
	if (!SetCurrentDirectoryA(folder.c_str()))
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Unable to access folder " << folder << ". Animations not exported.";
		return false;
	}
	set <string> used_names;
	unsigned int exported = 0;
	for (unsigned int a = 0; a < anims.size(); a++)
	{
		const CAL_ANIMATION &anim = anims[a];
		string name = CAL_AnimationName(anim, calname, a, used_names);
		CAL_ANIMATION_DATA anim_data;
		if (anim.nFrames == 0 || anim.nTracks - anim.nParentTracks != object.nodes.size() || !CAL_Get(data, anim.DATA_PTR, anim_data))
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << name << ": " << anim.nFrames << " frames, " << anim.nTracks - anim.nParentTracks << " tracks for " << object.nodes.size() << " nodes. Skipped.";
			continue;
		}
		SkeletalAnimation skanim;
		skanim.name = name;
		skanim.character = object.name;
		skanim.nFrames = anim.nFrames;
		bool valid = true;
		for (unsigned int n = 0; n < object.nodes.size() && valid; n++)
		{
			NodeAnimation node;
			node.node = object.nodes[n].group;
			node.joint = false;
			if (!CAL_ReadSetTrack(data, anim_data.TRACKS_PTR + sizeof(CAL_TRACK) * n, anim.nFrames, rest[object.nodes[n].group], node))
			{
				valid = false;
				break;
			}
			if (!node.tX.empty() || !node.rX.empty() || !node.sX.empty())
				skanim.nodes.push_back(node);
		}
		if (!valid)
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << name << ": invalid animation data. Skipped.";
			ok = false;
			continue;
		}
		map <uint32_t, CAL_PropPositions>::const_iterator prop = props.find(anim.animID);	// Filmati: origine dell'oggetto nel mondo
		if (prop != props.end() && !prop->second.frames.empty())
		{
			NodeAnimation root;
			root.node = object.name;
			root.joint = false;
			for (unsigned int f = 0; f < anim.nFrames; f++)
			{
				const POS_FRAME &p = prop->second.frames[min<size_t>(f, prop->second.frames.size() - 1)];
				root.tX.push_back(p.x);	root.tY.push_back(p.y);	root.tZ.push_back(p.z);
				root.vis.push_back(p.visibility > 0.5f ? 1.0f : 0.0f);
			}
			if (!CAL_Differs(root.vis, 1))
				root.vis.clear();
			skanim.nodes.push_back(root);
			if (prop->second.frames.size() != anim.nFrames)
				msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << name << ": " << anim.nFrames << " frames, " << prop->second.frames.size() << " positions in " << prop->second.xxxfile << ".";
			msg(msg::TGT::FILE, msg::TYP::LOG) << name << ": object position from " << prop->second.xxxfile << " (" << prop->second.object << ", " << prop->second.node << ")";
		}
		CAL_AnimationEvents(data, anim, node_names, hash_names, skanim);
		MA_Write_SkeletalAnimationFile(skanim, "../../Characters/" + object.name + ".MA");	// Stesso nome scritto da MA_Export
		FBX.SkeletalAnimation.push_back(move(skanim));
		exported++;
	}
	if (exported > 0)
		FBX_Export(calname, FBX);
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Output: " << exported << " animations in " << folder;
	return ok;
}


/*------------------------------------------------------------------------------------------------------------------
Scrive \NOMELIVELLO\Animations\<NOMECAL>\<NOMECAL>_TRANSITIONS.txt: le regole di passaggio tra le animazioni del CAL
(macchina a stati del runtime), che non si possono ricavare dalle curve: per ogni animazione flags, animazione
alternativa, raccordi diretti (CAL_LINK) e finestre di blend (CAL_BLENDRANGE / CAL_BLENDCLIP), poi la tabella dei
percorsi (CAL_LINKPATH) nell'ordine del file. Formato regolare, una voce per riga, per poter ricostruire i dati esatti:
ogni animazione citata e' scritta come NOME(0xHASH) (NOME = "?" se l'hash non e' un'animazione del file), i tempi sono
in frames (30 fps) con la precisione completa dei float. Al termine la cartella corrente torna \NOMELIVELLO.
------------------------------------------------------------------------------------------------------------------*/
static bool CAL_ExportTransitions (const vector <uint8_t> &data, const CAL_HEADER &header, const vector <CAL_ANIMATION> &anims, string calname)
{
	map <uint32_t, string> names;								// animID -> nome (prima animazione con quell'ID)
	auto RawName = [](const CAL_ANIMATION &anim) -> string
	{
		string name(anim.name, strnlen(anim.name, sizeof(anim.name)));
		for (char &c : name)
			if (!isgraph((unsigned char)c) || c == '(' || c == ')')
				c = '_';
		return name.empty() ? string("?") : name;
	};
	for (const CAL_ANIMATION &anim : anims)
		names.insert(make_pair(anim.animID, RawName(anim)));
	auto Hex = [](uint32_t value) -> string
	{
		stringstream s;
		s << "0x" << hex << uppercase << setw(8) << setfill('0') << value;
		return s.str();
	};
	auto Ref = [&](uint32_t hash) -> string						// Animazione citata: NOME(0xHASH)
	{
		map <uint32_t, string>::const_iterator n = names.find(hash);
		return (n != names.end() ? n->second : string("?")) + "(" + Hex(hash) + ")";
	};

	stringstream out;
	out << setprecision(9);
	out << "# TRAODLE - transizioni tra le animazioni di " << calname << ".CAL\n";
	out << "# ANIM indice NOME(animID) altanim=NOME(altanimID) flags=0x.. [MIRROR LOOP LINK TERMINATOR] frames=N\n";
	out << "#   LINK destinazione via=sinistra destra                       (CAL_LINK: raccordo verso la destinazione)\n";
	out << "#   BLEND destinazione clips=N                                   (CAL_BLENDRANGE)\n";
	out << "#     CLIP flags=0x.. source=inizio fine dest=inizio fine blend=durata   (CAL_BLENDCLIP, tempi in frames)\n";
	out << "# LINKPATH partenza arrivo via=intermedie...                     (CAL_LINKPATH, ordine del file)\n";
	out << "CAL " << calname << ".CAL animations=" << header.nAnims << " linkpaths=" << header.nLinkPaths << " uData=" << Hex(header.uData) << "\n";
	bool ok = true;
	unsigned int nLinks = 0, nBlends = 0;
	for (unsigned int a = 0; a < anims.size(); a++)
	{
		const CAL_ANIMATION &anim = anims[a];
		out << "\nANIM " << a << " " << RawName(anim) << "(" << Hex(anim.animID) << ") altanim=" << Ref(anim.altanimID) << " flags=" << Hex(anim.flags);
		for (const pair <uint32_t, const char*> &f : {make_pair((uint32_t)CAL_APBFLAG_MIRROR, "MIRROR"), make_pair((uint32_t)CAL_APBFLAG_LOOP, "LOOP"),
			make_pair((uint32_t)CAL_APBFLAG_LINK, "LINK"), make_pair((uint32_t)CAL_APBFLAG_TERMINATOR, "TERMINATOR")})
			if (anim.flags & f.first)
				out << " " << f.second;
		out << " frames=" << anim.nFrames << "\n";
		for (uint32_t l = 0; l < anim.nLinkRanges && l < 10000; l++)
		{
			CAL_LINK link;
			if (!CAL_Get(data, anim.LINKS_PTR + sizeof(CAL_LINK) * (size_t)l, link))
			{
				ok = false;
				break;
			}
			out << "  LINK " << Ref(link.destID) << " via=" << Ref(link.LLinkID) << " " << Ref(link.RLinkID) << "\n";
			nLinks++;
		}
		for (uint32_t b = 0; b < anim.nBlendRanges && b < 10000; b++)
		{
			CAL_BLENDRANGE range;
			if (!CAL_Get(data, anim.BLENDRANGES_PTR + sizeof(CAL_BLENDRANGE) * (size_t)b, range))
			{
				ok = false;
				break;
			}
			out << "  BLEND " << Ref(range.destID) << " clips=" << range.nClips << "\n";
			for (uint32_t c = 0; c < range.nClips && c < 1000; c++)
			{
				CAL_BLENDCLIP clip;
				if (!CAL_Get(data, range.BLENDCLIPS_PTR + sizeof(CAL_BLENDCLIP) * (size_t)c, clip))
				{
					ok = false;
					break;
				}
				out << "    CLIP flags=" << Hex(clip.flags) << " source=" << clip.sourceStart << " " << clip.sourceEnd << " dest=" << clip.destStart << " " << clip.destEnd << " blend=" << clip.blendTime << "\n";
			}
			nBlends++;
		}
	}
	if (header.nLinkPaths)
		out << "\n";
	for (uint32_t p = 0; p < header.nLinkPaths && p < 100000; p++)
	{
		CAL_LINKPATH path;
		if (!CAL_Get(data, header.LINKPATHS_PTR + sizeof(CAL_LINKPATH) * (size_t)p, path))
		{
			ok = false;
			break;
		}
		out << "LINKPATH " << Ref(path.sourceID) << " " << Ref(path.destID) << " via=";
		for (uint32_t k = 0; k < path.nLinks && k < 1000; k++)
		{
			uint32_t hash;
			if (!CAL_Get(data, path.LINKS_PTR + 4 * (size_t)k, hash))
			{
				ok = false;
				break;
			}
			out << (k ? " " : "") << Ref(hash);
		}
		out << "\n";
	}
	if (!ok)
		msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << calname << ".CAL: transition data out of file.";

	string folder = AOD_IO.folder_animations + calname;			// Cartella \NOMELIVELLO\Animations\NOMECAL
	CreateDirectoryA(AOD_IO.folder_animations.c_str(), NULL);
	CreateDirectoryA(folder.c_str(), NULL);
	string path = folder + "\\" + calname + "_TRANSITIONS.txt";
	ofstream file(path, ios::binary);
	file << out.str();
	SetCurrentDirectory(AOD_IO.folder_level_lpwstr);
	if (!file)
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Unable to write " << path;
		return false;
	}
	msg(msg::TGT::FILE, msg::TYP::LOG) << "Transitions: " << nLinks << " links, " << nBlends << " blend ranges, " << header.nLinkPaths << " link paths in " << path;
	return ok;
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
Se il CAL ha lo stesso nome di un oggetto animato (CHR "NODE") o se nessun personaggio corrisponde, le animazioni vengono
esportate per l'oggetto animato, con un nodo per traccia (CAL_ExportObject).
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
	string calname = filename.substr(0, filename.find(".CAL"));
	CAL_ExportTransitions(data, header, anims, calname);		// Regole di passaggio tra le animazioni (file di testo)

	// Ricerca del personaggio
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
	CHR_NodeObject object;										// Oggetto animato (CHR "NODE") con lo stesso nome del CAL
	if (CHR_FindNodeObject(calname, object) && object.nodes.size() == nBoneTracks)
		return CAL_ExportObject(data, anims, calname, object);
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
	if (!found)													// Oggetti animati fra i candidati
		for (const string &c : candidates)
			if (CHR_FindNodeObject(c.substr(0, c.find(".CHR")), object) && object.nodes.size() == nBoneTracks)
				return CAL_ExportObject(data, anims, calname, object);
	if (!found)
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << "No character or animated object (CHR) with " << nBoneTracks << " animated bones / nodes found for " << filename << ". Animations not exported.";
		CAL_ExportEvents(data, anims, calname, vector <string> (), map <uint32_t, string> ());	// Gli eventi vengono comunque scritti
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
	map <uint32_t, string> hash_names;							// Nomi degli hash delle chiavi: oggetti agganciati, mesh, sequenze
	for (const CHR_Attachment &a : character.skeleton.attachments)
		hash_names.insert(make_pair(a.hash, a.name));
	for (const pair <const uint32_t, vector <string>> &m : character.mesh_names)
		if (!m.second.empty())
			hash_names.insert(make_pair(m.first, m.second[0]));
	for (const BlendShapeAnimation &b : facial_base.anims)
		hash_names.insert(make_pair(b.id, b.name));
	CAL_ExportEvents(data, anims, calname, character.skeleton.joint_name, hash_names);	// Eventi delle animazioni (file di testo)

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
	FBX.Group.insert(FBX.Group.end(), character.groups.begin(), character.groups.end());	// Oggetti agganciati
	set <string> used_names;
	unsigned int exported = 0;
	bool ok = true;
	for (unsigned int a = 0; a < anims.size(); a++)
	{
		const CAL_ANIMATION &anim = anims[a];
		string name = CAL_AnimationName(anim, calname, a, used_names);

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
		vector <string> hidden = CS_HiddenAttachments(string(anim.name, strnlen(anim.name, sizeof(anim.name))));	// Switch del filmato
		if (!hidden.empty())
			msg(msg::TGT::FILE, msg::TYP::LOG) << name << ": " << hidden.size() << " attached objects hidden by the cutscene switches.";
		if (!CAL_ReadAnimation(data, anim, character, actor, skanim, hidden))
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
		CAL_AnimationEvents(data, anim, character.skeleton.joint_name, hash_names, skanim);	// Suoni, animazioni sovrapposte, effetti
		MA_Write_SkeletalAnimationFile(skanim, "../../Characters/" + character.name + ".MA", face.meshes.empty() ? nullptr : &face, face_targets);	// Stesso nome scritto da MA_Export
		FBX.SkeletalAnimation.push_back(move(skanim));
		exported++;
	}
	if (exported > 0)
		FBX_Export(calname, FBX);								// Un solo FBX: scheletro + una take per animazione
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Output: " << exported << " animations in " << folder;
	return ok;
}
