/*------------------------------------------------------------------------------------------------------------------
Scrittura Object definitions
INPUT: FBX_COUNT_CLASS FBX_Count
OUTPUT: stringstream &Definitions
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "FBX/FBX_Classes.h"


void FBX_Write_Object_definitions (FBX_EXPORT &FBX)
{
	stringstream out;
    out << "; Object definitions\n";
    out << ";------------------------------------------------------------------\n\n";
    out << "Definitions:  {\n";
    if (FBX.FBX_Count.AnimationStack > 0)   {
        out << "	ObjectType: \"AnimationStack\" {\n";
        out << "		Count: " << FBX.FBX_Count.AnimationStack << "\n";
        out << "	}\n";   }
    if (FBX.FBX_Count.AnimationLayer > 0)   {
        out << "	ObjectType: \"AnimationLayer\" {\n";
        out << "		Count: " << FBX.FBX_Count.AnimationLayer << "\n";
        out << "	}\n";   }
    if (FBX.FBX_Count.AnimationCurveNode > 0)   {
        out << "	ObjectType: \"AnimationCurveNode\" {\n";
        out << "		Count: " << FBX.FBX_Count.AnimationCurveNode << "\n";
        out << "	}\n";   }
    if (FBX.FBX_Count.AnimationCurve > 0)   {
        out << "	ObjectType: \"AnimationCurve\" {\n";
        out << "		Count: " << FBX.FBX_Count.AnimationCurve << "\n";
        out << "	}\n";   }
    if (FBX.FBX_Count.Model > 0)    {
        out << "	ObjectType: \"Model\" {\n";
        out << "		Count: " << FBX.FBX_Count.Model << "\n";
        out << "	}\n";   }
    if (FBX.FBX_Count.Geometry > 0) {
        out << "	ObjectType: \"Geometry\" {\n";
        out << "		Count: " << FBX.FBX_Count.Geometry << "\n";
        out << "	}\n";   }
    if (FBX.FBX_Count.Material > 0) {
        out << "	ObjectType: \"Material\" {\n";
        out << "		Count: " << FBX.FBX_Count.Material << "\n";
        out << "	}\n";   }
    if (FBX.FBX_Count.Texture > 0)  {
        out << "	ObjectType: \"Texture\" {\n";
        out << "		Count: " << FBX.FBX_Count.Texture << "\n";
        out << "	}\n";   }
    if (FBX.FBX_Count.NodeAttribute > 0)    {
        out << "	ObjectType: \"NodeAttribute\" {\n";
        out << "		Count: " << FBX.FBX_Count.NodeAttribute << "\n";
        out << "	}\n";   }
    if (FBX.FBX_Count.Pose > 0) {
        out << "	ObjectType: \"Pose\" {\n";
        out << "		Count: " << FBX.FBX_Count.Pose << "\n";
        out << "	}\n";   }
    if (FBX.FBX_Count.Deformer > 0) {
        out << "	ObjectType: \"Deformer\" {\n";
        out << "		Count: " << FBX.FBX_Count.Deformer << "\n";
        out << "	}\n";   }
    out << "}\n\n";

	FBX.FBX_Definitions << out.str();	// Copia il testo nello stringstream Definitions definitivo
}