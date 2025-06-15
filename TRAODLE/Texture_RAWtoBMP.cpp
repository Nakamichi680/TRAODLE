#include "stdafx.h"
#include "Classes.h"


void Texture_RAWtoBMP (string filename, unsigned int Xsize, unsigned int Ysize, unsigned int RAWsize, BMPType Type, bool FlipVertical, char* buffer)
{
	int BMP_size(RAWsize + 54);
	int offset_size = 54;
	if (Type == BMPType::GRAYSCALE)
	{
		BMP_size += 1024;														// Aggiunge la dimensione della rainbow table per la scala di grigio
		offset_size += 1024;
	}
	ofstream bmp;
	bmp.open(filename, ofstream::binary);
    bmp << char(0x42) << char(0x4d);											// Intestazione BMP
    bmp.write(reinterpret_cast<const char*>(&BMP_size), 4);						// Dimensione file BMP
    bmp << char(0x00) << char(0x00) << char(0x00) << char(0x00);
	bmp.write(reinterpret_cast<const char*>(&offset_size), 4);					// Offset inizio dati RAW texture
	bmp << char(0x28) << char(0x00) << char(0x00) << char(0x00);
    bmp.write(reinterpret_cast<const char*>(&Xsize), 4);						// DIMENSIONI DEI LATI DELLE TEXTURE
    bmp.write(reinterpret_cast<const char*>(&Ysize), 4);						// DIMENSIONI DEI LATI DELLE TEXTURE
    bmp << char(0x01) << char(0x00);
	if (Type == BMPType::BGRA || Type == BMPType::RGBA)
		bmp << char(0x20);														// 32 bit per pixel
	if (Type == BMPType::BGR || Type == BMPType::RGB)
		bmp << char(0x18);														// 24 bit per pixel
	if (Type == BMPType::GRAYSCALE)
		bmp << char(0x08);														// 8 bit per pixel
	bmp << char(0x00) << char(0x00) << char(0x00) << char(0x00) << char(0x00);
    bmp.write(reinterpret_cast<const char*>(&RAWsize), 4);						// Dimensioni dati RAW texture
    for (int h = 0; h < 16; h++)
        bmp << char(0x00);

	if (Type == BMPType::GRAYSCALE)												// Tabella colori per immagini monocromatiche a 8 bit per pixel
		for (unsigned int i = 0; i < 256; i++)
			bmp << char(i) << char(i) << char(i) << char(0x00);

	if (FlipVertical)											// Ribalta la texture verticalmente
	{
		char* buffer2 = new char[RAWsize];
		for (unsigned int row = 0; row < Ysize; row++)
		{
			if (Type == BMPType::BGRA || Type == BMPType::RGBA)
				memcpy(buffer2 + (Ysize - row - 1) * Xsize * 4, buffer + row * Xsize * 4, Xsize * 4);
			if (Type == BMPType::BGR || Type == BMPType::RGB)
				memcpy(buffer2 + (Ysize - row - 1) * Xsize * 3, buffer + row * Xsize * 3, Xsize * 3);
			if (Type == BMPType::GRAYSCALE)
				memcpy(buffer2 + (Ysize - row - 1) * Xsize, buffer + row * Xsize, Xsize);
		}
		memcpy(buffer, buffer2, RAWsize);
		delete[] buffer2;
	}

	if (Type == BMPType::BGRA)									// Effettua uno swap del primo e terzo byte per passare da RGBA a BGRA
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
		bmp.write(buffer2, RAWsize);											// Copia il buffer nel file di destinazione
		delete[] buffer2;
	}
	else
		if (Type == BMPType::BGR)								// Effettua uno swap del primo e terzo byte per passare da RGB a BGR
		{
			char* buffer2 = new char[RAWsize];
			for (unsigned int b = 0; b < RAWsize;)
			{
				buffer2[b] = buffer[b+2];
				buffer2[b+1] = buffer[b+1];
				buffer2[b+2] = buffer[b];
				b += 3;
			}
			bmp.write(buffer2, RAWsize);										// Copia il buffer nel file di destinazione
			delete[] buffer2;
		}
		else
			bmp.write(buffer, RAWsize);											// Copia il buffer nel file di destinazione
    bmp.close();
}