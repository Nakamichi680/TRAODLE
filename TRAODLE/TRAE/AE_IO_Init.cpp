/*------------------------------------------------------------------------------------------------------------------
Imposta i nomi di file e cartelle e ne controlla l'esistenza, chiede cosa esportare, mostra le prime due schermate di avvio
INPUT: char **argv
OUTPUT: AE_IO_CLASS &AE_IO
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Classes.h"


bool AE_IO_Init (char **argv)
{
	if (argv[1] == NULL)							// Se manca l'input utente il programma mostra un avviso e termina
	{
		msg(msg::TGT::CONS, msg::TYP::FATAL) << "ARGV[1] missing: specify a CLUSTER PS2/PSP/PC file and retry.";
		return false;
	}

	/////////////////////// PREPARAZIONE STRINGHE CARTELLE EXE/CLUSTER
    string input(argv[1]);
	if (input.find("\\") == string::npos)			// Se non trova "\" nella stringa in ingresso significa che l'EXE e il CLUSTER sono nella stessa cartella
    {
		GetCurrentDirectory(MAX, AE_IO.folder_exe_lpwstr);												/*		C:\Cartella_EXE_CLUSTER				*/
		AE_IO.folder_cluster_lpwstr = AE_IO.folder_exe_lpwstr;											/*		C:\Cartella_EXE_CLUSTER				*/
		AE_IO.folder_cluster = CW2A(AE_IO.folder_exe_lpwstr);											/*      C:\Cartella_EXE_CLUSTER				*/	
		if (AE_IO.folder_cluster.back() != '\\')					// Aggiunge "\" se assente alla fine del percorso (i files nel root dell'unità ce l'hanno già)
			AE_IO.folder_cluster.append("\\");															/*		C:\Cartella_EXE_CLUSTER\			*/
		AE_IO.file_cluster = input;																		/*		Nome_livello(.PC/.PS/.PSP)			*/
    }
	else											// Se trova "\" allora salva il percorso attuale dell'EXE e ricava da "input" il percorso del CLUSTER
	{
		GetModuleFileName(NULL, AE_IO.folder_exe_lpwstr, MAX);											/*		C:\Cartella_EXE\TRAODLE.EXE			*/
		string temp = CW2A(AE_IO.folder_exe_lpwstr);
		temp = temp.substr(0, temp.find_last_of("\\"));													/*      C:\Cartella_EXE	(o C:)				*/
		if (temp.find("\\") == string::npos)					// Aggiunge "\" se non ne trova nemmeno uno (per il root dell'unità)
			temp.append("\\");
		mbstowcs(AE_IO.folder_exe_lpwstr, temp.c_str(), MAX);											/*		C:\Cartella_EXE	(o C:\)				*/
		temp = input.substr(0, input.find_last_of("\\"));												/*      C:\Cartella_CLUSTER (o C:)			*/	
		if (temp.find("\\") == string::npos)					// Aggiunge "\" se non ne trova nemmeno uno (per il root dell'unità)
			temp.append("\\");
		mbstowcs(AE_IO.folder_cluster_lpwstr, temp.c_str(), MAX);										/*		C:\Cartella_CLUSTER (o C:\)			*/
		AE_IO.folder_cluster = input.substr(0, input.find_last_of("\\") + 1);							/*		C:\Cartella_CLUSTER\				*/
		AE_IO.file_cluster = input.substr(input.find_last_of("\\") + 1);								/*		Nome_livello(.PC/.PS/.PSP)			*/
	}

	/////////////////////// PREPARAZIONE NOME FILE DI INPUT (CLUSTER)
	transform(AE_IO.file_cluster.begin(), AE_IO.file_cluster.end(), AE_IO.file_cluster.begin(), ::toupper);		// Converte in maiuscolo il nome del file in ingresso
	unsigned int sz = AE_IO.file_cluster.size();
	if (AE_IO.file_cluster[sz-4] == '.' && AE_IO.file_cluster[sz-3] == 'P' && AE_IO.file_cluster[sz-2] == 'S' && AE_IO.file_cluster[sz-1] == 'P')	// File in ingresso: PSP
		AE_IO.levelname = AE_IO.file_cluster.substr(0, sz-4);			// Vengono saltati gli ultimi 4 caratteri dell'estensione .PSP
	else
		if (AE_IO.file_cluster[sz-3] == '.' && AE_IO.file_cluster[sz-2] == 'P' && AE_IO.file_cluster[sz-1] == 'S')									// File in ingresso: PS
			AE_IO.levelname = AE_IO.file_cluster.substr(0, sz-3);
		else
			if (AE_IO.file_cluster[sz-3] == '.' && AE_IO.file_cluster[sz-2] == 'P' && AE_IO.file_cluster[sz-1] == 'C')								// File in ingresso: PC
				AE_IO.levelname = AE_IO.file_cluster.substr(0, sz-3);
			else
				if (AE_IO.file_cluster[sz-4] == '.' && AE_IO.file_cluster[sz-3] == 'C' && AE_IO.file_cluster[sz-2] == 'L' && AE_IO.file_cluster[sz-1] == 'U')// File in ingresso: CLU
					AE_IO.levelname = AE_IO.file_cluster.substr(0, sz-4);
				else					// Se il file in ingresso non ha estensione si aggiunge .PSP, .PS o .PC e si vede se uno dei tre esiste
				{
					SetCurrentDirectory(AE_IO.folder_cluster_lpwstr);
					AE_IO.levelname = AE_IO.file_cluster;
					AE_IO.file_cluster.append(".PSP");
					ifstream TestIfExists(AE_IO.file_cluster, std::ios::binary);
					if (!TestIfExists.is_open())						// Se il file PSP non viene trovato si passa al file PS
					{
						AE_IO.file_cluster = AE_IO.file_cluster.substr(0, sz-4);
						AE_IO.file_cluster.append(".PS");
						TestIfExists.open(AE_IO.file_cluster, std::ios::binary);
						if (!TestIfExists.is_open())					// Se il file PS non viene trovato si passa al file PC
						{
							AE_IO.file_cluster = AE_IO.file_cluster.substr(0, sz-3);
							AE_IO.file_cluster.append(".PC");
							TestIfExists.open(AE_IO.file_cluster, std::ios::binary);
							if (!TestIfExists.is_open())				// Se il file PC non viene trovato si passa al file CLU
							{
								AE_IO.file_cluster = AE_IO.file_cluster.substr(0, sz-3);
								AE_IO.file_cluster.append(".CLU");
								TestIfExists.open(AE_IO.file_cluster, std::ios::binary);
								if (!TestIfExists.is_open())				// Se il file CLU non viene trovato allora viene mostrato l'errore di file mancante ed il programma termina
								{
									msg(msg::TGT::CONS, msg::TYP::FATAL) << AE_IO.levelname << ".CLU/.PSP/.PS/.PC not found.";
									return false;
								}
							}
						}
					}
					TestIfExists.close();
				}

	/////////////////////// PREPARAZIONE NOME CARTELLA LIVELLO (es. LEVEL3A\)
	AE_IO.folder_level = AE_IO.folder_cluster;
	AE_IO.folder_level.append(AE_IO.levelname);
	mbstowcs(AE_IO.folder_level_lpwstr, AE_IO.folder_level.c_str(), MAX);
	AE_IO.folder_level.append("\\");

	/////////////////////// PREPARAZIONE NOMI SOTTOCARTELLE (es. LEVEL3A\Models, LEVEL3A\Levels, LEVEL3A\Textures)
	AE_IO.folder_textures = AE_IO.folder_geometry = AE_IO.folder_models = AE_IO.folder_level;
	AE_IO.folder_textures.append("Textures");
	AE_IO.folder_geometry.append("Geometry");
	AE_IO.folder_models.append("Models");
	mbstowcs(AE_IO.folder_textures_lpwstr, AE_IO.folder_textures.c_str(), MAX);
	mbstowcs(AE_IO.folder_geometry_lpwstr, AE_IO.folder_geometry.c_str(), MAX);
	mbstowcs(AE_IO.folder_models_lpwstr, AE_IO.folder_models.c_str(), MAX);
	AE_IO.folder_textures.append("\\");
	AE_IO.folder_geometry.append("\\");
	AE_IO.folder_models.append("\\");

	/////////////////////// AVVIO INTERFACCIA GRAFICA UTENTE
	GetConsoleTitle(AE_IO.Console_OldTitle, MAX_PATH);		// Legge il nome iniziale dalla barra della console. Serve per ripristinarlo quando il programma termina
	SetConsoleTitle(AE_IO.Console_NewTitle);				// Imposta il nome del programma nella barra della console di Windows
	CONSOLE_FONT_INFOEX cfi;								// Struttura per la gestione della console di Windows
	cfi.cbSize = sizeof cfi;
	cfi.nFont = 0;
	cfi.dwFontSize.X = 8;									// Dimensione caratteri
	cfi.dwFontSize.Y = 14;									// Dimensione caratteri
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	wcscpy(cfi.FaceName, L"Lucida Console");				// Seleziona il carattere Lucida Console (in questo momento non è ancora impostato!!!)
	SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);		// Imposta dimensione caratteri e font
	//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xf0);				// Imposta il colore di testo e sfondo
	system("chcp 437 >nul");															// Imposta il codepage americano
	//system("mode 119, 43");
	//concolinit();																// Inizializzazione colori testo concol
	system("cls");
	return true;
}