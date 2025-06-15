#include "stdafx.h"
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "TRAOD/RMX/RMX_Struct.h"


void RMX_PS2_Room_Obj (ifstream &rmxfile, string name, string room_name, string layer, ofstream &out, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	RMX_PS2_ROOM_OBJ rmx_ps2_room_obj;

	// Lettura dati oggetti PS2 dal file RMX
	rmxfile.seekg(32, ios_base::cur);
	rmxfile.read(reinterpret_cast<char*>(&rmx_ps2_room_obj.Xpos), sizeof(rmx_ps2_room_obj.Xpos));
	rmxfile.read(reinterpret_cast<char*>(&rmx_ps2_room_obj.Ypos), sizeof(rmx_ps2_room_obj.Ypos));
	rmxfile.read(reinterpret_cast<char*>(&rmx_ps2_room_obj.Zpos), sizeof(rmx_ps2_room_obj.Zpos));
	rmxfile.seekg(4, ios_base::cur);
	rmxfile.read(reinterpret_cast<char*>(&rmx_ps2_room_obj.Xrot), sizeof(rmx_ps2_room_obj.Xrot));
	rmxfile.read(reinterpret_cast<char*>(&rmx_ps2_room_obj.Yrot), sizeof(rmx_ps2_room_obj.Yrot));
	rmxfile.read(reinterpret_cast<char*>(&rmx_ps2_room_obj.Zrot), sizeof(rmx_ps2_room_obj.Zrot));
	rmxfile.seekg(20, ios_base::cur);
	rmxfile.read(reinterpret_cast<char*>(&rmx_ps2_room_obj.Xscal), sizeof(rmx_ps2_room_obj.Xscal));
	rmxfile.read(reinterpret_cast<char*>(&rmx_ps2_room_obj.Yscal), sizeof(rmx_ps2_room_obj.Yscal));
	rmxfile.read(reinterpret_cast<char*>(&rmx_ps2_room_obj.Zscal), sizeof(rmx_ps2_room_obj.Zscal));
	rmxfile.seekg(8, ios_base::cur);
	rmxfile.read(reinterpret_cast<char*>(&rmx_ps2_room_obj.Unknown3), sizeof(rmx_ps2_room_obj.Unknown3));
	rmxfile.read(reinterpret_cast<char*>(&rmx_ps2_room_obj.Unknown4), sizeof(rmx_ps2_room_obj.Unknown4));
	rmxfile.read(reinterpret_cast<char*>(&rmx_ps2_room_obj.Static_flag), sizeof(rmx_ps2_room_obj.Static_flag));
	rmxfile.read(reinterpret_cast<char*>(&rmx_ps2_room_obj.BB_Xmin), sizeof(rmx_ps2_room_obj.BB_Xmin));
	rmxfile.read(reinterpret_cast<char*>(&rmx_ps2_room_obj.BB_Ymin), sizeof(rmx_ps2_room_obj.BB_Ymin));
	rmxfile.read(reinterpret_cast<char*>(&rmx_ps2_room_obj.BB_Zmin), sizeof(rmx_ps2_room_obj.BB_Zmin));
	rmxfile.seekg(4, ios_base::cur);
	rmxfile.read(reinterpret_cast<char*>(&rmx_ps2_room_obj.BB_Xmax), sizeof(rmx_ps2_room_obj.BB_Xmax));
	rmxfile.read(reinterpret_cast<char*>(&rmx_ps2_room_obj.BB_Ymax), sizeof(rmx_ps2_room_obj.BB_Ymax));
	rmxfile.read(reinterpret_cast<char*>(&rmx_ps2_room_obj.BB_Zmax), sizeof(rmx_ps2_room_obj.BB_Zmax));
	rmxfile.seekg(4, ios_base::cur);

	// Esportazione informazioni oggetto nel file TXT
	out << "	Hashed name: " << hex << rmx_ps2_room_obj.Unknown3 << dec << endl;
	if (rmx_ps2_room_obj.Static_flag == 4)
		out << "	Node type: STATIC\n";
	else
		out << "	Node type: DYNAMIC\n";
	out << left << setw(30) << "	Position [X/Y/Z]:" << right << setw(13) << rmx_ps2_room_obj.Xpos << setw(13) << rmx_ps2_room_obj.Ypos << setw(13) << rmx_ps2_room_obj.Zpos << endl;
	out << left << setw(30) << "	Rotation [X/Y/Z]:" << right << setw(13) << rmx_ps2_room_obj.Xrot << setw(13) << rmx_ps2_room_obj.Yrot << setw(13) << rmx_ps2_room_obj.Zrot << endl;
	out << left << setw(30) << "	Scale [X/Y/Z]:" << right << setw(13) << rmx_ps2_room_obj.Xscal << setw(13) << rmx_ps2_room_obj.Yscal << setw(13) << rmx_ps2_room_obj.Zscal << endl;
	out << left << setw(30) << "	Bounding box min [X/Y/Z]:" << right << setw(13) << rmx_ps2_room_obj.BB_Xmin << setw(13) << rmx_ps2_room_obj.BB_Ymin << setw(13) << rmx_ps2_room_obj.BB_Zmin << endl;
	out << left << setw(30) << "	Bounding box max [X/Y/Z]:" << right << setw(13) << rmx_ps2_room_obj.BB_Xmax << setw(13) << rmx_ps2_room_obj.BB_Ymax << setw(13) << rmx_ps2_room_obj.BB_Zmax << endl << endl;
	
	// Esportazione oggetto nei file FBX e MA
	Locator FBX_MA_PS2_Room_Obj;
	FBX_MA_PS2_Room_Obj.name = name;
	FBX_MA_PS2_Room_Obj.parent = room_name;
	FBX_MA_PS2_Room_Obj.layer = layer;									// Nome layer di appartenenza (solo file MA)
	FBX_MA_PS2_Room_Obj.FBX_parent = hashID(room_name, "Group");
	FBX_MA_PS2_Room_Obj.translate_flag = true;
	FBX_MA_PS2_Room_Obj.rotate_flag = true;
	FBX_MA_PS2_Room_Obj.scale_flag = true;
	FBX_MA_PS2_Room_Obj.tX = rmx_ps2_room_obj.Xpos;
	FBX_MA_PS2_Room_Obj.tY = rmx_ps2_room_obj.Ypos;
	FBX_MA_PS2_Room_Obj.tZ = rmx_ps2_room_obj.Zpos;
	FBX_MA_PS2_Room_Obj.rX = rmx_ps2_room_obj.Xrot;
	FBX_MA_PS2_Room_Obj.rY = rmx_ps2_room_obj.Yrot;
	FBX_MA_PS2_Room_Obj.rZ = rmx_ps2_room_obj.Zrot;
	FBX_MA_PS2_Room_Obj.sX = rmx_ps2_room_obj.Xscal;
	FBX_MA_PS2_Room_Obj.sY = rmx_ps2_room_obj.Yscal;
	FBX_MA_PS2_Room_Obj.sZ = rmx_ps2_room_obj.Zscal;
	FBX.Locator.push_back(FBX_MA_PS2_Room_Obj);			// Inserimento oggetto PS2 nel vettore dei locator del file FBX
	MA.Locator.push_back(FBX_MA_PS2_Room_Obj);			// Inserimento oggetto PS2 nel vettore dei locator del file MA
}