#include "stdafx.h"
#include "Classes.h"
#include "TRAOD/ZONE/ZONE_Functions.h"
#include "FBX/FBX_Functions.h"
#include "MA/MA_Functions.h"


bool Export_ZONE (string filename)
{
	FBX_EXPORT FBX;
	MA_EXPORT MA;
	vector <RoomInfo> RMX_Rooms;

	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Reading " << filename;
	SetCurrentDirectory(AOD_IO.folder_level_lpwstr);			// \NOMELIVELLO
	if (!GetRoomInfo(RMX_Rooms))								// Lettura hash e coordinate stanze dal file RMX
		return false;
	if (!ZONE_Read_Rooms(filename, RMX_Rooms, FBX, MA))			// Lettura stanze
		return false;
	if (!ZONE_Read_Fakes(filename, FBX, MA))					// Lettura Fakes
		return false;
	if (!ZONE_Read_Materials(filename, FBX, MA))				// Esportazione materiali ed aggiunta nei vettori FBX e MA
		return false;
	if (!ZONE_Read_Textures(filename, RMX_Rooms, FBX, MA))		// Esportazione textures ed aggiunta nei vettori FBX e MA
		return false;

	SetCurrentDirectory(AOD_IO.folder_zones_lpwstr);			// \NOMELIVELLO\Zones
	string zone = filename;
	zone.erase(0, (zone.find(".Z") + 1));						// Z&&
	string outfilename = AOD_IO.levelname;
	outfilename.append("_");
	outfilename.append(zone);									// NOMELIVELLO_Z&&
	FBX_Export(outfilename, FBX);								// Esportazione file FBX
	MA_Export(outfilename, MA);									// Esportazione file MA
	return true;
}