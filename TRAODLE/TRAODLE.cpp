#include "stdafx.h"
#include "Misc_Functions.h"
#include "TRAE/CLUSTER/CLUSTER_Functions.h"
#include "TRAE/LVC/LVC_Functions.h"
#include "TRAE/MDC/MDC_Functions.h"
#include "TRAE/TEXSET/TEXSET_Functions.h"
#include "TRAOD/CAM/CAM_Functions.h"
#include "TRAOD/ZONE/ZONE_Functions.h"
#include "TRAOD/GMX/GMX_Functions.h"
#include "TRAOD/RMX/RMX_Functions.h"
#include "TRAOD/CLN/CLN_Functions.h"
#include "TRAOD/CLZ/CLZ_Functions.h"
AOD_IO_CLASS AOD_IO;				// Classe Input/Output globale per AoD
AE_IO_CLASS AE_IO;					// Classe Input/Output globale per AE
ofstream msg_file_stream;
mutex mu;


int main(int argc, char **argv)
{
	chrono::steady_clock::time_point begin = chrono::steady_clock::now();
	msgInit();
	//FindBestThreadNumber();
	msg(msg::TGT::FILE_CONS, msg::TYP::OVR) << "Tomb Raider - The Angel of Darkness Level Exporter by Nakamichi680\n";
	msg(msg::TGT::FILE_CONS, msg::TYP::OVR) << "Version 0.200502\n";

	int mode = 1;			// 0 = Tomb Raider Anniversary Edition, 1 = Tomb Raider The Angel of Darkness

	switch (mode)
	{
	case 0:																	// TRAE
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Mode = 0 - AE";

		// INIZIALIZZAZIONE FILES E CARTELLE
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Initializing IO";
		if (!AE_IO_Init(argv))			// Se c'è qualsiasi errore in fase di avvio il programma termina
			return Fatal_Error_Terminate();
		
		msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Input filename: " << AE_IO.folder_cluster << AE_IO.file_cluster;

		// ESTRAZIONE FILES DAL CLUSTER
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Unpacking CLUSTER file";
		if (!Export_CLUSTER())
			return Fatal_Error_Terminate();
		
		// CREAZIONE CARTELLE PER FILE DI OUTPUT
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Creating target folders";
		SetCurrentDirectory(AE_IO.folder_level_lpwstr);
		if (AE_IO.SearchFileInClusterList("", AEFileType::LVC))
			CreateDirectory(AE_IO.folder_geometry_lpwstr, NULL);		// Crea la cartella \NOMELIVELLO\Geometry
		if (AE_IO.SearchFileInClusterList("", AEFileType::TXS))
			CreateDirectory(AE_IO.folder_textures_lpwstr, NULL);		// Crea la cartella \NOMELIVELLO\Textures
		if (AE_IO.SearchFileInClusterList("", AEFileType::MDC))
			CreateDirectory(AE_IO.folder_models_lpwstr, NULL);			// Crea la cartella \NOMELIVELLO\Models

		// BLOCCO CONVERSIONE FILES ESTRATTI DAL CLUSTER
		msg(msg::TGT::FILE, msg::TYP::LOG) << "*********************************";
		msg(msg::TGT::FILE, msg::TYP::LOG) << "*     Exporting files BEGIN     *";
		msg(msg::TGT::FILE, msg::TYP::LOG) << "*********************************";
		for (unsigned int i = 0; i < AE_IO.clusterfiles.size(); i++)
		{
			switch(AE_IO.clusterfiles[i].type)
			{
			case(AEFileType::LVC):
				if (!Export_LVC(AE_IO.clusterfiles[i].name))
					msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "LVC exporting completed with error(s)";
				break;
			case(AEFileType::TXS):
				if (!Export_TEXSET(AE_IO.clusterfiles[i].name))
					msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "TEXSET exporting completed with error(s)";
				break;
			case(AEFileType::AST):
				break;
			case(AEFileType::MDC):
				if (!Export_MDC(AE_IO.clusterfiles[i].name))
					msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "MDC exporting completed with error(s)";
				break;
			case(AEFileType::SEQ):
				break;
			case(AEFileType::CLU):
				break;
			}
		}
		msg(msg::TGT::FILE, msg::TYP::LOG) << "*********************************";
		msg(msg::TGT::FILE, msg::TYP::LOG) << "*      Exporting files END      *";
		msg(msg::TGT::FILE, msg::TYP::LOG) << "*********************************";
		break;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	case 1:																	// TRAOD
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Mode = 1 - AOD";

		// INIZIALIZZAZIONE FILES E CARTELLE
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Initializing IO";
		if (!AOD_IO_Init(argv))			// Se c'è qualsiasi errore in fase di avvio il programma termina
			return Fatal_Error_Terminate();

		msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Input filename: " << AOD_IO.folder_clzgmx << AOD_IO.file_clzgmx;

		// CONVERSIONE DA CLZ A GMX
		unsigned int sz = AOD_IO.file_clzgmx.size();
		if (AOD_IO.file_clzgmx[sz-3] == 'C' && AOD_IO.file_clzgmx[sz-2] == 'L' && AOD_IO.file_clzgmx[sz-1] == 'Z')
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "CLZ input file detected. Decompressing";
			if(!Decompress_CLZ())
				return Fatal_Error_Terminate();
		}

		// ESTRAZIONE FILES DAL GMX
		msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Unpacking GMX file";
		if (!Export_GMX())
			return Fatal_Error_Terminate();

		// CREAZIONE CARTELLE PER FILE DI OUTPUT
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Creating target folders";
		SetCurrentDirectory(AOD_IO.folder_level_lpwstr);
		if (AOD_IO.SearchFileInGMXList("", AoDFileType::CAM) || AOD_IO.SearchFileInGMXList("", AoDFileType::CBH))
			CreateDirectory(AOD_IO.folder_cameras_lpwstr, NULL);		// Crea la cartella \NOMELIVELLO\Cameras
		if (AOD_IO.SearchFileInGMXList("", AoDFileType::RMX))
			CreateDirectory(AOD_IO.folder_rooms_lpwstr, NULL);			// Crea la cartella \NOMELIVELLO\Rooms
		if (AOD_IO.SearchFileInGMXList("", AoDFileType::ZONE))
			CreateDirectory(AOD_IO.folder_zones_lpwstr, NULL);			// Crea la cartella \NOMELIVELLO\Zones
		if (AOD_IO.SearchFileInGMXList("", AoDFileType::CHR))
			CreateDirectory(AOD_IO.folder_characters_lpwstr, NULL);		// Crea la cartella \NOMELIVELLO\Characters
		if (AOD_IO.SearchFileInGMXList("", AoDFileType::TMT))
			CreateDirectory(AOD_IO.folder_blendshapes_lpwstr, NULL);	// Crea la cartella \NOMELIVELLO\Blendshapes
		if (AOD_IO.SearchFileInGMXList("", AoDFileType::CAL) || AOD_IO.SearchFileInGMXList("", AoDFileType::POS) ||
			AOD_IO.SearchFileInGMXList("", AoDFileType::XXX) || AOD_IO.SearchFileInGMXList("", AoDFileType::TMS))
			CreateDirectory(AOD_IO.folder_animations_lpwstr, NULL);		// Crea la cartella \NOMELIVELLO\Animations
		if (AOD_IO.SearchFileInGMXList("", AoDFileType::CLN))
			CreateDirectory(AOD_IO.folder_collisions_lpwstr, NULL);		// Crea la cartella \NOMELIVELLO\Collisions

		// BLOCCO CONVERSIONE FILES ESTRATTI DAL GMX
		msg(msg::TGT::FILE, msg::TYP::LOG) << "*********************************";
		msg(msg::TGT::FILE, msg::TYP::LOG) << "*     Exporting files BEGIN     *";
		msg(msg::TGT::FILE, msg::TYP::LOG) << "*********************************";

		for (unsigned int i = 0; i < AOD_IO.gmxfiles.size(); i++)
		{
			switch(AOD_IO.gmxfiles[i].type)
			{
			case(AoDFileType::RMX):
				if (!Export_RMX(AOD_IO.gmxfiles[i].name))
					msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "RMX exporting completed with error(s)";
				msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "";
				break;
			case(AoDFileType::ZONE):
				//if (!Export_ZONE(AOD_IO.gmxfiles[i].name))
					//msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "ZONE exporting completed with error(s)";
				msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "";
				break;
			case(AoDFileType::CAM):
				if (!Export_CAM(AOD_IO.gmxfiles[i].name))
					msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "CAM exporting completed with error(s)";
				msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "";
				break;
			case(AoDFileType::CLN):
				if (!Export_CLN(AOD_IO.gmxfiles[i].name))
					msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "CLN exporting completed with error(s)";
				msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "";
				break;
			}
		}
		msg(msg::TGT::FILE, msg::TYP::LOG) << "*********************************";
		msg(msg::TGT::FILE, msg::TYP::LOG) << "*      Exporting files END      *";
		msg(msg::TGT::FILE, msg::TYP::LOG) << "*********************************";
	}

	chrono::steady_clock::time_point end= chrono::steady_clock::now();
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << green << "Operation completed in " << chrono::duration_cast<chrono::seconds>(end - begin).count() << " seconds." << dark_white;
	SetConsoleTitle(AOD_IO.Console_OldTitle);													// Ripristina il vecchio nome della barra della console
	cout << endl;
	system("pause");
	msg_file_stream.close();
	return 0;
}
