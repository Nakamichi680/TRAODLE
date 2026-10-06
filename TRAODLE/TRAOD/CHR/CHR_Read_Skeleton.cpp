#include "stdafx.h"
#include <set>
#include "MATH/math.h"
#include "TRAOD/CHR/CHR_Functions.h"


/*------------------------------------------------------------------------------------------------------------------
Legge lo scheletro del file CHR, ricostruisce la gerarchia e calcola le matrici globali in posa di bind.
Le bones vengono esportate come joints (gerarchia, trasformazione locale e matrice di bind per lo skinning) nei file
FBX e MA. Il joint radice e' figlio del gruppo del personaggio (chrname).
------------------------------------------------------------------------------------------------------------------*/
bool CHR_Read_Skeleton (ifstream &chrfile, const CHR_HEADER &chr_header, string chrname, CHR_Skeleton &skeleton, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	vector <CHR_BONE> bones(chr_header.nBONES);
	chrfile.seekg(chr_header.SKELETON_PTR);
	for (unsigned int b = 0; b < chr_header.nBONES; b++)
		chrfile.read(reinterpret_cast<char*>(&bones[b]), sizeof(CHR_BONE));
	if (!chrfile)
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Error reading the skeleton.";
		return false;
	}
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Number of bones: " << chr_header.nBONES;

	// Ricostruzione della gerarchia: i 2 bit bassi di Hierarchy sono operazioni su uno stack di bones (vedi CHR_Struct.h)
	skeleton.parent.assign(chr_header.nBONES, -1);
	vector <int> stack;
	int last = 0;
	for (unsigned int b = 1; b < chr_header.nBONES; b++)
	{
		switch (bones[b].Hierarchy & CHR_HIER_STACK_MASK)
		{
		case 0:												// Figlia della bone precedente
			skeleton.parent[b] = last;
			break;
		case 1:												// Figlia della bone precedente, che diventa punto di ramificazione
			skeleton.parent[b] = last;
			stack.push_back(last);
			break;
		case 2:												// Figlia del punto di ramificazione corrente, che viene chiuso
			skeleton.parent[b] = stack.empty() ? 0 : stack.back();
			if (!stack.empty())
				stack.pop_back();
			break;
		case 3:												// Figlia del punto di ramificazione corrente, che resta aperto
			skeleton.parent[b] = stack.empty() ? 0 : stack.back();
			break;
		}
		last = b;
	}

	// Matrici locali e globali (convenzione a vettore riga: globale = locale * globale del padre)
	skeleton.local.resize(chr_header.nBONES);
	skeleton.global.resize(chr_header.nBONES);
	for (unsigned int b = 0; b < chr_header.nBONES; b++)
	{
		const float *m = bones[b].LocalMatrix;
		MATRIX &l = skeleton.local[b];
		l.m00 = m[0];	l.m01 = m[1];	l.m02 = m[2];	l.m03 = m[3];
		l.m10 = m[4];	l.m11 = m[5];	l.m12 = m[6];	l.m13 = m[7];
		l.m20 = m[8];	l.m21 = m[9];	l.m22 = m[10];	l.m23 = m[11];
		l.m30 = m[12];	l.m31 = m[13];	l.m32 = m[14];	l.m33 = m[15];
		skeleton.global[b] = (skeleton.parent[b] < 0) ? l : mathMulMatrices(l, skeleton.global[skeleton.parent[b]]);
	}

	// Raggio dei joints proporzionato alle dimensioni dello scheletro
	float minZ = 0, maxZ = 0;
	for (unsigned int b = 0; b < chr_header.nBONES; b++)
	{
		minZ = min(minZ, skeleton.global[b].m32);
		maxZ = max(maxZ, skeleton.global[b].m32);
	}
	float radius = max((maxZ - minZ) / 100, 0.5f);

	// Creazione joints. I nomi sono resi unici aggiungendo un suffisso numerico in caso di nomi ripetuti
	set <string> used_names;
	skeleton.joint_name.resize(chr_header.nBONES);
	for (unsigned int b = 0; b < chr_header.nBONES; b++)
	{
		string bone_name(bones[b].Bone_name, strnlen(bones[b].Bone_name, sizeof(bones[b].Bone_name)));
		string name = chrname + "_" + bone_name;
		if (used_names.count(name))
			name += "_" + to_string(b);
		used_names.insert(name);
		skeleton.joint_name[b] = name;
	}
	for (unsigned int b = 0; b < chr_header.nBONES; b++)
	{
		Joint joint;
		joint.name = skeleton.joint_name[b];
		if (skeleton.parent[b] < 0)
		{
			joint.parent = chrname;
			joint.FBX_parent = hashID(chrname, "Group");
		}
		else
		{
			joint.parent = skeleton.joint_name[skeleton.parent[b]];
			joint.FBX_parent = hashID(joint.parent, "Joint");
		}
		Vec3 t, r, s;
		mathMatrixDecompose(skeleton.local[b], &t, &r, &s);
		joint.translate_flag = joint.rotate_flag = joint.scale_flag = true;
		joint.tX = t.x;		joint.tY = t.y;		joint.tZ = t.z;
		joint.rX = r.x;		joint.rY = r.y;		joint.rZ = r.z;
		joint.sX = s.x;		joint.sY = s.y;		joint.sZ = s.z;
		joint.Radius = radius;
		const MATRIX &g = skeleton.global[b];
		float bind[16] = {g.m00, g.m01, g.m02, g.m03, g.m10, g.m11, g.m12, g.m13, g.m20, g.m21, g.m22, g.m23, g.m30, g.m31, g.m32, g.m33};
		joint.bindmatrix_flag = true;
		memcpy(joint.BindMatrix, bind, sizeof(bind));
		FBX.Joint.push_back(joint);
		MA.Joint.push_back(joint);
	}
	return true;
}
