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
	CLN_OCTREE cln_octree;
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

	msg(msg::TGT::FILE, msg::TYP::LOG) << "Number of octants: " << cln_octree.nDescendants + 1;
	msg(msg::TGT::FILE, msg::TYP::LOG) << "Number of triangles: " << cln_octree.Ptr_TList / 48;

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
	MA.Layer.push_back(octree_layer);				// Layer octree

	vector <string> attribute_list;

	// LETTURA COLLISIONI IN BASE ALL'OCTREE ED ESPORTAZIONE IN BOUNDING BOXES OCTREES
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
	}	
	
	// LETTURA INTERA GEOMETRIA DELLE COLLISIONI ED ESPORTAZIONE IN MESH UNICA
	clnfile.seekg(triangle_position);

	// Creazione classe mesh per geometria collisioni
	stringstream meshname;
	meshname << AOD_IO.levelname << "_CollisionMap";
	Mesh cln_mesh;
	cln_mesh.name = meshname.str();
	cln_mesh.layer = collisions_layer.name;
	cln_mesh.nV = cln_octree.Ptr_TList / 16;		// Numero di triangoli x 3
	cln_mesh.uv_set2_flag = cln_mesh.normals_flag = cln_mesh.tangents_flag = cln_mesh.binormals_flag = cln_mesh.vcolors_flag = false;
	cln_mesh.multimaterial = true;

	// Ogni ciclo esporta un triangolo
	for (unsigned int i = 0; i < (cln_octree.Ptr_TList / 48); i++)
	{
		Vec3 v1, v2, v3;
		unsigned int MissingAxis, attribute, composition, mapping, bitattrib;
		CLN_Get_Triangle(clnfile, v1, v2, v3, MissingAxis, attribute);
		composition = attribute & 0xFF;					// bits 0–7: Materiale della superficie (legno, pietra, metallo, neve, ecc.)
		mapping = (attribute >> 8) & 0x3;				// bits 8–9 (2 bits): Tipo di superficie (None, Floor, Ceiling)
		bitattrib = (attribute >> 10) & 0xFFF;			// bits 10–31 (22 bits): Interazione (morte, scale, parete scalabile, scivolo, ecc.)

		stringstream ssmaterial;
		string attribute_string = CLN_GetAttributeName(composition, mapping, bitattrib);
		ssmaterial << "ColMat_" << attribute_string;
		string temp = ssmaterial.str();
		vector <ObjectGroup>::iterator it;
		it = find_if(cln_mesh.Groups.begin(), cln_mesh.Groups.end(), [&temp](const ObjectGroup& current) {return current.material_name == temp; });
		if (it == cln_mesh.Groups.end())
		{
			ObjectGroup temp_group;
			temp_group.material_name = temp;
			temp_group.FaceIDs.push_back(i);
			cln_mesh.Groups.push_back(temp_group);
			attribute_list.push_back(attribute_string);
		}
		else
			cln_mesh.Groups[distance(cln_mesh.Groups.begin(), it)].FaceIDs.push_back(i);

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
		cln_mesh.U1.push_back(uv1.u / 768.0f);
		cln_mesh.V1.push_back(uv1.v / 768.0f);
		cln_mesh.U1.push_back(uv2.u / 768.0f);
		cln_mesh.V1.push_back(uv2.v / 768.0f);
		cln_mesh.U1.push_back(uv3.u / 768.0f);
		cln_mesh.V1.push_back(uv3.v / 768.0f);

		// Aggiunta indici spigoli triangolo
		Face triangle;
		triangle.v1 = i * 3;		triangle.v2 = i * 3 + 1;		triangle.v3 = i * 3 + 2;
		cln_mesh.Face.push_back(triangle);
	}

	FBX.Geometry.push_back(cln_mesh);						// Inserimento collisioni nel vettore mesh del file FBX
	MA.Mesh.push_back(cln_mesh);							// Inserimento collisioni nel vettore mesh del file MA

	// Creazione cartella per textures collisioni
	AOD_IO.folder_temp = AOD_IO.folder_collisions;
	AOD_IO.folder_temp.append("Collision_Textures");
	mbstowcs(AOD_IO.folder_temp_lpwstr, AOD_IO.folder_temp.c_str(), MAX);
	CreateDirectory(AOD_IO.folder_temp_lpwstr, NULL);		// Crea la cartella \NOMELIVELLO\Collisions\Collision_Textures
	SetCurrentDirectory(AOD_IO.folder_temp_lpwstr);

	// Creazione materiali e textures per composition
	for (unsigned int k = 0; k < attribute_list.size(); k++)
	{
		stringstream ssmaterial, diffuse, texture_name, pathfilename, filename;
		ssmaterial << "ColMat_" << attribute_list[k];
		diffuse << "ColTex_" << attribute_list[k];
		Material mat;
		mat.name = ssmaterial.str();
		mat.Type = Material::TYPE::LAMBERT;
		mat.color = diffuse.str();
		MA.Material.push_back(mat);

		Texture tex;
		texture_name << "ColTex_" << attribute_list[k];
		filename << "ColTex_" << attribute_list[k] << ".tga";
		pathfilename << AOD_IO.folder_temp << "\\" << filename.str();
		tex.name = texture_name.str();
		tex.filename = pathfilename.str();
		MA.Texture.push_back(tex);
		SaveTGAResource(CLN_GetResource(attribute_list[k]), filename.str());
	}

	return true;
}