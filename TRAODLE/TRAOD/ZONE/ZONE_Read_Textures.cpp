#include "stdafx.h"
#include "TRAOD/ZONE/ZONE_Struct.h"
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "Misc_Functions.h"


bool ZONE_Read_Textures (string filename, vector <RoomInfo> RMX_Rooms, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	ZONE_HEADER zone_header;
	ZONE_PS2_OBJ_SLOTS zone_ps2_obj_slots;
	ZONE_PS2_OBJ zone_ps2_obj;
	ZONE_MATERIALS_HEADER zone_materials_header;
	ZONE_MATERIALS_LIST zone_materials_list;
	ZONE_TEXTURE_LIST zone_texture_list;
	ZONE_MESH_HEADER1 zone_mesh_header1;
	ZONE_MESH_ROOM_HEADER zone_mesh_room_header;
	ZONE_MESH_ELEMENT zone_mesh_element;
	ZONE_MESH_HEADER2 zone_mesh_header2;
	ZONE_MESH_OBJECT_HEADER zone_mesh_object_header;
	ZONE_FAKES_HEADER zone_fakes_header;
	ZONE_FAKES_ELEMENT zone_fakes_element;
	
	///////////////////    APERTURA FILE ZONE
	SetCurrentDirectory(AOD_IO.folder_level_lpwstr);				// Imposta la cartella contenente il file ZONE
	ifstream zonefile(filename, std::ios::binary);
	if (!zonefile.is_open())
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " not found.";
		return false;
	}
	
	///////////////////    CREAZIONE CARTELLA PER TEXTURES
	string zonename = filename;
	zonename.erase(0, (zonename.find(".Z") + 1));
	AOD_IO.folder_temp = AOD_IO.folder_zones;
	AOD_IO.folder_temp.append(zonename);
	AOD_IO.folder_temp.append("_Textures");
	string folder_temp2 = AOD_IO.folder_temp;
	LPWSTR folder_temp2_lpwstr = new TCHAR[MAX];
	folder_temp2.append("\\Originals");
	mbstowcs(AOD_IO.folder_temp_lpwstr, AOD_IO.folder_temp.c_str(), MAX);
	mbstowcs(folder_temp2_lpwstr, folder_temp2.c_str(), MAX);
	CreateDirectory(AOD_IO.folder_temp_lpwstr, NULL);				// Crea la cartella \NOMELIVELLO\Zones\Z&&_Textures
	CreateDirectory(folder_temp2_lpwstr, NULL);						// Crea la cartella \NOMELIVELLO\Zones\Z&&_Textures\Originals

	///////////////////    LETTURA FILE ZONE
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
	zonefile.seekg(zone_header.TEXTURE_PTR);																			// Salta all'inizio del gruppo materiali
	zonefile.read(reinterpret_cast<char*>(&zone_materials_header.nMaterials), sizeof(zone_materials_header.nMaterials));
	zonefile.read(reinterpret_cast<char*>(&zone_materials_header.Unknown1), sizeof(zone_materials_header.Unknown1));
	zonefile.read(reinterpret_cast<char*>(&zone_materials_header.nTextures), sizeof(zone_materials_header.nTextures));
	zonefile.read(reinterpret_cast<char*>(&zone_materials_header.Unknown2), sizeof(zone_materials_header.Unknown2));
	zonefile.seekg(zone_materials_header.nMaterials * 24, zonefile.cur);												// Salta il blocco materiali

	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Number of textures: " << zone_materials_header.nTextures;

	streamoff texture_list_position = zonefile.tellg();
    streamoff texture_raw_position_accumulator = texture_list_position + zone_materials_header.nTextures * 40;

	for (unsigned int t = 0; t < zone_materials_header.nTextures; t++)		// Ogni ciclo fa una texture
	{
		unsigned int DXT, ColourBumpShadow;
		zonefile.seekg(texture_list_position + t * 40);
        zonefile.read(reinterpret_cast<char*>(&DXT), sizeof(zone_texture_list.DXT));											// DXT1, DXT3, DXT5 o RGBA
        zonefile.read(reinterpret_cast<char*>(&ColourBumpShadow), sizeof(zone_texture_list.ColourBumpShadow));					// 1 = shadow, 2 = diffuse, 4 = bump, 5 = fur
        zonefile.seekg(8, ios_base::cur);																						// Salta Unknown1 e Unknown2
        zonefile.read(reinterpret_cast<char*>(&zone_texture_list.Mips), sizeof(zone_texture_list.Mips));						// MIPS
        zonefile.read(reinterpret_cast<char*>(&zone_texture_list.Xsize), sizeof(zone_texture_list.Xsize));						// Dimensioni texture asse X
        zonefile.read(reinterpret_cast<char*>(&zone_texture_list.Ysize), sizeof(zone_texture_list.Ysize));						// Dimensioni texture asse Y
        zonefile.read(reinterpret_cast<char*>(&zone_texture_list.RAWsize), sizeof(zone_texture_list.RAWsize));					// Dimensioni texture in bytes
		zonefile.seekg(texture_raw_position_accumulator);							// Posiziona l'indice di lettura all'inizio dei dati della texture
		char* buffer = new char[zone_texture_list.RAWsize];							// Buffer di lettura
		zonefile.read(buffer, zone_texture_list.RAWsize);							// Legge i dati raw della texture e li mette nel buffer
		texture_raw_position_accumulator += zone_texture_list.RAWsize;

		stringstream texture_name, pathfilename, filename1, filename2;

		// Esportazione file textures
		if (DXT == 827611204)											// DXT1
		{
			filename1 << AOD_IO.levelname << "_" << zonename << "_" << t << ".dds";
			filename2 << AOD_IO.levelname << "_" << zonename << "_" << t << ".tga";
			SetCurrentDirectory(folder_temp2_lpwstr);
			Texture_RAWtoDDS(filename1.str(), zone_texture_list.Xsize, zone_texture_list.Ysize, zone_texture_list.Mips, zone_texture_list.RAWsize, DDSType::DXT1, true, buffer);
			char* buffer_rgb = new char[zone_texture_list.Xsize * zone_texture_list.Ysize * 3];
			Texture_DXT1toRGB(zone_texture_list.Xsize, zone_texture_list.Ysize, buffer, buffer_rgb);
			SetCurrentDirectory(AOD_IO.folder_temp_lpwstr);
			Texture_RAWtoTGA(filename2.str(), zone_texture_list.Xsize, zone_texture_list.Ysize, zone_texture_list.Xsize * zone_texture_list.Ysize * 3, TGAType::RGB, true, buffer_rgb);
			delete[] buffer_rgb;
		}

		if (DXT == 861165636)											// DXT3
		{
			filename1 << AOD_IO.levelname << "_" << zonename << "_" << t << ".dds";
			filename2 << AOD_IO.levelname << "_" << zonename << "_" << t << ".tga";
			SetCurrentDirectory(folder_temp2_lpwstr);
			Texture_RAWtoDDS(filename1.str(), zone_texture_list.Xsize, zone_texture_list.Ysize, zone_texture_list.Mips, zone_texture_list.RAWsize, DDSType::DXT3, true, buffer);
			char* buffer_rgba = new char[zone_texture_list.Xsize * zone_texture_list.Ysize * 4];
			Texture_DXT3toRGBA(zone_texture_list.Xsize, zone_texture_list.Ysize, buffer, buffer_rgba);
			SetCurrentDirectory(AOD_IO.folder_temp_lpwstr);
			Texture_RAWtoTGA(filename2.str(), zone_texture_list.Xsize, zone_texture_list.Ysize, zone_texture_list.Xsize * zone_texture_list.Ysize * 4, TGAType::RGBA, true, buffer_rgba);
			delete[] buffer_rgba;
		}

		if (DXT == 894720068)											// DXT5
		{
			filename1 << AOD_IO.levelname << "_" << zonename << "_" << t << ".dds";
			SetCurrentDirectory(folder_temp2_lpwstr);
			Texture_RAWtoDDS(filename1.str(), zone_texture_list.Xsize, zone_texture_list.Ysize, zone_texture_list.Mips, zone_texture_list.RAWsize, DDSType::DXT5, true, buffer);
		}

		if (DXT == 21)													// BMP
		{
			filename1 << AOD_IO.levelname << "_" << zonename << "_" << t << ".bmp";
			filename2 << AOD_IO.levelname << "_" << zonename << "_" << t << ".tga";
			SetCurrentDirectory(folder_temp2_lpwstr);
			Texture_RAWtoBMP(filename1.str(), zone_texture_list.Xsize, zone_texture_list.Ysize, zone_texture_list.RAWsize, BMPType::RGBA, true, buffer);
			SetCurrentDirectory(AOD_IO.folder_temp_lpwstr);
			Texture_RAWtoTGA(filename2.str(), zone_texture_list.Xsize, zone_texture_list.Ysize, zone_texture_list.RAWsize, TGAType::RGBA, true, buffer);
		}
		
		delete[] buffer;
		
		// Creazione classi texture per FBX e MA
		Texture tex;
		texture_name << AOD_IO.levelname << "_" << zonename << "_Texture_" << t;
		pathfilename << AOD_IO.folder_temp << "\\" << filename2.str();
		tex.name = texture_name.str();
		tex.filename = pathfilename.str();

		// Ricerca materiali con shadow map e rispettive mesh collegate (per impostare l'UVset 1 al posto dell'UVset 0 di default)
		for (unsigned int m = 0; m < zone_materials_header.nMaterials; m++)
		{
			zonefile.seekg(zone_header.TEXTURE_PTR + 28 + m * 24);	// Salta a ShadowMapID di ciascun materiale
			zonefile.read(reinterpret_cast<char*>(&zone_materials_list.ShadowMapID), sizeof(zone_materials_list.ShadowMapID));	// Slot 2: shadow map
			if (zone_materials_list.ShadowMapID == t)				// Se trova un materiale con la texture corrente usata come shadow map cerca le mesh che usano quel materiale
			{
				/////////////////////////////////////////////////////////////////////////
				//////////////////////////// CONTROLLO ROOMS ////////////////////////////
				zonefile.seekg(zone_header.MESH_PTR);
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_header1.nRooms), sizeof(zone_mesh_header1.nRooms));	// Lettura numero stanze
				
				for (unsigned int r = 0; r < zone_mesh_header1.nRooms; r++)											// Questo macroblocco "for" legge tutti i dati di ogni stanza
				{
					unsigned int RoomID, r_RMX;		
					streamoff header_position = zonefile.tellg();							// Memorizza la posizione iniziale del blocco vertici
					zonefile.read(reinterpret_cast<char*>(&RoomID), sizeof(zone_mesh_room_header.RoomID));									// ID stanza
					zonefile.read(reinterpret_cast<char*>(&zone_mesh_room_header.Room_size), sizeof(zone_mesh_room_header.Room_size));		// Dimensioni stanza bytes
					zonefile.seekg(8, ios_base::cur);				// Salta i due unknown1/2
					zonefile.read(reinterpret_cast<char*>(&zone_mesh_room_header.nVertices), sizeof(zone_mesh_room_header.nVertices));		// Numero vertici
					zonefile.seekg(4, ios_base::cur);				// Salta unknown3
					zonefile.read(reinterpret_cast<char*>(&zone_mesh_room_header.nIndices), sizeof(zone_mesh_room_header.nIndices));		// Numero indici del triangle strip
					zonefile.seekg(4, ios_base::cur);				// Salta unknown4
					zonefile.read(reinterpret_cast<char*>(&zone_mesh_room_header.nElements), sizeof(zone_mesh_room_header.nElements));		// Numero elementi
					streamoff elements_position = header_position + 56 + zone_mesh_room_header.nVertices * 40 + zone_mesh_room_header.nIndices * 2;

					vector <RoomInfo>::iterator it1 = find_if(RMX_Rooms.begin(), RMX_Rooms.end(), [&RoomID] (const RoomInfo& RMX_Rooms) {return RMX_Rooms.RoomHash == RoomID;});
					if (it1 != RMX_Rooms.end())
						r_RMX = distance(RMX_Rooms.begin (), it1);
					else
						r_RMX = 1000 + r;

					for (unsigned int el = 0; el < zone_mesh_room_header.nElements; el++)
					{
						zonefile.seekg(elements_position + el * 64);
						zonefile.seekg(12, ios_base::cur);			// Salta nElement_Triangles, nElement_Indices e Offset
						zonefile.read(reinterpret_cast<char*>(&zone_mesh_element.Material_Ref), sizeof(zone_mesh_element.Material_Ref));	// Lettura ID materiale
						
						if (zone_mesh_element.Material_Ref == m)	// Se viene trovata una mesh che usa quel materiale viene aggiunto il collegamento all'uvChooser
						{
							stringstream ssname;
							ssname << AOD_IO.levelname << "_" << zonename << "_ROOM_" << r_RMX << "_OBJ_" << el;
							tex.UV_mesh_list.resize(tex.UV_mesh_list.size() + 1);
							tex.UV_mesh_list[tex.UV_mesh_list.size()-1].mesh_name = ssname.str();
							tex.UV_mesh_list[tex.UV_mesh_list.size()-1].UVset = 1;
						}
					}
					zonefile.seekg(header_position + zone_mesh_room_header.Room_size + 4);			// Posiziona l'indice di lettura all'inizio della stanza successiva
				}

				/////////////////////////////////////////////////////////////////////////
				//////////////////////////// CONTROLLO FAKES ////////////////////////////
				zonefile.seekg(zone_header.EOF_PTR);
				zonefile.read(reinterpret_cast<char*>(&zone_fakes_header.P1_Fake_First), sizeof(zone_fakes_header.P1_Fake_First));
				if (zone_fakes_header.P1_Fake_First != 0)
				{
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
						zonefile.seekg(118, ios_base::cur);		// Salta fino alla posizione dell'indice dell'oggetto
						zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.Fake_List_Index), sizeof(zone_fakes_element.Fake_List_Index));

						// Lettura informazioni fake nel blocco oggetti PS2
						zonefile.seekg(zone_header.PS2_OBJ_PTR + zone_fakes_element.Fake_List_Index * 4 + 8);
						zonefile.read(reinterpret_cast<char*>(&zone_ps2_obj_slots.P1_Object), sizeof(zone_ps2_obj_slots.P1_Object));
						zonefile.seekg(zone_ps2_obj_slots.P1_Object + 32);

						unsigned int nPart = 0;				// Contatore del numero di parti di cui è composto un singolo oggetto

						do {
							zonefile.read(reinterpret_cast<char*>(&zone_ps2_obj.Obj_ID), sizeof(zone_ps2_obj.Obj_ID));
							if (zone_ps2_obj.Obj_ID == -4)
								zonefile.seekg(24, ios_base::cur);		// Salta 24 bytes
							if (zone_ps2_obj.Obj_ID >= 0)
							{
								streamoff next_zone_ps2_obj_position = 12 + zonefile.tellg();			// Salva la posizione attuale per tornarci dopo aver letto la mesh dell'oggetto
								zonefile.seekg(Object_offsets[zone_ps2_obj.Obj_ID] + 12);
								zonefile.read(reinterpret_cast<char*>(&zone_mesh_object_header.nVertices), sizeof(zone_mesh_object_header.nVertices));	// Numero vertici
								zonefile.seekg(4, ios_base::cur);		// Salta unknown3
								zonefile.read(reinterpret_cast<char*>(&zone_mesh_object_header.nIndices), sizeof(zone_mesh_object_header.nIndices));	// Numero indici del triangle strip
								zonefile.seekg(4, ios_base::cur);		// Salta unknown4
								zonefile.read(reinterpret_cast<char*>(&zone_mesh_object_header.nElements), sizeof(zone_mesh_object_header.nElements));	// Numero elementi
								streamoff elements_position = 20 + zonefile.tellg() + zone_mesh_object_header.nVertices * 40 + zone_mesh_object_header.nIndices * 2;
		
								for (unsigned int el = 0; el < zone_mesh_object_header.nElements; el++)
								{
									zonefile.seekg(elements_position + el * 64);
									zonefile.seekg(12, ios_base::cur);			// Salta nElement_Triangles, nElement_Indices e Offset
									zonefile.read(reinterpret_cast<char*>(&zone_mesh_element.Material_Ref), sizeof(zone_mesh_element.Material_Ref));	// Lettura ID materiale
						
									if (zone_mesh_element.Material_Ref == m)	// Se viene trovata una mesh che usa quel materiale viene aggiunto il collegamento all'uvChooser
									{
										stringstream ssname;
										ssname << AOD_IO.levelname << "_" << zonename << "_OBJECT_" << zone_fakes_element.Fake_List_Index << "_PART_" << nPart << "_MESH_" << el;
										tex.UV_mesh_list.resize(tex.UV_mesh_list.size() + 1);
										tex.UV_mesh_list[tex.UV_mesh_list.size()-1].mesh_name = ssname.str();
										tex.UV_mesh_list[tex.UV_mesh_list.size()-1].UVset = 1;
									}
								}
								nPart++;
								zonefile.seekg(next_zone_ps2_obj_position);							// Torna alla posizione precedente per leggere eventuali nuovi oggetti
							}
						} while (zone_ps2_obj.Obj_ID != -1);										// Quando Obj_ID è 0xFFFFFFFF l'oggetto è completato
						zonefile.seekg(zone_fakes_element.BegNext);
					} while (zone_fakes_element.BegNext);
				}
			}
		}
		MA.Texture.push_back(tex);
	}
	zonefile.close();
	return true;
}