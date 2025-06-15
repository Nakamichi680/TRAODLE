#include "stdafx.h"
#include "MA/MA_Classes.h"


void MA_Write_DisplayLayer (unsigned int l, MA_EXPORT &MA)
{
	stringstream out;
	float R = (float)(0xFF & (MA.Layer[l].Label_ARGB >> 16)) / 255;
	float G = (float)(0xFF & (MA.Layer[l].Label_ARGB >> 8)) / 255;
	float B = (float)(0xFF & MA.Layer[l].Label_ARGB) / 255;

	out << "createNode displayLayer -n \"" << MA.Layer[l].name << "\";\n";
	if (!MA.Layer[l].Visible)
		out << "	setAttr \".v\" no;\n";
	if (MA.Layer[l].Type == LayerDisplayType::Template)
		out << "	setAttr \".dt\" 1;\n";
	if (MA.Layer[l].Type == LayerDisplayType::Reference)
		out << "	setAttr \".dt\" 2;\n";
	out << "	setAttr \".ovrgbf\" yes;\n";
	out << "	setAttr \".ovrgb\" -type \"float3\" " << R << " " << G << " " << B << ";\n";
	out << "	setAttr \".do\" " << l + 1 << ";\n";
	
	MA.MA_Nodes << out.str();

	out.str("");

	out << "connectAttr \"layerManager.dli[" << l + 1 << "]\" \"" << MA.Layer[l].name << ".id\";\n";

	MA.MA_Connections << out.str();
}
