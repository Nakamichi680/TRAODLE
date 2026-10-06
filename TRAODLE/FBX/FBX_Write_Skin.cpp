/*------------------------------------------------------------------------------------------------------------------
Scrittura dello skinning di una mesh (Deformer Skin + un SubDeformer Cluster per ogni joint) e della BindPose.
La mesh deve essere in posa di bind con trasformazione globale identita'. Le matrici di bind dei joints sono lette
da Joint.BindMatrix.
INPUT: Mesh input
OUTPUT: FBX_EXPORT &FBX
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "FBX/FBX_Classes.h"
#include "MATH/math.h"


namespace {

const Joint *FindJoint (const FBX_EXPORT &FBX, const string &name)
{
	for (unsigned int j = 0; j < FBX.Joint.size(); j++)
		if (FBX.Joint[j].name == name)
			return &FBX.Joint[j];
	return nullptr;
}


MATRIX ToMatrix (const float m[16])
{
	MATRIX out;
	out.m00 = m[0];		out.m01 = m[1];		out.m02 = m[2];		out.m03 = m[3];
	out.m10 = m[4];		out.m11 = m[5];		out.m12 = m[6];		out.m13 = m[7];
	out.m20 = m[8];		out.m21 = m[9];		out.m22 = m[10];	out.m23 = m[11];
	out.m30 = m[12];	out.m31 = m[13];	out.m32 = m[14];	out.m33 = m[15];
	return out;
}


string MatrixText (const MATRIX &m)				// Matrice in formato FBX (per righe, traslazione negli elementi 12-14)
{
	stringstream out;
	out << setprecision(9);
	out << m.m00 << "," << m.m01 << "," << m.m02 << "," << m.m03 << "," << m.m10 << "," << m.m11 << "," << m.m12 << "," << m.m13 << ",";
	out << m.m20 << "," << m.m21 << "," << m.m22 << "," << m.m23 << "," << m.m30 << "," << m.m31 << "," << m.m32 << "," << m.m33;
	return out.str();
}

}		// namespace


void FBX_Write_Skin (const Mesh &input, FBX_EXPORT &FBX)
{
	stringstream out, conn;
	out << setprecision(9);
	string skin_id = hashID(input.name, "Skin");

	FBX.FBX_Count.Deformer++;
	out << "	Deformer: " << skin_id << ", \"Deformer::" << input.name << "\", \"Skin\" {\n";
	out << "		Version: 101\n";
	out << "		Link_DeformAcuracy: 50\n";
	out << "	}\n";
	conn << "	;Deformer::" << input.name << ", Geometry::\n";
	conn << "	C: \"OO\"," << skin_id << "," << hashID(input.name, "Geometry") << "\n\n";

	for (unsigned int c = 0; c < input.Skin.size(); c++)
	{
		const SkinCluster &cluster = input.Skin[c];
		const Joint *joint = FindJoint(FBX, cluster.joint);
		if (!joint || !joint->bindmatrix_flag)
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << input.name << ": joint " << cluster.joint << " not found. Skin cluster skipped.";
			continue;
		}
		MATRIX bind = ToMatrix(joint->BindMatrix);
		string cluster_id = hashID(input.name + "|" + cluster.joint, "Cluster");
		FBX.FBX_Count.Deformer++;
		out << "	Deformer: " << cluster_id << ", \"SubDeformer::\", \"Cluster\" {\n";
		out << "		Version: 100\n";
		out << "		UserData: \"\", \"\"\n";
		out << "		Indexes: *" << cluster.Vertex.size() << " {\n";
		out << "			a: ";
		for (unsigned int v = 0; v < cluster.Vertex.size(); v++)
			out << (v ? "," : "") << ((v && v % 30 == 0) ? "\n" : "") << cluster.Vertex[v];
		out << "\n		}\n";
		out << "		Weights: *" << cluster.Weight.size() << " {\n";
		out << "			a: ";
		for (unsigned int v = 0; v < cluster.Weight.size(); v++)
			out << (v ? "," : "") << ((v && v % 30 == 0) ? "\n" : "") << cluster.Weight[v];
		out << "\n		}\n";
		out << "		Transform: *16 {\n";						// Inversa della matrice globale del joint per la matrice globale della mesh (identita')
		out << "			a: " << MatrixText(mathMatrixInverse(bind)) << "\n";
		out << "		}\n";
		out << "		TransformLink: *16 {\n";					// Matrice globale del joint in posa di bind
		out << "			a: " << MatrixText(bind) << "\n";
		out << "		}\n";
		out << "	}\n";
		conn << "	;SubDeformer::, Deformer::" << input.name << "\n";
		conn << "	C: \"OO\"," << cluster_id << "," << skin_id << "\n\n";
		conn << "	;Model::" << cluster.joint << ", SubDeformer::\n";
		conn << "	C: \"OO\"," << hashID(cluster.joint, "Joint") << "," << cluster_id << "\n\n";
	}
	FBX.FBX_Properties << out.str();
	FBX.FBX_Connections << conn.str();
}


/*------------------------------------------------------------------------------------------------------------------
Scrittura della BindPose: matrici globali delle mesh deformate (identita') e dei joints nella posa di bind
------------------------------------------------------------------------------------------------------------------*/
void FBX_Write_SkinBindPose (FBX_EXPORT &FBX)
{
	vector <pair <string, MATRIX>> nodes;
	for (unsigned int g = 0; g < FBX.Geometry.size(); g++)
		if (!FBX.Geometry[g].Skin.empty())
			nodes.push_back(make_pair(hashID(FBX.Geometry[g].name, "Mesh"), MATRIX()));
	for (unsigned int j = 0; j < FBX.Joint.size(); j++)
		if (FBX.Joint[j].bindmatrix_flag)
			nodes.push_back(make_pair(hashID(FBX.Joint[j].name, "Joint"), ToMatrix(FBX.Joint[j].BindMatrix)));
	if (nodes.empty())
		return;

	FBX.FBX_Count.Pose++;
	stringstream out;
	out << "	Pose: " << hashID("BindPose", "Pose") << ", \"Pose::BIND_POSES\", \"BindPose\" {\n";
	out << "		Type: \"BindPose\"\n";
	out << "		Version: 100\n";
	out << "		NbPoseNodes: " << nodes.size() << "\n";
	for (unsigned int n = 0; n < nodes.size(); n++)
	{
		out << "		PoseNode:  {\n";
		out << "			Node: " << nodes[n].first << "\n";
		out << "			Matrix: *16 {\n";
		out << "				a: " << MatrixText(nodes[n].second) << "\n";
		out << "			}\n";
		out << "		}\n";
	}
	out << "	}\n";
	FBX.FBX_Properties << out.str();
}
