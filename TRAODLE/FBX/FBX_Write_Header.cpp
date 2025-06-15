/*------------------------------------------------------------------------------------------------------------------
Scrittura Header
INPUT: 
OUTPUT: stringstream &Header
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "FBX/FBX_Classes.h"


void FBX_Write_Header (FBX_EXPORT &FBX)
{
	stringstream out;
    out << "; FBX 7.1.0 project file\n";
    out << "; Copyright (C) 1997-2010 Autodesk Inc. and/or its licensors.\n";
    out << "; All rights reserved.\n";
    out << "; ----------------------------------------------------\n\n";
    out << "FBXHeaderExtension:  {\n";
    out << "	FBXHeaderVersion: 1003\n";
    out << "	FBXVersion: 7500\n";
    out << "	Creator: \"FBX SDK/FBX Plugins version 2011.3.1\"\n";
    out << "	SceneInfo: \"SceneInfo::GlobalInfo\", \"UserData\" {\n";
    out << "		Type: \"UserData\"\n";
    out << "		Version: 100\n";
    out << "		MetaData:  {\n";
    out << "			Version: 100\n";
    out << "			Title: \"\"\n";
    out << "			Subject: \"\"\n";
    out << "			Author: \"\"\n";
    out << "			Keywords: \"\"\n";
    out << "			Revision: \"\"\n";
    out << "			Comment: \"\"\n";
    out << "		}\n";
    out << "	}\n";
    out << "}\n";
    out << "GlobalSettings:  {\n";
    out << "	Version: 1000\n";
    out << "	Properties70:  {\n";
	if (FBX.UpAxis == 'z')
		out << "		P: \"UpAxis\", \"int\", \"Integer\", \"\",2\n";
	if (FBX.UpAxis == 'y')
		out << "		P: \"UpAxis\", \"int\", \"Integer\", \"\",1\n";

    out << "		P: \"UpAxisSign\", \"int\", \"Integer\", \"\",1\n";
    out << "		P: \"FrontAxis\", \"int\", \"Integer\", \"\",1\n";
    out << "		P: \"FrontAxisSign\", \"int\", \"Integer\", \"\",-1\n";
    out << "		P: \"CoordAxis\", \"int\", \"Integer\", \"\",0\n";
    out << "		P: \"CoordAxisSign\", \"int\", \"Integer\", \"\",1\n";
    out << "		P: \"OriginalUpAxis\", \"int\", \"Integer\", \"\",2\n";
    out << "		P: \"OriginalUpAxisSign\", \"int\", \"Integer\", \"\",1\n";
    out << "		P: \"UnitScaleFactor\", \"double\", \"Number\", \"\",1\n";
    out << "		P: \"OriginalUnitScaleFactor\", \"double\", \"Number\", \"\",1\n";
    out << "		P: \"AmbientColor\", \"ColorRGB\", \"Color\", \"\",0,0,0\n";
    out << "		P: \"DefaultCamera\", \"KString\", \"\", \"\", \"Producer Perspective\"\n";
    out << "		P: \"TimeMode\", \"enum\", \"\", \"\",6\n";
    out << "		P: \"TimeSpanStart\", \"KTime\", \"Time\", \"\",0\n";
    out << "		P: \"TimeSpanStop\", \"KTime\", \"Time\", \"\",15395386000000\n";
    out << "	}\n";
    out << "}\n\n";
	
	FBX.FBX_Header << out.str();		// Copia il testo nello stringstream Header definitivo
}