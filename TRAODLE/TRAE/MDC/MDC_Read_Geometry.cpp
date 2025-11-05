#include "stdafx.h"
#include "FBX/FBX_Classes.h"
#include "MATH/math.h"
#include "MA/MA_Classes.h"
#include "TRAE/MDC/MDC_Struct.h"


bool MDC_Read_Geometry (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	MDC_HEADER header;
	MDC_SKELETON_HEADER skeleton_header;
	//MDC_SKELETON_LIST1_HEADER skeleton_list1_header;
	//MDC_SKELETON_LIST1_ITEM skeleton_list1_item;
	MDC_SKELETON_BONE_OFFSETS skeleton_bone_offsets;
	MDC_SKELETON_BONE skeleton_bone;
	string mdcname = filename.substr(0, filename.size() - 4);


	///////////////////    APERTURA FILE MDC
	SetCurrentDirectory(AE_IO.folder_level_lpwstr);								// Imposta la cartella \NOMELIVELLO come attiva
	ifstream mdcfile(filename, std::ios::binary);
	if (!mdcfile.is_open())
	{
		cout << "\nFATAL ERROR: " << filename << " not found.";
		return false;
	}

	///////////////////    LETTURA HEADER
	mdcfile.read(reinterpret_cast<char*>(&header.Version), sizeof(header.Version));								// Lettura versione file (5Ch)
	mdcfile.read(reinterpret_cast<char*>(&header.TargetPlatform), sizeof(header.TargetPlatform));				// Lettura piattaforma
	mdcfile.read(reinterpret_cast<char*>(&header.Magic), sizeof(header.Magic));									// Lettura tipologia di file ("MDC")
	
	if (header.Magic != 4408397)								// Se il file non è valido la funzione termina
	{
		cout << "\nFATAL ERROR: " << filename << " is not a valid TRAE MDC file.";
		return false;
	}

	if (header.TargetPlatform != 0x100)
	{
		cout << "\nFATAL ERROR: " << filename << " is not a PC MDC file. PSP and PS2 MDC files are not supported.";
		return false;
	}

	if (header.Version == 92)
		cout << "\nTRAE MDC file v. 0.92 detected";

	mdcfile.read(reinterpret_cast<char*>(&header.Unknown1), sizeof(header.Unknown1));
	mdcfile.read(reinterpret_cast<char*>(&header.Name), sizeof(header.Name));
	mdcfile.seekg(20, ios_base::cur);									// Salta Unknown2/3/4/5/6
	mdcfile.read(reinterpret_cast<char*>(&header.PtrSkeleton), sizeof(header.PtrSkeleton));
	mdcfile.read(reinterpret_cast<char*>(&header.PtrGeometry), sizeof(header.PtrGeometry));
	mdcfile.read(reinterpret_cast<char*>(&header.PtrBlock3), sizeof(header.PtrBlock3));
	mdcfile.read(reinterpret_cast<char*>(&header.PtrBlock4), sizeof(header.PtrBlock4));

	///////////////////    LETTURA SCHELETRO
	mdcfile.seekg(header.PtrSkeleton);
	mdcfile.read(reinterpret_cast<char*>(&skeleton_header.nBones), sizeof(skeleton_header.nBones));
	mdcfile.read(reinterpret_cast<char*>(&skeleton_header.Unknown1), sizeof(skeleton_header.Unknown1));
	mdcfile.read(reinterpret_cast<char*>(&skeleton_header.PtrList1), sizeof(skeleton_header.PtrList1));
	mdcfile.read(reinterpret_cast<char*>(&skeleton_header.PtrBoneOffsets), sizeof(skeleton_header.PtrBoneOffsets));
	//mdcfile.read(reinterpret_cast<char*>(&skeleton_header.PtrSkeletonEnd), sizeof(skeleton_header.PtrSkeletonEnd));

	for (unsigned int b = 0; b < skeleton_header.nBones; b++)					// Ogni ciclo legge una bone
	{
		mdcfile.seekg(header.PtrSkeleton + skeleton_header.PtrBoneOffsets + b * 4);
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone_offsets.BoneOffset), sizeof(skeleton_bone_offsets.BoneOffset));
		mdcfile.seekg(header.PtrSkeleton + skeleton_bone_offsets.BoneOffset);

		VECTOR bone_rotation_quaternion;

		// Lettura bone
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.Unknown1), sizeof(skeleton_bone.Unknown1));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.Unknown2), sizeof(skeleton_bone.Unknown2));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.Parent), sizeof(skeleton_bone.Parent));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.Unknown4), sizeof(skeleton_bone.Unknown4));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.Unknown5), sizeof(skeleton_bone.Unknown5));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.X1), sizeof(skeleton_bone.X1));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.Y1), sizeof(skeleton_bone.Y1));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.Z1), sizeof(skeleton_bone.Z1));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.W1), sizeof(skeleton_bone.W1));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.X2), sizeof(skeleton_bone.X2));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.Y2), sizeof(skeleton_bone.Y2));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.Z2), sizeof(skeleton_bone.Z2));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.W2), sizeof(skeleton_bone.W2));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.Xpos), sizeof(skeleton_bone.Xpos));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.Ypos), sizeof(skeleton_bone.Ypos));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.Zpos), sizeof(skeleton_bone.Zpos));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.Padding1), sizeof(skeleton_bone.Padding1));
		mdcfile.read(reinterpret_cast<char*>(&bone_rotation_quaternion.x), sizeof(skeleton_bone.Xrot));
		mdcfile.read(reinterpret_cast<char*>(&bone_rotation_quaternion.y), sizeof(skeleton_bone.Yrot));
		mdcfile.read(reinterpret_cast<char*>(&bone_rotation_quaternion.z), sizeof(skeleton_bone.Zrot));
		mdcfile.read(reinterpret_cast<char*>(&bone_rotation_quaternion.w), sizeof(skeleton_bone.Wrot));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.Xscal), sizeof(skeleton_bone.Xscal));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.Yscal), sizeof(skeleton_bone.Yscal));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.Zscal), sizeof(skeleton_bone.Zscal));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.Padding2), sizeof(skeleton_bone.Padding2));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.a1), sizeof(skeleton_bone.a1));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.a2), sizeof(skeleton_bone.a2));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.a3), sizeof(skeleton_bone.a3));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.b1), sizeof(skeleton_bone.b1));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.b2), sizeof(skeleton_bone.b2));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.b3), sizeof(skeleton_bone.b3));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.Unknown6), sizeof(skeleton_bone.Unknown6));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.X6), sizeof(skeleton_bone.X6));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.Y6), sizeof(skeleton_bone.Y6));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.Z6), sizeof(skeleton_bone.Z6));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.W6), sizeof(skeleton_bone.W6));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.X7), sizeof(skeleton_bone.X7));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.Y7), sizeof(skeleton_bone.Y7));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.Z7), sizeof(skeleton_bone.Z7));
		mdcfile.read(reinterpret_cast<char*>(&skeleton_bone.W7), sizeof(skeleton_bone.W7));

		mathQuatNormalise(&bone_rotation_quaternion);
		MATRIX bone_rotation_matrix = mathQuatToMatrix(bone_rotation_quaternion);
		XYZ bone_rotation_euler = mathMatrixToEuler(bone_rotation_matrix);
		mathRadToDeg(&bone_rotation_euler);

		stringstream name, parent;
		name << header.Name << "_joint_" << b;
		Joint bone;
		bone.name = name.str();

		if (skeleton_bone.Parent != -1)
		{
			parent << header.Name << "_joint_" << skeleton_bone.Parent;
			bone.parent = parent.str();
			bone.FBX_parent = hashID(parent.str(), "Joint");
		}

		bone.translate_flag = bone.rotate_flag = true;
		bone.DrawLabel = true;
		bone.LabelType = Joint::LABELTYPE::OTHER;
		bone.LabelSide = Joint::LABELSIDE::LEFT;
		bone.LabelName = bone.name;
		bone.Radius = 0.015f;
		bone.rX = bone_rotation_euler.x;
		bone.rY = bone_rotation_euler.y;
		bone.rZ = bone_rotation_euler.z;
		bone.tX = skeleton_bone.Xpos;
		bone.tY = skeleton_bone.Ypos;
		bone.tZ = skeleton_bone.Zpos;

		FBX.Joint.push_back(bone);
		MA.Joint.push_back(bone);
		
	}

	mdcfile.close();
	return true;
}