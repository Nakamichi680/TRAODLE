#pragma once
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

unsigned int Isolate_duplicated_lights (MA_EXPORT &MA);

unsigned int Isolate_duplicated_water (MA_EXPORT &MA);