#include "stdafx.h"
#include "MA/MA_Classes.h"


void MA_Write_Material_Arnold(unsigned int m, MA_EXPORT &MA)		// DA SISTEMARE
{
	Material mat = MA.Material[m];
	stringstream out;
	out << setprecision(7);
	if (!mat.transparency.empty() && mat.Blend != Material::BLEND::NORMAL)	// Transparency non funziona con OVERLAY e SUBTRACT
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Conflict between transparency and blending type in material " << mat.name << ". Transparency will be ignored.";
		mat.transparency.clear();
	}

	// RELATIONSHIPS
	out << "relationship \"link\" \":lightLinker1\" \"" << mat.name << "SG.message\" \":defaultLightSet.message\";\n";
	out << "relationship \"shadowLink\" \":lightLinker1\" \"" << mat.name << "SG.message\" \":defaultLightSet.message\";\n";
	MA.MA_Relationships << out.str();
	out.str("");

	switch (mat.Special)
	{
	case (Material::SPECIAL::NO):
		out << "createNode aiStandardSurface -n \"" << mat.name << "\";\n";
		out << "	setAttr \".base\" 1;\n";								// Imposta Base Color Weight a 1 (default 0.8)
		if (mat.specular.empty())											// Imposta Specular Weight a 0 (default 1) se non c'è la texture envelope
			out << "	setAttr \".specular\" 0;\n";
		else																// Se c'è la texture envelope imposta i parametri di specular
		{
			if (mat.specular_color_R != 1 || mat.specular_color_G != 1 || mat.specular_color_B != 1)
				out << "	setAttr \".specular_color\" -type \"float3\" " << mat.specular_color_R << " " << mat.specular_color_G << " " << mat.specular_color_B << ";\n";
			out << "	setAttr \".specular_roughness\" 0;\n";
			out << "	setAttr \".specular_IOR\" 0.3;\n";
			if (mat.metalness != 0)
				out << "	setAttr \".metalness\" " << mat.metalness << ";\n";
		}
		if ((mat.opacity_R != 1 || mat.opacity_G != 1 || mat.opacity_B != 1) && mat.transparency.empty() && mat.Blend == Material::BLEND::NORMAL)
			out << "	setAttr \".opacity\" -type \"float3\" " << mat.opacity_R << " " << mat.opacity_G << " " << mat.opacity_B << ";\n";
		if (mat.Blend == Material::BLEND::SUBTRACT)							// Creazione node aiNegate per blending subtract
		{
			out << "createNode aiNegate -n \"" << mat.name << "aiNegate\";\n";
			out << "	setAttr \".input\" -type \"float3\" 1 1 1;\n";
		}
		out << "createNode shadingEngine -n \"" << mat.name << "SG\";\n";
		out << "	setAttr \".ihi\" 0;\n";
		out << "	setAttr \".ro\" yes;\n";
		out << "createNode materialInfo -n \"" << mat.name << "materialInfo\";\n";
		if (!mat.color.empty() && !mat.shadow.empty())						// Layered Texture Diffuse map + Shadow Map
		{
			out << "createNode layeredTexture -n \"" << mat.name << "layeredTexture\";\n";
			out << "	setAttr -s 2 \".cs\";\n";
			out << "	setAttr \".cs[0].a\" 1;\n";
			out << "	setAttr \".cs[0].bm\" 6;\n";
			out << "	setAttr \".cs[0].iv\" yes;\n";
			out << "	setAttr \".cs[1].a\" 1;\n";
			out << "	setAttr \".cs[1].bm\" 1;\n";
			out << "	setAttr \".cs[1].iv\" yes;\n";
		}
		if (!mat.bump.empty())
		{
			out << "createNode bump2d -n \"" << mat.name << "bump2d\";\n";
			out << "	setAttr \".bd\" 20;\n";								// Bump Depth
		}
		MA.MA_Nodes << out.str();
		out.str("");

		if (!mat.color.empty() && mat.shadow.empty())						// Solo diffuse map
		{
			if (mat.Blend == Material::BLEND::SUBTRACT)
				out << "connectAttr \"" << mat.color << ".oc\" \"" << mat.name << ".opacity\";\n";
			else
				out << "connectAttr \"" << mat.color << ".oc\" \"" << mat.name << ".base_color\";\n";
			out << "connectAttr \"" << mat.color << ".msg\" \"" << mat.name << "materialInfo.t\" -na;\n";
		}
		if (mat.color.empty() && !mat.shadow.empty())						// Solo shadow map
		{
			if (mat.Blend == Material::BLEND::SUBTRACT)
				out << "connectAttr \"" << mat.shadow << ".oc\" \"" << mat.name << ".opacity\";\n";
			else
				out << "connectAttr \"" << mat.shadow << ".oc\" \"" << mat.name << ".base_color\";\n";
			out << "connectAttr \"" << mat.shadow << ".msg\" \"" << mat.name << "materialInfo.t\" -na;\n";
		}
		if (!mat.color.empty() && !mat.shadow.empty())						// Diffuse map + shadow map
		{
			if (mat.Blend == Material::BLEND::SUBTRACT)
				out << "connectAttr \"" << mat.name << "layeredTexture.oc\" \"" << mat.name << ".opacity\";\n";
			else
				out << "connectAttr \"" << mat.name << "layeredTexture.oc\" \"" << mat.name << ".base_color\";\n";
			out << "connectAttr \"" << mat.name << "layeredTexture.msg\" \"" << mat.name << "materialInfo.t\" -na;\n";
			out << "connectAttr \"" << mat.color << ".oc\" \"" << mat.name << "layeredTexture.cs[0].c\";\n";
			out << "connectAttr \"" << mat.shadow << ".oc\" \"" << mat.name << "layeredTexture.cs[1].c\";\n";
			out << "connectAttr \"" << mat.name << "layeredTexture.msg\" \":defaultTextureList1.tx\" -na;\n";
		}
		if (!mat.bump.empty())												// Bump map
		{
			out << "connectAttr \"" << mat.name << "bump2d.o\" \"" << mat.name << ".n\";\n";
			out << "connectAttr \"" << mat.bump << ".oa\" \"" << mat.name << "bump2d.bv\";\n";
			out << "connectAttr \"" << mat.name << "bump2d.msg\" \":defaultRenderUtilityList1.u\" -na;\n";
		}
		if (!mat.transparency.empty() && mat.Blend == Material::BLEND::NORMAL)	// Trasparenza
		{
			out << "connectAttr \"" << mat.transparency << ".oa\" \"" << mat.name << ".opacityr\";\n";
			out << "connectAttr \"" << mat.transparency << ".oa\" \"" << mat.name << ".opacityg\";\n";
			out << "connectAttr \"" << mat.transparency << ".oa\" \"" << mat.name << ".opacityb\";\n";
		}
		if (mat.Blend == Material::BLEND::OVERLAY)							// OVERLAY
		{
			out << "connectAttr \"" << mat.color << ".oa\" \"" << mat.name << ".emission\";\n";
			out << "connectAttr \"" << mat.color << ".oc\" \"" << mat.name << ".opacity\";\n";
		}
		if (mat.Blend == Material::BLEND::SUBTRACT)							// SUBTRACT
		{
			out << "connectAttr \"" << mat.name << "aiNegate.out\" \"" << mat.name << ".base_color\";\n";
			out << "connectAttr \"" << mat.name << "aiNegate.msg\" \":defaultRenderUtilityList1.u\" -na;\n";
		}
		if (!mat.specular.empty())											// Specular map
		{
			out << "connectAttr \"" << mat.specular << ".oa\" \"" << mat.name << ".specular\";\n";
			out << "connectAttr \"" << mat.color << ".oc\" \"" << mat.name << ".specular_color\";\n";
		}
		out << "connectAttr \"" << mat.name << ".out\" \"" << mat.name << "SG.ss\";\n";
		out << "connectAttr \"" << mat.name << "SG.msg\" \"" << mat.name << "materialInfo.sg\";\n";
		out << "connectAttr \"" << mat.name << "SG.pa\" \":renderPartition.st\" -na;\n";
		out << "connectAttr \"" << mat.name << ".msg\" \"" << mat.name << "materialInfo.m\";\n";
		out << "connectAttr \"" << mat.name << ".msg\" \":defaultShaderList1.s\" -na;\n";
		MA.MA_Connections << out.str();
		break;

	case (Material::SPECIAL::GLASS):
		out << "createNode aiStandardSurface -n \"" << mat.name << "\";\n";
		out << "	setAttr \".base\" 0;\n";											// Imposta Base Color Weight a 0 (default 0.8)
		out << "	setAttr \".metalness\" " << mat.metalness << ";\n";
		out << "	setAttr \".specular_roughness\" 0;\n";
		out << "	setAttr \".specular_IOR\" 1.9;\n";
		out << "	setAttr \".specular_anisotropy\" 0.5;\n";
		out << "	setAttr \".transmission\" 1;\n";
		out << "	setAttr -av \".emission\" 0.1;\n";
		out << "createNode shadingEngine -n \"" << mat.name << "SG\";\n";
		out << "	setAttr \".ihi\" 0;\n";
		out << "	setAttr \".ro\" yes;\n";
		out << "createNode materialInfo -n \"" << mat.name << "materialInfo\";\n";

		if (!mat.bump.empty())
		{
			out << "createNode bump2d -n \"" << mat.name << "bump2d\";\n";
			out << "	setAttr \".bd\" 20;\n";								// Bump Depth
		}

		MA.MA_Nodes << out.str();
		out.str("");

		if (!mat.emissive.empty())											// Diffuse mappato su emissive
		{
			out << "connectAttr \"" << mat.emissive << ".oc\" \"" << mat.name << ".emission_color\";\n";
			out << "connectAttr \"" << mat.emissive << ".msg\" \"" << mat.name << "materialInfo.t\" -na;\n";
		}

		if (!mat.specular.empty())											// Specular map
			out << "connectAttr \"" << mat.specular << ".oa\" \"" << mat.name << ".specular\";\n";

		if (!mat.bump.empty())												// Bump map
		{
			out << "connectAttr \"" << mat.name << "bump2d.o\" \"" << mat.name << ".n\";\n";
			out << "connectAttr \"" << mat.bump << ".oa\" \"" << mat.name << "bump2d.bv\";\n";
			out << "connectAttr \"" << mat.name << "bump2d.msg\" \":defaultRenderUtilityList1.u\" -na;\n";
		}

		out << "connectAttr \"" << mat.name << ".out\" \"" << mat.name << "SG.ss\";\n";
		out << "connectAttr \"" << mat.name << "SG.msg\" \"" << mat.name << "materialInfo.sg\";\n";
		out << "connectAttr \"" << mat.name << "SG.pa\" \":renderPartition.st\" -na;\n";
		out << "connectAttr \"" << mat.name << ".msg\" \"" << mat.name << "materialInfo.m\";\n";
		out << "connectAttr \"" << mat.name << ".msg\" \":defaultShaderList1.s\" -na;\n";
		MA.MA_Connections << out.str();
		break;
	}

}