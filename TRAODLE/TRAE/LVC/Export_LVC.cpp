#include "stdafx.h"
#include "Classes.h"
#include "TRAE/LVC/LVC_Functions.h"
#include "FBX/FBX_Functions.h"
#include "MA/MA_Functions.h"


bool Export_LVC (string filename)
{
	FBX_EXPORT FBX;
	MA_EXPORT MA;
	FBX.UpAxis = 'y';
	MA.UpAxis = 'y';

	SetCurrentDirectory(AE_IO.folder_level_lpwstr);				// \NOMELIVELLO
	cout << "\nReading " << filename;
	if (!LVC_Read_Geometry(filename, FBX, MA))					// Lettura geometria livello
		return false;
	//if (!ZONE_Read_Textures(filename, FBX, MA))					// Esportazione textures ed aggiunta nei vettori FBX e MA
		//return false;

	SetCurrentDirectory(AE_IO.folder_geometry_lpwstr);			// \NOMELIVELLO\Geometry
	string outfilename = filename.substr(0, filename.size() - 4);
	cout << "\nWriting " << outfilename << ".FBX";
	FBX_Export(outfilename, FBX);								// Esportazione file FBX
	cout << "\nWriting " << outfilename << ".MA";
	MA_Export(outfilename, MA);									// Esportazione file MA
	return true;
}