#include "stdafx.h"
#include "MA/MA_Functions.h"
#include "TRAOD/CAM/CAM_Functions.h"
#include "FBX/FBX_Functions.h"

bool Export_CAM (string filename)
{
	FBX_EXPORT FBX;
	MA_EXPORT MA;
	
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Reading " << filename;
	SetCurrentDirectory(AOD_IO.folder_level_lpwstr);			// \NOMELIVELLO
	if (!CAM_Read(filename, FBX, MA))							// Lettura telecamera
		return false;
	SetCurrentDirectory(AOD_IO.folder_cameras_lpwstr);			// \NOMELIVELLO\Cameras
	string outfilename = filename.substr(0, filename.find(".CAM"));
	FBX_Export(outfilename, FBX);								// Esportazione file FBX
	MA_Export(outfilename, MA);									// Esportazione file
	return true;
}