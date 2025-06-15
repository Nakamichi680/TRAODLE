#include "stdafx.h"
#include "FBX/FBX_Classes.h"
#include "FBX/FBX_Functions.h"


void FBX_Write_AnimCurve_Subroutine (FBX_EXPORT &FBX, int a, int b, string Type)
{
	stringstream out, temp2;
	AnimationCurveData temp;
	temp2 << FBX.Animation[a].name << "_" << FBX.Animation[a].Curvenode[b].name;
	float Base_trasl = 0;

	if (Type == "AnimCurveTX")						// Traslazione X
	{
		temp = FBX.Animation[a].Curvenode[b].tX;
		Base_trasl = FBX.Animation[a].Curvenode[b].Xtrasl;
	}
	if (Type == "AnimCurveTY")						// Traslazione Y
	{
		temp = FBX.Animation[a].Curvenode[b].tY;
		Base_trasl = FBX.Animation[a].Curvenode[b].Ytrasl;
	}
	if (Type == "AnimCurveTZ")						// Traslazione Z
	{
		temp = FBX.Animation[a].Curvenode[b].tZ;
		Base_trasl = FBX.Animation[a].Curvenode[b].Ztrasl;
	}
	if (Type == "AnimCurveRX")						// Rotazione X
		temp = FBX.Animation[a].Curvenode[b].rX;
	if (Type == "AnimCurveRY")						// Rotazione Y
		temp = FBX.Animation[a].Curvenode[b].rY;
	if (Type == "AnimCurveRZ")						// Rotazione Z
		temp = FBX.Animation[a].Curvenode[b].rZ;
	if (Type == "AnimCurveSX")						// Scalatura X
		temp = FBX.Animation[a].Curvenode[b].sX;
	if (Type == "AnimCurveSY")						// Scalatura Y
		temp = FBX.Animation[a].Curvenode[b].sY;
	if (Type == "AnimCurveSZ")						// Scalatura Z
		temp = FBX.Animation[a].Curvenode[b].sZ;

	// Scrittura Properties
	out << "	AnimationCurve: " << hashID(temp2.str(), Type) << ", \"AnimCurve::\", \"\" {\n";
	out << "		Default: 0\n";
	out << "		KeyVer: 4008\n";
	out << "		KeyTime: *" << temp.KeyTime.size() << " {\n";
	out << "			a: ";
	for (unsigned int k = 0; k < temp.KeyTime.size(); k++)
    {
        out << temp.KeyTime[k];
        if (temp.KeyTime.size() - k != 1)
		{
            out << ",";
            if ((k + 1) % 18 == 0 && k != 0)  out << "\n";
		}
        else out << "\n";
    }
	out << "		}\n";
	out << "		KeyValueFloat: *" << temp.KeyValueFloat.size()  << " {\n";
	out << "			a: ";
	for (unsigned int k = 0; k < temp.KeyValueFloat.size(); k++)
	{
		out << temp.KeyValueFloat[k] + Base_trasl;
		if (temp.KeyValueFloat.size() - k != 1)
		{
			out << ",";
			if ((k + 1) % 26 == 0 && k != 0)  out << "\n";
		}
		else out << "\n";
	}
	out << "		}\n";
	out << "		KeyAttrFlags: *" << temp.KeyAttrFlags.size()  << " {\n";
	out << "			a: ";
    for (unsigned int k = 0; k < temp.KeyAttrFlags.size(); k++)
    {
        out << temp.KeyAttrFlags[k];
        if (temp.KeyAttrFlags.size() - k != 1)
		{
            out << ",";
            if ((k + 1) % 38 == 0 && k != 0)  out << "\n";
		}
        else out << "\n";
    }
	out << "		}\n";
	out << "		KeyAttrDataFloat: *" << temp.KeyAttrDataFloat.size()  << " {\n";
	out << "			a: ";
    for (unsigned int k = 0; k < temp.KeyAttrDataFloat.size(); k++)
    {
        out << temp.KeyAttrDataFloat[k];
        if (temp.KeyAttrDataFloat.size() - k != 1)
		{
            out << ",";
            if ((k + 1) % 31 == 0 && k != 0)  out << "\n";
		}
        else out << "\n";
    }
	out << "		}\n";
	out << "		KeyAttrRefCount: *" << temp.KeyAttrFlags.size()  << " {\n";
	out << "		a: ";
    for (unsigned int k = 0; k < temp.KeyAttrFlags.size(); k++)
    {
        out << 1;
        if (temp.KeyAttrFlags.size() - k != 1)
		{
            out << ",";
            if ((k + 1) % 100 == 0 && k != 0)  out << "\n";
		}
        else out << "\n";
    }
    out << "		}\n";
    out << "	}\n";

	FBX.FBX_Properties << out.str();
	FBX.FBX_Count.AnimationCurve++;

	out.str("");		// Pulisce lo stringstream

	// Scrittura Connections
	if (Type == "AnimCurveTX" || Type == "AnimCurveTY" || Type == "AnimCurveTZ")
		out << "	;AnimCurve::, AnimCurveNode::T\n";
	if (Type == "AnimCurveRX" || Type == "AnimCurveRY" || Type == "AnimCurveRZ")
		out << "	;AnimCurve::, AnimCurveNode::R\n";
	if (Type == "AnimCurveSX" || Type == "AnimCurveSY" || Type == "AnimCurveSZ")
		out << "	;AnimCurve::, AnimCurveNode::S\n";

	if (Type == "AnimCurveTX")
		out << "	C: \"OP\"," << hashID(temp2.str(), Type) << "," << hashID(temp2.str(), "AnimCurveNodeT") << ", \"d|X\"\n\n";		// AnimationCurve tX, AnimationCurveNode t
	if (Type == "AnimCurveTY")
		out << "	C: \"OP\"," << hashID(temp2.str(), Type) << "," << hashID(temp2.str(), "AnimCurveNodeT") << ", \"d|Y\"\n\n";		// AnimationCurve tY, AnimationCurveNode t
	if (Type == "AnimCurveTZ")
		out << "	C: \"OP\"," << hashID(temp2.str(), Type) << "," << hashID(temp2.str(), "AnimCurveNodeT") << ", \"d|Z\"\n\n";		// AnimationCurve tZ, AnimationCurveNode t

	if (Type == "AnimCurveRX")
		out << "	C: \"OP\"," << hashID(temp2.str(), Type) << "," << hashID(temp2.str(), "AnimCurveNodeR") << ", \"d|X\"\n\n";		// AnimationCurve rX, AnimationCurveNode r
	if (Type == "AnimCurveRY")
		out << "	C: \"OP\"," << hashID(temp2.str(), Type) << "," << hashID(temp2.str(), "AnimCurveNodeR") << ", \"d|Y\"\n\n";		// AnimationCurve rY, AnimationCurveNode r
	if (Type == "AnimCurveRZ")
		out << "	C: \"OP\"," << hashID(temp2.str(), Type) << "," << hashID(temp2.str(), "AnimCurveNodeR") << ", \"d|Z\"\n\n";		// AnimationCurve rZ, AnimationCurveNode r

	if (Type == "AnimCurveSX")
		out << "	C: \"OP\"," << hashID(temp2.str(), Type) << "," << hashID(temp2.str(), "AnimCurveNodeS") << ", \"d|X\"\n\n";		// AnimationCurve sX, AnimationCurveNode s
	if (Type == "AnimCurveSY")
		out << "	C: \"OP\"," << hashID(temp2.str(), Type) << "," << hashID(temp2.str(), "AnimCurveNodeS") << ", \"d|Y\"\n\n";		// AnimationCurve sY, AnimationCurveNode s
	if (Type == "AnimCurveSZ")
		out << "	C: \"OP\"," << hashID(temp2.str(), Type) << "," << hashID(temp2.str(), "AnimCurveNodeS") << ", \"d|Z\"\n\n";		// AnimationCurve sZ, AnimationCurveNode s

	FBX.FBX_Connections << out.str();
}