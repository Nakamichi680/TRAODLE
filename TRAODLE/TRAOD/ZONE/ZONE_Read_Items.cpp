#include "stdafx.h"
#include <map>
#include <set>
#include "TRAOD/ZONE/ZONE_Struct.h"
#include "TRAOD/ZONE/ZONE_Functions.h"
#include "Misc_Functions.h"


bool exportItemsBoundingBoxes = false;


/*------------------------------------------------------------------------------------------------------------------
Esporta gli Items del file ZONE, cio� tutti gli oggetti del blocco ZONE_MESH_OBJECT (ZONE_MESH_HEADER2.nObjects) ad
eccezione di quelli gi� esportati come Fakes e di quelli privi di vertici. Il nome di ogni Item contiene l'indice
dell'oggetto nel file ZONE. La posizione in world coordinates degli Items non � nota, quindi vengono disposti su una
griglia a fianco della geometria gi� esportata (Rooms e Fakes) e distanziati fra di loro in modo da non sovrapporsi.
Deve essere chiamata dopo ZONE_Read_Rooms e ZONE_Read_Fakes.
------------------------------------------------------------------------------------------------------------------*/
bool ZONE_Read_Items (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	ZONE_HEADER zone_header;
	ZONE_PS2_OBJ_SLOTS zone_ps2_obj_slots;
	ZONE_PS2_OBJ zone_ps2_obj;
	ZONE_MATERIALS_LIST zone_materials_list;
	ZONE_MESH_HEADER1 zone_mesh_header1;
	ZONE_MESH_ROOM_HEADER zone_mesh_room_header;
	ZONE_MESH_HEADER2 zone_mesh_header2;
	ZONE_MESH_OBJECT_HEADER zone_mesh_object_header;
	ZONE_MESH_VERTEX zone_mesh_vertex;
	ZONE_MESH_STRIP zone_mesh_strip;
	ZONE_MESH_ELEMENT zone_mesh_element;
	ZONE_FAKES_HEADER zone_fakes_header;
	ZONE_FAKES_ELEMENT zone_fakes_element;
	string zonename = filename;
	zonename.erase(0, (zonename.find(".Z") + 1));

	///////////////////    APERTURA FILE ZONE
	ifstream zonefile(filename, std::ios::binary);
	if (!zonefile.is_open())
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " not found.";
		return false;
	}

	// Lettura Header
	zonefile.read(reinterpret_cast<char*>(&zone_header.ZONE_ID), sizeof(zone_header.ZONE_ID));

	if (zone_header.ZONE_ID != 32)				// Se il file ZONE non � valido
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " is not a valid ZONE file.";
		return false;
	}

	zonefile.read(reinterpret_cast<char*>(&zone_header.TEXTURE_PTR), sizeof(zone_header.TEXTURE_PTR));
	zonefile.read(reinterpret_cast<char*>(&zone_header.PS2_OBJ_PTR), sizeof(zone_header.PS2_OBJ_PTR));
	zonefile.read(reinterpret_cast<char*>(&zone_header.MESH_PTR), sizeof(zone_header.MESH_PTR));
	zonefile.read(reinterpret_cast<char*>(&zone_header.EOF_PTR), sizeof(zone_header.EOF_PTR));

	// Lettura indici degli oggetti usati come Fakes (questi oggetti vengono saltati perch� gi� esportati da ZONE_Read_Fakes)
	set <unsigned int> Fake_indices;
	zonefile.seekg(zone_header.EOF_PTR);
	zonefile.read(reinterpret_cast<char*>(&zone_fakes_header.P1_Fake_First), sizeof(zone_fakes_header.P1_Fake_First));
	if (!zonefile)								// Se il blocco Fakes � assente non ci sono Fakes
	{
		zonefile.clear();
		zone_fakes_header.P1_Fake_First = 0;
	}
	if (zone_fakes_header.P1_Fake_First != 0)
	{
		zonefile.seekg(zone_fakes_header.P1_Fake_First);
		do {
			streamoff fake_position = zonefile.tellg();
			zonefile.seekg(4, ios_base::cur);		// Salta BegPrev
			zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.BegNext), sizeof(zone_fakes_element.BegNext));
			zonefile.seekg(fake_position + 126);	// Posizionamento su Fake_List_Index
			zonefile.read(reinterpret_cast<char*>(&zone_fakes_element.Fake_List_Index), sizeof(zone_fakes_element.Fake_List_Index));
			Fake_indices.insert(zone_fakes_element.Fake_List_Index);
			zonefile.seekg(zone_fakes_element.BegNext);
		} while (zone_fakes_element.BegNext);
	}

	// Ricerca posizione inizio blocco Objects
	zonefile.seekg(zone_header.MESH_PTR);
	zonefile.read(reinterpret_cast<char*>(&zone_mesh_header1.nRooms), sizeof(zone_mesh_header1.nRooms));
	for (unsigned int r = 0; r < zone_mesh_header1.nRooms; r++)													// Questo macroblocco "for" legge le dimensioni di ogni stanza per saltarla
	{
		zonefile.read(reinterpret_cast<char*>(&zone_mesh_room_header.RoomID), sizeof(zone_mesh_room_header.RoomID));			// ID stanza
		zonefile.read(reinterpret_cast<char*>(&zone_mesh_room_header.Room_size), sizeof(zone_mesh_room_header.Room_size));		// Dimensioni stanza in bytes
		zonefile.seekg(zone_mesh_room_header.Room_size - 4, ios_base::cur);														// Salta la stanza
	}

	// Salvataggio offsets di tutti gli oggetti
	zonefile.read(reinterpret_cast<char*>(&zone_mesh_header2.nObjects), sizeof(zone_mesh_header2.nObjects));
	vector <streamoff> Object_offsets (zone_mesh_header2.nObjects);
	for (unsigned int o = 0; o < zone_mesh_header2.nObjects; o++)
	{
		Object_offsets[o] = zonefile.tellg();
		zonefile.read(reinterpret_cast<char*>(&zone_mesh_object_header.Object_size), sizeof(zone_mesh_object_header.Object_size));		// Dimensioni oggetto in bytes
		zonefile.seekg(zone_mesh_object_header.Object_size - 4, ios_base::cur);															// Salta l'oggetto
	}

	// Lettura degli oggetti usati dai Fakes: i Fakes referenziano uno slot PS2_OBJ_SLOTS che a sua volta contiene la lista degli oggetti
	set <unsigned int> Fake_objects;
	for (set <unsigned int>::iterator it = Fake_indices.begin(); it != Fake_indices.end(); ++it)
	{
		zonefile.seekg(zone_header.PS2_OBJ_PTR + *it * 4 + 8);
		zonefile.read(reinterpret_cast<char*>(&zone_ps2_obj_slots.P1_Object), sizeof(zone_ps2_obj_slots.P1_Object));
		if (zone_ps2_obj_slots.P1_Object == 0xFFFFFFFF)		// Slot vuoto
			continue;
		zonefile.seekg(zone_ps2_obj_slots.P1_Object + 32);	// Salta Unknown1x...Unknown8w
		do {
			zonefile.read(reinterpret_cast<char*>(&zone_ps2_obj.Obj_ID), sizeof(zone_ps2_obj.Obj_ID));
			if (zone_ps2_obj.Obj_ID == -4)
				zonefile.seekg(24, ios_base::cur);		// Salta 24 bytes
			if (zone_ps2_obj.Obj_ID >= 0)
			{
				Fake_objects.insert(zone_ps2_obj.Obj_ID);
				zonefile.seekg(12, ios_base::cur);		// Salta Xrot, Yrot, Zrot
			}
		} while (zone_ps2_obj.Obj_ID != -1 && zonefile);
		zonefile.clear();
	}

	// CREAZIONE LAYERS ZONE ITEMS PER FILE MA
	Layer Zone_Items_layer, Zone_Items_BB_layer;
	stringstream Zone_Items_layer_name, Zone_Items_BB_layer_name;
	Zone_Items_layer_name << AOD_IO.levelname << "_" << zonename << "_Items";
	Zone_Items_BB_layer_name << Zone_Items_layer_name.str() << "_bounding_boxes";
	Zone_Items_layer.name = Zone_Items_layer_name.str();
	Zone_Items_layer.Label_ARGB = 0xFFD8A200;
	Zone_Items_BB_layer.name = Zone_Items_BB_layer_name.str();
	Zone_Items_BB_layer.Label_ARGB = 0xFF0000FF;
	Zone_Items_BB_layer.Visible = false;
	Zone_Items_BB_layer.Type = LayerDisplayType::Template;

	// Classe provvisoria contenente un Item completo. Gli Items vengono prima letti tutti e poi disposti nello spazio 3D
	class Item {
	public:
		Transform object;						// Gruppo principale dell'Item
		vector <Mesh> element;					// Elementi geometrici dell'Item
		bool BB_empty = true;
		Vec3 BBmin, BBmax;						// Bounding box dei vertici dell'Item in coordinate locali
	};
	vector <Item> Items;

	// Lettura Items: ogni oggetto del blocco ZONE_MESH_OBJECT e' un Item, ad eccezione di quelli gia' esportati come Fakes
	for (unsigned int o = 0; o < zone_mesh_header2.nObjects; o++)
	{
		if (Fake_objects.count(o))				// L'oggetto e' usato da un Fake ed e' gia' stato esportato
			continue;
		zonefile.seekg(Object_offsets[o]);

		Item item;

		// Creazione Transform oggetto
		stringstream temp1;
		temp1 << AOD_IO.levelname << "_" << zonename << "_ITEM_" << o;
		item.object.name = temp1.str();
		item.object.translate_flag = true;

		// Lettura mesh oggetto
		zonefile.seekg(12, ios_base::cur);		// Salta Object_size, Unknown1, Unknown2
		zonefile.read(reinterpret_cast<char*>(&zone_mesh_object_header.nVertices), sizeof(zone_mesh_object_header.nVertices));	// Numero vertici
		if (zone_mesh_object_header.nVertices == 0)		// Oggetto senza geometria: non viene esportato
		{
			msg(msg::TGT::FILE, msg::TYP::LOG) << item.object.name << " does not contain any geometry. Skipped.";
			continue;
		}
		zonefile.seekg(4, ios_base::cur);		// Salta unknown3
		zonefile.read(reinterpret_cast<char*>(&zone_mesh_object_header.nIndices), sizeof(zone_mesh_object_header.nIndices));	// Numero indici del triangle strip
		zonefile.seekg(4, ios_base::cur);		// Salta unknown4
		zonefile.read(reinterpret_cast<char*>(&zone_mesh_object_header.nElements), sizeof(zone_mesh_object_header.nElements));	// Numero elementi
		zonefile.seekg(20, ios_base::cur);		// Salta unknown5/6/7/8/9
		streamoff vertex_position = zonefile.tellg();												// Memorizza la posizione iniziale del blocco vertici
		streamoff strip_position = vertex_position + zone_mesh_object_header.nVertices * 40;		// Memorizza la posizione iniziale del blocco triangle strip
		streamoff elements_position = strip_position + zone_mesh_object_header.nIndices * 2;		// Memorizza la posizione iniziale del blocco elementi

		for (unsigned int el = 0; el < zone_mesh_object_header.nElements; el++)
		{
			Mesh element;				// Classe provvisoria contente i valori letti dal file ZONE. Va copiata nell'apposito array FBX e/o MA al termine dell'estrazione
			stringstream ssname, ssmaterial, ssbbname;
			ssname << AOD_IO.levelname << "_" << zonename << "_ITEM_" << o << "_OBJ_" << el;
			ssbbname << ssname.str() << "_BB";
			element.name = ssname.str();
			element.parent = item.object.name;
			element.layer = Zone_Items_layer_name.str();
			element.FBX_parent = hashID(item.object.name, "Group");

			// Lettura dati elemento
			Vec3 BBmin, BBmax;
			zonefile.seekg(elements_position + el * 64);
			zonefile.seekg(4, ios_base::cur);		// Salta nElement_Triangles
			zonefile.read(reinterpret_cast<char*>(&zone_mesh_element.nElement_Indices), sizeof(zone_mesh_element.nElement_Indices));	// Numero di indici dello strip
			zonefile.read(reinterpret_cast<char*>(&zone_mesh_element.Offset), sizeof(zone_mesh_element.Offset));						// Offset nello strip
			zonefile.read(reinterpret_cast<char*>(&zone_mesh_element.Material_Ref), sizeof(zone_mesh_element.Material_Ref));			// ID materiale
			zonefile.seekg(12, ios_base::cur);		// Salta Unknown1, Vbuffer_min e Vbuffer_max
			zonefile.read(reinterpret_cast<char*>(&zone_mesh_element.Draw_mode), sizeof(zone_mesh_element.Draw_mode));					// Tipologia di rendering
			zonefile.read(reinterpret_cast<char*>(&BBmin.x), sizeof(zone_mesh_element.BB_Xmin));										// Bounding box X min
			zonefile.read(reinterpret_cast<char*>(&BBmin.y), sizeof(zone_mesh_element.BB_Ymin));										// Bounding box Y min
			zonefile.read(reinterpret_cast<char*>(&BBmin.z), sizeof(zone_mesh_element.BB_Zmin));										// Bounding box Z min
			zonefile.seekg(4, ios_base::cur);		// Salta Unknown2
			zonefile.read(reinterpret_cast<char*>(&BBmax.x), sizeof(zone_mesh_element.BB_Xmax));										// Bounding box X max
			zonefile.read(reinterpret_cast<char*>(&BBmax.y), sizeof(zone_mesh_element.BB_Ymax));										// Bounding box Y max
			zonefile.read(reinterpret_cast<char*>(&BBmax.z), sizeof(zone_mesh_element.BB_Zmax));										// Bounding box Z max
			if (exportItemsBoundingBoxes)
				item.element.push_back(DrawBox(ssbbname.str(), item.object.name, Zone_Items_BB_layer_name.str(), BBmin, BBmax, 0x35500000));

			// Se l'elemento non contiene almeno 1 triangolo (numero indici almeno pari a 3) viene saltato
			if (zone_mesh_element.nElement_Indices < 3)
			{
				msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << element.name << " does not contain any triangle. Only Bounding Box will be exported.";
				continue;
			}

			// Aggiunta collegamento a materiale
			ssmaterial << AOD_IO.levelname << "_" << zonename << "_Material_" << zone_mesh_element.Material_Ref;
			element.material_name = ssmaterial.str();

			// Controllo double sided nel materiale associato
			zonefile.seekg(zone_header.TEXTURE_PTR + 16 + zone_mesh_element.Material_Ref * 24);
			zonefile.read(reinterpret_cast<char*>(&zone_materials_list.TextureMode), sizeof(zone_materials_list.TextureMode));
			zonefile.read(reinterpret_cast<char*>(&zone_materials_list.DoubleSided), sizeof(zone_materials_list.DoubleSided));
			element.doublesided = CheckDoubleSided(zone_materials_list.TextureMode, zone_materials_list.DoubleSided);
			element.uv_set2_flag = CheckShadowMap(zone_materials_list.TextureMode);

			// Lettura strip
			vector <unsigned int> strip(zone_mesh_element.nElement_Indices);
			vector <unsigned int> vertex_array;
			zonefile.seekg(strip_position + zone_mesh_element.Offset * 2);					// Posizionamento cursore di lettura all'inizio dello strip dell'elemento el
			for (unsigned int i = 0; i < zone_mesh_element.nElement_Indices; i++)			// Lettura strip del singolo elemento el
				zonefile.read(reinterpret_cast<char*>(&strip[i]), sizeof(zone_mesh_strip.Index));
			for (unsigned int i = 0; i < strip.size(); i++)
			{
				vector <unsigned int>::iterator it2 = find(vertex_array.begin(), vertex_array.end(), strip[i]);
				if (it2 == vertex_array.end())												// Se il vertice non viene trovato viene aggiunto alla lista
				{
					vertex_array.push_back(strip[i]);
					strip[i] = vertex_array.size() - 1;										// Gli indici dei vertici vengono aggiornati in base alla nuova lista vertex_array
				}
				else																		// Se il vertice viene trovato (precedentemente inserito) copia la sua posizione
					strip[i] = distance(vertex_array.begin (), it2);						// Gli indici dei vertici vengono aggiornati in base alla nuova lista vertex_array
			}
			element.Face = Calculate_Faces(strip, zone_mesh_element.Offset, zone_mesh_element.Draw_mode);		// Creazione lista facce
			element.nV = vertex_array.size();

			// Lettura vertici
			for (unsigned int v = 0; v < vertex_array.size(); v++)
			{
				zonefile.seekg(vertex_position + vertex_array[v] * 40);
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.X), sizeof(zone_mesh_vertex.X));					// Coordinata X
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Y), sizeof(zone_mesh_vertex.Y));					// Coordinata Y
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Z), sizeof(zone_mesh_vertex.Z));					// Coordinata Z
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.U1), sizeof(zone_mesh_vertex.U1));					// UV
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.V1), sizeof(zone_mesh_vertex.V1));					// UV
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.U2), sizeof(zone_mesh_vertex.U2));					// UV shadow map
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.V2), sizeof(zone_mesh_vertex.V2));					// UV shadow map
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Xn), sizeof(zone_mesh_vertex.Xn));					// Vertex normal X
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Yn), sizeof(zone_mesh_vertex.Yn));					// Vertex normal Y
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Zn), sizeof(zone_mesh_vertex.Zn));					// Vertex normal Z
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Xtg), sizeof(zone_mesh_vertex.Xtg));				// Vertex tangent X
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Ytg), sizeof(zone_mesh_vertex.Ytg));				// Vertex tangent Y
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Ztg), sizeof(zone_mesh_vertex.Ztg));				// Vertex tangent Z
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Xbn), sizeof(zone_mesh_vertex.Xbn));				// Vertex binormal X
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Ybn), sizeof(zone_mesh_vertex.Ybn));				// Vertex binormal Y
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.Zbn), sizeof(zone_mesh_vertex.Zbn));				// Vertex binormal Z
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.VC_red), sizeof(zone_mesh_vertex.VC_red));			// Vertex color R
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.VC_green), sizeof(zone_mesh_vertex.VC_green));		// Vertex color G
				zonefile.read(reinterpret_cast<char*>(&zone_mesh_vertex.VC_blue), sizeof(zone_mesh_vertex.VC_blue));		// Vertex color B
				element.X.push_back(zone_mesh_vertex.X);
				element.Y.push_back(zone_mesh_vertex.Y);
				element.Z.push_back(zone_mesh_vertex.Z);
				element.U1.push_back(zone_mesh_vertex.U1);
				element.V1.push_back(zone_mesh_vertex.V1);
				element.U2.push_back(zone_mesh_vertex.U2);
				element.V2.push_back(zone_mesh_vertex.V2);
				element.Xn.push_back(((float)zone_mesh_vertex.Xn - 128) / 127);
				element.Yn.push_back(((float)zone_mesh_vertex.Yn - 128) / 127);
				element.Zn.push_back(((float)zone_mesh_vertex.Zn - 128) / 127);
				element.Xtg.push_back(((float)zone_mesh_vertex.Xtg - 128) / 127);
				element.Ytg.push_back(((float)zone_mesh_vertex.Ytg - 128) / 127);
				element.Ztg.push_back(((float)zone_mesh_vertex.Ztg - 128) / 127);
				element.Xbn.push_back(((float)zone_mesh_vertex.Xbn - 128) / 127);
				element.Ybn.push_back(((float)zone_mesh_vertex.Ybn - 128) / 127);
				element.Zbn.push_back(((float)zone_mesh_vertex.Zbn - 128) / 127);
				element.A.push_back(1);
				element.R.push_back((float)zone_mesh_vertex.VC_red / 255);
				element.G.push_back((float)zone_mesh_vertex.VC_green / 255);
				element.B.push_back((float)zone_mesh_vertex.VC_blue / 255);
				if (item.BB_empty)
				{
					item.BBmin.x = item.BBmax.x = zone_mesh_vertex.X;
					item.BBmin.y = item.BBmax.y = zone_mesh_vertex.Y;
					item.BBmin.z = item.BBmax.z = zone_mesh_vertex.Z;
					item.BB_empty = false;
				}
				item.BBmin.x = min(item.BBmin.x, zone_mesh_vertex.X);	item.BBmax.x = max(item.BBmax.x, zone_mesh_vertex.X);
				item.BBmin.y = min(item.BBmin.y, zone_mesh_vertex.Y);	item.BBmax.y = max(item.BBmax.y, zone_mesh_vertex.Y);
				item.BBmin.z = min(item.BBmin.z, zone_mesh_vertex.Z);	item.BBmax.z = max(item.BBmax.z, zone_mesh_vertex.Z);
			}
			item.element.push_back(element);
		}

		if (!zonefile)
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Error reading " << item.object.name;
			return false;
		}
		if (!item.element.empty())
			Items.push_back(item);
	}
	zonefile.close();

	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Number of items: " << Items.size();
	if (Items.empty())
		return true;

	///////////////////    CALCOLO BOUNDING BOX DELLA GEOMETRIA GIA' ESPORTATA (ROOMS E FAKES)
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
			Vec3 p;
			p.x = FBX.Geometry[m].X[v];
			p.y = FBX.Geometry[m].Y[v];
			p.z = FBX.Geometry[m].Z[v];
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
	if (world_empty)
	{
		Wmin.x = Wmin.y = Wmin.z = 0;
		Wmax = Wmin;
	}

	///////////////////    DISPOSIZIONE ITEMS NELLO SPAZIO 3D
	// Gli Items sono disposti su una griglia nel piano XY, a fianco della geometria delle stanze (lato X+).
	// Ogni Item occupa una cella pari alla sua bounding box piu' un margine proporzionale alle sue dimensioni e viene
	// traslato in modo che il centro della bounding box coincida con il centro della cella (alcuni Items hanno i vertici
	// lontani dall'origine locale). Le celle sono inserite per righe (shelf packing) in ordine di altezza decrescente,
	// andando a capo quando la riga supera la larghezza della griglia, calcolata perche' la griglia sia circa quadrata.
	// La griglia e' centrata verticalmente e in profondita' (Z) rispetto alla geometria delle stanze.
	// La dimensione mediana e' calcolata solo sugli Items con geometria (molti Items hanno parti senza vertici e bounding
	// box nulla). Gli Items fuori scala (es. skydome, decine di volte piu' grandi della mediana) sono esclusi dalla griglia,
	// perche' ne falserebbero la larghezza riducendola a una sola riga, e vengono disposti a parte oltre il lato X+ della griglia.
	vector <float> Item_size(Items.size()), Item_sizes;
	for (unsigned int i = 0; i < Items.size(); i++)
	{
		Item_size[i] = max(Items[i].BBmax.x - Items[i].BBmin.x, Items[i].BBmax.y - Items[i].BBmin.y);
		if (!Items[i].BB_empty)
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
		cell_width[i] = Items[i].BBmax.x - Items[i].BBmin.x + gap;
		cell_height[i] = Items[i].BBmax.y - Items[i].BBmin.y + gap;
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
		if (cursor_X > 0 && cursor_X + cell_width[i] > grid_width)		// Nuova riga
		{
			cursor_X = 0;
			cursor_Y += row_height;
			row_height = 0;
		}
		Items[i].object.tX = origin_X + cursor_X + cell_width[i] / 2 - (Items[i].BBmin.x + Items[i].BBmax.x) / 2;
		Items[i].object.tZ = origin_Z - (Items[i].BBmin.z + Items[i].BBmax.z) / 2;
		cell_Y[i] = cursor_Y + cell_height[i] / 2;
		cursor_X += cell_width[i];
		row_height = max(row_height, cell_height[i]);
		grid_used_width = max(grid_used_width, cursor_X);
	}
	float grid_height = cursor_Y + row_height;
	float origin_Y = (Wmin.y + Wmax.y) / 2 - grid_height / 2;			// Griglia centrata verticalmente sulla geometria delle stanze
	for (unsigned int n = 0; n < order.size(); n++)
		Items[order[n]].object.tY = origin_Y + cell_Y[order[n]] - (Items[order[n]].BBmin.y + Items[order[n]].BBmax.y) / 2;

	// Items fuori scala: affiancati lungo X oltre la griglia, centrati in Y e Z come la griglia
	cursor_X = origin_X + grid_used_width + base_gap * 4;
	for (unsigned int n = 0; n < outliers.size(); n++)
	{
		unsigned int i = outliers[n];
		Items[i].object.tX = cursor_X + cell_width[i] / 2 - (Items[i].BBmin.x + Items[i].BBmax.x) / 2;
		Items[i].object.tY = (Wmin.y + Wmax.y) / 2 - (Items[i].BBmin.y + Items[i].BBmax.y) / 2;
		Items[i].object.tZ = origin_Z - (Items[i].BBmin.z + Items[i].BBmax.z) / 2;
		cursor_X += cell_width[i];
		msg(msg::TGT::FILE, msg::TYP::LOG) << Items[i].object.name << " is oversized (" << Item_size[i] << " vs median " << median_size << "): placed outside the Items grid.";
	}

	///////////////////    INSERIMENTO ITEMS NEI FILE FBX E MA
	MA.Layer.push_back(Zone_Items_layer);
	if (exportItemsBoundingBoxes)
		MA.Layer.push_back(Zone_Items_BB_layer);
	for (unsigned int i = 0; i < Items.size(); i++)
	{
		FBX.Group.push_back(Items[i].object);						// Inserimento gruppo oggetto nel file FBX
		MA.Transform.push_back(Items[i].object);					// Inserimento gruppo oggetto nel file MA
		for (unsigned int e = 0; e < Items[i].element.size(); e++)
		{
			FBX.Geometry.push_back(Items[i].element[e]);			// Inserimento elemento geometrico nel file FBX
			MA.Mesh.push_back(Items[i].element[e]);					// Inserimento elemento geometrico nel file MA
		}
	}
	return true;
}
