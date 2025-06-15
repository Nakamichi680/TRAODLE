#include "stdafx.h"
#include "MA/MA_Functions.h"
#include "TRAOD/RMX/RMX_Functions.h"
#include "FBX/FBX_Functions.h"

bool Export_RMX (string filename)
{
	FBX_EXPORT FBX;
	MA_EXPORT MA;
	
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Reading " << filename;
	SetCurrentDirectory(AOD_IO.folder_level_lpwstr);			// \NOMELIVELLO
	if (!RMX_Read(filename, FBX, MA))							// Lettura file RMX
		return false;
	SetCurrentDirectory(AOD_IO.folder_rooms_lpwstr);			// \NOMELIVELLO\Rooms

	msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << "Move redundant lights to separate layer: ENABLED";
	msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << Isolate_duplicated_lights(MA) << " lights moved to \"" << AOD_IO.levelname << "_duplicated_lights\" layer.";
	msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << "Move redundant water nodes to separate layer: ENABLED";
	msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << Isolate_duplicated_water(MA) << " water nodes moved to \"" << AOD_IO.levelname << "_duplicated_water\" layer.";

	FBX_Export(AOD_IO.levelname, FBX);							// Esportazione file FBX
	MA_Export(AOD_IO.levelname, MA);							// Esportazione file MA
	return true;
}