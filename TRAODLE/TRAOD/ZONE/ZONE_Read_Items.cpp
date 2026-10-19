#include "stdafx.h"
#include <map>
#include <set>
#include "TRAOD/ZONE/ZONE_Struct.h"
#include "TRAOD/ZONE/ZONE_Functions.h"


/*------------------------------------------------------------------------------------------------------------------
Esporta gli Items del file ZONE: gli oggetti (slot) che non sono istanziati da nessun nodo RMX_PS2_ROOM_OBJ o RMX_OBJECT
e da nessun Fake in tutto il livello (esportati nella prima zona in cui lo slot non e' vuoto), con la loro gerarchia, e le mesh
degli oggetti che non appartengono a nessuno slot. La loro posizione nel mondo non e' nota (potrebbero essere creati
dagli script), quindi vengono disposti su una griglia a fianco della geometria gia' esportata e distanziati fra di loro
in modo da non sovrapporsi. Deve essere chiamata dopo ZONE_Read_Rooms, ZONE_Read_Fakes e ZONE_Read_Instances.
Layer: NOMELIVELLO_Z&&_Items.
------------------------------------------------------------------------------------------------------------------*/
bool ZONE_Read_Items (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	ifstream zonefile(filename, std::ios::binary);
	if (!zonefile.is_open())
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " not found.";
		return false;
	}
	ZONE_ObjectData data;
	if (!ZONE_ReadObjectData(zonefile, filename, data))
		return false;
	int zone_index = atoi(data.zonename.substr(1).c_str());
	const ZONE_LevelObjects &level = ZONE_GetLevelObjects();

	Layer items_layer;
	items_layer.name = AOD_IO.levelname + "_" + data.zonename + "_Items";
	items_layer.Label_ARGB = 0xFFD8A200;

	// Lettura Items: slot non usati nel livello e mesh fuori da ogni slot
	vector <ZONE_ObjectExport> Items;
	bool ok = true;
	set <int> slot_meshes;										// Mesh usate dagli slot di questa zona
	for (unsigned int s = 0; s < data.slots.size(); s++)
		for (const ZONE_SlotCommand &cmd : data.slots[s])
			if (cmd.type >= 0)
				slot_meshes.insert(cmd.type);
	for (unsigned int s = 0; s < data.slots.size(); s++)
	{
		map <unsigned int, int>::const_iterator it = level.item_zone.find(s);
		if (it == level.item_zone.end() || it->second != zone_index)
			continue;
		stringstream name;
		name << AOD_IO.levelname << "_" << data.zonename << "_ITEM_" << s;
		ZONE_ObjectExport item;
		Transform root;
		root.name = name.str();
		root.layer = items_layer.name;
		root.translate_flag = true;
		root.int_attributes.push_back(make_pair(string("objectSlot"), (int)s));
		item.groups.push_back(root);
		ok &= ZONE_ExportSlot(zonefile, data, s, root.name, items_layer.name, item);
		if (!item.meshes.empty())
			Items.push_back(item);
	}
	for (unsigned int m = 0; m < data.mesh_offsets.size(); m++)
	{
		if (slot_meshes.count(m) || data.mesh_vertices[m] == 0)
			continue;
		stringstream name;
		name << AOD_IO.levelname << "_" << data.zonename << "_ITEM_MESH_" << m;
		ZONE_ObjectExport item;
		Transform root;
		root.name = name.str();
		root.layer = items_layer.name;
		root.translate_flag = true;
		item.groups.push_back(root);
		ok &= ZONE_ExportMesh(zonefile, data, m, root.name + "_M" + to_string(m), root.name, items_layer.name, MATRIX(), item);
		if (!item.meshes.empty())
			Items.push_back(item);
	}
	zonefile.close();

	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Items (objects without a position): " << Items.size();
	if (Items.empty())
		return ok;

	///////////////////    CALCOLO BOUNDING BOX DELLA GEOMETRIA GIA' ESPORTATA (ROOMS, FAKES E ISTANZE)
	// Le coordinate dei vertici vengono portate in world space risalendo la gerarchia dei gruppi (solo traslazione e scalatura, le rotazioni vengono ignorate)
	map <string, const Transform*> Groups;
	for (unsigned int g = 0; g < FBX.Group.size(); g++)
		Groups[FBX.Group[g].name] = &FBX.Group[g];
	bool world_empty = true;
	Vec3 Wmin, Wmax;
	for (unsigned int m = 0; m < FBX.Geometry.size(); m++)
	{
		vector <const Transform*> hierarchy;
		for (map <string, const Transform*>::iterator it = Groups.find(FBX.Geometry[m].parent); it != Groups.end(); it = Groups.find(it->second->parent))
		{
			hierarchy.push_back(it->second);
			if (hierarchy.size() > 64)				// Protezione contro gerarchie circolari
				break;
		}
		for (unsigned int v = 0; v < FBX.Geometry[m].X.size(); v++)
		{
			Vec3 p(FBX.Geometry[m].X[v], FBX.Geometry[m].Y[v], FBX.Geometry[m].Z[v]);
			for (unsigned int h = 0; h < hierarchy.size(); h++)
			{
				p.x = p.x * hierarchy[h]->sX + hierarchy[h]->tX;
				p.y = p.y * hierarchy[h]->sY + hierarchy[h]->tY;
				p.z = p.z * hierarchy[h]->sZ + hierarchy[h]->tZ;
			}
			if (world_empty)
			{
				Wmin = Wmax = p;
				world_empty = false;
			}
			Wmin.x = min(Wmin.x, p.x);	Wmin.y = min(Wmin.y, p.y);	Wmin.z = min(Wmin.z, p.z);
			Wmax.x = max(Wmax.x, p.x);	Wmax.y = max(Wmax.y, p.y);	Wmax.z = max(Wmax.z, p.z);
		}
	}

	///////////////////    DISPOSIZIONE ITEMS NELLO SPAZIO 3D
	// Gli Items sono disposti su una griglia nel piano XY, a fianco della geometria delle stanze (lato X+).
	// Ogni Item occupa una cella pari alla sua bounding box piu' un margine proporzionale alle sue dimensioni e viene
	// traslato in modo che il centro della bounding box coincida con il centro della cella (alcuni Items hanno i vertici
	// lontani dall'origine locale). Le celle sono inserite per righe (shelf packing) in ordine di altezza decrescente,
	// andando a capo quando la riga supera la larghezza della griglia, calcolata perche' la griglia sia circa quadrata.
	// La griglia e' centrata verticalmente e in profondita' (Z) rispetto alla geometria delle stanze.
	// Gli Items fuori scala (es. skydome, decine di volte piu' grandi della mediana) sono esclusi dalla griglia,
	// perche' ne falserebbero la larghezza riducendola a una sola riga, e vengono disposti a parte oltre il lato X+ della griglia.
	vector <float> Item_size(Items.size()), Item_sizes;
	for (unsigned int i = 0; i < Items.size(); i++)
	{
		Item_size[i] = max(Items[i].bbmax.x - Items[i].bbmin.x, Items[i].bbmax.y - Items[i].bbmin.y);
		if (!Items[i].bb_empty)
			Item_sizes.push_back(Item_size[i]);
	}
	float median_size = 0;
	if (!Item_sizes.empty())
	{
		nth_element(Item_sizes.begin(), Item_sizes.begin() + Item_sizes.size() / 2, Item_sizes.end());
		median_size = Item_sizes[Item_sizes.size() / 2];
	}
	float base_gap = max(median_size * 3.0f, 1.0f);						// Spazio minimo fra due Items adiacenti (300% della dimensione mediana)
	float outlier_size = median_size * 50;								// Dimensione oltre la quale un Item e' escluso dalla griglia

	vector <float> cell_width(Items.size()), cell_height(Items.size());
	vector <unsigned int> order, outliers;
	float total_area = 0, max_cell_width = 0;
	for (unsigned int i = 0; i < Items.size(); i++)
	{
		float gap = base_gap + 0.1f * Item_size[i];
		cell_width[i] = Items[i].bbmax.x - Items[i].bbmin.x + gap;
		cell_height[i] = Items[i].bbmax.y - Items[i].bbmin.y + gap;
		if (median_size > 0 && Item_size[i] > outlier_size)
		{
			outliers.push_back(i);
			continue;
		}
		total_area += cell_width[i] * cell_height[i];
		max_cell_width = max(max_cell_width, cell_width[i]);
		order.push_back(i);
	}
	stable_sort(order.begin(), order.end(), [&cell_height] (unsigned int a, unsigned int b) {return cell_height[a] > cell_height[b];});
	float grid_width = max(sqrt(total_area), max_cell_width);
	float world_size = max(Wmax.x - Wmin.x, Wmax.z - Wmin.z);
	float origin_X = Wmax.x + max(world_size * 0.1f, base_gap * 4);		// Distanza dalla geometria delle stanze
	float origin_Z = (Wmin.z + Wmax.z) / 2;

	vector <float> cell_Y(Items.size());								// Posizione Y del centro di ogni cella rispetto all'inizio della griglia
	float cursor_X = 0, cursor_Y = 0, row_height = 0, grid_used_width = 0;
	for (unsigned int n = 0; n < order.size(); n++)
	{
		unsigned int i = order[n];
		Transform &root = Items[i].groups[0];
		if (cursor_X > 0 && cursor_X + cell_width[i] > grid_width)		// Nuova riga
		{
			cursor_X = 0;
			cursor_Y += row_height;
			row_height = 0;
		}
		root.tX = origin_X + cursor_X + cell_width[i] / 2 - (Items[i].bbmin.x + Items[i].bbmax.x) / 2;
		root.tZ = origin_Z - (Items[i].bbmin.z + Items[i].bbmax.z) / 2;
		cell_Y[i] = cursor_Y + cell_height[i] / 2;
		cursor_X += cell_width[i];
		row_height = max(row_height, cell_height[i]);
		grid_used_width = max(grid_used_width, cursor_X);
	}
	float grid_height = cursor_Y + row_height;
	float origin_Y = (Wmin.y + Wmax.y) / 2 - grid_height / 2;			// Griglia centrata verticalmente sulla geometria delle stanze
	for (unsigned int n = 0; n < order.size(); n++)
		Items[order[n]].groups[0].tY = origin_Y + cell_Y[order[n]] - (Items[order[n]].bbmin.y + Items[order[n]].bbmax.y) / 2;

	// Items fuori scala: affiancati lungo X oltre la griglia, centrati in Y e Z come la griglia
	cursor_X = origin_X + grid_used_width + base_gap * 4;
	for (unsigned int n = 0; n < outliers.size(); n++)
	{
		unsigned int i = outliers[n];
		Transform &root = Items[i].groups[0];
		root.tX = cursor_X + cell_width[i] / 2 - (Items[i].bbmin.x + Items[i].bbmax.x) / 2;
		root.tY = (Wmin.y + Wmax.y) / 2 - (Items[i].bbmin.y + Items[i].bbmax.y) / 2;
		root.tZ = origin_Z - (Items[i].bbmin.z + Items[i].bbmax.z) / 2;
		cursor_X += cell_width[i];
		msg(msg::TGT::FILE, msg::TYP::LOG) << root.name << " is oversized (" << Item_size[i] << " vs median " << median_size << "): placed outside the Items grid.";
	}

	///////////////////    INSERIMENTO ITEMS NEI FILE FBX E MA
	MA.Layer.push_back(items_layer);
	for (const ZONE_ObjectExport &item : Items)
		item.PushTo(FBX, MA);
	return ok;
}
