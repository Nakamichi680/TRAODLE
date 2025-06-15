#include "stdafx.h"
#include "MA/MA_Classes.h"


void MA_Write_Texture (unsigned int t, MA_EXPORT &MA)
{
	replace(MA.Texture[t].filename.begin(), MA.Texture[t].filename.end(), '\\', '/');		// Sostituisce \ con /

	stringstream out;
	out << "createNode file -n \"" << MA.Texture[t].name << "\";\n";
	if (MA.Texture[t].AlphaIsLuminance)
		out << "	setAttr \".ail\" yes;\n";
	out << "	setAttr \".ftn\" -type \"string\" \"" << MA.Texture[t].filename << "\";\n";
	if (MA.Texture[t].Exposure != 0)
		out << "	setAttr \".exp\" " << MA.Texture[t].Exposure << ";\n";
	out << "	setAttr \".cs\" -type \"string\" \"sRGB\";\n";
	out << "createNode place2dTexture -n \"" << MA.Texture[t].name << "place2dTexture\";\n";
	if (MA.Texture[t].UV_mesh_list.size() > 0)
		out << "createNode uvChooser -n \"" << MA.Texture[t].name << "uvChooser\";\n";
	if (MA.Texture[t].UV_mesh_list.size() > 1)
		out << "	setAttr -s " << MA.Texture[t].UV_mesh_list.size() << " \".uvs\";\n";
	MA.MA_Nodes << out.str();
	out.str("");
	
	if (MA.Texture[t].UV_mesh_list.size() > 0)
	{
		out << "connectAttr \"" << MA.Texture[t].name << "uvChooser.ov1\" \"" << MA.Texture[t].name << "place2dTexture.vt1\";\n";
		out << "connectAttr \"" << MA.Texture[t].name << "uvChooser.ov2\" \"" << MA.Texture[t].name << "place2dTexture.vt2\";\n";
		out << "connectAttr \"" << MA.Texture[t].name << "uvChooser.ov3\" \"" << MA.Texture[t].name << "place2dTexture.vt3\";\n";
		out << "connectAttr \"" << MA.Texture[t].name << "uvChooser.oc1\" \"" << MA.Texture[t].name << "place2dTexture.vc1\";\n";
		out << "connectAttr \"" << MA.Texture[t].name << "uvChooser.ouv\" \"" << MA.Texture[t].name << "place2dTexture.uv\";\n";
		for (unsigned int m = 0; m < MA.Texture[t].UV_mesh_list.size(); m++)
			out << "connectAttr \"" << MA.Texture[t].UV_mesh_list[m].mesh_name << "Shape.uvst[" << MA.Texture[t].UV_mesh_list[m].UVset << "].uvsn\" \"" << MA.Texture[t].name << "uvChooser.uvs[" << m << "]\";\n";
	}
	out << "connectAttr \":defaultColorMgtGlobals.cme\" \"" << MA.Texture[t].name << ".cme\";\n";
	out << "connectAttr \":defaultColorMgtGlobals.cfe\" \"" << MA.Texture[t].name << ".cmcf\";\n";
	out << "connectAttr \":defaultColorMgtGlobals.cfp\" \"" << MA.Texture[t].name << ".cmcp\";\n";
	out << "connectAttr \":defaultColorMgtGlobals.wsn\" \"" << MA.Texture[t].name << ".ws\";\n";
	out << "connectAttr \"" << MA.Texture[t].name << "place2dTexture.c\" \"" << MA.Texture[t].name << ".c\";\n";			// place2dTexture::Coverage
	out << "connectAttr \"" << MA.Texture[t].name << "place2dTexture.tf\" \"" << MA.Texture[t].name << ".tf\";\n";			// place2dTexture::Translate Frame
	out << "connectAttr \"" << MA.Texture[t].name << "place2dTexture.rf\" \"" << MA.Texture[t].name << ".rf\";\n";			// place2dTexture::Rotate Frame
	out << "connectAttr \"" << MA.Texture[t].name << "place2dTexture.mu\" \"" << MA.Texture[t].name << ".mu\";\n";			// place2dTexture::Mirror U
	out << "connectAttr \"" << MA.Texture[t].name << "place2dTexture.mv\" \"" << MA.Texture[t].name << ".mv\";\n";			// place2dTexture::Mirror V
	out << "connectAttr \"" << MA.Texture[t].name << "place2dTexture.s\" \"" << MA.Texture[t].name << ".s\";\n";			// place2dTexture::Stagger
	out << "connectAttr \"" << MA.Texture[t].name << "place2dTexture.wu\" \"" << MA.Texture[t].name << ".wu\";\n";			// place2dTexture::Wrap U
	out << "connectAttr \"" << MA.Texture[t].name << "place2dTexture.wv\" \"" << MA.Texture[t].name << ".wv\";\n";			// place2dTexture::Wrap V
	out << "connectAttr \"" << MA.Texture[t].name << "place2dTexture.re\" \"" << MA.Texture[t].name << ".re\";\n";			// place2dTexture::Repeat UV
	out << "connectAttr \"" << MA.Texture[t].name << "place2dTexture.of\" \"" << MA.Texture[t].name << ".of\";\n";			// place2dTexture::Offset
	out << "connectAttr \"" << MA.Texture[t].name << "place2dTexture.r\" \"" << MA.Texture[t].name << ".ro\";\n";			// place2dTexture::Rotate UV
	out << "connectAttr \"" << MA.Texture[t].name << "place2dTexture.n\" \"" << MA.Texture[t].name << ".n\";\n";			// place2dTexture::Noise
	out << "connectAttr \"" << MA.Texture[t].name << "place2dTexture.vt1\" \"" << MA.Texture[t].name << ".vt1\";\n";		// place2dTexture::Vertex Uv One
	out << "connectAttr \"" << MA.Texture[t].name << "place2dTexture.vt2\" \"" << MA.Texture[t].name << ".vt2\";\n";		// place2dTexture::Vertex Uv Two
	out << "connectAttr \"" << MA.Texture[t].name << "place2dTexture.vt3\" \"" << MA.Texture[t].name << ".vt3\";\n";		// place2dTexture::Vertex Uv Three
	out << "connectAttr \"" << MA.Texture[t].name << "place2dTexture.vc1\" \"" << MA.Texture[t].name << ".vc1\";\n";		// place2dTexture::Vertex Camera One
	out << "connectAttr \"" << MA.Texture[t].name << "place2dTexture.o\" \"" << MA.Texture[t].name << ".uv\";\n";			// place2dTexture::Out UV
	out << "connectAttr \"" << MA.Texture[t].name << "place2dTexture.ofs\" \"" << MA.Texture[t].name << ".fs\";\n";			// place2dTexture::Out Uv Filter Size
	out << "connectAttr \"" << MA.Texture[t].name << "place2dTexture.msg\" \":defaultRenderUtilityList1.u\" -na;\n";
	out << "connectAttr \"" << MA.Texture[t].name << ".msg\" \":defaultTextureList1.tx\" -na;\n";
	MA.MA_Connections << out.str();
}