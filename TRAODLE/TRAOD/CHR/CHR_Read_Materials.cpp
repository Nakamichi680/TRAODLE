#include "stdafx.h"
#include "TRAOD/CHR/CHR_Functions.h"
#include "TRAOD/ZONE/ZONE_Functions.h"
#include "TRAOD/ZONE/ZONE_Struct.h"


/*------------------------------------------------------------------------------------------------------------------
Legge i materiali del file CHR e li aggiunge al file MA. Il record dei materiali ha lo stesso formato di quello dei
file ZONE, quindi vengono riutilizzate le funzioni di conversione dei materiali della ZONE (il nome del personaggio
prende il posto del nome della zona nei nomi di materiali e textures).
------------------------------------------------------------------------------------------------------------------*/
bool CHR_Read_Materials (ifstream &chrfile, const CHR_HEADER &chr_header, string chrname, MA_EXPORT &MA)
{
	CHR_MATERIALS_HEADER chr_materials_header;
	CHR_TEXTURES_HEADER chr_textures_header;
	CHR_TEXTURES_LIST chr_textures_list;

	chrfile.seekg(chr_header.TEXTURE_PTR);
	chrfile.read(reinterpret_cast<char*>(&chr_materials_header.nMaterials), sizeof(chr_materials_header.nMaterials));
	vector <ZONE_MATERIALS_LIST> materials(chr_materials_header.nMaterials);
	static_assert(sizeof(ZONE_MATERIALS_LIST) == sizeof(CHR_MATERIALS_LIST), "Materiali CHR e ZONE devono avere lo stesso formato");
	if (!materials.empty())
		chrfile.read(reinterpret_cast<char*>(materials.data()), materials.size() * sizeof(ZONE_MATERIALS_LIST));

	// Formato di ogni texture (le textures DXT3 e ARGB hanno il canale alfa, usato per la trasparenza)
	chrfile.read(reinterpret_cast<char*>(&chr_textures_header.nTextures), sizeof(chr_textures_header.nTextures));
	vector <uint32_t> texture_format;
	for (unsigned int t = 0; t < chr_textures_header.nTextures && chrfile; t++)
	{
		chrfile.read(reinterpret_cast<char*>(&chr_textures_list), sizeof(chr_textures_list));
		texture_format.push_back(chr_textures_list.DXT);
		chrfile.seekg(chr_textures_list.RAWsize, ios_base::cur);
	}
	if (!chrfile)
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Error reading materials.";
		return false;
	}
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Number of materials: " << materials.size();

	for (unsigned int m = 0; m < materials.size(); m++)
	{
		bool diffuse_transparent = false;
		int diffuse = materials[m].DiffuseID;
		if (diffuse >= 0 && (unsigned int)diffuse < texture_format.size())
			diffuse_transparent = (texture_format[diffuse] == 861165636 || texture_format[diffuse] == 894720068 || texture_format[diffuse] == 21);	// DXT3/DXT5/ARGB

		stringstream material_name;
		material_name << AOD_IO.levelname << "_" << chrname << "_Material_" << m;
		Material mat;
		switch (TargetRenderer)			// Switch per tipo di output (Maya Hardware 2.0 o Arnold)
		{
		case (1):
			mat = ZONE_Read_Materials_MayaHw20(chrname, diffuse_transparent, material_name.str(), materials[m]);
			break;
		case (2):
			mat = ZONE_Read_Materials_Arnold(chrname, diffuse_transparent, material_name.str(), materials[m]);
			break;
		}
		MA.Material.push_back(mat);
	}
	return true;
}
