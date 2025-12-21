#include "stdafx.h"
#include "MA/MA_Classes.h"


void MA_Write_Material_Empty (unsigned int m, MA_EXPORT &MA)
{
	Material mat = MA.Material[m];
	stringstream out;
	out << setprecision(7);

	// NODI
	out << "createNode shadingEngine -n \"" << mat.name << "SG\";\n";
	out << "	setAttr \".ihi\" 0;\n";
	out << "	setAttr \".ro\" yes;\n";	
	MA.MA_Nodes << out.str();
	out.str("");

	// RELATIONSHIPS
	out << "relationship \"link\" \":lightLinker1\" \"" << mat.name << "SG.message\" \":defaultLightSet.message\";\n";
	out << "relationship \"shadowLink\" \":lightLinker1\" \"" << mat.name << "SG.message\" \":defaultLightSet.message\";\n";
	MA.MA_Relationships << out.str();
	out.str("");

	// CONNECTATTR
	out << "connectAttr \"" << mat.name << "SG.pa\" \":renderPartition.st\" -na;\n";
	MA.MA_Connections << out.str();
}