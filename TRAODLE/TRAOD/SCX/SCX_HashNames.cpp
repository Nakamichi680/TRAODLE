#include "stdafx.h"
#include <map>
#include "Classes.h"
#include "hash_Functions.h"
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "TRAOD/RMX/RMX_Functions.h"
#include "TRAOD/CAL/CAL_Struct.h"
#include "TRAOD/CHR/CHR_Struct.h"
#include "TRAOD/SCX/SCX_Functions.h"


/*------------------------------------------------------------------------------------------------------------------
Nomi reali degli hash usati negli script. Gli script identificano animazioni, personaggi, bones e file con l'hash del
nome (GetHashValue): i nomi vengono ricavati dai file estratti dal GMX del livello e usati da AMX_Decompile (costanti
con nome) e da AMX_Disassemble (commenti).
Fonti, in ordine di priorita': nomi delle animazioni dei file CAL, nomi delle bones dei file CHR, personaggi del
database ACTOR.DB, nomi dei file contenuti nel GMX.
I nodi della mappa e i waypoint (mapFindHashedNodeTypedAMX, AddWaypoint, ...) restano hash: i loro nomi non sono
salvati in nessun file del gioco.
------------------------------------------------------------------------------------------------------------------*/
map <uint32_t, string> AMX_HashNames;


static void AddName (const string &name, uint32_t hash)
{
	if (name.empty() || hash < 0x10000 || AMX_HashNames.count(hash))		// Valori piccoli: troppo simili a costanti numeriche
		return;
	for (unsigned char c : name)
		if (c < 0x20 || c > 0x7E)
			return;
	AMX_HashNames[hash] = name;
}


static void AddName (const string &name)
{
	AddName(name, (uint32_t)GetHashValue(name.c_str()));
}


static void ReadCALNames (const string &filename)
{
	ifstream cal(filename, std::ios::binary);
	CAL_HEADER header;
	if (!cal.read(reinterpret_cast<char*>(&header), sizeof(header)) || header.Version != CAL_GAME_ANIMATION_VERSION || header.nAnims > 10000)
		return;
	vector <uint32_t> ptr(header.nAnims);
	cal.seekg(header.ANIM_LIST_PTR);
	if (header.nAnims && !cal.read(reinterpret_cast<char*>(ptr.data()), header.nAnims * sizeof(uint32_t)))
		return;
	vector <CAL_ANIMATION> anims;
	for (uint32_t a = 0; a < header.nAnims; a++)
	{
		CAL_ANIMATION anim;
		cal.seekg(ptr[a]);
		if (!cal.read(reinterpret_cast<char*>(&anim), sizeof(anim)))
			return;
		anim.name[63] = 0;
		AddName(anim.name);
		anims.push_back(anim);
	}
	for (const CAL_ANIMATION &anim : anims)				// animID diverso dall'hash del nome: copie rinominate (vedi CAL_Struct.h)
		AddName(anim.name, anim.animID);
}


static void ReadCHRNames (const string &filename)
{
	ifstream chr(filename, std::ios::binary);
	CHR_HEADER header;
	if (!chr.read(reinterpret_cast<char*>(&header), sizeof(header)) || header.CHR_MAGIC != 0 || header.nBONES > 1000)
		return;
	chr.seekg(header.SKELETON_PTR);
	for (uint32_t b = 0; b < header.nBONES; b++)
	{
		CHR_BONE bone;
		if (!chr.read(reinterpret_cast<char*>(&bone), sizeof(bone)))
			return;
		bone.Bone_name[63] = 0;
		AddName(bone.Bone_name);
	}
}


void SCX_CollectHashNames ()
{
	AMX_HashNames.clear();
	for (const auto &f : AOD_IO.gmxfiles)
		if (f.type == AoDFileType::CAL)
			ReadCALNames(f.name);
	for (const auto &f : AOD_IO.gmxfiles)
		if (f.type == AoDFileType::CHR)
			ReadCHRNames(f.name);
	map <uint32_t, string> actors;
	RMX_AddActorNames(actors);
	for (const pair <const uint32_t, string> &a : actors)
		AddName(a.second, a.first);
	for (const auto &f : AOD_IO.gmxfiles)
		AddName(f.name);
	msg(msg::TGT::FILE, msg::TYP::LOG) << AMX_HashNames.size() << " hash names collected for scripts.";
}


string AMX_HashName (int32_t value)
{
	map <uint32_t, string>::const_iterator it = AMX_HashNames.find((uint32_t)value);
	return it == AMX_HashNames.end() ? "" : it->second;
}
