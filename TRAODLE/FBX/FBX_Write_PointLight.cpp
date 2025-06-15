/*------------------------------------------------------------------------------------------------------------------
Scrittura Point Light
INPUT: Light input
OUTPUT: FBX_EXPORT &FBX
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "FBX/FBX_Classes.h"


void FBX_Write_PointLight (Light input, FBX_EXPORT &FBX)
{
	// Aggiunta Point Light al computo dei contatori FBX
	FBX.FBX_Count.Model++;
	FBX.FBX_Count.NodeAttribute++;

	// Scrittura Properties
	stringstream out;
	out << "	Model: " << hashID(input.name, "Light") << ", \"Model::" << input.name << "\", \"Light\" {\n";
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
  	out << "	NodeAttribute: " << hashID(input.name, "Light_NodeAttribute") << ", \"NodeAttribute::\", \"Light\" {\n";
	out << "		Properties70:  {\n";
	out << "			P: \"Color\", \"Color\", \"\", \"A\"," << input.R << "," << input.G << "," << input.B << "\n";
	out << "			P: \"Intensity\", \"Number\", \"\", \"A\"," << input.Intensity << "\n";
	out << "			P: \"DecayType\", \"enum\", \"\", \"\",1\n";
	out << "			P: \"CastShadows\", \"bool\", \"\", \"\",1\n";
	if (input.Decay_Near_Start >= 0 || input.Decay_Near_End >= 0)
		out << "			P: \"EnableNearAttenuation\", \"bool\", \"\", \"\",1\n";
	if (input.Decay_Near_Start >= 0)
		out << "			P: \"NearAttenuationStart\", \"Number\", \"\", \"A+\"," << input.Decay_Near_Start << "\n";
	if (input.Decay_Near_End >= 0)
		out << "			P: \"NearAttenuationEnd\", \"Number\", \"\", \"A+\"," << input.Decay_Near_End << "\n";
	if (input.Decay_Far_Start >= 0 || input.Decay_Far_End >= 0)
		out << "			P: \"EnableFarAttenuation\", \"bool\", \"\", \"\",1\n";
	if (input.Decay_Far_Start >= 0)
		out << "			P: \"FarAttenuationStart\", \"Number\", \"\", \"A+\"," << input.Decay_Far_Start << "\n";
	if (input.Decay_Far_End >= 0)
		out << "			P: \"FarAttenuationEnd\", \"Number\", \"\", \"A+\"," << input.Decay_Far_End << "\n";
	out << "		}\n";
	out << "		TypeFlags: \"Light\"\n";
	out << "		GeometryVersion: 124\n";
	out << "	}\n";
	FBX.FBX_Properties << out.str();			// Copia il testo nello stringstream Properties definitivo

	out.str("");		// Pulisce lo stringstream

	// Scrittura Connections
	out << "	;Model::" << input.name << ", Model::";
	if (input.FBX_parent == "0")
		out << "RootNode\n";
	else
		out << input.parent << "\n";
	out << "	C: \"OO\"," << hashID(input.name, "Light") << "," << input.FBX_parent << "\n\n";
	out << "	;NodeAttribute::, Model::" << input.name << "\n";
	out << "	C: \"OO\"," << hashID(input.name, "Light_NodeAttribute") << "," << hashID(input.name, "Light") << "\n\n";
	FBX.FBX_Connections << out.str();
}