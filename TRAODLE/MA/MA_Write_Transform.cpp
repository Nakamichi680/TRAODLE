#include "stdafx.h"
#include "MA/MA_Classes.h"
#include "MA/MA_Functions.h"


/*------------------------------------------------------------------------------------------------------------------
Attributi personalizzati (Extra Attributes) del nodo corrente: interi e numerici bloccati, testo modificabile.
------------------------------------------------------------------------------------------------------------------*/
void MA_Write_Attributes (stringstream &out, const vector <pair <string, int>> &ints, const vector <pair <string, float>> &floats, const vector <pair <string, string>> &strings)
{
	for (const pair <string, int> &a : ints)
	{
		out << "	addAttr -ci true -sn \"" << a.first << "\" -ln \"" << a.first << "\" -at \"long\";\n";
		out << "	setAttr -l on \"." << a.first << "\" " << a.second << ";\n";
	}
	for (const pair <string, float> &a : floats)
	{
		out << "	addAttr -ci true -sn \"" << a.first << "\" -ln \"" << a.first << "\" -at \"double\";\n";
		out << "	setAttr -l on \"." << a.first << "\" " << a.second << ";\n";
	}
	for (const pair <string, string> &a : strings)
	{
		out << "	addAttr -ci true -sn \"" << a.first << "\" -ln \"" << a.first << "\" -dt \"string\";\n";
		out << "	setAttr \"." << a.first << "\" -type \"string\" \"" << a.second << "\";\n";
	}
}


void MA_Write_Transform (unsigned int t, MA_EXPORT &MA)
{
	stringstream out;
	out << setprecision(7);
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
	if (MA.Transform[t].pivot_flag)												// Pivot, traslazioni dei pivot e rotate axis
	{
		const pair <const char*, const Vec3*> pivots[] = {{"rp", &MA.Transform[t].rotate_pivot}, {"rpt", &MA.Transform[t].rotate_pivot_translate},
			{"sp", &MA.Transform[t].scale_pivot}, {"spt", &MA.Transform[t].scale_pivot_translate}, {"ra", &MA.Transform[t].rotate_axis}};
		for (const auto &p : pivots)
			if (p.second->x != 0 || p.second->y != 0 || p.second->z != 0)
				out << "	setAttr \"." << p.first << "\" -type \"double3\" " << p.second->x << " " << p.second->y << " " << p.second->z << ";\n";
	}
	MA_Write_Attributes(out, MA.Transform[t].int_attributes, MA.Transform[t].float_attributes, MA.Transform[t].string_attributes);	// Extra Attributes
	MA.MA_Nodes << out.str();

	out.str("");

	if (MA.Transform[t].layer.size() > 0)
		out << "connectAttr \"" << MA.Transform[t].layer << ".di\" \"" << MA.Transform[t].name << ".do\";\n";

	MA.MA_Connections << out.str();
}