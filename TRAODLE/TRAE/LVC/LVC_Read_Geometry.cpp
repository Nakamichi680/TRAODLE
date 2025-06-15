#include "stdafx.h"
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "TRAE/LVC/LVC_Struct.h"
#include "TRAE/TEXSET/TEXSET_Struct.h"
#include "Misc_Functions.h"


bool LVC_Read_Geometry (string filename, FBX_EXPORT &FBX, MA_EXPORT &MA)
{
	LVC_HEADER header;
	LVC_GEOMETRY_POINTER geometry_pointer;
	LVC_GEOMETRY_HEADER geometry_header;
	LVC_GEOMETRY_MESH_POINTER mesh_pointer;
	LVC_GEOMETRY_MESH_HEADER mesh_header;
	LVC_GEOMETRY_MESH_VERTEX mesh_vertex;
	TEXSET_HEADER texset_header;
	TEXSET_OFFSET texset_offset;
	TEXSET_INDEX1 texset_index1;
	TEXSET_TEXTURE_DATA texset_data;
	LVC_UNKNOWN1_HEADER unknown1;
	LVC_UNKNOWN1_VERTS unknown1_verts;
	string lvcname = filename.substr(0, filename.size() - 4);


	///////////////////    APERTURA FILE LVC
	SetCurrentDirectory(AE_IO.folder_level_lpwstr);								// Imposta la cartella \NOMELIVELLO come attiva
	ifstream lvcfile(filename, std::ios::binary);
	if (!lvcfile.is_open())
	{
		cout << "\nFATAL ERROR: " << filename << " not found.";
		return false;
	}

	// Lettura Header
	lvcfile.read(reinterpret_cast<char*>(&header.Magic1), sizeof(header.Magic1));								// Lettura tipologia di file (96h)
	lvcfile.read(reinterpret_cast<char*>(&header.TargetPlatform), sizeof(header.TargetPlatform));				// Lettura piattaforma
	lvcfile.read(reinterpret_cast<char*>(&header.Magic2), sizeof(header.Magic2));								// Lettura tipologia di file ("LVC")
	
	if (header.Magic2 != 4413004)							// Se il file non è valido la funzione termina
	{
		cout << "\nFATAL ERROR: " << filename << " is not a valid TRAE LVC file.";
		return false;
	}

	if (header.TargetPlatform != 0x100)
	{
		cout << "\nFATAL ERROR: " << filename << " is not a PC LVC file. PSP and PS2 LVC files are not supported.";
		return false;
	}

	if (header.Magic1 == 150)
		cout << "\nTRAE LVC file v. 1.50 detected";
	if (header.Magic1 == 143)
		cout << "\nTRAE LVC file v. 1.43 detected";

	lvcfile.read(reinterpret_cast<char*>(&header.Unknown1), sizeof(header.Unknown1));
	lvcfile.read(reinterpret_cast<char*>(&header.nGeomPointers1), sizeof(header.nGeomPointers1));
	lvcfile.read(reinterpret_cast<char*>(&header.Unknown2), sizeof(header.Unknown2));
	lvcfile.read(reinterpret_cast<char*>(&header.Unknown3), sizeof(header.Unknown3));
	if (header.Magic1 == 150)
		lvcfile.read(reinterpret_cast<char*>(&header.nGeomPointers2), sizeof(header.nGeomPointers2));
	lvcfile.seekg(8, ios_base::cur);									// Salta Name
	lvcfile.read(reinterpret_cast<char*>(&header.PtrToGeomPointers), sizeof(header.PtrToGeomPointers));
	lvcfile.seekg(12, ios_base::cur);									// Salta PtrUnknown1, PtrUnknown2, PtrUnknown3
	lvcfile.read(reinterpret_cast<char*>(&header.PtrToTEXSET), sizeof(header.PtrToTEXSET));
	lvcfile.read(reinterpret_cast<char*>(&header.PtrToUNKNOWN1), sizeof(header.PtrToUNKNOWN1));

	// ESPORTAZIONE GEOMETRIA
	for (unsigned int g = 0; g < header.nGeomPointers1; g++)					// Ogni ciclo legge un gruppo geometry
	{
		stringstream temp;
		Transform group;
		temp << "LVC_" << lvcname << "_ROOM_" << g;
		group.name = temp.str();
		FBX.Group.push_back(group);								// Inserimento gruppo stanza nel file FBX
		MA.Transform.push_back(group);							// Inserimento gruppo stanza nel file MA

		lvcfile.seekg(header.PtrToGeomPointers + g * 4);
		lvcfile.read(reinterpret_cast<char*>(&geometry_pointer.Pointer), sizeof(geometry_pointer.Pointer));
		lvcfile.seekg(geometry_pointer.Pointer);
		lvcfile.seekg(44, ios_base::cur);								// Salta Unknown1
		lvcfile.read(reinterpret_cast<char*>(&geometry_header.nMeshes), sizeof(geometry_header.nMeshes));
		lvcfile.read(reinterpret_cast<char*>(&geometry_header.Unknown2), sizeof(geometry_header.Unknown2));
		lvcfile.read(reinterpret_cast<char*>(&geometry_header.PtrToIndices), sizeof(geometry_header.PtrToIndices));
		lvcfile.read(reinterpret_cast<char*>(&geometry_header.PtrToEndOfGeometry), sizeof(geometry_header.PtrToEndOfGeometry));
		streamoff mesh_pointer_begin_position = lvcfile.tellg();

		for (unsigned int m = 0; m < geometry_header.nMeshes; m++)				// Ogni ciclo legge un gruppo mesh
		{
			Mesh element;
			stringstream ssname;
			ssname << "LVC_" << lvcname << "_ROOM_" << g << "_OBJ_" << m;
			element.name = ssname.str();
			element.parent = group.name;
			element.FBX_parent = hashID(group.name, "Group");
			element.uv_set2_flag = false;
			element.normals_flag = false;
			element.binormals_flag = false;
			element.tangents_flag = false;

			lvcfile.seekg(mesh_pointer_begin_position + m * 4);
			lvcfile.read(reinterpret_cast<char*>(&mesh_pointer.Pointer), sizeof(mesh_pointer.Pointer));
			lvcfile.seekg(geometry_pointer.Pointer + mesh_pointer.Pointer);
			lvcfile.seekg(100, ios_base::cur);								// Salta Unknown1
			lvcfile.read(reinterpret_cast<char*>(&mesh_header.Material_hash), sizeof(mesh_header.Material_hash));
			lvcfile.seekg(192, ios_base::cur);								// Salta Unknown2
			lvcfile.read(reinterpret_cast<char*>(&mesh_header.nVertices), sizeof(mesh_header.nVertices));
			lvcfile.read(reinterpret_cast<char*>(&mesh_header.Unknown3), sizeof(mesh_header.Unknown3));

			element.nV = mesh_header.nVertices;

			// Aggiunta materiale
			stringstream ssmaterial;
			ssmaterial << "LVC_" <<  lvcname << "_Material_" << std::hex << mesh_header.Material_hash;
			element.material_name = ssmaterial.str();

			stringstream temp2;
			temp2 << filename << "_room_" << g << "_mesh_" << m << ".obj";

			for (unsigned int v = 0; v < mesh_header.nVertices; v++)			// Ogni ciclo legge un vertice
			{
				lvcfile.read(reinterpret_cast<char*>(&mesh_vertex.X), sizeof(mesh_vertex.X));
				lvcfile.read(reinterpret_cast<char*>(&mesh_vertex.Y), sizeof(mesh_vertex.Y));
				lvcfile.read(reinterpret_cast<char*>(&mesh_vertex.Z), sizeof(mesh_vertex.Z));
				lvcfile.read(reinterpret_cast<char*>(&mesh_vertex.VC_red), sizeof(mesh_vertex.VC_red));
				lvcfile.read(reinterpret_cast<char*>(&mesh_vertex.VC_green), sizeof(mesh_vertex.VC_green));
				lvcfile.read(reinterpret_cast<char*>(&mesh_vertex.VC_blue), sizeof(mesh_vertex.VC_blue));
				lvcfile.read(reinterpret_cast<char*>(&mesh_vertex.VC_alpha), sizeof(mesh_vertex.VC_alpha));
				lvcfile.read(reinterpret_cast<char*>(&mesh_vertex.U), sizeof(mesh_vertex.U));
				lvcfile.read(reinterpret_cast<char*>(&mesh_vertex.V), sizeof(mesh_vertex.V));
				lvcfile.read(reinterpret_cast<char*>(&mesh_vertex.Padding1), sizeof(mesh_vertex.Padding1));
				lvcfile.read(reinterpret_cast<char*>(&mesh_vertex.Padding2), sizeof(mesh_vertex.Padding2));
				lvcfile.read(reinterpret_cast<char*>(&mesh_vertex.Padding3), sizeof(mesh_vertex.Padding3));
				element.X.push_back(mesh_vertex.X);
				element.Y.push_back(mesh_vertex.Y);
				element.Z.push_back(mesh_vertex.Z);
				element.U1.push_back(mesh_vertex.U);
				element.V1.push_back(mesh_vertex.V);
				element.R.push_back((float)mesh_vertex.VC_red / 255);
				element.G.push_back((float)mesh_vertex.VC_green / 255);
				element.B.push_back((float)mesh_vertex.VC_blue / 255);
				element.A.push_back(1);
			}
			
			for (unsigned int v = 0; v < mesh_header.nVertices - 2; v++)			// Ogni ciclo legge un vertice
			{
				if (v % 2 == 0)		// Se pari
				{
					Face lvcface;
					lvcface.v1 = v;
					lvcface.v2 = v+1;
					lvcface.v3 = v+2;
					element.Face.push_back(lvcface);
				}
				else				// Se dispari
				{
					Face lvcface;
					lvcface.v1 = v+2;
					lvcface.v2 = v+1;
					lvcface.v3 = v;
					element.Face.push_back(lvcface);
				}
			}

			// Controllo mesh single/double sided
			lvcfile.seekg(header.PtrToTEXSET + 8);
			lvcfile.read(reinterpret_cast<char*>(&texset_header.PtrToMaterialsHeader), sizeof(texset_header.PtrToMaterialsHeader));
			lvcfile.read(reinterpret_cast<char*>(&texset_header.PtrToTextureData), sizeof(texset_header.PtrToTextureData));			// Puntatore ad inizio blocco dati texture
			lvcfile.seekg(16, ios_base::cur);
			lvcfile.read(reinterpret_cast<char*>(&texset_header.nTextures), sizeof(texset_header.nTextures));						// Numero di texture
			for (unsigned int m = 0; m < texset_header.nTextures; m++)
			{
				lvcfile.seekg(header.PtrToTEXSET + texset_header.PtrToMaterialsHeader + 4 + 8 * m);
				lvcfile.read(reinterpret_cast<char*>(&texset_offset.hash), sizeof(texset_offset.hash));
				if (texset_offset.hash == mesh_header.Material_hash)
				{
					lvcfile.read(reinterpret_cast<char*>(&texset_offset.Texture_offset), sizeof(texset_offset.Texture_offset));
					lvcfile.seekg(header.PtrToTEXSET + texset_header.PtrToTextureData + texset_offset.Texture_offset + 4);
					lvcfile.read(reinterpret_cast<char*>(&texset_data.MaterialType), sizeof(texset_data.MaterialType));				// Tipologia di materiale
					if (texset_data.MaterialType == 65 || texset_data.MaterialType == 81)
						element.doublesided = false;
				}
			}
			FBX.Geometry.push_back(element);						// Inserimento elemento geometrico nel file FBX
			MA.Mesh.push_back(element);								// Inserimento elemento geometrico nel file MA
		}
	}

	// ESPORTAZIONE MATERIALI E TEXTURES
	lvcfile.seekg(header.PtrToTEXSET);
	lvcfile.read(reinterpret_cast<char*>(&texset_header.Magic1), sizeof(texset_header.Magic1));							// Lettura tipologia di file (9)
	lvcfile.read(reinterpret_cast<char*>(&texset_header.TargetPlatform), sizeof(texset_header.TargetPlatform));			// Lettura piattaforma
	lvcfile.read(reinterpret_cast<char*>(&texset_header.Magic2), sizeof(texset_header.Magic2));							// Lettura tipologia di file ("TXS")
	
	if (texset_header.Magic1 != 9 || texset_header.Magic2 != 5462100)				// Se il file non è valido la funzione termina
	{
		cout << "\nFATAL ERROR: " << filename << " does not contain a valid TRAE Texture Set file.";
		return false;
	}

	if (texset_header.TargetPlatform != 0x100)
	{
		cout << "\nFATAL ERROR: " << filename << " does not contain a PC Texture Set file. PSP and PS2 Texture Set files are not supported.";
		return false;
	}

	lvcfile.read(reinterpret_cast<char*>(&texset_header.PtrToMaterialsHeader), sizeof(texset_header.PtrToMaterialsHeader));
	lvcfile.read(reinterpret_cast<char*>(&texset_header.PtrToTextureData), sizeof(texset_header.PtrToTextureData));				// Puntatore ad inizio blocco dati texture
	lvcfile.read(reinterpret_cast<char*>(&texset_header.PtrToMaterialsIndices), sizeof(texset_header.PtrToMaterialsIndices));	// Puntatore ad inizio blocco indici 1
	lvcfile.read(reinterpret_cast<char*>(&texset_header.PtrToTextureInfo), sizeof(texset_header.PtrToTextureInfo));				// Puntatore ad inizio blocco info texture
	lvcfile.read(reinterpret_cast<char*>(&texset_header.PtrToIndices2), sizeof(texset_header.PtrToIndices2));					// Puntatore ad inizio blocco indici 2
	lvcfile.read(reinterpret_cast<char*>(&texset_header.PtrToSomething4), sizeof(texset_header.PtrToSomething4));
	lvcfile.read(reinterpret_cast<char*>(&texset_header.nTextures), sizeof(texset_header.nTextures));							// Numero di texture

	stringstream sspath;
	sspath << AE_IO.folder_geometry << lvcname << "_Textures";
	//string folder_temp2 = sspath.str();
	//LPWSTR folder_temp2_lpwstr = new TCHAR[MAX];
	//folder_temp2.append("\\Originals");
	mbstowcs(AE_IO.folder_temp_lpwstr, sspath.str().c_str(), MAX);
	//mbstowcs(folder_temp2_lpwstr, folder_temp2.c_str(), MAX);
	CreateDirectory(AE_IO.folder_temp_lpwstr, NULL);								// Crea la cartella \NOMELIVELLO\Geometry\LVCNAME_Textures
	//CreateDirectory(folder_temp2_lpwstr, NULL);										// Crea la cartella \NOMELIVELLO\Geometry\LVCNAME_Textures\Originals
	
	ofstream debug;
	debug.open("debug_texset.txt");

	for (unsigned int m = 0; m < texset_header.nTextures; m++)
	{
		lvcfile.seekg(header.PtrToTEXSET + texset_header.PtrToMaterialsHeader + 4 + 8 * m);
		lvcfile.read(reinterpret_cast<char*>(&texset_offset.hash), sizeof(texset_offset.hash));
		lvcfile.read(reinterpret_cast<char*>(&texset_offset.Texture_offset), sizeof(texset_offset.Texture_offset));
		lvcfile.seekg(header.PtrToTEXSET + texset_header.PtrToMaterialsIndices + 4 + 8 * m);
		lvcfile.read(reinterpret_cast<char*>(&texset_index1.hash), sizeof(texset_index1.hash));
		lvcfile.read(reinterpret_cast<char*>(&texset_index1.Texture_ID), sizeof(texset_index1.Texture_ID));
		lvcfile.seekg(header.PtrToTEXSET + texset_header.PtrToTextureData + texset_offset.Texture_offset);
		lvcfile.read(reinterpret_cast<char*>(&texset_data.Xsize), sizeof(texset_data.Xsize));							// Dimensione X della texture (esponente di 2)
		lvcfile.read(reinterpret_cast<char*>(&texset_data.Ysize), sizeof(texset_data.Ysize));							// Dimensione Y della texture (esponente di 2)
		lvcfile.read(reinterpret_cast<char*>(&texset_data.MaterialType), sizeof(texset_data.MaterialType));				// Tipologia di materiale
		lvcfile.read(reinterpret_cast<char*>(&texset_data.Texture_hash), sizeof(texset_data.Texture_hash));				// Hash della texture
		lvcfile.seekg(4, ios_base::cur);									// Salta DEADBEAF
		lvcfile.read(reinterpret_cast<char*>(&texset_data.Texture_name), sizeof(texset_data.Texture_name));				// Nome della texture
		unsigned int RAWsize_bgra = (unsigned int)pow(2, texset_data.Xsize) * (unsigned int)pow(2, texset_data.Ysize) * 4;
		char* buffer = new char[RAWsize_bgra];								// Buffer di lettura
		lvcfile.read(buffer, RAWsize_bgra);										// Legge i dati raw della texture e li mette nel buffer
	





		debug << "Texture " << m << "     name: " << texset_data.Texture_name << "_" << std::hex << texset_data.Texture_hash << "     MaterialType: " << std::dec << texset_data.MaterialType << endl;
		stringstream filename1, pathfilename1, texture1_name, texture2_name, material_name;
		filename1 << texset_data.Texture_name << "_" << std::hex << texset_data.Texture_hash << ".tga";
		pathfilename1 << sspath.str() << "\\" << filename1.str();
		SetCurrentDirectory(AE_IO.folder_temp_lpwstr);								// Imposta la cartella \NOMELIVELLO\Geometry\LVCNAME_Textures come attiva
		Texture_RAWtoTGA(filename1.str(), (unsigned int)pow(2, texset_data.Xsize), (unsigned int)pow(2, texset_data.Ysize), RAWsize_bgra, TGAType::BGRA, false, buffer);
		delete[] buffer;
		
		Texture tex1, tex2;
		Material mat;
		material_name << "LVC_" <<  lvcname << "_Material_" << std::hex << texset_offset.hash;

		if (texset_data.MaterialType == 65)																				// DIFFUSE
		{
			texture1_name << "LVC_" << lvcname << "_Texture_" << texset_index1.Texture_ID;
			tex1.name = texture1_name.str();
			tex1.filename = pathfilename1.str();
			MA.Texture.push_back(tex1);
			mat.name = material_name.str();
			mat.color = texture1_name.str();
			MA.Material.push_back(mat);
		}

		if (texset_data.MaterialType == 81 || texset_data.MaterialType == 16449 || texset_data.MaterialType == 17473)	// DIFFUSE + TRANSPARENCY
		{
			texture1_name << "LVC_" << lvcname << "_Texture_" << texset_index1.Texture_ID;
			tex1.name = texture1_name.str();
			tex1.filename = pathfilename1.str();
			MA.Texture.push_back(tex1);
			mat.name = material_name.str();
			mat.color = texture1_name.str();
			mat.transparency = texture1_name.str();
			MA.Material.push_back(mat);
		}
		
		if (texset_data.MaterialType == 1105)																			// WATER/LAVA
		{
			texture1_name << "LVC_" << lvcname << "_Texture_" << texset_index1.Texture_ID << "_c";
			texture2_name << "LVC_" << lvcname << "_Texture_" << texset_index1.Texture_ID << "_t";
			tex1.name = texture1_name.str();
			tex1.filename = pathfilename1.str();
			tex1.Exposure = 2;
			tex2.name = texture2_name.str();
			tex2.filename = pathfilename1.str();
			tex2.AlphaIsLuminance = true;
			MA.Texture.push_back(tex1);
			MA.Texture.push_back(tex2);
			mat.name = material_name.str();
			mat.color = texture1_name.str();
			mat.transparency = texture2_name.str();
			MA.Material.push_back(mat);
		}


		



		/* originale
		unsigned int RAWsize_bgr = (unsigned int)pow(2, texset_data.Xsize) * (unsigned int)pow(2, texset_data.Ysize) * 3;
		unsigned int RAWsize_a = (unsigned int)pow(2, texset_data.Xsize) * (unsigned int)pow(2, texset_data.Ysize);
		char* buffer_rgb = new char[RAWsize_bgr];
		char* buffer_a = new char[RAWsize_a];
		
		debug << "Texture " << m << "     name: " << texset_data.Texture_name << "     Unknown1: " << texset_data.MaterialType << endl;

		stringstream texture_rgb_name, pathfilename_rgb, texture_a_name, pathfilename_a, filename1, filename2, filename3, material_name;

		texture_rgb_name << "LVC_" << lvcname << "_Texture_" << texset_index1.Texture_ID << "_rgb";
		texture_a_name << "LVC_" << lvcname << "_Texture_" << texset_index1.Texture_ID << "_a";
		filename1 << texset_data.Texture_name << "_" << std::hex << texset_data.Texture_hash << ".bmp";
		filename2 << texset_data.Texture_name << "_" << std::hex << texset_data.Texture_hash << "_rgb.bmp";
		filename3 << texset_data.Texture_name << "_" << std::hex << texset_data.Texture_hash << "_a.bmp";
		pathfilename_rgb << sspath.str() << "\\" << filename2.str();
		pathfilename_a << sspath.str() << "\\" << filename3.str();
		material_name << "LVC_" <<  lvcname << "_Material_" << std::hex << texset_offset.hash;
		SetCurrentDirectory(folder_temp2_lpwstr);									// Imposta la cartella \NOMELIVELLO\Geometry\LVCNAME_Textures\Originals come attiva
		Texture_RAWtoBMP(filename1.str(), (unsigned int)pow(2, texset_data.Xsize), (unsigned int)pow(2, texset_data.Ysize), RAWsize_bgra, BMPType::BGRA, false, buffer);
		Texture_RGBAtoRGB_A((unsigned int)pow(2, texset_data.Xsize), (unsigned int)pow(2, texset_data.Ysize), buffer, buffer_rgb, buffer_a);
		SetCurrentDirectory(AE_IO.folder_temp_lpwstr);								// Imposta la cartella \NOMELIVELLO\Geometry\LVCNAME_Textures come attiva
		Texture_RAWtoBMP(filename2.str(), (unsigned int)pow(2, texset_data.Xsize), (unsigned int)pow(2, texset_data.Ysize), RAWsize_bgr, BMPType::BGR, false, buffer_rgb);
		Texture_RAWtoBMP(filename3.str(), (unsigned int)pow(2, texset_data.Xsize), (unsigned int)pow(2, texset_data.Ysize), RAWsize_a, BMPType::GRAYSCALE, false, buffer_a);
		delete[] buffer_rgb;
		delete[] buffer_a;
		delete[] buffer;

		Texture tex_rgb, tex_alpha;
		tex_rgb.name = texture_rgb_name.str();
		tex_rgb.filename = pathfilename_rgb.str();
		MA.Texture.push_back(tex_rgb);
		tex_alpha.name = texture_a_name.str();
		tex_alpha.filename = pathfilename_a.str();
		MA.Texture.push_back(tex_alpha);

		Material mat;
		mat.name = material_name.str();
		mat.color = texture_rgb_name.str();
		mat.transparency = texture_a_name.str();
		MA.Material.push_back(mat);
		*/
	}

	debug.close();

	// LETTURA PARTE SCONOSCIUTA
	lvcfile.seekg(header.PtrToUNKNOWN1);
	lvcfile.read(reinterpret_cast<char*>(&unknown1.PtrToVerts1), sizeof(unknown1.PtrToVerts1));
	lvcfile.read(reinterpret_cast<char*>(&unknown1.PtrToVerts2), sizeof(unknown1.PtrToVerts2));
	lvcfile.read(reinterpret_cast<char*>(&unknown1.PtrToIndices), sizeof(unknown1.PtrToIndices));
	lvcfile.read(reinterpret_cast<char*>(&unknown1.PtrToVerts3), sizeof(unknown1.PtrToVerts3));
	lvcfile.read(reinterpret_cast<char*>(&unknown1.PtrToOffsets), sizeof(unknown1.PtrToOffsets));
	lvcfile.read(reinterpret_cast<char*>(&unknown1.Unknown1), sizeof(unknown1.Unknown1));
	lvcfile.read(reinterpret_cast<char*>(&unknown1.Unknown2), sizeof(unknown1.Unknown2));
	lvcfile.read(reinterpret_cast<char*>(&unknown1.nVertices), sizeof(unknown1.nVertices));
	lvcfile.read(reinterpret_cast<char*>(&unknown1.nOffsets), sizeof(unknown1.nOffsets));
	lvcfile.seekg(header.PtrToUNKNOWN1 + unknown1.PtrToVerts1);

	/*Mesh testmesh1;
	testmesh1.name = "unknown1";
	testmesh1.nV = unknown1.nVertices;
	testmesh1.uv_set1_flag = testmesh1.uv_set2_flag = testmesh1.normals_flag = testmesh1.tangents_flag = testmesh1.binormals_flag = false;*/

	for (unsigned int v = 0; v < unknown1.nVertices; v++)
	{
		stringstream locatorname, spherename;
		locatorname << "unknown1_locator_" << v;
		spherename << "unknown1_sphere_" << v;
		Locator testmesh1;
		testmesh1.name = locatorname.str();
		testmesh1.translate_flag = true;
		lvcfile.read(reinterpret_cast<char*>(&unknown1_verts.X), sizeof(unknown1_verts.X));
		lvcfile.read(reinterpret_cast<char*>(&unknown1_verts.Y), sizeof(unknown1_verts.Y));
		lvcfile.read(reinterpret_cast<char*>(&unknown1_verts.Z), sizeof(unknown1_verts.Z));
		lvcfile.read(reinterpret_cast<char*>(&unknown1_verts.W), sizeof(unknown1_verts.W));
		testmesh1.tX = (unknown1_verts.X);
		testmesh1.tY = (unknown1_verts.Y);
		testmesh1.tZ = (unknown1_verts.Z);
		//float R = (float)(0xFF & (unknown1_verts.W >> 16)) / 255;
		//float G = (float)(0xFF & (unknown1_verts.W >> 8)) / 255;
		//float B = (float)(0xFF & unknown1_verts.W) / 255;
		/*testmesh1.R.push_back(R);
		testmesh1.G.push_back(G);
		testmesh1.B.push_back(B);
		testmesh1.A.push_back(1);*/
		//MA.Locator.push_back(testmesh1);



		NurbsSurface MA_Sphere;
		MA_Sphere.name = spherename.str();
		MA_Sphere.Type = "Sphere";
		MA_Sphere.translate_flag = true;
		MA_Sphere.tX = unknown1_verts.X;
		MA_Sphere.tY = unknown1_verts.Y;
		MA_Sphere.tZ = unknown1_verts.Z;
		MA_Sphere.Radius = unknown1_verts.W;
		//MA.NurbsSurface.push_back(MA_innerSphere);
		MA.NurbsSurface.push_back(MA_Sphere);
	}



	//FBX.Geometry.push_back(testmesh1);
	


	lvcfile.close();
	return true;
}