/*------------------------------------------------------------------------------------------------------------------
Imposta i nomi di file e cartelle e ne controlla l'esistenza, chiede cosa esportare, mostra le prime due schermate di avvio
INPUT: char **argv
OUTPUT: AODRemastered_IO_CLASS &AODRemastered_IO
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Classes.h"


bool AODRemastered_IO_Init (char **argv)
{
	if (argv[1] == NULL)							// Se manca l'input utente il programma mostra un avviso e termina
	{
		msg(msg::TGT::CONS, msg::TYP::FATAL) << "ARGV[1] missing: specify a MSH file and retry.";
		return false;
	}

	/////////////////////// PREPARAZIONE STRINGHE CARTELLE EXE/MSH
	string input(argv[1]);
	if (input.find("\\") == string::npos)			// Se non trova "\" nella stringa in ingresso significa che l'EXE e il MSH sono nella stessa cartella
	{
		GetCurrentDirectory(MAX, AODRemastered_IO.folder_exe_lpwstr);									/*		C:\Cartella_EXE_MSH			*/
		AODRemastered_IO.folder_msh_lpwstr = AODRemastered_IO.folder_exe_lpwstr;						/*		C:\Cartella_EXE_MSH			*/
		AODRemastered_IO.folder_msh = CW2A(AODRemastered_IO.folder_exe_lpwstr);							/*      C:\Cartella_EXE_MSH			*/
		if (AODRemastered_IO.folder_msh.back() != '\\')				// Aggiunge "\" se assente alla fine del percorso (i files nel root dell'unità ce l'hanno già)
			AODRemastered_IO.folder_msh.append("\\");													/*		C:\Cartella_EXE_MSH\		*/
		AODRemastered_IO.file_msh = input;																/*		Nome_mesh(.MSH)				*/
	}
	else											// Se trova "\" allora salva il percorso attuale dell'EXE e ricava da "input" il percorso del MSH
	{
		GetModuleFileName(NULL, AODRemastered_IO.folder_exe_lpwstr, MAX);								/*		C:\Cartella_EXE\TRAODLE.EXE		*/
		string temp = CW2A(AODRemastered_IO.folder_exe_lpwstr);
		temp = temp.substr(0, temp.find_last_of("\\"));													/*      C:\Cartella_EXE	(o C:)			*/
		if (temp.find("\\") == string::npos)					// Aggiunge "\" se non ne trova nemmeno uno (per il root dell'unità)
			temp.append("\\");
		std::mbstowcs(AODRemastered_IO.folder_exe_lpwstr, temp.c_str(), MAX);							/*		C:\Cartella_EXE	(o C:\)			*/
		temp = input.substr(0, input.find_last_of("\\"));												/*      C:\Cartella_MSH (o C:)			*/
		if (temp.find("\\") == string::npos)					// Aggiunge "\" se non ne trova nemmeno uno (per il root dell'unità)
			temp.append("\\");
		std::mbstowcs(AODRemastered_IO.folder_msh_lpwstr, temp.c_str(), MAX);							/*		C:\Cartella_MSH (o C:\)			*/
		AODRemastered_IO.folder_msh = input.substr(0, input.find_last_of("\\") + 1);					/*		C:\Cartella_MSH\				*/
		AODRemastered_IO.file_msh = input.substr(input.find_last_of("\\") + 1);							/*		Nome_mesh(.MSH)					*/
	}

	/////////////////////// PREPARAZIONE NOME FILE DI INPUT (MSH)
	transform(AODRemastered_IO.file_msh.begin(), AODRemastered_IO.file_msh.end(), AODRemastered_IO.file_msh.begin(), ::toupper);		// Converte in maiuscolo il nome del file in ingresso
	unsigned int sz = AODRemastered_IO.file_msh.size();
	if (AODRemastered_IO.file_msh[sz - 4] == '.' && AODRemastered_IO.file_msh[sz - 3] == 'M' && AODRemastered_IO.file_msh[sz - 2] == 'S' && AODRemastered_IO.file_msh[sz - 1] == 'H')		// File in ingresso: MSH
		AODRemastered_IO.objectname = AODRemastered_IO.file_msh.substr(0, sz - 4);			// Vengono saltati gli ultimi 4 caratteri dell'estensione .MSH
	else					// Se il file in ingresso non ha estensione si aggiunge .MSH e si vede se esiste
	{
		SetCurrentDirectory(AODRemastered_IO.folder_msh_lpwstr);
		AODRemastered_IO.objectname = AODRemastered_IO.file_msh;
		AODRemastered_IO.file_msh.append(".MSH");
		ifstream TestIfExists(AODRemastered_IO.file_msh, std::ios::binary);
		if (!TestIfExists.is_open())							// Se il file MSH non viene trovato allora viene mostrato l'errore di file mancante ed il programma termina
		{
			msg(msg::TGT::CONS, msg::TYP::FATAL) << AODRemastered_IO.objectname << ".MSH not found.";
			return false;
		}
		TestIfExists.close();
	}

	/////////////////////// PREPARAZIONE NOME CARTELLA MESH (es. 50093659\)
	AODRemastered_IO.folder_object = AODRemastered_IO.folder_msh;
	AODRemastered_IO.folder_object.append(AODRemastered_IO.objectname);
	std::mbstowcs(AODRemastered_IO.folder_object_lpwstr, AODRemastered_IO.folder_object.c_str(), MAX);
	AODRemastered_IO.folder_object.append("\\");


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
	std::wcscpy(cfi.FaceName, L"Lucida Console");			// Seleziona il carattere Lucida Console (in questo momento non è ancora impostato!!!)
	SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);		// Imposta dimensione caratteri e font
	//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xf0);				// Imposta il colore di testo e sfondo
	std::system("chcp 437 >nul");												// Imposta il codepage americano
	//system("mode 119, 43");
	//concolinit();																// Inizializzazione colori testo concol
	return true;
}