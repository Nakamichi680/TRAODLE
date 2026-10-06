/*------------------------------------------------------------------------------------------------------------------
Scrittura dello skinning di una mesh: nodo skinCluster con i pesi dei vertici e le matrici di bind dei joints,
collegato alla shape intermedia "<mesh>ShapeOrig" (geometria in posa di bind, scritta da MA_Write_Mesh) e alla shape
di output "<mesh>Shape". Struttura classica dei deformer di Maya: groupId, groupParts e objectSet.
La mesh deve essere in posa di bind con trasformazione globale identita'.
INPUT: Mesh mesh
OUTPUT: MA_EXPORT &MA
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "MA/MA_Classes.h"
#include "MATH/math.h"


void MA_Write_SkinCluster (const Mesh &mesh, MA_EXPORT &MA)
{
	string skin = mesh.name + "_skinCluster";
	string group_id = mesh.name + "_skinGroupId";
	string group_parts = mesh.name + "_skinGroupParts";
	string skin_set = mesh.name + "_skinSet";

	// Influenze valide (joints presenti nella scena con matrice di bind)
	vector <const Joint*> joints;
	vector <unsigned int> clusters;
	for (unsigned int c = 0; c < mesh.Skin.size(); c++)
	{
		const Joint *joint = nullptr;
		for (unsigned int j = 0; j < MA.Joint.size() && !joint; j++)
			if (MA.Joint[j].name == mesh.Skin[c].joint && MA.Joint[j].bindmatrix_flag)
				joint = &MA.Joint[j];
		if (!joint)
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << mesh.name << ": joint " << mesh.Skin[c].joint << " not found. Skin influence skipped.";
			continue;
		}
		joints.push_back(joint);
		clusters.push_back(c);
	}

	// Pesi per vertice: (indice influenza, peso)
	vector <vector <pair <unsigned int, float>>> weights(mesh.nV);
	for (unsigned int i = 0; i < clusters.size(); i++)
	{
		const SkinCluster &cluster = mesh.Skin[clusters[i]];
		for (unsigned int k = 0; k < cluster.Vertex.size(); k++)
			if (cluster.Vertex[k] < mesh.nV)
				weights[cluster.Vertex[k]].push_back(make_pair(i, cluster.Weight[k]));
	}
	unsigned int max_influences = 1;
	for (unsigned int v = 0; v < mesh.nV; v++)
		max_influences = max(max_influences, (unsigned int)weights[v].size());

	stringstream out;
	out << setprecision(9);
	out << "createNode skinCluster -n \"" << skin << "\";\n";
	out << "	setAttr -s " << mesh.nV << " \".wl\";\n";
	for (unsigned int v = 0; v < mesh.nV; v++)
	{
		out << "	setAttr -s " << weights[v].size() << " \".wl[" << v << "].w\";\n";
		for (unsigned int k = 0; k < weights[v].size(); k++)
			out << "	setAttr \".wl[" << v << "].w[" << weights[v][k].first << "]\" " << weights[v][k].second << ";\n";
	}
	out << "	setAttr -s " << joints.size() << " \".pm\";\n";
	for (unsigned int i = 0; i < joints.size(); i++)			// Matrici di bind inverse dei joints
	{
		const float *b = joints[i]->BindMatrix;
		MATRIX bind;
		bind.m00 = b[0];	bind.m01 = b[1];	bind.m02 = b[2];	bind.m03 = b[3];
		bind.m10 = b[4];	bind.m11 = b[5];	bind.m12 = b[6];	bind.m13 = b[7];
		bind.m20 = b[8];	bind.m21 = b[9];	bind.m22 = b[10];	bind.m23 = b[11];
		bind.m30 = b[12];	bind.m31 = b[13];	bind.m32 = b[14];	bind.m33 = b[15];
		MATRIX inv = mathMatrixInverse(bind);
		out << "	setAttr \".pm[" << i << "]\" -type \"matrix\" " << inv.m00 << " " << inv.m01 << " " << inv.m02 << " " << inv.m03 << " "
			<< inv.m10 << " " << inv.m11 << " " << inv.m12 << " " << inv.m13 << " " << inv.m20 << " " << inv.m21 << " " << inv.m22 << " " << inv.m23 << " "
			<< inv.m30 << " " << inv.m31 << " " << inv.m32 << " " << inv.m33 << ";\n";
	}
	out << "	setAttr \".gm\" -type \"matrix\" 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1;\n";		// Matrice globale della mesh in posa di bind
	out << "	setAttr -s " << joints.size() << " \".ma\";\n";
	out << "	setAttr -s " << joints.size() << " \".dpf[0:" << joints.size() - 1 << "]\"";
	for (unsigned int i = 0; i < joints.size(); i++)
		out << " 4";
	out << ";\n";
	out << "	setAttr -s " << joints.size() << " \".lw\";\n";
	out << "	setAttr \".mmi\" yes;\n";
	out << "	setAttr \".mi\" " << max_influences << ";\n";
	out << "	setAttr \".ucm\" yes;\n";
	out << "createNode groupId -n \"" << group_id << "\";\n";
	out << "	setAttr \".ihi\" 0;\n";
	out << "createNode groupParts -n \"" << group_parts << "\";\n";
	out << "	setAttr \".ihi\" 0;\n";
	out << "	setAttr \".ic\" -type \"componentList\" 1 \"vtx[*]\";\n";
	out << "createNode objectSet -n \"" << skin_set << "\";\n";
	out << "	setAttr \".ihi\" 0;\n";
	out << "	setAttr \".vo\" yes;\n";
	MA.MA_Nodes << out.str();

	out.str("");
	if (mesh.BlendShape.empty())							// Geometria in ingresso: posa di bind oppure uscita del blendShape
		out << "connectAttr \"" << mesh.name << "ShapeOrig.w\" \"" << group_parts << ".ig\";\n";
	else
		out << "connectAttr \"" << mesh.name << "_blendShape.og[0]\" \"" << group_parts << ".ig\";\n";
	out << "connectAttr \"" << group_id << ".id\" \"" << group_parts << ".gi\";\n";
	out << "connectAttr \"" << group_parts << ".og\" \"" << skin << ".ip[0].ig\";\n";
	out << "connectAttr \"" << group_id << ".id\" \"" << skin << ".ip[0].gi\";\n";
	out << "connectAttr \"" << skin << ".og[0]\" \"" << mesh.name << "Shape.i\";\n";
	out << "connectAttr \"" << group_id << ".id\" \"" << mesh.name << "Shape.iog.og[0].gid\";\n";
	out << "connectAttr \"" << skin_set << ".mwc\" \"" << mesh.name << "Shape.iog.og[0].gco\";\n";
	out << "connectAttr \"" << group_id << ".msg\" \"" << skin_set << ".gn\" -na;\n";
	out << "connectAttr \"" << mesh.name << "Shape.iog.og[0]\" \"" << skin_set << ".dsm\" -na;\n";
	out << "connectAttr \"" << skin << ".msg\" \"" << skin_set << ".ub[0]\";\n";
	for (unsigned int i = 0; i < joints.size(); i++)
	{
		out << "connectAttr \"" << joints[i]->name << ".wm\" \"" << skin << ".ma[" << i << "]\";\n";
		out << "connectAttr \"" << joints[i]->name << ".obcc\" \"" << skin << ".ifcl[" << i << "]\";\n";
	}
	MA.MA_Connections << out.str();
}
