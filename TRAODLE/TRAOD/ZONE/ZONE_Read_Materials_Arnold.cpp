#include "stdafx.h"
#include "Classes.h"
#include "TRAOD/ZONE/ZONE_Struct.h"


Material ZONE_Read_Materials_Arnold(string zonename, bool diffuse_transparent, string material_name, const ZONE_MATERIALS_LIST& zone_materials_list)
{
	stringstream slot1, slot2, slot3, slot4;
	Material mat;
	mat.name = material_name;
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
		if (zone_materials_list.DiffuseID != -1)
			msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << material_name << " has texture(s) but diffuse color is disabled.";
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
	case (0b10000100):										// FUR (da fare)
		
		break;
	case (0b10001100):										// GREEN
		mat.Type = Material::TYPE::EMPTY;
		break;
	case (0b10010100):										// GLOW (da fare)

		break;
	case (0b10011000):										// SNOW (da fare)
		
		break;
	case (0b10011100):										// GLASS
		mat.Type = Material::TYPE::AISTANDARDSURFACE;
		if (zone_materials_list.DiffuseID != -1)
			mat.emissive = slot1.str();
		if (zone_materials_list.BumpSpecID != -1)
		{
			mat.specular = slot3.str();
			mat.bump = slot3.str();
		}
		mat.metalness = 0;
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

	return mat;
}