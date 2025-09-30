#include "stdafx.h"
#include "Classes.h"
#include "TRAOD Remastered/MESH/MESH_Functions.h"
#include "FBX/FBX_Functions.h"
#include "MA/MA_Functions.h"


bool Export_MESH (string filename)
{
	FBX_EXPORT FBX;
	MA_EXPORT MA;

	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Reading " << filename;
	SetCurrentDirectory(AODRemastered_IO.folder_msh_lpwstr);	// Cartella contenente il file MSH
	if (!MESH_Read_Object(filename, FBX, MA))					// Lettura geometria oggetto
		return false;

	SetCurrentDirectory(AODRemastered_IO.folder_object_lpwstr);	// \NOMEFILEMSH
	FBX_Export(AODRemastered_IO.objectname, FBX);				// Esportazione file FBX
	MA_Export(AODRemastered_IO.objectname, MA);					// Esportazione file MA
	return true;
}