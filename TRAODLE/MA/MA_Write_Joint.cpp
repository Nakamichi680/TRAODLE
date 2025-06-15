#include "stdafx.h"
#include "MA/MA_Classes.h"


void MA_Write_Joint (unsigned int j, MA_EXPORT &MA)
{
	stringstream out;
	out << "createNode joint -n \"" << MA.Joint[j].name << "\"";
	if (MA.Joint[j].parent.size() > 0)
		out << " -p \"" << MA.Joint[j].parent << "\"";
	out << ";\n";
	if (MA.Joint[j].translate_flag)
		out << "	setAttr \".t\" -type \"double3\"" << MA.Joint[j].tX << " " << MA.Joint[j].tY << " " << MA.Joint[j].tZ << " ;\n";
	if (MA.Joint[j].rotate_flag)
		out << "	setAttr \".r\" -type \"double3\"" << MA.Joint[j].rX << " " << MA.Joint[j].rY << " " << MA.Joint[j].rZ << " ;\n";
	if (MA.Joint[j].scale_flag)
		out << "	setAttr \".s\" -type \"double3\"" << MA.Joint[j].sX << " " << MA.Joint[j].sY << " " << MA.Joint[j].sZ << " ;\n";
	if (!MA.Joint[j].InheritsTransform)
		out << "	setAttr \".it\" no;\n";
	if (MA.Joint[j].DrawLabel)
		out << "	setAttr \".dl\" yes;\n";
	if (MA.Joint[j].LabelSide != Joint::LABELSIDE::CENTER)
		out << "	setAttr \".sd\"" << int(MA.Joint[j].LabelSide) << ";\n";
	if (MA.Joint[j].LabelType != Joint::LABELTYPE::NONE)
		out << "	setAttr \".typ\"" << int(MA.Joint[j].LabelType) << ";\n";
	if (MA.Joint[j].LabelName.size() > 0)
		out << "	setAttr \".otp\" -type \"string\" \"" << MA.Joint[j].LabelName << "\";\n";
	out << "	setAttr \".radi\" " << MA.Joint[j].Radius << ";\n";
	MA.MA_Nodes << out.str();
	
	out.str("");

	if (MA.Joint[j].layer.size() > 0)
		out << "connectAttr \"" << MA.Joint[j].layer << ".di\" \"" << MA.Joint[j].name << ".do\";\n";
	if (MA.Joint[j].parent.size() > 0)
		out << "connectAttr \"" << MA.Joint[j].parent << ".s\" \"" << MA.Joint[j].name << ".is\";\n";
	MA.MA_Connections << out.str();
}