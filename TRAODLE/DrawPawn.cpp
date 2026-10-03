/*------------------------------------------------------------------------------------------------------------------
Restituisce la mesh di una "pedina" che rappresenta la posizione e l'orientamento di un personaggio: un prisma
ottagonale verticale (base a Z = 0, alto "height") con un naso a piramide sul lato frontale.
Le coordinate sono locali: la mesh va parentata ad un oggetto che ne contiene posizione e rotazione.
INPUT:	forward = direzione frontale nel piano XY (vettore unitario)
OUTPUT: Mesh output
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Classes.h"
#include "FBX/FBX_Classes.h"


Mesh DrawPawn (string name, string parent, string layer, float height, float radius, float noseLength, Vec3 forward, unsigned int VC_ARGB)
{
	Mesh output;
	output.name = name;
	output.parent = parent;
	output.layer = layer;
	output.FBX_parent = hashID(parent, "Group");
	output.nV = 0;
	output.uv_set1_flag = false;
	output.uv_set2_flag = false;
	output.normals_flag = false;
	output.tangents_flag = false;
	output.binormals_flag = false;
	float A = (float)(0xFF & (VC_ARGB >> 24)) / 255;
	float R = (float)(0xFF & (VC_ARGB >> 16)) / 255;
	float G = (float)(0xFF & (VC_ARGB >> 8)) / 255;
	float B = (float)(0xFF & VC_ARGB) / 255;

	// Ogni faccia ha vertici propri (shading piatto, come DrawBox). Ordine dei vertici antiorario visto dall'esterno
	auto AddVertex = [&](const Vec3 &v)
	{
		output.X.push_back(v.x);	output.Y.push_back(v.y);	output.Z.push_back(v.z);
		output.A.push_back(A);		output.R.push_back(R);		output.G.push_back(G);		output.B.push_back(B);
		return (int)output.nV++;
	};
	auto AddTriangle = [&](const Vec3 &a, const Vec3 &b, const Vec3 &c)
	{
		Face f;
		f.TrisOrQuads = 3;
		f.v1 = AddVertex(a);	f.v2 = AddVertex(b);	f.v3 = AddVertex(c);
		output.Face.push_back(f);
	};
	auto AddQuad = [&](const Vec3 &a, const Vec3 &b, const Vec3 &c, const Vec3 &d)
	{
		Face f;
		f.TrisOrQuads = 4;
		f.v1 = AddVertex(a);	f.v2 = AddVertex(b);	f.v3 = AddVertex(c);	f.v4 = AddVertex(d);
		output.Face.push_back(f);
	};

	// Sistema di riferimento locale: f = avanti, s = sinistra (f ruotato di 90 gradi in senso antiorario), z = alto
	Vec3 f = forward;
	Vec3 s(-forward.y, forward.x, 0);
	Vec3 up(0, 0, 1);

	// Corpo: prisma ottagonale (un lato e' rivolto in avanti)
	const unsigned int sides = 8;
	const float pi = 3.14159265358979f;
	vector <Vec3> ring;
	for (unsigned int k = 0; k < sides; k++)
	{
		float angle = 2 * pi * k / sides + pi / sides;
		ring.push_back(f * (radius * cos(angle)) + s * (radius * sin(angle)));
	}
	Vec3 top = up * height;
	for (unsigned int k = 0; k < sides; k++)
	{
		const Vec3 &p0 = ring[k], &p1 = ring[(k + 1) % sides];
		AddQuad(p0, p1, p1 + top, p0 + top);				// Lato
		AddTriangle(top, p0 + top, p1 + top);				// Coperchio superiore
		AddTriangle(Vec3(), p1, p0);						// Base inferiore
	}

	// Naso: piramide a base quadrata che sporge dal lato frontale all'altezza del petto
	float w = radius * 0.35f;
	Vec3 C = f * (radius * 0.8f) + up * (height * 0.75f);
	Vec3 apex = f * (radius + noseLength) + up * (height * 0.75f);
	Vec3 TR = C + s * w + up * w, TL = C - s * w + up * w;		// Visti da davanti (osservatore rivolto verso -f): la destra dell'osservatore e' +s
	Vec3 BR = C + s * w - up * w, BL = C - s * w - up * w;
	AddTriangle(TR, TL, apex);								// Sopra
	AddTriangle(TL, BL, apex);								// Lato sinistro (visto da davanti)
	AddTriangle(BL, BR, apex);								// Sotto
	AddTriangle(BR, TR, apex);								// Lato destro
	return output;
}
