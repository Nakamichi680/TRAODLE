#include "stdafx.h"
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "Misc_Functions.h"
#include "TRAOD/ZONE/ZONE_Struct.h"


bool ZONE_Read_Materials (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	ZONE_HEADER zone_header;
	ZONE_MATERIALS_HEADER zone_materials_header;
	ZONE_MATERIALS_LIST zone_materials_list;
	ZONE_TEXTURE_LIST zone_texture_list;

	///////////////////    APERTURA FILE ZONE
	SetCurrentDirectory(AOD_IO.folder_level_lpwstr);			// Imposta la cartella contenente il file ZONE
	ifstream zonefile(filename, std::ios::binary);
	if (!zonefile.is_open())
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " not found.";
		return false;
	}

	///////////////////    CREAZIONE STRINGA CON NOME ZONA (es. Z01)
	string zonename = filename;
	zonename.erase(0, (zonename.find(".Z") + 1));

	///////////////////    LETTURA FILE ZONE
	zonefile.read(reinterpret_cast<char*>(&zone_header.ZONE_ID), sizeof(zone_header.ZONE_ID));

	if (zone_header.ZONE_ID != 32)			// Se il file ZONE non è valido
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " is not a valid ZONE file.";
		return false;
	}

	zonefile.read(reinterpret_cast<char*>(&zone_header.TEXTURE_PTR), sizeof(zone_header.TEXTURE_PTR));
    zonefile.seekg(zone_header.TEXTURE_PTR);
    zonefile.read(reinterpret_cast<char*>(&zone_materials_header.nMaterials), sizeof(zone_materials_header.nMaterials));	// Legge il numero di materiali
	zonefile.read(reinterpret_cast<char*>(&zone_materials_header.Unknown1), sizeof(zone_materials_header.Unknown1));
	zonefile.read(reinterpret_cast<char*>(&zone_materials_header.nTextures), sizeof(zone_materials_header.nTextures));
	zonefile.read(reinterpret_cast<char*>(&zone_materials_header.Unknown2), sizeof(zone_materials_header.Unknown2));

	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Number of materials: " << zone_materials_header.nMaterials;

	for (unsigned int m = 0; m < zone_materials_header.nMaterials; m++)
	{
		// Lettura materiale
		zonefile.read(reinterpret_cast<char*>(&zone_materials_list.TextureMode), sizeof(zone_materials_list.TextureMode));	// Tipologia di materiale
		zonefile.read(reinterpret_cast<char*>(&zone_materials_list.DoubleSided), sizeof(zone_materials_list.DoubleSided));	// Texture a doppia faccia
		zonefile.read(reinterpret_cast<char*>(&zone_materials_list.Unknown1), sizeof(zone_materials_list.Unknown1));		// Sembrerebbe sempre pari a 00000004h
		zonefile.read(reinterpret_cast<char*>(&zone_materials_list.DiffuseID), sizeof(zone_materials_list.DiffuseID));		// Slot 1: texture principale
		zonefile.read(reinterpret_cast<char*>(&zone_materials_list.ShadowMapID), sizeof(zone_materials_list.ShadowMapID));	// Slot 2: shadow map
		zonefile.read(reinterpret_cast<char*>(&zone_materials_list.BumpSpecID), sizeof(zone_materials_list.BumpSpecID));	// Slot 3: bump map e riflessi
		zonefile.read(reinterpret_cast<char*>(&zone_materials_list.FurID), sizeof(zone_materials_list.FurID));				// Slot 4: texture ARGB usata per fur (principalmente ciuffi di erba)
		streamoff return_position = zonefile.tellg();

		// Controllo trasparenza texture Diffuse
		bool diffuse_transparent = false;
		zonefile.seekg(zone_header.TEXTURE_PTR + 16 + zone_materials_header.nMaterials * 24 + zone_materials_list.DiffuseID * 40);	// Salta all'inizio dell'header della texture
		zonefile.read(reinterpret_cast<char*>(&zone_texture_list.DXT), sizeof(zone_texture_list.DXT));
		if (zone_texture_list.DXT == 861165636 || zone_texture_list.DXT == 894720068 || zone_texture_list.DXT == 21)				// Se la texture è DXT3/DXT5/ARGB 
			diffuse_transparent = true;
		zonefile.seekg(return_position);																							// Ritorna al materiale successivo

		// Creazione classe materiale per FBX e MA
		stringstream material_name, slot1, slot2, slot3, slot4;
		material_name << AOD_IO.levelname << "_" << zonename << "_Material_" << m;
		Material mat;
		mat.name = material_name.str();
		slot1 << AOD_IO.levelname << "_" << zonename << "_Texture_" << zone_materials_list.DiffuseID;
		slot2 << AOD_IO.levelname << "_" << zonename << "_Texture_" << zone_materials_list.ShadowMapID;
		slot3 << AOD_IO.levelname << "_" << zonename << "_Texture_" << zone_materials_list.BumpSpecID;
		slot4 << AOD_IO.levelname << "_" << zonename << "_Texture_" << zone_materials_list.FurID;
		
		// Decodifica materiale
		short mtype = zone_materials_list.TextureMode & 0b1111111111111100;		// Contiene esclusivamente le informazioni sul tipo di materiale
		short btype = zone_materials_list.TextureMode - mtype;					// Contiene esclusivamente le informazioni sul tipo di blending
		switch (mtype)
		{
		case (0):												// NONE
			mat.Type = Material::TYPE::AISTANDARDSURFACE;
			break;
		case (0b100):
		case (0b100100):										// DIFFUSE
			mat.Type = Material::TYPE::AISTANDARDSURFACE;
			if (zone_materials_list.DiffuseID != -1)
				mat.color = slot1.str();
			if (diffuse_transparent && btype == 3)
				mat.transparency = slot1.str();
			break;
		case (0b1000):											// BUMP
			mat.Type = Material::TYPE::AISTANDARDSURFACE;
			if (zone_materials_list.BumpSpecID != -1)
				mat.bump = slot3.str();
			break;
		case (0b1100):
		case (0b101100):										// DIFFUSEBUMP
			mat.Type = Material::TYPE::AISTANDARDSURFACE;
			if (zone_materials_list.DiffuseID != -1)
				mat.color = slot1.str();
			if (diffuse_transparent && btype == 3)
				mat.transparency = slot1.str();
			if (zone_materials_list.BumpSpecID != -1)
				mat.bump = slot3.str();
			break;
		case (0b10000):											// LIGHT
			mat.Type = Material::TYPE::AISTANDARDSURFACE;
			if (zone_materials_list.ShadowMapID != -1)
				mat.shadow = slot2.str();
			break;
		case (0b10100):
		case (0b110100):										// DIFFUSELIGHT
			mat.Type = Material::TYPE::AISTANDARDSURFACE;
			if (zone_materials_list.DiffuseID != -1)
				mat.color = slot1.str();
			if (diffuse_transparent && btype == 3)
				mat.transparency = slot1.str();
			if (zone_materials_list.ShadowMapID != -1)
				mat.shadow = slot2.str();
			break;
		case (0b11000):											// BUMPLIGHT
			mat.Type = Material::TYPE::AISTANDARDSURFACE;
			if (zone_materials_list.ShadowMapID != -1)
				mat.shadow = slot2.str();
			if (zone_materials_list.BumpSpecID != -1)
				mat.bump = slot3.str();
			break;
		case (0b11100):
		case (0b111100):										// DIFFUSEBUMPLIGHT
			mat.Type = Material::TYPE::AISTANDARDSURFACE;
			if (zone_materials_list.DiffuseID != -1)
				mat.color = slot1.str();
			if (diffuse_transparent && btype == 3)
				mat.transparency = slot1.str();
			if (zone_materials_list.ShadowMapID != -1)
				mat.shadow = slot2.str();
			if (zone_materials_list.BumpSpecID != -1)
				mat.bump = slot3.str();
			break;
		case (0b1000100):
		case (0b1100100):										// DIFFUSEENVELOPE
			mat.Type = Material::TYPE::AISTANDARDSURFACE;
			if (zone_materials_list.DiffuseID != -1)
				mat.color = slot1.str();
			if (diffuse_transparent && btype == 3)
				mat.transparency = slot1.str();
			if (zone_materials_list.BumpSpecID != -1)
				mat.specular = slot3.str();
			break;
		case (0b1001100):
		case (0b1101100):										// DIFFUSEBUMPENVELOPE
			mat.Type = Material::TYPE::AISTANDARDSURFACE;
			if (zone_materials_list.DiffuseID != -1)
				mat.color = slot1.str();
			if (diffuse_transparent && btype == 3)
				mat.transparency = slot1.str();
			if (zone_materials_list.BumpSpecID != -1)
			{
				mat.bump = slot3.str();
				mat.specular = slot3.str();
			}
			break;
		case (0b1010100):
		case (0b1110100):										// DIFFUSEENVELOPELIGHT
			mat.Type = Material::TYPE::AISTANDARDSURFACE;
			if (zone_materials_list.DiffuseID != -1)
				mat.color = slot1.str();
			if (diffuse_transparent && btype == 3)
				mat.transparency = slot1.str();
			if (zone_materials_list.BumpSpecID != -1)
				mat.specular = slot3.str();
			if (zone_materials_list.ShadowMapID != -1)
				mat.shadow = slot2.str();
			break;
		case (0b1011100):
		case (0b1111100):										// DIFFUSEBUMPENVELOPELIGHT
			mat.Type = Material::TYPE::AISTANDARDSURFACE;
			if (zone_materials_list.DiffuseID != -1)
				mat.color = slot1.str();
			if (diffuse_transparent && btype == 3)
				mat.transparency = slot1.str();
			if (zone_materials_list.BumpSpecID != -1)
			{
				mat.bump = slot3.str();
				mat.specular = slot3.str();
			}
			if (zone_materials_list.ShadowMapID != -1)
				mat.shadow = slot2.str();
			break;
		case (0b10000100):
			//mat.type = AoDMaterialType::FUR;
			break;
		case (0B10001100):										// GREEN
			mat.Type = Material::TYPE::EMPTY;
			break;
		case (0b10010100):

			//mat.type = AoDMaterialType::GLOW;
			break;
		case (0b10011000):
			//mat.type = AoDMaterialType::SNOW;
			break;
		case (0b10011100):										// GLASS
			mat.Type = Material::TYPE::AISTANDARDSURFACE;
			if (zone_materials_list.DiffuseID != -1)
				mat.color = slot1.str();
			if (diffuse_transparent && btype == 3)
				mat.transparency = slot1.str();
			if (zone_materials_list.BumpSpecID != -1)
				mat.specular = slot3.str();
			//mat.metalness = 0;
			//mat.specular_color_R = mat.specular_color_G = mat.specular_color_B = 0.65f;
			mat.opacity_R = mat.opacity_G = mat.opacity_B = 0.5f;
			break;
		}

		switch (btype)
		{
		case (0):
			if (zone_materials_list.DoubleSided)				// SUBTRACT
				mat.Blend = Material::BLEND::SUBTRACT;
			else
				mat.Blend = Material::BLEND::NORMAL;
			break;
		case (1):
		case (3):
			mat.Blend = Material::BLEND::NORMAL;
			break;
		case (2):												// OVERLAY
			mat.Blend = Material::BLEND::OVERLAY;
		}

		mat.Type = Material::TYPE::LAMBERT;		// Togliere per passare ad Arnold
		MA.Material.push_back(mat);
	}
	zonefile.close();
	return true;
}