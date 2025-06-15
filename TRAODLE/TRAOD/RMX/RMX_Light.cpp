#include "stdafx.h"
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "TRAOD/RMX/RMX_Struct.h"


void RMX_Light (ifstream &rmxfile, string name, string room_name, string layer, ofstream &out, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	RMX_LIGHT rmx_light;

	// Lettura dati luce dal file RMX
	rmxfile.seekg(32, ios_base::cur);
	rmxfile.read(reinterpret_cast<char*>(&rmx_light.Xpos), sizeof(rmx_light.Xpos));						// Posizione X della luce
	rmxfile.read(reinterpret_cast<char*>(&rmx_light.Ypos), sizeof(rmx_light.Ypos));						// Posizione Y della luce
	rmxfile.read(reinterpret_cast<char*>(&rmx_light.Zpos), sizeof(rmx_light.Zpos));						// Posizione Z della luce
	rmxfile.seekg(56, ios_base::cur);
	rmxfile.read(reinterpret_cast<char*>(&rmx_light.Light_ID), sizeof(rmx_light.Light_ID));				// Hash del nome della luce
	rmxfile.seekg(4, ios_base::cur);
	rmxfile.read(reinterpret_cast<char*>(&rmx_light.Static_flag), sizeof(rmx_light.Static_flag));		// 4 = nodo statico, altrimenti legato a script
	rmxfile.read(reinterpret_cast<char*>(&rmx_light.R), sizeof(rmx_light.R));							// Intensità colore rosso della luce (0-255)
	rmxfile.read(reinterpret_cast<char*>(&rmx_light.G), sizeof(rmx_light.G));							// Intensità colore verde della luce (0-255)
	rmxfile.read(reinterpret_cast<char*>(&rmx_light.B), sizeof(rmx_light.B));							// Intensità colore blu della luce (0-255)
	rmxfile.seekg(4, ios_base::cur);
	rmxfile.read(reinterpret_cast<char*>(&rmx_light.intRadius), sizeof(rmx_light.intRadius));			// Diametro interno della luce
	rmxfile.read(reinterpret_cast<char*>(&rmx_light.extRadius), sizeof(rmx_light.extRadius));			// Diametro esterno della luce
	rmxfile.seekg(8, ios_base::cur);

	// Avviso errore raggio interno esterno
	if (rmx_light.extRadius - rmx_light.intRadius < 5)
		msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << "extRadius - intRadius < 5 in light " << name;

	// Esportazione informazioni luce nel file TXT
	out << "	Hashed name: " << hex << rmx_light.Light_ID << dec << endl;
	if (rmx_light.Static_flag == 4)
		out << "	Node type: STATIC\n";
	else
		out << "	Node type: DYNAMIC\n";
	out << left << setw(30) << "	Position [X/Y/Z]:" << right << setw(13) << rmx_light.Xpos << setw(13) << rmx_light.Ypos << setw(13) << rmx_light.Zpos << endl;
	out << left << setw(30) << "	Colour [R/G/B]:" << right << setw(13) << rmx_light.R << setw(13) << rmx_light.G << setw(13) << rmx_light.B << endl;
	out << left << setw(30) << "	Attenuation [near/far]:" << right << setw(13) << rmx_light.intRadius << setw(13) << rmx_light.extRadius << endl << endl;

	// Esportazione luce nei file FBX e MA
	Light FBX_MA_light;
	FBX_MA_light.name = name;										// Nome dell'oggetto (es. PARIS2D_ROOM_2_LIGHT_5)
	FBX_MA_light.parent = room_name;								// Nome del nodo padre (es. PARIS2D_ROOM_2)
	FBX_MA_light.layer = layer;										// Nome layer di appartenenza (solo file MA)
	FBX_MA_light.FBX_parent = hashID(room_name, "Group");
	FBX_MA_light.translate_flag = true;
	FBX_MA_light.tX = rmx_light.Xpos;
	FBX_MA_light.tY = rmx_light.Ypos;
	FBX_MA_light.tZ = rmx_light.Zpos;
	FBX_MA_light.R = rmx_light.R / 255;
	FBX_MA_light.G = rmx_light.G / 255;
	FBX_MA_light.B = rmx_light.B / 255;
	FBX_MA_light.Intensity = rmx_light.extRadius * 10000;
	FBX_MA_light.Decay_Near_Start = 0;
	FBX_MA_light.Decay_Near_End = rmx_light.extRadius;
	FBX_MA_light.Decay_Far_Start = rmx_light.intRadius;
	FBX_MA_light.Decay_Far_End = rmx_light.extRadius;
	FBX.Light.push_back(FBX_MA_light);		// Inserimento luce nel vettore delle luci del file FBX
	MA.Light.push_back(FBX_MA_light);		// Inserimento luce nel vettore delle luci del file MA

	// Esportazione sfera nurbs nel file MA
	NurbsSurface MA_innerSphere, MA_outerSphere;
	stringstream inner, outer;
	inner << name << "_inRadius";
	outer << name << "_outRadius";
	MA_innerSphere.name = inner.str();
	MA_outerSphere.name = outer.str();
	MA_innerSphere.parent = MA_outerSphere.parent = room_name;		// Nome del nodo padre (es. PARIS2D_ROOM_2)
	MA_innerSphere.layer = MA_outerSphere.layer = layer;			// Nome layer di appartenenza (solo file MA)
	MA_innerSphere.Type = MA_outerSphere.Type = "Sphere";
	MA_innerSphere.translate_flag = MA_outerSphere.translate_flag = true;
	MA_innerSphere.tX = MA_outerSphere.tX = rmx_light.Xpos;
	MA_innerSphere.tY = MA_outerSphere.tY = rmx_light.Ypos;
	MA_innerSphere.tZ = MA_outerSphere.tZ = rmx_light.Zpos;
	MA_innerSphere.Radius = rmx_light.intRadius;
	MA_outerSphere.Radius = rmx_light.extRadius;
	//MA.NurbsSurface.push_back(MA_innerSphere);
	//MA.NurbsSurface.push_back(MA_outerSphere);
}