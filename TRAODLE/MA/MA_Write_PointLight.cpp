#include "stdafx.h"
#include "MA/MA_Classes.h"


void MA_Write_PointLight (unsigned int l, MA_EXPORT &MA)
{
	stringstream out;
	out << "createNode transform -n \"" << MA.Light[l].name << "\"";
	if (MA.Light[l].parent.size() > 0)
		out << " -p \"" << MA.Light[l].parent << "\"";
	out << ";\n";
	if (MA.Light[l].translate_flag)
		out << "	setAttr \".t\" -type \"double3\" " << MA.Light[l].tX << " " << MA.Light[l].tY << " " << MA.Light[l].tZ << " ;\n";
	if (MA.Light[l].rotate_flag)
		out << "	setAttr \".r\" -type \"double3\" " << MA.Light[l].rX << " " << MA.Light[l].rY << " " << MA.Light[l].rZ << " ;\n";
	if (MA.Light[l].scale_flag)
		out << "	setAttr \".s\" -type \"double3\" " << MA.Light[l].sX << " " << MA.Light[l].sY << " " << MA.Light[l].sZ << " ;\n";
	out << "createNode pointLight -n \"" << MA.Light[l].name << "Shape\" -p \"" << MA.Light[l].name << "\";\n";
	out << "	setAttr -k off \".v\";\n";
	out << "	setAttr \".cl\" -type \"float3\"" << MA.Light[l].R << " " << MA.Light[l].G << " " << MA.Light[l].B << " ;\n";		// Colore
	out << "	setAttr \".in\" " << MA.Light[l].Intensity << ";\n";																// Intensità
	out << "	setAttr \".ai_samples\" 2;\n";																// Regola la qualità delle ombre
	out << "	setAttr \".ai_specular\" 0;\n";
	out << "	setAttr \".ai_sss\" 0;\n";
	out << "	setAttr \".ai_indirect\" 1;\n";																// Luce indiretta (espande la zona illuminata)
	out << "	setAttr \".ai_volume\" 0;\n";
	out << "	setAttr \".ai_radius\" 20;\n";																// Raggio della luce (per soft shadows). Eliminare per hard shadows
	out << "createNode aiLightDecay -n \"" << MA.Light[l].name << "Decay\";\n";
	out << "	setAttr \".use_near_atten\" yes;\n";
	out << "	setAttr \".use_far_atten\" yes;\n";
	if (MA.Light[l].Decay_Near_Start != 0)
		out << "	setAttr \".near_start\" " << MA.Light[l].Decay_Near_Start << ";\n";
	if (MA.Light[l].Decay_Near_End != 0)
		out << "	setAttr \".near_end\" " << MA.Light[l].Decay_Near_End << ";\n";
	if (MA.Light[l].Decay_Far_Start != 0)
		out << "	setAttr \".far_start\" " << MA.Light[l].Decay_Far_Start << ";\n";
	if (MA.Light[l].Decay_Far_End != 0)
		out << "	setAttr \".far_end\" " << MA.Light[l].Decay_Far_End << ";\n";
	
	MA.MA_Nodes << out.str();

	out.str("");
	out << "connectAttr \"" << MA.Light[l].name << "Decay.msg\" \"" << MA.Light[l].name << "Shape.ai_filters[0]\";\n";
	out << "connectAttr \"" << MA.Light[l].name << "Decay.msg\" \":defaultRenderUtilityList1.u\" -na;\n";
	out << "connectAttr \"" << MA.Light[l].name << ".iog\" \":defaultLightSet.dsm\" -na;\n";
	if (MA.Light[l].layer.size() > 0)
		out << "connectAttr \"" << MA.Light[l].layer << ".di\" \"" << MA.Light[l].name << ".do\";\n";

	MA.MA_Connections << out.str();
}