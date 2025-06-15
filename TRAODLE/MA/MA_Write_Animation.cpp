#include "stdafx.h"
#include "MA/MA_Classes.h"
#include "MA/MA_Functions.h"

void MA_Write_Animation (unsigned int a, MA_EXPORT &MA)
{
	stringstream node, conn;
	for (unsigned int b = 0; b < MA.Animation[a].size(); b++)
	{
		///////////////		TRASLAZIONI
		if (MA.Animation[a][b].translateX_flag)
		{
			node << "createNode animCurveTL -n \"" << MA.Animation[a][b].name << "_translateX\";\n";
			node << MA_Write_Animation_Subroutine(&MA.Animation[a][b].tX);
			conn << "connectAttr \"" << MA.Animation[a][b].name << "_translateX.o\" \"" << MA.Animation[a][b].parent << ".tx\";\n";
		}
		if (MA.Animation[a][b].translateY_flag)
		{
			node << "createNode animCurveTL -n \"" << MA.Animation[a][b].name << "_translateY\";\n";
			node << MA_Write_Animation_Subroutine(&MA.Animation[a][b].tY);
			conn << "connectAttr \"" << MA.Animation[a][b].name << "_translateY.o\" \"" << MA.Animation[a][b].parent << ".ty\";\n";
		}
		if (MA.Animation[a][b].translateZ_flag)
		{
			node << "createNode animCurveTL -n \"" << MA.Animation[a][b].name << "_translateZ\";\n";
			node << MA_Write_Animation_Subroutine(&MA.Animation[a][b].tZ);
			conn << "connectAttr \"" << MA.Animation[a][b].name << "_translateZ.o\" \"" << MA.Animation[a][b].parent << ".tz\";\n";
		}
		///////////////		ROTAZIONI
		if (MA.Animation[a][b].rotateX_flag)
		{
			node << "createNode animCurveTA -n \"" << MA.Animation[a][b].name << "_rotateX\";\n";
			node << MA_Write_Animation_Subroutine(&MA.Animation[a][b].rX);
			conn << "connectAttr \"" << MA.Animation[a][b].name << "_rotateX.o\" \"" << MA.Animation[a][b].parent << ".rx\";\n";
		}
		if (MA.Animation[a][b].rotateY_flag)
		{
			node << "createNode animCurveTA -n \"" << MA.Animation[a][b].name << "_rotateY\";\n";
			node << MA_Write_Animation_Subroutine(&MA.Animation[a][b].rY);
			conn << "connectAttr \"" << MA.Animation[a][b].name << "_rotateY.o\" \"" << MA.Animation[a][b].parent << ".ry\";\n";
		}
		if (MA.Animation[a][b].rotateZ_flag)
		{
			node << "createNode animCurveTA -n \"" << MA.Animation[a][b].name << "_rotateZ\";\n";
			node << MA_Write_Animation_Subroutine(&MA.Animation[a][b].rZ);
			conn << "connectAttr \"" << MA.Animation[a][b].name << "_rotateZ.o\" \"" << MA.Animation[a][b].parent << ".rz\";\n";
		}
		///////////////		SCALATURE
		if (MA.Animation[a][b].scaleX_flag)
		{
			node << "createNode animCurveTU -n \"" << MA.Animation[a][b].name << "_scaleX\";\n";
			node << MA_Write_Animation_Subroutine(&MA.Animation[a][b].sX);
			conn << "connectAttr \"" << MA.Animation[a][b].name << "_scaleX.o\" \"" << MA.Animation[a][b].parent << ".sx\";\n";
		}
		if (MA.Animation[a][b].scaleY_flag)
		{
			node << "createNode animCurveTU -n \"" << MA.Animation[a][b].name << "_scaleY\";\n";
			node << MA_Write_Animation_Subroutine(&MA.Animation[a][b].sY);
			conn << "connectAttr \"" << MA.Animation[a][b].name << "_scaleY.o\" \"" << MA.Animation[a][b].parent << ".sy\";\n";
		}
		if (MA.Animation[a][b].scaleZ_flag)
		{
			node << "createNode animCurveTU -n \"" << MA.Animation[a][b].name << "_scaleZ\";\n";
			node << MA_Write_Animation_Subroutine(&MA.Animation[a][b].sZ);
			conn << "connectAttr \"" << MA.Animation[a][b].name << "_scaleZ.o\" \"" << MA.Animation[a][b].parent << ".sz\";\n";
		}
		///////////////		LUNGHEZZA FOCALE
		if (MA.Animation[a][b].focalLength_flag)
		{
			node << "createNode animCurveTU -n \"" << MA.Animation[a][b].name << "Shape_focalLength\";\n";
			node << MA_Write_Animation_Subroutine(&MA.Animation[a][b].fl);
			conn << "connectAttr \"" << MA.Animation[a][b].name << "Shape_focalLength.o\" \"" << MA.Animation[a][b].parent << "Shape.fl\";\n";
		}
		///////////////		CENTER OF INTEREST
		if (MA.Animation[a][b].centerOfInterest_flag)
		{
			node << "createNode animCurveTL -n \"" << MA.Animation[a][b].name << "Shape_centerOfInterest\";\n";
			node << MA_Write_Animation_Subroutine(&MA.Animation[a][b].coi);
			conn << "connectAttr \"" << MA.Animation[a][b].name << "Shape_centerOfInterest.o\" \"" << MA.Animation[a][b].parent << "Shape.coi\";\n";
		}
	}

	MA.MA_Nodes << node.str();
	MA.MA_Connections << conn.str();
}
