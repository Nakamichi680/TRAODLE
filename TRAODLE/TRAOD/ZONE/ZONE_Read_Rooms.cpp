#include "stdafx.h"
#include "TRAOD/ZONE/ZONE_Struct.h"
#include "TRAOD/ZONE/ZONE_Functions.h"
#include "Misc_Functions.h"


bool ZONE_Read_Rooms (string filename, vector <RoomInfo> RMX_Rooms, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	ZONE_HEADER zone_header;
	ZONE_MATERIALS_LIST zone_materials_list;
	ZONE_MESH_HEADER1 zone_mesh_header1;
	ZONE_MESH_ROOM_HEADER zone_mesh_room_header;
	ZONE_MESH_VERTEX zone_mesh_vertex;
	ZONE_MESH_STRIP zone_mesh_strip;
	ZONE_MESH_ELEMENT zone_mesh_element;
	string zonename = filename;
	zonename.erase(0, (zonename.find(".Z") + 1));

	///////////////////    APERTURA FILE ZONE
	ifstream zonefile(filename, std::ios::binary);
	if (!zonefile.is_open())
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " not found.";
		return false;
	}

	// CREAZIONE LAYERS ZONE PER FILE MA
	Layer Zone_layer, Zone_BB_layer;
	stringstream Zone_layer_name, Zone_BB_layer_name;
	Zone_layer_name << AOD_IO.levelname << "_" << zonename;
	Zone_BB_layer_name << Zone_layer_name.str() << "_bounding_boxes";
	Zone_layer.name = Zone_layer_name.str();
	Zone_layer.Label_ARGB = 0xFF00D822;
	MA.Layer.push_back(Zone_layer);
	Zone_BB_layer.name = Zone_BB_layer_name.str();
	Zone_BB_layer.Label_ARGB = 0xFF0000FF;
	Zone_BB_layer.Visible = false;
	Zone_BB_layer.Type = LayerDisplayType::Template;
	MA.Layer.push_back(Zone_BB_layer);

	// Lettura Header
	zonefile.read(reinterpret_cast<char*>(&zone_header.ZONE_ID), sizeof(zone_header.ZONE_ID));

	if (zone_header.ZONE_ID != 32)			// Se il file ZONE non è valido
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " is not a valid ZONE file.";
		return false;
	}

	zonefile.read(reinterpret_cast<char*>(&zone_header.TEXTURE_PTR), sizeof(zone_header.TEXTURE_PTR));
	zonefile.read(reinterpret_cast<char*>(&zone_header.PS2_OBJ_PTR), sizeof(zone_header.PS2_OBJ_PTR));
	zonefile.read(reinterpret_cast<char*>(&zone_header.MESH_PTR), sizeof(zone_header.MESH_PTR));
	zonefile.read(reinterpret_cast<char*>(&zone_header.EOF_PTR), sizeof(zone_header.EOF_PTR));

	// Lettura stanze
	zonefile.seekg(zone_header.MESH_PTR);
	zonefile.read(reinterpret_cast<char*>(&zone_mesh_header1.nRooms), sizeof(zone_mesh_header1.nRooms));			// Lettura numero stanze
	
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Number of rooms: " << zone_mesh_header1.nRooms;

	for (unsigned int r = 0; r < zone_mesh_header1.nRooms; r++)													// Questo macroblocco "for" legge tutti i dati di ogni stanza
	{
		unsigned int RoomID, r_RMX;		
		streamoff header_position = zonefile.tellg();							// Memorizza la posizione iniziale del blocco vertici
		zonefile.read(reinterpret_cast<char*>(&RoomID), sizeof(zone_mesh_room_header.RoomID));									// ID stanza
		zonefile.read(reinterpret_cast<char*>(&zone_mesh_room_header.Room_size), sizeof(zone_mesh_room_header.Room_size));		// Dimensioni stanza bytes
		zonefile.seekg(8, ios_base::cur);		// Salta i due unknown1/2
		zonefile.read(reinterpret_cast<char*>(&zone_mesh_room_header.nVertices), sizeof(zone_mesh_room_header.nVertices));		// Numero vertici
		zonefile.seekg(4, ios_base::cur);		// Salta unknown3
		zonefile.read(reinterpret_cast<char*>(&zone_mesh_room_header.nIndices), sizeof(zone_mesh_room_header.nIndices));		// Numero indici del triangle strip
		zonefile.seekg(4, ios_base::cur);		// Salta unknown4
		zonefile.read(reinterpret_cast<char*>(&zone_mesh_room_header.nElements), sizeof(zone_mesh_room_header.nElements));		// Numero elementi
		zonefile.seekg(20, ios_base::cur);		// Salta unknown5/6/7/8/9
		streamoff vertex_position = header_position + 56;											// Memorizza la posizione iniziale del blocco vertici
		streamoff strip_position = vertex_position + zone_mesh_room_header.nVertices * 40;			// Memorizza la posizione iniziale del blocco triangle strip
		streamoff elements_position = strip_position + zone_mesh_room_header.nIndices * 2;			// Memorizza la posizione iniziale del blocco elementi
		
		Transform room;
		stringstream temp;
		vector <RoomInfo>::iterator it1 = find_if(RMX_Rooms.begin(), RMX_Rooms.end(), [&RoomID] (const RoomInfo& RMX_Rooms) {return RMX_Rooms.RoomHash == RoomID;});
		if (it1 != RMX_Rooms.end())
		{
			r_RMX = distance(RMX_Rooms.begin (), it1);
			room.translate_flag = true;
			room.tX = RMX_Rooms[r_RMX].tX;
			room.tY = RMX_Rooms[r_RMX].tY;
			room.tZ = RMX_Rooms[r_RMX].tZ;
		}
		else
			r_RMX = 1000 + r;
		temp << AOD_IO.levelname << "_" << zonename << "_ROOM_" << r_RMX << "_" << std::hex << RoomID << std::dec;
		room.name = temp.str();
		FBX.Group.push_back(room);									// Inserimento gruppo stanza nel file FBX
		MA.Transform.push_back(room);								// Inserimento gruppo stanza nel file MA

		for (unsigned int el = 0; el < zone_mesh_room_header.nElements; el++)
		{
			Mesh element;				// Classe provvisoria contente i valori letti dal file ZONE. Va copiata nell'apposito array FBX e/o MA al termine dell'estrazione
			stringstream ssname, ssmaterial, ssbbname;
			ssname << AOD_IO.levelname << "_" << zonename << "_ROOM_" << r_RMX << "_OBJ_" << el;
			ssbbname << ssname.str() << "_BB";
			element.name = ssname.str();
			element.parent = room.name;
			element.layer = Zone_layer_name.str();
			element.FBX_parent = hashID(room.name, "Group");

			// Lettura dati elemento
			XYZ BBmin, BBmax;
			zonefile.seekg(elements_position + el * 64);
			zonefile.seekg(4, ios_base::cur);		// Salta nElement_Triangles
			zonefile.read(reinterpret_cast<char*>(&zone_mesh_element.nElement_Indices), sizeof(zone_mesh_element.nElement_Indices));	// Numero di indici dello strip
			zonefile.read(reinterpret_cast<char*>(&zone_mesh_element.Offset), sizeof(zone_mesh_element.Offset));						// Offset nello strip
			zonefile.read(reinterpret_cast<char*>(&zone_mesh_element.Material_Ref), sizeof(zone_mesh_element.Material_Ref));			// ID materiale
			zonefile.seekg(12, ios_base::cur);		// Salta Unknown1, Vbuffer_min e Vbuffer_max
			zonefile.read(reinterpret_cast<char*>(&zone_mesh_element.Draw_mode), sizeof(zone_mesh_element.Draw_mode));					// Tipologia di rendering
			zonefile.read(reinterpret_cast<char*>(&BBmin.x), sizeof(zone_mesh_element.BB_Xmin));										// Bounding box X min
			zonefile.read(reinterpret_cast<char*>(&BBmin.y), sizeof(zone_mesh_element.BB_Ymin));										// Bounding box Y min
			zonefile.read(reinterpret_cast<char*>(&BBmin.z), sizeof(zone_mesh_element.BB_Zmin));										// Bounding box Z min
			zonefile.seekg(4, ios_base::cur);		// Salta Unknown2
			zonefile.read(reinterpret_cast<char*>(&BBmax.x), sizeof(zone_mesh_element.BB_Xmax));										// Bounding box X max
			zonefile.read(reinterpret_cast<char*>(&BBmax.y), sizeof(zone_mesh_element.BB_Ymax));										// Bounding box Y max
			zonefile.read(reinterpret_cast<char*>(&BBmax.z), sizeof(zone_mesh_element.BB_Xmax));										// Bounding box Z max
			FBX.Geometry.push_back(DrawBox(ssbbname.str(), room.name, Zone_BB_layer_name.str(), BBmin, BBmax, 0x35500000));
			MA.Mesh.push_back(DrawBox(ssbbname.str(), room.name, Zone_BB_layer_name.str(), BBmin, BBmax, 0x35500000));

			// Se l'elemento non contiene almeno 1 triangolo (numero indici almeno pari a 3) viene saltato
			if (zone_mesh_element.nElement_Indices < 3)
			{
				msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << element.name << " does not contain any triangle. Only Bounding Box will be exported.";
				continue;
			}

			// Aggiunta collegamento a materiale
			ssmaterial << AOD_IO.levelname << "_" << zonename << "_Material_" << zone_mesh_element.Material_Ref;
			element.material_name = ssmaterial.str();

			// Controllo double sided e shadow map nel materiale associato
			zonefile.seekg(zone_header.TEXTURE_PTR + 16 + zone_mesh_element.Material_Ref * 24);
			zonefile.read(reinterpret_cast<char*>(&zone_materials_list.TextureMode), sizeof(zone_materials_list.TextureMode));
			zonefile.read(reinterpret_cast<char*>(&zone_materials_list.DoubleSided), sizeof(zone_materials_list.DoubleSided));
			element.doublesided = CheckDoubleSided(zone_materials_list.TextureMode, zone_materials_list.DoubleSided);
			element.uv_set2_flag = CheckShadowMap(zone_materials_list.TextureMode);

			// Lettura strip
			vector <unsigned int> strip(zone_mesh_element.nElement_Indices);
			vector <unsigned int> vertex_array;
			zonefile.seekg(strip_position + zone_mesh_element.Offset * 2);					// Posizionamento cursore di lettura all'inizio dello strip dell'elemento el
			for (unsigned int i = 0; i < zone_mesh_element.nElement_Indices; i++)			// Lettura strip del singolo elemento el
				zonefile.read(reinterpret_cast<char*>(&strip[i]), sizeof(zone_mesh_strip.Index));
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
				zonefile.seekg(vertex_position + vertex_array[v] * 40);
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.X), sizeof(zone_mesh_vertex.X));					// Coordinata X
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Y), sizeof(zone_mesh_vertex.Y));					// Coordinata Y
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Z), sizeof(zone_mesh_vertex.Z));					// Coordinata Z
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.U1), sizeof(zone_mesh_vertex.U1));					// UV
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.V1), sizeof(zone_mesh_vertex.V1));					// UV
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.U2), sizeof(zone_mesh_vertex.U2));					// UV shadow map
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.V2), sizeof(zone_mesh_vertex.V2));					// UV shadow map
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Xn), sizeof(zone_mesh_vertex.Xn));					// Vertex normal X
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Yn), sizeof(zone_mesh_vertex.Yn));					// Vertex normal Y
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Zn), sizeof(zone_mesh_vertex.Zn));					// Vertex normal Z
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Xtg), sizeof(zone_mesh_vertex.Xtg));				// Vertex tangent X
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Ytg), sizeof(zone_mesh_vertex.Ytg));				// Vertex tangent Y
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Ztg), sizeof(zone_mesh_vertex.Ztg));				// Vertex tangent Z
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Xbn), sizeof(zone_mesh_vertex.Xbn));				// Vertex binormal X
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Ybn), sizeof(zone_mesh_vertex.Ybn));				// Vertex binormal Y
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Zbn), sizeof(zone_mesh_vertex.Zbn));				// Vertex binormal Z
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.VC_red), sizeof(zone_mesh_vertex.VC_red));			// Vertex color R
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.VC_green), sizeof(zone_mesh_vertex.VC_green));		// Vertex color G
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.VC_blue), sizeof(zone_mesh_vertex.VC_blue));		// Vertex color B
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
		zonefile.seekg(header_position + zone_mesh_room_header.Room_size + 4);			// Posiziona l'indice di lettura all'inizio della stanza successiva
	}
	zonefile.close();
	return true;
}