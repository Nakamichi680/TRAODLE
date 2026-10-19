/*------------------------------------------------------------------------------------------------------------------
Scrittura gruppo
INPUT: Transform input
OUTPUT: FBX_EXPORT &FBX
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "FBX/FBX_Classes.h"
#include "MATH/math.h"


// Angoli XYZ (gradi) della rotazione inversa: Maya importa il rotate axis come inversa della PostRotation dell'FBX
static Vec3 FBX_InverseRotation (const Vec3 &r)
{
	MATRIX m;
	const float angles[3] = {r.x, r.y, r.z};
	for (int axis = 0; axis < 3; axis++)						// Rx * Ry * Rz (vettore riga, ordine XYZ di Maya)
	{
		float c = cosf(angles[axis] * (float)M_PI / 180), s = sinf(angles[axis] * (float)M_PI / 180);
		MATRIX rot;
		float *e[3][3] = {{&rot.m00, &rot.m01, &rot.m02}, {&rot.m10, &rot.m11, &rot.m12}, {&rot.m20, &rot.m21, &rot.m22}};
		int i = (axis + 1) % 3, j = (axis + 2) % 3;
		*e[i][i] = c;	*e[i][j] = s;	*e[j][i] = -s;	*e[j][j] = c;
		m = mathMulMatrices(m, rot);
	}
	Vec3 t, inverse, scale;
	mathMatrixDecompose(mathTransposeAMatrix(m), &t, &inverse, &scale);
	return inverse;
}


void FBX_Write_Group (Transform input, FBX_EXPORT &FBX)
{
	// Aggiunta gruppo al computo dei contatori FBX
	FBX.FBX_Count.Model++;
	FBX.FBX_Count.NodeAttribute++;

	// Scrittura Properties
	stringstream out;
	out << "	Model: " << hashID(input.name, "Group") << ", \"Model::" << input.name << "\", \"Null\" {\n";
	out << "		Version: 232\n";
	out << "		Properties70:  {\n";
	out << "			P: \"RotationActive\", \"bool\", \"\", \"\",1\n";
	out << "			P: \"InheritType\", \"enum\", \"\", \"\",1\n";
	out << "			P: \"ScalingMax\", \"Vector3D\", \"Vector\", \"\",0,0,0\n";
	out << "			P: \"DefaultAttributeIndex\", \"int\", \"Integer\", \"\",0\n";
	if (input.translate_flag)
		out << "			P: \"Lcl Translation\", \"Lcl Translation\", \"\", \"A\"," << input.tX << "," << input.tY << "," << input.tZ << "\n";
	if (input.rotate_flag)
		out << "			P: \"Lcl Rotation\", \"Lcl Rotation\", \"\", \"A\"," << input.rX << "," << input.rY << "," << input.rZ << "\n";
	if (input.scale_flag)
		out << "			P: \"Lcl Scaling\", \"Lcl Scaling\", \"\", \"A\"," << input.sX << "," << input.sY << "," << input.sZ << "\n";
	if (input.pivot_flag)												// Pivot di Maya: Roff = rotate pivot translate, Soff = scale pivot translate, Rpost = inversa del rotate axis
	{
		Vec3 post = FBX_InverseRotation(input.rotate_axis);
		const pair <const char*, const Vec3*> pivots[] = {{"RotationOffset", &input.rotate_pivot_translate}, {"RotationPivot", &input.rotate_pivot},
			{"ScalingOffset", &input.scale_pivot_translate}, {"ScalingPivot", &input.scale_pivot}, {"PostRotation", &post}};
		for (const auto &p : pivots)
			if (p.second->x != 0 || p.second->y != 0 || p.second->z != 0)
				out << "			P: \"" << p.first << "\", \"Vector3D\", \"Vector\", \"\"," << p.second->x << "," << p.second->y << "," << p.second->z << "\n";
	}
	for (const pair <string, int> &a : input.int_attributes)			// Attributi personalizzati (proprieta' utente)
		out << "			P: \"" << a.first << "\", \"int\", \"Integer\", \"U\"," << a.second << "\n";
	for (const pair <string, float> &a : input.float_attributes)
		out << "			P: \"" << a.first << "\", \"double\", \"Number\", \"U\"," << a.second << "\n";
	for (const pair <string, string> &a : input.string_attributes)
		out << "			P: \"" << a.first << "\", \"KString\", \"\", \"U\", \"" << a.second << "\"\n";
	out << "		}\n";
	out << "		Shading: Y\n";
	out << "		Culling: \"CullingOff\"\n";
	out << "	}\n";
	out << "	NodeAttribute: " << hashID(input.name, "Group_NodeAttribute") << ", \"NodeAttribute::\", \"Null\" {\n";
	out << "		Properties70:  {\n";
	out << "			P: \"Look\", \"enum\", \"\", \"\",0\n";
	out << "		}\n";
	out << "		TypeFlags: \"Null\"\n";
	out << "	}\n";
	FBX.FBX_Properties << out.str();			// Copia il testo nello stringstream Properties definitivo

	out.str("");		// Pulisce lo stringstream

	// Scrittura Connections
	out << "	;Model::" << input.name << ", Model::";
	if (input.FBX_parent == "0")
		out << "RootNode\n";
	else
		out << input.parent << "\n";
	out << "	C: \"OO\"," << hashID(input.name, "Group") << "," << input.FBX_parent << "\n\n";
	out << "	;NodeAttribute::, Model::" << input.name << "\n";
	out << "	C: \"OO\"," << hashID(input.name, "Group_NodeAttribute") << "," << hashID(input.name, "Group") << "\n\n";
	FBX.FBX_Connections << out.str();
}