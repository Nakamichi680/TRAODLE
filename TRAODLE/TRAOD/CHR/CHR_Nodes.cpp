#include "stdafx.h"
#include <map>
#include <set>
#include "Classes.h"
#include "MATH/math.h"
#include "MA/MA_Functions.h"
#include "FBX/FBX_Functions.h"
#include "TRAOD/CHR/CHR_Functions.h"
#include "TRAOD/ZONE/ZONE_Struct.h"
#include "TRAOD/ZONE/ZONE_Functions.h"
#include "hash_Functions.h"
#include "TRAOD/EVX/EVX_Functions.h"
#include "TRAOD/CAL/CAL_Functions.h"


/*------------------------------------------------------------------------------------------------------------------
Oggetti animati (file CHR "NODE", vedi CHR_Struct.h): porte, cassetti, leve, macchinari, ... Il CHR contiene solo i
nodi (transform di Maya con pivot), le mesh sono quelle degli oggetti del file ZONE del livello con lo stesso nome dei
nodi, contenute in uno slot (ZONE_PS2_OBJ_SLOTS) piazzato nel livello da un nodo RMX_OBJECT. Le animazioni sono nel CAL
omonimo (una traccia per nodo, vedi Export_CAL).
------------------------------------------------------------------------------------------------------------------*/


static string CHR_NodeGroupName (string name)			// Nome valido per un nodo di Maya
{
	for (char &c : name)
		if (!isalnum((unsigned char)c) && c != '_')
			c = '_';
	return name;
}


static bool CHR_IsIdentity (const MATRIX &m)
{
	const float a[16] = {m.m00, m.m01, m.m02, m.m03, m.m10, m.m11, m.m12, m.m13, m.m20, m.m21, m.m22, m.m23, m.m30, m.m31, m.m32, m.m33};
	for (int i = 0; i < 16; i++)
		if (fabsf(a[i] - ((i % 5 == 0) ? 1.0f : 0.0f)) > 1e-4f)
			return false;
	return true;
}


/*------------------------------------------------------------------------------------------------------------------
Oggetti dei file ZONE del livello (letti una volta sola). Va chiamata con la cartella corrente \NOMELIVELLO.
------------------------------------------------------------------------------------------------------------------*/
static const map <string, ZONE_ObjectData> &CHR_ZoneData ()
{
	static map <string, ZONE_ObjectData> zones;
	static bool loaded = false;
	if (loaded)
		return zones;
	loaded = true;
	for (const auto &f : AOD_IO.gmxfiles)
	{
		size_t z = f.name.rfind(".Z");
		if (f.type != AoDFileType::ZONE || z == string::npos || f.name.size() != z + 4 || !isdigit((unsigned char)f.name[z + 2]) || !isdigit((unsigned char)f.name[z + 3]))
			continue;
		ifstream zonefile(f.name, std::ios::binary);
		ZONE_ObjectData data;
		if (zonefile.is_open() && ZONE_ReadObjectData(zonefile, f.name, data))
			zones[f.name] = move(data);
	}
	return zones;
}


/*------------------------------------------------------------------------------------------------------------------
Lettura dei nodi di un CHR "NODE" (gerarchia dai flags push/pop come le bones dei personaggi). Falso se il file non e'
un oggetto animato.
------------------------------------------------------------------------------------------------------------------*/
static bool CHR_Read_Nodes (string chrname, CHR_NodeObject &object)
{
	ifstream chrfile(chrname + ".CHR", std::ios::binary);
	CHR_NODE_HEADER header;
	if (!chrfile.read(reinterpret_cast<char*>(&header), sizeof(header)) || header.Magic != CHR_NODE_MAGIC || header.nNodes == 0 || header.nNodes > 10000)
		return false;
	vector <int> stack;
	int current = -1;
	set <string> used;
	for (uint32_t n = 0; n < header.nNodes; n++)
	{
		uint32_t length = 0;
		chrfile.read(reinterpret_cast<char*>(&length), sizeof(length));
		if (!chrfile || length > 1024)
			return false;
		string name(length, '\0');
		chrfile.read(&name[0], length);
		name.resize(strnlen(name.c_str(), length));
		CHR_NODE_DATA d;
		if (!chrfile.read(reinterpret_cast<char*>(&d), sizeof(d)))
			return false;

		CHR_Node node;
		node.name = name;
		node.flags = d.Flags;
		string upper = name;
		for (char &c : upper)
			c = (char)toupper((unsigned char)c);
		node.mesh_hash = (uint32_t)GetHashValue(upper.c_str());
		node.rotate_pivot = Vec3(d.RotatePivot[0], d.RotatePivot[1], d.RotatePivot[2]);
		node.scale_pivot = Vec3(d.ScalePivot[0], d.ScalePivot[1], d.ScalePivot[2]);
		node.rotate_pivot_translate = Vec3(d.RotatePivotTranslate[0], d.RotatePivotTranslate[1], d.RotatePivotTranslate[2]);
		node.scale_pivot_translate = Vec3(d.ScalePivotTranslate[0], d.ScalePivotTranslate[1], d.ScalePivotTranslate[2]);
		memcpy(&node.orient, d.RotateOrient, sizeof(d.RotateOrient));
		if (node.flags & CHR_NODEFLAG_POP)						// Ripristina la matrice salvata
		{
			if (!stack.empty())
			{
				current = stack.back();
				stack.pop_back();
			}
		}
		if (node.flags & CHR_NODEFLAG_PUSH)						// Salva la matrice corrente
			stack.push_back(current);
		node.parent = current;
		current = (int)n;
		node.group = CHR_NodeGroupName(object.name + "_" + name);
		if (used.count(node.group))
			node.group += "_" + to_string(n);
		used.insert(node.group);
		object.nodes.push_back(node);
	}
	return true;
}


static int CHR_ZoneIndex (const string &zonefile)			// Indice della zona dall'estensione (.Z00, .Z01, ...)
{
	size_t z = zonefile.rfind(".Z");
	return (z == string::npos) ? -1 : atoi(zonefile.substr(z + 2).c_str());
}


static unsigned int CHR_MatchSlot (const ZONE_ObjectData &data, unsigned int slot, const CHR_NodeObject &object)	// Nodi con mesh nello slot
{
	set <uint32_t> hashes;
	for (const ZONE_SlotCommand &cmd : data.slots[slot])
		if (cmd.type >= 0 && (unsigned int)cmd.type < data.mesh_hash.size())
			hashes.insert(data.mesh_hash[cmd.type]);
	unsigned int matched = 0;
	for (const CHR_Node &node : object.nodes)
		matched += hashes.count(node.mesh_hash) ? 1 : 0;
	return matched;
}


/*------------------------------------------------------------------------------------------------------------------
Slot dei file ZONE collegati esplicitamente ad un CHR "NODE", come fa il gioco (zona, slot):
- oggetti del livello: i RMX_OBJECT il cui Object Template (EVX_OBJTEMPLATE, indice RMX_OBJECT.ObjectIndex) ha
  AnimFileName uguale al nome del CHR (es. DOOR_LEFT, weed);
- oggetti dei filmati (OBJECTnn_<FILMATO>): il RMX_OBJECT del nodo indicato per OBJECTnn nel file <FILMATO>.XXX (es.
  OBJECT03_CS_6_21A -> objects__gong1).
------------------------------------------------------------------------------------------------------------------*/
static set <pair <int, unsigned int>> CHR_LinkedSlots (const string &chrname)
{
	auto Upper = [](string s) { for (char &c : s) c = (char)toupper((unsigned char)c); return s; };
	string name = Upper(chrname);
	set <unsigned int> templates;
	const vector <EVX_OBJTEMPLATE> &t = EVX_GetTemplates();
	for (unsigned int i = 0; i < t.size(); i++)
		if (Upper(t[i].AnimFileName) == name)
			templates.insert(i);
	set <uint32_t> nodes;
	for (const pair <const uint32_t, CAL_PropPositions> &p : CAL_Read_PropPositions())
	{
		string xxx = p.second.xxxfile.substr(0, p.second.xxxfile.find('.'));
		if (Upper(p.second.object + "_" + xxx) == name)
			nodes.insert((uint32_t)GetHashValue(p.second.node.c_str()));
	}
	set <pair <int, unsigned int>> slots;
	for (const ZONE_Instance &instance : ZONE_GetLevelObjects().instances)
		if (instance.interactive && (templates.count(instance.object_index) || nodes.count(instance.hash)))
			slots.insert(make_pair(instance.zone, instance.slot));
	return slots;
}


/*------------------------------------------------------------------------------------------------------------------
Legge i nodi di un CHR "NODE" e sceglie lo slot dei file ZONE con le sue mesh: tra gli slot collegati esplicitamente
(CHR_LinkedSlots) quello con le mesh del maggior numero di nodi; se non ce ne sono, lo slot di tutto il livello con
le mesh del maggior numero di nodi (stesso nome), preferendo a parita' gli slot piazzati da un nodo RMX_OBJECT.
Falso se il CHR non e' un oggetto animato. Va chiamata con la cartella corrente \NOMELIVELLO.
------------------------------------------------------------------------------------------------------------------*/
bool CHR_FindNodeObject (string chrname, CHR_NodeObject &object)
{
	object = CHR_NodeObject();
	object.name = chrname;
	if (!CHR_Read_Nodes(chrname, object))
		return false;
	set <pair <int, unsigned int>> placed;						// Slot degli oggetti interattivi
	for (const ZONE_Instance &instance : ZONE_GetLevelObjects().instances)
		if (instance.interactive)
			placed.insert(make_pair(instance.zone, instance.slot));
	set <pair <int, unsigned int>> linked = CHR_LinkedSlots(chrname);
	object.linked = !linked.empty();
	bool best_placed = false;
	for (const pair <const string, ZONE_ObjectData> &z : CHR_ZoneData())
	{
		int zone = CHR_ZoneIndex(z.first);
		for (unsigned int s = 0; s < z.second.slots.size(); s++)
		{
			if (object.linked && !linked.count(make_pair(zone, s)))
				continue;										// Solo gli slot collegati
			unsigned int matched = CHR_MatchSlot(z.second, s, object);
			if (object.linked && object.zonefile.empty())		// Il primo slot collegato anche senza nomi in comune
			{
				object.matched = matched;
				object.zonefile = z.first;
				object.zone = zone;
				object.slot = s;
			}
			bool is_placed = placed.count(make_pair(zone, s)) > 0;
			if (matched > object.matched || (matched > 0 && matched == object.matched && is_placed && !best_placed))
			{
				object.matched = matched;
				object.zonefile = z.first;
				object.zone = zone;
				object.slot = s;
				best_placed = is_placed;
			}
		}
	}
	if (object.zonefile.empty())								// Nessuno slot: zona con le mesh dei nodi fuori dagli slot
		for (const pair <const string, ZONE_ObjectData> &z : CHR_ZoneData())
		{
			for (const CHR_Node &node : object.nodes)
				if (node.mesh_hash && find(z.second.mesh_hash.begin(), z.second.mesh_hash.end(), node.mesh_hash) != z.second.mesh_hash.end())
				{
					object.zonefile = z.first;
					object.zone = CHR_ZoneIndex(z.first);
					object.slot = (unsigned int)z.second.slots.size();	// Nessuno slot
					break;
				}
			if (!object.zonefile.empty())
				break;
		}
	return true;
}


/*------------------------------------------------------------------------------------------------------------------
Oggetto animato di ogni slot dei file ZONE del livello: (zona, slot) -> nome del CHR "NODE" con le mesh del maggior
numero di nodi nello slot (a parita', quello con la percentuale maggiore di nodi trovati, poi il primo in ordine
alfabetico). Calcolato una volta sola; va chiamata con la cartella corrente \NOMELIVELLO.
------------------------------------------------------------------------------------------------------------------*/
const map <pair <int, unsigned int>, string> &CHR_GetNodeObjectSlots ()
{
	static map <pair <int, unsigned int>, string> slots;
	static bool loaded = false;
	if (loaded)
		return slots;
	loaded = true;
	vector <CHR_NodeObject> objects;
	for (const auto &f : AOD_IO.gmxfiles)
		if (f.type == AoDFileType::CHR)
		{
			CHR_NodeObject object;
			object.name = f.name.substr(0, f.name.find(".CHR"));
			if (CHR_Read_Nodes(object.name, object))
				objects.push_back(object);
		}
	sort(objects.begin(), objects.end(), [](const CHR_NodeObject &a, const CHR_NodeObject &b) { return a.name < b.name; });
	set <pair <int, unsigned int>> linked;						// Slot collegati esplicitamente (template / XXX)
	for (const CHR_NodeObject &object : objects)
		for (const pair <int, unsigned int> &s : CHR_LinkedSlots(object.name))
			if (!slots.count(s))
			{
				slots[s] = object.name;
				linked.insert(s);
			}
	map <pair <int, unsigned int>, float> score;
	for (const pair <const string, ZONE_ObjectData> &z : CHR_ZoneData())
		for (unsigned int s = 0; s < z.second.slots.size(); s++)
			for (const CHR_NodeObject &object : objects)
			{
				unsigned int matched = CHR_MatchSlot(z.second, s, object);
				float value = matched + (float)matched / (object.nodes.size() + 1);
				pair <int, unsigned int> key(CHR_ZoneIndex(z.first), s);
				if (linked.count(key))
					continue;
				if (matched > 0 && (!score.count(key) || value > score[key]))
				{
					score[key] = value;
					slots[key] = object.name;
				}
			}
	return slots;
}


/*------------------------------------------------------------------------------------------------------------------
Traslazione e rotazione di un nodo (con i suoi pivot e rotate axis) tali che la sua matrice locale sia L:
L = Tr(ST - RP) * RA * R * Tr(RP + RT + T) (senza scala), quindi R = RA^-1 * L e T = L.t - (ST - RP) * L - RP - RT.
------------------------------------------------------------------------------------------------------------------*/
static void CHR_NodePose (const CHR_Node &node, const MATRIX &L, Transform &group)
{
	MATRIX Q = L;
	Q.m30 = Q.m31 = Q.m32 = 0;
	MATRIX RA = node.orient;
	RA.m30 = RA.m31 = RA.m32 = 0;
	Vec3 t, r, s;
	mathMatrixDecompose(mathMulMatrices(mathMatrixInverse(RA), Q), &t, &r, &s);
	Vec3 a = node.scale_pivot_translate - node.rotate_pivot;
	Vec3 aq(a.x * Q.m00 + a.y * Q.m10 + a.z * Q.m20, a.x * Q.m01 + a.y * Q.m11 + a.z * Q.m21, a.x * Q.m02 + a.y * Q.m12 + a.z * Q.m22);
	group.tX = L.m30 - aq.x - node.rotate_pivot.x - node.rotate_pivot_translate.x;
	group.tY = L.m31 - aq.y - node.rotate_pivot.y - node.rotate_pivot_translate.y;
	group.tZ = L.m32 - aq.z - node.rotate_pivot.z - node.rotate_pivot_translate.z;
	group.rX = r.x;	group.rY = r.y;	group.rZ = r.z;
}


/*------------------------------------------------------------------------------------------------------------------
Modello di un oggetto animato: gruppo radice (nome del CHR), un gruppo per nodo (<CHR>_<nodo>) con pivot, traslazioni
dei pivot e rotate axis, e le mesh dello slot del file ZONE:
- mesh di ogni nodo come le cerca il gioco (mapFindAliasMesh): nello slot dell'oggetto una mesh con lo stesso nome
  (hash) o con ZONE_MESHFLAG_ALIAS e AliasHash uguale; altrimenti la prima mesh con lo stesso hash in tutta la zona
  (mapFindMeshFromHash: es. il gong di CS_6_21A ha il gong nello slot 221 e trave e catene nello slot 222), con la sua
  trasformazione nel proprio slot. Le loro collisioni (ZONE_OBJECT_INFO.CollisionIndex) seguono la mesh. Figlie del
  nodo; le altre mesh dello slot figlie del gruppo radice (parti fisse). I vertici sono nello spazio dell'oggetto;
- posa di riposo: quella dello slot del file ZONE (trasformazione del nodo della lista di comandi che contiene la prima
  mesh del nodo, scomposta con i pivot del nodo), che e' la posa dell'oggetto nel livello;
- mesh con una trasformazione diversa da quella del loro nodo: in un gruppo intermedio (<mesh>_T).
Le collisioni vanno nel layer nascosto <CHR>_Collisions. Va chiamata con la cartella corrente \NOMELIVELLO.
------------------------------------------------------------------------------------------------------------------*/
bool CHR_Build_NodeObject (const CHR_NodeObject &object, FBX_EXPORT &FBX, MA_EXPORT &MA, unsigned int *nodes_with_mesh)
{
	ZONE_ObjectExport out;
	Transform root;
	root.name = object.name;
	root.translate_flag = root.rotate_flag = root.scale_flag = true;
	out.groups.push_back(root);

	const ZONE_ObjectData *data = nullptr;
	map <string, ZONE_ObjectData>::const_iterator z = CHR_ZoneData().find(object.zonefile);
	if (z != CHR_ZoneData().end())
		data = &z->second;
	vector <pair <int, MATRIX>> meshes;
	if (data && object.slot < data->slots.size())
		meshes = ZONE_SlotMeshes(*data, object.slot);
	vector <int> mesh_node(meshes.size(), -1);						// Nodo di ogni mesh
	for (unsigned int m = 0; m < meshes.size(); m++)
		for (unsigned int n = 0; n < object.nodes.size() && mesh_node[m] < 0; n++)
			if (data->mesh_hash[meshes[m].first] == object.nodes[n].mesh_hash ||
				((data->mesh_flags[meshes[m].first] & ZONE_MESHFLAG_ALIAS) && data->mesh_alias[meshes[m].first] == object.nodes[n].mesh_hash))
				mesh_node[m] = n;
	if (data)														// Nodi senza mesh nello slot: mesh con lo stesso hash nella zona
	{
		set <unsigned int> found;
		for (int n : mesh_node)
			if (n >= 0)
				found.insert(n);
		map <uint32_t, unsigned int> wanted;						// Hash -> nodo
		for (unsigned int n = 0; n < object.nodes.size(); n++)
			if (!found.count(n) && object.nodes[n].mesh_hash && !wanted.count(object.nodes[n].mesh_hash))
				wanted[object.nodes[n].mesh_hash] = n;
		for (unsigned int s = 0; s < data->slots.size() && !wanted.empty(); s++)
		{
			if (s == object.slot)
				continue;
			vector <pair <int, MATRIX>> other = ZONE_SlotMeshes(*data, s);
			for (const pair <int, MATRIX> &m : other)
			{
				map <uint32_t, unsigned int>::iterator w = wanted.find(data->mesh_hash[m.first]);
				if (w == wanted.end() || data->mesh_collision[m.first])
					continue;
				meshes.push_back(m);
				mesh_node.push_back((int)w->second);
				for (const pair <int, MATRIX> &c : other)			// Collisione della mesh
					if (c.first == data->mesh_link[m.first])
					{
						meshes.push_back(c);
						mesh_node.push_back((int)w->second);
					}
				wanted.erase(w);
			}
		}
		for (unsigned int i = 0; i < data->mesh_hash.size() && !wanted.empty(); i++)	// Mesh fuori da ogni slot
		{
			map <uint32_t, unsigned int>::iterator w = wanted.find(data->mesh_hash[i]);
			if (w == wanted.end() || data->mesh_collision[i] || (data->mesh_vertices[i] == 0 && data->mesh_base[i] < 0))
				continue;
			meshes.push_back(make_pair((int)i, MATRIX()));
			mesh_node.push_back((int)w->second);
			wanted.erase(w);
		}
	}
	for (unsigned int m = 0; m < meshes.size(); m++)				// Collisioni delle mesh di un nodo
		for (unsigned int m2 = 0; m2 < meshes.size() && mesh_node[m] < 0; m2++)
			if (mesh_node[m2] >= 0 && data->mesh_link[meshes[m2].first] == meshes[m].first)
				mesh_node[m] = mesh_node[m2];

	if (nodes_with_mesh)
	{
		set <int> n_found(mesh_node.begin(), mesh_node.end());
		n_found.erase(-1);
		*nodes_with_mesh = (unsigned int)n_found.size();
	}

	// Nodi: posa di riposo dalla prima mesh del nodo (i padri precedono sempre i figli)
	vector <MATRIX> world(object.nodes.size());
	for (unsigned int n = 0; n < object.nodes.size(); n++)
	{
		const CHR_Node &node = object.nodes[n];
		MATRIX parent_world = (node.parent >= 0) ? world[node.parent] : MATRIX();
		Transform group;
		group.name = node.group;
		group.parent = (node.parent >= 0) ? object.nodes[node.parent].group : root.name;
		group.FBX_parent = hashID(group.parent, "Group");
		group.translate_flag = group.rotate_flag = group.scale_flag = true;
		group.pivot_flag = true;
		group.rotate_pivot = node.rotate_pivot;
		group.scale_pivot = node.scale_pivot;
		group.rotate_pivot_translate = node.rotate_pivot_translate;
		group.scale_pivot_translate = node.scale_pivot_translate;
		MATRIX RA = node.orient;
		RA.m30 = RA.m31 = RA.m32 = 0;
		Vec3 t, r, s;
		mathMatrixDecompose(RA, &t, &r, &s);
		group.rotate_axis = r;
		int first = -1;
		for (unsigned int m = 0; m < meshes.size() && first < 0; m++)
			if (mesh_node[m] == (int)n)
				first = m;
		if (first >= 0)
		{
			world[n] = meshes[first].second;
			CHR_NodePose(node, mathMulMatrices(world[n], mathMatrixInverse(parent_world)), group);
		}
		else														// Nodo senza mesh: traslazione e rotazione nulle
		{
			MATRIX local;
			local.m30 = node.rotate_pivot_translate.x + node.scale_pivot_translate.x;
			local.m31 = node.rotate_pivot_translate.y + node.scale_pivot_translate.y;
			local.m32 = node.rotate_pivot_translate.z + node.scale_pivot_translate.z;
			world[n] = mathMulMatrices(local, parent_world);
		}
		out.groups.push_back(group);
	}

	// Mesh dello slot
	bool ok = true;
	if (data)
	{
		ifstream zonefile(object.zonefile, std::ios::binary);
		set <string> used;
		for (unsigned int m = 0; m < meshes.size(); m++)
		{
			int mesh = meshes[m].first;
			if (data->mesh_vertices[mesh] == 0 && data->mesh_base[mesh] < 0 && !data->mesh_collision[mesh])
				continue;											// Nessuna geometria (lotti PS2)
			int n = mesh_node[m];
			string parent = (n >= 0) ? object.nodes[n].group : root.name;
			MATRIX relative = mathMulMatrices(meshes[m].second, mathMatrixInverse((n >= 0) ? world[n] : MATRIX()));
			string name = parent + "_M" + to_string(mesh);
			if (used.count(name))
				name += "_" + to_string(m);
			used.insert(name);
			if (!CHR_IsIdentity(relative))							// Gruppo intermedio con la trasformazione della mesh
			{
				Transform group;
				group.name = name + "_T";
				group.parent = parent;
				group.FBX_parent = hashID(parent, "Group");
				group.translate_flag = group.rotate_flag = true;
				Vec3 t, r, s;
				mathMatrixDecompose(relative, &t, &r, &s);
				group.tX = t.x;	group.tY = t.y;	group.tZ = t.z;
				group.rX = r.x;	group.rY = r.y;	group.rZ = r.z;
				out.groups.push_back(group);
				parent = group.name;
			}
			if (data->mesh_collision[mesh])
				ok &= ZONE_ExportCollision(zonefile, *data, mesh, name + "_COL", parent, object.name + "_Collisions", MATRIX(), out);
			else
				ok &= ZONE_ExportMesh(zonefile, *data, mesh, name, parent, "", MATRIX(), out);
		}
	}
	out.PushTo(FBX, MA);
	return ok;
}


/*------------------------------------------------------------------------------------------------------------------
Esporta un oggetto animato (CHR "NODE") nei formati FBX e MA nella cartella \NOMELIVELLO\Characters (vedi
CHR_Build_NodeObject), con i materiali e le textures della zona per il file MA. Le animazioni del CAL omonimo
referenziano questo file (Export_CAL).
------------------------------------------------------------------------------------------------------------------*/
bool Export_CHR_Node (string chrname)
{
	SetCurrentDirectory(AOD_IO.folder_level_lpwstr);			// \NOMELIVELLO
	CHR_NodeObject object;
	if (!CHR_FindNodeObject(chrname, object))
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << chrname << ".CHR is not a valid animated object (NODE) file.";
		return false;
	}
	if (object.zonefile.empty())
		msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << chrname << ".CHR: no ZONE object contains the meshes of its nodes. Only the " << object.nodes.size() << " nodes are exported.";
	FBX_EXPORT FBX;
	MA_EXPORT MA;
	unsigned int with_mesh = 0;
	bool ok = CHR_Build_NodeObject(object, FBX, MA, &with_mesh);
	if (!object.zonefile.empty())
		msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Animated object: " << object.nodes.size() << " nodes, " << with_mesh << " with meshes (" << object.matched << " in " << object.zonefile << " object slot " << object.slot << (object.linked ? ", linked" : ", by mesh names") << ")";
	if (!object.zonefile.empty() && with_mesh < object.nodes.size())
		msg(msg::TGT::FILE, msg::TYP::WARN) << chrname << ".CHR: " << object.nodes.size() - with_mesh << " nodes without meshes.";
	if (!object.zonefile.empty())
		ok &= ZONE_ObjectMaterials(object.zonefile, MA);
	SetCurrentDirectory(AOD_IO.folder_characters_lpwstr);		// \NOMELIVELLO\Characters
	FBX_Export(chrname, FBX);
	MA_Export(chrname, MA);
	return ok;
}
