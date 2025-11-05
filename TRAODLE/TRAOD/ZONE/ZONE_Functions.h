#pragma once
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "TRAOD/ZONE/ZONE_Struct.h"

bool Export_ZONE (string filename);

bool GetRoomInfo (vector <RoomInfo> &RMX_Rooms);

bool CheckDoubleSided (unsigned short TextureMode, unsigned int DoubleSided);

bool CheckShadowMap (unsigned short TextureMode);

bool ZONE_Read_Rooms (string filename, vector <RoomInfo> RMX_Rooms, FBX_EXPORT &FBX, MA_EXPORT &MA);

bool ZONE_Read_Rooms_MT (string name1, string name2, FBX_EXPORT &FBX, MA_EXPORT &MA);

bool ZONE_Read_Fakes (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA);

bool ZONE_Read_Textures (string filename, vector <RoomInfo> RMX_Rooms, FBX_EXPORT &FBX, MA_EXPORT &MA);

bool ZONE_Read_Materials (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA);

Material ZONE_Read_Materials_Arnold(string zonename, bool diffuse_transparent, string material_name, const ZONE_MATERIALS_LIST& zone_materials_list);

Material ZONE_Read_Materials_MayaHw20(string zonename, bool diffuse_transparent, string material_name, const ZONE_MATERIALS_LIST& zone_materials_list);

vector <Face> Calculate_Faces (vector <unsigned int> strip, unsigned int Begin_position, unsigned int Draw_mode);