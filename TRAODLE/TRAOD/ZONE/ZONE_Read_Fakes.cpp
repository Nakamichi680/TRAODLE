#include "stdafx.h"
#include "TRAOD/ZONE/ZONE_Struct.h"
#include "TRAOD/ZONE/ZONE_Functions.h"


/*------------------------------------------------------------------------------------------------------------------
Esporta i Fakes del file ZONE (lista ZONE_FAKES_ELEMENT dopo EOF_PTR): ogni Fake istanzia l'oggetto di uno slot
(Fake_List_Index) con la sua posizione e scala, con la gerarchia dello slot (vedi ZONE_ExportSlot).
Layer: NOMELIVELLO_Z&&_Fakes.
------------------------------------------------------------------------------------------------------------------*/
bool ZONE_Read_Fakes (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA)
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

	// Primo Fake (0 se non ci sono Fakes)
	uint32_t fake = 0;
	zonefile.seekg(data.header.EOF_PTR);
	zonefile.read(reinterpret_cast<char*>(&fake), sizeof(fake));
	if (!zonefile || fake == 0)
		return true;

	Layer layer;
	layer.name = AOD_IO.levelname + "_" + data.zonename + "_Fakes";
	layer.Label_ARGB = 0xFF00D822;
	MA.Layer.push_back(layer);

	bool ok = true;
	unsigned int exported = 0;
	for (unsigned int guard = 0; fake != 0 && guard < 100000; guard++)
	{
		ZONE_FAKES_ELEMENT element;
		zonefile.seekg(fake);
		zonefile.read(reinterpret_cast<char*>(&element.BegPrev), sizeof(element.BegPrev));
		zonefile.read(reinterpret_cast<char*>(&element.BegNext), sizeof(element.BegNext));
		zonefile.seekg(fake + 24);
		zonefile.read(reinterpret_cast<char*>(&element.Unknown1), sizeof(element.Unknown1));
		zonefile.seekg(fake + 32);
		zonefile.read(reinterpret_cast<char*>(&element.Xpos), sizeof(element.Xpos));
		zonefile.read(reinterpret_cast<char*>(&element.Ypos), sizeof(element.Ypos));
		zonefile.read(reinterpret_cast<char*>(&element.Zpos), sizeof(element.Zpos));
		zonefile.seekg(fake + 80);
		zonefile.read(reinterpret_cast<char*>(&element.Xscal), sizeof(element.Xscal));
		zonefile.read(reinterpret_cast<char*>(&element.Yscal), sizeof(element.Yscal));
		zonefile.read(reinterpret_cast<char*>(&element.Zscal), sizeof(element.Zscal));
		zonefile.seekg(fake + 100);
		zonefile.read(reinterpret_cast<char*>(&element.Unknown4), sizeof(element.Unknown4));
		zonefile.read(reinterpret_cast<char*>(&element.Unknown5), sizeof(element.Unknown5));
		zonefile.read(reinterpret_cast<char*>(&element.Static_FollowCam), sizeof(element.Static_FollowCam));
		zonefile.seekg(fake + 126);
		zonefile.read(reinterpret_cast<char*>(&element.Fake_List_Index), sizeof(element.Fake_List_Index));
		if (!zonefile)
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Error reading the Fakes of " << filename;
			return false;
		}
		if (element.Static_FollowCam == 8)
			msg(msg::TGT::FILE, msg::TYP::LOG) << "Fake " << hex << element.Unknown4 << dec << " is camera oriented.";

		if (element.Fake_List_Index < data.slots.size() && !data.slots[element.Fake_List_Index].empty())
		{
			stringstream name;
			name << AOD_IO.levelname << "_" << data.zonename << "_FAKE_" << element.Fake_List_Index;
			ZONE_ObjectExport object;
			Transform root;
			root.name = name.str();
			root.layer = layer.name;
			root.translate_flag = root.scale_flag = true;
			root.tX = element.Xpos;		root.tY = element.Ypos;		root.tZ = element.Zpos;
			root.sX = element.Xscal;	root.sY = element.Yscal;	root.sZ = element.Zscal;
			root.int_attributes.push_back(make_pair(string("objectSlot"), (int)element.Fake_List_Index));
			root.int_attributes.push_back(make_pair(string("fakeIndex"), (int)element.Unknown1));	// UserPad: bit della maschera fakeVisibility
			root.int_attributes.push_back(make_pair(string("followCamera"), element.Static_FollowCam & 8 ? 1 : 0));
			object.groups.push_back(root);
			ok &= ZONE_ExportSlot(zonefile, data, element.Fake_List_Index, root.name, layer.name, object);
			object.PushTo(FBX, MA);
			exported++;
		}
		else
			msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << filename << ": Fake with empty object slot " << element.Fake_List_Index << ". Skipped.";
		if (element.BegNext == fake)
			break;
		fake = element.BegNext;
	}
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Fakes: " << exported;
	return ok;
}
