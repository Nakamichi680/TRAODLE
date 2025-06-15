#include "stdafx.h"
#include "TRAE/TEXSET/TEXSET_Struct.h"
//#include "TRAE/TRAE_Functions.h"
#include "Misc_Functions.h"


bool Export_TEXSET (string filename)
{
	TEXSET_HEADER header;
	TEXSET_TEXTURE_INFO info;
	TEXSET_TEXTURE_DATA data;

	///////////////////    APERTURA FILE TEXSET
	SetCurrentDirectory(AE_IO.folder_level_lpwstr);								// Imposta la cartella \NOMELIVELLO come attiva
	ifstream texsetfile(filename, std::ios::binary);
	if (!texsetfile.is_open())
	{
		cout << "\nFATAL ERROR: " << filename << " not found.";
		return false;
	}

	// Lettura Header
	texsetfile.read(reinterpret_cast<char*>(&header.Magic1), sizeof(header.Magic1));							// Lettura tipologia di file (9)
	texsetfile.read(reinterpret_cast<char*>(&header.TargetPlatform), sizeof(header.TargetPlatform));			// Lettura piattaforma
	texsetfile.read(reinterpret_cast<char*>(&header.Magic2), sizeof(header.Magic2));							// Lettura tipologia di file ("TXS")
	
	if (header.Magic1 != 9 || header.Magic2 != 5462100)							// Se il file non è valido la funzione termina
	{
		cout << "\nFATAL ERROR: " << filename << " is not a valid TRAE Texture Set file.";
		return false;
	}

	if (header.TargetPlatform != 0x100 && header.TargetPlatform != 0x1000)
	{
		cout << "\nFATAL ERROR: " << filename << " is not a PC/PSP Texture Set file. PS2 Texture Set files are not supported.";
		return false;
	}

	texsetfile.read(reinterpret_cast<char*>(&header.PtrToMaterialsHeader), sizeof(header.PtrToMaterialsHeader));
	texsetfile.read(reinterpret_cast<char*>(&header.PtrToTextureData), sizeof(header.PtrToTextureData));			// Puntatore ad inizio blocco dati texture
	texsetfile.read(reinterpret_cast<char*>(&header.PtrToMaterialsIndices), sizeof(header.PtrToMaterialsIndices));	// Puntatore ad inizio blocco indici 1
	texsetfile.read(reinterpret_cast<char*>(&header.PtrToTextureInfo), sizeof(header.PtrToTextureInfo));			// Puntatore ad inizio blocco info texture
	texsetfile.read(reinterpret_cast<char*>(&header.PtrToIndices2), sizeof(header.PtrToIndices2));					// Puntatore ad inizio blocco indici 2
	texsetfile.read(reinterpret_cast<char*>(&header.PtrToSomething4), sizeof(header.PtrToSomething4));
	texsetfile.read(reinterpret_cast<char*>(&header.nTextures), sizeof(header.nTextures));							// Numero di texture

	AE_IO.folder_temp = AE_IO.folder_textures;
	AE_IO.folder_temp.append(filename.substr(0, filename.size()-4));
	mbstowcs(AE_IO.folder_temp_lpwstr, AE_IO.folder_temp.c_str(), MAX);
	CreateDirectory(AE_IO.folder_temp_lpwstr, NULL);							// Crea la cartella \NOMELIVELLO\Textures\NomeTexture
	SetCurrentDirectory(AE_IO.folder_temp_lpwstr);								// Imposta la cartella \NOMELIVELLO\Textures\NomeTexture come attiva
	// Lettura ed esportazione textures
	for (unsigned int t = 0; t < header.nTextures; t++)							// Questo macroblocco "for" legge tutti i dati di ciascuna texture e la esporta
	{
		texsetfile.seekg(header.PtrToTextureInfo + t * 36);
		texsetfile.read(reinterpret_cast<char*>(&info.Xsize), sizeof(info.Xsize));								// Dimensione X della texture (esponente di 2)
		texsetfile.read(reinterpret_cast<char*>(&info.Ysize), sizeof(info.Ysize));								// Dimensione Y della texture (esponente di 2)
		texsetfile.seekg(12, ios_base::cur);									// Salta il padding, Texture_ID, Unknown1 ed Unknown2
		texsetfile.read(reinterpret_cast<char*>(&info.Texture_name), sizeof(info.Texture_name));				// Nome della texture
		texsetfile.read(reinterpret_cast<char*>(&info.Texture_offset), sizeof(info.Texture_offset));			// Posizione della texture rispetto a header.PtrToTextureData
		texsetfile.seekg(header.PtrToTextureData + info.Texture_offset);		// Posizionamento lettura ad inizio del blocco Texture Data
		texsetfile.read(reinterpret_cast<char*>(&data.Xsize), sizeof(data.Xsize));								// Dimensione X della texture (esponente di 2)
		texsetfile.read(reinterpret_cast<char*>(&data.Ysize), sizeof(data.Ysize));								// Dimensione Y della texture (esponente di 2)
		if (data.Xsize != info.Xsize || data.Ysize != info.Ysize)
			cout << "\nERROR: difference at texture " << t << ". X/Y size mismatch between data and info blocks.";
		texsetfile.seekg(28, ios_base::cur);									// Salta Unknown1, Texture_hash, DEADBEAF e Texture_name
		unsigned int RAWsize = (unsigned int)pow(2, info.Xsize) * (unsigned int)pow(2, info.Ysize) * 4;
		char* buffer = new char[RAWsize];										// Buffer di lettura
		string temp = info.Texture_name;
		temp += ".BMP";
		
		if (header.TargetPlatform == 0x100)		// PC
		{
			texsetfile.read(buffer, RAWsize);									// Legge i dati raw della texture e li mette nel buffer
			Texture_RAWtoBMP(temp, (unsigned int)pow(2, info.Xsize), (unsigned int)pow(2, info.Ysize), RAWsize, BMPType::BGRA, false, buffer);
		}

		if (header.TargetPlatform == 0x1000)		// PSP
		{
			//texsetfile.seekg(4, ios_base::cur);
			unsigned int boh1, colortablesize;
			texsetfile.read(reinterpret_cast<char*>(&boh1), sizeof(boh1));
			texsetfile.read(reinterpret_cast<char*>(&colortablesize), sizeof(colortablesize));
			texsetfile.seekg(24, ios_base::cur);
			cout << "\nboh1: " << boh1;
			//cout << "\ncolorablesize: " << colortablesize;

			vector <unsigned int> colortable (colortablesize / 4);

			for (unsigned int t = 0; t < colortablesize / 4; t++)													// Lettura tabella colori
				texsetfile.read(reinterpret_cast<char*>(&colortable[t]), 4);

			//cout << "\nXsize: " << (unsigned int)pow(2, info.Xsize) << "      Ysize: " << (unsigned int)pow(2, info.Ysize);
			char* buffer2 = new char[(unsigned int)pow(2, info.Xsize) * (unsigned int)pow(2, info.Ysize)];
			texsetfile.read(buffer2, (unsigned int)pow(2, info.Xsize) * (unsigned int)pow(2, info.Ysize));

			for (unsigned int p = 0; p < (unsigned int)pow(2, info.Xsize) * (unsigned int)pow(2, info.Ysize); p++)	// Lettura indici pixels
				memcpy(buffer + p * 4, &colortable[buffer2[p]], 4);

			Texture_RAWtoBMP(temp, (unsigned int)pow(2, info.Xsize), (unsigned int)pow(2, info.Ysize), RAWsize, BMPType::BGRA, false, buffer);
		}
		delete[] buffer;
	}
	texsetfile.close();
	return true;
}