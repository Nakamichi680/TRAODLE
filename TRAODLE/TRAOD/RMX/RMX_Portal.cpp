#include "stdafx.h"
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "TRAOD/RMX/RMX_Struct.h"
#include "Misc_Functions.h"


void RMX_Portal (ifstream &rmxfile, string name, string room_name, string layer, ofstream &out, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	RMX_PORTAL rmx_portal;
	XYZ v0, v1, v2, v3;

	// Lettura dati portale dal file RMX
	rmxfile.read(reinterpret_cast<char*>(&rmx_portal.Unknown1), sizeof(rmx_portal.Unknown1));
	rmxfile.read(reinterpret_cast<char*>(&rmx_portal.Unknown2), sizeof(rmx_portal.Unknown2));
	rmxfile.read(reinterpret_cast<char*>(&rmx_portal.DestRoom), sizeof(rmx_portal.DestRoom));					// ID della stanza a cui il portale conduce
	rmxfile.read(reinterpret_cast<char*>(&rmx_portal.null1), sizeof(rmx_portal.null1));
	rmxfile.seekg(28, ios_base::cur);
	rmxfile.read(reinterpret_cast<char*>(&rmx_portal.Unknown4), sizeof(rmx_portal.Unknown4));
	rmxfile.read(reinterpret_cast<char*>(&v0.x), sizeof(rmx_portal.v0_X));
	rmxfile.read(reinterpret_cast<char*>(&v0.y), sizeof(rmx_portal.v0_Y));
	rmxfile.read(reinterpret_cast<char*>(&v0.z), sizeof(rmx_portal.v0_Z));
	rmxfile.read(reinterpret_cast<char*>(&rmx_portal.Unknown5), sizeof(rmx_portal.Unknown5));
	rmxfile.read(reinterpret_cast<char*>(&v1.x), sizeof(rmx_portal.v1_X));
	rmxfile.read(reinterpret_cast<char*>(&v1.y), sizeof(rmx_portal.v1_Y));
	rmxfile.read(reinterpret_cast<char*>(&v1.z), sizeof(rmx_portal.v1_Z));
	rmxfile.read(reinterpret_cast<char*>(&rmx_portal.Unknown6), sizeof(rmx_portal.Unknown6));
	rmxfile.read(reinterpret_cast<char*>(&v2.x), sizeof(rmx_portal.v2_X));
	rmxfile.read(reinterpret_cast<char*>(&v2.y), sizeof(rmx_portal.v2_Y));
	rmxfile.read(reinterpret_cast<char*>(&v2.z), sizeof(rmx_portal.v2_Z));
	rmxfile.read(reinterpret_cast<char*>(&rmx_portal.Unknown7), sizeof(rmx_portal.Unknown7));
	rmxfile.read(reinterpret_cast<char*>(&v3.x), sizeof(rmx_portal.v3_X));
	rmxfile.read(reinterpret_cast<char*>(&v3.y), sizeof(rmx_portal.v3_Y));
	rmxfile.read(reinterpret_cast<char*>(&v3.z), sizeof(rmx_portal.v3_Z));
	rmxfile.read(reinterpret_cast<char*>(&rmx_portal.Unknown8), sizeof(rmx_portal.Unknown8));

	// Esportazione informazioni portale nel file TXT
	out << "	Unknown 1: " << rmx_portal.Unknown1 << endl;
	out << "	Unknown 2: " << rmx_portal.Unknown2 << endl;
	out << "	Destination room: " << rmx_portal.DestRoom << endl;
	out << "	Unknown 4: " << rmx_portal.Unknown4 << endl;
	out << "	Unknown 5: " << rmx_portal.Unknown5 << endl;
	out << "	Unknown 6: " << rmx_portal.Unknown6 << endl;
	out << "	Unknown 7: " << rmx_portal.Unknown7 << endl;
	out << "	Unknown 8: " << rmx_portal.Unknown8 << endl;
	out << left << setw(30) << "	Rect v0 [X/Y/Z]:" << right << setw(13) << v0.x << setw(13) << v0.y << setw(13) << v0.z << endl;
	out << left << setw(30) << "	Rect v1 [X/Y/Z]:" << right << setw(13) << v1.x << setw(13) << v1.y << setw(13) << v1.z << endl;
	out << left << setw(30) << "	Rect v2 [X/Y/Z]:" << right << setw(13) << v2.x << setw(13) << v2.y << setw(13) << v2.z << endl;
	out << left << setw(30) << "	Rect v3 [X/Y/Z]:" << right << setw(13) << v3.x << setw(13) << v3.y << setw(13) << v3.z << endl;

	// Creazione mesh rettangolo del portale
	stringstream temp;
	temp << name << "_TO_ROOM_" << rmx_portal.DestRoom;
	Mesh output = DrawRectangle(temp.str(), room_name, layer, v0, v1, v2, v3, 0x3FD400C7);
	FBX.Geometry.push_back(output);						// Inserimento portale nel vettore mesh del file FBX
	MA.Mesh.push_back(output);							// Inserimento portale nel vettore mesh del file MA	
}