#pragma once
#include <map>
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "TRAOD/CHR/CHR_Struct.h"
#include "TRAOD/CHR/TMT_Struct.h"


class CHR_Skeleton {						// Scheletro letto dal file CHR, usato per posizionare i vertici e creare lo skinning
public:
	vector <string> joint_name;				// Nome del joint esportato per ogni bone
	vector <int> parent;					// Indice della bone padre (-1 per la radice)
	vector <MATRIX> local;					// Trasformazione rispetto alla bone padre
	vector <MATRIX> global;					// Trasformazione globale in posa di bind
};


class CHR_Morph {							// Blend shapes del volto letti da un file TMT
public:
	string name;							// Nome del file TMT senza estensione
	uint32_t Id = 0;						// TMT_HEADER.Id (TargetId delle animazioni facciali)
	unsigned int nTargets = 0;
	unsigned int nVertices = 0;
	vector <TMT_VERTEX> base;				// Vertici base (nello spazio della bone della MESH2)
	vector <vector <TMT_VERTEX>> target;	// Offset dei vertici per ogni target: target[t][v]
	mutable bool used = false;				// Vero se applicato ad una mesh del CHR
	mutable vector <string> meshes;			// Mesh esportate con il blend shape (elementi della mesh del volto)
	mutable vector <string> targets;		// Nomi dei target
	map <uint32_t, vector <int>> remap;	// Altri TMT dello stesso volto (Id): target del morph corrispondente ad ognuno dei loro target
};


bool Export_CHR (string filename);

map <uint32_t, CHR_Morph> CHR_Read_Morphs (string chrname, string preferred = "");

vector <BlendShapeAnimation> CHR_Read_FacialAnimations (const CHR_Morph &morph, string chrname);

bool CHR_Read_Skeleton (ifstream &chrfile, const CHR_HEADER &chr_header, string chrname, CHR_Skeleton &skeleton, FBX_EXPORT &FBX, MA_EXPORT &MA);

bool CHR_Read_Meshes (ifstream &chrfile, const CHR_HEADER &chr_header, string chrname, const CHR_Skeleton &skeleton, const map <uint32_t, CHR_Morph> &morphs, FBX_EXPORT &FBX, MA_EXPORT &MA);

bool CHR_Read_Materials (ifstream &chrfile, const CHR_HEADER &chr_header, string chrname, MA_EXPORT &MA);

bool CHR_Read_Textures (ifstream &chrfile, const CHR_HEADER &chr_header, string chrname, MA_EXPORT &MA);
