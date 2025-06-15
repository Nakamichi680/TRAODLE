#include "stdafx.h"
#include "TRAOD/ZONE/ZONE_Struct.h"
#include "TRAOD/ZONE/ZONE_Functions.h"
#include "Misc_Functions.h"


bool ZONE_Read_Fakes (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	ZONE_HEADER zone_header;
	ZONE_PS2_OBJ_SLOTS zone_ps2_obj_slots;
	ZONE_PS2_OBJ zone_ps2_obj;
	ZONE_MATERIALS_LIST zone_materials_list;
	ZONE_MESH_HEADER1 zone_mesh_header1;
	ZONE_MESH_ROOM_HEADER zone_mesh_room_header;
	ZONE_MESH_HEADER2 zone_mesh_header2;
	ZONE_MESH_OBJECT_HEADER zone_mesh_object_header;
	ZONE_MESH_VERTEX zone_mesh_vertex;
	ZONE_MESH_STRIP zone_mesh_strip;
	ZONE_MESH_ELEMENT zone_mesh_element;
	ZONE_FAKES_HEADER zone_fakes_header;
	ZONE_FAKES_ELEMENT zone_fakes_element;
	string zonename = filename;
	zonename.erase(0, (zonename.find(".Z") + 1));

	///////////////////    APERTURA FILE ZONE
	ifstream zonefile(filename, std::ios::binary);
	if (!zonefile.is_open())
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " not found.";
		return false;
	}

	// Lettura Header
	zonefile.read(reinterpret_cast<char*>(&zone_header.ZONE_ID), sizeof(zone_header.ZONE_ID));

	if (zone_header.ZONE_ID != 32)				// Se il file ZONE non è valido
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " is not a valid ZONE file.";
		return false;
	}

	zonefile.read(reinterpret_cast<char*>(&zone_header.TEXTURE_PTR), sizeof(zone_header.TEXTURE_PTR));
	zonefile.read(reinterpret_cast<char*>(&zone_header.PS2_OBJ_PTR), sizeof(zone_header.PS2_OBJ_PTR));
	zonefile.read(reinterpret_cast<char*>(&zone_header.MESH_PTR), sizeof(zone_header.MESH_PTR));
	zonefile.read(reinterpret_cast<char*>(&zone_header.EOF_PTR), sizeof(zone_header.EOF_PTR));
	
	// Lettura header fakes (se presente)
	zonefile.seekg(zone_header.EOF_PTR);
	zonefile.read(reinterpret_cast<char*>(&zone_fakes_header.P1_Fake_First), sizeof(zone_fakes_header.P1_Fake_First));
	if (zone_fakes_header.P1_Fake_First == 0)	// Se questo valore è pari a 0 non ci sono Fakes e la funzione termina
		return true;

	// CREAZIONE LAYERS ZONE FAKES PER FILE MA
	Layer Zone_Fakes_layer, Zone_Fakes_BB_layer;
	stringstream Zone_Fakes_layer_name, Zone_Fakes_BB_layer_name;
	Zone_Fakes_layer_name << AOD_IO.levelname << "_" << zonename << "_Fakes";
	Zone_Fakes_BB_layer_name << Zone_Fakes_layer_name.str() << "_bounding_boxes";
	Zone_Fakes_layer.name = Zone_Fakes_layer_name.str();
	Zone_Fakes_layer.Label_ARGB = 0xFF00D822;
	MA.Layer.push_back(Zone_Fakes_layer);
	Zone_Fakes_BB_layer.name = Zone_Fakes_BB_layer_name.str();
	Zone_Fakes_BB_layer.Label_ARGB = 0xFF0000FF;
	Zone_Fakes_BB_layer.Visible = false;
	Zone_Fakes_BB_layer.Type = LayerDisplayType::Template;
	MA.Layer.push_back(Zone_Fakes_BB_layer);
	
	// Ricerca posizione inizio blocco Objects
	zonefile.seekg(zone_header.MESH_PTR);
	zonefile.read(reinterpret_cast<char*>(&zone_mesh_header1.nRooms), sizeof(zone_mesh_header1.nRooms));
	for (unsigned int r = 0; r < zone_mesh_header1.nRooms; r++)													// Questo macroblocco "for" legge le dimensioni di ogni stanza per saltarla
	{
		zonefile.read(reinterpret_cast<char*>(&zone_mesh_room_header.RoomID), sizeof(zone_mesh_room_header.RoomID));			// ID stanza
		zonefile.read(reinterpret_cast<char*>(&zone_mesh_room_header.Room_size), sizeof(zone_mesh_room_header.Room_size));		// Dimensioni stanza in bytes
		zonefile.seekg(zone_mesh_room_header.Room_size - 4, ios_base::cur);														// Salta la stanza
	}

	// Salvataggio offsets di tutti gli oggetti
	zonefile.read(reinterpret_cast<char*>(&zone_mesh_header2.nObjects), sizeof(zone_mesh_header2.nObjects));
	vector <streamoff> Object_offsets (zone_mesh_header2.nObjects);
	for (unsigned int o = 0; o < zone_mesh_header2.nObjects; o++)
	{
		Object_offsets[o] = zonefile.tellg();
		zonefile.read(reinterpret_cast<char*>(&zone_mesh_object_header.Object_size), sizeof(zone_mesh_object_header.Object_size));		// Dimensioni oggetto in bytes
		zonefile.seekg(zone_mesh_object_header.Object_size - 4, ios_base::cur);															// Salta la stanza
	}

	// Lettura Fakes
	zonefile.seekg(zone_fakes_header.P1_Fake_First);
	do {
		// Lettura informazioni elemento fake
		zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.BegPrev), sizeof(zone_fakes_element.BegPrev));
		zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.BegNext), sizeof(zone_fakes_element.BegNext));
		zonefile.seekg(16, ios_base::cur);		// Salta null1/2/3/4
		zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.Unknown1), sizeof(zone_fakes_element.Unknown1));
		zonefile.seekg(4, ios_base::cur);		// Salta null5
		zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.Xpos), sizeof(zone_fakes_element.Xpos));
		zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.Ypos), sizeof(zone_fakes_element.Ypos));
		zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.Zpos), sizeof(zone_fakes_element.Zpos));
		zonefile.seekg(36, ios_base::cur);		// Salta Wpos, Unknown2, Unknown3, null6
		zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.Xscal), sizeof(zone_fakes_element.Xscal));
		zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.Yscal), sizeof(zone_fakes_element.Yscal));
		zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.Zscal), sizeof(zone_fakes_element.Zscal));
		zonefile.seekg(8, ios_base::cur);		// Salta null7
		zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.Unknown4), sizeof(zone_fakes_element.Unknown4));
		zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.Unknown5), sizeof(zone_fakes_element.Unknown5));
		zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.Static_FollowCam), sizeof(zone_fakes_element.Static_FollowCam));
		if (zone_fakes_element.Static_FollowCam == 8)
			msg(msg::TGT::FILE_CONS, msg::TYP::DBG) << "Fake " << hex << zone_fakes_element.Unknown4 << dec << " is camera oriented.";
		zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.BB_Xmin), sizeof(zone_fakes_element.BB_Xmin));
		zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.BB_Ymin), sizeof(zone_fakes_element.BB_Ymin));
		zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.BB_Zmin), sizeof(zone_fakes_element.BB_Zmin));
		zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.Unknown6), sizeof(zone_fakes_element.Unknown6));
		zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.Fake_List_Index), sizeof(zone_fakes_element.Fake_List_Index));
		zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.BB_Xmax), sizeof(zone_fakes_element.BB_Xmax));
		zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.BB_Ymax), sizeof(zone_fakes_element.BB_Ymax));
		zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.BB_Zmax), sizeof(zone_fakes_element.BB_Zmax));

		// Lettura informazioni fake nel blocco oggetti PS2
		zonefile.seekg(zone_header.PS2_OBJ_PTR + zone_fakes_element.Fake_List_Index * 4 + 8);
		zonefile.read(reinterpret_cast<char*>(&zone_ps2_obj_slots.P1_Object), sizeof(zone_ps2_obj_slots.P1_Object));
		zonefile.seekg(zone_ps2_obj_slots.P1_Object);
		zonefile.read(reinterpret_cast<char*>(&zone_ps2_obj.Unknown1x), sizeof(zone_ps2_obj.Unknown1x));
		zonefile.read(reinterpret_cast<char*>(&zone_ps2_obj.Unknown2y), sizeof(zone_ps2_obj.Unknown2y));
		zonefile.read(reinterpret_cast<char*>(&zone_ps2_obj.Unknown3z), sizeof(zone_ps2_obj.Unknown3z));
		zonefile.read(reinterpret_cast<char*>(&zone_ps2_obj.Unknown4w), sizeof(zone_ps2_obj.Unknown4w));
		zonefile.read(reinterpret_cast<char*>(&zone_ps2_obj.Unknown5x), sizeof(zone_ps2_obj.Unknown5x));
		zonefile.read(reinterpret_cast<char*>(&zone_ps2_obj.Unknown6y), sizeof(zone_ps2_obj.Unknown6y));
		zonefile.read(reinterpret_cast<char*>(&zone_ps2_obj.Unknown7z), sizeof(zone_ps2_obj.Unknown7z));
		zonefile.read(reinterpret_cast<char*>(&zone_ps2_obj.Unknown8w), sizeof(zone_ps2_obj.Unknown8w));

		unsigned int nPart = 0;				// Contatore del numero di parti di cui è composto un singolo oggetto
		//msg(msg::TGT::FILE_CONS, msg::TYP::DBG) << "Reading fake object n. " << zone_fakes_element.Fake_List_Index;

		// Creazione Transform oggetto
		Transform object;
		stringstream temp1;
		temp1 << AOD_IO.levelname << "_" << zonename << "_OBJECT_" << zone_fakes_element.Fake_List_Index;
		object.name = temp1.str();
		object.translate_flag = object.scale_flag = true;
		object.tX = zone_fakes_element.Xpos;
		object.tY = zone_fakes_element.Ypos;
		object.tZ = zone_fakes_element.Zpos;
		object.sX = zone_fakes_element.Xscal;
		object.sY = zone_fakes_element.Yscal;
		object.sZ = zone_fakes_element.Zscal;
		FBX.Group.push_back(object);								// Inserimento gruppo oggetto nel file FBX
		MA.Transform.push_back(object);								// Inserimento gruppo oggetto nel file MA

		do {
			zonefile.read(reinterpret_cast<char*>(&zone_ps2_obj.Obj_ID), sizeof(zone_ps2_obj.Obj_ID));
			if (zone_ps2_obj.Obj_ID == -4)
				zonefile.seekg(24, ios_base::cur);		// Salta 24 bytes
			if (zone_ps2_obj.Obj_ID >= 0)
			{
				//msg(msg::TGT::FILE_CONS, msg::TYP::DBG) << "Reading fake object n. " << zone_fakes_element.Fake_List_Index << "   part " << nPart;
				zonefile.read(reinterpret_cast<char*>(&zone_ps2_obj.Xrot), sizeof(zone_ps2_obj.Xrot));
				zonefile.read(reinterpret_cast<char*>(&zone_ps2_obj.Yrot), sizeof(zone_ps2_obj.Yrot));
				zonefile.read(reinterpret_cast<char*>(&zone_ps2_obj.Yrot), sizeof(zone_ps2_obj.Yrot));
				streamoff next_zone_ps2_obj_position = zonefile.tellg();			// Salva la posizione attuale per tornarci dopo aver letto la mesh dell'oggetto
				zonefile.seekg(Object_offsets[zone_ps2_obj.Obj_ID]);

				// Creazione Transform parte oggetto
				Transform part;
				stringstream temp2;
				temp2 << AOD_IO.levelname << "_" << zonename << "_OBJECT_" << zone_fakes_element.Fake_List_Index << "_PART_" << nPart;
				part.name = temp2.str();
				part.parent = temp1.str();
				part.FBX_parent = hashID(temp1.str(), "Group");
				part.rotate_flag = true;
				part.rX = zone_ps2_obj.Xrot;
				part.rY = zone_ps2_obj.Yrot;
				part.rZ = zone_ps2_obj.Zrot;
				FBX.Group.push_back(part);							// Inserimento gruppo oggetto nel file FBX
				MA.Transform.push_back(part);						// Inserimento gruppo oggetto nel file MA

				// Lettura mesh parte oggetto
				zonefile.seekg(12, ios_base::cur);		// Salta Object_size, Unknown1, Unknown2
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_object_header.nVertices), sizeof(zone_mesh_object_header.nVertices));	// Numero vertici
				zonefile.seekg(4, ios_base::cur);		// Salta unknown3
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_object_header.nIndices), sizeof(zone_mesh_object_header.nIndices));	// Numero indici del triangle strip
				zonefile.seekg(4, ios_base::cur);		// Salta unknown4
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_object_header.nElements), sizeof(zone_mesh_object_header.nElements));	// Numero elementi
				zonefile.seekg(20, ios_base::cur);		// Salta unknown5/6/7/8/9
				streamoff vertex_position = zonefile.tellg();												// Memorizza la posizione iniziale del blocco vertici
				streamoff strip_position = vertex_position + zone_mesh_object_header.nVertices * 40;		// Memorizza la posizione iniziale del blocco triangle strip
				streamoff elements_position = strip_position + zone_mesh_object_header.nIndices * 2;		// Memorizza la posizione iniziale del blocco elementi
		
				for (unsigned int el = 0; el < zone_mesh_object_header.nElements; el++)
				{
					Mesh element;				// Classe provvisoria contente i valori letti dal file ZONE. Va copiata nell'apposito array FBX e/o MA al termine dell'estrazione
					stringstream ssname, ssmaterial, ssbbname;
					ssname << AOD_IO.levelname << "_" << zonename << "_OBJECT_" << zone_fakes_element.Fake_List_Index << "_PART_" << nPart << "_MESH_" << el;
					ssbbname << ssname.str() << "_BB";
					element.name = ssname.str();
					element.parent = part.name;
					element.layer = Zone_Fakes_layer_name.str();
					element.FBX_parent = hashID(part.name, "Group");
					//msg(msg::TGT::FILE_CONS, msg::TYP::DBG) << "Reading " << ssname.str();

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
					FBX.Geometry.push_back(DrawBox(ssbbname.str(), part.name, Zone_Fakes_BB_layer_name.str(), BBmin, BBmax, 0x35500000));
					MA.Mesh.push_back(DrawBox(ssbbname.str(), part.name, Zone_Fakes_BB_layer_name.str(), BBmin, BBmax, 0x35500000));

					// Se l'elemento non contiene almeno 1 triangolo (numero indici almeno pari a 3) viene saltato
					if (zone_mesh_element.nElement_Indices < 3)
					{
						msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << element.name << " does not contain any triangle. Only Bounding Box will be exported.";
						continue;
					}

					// Aggiunta collegamento a materiale
					ssmaterial << AOD_IO.levelname << "_" << zonename << "_Material_" << zone_mesh_element.Material_Ref;
					element.material_name = ssmaterial.str();

					// Controllo double sided nel materiale associato
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
				nPart++;
				zonefile.seekg(next_zone_ps2_obj_position);							// Torna alla posizione precedente per leggere eventuali nuovi oggetti
			}
		} while (zone_ps2_obj.Obj_ID != -1);										// Quando Obj_ID è 0xFFFFFFFF l'oggetto è completato
		zonefile.seekg(zone_fakes_element.BegNext);
	} while (zone_fakes_element.BegNext);
	zonefile.close();
	return true;
}