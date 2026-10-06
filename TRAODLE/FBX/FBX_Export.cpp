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

	for (unsigned int g = 0; g < FBX.Geometry.size(); g++)			// Scrittura blend shapes
		if (!FBX.Geometry[g].BlendShape.empty())
			FBX_Write_BlendShape(FBX.Geometry[g], FBX);

	if (FBX.BlendShapeAnimation.size() > 0)							// Animazioni dei blend shapes (una take per animazione)
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing blend shape animations";
	for (unsigned int a = 0; a < FBX.BlendShapeAnimation.size(); a++)
		FBX_Write_BlendShapeAnimation(FBX.BlendShapeAnimation[a], FBX);
	set <string> animated;											// Gruppi di canali animati in almeno una take
	for (const SkeletalAnimation &anim : FBX.SkeletalAnimation)
		for (const NodeAnimation &node : anim.nodes)
		{
			if (!node.tX.empty())
				animated.insert(node.node + "|T");
			if (!node.rX.empty())
				animated.insert(node.node + "|R");
			if (!node.sX.empty())
				animated.insert(node.node + "|S");
		}
	for (unsigned int a = 0; a < FBX.SkeletalAnimation.size(); a++)			// Animazioni scheletriche (una take per animazione)
		FBX_Write_SkeletalAnimation(FBX.SkeletalAnimation[a], animated, FBX);

	bool skinned = false;
	for (unsigned int g = 0; g < FBX.Geometry.size(); g++)			// Scrittura skinning delle mesh deformate da uno scheletro
		if (!FBX.Geometry[g].Skin.empty())
		{
			FBX_Write_Skin(FBX.Geometry[g], FBX);
			skinned = true;
		}
	if (skinned)
	{
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing skin deformers and bind pose";
		FBX_Write_SkinBindPose(FBX);
	}

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
	out << FBX_Write_Takes(FBX);
	out.close();

}