#include "stdafx.h"
#include "Classes.h"
#include "TRAE/TRAE_Functions.h"
#include "FBX/FBX_Functions.h"
#include "MA/MA_Functions.h"


bool Export_TRAE (string filename)
{
	FBX_EXPORT FBX;
	MA_EXPORT MA;
	FBX.UpAxis = 'y';



	//SetCurrentDirectory(IO.folder_exe_lpwstr);					// Cartella del file exe del programma
	cout << "\nReading TRAE file " << filename << endl;
	if (!TRAE_GEOMRAW_Read(filename, FBX, MA))						// Lettura file GEOM
		return false;

	//TRAE_TEXSETPC_Read(filename, FBX, MA);


	string outfilename = filename;
	outfilename.append("_output");
	cout << "\n\nExporting " << outfilename << ".FBX";
	FBX_Export(outfilename, FBX);								// Esportazione file FBX
	cout << "\nExporting " << outfilename << ".MA";
	MA_Export(outfilename, MA);									// Esportazione file MA
	
	cout << "\nOperation completed." << endl;
	system("pause");
	return true;
}