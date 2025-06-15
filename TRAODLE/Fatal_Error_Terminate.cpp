/*------------------------------------------------------------------------------------------------------------------
Termina il programma in caso di errore
INPUT: 
OUTPUT: 1
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Classes.h"


int Fatal_Error_Terminate()
{
	cout << endl << endl;
	system("pause");
	SetConsoleTitle(AOD_IO.Console_OldTitle);									// Ripristina il vecchio nome della barra della console
	msg_file_stream.close();
	return 1;
}