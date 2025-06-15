#include "stdafx.h"
#include "FBX/FBX_Classes.h"
#include "FBX/FBX_Functions.h"

void FBX_Export (string output_filename, FBX_EXPORT &FBX)
{
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Output filename: " << output_filename << ".FBX";
	FBX.Clear();
	FBX.Init();
	msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing header";
	FBX_Write_Header(FBX);
	
	if (FBX.Group.size() > 0)
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing groups";
	for (unsigned int g = 0; g < FBX.Group.size(); g++)				// Scrittura gruppi transform
		FBX_Write_Group(FBX.Group[g], FBX);

	if (FBX.Locator.size() > 0)
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing locators";
	for (unsigned int l = 0; l < FBX.Locator.size(); l++)			// Scrittura locator
		FBX_Write_Locator(FBX.Locator[l], FBX);

	if (FBX.Geometry.size() > 0)
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing geometries";
	for (unsigned int g = 0; g < FBX.Geometry.size(); g++)			// Scrittura geometria (DA CONTROLLARE)
		FBX_Write_Geometry(FBX.Geometry[g], FBX);

	if (FBX.Camera.size() > 0)
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing cameras";
	for (unsigned int c = 0; c < FBX.Camera.size(); c++)			// Scrittura telecamere
		FBX_Write_Camera(FBX.Camera[c], FBX);

	if (FBX.Joint.size() > 0)
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing joints";
	for (unsigned int j = 0; j < FBX.Joint.size(); j++)				// Scrittura joints
		FBX_Write_Joint(FBX.Joint[j], FBX);

	/*for (unsigned int l = 0; l < FBX.Light.size(); l++)				// Scrittura luci (WIP!!!!!!!!!!!!!!)
	{
		if (FBX.Light[l].type == "Point")
			FBX_Write_PointLight(FBX.Light[l], FBX);
	}*/



	//if (FBX.Animation.size() > 0)									// DA RICONTROLLARE
		//FBX_Write_Animation(a, FBX);
		


	msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing object definitions";
	FBX_Write_Object_definitions(FBX);		// QUESTO VA PER ULTIMO (O COMUNQUE DOPO PROPERTIES)!!!!
	FBX.Close();

	// Scrittura degli stringstream nel file definitivo
	ofstream out;
	output_filename.append(".FBX");
	out.open(output_filename);
	out << FBX.FBX_Header.str();
	out << FBX.FBX_Definitions.str();
	out << FBX.FBX_Properties.str();
	out << FBX.FBX_Connections.str();
	out.close();

}