/*------------------------------------------------------------------------------------------------------------------
Imposta i nomi di file e cartelle e ne controlla l'esistenza, chiede cosa esportare, mostra le prime due schermate di avvio
INPUT: char **argv
OUTPUT: AOD_IO_CLASS &AOD_IO
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Classes.h"


bool AOD_IO_Init (char **argv)
{
	if (argv[1] == NULL)							// Se manca l'input utente il programma mostra un avviso e termina
	{
		msg(msg::TGT::CONS, msg::TYP::FATAL) << "ARGV[1] missing: specify a GMX or CLZ file and retry.";
		return false;
	}

	/////////////////////// PREPARAZIONE STRINGHE CARTELLE EXE/CLZ/GMX
    string input(argv[1]);
	if (input.find("\\") == string::npos)			// Se non trova "\" nella stringa in ingresso significa che l'EXE e il CLZ/GMX sono nella stessa cartella
    {
		GetCurrentDirectory(MAX, AOD_IO.folder_exe_lpwstr);												/*		C:\Cartella_EXE_CLZGMX				*/
		AOD_IO.folder_clzgmx_lpwstr = AOD_IO.folder_exe_lpwstr;											/*		C:\Cartella_EXE_CLZGMX				*/
		AOD_IO.folder_clzgmx = CW2A(AOD_IO.folder_exe_lpwstr);											/*      C:\Cartella_EXE_CLZGMX				*/	
		if (AOD_IO.folder_clzgmx.back() != '\\')					// Aggiunge "\" se assente alla fine del percorso (i files nel root dell'unità ce l'hanno già)
			AOD_IO.folder_clzgmx.append("\\");															/*		C:\Cartella_EXE_CLZGMX\				*/
		AOD_IO.file_clzgmx = input;																		/*		Nome_livello(.CLZ/.GMX)				*/
    }
	else											// Se trova "\" allora salva il percorso attuale dell'EXE e ricava da "input" il percorso del CLZ/GMX
	{
		GetModuleFileName(NULL, AOD_IO.folder_exe_lpwstr, MAX);											/*		C:\Cartella_EXE\TRAODLE.EXE			*/
		string temp = CW2A(AOD_IO.folder_exe_lpwstr);
		temp = temp.substr(0, temp.find_last_of("\\"));													/*      C:\Cartella_EXE	(o C:)				*/
		if (temp.find("\\") == string::npos)					// Aggiunge "\" se non ne trova nemmeno uno (per il root dell'unità)
			temp.append("\\");
		mbstowcs(AOD_IO.folder_exe_lpwstr, temp.c_str(), MAX);											/*		C:\Cartella_EXE	(o C:\)				*/
		temp = input.substr(0, input.find_last_of("\\"));												/*      C:\Cartella_CLZGMX (o C:)			*/	
		if (temp.find("\\") == string::npos)					// Aggiunge "\" se non ne trova nemmeno uno (per il root dell'unità)
			temp.append("\\");
		mbstowcs(AOD_IO.folder_clzgmx_lpwstr, temp.c_str(), MAX);										/*		C:\Cartella_CLZGMX (o C:\)			*/
		AOD_IO.folder_clzgmx = input.substr(0, input.find_last_of("\\") + 1);							/*		C:\Cartella_CLZGMX\					*/
		AOD_IO.file_clzgmx = input.substr(input.find_last_of("\\") + 1);								/*		Nome_livello(.CLZ/.GMX)				*/
	}

	/////////////////////// PREPARAZIONE NOME FILE DI INPUT (CLZ/GMX)
	transform(AOD_IO.file_clzgmx.begin(), AOD_IO.file_clzgmx.end(), AOD_IO.file_clzgmx.begin(), ::toupper);		// Converte in maiuscolo il nome del file in ingresso
	unsigned int sz = AOD_IO.file_clzgmx.size();
	if (AOD_IO.file_clzgmx[sz-4] == '.' && AOD_IO.file_clzgmx[sz-3] == 'C' && AOD_IO.file_clzgmx[sz-2] == 'L' && AOD_IO.file_clzgmx[sz-1] == 'Z')		// File in ingresso: CLZ
		AOD_IO.levelname = AOD_IO.file_clzgmx.substr(0, sz-8);			// Vengono saltati gli ultimi 8 caratteri dell'estensione .GMX.CLZ
	else
		if (AOD_IO.file_clzgmx[sz-4] == '.' && AOD_IO.file_clzgmx[sz-3] == 'G' && AOD_IO.file_clzgmx[sz-2] == 'M' && AOD_IO.file_clzgmx[sz-1] == 'X')	// File in ingresso: GMX
			AOD_IO.levelname = AOD_IO.file_clzgmx.substr(0, sz-4);
		else					// Se il file in ingresso non ha estensione si aggiunge .GMX e .CLZ e si vede se uno dei due esiste. GMX ha la precedenza su CLZ
		{
			SetCurrentDirectory(AOD_IO.folder_clzgmx_lpwstr);
			AOD_IO.levelname = AOD_IO.file_clzgmx;
			AOD_IO.file_clzgmx.append(".GMX");
			ifstream TestIfExists(AOD_IO.file_clzgmx, std::ios::binary);
			if (!TestIfExists.is_open())							// Se il file GMX non viene trovato si passa al file CLZ
			{
				AOD_IO.file_clzgmx.append(".CLZ");
				TestIfExists.open(AOD_IO.file_clzgmx, std::ios::binary);
				if (!TestIfExists.is_open())						// Se il file CLZ non viene trovato allora viene mostrato l'errore di file mancante ed il programma termina
				{
					msg(msg::TGT::CONS, msg::TYP::FATAL) << AOD_IO.levelname << ".GMX/.GMX.CLZ not found.";
					return false;
				}
			}
			TestIfExists.close();
		}

	/////////////////////// PREPARAZIONE NOME CARTELLA LIVELLO (es. PARIS1\)
	AOD_IO.folder_level = AOD_IO.folder_clzgmx;
	AOD_IO.folder_level.append(AOD_IO.levelname);
	mbstowcs(AOD_IO.folder_level_lpwstr, AOD_IO.folder_level.c_str(), MAX);
	AOD_IO.folder_level.append("\\");

	/////////////////////// PREPARAZIONE NOMI SOTTOCARTELLE (es. PARIS1\Cameras, PARIS1\Animations, PARIS1\Zones)
	AOD_IO.folder_cameras = AOD_IO.folder_animations = AOD_IO.folder_zones = AOD_IO.folder_rooms = AOD_IO.folder_level;
	AOD_IO.folder_cameras.append("Cameras");
	AOD_IO.folder_animations.append("Animations");
	AOD_IO.folder_zones.append("Zones");
	AOD_IO.folder_rooms.append("Rooms");
	AOD_IO.folder_blendshapes.append("Blendshapes");
	AOD_IO.folder_collisions.append("Collisions");
	AOD_IO.folder_characters.append("Characters");
	mbstowcs(AOD_IO.folder_cameras_lpwstr, AOD_IO.folder_cameras.c_str(), MAX);
	mbstowcs(AOD_IO.folder_animations_lpwstr, AOD_IO.folder_animations.c_str(), MAX);
	mbstowcs(AOD_IO.folder_zones_lpwstr, AOD_IO.folder_zones.c_str(), MAX);
	mbstowcs(AOD_IO.folder_rooms_lpwstr, AOD_IO.folder_rooms.c_str(), MAX);
	mbstowcs(AOD_IO.folder_blendshapes_lpwstr, AOD_IO.folder_blendshapes.c_str(), MAX);
	mbstowcs(AOD_IO.folder_collisions_lpwstr, AOD_IO.folder_collisions.c_str(), MAX);
	mbstowcs(AOD_IO.folder_characters_lpwstr, AOD_IO.folder_characters.c_str(), MAX);
	AOD_IO.folder_cameras.append("\\");
	AOD_IO.folder_animations.append("\\");
	AOD_IO.folder_zones.append("\\");
	AOD_IO.folder_rooms.append("\\");
	AOD_IO.folder_blendshapes.append("\\");
	AOD_IO.folder_collisions.append("\\");
	AOD_IO.folder_characters.append("\\");

	/////////////////////// AVVIO INTERFACCIA GRAFICA UTENTE
	GetConsoleTitle(AOD_IO.Console_OldTitle, MAX_PATH);		// Legge il nome iniziale dalla barra della console. Serve per ripristinarlo quando il programma termina
	SetConsoleTitle(AOD_IO.Console_NewTitle);				// Imposta il nome del programma nella barra della console di Windows
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
	system("chcp 437 >nul");														// Imposta il codepage americano
	//system("mode 119, 43");
	//concolinit();																// Inizializzazione colori testo concol
	return true;
}