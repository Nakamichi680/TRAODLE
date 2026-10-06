#include "stdafx.h"
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "TRAOD/RMX/RMX_Functions.h"
#include "hash_Functions.h"


/*------------------------------------------------------------------------------------------------------------------
Database dei personaggi del gioco (contenuto del file ACTOR.DB: uint32_t nActors + nActors record da 36 bytes).
Record originale [struct ActorDb in Tools\Db2Game\db2game.cpp, generato da "Character List.txt"]:
	ID			[nKey] Identificativo usato da RMX_CHARLOC.ActorID
	Health		[iLifeMax] Salute iniziale (-1 = personaggio non uccidibile)
	Type		[iClassID] 1 = player, 2 = neutral (personaggio non giocante / versione da cutscene), 3 = enemy, 4 = obj
	BaseID		[iCloneOf] ID del modello base (tutte le varianti di Lara hanno 22)
	Name		[resource] Nome della risorsa (16 caratteri), seguito da iFlags (1 = RAGDOLL_DEATH, 2 = RAGDOLL_HIT)
------------------------------------------------------------------------------------------------------------------*/
static const RMX_Actor Actors[] = {
	{ 99,     -1, 2,  99, "ANTON"},
	{ 48,   1000, 3,  48, "AQUATIC"},
	{141,    100, 3, 141, "LVARMED"},
	{ 27,    400, 3,  27, "ASSASSIN"},
	{148,     50, 3, 148, "BAT"},
	{ 44,     -1, 2,  44, "X_JANITO"},
	{152,    200, 3,  77, "BIO_NEF"},
	{123,    100, 3,  89, "BIOSUIT"},
	{ 53,   1000, 3,  53, "BOAZ_BIG"},
	{ 54,   1000, 3,  54, "BOAZ_FLY"},
	{ 55,     -1, 3,  55, "BOAZ_POD"},
	{140,    200, 3,  83, "SPLINTER"},
	{ 36,     -1, 2,  36, "BOUCHARD"},
	{ 51,     -1, 2,  51, "BOXER1"},
	{120,    500, 3, 120, "OBSCURA"},
	{ 32,    100, 3,  58, "CABAL_A"},
	{ 56,    100, 3,  58, "CABAL_B"},
	{136,     -1, 2,  51, "BOXER3"},
	{110,     -1, 2, 110, "CHAUFFER"},
	{ 40,     -1, 2,  40, "CHINAMAN"},
	{172,      0, 2, 172, "CS_GUARD"},
	{168,      0, 2, 168, "CS_COP_A"},
	{169,      0, 2, 168, "CS_COP_B"},
	{170,      0, 2, 168, "CS_COP_C"},
	{171,      0, 2, 168, "CS_COP_D"},
	{174,      0, 2, 174, "CS_STRAHOVA"},
	{ 84,     -1, 2,  84, "RENNES"},
	{ 39,     -1, 2,  39, "DOORMAN"},
	{155,     -1, 2, 155, "DOORVINE"},
	{ 62,     -1, 2,  62, "DRUGDEAL"},
	{ 23,   1000, 3,  23, "ECK"},
	{145,   1000, 3,  23, "ECK_FIRE"},
	{151,    100, 3, 151, "EEL"},
	{142,    256, 3,  69, "F_KNIGHT"},
	{150,     -1, 2, 150, "FISH"},
	{ 46,     -1, 2,  46, "FRANCINE"},
	{118,     -1, 3, 118, "GAS_BAG"},
	{ 79,     -1, 2,  79, "MULLER"},
	{ 65,     75, 3,  65, "DOG"},
	{ 31,    101, 3,  31, "GUNDERSO"},
	{ 43,     -1, 2,  43, "JANICE"},
	{ 68,     -1, 3,  68, "KAREL"},
	{122,    100, 3,  68, "KARELNEF"},
	{161,     -1, 3, 161, "KARELNC"},
	{ 47,     -1, 3,  47, "BOAZ"},
	{114,     -1, 2,  24, "KURTIS"},
	{ 24,    100, 1,  24, "KURTIS"},
	{ 72,    150, 3,  72, "LAB_1"},
	{ 73,    150, 2,  72, "LAB_2"},
	{113,     -1, 2,  22, "LARA"},
	{127,    100, 1,  22, "LARAC2"},
	{128,     -1, 2,  22, "LARAC2"},
	{125,    100, 1,  22, "LARAC1"},
	{126,     -1, 2,  22, "LARAC1"},
	{ 22,    100, 1,  22, "LARA"},
	{129,    100, 1,  22, "LARAD"},
	{130,     -1, 2,  22, "LARAD"},
	{131,    100, 1,  22, "LARAS"},
	{132,     -1, 2,  22, "LARAS"},
	{117,     -1, 3, 117, "LICKER"},
	{ 34,     -1, 4,  34, "BBC"},
	{ 30,    100, 3,  30, "GUARD"},
	{137,    100, 3,  30, "GUARDB"},
	{138,    100, 3,  30, "GUARDC"},
	{ 75,     -1, 2,  75, "LUDDICK"},
	{ 86,     -1, 2,  86, "ROUZIC"},
	{ 26,     -1, 2,  26, "MELTED"},
	{ 77,    250, 3,  77, "MENTAL_N"},
	{134,     -1, 2, 134, "MENTAL_C"},
	{ 78,    100, 3,  78, "MENTAL_P"},
	{133,    100, 3,  78, "MENTALPB"},
	{159,    100, 3,  78, "MENTALPC"},
	{135,    100, 3,  78, "MENTALPD"},
	{ 38,     -1, 2,  38, "CARVIER"},
	{ 80,     -1, 2,  80, "PIERRE"},
	{ 81,    200, 3,  81, "PODCREEP"},
	{164,    100, 3,  81, "PODCREPB"},
	{165,    100, 3,  81, "PODCREPC"},
	{ 58,    100, 3,  58, "COP_A"},
	{ 59,    100, 3,  58, "COP_B"},
	{ 60,    100, 3,  58, "COP_C"},
	{ 61,    100, 3,  58, "COP_D"},
	{156,    100, 3,  58, "COP_GAS"},
	{162,    100, 3,  58, "COP_DGAS"},
	{153,    128, 3,  69, "P_KNIGHT"},
	{ 82,   1000, 3,  82, "PROTO"},
	{ 83,     40, 3,  83, "RAT"},
	{111,     -1, 2, 111, "RAVEN"},
	{112,     -1, 4, 112, "ROPE"},
	{121,     -1, 3, 121, "SCUBA"},
	{158,     -1, 4, 158, "SCUBA_ST"},
	{143,    100, 3,  58, "AGENT_A"},
	{144,    100, 3,  58, "AGENT_B"},
	{ 25,     -1, 4,  25, "SHAMAN"},
	{ 87,    500, 3,  87, "SLEEPER"},
	{160,     -1, 4, 160, "SLROPE"},
	{ 88,     75, 3,  88, "SLINKY"},
	{154,     -1, 3, 151, "SMALLEEL"},
	{ 89,    150, 3,  89, "STRAHOVA"},
	{146,    100, 3,  89, "STRNMSKA"},
	{147,    100, 3,  89, "STRNMSKB"},
	{ 29,     -1, 4,  29, "STREET"},
	{139,     -1, 3, 139, "SWARMRAT"},
	{ 91,     -1, 2,  91, "TRAMP_A"},
	{ 92,     -1, 2,  91, "TRAMP_B"},
	{ 93,     -1, 2,  93, "TRAMPDOG"},
	{116,     -1, 3, 116, "TRIFFID"},
	{ 69,    128, 3,  69, "KNIGHT"},
	{ 94,     -1, 3,  94, "VEG_EGG"},
	{ 33,     -1, 4,  33, "VCNEIGH"},
	{ 95,     -1, 4,  95, "VONCROY"},
	{ 52,     -1, 2,  51, "BOXER2"},
};


// Restituisce il personaggio con l'ID indicato, nullptr se l'ID non e' presente nel database
const RMX_Actor *RMX_GetActor (uint32_t id)
{
	for (unsigned int i = 0; i < sizeof(Actors) / sizeof(Actors[0]); i++)
		if (Actors[i].ID == id)
			return &Actors[i];
	return nullptr;
}


// Aggiunge alla mappa hash del nome -> nome tutti i personaggi del database
void RMX_AddActorNames (map <uint32_t, string> &names)
{
	for (unsigned int i = 0; i < sizeof(Actors) / sizeof(Actors[0]); i++)
		names[(uint32_t)GetHashValue(Actors[i].Name)] = Actors[i].Name;
}


// Restituisce il personaggio il cui nome ha l'hash indicato (GetHashValue), nullptr se non presente nel database
const RMX_Actor *RMX_GetActorByHash (uint32_t hash)
{
	for (unsigned int i = 0; i < sizeof(Actors) / sizeof(Actors[0]); i++)
		if ((uint32_t)GetHashValue(Actors[i].Name) == hash)
			return &Actors[i];
	return nullptr;
}
