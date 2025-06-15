#include "stdafx.h"
#include "Misc_Functions.h"
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "TRAOD/RMX/RMX_Struct.h"


void RMX_Water (ifstream &rmxfile, string name, string room_name, string layer, ofstream &out, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	RMX_WATER rmx_water;
	XYZ WBmin, WBmax;				// Water Box

	// Lettura dati acqua dal file RMX
	rmxfile.seekg(32, ios_base::cur);
	rmxfile.read(reinterpret_cast<char*>(&rmx_water.X1), sizeof(rmx_water.X1));
	rmxfile.read(reinterpret_cast<char*>(&rmx_water.Y1), sizeof(rmx_water.Y1));
	rmxfile.read(reinterpret_cast<char*>(&rmx_water.Z1), sizeof(rmx_water.Z1));
	rmxfile.seekg(36, ios_base::cur);
	rmxfile.read(reinterpret_cast<char*>(&rmx_water.X2), sizeof(rmx_water.X2));
	rmxfile.read(reinterpret_cast<char*>(&rmx_water.Y2), sizeof(rmx_water.Y2));
	rmxfile.read(reinterpret_cast<char*>(&rmx_water.Z2), sizeof(rmx_water.Z2));
	rmxfile.seekg(8, ios_base::cur);
	rmxfile.read(reinterpret_cast<char*>(&rmx_water.Unknown1), sizeof(rmx_water.Unknown1));
	rmxfile.read(reinterpret_cast<char*>(&rmx_water.Unknown2), sizeof(rmx_water.Unknown2));
	rmxfile.seekg(248, ios_base::cur);
	rmxfile.read(reinterpret_cast<char*>(&WBmin.x), sizeof(rmx_water.Xmin));
	rmxfile.read(reinterpret_cast<char*>(&WBmax.x), sizeof(rmx_water.Xmax));
	rmxfile.read(reinterpret_cast<char*>(&WBmin.y), sizeof(rmx_water.Ymin));
	rmxfile.read(reinterpret_cast<char*>(&WBmax.y), sizeof(rmx_water.Ymax));
	rmxfile.read(reinterpret_cast<char*>(&WBmin.z), sizeof(rmx_water.Zmin));
	rmxfile.read(reinterpret_cast<char*>(&WBmax.z), sizeof(rmx_water.Zmax));
	rmxfile.read(reinterpret_cast<char*>(&rmx_water.ARGB_PC), sizeof(rmx_water.ARGB_PC));
	rmxfile.read(reinterpret_cast<char*>(&rmx_water.ARGB_PS2), sizeof(rmx_water.ARGB_PS2));
	rmxfile.read(reinterpret_cast<char*>(&rmx_water.Unknown5), sizeof(rmx_water.Unknown5));
	rmxfile.read(reinterpret_cast<char*>(&rmx_water.Water_roughness), sizeof(rmx_water.Water_roughness));
	rmxfile.read(reinterpret_cast<char*>(&rmx_water.Waterpatch_size), sizeof(rmx_water.Waterpatch_size));
	rmxfile.seekg(4, ios_base::cur);
	rmxfile.read(reinterpret_cast<char*>(&rmx_water.Unknown7), sizeof(rmx_water.Unknown7));
	rmxfile.read(reinterpret_cast<char*>(&rmx_water.Unknown8), sizeof(rmx_water.Unknown8));
	rmxfile.read(reinterpret_cast<char*>(&rmx_water.Unknown9), sizeof(rmx_water.Unknown9));
	rmxfile.seekg(20, ios_base::cur);

	// Esportazione informazioni acqua nel file TXT
	out << left << setw(30) << "	WBmin [X/Y/Z]:" << right << setw(13) << WBmin.x << setw(13) << WBmin.y << setw(13) << WBmin.z << endl;
	out << left << setw(30) << "	WBmax [X/Y/Z]:" << right << setw(13) << WBmax.x << setw(13) << WBmax.y << setw(13) << WBmax.z << endl;
	out << left << setw(30) << "	Position1 [X/Y/Z]:" << right << setw(13) << rmx_water.X1 << setw(13) << rmx_water.Y1 << setw(13) << rmx_water.Z1 << endl;
	out << left << setw(30) << "	Position2 [X/Y/Z]:" << right << setw(13) << rmx_water.X2 << setw(13) << rmx_water.Y2 << setw(13) << rmx_water.Z2 << endl;
	out << "	Water roughness: " << rmx_water.Water_roughness << endl;
	out << "	Waterpatch size: " << rmx_water.Waterpatch_size << endl;
	out << "	Waterpatch color PC: " << std::hex << rmx_water.ARGB_PC << std::dec << endl;
	out << "	Waterpatch color PS2: " << std::hex << rmx_water.ARGB_PS2 << std::dec << endl;
	out << "	Unknown 1: " << rmx_water.Unknown1 << endl;
	out << "	Unknown 2: " << rmx_water.Unknown2 << endl;
	out << "	Unknown 5: " << rmx_water.Unknown5 << endl;
	out << "	Unknown 7: " << rmx_water.Unknown7 << endl;
	out << "	Unknown 8: " << rmx_water.Unknown8 << endl;
	out << "	Unknown 9: " << rmx_water.Unknown9 << endl;

	// Calcolo dimensioni ingame waterpatch
	float width, height;
	unsigned int subdiv_width, subdiv_height;
	subdiv_width = (unsigned int)((WBmax.x - WBmin.x) / (1024 * rmx_water.Waterpatch_size));
	subdiv_height = (unsigned int)((WBmax.y - WBmin.y) / (1024 * rmx_water.Waterpatch_size));
	width = subdiv_width * (1024 * rmx_water.Waterpatch_size);
	height = subdiv_height * (1024 * rmx_water.Waterpatch_size);
	/*msg(msg::TGT::FILE_CONS, msg::TYP::DBG) << "width pre: " << WBmax.x - WBmin.x;
	msg(msg::TGT::FILE_CONS, msg::TYP::DBG) << "height pre: " << WBmax.y - WBmin.y;
	msg(msg::TGT::FILE_CONS, msg::TYP::DBG) << "subdiv_width: " << subdiv_width;
	msg(msg::TGT::FILE_CONS, msg::TYP::DBG) << "subdiv_height: " << subdiv_height;
	msg(msg::TGT::FILE_CONS, msg::TYP::DBG) << "width post: " << width;
	msg(msg::TGT::FILE_CONS, msg::TYP::DBG) << "height post: " << height;
	msg(msg::TGT::FILE_CONS, msg::TYP::DBG) << "depth: " << WBmax.z - WBmin.z;*/

	string name1, name2, name3, name4;
	name1 = name2 = name3 = name4 = name;
	name1.append("_UNK1");
	name2.append("_UNK2");
	name3.append("_waterpatch");
	name4.append("_BOX");

	// Esportazione locator nei file FBX e MA
	Locator FBX_MA_Water_Locator;
	FBX_MA_Water_Locator.parent = room_name;
	FBX_MA_Water_Locator.layer = layer;						// Nome layer di appartenenza (solo file MA)
	FBX_MA_Water_Locator.FBX_parent = hashID(room_name, "Group");
	FBX_MA_Water_Locator.translate_flag = true;

	FBX_MA_Water_Locator.name = name1;
	FBX_MA_Water_Locator.tX = rmx_water.X1;
	FBX_MA_Water_Locator.tY = rmx_water.Y1;
	FBX_MA_Water_Locator.tZ = rmx_water.Z1;
	FBX.Locator.push_back(FBX_MA_Water_Locator);			// Inserimento oggetto PS2 nel vettore dei locator del file FBX
	MA.Locator.push_back(FBX_MA_Water_Locator);				// Inserimento oggetto PS2 nel vettore dei locator del file MA

	FBX_MA_Water_Locator.name = name2;
	FBX_MA_Water_Locator.tX = rmx_water.X2;
	FBX_MA_Water_Locator.tY = rmx_water.Y2;
	FBX_MA_Water_Locator.tZ = rmx_water.Z2;
	FBX.Locator.push_back(FBX_MA_Water_Locator);			// Inserimento oggetto PS2 nel vettore dei locator del file FBX
	MA.Locator.push_back(FBX_MA_Water_Locator);				// Inserimento oggetto PS2 nel vettore dei locator del file MA
	
	// Esportazione Water Box
	FBX.Geometry.push_back(DrawBox(name4, room_name, "", WBmin, WBmax, 0x15007BFF));
	MA.Mesh.push_back(DrawBox(name4, room_name, layer, WBmin, WBmax, 0x15007BFF));

	// Esportazione Water Patch
	PolyPlane waterpatch;
	waterpatch.name = name3;
	waterpatch.parent = room_name;
	waterpatch.layer = layer;
	waterpatch.translate_flag = true;
	waterpatch.Visible = false;
	waterpatch.tX = WBmin.x + width / 2;
	waterpatch.tY = WBmin.y + height / 2;
	waterpatch.tZ = WBmax.z;
	waterpatch.Width = width;
	waterpatch.Height = height;
	waterpatch.Subdiv_Width = subdiv_width;
	waterpatch.Subdiv_Height = subdiv_height;
	MA.PolyPlane.push_back(waterpatch);

	// Esportazione Boss Wave
	BossWave wave;
	wave.name = name;
	wave.parent = room_name;
	wave.layer = layer;
	wave.inputmesh_name = name3;
	wave.rotate_flag = wave.rotate_pivot_flag = wave.scale_pivot_flag = true;
	wave.rX = 90;
	wave.rpX = WBmin.x + width / 2;
	wave.rpY = WBmin.y + height / 2;
	wave.rpZ = WBmax.z;
	wave.spX = WBmin.x + width / 2;
	wave.spY = WBmin.y + height / 2;
	wave.spZ = WBmax.z;
	wave.patchSizeX = max(height, width) * rmx_water.Waterpatch_size;
	wave.patchSizeZ = max(height, width) * rmx_water.Waterpatch_size;
	wave.spaceScale = 0.01f;
	wave.waveHeight = rmx_water.Water_roughness * rmx_water.Waterpatch_size;
	//wave.windSpeed = rmx_water.Water_roughness / rmx_water.Waterpatch_size;
	wave.oceanDepth = WBmax.z - WBmin.z;
	MA.BossWave.push_back(wave);
}