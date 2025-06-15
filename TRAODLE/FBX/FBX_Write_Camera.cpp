#include "stdafx.h"
#include "FBX/FBX_Classes.h"


void FBX_Write_Camera (Camera input, FBX_EXPORT &FBX)
{
	// Aggiunta telecamera al computo dei contatori FBX
	FBX.FBX_Count.Model++;
	FBX.FBX_Count.NodeAttribute++;

	// Scrittura Properties
	stringstream out;
	out << "	Model: " << hashID(input.name, "Camera") << ", \"Model::" << input.name << "\", \"Camera\" {\n";
	out << "		Properties70:  {\n";
	out << "			P: \"PostRotation\", \"Vector3D\", \"Vector\", \"\",0,-90,0\n";
	out << "			P: \"DefaultAttributeIndex\", \"int\", \"Integer\", \"\",0\n";
	if (input.translate_flag)
		out << "			P: \"Lcl Translation\", \"Lcl Translation\", \"\", \"A\"," << input.tX << "," << input.tY << "," << input.tZ << "\n";
	if (input.rotate_flag)
		out << "			P: \"Lcl Rotation\", \"Lcl Rotation\", \"\", \"A\"," << input.rX << "," << input.rY << "," << input.rZ << "\n";
	if (input.scale_flag)
		out << "			P: \"Lcl Scaling\", \"Lcl Scaling\", \"\", \"A\"," << input.sX << "," << input.sY << "," << input.sZ << "\n";
	out << "		}\n";
	out << "	}\n";
	out << "	NodeAttribute: " << hashID(input.name, "Camera_NodeAttribute") << ", \"NodeAttribute::" << input.name << "\", \"Camera\" {\n";
	out << "	}\n";
	FBX.FBX_Properties << out.str();			// Copia il testo nello stringstream Properties definitivo

	out.str("");		// Pulisce lo stringstream

	// Scrittura Connections
	out << "	;Model::" << input.name << ", Model::";
	if (input.FBX_parent == "0")
		out << "RootNode\n";
	else
		out << input.parent << "\n";
	out << "	C: \"OO\"," << hashID(input.name, "Camera") << "," << input.FBX_parent << "\n\n";
	out << "	;NodeAttribute::" << input.name << ", Model::" << input.name << "\n";
	out << "	C: \"OO\"," << hashID(input.name, "Camera_NodeAttribute") << "," << hashID(input.name, "Camera") << "\n\n";
	FBX.FBX_Connections << out.str();
}