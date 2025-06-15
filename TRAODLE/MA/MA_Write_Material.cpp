#include "stdafx.h"
#include "MA/MA_Classes.h"


void MA_Write_Material (unsigned int m, MA_EXPORT &MA)		// DA SISTEMARE
{
	stringstream out;
	if (!MA.Material[m].transparency.empty() && MA.Material[m].Blend != Material::BLEND::NORMAL)	// Transparency non funziona con OVERLAY e SUBTRACT
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Conflict between transparency and blending type in material " << MA.Material[m].name << ". Transparency will be ignored.";

	switch (MA.Material[m].Type)
	{
	case (Material::TYPE::EMPTY):
		out << "createNode shadingEngine -n \"" << MA.Material[m].name << "SG\";\n";
		out << "	setAttr \".ihi\" 0;\n";
		out << "	setAttr \".ro\" yes;\n";	
		MA.MA_Nodes << out.str();
		out.str("");

		out << "relationship \"link\" \":lightLinker1\" \"" << MA.Material[m].name << "SG.message\" \":defaultLightSet.message\";\n";
		out << "relationship \"shadowLink\" \":lightLinker1\" \"" << MA.Material[m].name << "SG.message\" \":defaultLightSet.message\";\n";
		MA.MA_Relationships << out.str();
		out.str("");

		out << "connectAttr \"" << MA.Material[m].name << "SG.pa\" \":renderPartition.st\" -na;\n";
		MA.MA_Connections << out.str();
		break;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	case (Material::TYPE::LAMBERT):
		out << "createNode lambert -n \"" << MA.Material[m].name << "\";\n";
		out << "	setAttr \".dc\" 1;\n";												// Imposta l'intensità Diffuse a 1 (default 0.8)
		out << "createNode shadingEngine -n \"" << MA.Material[m].name << "SG\";\n";
		out << "	setAttr \".ihi\" 0;\n";
		out << "	setAttr \".ro\" yes;\n";
		out << "createNode materialInfo -n \"" << MA.Material[m].name << "materialInfo\";\n";
		if (!MA.Material[m].color.empty() && !MA.Material[m].shadow.empty())			// Layered Texture Diffuse map + Shadow Map
		{
			out << "createNode layeredTexture -n \"" << MA.Material[m].name << "layeredTexture\";\n";
			out << "	setAttr -s 2 \".cs\";\n";
			out << "	setAttr \".cs[0].a\" 1;\n";
			out << "	setAttr \".cs[0].bm\" 6;\n";
			out << "	setAttr \".cs[0].iv\" yes;\n";
			out << "	setAttr \".cs[1].a\" 1;\n";
			out << "	setAttr \".cs[1].bm\" 1;\n";
			out << "	setAttr \".cs[1].iv\" yes;\n";
		}
		if (!MA.Material[m].bump.empty())
			out << "createNode bump2d -n \"" << MA.Material[m].name << "bump2d\";\n";
		MA.MA_Nodes << out.str();
		out.str("");

		out << "relationship \"link\" \":lightLinker1\" \"" << MA.Material[m].name << "SG.message\" \":defaultLightSet.message\";\n";
		out << "relationship \"shadowLink\" \":lightLinker1\" \"" << MA.Material[m].name << "SG.message\" \":defaultLightSet.message\";\n";
		MA.MA_Relationships << out.str();
		out.str("");

		if (!MA.Material[m].color.empty() && MA.Material[m].shadow.empty())				// Solo diffuse map
		{
			out << "connectAttr \"" << MA.Material[m].color << ".oc\" \"" << MA.Material[m].name << ".c\";\n";
			out << "connectAttr \"" << MA.Material[m].color << ".msg\" \"" << MA.Material[m].name << "materialInfo.t\" -na;\n";
		}
		if (MA.Material[m].color.empty() && !MA.Material[m].shadow.empty())				// Solo shadow map
		{
			out << "connectAttr \"" << MA.Material[m].shadow << ".oc\" \"" << MA.Material[m].name << ".c\";\n";
			out << "connectAttr \"" << MA.Material[m].shadow << ".msg\" \"" << MA.Material[m].name << "materialInfo.t\" -na;\n";
		}
		if (!MA.Material[m].color.empty() && !MA.Material[m].shadow.empty())			// Diffuse map + shadow map
		{
			out << "connectAttr \"" << MA.Material[m].name << "layeredTexture.oc\" \"" << MA.Material[m].name << ".c\";\n";
			out << "connectAttr \"" << MA.Material[m].name << "layeredTexture.msg\" \"" << MA.Material[m].name << "materialInfo.t\" -na;\n";
			out << "connectAttr \"" << MA.Material[m].color << ".oc\" \"" << MA.Material[m].name << "layeredTexture.cs[0].c\";\n";
			out << "connectAttr \"" << MA.Material[m].shadow << ".oc\" \"" << MA.Material[m].name << "layeredTexture.cs[1].c\";\n";
			out << "connectAttr \"" << MA.Material[m].name << "layeredTexture.msg\" \":defaultTextureList1.tx\" -na;\n";
		}
		if (!MA.Material[m].bump.empty())												// Bump map
		{
			out << "connectAttr \"" << MA.Material[m].name << "bump2d.o\" \"" << MA.Material[m].name << ".n\";\n";
			out << "connectAttr \"" << MA.Material[m].bump << ".oa\" \"" << MA.Material[m].name << "bump2d.bv\";\n";
			out << "connectAttr \"" << MA.Material[m].name << "bump2d.msg\" \":defaultRenderUtilityList.u\" -na;\n";
		}
		if (!MA.Material[m].transparency.empty())										// Trasparenza
			out << "connectAttr \"" << MA.Material[m].transparency << ".ot\" \"" << MA.Material[m].name << ".it\";\n";
		out << "connectAttr \"" << MA.Material[m].name << ".oc\" \"" << MA.Material[m].name << "SG.ss\";\n";
		out << "connectAttr \"" << MA.Material[m].name << "SG.msg\" \"" << MA.Material[m].name << "materialInfo.sg\";\n";
		out << "connectAttr \"" << MA.Material[m].name << "SG.pa\" \":renderPartition.st\" -na;\n";
		out << "connectAttr \"" << MA.Material[m].name << ".msg\" \"" << MA.Material[m].name << "materialInfo.m\";\n";
		out << "connectAttr \"" << MA.Material[m].name << ".msg\" \":defaultShaderList1.s\" -na;\n";
		MA.MA_Connections << out.str();
		break;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	case (Material::TYPE::AISTANDARDSURFACE):
		out << "relationship \"link\" \":lightLinker1\" \"" << MA.Material[m].name << "SG.message\" \":defaultLightSet.message\";\n";
		out << "relationship \"shadowLink\" \":lightLinker1\" \"" << MA.Material[m].name << "SG.message\" \":defaultLightSet.message\";\n";
		MA.MA_Relationships << out.str();
		out.str("");

		switch (MA.Material[m].Special)
		{
		case (Material::SPECIAL::NO):
			out << "createNode aiStandardSurface -n \"" << MA.Material[m].name << "\";\n";
			out << "	setAttr \".base\" 1;\n";											// Imposta Base Color Weight a 1 (default 0.8)
			if (MA.Material[m].specular.empty())											// Imposta Specular Weight a 0 (default 1) se non c'è la texture envelope
				out << "	setAttr \".specular\" 0;\n";
			else																			// Se c'è la texture envelope imposta i parametri di specular
			{
				if (MA.Material[m].specular_color_R != 1 || MA.Material[m].specular_color_G != 1 || MA.Material[m].specular_color_B != 1)
					out << "	setAttr \".specular_color\" -type \"float3\" " << MA.Material[m].specular_color_R << " " << MA.Material[m].specular_color_G << " " << MA.Material[m].specular_color_B << ";\n";
				out << "	setAttr \".specular_roughness\" 0;\n";
				out << "	setAttr \".specular_IOR\" 0.3;\n";
				if (MA.Material[m].metalness != 0)
					out << "	setAttr \".metalness\" " << MA.Material[m].metalness << ";\n";
			}
			if ((MA.Material[m].opacity_R != 1 || MA.Material[m].opacity_G != 1 || MA.Material[m].opacity_B != 1) &&  MA.Material[m].transparency.empty() && MA.Material[m].Blend == Material::BLEND::NORMAL)
				out << "	setAttr \".opacity\" -type \"float3\" " << MA.Material[m].opacity_R << " " << MA.Material[m].opacity_G << " " << MA.Material[m].opacity_B << ";\n";
			if (MA.Material[m].Blend == Material::BLEND::SUBTRACT)							// Creazione node aiNegate per blending subtract
			{
				out << "createNode aiNegate -n \"" << MA.Material[m].name << "aiNegate\";\n";
				out << "	setAttr \".input\" -type \"float3\" 1 1 1;\n";
			}
			out << "createNode shadingEngine -n \"" << MA.Material[m].name << "SG\";\n";
			out << "	setAttr \".ihi\" 0;\n";
			out << "	setAttr \".ro\" yes;\n";
			out << "createNode materialInfo -n \"" << MA.Material[m].name << "materialInfo\";\n";
			if (!MA.Material[m].color.empty() && !MA.Material[m].shadow.empty())			// Layered Texture Diffuse map + Shadow Map
			{
				out << "createNode layeredTexture -n \"" << MA.Material[m].name << "layeredTexture\";\n";
				out << "	setAttr -s 2 \".cs\";\n";
				out << "	setAttr \".cs[0].a\" 1;\n";
				out << "	setAttr \".cs[0].bm\" 6;\n";
				out << "	setAttr \".cs[0].iv\" yes;\n";
				out << "	setAttr \".cs[1].a\" 1;\n";
				out << "	setAttr \".cs[1].bm\" 1;\n";
				out << "	setAttr \".cs[1].iv\" yes;\n";
			}
			if (!MA.Material[m].bump.empty())
			{
				out << "createNode bump2d -n \"" << MA.Material[m].name << "bump2d\";\n";
				out << "	setAttr \".bd\" 20;\n";											// Bump Depth
			}
			MA.MA_Nodes << out.str();
			out.str("");

			if (!MA.Material[m].color.empty() && MA.Material[m].shadow.empty())								// Solo diffuse map
			{
				if (MA.Material[m].Blend == Material::BLEND::SUBTRACT)
					out << "connectAttr \"" << MA.Material[m].color << ".oc\" \"" << MA.Material[m].name << ".opacity\";\n";
				else
					out << "connectAttr \"" << MA.Material[m].color << ".oc\" \"" << MA.Material[m].name << ".base_color\";\n";
				out << "connectAttr \"" << MA.Material[m].color << ".msg\" \"" << MA.Material[m].name << "materialInfo.t\" -na;\n";
			}
			if (MA.Material[m].color.empty() && !MA.Material[m].shadow.empty())								// Solo shadow map
			{
				if (MA.Material[m].Blend == Material::BLEND::SUBTRACT)
					out << "connectAttr \"" << MA.Material[m].shadow << ".oc\" \"" << MA.Material[m].name << ".opacity\";\n";
				else
					out << "connectAttr \"" << MA.Material[m].shadow << ".oc\" \"" << MA.Material[m].name << ".base_color\";\n";
				out << "connectAttr \"" << MA.Material[m].shadow << ".msg\" \"" << MA.Material[m].name << "materialInfo.t\" -na;\n";
			}
			if (!MA.Material[m].color.empty() && !MA.Material[m].shadow.empty())							// Diffuse map + shadow map
			{
				if (MA.Material[m].Blend == Material::BLEND::SUBTRACT)
					out << "connectAttr \"" << MA.Material[m].name << "layeredTexture.oc\" \"" << MA.Material[m].name << ".opacity\";\n";
				else
					out << "connectAttr \"" << MA.Material[m].name << "layeredTexture.oc\" \"" << MA.Material[m].name << ".base_color\";\n";
				out << "connectAttr \"" << MA.Material[m].name << "layeredTexture.msg\" \"" << MA.Material[m].name << "materialInfo.t\" -na;\n";
				out << "connectAttr \"" << MA.Material[m].color << ".oc\" \"" << MA.Material[m].name << "layeredTexture.cs[0].c\";\n";
				out << "connectAttr \"" << MA.Material[m].shadow << ".oc\" \"" << MA.Material[m].name << "layeredTexture.cs[1].c\";\n";
				out << "connectAttr \"" << MA.Material[m].name << "layeredTexture.msg\" \":defaultTextureList1.tx\" -na;\n";
			}
			if (!MA.Material[m].bump.empty())																// Bump map
			{
				out << "connectAttr \"" << MA.Material[m].name << "bump2d.o\" \"" << MA.Material[m].name << ".n\";\n";
				out << "connectAttr \"" << MA.Material[m].bump << ".oa\" \"" << MA.Material[m].name << "bump2d.bv\";\n";
				out << "connectAttr \"" << MA.Material[m].name << "bump2d.msg\" \":defaultRenderUtilityList.u\" -na;\n";
			}
			if (!MA.Material[m].transparency.empty() && MA.Material[m].Blend == Material::BLEND::NORMAL)	// Trasparenza
			{
				out << "connectAttr \"" << MA.Material[m].transparency << ".oa\" \"" << MA.Material[m].name << ".opacityr\";\n";
				out << "connectAttr \"" << MA.Material[m].transparency << ".oa\" \"" << MA.Material[m].name << ".opacityg\";\n";
				out << "connectAttr \"" << MA.Material[m].transparency << ".oa\" \"" << MA.Material[m].name << ".opacityb\";\n";
			}
			if (MA.Material[m].Blend == Material::BLEND::OVERLAY)											// OVERLAY
			{
				out << "connectAttr \"" << MA.Material[m].color << ".oa\" \"" << MA.Material[m].name << ".emission\";\n";
				out << "connectAttr \"" << MA.Material[m].color << ".oc\" \"" << MA.Material[m].name << ".opacity\";\n";
			}
			if (MA.Material[m].Blend == Material::BLEND::SUBTRACT)											// SUBTRACT
			{
				out << "connectAttr \"" << MA.Material[m].name << "aiNegate.out\" \"" << MA.Material[m].name << ".base_color\";\n";
				out << "connectAttr \"" << MA.Material[m].name << "aiNegate.msg\" \":defaultRenderUtilityList.u\" -na;\n";
			}
			if (!MA.Material[m].specular.empty())																		// Specular map
			{
				out << "connectAttr \"" << MA.Material[m].specular << ".oa\" \"" << MA.Material[m].name << ".specular\";\n";
				out << "connectAttr \"" << MA.Material[m].color << ".oc\" \"" << MA.Material[m].name << ".specular_color\";\n";
			}
			out << "connectAttr \"" << MA.Material[m].name << ".out\" \"" << MA.Material[m].name << "SG.ss\";\n";
			out << "connectAttr \"" << MA.Material[m].name << "SG.msg\" \"" << MA.Material[m].name << "materialInfo.sg\";\n";
			out << "connectAttr \"" << MA.Material[m].name << "SG.pa\" \":renderPartition.st\" -na;\n";
			out << "connectAttr \"" << MA.Material[m].name << ".msg\" \"" << MA.Material[m].name << "materialInfo.m\";\n";
			out << "connectAttr \"" << MA.Material[m].name << ".msg\" \":defaultShaderList1.s\" -na;\n";
			MA.MA_Connections << out.str();
			break;

		case (Material::SPECIAL::GLASS):
			out << "createNode aiStandardSurface -n \"" << MA.Material[m].name << "\";\n";
			out << "	setAttr \".base\" 1;\n";											// Imposta Base Color Weight a 1 (default 0.8)
			out << "	setAttr \".specular_color\" -type \"float3\" 0.65 0.65 0.65;\n";
			out << "	setAttr \".specular_roughness\" 0;\n";
			out << "	setAttr \".specular_IOR\" 0.3;\n";
			if (MA.Material[m].transparency.empty() && MA.Material[m].Blend == Material::BLEND::NORMAL)
				out << "	setAttr \".opacity\" -type \"float3\" 0.5 0.5 0.5;\n";
			out << "createNode shadingEngine -n \"" << MA.Material[m].name << "SG\";\n";
			out << "	setAttr \".ihi\" 0;\n";
			out << "	setAttr \".ro\" yes;\n";
			out << "createNode materialInfo -n \"" << MA.Material[m].name << "materialInfo\";\n";
			MA.MA_Nodes << out.str();
			out.str("");

			if (!MA.Material[m].color.empty())												// Solo diffuse map
			{
				if (MA.Material[m].Blend == Material::BLEND::SUBTRACT)
					out << "connectAttr \"" << MA.Material[m].color << ".oc\" \"" << MA.Material[m].name << ".opacity\";\n";
				else
					out << "connectAttr \"" << MA.Material[m].color << ".oc\" \"" << MA.Material[m].name << ".base_color\";\n";
				out << "connectAttr \"" << MA.Material[m].color << ".msg\" \"" << MA.Material[m].name << "materialInfo.t\" -na;\n";
			}
			if (!MA.Material[m].transparency.empty() && MA.Material[m].Blend == Material::BLEND::NORMAL)	// Trasparenza
			{
				out << "connectAttr \"" << MA.Material[m].transparency << ".oa\" \"" << MA.Material[m].name << ".opacityr\";\n";
				out << "connectAttr \"" << MA.Material[m].transparency << ".oa\" \"" << MA.Material[m].name << ".opacityg\";\n";
				out << "connectAttr \"" << MA.Material[m].transparency << ".oa\" \"" << MA.Material[m].name << ".opacityb\";\n";
			}
			/*if (MA.Material[m].Blend == Material::BLEND::OVERLAY)											// OVERLAY
			{
				out << "connectAttr \"" << MA.Material[m].color << ".oa\" \"" << MA.Material[m].name << ".emission\";\n";
				out << "connectAttr \"" << MA.Material[m].color << ".oc\" \"" << MA.Material[m].name << ".opacity\";\n";
			}
			if (MA.Material[m].Blend == Material::BLEND::SUBTRACT)											// SUBTRACT
			{
				out << "connectAttr \"" << MA.Material[m].name << "aiNegate.out\" \"" << MA.Material[m].name << ".base_color\";\n";
				out << "connectAttr \"" << MA.Material[m].name << "aiNegate.msg\" \":defaultRenderUtilityList.u\" -na;\n";
			}*/
			if (!MA.Material[m].specular.empty())															// Specular map
			{
				out << "connectAttr \"" << MA.Material[m].specular << ".oa\" \"" << MA.Material[m].name << ".specular\";\n";
				out << "connectAttr \"" << MA.Material[m].color << ".oc\" \"" << MA.Material[m].name << ".specular_color\";\n";
			}
			out << "connectAttr \"" << MA.Material[m].name << ".out\" \"" << MA.Material[m].name << "SG.ss\";\n";
			out << "connectAttr \"" << MA.Material[m].name << "SG.msg\" \"" << MA.Material[m].name << "materialInfo.sg\";\n";
			out << "connectAttr \"" << MA.Material[m].name << "SG.pa\" \":renderPartition.st\" -na;\n";
			out << "connectAttr \"" << MA.Material[m].name << ".msg\" \"" << MA.Material[m].name << "materialInfo.m\";\n";
			out << "connectAttr \"" << MA.Material[m].name << ".msg\" \":defaultShaderList1.s\" -na;\n";
			MA.MA_Connections << out.str();
			break;
		
		}
		

	}
	return;
}