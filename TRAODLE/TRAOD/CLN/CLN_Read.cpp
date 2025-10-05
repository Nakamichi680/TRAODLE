#include "stdafx.h"
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "TRAOD/CLN/CLN_Struct.h"
#include "Misc_Functions.h"
#include "TRAOD/CLN/CLN_Functions.h"
#include "resource.h"


class Octant
{
public:
	string name = "0";				// Valore di default per primo ottante
	string parent = "";				// Valore di default per primo ottante
	XYZ Vmin;
	XYZ Vmax;
	int Ptr_Octant = 0;				// Valore di default per primo ottante
	int Ptr_TList = -1;				// Valore di default per ottanti con discendenti (vanno ignorati ai fini della lettura dei triangoli)
};


bool CLN_Read (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	//if (SaveTGAResource(IDR_COLLISION_STONE2, "FANCULO.tga"))

	CLN_OCTREE cln_octree;
	CLN_TLIST cln_tlist;
	vector <Octant> octree (1);

	ifstream clnfile(filename, std::ios::binary);
	if (!clnfile.is_open())
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " not found.";
		return false;
	}

	clnfile.read(reinterpret_cast<char*>(&cln_octree.PtrToParent), sizeof(cln_octree.PtrToParent));		// Il primo PtrToParent è sempre FFFFFFFFh
	
	if (cln_octree.PtrToParent != 4294967295)		// Se il file CLN non è valido
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " is not a valid CLN file.";
		return false;
	}

	// Memorizzazione inizio blocco CLN_TRIANGLE
	clnfile.read(reinterpret_cast<char*>(&cln_octree.Ptr_TList), sizeof(cln_octree.Ptr_TList));			// Dimensione del blocco CLN_TRIANGLE
	clnfile.seekg(20, ios_base::cur);
	clnfile.read(reinterpret_cast<char*>(&cln_octree.nDescendants), sizeof(cln_octree.nDescendants));
	clnfile.seekg(80 * (cln_octree.nDescendants + 1));
	streamoff triangle_position = clnfile.tellg();

	msg(msg::TGT::FILE, msg::TYP::LOG) << "Number of patches: " << cln_octree.nDescendants + 1;
	msg(msg::TGT::FILE, msg::TYP::LOG) << "Number of triangles: " << cln_octree.Ptr_TList / 48;

	ofstream clndebug;
	clndebug.open("clndebug.txt");
	clndebug << endl << filename << ": n collision triangles: " << cln_octree.Ptr_TList / 48;
	clndebug << endl << "                                            ";
	//msg(msg::TGT::FILE, msg::TYP::DBG) << "                                            ";
	for (unsigned int f = 0; f < 100; f++)
	{
		//msg(msg::TGT::FILE, msg::TYP::OVR) << right << setw(7) << f;
		clndebug << right << setw(7) << f;
	}
	clndebug << endl;

	// Creazione layer collisioni per file Maya ASCII
	stringstream layername1, layername2;
	layername1 << AOD_IO.levelname << "_Collisions";
	layername2 << layername1.str() << "_octree";
	Layer collisions_layer, octree_layer;
	collisions_layer.name = layername1.str();
	collisions_layer.Label_ARGB = 0xFFFDBC2E;
	octree_layer.name = layername2.str();
	octree_layer.Label_ARGB = 0xFFFDBC2E;
	MA.Layer.push_back(collisions_layer);			// Layer collisioni in mesh singola
	MA.Layer.push_back(octree_layer);				// Layer octree/collisioni in mesh multiple

	vector <unsigned short> vtype, vpadding1, vpadding2, vcomposition, vmapping, vbitattrib;



	// LETTURA COLLISIONI IN BASE ALL'OCTREE ED ESPORTAZIONE IN MESH MULTIPLE
	// Lettura octree
	for (unsigned int o = 0; o < octree.size(); o++)
	{
		clnfile.seekg(octree[o].Ptr_Octant);																// Posiziona l'indice di lettura all'inizio dell'ottante
		clnfile.read(reinterpret_cast<char*>(&cln_octree.PtrToParent), sizeof(cln_octree.PtrToParent));		// PtrToParent
		clnfile.read(reinterpret_cast<char*>(&octree[o].Ptr_TList), sizeof(cln_octree.Ptr_TList));			// Puntatore a inizio lista triangoli per quell'ottante
		clnfile.seekg(4, ios_base::cur);																	// Salta Unknown2
		clnfile.read(reinterpret_cast<char*>(&cln_octree.nChildren), sizeof(cln_octree.nChildren));
		clnfile.read(reinterpret_cast<char*>(&cln_octree.ChildrenIndices), sizeof(cln_octree.ChildrenIndices));
		clnfile.read(reinterpret_cast<char*>(&octree[o].Vmin.x), sizeof(cln_octree.Xmin));
		clnfile.read(reinterpret_cast<char*>(&octree[o].Vmin.y), sizeof(cln_octree.Ymin));
		clnfile.read(reinterpret_cast<char*>(&octree[o].Vmin.z), sizeof(cln_octree.Zmin));
		clnfile.read(reinterpret_cast<char*>(&cln_octree.nDescendants), sizeof(cln_octree.nDescendants));
		clnfile.read(reinterpret_cast<char*>(&octree[o].Vmax.x), sizeof(cln_octree.Xmax));
		clnfile.read(reinterpret_cast<char*>(&octree[o].Vmax.y), sizeof(cln_octree.Ymax));
		clnfile.read(reinterpret_cast<char*>(&octree[o].Vmax.z), sizeof(cln_octree.Zmax));
		clnfile.read(reinterpret_cast<char*>(&cln_octree.nTriangles), sizeof(cln_octree.nTriangles));
		octree[o].Vmin.x *= 1024;
		octree[o].Vmin.y *= 1024;
		octree[o].Vmin.z *= 1024;
		octree[o].Vmax.x *= 1024;
		octree[o].Vmax.y *= 1024;
		octree[o].Vmax.z *= 1024;

		for (unsigned int c = 0; c < 8; c++)		// Viene usato Ptr_Child1 per tutti gli 8 discendenti dell'ottante
		{
			clnfile.read(reinterpret_cast<char*>(&cln_octree.Ptr_Child1), sizeof(cln_octree.Ptr_Child1));
			if (cln_octree.Ptr_Child1 > 0)
			{
				stringstream ss1;
				ss1 << octree[o].name << "_" << c;
				Octant temp_octant;
				temp_octant.name = ss1.str();
				temp_octant.parent = octree[o].name;
				temp_octant.Ptr_Octant = cln_octree.Ptr_Child1;
				octree.push_back(temp_octant);
			}
		}

		// Creazione gruppo
		stringstream ss1, ss2;
		ss1 << AOD_IO.levelname << "_CLN_OCTANT_" << octree[o].name;
		ss2 << AOD_IO.levelname << "_CLN_OCTANT_" << octree[o].parent;
		Transform octant_group;
		octant_group.name = ss1.str();
		if (!octree[o].parent.empty())
		{
			octant_group.parent = ss2.str();
			octant_group.FBX_parent = hashID(ss2.str(), "Group");
		}
		octant_group.layer = octree_layer.name;
		FBX.Group.push_back(octant_group);
		MA.Transform.push_back(octant_group);

		// Creazione parallelepipedo ottante
		stringstream ss3;
		ss3 << AOD_IO.levelname << "_CLN_OCTANT_" << octree[o].name << "_box";
		FBX.Geometry.push_back(DrawBox(ss3.str(), ss1.str(), octree_layer.name, octree[o].Vmin, octree[o].Vmax, 0x25FDBC2E));
		MA.Mesh.push_back(DrawBox(ss3.str(), ss1.str(), octree_layer.name, octree[o].Vmin, octree[o].Vmax, 0x25FDBC2E));
		
		
		
		

		// Esportazione triangoli per ottanti senza discendenti
		if (cln_octree.nChildren == 0)
		{
			//msg(msg::TGT::FILE, msg::TYP::DBG) << "octant " << left << setw(10) << octree[o].name << "   nTriangles: " << left << setw(5) << cln_octree.nTriangles << "Indices: ";
			clndebug << endl << "octant " << left << setw(10) << octree[o].name << "   nTriangles: " << left << setw(5) << cln_octree.nTriangles << "Indices: ";
			clnfile.seekg(triangle_position + octree[o].Ptr_TList);
			unsigned int t = 0;
			while (t < cln_octree.nTriangles)
			{
				clnfile.read(reinterpret_cast<char*>(&cln_tlist.attribute), sizeof(cln_tlist.attribute));
				clnfile.read(reinterpret_cast<char*>(&cln_tlist.Unknown2), sizeof(cln_tlist.Unknown2));
				clnfile.read(reinterpret_cast<char*>(&cln_tlist.nIndices), sizeof(cln_tlist.nIndices));

				unsigned short composition, mapping, bitattrib;

				composition = cln_tlist.attribute & 0xFF;               // bits 0–7
				mapping = (cln_tlist.attribute >> 8) & 0x3;				// bits 8–9 (2 bits)
				bitattrib = (cln_tlist.attribute >> 10) & 0x003FFFFF;	// bits 10–31 (22 bits)


				if (find(vtype.begin(), vtype.end(), cln_tlist.attribute) == vtype.end())
					vtype.push_back(cln_tlist.attribute);
				if (find(vpadding1.begin(), vpadding1.end(), cln_tlist.Unknown2) == vpadding1.end())
					vpadding1.push_back(cln_tlist.Unknown2);
				//if (find(vpadding2.begin(), vpadding2.end(), cln_tlist.Unknown3) == vpadding2.end())
					//vpadding2.push_back(cln_tlist.Unknown3);
				if (find(vcomposition.begin(), vcomposition.end(), composition) == vcomposition.end())
					vcomposition.push_back(composition);
				if (find(vmapping.begin(), vmapping.end(), mapping) == vmapping.end())
					vmapping.push_back(mapping);
				if (find(vbitattrib.begin(), vbitattrib.end(), bitattrib) == vbitattrib.end())
					vbitattrib.push_back(bitattrib);

				if (cln_tlist.nIndices > 0)
				{
					stringstream meshname, ssmaterial;
					meshname << ss1.str() << "_tris_" << t;
					ssmaterial << AOD_IO.levelname << "_Collision_" << GetCompositionName(composition);
					
					Mesh temp_mesh;
					temp_mesh.name = meshname.str();
					temp_mesh.parent = ss1.str();
					temp_mesh.FBX_parent = hashID(ss1.str(), "Group");
					temp_mesh.layer = octree_layer.name;
					temp_mesh.material_name = ssmaterial.str();
					temp_mesh.nV = cln_tlist.nIndices * 3;
					temp_mesh.uv_set2_flag = temp_mesh.normals_flag = temp_mesh.tangents_flag = temp_mesh.binormals_flag = temp_mesh.vcolors_flag = false;

					for (unsigned int i = 0; i < cln_tlist.nIndices; i++)
					{
						clnfile.read(reinterpret_cast<char*>(&cln_tlist.Index), sizeof(cln_tlist.Index));
						streamoff old_position = clnfile.tellg();
						clndebug << right << setw(7) << cln_tlist.Index;
						clnfile.seekg(triangle_position + cln_tlist.Index * 48);
						Vec3 v1, v2, v3;
						unsigned int MissingAxis, Unknown;
						CLN_Get_Triangle(clnfile, v1, v2, v3, MissingAxis, Unknown);
						clnfile.seekg(old_position);
						temp_mesh.X.push_back(v1.x);
						temp_mesh.X.push_back(v2.x);
						temp_mesh.X.push_back(v3.x);
						temp_mesh.Y.push_back(v1.y);
						temp_mesh.Y.push_back(v2.y);
						temp_mesh.Y.push_back(v3.y);
						temp_mesh.Z.push_back(v1.z);
						temp_mesh.Z.push_back(v2.z);
						temp_mesh.Z.push_back(v3.z);

						// Ottimizzazione UV per evitare texture stretching
						Vec3 edge1 = v2 - v1;
						Vec3 edge2 = v3 - v1;
						Vec3 normal = edge1.cross(edge2).normalized();
						Vec3 tangent = edge1.normalized();							// asse U
						Vec3 bitangent = normal.cross(tangent).normalized();		// asse V
						Vec2 uv1(0.0f, 0.0f);										// origine
						Vec2 uv2((v2 - v1).dot(tangent), (v2 - v1).dot(bitangent));
						Vec2 uv3((v3 - v1).dot(tangent), (v3 - v1).dot(bitangent));
						temp_mesh.U1.push_back(uv1.u / 1024);
						temp_mesh.V1.push_back(uv1.v / 1024);
						temp_mesh.U1.push_back(uv2.u / 1024);
						temp_mesh.V1.push_back(uv2.v / 1024);
						temp_mesh.U1.push_back(uv3.u / 1024);
						temp_mesh.V1.push_back(uv3.v / 1024);

						Face triangle;
						triangle.v1 = i * 3;		triangle.v2 = i * 3 + 1;		triangle.v3 = i * 3 + 2;
						temp_mesh.Face.push_back(triangle);
					}

					FBX.Geometry.push_back(temp_mesh);
					MA.Mesh.push_back(temp_mesh);
				}
				t += cln_tlist.nIndices;
			}
		}
	}	
	


	clndebug.close();





	// LETTURA INTERA GEOMETRIA DELLE COLLISIONI ED ESPORTAZIONE IN MESH UNICA
	clnfile.seekg(triangle_position);

	// Creazione classe mesh per geometria collisioni
	stringstream meshname;
	meshname << AOD_IO.levelname << "_Collisions";
	Mesh cln_mesh;
	cln_mesh.name = meshname.str();
	cln_mesh.layer = collisions_layer.name;
	cln_mesh.nV = cln_octree.Ptr_TList / 16;		// Numero di triangoli x 3
	cln_mesh.uv_set2_flag = cln_mesh.normals_flag = cln_mesh.tangents_flag = cln_mesh.binormals_flag = cln_mesh.vcolors_flag = false;

	vector <unsigned int> attribute_list;

	for (unsigned int i = 0; i < (cln_octree.Ptr_TList / 48); i++)
	{
		Vec3 v1, v2, v3;
		unsigned int MissingAxis, attribute, composition, mapping, bitattrib;
		CLN_Get_Triangle(clnfile, v1, v2, v3, MissingAxis, attribute);
		composition = attribute & 0xFF;					// bits 0–7: Materiale della superficie (legno, pietra, metallo, neve, ecc.)
		mapping = (attribute >> 8) & 0x3;				// bits 8–9 (2 bits): Tipo di superficie (None, Floor, Ceiling)
		bitattrib = (attribute >> 10) & 0x003FFFFF;		// bits 10–31 (22 bits): Interazione (morte, scale, parete scalabile, scivolo, ecc.)

		if (find(attribute_list.begin(), attribute_list.end(), attribute) == attribute_list.end())
			attribute_list.push_back(attribute);

		// Aggiunta coordinate vertici
		cln_mesh.X.push_back(v1.x);
		cln_mesh.X.push_back(v2.x);
		cln_mesh.X.push_back(v3.x);
		cln_mesh.Y.push_back(v1.y);
		cln_mesh.Y.push_back(v2.y);
		cln_mesh.Y.push_back(v3.y);
		cln_mesh.Z.push_back(v1.z);
		cln_mesh.Z.push_back(v2.z);
		cln_mesh.Z.push_back(v3.z);
		
		// Ottimizzazione UV per evitare texture stretching
		Vec3 edge1 = v2 - v1;
		Vec3 edge2 = v3 - v1;
		Vec3 normal = edge1.cross(edge2).normalized();
		Vec3 tangent = edge1.normalized();							// asse U
		Vec3 bitangent = normal.cross(tangent).normalized();		// asse V
		Vec2 uv1(0.0f, 0.0f);										// origine
		Vec2 uv2((v2 - v1).dot(tangent), (v2 - v1).dot(bitangent));
		Vec2 uv3((v3 - v1).dot(tangent), (v3 - v1).dot(bitangent));
		cln_mesh.U1.push_back(uv1.u / 1024);
		cln_mesh.V1.push_back(uv1.v / 1024);
		cln_mesh.U1.push_back(uv2.u / 1024);
		cln_mesh.V1.push_back(uv2.v / 1024);
		cln_mesh.U1.push_back(uv3.u / 1024);
		cln_mesh.V1.push_back(uv3.v / 1024);

		// Aggiunta indici spigoli triangolo
		Face triangle;
		triangle.v1 = i * 3;		triangle.v2 = i * 3 + 1;		triangle.v3 = i * 3 + 2;
		cln_mesh.Face.push_back(triangle);
	}

	msg(msg::TGT::FILE, msg::TYP::DBG) << "TRIANGLE.Unknown values:";
	for (unsigned int k = 0; k < tags_list.size(); k++)
		msg(msg::TGT::FILE, msg::TYP::OVR) << "  " << std::hex << tags_list[k] << std::dec;

	msg(msg::TGT::FILE, msg::TYP::DBG) << "TLIST.attribute values:";
	for (unsigned int k = 0; k < vtype.size(); k++)
		msg(msg::TGT::FILE, msg::TYP::OVR) << "  " << std::hex << vtype[k] << std::dec;

	msg(msg::TGT::FILE, msg::TYP::DBG) << "TLIST.Unknown2 values:";
	for (unsigned int k = 0; k < vpadding1.size(); k++)
		msg(msg::TGT::FILE, msg::TYP::OVR) << "  " << std::hex << vpadding1[k] << std::dec;

	//msg(msg::TGT::FILE, msg::TYP::DBG) << "TLIST.Unknown3 values:";
	//for (unsigned int k = 0; k < vpadding2.size(); k++)
		//msg(msg::TGT::FILE, msg::TYP::OVR) << "  " << std::hex << vpadding2[k] << std::dec;

	msg(msg::TGT::FILE, msg::TYP::DBG) << "Composition values:";
	for (unsigned int k = 0; k < vcomposition.size(); k++)
		msg(msg::TGT::FILE, msg::TYP::OVR) << "  " << vcomposition[k];
	msg(msg::TGT::FILE, msg::TYP::DBG) << "mapping values:";
	for (unsigned int k = 0; k < vmapping.size(); k++)
		msg(msg::TGT::FILE, msg::TYP::OVR) << "  " << vmapping[k];
	msg(msg::TGT::FILE, msg::TYP::DBG) << "bitattrib values:";
	for (unsigned int k = 0; k < vbitattrib.size(); k++)
		msg(msg::TGT::FILE, msg::TYP::OVR) << "  " << vbitattrib[k];

	FBX.Geometry.push_back(cln_mesh);						// Inserimento collisioni nel vettore mesh del file FBX
	MA.Mesh.push_back(cln_mesh);							// Inserimento collisioni nel vettore mesh del file MA

	// Creazione cartella per textures collisioni
	AOD_IO.folder_temp = AOD_IO.folder_collisions;
	AOD_IO.folder_temp.append("Collision_Textures");
	mbstowcs(AOD_IO.folder_temp_lpwstr, AOD_IO.folder_temp.c_str(), MAX);
	CreateDirectory(AOD_IO.folder_temp_lpwstr, NULL);		// Crea la cartella \NOMELIVELLO\Collisions\Collision_Textures
	SetCurrentDirectory(AOD_IO.folder_temp_lpwstr);

	// Creazione materiali e textures per composition
	for (unsigned int k = 0; k < vcomposition.size(); k++)
	{
		stringstream ssmaterial, diffuse, texture_name, pathfilename, filename;
		ssmaterial << AOD_IO.levelname << "_Collision_" << GetCompositionName(vcomposition[k]);
		diffuse << "Collision_Texture_" << GetCompositionName(vcomposition[k]);
		Material mat;
		mat.name = ssmaterial.str();
		mat.Type = Material::TYPE::LAMBERT;
		mat.color = diffuse.str();
		MA.Material.push_back(mat);

		Texture tex;
		texture_name << "Collision_Texture_" << GetCompositionName(vcomposition[k]);
		filename << "Collision_Tex_" << GetCompositionName(vcomposition[k]) << ".tga";
		pathfilename << AOD_IO.folder_temp << "\\" << filename.str();
		tex.name = texture_name.str();
		tex.filename = pathfilename.str();
		MA.Texture.push_back(tex);
		SaveTGAResource(vcomposition[k] + 100, filename.str());
	}


	return true;
}