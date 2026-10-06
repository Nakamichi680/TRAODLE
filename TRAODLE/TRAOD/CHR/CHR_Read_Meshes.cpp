#include "stdafx.h"
#include <map>
#include "MATH/math.h"
#include "TRAOD/CHR/CHR_Functions.h"
#include "TRAOD/ZONE/ZONE_Functions.h"


namespace {

// Vertice gia' trasformato nella posa di bind, con le influenze delle bones
struct CHR_Vertex {
	Vec3 pos, normal;
	float u, v;
	unsigned int bone1, bone2;
	float weight1;					// Peso della bone 1 (il peso della bone 2 e' 1 - weight1)
	bool valid = true;				// Falso se il vertice non ha una bone valida (indice 0xFF, posizione nulla): i triangoli che lo usano vengono scartati
	vector <Vec3> morph;			// Offset della posizione per ogni target del blend shape (coordinate del mondo), vuoto se assente
};


Vec3 TransformVector (const MATRIX &m, float x, float y, float z)		// Solo rotazione (offset dei blend shapes)
{
	return Vec3(x * m.m00 + y * m.m10 + z * m.m20, x * m.m01 + y * m.m11 + z * m.m21, x * m.m02 + y * m.m12 + z * m.m22);
}


Vec3 TransformPoint (const MATRIX &m, float x, float y, float z)
{
	return Vec3(x * m.m00 + y * m.m10 + z * m.m20 + m.m30, x * m.m01 + y * m.m11 + z * m.m21 + m.m31, x * m.m02 + y * m.m12 + z * m.m22 + m.m32);
}


Vec3 TransformNormal (const MATRIX &m, float x, float y, float z)
{
	Vec3 n(x * m.m00 + y * m.m10 + z * m.m20, x * m.m01 + y * m.m11 + z * m.m21, x * m.m02 + y * m.m12 + z * m.m22);
	float len = sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
	return (len > 0) ? n / len : n;
}


/*------------------------------------------------------------------------------------------------------------------
Legge strip ed elementi di una mesh (MESH1 o MESH2) e crea una Mesh per ogni elemento, con skinning sulle bones
dello scheletro. I vertici in ingresso sono gia' nella posa di bind (coordinate del mondo).
Se target_names non e' vuoto i vertici contengono gli offset dei blend shapes, che vengono aggiunti ad ogni Mesh.
------------------------------------------------------------------------------------------------------------------*/
bool ReadElements (ifstream &chrfile, const vector <CHR_Vertex> &vertices, string basename, string chrname, const CHR_Skeleton &skeleton, const vector <string> &target_names, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	CHR_MESH_STRIP_HEADER chr_mesh_strip_header;
	CHR_MESH_ELEMENT_HEADER chr_mesh_element_header;
	chrfile.read(reinterpret_cast<char*>(&chr_mesh_strip_header.nIndices), sizeof(chr_mesh_strip_header.nIndices));
	vector <uint16_t> strip(chr_mesh_strip_header.nIndices);
	if (!strip.empty())
		chrfile.read(reinterpret_cast<char*>(strip.data()), strip.size() * sizeof(uint16_t));
	chrfile.read(reinterpret_cast<char*>(&chr_mesh_element_header.nElements), sizeof(chr_mesh_element_header.nElements));
	vector <CHR_MESH_ELEMENT> elements(chr_mesh_element_header.nElements);
	if (!elements.empty())
		chrfile.read(reinterpret_cast<char*>(elements.data()), elements.size() * sizeof(CHR_MESH_ELEMENT));
	if (!chrfile)
		return false;

	for (unsigned int el = 0; el < elements.size(); el++)
	{
		const CHR_MESH_ELEMENT &element = elements[el];
		if (element.nElement_Indices < 3 || element.Offset < 0 || (size_t)element.Offset + element.nElement_Indices > strip.size())
		{
			msg(msg::TGT::FILE, msg::TYP::WARN) << basename << " element " << el << " has no valid triangles. Skipped.";
			continue;
		}

		// Indici dell'elemento rinumerati: i vertici usati diventano consecutivi
		map <unsigned int, unsigned int> remap;
		vector <unsigned int> vertex_array, local_strip;
		for (int i = 0; i < element.nElement_Indices; i++)
		{
			unsigned int index = strip[element.Offset + i];
			if (index >= vertices.size())
			{
				msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << basename << " element " << el << " references a missing vertex.";
				return false;
			}
			map <unsigned int, unsigned int>::iterator it = remap.find(index);
			if (it == remap.end())
			{
				it = remap.insert(make_pair(index, (unsigned int)vertex_array.size())).first;
				vertex_array.push_back(index);
			}
			local_strip.push_back(it->second);
		}

		Mesh mesh;
		stringstream ssname, ssmaterial;
		ssname << basename << "_" << el;
		ssmaterial << AOD_IO.levelname << "_" << chrname << "_Material_" << element.Material_Ref;
		mesh.name = ssname.str();
		mesh.parent = chrname;
		mesh.FBX_parent = hashID(chrname, "Group");
		mesh.material_name = ssmaterial.str();
		mesh.uv_set2_flag = false;
		mesh.tangents_flag = false;
		mesh.binormals_flag = false;
		mesh.vcolors_flag = false;
		vector <Face> faces = Calculate_Faces(local_strip, element.Offset, element.Draw_mode);
		unsigned int discarded = 0;
		for (unsigned int f = 0; f < faces.size(); f++)
			if (vertices[vertex_array[faces[f].v1]].valid && vertices[vertex_array[faces[f].v2]].valid && vertices[vertex_array[faces[f].v3]].valid)
				mesh.Face.push_back(faces[f]);
			else
				discarded++;
		if (discarded)
			msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << basename << " element " << el << ": " << discarded << " triangles use vertices without a valid bone. Discarded.";
		mesh.nV = vertex_array.size();

		// Vertici e skinning: un SkinCluster per ogni bone usata dall'elemento
		map <unsigned int, unsigned int> cluster_of_bone;
		auto AddWeight = [&](unsigned int bone, unsigned int vertex, float weight)
		{
			if (weight <= 0)
				return;
			map <unsigned int, unsigned int>::iterator it = cluster_of_bone.find(bone);
			if (it == cluster_of_bone.end())
			{
				SkinCluster cluster;
				cluster.joint = skeleton.joint_name[bone];
				mesh.Skin.push_back(cluster);
				it = cluster_of_bone.insert(make_pair(bone, (unsigned int)mesh.Skin.size() - 1)).first;
			}
			mesh.Skin[it->second].Vertex.push_back(vertex);
			mesh.Skin[it->second].Weight.push_back(weight);
		};
		for (unsigned int v = 0; v < vertex_array.size(); v++)
		{
			const CHR_Vertex &vertex = vertices[vertex_array[v]];
			mesh.X.push_back(vertex.pos.x);		mesh.Y.push_back(vertex.pos.y);		mesh.Z.push_back(vertex.pos.z);
			mesh.Xn.push_back(vertex.normal.x);	mesh.Yn.push_back(vertex.normal.y);	mesh.Zn.push_back(vertex.normal.z);
			mesh.U1.push_back(vertex.u);		mesh.V1.push_back(vertex.v);
			if (vertex.bone1 == vertex.bone2)
				AddWeight(vertex.bone1, v, 1);
			else
			{
				AddWeight(vertex.bone1, v, vertex.weight1);
				AddWeight(vertex.bone2, v, 1 - vertex.weight1);
			}
		}

		// Blend shapes: offset dei vertici dell'elemento per ogni target
		for (unsigned int t = 0; t < target_names.size(); t++)
		{
			BlendShapeTarget target;
			target.name = target_names[t];
			for (unsigned int v = 0; v < vertex_array.size(); v++)
			{
				const Vec3 &d = vertices[vertex_array[v]].morph[t];
				target.dX.push_back(d.x);	target.dY.push_back(d.y);	target.dZ.push_back(d.z);
			}
			mesh.BlendShape.push_back(target);
		}
		FBX.Geometry.push_back(mesh);
		MA.Mesh.push_back(mesh);
	}
	return true;
}

}		// namespace


/*------------------------------------------------------------------------------------------------------------------
Legge il blocco mesh del file CHR:
 - MESH1: mesh deformata, ogni vertice ha una posizione nello spazio di ciascuna delle sue due bones
 - MESH2: mesh rigide, ognuna nello spazio di una sola bone
I vertici vengono portati nella posa di bind (coordinate del mondo) usando le matrici globali dello scheletro.
------------------------------------------------------------------------------------------------------------------*/
bool CHR_Read_Meshes (ifstream &chrfile, const CHR_HEADER &chr_header, string chrname, const CHR_Skeleton &skeleton, const map <uint32_t, CHR_Morph> &morphs, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	const vector <string> no_targets;
	CHR_MESH12_HEADER chr_mesh12_header;
	CHR_MESH1_HEADER chr_mesh1_header;
	CHR_MESH2_LIST chr_mesh2_list;
	CHR_MESH2_HEADER chr_mesh2_header;
	const unsigned int nBones = chr_header.nBONES;

	chrfile.seekg(chr_header.MESH1_PTR);
	chrfile.read(reinterpret_cast<char*>(&chr_mesh12_header), sizeof(chr_mesh12_header));
	streamoff mesh_end = chrfile.tellg() + (streamoff)chr_mesh12_header.SizeMESH12;

	///////////////////    MESH1
	chrfile.read(reinterpret_cast<char*>(&chr_mesh1_header), sizeof(chr_mesh1_header));
	vector <CHR_MESH1_VERTEX> raw1(chr_mesh1_header.nVertices);
	if (!raw1.empty())
		chrfile.read(reinterpret_cast<char*>(raw1.data()), raw1.size() * sizeof(CHR_MESH1_VERTEX));
	if (!chrfile)
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Error reading MESH1 vertices.";
		return false;
	}
	vector <CHR_Vertex> vertices(raw1.size());
	for (unsigned int v = 0; v < raw1.size(); v++)
	{
		const CHR_MESH1_VERTEX &r = raw1[v];
		if (r.Bone1 >= nBones || r.Bone2 >= nBones)		// Vertice senza bone valida (in COP_A.CHR: indice 0xFF e posizione nulla)
		{
			vertices[v].valid = false;
			vertices[v].bone1 = vertices[v].bone2 = 0;
			vertices[v].weight1 = 1;
			vertices[v].u = r.U / 32767.0f;
			vertices[v].v = r.V / 32767.0f;
			continue;
		}
		float w1 = r.Weight / 32767.0f;
		if (r.Bone1 == r.Bone2)
			w1 = 1;
		Vec3 p1 = TransformPoint(skeleton.global[r.Bone1], r.X1 / 16.0f, r.Y1 / 16.0f, r.Z1 / 16.0f);
		Vec3 p2 = TransformPoint(skeleton.global[r.Bone2], r.X2 / 16.0f, r.Y2 / 16.0f, r.Z2 / 16.0f);
		Vec3 n1 = TransformNormal(skeleton.global[r.Bone1], r.X1n / 127.0f, r.Y1n / 127.0f, r.Z1n / 127.0f);
		Vec3 n2 = TransformNormal(skeleton.global[r.Bone2], r.X2n / 127.0f, r.Y2n / 127.0f, r.Z2n / 127.0f);
		CHR_Vertex &out = vertices[v];
		out.pos = p1 * w1 + p2 * (1 - w1);
		out.normal = n1 * w1 + n2 * (1 - w1);
		float len = sqrt(out.normal.x * out.normal.x + out.normal.y * out.normal.y + out.normal.z * out.normal.z);
		if (len > 0)
			out.normal = out.normal / len;
		out.u = r.U / 32767.0f;
		out.v = r.V / 32767.0f;
		out.bone1 = r.Bone1;
		out.bone2 = r.Bone2;
		out.weight1 = w1;
	}
	stringstream mesh1_name;
	mesh1_name << chrname << "_MESH1";
	if (!ReadElements(chrfile, vertices, mesh1_name.str(), chrname, skeleton, no_targets, FBX, MA))
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Error reading MESH1.";
		return false;
	}

	///////////////////    MESH2
	chrfile.read(reinterpret_cast<char*>(&chr_mesh2_list.nMeshes2), sizeof(chr_mesh2_list.nMeshes2));
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Number of meshes: 1 deformed (" << chr_mesh1_header.nVertices << " vertices) + " << chr_mesh2_list.nMeshes2 << " rigid";
	map <unsigned int, unsigned int> meshes_per_bone;					// Per numerare le MESH2 collegate alla stessa bone
	for (unsigned int m = 0; m < chr_mesh2_list.nMeshes2; m++)
	{
		chrfile.read(reinterpret_cast<char*>(&chr_mesh2_header), sizeof(chr_mesh2_header));
		if (!chrfile || chr_mesh2_header.Bone_ref >= nBones)
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Error reading MESH2 " << m << ".";
			return false;
		}
		vector <CHR_MESH2_VERTEX> raw2(chr_mesh2_header.nVertices);
		if (!raw2.empty())
			chrfile.read(reinterpret_cast<char*>(raw2.data()), raw2.size() * sizeof(CHR_MESH2_VERTEX));
		const MATRIX &g = skeleton.global[chr_mesh2_header.Bone_ref];
		vector <CHR_Vertex> vertices2(raw2.size());
		for (unsigned int v = 0; v < raw2.size(); v++)
		{
			const CHR_MESH2_VERTEX &r = raw2[v];
			CHR_Vertex &out = vertices2[v];
			out.pos = TransformPoint(g, r.X / 16.0f, r.Y / 16.0f, r.Z / 16.0f);
			out.normal = TransformNormal(g, r.Xn / 127.0f, r.Yn / 127.0f, r.Zn / 127.0f);
			out.u = r.U / 32767.0f;
			out.v = r.V / 32767.0f;
			out.bone1 = out.bone2 = chr_mesh2_header.Bone_ref;
			out.weight1 = 1;
		}
		// Nome: bone di appartenenza + numero progressivo delle mesh di quella bone
		stringstream mesh2_name;
		mesh2_name << skeleton.joint_name[chr_mesh2_header.Bone_ref] << "_MESH2_" << meshes_per_bone[chr_mesh2_header.Bone_ref]++;

		// Blend shapes: TMT il cui BaseId e' l'ID di questa mesh (stessi vertici nello stesso ordine, vedi TMT_Struct.h)
		vector <string> target_names;
		map <uint32_t, CHR_Morph>::const_iterator morph = morphs.find(chr_mesh2_header.ID);
		if (morph != morphs.end())
		{
			if (morph->second.nVertices != raw2.size())
				msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << morph->second.name << ".TMT: " << morph->second.nVertices << " vertices instead of " << raw2.size() << " (" << mesh2_name.str() << "). Blend shapes not exported.";
			else
			{
				for (unsigned int t = 0; t < morph->second.nTargets; t++)
					target_names.push_back("Morph" + to_string(t + 1));
				for (unsigned int v = 0; v < raw2.size(); v++)
					for (unsigned int t = 0; t < morph->second.nTargets; t++)
					{
						const TMT_VERTEX &d = morph->second.target[t][v];
						Vec3 w = TransformVector(g, d.X, d.Y, d.Z);
						if (fabs(w.x) < 1e-5f) w.x = 0;		// Gli offset nulli restano nulli dopo la rotazione
						if (fabs(w.y) < 1e-5f) w.y = 0;
						if (fabs(w.z) < 1e-5f) w.z = 0;
						vertices2[v].morph.push_back(w);
					}
				morph->second.used = true;
				morph->second.targets = target_names;
				msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Blend shapes: " << morph->second.name << ".TMT (" << morph->second.nTargets << " targets) applied to " << mesh2_name.str();
			}
		}
		size_t first_mesh = FBX.Geometry.size();
		if (!ReadElements(chrfile, vertices2, mesh2_name.str(), chrname, skeleton, target_names, FBX, MA))
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Error reading MESH2 " << m << ".";
			return false;
		}
		if (!target_names.empty())								// Mesh con il blend shape (per le animazioni facciali)
			for (size_t g = first_mesh; g < FBX.Geometry.size(); g++)
				morph->second.meshes.push_back(FBX.Geometry[g].name);
	}
	if (chrfile.tellg() != mesh_end)
		msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << "Mesh block size mismatch (" << chrfile.tellg() << " instead of " << mesh_end << ").";
	return true;
}
