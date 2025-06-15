/*------------------------------------------------------------------------------------------------------------------
Inizializza l'interfaccia di output su console e su file
INPUT: 
OUTPUT:
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"


void msgInit ()
{
	concolinit();
	time_t rawtime;
	struct tm * timeinfo;
	char filename[40];
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	strftime (filename, 40, "TRAODLE %F %H-%M-%S.log", timeinfo);
	msg_file_stream.open(filename);
}