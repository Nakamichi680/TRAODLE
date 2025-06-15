#include "stdafx.h"
#include "Classes.h"
#include "TRAOD/CLN/CLN_Struct.h"


void CLN_Get_Triangle(ifstream &clnfile, XYZ &v0, XYZ &v1, XYZ &v2, unsigned int &MissingAxis, unsigned int &Unknown)
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
	clnfile.read(reinterpret_cast<char*>(&Unknown), sizeof(cln_triangle.Unknown));
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
		v0.y = cln_triangle.v1a;
		v1.y = cln_triangle.v2a;
		v2.y = cln_triangle.v3a;
		v0.z = cln_triangle.v1b;
		v1.z = cln_triangle.v2b;
		v2.z = cln_triangle.v3b;
		v0.x = (v0.y * cln_triangle.vecY + v0.z * cln_triangle.vecZ + cln_triangle.ConstValue) / -cln_triangle.vecX;
		v1.x = (v1.y * cln_triangle.vecY + v1.z * cln_triangle.vecZ + cln_triangle.ConstValue) / -cln_triangle.vecX;
		v2.x = (v2.y * cln_triangle.vecY + v2.z * cln_triangle.vecZ + cln_triangle.ConstValue) / -cln_triangle.vecX;
		break;
	case (2):
		v0.x = cln_triangle.v1a;
		v1.x = cln_triangle.v2a;
		v2.x = cln_triangle.v3a;
		v0.z = cln_triangle.v1b;
		v1.z = cln_triangle.v2b;
		v2.z = cln_triangle.v3b;
		v0.y = (v0.x * cln_triangle.vecX + v0.z * cln_triangle.vecZ + cln_triangle.ConstValue) / -cln_triangle.vecY;
		v1.y = (v1.x * cln_triangle.vecX + v1.z * cln_triangle.vecZ + cln_triangle.ConstValue) / -cln_triangle.vecY;
		v2.y = (v2.x * cln_triangle.vecX + v2.z * cln_triangle.vecZ + cln_triangle.ConstValue) / -cln_triangle.vecY;
		break;
	case (3):
		v0.x = cln_triangle.v1a;
		v1.x = cln_triangle.v2a;
		v2.x = cln_triangle.v3a;
		v0.y = cln_triangle.v1b;
		v1.y = cln_triangle.v2b;
		v2.y = cln_triangle.v3b;
		v0.z = (v0.x * cln_triangle.vecX + v0.y * cln_triangle.vecY + cln_triangle.ConstValue) / -cln_triangle.vecZ;
		v1.z = (v1.x * cln_triangle.vecX + v1.y * cln_triangle.vecY + cln_triangle.ConstValue) / -cln_triangle.vecZ;
		v2.z = (v2.x * cln_triangle.vecX + v2.y * cln_triangle.vecY + cln_triangle.ConstValue) / -cln_triangle.vecZ;
	}

	MissingAxis = cln_triangle.MissingAxis;
}