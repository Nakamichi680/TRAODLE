#include "stdafx.h"
#include "Misc_Functions.h"
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "TRAOD/RMX/RMX_Struct.h"
#include "TRAOD/RMX/RMX_Functions.h"


// Dimensioni della pedina (unita' del gioco) e direzione frontale di un personaggio con rotazione 0
static const float PAWN_HEIGHT = 1000;				// Lara (LARAC2.CHR) e' alta circa 1080 unita'
static const float PAWN_RADIUS = 200;
static const float PAWN_NOSE = 250;
static const Vec3 CHARLOC_FORWARD(0, -1, 0);			// Asse verso cui guarda un personaggio con Zrot = 0 (verificato in Maya)


/*------------------------------------------------------------------------------------------------------------------
Legge una posizione di partenza di un personaggio (il cursore di lettura deve essere all'inizio del nodo) e la esporta
nei file FBX e MA come locator (posizione e rotazione) con una pedina figlia che ne mostra l'orientamento.
Il nome del locator e' quello del personaggio (database ACTOR.DB), numerato per renderlo unico nel livello.
------------------------------------------------------------------------------------------------------------------*/
void RMX_Charloc (ifstream &rmxfile, string room_name, string layer, string pawn_layer, ofstream &out, FBX_EXPORT &FBX, MA_EXPORT &MA, map <string, unsigned int> &name_count)
{
	RMX_CHARLOC rmx_charloc;
	rmxfile.read(reinterpret_cast<char*>(&rmx_charloc), sizeof(rmx_charloc));

	const RMX_Actor *actor = RMX_GetActor(rmx_charloc.ActorID);
	string actor_name = actor ? actor->Name : "ACTOR" + to_string(rmx_charloc.ActorID);
	if (!actor)
		msg(msg::TGT::FILE, msg::TYP::WARN) << "Unknown actor ID " << rmx_charloc.ActorID << " in " << room_name << ".";
	stringstream ssname;
	ssname << AOD_IO.levelname << "_CHAR_" << actor_name << "_" << name_count[actor_name]++;
	string name = ssname.str();

	// Esportazione informazioni personaggio nel file TXT
	out << "	Name: " << name << endl;
	out << "	Hashed name: " << hex << rmx_charloc.Node.Hash << dec << endl;
	out << "	Actor ID: " << rmx_charloc.ActorID;
	if (actor)
		out << " (" << actor->Name << ", health " << actor->Health << ", type " << actor->Type << ")";
	out << endl;
	out << "	Character hash: " << hex << rmx_charloc.CharHash << dec << endl;
	out << "	Flags: " << rmx_charloc.Flags1 << " " << rmx_charloc.Flags2 << endl;
	out << left << setw(30) << "	Position [X/Y/Z]:" << right << setw(13) << rmx_charloc.Node.Xpos << setw(13) << rmx_charloc.Node.Ypos << setw(13) << rmx_charloc.Node.Zpos << endl;
	out << left << setw(30) << "	Rotation [X/Y/Z]:" << right << setw(13) << rmx_charloc.Node.Xrot << setw(13) << rmx_charloc.Node.Yrot << setw(13) << rmx_charloc.Node.Zrot << endl;

	// Locator: posizione e rotazione del personaggio
	Locator FBX_MA_Charloc;
	FBX_MA_Charloc.name = name;
	FBX_MA_Charloc.parent = room_name;
	FBX_MA_Charloc.layer = layer;										// Nome layer di appartenenza (solo file MA)
	FBX_MA_Charloc.FBX_parent = hashID(room_name, "Group");
	FBX_MA_Charloc.translate_flag = true;
	FBX_MA_Charloc.rotate_flag = true;
	FBX_MA_Charloc.tX = rmx_charloc.Node.Xpos;
	FBX_MA_Charloc.tY = rmx_charloc.Node.Ypos;
	FBX_MA_Charloc.tZ = rmx_charloc.Node.Zpos;
	FBX_MA_Charloc.rX = rmx_charloc.Node.Xrot;
	FBX_MA_Charloc.rY = rmx_charloc.Node.Yrot;
	FBX_MA_Charloc.rZ = rmx_charloc.Node.Zrot;
	FBX.Locator.push_back(FBX_MA_Charloc);
	MA.Locator.push_back(FBX_MA_Charloc);

	// Pedina figlia del locator. Colore: verde = Lara giocabile, giallo = non giocante, rosso = nemico, grigio = sconosciuto
	unsigned int color = 0xFF808080;
	if (actor)
		color = (actor->Type == 1) ? 0xFF20D020 : (actor->Type == 2) ? 0xFFE0C020 : 0xFFE03020;
	Mesh pawn = DrawPawn(name + "_PAWN", name, pawn_layer, PAWN_HEIGHT, PAWN_RADIUS, PAWN_NOSE, CHARLOC_FORWARD, color);
	pawn.FBX_parent = hashID(name, "Locator");
	FBX.Geometry.push_back(pawn);
	MA.Mesh.push_back(pawn);
}
