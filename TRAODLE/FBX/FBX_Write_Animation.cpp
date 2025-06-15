#include "stdafx.h"
#include "FBX/FBX_Classes.h"
#include "FBX/FBX_Functions.h"


void FBX_Write_Animation (int a, FBX_EXPORT &FBX)
{
	stringstream out;

	/////////////////////// SCRITTURA ANIMATIONSTACK
	unsigned long long time = FBX.Animation[a].nFrames * FBXframe1;
    out << "	AnimationStack: " << hashID(FBX.Animation[a].name, "AnimStack") << ", \"AnimStack::" << FBX.Animation[a].name << "\", \"\" {\n";
	out << "		Properties70:  {\n";
	out << "			P: \"LocalStart\", \"KTime\", \"Time\", \"\"," << FBXframe1 << endl;
	out << "			P: \"LocalStop\", \"KTime\", \"Time\", \"\"," << time << endl;
	out << "			P: \"ReferenceStart\", \"KTime\", \"Time\", \"\"," << FBXframe1 << endl;
	out << "			P: \"ReferenceStop\", \"KTime\", \"Time\", \"\"," << time << endl;
	out << "		}\n";
	out << "	}\n";
	FBX.FBX_Count.AnimationStack = 1;

	/////////////////////// SCRITTURA ANIMATIONLAYER
	out << "	AnimationLayer: " << hashID(FBX.Animation[a].name, "AnimLayer") << ", \"AnimLayer::BaseLayer\", \"\" {\n";
	out << "	}\n";
	FBX.FBX_Count.AnimationLayer = 1;

	for (unsigned int b = 0; b < FBX.Animation[a].nJoints; b++)			// Ogni ciclo fa una joint
	{
		/////////////////////// SCRITTURA ANIMATIONCURVENODE
		out << FBX.Animation[a].Curvenode[b].Properties_Text(FBX.Animation[a].name);
		if (FBX.Animation[a].Curvenode[b].tX_flag || FBX.Animation[a].Curvenode[b].tY_flag || FBX.Animation[a].Curvenode[b].tZ_flag)
			FBX.FBX_Count.AnimationCurveNode++;
		if (FBX.Animation[a].Curvenode[b].rX_flag || FBX.Animation[a].Curvenode[b].rY_flag || FBX.Animation[a].Curvenode[b].rZ_flag)
			FBX.FBX_Count.AnimationCurveNode++;
		if (FBX.Animation[a].Curvenode[b].sX_flag || FBX.Animation[a].Curvenode[b].sY_flag || FBX.Animation[a].Curvenode[b].sZ_flag)
			FBX.FBX_Count.AnimationCurveNode++;

		/////////////////////// SCRITTURA ANIMATIONCURVE
		// Traslazioni
		if (FBX.Animation[a].Curvenode[b].tX_flag)
			FBX_Write_AnimCurve_Subroutine(FBX, a, b, "AnimCurveTX");
		if (FBX.Animation[a].Curvenode[b].tY_flag)
			FBX_Write_AnimCurve_Subroutine(FBX, a, b, "AnimCurveTY");
		if (FBX.Animation[a].Curvenode[b].tZ_flag)
			FBX_Write_AnimCurve_Subroutine(FBX, a, b, "AnimCurveTZ");
		// Rotazioni
		if (FBX.Animation[a].Curvenode[b].rX_flag)
			FBX_Write_AnimCurve_Subroutine(FBX, a, b, "AnimCurveRX");
		if (FBX.Animation[a].Curvenode[b].rY_flag)
			FBX_Write_AnimCurve_Subroutine(FBX, a, b, "AnimCurveRY");
		if (FBX.Animation[a].Curvenode[b].rZ_flag)
			FBX_Write_AnimCurve_Subroutine(FBX, a, b, "AnimCurveRZ");
		// Scalature
		if (FBX.Animation[a].Curvenode[b].sX_flag)
			FBX_Write_AnimCurve_Subroutine(FBX, a, b, "AnimCurveSX");
		if (FBX.Animation[a].Curvenode[b].sY_flag)
			FBX_Write_AnimCurve_Subroutine(FBX, a, b, "AnimCurveSY");
		if (FBX.Animation[a].Curvenode[b].sZ_flag)
			FBX_Write_AnimCurve_Subroutine(FBX, a, b, "AnimCurveSZ");
	}

	FBX.FBX_Properties << out.str();

	out.str("");

	out << "	;AnimLayer::BaseLayer, AnimStack::" << FBX.Animation[a].name << "\n";
	out << "	C: \"OO\"," << hashID(FBX.Animation[a].name, "AnimLayer") << "," << hashID(FBX.Animation[a].name, "AnimStack") << "\n\n";		// AnimationLayer, AnimationStack
	for (unsigned int b = 0; b < FBX.Animation[a].nJoints; b++)
		out << FBX.Animation[a].Curvenode[b].Connections_Text(FBX.Animation[a].name);

	FBX.FBX_Connections << out.str();
}