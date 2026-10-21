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
	out << "		Properties70:  {\n";											// Ottica come nei file MA (pollici, mm)
	out << "			P: \"FilmWidth\", \"double\", \"Number\", \"\"," << input.hfa << "\n";
	out << "			P: \"FilmHeight\", \"double\", \"Number\", \"\"," << input.vfa << "\n";
	out << "			P: \"ApertureMode\", \"enum\", \"\", \"\",3\n";				// Ottica definita dalla lunghezza focale (altrimenti FocalLength viene ignorata)
	out << "			P: \"GateFit\", \"enum\", \"\", \"\",2\n";
	out << "			P: \"FocalLength\", \"Number\", \"\", \"A+\"," << input.fl << "\n";
	out << "			P: \"NearPlane\", \"double\", \"Number\", \"\"," << input.ncp << "\n";
	out << "			P: \"FarPlane\", \"double\", \"Number\", \"\"," << input.fcp << "\n";
	out << "		}\n";
	out << "		TypeFlags: \"Camera\"\n";
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