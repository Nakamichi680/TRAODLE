#include "stdafx.h"
#include "Misc_Functions.h"
#include "AOD_Names.h"
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "TRAOD/RMX/RMX_Struct.h"
#include "TRAOD/RMX/RMX_Functions.h"
#include "TRAOD/SCX/SCX_Functions.h"


// "_<nome originale>" del nodo che inizia alla posizione corrente del file (hash a +100), "" se il nome non e' noto
static string RMX_NodeSuffix (ifstream &rmxfile)
{
	streamoff pos = rmxfile.tellg();
	uint32_t hash = 0;
	rmxfile.seekg(pos + 100);
	rmxfile.read(reinterpret_cast<char*>(&hash), sizeof(hash));
	rmxfile.clear();
	rmxfile.seekg(pos);
	string name = AOD_NodeName(hash);
	return name.empty() ? "" : "_" + AOD_MayaName(name);
}


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
	bool Export_Emitters = true;
	bool Export_Triggers = true;
	bool Export_Portals = true;
	bool Export_Waypoints = true;
	bool Export_Characters = true;
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

	// Layers waypoints: locators, archi del grafo e archi percorribili in una sola direzione
	Layer Waypoints_layer, Waypoint_graph_layer, Waypoint_graph_oneway_layer;
	Waypoints_layer.name = AOD_IO.levelname + "_waypoints";
	Waypoints_layer.Label_ARGB = 0xFF00FF00;
	Waypoint_graph_layer.name = AOD_IO.levelname + "_waypoint_graph";
	Waypoint_graph_layer.Label_ARGB = 0xFF00C0FF;
	Waypoint_graph_oneway_layer.name = AOD_IO.levelname + "_waypoint_graph_oneway";
	Waypoint_graph_oneway_layer.Label_ARGB = 0xFFFF2020;
	if (Export_Waypoints)
	{
		MA.Layer.push_back(Waypoints_layer);
		MA.Layer.push_back(Waypoint_graph_layer);
		MA.Layer.push_back(Waypoint_graph_oneway_layer);
	}
	vector <RMX_WaypointInfo> waypoints;						// Waypoints di tutte le stanze, per la costruzione del grafo

	// Layers personaggi: locators e pedine
	Layer Characters_layer, Character_pawns_layer;
	Characters_layer.name = AOD_IO.levelname + "_characters";
	Characters_layer.Label_ARGB = 0xFFFF8000;
	Character_pawns_layer.name = AOD_IO.levelname + "_character_pawns";
	Character_pawns_layer.Label_ARGB = 0xFFFF8000;
	if (Export_Characters)
	{
		MA.Layer.push_back(Characters_layer);
		MA.Layer.push_back(Character_pawns_layer);
	}
	// Layers emitter (effetti) e trigger, nascosti
	Layer Emitters_layer, Triggers_layer;
	Emitters_layer.name = AOD_IO.levelname + "_emitters";
	Emitters_layer.Label_ARGB = 0xFFFF6000;
	Triggers_layer.name = AOD_IO.levelname + "_triggers";
	Triggers_layer.Label_ARGB = 0xFF00FF40;
	Emitters_layer.Visible = Triggers_layer.Visible = false;
	if (Export_Emitters)
		MA.Layer.push_back(Emitters_layer);
	if (Export_Triggers)
		MA.Layer.push_back(Triggers_layer);
	stringstream triggers_text;									// File <LIVELLO>_TRIGGERS.txt
	unsigned int nEmitters = 0, nTriggers = 0;
	SCX_CollectHashNames();										// Nomi degli hash noti agli script (destinatari di trigger ed emitter)

	map <string, unsigned int> character_names;				// Contatore dei personaggi con lo stesso nome (per rendere unici i nomi)


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

	vector <vector <uint32_t>> room_masks;			// Maschera di visibilita' di ogni stanza
	// Lettura stanze
	for (unsigned int r = 0; r < rmx_header.nRooms; r++)
	{
		Vec3 BBmin, BBmax;
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
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.NL_OBJ_First), sizeof(rmx_room.NL_OBJ_First));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.NL_OBJ_Last), sizeof(rmx_room.NL_OBJ_Last));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.NL_LIGHT_First), sizeof(rmx_room.NL_LIGHT_First));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.NL_LIGHT_Last), sizeof(rmx_room.NL_LIGHT_Last));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.NL_CHARLOC_First), sizeof(rmx_room.NL_CHARLOC_First));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.NL_CHARLOC_Last), sizeof(rmx_room.NL_CHARLOC_Last));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.NL_TRIGGER_First), sizeof(rmx_room.NL_TRIGGER_First));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.NL_TRIGGER_Last), sizeof(rmx_room.NL_TRIGGER_Last));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.NL_PORTAL_First), sizeof(rmx_room.NL_PORTAL_First));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.NL_PORTAL_Last), sizeof(rmx_room.NL_PORTAL_Last));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.NL_WAYPOINT_First), sizeof(rmx_room.NL_WAYPOINT_First));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.NL_WAYPOINT_Last), sizeof(rmx_room.NL_WAYPOINT_Last));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.NL_ACTOR_First), sizeof(rmx_room.NL_ACTOR_First));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.NL_ACTOR_Last), sizeof(rmx_room.NL_ACTOR_Last));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.NL_WATER_First), sizeof(rmx_room.NL_WATER_First));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.NL_WATER_Last), sizeof(rmx_room.NL_WATER_Last));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.NL_EMITTER_First), sizeof(rmx_room.NL_EMITTER_First));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.NL_EMITTER_Last), sizeof(rmx_room.NL_EMITTER_Last));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.NL_CAMERA_First), sizeof(rmx_room.NL_CAMERA_First));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.NL_CAMERA_Last), sizeof(rmx_room.NL_CAMERA_Last));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.P11_PS2_Room_Obj_First), sizeof(rmx_room.P11_PS2_Room_Obj_First));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.P11_PS2_Room_Obj_Last), sizeof(rmx_room.P11_PS2_Room_Obj_Last));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.P12_Audio_Locator_First), sizeof(rmx_room.P12_Audio_Locator_First));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.P12_Audio_Locator_Last), sizeof(rmx_room.P12_Audio_Locator_Last));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset22), sizeof(rmx_room.Offset22));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset23), sizeof(rmx_room.Offset23));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset24), sizeof(rmx_room.Offset24));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset25), sizeof(rmx_room.Offset25));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.P15_Portal_First), sizeof(rmx_room.P15_Portal_First));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.nPortals), sizeof(rmx_room.nPortals));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset28), sizeof(rmx_room.Offset28));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset29), sizeof(rmx_room.Offset29));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Indoor), sizeof(rmx_room.Indoor));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.ReverbID), sizeof(rmx_room.ReverbID));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Collision_set), sizeof(rmx_room.Collision_set));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Collision_set_current), sizeof(rmx_room.Collision_set_current));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset32), sizeof(rmx_room.Offset32));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.Offset33), sizeof(rmx_room.Offset33));

		rmxfile.read(reinterpret_cast<char*>(rmx_room.iFlipRooms), sizeof(rmx_room.iFlipRooms));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.bIsFlipRoom), sizeof(rmx_room.bIsFlipRoom));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.nFlipRooms), sizeof(rmx_room.nFlipRooms));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.FlipIndex), sizeof(rmx_room.FlipIndex));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.OriginalStripCount), sizeof(rmx_room.OriginalStripCount));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.OriginalStripIndices), sizeof(rmx_room.OriginalStripIndices));
		rmxfile.read(reinterpret_cast<char*>(&rmx_room.RoomFlags), sizeof(rmx_room.RoomFlags));

		// Visibilita' delle stanze: maschera di bit con un bit per stanza (RoomMask_Begin, vedi RMX_Struct.h)
		vector <uint32_t> room_mask((rmx_header.nRooms + 31) / 32, 0);
		if (rmx_room.Offset32 != 0 && rmx_room.Offset33 > rmx_room.Offset32)
		{
			rmxfile.seekg(rmx_offsets.Offset + rmx_room.Offset32, ios_base::beg);
			rmxfile.read(reinterpret_cast<char*>(room_mask.data()), room_mask.size() * sizeof(uint32_t));
			if (!((room_mask[r >> 5] >> (r & 31)) & 1))
				msg(msg::TGT::FILE, msg::TYP::WARN) << "Room " << r << ": own bit not set in the room visibility mask.";
		}
		room_masks.push_back(room_mask);

		// Esportazione gruppo stanza nei file FBX e MA
		Transform FBX_MA_room_group;
		stringstream room_name, bbox_name;
		room_name << AOD_IO.levelname << "_ROOM_" << r << "_";					// Nome originale della stanza se noto, altrimenti hash
		if (AOD_NodeName(rmx_room.Room_ID2).empty())
			room_name << std::hex << rmx_room.Room_ID2 << std::dec;
		else
			room_name << AOD_MayaName(AOD_NodeName(rmx_room.Room_ID2));
		bbox_name << AOD_IO.levelname << "_ROOM_" << r << "_Bounding_Box";

		FBX_MA_room_group.name = room_name.str();
		FBX_MA_room_group.int_attributes.push_back(make_pair(string("roomIndex"), (int)r));		// Attributi: indice della stanza e stanze visibili
		string visible_rooms;
		for (uint32_t o = 0; o < rmx_header.nRooms; o++)
			if (o != r && ((room_mask[o >> 5] >> (o & 31)) & 1))
				visible_rooms += (visible_rooms.empty() ? "" : " ") + to_string(o);
		FBX_MA_room_group.string_attributes.push_back(make_pair(string("visibleRooms"), visible_rooms));
		string flip_rooms;																// Versioni alternative della stanza (stanze flip)
		for (uint32_t f = 0; f < min(rmx_room.nFlipRooms, 4u); f++)
			flip_rooms += (flip_rooms.empty() ? "" : " ") + to_string(rmx_room.iFlipRooms[f]);
		if (!flip_rooms.empty())
			FBX_MA_room_group.string_attributes.push_back(make_pair(string("flipRooms"), flip_rooms));
		if (rmx_room.bIsFlipRoom)
			FBX_MA_room_group.int_attributes.push_back(make_pair(string("isFlipRoom"), 1));
		if (rmx_room.Indoor)
			FBX_MA_room_group.int_attributes.push_back(make_pair(string("indoor"), 1));
		FBX_MA_room_group.int_attributes.push_back(make_pair(string("reverb"), (int)rmx_room.ReverbID));		// Preset di riverbero
		FBX_MA_room_group.string_attributes.push_back(make_pair(string("reverbPreset"), Reverb_preset(rmx_room.ReverbID)));
		if (rmx_room.Collision_set != 0)												// Set di collisioni (CollisionFlipIndex)
			FBX_MA_room_group.int_attributes.push_back(make_pair(string("collisionSet"), (int)rmx_room.Collision_set));
		if (rmx_room.Offset33 != 0)														// Maschera di visibilita' dei Fakes: termina al primo
		{																				// dato successivo della stanza (portali, zone, nodi)
			uint32_t end = 0;
			const uint32_t next[] = {rmx_room.P15_Portal_First, rmx_room.Offset28, rmx_room.NL_OBJ_First, rmx_room.NL_LIGHT_First, rmx_room.NL_CHARLOC_First,
				rmx_room.NL_TRIGGER_First, rmx_room.NL_WAYPOINT_First, rmx_room.NL_WATER_First, rmx_room.NL_EMITTER_First,
				rmx_room.P11_PS2_Room_Obj_First, rmx_room.P12_Audio_Locator_First};
			for (uint32_t n : next)
				if (n >= rmx_room.Offset33 && (end == 0 || n < end))		// Uguale: maschera vuota (es. PRAGUE4A)
					end = n;
			unsigned int nWords = end ? min((end - rmx_room.Offset33) / 4, 256u) : 0;
			if (nWords)
			{
				vector <uint32_t> fake_mask(nWords, 0);
				rmxfile.seekg(rmx_offsets.Offset + rmx_room.Offset33, ios_base::beg);
				rmxfile.read(reinterpret_cast<char*>(fake_mask.data()), nWords * sizeof(uint32_t));
				while (!fake_mask.empty() && fake_mask.back() == 0)						// Zeri di allineamento a 16 bytes
					fake_mask.pop_back();
				string text;															// Parole di 32 bit in esadecimale: bit i della parola
				for (uint32_t w : fake_mask)											// i / 32 = Fake con indice i
				{
					char buf[16];
					snprintf(buf, sizeof(buf), "%s%08X", text.empty() ? "" : " ", w);
					text += buf;
				}
				if (!text.empty())														// Livelli senza Fakes: solo zeri
					FBX_MA_room_group.string_attributes.push_back(make_pair(string("fakeVisibility"), text));
			}
		}
		FBX.Group.push_back(FBX_MA_room_group);			// Inserimento gruppo stanza nel vettore dei gruppi del file FBX
		MA.Transform.push_back(FBX_MA_room_group);		// Inserimento gruppo stanza nel vettore dei gruppi del file MA
		if (Export_Bounding_Box)
		{
			FBX.Geometry.push_back(DrawBox(bbox_name.str(), room_name.str(), "", BBmin, BBmax, 0x2A000000));
			MA.PolyCube.push_back(DrawPolyCube(bbox_name.str(), room_name.str(), Bounding_boxes_layer_name.str(), "", BBmin, BBmax, true, 0x2A000000));
		}


		out << "ROOM " << r << endl;
		out << "Reverb preset: " << Reverb_preset(rmx_room.ReverbID) << endl;
		if (rmx_room.P15_Portal_First != 0)
			out << "Number of portals: " << rmx_room.nPortals << endl;

		if (rmx_room.NL_OBJ_First != 0)
			out << "Object first: " << rmx_room.NL_OBJ_First << endl;
		if (rmx_room.NL_OBJ_Last != 0)
			out << "Object last: " << rmx_room.NL_OBJ_Last << endl;
		if (rmx_room.NL_LIGHT_First != 0)
			out << "Light first: " << rmx_room.NL_LIGHT_First << endl;
		if (rmx_room.NL_LIGHT_Last != 0)
			out << "Light last: " << rmx_room.NL_LIGHT_Last << endl;
		if (rmx_room.NL_CHARLOC_First != 0)
			out << "Actor First: " << rmx_room.NL_CHARLOC_First << endl;
		if (rmx_room.NL_CHARLOC_Last != 0)
			out << "Actor Last: " << rmx_room.NL_CHARLOC_Last << endl;
		if (rmx_room.NL_TRIGGER_First != 0)
			out << "Trigger first: " << rmx_room.NL_TRIGGER_First << endl;
		if (rmx_room.NL_TRIGGER_Last != 0)
			out << "Trigger last: " << rmx_room.NL_TRIGGER_Last << endl;
		if (rmx_room.NL_WAYPOINT_First != 0)
			out << "Waypoint first: " << rmx_room.NL_WAYPOINT_First << endl;
		if (rmx_room.NL_WAYPOINT_Last != 0)
			out << "Waypoint last: " << rmx_room.NL_WAYPOINT_Last << endl;

		if (rmx_room.NL_PORTAL_First != 0)
			out << "Offset 6: " << rmx_room.NL_PORTAL_First << endl;
		if (rmx_room.NL_PORTAL_Last != 0)
			out << "Offset 7: " << rmx_room.NL_PORTAL_Last << endl;
		if (rmx_room.NL_ACTOR_First != 0)
			out << "Offset 10: " << rmx_room.NL_ACTOR_First << endl;
		if (rmx_room.NL_ACTOR_Last != 0)
			out << "Offset 11: " << rmx_room.NL_ACTOR_Last << endl;
		if (rmx_room.NL_CAMERA_First != 0)
			out << "Offset 16: " << rmx_room.NL_CAMERA_First << endl;
		if (rmx_room.NL_CAMERA_Last != 0)
			out << "Offset 17: " << rmx_room.NL_CAMERA_Last << endl;

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
		if (rmx_room.Indoor != 0)
			out << "Indoor: " << rmx_room.Indoor << endl;
		if (rmx_room.Offset32 != 0)
			out << "Offset 32: " << rmx_room.Offset32 << endl;
		if (rmx_room.Offset33 != 0)
			out << "Offset 33: " << rmx_room.Offset33 << endl;

		out << "Flip rooms: " << rmx_room.iFlipRooms[0] << " " << rmx_room.iFlipRooms[1] << " " << rmx_room.iFlipRooms[2] << " " << rmx_room.iFlipRooms[3];
		out << " (" << rmx_room.nFlipRooms << "), is flip room: " << rmx_room.bIsFlipRoom << endl;

		if (rmx_room.NL_LIGHT_First != 0 && Export_Light)						// Lettura ed esportazione luci statiche
		{
			rmxfile.seekg(rmx_offsets.Offset + rmx_room.NL_LIGHT_First, ios_base::beg);
			for (unsigned int l = 0; l < (rmx_room.NL_LIGHT_Last - rmx_room.NL_LIGHT_First) / 144 + 1; l++)
			{
				stringstream temp;
				temp << AOD_IO.levelname << "_ROOM_" << r << "_LIGHT_" << l << RMX_NodeSuffix(rmxfile);
				out << "#LIGHT " << l << endl;
				RMX_Light(rmxfile, temp.str(), room_name.str(), Lights_layer_name.str(), out, FBX, MA);
			}
		}

		if (rmx_room.NL_WATER_First != 0 && Export_Water)						// Lettura ed esportazione acqua
		{
			rmxfile.seekg(rmx_offsets.Offset + rmx_room.NL_WATER_First, ios_base::beg);
			for (unsigned int w = 0; w < (rmx_room.NL_WATER_Last - rmx_room.NL_WATER_First) / 432 + 1; w++)
			{
				stringstream temp;
				temp << AOD_IO.levelname << "_ROOM_" << r << "_WATER_" << w << RMX_NodeSuffix(rmxfile);
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
				temp << AOD_IO.levelname << "_ROOM_" << r << "_PS2OBJ_" << o << RMX_NodeSuffix(rmxfile);
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
				temp << AOD_IO.levelname << "_ROOM_" << r << "_AUDIO_LOCATOR_" << a << RMX_NodeSuffix(rmxfile);
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

		if (rmx_room.NL_WAYPOINT_First != 0 && Export_Waypoints)				// Lettura ed esportazione waypoints (nodi di dimensione variabile: si segue la catena BegNext)
		{
			uint32_t w = rmx_room.NL_WAYPOINT_First;
			for (unsigned int n = 0; w != 0; n++)
			{
				rmxfile.seekg(rmx_offsets.Offset + w, ios_base::beg);
				out << "#WAYPOINT " << n << endl;
				uint32_t next = RMX_Waypoint(rmxfile, room_name.str(), Waypoints_layer.name, out, FBX, MA, waypoints);
				if (w == rmx_room.NL_WAYPOINT_Last || !rmxfile)
					break;
				w = next;
			}
		}

		if (rmx_room.NL_CHARLOC_First != 0 && Export_Characters)				// Lettura ed esportazione posizioni di partenza dei personaggi
		{
			uint32_t c = rmx_room.NL_CHARLOC_First;
			for (unsigned int n = 0; c != 0; n++)
			{
				rmxfile.seekg(rmx_offsets.Offset + c, ios_base::beg);
				out << "#CHARACTER " << n << endl;
				RMX_Charloc(rmxfile, room_name.str(), Characters_layer.name, Character_pawns_layer.name, out, FBX, MA, character_names);
				if (c == rmx_room.NL_CHARLOC_Last || !rmxfile)
					break;
				rmxfile.seekg(rmx_offsets.Offset + c + 4, ios_base::beg);		// BegNext
				rmxfile.read(reinterpret_cast<char*>(&c), sizeof(c));
			}
		}

		if (rmx_room.NL_EMITTER_First != 0 && Export_Emitters)					// Lettura ed esportazione emitter (nodi di dimensione variabile: catena BegNext)
		{
			uint32_t e = rmx_room.NL_EMITTER_First;
			for (unsigned int n = 0; e != 0 && n < 100000; n++)
			{
				rmxfile.seekg(rmx_offsets.Offset + e, ios_base::beg);
				stringstream temp;
				temp << AOD_IO.levelname << "_ROOM_" << r << "_EMITTER_" << n << RMX_NodeSuffix(rmxfile);
				out << "#EMITTER " << n << endl;
				uint32_t next = RMX_Emitter(rmxfile, temp.str(), room_name.str(), Emitters_layer.name, out, FBX, MA);
				nEmitters++;
				if (e == rmx_room.NL_EMITTER_Last || next <= e || !rmxfile)
					break;
				e = next;
			}
		}

		if (rmx_room.NL_TRIGGER_First != 0 && Export_Triggers)					// Lettura ed esportazione trigger (nodi di dimensione variabile: catena BegNext)
		{
			uint32_t t = rmx_room.NL_TRIGGER_First;
			for (unsigned int n = 0; t != 0 && n < 100000; n++)
			{
				rmxfile.seekg(rmx_offsets.Offset + t, ios_base::beg);
				stringstream temp;
				temp << AOD_IO.levelname << "_ROOM_" << r << "_TRIGGER_" << n << RMX_NodeSuffix(rmxfile);
				out << "#TRIGGER " << n << endl;
				uint32_t next = RMX_Trigger(rmxfile, temp.str(), room_name.str(), r, Triggers_layer.name, out, triggers_text, FBX, MA);
				nTriggers++;
				if (t == rmx_room.NL_TRIGGER_Last || next <= t || !rmxfile)
					break;
				t = next;
			}
		}

		out << endl << endl;





	}

	// Coppie di stanze con visibilita' non simmetrica (A vede B ma B non vede A)
	unsigned int asymmetric = 0;
	auto HasMask = [&](uint32_t room) { for (uint32_t w : room_masks[room]) if (w) return true; return false; };	// Stanze senza maschera escluse
	for (uint32_t a = 0; a < room_masks.size(); a++)
		for (uint32_t b = a + 1; b < room_masks.size(); b++)
			if (HasMask(a) && HasMask(b) && ((room_masks[a][b >> 5] >> (b & 31)) & 1) != ((room_masks[b][a >> 5] >> (a & 31)) & 1))
			{
				msg(msg::TGT::FILE, msg::TYP::LOG) << "Room visibility not symmetric: rooms " << a << " and " << b;
				asymmetric++;
			}
	if (asymmetric)
		msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Room visibility: " << asymmetric << " room pairs are not symmetric (see log).";

	// Emitter e trigger: conteggi e file di testo dei trigger nella cartella \NOMELIVELLO\Scripts
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Emitters: " << nEmitters << ", triggers: " << nTriggers;
	if (nTriggers)
	{
		CreateDirectory(AOD_IO.folder_scx_lpwstr, NULL);
		string triggers_file = AOD_IO.folder_scx + AOD_IO.levelname + "_TRIGGERS.txt";
		ofstream txt(triggers_file);
		if (txt.is_open())
		{
			txt << "// Trigger del livello " << AOD_IO.levelname << " (" << filename << "): " << nTriggers << " trigger\n";
			txt << "// Formato: nome {stanza, posizione, dimensioni del volume, rotazione, flags}. Le righe \"<=\" sono i messaggi ricevuti dal\n";
			txt << "// trigger e il gruppo di eventi avviato (file _EVENTS.txt); le altre righe sono le azioni eseguite quando il trigger si\n";
			txt << "// attiva: messaggio -> destinatario [parametro]. Hash senza nome: nomi non noti.\n\n";
			txt << triggers_text.str();
			msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Output filename: " << AOD_IO.levelname << "_TRIGGERS.txt";
		}
		else
			msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Unable to create " << triggers_file;
	}

	// Grafo dei waypoints (dopo la lettura di tutte le stanze: i collegamenti possono unire waypoints di stanze diverse)
	if (Export_Waypoints && !waypoints.empty())
		RMX_Waypoint_Graph(waypoints, Waypoint_graph_layer.name, Waypoint_graph_oneway_layer.name, MA);

	return true;
}