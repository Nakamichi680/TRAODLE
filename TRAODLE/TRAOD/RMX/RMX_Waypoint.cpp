#include "stdafx.h"
#include <map>
#include <set>
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "TRAOD/RMX/RMX_Struct.h"
#include "TRAOD/RMX/RMX_Functions.h"


/*------------------------------------------------------------------------------------------------------------------
Legge un waypoint (il cursore di lettura deve essere all'inizio del nodo) e lo esporta come locator nei file FBX e MA.
Il waypoint viene aggiunto al vettore waypoints per la costruzione del grafo. Ritorna BegNext (offset del waypoint
successivo dall'inizio della stanza, 0 se e' l'ultimo).
Del nodo sono validi solo posizione, hash, tipo e collegamenti: il resto e' memoria non inizializzata (vedi RMX_Struct.h).
------------------------------------------------------------------------------------------------------------------*/
uint32_t RMX_Waypoint (ifstream &rmxfile, string room_name, string layer, ofstream &out, FBX_EXPORT &FBX, MA_EXPORT &MA, vector <RMX_WaypointInfo> &waypoints)
{
	RMX_WAYPOINT rmx_waypoint;
	rmxfile.read(reinterpret_cast<char*>(&rmx_waypoint), sizeof(rmx_waypoint));
	RMX_WaypointInfo wp;
	wp.hash = rmx_waypoint.Node.Hash;
	wp.pos = Vec3(rmx_waypoint.Node.Xpos, rmx_waypoint.Node.Ypos, rmx_waypoint.Node.Zpos);
	for (uint32_t k = 0; k < rmx_waypoint.nLinks; k++)
	{
		uint32_t link;
		rmxfile.read(reinterpret_cast<char*>(&link), sizeof(link));
		wp.links.push_back(link);
	}

	// Nome del locator: hash del waypoint (stesso valore usato negli script AMX). Gli hash duplicati ricevono un suffisso
	stringstream ssname;
	ssname << AOD_IO.levelname << "_WP_" << hex << uppercase << setw(8) << setfill('0') << wp.hash;
	wp.name = ssname.str();
	unsigned int duplicates = 0;
	for (unsigned int w = 0; w < waypoints.size(); w++)
		if (waypoints[w].hash == wp.hash)
			duplicates++;
	if (duplicates)
	{
		msg(msg::TGT::FILE, msg::TYP::WARN) << "Duplicated waypoint hash " << wp.name << ".";
		wp.name += "_" + to_string(duplicates);
	}

	// Esportazione informazioni waypoint nel file TXT
	out << "	Hashed name: " << hex << wp.hash << dec << endl;
	out << left << setw(30) << "	Position [X/Y/Z]:" << right << setw(13) << wp.pos.x << setw(13) << wp.pos.y << setw(13) << wp.pos.z << endl;
	out << "	Links (" << wp.links.size() << "):";
	for (unsigned int k = 0; k < wp.links.size(); k++)
		out << " " << hex << wp.links[k] << dec;
	out << endl;

	// Esportazione waypoint nei file FBX e MA
	Locator FBX_MA_Waypoint;
	FBX_MA_Waypoint.name = wp.name;
	FBX_MA_Waypoint.parent = room_name;
	FBX_MA_Waypoint.layer = layer;										// Nome layer di appartenenza (solo file MA)
	FBX_MA_Waypoint.FBX_parent = hashID(room_name, "Group");
	FBX_MA_Waypoint.translate_flag = true;
	FBX_MA_Waypoint.tX = wp.pos.x;
	FBX_MA_Waypoint.tY = wp.pos.y;
	FBX_MA_Waypoint.tZ = wp.pos.z;
	FBX_MA_Waypoint.scale_flag = true;									// Locator ingrandito per renderlo visibile nella scala dei livelli
	FBX_MA_Waypoint.sX = FBX_MA_Waypoint.sY = FBX_MA_Waypoint.sZ = 50;
	FBX.Locator.push_back(FBX_MA_Waypoint);
	MA.Locator.push_back(FBX_MA_Waypoint);

	waypoints.push_back(wp);
	return rmx_waypoint.Node.BegNext;
}


/*------------------------------------------------------------------------------------------------------------------
Costruisce il grafo dei waypoint di tutto il livello (i collegamenti possono unire waypoint di stanze diverse) e lo
esporta nel file MA: un gruppo con una curva lineare per ogni arco. I collegamenti sono quasi sempre presenti in
entrambe le direzioni: ogni coppia viene disegnata una volta sola. Gli archi presenti in una sola direzione vengono
messi su un layer separato.
------------------------------------------------------------------------------------------------------------------*/
void RMX_Waypoint_Graph (const vector <RMX_WaypointInfo> &waypoints, string layer, string oneway_layer, MA_EXPORT &MA)
{
	map <uint32_t, unsigned int> index;					// Hash -> indice del waypoint (in caso di hash duplicati vale il primo)
	for (unsigned int w = 0; w < waypoints.size(); w++)
		if (!index.count(waypoints[w].hash))
			index[waypoints[w].hash] = w;

	Transform graph_group;
	graph_group.name = AOD_IO.levelname + "_WAYPOINT_LINKS";
	MA.Transform.push_back(graph_group);

	set <pair <unsigned int, unsigned int>> edges;
	unsigned int nOneWay = 0, nSelf = 0, nMissing = 0;
	for (unsigned int a = 0; a < waypoints.size(); a++)
		for (unsigned int k = 0; k < waypoints[a].links.size(); k++)
		{
			map <uint32_t, unsigned int>::const_iterator it = index.find(waypoints[a].links[k]);
			if (it == index.end())
			{
				nMissing++;
				msg(msg::TGT::FILE, msg::TYP::WARN) << waypoints[a].name << " links to missing waypoint " << hex << waypoints[a].links[k] << dec << ". Link skipped.";
				continue;
			}
			unsigned int b = it->second;
			if (b == a)
			{
				nSelf++;
				continue;
			}
			if (!edges.insert(make_pair(min(a, b), max(a, b))).second)		// Arco gia' esportato (collegamento inverso)
				continue;
			const vector <uint32_t> &back = waypoints[b].links;
			bool oneway = find(back.begin(), back.end(), waypoints[a].hash) == back.end();
			if (oneway)
				nOneWay++;

			NurbsCurve edge;
			edge.name = graph_group.name + "_" + waypoints[a].name.substr(AOD_IO.levelname.size() + 1) + "_" + waypoints[b].name.substr(AOD_IO.levelname.size() + 4);
			edge.parent = graph_group.name;
			edge.layer = oneway ? oneway_layer : layer;
			edge.Points.push_back(waypoints[a].pos);
			edge.Points.push_back(waypoints[b].pos);
			MA.NurbsCurve.push_back(edge);
		}
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Number of waypoints: " << waypoints.size() << ", links: " << edges.size() << " (" << nOneWay << " one-way)";
	if (nSelf)
		msg(msg::TGT::FILE, msg::TYP::LOG) << nSelf << " waypoint links to itself skipped.";
}
