#include "stdafx.h"
#include <map>
#include <set>
#include "Classes.h"
#include "MATH/math.h"
#include "AOD_Names.h"
#include "hash_Functions.h"
#include "TRAOD/CHR/CHR_Functions.h"


static MATRIX CHR_Translation (const Vec3 &v)
{
	MATRIX m;
	m.m30 = v.x;	m.m31 = v.y;	m.m32 = v.z;
	return m;
}


// Matrice di rotazione (vettore riga) di un quaternione, come CAL_ReadAnimation
MATRIX CHR_QuaternionMatrix (float qx, float qy, float qz, float qw)
{
	float len = sqrtf(qx * qx + qy * qy + qz * qz + qw * qw);
	if (len > 0)
	{
		qx /= len;	qy /= len;	qz /= len;	qw /= len;
	}
	else
		qw = 1;
	MATRIX m;
	m.m00 = 1 - 2 * (qy * qy + qz * qz);	m.m01 = 2 * (qx * qy + qz * qw);		m.m02 = 2 * (qx * qz - qy * qw);
	m.m10 = 2 * (qx * qy - qz * qw);		m.m11 = 1 - 2 * (qx * qx + qz * qz);	m.m12 = 2 * (qy * qz + qx * qw);
	m.m20 = 2 * (qx * qz + qy * qw);		m.m21 = 2 * (qy * qz - qx * qw);		m.m22 = 1 - 2 * (qx * qx + qy * qy);
	return m;
}


/*------------------------------------------------------------------------------------------------------------------
Legge gli oggetti agganciati del CHR (blocco da HeaderSize a SKELETON_PTR, vedi OGGETTI AGGANCIATI in CHR_Struct.h) e
crea un gruppo per ogni nodo (<CHR>_ATTACH_<nodo>), figlio del joint della bone di aggancio o del nodo padre, con la
posa di riposo (traslazione, rotazione XYZ dal quaternione, scala), i pivot e il rotate axis. Le matrici globali in posa
di bind servono a CHR_Read_Meshes per le MESH2 dei nodi. Va chiamata dopo CHR_Read_Skeleton.
------------------------------------------------------------------------------------------------------------------*/
bool CHR_Read_Attachments (ifstream &chrfile, const CHR_HEADER &chr_header, string chrname, CHR_Skeleton &skeleton, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	skeleton.attachments.clear();
	if (chr_header.SKELETON_PTR <= chr_header.HeaderSize)
		return true;
	vector <uint8_t> data(chr_header.SKELETON_PTR);						// Dall'inizio del file (allineamenti relativi al file)
	map <uint32_t, int> bone_of_hash;
	chrfile.clear();
	chrfile.seekg(0);
	if (!chrfile.read(reinterpret_cast<char*>(data.data()), data.size()))
		return false;
	for (unsigned int b = 0; b < skeleton.joint_name.size(); b++)
	{
		CHR_BONE bone;
		chrfile.seekg(chr_header.SKELETON_PTR + (streamoff)b * sizeof(CHR_BONE));
		if (chrfile.read(reinterpret_cast<char*>(&bone), sizeof(bone)))
			bone_of_hash[bone.Bone_name_hashed] = b;
	}
	chrfile.clear();

	auto Get = [&](size_t o) { uint32_t v = 0; if (o + 4 <= data.size()) memcpy(&v, &data[o], 4); return v; };
	set <string> used;
	size_t o = chr_header.HeaderSize;
	while (o + sizeof(CHR_ATTACH_GROUP) <= data.size())
	{
		CHR_ATTACH_GROUP group;
		memcpy(&group, &data[o], sizeof(group));
		o += sizeof(group);
		if (group.nNodes == 0)
			continue;
		if (group.nNodes > 100)
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << chrname << ".CHR: invalid attached object block. Attached objects not exported.";
			break;
		}
		map <uint32_t, int>::iterator bone = bone_of_hash.find(group.BoneHash);
		if (bone == bone_of_hash.end())
			msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << chrname << ".CHR: attached objects on unknown bone " << hex << group.BoneHash << dec << ". Skipped.";
		vector <int> stack;
		int current = -1;
		for (uint32_t n = 0; n < group.nNodes; n++)
		{
			size_t end = o;
			while (end < data.size() && data[end])
				end++;
			if (end >= data.size())
				return true;
			string name(reinterpret_cast<const char*>(&data[o]), end - o);
			o = (end + 4) & ~(size_t)3;									// Nome terminato da 0, allineato a 4 bytes
			uint32_t hash = Get(o), flags = Get(o + 4);
			o = (o + 8 + 15) & ~(size_t)15;								// Padding fino a 16 bytes
			if (o + sizeof(CHR_ATTACH_NODE) > data.size())
				return true;
			CHR_ATTACH_NODE node;
			memcpy(&node, &data[o], sizeof(node));
			o += sizeof(node);
			if (bone == bone_of_hash.end())
				continue;

			CHR_Attachment a;
			a.name = name;
			a.hash = hash;
			a.bone = bone->second;
			if (flags & CHR_BONEFLAG_MATRIXPOP)							// Gerarchia come per le bones
			{
				if (!stack.empty())
				{
					current = stack.back();
					stack.pop_back();
				}
			}
			if (flags & CHR_BONEFLAG_MATRIXPUSH)
				stack.push_back(current);
			a.parent = current;
			current = (int)skeleton.attachments.size();
			a.group = AOD_MayaName(chrname + "_ATTACH_" + name);
			if (used.count(a.group))
				a.group += "_" + to_string(skeleton.attachments.size());
			used.insert(a.group);

			Transform &t = a.transform;
			t.name = a.group;
			t.parent = (a.parent >= 0) ? skeleton.attachments[a.parent].group : skeleton.joint_name[a.bone];
			t.FBX_parent = (a.parent >= 0) ? hashID(t.parent, "Group") : hashID(t.parent, "Joint");
			t.translate_flag = t.rotate_flag = t.scale_flag = true;
			t.tX = node.Translate[0];	t.tY = node.Translate[1];	t.tZ = node.Translate[2];
			t.sX = node.Scale[0];		t.sY = node.Scale[1];		t.sZ = node.Scale[2];
			MATRIX R = CHR_QuaternionMatrix(node.Rotate[0], node.Rotate[1], node.Rotate[2], node.Rotate[3]);
			Vec3 tr, rot, sc;
			mathMatrixDecompose(R, &tr, &rot, &sc);
			t.rX = rot.x;	t.rY = rot.y;	t.rZ = rot.z;
			t.pivot_flag = true;
			t.rotate_pivot = Vec3(node.RotatePivot[0], node.RotatePivot[1], node.RotatePivot[2]);
			t.scale_pivot = Vec3(node.ScalePivot[0], node.ScalePivot[1], node.ScalePivot[2]);
			t.rotate_pivot_translate = Vec3(node.RotatePivotTranslate[0], node.RotatePivotTranslate[1], node.RotatePivotTranslate[2]);
			t.scale_pivot_translate = Vec3(node.ScalePivotTranslate[0], node.ScalePivotTranslate[1], node.ScalePivotTranslate[2]);
			MATRIX RA;
			memcpy(&RA, node.RotateOrient, sizeof(RA));
			RA.m30 = RA.m31 = RA.m32 = 0;
			mathMatrixDecompose(RA, &tr, &rot, &sc);
			t.rotate_axis = rot;

			// Matrice locale come Maya: Tr(-SP) S Tr(SP) Tr(ST) Tr(-RP) RA R Tr(RP) Tr(RT) Tr(T)
			MATRIX S;
			S.m00 = t.sX;	S.m11 = t.sY;	S.m22 = t.sZ;
			Vec3 zero;
			MATRIX L = CHR_Translation(zero - t.scale_pivot);
			L = mathMulMatrices(L, S);
			L = mathMulMatrices(L, CHR_Translation(t.scale_pivot + t.scale_pivot_translate - t.rotate_pivot));
			L = mathMulMatrices(L, RA);
			L = mathMulMatrices(L, R);
			L = mathMulMatrices(L, CHR_Translation(t.rotate_pivot + t.rotate_pivot_translate + Vec3(t.tX, t.tY, t.tZ)));
			a.world = mathMulMatrices(L, (a.parent >= 0) ? skeleton.attachments[a.parent].world : skeleton.global[a.bone]);
			skeleton.attachments.push_back(a);
			FBX.Group.push_back(t);
			MA.Transform.push_back(t);
		}
	}
	if (!skeleton.attachments.empty())
		msg(msg::TGT::FILE, msg::TYP::LOG) << chrname << ".CHR: " << skeleton.attachments.size() << " attached object nodes.";
	return true;
}
