#include "stdafx.h"
#include "TRAOD/RMX/RMX_Struct.h"
#include "Classes.h"


bool GetRoomInfo (vector <RoomInfo> &RMX_Rooms)
{
	RMX_HEADER rmx_header;
	RMX_OFFSETS rmx_offsets;
	RMX_ROOM rmx_room;
	string RMX_Filename = AOD_IO.levelname;
	RMX_Filename.append(".RMX");

	// LETTURA FILE RMX
	ifstream rmxfile(RMX_Filename, std::ios::binary);
	if (!rmxfile.is_open())
	{
		cout << "\nFATAL ERROR: " << RMX_Filename << " not found.";
		return false;
	}

	// Lettura header
	rmxfile.read(reinterpret_cast<char*>(&rmx_header.Version), sizeof(rmx_header.Version));						// Lettura versione file RMX
	rmxfile.seekg(12, ios_base::cur);
	rmxfile.read(reinterpret_cast<char*>(&rmx_header.nRooms), sizeof(rmx_header.nRooms));						// Lettura numero di stanze

	RMX_Rooms.resize(rmx_header.nRooms);

	if (rmx_header.Version != 3.6f)				// Se la versione di file RMX non è la 3.6
	{
		cout << "Error - RMX version mismatch\n";
		return false;
	}

	for (unsigned int r = 0; r < rmx_header.nRooms; r++)
	{
		rmxfile.seekg(20 + 4 * r, ios_base::beg);
		rmxfile.read(reinterpret_cast<char*>(&rmx_offsets.Offset), sizeof(rmx_offsets.Offset));
		rmxfile.seekg(rmx_offsets.Offset + 112);
		rmxfile.read(reinterpret_cast<char*>(&RMX_Rooms[r].tX), sizeof(rmx_room.Room_Xt));
		rmxfile.read(reinterpret_cast<char*>(&RMX_Rooms[r].tY), sizeof(rmx_room.Room_Yt));
		rmxfile.read(reinterpret_cast<char*>(&RMX_Rooms[r].tZ), sizeof(rmx_room.Room_Zt));
		rmxfile.seekg(44, ios_base::cur);
		rmxfile.read(reinterpret_cast<char*>(&RMX_Rooms[r].RoomHash), sizeof(rmx_room.Room_ID2));
	}
	return true;
}