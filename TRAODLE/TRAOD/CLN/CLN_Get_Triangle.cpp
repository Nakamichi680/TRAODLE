#include "stdafx.h"
#include "Classes.h"
#include "TRAOD/CLN/CLN_Struct.h"


void CLN_Get_Triangle(ifstream &clnfile, Vec3 &v1, Vec3 &v2, Vec3 &v3, unsigned int &MissingAxis, unsigned int &attribute)
{
	CLN_TRIANGLE cln_triangle;

	clnfile.read(reinterpret_cast<char*>(&cln_triangle.vecX), sizeof(cln_triangle.vecX));
	clnfile.read(reinterpret_cast<char*>(&cln_triangle.vecY), sizeof(cln_triangle.vecY));
	clnfile.read(reinterpret_cast<char*>(&cln_triangle.vecZ), sizeof(cln_triangle.vecZ));
	clnfile.read(reinterpret_cast<char*>(&cln_triangle.ConstValue), sizeof(cln_triangle.ConstValue));
	clnfile.read(reinterpret_cast<char*>(&cln_triangle.v1a), sizeof(cln_triangle.v1a));
	clnfile.read(reinterpret_cast<char*>(&cln_triangle.v2a), sizeof(cln_triangle.v2a));
	clnfile.read(reinterpret_cast<char*>(&cln_triangle.v3a), sizeof(cln_triangle.v3a));
	clnfile.read(reinterpret_cast<char*>(&cln_triangle.MissingAxis), sizeof(cln_triangle.MissingAxis));
	clnfile.read(reinterpret_cast<char*>(&cln_triangle.v1b), sizeof(cln_triangle.v1b));
	clnfile.read(reinterpret_cast<char*>(&cln_triangle.v2b), sizeof(cln_triangle.v2b));
	clnfile.read(reinterpret_cast<char*>(&cln_triangle.v3b), sizeof(cln_triangle.v3b));
	clnfile.read(reinterpret_cast<char*>(&attribute), sizeof(cln_triangle.attribute));
	cln_triangle.ConstValue *= 1024;
	cln_triangle.v1a *= 1024;
	cln_triangle.v2a *= 1024;
	cln_triangle.v3a *= 1024;
	cln_triangle.v1b *= 1024;
	cln_triangle.v2b *= 1024;
	cln_triangle.v3b *= 1024;

	switch (cln_triangle.MissingAxis)
	{
	case (1):
		v1.y = cln_triangle.v1a;
		v2.y = cln_triangle.v2a;
		v3.y = cln_triangle.v3a;
		v1.z = cln_triangle.v1b;
		v2.z = cln_triangle.v2b;
		v3.z = cln_triangle.v3b;
		v1.x = (v1.y * cln_triangle.vecY + v1.z * cln_triangle.vecZ + cln_triangle.ConstValue) / -cln_triangle.vecX;
		v2.x = (v2.y * cln_triangle.vecY + v2.z * cln_triangle.vecZ + cln_triangle.ConstValue) / -cln_triangle.vecX;
		v3.x = (v3.y * cln_triangle.vecY + v3.z * cln_triangle.vecZ + cln_triangle.ConstValue) / -cln_triangle.vecX;
		break;
	case (2):
		v1.x = cln_triangle.v1a;
		v2.x = cln_triangle.v2a;
		v3.x = cln_triangle.v3a;
		v1.z = cln_triangle.v1b;
		v2.z = cln_triangle.v2b;
		v3.z = cln_triangle.v3b;
		v1.y = (v1.x * cln_triangle.vecX + v1.z * cln_triangle.vecZ + cln_triangle.ConstValue) / -cln_triangle.vecY;
		v2.y = (v2.x * cln_triangle.vecX + v2.z * cln_triangle.vecZ + cln_triangle.ConstValue) / -cln_triangle.vecY;
		v3.y = (v3.x * cln_triangle.vecX + v3.z * cln_triangle.vecZ + cln_triangle.ConstValue) / -cln_triangle.vecY;
		break;
	case (3):
		v1.x = cln_triangle.v1a;
		v2.x = cln_triangle.v2a;
		v3.x = cln_triangle.v3a;
		v1.y = cln_triangle.v1b;
		v2.y = cln_triangle.v2b;
		v3.y = cln_triangle.v3b;
		v1.z = (v1.x * cln_triangle.vecX + v1.y * cln_triangle.vecY + cln_triangle.ConstValue) / -cln_triangle.vecZ;
		v2.z = (v2.x * cln_triangle.vecX + v2.y * cln_triangle.vecY + cln_triangle.ConstValue) / -cln_triangle.vecZ;
		v3.z = (v3.x * cln_triangle.vecX + v3.y * cln_triangle.vecY + cln_triangle.ConstValue) / -cln_triangle.vecZ;
	}

	MissingAxis = cln_triangle.MissingAxis;
}