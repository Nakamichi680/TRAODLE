#include "stdafx.h"
#include "MA/MA_Classes.h"
#include "MA/MA_Functions.h"

void MA_Export (string output_filename, MA_EXPORT &MA)
{
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Output filename: " << output_filename << ".MA";
	unsigned int a = 0;

	msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing header";
	MA_Write_Header(MA);
	msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing default nodes";
	MA_Write_DefaultNodes(MA);

	if (MA.Transform.size() > 0)
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing transforms";
	for (unsigned int t = 0; t < MA.Transform.size(); t++)
		MA_Write_Transform(t, MA);

	if (MA.Light.size() > 0)
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing lights";
	for (unsigned int l = 0; l < MA.Light.size(); l++)
		MA_Write_PointLight(l, MA);

	if (MA.Camera.size() > 0)
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing cameras";
	for (unsigned int c = 0; c < MA.Camera.size(); c++)
		MA_Write_Camera(c, MA);
	
	if (MA.Locator.size() > 0)
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing locators";
	for (unsigned int l = 0; l < MA.Locator.size(); l++)
		MA_Write_Locator(l, MA);

	if (MA.Animation.size() > 0)
	{
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing animations";
		MA_Write_Animation(a, MA);
	}

	if (MA.Mesh.size() > 0)
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing meshes";
	for (unsigned int m = 0; m < MA.Mesh.size(); m++)
		MA_Write_Mesh(MA.Mesh[m], MA);

	if (MA.NurbsSurface.size() > 0)
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing NURBS surfaces";
	for (unsigned int n = 0; n < MA.NurbsSurface.size(); n++)
		MA_Write_NurbsSurface(n, MA);

	if (MA.PolyPlane.size() > 0)
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing PolyPlane surfaces";
	for (unsigned int p = 0; p < MA.PolyPlane.size(); p++)
		MA_Write_PolyPlane(p, MA);

	if (MA.BossWave.size() > 0)
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing Boss waves";
	for (unsigned int b = 0; b < MA.BossWave.size(); b++)
		MA_Write_BossWave(b, MA);

	if (MA.Material.size() > 0)
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing materials";
	for (unsigned int m = 0; m < MA.Material.size(); m++)
		MA_Write_Material(m, MA);

	if (MA.Texture.size() > 0)
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing textures";
	for (unsigned int t = 0; t < MA.Texture.size(); t++)
		MA_Write_Texture(t, MA);

	if (MA.Joint.size() > 0)
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing joints";
	for (unsigned int j = 0; j < MA.Joint.size(); j++)
		MA_Write_Joint(j, MA);

	if (MA.Layer.size() > 0)
		msg(msg::TGT::FILE, msg::TYP::LOG) << "Writing layers";
	for (unsigned int l = 0; l < MA.Layer.size(); l++)			// Questo può andare per ultimo
		MA_Write_DisplayLayer(l, MA);
	


	ofstream out;
	output_filename.append(".MA");
	out.open(output_filename);
	out << MA.MA_Header.str();
	out << MA.MA_Nodes.str();
	out << MA.MA_Select.str();
	out << MA.MA_Connections.str();
	out << MA.MA_Relationships.str();
	out.close();
}