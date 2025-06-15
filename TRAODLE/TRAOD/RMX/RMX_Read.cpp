#include "stdafx.h"
#include "Misc_Functions.h"
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "TRAOD/RMX/RMX_Struct.h"
#include "TRAOD/RMX/RMX_Functions.h"


bool RMX_Read (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	RMX_HEADER rmx_header;
	RMX_OFFSETS rmx_offsets;
	RMX_ROOM rmx_room;

	/////////////////  SWITCHES GENERALI DI ESPORTAZIONE DEL FILE RMX
	bool Export_Bounding_Box = true;
	bool Export_Light = true;
	bool Export_Water = true;
	bool Export_PS2_Room_Obj = true;
	bool Export_Audio_Locator = true;
	bool Export_Fog = true;
	bool Export_Portals = true;
	//////////////////////////////////////////////////////////////////


	// INIZIALIZZAZIONE LAYERS FILE MA. L'INSERIMENTO NEL VETTORE LAYERS AVVIENE SUCCESSIVAMENTE SOTTO IL CONTROLLO DEGLI SWITCHES GENERALI
	Layer Bounding_boxes_layer, Water_boxes_layer, Lights_layer, PS2_room_objects_layer, Audio_locators_layer, Portals_layer;
	stringstream Bounding_boxes_layer_name, Water_boxes_layer_name, Lights_layer_name, PS2_room_objects_layer_name, Audio_locators_layer_name, Portals_layer_name;
	
	Bounding_boxes_layer_name << AOD_IO.levelname << "_bounding_boxes";
	Water_boxes_layer_name << AOD_IO.levelname << "_water_boxes";
	Lights_layer_name << AOD_IO.levelname << "_lights";
	PS2_room_objects_layer_name << AOD_IO.levelname << "_PS2_room_objects";
	Audio_locators_layer_name << AOD_IO.levelname << "_audio_locators";
	Portals_layer_name << AOD_IO.levelname << "_portals";

	Bounding_boxes_layer.name = Bounding_boxes_layer_name.str();
	Bounding_boxes_layer.Label_ARGB = 0xFF707070;
	Bounding_boxes_layer.Type = LayerDisplayType::Template;
	Water_boxes_layer.name = Water_boxes_layer_name.str();
	Water_boxes_layer.Label_ARGB = 0x20007BFF;
	Lights_layer.name = Lights_layer_name.str();
	Lights_layer.Label_ARGB = 0xFFFFFF00;
	Lights_layer.Type = LayerDisplayType::Normal;
	PS2_room_objects_layer.name = PS2_room_objects_layer_name.str();
	PS2_room_objects_layer.Label_ARGB = 0xFFFFAC2E;
	Audio_locators_layer.name = Audio_locators_layer_name.str();
	Audio_locators_layer.Label_ARGB = 0xFFFFACD0;
	Portals_layer.name = Portals_layer_name.str();
	Portals_layer.Label_ARGB = 0x3FD400C7;
	Bounding_boxes_layer.Visible = Water_boxes_layer.Visible = PS2_room_objects_layer.Visible = Audio_locators_layer.Visible = Portals_layer.Visible = false;
	MA.Layer.push_back(Bounding_boxes_layer);					// Inserimento layer bounding boxes nel file MA
	MA.Layer.push_back(Water_boxes_layer);						// Inserimento layer box acqua nel file MA
	MA.Layer.push_back(Lights_layer);							// Inserimento layer luci nel file MA
	MA.Layer.push_back(PS2_room_objects_layer);					// Inserimento layer oggetti PS2 nel file MA
	MA.Layer.push_back(Audio_locators_layer);					// Inserimento layer audio locators nel file MA
	MA.Layer.push_back(Portals_layer);							// Inserimento layer portals nel file MA


	string debug = AOD_IO.levelname;
	debug.append("_RMXdebug.txt");
	ofstream out;
	out.open(debug);


	// LETTURA FILE RMX
	ifstream rmxfile(filename, std::ios::binary);
	if (!rmxfile.is_open())
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " not found.";
		return false;
	}

	// Lettura header
	rmxfile.read(reinterpret_cast<char*>(&rmx_header.Version), sizeof(rmx_header.Version));						// Lettura versione file RMX
	
	if (rmx_header.Version != 3.6f)				// Se la versione di file RMX non è la 3.6
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "RMX version mismatch.";
		return false;
	}

	rmxfile.read(reinterpret_cast<char*>(&rmx_header.DefaultZone), sizeof(rmx_header.DefaultZone));				// Lettura zona di default
	rmxfile.read(reinterpret_cast<char*>(&rmx_header.DefaultReverb), sizeof(rmx_header.DefaultReverb));			// Lettura tipo di riverbero di default
	rmxfile.seekg(4, ios_base::cur);
	rmxfile.read(reinterpret_cast<char*>(&rmx_header.nRooms), sizeof(rmx_header.nRooms));						// Lettura numero di stanze

	msg(msg::TGT::FILE, msg::TYP::LOG) << "File version: 3.6";
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Number of rooms: " << rmx_header.nRooms;

	out << "----- Tomb Raider: The Angel of Darkness Level Exporter by Nakamichi680 ------\n";
	out << "\nSelected file: " << AOD_IO.levelname << endl;
	out << "\nRMX Version: " << rmx_header.Version;
	out << "\nDefault Zone: " << rmx_header.DefaultZone;
	out << "\nDefault Reverb: " << Reverb_preset(rmx_header.DefaultReverb);
	out << "\nNumber of rooms: " << rmx_header.nRooms;
	out << "\n------------\n\n";

	// Lettura stanze
	for (unsigned int r = 0; r < rmx_header.nRooms; r++)
	{
		XYZ BBmin, BBmax;
		rmxfile.seekg(20 + 4 * r, ios_base::beg);
		rmxfile.read(reinterpret_cast<char*>(&rmx_offsets.Offset), sizeof(rmx_offsets.Offset));
		rmxfile.seekg(rmx_offsets.Offset + 100);
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Room_ID), sizeof(rmx_room.Room_ID));
		rmxfile.seekg(8, ios_base::cur);
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Room_Xt), sizeof(rmx_room.Room_Xt));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Room_Yt), sizeof(rmx_room.Room_Yt));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Room_Zt), sizeof(rmx_room.Room_Zt));
		rmxfile.seekg(4, ios_base::cur);
		rmxfile.read(reinterpret_cast<char*>(&BBmin.x), sizeof(rmx_room.BB_Xmin));
		rmxfile.read(reinterpret_cast<char*>(&BBmin.y), sizeof(rmx_room.BB_Ymin));
		rmxfile.read(reinterpret_cast<char*>(&BBmin.z), sizeof(rmx_room.BB_Zmin));
		rmxfile.seekg(4, ios_base::cur);
		rmxfile.read(reinterpret_cast<char*>(&BBmax.x), sizeof(rmx_room.BB_Xmax));
		rmxfile.read(reinterpret_cast<char*>(&BBmax.y), sizeof(rmx_room.BB_Ymax));
		rmxfile.read(reinterpret_cast<char*>(&BBmax.z), sizeof(rmx_room.BB_Zmax));
		rmxfile.seekg(12, ios_base::cur);
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Room_ID2), sizeof(rmx_room.Room_ID2));
		rmxfile.seekg(12, ios_base::cur);
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.P1_Object_First), sizeof(rmx_room.P1_Object_First));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.P1_Object_Last), sizeof(rmx_room.P1_Object_Last));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.P2_Light_First), sizeof(rmx_room.P2_Light_First));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.P2_Light_Last), sizeof(rmx_room.P2_Light_Last));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset2), sizeof(rmx_room.Offset2));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset3), sizeof(rmx_room.Offset3));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.P4_Trigger_First), sizeof(rmx_room.P4_Trigger_First));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.P4_Trigger_Last), sizeof(rmx_room.P4_Trigger_Last));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset6), sizeof(rmx_room.Offset10));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset7), sizeof(rmx_room.Offset10));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset8), sizeof(rmx_room.Offset10));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset9), sizeof(rmx_room.Offset10));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset10), sizeof(rmx_room.Offset10));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset11), sizeof(rmx_room.Offset10));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.P8_Water_First), sizeof(rmx_room.P8_Water_First));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.P8_Water_Last), sizeof(rmx_room.P8_Water_Last));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.P9_Fog_First), sizeof(rmx_room.P9_Fog_First));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.P9_Fog_Last), sizeof(rmx_room.P9_Fog_Last));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset16), sizeof(rmx_room.Offset10));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset17), sizeof(rmx_room.Offset10));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.P11_PS2_Room_Obj_First), sizeof(rmx_room.P11_PS2_Room_Obj_First));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.P11_PS2_Room_Obj_Last), sizeof(rmx_room.P11_PS2_Room_Obj_Last));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.P12_Audio_Locator_First), sizeof(rmx_room.P12_Audio_Locator_First));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.P12_Audio_Locator_Last), sizeof(rmx_room.P12_Audio_Locator_Last));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset22), sizeof(rmx_room.Offset2));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset23), sizeof(rmx_room.Offset3));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset24), sizeof(rmx_room.Offset24));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset25), sizeof(rmx_room.Offset10));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.P15_Portal_First), sizeof(rmx_room.P15_Portal_First));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.nPortals), sizeof(rmx_room.nPortals));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset28), sizeof(rmx_room.Offset10));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset29), sizeof(rmx_room.Offset10));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset30), sizeof(rmx_room.Offset10));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.ReverbID), sizeof(rmx_room.ReverbID));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset32), sizeof(rmx_room.Offset2));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset33), sizeof(rmx_room.Offset3));

		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Unknown1), sizeof(rmx_room.Offset3));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Unknown2), sizeof(rmx_room.Offset3));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Unknown3), sizeof(rmx_room.Offset3));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Unknown4), sizeof(rmx_room.Offset3));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Unknown5), sizeof(rmx_room.Offset3));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Unknown6), sizeof(rmx_room.Offset3));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Unknown7), sizeof(rmx_room.Offset3));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Unknown8), sizeof(rmx_room.Offset3));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Unknown9), sizeof(rmx_room.Offset3));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Unknown10), sizeof(rmx_room.Offset3));

		// Esportazione gruppo stanza nei file FBX e MA
		Transform FBX_MA_room_group;
		stringstream room_name, bbox_name;
		room_name << AOD_IO.levelname << "_ROOM_" << r << "_" << std::hex << rmx_room.Room_ID2 << std::dec;
		bbox_name << AOD_IO.levelname << "_ROOM_" << r << "_Bounding_Box";
		FBX_MA_room_group.name = room_name.str();
		FBX.Group.push_back(FBX_MA_room_group);			// Inserimento gruppo stanza nel vettore dei gruppi del file FBX
		MA.Transform.push_back(FBX_MA_room_group);		// Inserimento gruppo stanza nel vettore dei gruppi del file MA
		if (Export_Bounding_Box)
		{
			FBX.Geometry.push_back(DrawBox(bbox_name.str(), room_name.str(), "", BBmin, BBmax, 0x2A000000));
			MA.Mesh.push_back(DrawBox(bbox_name.str(), room_name.str(), Bounding_boxes_layer_name.str(), BBmin, BBmax, 0x2A000000));
		}


		out << "ROOM " << r << endl;
		out << "Reverb preset: " << Reverb_preset(rmx_room.ReverbID) << endl;
		if (rmx_room.P15_Portal_First != 0)
			out << "Number of portals: " << rmx_room.nPortals << endl;

		if (rmx_room.P1_Object_First != 0)
			out << "Object first: " << rmx_room.P1_Object_First << endl;
		if (rmx_room.P1_Object_Last != 0)
			out << "Object last: " << rmx_room.P1_Object_Last << endl;
		/*if (rmx_room.P2_Light_First != 0)
			out << "Light first: " << rmx_room.P2_Light_First << endl;
		if (rmx_room.P2_Light_Last != 0)
			out << "Light last: " << rmx_room.P2_Light_Last << endl;*/
		
		if (rmx_room.Offset2 != 0)
			out << "Offset 2: " << rmx_room.Offset2 << endl;
		if (rmx_room.Offset3 != 0)
			out << "Offset 3: " << rmx_room.Offset3 << endl;

		if (rmx_room.P4_Trigger_First != 0)
			out << "Trigger first: " << rmx_room.P4_Trigger_First << endl;
		if (rmx_room.P4_Trigger_Last != 0)
			out << "Trigger last: " << rmx_room.P4_Trigger_Last << endl;

		if (rmx_room.Offset6 != 0)
			out << "Offset 6: " << rmx_room.Offset6 << endl;
		if (rmx_room.Offset7 != 0)
			out << "Offset 7: " << rmx_room.Offset7 << endl;
		if (rmx_room.Offset8 != 0)
			out << "Offset 8: " << rmx_room.Offset8 << endl;
		if (rmx_room.Offset9 != 0)
			out << "Offset 9: " << rmx_room.Offset9 << endl;
		if (rmx_room.Offset10 != 0)
			out << "Offset 10: " << rmx_room.Offset10 << endl;
		if (rmx_room.Offset11 != 0)
			out << "Offset 11: " << rmx_room.Offset11 << endl;

		if (rmx_room.Offset16 != 0)
			out << "Offset 16: " << rmx_room.Offset16 << endl;
		if (rmx_room.Offset17 != 0)
			out << "Offset 17: " << rmx_room.Offset17 << endl;

		if (rmx_room.Offset22 != 0)
			out << "Offset 22: " << rmx_room.Offset22 << endl;
		if (rmx_room.Offset23 != 0)
			out << "Offset 23: " << rmx_room.Offset23 << endl;
		if (rmx_room.Offset24 != 0)
			out << "Offset 24: " << rmx_room.Offset24 << endl;
		if (rmx_room.Offset25 != 0)
			out << "Offset 25: " << rmx_room.Offset25 << endl;
		if (rmx_room.Offset28 != 0)
			out << "Offset 28: " << rmx_room.Offset28 << endl;
		if (rmx_room.Offset29 != 0)
			out << "Offset 29: " << rmx_room.Offset29 << endl;
		if (rmx_room.Offset30 != 0)
			out << "Offset 30: " << rmx_room.Offset30 << endl;
		if (rmx_room.Offset32 != 0)
			out << "Offset 32: " << rmx_room.Offset32 << endl;
		if (rmx_room.Offset33 != 0)
			out << "Offset 33: " << rmx_room.Offset33 << endl;

		out << "Unknown 1: " << rmx_room.Unknown1 << endl;
		out << "Unknown 2: " << rmx_room.Unknown2 << endl;
		out << "Unknown 3: " << rmx_room.Unknown3 << endl;
		out << "Unknown 4: " << rmx_room.Unknown4 << endl;
		out << "Unknown 5: " << rmx_room.Unknown5 << endl;
		out << "Unknown 6: " << rmx_room.Unknown6 << endl;
		out << "Unknown 7: " << rmx_room.Unknown7 << endl;
		out << "Unknown 8: " << rmx_room.Unknown8 << endl;
		out << "Unknown 9: " << rmx_room.Unknown9 << endl;
		out << "Unknown 10: " << rmx_room.Unknown10 << endl;

		if (rmx_room.P2_Light_First != 0 && Export_Light)						// Lettura ed esportazione luci statiche
		{
			rmxfile.seekg(rmx_offsets.Offset + rmx_room.P2_Light_First, ios_base::beg);
			for (unsigned int l = 0; l < (rmx_room.P2_Light_Last - rmx_room.P2_Light_First) / 144 + 1; l++)
			{
				stringstream temp;
				temp << AOD_IO.levelname << "_ROOM_" << r << "_LIGHT_" << l;
				out << "#LIGHT " << l << endl;
				RMX_Light(rmxfile, temp.str(), room_name.str(), Lights_layer_name.str(), out, FBX, MA);
			}
		}

		if (rmx_room.P8_Water_First != 0 && Export_Water)						// Lettura ed esportazione acqua
		{
			rmxfile.seekg(rmx_offsets.Offset + rmx_room.P8_Water_First, ios_base::beg);
			for (unsigned int w = 0; w < (rmx_room.P8_Water_Last - rmx_room.P8_Water_First) / 432 + 1; w++)
			{
				stringstream temp;
				temp << AOD_IO.levelname << "_ROOM_" << r << "_WATER_" << w;
				out << "#WATER " << w << endl;
				RMX_Water(rmxfile, temp.str(), room_name.str(), Water_boxes_layer_name.str(), out, FBX, MA);
			}
		}

		if (rmx_room.P11_PS2_Room_Obj_First != 0 && Export_PS2_Room_Obj)		// Lettura ed esportazione coordinate oggetti PS2 (inutile)
		{
			rmxfile.seekg(rmx_offsets.Offset + rmx_room.P11_PS2_Room_Obj_First, ios_base::beg);
			for (unsigned int o = 0; o < (rmx_room.P11_PS2_Room_Obj_Last - rmx_room.P11_PS2_Room_Obj_First) / 144 + 1; o++)
			{
				stringstream temp;
				temp << AOD_IO.levelname << "_ROOM_" << r << "_PS2OBJ_" << o;
				out << "#PS2OBJ " << o << endl;
				RMX_PS2_Room_Obj(rmxfile, temp.str(), room_name.str(), PS2_room_objects_layer_name.str(), out, FBX, MA);
			}
		}

		if (rmx_room.P12_Audio_Locator_First != 0 && Export_Audio_Locator)		// Lettura ed esportazione audio locators
		{
			rmxfile.seekg(rmx_offsets.Offset + rmx_room.P12_Audio_Locator_First, ios_base::beg);
			for (unsigned int a = 0; a < (rmx_room.P12_Audio_Locator_Last - rmx_room.P12_Audio_Locator_First) / 368 + 1; a++)
			{
				stringstream temp;
				temp << AOD_IO.levelname << "_ROOM_" << r << "_AUDIO_LOCATOR_" << a;
				out << "#AUDIO LOCATOR " << a << endl;
				RMX_Audio_Locator(rmxfile, temp.str(), room_name.str(), Audio_locators_layer_name.str(), out, FBX, MA);
			}
		}

		if (rmx_room.P15_Portal_First != 0 && Export_Portals)					// Lettura ed esportazione portali
		{
			rmxfile.seekg(rmx_offsets.Offset + rmx_room.P15_Portal_First, ios_base::beg);
			for (unsigned int p = 0; p < rmx_room.nPortals; p++)
			{
				stringstream temp;
				temp << AOD_IO.levelname << "_ROOM_" << r << "_PORTAL_" << p;
				out << "#PORTAL " << p << endl;
				RMX_Portal(rmxfile, temp.str(), room_name.str(), Portals_layer_name.str(), out, FBX, MA);
			}
		}

		out << endl << endl;





	}

	

	return true;
}