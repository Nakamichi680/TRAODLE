#include "stdafx.h"
#include "Classes.h"
#include "TRAE/MDC/MDC_Functions.h"
#include "FBX/FBX_Functions.h"
#include "MA/MA_Functions.h"


bool Export_MDC (string filename)
{
	FBX_EXPORT FBX;
	MA_EXPORT MA;
	FBX.UpAxis = 'y';
	MA.UpAxis = 'y';
	MA.NearClipPlane = 0.01f;

	SetCurrentDirectory(AE_IO.folder_level_lpwstr);				// \NOMELIVELLO
	cout << "\nReading " << filename;
	if (!MDC_Read_Geometry(filename, FBX, MA))					// Lettura geometria modelli oggetti
		return false;

	SetCurrentDirectory(AE_IO.folder_models_lpwstr);			// \NOMELIVELLO\Models
	string outfilename = filename.substr(0, filename.size() - 4);
	cout << "\nWriting " << outfilename << ".FBX";
	FBX_Export(outfilename, FBX);								// Esportazione file FBX
	cout << "\nWriting " << outfilename << ".MA";
	MA_Export(outfilename, MA);									// Esportazione file MA
	return true;
}