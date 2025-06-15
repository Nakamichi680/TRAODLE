#include "stdafx.h"
#include "MA/MA_Functions.h"
#include "FBX/FBX_Functions.h"
#include "TRAOD/CLN/CLN_Functions.h"


bool Export_CLN (string filename)
{
	FBX_EXPORT FBX;
	MA_EXPORT MA;
	return true;																					// DA TOGLIERE
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Reading " << filename;
	SetCurrentDirectory(AOD_IO.folder_level_lpwstr);			// \NOMELIVELLO
	if (!CLN_Read(filename, FBX, MA))							// Lettura collisioni
		return false;
	SetCurrentDirectory(AOD_IO.folder_collisions_lpwstr);		// \NOMELIVELLO\Collisions
	string outfilename = AOD_IO.levelname;
	outfilename.append("_collisions");
	FBX_Export(outfilename, FBX);								// Esportazione file FBX
	MA_Export(outfilename, MA);									// Esportazione file
	return true;
}