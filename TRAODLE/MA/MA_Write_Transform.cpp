#include "stdafx.h"
#include "MA/MA_Classes.h"


void MA_Write_Transform (unsigned int t, MA_EXPORT &MA)
{
	stringstream out;

	out << "createNode transform -n \"" << MA.Transform[t].name << "\"";
	if (MA.Transform[t].parent.size() > 0)
		out << " -p \"" << MA.Transform[t].parent << "\"";
	out << ";\n";
	if (MA.Transform[t].translate_flag)
		out << "	setAttr \".t\" -type \"double3\" " << MA.Transform[t].tX << " " << MA.Transform[t].tY << " " << MA.Transform[t].tZ << ";\n";
	if (MA.Transform[t].rotate_flag)
		out << "	setAttr \".r\" -type \"double3\" " << MA.Transform[t].rX << " " << MA.Transform[t].rY << " " << MA.Transform[t].rZ << ";\n";
	if (MA.Transform[t].scale_flag)
		out << "	setAttr \".s\" -type \"double3\" " << MA.Transform[t].sX << " " << MA.Transform[t].sY << " " << MA.Transform[t].sZ << ";\n";
	MA.MA_Nodes << out.str();
}