#pragma once
#include "Classes.h"
#include "FBX/FBX_Classes.h"
#include "TRAOD/CHR/CHR_Struct.h"


void FBX_Export (string output_filename, FBX_EXPORT &FBX);
void FBX_Write_Group (Transform input, FBX_EXPORT &FBX);
void FBX_Write_Locator (Locator input, FBX_EXPORT &FBX);
void FBX_Write_Camera (Camera input, FBX_EXPORT &FBX);
void FBX_Write_PointLight (Light input, FBX_EXPORT &FBX);
void FBX_Write_AmbientLight (Light input, FBX_EXPORT &FBX);
void FBX_Write_SpotLight (Light input, FBX_EXPORT &FBX);
void FBX_Write_Geometry (Mesh input, FBX_EXPORT &FBX);
void FBX_Write_Texture (Texture input, FBX_EXPORT &FBX);
void FBX_Write_Joint (Joint input, FBX_EXPORT &FBX);


// Scrittura AnimationStack, AnimationLayer, AnimationCurveNode ed AnimationCurve
void FBX_Write_Animation (int a, FBX_EXPORT &FBX);

// Scrittura AnimationCurve
void FBX_Write_AnimCurve_Subroutine (FBX_EXPORT &FBX, int a, int b, string Type);

// Scrittura BindPose
void FBX_Write_BindPose (FBX_COUNT_CLASS FBX_Count,
						 MESH1_CLASS &mesh1,
						 vector <MESH2_CLASS> &mesh2,
						 vector < vector <int> > &MESH1_FACES,
						 vector < vector < vector <int> > > &MESH2_FACES,
						 CHR_HEADER &chr_header,
						 CHR_MESH2_LIST &chr_mesh2_list,
						 vector <MATRIX> &SKquat,
						 ofstream &out);

// Scrittura BindPose del blend shape
void FBX_Write_BindPose_Blendshape (FBX_COUNT_CLASS FBX_Count, CHR_HEADER &chr_header, TMT_CLASS &tmt, vector <MATRIX> &SKquat, ofstream &out);

// Scrittura Null, Model, Geometry, Deformer, SubDeformer e BlendShape Geometry del file dei blend shapes
void FBX_Write_Blendshape (TMT_CLASS &tmt, vector <int> &BLENDSHAPE_FACES, vector <MATRIX> &SKquat, ofstream &out);

// Scrittura bones
void FBX_Write_Bone (CHR_HEADER &chr_header, vector <string> &Bone_name, vector <MATRIX> &BONESquat, ofstream &out);

// Scrittura Connections delle animazioni
void FBX_Write_Connection_Animation (int a, vector <int> &Anim_bones, vector < vector <AnimationCurveNode> > &Ani_curvenode, ofstream &out);

// Scrittura Connections del blendshape
void FBX_Write_Connection_Blendshape (TMT_CLASS &tmt, ofstream &out);

// Scrittura Connections delle bones
void FBX_Write_Connection_Bone (vector <unsigned int> &Hierarchy, CHR_HEADER &chr_header, ofstream &out);

// Scrittura Connections dei materiali
void FBX_Write_Connection_Material (vector <TextureHeader> &TextureHeader, Material *Materials, ofstream &out);

// Scrittura Connections del gruppo MESH 1
void FBX_Write_Connection_Mesh1 (MESH1_CLASS &mesh1,
								 vector < vector <int> > &MESH1_FACES,
								 vector < vector <unsigned short> > &MESH1_vINDX,
								 CHR_HEADER &chr_header,
								 ofstream &out);

// Scrittura Connections del gruppo MESH 2
void FBX_Write_Connection_Mesh2 (vector <MESH2_CLASS> &mesh2,
								 vector < vector < vector <int> > > &MESH2_FACES,
								 CHR_MESH2_LIST &chr_mesh2_list,
								 ofstream &out);

// Scrittura Header
void FBX_Write_Header (FBX_EXPORT &FBX);

// Scrittura materiale
void FBX_Write_Material (Material *Materials, unsigned int *Nlambert, unsigned int *Nphong, ofstream &out);

// Scrittura Null, Model, Geometry, Deformer e SubDeformer del gruppo MESH 1
void FBX_Write_Mesh1 (MESH1_CLASS &mesh1,
					  vector < vector <int> > &MESH1_FACES,
					  vector < vector <unsigned short> > &MESH1_vINDX,
					  CHR_HEADER &chr_header,
					  vector <MATRIX> &SKquat,
					  ofstream &out);

// Scrittura Null, Model, Geometry, Deformer e SubDeformer del gruppo MESH 2
void FBX_Write_Mesh2 (vector <MESH2_CLASS> &mesh2,
					  vector < vector < vector <int> > > &MESH2_FACES,
					  vector < vector < vector <unsigned short> > > &MESH2_vINDX,
					  CHR_MESH2_LIST &chr_mesh2_list,
					  vector <MATRIX> &SKquat,
					  ofstream &out);

// Scrittura Object definitions
void FBX_Write_Object_definitions (FBX_EXPORT &FBX);

// Scrittura dati texture
void FBX_Write_Texture (AOD_IO_CLASS AOD_IO, TextureHeader *TextureHeader, ofstream &out);

// Generatore ID per nodi del file FBX. Scrive il tipo di nodo (prime due cifre) e fa il padding degi numeri a seguire
//string nodeID (unsigned int node_type, unsigned int value1 = 0, unsigned int value2 = 0, unsigned int value3 = 0);

