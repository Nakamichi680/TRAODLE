#include "stdafx.h"
#include "MA/MA_Classes.h"
#include "MA/MA_Functions.h"

void MA_Write_Header (MA_EXPORT &MA)
{
	stringstream out;
	out << "//Maya ASCII 2016 scene\n";
	out << "//Codeset: 1252\n";
	out << "requires maya \"2016\";\n";
	out << "currentUnit -l centimeter -a degree -t film;\n";
			
	MA.MA_Header << out.str();		// Copia il testo nello stringstream Header definitivo
}