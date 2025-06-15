/*------------------------------------------------------------------------------------------------------------------
Scrittura Texture
INPUT: Texture input
OUTPUT: FBX_EXPORT &FBX
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "FBX/FBX_Classes.h"


void FBX_Write_Texture (Texture input, FBX_EXPORT &FBX)
{
	// Aggiunta gruppo al computo dei contatori FBX
	FBX.FBX_Count.Texture++;

	// Scrittura Properties
	stringstream out;
	out << "	Texture: " << hashID(input.name, "Texture") << ", \"Texture::" << input.name << "\", \"\" {\n";
    out << "		Type: \"TextureVideoClip\"\n";
    out << "		Version: 202\n";
    out << "		TextureName: \"Texture::" << input.name << "\"\n";
    out << "		Properties70:  {\n";
    out << "			P: \"UVSet\", \"KString\", \"\", \"\", \"UVChannel_" << input.UVChannel << "\"\n";
    out << "			P: \"UseMaterial\", \"bool\", \"\", \"\",1\n";
    out << "		}\n";
    out << "		FileName: \"" << input.filename << "\"\n";
    out << "		ModelUVTranslation: 0,0\n";
    out << "		ModelUVScaling: 1,1\n";
    out << "		Texture_Alpha_Source: \"None\"\n";
    out << "		Cropping: 0,0,0,0\n";
    out << "	}\n";
	FBX.FBX_Properties << out.str();			// Copia il testo nello stringstream Properties definitivo
}