#include "stdafx.h"
#include "Classes.h"
#include "MA/MA_Classes.h"


void MA_Write_Mesh (Mesh mesh, MA_EXPORT &MA)
{
	unsigned int n1, n2, i, v1, v2, v3, v4, NumOfEdges = 0;
	vector <Edge> Ed;
	vector <Edge>::iterator it, itr;
	vector <unsigned int> normals_array;
	vector <MA_Face> Fc;
	stringstream out;

	if (mesh.Face.size() == 0)		// Salta l'estrazione se la mesh non ha facce
		return;

	// Preparazione lista normals, lista spigoli e lista facce
	for (unsigned int f = 0; f < mesh.Face.size(); f++)
	{
		MA_Face Face;
		Edge e1, e1r, e2, e2r, e3, e3r, e4, e4r;								// La "r" indica che lo spigolo è ribaltato
		switch (mesh.Face[f].TrisOrQuads)
		{
		case(3):										// TRIANGOLI
			NumOfEdges += 3;
			v1 = mesh.Face[f].v1;
			v2 = mesh.Face[f].v2;
			v3 = mesh.Face[f].v3;
			e1.v1 = v1;		e1.v2 = v2;		e1r.v1 = v2;	e1r.v2 = v1;
			e2.v1 = v2;		e2.v2 = v3;		e2r.v1 = v3;	e2r.v2 = v2;
			e3.v1 = v3;		e3.v2 = v1;		e3r.v1 = v1;	e3r.v2 = v3;
			it = find_if(Ed.begin(), Ed.end(), [&e1] (const Edge& current) {return current.v1 == e1.v1 && current.v2 == e1.v2;});		// Cerca lo spigolo 1 nel vettore degli spigoli
			itr = find_if(Ed.begin(), Ed.end(), [&e1r] (const Edge& current) {return current.v1 == e1r.v1 && current.v2 == e1r.v2;});	// Cerca lo spigolo 1r nel vettore degli spigoli
			if (it != Ed.end())
				Face.e1 = distance(Ed.begin(), it);
			else
				if (itr != Ed.end())
					Face.e1 = distance(Ed.begin(), itr) ^ -1;
				else
				{
					Ed.push_back(e1);
					Face.e1 = Ed.size() - 1;
				}
			it = find_if(Ed.begin(), Ed.end(), [&e2] (const Edge& current) {return current.v1 == e2.v1 && current.v2 == e2.v2;});		// Cerca lo spigolo 2 nel vettore degli spigoli
			itr = find_if(Ed.begin(), Ed.end(), [&e2r] (const Edge& current) {return current.v1 == e2r.v1 && current.v2 == e2r.v2;});	// Cerca lo spigolo 2r nel vettore degli spigoli
			if(it != Ed.end())
				Face.e2 = distance(Ed.begin(), it);
			else
				if (itr != Ed.end())
					Face.e2 = distance(Ed.begin(), itr) ^ -1;
				else
				{
					Ed.push_back(e2);
					Face.e2 = Ed.size() - 1;
				}
			it = find_if(Ed.begin(), Ed.end(), [&e3] (const Edge& current) {return current.v1 == e3.v1 && current.v2 == e3.v2;});		// Cerca lo spigolo 3 nel vettore degli spigoli
			itr = find_if(Ed.begin(), Ed.end(), [&e3r] (const Edge& current) {return current.v1 == e3r.v1 && current.v2 == e3r.v2;});	// Cerca lo spigolo 3r nel vettore degli spigoli
			if(it != Ed.end())
				Face.e3 = distance(Ed.begin(), it);
			else
				if (itr != Ed.end())
					Face.e3 = distance(Ed.begin(), itr) ^ -1;
				else
				{
					Ed.push_back(e3);
					Face.e3 = Ed.size() - 1;
				}
			break;
		case(4):										// QUADRILATERI
			NumOfEdges += 4;
			Face.TrisOrQuads = 4;
			v1 = mesh.Face[f].v1;
			v2 = mesh.Face[f].v2;
			v3 = mesh.Face[f].v3;
			v4 = mesh.Face[f].v4;
			e1.v1 = v1;		e1.v2 = v2;		e1r.v1 = v2;	e1r.v2 = v1;
			e2.v1 = v2;		e2.v2 = v3;		e2r.v1 = v3;	e2r.v2 = v2;
			e3.v1 = v3;		e3.v2 = v4;		e3r.v1 = v4;	e3r.v2 = v3;
			e4.v1 = v4;		e4.v2 = v1;		e4r.v1 = v1;	e4r.v2 = v4;
			it = find_if(Ed.begin(), Ed.end(), [&e1] (const Edge& current) {return current.v1 == e1.v1 && current.v2 == e1.v2;});		// Cerca lo spigolo 1 nel vettore degli spigoli
			itr = find_if(Ed.begin(), Ed.end(), [&e1r] (const Edge& current) {return current.v1 == e1r.v1 && current.v2 == e1r.v2;});	// Cerca lo spigolo 1r nel vettore degli spigoli
			if (it != Ed.end())
				Face.e1 = distance(Ed.begin(), it);
			else
				if (itr != Ed.end())
					Face.e1 = distance(Ed.begin(), itr) ^ -1;
				else
				{
					Ed.push_back(e1);
					Face.e1 = Ed.size() - 1;
				}
			it = find_if(Ed.begin(), Ed.end(), [&e2] (const Edge& current) {return current.v1 == e2.v1 && current.v2 == e2.v2;});		// Cerca lo spigolo 2 nel vettore degli spigoli
			itr = find_if(Ed.begin(), Ed.end(), [&e2r] (const Edge& current) {return current.v1 == e2r.v1 && current.v2 == e2r.v2;});	// Cerca lo spigolo 2r nel vettore degli spigoli
			if(it != Ed.end())
				Face.e2 = distance(Ed.begin(), it);
			else
				if (itr != Ed.end())
					Face.e2 = distance(Ed.begin(), itr) ^ -1;
				else
				{
					Ed.push_back(e2);
					Face.e2 = Ed.size() - 1;
				}
			it = find_if(Ed.begin(), Ed.end(), [&e3] (const Edge& current) {return current.v1 == e3.v1 && current.v2 == e3.v2;});		// Cerca lo spigolo 3 nel vettore degli spigoli
			itr = find_if(Ed.begin(), Ed.end(), [&e3r] (const Edge& current) {return current.v1 == e3r.v1 && current.v2 == e3r.v2;});	// Cerca lo spigolo 3r nel vettore degli spigoli
			if(it != Ed.end())
				Face.e3 = distance(Ed.begin(), it);
			else
				if (itr != Ed.end())
					Face.e3 = distance(Ed.begin(), itr) ^ -1;
				else
				{
					Ed.push_back(e3);
					Face.e3 = Ed.size() - 1;
				}
			it = find_if(Ed.begin(), Ed.end(), [&e4] (const Edge& current) {return current.v1 == e4.v1 && current.v2 == e4.v2;});		// Cerca lo spigolo 4 nel vettore degli spigoli
			itr = find_if(Ed.begin(), Ed.end(), [&e4r] (const Edge& current) {return current.v1 == e4r.v1 && current.v2 == e4r.v2;});	// Cerca lo spigolo 4r nel vettore degli spigoli
			if(it != Ed.end())
				Face.e4 = distance(Ed.begin(), it);
			else
				if (itr != Ed.end())
					Face.e4 = distance(Ed.begin(), itr) ^ -1;
				else
				{
					Ed.push_back(e4);
					Face.e4 = Ed.size() - 1;
				}
			break;
		}

		// Aggiunta indici vertici al vettore normals
		vector <unsigned int> normals_temp;
		switch (Face.TrisOrQuads)
		{
		case(3):
			if (Face.e1 >= 0)
			{
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e1].v1) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e1].v1);
					normals_temp.push_back(Ed[Face.e1].v1);
				}
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e1].v2) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e1].v2);
					normals_temp.push_back(Ed[Face.e1].v2);
				}
			}
			else
			{
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e1 ^ -1].v2) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e1 ^ -1].v2);
					normals_temp.push_back(Ed[Face.e1 ^ -1].v2);
				}
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e1 ^ -1].v1) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e1 ^ -1].v1);
					normals_temp.push_back(Ed[Face.e1 ^ -1].v1);
				}
			}
			if (Face.e2 >= 0)
			{
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e2].v1) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e2].v1);
					normals_temp.push_back(Ed[Face.e2].v1);
				}
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e2].v2) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e2].v2);
					normals_temp.push_back(Ed[Face.e2].v2);
				}
			}
			else
			{
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e2 ^ -1].v2) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e2 ^ -1].v2);
					normals_temp.push_back(Ed[Face.e2 ^ -1].v2);
				}
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e2 ^ -1].v1) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e2 ^ -1].v1);
					normals_temp.push_back(Ed[Face.e2 ^ -1].v1);
				}
			}
			if (Face.e3 >= 0)
			{
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e3].v1) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e3].v1);
					normals_temp.push_back(Ed[Face.e3].v1);
				}
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e3].v2) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e3].v2);
					normals_temp.push_back(Ed[Face.e3].v2);
				}
			}
			else
			{
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e3 ^ -1].v2) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e3 ^ -1].v2);
					normals_temp.push_back(Ed[Face.e3 ^ -1].v2);
				}
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e3 ^ -1].v1) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e3 ^ -1].v1);
					normals_temp.push_back(Ed[Face.e3 ^ -1].v1);
				}
			}
			break;
		case(4):
			if (Face.e1 >= 0)
			{
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e1].v1) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e1].v1);
					normals_temp.push_back(Ed[Face.e1].v1);
				}
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e1].v2) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e1].v2);
					normals_temp.push_back(Ed[Face.e1].v2);
				}
			}
			else
			{
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e1 ^ -1].v2) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e1 ^ -1].v2);
					normals_temp.push_back(Ed[Face.e1 ^ -1].v2);
				}
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e1 ^ -1].v1) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e1 ^ -1].v1);
					normals_temp.push_back(Ed[Face.e1 ^ -1].v1);
				}
			}
			if (Face.e2 >= 0)
			{
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e2].v1) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e2].v1);
					normals_temp.push_back(Ed[Face.e2].v1);
				}
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e2].v2) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e2].v2);
					normals_temp.push_back(Ed[Face.e2].v2);
				}
			}
			else
			{
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e2 ^ -1].v2) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e2 ^ -1].v2);
					normals_temp.push_back(Ed[Face.e2 ^ -1].v2);
				}
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e2 ^ -1].v1) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e2 ^ -1].v1);
					normals_temp.push_back(Ed[Face.e2 ^ -1].v1);
				}
			}
			if (Face.e3 >= 0)
			{
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e3].v1) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e3].v1);
					normals_temp.push_back(Ed[Face.e3].v1);
				}
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e3].v2) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e3].v2);
					normals_temp.push_back(Ed[Face.e3].v2);
				}
			}
			else
			{
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e3 ^ -1].v2) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e3 ^ -1].v2);
					normals_temp.push_back(Ed[Face.e3 ^ -1].v2);
				}
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e3 ^ -1].v1) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e3 ^ -1].v1);
					normals_temp.push_back(Ed[Face.e3 ^ -1].v1);
				}
			}
			if (Face.e4 >= 0)
			{
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e4].v1) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e4].v1);
					normals_temp.push_back(Ed[Face.e4].v1);
				}
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e4].v2) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e4].v2);
					normals_temp.push_back(Ed[Face.e4].v2);
				}
			}
			else
			{
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e4 ^ -1].v2) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e4 ^ -1].v2);
					normals_temp.push_back(Ed[Face.e4 ^ -1].v2);
				}
				if (find(normals_temp.begin(), normals_temp.end(), Ed[Face.e4 ^ -1].v1) == normals_temp.end())
				{
					normals_array.push_back(Ed[Face.e4 ^ -1].v1);
					normals_temp.push_back(Ed[Face.e4 ^ -1].v1);
				}
			}
		}
		Fc.push_back(Face);		
	}

	out << "createNode transform -n \"" << mesh.name << "\"";
	if (mesh.parent.size() > 0)
		out << " -p \"" << mesh.parent << "\"";
	out << ";\n";
	out << "createNode mesh -n \"" << mesh.name << "Shape\" -p \"" << mesh.name << "\";\n";
	out << "	setAttr -k off \".v\";\n";
	out << "	setAttr \".vir\" yes;\n";
	out << "	setAttr \".vif\" yes;\n";

	// Scrittura UVs
	if (mesh.uv_set1_flag && mesh.uv_set2_flag)
		out << "	setAttr -s 2 \".uvst\";\n";

	if (mesh.uv_set1_flag)		// SET UV 1
	{
		n1 = 0;
		n2 = 249;
		i = 0;

		out << "	setAttr \".uvst[0].uvsn\" -type \"string\" \"UVChannel_1\";\n";
		if (mesh.nV > 250)
			out << "	setAttr -s " << mesh.nV << " \".uvst[0].uvsp\";\n";
		do
		{
			n2 = min(mesh.nV - 1, n2);
			out << "	setAttr ";
			if (mesh.nV <= 250)
				out << "-s " << n2 + 1 << " ";
			if (n1 == n2)
				out << "\".uvst[0].uvsp[" << n1 << "]\" ";
			else
				out << "\".uvst[0].uvsp[" << n1 << ":" << n2 << "]\" ";
			if (n1 == 0)															// Scrive il parametro solo al primo gruppo
				out << "-type \"float2\" ";
			for (i = n1; i <= n2; i++)
			{
				out << " " << mesh.U1[i] << " " << mesh.V1[i];						// Scrive i valori
				if ((i + 1) % 5 == 0 && i != n2)									// Va a capo ogni 5
					out << "\n		";
			}
			out << ";\n";															// Chiude il gruppo dati
			n1 += 250;
			n2 += 250;
		} while (i < mesh.nV);
	}

	if (mesh.uv_set2_flag)		// SET UV 2
	{
		n1 = 0;
		n2 = 249;
		i = 0;
		out << "	setAttr \".uvst[1].uvsn\" -type \"string\" \"UVChannel_2\";\n";
		if (mesh.nV > 250)
			out << "	setAttr -s " << mesh.nV << " \".uvst[1].uvsp\";\n";
		do
		{
			n2 = min(mesh.nV - 1, n2);
			out << "	setAttr ";
			if (mesh.nV <= 250)
				out << "-s " << n2 + 1 << " ";
			if (n1 == n2)
				out << "\".uvst[1].uvsp[" << n1 << "]\" ";
			else
				out << "\".uvst[1].uvsp[" << n1 << ":" << n2 << "]\" ";
			if (n1 == 0)															// Scrive il parametro solo al primo gruppo
				out << "-type \"float2\" ";
			for (i = n1; i <= n2; i++)
			{
				out << " " << mesh.U2[i] << " " << mesh.V2[i];						// Scrive i valori
				if ((i + 1) % 5 == 0 && i != n2)									// Va a capo ogni 5
					out << "\n		";
			}
			out << ";\n";															// Chiude il gruppo dati
			n1 += 250;
			n2 += 250;
		} while (i < mesh.nV);
		if (!mesh.uv_set1_flag)
			out << "	setAttr \".cuvs\" -type \"string\" \"UVChannel_2\";\n";
	}

	if (mesh.uv_set1_flag)
		out << "	setAttr \".cuvs\" -type \"string\" \"UVChannel_1\";\n";
	out << "	setAttr \".dcol\" yes;\n";
	out << "	setAttr \".dcc\" -type \"string\" \"Ambient+Diffuse\";\n";
	if (!mesh.doublesided)
		out << "	setAttr \".ds\" no;\n";

	// Scrittura vertex colors
	if (mesh.vcolors_flag)
	{
		n1 = 0;
		n2 = 124;
		i = 0;
		out << "	setAttr \".ccls\" -type \"string\" \"colorSet0\";\n";
		out << "	setAttr \".clst[0].clsn\" -type \"string\" \"colorSet0\";\n";

		if (mesh.nV > 125)
			out << "	setAttr -s " << mesh.nV << " \".clst[0].clsp\";\n";
		do
		{
			n2 = min(mesh.nV - 1, n2);
			out << "	setAttr ";
			if (mesh.nV <= 125)
				out << "-s " << n2 + 1 << " ";
			if (n1 == n2)
				out << "\".clst[0].clsp[" << n1 << "]\" ";
			else
				out << "\".clst[0].clsp[" << n1 << ":" << n2 << "]\" ";
			for (i = n1; i <= n2; i++)
			{
				out << " " << mesh.R[i] << " " << mesh.G[i] << " " << mesh.B[i] << " " << mesh.A[i];	// Scrive i valori
				if ((i + 1) % 3 == 0 && i != n2)														// Va a capo ogni 3
					out << "\n		";
			}
			out << ";\n";																				// Chiude il gruppo dati
			n1 += 125;
			n2 += 125;
		} while (i < mesh.nV);

		/*unsigned int currentvertex = 1;
		unsigned int currentface = 0;
		unsigned int nFaces = 0;
		for (unsigned int j = 0; j < mesh.Face.size(); j++)
			nFaces += mesh.Face[j].TrisOrQuads;

		//cout << "mesh: " << mesh.name << endl;
		//cout << "nFaces " << nFaces << endl;

		if (nFaces > 125)
			out << "	setAttr -s " << nFaces << " \".clst[0].clsp\";\n";
		do
		{
			n2 = min(nFaces - 1, n2);
			out << "	setAttr ";
			if (nFaces <= 125)
				out << "-s " << n2 + 1 << " ";
			if (n1 == n2)
				out << "\".clst[0].clsp[" << n1 << "]\" ";
			else
				out << "\".clst[0].clsp[" << n1 << ":" << n2 << "]\" ";
			
			for (i = n1; i <= n2; i++)
			{
				//cout << "i: " << i << "     n1: " << n1 << "      n2: " << n2 << "      currentface: " << currentface << "       currentvertex: " << currentvertex << endl;

				switch (mesh.Face[currentface].TrisOrQuads)
				{
				case 3:
					switch (currentvertex)
					{
					case 1:
						out << " " << mesh.R[mesh.Face[currentface].v1] << " " << mesh.G[mesh.Face[currentface].v1] << " " << mesh.B[mesh.Face[currentface].v1] << " " << mesh.A[mesh.Face[currentface].v1];	// Scrive i valori
						currentvertex = 2;
						break;
					case 2:
						out << " " << mesh.R[mesh.Face[currentface].v2] << " " << mesh.G[mesh.Face[currentface].v2] << " " << mesh.B[mesh.Face[currentface].v2] << " " << mesh.A[mesh.Face[currentface].v2];	// Scrive i valori
						currentvertex = 3;
						break;
					case 3:
						out << " " << mesh.R[mesh.Face[currentface].v3] << " " << mesh.G[mesh.Face[currentface].v3] << " " << mesh.B[mesh.Face[currentface].v3] << " " << mesh.A[mesh.Face[currentface].v3];	// Scrive i valori
						currentvertex = 1;
						currentface++;
						break;
					}
					break;

				case 4:
					switch (currentvertex)
					{
					case 1:
						out << " " << mesh.R[mesh.Face[currentface].v1] << " " << mesh.G[mesh.Face[currentface].v1] << " " << mesh.B[mesh.Face[currentface].v1] << " " << mesh.A[mesh.Face[currentface].v1];	// Scrive i valori
						currentvertex = 2;
						break;
					case 2:
						out << " " << mesh.R[mesh.Face[currentface].v2] << " " << mesh.G[mesh.Face[currentface].v2] << " " << mesh.B[mesh.Face[currentface].v2] << " " << mesh.A[mesh.Face[currentface].v2];	// Scrive i valori
						currentvertex = 3;
						break;
					case 3:
						out << " " << mesh.R[mesh.Face[currentface].v3] << " " << mesh.G[mesh.Face[currentface].v3] << " " << mesh.B[mesh.Face[currentface].v3] << " " << mesh.A[mesh.Face[currentface].v3];	// Scrive i valori
						currentvertex = 4;
						break;
					case 4:
						out << " " << mesh.R[mesh.Face[currentface].v4] << " " << mesh.G[mesh.Face[currentface].v4] << " " << mesh.B[mesh.Face[currentface].v4] << " " << mesh.A[mesh.Face[currentface].v4];	// Scrive i valori
						currentvertex = 1;
						currentface++;
					}
				}

				if ((i + 1) % 3 == 0 && i != n2)														// Va a capo ogni 3
					out << "\n		";
			}
			out << ";\n";																				// Chiude il gruppo dati
			n1 += 125;
			n2 += 125;
		} while (i < nFaces);*/
	}

	// Scrittura vertici
	n1 = 0;
	n2 = 165;
	i = 0;

	if (mesh.nV > 166)
		out << "	setAttr -s " << mesh.nV << " \".vt\";\n";
	do
	{
		n2 = min(mesh.nV - 1, n2);
		out << "	setAttr ";
		if (mesh.nV <= 166)
			out << "-s " << n2 + 1 << " ";
		if (n1 == n2)
			out << "\".vt[" << n1 << "]\" ";
		else
			out << "\".vt[" << n1 << ":" << n2 << "]\" ";
		for (i = n1; i <= n2; i++)
		{
			out << " " << mesh.X[i] << " " << mesh.Y[i] << " " << mesh.Z[i];		// Scrive i valori
			if ((i + 1) % 5 == 0 && i != n2)										// Va a capo ogni 5
				out << "\n		";
		}
		out << ";\n";																// Chiude il gruppo dati
		n1 += 166;
		n2 += 166;
	} while (i < mesh.nV);

	// Scrittura spigoli e preparazione lista normals
	n1 = 0;
	n2 = 165;
	i = 0;
	if (Ed.size() > 166)
		out << "	setAttr -s " << Ed.size() << " \".ed\";\n";
	do
	{
		n2 = min(Ed.size() - 1, n2);
		out << "	setAttr ";
		if (Ed.size() <= 166)
			out << "-s " << n2 + 1 << " ";
		if (n1 == n2)
			out << "\".ed[" << n1 << "]\" ";
		else
			out << "\".ed[" << n1 << ":" << n2 << "]\" ";
		for (i = n1; i <= n2; i++)
		{
			out << " " << Ed[i].v1 << " " << Ed[i].v2 << " 0";						// Scrive i valori. Il numero 1 alla fine indica un "hard edge"
			if ((i + 1) % 9 == 0 && i != n2)										// Va a capo ogni 9
				out << "\n		";
		}
		out << ";\n";																// Chiude il gruppo dati
		n1 += 166;
		n2 += 166;
	} while (i < Ed.size());

	// Scrittura normals
	// N.B. I normals non sono inseriti nello stesso ordine dei vertici ma in base all'ordine in cui tale vertice appare nella lista degli spigoli
	if (mesh.normals_flag)
	{
		n1 = 0;
		n2 = 165;
		i = 0;

		if (normals_array.size() > 166)
			out << "	setAttr -s " << normals_array.size() << " \".n\";\n";
		do
		{
			n2 = min(normals_array.size() - 1, n2);
			out << "	setAttr ";
			if (normals_array.size() <= 166)
				out << "-s " << n2 + 1 << " ";
			if (n1 == n2)
				out << "\".n[" << n1 << "]\" -type \"float3\" ";
			else
				out << "\".n[" << n1 << ":" << n2 << "]\" -type \"float3\" ";
			for (i = n1; i <= n2; i++)
			{
				out << " " << mesh.Xn[normals_array[i]] << " " << mesh.Yn[normals_array[i]] << " " << mesh.Zn[normals_array[i]];		// Scrive i valori
				if ((i + 1) % 5 == 0 && i != n2)										// Va a capo ogni 5
					out << "\n		";
			}
			out << ";\n";																// Chiude il gruppo dati
			n1 += 166;
			n2 += 166;
		} while (i < normals_array.size());
	}

	// Scrittura facce
	n1 = 0;
	n2 = 499;
	i = 0;
	unsigned int vc_counter = 0;

	if (Fc.size() > 500)
		out << "	setAttr -s " << Fc.size() << " -ch " << NumOfEdges << " \".fc\";\n";
	do
	{
		n2 = min(Fc.size() - 1, n2);
		out << "	setAttr ";
		if (Fc.size() <= 500 && Fc.size() != 1)
			out << "-s " << n2 + 1 << " -ch " << NumOfEdges << " ";
		if (Fc.size() == 1)
			out << "-ch " << NumOfEdges << " \".fc[0]\" -type \"polyFaces\"";
		else
			out << "\".fc[" << n1 << ":" << n2 << "]\" -type \"polyFaces\"";
		for (i = n1; i <= n2; i++)
		{
			out << "\n";
			if (Fc[i].TrisOrQuads == 3)
			{
				out << "		f 3 " << Fc[i].e1 << " " << Fc[i].e2 << " " << Fc[i].e3 << "\n";															// Facce
				if (mesh.uv_set1_flag)
					out << "		mu 0 3 " << mesh.Face[i].v1 << " " << mesh.Face[i].v2 << " " << mesh.Face[i].v3 << "\n";								// Set UV 1
				if (mesh.uv_set2_flag)
					out << "		mu 1 3 " << mesh.Face[i].v1 << " " << mesh.Face[i].v2 << " " << mesh.Face[i].v3 << "\n";								// Set UV 2
				if (mesh.vcolors_flag)
					out << "		mc 0 3 " << mesh.Face[i].v1 << " " << mesh.Face[i].v2 << " " << mesh.Face[i].v3;										// Vertex color
				vc_counter += 3;
			}
			if (Fc[i].TrisOrQuads == 4)
			{
				out << "		f 4 " << Fc[i].e1 << " " << Fc[i].e2 << " " << Fc[i].e3 << " " << Fc[i].e4 << "\n";											// Facce
				if (mesh.uv_set1_flag)
					out << "		mu 0 4 " << mesh.Face[i].v1 << " " << mesh.Face[i].v2 << " " << mesh.Face[i].v3 << " " << mesh.Face[i].v4 <<"\n";		// Set UV 1
				if (mesh.uv_set2_flag)
					out << "		mu 1 4 " << mesh.Face[i].v1 << " " << mesh.Face[i].v2 << " " << mesh.Face[i].v3 << " " << mesh.Face[i].v4 <<"\n";		// Set UV 2
				if (mesh.vcolors_flag)
					out << "		mc 0 4 " << mesh.Face[i].v1 << " " << mesh.Face[i].v2 << " " << mesh.Face[i].v3 << " " << mesh.Face[i].v4;				// Vertex color
				vc_counter += 4;
			}
		}
		out << ";\n";																// Chiude il gruppo dati
		n1 += 500;
		n2 += 500;
	} while (i < Fc.size());

	out << "	setAttr \".cd\" -type \"dataPolyComponent\" Index_Data Edge 0;\n";
	out << "	setAttr \".cvd\" -type \"dataPolyComponent\" Index_Data Vertex 0;\n";
	if (mesh.uv_set1_flag && mesh.uv_set2_flag)
		out << "	setAttr -s 2 \".pd\";\n";
	if (mesh.uv_set1_flag)
		out << "	setAttr \".pd[0]\" -type \"dataPolyComponent\" Index_Data UV 0;\n";
	if (mesh.uv_set2_flag)
		out << "	setAttr \".pd[1]\" -type \"dataPolyComponent\" Index_Data UV 0;\n";
	out << "	setAttr \".hfd\" -type \"dataPolyComponent\" Index_Data Face 0;\n";
	out << "	setAttr \".matb\" 3;\n";
	out << "	setAttr \".db\" yes;\n";
	out << "	setAttr \".bw\" 1.704961;\n";

	MA.MA_Nodes << out.str();
	out.str("");

	if (mesh.material_name.size() > 0)
		out << "connectAttr \"" << mesh.name << "Shape.iog\" \"" << mesh.material_name << "SG.dsm\" -na;\n";
	else
		out << "connectAttr \"" << mesh.name << "Shape.iog\" \":initialShadingGroup.dsm\" -na;\n";

	if (mesh.layer.size() > 0)
		out << "connectAttr \"" << mesh.layer << ".di\" \"" << mesh.name << ".do\";\n";

	MA.MA_Connections << out.str();
}