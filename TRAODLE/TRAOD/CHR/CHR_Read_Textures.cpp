#include "stdafx.h"
#include "Misc_Functions.h"
#include "TRAOD/CHR/CHR_Functions.h"


/*------------------------------------------------------------------------------------------------------------------
Esporta le textures del file CHR (stessa organizzazione delle textures delle zone):
 - \NOMELIVELLO\Characters\NOMECHR_Textures\Originals: file originali (DDS o BMP)
 - \NOMELIVELLO\Characters\NOMECHR_Textures: versioni TGA usate dai materiali del file MA
Ogni texture e' composta da CHR_TEXTURES_LIST (28 bytes) seguito dai dati raw.
------------------------------------------------------------------------------------------------------------------*/
bool CHR_Read_Textures (ifstream &chrfile, const CHR_HEADER &chr_header, string chrname, MA_EXPORT &MA)
{
	CHR_MATERIALS_HEADER chr_materials_header;
	CHR_TEXTURES_HEADER chr_textures_header;
	CHR_TEXTURES_LIST chr_textures_list;

	///////////////////    CREAZIONE CARTELLA PER TEXTURES
	string folder = AOD_IO.folder_characters + chrname + "_Textures";
	string folder_originals = folder + "\\Originals";
	LPWSTR folder_lpwstr = new TCHAR[MAX];
	LPWSTR folder_originals_lpwstr = new TCHAR[MAX];
	mbstowcs(folder_lpwstr, folder.c_str(), MAX);
	mbstowcs(folder_originals_lpwstr, folder_originals.c_str(), MAX);
	CreateDirectory(folder_lpwstr, NULL);							// Crea la cartella \NOMELIVELLO\Characters\NOMECHR_Textures
	CreateDirectory(folder_originals_lpwstr, NULL);					// Crea la cartella \NOMELIVELLO\Characters\NOMECHR_Textures\Originals

	///////////////////    LETTURA TEXTURES
	chrfile.seekg(chr_header.TEXTURE_PTR);
	chrfile.read(reinterpret_cast<char*>(&chr_materials_header.nMaterials), sizeof(chr_materials_header.nMaterials));
	chrfile.seekg(chr_materials_header.nMaterials * sizeof(CHR_MATERIALS_LIST), ios_base::cur);		// Salta il blocco materiali
	chrfile.read(reinterpret_cast<char*>(&chr_textures_header.nTextures), sizeof(chr_textures_header.nTextures));
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Number of textures: " << chr_textures_header.nTextures;

	bool result = true;
	for (unsigned int t = 0; t < chr_textures_header.nTextures; t++)
	{
		chrfile.read(reinterpret_cast<char*>(&chr_textures_list), sizeof(chr_textures_list));
		if (!chrfile)
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Error reading texture " << t << ".";
			result = false;
			break;
		}
		char* buffer = new char[chr_textures_list.RAWsize];
		chrfile.read(buffer, chr_textures_list.RAWsize);
		unsigned int Xsize = chr_textures_list.Xsize, Ysize = chr_textures_list.Ysize;

		stringstream filename1, filename2;
		filename1 << AOD_IO.levelname << "_" << chrname << "_" << t;
		filename2 << AOD_IO.levelname << "_" << chrname << "_" << t << ".tga";
		switch (chr_textures_list.DXT)
		{
		case (827611204):											// DXT1
		{
			SetCurrentDirectory(folder_originals_lpwstr);
			Texture_RAWtoDDS(filename1.str() + ".dds", Xsize, Ysize, chr_textures_list.Mips, chr_textures_list.RAWsize, DDSType::DXT1, true, buffer);
			char* buffer_rgb = new char[Xsize * Ysize * 3];
			Texture_DXT1toRGB(Xsize, Ysize, buffer, buffer_rgb);
			SetCurrentDirectory(folder_lpwstr);
			Texture_RAWtoTGA(filename2.str(), Xsize, Ysize, Xsize * Ysize * 3, TGAType::RGB, true, buffer_rgb);
			delete[] buffer_rgb;
			break;
		}
		case (861165636):											// DXT3
		{
			SetCurrentDirectory(folder_originals_lpwstr);
			Texture_RAWtoDDS(filename1.str() + ".dds", Xsize, Ysize, chr_textures_list.Mips, chr_textures_list.RAWsize, DDSType::DXT3, true, buffer);
			char* buffer_rgba = new char[Xsize * Ysize * 4];
			Texture_DXT3toRGBA(Xsize, Ysize, buffer, buffer_rgba);
			SetCurrentDirectory(folder_lpwstr);
			Texture_RAWtoTGA(filename2.str(), Xsize, Ysize, Xsize * Ysize * 4, TGAType::RGBA, true, buffer_rgba);
			delete[] buffer_rgba;
			break;
		}
		case (894720068):											// DXT5 (non usato nei CHR noti): solo DDS
			SetCurrentDirectory(folder_originals_lpwstr);
			Texture_RAWtoDDS(filename1.str() + ".dds", Xsize, Ysize, chr_textures_list.Mips, chr_textures_list.RAWsize, DDSType::DXT5, true, buffer);
			break;
		case (21):													// ARGB 8888
			SetCurrentDirectory(folder_originals_lpwstr);
			Texture_RAWtoBMP(filename1.str() + ".bmp", Xsize, Ysize, chr_textures_list.RAWsize, BMPType::RGBA, true, buffer);
			SetCurrentDirectory(folder_lpwstr);
			Texture_RAWtoTGA(filename2.str(), Xsize, Ysize, chr_textures_list.RAWsize, TGAType::RGBA, true, buffer);
			break;
		default:
			msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << "Texture " << t << " has an unknown format (" << chr_textures_list.DXT << ").";
			break;
		}
		delete[] buffer;

		// Creazione texture per il file MA
		Texture tex;
		stringstream texture_name;
		texture_name << AOD_IO.levelname << "_" << chrname << "_Texture_" << t;
		tex.name = texture_name.str();
		tex.filename = folder + "\\" + filename2.str();
		MA.Texture.push_back(tex);
	}
	delete[] folder_lpwstr;
	delete[] folder_originals_lpwstr;
	return result;
}
