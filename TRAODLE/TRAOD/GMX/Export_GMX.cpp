#include "stdafx.h"
#include "Classes.h"
#include "TRAOD/GMX/GMX_Struct.h"
#include "TRAOD/GMX/GMX_Functions.h"
#include "hash_Functions.h"
const char* FILELIST_NAME = "reGMX_filelist";
const uint8_t key[] = { 0x47, 0x4D, 0x58, 0x52, 0x65, 0x47, 0x4D, 0x58 }; // "GMXReGMX"
const size_t keyLen = sizeof(key);



bool Export_GMX ()
{
	GMX_HEADER gmx_header;

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

	vector <GMX_LIST> gmx_list(gmx_header.nFiles);
	vector <string> filenames;
	bool hiddenfilefound = false;

	for (unsigned short i = 0; i < gmx_header.nFiles; i++)
	{
		gmx.seekg(8 + i * 12);
		gmx.read(reinterpret_cast<char*>(&gmx_list[i].Filename), sizeof(gmx_list[i].Filename));		// Legge l'hash del nome del file
		gmx.read(reinterpret_cast<char*>(&gmx_list[i].Offset), sizeof(gmx_list[i].Offset));			// Legge l'offset del file
		gmx.read(reinterpret_cast<char*>(&gmx_list[i].Size), sizeof(gmx_list[i].Size));				// Legge la dimensione del file

		if (gmx_list[i].Filename == GetHashValue(FILELIST_NAME))
		{
			hiddenfilefound = true;
			gmx.seekg(gmx_list[i].Offset + 2048);													// Posiziona l'indice di lettura all'inizio del file da estrarre
			char* buffer = new char[gmx_list[i].Size];												// Buffer di lettura
			gmx.read(buffer, gmx_list[i].Size);														// Legge il gmx e lo mette nel buffer
			
			string decrypted(gmx_list[i].Size, '\0');

			for (unsigned int j = 0; j < gmx_list[i].Size; j++)
				decrypted[j] = static_cast<char>(buffer[j] ^ key[j % keyLen]);

			// Parsing del contenuto in righe
			stringstream ss(decrypted);
			string line;

			while (getline(ss, line))
			{
				// Rimuovi eventuali caratteri '\r' (per compatibilità Windows/Unix)
				if (!line.empty() && line.back() == '\r')
					line.pop_back();

				// Aggiungi solo righe non vuote
				if (!line.empty())
					filenames.push_back(line);
			}
			delete[] buffer;																		// Elimina il buffer
		}
	}

	if (hiddenfilefound)
		msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << gmx_header.nFiles - 1 << " files found.";
	else
		msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << gmx_header.nFiles << " files found.";

	CreateDirectory(AOD_IO.folder_level_lpwstr, NULL);	// Crea la cartella \NOMELIVELLO
	SetCurrentDirectory(AOD_IO.folder_level_lpwstr);	// Imposta la cartella \NOMELIVELLO come attiva

	for (unsigned short i = 0; i < gmx_header.nFiles; i++)
	{
		// Salta l'esportazione del file nascosto
		if (gmx_list[i].Filename == GetHashValue(FILELIST_NAME))
			continue;

		// Esporta gli altri file
		gmx.seekg(gmx_list[i].Offset + 2048);														// Posiziona l'indice di lettura all'inizio del file da estrarre
		ofstream out;
		out.open(AoD_GetFilenameFromHash(gmx_list[i].Filename, filenames), ofstream::binary);
		char* buffer = new char[gmx_list[i].Size];													// Buffer di lettura
		gmx.read(buffer, gmx_list[i].Size);															// Legge il gmx e lo mette nel buffer
		out.write(buffer, gmx_list[i].Size);														// Copia il buffer nel file di destinazione
		delete[] buffer;																			// Elimina il buffer
		out.close();
	}

	gmx.close();
	return true;
}