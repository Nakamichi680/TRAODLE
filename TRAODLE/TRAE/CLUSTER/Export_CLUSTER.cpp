#include "stdafx.h"
#include "Classes.h"
#include "TRAE/CLUSTER/CLUSTER_Struct.h"
#include "TRAE/CLUSTER/CLUSTER_Functions.h"


bool Export_CLUSTER ()
{
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Reading " << AE_IO.file_cluster;

	CLUSTER_HEADER header;
	CLUSTER_FILE_LIST list;

	SetCurrentDirectory(AE_IO.folder_cluster_lpwstr);		// Imposta la cartella in cui si trova il file cluster
	ifstream cluster(AE_IO.file_cluster, std::ios::binary);
	
	if (!cluster.is_open())									// Se il file non viene aperto la funzione termina
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::FATAL) << AE_IO.file_cluster << " not found.";
		return false;
	}

	cluster.read(reinterpret_cast<char*>(&header.Magic1), sizeof(header.Magic1));
	cluster.read(reinterpret_cast<char*>(&header.TargetPlatform), sizeof(header.TargetPlatform));
	cluster.read(reinterpret_cast<char*>(&header.Magic2), sizeof(header.Magic2));

	if (header.Magic1 != 1 || header.Magic2 != 5590083)		// Se il file cluster non è valido la funzione termina
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::FATAL) << AE_IO.file_cluster << " is not a valid TRAE Cluster file.";
		return false;
	}

	switch (header.TargetPlatform)
	{
	case 0x1000:
		msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Cluster file target platform: PSP";
		break;
	case 0x200:
		msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Cluster file target platform: PS2";
		break;
	case 0x100:
		msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Cluster file target platform: PC";
		break;
	default:
		msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Cluster file target platform: Unknown";
	}

	cluster.read(reinterpret_cast<char*>(&header.HeaderSize), sizeof(header.HeaderSize));
	cluster.read(reinterpret_cast<char*>(&header.nFiles), sizeof(header.nFiles));
	cluster.seekg(16, ios_base::cur);						// Salta FilenameHash e Description

	if (header.nFiles == 0)									// Se il file cluster non contiene alcun file
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::FATAL) << AE_IO.file_cluster << " does not contain any file.";
		return false;
	}
	
	CreateDirectory(AE_IO.folder_level_lpwstr, NULL);											// Crea la cartella \NOMELIVELLO
	SetCurrentDirectory(AE_IO.folder_level_lpwstr);												// Imposta la cartella \NOMELIVELLO come attiva

	for (unsigned int f = 0; f < header.nFiles; f++)
	{
		cluster.seekg(32 + f * 16);
		cluster.read(reinterpret_cast<char*>(&list.FileListEnd), sizeof(list.FileListEnd));
		cluster.read(reinterpret_cast<char*>(&list.Filesize), sizeof(list.Filesize));
		cluster.read(reinterpret_cast<char*>(&list.Offset), sizeof(list.Offset));
		cluster.read(reinterpret_cast<char*>(&list.Hash), sizeof(list.Hash));
		cluster.seekg(list.Offset);																// Posiziona l'indice di lettura all'inizio del file da estrarre
		char* buffer = new char[list.Filesize];													// Buffer di lettura
		cluster.read(buffer, list.Filesize);													// Legge il cluster e lo mette nel buffer
		ofstream out;
		out.open(AE_GetFilenameFromHash(list.Hash, buffer), ofstream::binary);
		out.write(buffer, list.Filesize);														// Copia il buffer nel file di destinazione
		delete[] buffer;																		// Elimina il buffer
		out.close();
	}

	cluster.close();
	cout << endl << header.nFiles << " files succesfully extracted.";
	return true;
}