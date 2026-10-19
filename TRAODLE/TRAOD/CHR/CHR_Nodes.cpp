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
Legge i nodi di un CHR "NODE" e cerca lo slot dei file ZONE del livello che contiene le mesh del maggior numero di nodi,
preferendo a parita' gli slot piazzati da un nodo RMX_OBJECT. Falso se il CHR non e' un oggetto animato. Va chiamata con
la cartella corrente \NOMELIVELLO.
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
	bool best_placed = false;
	for (const pair <const string, ZONE_ObjectData> &z : CHR_ZoneData())
	{
		int zone = CHR_ZoneIndex(z.first);
		for (unsigned int s = 0; s < z.second.slots.size(); s++)
		{
			unsigned int matched = CHR_MatchSlot(z.second, s, object);
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
	map <pair <int, unsigned int>, float> score;
	for (const pair <const string, ZONE_ObjectData> &z : CHR_ZoneData())
		for (unsigned int s = 0; s < z.second.slots.size(); s++)
			for (const CHR_NodeObject &object : objects)
			{
				unsigned int matched = CHR_MatchSlot(z.second, s, object);
				float value = matched + (float)matched / (object.nodes.size() + 1);
				pair <int, unsigned int> key(CHR_ZoneIndex(z.first), s);
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
- mesh con lo stesso nome di un nodo (e la loro collisione, ZONE_OBJECT_INFO.CollisionIndex): figlie del nodo,
  le altre figlie del gruppo radice (parti fisse). I vertici sono nello spazio dell'oggetto;
- posa di riposo: quella dello slot del file ZONE (trasformazione del nodo della lista di comandi che contiene la prima
  mesh del nodo, scomposta con i pivot del nodo), che e' la posa dell'oggetto nel livello;
- mesh con una trasformazione diversa da quella del loro nodo: in un gruppo intermedio (<mesh>_T).
Le collisioni vanno nel layer nascosto <CHR>_Collisions. Va chiamata con la cartella corrente \NOMELIVELLO.
------------------------------------------------------------------------------------------------------------------*/
bool CHR_Build_NodeObject (const CHR_NodeObject &object, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	ZONE_ObjectExport out;
	Transform root;
	root.name = object.name;
	root.translate_flag = root.rotate_flag = root.scale_flag = true;
	out.groups.push_back(root);

	const ZONE_ObjectData *data = nullptr;
	map <string, ZONE_ObjectData>::const_iterator z = CHR_ZoneData().find(object.zonefile);
	if (z != CHR_ZoneData().end() && object.slot < z->second.slots.size())
		data = &z->second;
	vector <pair <int, MATRIX>> meshes;
	if (data)
		meshes = ZONE_SlotMeshes(*data, object.slot);
	vector <int> mesh_node(meshes.size(), -1);						// Nodo di ogni mesh
	for (unsigned int m = 0; m < meshes.size(); m++)
		for (unsigned int n = 0; n < object.nodes.size() && mesh_node[m] < 0; n++)
			if (data->mesh_hash[meshes[m].first] == object.nodes[n].mesh_hash)
				mesh_node[m] = n;
	for (unsigned int m = 0; m < meshes.size(); m++)				// Collisioni delle mesh di un nodo
		for (unsigned int m2 = 0; m2 < meshes.size() && mesh_node[m] < 0; m2++)
			if (mesh_node[m2] >= 0 && data->mesh_link[meshes[m2].first] == meshes[m].first)
				mesh_node[m] = mesh_node[m2];

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
	else
		msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Animated object: " << object.nodes.size() << " nodes, meshes of " << object.matched << " nodes in " << object.zonefile << " object slot " << object.slot;
	FBX_EXPORT FBX;
	MA_EXPORT MA;
	bool ok = CHR_Build_NodeObject(object, FBX, MA);
	if (!object.zonefile.empty())
		ok &= ZONE_ObjectMaterials(object.zonefile, MA);
	SetCurrentDirectory(AOD_IO.folder_characters_lpwstr);		// \NOMELIVELLO\Characters
	FBX_Export(chrname, FBX);
	MA_Export(chrname, MA);
	return ok;
}
