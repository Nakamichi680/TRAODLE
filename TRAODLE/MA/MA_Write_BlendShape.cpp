/*------------------------------------------------------------------------------------------------------------------
Scrittura dei blend shapes di una mesh deformata: nodo blendShape con gli offset dei target salvati nel nodo stesso
(inputTargetItem[6000].inputPointsTarget / inputComponentsTarget, senza mesh target separate) e un peso per target
con il nome del target come alias. Catena dei deformer: <mesh>ShapeOrig -> blendShape -> skinCluster -> <mesh>Shape
(vedi MA_Write_SkinCluster). Struttura classica dei deformer di Maya: groupId, groupParts e objectSet.
INPUT: Mesh mesh
OUTPUT: MA_EXPORT &MA
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "MA/MA_Classes.h"


void MA_Write_BlendShape (const Mesh &mesh, MA_EXPORT &MA)
{
	string blend = mesh.name + "_blendShape";
	string group_id = mesh.name + "_blendGroupId";
	string group_parts = mesh.name + "_blendGroupParts";
	string blend_set = mesh.name + "_blendSet";
	unsigned int nTargets = mesh.BlendShape.size();

	stringstream out;
	out << setprecision(9);
	out << "createNode blendShape -n \"" << blend << "\";\n";
	out << "	addAttr -ci true -h true -sn \"aal\" -ln \"attributeAliasList\" -dt \"attributeAlias\";\n";
	out << "	setAttr -s " << nTargets << " \".w[0:" << nTargets - 1 << "]\"";
	for (unsigned int t = 0; t < nTargets; t++)
		out << " 0";
	out << ";\n";
	for (unsigned int t = 0; t < nTargets; t++)
	{
		const BlendShapeTarget &target = mesh.BlendShape[t];
		vector <unsigned int> indexes;							// Vertici spostati dal target
		for (unsigned int v = 0; v < target.dX.size(); v++)
			if (target.dX[v] != 0 || target.dY[v] != 0 || target.dZ[v] != 0)
				indexes.push_back(v);
		if (indexes.empty())
			continue;
		out << "	setAttr \".it[0].itg[" << t << "].iti[6000].ipt\" -type \"pointArray\" " << indexes.size();
		for (unsigned int i = 0; i < indexes.size(); i++)
			out << " " << target.dX[indexes[i]] << " " << target.dY[indexes[i]] << " " << target.dZ[indexes[i]] << " 1";
		out << ";\n";
		out << "	setAttr \".it[0].itg[" << t << "].iti[6000].ict\" -type \"componentList\" " << indexes.size();
		for (unsigned int i = 0; i < indexes.size(); i++)
			out << " \"vtx[" << indexes[i] << "]\"";
		out << ";\n";
	}
	out << "	setAttr \".aal\" -type \"attributeAlias\" {";
	for (unsigned int t = 0; t < nTargets; t++)
		out << (t ? "," : "") << "\"" << mesh.BlendShape[t].name << "\",\"weight[" << t << "]\"";
	out << "};\n";
	out << "createNode groupId -n \"" << group_id << "\";\n";
	out << "	setAttr \".ihi\" 0;\n";
	out << "createNode groupParts -n \"" << group_parts << "\";\n";
	out << "	setAttr \".ihi\" 0;\n";
	out << "	setAttr \".ic\" -type \"componentList\" 1 \"vtx[*]\";\n";
	out << "createNode objectSet -n \"" << blend_set << "\";\n";
	out << "	setAttr \".ihi\" 0;\n";
	out << "	setAttr \".vo\" yes;\n";
	MA.MA_Nodes << out.str();

	out.str("");
	out << "connectAttr \"" << mesh.name << "ShapeOrig.w\" \"" << group_parts << ".ig\";\n";
	out << "connectAttr \"" << group_id << ".id\" \"" << group_parts << ".gi\";\n";
	out << "connectAttr \"" << group_parts << ".og\" \"" << blend << ".ip[0].ig\";\n";
	out << "connectAttr \"" << group_id << ".id\" \"" << blend << ".ip[0].gi\";\n";
	out << "connectAttr \"" << group_id << ".id\" \"" << mesh.name << "Shape.iog.og[1].gid\";\n";
	out << "connectAttr \"" << blend_set << ".mwc\" \"" << mesh.name << "Shape.iog.og[1].gco\";\n";
	out << "connectAttr \"" << group_id << ".msg\" \"" << blend_set << ".gn\" -na;\n";
	out << "connectAttr \"" << mesh.name << "Shape.iog.og[1]\" \"" << blend_set << ".dsm\" -na;\n";
	out << "connectAttr \"" << blend << ".msg\" \"" << blend_set << ".ub[0]\";\n";
	MA.MA_Connections << out.str();
}
