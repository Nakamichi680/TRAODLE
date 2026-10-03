#pragma once
#include <map>
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"


bool Export_RMX (string filename);

bool RMX_Read (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA);

string Reverb_preset (unsigned int ReverbID);

void RMX_Light (ifstream &rmxfile, string name, string room_name, string layer, ofstream &out, FBX_EXPORT &FBX, MA_EXPORT &MA);

void RMX_Water (ifstream &rmxfile, string name, string room_name, string layer, ofstream &out, FBX_EXPORT &FBX, MA_EXPORT &MA);

void RMX_PS2_Room_Obj (ifstream &rmxfile, string name, string room_name, string layer, ofstream &out, FBX_EXPORT &FBX, MA_EXPORT &MA);

void RMX_Audio_Locator (ifstream &rmxfile, string name, string room_name, string layer, ofstream &out, FBX_EXPORT &FBX, MA_EXPORT &MA);

void RMX_Portal (ifstream &rmxfile, string name, string room_name, string layer, ofstream &out, FBX_EXPORT &FBX, MA_EXPORT &MA);

class RMX_WaypointInfo {				// Dati di un waypoint necessari per costruire il grafo dopo aver letto tutte le stanze
public:
	uint32_t hash;						// Hash del nome del waypoint
	string name;						// Nome del locator esportato
	Vec3 pos;							// Posizione (coordinate del mondo)
	vector <uint32_t> links;			// Hash dei waypoint collegati
};

uint32_t RMX_Waypoint (ifstream &rmxfile, string room_name, string layer, ofstream &out, FBX_EXPORT &FBX, MA_EXPORT &MA, vector <RMX_WaypointInfo> &waypoints);

void RMX_Waypoint_Graph (const vector <RMX_WaypointInfo> &waypoints, string layer, string oneway_layer, MA_EXPORT &MA);

struct RMX_Actor {						// Record del database dei personaggi (ACTOR.DB)
	uint32_t ID;						// Identificativo (RMX_CHARLOC.ActorID)
	int Health;							// Salute iniziale (-1 = non uccidibile)
	uint32_t Type;						// 1 = Lara giocabile, 2 = non giocante / cutscene, 3 = nemico
	uint32_t BaseID;					// ID del modello base
	const char *Name;					// Nome del personaggio
};

const RMX_Actor *RMX_GetActor (uint32_t id);

void RMX_Charloc (ifstream &rmxfile, string room_name, string layer, string pawn_layer, ofstream &out, FBX_EXPORT &FBX, MA_EXPORT &MA, map <string, unsigned int> &name_count);

unsigned int Isolate_duplicated_lights (MA_EXPORT &MA);

unsigned int Isolate_duplicated_water (MA_EXPORT &MA);