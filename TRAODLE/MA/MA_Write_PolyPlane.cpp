#include "stdafx.h"
#include "MA/MA_Classes.h"


void MA_Write_PolyPlane (unsigned int p, MA_EXPORT &MA)
{
	stringstream out;
	out << "createNode transform -n \"" << MA.PolyPlane[p].name << "\"";
	if (MA.PolyPlane[p].parent.size() > 0)
		out << " -p \"" << MA.PolyPlane[p].parent << "\"";
	out << ";\n";
	if (!MA.PolyPlane[p].Visible)						// Nasconde l'oggetto se Visible è su false
		out << "	setAttr \".v\" no;\n";
	if (MA.PolyPlane[p].translate_flag)
		out << "	setAttr \".t\" -type \"double3\" " << MA.PolyPlane[p].tX << " " << MA.PolyPlane[p].tY << " " << MA.PolyPlane[p].tZ << ";\n";
	if (MA.PolyPlane[p].rotate_flag)
		out << "	setAttr \".r\" -type \"double3\" " << MA.PolyPlane[p].rX << " " << MA.PolyPlane[p].rY << " " << MA.PolyPlane[p].rZ << ";\n";
	if (MA.PolyPlane[p].scale_flag)
		out << "	setAttr \".s\" -type \"double3\" " << MA.PolyPlane[p].sX << " " << MA.PolyPlane[p].sY << " " << MA.PolyPlane[p].sZ << ";\n";
	if (MA.PolyPlane[p].rotate_pivot_flag)
		out << "	setAttr \".rp\" -type \"double3\" " << MA.PolyPlane[p].rpX << " " << MA.PolyPlane[p].rpY << " " << MA.PolyPlane[p].rpZ << ";\n";
	if (MA.PolyPlane[p].scale_pivot_flag)
		out << "	setAttr \".sp\" -type \"double3\" " << MA.PolyPlane[p].spX << " " << MA.PolyPlane[p].spY << " " << MA.PolyPlane[p].spZ << ";\n";
	out << "createNode mesh -n \"" << MA.PolyPlane[p].name << "Shape\" -p \"" << MA.PolyPlane[p].name << "\";\n";
	out << "	setAttr -k off \".v\";\n";
	out << "	setAttr \".vir\" yes;\n";
	out << "	setAttr \".vif\" yes;\n";
	out << "	setAttr \".uvst[0].uvsn\" -type \"string\" \"UVChannel_1\";\n";
	out << "	setAttr \".cuvs\" -type \"string\" \"UVChannel_1\";\n";
	out << "	setAttr \".dcc\" -type \"string\" \"Ambient+Diffuse\";\n";
	out << "	setAttr \".covm[0]\" 0 1 1;\n";
	out << "	setAttr \".cdvm[0]\" 0 1 1;\n";
	out << "createNode polyPlane -n \"" << MA.PolyPlane[p].name << "polyPlane\";\n";
	out << "	setAttr \".w\" " << MA.PolyPlane[p].Width << ";\n";
	out << "	setAttr \".h\" " << MA.PolyPlane[p].Height << ";\n";
	out << "	setAttr \".sw\" " << MA.PolyPlane[p].Subdiv_Width << ";\n";
	out << "	setAttr \".sh\" " << MA.PolyPlane[p].Subdiv_Height << ";\n";

	MA.MA_Nodes << out.str();
	
	out.str("");

	out << "connectAttr \"" << MA.PolyPlane[p].name << "polyPlane.out\" \"" << MA.PolyPlane[p].name << "Shape.i\";\n";
	if (MA.PolyPlane[p].material_name.size() > 0)
		out << "connectAttr \"" << MA.PolyPlane[p].name << "Shape.iog\" \"" << MA.PolyPlane[p].material_name << "SG.dsm\" -na;\n";
	else
		out << "connectAttr \"" << MA.PolyPlane[p].name << "Shape.iog\" \":initialShadingGroup.dsm\" -na;\n";
	if (MA.PolyPlane[p].layer.size() > 0)
		out << "connectAttr \"" << MA.PolyPlane[p].layer << ".di\" \"" << MA.PolyPlane[p].name << ".do\";\n";

	MA.MA_Connections << out.str();
}