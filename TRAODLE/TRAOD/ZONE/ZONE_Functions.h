#pragma once
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "TRAOD/ZONE/ZONE_Struct.h"
#include <map>
#include <set>

bool Export_ZONE (string filename);

bool GetRoomInfo (vector <RoomInfo> &RMX_Rooms);

bool CheckDoubleSided (unsigned short TextureMode, unsigned int DoubleSided);

bool CheckShadowMap (unsigned short TextureMode);

bool CheckFur (unsigned short TextureMode);

bool ZONE_Read_Rooms (string filename, vector <RoomInfo> RMX_Rooms, FBX_EXPORT &FBX, MA_EXPORT &MA);

bool ZONE_Read_Rooms_MT (string name1, string name2, FBX_EXPORT &FBX, MA_EXPORT &MA);

bool ZONE_Read_Fakes (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA);

bool ZONE_Read_Items (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA);

bool ZONE_Read_Textures (string filename, vector <RoomInfo> RMX_Rooms, FBX_EXPORT &FBX, MA_EXPORT &MA);

bool ZONE_Read_Materials (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA);

Material ZONE_Read_Materials_Arnold(string zonename, bool diffuse_transparent, string material_name, const ZONE_MATERIALS_LIST& zone_materials_list);

Material ZONE_Read_Materials_MayaHw20(string zonename, bool diffuse_transparent, string material_name, const ZONE_MATERIALS_LIST& zone_materials_list);

vector <Face> Calculate_Faces (vector <unsigned int> strip, unsigned int Begin_position, unsigned int Draw_mode);

// Oggetti del file ZONE (ZONE_Objects.cpp)
class ZONE_SlotCommand {						// Comando della lista di un oggetto (slot): -2 push, -3 pop, -4 trasformazione, >= 0 mesh
public:
	int type = -3;
	float value[6] = {0, 0, 0, 0, 0, 0};		// -4: traslazione X Y Z + rotazione X Y Z in gradi
};

class ZONE_ObjectData {							// Oggetti di un file ZONE
public:
	string zonename;							// Estensione della zona (es. Z00)
	ZONE_HEADER header;
	vector <streamoff> mesh_offsets;			// Offset di ogni mesh degli oggetti (ZONE_MESH_OBJECT_HEADER)
	vector <uint32_t> mesh_vertices;			// Numero di vertici PC di ogni mesh (0: collisioni e copie)
	vector <int> mesh_base;					// Mesh copie di un'altra mesh: indice della mesh base (ZONE_OBJECT_INFO.BaseIndex), altrimenti -1
	vector <uint32_t> mesh_collision;			// Collisioni: offset di ZONE_OBJECT_COLLISION, altrimenti 0
	vector <uint32_t> mesh_hash;				// Hash del nome di ogni mesh (ZONE_OBJECT_INFO.Hash)
	vector <int> mesh_link;					// Collisione della mesh (ZONE_OBJECT_INFO.CollisionIndex), altrimenti -1
	vector <uint32_t> mesh_flags;				// ZONE_OBJECT_INFO.Flags (ZONE_MESHFLAG_*)
	vector <uint32_t> mesh_alias;				// ZONE_OBJECT_INFO.ParentHash (AliasHash, con ZONE_MESHFLAG_ALIAS)
	vector <uint32_t> mesh_broken;				// ZONE_OBJECT_INFO.Unknown2 (nBroken)
	vector <vector <ZONE_SlotCommand>> slots;	// Comandi di ogni slot (vuoto = slot vuoto)
};

class ZONE_ObjectExport {						// Gruppi e mesh di un oggetto esportato
public:
	vector <Transform> groups;
	vector <Mesh> meshes;
	bool bb_empty = true;
	Vec3 bbmin, bbmax;							// Bounding box nello spazio del gruppo radice
	void Add (const Vec3 &p);
	set <string> collision_attributes;		// Attributi delle collisioni (materiali ColMat_<attributo>)
	void PushTo (FBX_EXPORT &FBX, MA_EXPORT &MA) const;
};

class ZONE_Instance {							// Istanza di un oggetto (RMX_PS2_ROOM_OBJ o RMX_OBJECT)
public:
	unsigned int room;							// Indice della stanza nel file RMX
	float pos[3], rot[3], scl[3];				// Trasformazione nel mondo (rotazione XYZ in gradi)
	unsigned int slot;							// Slot dell'oggetto
	int zone;									// Indice della zona in cui viene esportata
	bool interactive = false;					// Oggetto interattivo (RMX_OBJECT)
	unsigned int object_index = 0;				// RMX_OBJECT.ObjectIndex
	uint32_t obj_flags = 0;						// RMX_OBJECT.ObjFlags
	unsigned int db_key = 0;					// RMX_OBJECT.absDBKey
	unsigned int n_elements = 0;				// RMX_OBJECT.nElements
	uint32_t hash = 0;							// Hash del nome del nodo RMX
};

class ZONE_LevelObjects {						// Oggetti dell'intero livello
public:
	vector <ZONE_Instance> instances;			// Istanze esportabili
	set <unsigned int> used_slots;				// Slot usati da istanze o Fakes
	map <unsigned int, int> item_zone;			// Slot non usati -> zona in cui esportarli come Items
	unsigned int nTemplates = 0;				// Numero di Object Template (massimo RMX_OBJECT.ObjectIndex + 1)
};

MATRIX ZONE_Matrix (const float t[3], const float r[3], const float s[3]);

bool ZONE_ReadObjectData (ifstream &zonefile, string filename, ZONE_ObjectData &data);

bool ZONE_ExportMesh (ifstream &zonefile, const ZONE_ObjectData &data, unsigned int mesh, string name, string parent, string layer, const MATRIX &matrix, ZONE_ObjectExport &out);

bool ZONE_ExportCollision (ifstream &zonefile, const ZONE_ObjectData &data, unsigned int mesh, string name, string parent, string layer, const MATRIX &matrix, ZONE_ObjectExport &out);

bool ZONE_ExportSlot (ifstream &zonefile, const ZONE_ObjectData &data, unsigned int slot, string root, string layer, ZONE_ObjectExport &out);

vector <pair <int, MATRIX>> ZONE_SlotMeshes (const ZONE_ObjectData &data, unsigned int slot);

bool ZONE_ObjectMaterials (string filename, MA_EXPORT &MA);

const ZONE_LevelObjects &ZONE_GetLevelObjects ();

bool ZONE_Read_Instances (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA);
