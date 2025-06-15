#include "stdafx.h"
#include "Classes.h"


void Texture_RAWtoDDS (string filename, unsigned int Xsize, unsigned int Ysize, unsigned int Mips, unsigned int RAWsize, DDSType Type, bool FlipVertical, char* buffer)
{
	ofstream dds;
    dds.open(filename, ofstream::binary);
    dds << char(0x44) << char(0x44) << char(0x53) << char(0x20) << char(0x7c) << char(0x00) << char(0x00) << char(0x00) << char(0x07) << char(0x10) << char(0x0a) << char(0x00);
    dds.write(reinterpret_cast<const char*>(&Xsize), 4);			// DIMENSIONI DEI LATI DELLE TEXTURE
    dds.write(reinterpret_cast<const char*>(&Ysize), 4);			// DIMENSIONI DEI LATI DELLE TEXTURE
    for (int h = 0; h < 8; h++)
		dds << char(0x00);
    dds.write(reinterpret_cast<const char*>(&Mips), 4);				// MIPMAPS
    for (int h = 0; h < 44; h++)
        dds << char(0x00);
    dds << char(0x20) << char(0x00) << char(0x00) << char(0x00) << char(0x04) << char(0x00) << char(0x00) << char(0x00);
	if (Type == DDSType::DXT1)
		dds << char(0x44) << char(0x58) << char(0x54) << char(0x31);
	if (Type == DDSType::DXT3)
		dds << char(0x44) << char(0x58) << char(0x54) << char(0x33);
	if (Type == DDSType::DXT5)
		dds << char(0x44) << char(0x58) << char(0x54) << char(0x35);
    for (int h = 0; h < 20; h++)
        dds << char(0x00);
    dds << char(0x08) << char(0x10) << char(0x40);
    for (int h = 0; h < 17; h++)
        dds << char(0x00);

	char* temp = new char[RAWsize];

	if (Type == DDSType::DXT1 && FlipVertical)		// Flip verticale DXT1
	{
		for (unsigned int b = 0; b < RAWsize; b += 8)		// Ribalta ciascun blocco di 4x4 pixels
		{
			memcpy(temp + b, buffer + b, 4);				// La color table di 4 byte viene lasciata nella medesima posizione
			memcpy(temp + b + 4, buffer + b + 7, 1);
			memcpy(temp + b + 5, buffer + b + 6, 1);
			memcpy(temp + b + 6, buffer + b + 5, 1);
			memcpy(temp + b + 7, buffer + b + 4, 1);
		}
		unsigned int base = 0;
		for (unsigned int b = 0; b < RAWsize;)				// Ribalta ciascuna riga di 4 pixels di altezza
		{
			unsigned int line_beg = base + Xsize * Ysize / 2 - (b - base + Xsize * 2);
			memcpy(buffer + b, temp + line_beg, Xsize * 2);
			b += Xsize * 2;
			if (b - base == Xsize * Ysize / 2)
			{
				base = b;
				Xsize /= 2;
				Ysize /= 2;
			}
		}
	}

	if (Type == DDSType::DXT3 && FlipVertical)				// Flip verticale DXT3
	{
		for (unsigned int b = 0; b < RAWsize; b += 16)		// Ribalta ciascun blocco di 4x4 pixels
		{
			memcpy(temp + b, buffer + b + 6, 2);
			memcpy(temp + b + 2, buffer + b + 4, 2);
			memcpy(temp + b + 4, buffer + b + 2, 2);
			memcpy(temp + b + 6, buffer + b, 2);
			memcpy(temp + b + 8, buffer + b + 8, 4);		// La color table di 4 byte viene lasciata nella medesima posizione
			memcpy(temp + b + 12, buffer + b + 15, 1);
			memcpy(temp + b + 13, buffer + b + 14, 1);
			memcpy(temp + b + 14, buffer + b + 13, 1);
			memcpy(temp + b + 15, buffer + b + 12, 1);
		}
		unsigned int base = 0;
		for (unsigned int b = 0; b < RAWsize;)				// Ribalta ciascuna riga di 4 pixels di altezza
		{
			unsigned int line_beg = base + Xsize * Ysize - (b - base + Xsize * 4);
			memcpy(buffer + b, temp + line_beg, Xsize * 4);
			b += Xsize * 4;
			if (b - base == Xsize * Ysize)
			{
				base = b;
				Xsize /= 2;
				Ysize /= 2;
			}
		}
	}

	if (Type == DDSType::DXT5 && FlipVertical)				// Flip verticale DXT5             !!!!!!!!!!!!!!!!!! DA IMPLEMENTARE !!!!!!!!!!!!!!!!!!
		msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << "DXT5 vertical flipping not supported yet. Texture will be exported unflipped.";

	delete[] temp;
	dds.write(buffer, RAWsize);										// Copia il buffer nel file di destinazione
    dds.close();
}