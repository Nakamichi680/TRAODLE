#include "stdafx.h"
#include "MA/MA_Classes.h"

void MA_Write_NurbsSurface (unsigned int n, MA_EXPORT &MA)
{
	stringstream out;
	out << "createNode transform -n \"" << MA.NurbsSurface[n].name << "\"";
	if (MA.NurbsSurface[n].parent.size() > 0)
		out << " -p \"" << MA.NurbsSurface[n].parent << "\"";
	out << ";\n";
	if (MA.NurbsSurface[n].translate_flag)
		out << "	setAttr \".t\" -type \"double3\" " << MA.NurbsSurface[n].tX << " " << MA.NurbsSurface[n].tY << " " << MA.NurbsSurface[n].tZ << " ;\n";
	if (MA.NurbsSurface[n].rotate_flag)
		out << "	setAttr \".r\" -type \"double3\" " << MA.NurbsSurface[n].rX << " " << MA.NurbsSurface[n].rY << " " << MA.NurbsSurface[n].rZ << " ;\n";
	if (MA.NurbsSurface[n].scale_flag)
		out << "	setAttr \".s\" -type \"double3\" " << MA.NurbsSurface[n].sX << " " << MA.NurbsSurface[n].sY << " " << MA.NurbsSurface[n].sZ << " ;\n";	
	out << "createNode nurbsSurface -n \"" << MA.NurbsSurface[n].name << "Shape\" -p \"" << MA.NurbsSurface[n].name << "\";\n";
	out << "	setAttr -k off \".v\";\n";
	out << "	setAttr \".vir\" yes;\n";
	out << "	setAttr \".vif\" yes;\n";
	out << "	setAttr \".covm[0]\" 0 1 1;\n";
	out << "	setAttr \".cdvm[0]\" 0 1 1;\n";
	out << "	setAttr \".dvu\" 0;\n";
	out << "	setAttr \".dvv\" 0;\n";
	out << "	setAttr \".cpr\" 4;\n";
	out << "	setAttr \".cps\" 4;\n";
	out << "	setAttr \".nufa\" 4.5;\n";
	out << "	setAttr \".nvfa\" 4.5;\n";

	if (MA.NurbsSurface[n].Type == "Sphere")
	{
		out << "createNode makeNurbSphere -n \"makeNurbSphere_" << MA.NurbsSurface[n].name << "\";\n";
		out << "	setAttr \".ax\" -type \"double3\" 0 0 1 ;\n";
		out << "	setAttr \".r\" " << MA.NurbsSurface[n].Radius << ";\n";
		out << "	setAttr \".s\" " << MA.NurbsSurface[n].Sections << ";\n";
		out << "	setAttr \".nsp\" " << MA.NurbsSurface[n].Spans << ";\n";
	}

	MA.MA_Nodes << out.str();
	
	out.str("");

	if (MA.NurbsSurface[n].Type == "Sphere")
		out << "connectAttr \"makeNurbSphere_" << MA.NurbsSurface[n].name << ".os\" \"" << MA.NurbsSurface[n].name << "Shape.cr\";\n";

	out << "connectAttr \"" << MA.NurbsSurface[n].name << "Shape.iog\" \":initialShadingGroup.dsm\" -na;\n";
	if (MA.NurbsSurface[n].layer.size() > 0)
		out << "connectAttr \"" << MA.NurbsSurface[n].layer << ".di\" \"" << MA.NurbsSurface[n].name << ".do\";\n";

	MA.MA_Connections << out.str();
}