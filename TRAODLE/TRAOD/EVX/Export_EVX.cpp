#include "stdafx.h"
#include "Classes.h"
#include "TRAOD/SCX/SCX_Functions.h"
#include "TRAOD/EVX/EVX_Functions.h"
#include "TRAOD/RMX/RMX_Struct.h"
#include "TRAOD/RMX/RMX_Functions.h"
#include <cstddef>


// Code dei messaggi dei nodi (RMX_MSG): contatore e inizio della coda per tipo di nodo [LOG]
static_assert(offsetof(RMX_OBJECT, nMessages) == 372, "RMX_OBJECT.nMessages");
static_assert(offsetof(RMX_TRIGGER, nActions) == 388 && offsetof(RMX_TRIGGER, nConditions) == 404, "RMX_TRIGGER");
static_assert(offsetof(RMX_WATER, nMessages) == 416, "RMX_WATER.nMessages");
static_assert(offsetof(RMX_EMITTER, nMessages) == 408, "RMX_EMITTER.nMessages");


class EVX_Binding {							// Nodo che avvia un gruppo di eventi quando riceve un messaggio (RMX_MSG)
public:
	uint32_t node;							// Hash del nodo
	uint32_t message;						// Messaggio NM_* ricevuto
};


/*------------------------------------------------------------------------------------------------------------------
Messaggi del sistema di messaggistica della mappa (enum NM_* di Tools\Exporter\map_msg.h, nello stesso ordine) con la
descrizione originale. "script X:" indica la funzione degli script SCX chiamata dal messaggio.
------------------------------------------------------------------------------------------------------------------*/
static const struct { const char *name, *description; } EVX_Messages[] = {
	{"NM_INIT", "script OnInit: One time node init"},
	{"NM_USERINIT", "script OnUserInit: Extra init functions for scripts..."},
	{"NM_DAMAGE", "script OnDamage: Receives damage"},
	{"NM_DESTROY", "script OnDestroy: Destroyed (TODO: Suss interaction with OnDamage)"},
	{"NM_ACTORCOLLIDE", "script OnActorCollide: Actor collision - triggers use this too!"},
	{"NM_ACTIVATE", "script OnActivate: Becomes active - game state (as opposed to visual / process state)"},
	{"NM_UPDATE", "script OnUpdate: Every frame"},
	{"NM_MAXOVERRIDES", ""},
	{"NM_CHARENTRY", "Character enters box (for cameras, at the mo)"},
	{"NM_CHAREXIT", "Character leaves box"},
	{"NM_ANIMATE", ""},
	{"NM_DRAW", ""},
	{"NM_RELEASE", ""},
	{"NM_FXTRIG", "FX trigger event"},
	{"NM_LADDER", "Ladder trigger event"},
	{"NM_INITRESOURCE", "Initialize resources for something...."},
	{"NM_PREUPDATE", "Pre update message"},
	{"NM_POSTUPDATE", "Post update message"},
	{"NM_WINDOW", "Window trigger event"},
	{"NM_NUDGEPOSX", "Nudge node in direction"},
	{"NM_NUDGEPOSY", ""},
	{"NM_NUDGENEGX", "Nudge node in direction"},
	{"NM_NUDGENEGY", ""},
	{"NM_CONVERSATIONSTART", "Converstion Starts (for cameras, at the mo)"},
	{"NM_CONVERSATIONEND", "Conversation Ends"},
	{"NM_OPEN", "Open trigger event"},
	{"NM_CAMERAENTRY", "Camera enters box"},
	{"NM_CAMERAEXIT", "Camera exits box"},
	{"NM_OBJCOLLIDE", "Something has hit an object"},
	{"NM_PICKUP", "Something has hit a pickup"},
	{"NM_REMOVE", "Remove the object from the world"},
	{"NM_SETOBJECTFLAGS", "Set the specified ObjFlags bits of a MAP_OBJNODE (OF_XXX)"},
	{"NM_CLEAROBJECTFLAGS", "Clear the specified flags"},
	{"NM_PLAYERCOLLIDE", "Player collides"},
	{"NM_BGCOLLIDE", "Someting collides with background"},
	{"NM_STARTTIMER", "Start node timer (not implemented yet)"},
	{"NM_STARTTIMERRANGE", "Start node timer with min max random range"},
	{"NM_TIMEREVENT", "Node timer times out (not implemented)"},
	{"NM_PIPESWING", "Pipeswing trigger (like ladder trigger)"},
	{"NM_WALLCLIMB", "Wall climb trigger (like ladder trigger and pipeswing trigger!)"},
	{"NM_DISPATCHEVENT", "Dispatch an event group (for triggers only!)"},
	{"NM_STARTPAUSETIMER", "Start a pause timer , and hold the event list pointer until the timer times out"},
	{"NM_STARTPAUSETIMERRANGE", "As above, but with min max random range"},
	{"NM_ENDPAUSELIST", "A paused list has finished (can also be used to halt paused list)"},
	{"NM_PLAYEROPEN", "Pre open event - player must get into position first!"},
	{"NM_CLOSE", "Close event"},
	{"NM_ANIMFINISH", "Anim finish event"},
	{"NM_PUZZLECLEAR", "Puzzle cleared event"},
	{"NM_USEPUZZLE_A", "Puzzle item used on node"},
	{"NM_USEPUZZLE_B", "Puzzle item used on node"},
	{"NM_USEPUZZLE_C", "Puzzle item used on node"},
	{"NM_USEPUZZLE_D", "Puzzle item used on node"},
	{"NM_UNLOCK", "Guess"},
	{"NM_PLY_OBJEVENT", "Object event signals player control"},
	{"NM_SWITCHON", "As OPEN , PLAYEROPEN , CLOSE"},
	{"NM_SWITCHOFF", ""},
	{"NM_PLAYERSWITCHON", ""},
	{"NM_SWITCHEDON", "Sent to object when switch on / off sequence finishes"},
	{"NM_SWITCHEDOFF", ""},
	{"NM_UPGRADE_UPPERBODY", "Upgrades - uParam is ABSOLUTE upgrade level"},
	{"NM_UPGRADE_LOWERBODY", ""},
	{"NM_UPGRADE_GRIP", ""},
	{"NM_UPGRADE_JUMP", ""},
	{"NM_TAKE", "Take (replaces search, as search is combined with open)"},
	{"NM_TAKEFINISH", "Take has finished (on NM_ANIMFINISH)"},
	{"NM_GIVEITEM", "Give item to player"},
	{"NM_PLAYERUNLOCK", "Make player do unlock anim"},
	{"NM_DEACTIVATE", "Clear NF_ACTIVE flag"},
	{"NM_OPENFINISH", "Open has finished (on NM_ANIMFINISH)"},
	{"NM_CLOSEFINISH", "Close has finished (on NM_ANIMFINISHED)"},
	{"NM_VOICEPROMPT", "Voice prompt"},
	{"NM_STARTWATERRAISE", "Start water node surface raising"},
	{"NM_STARTWATERLOWER", "Start water node surface lowering"},
	{"NM_STOPWATER", "Stop  water node surface"},
	{"NM_SHOWWATER", "Make water visible"},
	{"NM_HIDEWATER", "Make water invisible"},
	{"NM_SETPUZZLECOUNT", "Set the puzzle count on an object"},
	{"NM_FLIPROOM_RESET", "Flip to 'initial' room"},
	{"NM_FLIPROOM0", "Flip to first flip map"},
	{"NM_FLIPROOM1", "And so on..."},
	{"NM_FLIPROOM2", ""},
	{"NM_FLIPROOM3", ""},
	{"NM_PLAYANIM", "Play named anim"},
	{"NM_SETROTX", "Set angles , integer values in degrees"},
	{"NM_SETROTY", ""},
	{"NM_SETROTZ", ""},
	{"NM_USER0", "Custom user messges - use them for owt!"},
	{"NM_USER1", ""},
	{"NM_USER2", ""},
	{"NM_USER3", ""},
	{"NM_USER4", ""},
	{"NM_USER5", ""},
	{"NM_USER6", ""},
	{"NM_USER7", ""},
	{"NM_DAMAGESTAGE", ""},
	{"NM_LIFTPLATFORM", "Character / node is on moving platform trigger"},
	{"NM_TIMEDACTIVATE", "Activate a node for a set time"},
	{"NM_PLAYANIM_SLOT0", "Play anims in door / switch slot 0 -> 3 (maps to some of the open anims)"},
	{"NM_PLAYANIM_SLOT1", ""},
	{"NM_PLAYANIM_SLOT2", ""},
	{"NM_PLAYANIM_SLOT3", ""},
	{"NM_DEFLECT", "Deflect a node"},
	{"NM_ENDLEVEL", "End the level"},
	{"NM_ANIMEVENT0", "Animation event"},
	{"NM_ANIMEVENT1", ""},
	{"NM_ANIMEVENT2", ""},
	{"NM_ANIMEVENT3", ""},
	{"NM_ANIMEVENT4", ""},
	{"NM_ANIMEVENT5", ""},
	{"NM_ANIMEVENT6", ""},
	{"NM_ANIMEVENT7", ""},
	{"NM_CUEMUSIC", "Cue audio"},
	{"NM_SAVESTATUS", "?     Save status (save game)"},
	{"NM_LOADSTATUS", "?     Load status (load game)"},
	{"NM_ANIMFINISH_SLOT0", "Sent when anim on slot 0-3 finishes"},
	{"NM_ANIMFINISH_SLOT1", ""},
	{"NM_ANIMFINISH_SLOT2", ""},
	{"NM_ANIMFINISH_SLOT3", ""},
	{"NM_PUSHEVENT", "Sent to object during push/pull stuff"},
	{"NM_TIMEDREMOVE", "Remove object on time out"},
	{"NM_ACTIVATEONTIMER", "Activate object on time out"},
	{"NM_CUTSCENE", ""},
	{"NM_HEADLOOK", ""},
	{"NM_LEDGESHIMMY", ""},
	{"NM_STARTAUDIOEVENT", ""},
	{"NM_STOPAUDIOEVENT", ""},
	{"NM_AUDIOEVENTFADE", "Fade to uParam volume"},
	{"NM_LOCKPLAYER", "Lock player control"},
	{"NM_UNLOCKPLAYER", ""},
	{"NM_HARMPLAYER", ""},
	{"NM_ZONESWITCH", "Normal zone switch (same as charentry -> zonemanager)"},
	{"NM_ZONESWITCH_DOOR", "As above, but on door..."},
	{"NM_DISABLETRIGGER", "Disable action triggers"},
	{"NM_ENABLETRIGGER", "Enable    '      '"},
	{"NM_HIDEOBJECT", "Hide + disable process on object"},
	{"NM_SHOWOBJECT", ""},
	{"NM_CUTSCENE_END", "Sent to cutscene trigger when a cutscene ends"},
	{"NM_SETXVEL", "Set object velocities"},
	{"NM_SETYVEL", ""},
	{"NM_SETZVEL", ""},
	{"NM_SETVELDECAY", "Set velocity decay rate"},
	{"NM_LIFTSIGNAL", "Signal lift area"},
	{"NM_INCREMENT_UVAR", "Increment uParam user value"},
	{"NM_BLAST", "Radial blast"},
	{"NM_PLAYANIMSLOT_UVAR", "Play anim in slot referenced by userVar[uParam]"},
	{"NM_PLAYANIMSLOT", "Play anim in slot referenced by uParam"},
	{"NM_ANIMFINISH_SLOT4", "Sent when anim on slot 4-11 finishes"},
	{"NM_ANIMFINISH_SLOT5", ""},
	{"NM_ANIMFINISH_SLOT6", ""},
	{"NM_ANIMFINISH_SLOT7", ""},
	{"NM_ANIMFINISH_SLOT8", ""},
	{"NM_ANIMFINISH_SLOT9", ""},
	{"NM_ANIMFINISH_SLOT10", ""},
	{"NM_ANIMFINISH_SLOT11", ""},
	{"NM_CLEAR_UVAR", "Clear userVariable[uParam]"},
	{"NM_INVALIDATE_UVAR", "Set userVariable[uParam] to -1"},
	{"NM_INVISIBLE", "Send to character, make it invisible (to the game LOGIC - not to the eye...)"},
	{"NM_DIRECT_ANIMEVENT0", "Animation event - sent directly to pre set target node"},
	{"NM_DIRECT_ANIMEVENT1", ""},
	{"NM_DIRECT_ANIMEVENT2", ""},
	{"NM_DIRECT_ANIMEVENT3", ""},
	{"NM_DIRECT_ANIMEVENT4", ""},
	{"NM_DIRECT_ANIMEVENT5", ""},
	{"NM_DIRECT_ANIMEVENT6", ""},
	{"NM_DIRECT_ANIMEVENT7", ""},
	{"NM_PLAYANIMSLOT_UVAR_OFFSET", "16:16 parameter - uVar:Offset - plays animslot userVar[uParam] + offset"},
	{"NM_WARP_PLAYER", "Warp player to another node pos"},
	{"NM_SETWATER_AMP", "Set amplitude on dynamic water"},
	{"NM_SETWATER_FREQ", "Set frequency on dynamic water"},
	{"NM_AUDIO_DEACTIVATE_CHAIN", "Something nigel asked for..."},
	{"NM_DECREMENT_UVAR", "Decrement uParam user value"},
	{"NM_UVAR_MAX", "Clip a uvar to max value (16:16 uVar:value)"},
	{"NM_UVAR_MIN", "Clip a uvar to min value (16:16 uVar:value)"},
	{"NM_SETNODEFLAGS", ""},
	{"NM_CLEARNODEFLAGS", ""},
	{"NM_PRECUTSCENE", "Sent to a trigger just before cutscene plays"},
	{"NM_INSTANT_CLOSE", "Send to a door - close instantly"},
	{"NM_DIARY_ENTRY", ""},
};
static const unsigned int EVX_nMessages = sizeof(EVX_Messages) / sizeof(EVX_Messages[0]);


string EVX_MessageName (uint32_t id)
{
	return id < EVX_nMessages ? string(EVX_Messages[id].name) : "MESSAGE_" + to_string(id);
}


/*------------------------------------------------------------------------------------------------------------------
Indice dei nodi della mappa (file <LIVELLO>.RMX, vedi RMX_Struct.h): per ogni hash il tipo di nodo, la stanza e la
posizione, per descrivere i destinatari dei messaggi (i nomi dei nodi non sono salvati nei file del gioco).
In bindings vengono raccolti, per indice del gruppo di eventi, i nodi che lo avviano: code RMX_MSG (messaggio ricevuto,
indice del gruppo) di oggetti, trigger (dopo le azioni), volumi d'acqua (16 bytes per messaggio) ed emitter.
------------------------------------------------------------------------------------------------------------------*/
static map <uint32_t, string> EVX_ReadNodeIndex (map <uint32_t, vector <EVX_Binding>> &bindings)
{
	static const char *types[] = {"OBJECT", "LIGHT", "CHARLOC", "TRIGGER", "PORTAL", "WAYPOINT", "ACTOR", "WATER", "EMITTER",
								  "CAMERA", "PS2_ROOM_OBJ", "AUDIO_LOCATOR", "ROOM"};
	map <uint32_t, string> index;
	ifstream rmxfile(AOD_IO.levelname + ".RMX", std::ios::binary);
	vector <uint8_t> data((istreambuf_iterator<char>(rmxfile)), istreambuf_iterator<char>());
	RMX_HEADER header;
	if (data.size() < sizeof(header))
		return index;
	memcpy(&header, data.data(), sizeof(header));
	for (uint32_t r = 0; r < header.nRooms && sizeof(RMX_HEADER) + 4 * (r + 1) <= data.size(); r++)
	{
		uint32_t room_offset;
		memcpy(&room_offset, &data[sizeof(RMX_HEADER) + 4 * r], 4);
		RMX_ROOM room;
		if ((size_t)room_offset + sizeof(room) > data.size())
			continue;
		memcpy(&room, &data[room_offset], sizeof(room));
		stringstream room_text;
		room_text << "ROOM " << r;
		index[room.Room_ID] = room_text.str();
		const uint32_t *lists = &room.NL_OBJ_First;					// Coppie (primo, ultimo) per tipo di nodo
		for (uint32_t t = 0; t < 12; t++)
		{
			uint32_t node = lists[2 * t];
			for (unsigned int guard = 0; node && guard < 100000; guard++)
			{
				RMX_NODE_HEADER h;
				if ((size_t)room_offset + node + sizeof(h) > data.size())
					break;
				memcpy(&h, &data[room_offset + node], sizeof(h));
				stringstream text;
				text << (h.Type < 13 ? types[h.Type] : "NODE") << ", room " << r << ", pos " << fixed << setprecision(0) << h.Xpos << " " << h.Ypos << " " << h.Zpos;
				if (h.Type == 2 && (size_t)room_offset + node + sizeof(RMX_CHARLOC) <= data.size())	// Personaggio (ACTOR.DB)
				{
					RMX_CHARLOC charloc;
					memcpy(&charloc, &data[room_offset + node], sizeof(charloc));
					const RMX_Actor *actor = RMX_GetActor(charloc.ActorID);
					if (actor)
						text << ", " << actor->Name;
				}
				if (!index.count(h.Hash))
					index[h.Hash] = text.str();
				size_t o = (size_t)room_offset + node;					// Coda dei messaggi
				uint32_t count = 0, tail = 0, stride = sizeof(RMX_MSG);
				auto Get = [&](size_t offset) { uint32_t v = 0; if (o + offset + 4 <= data.size()) memcpy(&v, &data[o + offset], 4); return v; };
				if (h.Type == 0)
				{
					count = Get(offsetof(RMX_OBJECT, nMessages));
					tail = sizeof(RMX_OBJECT);
				}
				else if (h.Type == 3)
				{
					count = Get(offsetof(RMX_TRIGGER, nConditions));
					tail = sizeof(RMX_TRIGGER) + sizeof(RMX_TRIGGER_ACTION) * min(Get(offsetof(RMX_TRIGGER, nActions)), 1000u);
				}
				else if (h.Type == 7)
				{
					count = Get(offsetof(RMX_WATER, nMessages));
					tail = sizeof(RMX_WATER);
					stride = 16;
				}
				else if (h.Type == 8)
				{
					count = Get(offsetof(RMX_EMITTER, nMessages));
					tail = sizeof(RMX_EMITTER);
				}
				for (uint32_t m = 0; m < count && m < 100 && o + tail + stride * (m + 1) <= data.size(); m++)
				{
					RMX_MSG message;
					memcpy(&message, &data[o + tail + stride * m], sizeof(message));
					bindings[message.Param].push_back({h.Hash, message.MsgID});
				}
				if (h.BegNext <= node)
					break;
				node = h.BegNext;
			}
		}
	}
	return index;
}


static string EVX_Hash (uint32_t hash)					// Nome dell'hash (dizionario degli script) o valore esadecimale
{
	stringstream ss;
	ss << "0x" << hex << uppercase << setw(8) << setfill('0') << hash;
	string name = AMX_HashName((int32_t)hash);
	return name.empty() ? ss.str() : name + " (" + ss.str() + ")";
}


/*------------------------------------------------------------------------------------------------------------------
Esporta i gruppi di eventi di un file EVX (vedi EVX_Struct.h) in un file di testo leggibile nella cartella
\NOMELIVELLO\Scripts (<NOME>_EVENTS.txt): per ogni gruppo i nodi della mappa che lo avviano (messaggio ricevuto) e
l'elenco dei messaggi inviati, con destinatario e parametro.
Gli hash vengono sostituiti dai nomi del dizionario degli script (SCX_CollectHashNames) quando sono noti; i destinatari
che sono nodi della mappa (file RMX) sono descritti con tipo, stanza e posizione.
------------------------------------------------------------------------------------------------------------------*/
bool Export_EVX (string filename)
{
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Reading " << filename;
	SetCurrentDirectory(AOD_IO.folder_level_lpwstr);			// \NOMELIVELLO
	vector <uint8_t> data;
	{
		ifstream evxfile(filename, std::ios::binary);
		if (!evxfile.is_open())
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " not found.";
			return false;
		}
		data.assign(istreambuf_iterator<char>(evxfile), istreambuf_iterator<char>());
	}
	EVX_HEADER header;
	if (data.size() < sizeof(header))
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " is not a valid EVX file.";
		return false;
	}
	memcpy(&header, data.data(), sizeof(header));
	if (header.Version != 8 || sizeof(EVX_HEADER) + (size_t)header.nGroups * sizeof(EVX_GROUP) > data.size())
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " is not a valid EVX file.";
		return false;
	}
	SCX_CollectHashNames();										// Dizionario dei nomi degli hash
	map <uint32_t, vector <EVX_Binding>> bindings;				// Nodi che avviano ogni gruppo
	map <uint32_t, string> nodes = EVX_ReadNodeIndex(bindings);	// Nodi della mappa (destinatari)
	auto Target = [&](uint32_t hash) -> string
	{
		string text = EVX_Hash(hash);
		map <uint32_t, string>::const_iterator node = nodes.find(hash);
		return node == nodes.end() ? text : text + " {" + node->second + "}";
	};

	stringstream out;
	string name = filename.substr(0, filename.find('.'));
	out << "// Gruppi di eventi del livello " << AOD_IO.levelname << " (" << filename << "): " << header.nGroups << " gruppi\n";
	out << "// Ogni gruppo invia i suoi messaggi in ordine quando viene attivato (messaggio di un oggetto a cui e' legato o NM_DISPATCHEVENT).\n";
	out << "// Formato: messaggio -> destinatario [parametro]. <self> = il nodo a cui e' legato il gruppo. Hash senza nome: nomi non salvati nel gioco.\n";
	out << "// Le righe \"<=\" sono i nodi della mappa che avviano il gruppo quando ricevono il messaggio indicato (code RMX_MSG del file RMX).\n\n";
	bool ok = true;
	unsigned int nMessages = 0, nBindings = 0;
	for (uint32_t g = 0; g < header.nGroups; g++)
	{
		EVX_GROUP group;
		memcpy(&group, &data[sizeof(EVX_HEADER) + g * sizeof(EVX_GROUP)], sizeof(group));
		uint32_t count = 0;
		if ((size_t)group.Offset + 4 > data.size())
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << ": event group " << g << " out of file.";
			ok = false;
			continue;
		}
		memcpy(&count, &data[group.Offset], 4);
		if ((size_t)group.Offset + 4 + (size_t)count * sizeof(EVX_MESSAGE) > data.size())
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << ": event group " << g << " out of file.";
			ok = false;
			continue;
		}
		out << "EVENT GROUP " << EVX_Hash(group.Hash) << "\n";
		for (const EVX_Binding &b : bindings[g])					// Nodi che avviano il gruppo
		{
			out << "  <= " << left << setw(28) << (b.message < EVX_nMessages ? string(EVX_Messages[b.message].name) : "MESSAGE_" + to_string(b.message));
			out << " from " << Target(b.node) << "\n";
			nBindings++;
		}
		for (uint32_t m = 0; m < count; m++)
		{
			EVX_MESSAGE message;
			memcpy(&message, &data[group.Offset + 4 + m * sizeof(EVX_MESSAGE)], sizeof(message));
			uint32_t id = message.Message & EVX_MSG_MASK;
			stringstream line;
			line << "    ";
			if (id < EVX_nMessages)
				line << left << setw(28) << EVX_Messages[id].name;
			else
				line << "MESSAGE_" << left << setw(20) << id;
			line << " -> " << ((message.Message & EVX_MSG_SELF) && message.Target == 0 ? string("<self>") : Target(message.Target));
			if (message.Param)
			{
				string param_name = AMX_HashName((int32_t)message.Param);
				line << "   [" << (param_name.empty() ? to_string(message.Param) : param_name);
				line << " = 0x" << hex << uppercase << message.Param << dec << "]";
			}
			if (id < EVX_nMessages && EVX_Messages[id].description[0])
			{
				string text = line.str();
				text.append(text.size() < 98 ? 100 - text.size() : 2, ' ');
				out << text << "// " << EVX_Messages[id].description << "\n";
			}
			else
				out << line.str() << "\n";
			nMessages++;
		}
		out << "\n";
	}

	// File di testo nella cartella \NOMELIVELLO\Scripts
	CreateDirectory(AOD_IO.folder_scx_lpwstr, NULL);
	if (!SetCurrentDirectory(AOD_IO.folder_scx_lpwstr))
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Unable to access folder " << AOD_IO.folder_scx << ". Events not exported.";
		return false;
	}
	ofstream txt(name + "_EVENTS.txt");
	if (!txt.is_open())
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Unable to create " << name << "_EVENTS.txt";
		return false;
	}
	txt << out.str();
	txt.close();
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Output: " << name << "_EVENTS.txt (" << header.nGroups << " event groups, " << nMessages << " messages, " << nBindings << " node bindings)";
	return ok;
}


/*------------------------------------------------------------------------------------------------------------------
Object Template del livello (EVX_OBJTEMPLATE dopo i gruppi di eventi del file <LIVELLO>.EVX), letti una volta sola.
------------------------------------------------------------------------------------------------------------------*/
const vector <EVX_OBJTEMPLATE> &EVX_GetTemplates ()
{
	static vector <EVX_OBJTEMPLATE> templates;
	static string level;
	if (level == AOD_IO.levelname)
		return templates;
	level = AOD_IO.levelname;
	templates.clear();
	ifstream evx(AOD_IO.folder_level + AOD_IO.levelname + ".EVX", ios::binary);
	vector <uint8_t> data((istreambuf_iterator<char>(evx)), istreambuf_iterator<char>());
	EVX_HEADER header;
	EVX_TEMPLATES_HEADER th;
	if (data.size() < sizeof(header))
		return templates;
	memcpy(&header, data.data(), sizeof(header));
	if ((size_t)header.Size + sizeof(th) > data.size())
		return templates;
	memcpy(&th, &data[header.Size], sizeof(th));
	for (uint32_t t = 0; t < th.nTemplates && t < 10000; t++)
	{
		size_t offset = (size_t)header.Size + sizeof(th) + sizeof(EVX_OBJTEMPLATE) * t;
		if (offset + sizeof(EVX_OBJTEMPLATE) > data.size())
			break;
		EVX_OBJTEMPLATE ot;
		memcpy(&ot, &data[offset], sizeof(ot));
		ot.AnimFileName[31] = 0;
		templates.push_back(ot);
	}
	return templates;
}
