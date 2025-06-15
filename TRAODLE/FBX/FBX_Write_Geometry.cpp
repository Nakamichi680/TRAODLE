#include "stdafx.h"
#include "FBX/FBX_Classes.h"


void FBX_Write_Geometry (Mesh input, FBX_EXPORT &FBX)
{
	// Aggiunta modello geometrico al computo dei contatori FBX
	FBX.FBX_Count.Model++;
	FBX.FBX_Count.Geometry++;

	// Scrittura Properties
	stringstream out;
	out << "	Model: " << hashID(input.name, "Mesh") << ", \"Model::" << input.name << "\", \"Mesh\" {\n";
    out << "		Version: 232\n";
    out << "		Properties70:  {\n";
	out << "			P: \"RotationActive\", \"bool\", \"\", \"\",1\n";
    out << "			P: \"InheritType\", \"enum\", \"\", \"\",1\n";
    out << "			P: \"ScalingMin\", \"Vector3D\", \"Vector\", \"\",1,1,1\n";
    out << "			P: \"DefaultAttributeIndex\", \"int\", \"Integer\", \"\",0\n";
    out << "		}\n";
    out << "		Shading: T\n";
    out << "		Culling: \"CullingOff\"\n";
    out << "	}\n";
    out << "	Geometry: " << hashID(input.name, "Geometry") << ", \"Geometry::\", \"Mesh\" {\n";
    out << "		Properties70:  {\n";
    out << "			P: \"Color\", \"ColorRGB\", \"Color\", \"\",0.525490196078431,0.431372549019608,0.0313725490196078\n";
    out << "		}\n";

    // Scrive i vertici
    out << "		Vertices: *" << input.nV * 3 << " {\n";					// Numero di vertici x 3
    out << "			a: ";
    for (unsigned int v = 0; v < input.nV; v++)
    {
        out << input.X[v] << "," << input.Y[v] << "," << input.Z[v];
        if (input.nV - 1 == v)
			out << "\n";
        else
		{
			out << ",";
			if ((v + 1) % 30 == 0 && v != 0)
				out << "\n";
		}
    }
    out << "		}\n";

	// Calcolo del numero di indici
	unsigned int nIndices = 0;
	for (unsigned int I = 0; I < input.Face.size(); I++)
		if (input.Face[I].TrisOrQuads == 3)
			nIndices += 3;			// La faccia è un triangolo
		else
			nIndices += 4;			// La faccia è un quadrilatero

    // Scrive le facce
    out << "		PolygonVertexIndex: *" << nIndices << " {\n";			// Numero di indici di vertici
    out << "			a: ";
    for (unsigned int F = 0; F < input.Face.size(); F++)
    {
		if (input.Face[F].TrisOrQuads == 3)
			out << input.Face[F].v1 << "," << input.Face[F].v2 << "," << (input.Face[F].v3 ^ -1);
		else
			out << input.Face[F].v1 << "," << input.Face[F].v2 << "," << input.Face[F].v3 << "," << (input.Face[F].v4 ^ -1);

        if (input.Face.size() - 1 == F)
            out << "\n";
		else
		{
			out << ",";
			if ((F + 1) % 30 == 0 && F != 0)
				out << "\n";
		}
    }
    out << "		}\n";
	out << "		GeometryVersion: 124\n";

    // Scrive i normals
	if (input.normals_flag)
	{
		out << "		LayerElementNormal: 0 {\n";
		out << "			Version: 101\n";
		out << "			Name: \"\"\n";
		out << "			MappingInformationType: \"ByVertice\"\n";
		out << "			ReferenceInformationType: \"Direct\"\n";
		out << "			Normals: *" << input.nV * 3 << " {\n";			// Numero di normals x 3
		out << "				a: ";
		for (unsigned int n = 0; n < input.nV; n++)
		{
			out << input.Xn[n] << "," << input.Yn[n] << "," << input.Zn[n];
			if (input.nV - 1 == n)
				out << "\n";
			else
			{
				out << ",";
				if ((n + 1) % 30 == 0 && n != 0)
					out << "\n";
			}
		}
		out << "			}\n";
		out << "		}\n";
	}

	// Scrive i vertex colors
	if (input.vcolors_flag)
	{
		out << "		LayerElementColor: 0 {\n";
		out << "			Version: 101\n";
		out << "			Name: \"\"\n";
		out << "			MappingInformationType: \"ByVertice\"\n";
		out << "			ReferenceInformationType: \"Direct\"\n";
		out << "			Colors: *" << input.nV * 4 << " {\n";			// Numero di vertex colors x 4 (RGBA)
		out << "				a: ";
		for (unsigned int vc = 0; vc < input.nV; vc++)
		{
			out << input.R[vc] << "," << input.G[vc] << "," << input.B[vc] << "," << input.A[vc] ;
			if (input.nV - 1 == vc)
				out << "\n";
			else
			{
				out << ",";
				if ((vc + 1) % 30 == 0 && vc != 0)
					out << "\n";
			}
		}
		out << "			}\n";
		out << "		}\n";
	}

    // Scrive i tangents
	if (input.tangents_flag)
	{
		out << "		LayerElementTangent: 0 {\n";
		out << "			Version: 101\n";
		out << "			Name: \"UVChannel_1\"\n";
		out << "			MappingInformationType: \"ByVertice\"\n";
		out << "			ReferenceInformationType: \"Direct\"\n";
		out << "			Tangents: *" << input.nV * 3 << " {\n";			// Numero di tangents x 3
		out << "				a: ";
		for (unsigned int tg = 0; tg < input.nV; tg++)
		{
			out << input.Xtg[tg] << "," << input.Ytg[tg] << "," << input.Ztg[tg];
			if (input.nV - 1 == tg)
				out << "\n";
			else
			{
				out << ",";
				if ((tg + 1) % 30 == 0 && tg != 0)
					out << "\n";
			}
		}
		out << "			}\n";
		out << "		}\n";
	}

    // Scrive i binormals
	if (input.binormals_flag)
	{
		out << "		LayerElementBinormal: 0 {\n";
		out << "			Version: 101\n";
		out << "			Name: \"UVChannel_1\"\n";
		out << "			MappingInformationType: \"ByVertice\"\n";
		out << "			ReferenceInformationType: \"Direct\"\n";
		out << "			Binormals: *" << input.nV * 3 << " {\n";		// Numero di binormals x 3
		out << "				a: ";
		for (unsigned int bn = 0; bn < input.nV; bn++)
		{
			out << input.Xbn[bn] << "," << input.Ybn[bn] << "," << input.Zbn[bn];
			if (input.nV - 1 == bn)
				out << "\n";
			else
			{
				out << ",";
				if ((bn + 1) % 30 == 0 && bn != 0)
					out << "\n";
			}
		}
		out << "			}\n";
		out << "		}\n";
	}

    // Scrive gli UVs 1
	if (input.uv_set1_flag)
	{
		out << "		LayerElementUV: 0 {\n";
		out << "			Version: 101\n";
		out << "			Name: \"UVChannel_1\"\n";
		out << "			MappingInformationType: \"ByVertice\"\n";
		out << "			ReferenceInformationType: \"Direct\"\n";
		out << "			UV: *" << input.nV * 2 << " {\n";				// Numero di UV x 2
		out << "				a: ";
		for (unsigned int uv = 0; uv < input.nV; uv++)
		{
			out << input.U1[uv] << "," << input.V1[uv];
			if (input.nV - 1 == uv)
				out << "\n";
			else
			{
				out << ",";
				if ((uv + 1) % 40 == 0 && uv != 0)
					out << "\n";
			}
		}
		out << "			}\n";
		out << "		}\n";
	}

	// Scrive gli UVs 2
	if (input.uv_set2_flag)
	{
		out << "		LayerElementUV: 0 {\n";
		out << "			Version: 101\n";
		out << "			Name: \"UVChannel_2\"\n";
		out << "			MappingInformationType: \"ByVertice\"\n";
		out << "			ReferenceInformationType: \"Direct\"\n";
		out << "			UV: *" << input.nV * 2 << " {\n";				// Numero di UV x 2
		out << "				a: ";
		for (unsigned int uv = 0; uv < input.nV; uv++)
		{
			out << input.U2[uv] << "," << input.V2[uv];
			if (input.nV - 1 == uv)
				out << "\n";
			else
			{
				out << ",";
				if ((uv + 1) % 40 == 0 && uv != 0)
					out << "\n";
			}
		}
		out << "			}\n";
		out << "		}\n";
	}

	// Scrive il materiale di default
    out << "		LayerElementMaterial: 0 {\n";
    out << "			Version: 101\n";
    out << "			Name: \"\"\n";
    out << "			MappingInformationType: \"AllSame\"\n";
    out << "			ReferenceInformationType: \"IndexToDirect\"\n";
    out << "			Materials: *1 {\n";
    out << "				a: 0\n";
    out << "			}\n";
    out << "		}\n";

	// Scrive i layers per normals, vertex colors, binormals, tangents, UVs
	out << "		Layer: 0 {\n";
	out << "			Version: 100\n";
	if (input.normals_flag)
	{
		out << "			LayerElement:  {\n";
		out << "				Type: \"LayerElementNormal\"\n";
		out << "				TypedIndex: 0\n";
		out << "			}\n";
	}
	if (input.vcolors_flag)
	{
		out << "			LayerElement:  {\n";
		out << "				Type: \"LayerElementColor\"\n";
		out << "				TypedIndex: 0\n";
		out << "			}\n";
	}
	if (input.binormals_flag)
	{
		out << "			LayerElement:  {\n";
		out << "				Type: \"LayerElementBinormal\"\n";
		out << "				TypedIndex: 0\n";
		out << "			}\n";
	}
	if (input.tangents_flag)
	{
		out << "			LayerElement:  {\n";
		out << "				Type: \"LayerElementTangent\"\n";
		out << "				TypedIndex: 0\n";
		out << "			}\n";
	}
	out << "			LayerElement:  {\n";
	out << "				Type: \"LayerElementMaterial\"\n";
	out << "				TypedIndex: 0\n";
	out << "			}\n";
	if (input.uv_set1_flag)
	{
		out << "			LayerElement:  {\n";
		out << "				Type: \"LayerElementUV\"\n";
		out << "				TypedIndex: 0\n";
		out << "			}\n";
	}
	out << "		}\n";
	if (input.uv_set2_flag)
	{
		out << "		Layer: 1 {\n";
		out << "			Version: 100\n";
		out << "			LayerElement:  {\n";
		out << "				Type: \"LayerElementUV\"\n";
		out << "				TypedIndex: 1\n";
		out << "			}\n";
		out << "		}\n";
	}

	out << "	}\n";
	FBX.FBX_Properties << out.str();			// Copia il testo nello stringstream Properties definitivo

	out.str("");		// Pulisce lo stringstream

	// Scrittura Connections
	out << "	;Model::" << input.name << ", Model::";
	if (input.FBX_parent == "0")
		out << "RootNode\n";
	else
		out << input.parent << "\n";
	out << "	C: \"OO\"," << hashID(input.name, "Mesh") << "," << input.FBX_parent << "\n\n";
	out << "	;Geometry::, Model::" << input.name << "\n";
	out << "	C: \"OO\"," << hashID(input.name, "Geometry") << "," << hashID(input.name, "Mesh") << "\n\n";
	FBX.FBX_Connections << out.str();



/*
    /////////////////////// SCRITTURA DEFORMER
    out << "	Deformer: " << nodeID(13,el) << ", \"Deformer::\", \"Skin\" {\n";
    out << "		Version: 101\n";
    out << "		Link_DeformAcuracy: 50\n";
    out << "	}\n\n";

    /////////////////////// SCRITTURA SUBDEFORMER
    for (unsigned int b = 0; b < chr_header.nBONES; b++)	// Questo ciclo controlla se ogni bone è presente nell'elemento ed eventualmente scrive il Subdeformer
        for (unsigned int check_v = 0; check_v < MESH1_vINDX[el].size(); check_v++)
            if (mesh1.Bone1[MESH1_vINDX[el][check_v]] == b || mesh1.Bone2[MESH1_vINDX[el][check_v]] == b)				// Se la bone viene trovata
            {
                check_v = MESH1_vINDX[el].size() + 1;		// Serve ad impedire che venga controllata la stessa bone più volte interrompendo il ciclo for a monte
                int nIndexes = 0;
                for (unsigned int count_v = 0; count_v < MESH1_vINDX[el].size(); count_v++)			// Conta il numero di vertici associati alla bone trovata
                    if (mesh1.Bone1[MESH1_vINDX[el][count_v]] == b || mesh1.Bone2[MESH1_vINDX[el][count_v]] == b)
                        nIndexes++;
                out << "	Deformer: " << nodeID(14,b,el) << ", \"SubDeformer::\", \"Cluster\" {\n";
                out << "		Version: 100\n";
                out << "		UserData: \"\", \"\"\n";
                out << "		Indexes: *" << nIndexes << " {\n";
                out << "			a: ";
                int comma_newline = 0;
                for (unsigned int count_v = 0; count_v < MESH1_vINDX[el].size(); count_v++)		// SCRITTURA INDICI
                    if (mesh1.Bone1[MESH1_vINDX[el][count_v]] == b || mesh1.Bone2[MESH1_vINDX[el][count_v]] == b)
                    {
                        if (comma_newline == 0)
                            out << count_v;
                        else
                            if (comma_newline % 30 == 0)			// Se vero, va capo ogni 30 numeri inseriti
                                out << ",\n" << count_v;
                            else
                                out << "," << count_v;
                        comma_newline++;
                    }
                out << "\n";
                out << "		}\n";
                out << "		Weights: *" << nIndexes << " {\n";
                out << "			a: ";
                comma_newline = 0;
                for (unsigned int count_v = 0; count_v < MESH1_vINDX[el].size(); count_v++)		// SCRITTURA PESI
                {
                    if (mesh1.Bone1[MESH1_vINDX[el][count_v]] == b && mesh1.Bone2[MESH1_vINDX[el][count_v]] == b)	// Se il vertice è associato ad un'unica bone scrive W1
                    {
                        if (comma_newline == 0)
                            out << mesh1.W1[MESH1_vINDX[el][count_v]];
                        else
                            if (comma_newline % 30 == 0)			// Se vero, va capo ogni 30 numeri inseriti
                                out << ",\n" << mesh1.W1[MESH1_vINDX[el][count_v]];
                            else
                                out << "," << mesh1.W1[MESH1_vINDX[el][count_v]];
                        comma_newline++;
                    }
                    else
                        if (mesh1.Bone1[MESH1_vINDX[el][count_v]] == b)                                             // Se il vertice è associato alla bone numero 1 scrive W1
                        {
                            if (comma_newline == 0)
                                out << mesh1.W1[MESH1_vINDX[el][count_v]];
                            else
                                if (comma_newline % 30 == 0)		// Se vero, va capo ogni 30 numeri inseriti
                                    out << ",\n" << mesh1.W1[MESH1_vINDX[el][count_v]];
                                else
                                    out << "," << mesh1.W1[MESH1_vINDX[el][count_v]];
                            comma_newline++;
                        }
                        else
                            if (mesh1.Bone2[MESH1_vINDX[el][count_v]] == b)                                         // Se il vertice è associato alla bone numero 2 scrive W2
                            {
                                if (comma_newline == 0)
                                    out << mesh1.W2[MESH1_vINDX[el][count_v]];
                                else
                                    if (comma_newline % 30 == 0)	// Se vero, va capo ogni 30 numeri inseriti
                                        out << ",\n" << mesh1.W2[MESH1_vINDX[el][count_v]];
                                    else
                                        out << "," << mesh1.W2[MESH1_vINDX[el][count_v]];
                                comma_newline++;
                            }
                }
                out << "\n";
                out << "		}\n";
                out << "		Transform: *16 {\n";
                out << "			a: 1,0,0,0,0,1,0,0,0,0,1,0,";
                out << SKquat[b].m30 * -1 << "," << SKquat[b].m31 * -1 << "," << SKquat[b].m32 * -1 << "," << SKquat[b].m33 << "\n";	// Traslazione
                out << "		}\n";
                out << "		TransformLink: *16 {\n";
                out << "			a: 1,0,0,0,-0,1,0,0,0,-0,1,0,";
                out << SKquat[b].m30 << "," << SKquat[b].m31 << "," << SKquat[b].m32 << "," << SKquat[b].m33 << "\n";					// Traslazione
                out << "		}\n";
                out << " 	}\n\n";
            }
			*/
}