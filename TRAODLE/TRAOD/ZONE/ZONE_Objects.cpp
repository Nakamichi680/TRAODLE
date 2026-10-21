#include "stdafx.h"
#include <map>
#include <set>
#include "MATH/math.h"
#include "TRAOD/ZONE/ZONE_Struct.h"
#include "TRAOD/ZONE/ZONE_Functions.h"
#include "TRAOD/RMX/RMX_Struct.h"
#include "TRAOD/CLN/CLN_Functions.h"
#include "Misc_Functions.h"
#include "TRAOD/CHR/CHR_Functions.h"
#include "AOD_Names.h"
#include "TRAOD/CSS/CSS_Functions.h"


/*------------------------------------------------------------------------------------------------------------------
Oggetti del file ZONE (vedi ZONE_Struct.h): ogni slot (ZONE_PS2_OBJ_SLOTS) e' un oggetto composto da una o piu' mesh del
blocco ZONE_MESH_OBJECT organizzate in una gerarchia di nodi (comandi -2/-3 push/pop, -4 trasformazione del nodo appena
aperto: traslazione + rotazione XYZ in gradi, relativa al nodo padre). Gli oggetti vengono istanziati dai nodi
RMX_PS2_ROOM_OBJ del file RMX (posizione, rotazione XYZ e scala nel mondo, verificato confrontando le bounding box del
file RMX su 7639 istanze) e dai Fakes del file ZONE.
------------------------------------------------------------------------------------------------------------------*/


// Matrice (vettore riga) scala * rotazione X * rotazione Y * rotazione Z * traslazione (ordine di rotazione XYZ di Maya)
MATRIX ZONE_Matrix (const float t[3], const float r[3], const float s[3])
{
	MATRIX m;
	m.m00 = s[0];	m.m11 = s[1];	m.m22 = s[2];
	for (int axis = 0; axis < 3; axis++)
	{
		float c = cosf(r[axis] * (float)M_PI / 180), sn = sinf(r[axis] * (float)M_PI / 180);
		MATRIX rot;
		int i = (axis + 1) % 3, j = (axis + 2) % 3;				// X: (1, 2), Y: (2, 0), Z: (0, 1)
		float *e[4][4] = {{&rot.m00, &rot.m01, &rot.m02, &rot.m03}, {&rot.m10, &rot.m11, &rot.m12, &rot.m13},
						  {&rot.m20, &rot.m21, &rot.m22, &rot.m23}, {&rot.m30, &rot.m31, &rot.m32, &rot.m33}};
		*e[i][i] = c;	*e[i][j] = sn;	*e[j][i] = -sn;	*e[j][j] = c;
		m = mathMulMatrices(m, rot);
	}
	m.m30 = t[0];	m.m31 = t[1];	m.m32 = t[2];
	return m;
}


static Vec3 ZONE_Transform (const Vec3 &p, const MATRIX &m)
{
	return Vec3(p.x * m.m00 + p.y * m.m10 + p.z * m.m20 + m.m30, p.x * m.m01 + p.y * m.m11 + p.z * m.m21 + m.m31, p.x * m.m02 + p.y * m.m12 + p.z * m.m22 + m.m32);
}


void ZONE_ObjectExport::Add (const Vec3 &p)
{
	if (bb_empty)
	{
		bbmin = bbmax = p;
		bb_empty = false;
	}
	bbmin.x = min(bbmin.x, p.x);	bbmin.y = min(bbmin.y, p.y);	bbmin.z = min(bbmin.z, p.z);
	bbmax.x = max(bbmax.x, p.x);	bbmax.y = max(bbmax.y, p.y);	bbmax.z = max(bbmax.z, p.z);
}


void ZONE_ObjectExport::PushTo (FBX_EXPORT &FBX, MA_EXPORT &MA) const
{
	for (const Transform &g : groups)
	{
		FBX.Group.push_back(g);
		MA.Transform.push_back(g);
	}
	for (const Mesh &m : meshes)
	{
		FBX.Geometry.push_back(m);
		MA.Mesh.push_back(m);
	}
	if (collision_attributes.empty())
		return;
	for (const Mesh &m : meshes)											// Layer delle collisioni (nascosto)
		if (m.multimaterial && none_of(MA.Layer.begin(), MA.Layer.end(), [&m](const Layer &l) {return l.name == m.layer; }))
		{
			Layer layer;
			layer.name = m.layer;
			layer.Label_ARGB = 0xFFFDBC2E;
			layer.Visible = false;
			MA.Layer.push_back(layer);
		}
	string folder = AOD_IO.folder_collisions + "Collision_Textures";		// Textures degli attributi (come CLN_Read)
	for (const string &attribute : collision_attributes)
	{
		string material = "ColMat_" + attribute;
		if (any_of(MA.Material.begin(), MA.Material.end(), [&material](const Material &m) {return m.name == material; }))
			continue;
		Material mat;
		mat.name = material;
		mat.Type = Material::TYPE::LAMBERT;
		mat.color = "ColTex_" + attribute;
		MA.Material.push_back(mat);
		Texture tex;
		tex.name = "ColTex_" + attribute;
		tex.filename = folder + "\\ColTex_" + attribute + ".tga";
		MA.Texture.push_back(tex);
		CreateDirectoryA(AOD_IO.folder_collisions.c_str(), NULL);
		CreateDirectoryA(folder.c_str(), NULL);
		SaveTGAResource(CLN_GetResource(attribute), tex.filename);
	}
}


/*------------------------------------------------------------------------------------------------------------------
Lettura di header, offsets delle mesh degli oggetti, numero di vertici di ogni mesh e liste di comandi degli slot
------------------------------------------------------------------------------------------------------------------*/
bool ZONE_ReadObjectData (ifstream &zonefile, string filename, ZONE_ObjectData &data)
{
	data.zonename = filename.substr(filename.find(".Z") + 1);
	zonefile.seekg(0);
	zonefile.read(reinterpret_cast<char*>(&data.header), sizeof(data.header));
	if (!zonefile || data.header.ZONE_ID != 32)
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " is not a valid ZONE file.";
		return false;
	}

	// Mesh degli oggetti: si saltano le stanze del blocco ZONE_MESH
	uint32_t nRooms = 0, nObjects = 0;
	zonefile.seekg(data.header.MESH_PTR);
	zonefile.read(reinterpret_cast<char*>(&nRooms), sizeof(nRooms));
	for (uint32_t r = 0; r < nRooms && zonefile; r++)
	{
		uint32_t room_id, room_size;
		zonefile.read(reinterpret_cast<char*>(&room_id), sizeof(room_id));
		zonefile.read(reinterpret_cast<char*>(&room_size), sizeof(room_size));
		zonefile.seekg(room_size - 4, ios_base::cur);
	}
	zonefile.read(reinterpret_cast<char*>(&nObjects), sizeof(nObjects));
	for (uint32_t o = 0; o < nObjects && zonefile; o++)
	{
		streamoff offset = zonefile.tellg();
		ZONE_MESH_OBJECT_HEADER h;
		zonefile.read(reinterpret_cast<char*>(&h), sizeof(h));
		data.mesh_offsets.push_back(offset);
		data.mesh_vertices.push_back(h.nVertices);
		zonefile.seekg(offset + h.Object_size);
	}

	// Liste di comandi degli slot
	ZONE_PS2_OBJ_HEADER objheader;
	zonefile.seekg(data.header.PS2_OBJ_PTR);
	zonefile.read(reinterpret_cast<char*>(&objheader), sizeof(objheader));
	vector <uint32_t> slot_ptr(objheader.nSlots);
	if (objheader.nSlots)
		zonefile.read(reinterpret_cast<char*>(slot_ptr.data()), slot_ptr.size() * sizeof(uint32_t));
	if (!zonefile)
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Error reading the objects of " << filename;
		return false;
	}
	// Dalle informazioni delle mesh (ZONE_OBJECT_INFO): copie di un'altra mesh (BaseIndex, nessun vertice PC, usano la
	// geometria della mesh base) e collisioni (Flags bit 0, DATA_PTR = ZONE_OBJECT_COLLISION)
	data.mesh_base.assign(data.mesh_offsets.size(), -1);
	data.mesh_collision.assign(data.mesh_offsets.size(), 0);
	data.mesh_hash.assign(data.mesh_offsets.size(), 0);
	data.mesh_link.assign(data.mesh_offsets.size(), -1);
	data.mesh_flags.assign(data.mesh_offsets.size(), 0);
	data.mesh_alias.assign(data.mesh_offsets.size(), 0);
	data.mesh_broken.assign(data.mesh_offsets.size(), 0);
	vector <uint32_t> collision_ptr(data.mesh_offsets.size(), 0);
	for (uint32_t i = 0; i < objheader.nObjects && i < data.mesh_base.size(); i++)
	{
		ZONE_OBJECT_INFO info;
		zonefile.seekg(data.header.PS2_OBJ_PTR + 8 + 4 * (streamoff)objheader.nSlots + 80 * (streamoff)i);
		if (!zonefile.read(reinterpret_cast<char*>(&info), sizeof(info)))
			break;
		data.mesh_hash[i] = info.Hash;
		data.mesh_flags[i] = info.Flags;
		data.mesh_alias[i] = info.ParentHash;
		data.mesh_broken[i] = info.Unknown2;
		if (info.CollisionIndex < data.mesh_offsets.size())
			data.mesh_link[i] = (int)info.CollisionIndex;
		if ((info.Flags & 1) && info.nBatches == 0)			// Collisione: nodo ZONE_OBJECT_COLLISION valido
			collision_ptr[i] = info.DATA_PTR;
		if (info.BaseIndex < data.mesh_offsets.size() && data.mesh_vertices[i] == 0 && data.mesh_vertices[info.BaseIndex] > 0)
			data.mesh_base[i] = (int)info.BaseIndex;
	}
	for (unsigned int i = 0; i < collision_ptr.size(); i++)	// Verifica dei nodi di collisione (22 elementi puntano invece a un lotto PS2)
	{
		ZONE_OBJECT_COLLISION node;
		zonefile.clear();
		zonefile.seekg(collision_ptr[i]);
		if (collision_ptr[i] && zonefile.read(reinterpret_cast<char*>(&node), sizeof(node)) && node.nTriangles > 0 && node.nTriangles < 100000 &&
			node.TLIST_PTR == node.TRIANGLES_PTR + 48 * node.nTriangles)
			data.mesh_collision[i] = collision_ptr[i];
	}
	zonefile.clear();
	data.slots.resize(objheader.nSlots);
	for (uint32_t s = 0; s < objheader.nSlots; s++)
	{
		if (slot_ptr[s] == 0xFFFFFFFF)							// Slot vuoto
			continue;
		zonefile.seekg(slot_ptr[s] + 32);						// Salta la bounding box dell'oggetto
		for (unsigned int guard = 0; guard < 100000; guard++)
		{
			ZONE_SlotCommand cmd;
			zonefile.read(reinterpret_cast<char*>(&cmd.type), sizeof(cmd.type));
			if (!zonefile || cmd.type == -1)
				break;
			if (cmd.type == -4)
				zonefile.read(reinterpret_cast<char*>(cmd.value), sizeof(cmd.value));		// Traslazione + rotazione
			else if (cmd.type >= 0)
				zonefile.seekg(12, ios_base::cur);					// 3 float sempre 0 (padding)
			else if (cmd.type != -2 && cmd.type != -3)
			{
				msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << filename << ": unknown command " << cmd.type << " in object slot " << s << ".";
				break;
			}
			data.slots[s].push_back(cmd);
		}
		if (data.slots[s].empty())								// Slot senza comandi: considerato con un nodo vuoto
			data.slots[s].push_back(ZONE_SlotCommand());
	}
	zonefile.clear();
	return true;
}


/*------------------------------------------------------------------------------------------------------------------
Lettura degli elementi di una mesh degli oggetti (blocco ZONE_MESH_OBJECT): un Mesh per ogni elemento, figlio del gruppo
parent. matrix e' la trasformazione dal gruppo parent al gruppo radice dell'oggetto, usata per la bounding box.
------------------------------------------------------------------------------------------------------------------*/
bool ZONE_ExportMesh (ifstream &zonefile, const ZONE_ObjectData &data, unsigned int mesh, string name, string parent, string layer, const MATRIX &matrix, ZONE_ObjectExport &out)
{
	ZONE_MESH_OBJECT_HEADER h;
	unsigned int source = (mesh < data.mesh_base.size() && data.mesh_base[mesh] >= 0) ? data.mesh_base[mesh] : mesh;	// Copia: geometria della mesh base
	zonefile.seekg(data.mesh_offsets[source]);
	zonefile.read(reinterpret_cast<char*>(&h), sizeof(h));
	streamoff vertex_position = zonefile.tellg();
	streamoff strip_position = vertex_position + (streamoff)h.nVertices * 40;
	streamoff elements_position = strip_position + (streamoff)h.nIndices * 2;
	string mesh_name = AOD_MeshName(data.mesh_hash[mesh]);		// Nome originale della mesh (log di WorldEdit)
	if (!mesh_name.empty())
		name += "_" + AOD_MayaName(mesh_name);
	for (unsigned int el = 0; el < h.nElements; el++)
	{
		ZONE_MESH_ELEMENT element_header;
		zonefile.seekg(elements_position + el * 64);
		zonefile.read(reinterpret_cast<char*>(&element_header), sizeof(element_header));
		stringstream ssname, ssmaterial;
		ssname << name << "_" << el;
		if (element_header.nElement_Indices < 3)				// Elemento senza triangoli
		{
			msg(msg::TGT::FILE, msg::TYP::WARN) << ssname.str() << " does not contain any triangle. Skipped.";
			continue;
		}
		Mesh element;
		element.name = ssname.str();
		element.parent = parent;
		element.layer = layer;
		element.FBX_parent = hashID(parent, "Group");
		if (mesh < data.mesh_flags.size())						// Attributi della mesh (ZONE_OBJECT_INFO)
		{
			string flags;
			const pair <uint32_t, const char*> known[] = {{ZONE_MESHFLAG_ALIAS, "ALIAS"}, {ZONE_MESHFLAG_SOLID, "SOLID"}, {ZONE_MESHFLAG_CAPSULE, "CAPSULE"}, {ZONE_MESHFLAG_LIT, "LIT"}};
			for (const pair <uint32_t, const char*> &k : known)
				if (data.mesh_flags[mesh] & k.first)
					flags += (flags.empty() ? "" : " ") + string(k.second);
			element.int_attributes.push_back(make_pair(string("meshFlags"), (int)data.mesh_flags[mesh]));
			if (!flags.empty())
				element.string_attributes.push_back(make_pair(string("meshFlagsText"), flags));
			if (data.mesh_flags[mesh] & ZONE_MESHFLAG_ALIAS)		// Nome alternativo (mapFindAliasMesh)
			{
				char hash[16];
				snprintf(hash, sizeof(hash), "%08X", data.mesh_alias[mesh]);
				string alias = AOD_MeshName(data.mesh_alias[mesh]);
				element.string_attributes.push_back(make_pair(string("aliasHash"), alias.empty() ? string(hash) : alias));
			}
			if (data.mesh_broken[mesh])
				element.int_attributes.push_back(make_pair(string("nBroken"), (int)data.mesh_broken[mesh]));
		}
		ssmaterial << AOD_IO.levelname << "_" << data.zonename << "_Material_" << element_header.Material_Ref;
		element.material_name = ssmaterial.str();

		// Double sided e shadow map dal materiale
		ZONE_MATERIALS_LIST material;
		zonefile.seekg(data.header.TEXTURE_PTR + 16 + element_header.Material_Ref * 24);
		zonefile.read(reinterpret_cast<char*>(&material.TextureMode), sizeof(material.TextureMode));
		zonefile.read(reinterpret_cast<char*>(&material.DoubleSided), sizeof(material.DoubleSided));
		element.doublesided = CheckDoubleSided(material.TextureMode, material.DoubleSided);
		element.uv_set2_flag = CheckShadowMap(material.TextureMode);

		// Strip e lista dei vertici usati dall'elemento
		vector <unsigned int> strip(element_header.nElement_Indices);
		zonefile.seekg(strip_position + element_header.Offset * 2);
		for (unsigned int i = 0; i < strip.size(); i++)
		{
			uint16_t index = 0;
			zonefile.read(reinterpret_cast<char*>(&index), sizeof(index));
			strip[i] = index;
		}
		vector <unsigned int> vertex_array;
		map <unsigned int, unsigned int> remap;
		for (unsigned int i = 0; i < strip.size(); i++)
		{
			map <unsigned int, unsigned int>::iterator it = remap.find(strip[i]);
			if (it == remap.end())
			{
				it = remap.insert(make_pair(strip[i], (unsigned int)vertex_array.size())).first;
				vertex_array.push_back(strip[i]);
			}
			strip[i] = it->second;
		}
		element.Face = Calculate_Faces(strip, element_header.Offset, element_header.Draw_mode);
		element.nV = vertex_array.size();

		// Vertici
		for (unsigned int v = 0; v < vertex_array.size(); v++)
		{
			ZONE_MESH_VERTEX vertex;
			zonefile.seekg(vertex_position + (streamoff)vertex_array[v] * 40);
			zonefile.read(reinterpret_cast<char*>(&vertex), sizeof(vertex));
			element.X.push_back(vertex.X);
			element.Y.push_back(vertex.Y);
			element.Z.push_back(vertex.Z);
			element.U1.push_back(vertex.U1);
			element.V1.push_back(vertex.V1);
			element.U2.push_back(vertex.U2);
			element.V2.push_back(vertex.V2);
			element.Xn.push_back(((float)vertex.Xn - 128) / 127);
			element.Yn.push_back(((float)vertex.Yn - 128) / 127);
			element.Zn.push_back(((float)vertex.Zn - 128) / 127);
			element.Xtg.push_back(((float)vertex.Xtg - 128) / 127);
			element.Ytg.push_back(((float)vertex.Ytg - 128) / 127);
			element.Ztg.push_back(((float)vertex.Ztg - 128) / 127);
			element.Xbn.push_back(((float)vertex.Xbn - 128) / 127);
			element.Ybn.push_back(((float)vertex.Ybn - 128) / 127);
			element.Zbn.push_back(((float)vertex.Zbn - 128) / 127);
			element.A.push_back(1);
			element.R.push_back((float)vertex.VC_red / 255);
			element.G.push_back((float)vertex.VC_green / 255);
			element.B.push_back((float)vertex.VC_blue / 255);
			out.Add(ZONE_Transform(Vec3(vertex.X, vertex.Y, vertex.Z), matrix));
		}
		out.meshes.push_back(element);
	}
	if (!zonefile)
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Error reading " << name;
		zonefile.clear();
		return false;
	}
	return true;
}


/*------------------------------------------------------------------------------------------------------------------
Lettura della collisione di una mesh degli oggetti (ZONE_OBJECT_COLLISION, stesso formato dei triangoli dei file CLN,
coordinate locali dell'oggetto): una mesh con un materiale per attributo di collisione (ColMat_<attributo>, vedi
CLN_Read) e UV planari per triangolo. I materiali vengono creati da ZONE_ObjectExport::PushTo.
------------------------------------------------------------------------------------------------------------------*/
bool ZONE_ExportCollision (ifstream &zonefile, const ZONE_ObjectData &data, unsigned int mesh, string name, string parent, string layer, const MATRIX &matrix, ZONE_ObjectExport &out)
{
	ZONE_OBJECT_COLLISION node;
	zonefile.seekg(data.mesh_collision[mesh]);
	zonefile.read(reinterpret_cast<char*>(&node), sizeof(node));
	Mesh collision;
	string mesh_name = AOD_MeshName(data.mesh_hash[mesh]);		// Nome originale della mesh di collisione
	collision.name = mesh_name.empty() ? name : name + "_" + AOD_MayaName(mesh_name);
	collision.parent = parent;
	collision.layer = layer;
	collision.FBX_parent = hashID(parent, "Group");
	collision.nV = node.nTriangles * 3;
	collision.uv_set2_flag = collision.normals_flag = collision.tangents_flag = collision.binormals_flag = collision.vcolors_flag = collision.doublesided = false;
	collision.multimaterial = true;
	zonefile.seekg(node.TRIANGLES_PTR);
	for (unsigned int i = 0; i < node.nTriangles; i++)
	{
		Vec3 v1, v2, v3;
		unsigned int MissingAxis, attribute;
		CLN_Get_Triangle(zonefile, v1, v2, v3, MissingAxis, attribute);
		string attribute_name = CLN_GetAttributeName(attribute & 0xFF, (attribute >> 8) & 0x3, (attribute >> 10) & 0xFFF);
		string material = "ColMat_" + attribute_name;
		vector <ObjectGroup>::iterator it = find_if(collision.Groups.begin(), collision.Groups.end(), [&material](const ObjectGroup &g) {return g.material_name == material; });
		if (it == collision.Groups.end())
		{
			ObjectGroup group;
			group.material_name = material;
			collision.Groups.push_back(group);
			it = collision.Groups.end() - 1;
		}
		it->FaceIDs.push_back(i);
		out.collision_attributes.insert(attribute_name);
		for (const Vec3 *v : {&v1, &v2, &v3})
		{
			collision.X.push_back(v->x);
			collision.Y.push_back(v->y);
			collision.Z.push_back(v->z);
			out.Add(ZONE_Transform(*v, matrix));
		}
		Vec3 edge1 = v2 - v1, edge2 = v3 - v1;							// UV planari (come CLN_Read)
		Vec3 normal = edge1.cross(edge2).normalized();
		Vec3 tangent = edge1.normalized();
		Vec3 bitangent = normal.cross(tangent).normalized();
		collision.U1.push_back(0);
		collision.V1.push_back(0);
		collision.U1.push_back(edge1.dot(tangent) / 768.0f);
		collision.V1.push_back(edge1.dot(bitangent) / 768.0f);
		collision.U1.push_back(edge2.dot(tangent) / 768.0f);
		collision.V1.push_back(edge2.dot(bitangent) / 768.0f);
		Face triangle;
		triangle.v1 = i * 3;	triangle.v2 = i * 3 + 1;	triangle.v3 = i * 3 + 2;
		collision.Face.push_back(triangle);
	}
	if (!zonefile)
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Error reading " << name;
		zonefile.clear();
		return false;
	}
	out.meshes.push_back(collision);
	return true;
}


/*------------------------------------------------------------------------------------------------------------------
Esporta uno slot con la sua gerarchia sotto il gruppo radice root (creato dal chiamante): un gruppo per ogni nodo che
contiene mesh (direttamente o nei discendenti) con la sua trasformazione locale, e le mesh con geometria (le copie di
un'altra mesh usano la geometria della mesh base, le collisioni non hanno vertici PC). Bounding box nello spazio di root.
------------------------------------------------------------------------------------------------------------------*/
bool ZONE_ExportSlot (ifstream &zonefile, const ZONE_ObjectData &data, unsigned int slot, string root, string layer, ZONE_ObjectExport &out)
{
	struct Node { int parent; bool has_transform = false; float t[3] = {0, 0, 0}, r[3] = {0, 0, 0}; vector <int> meshes; bool used = false; };
	vector <Node> nodes(1);
	nodes[0].parent = -1;
	vector <int> stack;
	int current = 0;
	for (const ZONE_SlotCommand &cmd : data.slots[slot])
	{
		if (cmd.type == -2)										// Nuovo nodo figlio
		{
			Node node;
			node.parent = current;
			nodes.push_back(node);
			stack.push_back(current);
			current = (int)nodes.size() - 1;
		}
		else if (cmd.type == -3)								// Fine del nodo
		{
			if (!stack.empty())
			{
				current = stack.back();
				stack.pop_back();
			}
		}
		else if (cmd.type == -4)								// Trasformazione del nodo corrente
		{
			nodes[current].has_transform = true;
			memcpy(nodes[current].t, cmd.value, sizeof(nodes[current].t));
			memcpy(nodes[current].r, cmd.value + 3, sizeof(nodes[current].r));
		}
		else if (cmd.type >= 0 && (unsigned int)cmd.type < data.mesh_vertices.size() && (data.mesh_vertices[cmd.type] > 0 || data.mesh_base[cmd.type] >= 0 || data.mesh_collision[cmd.type]))
			nodes[current].meshes.push_back(cmd.type);
	}
	for (unsigned int n = 0; n < nodes.size(); n++)			// Nodi da esportare: con mesh o con discendenti con mesh
		if (!nodes[n].meshes.empty())
			for (int p = n; p >= 0 && !nodes[p].used; p = nodes[p].parent)
				nodes[p].used = true;

	vector <string> names(nodes.size());
	vector <MATRIX> matrices(nodes.size());
	names[0] = root;
	bool ok = true;
	for (unsigned int n = 0; n < nodes.size(); n++)			// I padri precedono sempre i figli
	{
		if (!nodes[n].used)
			continue;
		if (n > 0)
		{
			stringstream name;
			name << root << "_N" << n;
			names[n] = name.str();
			Transform group;
			group.name = names[n];
			group.parent = names[nodes[n].parent];
			group.FBX_parent = hashID(group.parent, "Group");
			group.layer = layer;
			if (nodes[n].has_transform)
			{
				group.translate_flag = group.rotate_flag = true;
				group.tX = nodes[n].t[0];	group.tY = nodes[n].t[1];	group.tZ = nodes[n].t[2];
				group.rX = nodes[n].r[0];	group.rY = nodes[n].r[1];	group.rZ = nodes[n].r[2];
			}
			out.groups.push_back(group);
			float one[3] = {1, 1, 1};
			matrices[n] = mathMulMatrices(ZONE_Matrix(nodes[n].t, nodes[n].r, one), matrices[nodes[n].parent]);
		}
		for (int mesh : nodes[n].meshes)
		{
			stringstream name;
			name << root << "_M" << mesh;
			if (data.mesh_collision[mesh])						// Collisione dell'oggetto
				ok &= ZONE_ExportCollision(zonefile, data, mesh, name.str() + "_COL", names[n], AOD_IO.levelname + "_" + data.zonename + "_Object_Collisions", matrices[n], out);
			else
				ok &= ZONE_ExportMesh(zonefile, data, mesh, name.str(), names[n], layer, matrices[n], out);
		}
	}
	return ok;
}


/*------------------------------------------------------------------------------------------------------------------
Mesh di uno slot con la trasformazione dal nodo che le contiene al gruppo radice dell'oggetto (matrice a vettore riga),
nell'ordine della lista di comandi.
------------------------------------------------------------------------------------------------------------------*/
vector <pair <int, MATRIX>> ZONE_SlotMeshes (const ZONE_ObjectData &data, unsigned int slot)
{
	vector <pair <int, MATRIX>> meshes;
	if (slot >= data.slots.size())
		return meshes;
	vector <MATRIX> stack;
	MATRIX current, parent;
	for (const ZONE_SlotCommand &cmd : data.slots[slot])
	{
		if (cmd.type == -2)										// Nuovo nodo figlio
		{
			stack.push_back(current);
			parent = current;
		}
		else if (cmd.type == -3)								// Fine del nodo
		{
			if (!stack.empty())
			{
				current = stack.back();
				stack.pop_back();
			}
			parent = stack.empty() ? MATRIX() : stack.back();
		}
		else if (cmd.type == -4)								// Trasformazione del nodo corrente (relativa al padre)
		{
			float one[3] = {1, 1, 1};
			current = mathMulMatrices(ZONE_Matrix(cmd.value, cmd.value + 3, one), parent);
		}
		else if (cmd.type >= 0 && (unsigned int)cmd.type < data.mesh_offsets.size())
			meshes.push_back(make_pair(cmd.type, current));
	}
	return meshes;
}


/*------------------------------------------------------------------------------------------------------------------
Materiali e textures del file ZONE usati dalle mesh in MA (es. modello di un oggetto animato esportato a parte): gli
stessi nodi creati da ZONE_Read_Materials e ZONE_Read_Textures, con i file delle textures scritti dall'esportazione
della zona (\NOMELIVELLO\Zones\Z&&_Textures; le DXT5 solo in Originals come DDS). Va chiamata con la cartella corrente
\NOMELIVELLO.
------------------------------------------------------------------------------------------------------------------*/
bool ZONE_ObjectMaterials (string filename, MA_EXPORT &MA)
{
	set <string> used;
	for (const Mesh &m : MA.Mesh)
		used.insert(m.material_name);
	MA_EXPORT all;
	FBX_EXPORT dummy;
	msg::Quiet() = true;
	bool ok = ZONE_Read_Materials(filename, dummy, all);
	msg::Quiet() = false;
	if (!ok)
		return false;
	ifstream zonefile(filename, std::ios::binary);
	ZONE_HEADER header;
	ZONE_MATERIALS_HEADER materials;
	zonefile.read(reinterpret_cast<char*>(&header), sizeof(header));
	zonefile.seekg(header.TEXTURE_PTR);
	zonefile.read(reinterpret_cast<char*>(&materials), sizeof(materials));
	string zonename = filename.substr(filename.find(".Z") + 1);
	string prefix = AOD_IO.levelname + "_" + zonename + "_Texture_";
	set <string> textures;
	for (const Material &mat : all.Material)
		if (used.count(mat.name))
		{
			MA.Material.push_back(mat);
			for (const string *t : {&mat.color, &mat.shadow, &mat.transparency, &mat.bump, &mat.specular, &mat.reflection, &mat.emissive, &mat.displacement})
				if (t->compare(0, prefix.size(), prefix) == 0)
					textures.insert(*t);
		}
	for (const string &name : textures)
	{
		unsigned int t = atoi(name.substr(prefix.size()).c_str());
		uint32_t dxt = 0;
		zonefile.seekg(header.TEXTURE_PTR + 16 + 24 * (streamoff)materials.nMaterials + 40 * (streamoff)t);
		zonefile.read(reinterpret_cast<char*>(&dxt), sizeof(dxt));
		Texture tex;
		tex.name = name;
		string folder = AOD_IO.folder_zones + zonename + "_Textures\\";
		string file = AOD_IO.levelname + "_" + zonename + "_" + to_string(t);
		tex.filename = (dxt == 894720068) ? folder + "Originals\\" + file + ".dds" : folder + file + ".tga";	// DXT5: solo DDS
		MA.Texture.push_back(tex);
	}
	return true;
}


/*------------------------------------------------------------------------------------------------------------------
Informazioni sugli oggetti dell'intero livello (tutti i file ZONE e il file RMX), calcolate una volta sola:
- istanze RMX_PS2_ROOM_OBJ (oggetti statici) e RMX_OBJECT (oggetti interattivi, slot nel campo ZoneSlot) e file ZONE in
  cui esportarle. Le zone dello stesso livello condividono la numerazione degli
  slot (lo stesso oggetto puo' essere presente in piu' zone): ogni istanza viene esportata in una sola zona, la prima
  della ZoneList della stanza (RMX_ROOM.Offset28) in cui lo slot non e' vuoto, altrimenti la prima zona che contiene la
  stanza (ZONE_PS2_ROOM_ENTRY), altrimenti la prima zona in cui lo slot non e' vuoto. Le istanze il cui slot e' vuoto in
  tutte le zone del livello non vengono esportate;
- slot usati da istanze (statiche o interattive) o Fakes in qualunque zona (gli altri sono esportati come Items, nella prima zona in cui lo slot
  non e' vuoto).
Va chiamata con la cartella corrente \NOMELIVELLO.
------------------------------------------------------------------------------------------------------------------*/
const ZONE_LevelObjects &ZONE_GetLevelObjects ()
{
	static ZONE_LevelObjects level;
	static bool loaded = false;
	if (loaded)
		return level;
	loaded = true;

	// Zone del livello: slot non vuoti, stanze PS2, slot usati dai Fakes
	map <int, vector <bool>> slot_full;						// Indice della zona -> slot non vuoti
	map <int, set <uint32_t>> zone_rooms;					// Indice della zona -> stanze
	for (const auto &f : AOD_IO.gmxfiles)
	{
		if (f.type != AoDFileType::ZONE)
			continue;
		size_t z = f.name.rfind(".Z");
		if (z == string::npos || f.name.size() != z + 4 || !isdigit((unsigned char)f.name[z + 2]) || !isdigit((unsigned char)f.name[z + 3]))
			continue;
		int index = atoi(f.name.substr(z + 2).c_str());
		ifstream zonefile(f.name, std::ios::binary);
		ZONE_HEADER header;
		if (!zonefile.read(reinterpret_cast<char*>(&header), sizeof(header)) || header.ZONE_ID != 32)
			continue;
		uint32_t nRooms = 0;
		zonefile.read(reinterpret_cast<char*>(&nRooms), sizeof(nRooms));
		for (uint32_t r = 0; r < nRooms && zonefile; r++)
		{
			ZONE_PS2_ROOM_ENTRY entry;
			zonefile.read(reinterpret_cast<char*>(&entry), sizeof(entry));
			zone_rooms[index].insert(entry.RoomID);
		}
		ZONE_PS2_OBJ_HEADER objheader;
		zonefile.seekg(header.PS2_OBJ_PTR);
		zonefile.read(reinterpret_cast<char*>(&objheader), sizeof(objheader));
		vector <uint32_t> slot_ptr(objheader.nSlots);
		if (objheader.nSlots)
			zonefile.read(reinterpret_cast<char*>(slot_ptr.data()), slot_ptr.size() * sizeof(uint32_t));
		vector <bool> &full = slot_full[index];
		for (uint32_t ptr : slot_ptr)
			full.push_back(ptr != 0xFFFFFFFF);
		uint32_t fake = 0;											// Fakes
		zonefile.seekg(header.EOF_PTR);
		zonefile.read(reinterpret_cast<char*>(&fake), sizeof(fake));
		for (unsigned int guard = 0; zonefile && fake && guard < 100000; guard++)
		{
			uint16_t slot = 0;
			uint32_t next = 0;
			zonefile.seekg(fake + 4);
			zonefile.read(reinterpret_cast<char*>(&next), sizeof(next));
			zonefile.seekg(fake + 126);
			zonefile.read(reinterpret_cast<char*>(&slot), sizeof(slot));
			level.used_slots.insert(slot);
			fake = next;
		}
	}
	auto SlotFull = [&](int zone, unsigned int slot) { return slot_full.count(zone) && slot < slot_full[zone].size() && slot_full[zone][slot]; };

	// Istanze del file RMX
	ifstream rmxfile(AOD_IO.levelname + ".RMX", std::ios::binary);
	RMX_HEADER rmx_header;
	unsigned int skipped = 0;
	if (rmxfile.read(reinterpret_cast<char*>(&rmx_header), sizeof(rmx_header)))
		for (uint32_t r = 0; r < rmx_header.nRooms; r++)
		{
			uint32_t room_offset = 0;
			rmxfile.seekg(sizeof(RMX_HEADER) + 4 * r);
			rmxfile.read(reinterpret_cast<char*>(&room_offset), sizeof(room_offset));
			RMX_ROOM room;
			rmxfile.seekg(room_offset);
			if (!rmxfile.read(reinterpret_cast<char*>(&room), sizeof(room)))
				break;
			vector <int> zone_list(min(room.Offset29, 16u));
			rmxfile.seekg(room_offset + room.Offset28);
			for (int &z : zone_list)
			{
				uint32_t value = 0;
				rmxfile.read(reinterpret_cast<char*>(&value), sizeof(value));
				z = (int)value;
			}
			auto AddInstance = [&](ZONE_Instance instance)		// Zona: ZoneList, zona che contiene la stanza, qualunque zona
			{
				instance.room = r;
				instance.zone = -1;
				for (int z : zone_list)
					if (instance.zone < 0 && SlotFull(z, instance.slot))
						instance.zone = z;
				for (const pair <const int, set <uint32_t>> &z : zone_rooms)
					if (instance.zone < 0 && z.second.count(room.Room_ID) && SlotFull(z.first, instance.slot))
						instance.zone = z.first;
				for (const pair <const int, vector <bool>> &z : slot_full)
					if (instance.zone < 0 && SlotFull(z.first, instance.slot))
						instance.zone = z.first;
				if (instance.zone >= 0)
				{
					level.instances.push_back(instance);
					level.used_slots.insert(instance.slot);
				}
				else
					skipped++;
			};
			uint32_t node = room.P11_PS2_Room_Obj_First;			// Oggetti statici (RMX_PS2_ROOM_OBJ)
			for (unsigned int guard = 0; node && guard < 100000; guard++)
			{
				RMX_PS2_ROOM_OBJ obj;
				rmxfile.seekg(room_offset + node);
				if (!rmxfile.read(reinterpret_cast<char*>(&obj), sizeof(obj)))
					break;
				ZONE_Instance instance;
				instance.pos[0] = obj.Xpos;	instance.pos[1] = obj.Ypos;	instance.pos[2] = obj.Zpos;
				instance.rot[0] = obj.Xrot;	instance.rot[1] = obj.Yrot;	instance.rot[2] = obj.Zrot;
				instance.scl[0] = obj.Xscal;	instance.scl[1] = obj.Yscal;	instance.scl[2] = obj.Zscal;
				instance.slot = obj.ZoneSlot;
				instance.hash = obj.Unknown3;
				AddInstance(instance);
				if (obj.BegNext <= node)
					break;
				node = obj.BegNext;
			}
			node = room.NL_OBJ_First;								// Oggetti interattivi (RMX_OBJECT)
			for (unsigned int guard = 0; node && guard < 100000; guard++)
			{
				RMX_OBJECT obj;
				rmxfile.seekg(room_offset + node);
				if (!rmxfile.read(reinterpret_cast<char*>(&obj), sizeof(obj)))
					break;
				ZONE_Instance instance;
				instance.pos[0] = obj.Node.Xpos;	instance.pos[1] = obj.Node.Ypos;	instance.pos[2] = obj.Node.Zpos;
				instance.rot[0] = obj.Node.Xrot;	instance.rot[1] = obj.Node.Yrot;	instance.rot[2] = obj.Node.Zrot;
				instance.scl[0] = obj.Node.Xscal;	instance.scl[1] = obj.Node.Yscal;	instance.scl[2] = obj.Node.Zscal;
				instance.slot = obj.ZoneSlot;
				instance.interactive = true;
				instance.object_index = obj.ObjectIndex;
				instance.obj_flags = obj.ObjFlags;
				instance.db_key = obj.absDBKey;
				instance.n_elements = obj.nElements;
				instance.hash = obj.Node.Hash;
				level.nTemplates = max(level.nTemplates, (unsigned int)obj.ObjectIndex + 1);
				AddInstance(instance);
				if (obj.Node.BegNext <= node)
					break;
				node = obj.Node.BegNext;
			}
		}
	if (skipped)
		msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << skipped << " object instances of " << AOD_IO.levelname << ".RMX use object slots that are empty in all the ZONE files of the level. Skipped.";

	// Slot non usati: esportati come Items nella prima zona in cui non sono vuoti
	for (const pair <const int, vector <bool>> &z : slot_full)
		for (unsigned int s = 0; s < z.second.size(); s++)
			if (z.second[s] && !level.used_slots.count(s) && !level.item_zone.count(s))
				level.item_zone[s] = z.first;
	return level;
}


/*------------------------------------------------------------------------------------------------------------------
Esporta le istanze assegnate a questa zona: un gruppo per istanza (posizione, rotazione e scala nel mondo) con la
gerarchia dello slot. Layer: NOMELIVELLO_Z&&_Objects per gli oggetti statici (RMX_PS2_ROOM_OBJ, gruppi _OBJ_slot_n),
NOMELIVELLO_Z&&_Interactive per gli oggetti interattivi (RMX_OBJECT, gruppi _IOBJ_slot_n, con gli attributi
objectIndex e objectTemplate = Object Template dell'oggetto, se noto). Tutte le istanze hanno l'attributo rmxNode = nome
originale del nodo RMX (o il suo hash se non e' noto), aggiunto anche al nome del gruppo. Se lo slot contiene le mesh di un oggetto animato (CHR "NODE"),
l'attributo animatedObject ne indica il nome (modello in \Characters, animazioni in \Animations).
------------------------------------------------------------------------------------------------------------------*/
bool ZONE_Read_Instances (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	ifstream zonefile(filename, std::ios::binary);
	if (!zonefile.is_open())
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " not found.";
		return false;
	}
	ZONE_ObjectData data;
	if (!ZONE_ReadObjectData(zonefile, filename, data))
		return false;
	int zone_index = atoi(data.zonename.substr(1).c_str());
	const ZONE_LevelObjects &level = ZONE_GetLevelObjects();

	Layer layers[2];
	layers[0].name = AOD_IO.levelname + "_" + data.zonename + "_Objects";
	layers[0].Label_ARGB = 0xFF2E86C1;
	layers[0].Visible = false;								// Oggetti statici: la versione PC li contiene gia' nelle mesh delle stanze
	layers[1].name = AOD_IO.levelname + "_" + data.zonename + "_Interactive";
	layers[1].Label_ARGB = 0xFFC0392B;
	map <unsigned int, unsigned int> copies[2];				// Numero di istanze per slot (per i nomi)
	unsigned int exported[2] = {0, 0};
	bool ok = true;
	for (const ZONE_Instance &instance : level.instances)
	{
		if (instance.zone != zone_index || instance.slot >= data.slots.size() || data.slots[instance.slot].empty())
			continue;
		int type = instance.interactive ? 1 : 0;
		stringstream name;
		name << AOD_IO.levelname << "_" << data.zonename << (type ? "_IOBJ_" : "_OBJ_") << instance.slot << "_" << copies[type][instance.slot]++;
		string node_name = AOD_NodeName(instance.hash);			// Nome originale del nodo RMX
		if (!node_name.empty())
			name << "_" << AOD_MayaName(node_name);
		ZONE_ObjectExport object;
		Transform root;
		root.name = name.str();
		root.layer = layers[type].name;
		root.translate_flag = root.rotate_flag = root.scale_flag = true;
		root.tX = instance.pos[0];	root.tY = instance.pos[1];	root.tZ = instance.pos[2];
		root.rX = instance.rot[0];	root.rY = instance.rot[1];	root.rZ = instance.rot[2];
		root.sX = instance.scl[0];	root.sY = instance.scl[1];	root.sZ = instance.scl[2];
		root.int_attributes.push_back(make_pair(string("objectSlot"), (int)instance.slot));		// Slot dell'oggetto e stanza RMX
		root.int_attributes.push_back(make_pair(string("rmxRoom"), (int)instance.room));
		char hash[16];
		snprintf(hash, sizeof(hash), "%08X", instance.hash);
		root.string_attributes.push_back(make_pair(string("rmxNode"), node_name.empty() ? string(hash) : node_name));	// Nome del nodo RMX, altrimenti hash
		string cutscenes = CS_CutscenesHidingNode(instance.hash);	// Filmati che nascondono l'oggetto (#MAP WORLD OFF)
		if (!cutscenes.empty())
			root.string_attributes.push_back(make_pair(string("cutsceneOff"), cutscenes));
		if (instance.interactive)
		{
			root.int_attributes.push_back(make_pair(string("objectIndex"), (int)instance.object_index));
			root.int_attributes.push_back(make_pair(string("objFlags"), (int)instance.obj_flags));	// RMX_OBJECT.ObjFlags
			string flags_text;													// Bit noti (dal codice del gioco)
			const pair <uint32_t, const char*> known[] = {{0x10, "ROCKING"}, {0x10000, "NOPHYSICS"}, {0x20000, "OPENABLE"}, {0x40000, "OPEN"}, {0x1000000, "SWITCHON"}};
			for (const pair <uint32_t, const char*> &k : known)
				if (instance.obj_flags & k.first)
					flags_text += (flags_text.empty() ? "" : " ") + string(k.second);
			if (!flags_text.empty())
				root.string_attributes.push_back(make_pair(string("objFlagsText"), flags_text));
			root.int_attributes.push_back(make_pair(string("dbKey"), (int)instance.db_key));		// Chiave nel database degli oggetti
			root.int_attributes.push_back(make_pair(string("nElements"), (int)instance.n_elements));
			string template_name = AOD_TemplateName(instance.object_index, level.nTemplates);
			if (!template_name.empty())
				root.string_attributes.push_back(make_pair(string("objectTemplate"), template_name));	// Object Template (log di WorldEdit)
		}
		map <pair <int, unsigned int>, string>::const_iterator animated = CHR_GetNodeObjectSlots().find(make_pair(zone_index, instance.slot));
		if (animated != CHR_GetNodeObjectSlots().end())			// Oggetto animato (CHR "NODE") con le mesh di questo slot
			root.string_attributes.push_back(make_pair(string("animatedObject"), animated->second));
		object.groups.push_back(root);
		ok &= ZONE_ExportSlot(zonefile, data, instance.slot, root.name, layers[type].name, object);
		object.PushTo(FBX, MA);
		exported[type]++;
	}
	for (int type = 0; type < 2; type++)
		if (exported[type])
			MA.Layer.push_back(layers[type]);
	if (exported[0] || exported[1])
		msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Object instances: " << exported[0] << " static, " << exported[1] << " interactive";
	return ok;
}
