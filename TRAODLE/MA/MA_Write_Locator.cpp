#include "stdafx.h"
#include "MA/MA_Classes.h"


void MA_Write_Locator (unsigned int l, MA_EXPORT &MA)
{
	stringstream out;
	out << "createNode transform -n \"" << MA.Locator[l].name << "\"";
	if (MA.Locator[l].parent.size() > 0)
		out << " -p \"" << MA.Locator[l].parent << "\"";
	out << ";\n";
	//out << "	setAttr \".dla\" yes;\n";			// Mostra gli assi
	out << "	setAttr \".dh\" yes;\n";			// Mostra la croce
	if (MA.Locator[l].translate_flag)
		out << "	setAttr \".t\" -type \"double3\" " << MA.Locator[l].tX << " " << MA.Locator[l].tY << " " << MA.Locator[l].tZ << ";\n";
	if (MA.Locator[l].rotate_flag)
		out << "	setAttr \".r\" -type \"double3\" " << MA.Locator[l].rX << " " << MA.Locator[l].rY << " " << MA.Locator[l].rZ << ";\n";
	if (MA.Locator[l].scale_flag)
		out << "	setAttr \".s\" -type \"double3\" " << MA.Locator[l].sX << " " << MA.Locator[l].sY << " " << MA.Locator[l].sZ << ";\n";	
	out << "createNode locator -n \"" << MA.Locator[l].name << "Shape\" -p \"" << MA.Locator[l].name << "\";\n";
	out << "	setAttr -k off \".v\";\n";
	MA.MA_Nodes << out.str();
	
	out.str("");

	if (MA.Locator[l].layer.size() > 0)
		out << "connectAttr \"" << MA.Locator[l].layer << ".di\" \"" << MA.Locator[l].name << ".do\";\n";

	MA.MA_Connections << out.str();
}