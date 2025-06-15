#include "stdafx.h"
#include "TRAE/GEOM_Struct.h"
#include "TRAE/TRAE_Functions.h"


bool TRAE_GEOMRAW_Read (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	TRAE_GEOM_HEADER header;
	TRAE_GEOM_VERTEX vertex;
	TRAE_GEOM_TRIANGLE triangle;
	
	///////////////////    APERTURA FILE GEOM
	ifstream geomrawfile(filename, std::ios::binary);
	if (!geomrawfile.is_open())
	{
		cout << "\nFATAL ERROR: " << filename << " not found.";
		return false;
	}

	// Lettura Header
	geomrawfile.read(reinterpret_cast<char*>(&header.ID), sizeof(header.ID));
	if (header.ID != 5065043)
	{
		cout << "\nFATAL ERROR: " << filename << " is not a valid TRAE Geometry RAW file.";
		return false;
	}
	geomrawfile.read(reinterpret_cast<char*>(&header.FileVersion), sizeof(header.FileVersion));
	geomrawfile.read(reinterpret_cast<char*>(&header.nVertices), sizeof(header.nVertices));
	geomrawfile.read(reinterpret_cast<char*>(&header.nTriangles), sizeof(header.nTriangles));
	geomrawfile.read(reinterpret_cast<char*>(&header.nElements), sizeof(header.nElements));
	cout << "\nTRAE Geometry RAW file v. " << header.FileVersion;
	cout << "\nnVerts: " << header.nVertices;
	cout << "\nnTris: " << header.nTriangles;
	cout << "\nnObjects: " << header.nElements;
	geomrawfile.seekg(32, ios_base::cur);						// Salta Unknown1-8
	streamoff vertex_position = geomrawfile.tellg();															// Memorizza la posizione di inizio del blocco vertici
	geomrawfile.seekg(header.nVertices * 400, ios_base::cur);	// Salta il blocco vertici

	vector < vector <unsigned int> > v1 (header.nElements);
	vector < vector <unsigned int> > v2 (header.nElements);
	vector < vector <unsigned int> > v3 (header.nElements);

	// Lettura di tutti le facce e riorganizzazione indici dei vertici per elemento
	for (unsigned int f = 0; f < header.nTriangles; f++)														// Ogni ciclo legge un triangolo
	{
		geomrawfile.read(reinterpret_cast<char*>(&triangle.Element_index), sizeof(triangle.Element_index));
		geomrawfile.read(reinterpret_cast<char*>(&triangle.v1_index), sizeof(triangle.v1_index));
		geomrawfile.read(reinterpret_cast<char*>(&triangle.v2_index), sizeof(triangle.v2_index));
		geomrawfile.read(reinterpret_cast<char*>(&triangle.v3_index), sizeof(triangle.v3_index));
		v1[triangle.Element_index].push_back(triangle.v1_index);
		v2[triangle.Element_index].push_back(triangle.v2_index);
		v3[triangle.Element_index].push_back(triangle.v3_index);
	}

	for (unsigned int el = 0; el < header.nElements; el++)
	{
		Mesh element;				// Classe provvisoria contente i valori letti dal file GEOM. Va copiata nell'apposito array FBX e/o MA al termine dell'estrazione
		stringstream ssname;
		ssname << "TRAE_GEOM_OBJECT_" << el;
		element.name = ssname.str();
		element.binormals_flag = false;
		element.tangents_flag = false;
		element.uv_set1_flag = false;
		element.uv_set2_flag = false;
		element.normals_flag = false;
		//element.vcolors_flag = false;
		vector <unsigned int> vertex_array;
		
		for (unsigned int i = 0; i < v1[el].size(); i++)
		{
			vector <unsigned int>::iterator it2 = find(vertex_array.begin(), vertex_array.end(), v1[el][i]);
			if (it2 == vertex_array.end())												// Se il vertice non viene trovato viene aggiunto alla lista
			{
				vertex_array.push_back(v1[el][i]);
				v1[el][i] = vertex_array.size() - 1;									// Gli indici dei vertici vengono aggiornati in base alla nuova lista vertex_array
			}
			else																		// Se il vertice viene trovato (precedentemente inserito) copia la sua posizione
				v1[el][i] = distance(vertex_array.begin (), it2);						// Gli indici dei vertici vengono aggiornati in base alla nuova lista vertex_array
		}

		for (unsigned int i = 0; i < v2[el].size(); i++)
		{
			vector <unsigned int>::iterator it2 = find(vertex_array.begin(), vertex_array.end(), v2[el][i]);
			if (it2 == vertex_array.end())												// Se il vertice non viene trovato viene aggiunto alla lista
			{
				vertex_array.push_back(v2[el][i]);
				v2[el][i] = vertex_array.size() - 1;									// Gli indici dei vertici vengono aggiornati in base alla nuova lista vertex_array
			}
			else																		// Se il vertice viene trovato (precedentemente inserito) copia la sua posizione
				v2[el][i] = distance(vertex_array.begin (), it2);						// Gli indici dei vertici vengono aggiornati in base alla nuova lista vertex_array
		}

		for (unsigned int i = 0; i < v3[el].size(); i++)
		{
			vector <unsigned int>::iterator it2 = find(vertex_array.begin(), vertex_array.end(), v3[el][i]);
			if (it2 == vertex_array.end())												// Se il vertice non viene trovato viene aggiunto alla lista
			{
				vertex_array.push_back(v3[el][i]);
				v3[el][i] = vertex_array.size() - 1;									// Gli indici dei vertici vengono aggiornati in base alla nuova lista vertex_array
			}
			else																		// Se il vertice viene trovato (precedentemente inserito) copia la sua posizione
				v3[el][i] = distance(vertex_array.begin (), it2);						// Gli indici dei vertici vengono aggiornati in base alla nuova lista vertex_array
		}

		// Aggiunta facce alla classe element
		for (unsigned int f = 0; f < v1[el].size(); f++)
		{
			Face temp;
			temp.v1 = v1[el][f];
			temp.v2 = v2[el][f];
			temp.v3 = v3[el][f];
			element.Face.push_back(temp);
		}

		// Lettura vertici
		element.nV = vertex_array.size();
		for (unsigned int v = 0; v < vertex_array.size(); v++)
		{
			geomrawfile.seekg(vertex_position + vertex_array[v] * 400);
			geomrawfile.seekg(24, ios_base::cur);				// Salta Unknown1-6
			geomrawfile.read(reinterpret_cast<char*>(&vertex.R), sizeof(vertex.R));										// Vertex color R
			geomrawfile.read(reinterpret_cast<char*>(&vertex.G), sizeof(vertex.G));										// Vertex color G
			geomrawfile.read(reinterpret_cast<char*>(&vertex.B), sizeof(vertex.B));										// Vertex color B
			geomrawfile.read(reinterpret_cast<char*>(&vertex.A), sizeof(vertex.A));										// Vertex color A
			geomrawfile.seekg(12, ios_base::cur);				// Salta Padding1, Padding2, Unknown7
			geomrawfile.read(reinterpret_cast<char*>(&vertex.X1), sizeof(vertex.X1));									// Coordinata X (set 1)
			geomrawfile.read(reinterpret_cast<char*>(&vertex.Y1), sizeof(vertex.Y1));									// Coordinata Y (set 1)
			geomrawfile.read(reinterpret_cast<char*>(&vertex.Z1), sizeof(vertex.Z1));									// Coordinata Z (set 1)
			geomrawfile.read(reinterpret_cast<char*>(&vertex.Xn1), sizeof(vertex.Xn1));									// Vertex normal X (set 1)
			geomrawfile.read(reinterpret_cast<char*>(&vertex.Yn1), sizeof(vertex.Yn1));									// Vertex normal Y (set 1)
			geomrawfile.read(reinterpret_cast<char*>(&vertex.Zn1), sizeof(vertex.Zn1));									// Vertex normal Z (set 1)
			element.X.push_back(vertex.X1);
			element.Y.push_back(vertex.Y1);
			element.Z.push_back(vertex.Z1);
			element.Xn.push_back(vertex.Xn1);
			element.Yn.push_back(vertex.Yn1);
			element.Zn.push_back(vertex.Zn1);
			element.R.push_back(vertex.R);
			element.G.push_back(vertex.G);
			element.B.push_back(vertex.B);
			element.A.push_back(1);
		}
		FBX.Geometry.push_back(element);						// Inserimento elemento geometrico nel file FBX
		MA.Mesh.push_back(element);								// Inserimento elemento geometrico nel file MA
	}
	geomrawfile.close();
	return true;
}