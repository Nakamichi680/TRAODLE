#include "stdafx.h"
#include "MA/MA_Classes.h"


void MA_Write_PolyCube (unsigned int p, MA_EXPORT &MA)
{
	stringstream out;
	out << setprecision(7);
	out << "createNode transform -n \"" << MA.PolyCube[p].name << "\"";
	if (MA.PolyCube[p].parent.size() > 0)
		out << " -p \"" << MA.PolyCube[p].parent << "\"";
	out << ";\n";
	if (!MA.PolyCube[p].Visible)						// Nasconde l'oggetto se Visible è su false
		out << "	setAttr \".v\" no;\n";
	if (MA.PolyCube[p].translate_flag)
		out << "	setAttr \".t\" -type \"double3\" " << MA.PolyCube[p].tX << " " << MA.PolyCube[p].tY << " " << MA.PolyCube[p].tZ << ";\n";
	if (MA.PolyCube[p].rotate_flag)
		out << "	setAttr \".r\" -type \"double3\" " << MA.PolyCube[p].rX << " " << MA.PolyCube[p].rY << " " << MA.PolyCube[p].rZ << ";\n";
	if (MA.PolyCube[p].scale_flag)
		out << "	setAttr \".s\" -type \"double3\" " << MA.PolyCube[p].sX << " " << MA.PolyCube[p].sY << " " << MA.PolyCube[p].sZ << ";\n";
	out << "createNode mesh -n \"" << MA.PolyCube[p].name << "Shape\" -p \"" << MA.PolyCube[p].name << "\";\n";
	out << "	setAttr -k off \".v\";\n";
	out << "	setAttr \".vir\" yes;\n";
	out << "	setAttr \".vif\" yes;\n";
	out << "	setAttr \".uvst[0].uvsn\" -type \"string\" \"UVChannel_1\";\n";
	out << "	setAttr \".cuvs\" -type \"string\" \"UVChannel_1\";\n";
	out << "	setAttr \".dcol\" yes;\n";
	out << "	setAttr \".dcc\" -type \"string\" \"Ambient+Diffuse\";\n";
	if (MA.PolyCube[p].VC_ARGB != 0xFF7F7F7F)
	{
		out << "	setAttr \".ccls\" -type \"string\" \"colorSet0\";\n";
		out << "	setAttr \".clst[0].clsn\" -type \"string\" \"colorSet0\";\n";
	}
	out << "	setAttr \".covm[0]\" 0 1 1;\n";
	out << "	setAttr \".cdvm[0]\" 0 1 1;\n";
	out << "createNode polyCube -n \"" << MA.PolyCube[p].name << "polyCube\";\n";
	out << "	setAttr \".cuv\" 4;\n";
	if (MA.PolyCube[p].VC_ARGB != 0xFF7F7F7F)
	{
		float A = (float)(0xFF & (MA.PolyCube[p].VC_ARGB >> 24)) / 255;
		float R = (float)(0xFF & (MA.PolyCube[p].VC_ARGB >> 16)) / 255;
		float G = (float)(0xFF & (MA.PolyCube[p].VC_ARGB >> 8)) / 255;
		float B = (float)(0xFF & MA.PolyCube[p].VC_ARGB) / 255;
		out << "createNode polyColorPerVertex -n \"" << MA.PolyCube[p].name << "polyColorPerVertex\";\n";
		out << "	setAttr \".uopa\" yes;\n";
		out << "	setAttr \".vclr[0].vrgb\" -type \"float3\" " << R << " " << G << " " << B << ";\n";
		out << "	setAttr \".vclr[0].vxal\" " << A << ";\n";
		out << "	setAttr \".vclr[1].vrgb\" -type \"float3\" " << R << " " << G << " " << B << ";\n";
		out << "	setAttr \".vclr[1].vxal\" " << A << ";\n";
		out << "	setAttr \".vclr[2].vrgb\" -type \"float3\" " << R << " " << G << " " << B << ";\n";
		out << "	setAttr \".vclr[2].vxal\" " << A << ";\n";
		out << "	setAttr \".vclr[3].vrgb\" -type \"float3\" " << R << " " << G << " " << B << ";\n";
		out << "	setAttr \".vclr[3].vxal\" " << A << ";\n";
		out << "	setAttr \".vclr[4].vrgb\" -type \"float3\" " << R << " " << G << " " << B << ";\n";
		out << "	setAttr \".vclr[4].vxal\" " << A << ";\n";
		out << "	setAttr \".vclr[5].vrgb\" -type \"float3\" " << R << " " << G << " " << B << ";\n";
		out << "	setAttr \".vclr[5].vxal\" " << A << ";\n";
		out << "	setAttr \".vclr[6].vrgb\" -type \"float3\" " << R << " " << G << " " << B << ";\n";
		out << "	setAttr \".vclr[6].vxal\" " << A << ";\n";
		out << "	setAttr \".vclr[7].vrgb\" -type \"float3\" " << R << " " << G << " " << B << ";\n";
		out << "	setAttr \".vclr[7].vxal\" " << A << ";\n";
		out << "	setAttr \".cn\" -type \"string\" \"colorSet1\";\n";
		out << "	setAttr \".clam\" no;\n";
	}

	MA.MA_Nodes << out.str();

	out.str("");

	// Scrittura vertex color
	if (MA.PolyCube[p].VC_ARGB == 0xFF7F7F7F)
		out << "connectAttr \"" << MA.PolyCube[p].name << "polyCube.out\" \"" << MA.PolyCube[p].name << "Shape.i\";\n";
	else
	{
		out << "connectAttr \"" << MA.PolyCube[p].name << "polyColorPerVertex.out\" \"" << MA.PolyCube[p].name << "Shape.i\";\n";
		out << "connectAttr \"" << MA.PolyCube[p].name << "polyCube.out\" \"" << MA.PolyCube[p].name << "polyColorPerVertex.ip\";\n";
	}

	// Scrittura materiale
	if (MA.PolyCube[p].material_name.size() > 0)
		out << "connectAttr \"" << MA.PolyCube[p].name << "Shape.iog\" \"" << MA.PolyCube[p].material_name << "SG.dsm\" -na;\n";
	else
		out << "connectAttr \"" << MA.PolyCube[p].name << "Shape.iog\" \":initialShadingGroup.dsm\" -na;\n";

	// Scrittura layer
	if (MA.PolyCube[p].layer.size() > 0)
		out << "connectAttr \"" << MA.PolyCube[p].layer << ".di\" \"" << MA.PolyCube[p].name << ".do\";\n";
	

	MA.MA_Connections << out.str();
}