/*------------------------------------------------------------------------------------------------------------------
Scrittura Locator
INPUT: Locator input
OUTPUT: FBX_EXPORT &FBX
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "FBX/FBX_Classes.h"


void FBX_Write_Locator (Locator input, FBX_EXPORT &FBX)
{
	// Aggiunta gruppo al computo dei contatori FBX
	FBX.FBX_Count.Model++;
	FBX.FBX_Count.NodeAttribute++;

	// Scrittura Properties
	stringstream out;
	out << "	Model: " << hashID(input.name, "Locator") << ", \"Model::" << input.name << "\", \"Null\" {\n";
	out << "		Version: 232\n";
	out << "		Properties70:  {\n";
	out << "			P: \"RotationActive\", \"bool\", \"\", \"\",1\n";
	out << "			P: \"InheritType\", \"enum\", \"\", \"\",1\n";
	out << "			P: \"ScalingMax\", \"Vector3D\", \"Vector\", \"\",0,0,0\n";
	out << "			P: \"DefaultAttributeIndex\", \"int\", \"Integer\", \"\",0\n";
	if (input.translate_flag)
		out << "			P: \"Lcl Translation\", \"Lcl Translation\", \"\", \"A\"," << input.tX << "," << input.tY << "," << input.tZ << "\n";
	if (input.rotate_flag)
		out << "			P: \"Lcl Rotation\", \"Lcl Rotation\", \"\", \"A\"," << input.rX << "," << input.rY << "," << input.rZ << "\n";
	if (input.scale_flag)
		out << "			P: \"Lcl Scaling\", \"Lcl Scaling\", \"\", \"A\"," << input.sX << "," << input.sY << "," << input.sZ << "\n";
	out << "		}\n";
	out << "		Shading: Y\n";
	out << "		Culling: \"CullingOff\"\n";
	out << "	}\n";
	out << "	NodeAttribute: " <<  hashID(input.name, "Locator_NodeAttribute") << ", \"NodeAttribute::\", \"Null\" {\n";
	out << "		TypeFlags: \"Null\"\n";
	out << "	}\n";
	FBX.FBX_Properties << out.str();			// Copia il testo nello stringstream Properties definitivo

	out.str("");		// Pulisce lo stringstream

	// Scrittura Connections
	out << "	;Model::" << input.name << ", Model::";
	if (input.FBX_parent == "0")
		out << "RootNode\n";
	else
		out << input.parent << "\n";
	out << "	C: \"OO\"," << hashID(input.name, "Locator") << "," << input.FBX_parent << "\n\n";
	out << "	;NodeAttribute::, Model::" << input.name << "\n";
	out << "	C: \"OO\"," << hashID(input.name, "Locator_NodeAttribute") << "," << hashID(input.name, "Locator") << "\n\n";
	FBX.FBX_Connections << out.str();
}