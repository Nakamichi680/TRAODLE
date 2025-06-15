#include "stdafx.h"
#include "Classes.h"
#include "TRAOD/GMX/GMX_Struct.h"
#include "TRAOD/GMX/GMX_Functions.h"


bool Export_GMX ()
{
	GMX_HEADER gmx_header;
	GMX_LIST gmx_list;

	SetCurrentDirectory(AOD_IO.folder_clzgmx_lpwstr);	// Imposta la cartella in cui si trova il file GMX
	ifstream gmx(AOD_IO.file_clzgmx, std::ios::binary);

	if (!gmx.is_open())							// Se il file non viene aperto la funzione termina
	{
		msg(msg::TGT::CONS, msg::TYP::FATAL) << AOD_IO.file_clzgmx << " not found.";
		return false;
	}

	gmx.read(reinterpret_cast<char*>(&gmx_header.GMX_Version), sizeof(gmx_header.GMX_Version));		// Legge l'ID del file
	
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "GMX file version: " << gmx_header.GMX_Version;


	gmx.read(reinterpret_cast<char*>(&gmx_header.nFiles), sizeof(gmx_header.nFiles));

	if (gmx_header.nFiles == 0)					// Se il file GMX non contiene alcun file
	{
		msg(msg::TGT::CONS, msg::TYP::FATAL) << AOD_IO.file_clzgmx << " does not contain any file.";
		return false;
	}

	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << gmx_header.nFiles << " files found.";

	CreateDirectory(AOD_IO.folder_level_lpwstr, NULL);	// Crea la cartella \NOMELIVELLO
	SetCurrentDirectory(AOD_IO.folder_level_lpwstr);	// Imposta la cartella \NOMELIVELLO come attiva

	for (unsigned short i = 0; i < gmx_header.nFiles; i++)
	{
		gmx.seekg(8 + i * 12);
		gmx.read(reinterpret_cast<char*>(&gmx_list.Filename), sizeof(gmx_list.Filename));			// Legge l'hash del nome del file
		gmx.read(reinterpret_cast<char*>(&gmx_list.Offset), sizeof(gmx_list.Offset));				// Legge l'offset del file
		gmx.read(reinterpret_cast<char*>(&gmx_list.Size), sizeof(gmx_list.Size));					// Legge la dimensione del file
		gmx.seekg(gmx_list.Offset + 2048);															// Posiziona l'indice di lettura all'inizio del file da estrarre
		ofstream out;
		out.open(AoD_GetFilenameFromHash(gmx_list.Filename), ofstream::binary);
		char* buffer = new char[gmx_list.Size];														// Buffer di lettura
		gmx.read(buffer, gmx_list.Size);															// Legge il gmx e lo mette nel buffer
		out.write(buffer, gmx_list.Size);															// Copia il buffer nel file di destinazione
		delete[] buffer;																			// Elimina il buffer
		out.close();
	}

	gmx.close();
	return true;
}