#include "stdafx.h"
#include "TRAOD/ZONE/ZONE_Struct.h"
#include "TRAOD/ZONE/ZONE_Functions.h"

// Supporto al multithreading abbandonato, ignorare questa funzione

void Export_Element (atomic_int &ActiveThreads,
					 unsigned int r,
					 unsigned int el,
					 unsigned int RoomID,
					 streamoff elements_position,
					 streamoff strip_position,
					 streamoff vertex_position,
					 ifstream &zonefile,
					 FBX_EXPORT &FBX,
					 MA_EXPORT &MA)
{
	ZONE_MESH_VERTEX zone_mesh_vertex;
	ZONE_MESH_STRIP zone_mesh_strip;
	ZONE_MESH_ELEMENT zone_mesh_element;

	Mesh element;				// Classe provvisoria contente i valori letti dal file ZONE. Va copiata nell'apposito array FBX e/o MA al termine dell'estrazione
	stringstream ssname, ssparent;
	ssname << "ROOM_" << r << "_OBJ_" << el;
	ssparent << "ROOM_" << r << "_" << std::hex << RoomID << std::dec;
	element.name = ssname.str();
	element.parent = ssparent.str();
	element.FBX_parent = hashID(ssparent.str(), "Group");

	// Lettura dati elemento
	mu.lock();
	zonefile.seekg(elements_position + el * 64);
	zonefile.seekg(4, ios_base::cur);		// Salta nElement_Triangles
	zonefile.read(reinterpret_cast<char*>(&zone_mesh_element.nElement_Indices), sizeof(zone_mesh_element.nElement_Indices));	// Numero di indici dello strip
	zonefile.read(reinterpret_cast<char*>(&zone_mesh_element.Offset), sizeof(zone_mesh_element.Offset));						// Offset nello strip
	zonefile.read(reinterpret_cast<char*>(&zone_mesh_element.Material_Ref), sizeof(zone_mesh_element.Material_Ref));			// ID materiale
	zonefile.seekg(12, ios_base::cur);		// Salta Unknown1, Vbuffer_min e Vbuffer_max
	zonefile.read(reinterpret_cast<char*>(&zone_mesh_element.Draw_mode), sizeof(zone_mesh_element.Draw_mode));					// Tipologia di rendering

	// Lettura strip
	vector <unsigned int> strip(zone_mesh_element.nElement_Indices);
	vector <unsigned int> vertex_array;
	zonefile.seekg(strip_position + zone_mesh_element.Offset * 2);					// Posizionamento cursore di lettura all'inizio dello strip dell'elemento el
	for (unsigned int i = 0; i < zone_mesh_element.nElement_Indices; i++)			// Lettura strip del singolo elemento el
		zonefile.read(reinterpret_cast<char*>(&strip[i]), sizeof(zone_mesh_strip.Index));
	mu.unlock();
	for (unsigned int i = 0; i < strip.size(); i++)
	{
		vector <unsigned int>::iterator it = find(vertex_array.begin(), vertex_array.end(), strip[i]);
		if (it == vertex_array.end())												// Se il vertice non viene trovato viene aggiunto alla lista
		{
			vertex_array.push_back(strip[i]);
			strip[i] = vertex_array.size() - 1;										// Gli indici dei vertici vengono aggiornati in base alla nuova lista vertex_array
		}
		else																		// Se il vertice viene trovato (precedentemente inserito) copia la sua posizione
			strip[i] = distance(vertex_array.begin (), it);							// Gli indici dei vertici vengono aggiornati in base alla nuova lista vertex_array
	}
	element.Face = Calculate_Faces(strip, zone_mesh_element.Offset, zone_mesh_element.Draw_mode);		// Creazione lista facce
	element.nV = vertex_array.size();

	// Lettura vertici
	for (unsigned int v = 0; v < vertex_array.size(); v++)
	{
		mu.lock();
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
		mu.unlock();
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
		element.R.push_back((float)zone_mesh_vertex.VC_red / 255);
		element.G.push_back((float)zone_mesh_vertex.VC_green / 255);
		element.B.push_back((float)zone_mesh_vertex.VC_blue / 255);
	}
	mu.lock();
	FBX.Geometry.push_back(element);						// Inserimento elemento geometrico nel file FBX
	MA.Mesh.push_back(element);								// Inserimento elemento geometrico nel file MA
	mu.unlock();
	ActiveThreads--;
}


bool ZONE_Read_Rooms_MT (string name, string name2, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	//chrono::steady_clock::time_point begin = chrono::steady_clock::now();
	ZONE_HEADER zone_header;
	//ZONE_MATERIALS_HEADER zone_materials_header;
	//ZONE_MATERIALS_LIST zone_materials_list;
	//ZONE_TEXTURE_LIST zone_texture_list;
	ZONE_MESH_HEADER1 zone_mesh_header1;
	ZONE_MESH_ROOM_HEADER zone_mesh_room_header;
	//ZONE_MESH_OBJECT_HEADER zone_mesh_object_header;

	// LETTURA FILE ZONE
	ifstream zonefile(name, std::ios::binary);

	// Lettura Header
	zonefile.seekg(4);																							// Salta i primi 4 byte dell'ID
	zonefile.read(reinterpret_cast<char*>(&zone_header.TEXTURE_PTR), sizeof(zone_header.TEXTURE_PTR));
	zonefile.seekg(4, ios_base::cur);																			// Salta UNKNOWN_PTR
	zonefile.read(reinterpret_cast<char*>(&zone_header.MESH_PTR), sizeof(zone_header.MESH_PTR));
	zonefile.read(reinterpret_cast<char*>(&zone_header.EOF_PTR), sizeof(zone_header.EOF_PTR));

	// Lettura stanze
	zonefile.seekg(zone_header.MESH_PTR);
	zonefile.read(reinterpret_cast<char*>(&zone_mesh_header1.nRooms), sizeof(zone_mesh_header1.nRooms));			// Lettura numero stanze

	for (unsigned int r = 0; r < zone_mesh_header1.nRooms; r++)													// Questo macroblocco "for" legge tutti i dati di ogni stanza
	{
		streamoff header_position = zonefile.tellg();							// Memorizza la posizione iniziale del blocco vertici
		zonefile.read(reinterpret_cast<char*>(&zone_mesh_room_header.RoomID), sizeof(zone_mesh_room_header.RoomID));			// ID stanza
		zonefile.read(reinterpret_cast<char*>(&zone_mesh_room_header.Room_size), sizeof(zone_mesh_room_header.Room_size));		// Dimensioni stanza bytes
		zonefile.seekg(8, ios_base::cur);		// Salta i due unknown1/2
		zonefile.read(reinterpret_cast<char*>(&zone_mesh_room_header.nVertices), sizeof(zone_mesh_room_header.nVertices));		// Numero vertici
		zonefile.seekg(4, ios_base::cur);		// Salta unknown3
		zonefile.read(reinterpret_cast<char*>(&zone_mesh_room_header.nIndices), sizeof(zone_mesh_room_header.nIndices));		// Numero indici del triangle strip
		zonefile.seekg(4, ios_base::cur);		// Salta unknown4
		zonefile.read(reinterpret_cast<char*>(&zone_mesh_room_header.nElements), sizeof(zone_mesh_room_header.nElements));		// Numero elementi
		zonefile.seekg(20, ios_base::cur);		// Salta unknown5/6/7/8/9
		streamoff vertex_position = header_position + 56;											// Memorizza la posizione iniziale del blocco vertici
		streamoff strip_position = vertex_position + zone_mesh_room_header.nVertices * 40;			// Memorizza la posizione iniziale del blocco triangle strip
		streamoff elements_position = strip_position + zone_mesh_room_header.nIndices * 2;			// Memorizza la posizione iniziale del blocco elementi

		Transform room;
		stringstream temp;
		temp << "ROOM_" << r << "_" << std::hex << zone_mesh_room_header.RoomID << std::dec;
		room.name = temp.str();
		FBX.Group.push_back(room);									// Inserimento gruppo stanza nel file FBX
		MA.Transform.push_back(room);								// Inserimento gruppo stanza nel file MA

		vector <thread> Threads_Element;
		atomic_int ActiveThreads = 0;

		for (unsigned int el = 0; el < zone_mesh_room_header.nElements;)
		{
			if (ActiveThreads < MaxThreadLimit)
			{
				ActiveThreads++;
				Threads_Element.push_back(thread(Export_Element, ref(ActiveThreads), r, el, zone_mesh_room_header.RoomID, elements_position, strip_position, vertex_position, ref(zonefile), ref(FBX), ref(MA)));
				el++;
			}

		}
		for (unsigned int t = 0; t < Threads_Element.size(); t++)
			Threads_Element[t].join();

		zonefile.seekg(header_position + zone_mesh_room_header.Room_size + 4);			// Posizione l'indice di lettura all'inizio della stanza successiva
	}
	//chrono::steady_clock::time_point end= chrono::steady_clock::now();
	//cout << " Zone read time = " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " milliseconds" << endl;
	return true;
}