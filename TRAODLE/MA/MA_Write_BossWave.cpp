#include "stdafx.h"
#include "MA/MA_Classes.h"


void MA_Write_BossWave (unsigned int b, MA_EXPORT &MA)
{
	stringstream out;
	out << "createNode transform -n \"" << MA.BossWave[b].name << "\"";
	if (MA.BossWave[b].parent.size() > 0)
		out << " -p \"" << MA.BossWave[b].parent << "\"";
	out << ";\n";
	if (MA.BossWave[b].translate_flag)
		out << "	setAttr \".t\" -type \"double3\" " << MA.BossWave[b].tX << " " << MA.BossWave[b].tY << " " << MA.BossWave[b].tZ << ";\n";
	if (MA.BossWave[b].rotate_flag)
		out << "	setAttr \".r\" -type \"double3\" " << MA.BossWave[b].rX << " " << MA.BossWave[b].rY << " " << MA.BossWave[b].rZ << ";\n";
	if (MA.BossWave[b].scale_flag)
		out << "	setAttr \".s\" -type \"double3\" " << MA.BossWave[b].sX << " " << MA.BossWave[b].sY << " " << MA.BossWave[b].sZ << ";\n";
	if (MA.BossWave[b].rotate_pivot_flag)
		out << "	setAttr \".rp\" -type \"double3\" " << MA.BossWave[b].rpX << " " << MA.BossWave[b].rpY << " " << MA.BossWave[b].rpZ << ";\n";
	if (MA.BossWave[b].scale_pivot_flag)
		out << "	setAttr \".sp\" -type \"double3\" " << MA.BossWave[b].spX << " " << MA.BossWave[b].spY << " " << MA.BossWave[b].spZ << ";\n";
	out << "createNode mesh -n \"" << MA.BossWave[b].name << "Shape\" -p \"" << MA.BossWave[b].name << "\";\n";
	out << "	setAttr -k off \".v\";\n";
	out << "	setAttr \".vir\" yes;\n";
	out << "	setAttr \".vif\" yes;\n";
	out << "	setAttr \".uvst[0].uvsn\" -type \"string\" \"UVChannel_1\";\n";
	out << "	setAttr \".cuvs\" -type \"string\" \"UVChannel_1\";\n";
	out << "	setAttr \".dcc\" -type \"string\" \"Ambient+Diffuse\";\n";
	out << "	setAttr \".covm[0]\" 0 1 1;\n";
	out << "	setAttr \".cdvm[0]\" 0 1 1;\n";
	out << "createNode BossSpectralWave -n \"" << MA.BossWave[b].name << "BossSpectralWave\";\n";
	out << "	setAttr \".startFrame\" 1;\n";
	out << "	setAttr \".patchSizeX\" " << MA.BossWave[b].patchSizeX << ";\n";
	out << "	setAttr \".patchSizeZ\" " << MA.BossWave[b].patchSizeZ << ";\n";
	if (MA.BossWave[b].spaceScale != 1)
		out << "	setAttr \".spaceScale\" " << MA.BossWave[b].spaceScale << ";\n";
	out << "	setAttr \".dirSpectra\" 7;\n";
	out << "	setAttr \".seed\" " << b << ";\n";
	out << "	setAttr \".waveHeight\" " << MA.BossWave[b].waveHeight << ";\n";
	out << "	setAttr \".windSpeed\" " << MA.BossWave[b].windSpeed << ";\n";
	if (MA.BossWave[b].oceanDepth != 10000)
		out << "	setAttr \".oceanDepth\" " << MA.BossWave[b].oceanDepth << ";\n";
	out << "createNode BossBlender -n \"" << MA.BossWave[b].name << "BossBlender\";\n";

	MA.MA_Nodes << out.str();
	out.str("");

	out << "connectAttr \"" << MA.BossWave[b].name << "BossBlender.outMesh\" \"" << MA.BossWave[b].name << "Shape.i\";\n";
	out << "connectAttr \"" << MA.BossWave[b].inputmesh_name << "Shape.pm\" \"" << MA.BossWave[b].name << "BossSpectralWave.parentMatrix\";\n";
	out << "connectAttr \"" << MA.BossWave[b].inputmesh_name << "Shape.bb\" \"" << MA.BossWave[b].name << "BossSpectralWave.boundingBox\";\n";
	out << "connectAttr \":time1.o\" \"" << MA.BossWave[b].name << "BossSpectralWave.time\";\n";
	out << "connectAttr \"" << MA.BossWave[b].inputmesh_name << "Shape.w\" \"" << MA.BossWave[b].name << "BossBlender.inMesh\";\n";
	out << "connectAttr \":time1.o\" \"" << MA.BossWave[b].name << "BossBlender.time\";\n";
	out << "connectAttr \"" << MA.BossWave[b].name << "BossSpectralWave.outWave\" \"" << MA.BossWave[b].name << "BossBlender.inwave\" -na;\n";

	if (MA.BossWave[b].material_name.size() > 0)
		out << "connectAttr \"" << MA.BossWave[b].name << "Shape.iog\" \"" << MA.BossWave[b].material_name << "SG.dsm\" -na;\n";
	else
		out << "connectAttr \"" << MA.BossWave[b].name << "Shape.iog\" \":initialShadingGroup.dsm\" -na;\n";
	if (MA.BossWave[b].layer.size() > 0)
		out << "connectAttr \"" << MA.BossWave[b].layer << ".di\" \"" << MA.BossWave[b].name << ".do\";\n";

	MA.MA_Connections << out.str();
}