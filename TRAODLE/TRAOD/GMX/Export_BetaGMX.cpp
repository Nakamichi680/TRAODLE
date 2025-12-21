#include "stdafx.h"
#include "Classes.h"


struct BETAGMX_LIST				// SIZE: 2048 bytes
{
	char Filename[24];			// Filename + extension
	uint32_t Offset;			// Offset from begin of the file + 2048 bytes
	uint32_t Size;				// Size of the file in bytes
};


bool Export_BetaGMX ()
{
	SetCurrentDirectory(AOD_IO.folder_clzgmx_lpwstr);	// Imposta la cartella in cui si trova il file GMX
	ifstream betagmx(AOD_IO.file_clzgmx, std::ios::binary);

	if (!betagmx.is_open())							// Se il file non viene aperto la funzione termina
	{
		msg(msg::TGT::CONS, msg::TYP::FATAL) << AOD_IO.file_clzgmx << " not found.";
		return false;
	}

	vector <BETAGMX_LIST> betagmx_list;

	for (unsigned short i = 0; i < 64; i++)
	{
		BETAGMX_LIST temp;
		betagmx.read(temp.Filename, sizeof(temp.Filename));
		betagmx.read(reinterpret_cast<char*>(&temp.Offset), sizeof(temp.Offset));			// Legge l'offset del file
		betagmx.read(reinterpret_cast<char*>(&temp.Size), sizeof(temp.Size));				// Legge la dimensione del file
		if (temp.Size > 0)
			betagmx_list.push_back(temp);
	}

	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << betagmx_list.size() << " files found.";

	CreateDirectory(AOD_IO.folder_level_lpwstr, NULL);	// Crea la cartella \NOMELIVELLO
	SetCurrentDirectory(AOD_IO.folder_level_lpwstr);	// Imposta la cartella \NOMELIVELLO come attiva

	for (unsigned short i = 0; i < betagmx_list.size(); i++)
	{
		string name(betagmx_list[i].Filename);

		size_t pos = name.find_last_of('.');
		string extension = (pos != std::string::npos) ? name.substr(pos + 1) : "";
		if (extension == "CAL")				name = AOD_IO.AddFileToGMXList(name, AoDFileType::CAL);
		if (extension == "CHR")				name = AOD_IO.AddFileToGMXList(name, AoDFileType::CHR);
		if (extension == "CSS")				name = AOD_IO.AddFileToGMXList(name, AoDFileType::CSS);
		if (extension == "TXT")				name = AOD_IO.AddFileToGMXList(name, AoDFileType::TXT);
		if (extension == "CAM")				name = AOD_IO.AddFileToGMXList(name, AoDFileType::CAM);
		if (extension == "POS")				name = AOD_IO.AddFileToGMXList(name, AoDFileType::POS);
		if (extension == "TMS")				name = AOD_IO.AddFileToGMXList(name, AoDFileType::TMS);
		if (extension == "TMT")				name = AOD_IO.AddFileToGMXList(name, AoDFileType::TMT);
		if (extension == "XXX")				name = AOD_IO.AddFileToGMXList(name, AoDFileType::XXX);
		if (extension == "CBH")				name = AOD_IO.AddFileToGMXList(name, AoDFileType::CBH);
		if (extension == "CLN")				name = AOD_IO.AddFileToGMXList(name, AoDFileType::CLN);
		if (extension == "EVX")				name = AOD_IO.AddFileToGMXList(name, AoDFileType::EVX);
		if (extension == "RMX")				name = AOD_IO.AddFileToGMXList(name, AoDFileType::RMX);
		if (extension == "SCX")				name = AOD_IO.AddFileToGMXList(name, AoDFileType::SCX);

		for (unsigned int i = 0; i < 100; i++)			// Il level exporter supporta fino a 100 zone (il massimo usato nel gioco è 7)
		{
			stringstream zone;
			if (i < 10)
				zone << "Z0" << i;
			else
				zone << "Z" << i;
			if (extension == zone.str())	name = AOD_IO.AddFileToGMXList(name, AoDFileType::ZONE);
		}

		for (unsigned int i = 0; i < 200; i++)			// Curve di animazione dei blendshapes (tipo 2, contiene molteplici animazioni)
		{
			stringstream tms2;
			tms2 << i;
			if (extension == tms2.str())	name = AOD_IO.AddFileToGMXList(name, AoDFileType::TMS);
		}

		// Esporta i file
		betagmx.seekg(betagmx_list[i].Offset + 2048);												// Posiziona l'indice di lettura all'inizio del file da estrarre
		ofstream out;
		out.open(name, ofstream::binary);
		char* buffer = new char[betagmx_list[i].Size];												// Buffer di lettura
		betagmx.read(buffer, betagmx_list[i].Size);													// Legge il gmx e lo mette nel buffer
		out.write(buffer, betagmx_list[i].Size);													// Copia il buffer nel file di destinazione
		delete[] buffer;																			// Elimina il buffer
		out.close();
	}

	betagmx.close();
	return true;
}