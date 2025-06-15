#include "stdafx.h"
#include "MA/MA_Classes.h"
//#include "MA_Functions.h"

void MA_Write_Camera (unsigned int c, MA_EXPORT &MA)
{
	stringstream out;
	out << "createNode transform -n \"" << MA.Camera[c].name << "\";\n";
	out << "	setAttr \".s\" -type \"double3\" 120 120 120;\n";
	out << "createNode camera -n \"" << MA.Camera[c].name << "Shape\" -p \"" << MA.Camera[c].name << "\";\n";
	out << "	setAttr -k off \".v\";\n";
	out << "	setAttr \".rnd\" no;\n";
	out << "	setAttr \".cap\" -type \"double2\" " << MA.Camera[c].hfa << " " << MA.Camera[c].vfa << ";\n";
	out << "	setAttr \".fl\" " << MA.Camera[c].fl << ";\n";
	out << "	setAttr \".coi\" " << MA.Camera[c].coi << ";\n";
	out << "	setAttr \".ncp\" " << MA.Camera[c].ncp << ";\n";
	out << "	setAttr \".fcp\" " << MA.Camera[c].fcp << ";\n";
	out << "	setAttr \".ff\" 0;\n";
	out << "	setAttr \".ow\" 30;\n";
	out << "	setAttr \".imn\" -type \"string\" \"" << MA.Camera[c].name << "\";\n";
	out << "	setAttr \".den\" -type \"string\" \"" << MA.Camera[c].name << "_depth\";\n";
	out << "	setAttr \".man\" -type \"string\" \"" << MA.Camera[c].name << "_mask\";\n";

	MA.MA_Nodes << out.str();
	
	out.str("");

	if (MA.Camera[c].layer.size() > 0)
		out << "connectAttr \"" << MA.Camera[c].layer << ".di\" \"" << MA.Camera[c].name << ".do\";\n";

	MA.MA_Connections << out.str();
}