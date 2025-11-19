#include "stdafx.h"
#include "TRAOD/ZONE/ZONE_Struct.h"
#include "TRAOD Remastered/MESH/MESH_Functions.h"
#include "Misc_Functions.h"


bool MESH_Read_Object (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	ZONE_MESH_OBJECT_HEADER zone_mesh_object_header;
	ZONE_MESH_VERTEX zone_mesh_vertex;
	ZONE_MESH_STRIP zone_mesh_strip;
	ZONE_MESH_ELEMENT zone_mesh_element;

	///////////////////    APERTURA FILE MESH
	ifstream meshfile(filename, std::ios::binary);
	if (!meshfile.is_open())
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " not found.";
		return false;
	}

	// CREAZIONE LAYERS ZONE PER FILE MA
	Layer Mesh_layer, Mesh_BB_layer;
	Transform Mesh_group;
	stringstream Mesh_layer_name, Mesh_BB_layer_name, Mesh_group_name;
	Mesh_layer_name << "MSH_" << AODRemastered_IO.objectname;
	Mesh_BB_layer_name << Mesh_layer_name.str() << "_bounding_boxes";
	Mesh_group_name << "MESH_" << AODRemastered_IO.objectname;
	Mesh_layer.name = Mesh_layer_name.str();
	Mesh_layer.Label_ARGB = 0xFF00D822;
	MA.Layer.push_back(Mesh_layer);
	Mesh_BB_layer.name = Mesh_BB_layer_name.str();
	Mesh_BB_layer.Label_ARGB = 0xFF0000FF;
	Mesh_BB_layer.Visible = false;
	Mesh_BB_layer.Type = LayerDisplayType::Template;
	MA.Layer.push_back(Mesh_BB_layer);
	Mesh_group.name = Mesh_group_name.str();
	FBX.Group.push_back(Mesh_group);								// Inserimento gruppo oggetto nel file FBX
	MA.Transform.push_back(Mesh_group);								// Inserimento gruppo oggetto nel file MA

	// Lettura Header oggetto
	meshfile.read(reinterpret_cast<char*>(&zone_mesh_object_header.Object_size), sizeof(zone_mesh_object_header.Object_size));	// Dimensione oggetto in bytes
	meshfile.seekg(8, ios_base::cur);		// Salta i due unknown1/2
	meshfile.read(reinterpret_cast<char*>(&zone_mesh_object_header.nVertices), sizeof(zone_mesh_object_header.nVertices));		// Numero vertici
	meshfile.seekg(4, ios_base::cur);		// Salta unknown3
	meshfile.read(reinterpret_cast<char*>(&zone_mesh_object_header.nIndices), sizeof(zone_mesh_object_header.nIndices));		// Numero indici del triangle strip
	meshfile.seekg(4, ios_base::cur);		// Salta unknown4
	meshfile.read(reinterpret_cast<char*>(&zone_mesh_object_header.nElements), sizeof(zone_mesh_object_header.nElements));		// Numero elementi
	streamoff vertex_position = 52;																// Memorizza la posizione iniziale del blocco vertici
	streamoff strip_position = vertex_position + zone_mesh_object_header.nVertices * 40;		// Memorizza la posizione iniziale del blocco triangle strip
	streamoff elements_position = strip_position + zone_mesh_object_header.nIndices * 2;		// Memorizza la posizione iniziale del blocco elementi
	
	// Vettore contenente tutti i nomi dei materiali usati nel file msh
	vector <string> Material_list;

	for (unsigned int el = 0; el < zone_mesh_object_header.nElements; el++)
	{
		Mesh element;				// Classe provvisoria contente i valori letti dal file MSH. Va copiata nell'apposito array FBX e/o MA al termine dell'estrazione
		stringstream ssname, ssmaterial, ssbbname;
		ssname << "MESH_" << AODRemastered_IO.objectname << "_PART_" << el;
		ssbbname << ssname.str() << "_BB";
		element.name = ssname.str();
		element.parent = Mesh_group.name;
		element.layer = Mesh_layer_name.str();
		element.FBX_parent = hashID(Mesh_group.name, "Group");

		// Lettura dati elemento
		XYZ BBmin, BBmax;
		meshfile.seekg(elements_position + el * 64);
		meshfile.seekg(4, ios_base::cur);		// Salta nElement_Triangles
		meshfile.read(reinterpret_cast<char*>(&zone_mesh_element.nElement_Indices), sizeof(zone_mesh_element.nElement_Indices));	// Numero di indici dello strip
		meshfile.read(reinterpret_cast<char*>(&zone_mesh_element.Offset), sizeof(zone_mesh_element.Offset));						// Offset nello strip
		meshfile.read(reinterpret_cast<char*>(&zone_mesh_element.Material_Ref), sizeof(zone_mesh_element.Material_Ref));			// ID materiale
		meshfile.seekg(12, ios_base::cur);		// Salta Unknown1, Vbuffer_min e Vbuffer_max
		meshfile.read(reinterpret_cast<char*>(&zone_mesh_element.Draw_mode), sizeof(zone_mesh_element.Draw_mode));					// Tipologia di rendering
		meshfile.read(reinterpret_cast<char*>(&BBmin.x), sizeof(zone_mesh_element.BB_Xmin));										// Bounding box X min
		meshfile.read(reinterpret_cast<char*>(&BBmin.y), sizeof(zone_mesh_element.BB_Ymin));										// Bounding box Y min
		meshfile.read(reinterpret_cast<char*>(&BBmin.z), sizeof(zone_mesh_element.BB_Zmin));										// Bounding box Z min
		meshfile.seekg(4, ios_base::cur);		// Salta Unknown2
		meshfile.read(reinterpret_cast<char*>(&BBmax.x), sizeof(zone_mesh_element.BB_Xmax));										// Bounding box X max
		meshfile.read(reinterpret_cast<char*>(&BBmax.y), sizeof(zone_mesh_element.BB_Ymax));										// Bounding box Y max
		meshfile.read(reinterpret_cast<char*>(&BBmax.z), sizeof(zone_mesh_element.BB_Xmax));										// Bounding box Z max
		FBX.Geometry.push_back(DrawBox(ssbbname.str(), Mesh_group_name.str(), Mesh_BB_layer_name.str(), BBmin, BBmax, 0x35500000));
		MA.Mesh.push_back(DrawBox(ssbbname.str(), Mesh_group_name.str(), Mesh_BB_layer_name.str(), BBmin, BBmax, 0x35500000));

		// Se l'elemento non contiene almeno 1 triangolo (numero indici almeno pari a 3) viene saltato
		if (zone_mesh_element.nElement_Indices < 3)
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << element.name << " does not contain any triangle. Only Bounding Box will be exported.";
			continue;
		}

		// Aggiunta collegamento a materiale
		if (zone_mesh_element.Material_Ref >= 0)
			ssmaterial << "MESH_" << AODRemastered_IO.objectname << "_Material_N" << zone_mesh_element.Material_Ref;
		else
			ssmaterial << "MESH_" << AODRemastered_IO.objectname << "_Material_" << to_string(-zone_mesh_element.Material_Ref);

		element.material_name = ssmaterial.str();
		if (find(Material_list.begin(), Material_list.end(), ssmaterial.str()) == Material_list.end())
			Material_list.push_back(ssmaterial.str());

		// Lettura strip
		vector <unsigned int> strip(zone_mesh_element.nElement_Indices);
		vector <unsigned int> vertex_array;
		meshfile.seekg(strip_position + zone_mesh_element.Offset * 2);					// Posizionamento cursore di lettura all'inizio dello strip dell'elemento el
		for (unsigned int i = 0; i < zone_mesh_element.nElement_Indices; i++)			// Lettura strip del singolo elemento el
			meshfile.read(reinterpret_cast<char*>(&strip[i]), sizeof(zone_mesh_strip.Index));
		for (unsigned int i = 0; i < strip.size(); i++)
		{
			vector <unsigned int>::iterator it2 = find(vertex_array.begin(), vertex_array.end(), strip[i]);
			if (it2 == vertex_array.end())												// Se il vertice non viene trovato viene aggiunto alla lista
			{
				vertex_array.push_back(strip[i]);
				strip[i] = vertex_array.size() - 1;										// Gli indici dei vertici vengono aggiornati in base alla nuova lista vertex_array
			}
			else																		// Se il vertice viene trovato (precedentemente inserito) copia la sua posizione
				strip[i] = distance(vertex_array.begin (), it2);						// Gli indici dei vertici vengono aggiornati in base alla nuova lista vertex_array
		}
		element.Face = Calculate_Faces(strip, zone_mesh_element.Offset, zone_mesh_element.Draw_mode);		// Creazione lista facce
		element.nV = vertex_array.size();

		// Lettura vertici
		for (unsigned int v = 0; v < vertex_array.size(); v++)
		{
			meshfile.seekg(vertex_position + vertex_array[v] * 40);
			meshfile.read(reinterpret_cast<char*>(&zone_mesh_vertex.X), sizeof(zone_mesh_vertex.X));					// Coordinata X
			meshfile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Y), sizeof(zone_mesh_vertex.Y));					// Coordinata Y
			meshfile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Z), sizeof(zone_mesh_vertex.Z));					// Coordinata Z
			meshfile.read(reinterpret_cast<char*>(&zone_mesh_vertex.U1), sizeof(zone_mesh_vertex.U1));					// UV
			meshfile.read(reinterpret_cast<char*>(&zone_mesh_vertex.V1), sizeof(zone_mesh_vertex.V1));					// UV
			meshfile.read(reinterpret_cast<char*>(&zone_mesh_vertex.U2), sizeof(zone_mesh_vertex.U2));					// UV shadow map
			meshfile.read(reinterpret_cast<char*>(&zone_mesh_vertex.V2), sizeof(zone_mesh_vertex.V2));					// UV shadow map
			meshfile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Xn), sizeof(zone_mesh_vertex.Xn));					// Vertex normal X
			meshfile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Yn), sizeof(zone_mesh_vertex.Yn));					// Vertex normal Y
			meshfile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Zn), sizeof(zone_mesh_vertex.Zn));					// Vertex normal Z
			meshfile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Xtg), sizeof(zone_mesh_vertex.Xtg));				// Vertex tangent X
			meshfile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Ytg), sizeof(zone_mesh_vertex.Ytg));				// Vertex tangent Y
			meshfile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Ztg), sizeof(zone_mesh_vertex.Ztg));				// Vertex tangent Z
			meshfile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Xbn), sizeof(zone_mesh_vertex.Xbn));				// Vertex binormal X
			meshfile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Ybn), sizeof(zone_mesh_vertex.Ybn));				// Vertex binormal Y
			meshfile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Zbn), sizeof(zone_mesh_vertex.Zbn));				// Vertex binormal Z
			meshfile.read(reinterpret_cast<char*>(&zone_mesh_vertex.VC_red), sizeof(zone_mesh_vertex.VC_red));			// Vertex color R
			meshfile.read(reinterpret_cast<char*>(&zone_mesh_vertex.VC_green), sizeof(zone_mesh_vertex.VC_green));		// Vertex color G
			meshfile.read(reinterpret_cast<char*>(&zone_mesh_vertex.VC_blue), sizeof(zone_mesh_vertex.VC_blue));		// Vertex color B
			element.X.push_back(zone_mesh_vertex.X);
			element.Y.push_back(zone_mesh_vertex.Y);
			element.Z.push_back(zone_mesh_vertex.Z);
			element.U1.push_back(zone_mesh_vertex.U1);
			element.V1.push_back(zone_mesh_vertex.V1);
			element.U2.push_back(zone_mesh_vertex.U2);
			element.V2.push_back(zone_mesh_vertex.V2);
			element.Xn.push_back(((float)zone_mesh_vertex.Xn - 128) / 127);
			element.Yn.push_back(((float)zone_mesh_vertex.Yn - 128) / 127);
			element.Zn.push_back(((float)zone_mesh_vertex.Zn - 128) / 127);
			element.Xtg.push_back(((float)zone_mesh_vertex.Xtg - 128) / 127);
			element.Ytg.push_back(((float)zone_mesh_vertex.Ytg - 128) / 127);
			element.Ztg.push_back(((float)zone_mesh_vertex.Ztg - 128) / 127);
			element.Xbn.push_back(((float)zone_mesh_vertex.Xbn - 128) / 127);
			element.Ybn.push_back(((float)zone_mesh_vertex.Ybn - 128) / 127);
			element.Zbn.push_back(((float)zone_mesh_vertex.Zbn - 128) / 127);
			element.A.push_back(1);
			element.R.push_back((float)zone_mesh_vertex.VC_red / 255);
			element.G.push_back((float)zone_mesh_vertex.VC_green / 255);
			element.B.push_back((float)zone_mesh_vertex.VC_blue / 255);
		}
		FBX.Geometry.push_back(element);						// Inserimento elemento geometrico nel file FBX
		MA.Mesh.push_back(element);								// Inserimento elemento geometrico nel file MA
	}

	meshfile.close();

	// Creazione materiali dummy
	for (unsigned int m = 0; m < Material_list.size(); m++)
	{
		Material mat;
		mat.name = Material_list[m];
		mat.Type = Material::TYPE::LAMBERT;
		MA.Material.push_back(mat);
	}

	return true;
}