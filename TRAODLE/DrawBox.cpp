/*------------------------------------------------------------------------------------------------------------------
Dati i due vertici diagonalmente opposti, restituisce la mesh di un parallelepipedo
INPUT: 
OUTPUT: Mesh output
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Classes.h"
#include "FBX/FBX_Classes.h"


Mesh DrawBox (string name, string parent, string layer, XYZ Vmin, XYZ Vmax, unsigned int VC_ARGB)
{
	Mesh output;
	output.name = name;
	output.parent = parent;
	output.layer = layer;
	output.FBX_parent = hashID(parent, "Group");
	output.nV = 24;
	output.uv_set1_flag = false;
	output.uv_set2_flag = false;
	output.normals_flag = false;
	output.tangents_flag = false;
	output.binormals_flag = false;
	float A = (float)(0xFF & (VC_ARGB >> 24)) / 255;
	float R = (float)(0xFF & (VC_ARGB >> 16)) / 255;
	float G = (float)(0xFF & (VC_ARGB >> 8)) / 255;
	float B = (float)(0xFF & VC_ARGB) / 255;	

	output.X.push_back(Vmin.x);			output.Y.push_back(Vmin.y);			output.Z.push_back(Vmin.z);			// Vertice 0
	output.X.push_back(Vmax.x);			output.Y.push_back(Vmin.y);			output.Z.push_back(Vmin.z);			// Vertice 1
	output.X.push_back(Vmax.x);			output.Y.push_back(Vmin.y);			output.Z.push_back(Vmax.z);			// Vertice 2
	output.X.push_back(Vmin.x);			output.Y.push_back(Vmin.y);			output.Z.push_back(Vmax.z);			// Vertice 3
	output.X.push_back(Vmin.x);			output.Y.push_back(Vmin.y);			output.Z.push_back(Vmin.z);			// Vertice 4
	output.X.push_back(Vmax.x);			output.Y.push_back(Vmin.y);			output.Z.push_back(Vmin.z);			// Vertice 5
	output.X.push_back(Vmax.x);			output.Y.push_back(Vmax.y);			output.Z.push_back(Vmin.z);			// Vertice 6
	output.X.push_back(Vmin.x);			output.Y.push_back(Vmax.y);			output.Z.push_back(Vmin.z);			// Vertice 7
	output.X.push_back(Vmax.x);			output.Y.push_back(Vmin.y);			output.Z.push_back(Vmin.z);			// Vertice 8
	output.X.push_back(Vmax.x);			output.Y.push_back(Vmax.y);			output.Z.push_back(Vmin.z);			// Vertice 9
	output.X.push_back(Vmax.x);			output.Y.push_back(Vmax.y);			output.Z.push_back(Vmax.z);			// Vertice 10
	output.X.push_back(Vmax.x);			output.Y.push_back(Vmin.y);			output.Z.push_back(Vmax.z);			// Vertice 11
	output.X.push_back(Vmax.x);			output.Y.push_back(Vmin.y);			output.Z.push_back(Vmax.z);			// Vertice 12
	output.X.push_back(Vmax.x);			output.Y.push_back(Vmax.y);			output.Z.push_back(Vmax.z);			// Vertice 13
	output.X.push_back(Vmin.x);			output.Y.push_back(Vmax.y);			output.Z.push_back(Vmax.z);			// Vertice 14
	output.X.push_back(Vmin.x);			output.Y.push_back(Vmin.y);			output.Z.push_back(Vmax.z);			// Vertice 15
	output.X.push_back(Vmin.x);			output.Y.push_back(Vmin.y);			output.Z.push_back(Vmax.z);			// Vertice 16
	output.X.push_back(Vmin.x);			output.Y.push_back(Vmin.y);			output.Z.push_back(Vmin.z);			// Vertice 17
	output.X.push_back(Vmin.x);			output.Y.push_back(Vmax.y);			output.Z.push_back(Vmin.z);			// Vertice 18
	output.X.push_back(Vmin.x);			output.Y.push_back(Vmax.y);			output.Z.push_back(Vmax.z);			// Vertice 19
	output.X.push_back(Vmin.x);			output.Y.push_back(Vmax.y);			output.Z.push_back(Vmax.z);			// Vertice 20
	output.X.push_back(Vmax.x);			output.Y.push_back(Vmax.y);			output.Z.push_back(Vmax.z);			// Vertice 21
	output.X.push_back(Vmax.x);			output.Y.push_back(Vmax.y);			output.Z.push_back(Vmin.z);			// Vertice 22
	output.X.push_back(Vmin.x);			output.Y.push_back(Vmax.y);			output.Z.push_back(Vmin.z);			// Vertice 23

	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 0
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 1
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 2
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 3
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 4
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 5
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 6
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 7
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 8
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 9
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 10
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 11
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 12
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 13
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 14
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 15
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 16
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 17
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 18
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 19
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 20
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 21
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 22
	output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);			// Vertice 23

	Face f1, f2, f3, f4, f5, f6;
	f1.TrisOrQuads = 4;		f1.v1 = 0;		f1.v2 = 1;		f1.v3 = 2;		f1.v4 = 3;		output.Face.push_back(f1);
	f2.TrisOrQuads = 4;		f2.v1 = 5;		f2.v2 = 4;		f2.v3 = 7;		f2.v4 = 6;		output.Face.push_back(f2);
	f3.TrisOrQuads = 4;		f3.v1 = 11;		f3.v2 = 8;		f3.v3 = 9;		f3.v4 = 10;		output.Face.push_back(f3);
	f4.TrisOrQuads = 4;		f4.v1 = 14;		f4.v2 = 13;		f4.v3 = 22;		f4.v4 = 23;		output.Face.push_back(f4);
	f5.TrisOrQuads = 4;		f5.v1 = 16;		f5.v2 = 19;		f5.v3 = 18;		f5.v4 = 17;		output.Face.push_back(f5);
	f6.TrisOrQuads = 4;		f6.v1 = 21;		f6.v2 = 20;		f6.v3 = 15;		f6.v4 = 12;		output.Face.push_back(f6);
	return output;
}