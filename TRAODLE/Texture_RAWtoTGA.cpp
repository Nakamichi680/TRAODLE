#include "stdafx.h"
#include "Classes.h"


void Texture_RAWtoTGA (string filename, unsigned int Xsize, unsigned int Ysize, unsigned int RAWsize, TGAType Type, bool FlipVertical, char* buffer)
{
	ofstream tga;
	tga.open(filename, ofstream::binary);
	tga << char(0x00) << char(0x00);
	if (Type == TGAType::GRAYSCALE)
		tga << char(0x03);
	else
		tga << char(0x02);
	tga << char(0x00) << char(0x00) << char(0x00) << char(0x00) << char(0x00) << char(0x00) << char(0x00) << char(0x00) << char(0x00);
	tga.write(reinterpret_cast<const char*>(&Xsize), 2);						// DIMENSIONI DEI LATI DELLE TEXTURE
    tga.write(reinterpret_cast<const char*>(&Ysize), 2);						// DIMENSIONI DEI LATI DELLE TEXTURE
	if (Type == TGAType::BGRA || Type == TGAType::RGBA)
		tga << char(0x20) << char(0x08);										// 32 bit per pixel
	if (Type == TGAType::BGR || Type == TGAType::RGB)
		tga << char(0x18) << char(0x00);										// 24 bit per pixel
	if (Type == TGAType::GRAYSCALE)
		tga << char(0x08) << char(0x08);										// 8 bit per pixel

	if (FlipVertical)											// Ribalta la texture verticalmente
	{
		char* buffer2 = new char[RAWsize];
		for (unsigned int row = 0; row < Ysize; row++)
		{
			if (Type == TGAType::BGRA || Type == TGAType::RGBA)
				memcpy(buffer2 + (Ysize - row - 1) * Xsize * 4, buffer + row * Xsize * 4, Xsize * 4);
			if (Type == TGAType::BGR || Type == TGAType::RGB)
				memcpy(buffer2 + (Ysize - row - 1) * Xsize * 3, buffer + row * Xsize * 3, Xsize * 3);
			if (Type == TGAType::GRAYSCALE)
				memcpy(buffer2 + (Ysize - row - 1) * Xsize, buffer + row * Xsize, Xsize);
		}
		memcpy(buffer, buffer2, RAWsize);
		delete[] buffer2;
	}

	if (Type == TGAType::BGRA)									// Effettua uno swap del primo e terzo byte per passare da RGBA a BGRA
	{
		char* buffer2 = new char[RAWsize];
		for (unsigned int b = 0; b < RAWsize;)
		{
			buffer2[b] = buffer[b+2];
			buffer2[b+1] = buffer[b+1];
			buffer2[b+2] = buffer[b];
			buffer2[b+3] = buffer[b+3];
			b += 4;
		}
		tga.write(buffer2, RAWsize);											// Copia il buffer nel file di destinazione
		delete[] buffer2;
	}
	else
		if (Type == TGAType::BGR)								// Effettua uno swap del primo e terzo byte per passare da RGB a BGR
		{
			char* buffer2 = new char[RAWsize];
			for (unsigned int b = 0; b < RAWsize;)
			{
				buffer2[b] = buffer[b+2];
				buffer2[b+1] = buffer[b+1];
				buffer2[b+2] = buffer[b];
				b += 3;
			}
			tga.write(buffer2, RAWsize);										// Copia il buffer nel file di destinazione
			delete[] buffer2;
		}
		else
			tga.write(buffer, RAWsize);											// Copia il buffer nel file di destinazione
	
	tga << char(0x00) << char(0x00) << char(0x00) << char(0x00) << char(0x00) << char(0x00) << char(0x00) << char(0x00) << char(0x54);
	tga << char(0x52) << char(0x55) << char(0x45) << char(0x56) << char(0x49) << char(0x53) << char(0x49) << char(0x4F) << char(0x4E);
	tga << char(0x2D) << char(0x58) << char(0x46) << char(0x49) << char(0x4C) << char(0x45) << char(0x2E) << char(0x00);
	
    tga.close();
}