#include "stdafx.h"
#include <map>
#include "Classes.h"
#include "Misc_Functions.h"
#include "AOD_Names.h"
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "TRAOD/RMX/RMX_Struct.h"
#include "TRAOD/RMX/RMX_Functions.h"
#include "TRAOD/EVX/EVX_Functions.h"
#include "TRAOD/SCX/SCX_Functions.h"


/*------------------------------------------------------------------------------------------------------------------
Emitter (effetti: nebbia, fuoco, pioggia, fumo, bagliori, laser, ...) e trigger del file RMX (vedi RMX_Struct.h).
I nomi di nodi, gruppi di eventi e template degli effetti vengono dal dizionario dei nomi originali (AOD_Names.cpp) e
dal dizionario degli hash degli script (SCX_CollectHashNames, che deve essere gia' stato chiamato).
------------------------------------------------------------------------------------------------------------------*/


string RMX_NodeLabel (uint32_t hash)						// Nome originale del nodo, altrimenti nome noto agli script, altrimenti hash
{
	string name = AOD_NodeName(hash);
	if (name.empty())
		name = AMX_HashName((int32_t)hash);
	if (!name.empty())
		return name;
	char text[16];
	snprintf(text, sizeof(text), "0x%08X", hash);
	return text;
}


static string RMX_EventLabel (uint32_t index)				// Nome del gruppo di eventi di indice dato del file <LIVELLO>.EVX
{
	static vector <uint32_t> groups;
	static bool loaded = false;
	if (!loaded)
	{
		loaded = true;
		ifstream evxfile(AOD_IO.levelname + ".EVX", std::ios::binary);
		uint32_t header[3] = {0, 0, 0};
		if (evxfile.read(reinterpret_cast<char*>(header), sizeof(header)) && header[0] == 8 && header[2] < 100000)
			for (uint32_t g = 0; g < header[2]; g++)
			{
				uint32_t group[2];
				if (!evxfile.read(reinterpret_cast<char*>(group), sizeof(group)))
					break;
				groups.push_back(group[1]);
			}
	}
	if (index >= groups.size())
		return "event " + to_string(index);
	string name = AOD_EventName(groups[index]);
	if (name.empty())
		name = AMX_HashName((int32_t)groups[index]);
	char text[32];
	snprintf(text, sizeof(text), "0x%08X", groups[index]);
	return name.empty() ? string(text) : name;
}


// Messaggi ricevuti dal nodo e gruppi di eventi avviati (code RMX_MSG): "NM_X => evento; ..."
string RMX_MessagesText (const vector <RMX_MSG> &messages, string separator)
{
	string text;
	for (const RMX_MSG &m : messages)
		text += (text.empty() ? "" : separator) + EVX_MessageName(m.MsgID) + " => " + RMX_EventLabel(m.Param);
	return text;
}


/*------------------------------------------------------------------------------------------------------------------
Emitter (nodo alla posizione corrente del file): un gruppo nella stanza con il volume dell'effetto e gli attributi
effectTemplate (template di effects.fsc), effectShape (BOX / LINE / POINT), oneShot, colore (colorR/G/B/A), il secondo
blocco di 4 bytes (color2_0..3) e i due parametri numerici (param1, param2: significato dipendente dal template),
linkedObject (nodo collegato) e messages (messaggi ricevuti -> gruppi di eventi). Forme:
- BOX e POINT: box centrato nel nodo con semi-dimensioni = scala, ruotato con il nodo (BoxMin/BoxMax = pos -/+ scala);
- LINE: segmento dalla posizione del nodo (BoxMin) al punto finale (BoxMax), nelle coordinate del mondo: curva nel
  file MA, triangolo sottile nel file FBX.
Restituisce l'offset del nodo successivo della lista (RMX_NODE_HEADER.BegNext).
------------------------------------------------------------------------------------------------------------------*/
uint32_t RMX_Emitter (ifstream &rmxfile, string name, string room_name, string layer, ofstream &out, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	RMX_EMITTER e;
	rmxfile.read(reinterpret_cast<char*>(&e), sizeof(e));
	vector <RMX_MSG> messages(min(e.nMessages, 100u));
	if (!messages.empty())
		rmxfile.read(reinterpret_cast<char*>(messages.data()), messages.size() * sizeof(RMX_MSG));
	if (!rmxfile)
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << "Error reading emitter " << name << ".";
		rmxfile.clear();
		return 0;
	}
	const AOD_Effect *effect = AOD_EffectTemplate(e.EffectHash);
	bool line = effect && effect->shape == "ST_LINE";
	char hash[16];
	snprintf(hash, sizeof(hash), "0x%08X", e.EffectHash);

	Transform group;
	group.name = name;
	group.parent = room_name;
	group.FBX_parent = hashID(room_name, "Group");
	group.layer = layer;
	group.translate_flag = true;
	group.tX = e.Node.Xpos;	group.tY = e.Node.Ypos;	group.tZ = e.Node.Zpos;
	if (!line)																	// Le linee hanno gli estremi nel mondo
	{
		group.rotate_flag = true;
		group.rX = e.Node.Xrot;	group.rY = e.Node.Yrot;	group.rZ = e.Node.Zrot;
	}
	group.string_attributes.push_back(make_pair(string("effectTemplate"), effect ? effect->name : string(hash)));
	group.string_attributes.push_back(make_pair(string("effectShape"), effect ? effect->shape.substr(3) : string("?")));
	group.int_attributes.push_back(make_pair(string("oneShot"), effect && effect->oneshot ? 1 : 0));
	const pair <const char*, uint8_t> bytes[] = {{"colorR", e.R}, {"colorG", e.G}, {"colorB", e.B}, {"colorA", e.A},
		{"color2R", e.Color2[0]}, {"color2G", e.Color2[1]}, {"color2B", e.Color2[2]}, {"color2A", e.Color2[3]}};
	for (const auto &b : bytes)
		group.int_attributes.push_back(make_pair(string(b.first), (int)b.second));
	group.float_attributes.push_back(make_pair(string("effectScale"), e.Scale));
	group.float_attributes.push_back(make_pair(string("effectRate"), e.Rate));
	if (e.LinkedObject != 0 && e.LinkedObject != 0x0003518B)					// 0x0003518B = GetHashValue("pad"): nessuno
		group.string_attributes.push_back(make_pair(string("boundNode"), RMX_NodeLabel(e.LinkedObject)));	// L'effetto segue il nodo
	if (!(e.Node.Flags & 1))
		group.int_attributes.push_back(make_pair(string("inactive"), 1));		// Attivato da un messaggio
	if (!messages.empty())
		group.string_attributes.push_back(make_pair(string("messages"), RMX_MessagesText(messages, "; ")));
	FBX.Group.push_back(group);
	MA.Transform.push_back(group);

	unsigned int color = (e.R | e.G | e.B) ? 0x60000000 | (e.R << 16) | (e.G << 8) | e.B : 0x60FF8000;	// Colore dell'effetto
	if (line)
	{
		Vec3 end(e.BoxMax[0] - e.Node.Xpos, e.BoxMax[1] - e.Node.Ypos, e.BoxMax[2] - e.Node.Zpos);
		NurbsCurve curve;
		curve.name = name + "_Line";
		curve.parent = name;
		curve.layer = layer;
		curve.Points.push_back(Vec3());
		curve.Points.push_back(end);
		MA.NurbsCurve.push_back(curve);
		FBX.Geometry.push_back(DrawTriangle(name + "_Line", name, layer, Vec3(), end, end + Vec3(0, 0, 1), color));
	}
	else
	{
		Vec3 half(e.Node.Xscal, e.Node.Yscal, e.Node.Zscal);
		Vec3 zero;
		FBX.Geometry.push_back(DrawBox(name + "_Box", name, layer, zero - half, half, color));
		MA.PolyCube.push_back(DrawPolyCube(name + "_Box", name, layer, "", zero - half, half, true, color));
	}

	out << "	Name: " << name << endl;
	out << "	Hashed name: " << RMX_NodeLabel(e.Node.Hash) << endl;
	out << "	Effect: " << (effect ? effect->name + " (" + effect->shape + (effect->oneshot ? ", one shot)" : ")") : string(hash)) << endl;
	out << "	Color: " << (int)e.R << " " << (int)e.G << " " << (int)e.B << " " << (int)e.A << ", color2: " << (int)e.Color2[0] << " " << (int)e.Color2[1] << " " << (int)e.Color2[2] << " " << (int)e.Color2[3] << endl;
	out << "	Scale: " << e.Scale << ", rate: " << e.Rate << (e.Node.Flags & 1 ? "" : ", inactive") << endl;
	if (!messages.empty())
		out << "	Messages: " << RMX_MessagesText(messages, "; ") << endl;
	return e.Node.BegNext;
}


/*------------------------------------------------------------------------------------------------------------------
Flag di un trigger (RMX_TRIGFLAG_*) come testo, es. "PLAYER TRACKED ACTION"; i bit sconosciuti in esadecimale.
------------------------------------------------------------------------------------------------------------------*/
static string RMX_TriggerFlagsText (uint32_t flags)
{
	static const pair <uint32_t, const char*> names[] = {{RMX_TRIGFLAG_PLAYER, "PLAYER"}, {RMX_TRIGFLAG_CHARACTERS, "CHARACTERS"},
		{RMX_TRIGFLAG_ONESHOT, "ONESHOT"}, {RMX_TRIGFLAG_TRACKED, "TRACKED"}, {RMX_TRIGFLAG_BOUND, "BOUND"}, {RMX_TRIGFLAG_ACTION, "ACTION"},
		{RMX_TRIGFLAG_FACING, "FACING"}, {RMX_TRIGFLAG_CAMERA, "CAMERA"}, {RMX_TRIGFLAG_ACTUATOR, "ACTUATOR"}, {RMX_TRIGFLAG_REPEAT, "REPEAT"},
		{RMX_TRIGFLAG_NOACTION, "NOACTION"}, {RMX_TRIGFLAG_BONE, "BONE"}};
	string text;
	for (const auto &n : names)
		if (flags & n.first)
		{
			text += (text.empty() ? "" : " ") + string(n.second);
			flags &= ~n.first;
		}
	if (flags)
	{
		stringstream rest;
		rest << "0x" << hex << uppercase << flags;
		text += (text.empty() ? "" : " ") + rest.str();
	}
	return text.empty() ? "0" : text;
}


/*------------------------------------------------------------------------------------------------------------------
Trigger (nodo alla posizione corrente del file): un gruppo nella stanza con il volume (box locale -/+ scala, ruotato con
il nodo) e gli attributi triggerFlags (testo, vedi RMX_TRIGFLAG_*), actions (messaggi inviati quando il trigger si attiva:
messaggio -> destinatario [parametro]), messages (messaggi ricevuti -> gruppi di eventi), script, boundNode, boundBone,
actuator e inactive. Le stesse informazioni vengono aggiunte a text (file <LIVELLO>_TRIGGERS.txt). Restituisce l'offset
del nodo successivo della lista (RMX_NODE_HEADER.BegNext).
------------------------------------------------------------------------------------------------------------------*/
uint32_t RMX_Trigger (ifstream &rmxfile, string name, string room_name, unsigned int room, string layer, ofstream &out, stringstream &text, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	RMX_TRIGGER t;
	rmxfile.read(reinterpret_cast<char*>(&t), sizeof(t));
	vector <RMX_TRIGGER_ACTION> actions(min(t.nActions, 1000u));
	if (!actions.empty())
		rmxfile.read(reinterpret_cast<char*>(actions.data()), actions.size() * sizeof(RMX_TRIGGER_ACTION));
	vector <RMX_MSG> messages(min(t.nConditions, 100u));
	if (!messages.empty())
		rmxfile.read(reinterpret_cast<char*>(messages.data()), messages.size() * sizeof(RMX_MSG));
	if (!rmxfile)
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << "Error reading trigger " << name << ".";
		rmxfile.clear();
		return 0;
	}

	vector <string> action_text;
	for (const RMX_TRIGGER_ACTION &a : actions)
	{
		string line = EVX_MessageName(a.MsgID) + " -> " + RMX_NodeLabel(a.TargetHash);
		if (a.Param)
		{
			string param;														// Parametro: hash (nodo, waypoint, filmato, ...) o numero
			if (a.Param >= 0x10000)
				param = AOD_NodeName(a.Param).empty() ? AMX_HashName((int32_t)a.Param) : AOD_NodeName(a.Param);
			line += " [" + (param.empty() ? to_string(a.Param) : param) + "]";
		}
		action_text.push_back(line);
	}
	vector <pair <string, string>> linked;										// Nodi collegati (attributo, nome)
	if (t.LinkedHash != 0)
		linked.push_back(make_pair(string("script"), RMX_NodeLabel(t.LinkedHash)));
	if ((t.TriggerFlags & RMX_TRIGFLAG_BOUND) && t.BindNodeHash != 0)
		linked.push_back(make_pair(string("boundNode"), RMX_NodeLabel(t.BindNodeHash)));
	if (t.BindShapeHash != 0)
		linked.push_back(make_pair(string("boundBone"), RMX_NodeLabel(t.BindShapeHash)));
	if (t.ActuatorHash != 0)
		linked.push_back(make_pair(string("actuator"), RMX_NodeLabel(t.ActuatorHash)));
	string flags = RMX_TriggerFlagsText(t.TriggerFlags);

	Transform group;
	group.name = name;
	group.parent = room_name;
	group.FBX_parent = hashID(room_name, "Group");
	group.layer = layer;
	group.translate_flag = group.rotate_flag = true;
	group.tX = t.Node.Xpos;	group.tY = t.Node.Ypos;	group.tZ = t.Node.Zpos;
	group.rX = t.Node.Xrot;	group.rY = t.Node.Yrot;	group.rZ = t.Node.Zrot;
	group.string_attributes.push_back(make_pair(string("triggerFlags"), flags));
	if (t.MinorAxis != 0)
	{
		float minor_axis;
		memcpy(&minor_axis, &t.MinorAxis, sizeof(minor_axis));
		group.float_attributes.push_back(make_pair(string("minorAxis"), minor_axis));
	}
	if (t.SecondaryID != 0)
		group.int_attributes.push_back(make_pair(string("secondaryID"), (int)t.SecondaryID));
	if (!(t.Node.Flags & 1))
		group.int_attributes.push_back(make_pair(string("inactive"), 1));		// Attivato da un messaggio
	string all_actions;
	for (const string &a : action_text)
		all_actions += (all_actions.empty() ? "" : "; ") + a;
	group.string_attributes.push_back(make_pair(string("actions"), all_actions));
	if (!messages.empty())
		group.string_attributes.push_back(make_pair(string("messages"), RMX_MessagesText(messages, "; ")));
	for (const pair <string, string> &l : linked)
		group.string_attributes.push_back(l);
	FBX.Group.push_back(group);
	MA.Transform.push_back(group);
	Vec3 bbmin(t.BoxMin[0], t.BoxMin[1], t.BoxMin[2]), bbmax(t.BoxMax[0], t.BoxMax[1], t.BoxMax[2]);
	FBX.Geometry.push_back(DrawBox(name + "_Box", name, layer, bbmin, bbmax, 0x3000FF40));
	MA.PolyCube.push_back(DrawPolyCube(name + "_Box", name, layer, "", bbmin, bbmax, true, 0x3000FF40));

	// File di testo dei trigger
	text << "TRIGGER " << RMX_NodeLabel(t.Node.Hash) << "   {room " << room << ", pos " << fixed << setprecision(0) << t.Node.Xpos << " " << t.Node.Ypos << " " << t.Node.Zpos;
	text << ", size " << 2 * t.Node.Xscal << " " << 2 * t.Node.Yscal << " " << 2 * t.Node.Zscal;
	if (t.Node.Xrot != 0 || t.Node.Yrot != 0 || t.Node.Zrot != 0)
		text << ", rot " << t.Node.Xrot << " " << t.Node.Yrot << " " << t.Node.Zrot;
	text << ", flags " << flags << (t.Node.Flags & 1 ? "" : ", inactive") << "}" << defaultfloat << setprecision(6) << "\n";
	for (const pair <string, string> &l : linked)
		text << "    " << l.first << ": " << l.second << "\n";
	for (const RMX_MSG &m : messages)
		text << "  <= " << left << setw(28) << EVX_MessageName(m.MsgID) << " => " << RMX_EventLabel(m.Param) << right << "\n";
	for (const string &a : action_text)
		text << "    " << a << "\n";
	text << "\n";

	out << "	Name: " << name << endl;
	out << "	Hashed name: " << RMX_NodeLabel(t.Node.Hash) << endl;
	out << "	Flags: " << flags << ", actions: " << actions.size() << ", messages: " << messages.size() << endl;
	return t.Node.BegNext;
}
