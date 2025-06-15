/*------------------------------------------------------------------------------------------------------------------
Dati tre vertici, restituisce la mesh di un triangolo
INPUT: 
OUTPUT: Mesh output
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Classes.h"
#include "FBX/FBX_Classes.h"


Mesh DrawTriangle (string name, string parent, string layer, XYZ v0, XYZ v1, XYZ v2, unsigned int VC_ARGB)
{
	Mesh output;
	output.name = name;
	output.parent = parent;
	output.layer = layer;
	output.FBX_parent = hashID(parent, "Group");
	output.nV = 3;
	output.uv_set1_flag = false;
	output.uv_set2_flag = false;
	output.normals_flag = false;
	output.tangents_flag = false;
	output.binormals_flag = false;
	float A = (float)(0xFF & (VC_ARGB >> 24)) / 255;
	float R = (float)(0xFF & (VC_ARGB >> 16)) / 255;
	float G = (float)(0xFF & (VC_ARGB >> 8)) / 255;
	float B = (float)(0xFF & VC_ARGB) / 255;

	output.X.push_back(v0.x);			output.Y.push_back(v0.y);			output.Z.push_back(v0.z);			// Vertice 0
	output.X.push_back(v1.x);			output.Y.push_back(v1.y);			output.Z.push_back(v1.z);			// Vertice 1
	output.X.push_back(v2.x);			output.Y.push_back(v2.y);			output.Z.push_back(v2.z);			// Vertice 2

	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 0
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 1
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 2

	Face f1;
	f1.TrisOrQuads = 3;		f1.v1 = 0;		f1.v2 = 1;		f1.v3 = 2;		output.Face.push_back(f1);
	return output;
}