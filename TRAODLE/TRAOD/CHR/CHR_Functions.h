#pragma once
#include <map>
#include "FBX/FBX_Classes.h"
#include "MA/MA_Classes.h"
#include "TRAOD/CHR/CHR_Struct.h"
#include "TRAOD/CHR/TMT_Struct.h"


class CHR_Attachment {						// Nodo agganciato ad una bone (arma, fondina, ...: vedi OGGETTI AGGANCIATI in CHR_Struct.h)
public:
	string name;							// Nome del nodo nel CHR (es. "SHOTGUN")
	string group;							// Nome del gruppo esportato (<CHR>_<nodo>)
	uint32_t hash = 0;						// GetHashValue(name): hash usato da ParentTracks e chiavi MAKE_VISIBLE / INVISIBLE
	int bone = -1;							// Bone di aggancio
	int parent = -1;						// Nodo padre (-1: la bone di aggancio)
	Transform transform;					// Gruppo esportato: posa di riposo, pivot e rotate axis
	MATRIX world;							// Matrice globale in posa di bind (vettore riga)
};

class CHR_Skeleton {						// Scheletro letto dal file CHR, usato per posizionare i vertici e creare lo skinning
public:
	vector <string> joint_name;				// Nome del joint esportato per ogni bone
	vector <int> parent;					// Indice della bone padre (-1 per la radice)
	vector <MATRIX> local;					// Trasformazione rispetto alla bone padre
	vector <MATRIX> global;					// Trasformazione globale in posa di bind
	vector <CHR_Attachment> attachments;	// Oggetti agganciati (CHR_Read_Attachments)
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


class CHR_Node {							// Nodo di un oggetto animato (CHR "NODE"): un transform di Maya con pivot
public:
	string name;							// Nome del nodo nel CHR (es. "d_leftshape")
	string group;							// Nome del gruppo esportato
	uint32_t mesh_hash = 0;					// GetHashValue(nome in maiuscolo) = ZONE_OBJECT_INFO.Hash delle sue mesh
	uint32_t flags = 0;						// CHR_NODEFLAG_PUSH / CHR_NODEFLAG_POP
	int parent = -1;						// Nodo padre (-1: gruppo radice dell'oggetto)
	Vec3 rotate_pivot, scale_pivot, rotate_pivot_translate, scale_pivot_translate;
	MATRIX orient;							// Rotate axis (matrice a vettore riga)
};


class CHR_NodeObject {						// Oggetto animato: nodi del CHR e mesh dello slot del file ZONE che li contiene
public:
	string name;							// Nome del CHR senza estensione (= gruppo radice)
	vector <CHR_Node> nodes;
	string zonefile;						// File ZONE con le mesh (vuoto se non trovate)
	int zone = -1;							// Indice della zona
	unsigned int slot = 0;					// Slot dell'oggetto nel file ZONE
	unsigned int matched = 0;				// Nodi con almeno una mesh nello slot
	bool linked = false;					// Slot collegato esplicitamente (Object Template dell'EVX o file XXX)
};


bool Export_CHR (string filename);

bool Export_CHR_Node (string chrname);

bool CHR_FindNodeObject (string chrname, CHR_NodeObject &object);

bool CHR_Build_NodeObject (const CHR_NodeObject &object, FBX_EXPORT &FBX, MA_EXPORT &MA, unsigned int *nodes_with_mesh = nullptr);

const map <pair <int, unsigned int>, string> &CHR_GetNodeObjectSlots ();

map <uint32_t, CHR_Morph> CHR_Read_Morphs (string chrname, string preferred = "");

vector <BlendShapeAnimation> CHR_Read_FacialAnimations (const CHR_Morph &morph, string chrname);

bool CHR_Read_Skeleton (ifstream &chrfile, const CHR_HEADER &chr_header, string chrname, CHR_Skeleton &skeleton, FBX_EXPORT &FBX, MA_EXPORT &MA);

bool CHR_Read_Attachments (ifstream &chrfile, const CHR_HEADER &chr_header, string chrname, CHR_Skeleton &skeleton, FBX_EXPORT &FBX, MA_EXPORT &MA);

bool CHR_Read_Meshes (ifstream &chrfile, const CHR_HEADER &chr_header, string chrname, const CHR_Skeleton &skeleton, const map <uint32_t, CHR_Morph> &morphs, FBX_EXPORT &FBX, MA_EXPORT &MA, map <uint32_t, vector <string>> *mesh_names = nullptr);	// mesh_names: ID della mesh -> mesh esportate

bool CHR_Read_Materials (ifstream &chrfile, const CHR_HEADER &chr_header, string chrname, MA_EXPORT &MA);

bool CHR_Read_Textures (ifstream &chrfile, const CHR_HEADER &chr_header, string chrname, MA_EXPORT &MA);
