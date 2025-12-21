#include "stdafx.h"
#include "MA/MA_Classes.h"


void MA_Write_Material_Phong(unsigned int m, MA_EXPORT &MA)
{
	Material mat = MA.Material[m];
	stringstream out;
	out << setprecision(7);
	if (!mat.transparency.empty() && mat.Blend != Material::BLEND::NORMAL)	// Transparency non funziona con OVERLAY e SUBTRACT
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Conflict between transparency and blending type in material " << mat.name << ". Transparency will be ignored.";
		mat.transparency.clear();
	}

	// NODI
	out << "createNode phong -n \"" << mat.name << "\";\n";
	out << "	setAttr \".dc\" 1;\n";												// Imposta l'intensità Diffuse a 1 (default 0.8)
	out << "	setAttr \".trsd\" 0;\n";											// Imposta l'intensità Translucence a 0
	if (mat.specular.empty())
		out << "	setAttr \".sc\" -type \"float3\" " << mat.specular_color_R << " " << mat.specular_color_G << " " << mat.specular_color_B << ";\n";
	out << "	setAttr \".rc\" -type \"float3\" " << mat.reflectivity_color_R << " " << mat.reflectivity_color_G << " " << mat.reflectivity_color_B << ";\n";
	out << "createNode shadingEngine -n \"" << mat.name << "SG\";\n";
	out << "	setAttr \".ihi\" 0;\n";
	out << "	setAttr \".ro\" yes;\n";
	out << "createNode materialInfo -n \"" << mat.name << "materialInfo\";\n";
	if (!mat.color.empty() && !mat.shadow.empty())			// Layered Texture Diffuse map + Shadow Map
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
		out << "createNode bump2d -n \"" << mat.name << "bump2d\";\n";
	if (mat.Blend == Material::BLEND::SUBTRACT)
	{
		out << "createNode colorComposite -n \"" << mat.name << "colorComposite\";\n";
		out << "	setAttr \"._ca\" -type \"float3\" 0.7 0.7 0.7;\n";
		out << "	setAttr \"._op\" 1;\n";
	}
	MA.MA_Nodes << out.str();
	out.str("");

	// RELATIONSHIPS
	out << "relationship \"link\" \":lightLinker1\" \"" << mat.name << "SG.message\" \":defaultLightSet.message\";\n";
	out << "relationship \"shadowLink\" \":lightLinker1\" \"" << mat.name << "SG.message\" \":defaultLightSet.message\";\n";
	MA.MA_Relationships << out.str();
	out.str("");

	// CONNECTATTR
	if (!mat.color.empty() && mat.shadow.empty())				// Solo diffuse map
	{
		switch (mat.Blend)
		{
		case (Material::BLEND::NORMAL):
			out << "connectAttr \"" << mat.color << ".oc\" \"" << mat.name << ".c\";\n";
			break;
		case (Material::BLEND::SUBTRACT):
			out << "connectAttr \"" << mat.name << "colorComposite.oc\" \"" << mat.name << ".c\";\n";
			out << "connectAttr \"" << mat.name << "colorComposite.msg\" \"" << mat.name << "materialInfo.t\" -na;\n";
			out << "connectAttr \"" << mat.name << "colorComposite.msg\" \":defaultRenderUtilityList1.u\" -na;\n";
			out << "connectAttr \"" << mat.color << ".ot\" \"" << mat.name << ".it\";\n";
			out << "connectAttr \"" << mat.color << ".oc\" \"" << mat.name << "colorComposite._cb\";\n";
			break;
		case (Material::BLEND::OVERLAY):
			out << "connectAttr \"" << mat.color << ".oc\" \"" << mat.name << ".c\";\n";
			out << "connectAttr \"" << mat.color << ".oc\" \"" << mat.name << ".ic\";\n";
			out << "connectAttr \"" << mat.color << ".ot\" \"" << mat.name << ".it\";\n";
			break;
		}
		out << "connectAttr \"" << mat.color << ".msg\" \"" << mat.name << "materialInfo.t\" -na;\n";
	}


	if (mat.color.empty() && !mat.shadow.empty())				// Solo shadow map
	{
		switch (mat.Blend)
		{
		case (Material::BLEND::NORMAL):
			out << "connectAttr \"" << mat.shadow << ".oc\" \"" << mat.name << ".c\";\n";
			break;
		case (Material::BLEND::SUBTRACT):
			out << "connectAttr \"" << mat.name << "colorComposite.oc\" \"" << mat.name << ".c\";\n";
			out << "connectAttr \"" << mat.name << "colorComposite.msg\" \"" << mat.name << "materialInfo.t\" -na;\n";
			out << "connectAttr \"" << mat.name << "colorComposite.msg\" \":defaultRenderUtilityList1.u\" -na;\n";
			out << "connectAttr \"" << mat.shadow << ".ot\" \"" << mat.name << ".it\";\n";
			out << "connectAttr \"" << mat.shadow << ".oc\" \"" << mat.name << "colorComposite._cb\";\n";
			break;
		case (Material::BLEND::OVERLAY):
			out << "connectAttr \"" << mat.shadow << ".oc\" \"" << mat.name << ".c\";\n";
			out << "connectAttr \"" << mat.shadow << ".oc\" \"" << mat.name << ".ic\";\n";
			out << "connectAttr \"" << mat.shadow << ".ot\" \"" << mat.name << ".it\";\n";
			break;
		}
		out << "connectAttr \"" << mat.shadow << ".msg\" \"" << mat.name << "materialInfo.t\" -na;\n";
	}


	if (!mat.color.empty() && !mat.shadow.empty())			// Layered texture Diffuse map + Shadow map
	{
		switch (mat.Blend)
		{
		case (Material::BLEND::NORMAL):
			out << "connectAttr \"" << mat.name << "layeredTexture.oc\" \"" << mat.name << ".c\";\n";
			break;
		case (Material::BLEND::SUBTRACT):
			out << "connectAttr \"" << mat.name << "colorComposite.oc\" \"" << mat.name << ".c\";\n";
			out << "connectAttr \"" << mat.name << "colorComposite.msg\" \"" << mat.name << "materialInfo.t\" -na;\n";
			out << "connectAttr \"" << mat.name << "colorComposite.msg\" \":defaultRenderUtilityList1.u\" -na;\n";
			out << "connectAttr \"" << mat.name << "layeredTexture.ot\" \"" << mat.name << ".it\";\n";
			out << "connectAttr \"" << mat.name << "layeredTexture.oc\" \"" << mat.name << "colorComposite._cb\";\n";
			break;
		case (Material::BLEND::OVERLAY):
			out << "connectAttr \"" << mat.name << "layeredTexture.oc\" \"" << mat.name << ".c\";\n";
			out << "connectAttr \"" << mat.name << "layeredTexture.oc\" \"" << mat.name << ".ic\";\n";
			out << "connectAttr \"" << mat.name << "layeredTexture.ot\" \"" << mat.name << ".it\";\n";
			break;
		}
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


	if (!mat.transparency.empty())										// Trasparenza
		out << "connectAttr \"" << mat.transparency << ".ot\" \"" << mat.name << ".it\";\n";


	if (!mat.emissive.empty())											// Glow
		out << "connectAttr \"" << mat.emissive << ".oa\" \"" << mat.name << ".gi\";\n";


	if (!mat.reflection.empty())										// Envelope
		out << "connectAttr \"" << mat.reflection << ".oa\" \"" << mat.name << ".rfl\";\n";


	if (mat.Special == Material::SPECIAL::GLASS)						// Aggiunta Incandescence e Transparency per Glass
	{
		out << "connectAttr \"" << mat.color << ".oc\" \"" << mat.name << ".ic\";\n";
		out << "connectAttr \"" << mat.reflection << ".ot\" \"" << mat.name << ".it\";\n";
	}

	out << "connectAttr \"" << mat.name << ".oc\" \"" << mat.name << "SG.ss\";\n";
	out << "connectAttr \"" << mat.name << "SG.msg\" \"" << mat.name << "materialInfo.sg\";\n";
	out << "connectAttr \"" << mat.name << "SG.pa\" \":renderPartition.st\" -na;\n";
	out << "connectAttr \"" << mat.name << ".msg\" \"" << mat.name << "materialInfo.m\";\n";
	out << "connectAttr \"" << mat.name << ".msg\" \":defaultShaderList1.s\" -na;\n";
	MA.MA_Connections << out.str();
}