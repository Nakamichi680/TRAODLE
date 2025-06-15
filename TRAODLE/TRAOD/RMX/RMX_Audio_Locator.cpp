#include "stdafx.h"
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "TRAOD/RMX/RMX_Struct.h"


void RMX_Audio_Locator (ifstream &rmxfile, string name, string room_name, string layer, ofstream &out, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	RMX_AUDIO_LOCATOR rmx_audio_locator;

	// Lettura dati audio locator dal file RMX
	rmxfile.seekg(32, ios_base::cur);
	rmxfile.read(reinterpret_cast<char*>(&rmx_audio_locator.Xpos), sizeof(rmx_audio_locator.Xpos));					// Posizione X del locator
	rmxfile.read(reinterpret_cast<char*>(&rmx_audio_locator.Ypos), sizeof(rmx_audio_locator.Ypos));					// Posizione Y del locator
	rmxfile.read(reinterpret_cast<char*>(&rmx_audio_locator.Zpos), sizeof(rmx_audio_locator.Zpos));					// Posizione Z del locator
	rmxfile.seekg(36, ios_base::cur);
	rmxfile.read(reinterpret_cast<char*>(&rmx_audio_locator.Unknown1), sizeof(rmx_audio_locator.Unknown1));			// Sempre 00008042h ?
	rmxfile.read(reinterpret_cast<char*>(&rmx_audio_locator.Unknown2), sizeof(rmx_audio_locator.Unknown2));			// Sempre 00008042h ?
	rmxfile.read(reinterpret_cast<char*>(&rmx_audio_locator.Unknown3), sizeof(rmx_audio_locator.Unknown3));			// Sempre 00008042h ?
	rmxfile.seekg(8, ios_base::cur);
	rmxfile.read(reinterpret_cast<char*>(&rmx_audio_locator.Unknown4), sizeof(rmx_audio_locator.Unknown4));			// Forse è un hash
	rmxfile.read(reinterpret_cast<char*>(&rmx_audio_locator.Unknown5), sizeof(rmx_audio_locator.Unknown5));			// Sempre 0B000000h ?
	rmxfile.read(reinterpret_cast<char*>(&rmx_audio_locator.Unknown6), sizeof(rmx_audio_locator.Unknown6));			// Qualcosa legato al tipo di audio
	rmxfile.seekg(240, ios_base::cur);
	rmxfile.read(reinterpret_cast<char*>(&rmx_audio_locator.AudioTrack), sizeof(rmx_audio_locator.AudioTrack));		// Il numero della traccia audio
	rmxfile.read(reinterpret_cast<char*>(&rmx_audio_locator.Unknown8), sizeof(rmx_audio_locator.Unknown8));			// A volte 0, altre volte 3
	rmxfile.seekg(8, ios_base::cur);

	// Esportazione informazioni audio locator nel file TXT
	out << "	Hashed name: " << hex << rmx_audio_locator.Unknown4 << dec << endl;
	/*if (rmx_light.Static_flag == 4)
		out << "	Node type: STATIC\n";
	else
		out << "	Node type: DYNAMIC\n";*/
	out << "	Unknown 5 (0Bh script?): " << rmx_audio_locator.Unknown5 << endl;
	out << "	Unknown 6 (0-1 loop?): " << rmx_audio_locator.Unknown6 << endl;
	out << "	Audio track: " << rmx_audio_locator.AudioTrack << endl;
	out << "	Unknown 8 (0-3?): " << rmx_audio_locator.Unknown8 << endl;
	out << left << setw(30) << "	Position [X/Y/Z]:" << right << setw(13) << rmx_audio_locator.Xpos << setw(13) << rmx_audio_locator.Ypos << setw(13) << rmx_audio_locator.Zpos << endl;
	out << left << setw(30) << "	Scaling? [X/Y/Z]:" << right << setw(13) << rmx_audio_locator.Unknown1 << setw(13) << rmx_audio_locator.Unknown2 << setw(13) << rmx_audio_locator.Unknown3 << endl;

	// Esportazione audio locator nei file FBX e MA
	Locator FBX_MA_Audio_Locator;
	FBX_MA_Audio_Locator.name = name;
	FBX_MA_Audio_Locator.parent = room_name;
	FBX_MA_Audio_Locator.layer = layer;									// Nome layer di appartenenza (solo file MA)
	FBX_MA_Audio_Locator.FBX_parent = hashID(room_name, "Group");
	FBX_MA_Audio_Locator.translate_flag = true;
	FBX_MA_Audio_Locator.scale_flag = true;
	FBX_MA_Audio_Locator.tX = rmx_audio_locator.Xpos;
	FBX_MA_Audio_Locator.tY = rmx_audio_locator.Ypos;
	FBX_MA_Audio_Locator.tZ = rmx_audio_locator.Zpos;
	//FBX_MA_Audio_Locator.sX = rmx_audio_locator.Unknown1;
	//FBX_MA_Audio_Locator.sY = rmx_audio_locator.Unknown2;
	//FBX_MA_Audio_Locator.sZ = rmx_audio_locator.Unknown3;
	FBX.Locator.push_back(FBX_MA_Audio_Locator);			// Inserimento oggetto PS2 nel vettore dei locator del file FBX
	MA.Locator.push_back(FBX_MA_Audio_Locator);				// Inserimento oggetto PS2 nel vettore dei locator del file MA
}