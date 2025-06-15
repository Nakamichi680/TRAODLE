#pragma once
#include "stdafx.h"


enum class AEFileType {LVC, TXS, AST, MDC, SEQ, CLU, AWAD, FNT, LINK, UNKNOWN};
enum class AoDFileType {CAL, CHR, CSS, TXT, CAM, CBH, CLN, EVX, POS, RMX, SCX, TMS, TMT, XXX, ZONE, UNKNOWN};
enum class BMPType {RGBA, BGRA, RGB, BGR, GRAYSCALE};
enum class TGAType {RGBA, BGRA, RGB, BGR, GRAYSCALE};
enum class DDSType {DXT1, DXT3, DXT5};
/*enum class AoDMaterialType {NONE,
							DIFFUSE,
							BUMP,
							DIFFUSEBUMP,
							LIGHT,
							DIFFUSELIGHT,
							BUMPLIGHT,
							DIFFUSEBUMPLIGHT,
							DIFFUSEENVELOPE,
							DIFFUSEBUMPENVELOPE,
							DIFFUSEENVELOPELIGHT,
							DIFFUSEBUMPENVELOPELIGHT,
							FUR,
							GREEN,
							GLOW,
							SNOW,
							GLASS,
							DIFFUSESPECULAR,
							DIFFUSEBUMPSPECULAR,
							DIFFUSESPECULARENVELOPE,
							DIFFUSEBUMPSPECULARENVELOPE,
							IRIDESCENCE,
							HAIR,
							METAL,
							SKIN};*/


class AOD_IO_CLASS {
private:
	class GMXlist {
	public:
		string name;						// Nome + estensione
		AoDFileType type;					// Tipologia di file
	};

public:
	TCHAR Console_OldTitle[MAX_PATH];
	LPCWSTR Console_NewTitle = L"Tomb Raider - The Angel of Darkness Level Exporter  (by Nakamichi680)";
	LPWSTR folder_exe_lpwstr = new TCHAR[MAX];			// Cartella in cui si trova il programma
	LPWSTR folder_clzgmx_lpwstr = new TCHAR[MAX];		// Cartella in cui si trovano i file CLZ/GMX
	string folder_clzgmx;								// Cartella in cui si trovano i file CLZ/GMX con backslash \ alla fine del nome (non usare per cambiare cartella)
	LPWSTR folder_level_lpwstr = new TCHAR[MAX];		// Cartella in cui verranno spacchettati i file del livello (es. \PARIS1)
	string folder_level;								// Cartella in cui verranno spacchettati i file del livello (es. \PARIS1\) con backslash \ alla fine del nome
	LPWSTR folder_cameras_lpwstr = new TCHAR[MAX];		// Cartella in cui verranno salvate le telecamere
	string folder_cameras;								// Cartella in cui verranno salvate le telecamere con backslash \ alla fine del nome
	LPWSTR folder_animations_lpwstr = new TCHAR[MAX];	// Cartella in cui verranno salvate le animazioni
	string folder_animations;							// Cartella in cui verranno salvate le animazioni con backslash \ alla fine del nome
	LPWSTR folder_zones_lpwstr = new TCHAR[MAX];		// Cartella in cui verranno salvate le zone
	string folder_zones;								// Cartella in cui verranno salvate le zone con backslash \ alla fine del nome
	LPWSTR folder_rooms_lpwstr = new TCHAR[MAX];		// Cartella in cui verranno salvate le stanze (file RMX)
	string folder_rooms;								// Cartella in cui verranno salvate le stanze (file RMX) con backslash \ alla fine del nome
	LPWSTR folder_blendshapes_lpwstr = new TCHAR[MAX];	// Cartella in cui verranno salvati i blendshapes
	string folder_blendshapes;							// Cartella in cui verranno salvati i blendshapes con backslash \ alla fine del nome
	LPWSTR folder_collisions_lpwstr = new TCHAR[MAX];	// Cartella in cui verranno salvate le collisioni
	string folder_collisions;							// Cartella in cui verranno salvate le collisioni con backslash \ alla fine del nome
	LPWSTR folder_characters_lpwstr = new TCHAR[MAX];	// Cartella in cui verranno salvati i personaggi
	string folder_characters;							// Cartella in cui verranno salvati i personaggi con backslash \ alla fine del nome
	string file_clzgmx;									// Nome del file del livello (può avere estensione GMX o CLZ, es. PARIS1.GMX o PARIS1.GMX.CLZ)
	string levelname;									// Nome del livello (es. PARIS1)
	LPWSTR folder_temp_lpwstr = new TCHAR[MAX];			// Cartella temporanea di lavoro per le varie subroutines
	string folder_temp;									// Cartella temporanea di lavoro per le varie subroutines con backslash \ alla fine del nome
	vector <GMXlist> gmxfiles;							// Vettore contenente la lista di file esportati dal GMX

	string AddFileToGMXList (string name, AoDFileType type)		// Aggiunge un file alla lista dei file contenuti nel GMX e ritorna il nome del file in ingresso
	{
		GMXlist temp {name, type};
		gmxfiles.push_back(temp);
		return name;
	}

	bool SearchFileInGMXList (string name, AoDFileType type)	// Verifica se il file o il tipo di file è presente nell'elenco dei files estratti dal GMX
	{
		GMXlist temp {name, type};
		vector<GMXlist>::iterator it = find_if(gmxfiles.begin(), gmxfiles.end(), [&temp] (const GMXlist& current) {return current.name == temp.name || current.type == temp.type;});
		return (it != gmxfiles.end());
	}

	AOD_IO_CLASS()												// Constructor
	{
		GetConsoleTitle(AOD_IO.Console_OldTitle, MAX_PATH);
	}

	/*LPWSTR folder_ani_lpwstr () {
		LPWSTR temp_lpwstr = new TCHAR[MAX];
		string temp = fbx_folder;
		temp.append(chr_name);
		temp.append("_ANIMATIONS");
		mbstowcs(temp_lpwstr, temp.c_str(), MAX);
		return temp_lpwstr;
	};
	string filename_FBX_CAL (char animation_name[64]) {
		string temp = animation_name;
		temp.append(".FBX");
		return temp;
	};
	string filename_FBX_CHR () {
		string temp = chr_name;
		temp.append(".FBX");
		return temp;
	};
	string filename_FBX_TMT () {
		string temp = chr_name;
		temp.append("_BLENDSHAPES.FBX");
		return temp;
	};
	string chr_name;
	string CHR;											// Stringa contenente il nome del file CHR con estensione
	string CAL;											// Stringa contenente il nome del file CAL con estensione
	string TMT;											// Stringa contenente il nome del file TMT con estensione
	*/
};



class AE_IO_CLASS {
private:
	class CLUSTERlist {
	public:
		string name;						// Nome + estensione oppure hash
		AEFileType type;					// Tipologia di file
	};

public:
	TCHAR Console_OldTitle[MAX_PATH];
	LPCWSTR Console_NewTitle = L"Tomb Raider - Anniversary Edition Level Exporter";
	LPWSTR folder_exe_lpwstr = new TCHAR[MAX];			// Cartella in cui si trova il programma
	LPWSTR folder_cluster_lpwstr = new TCHAR[MAX];		// Cartella in cui si trova il file cluster
	string folder_cluster;								// Cartella in cui si trova il file cluster con backslash \ alla fine del nome (non usare per cambiare cartella)
	LPWSTR folder_level_lpwstr = new TCHAR[MAX];		// Cartella in cui verranno spacchettati i file del livello (es. \LEVEL3A)
	string folder_level;								// Cartella in cui verranno spacchettati i file del livello (es. \LEVEL3A\) con backslash \ alla fine del nome
	LPWSTR folder_textures_lpwstr = new TCHAR[MAX];		// Cartella in cui verranno salvate le textures
	string folder_textures;								// Cartella in cui verranno salvate le textures con backslash \ alla fine del nome
	LPWSTR folder_geometry_lpwstr = new TCHAR[MAX];		// Cartella in cui verranno salvate le mesh del livello
	string folder_geometry;								// Cartella in cui verranno salvate le mesh del livello con backslash \ alla fine del nome
	LPWSTR folder_models_lpwstr = new TCHAR[MAX];		// Cartella in cui verranno salvate le mesh dei modelli degli oggetti
	string folder_models;								// Cartella in cui verranno salvate le mesh dei modelli degli oggetti con backslash \ alla fine del nome
	string file_cluster;								// Nome del file del livello (può avere estensione PS, PC o PSP)
	string levelname;									// Nome del livello (es. LEVEL3A)
	LPWSTR folder_temp_lpwstr = new TCHAR[MAX];			// Cartella temporanea di lavoro per le varie subroutines
	string folder_temp;									// Cartella temporanea di lavoro per le varie subroutines con backslash \ alla fine del nome
	vector <CLUSTERlist> clusterfiles;					// Vettore contenente la lista di file esportati dal file cluster

	string AddFileToClusterList (string name, AEFileType type)	// Aggiunge un file alla lista dei file contenuti nel file cluster e ritorna il nome del file in ingresso
	{
		CLUSTERlist temp {name, type};
		clusterfiles.push_back(temp);
		return name;
	}

	bool SearchFileInClusterList (string name, AEFileType type)	// Verifica se il file o il tipo di file è presente nell'elenco dei files estratti dal file cluster
	{
		CLUSTERlist temp {name, type};
		vector<CLUSTERlist>::iterator it = find_if(clusterfiles.begin(), clusterfiles.end(), [&temp] (const CLUSTERlist& current) {return current.name == temp.name || current.type == temp.type;});
		return (it != clusterfiles.end());
	}

	AE_IO_CLASS()												// Constructor
	{
		GetConsoleTitle(AE_IO.Console_OldTitle, MAX_PATH);
	}
};


class Face {
public:
	unsigned char TrisOrQuads = 3;		// Di default è impostato su "Tris"
	int v1;
	int v2;
	int v3;
	int v4;
};


class XYZ {
public:
	float x = 0, y = 0, z = 0;
};


struct VECTOR {
public:
	float x = 0, y = 0, z = 0, w = 1;
};


struct MATRIX {
	float m00 = 1, m01 = 0, m02 = 0, m03 = 0;
	float m10 = 0, m11 = 1, m12 = 0, m13 = 0;
	float m20 = 0, m21 = 0, m22 = 1, m23 = 0;
	float m30 = 0, m31 = 0, m32 = 0, m33 = 1;
};


class MA_Face {
public: 
	unsigned char TrisOrQuads = 3;		// Di default è impostato su "Tris"
	int e1;
	int e2;
	int e3;
	int e4;
};


class Edge {
public: 
	int v1;
	int v2;
};


class TextureHeader {
public:
	unsigned int Number;								// Numero della texture
    int DXT;											// 0 = ARGB, 1 = DXT1, 3 = DXT3, 5 = DXT5
    int ColourBumpShadow;								// 1 = shadow map, 2 = colour map, 4 = bump map, 5 = fur
};


class RoomInfo {
public:
	unsigned int RoomHash;
	float tX;
	float tY;
	float tZ;
};


class MESH1_CLASS {
public:
    string name;										// Nome originale (hash) o ricavato da Get_mesh_name (usato per Null)
    bool hashed;										// True se "name" è un hash
    unsigned int nElements;								// Il numero di elementi del gruppo MESH1
    vector <float> X;
    vector <float> Y;
    vector <float> Z;
    vector <float> Xn;
    vector <float> Yn;
    vector <float> Zn;
    vector <float> Xtg;
    vector <float> Ytg;
    vector <float> Ztg;
    vector <float> Xbn;
    vector <float> Ybn;
    vector <float> Zbn;
    vector <float> U;
    vector <float> V;
    vector <float> W1;
    vector <float> W2;
    vector <uint8_t> Bone1;
    vector <uint8_t> Bone2;
    vector <unsigned short> arrIndex;					// Contiene lo strip
    vector <unsigned short> arrEl_Ind;					// Contiene il numero di indici dell'elemento
    vector <unsigned short> arrOffset;					// Contiene l'offset dell'elemento nel triangle strip
    vector <unsigned short> arrMat_Ref;					// ID materiale
    vector <unsigned short> arrDraw_Mode;				// Draw mode (4 o 5)
    string name_lower () {								// Restituisce lo stesso nome di "name" ma in minuscolo (usato per Mesh)
        string out = name;
        transform(out.begin(), out.end(), out.begin(), ::tolower);
        return out;
    };
};


class MESH2_CLASS {
public:
    string name;										// Nome originale (hash) o ricavato da Get_mesh_name (usato per Null)
    bool hashed;										// True se "name" è un hash
    int Bone;											// La bone a cui è associato il gruppo MESH2
    unsigned int nV;									// Il numero di vertici del gruppo MESH2
    unsigned int nElements;								// Il numero di elementi del gruppo MESH2
    vector <float> X;
    vector <float> Y;
    vector <float> Z;
    vector <float> Xn;
    vector <float> Yn;
    vector <float> Zn;
    vector <float> Xtg;
    vector <float> Ytg;
    vector <float> Ztg;
    vector <float> Xbn;
    vector <float> Ybn;
    vector <float> Zbn;
    vector <float> U;
    vector <float> V;
    vector <unsigned short> arrIndex;					// Contiene lo strip
    vector <unsigned short> arrEl_Ind;					// Contiene il numero di indici dell'elemento
    vector <unsigned short> arrOffset;					// Contiene l'offset dell'elemento nel triangle strip
    vector <unsigned short> arrMat_Ref;					// ID materiale
    vector <unsigned short> arrDraw_Mode;				// Draw mode (4 o 5)
    string name_lower () {								// Restituisce lo stesso nome di "name" ma in minuscolo (usato per Mesh)
        string out = name;
        transform(out.begin(), out.end(), out.begin(), ::tolower);
        return out;
    };
};


class TMT_CLASS {
public:
	string name;										// Nome originale (hash) o ricavato da Get_mesh_name (usato per Null)
	bool hashed;										// True se "name" è un hash
	unsigned int nBlendshapes = 0;						// Numero di blendshapes presenti (a cui va aggiunto 1 per la mesh normale)
	unsigned int nV;									// Numero di vertici del blendshape
	unsigned int mesh2_group;							// Indice del gruppo MESH 2 a cui il blendshape è associato
	unsigned int mesh2_bone;							// Indice della bone dello scheletro associata al blendshape
	//Material TMT_Material;								// Il materiale del TMT è forzato a "Diffuse" (4)
	vector < vector <int> > BLENDSHAPE_vINDX;			// Contiene gli indici dei vertici diversi dalla mesh normale di ogni blendshape (per file FBX)		
	vector < vector <float> > X;
	vector < vector <float> > Y;
	vector < vector <float> > Z;
	vector < vector <float> > Xn;
	vector < vector <float> > Yn;
	vector < vector <float> > Zn;
	vector < vector <float> > U;
	vector < vector <float> > V;
	string name_lower () {								// Restituisce lo stesso nome di "name" ma in minuscolo (usato per Mesh)
		string out = name;
		transform(out.begin(), out.end(), out.begin(), ::tolower);
		return out;
	};
};





// WIP


class Material {
public:
	enum class TYPE {EMPTY, LAMBERT, PHONG, BLINN, AISTANDARDSURFACE};
	enum class BLEND {NORMAL, SUBTRACT, OVERLAY};
	enum class SPECIAL {NO, FUR, GLOW, SNOW, GLASS};
	string name;										// Nome del materiale
	TYPE Type = TYPE::EMPTY;							// Tipologia di shader
	BLEND Blend = BLEND::NORMAL;						// Tipologia di blending
	SPECIAL Special = SPECIAL::NO;						// Materiale speciale, valore di default = NO
	string color;										// Texture diffuse
	string shadow;										// Texture shadow map
	string transparency;								// Texture trasparenza
	string bump;										// Texture bump map
	string specular;									// Texture specular/envelope
	string emissive;									// Texture per effetto glow
	float metalness = 0.3f;
	float specular_color_R = 1;							// Colore riflesso 0-1
	float specular_color_G = 1;							// Colore riflesso 0-1
	float specular_color_B = 1;							// Colore riflesso 0-1
	float opacity_R = 1;								// Valore trasparenza 0-1
	float opacity_G = 1;								// Valore trasparenza 0-1
	float opacity_B = 1;								// Valore trasparenza 0-1
};








///////////// CLASSI BUONE
class Joint {
public:
	enum class LABELSIDE {CENTER, LEFT, RIGHT, NONE};
	enum class LABELTYPE {NONE, ROOT, HIP, KNEE, FOOT, TOE, SPINE, NECK, HEAD, COLLAR, SHOULDER, ELBOW, HAND, FINGER, THUMB, PROPA, PROPB, PROPC, OTHER, INDEX_FINGER, MIDDLE_FINGER, RING_FINGER, PINKY_FINGER, EXTRA_FINGER, BIG_TOE, INDEX_TOE, MIDDLE_TOE, RING_TOE, PINKY_TOE, FOOT_THUMB};
	string name;
	string parent;						// Nome del gruppo/oggetto da cui dipende
	string FBX_parent = "0";			// hashID del gruppo/oggetto da cui dipende
	string layer;						// Nome layer di appartenenza, se vuoto non appartiene ad alcun layer (solo file Maya ASCII)
	bool translate_flag = false;
	bool rotate_flag = false;
	bool scale_flag = false;
	float tX = 0;						// Traslazione X
	float tY = 0;						// Traslazione Y
	float tZ = 0;						// Traslazione Z
	float rX = 0;						// Rotazione X
	float rY = 0;						// Rotazione Y
	float rZ = 0;						// Rotazione Z
	float sX = 1;						// Scalatura X
	float sY = 1;						// Scalatura Y
	float sZ = 1;						// Scalatura Z
	float Radius = 20;					// Raggio sfera joint. Nel file FBX va moltiplicato per 33
	bool InheritsTransform = true;
	bool DrawLabel = false;
	LABELSIDE LabelSide = LABELSIDE::CENTER;
	LABELTYPE LabelType = LABELTYPE::OTHER;
	string LabelName = "";				// Il LabelName viene usato solo LabelType è su Other
};


class Texture {
private:
	class uvChooser {
	public:
		unsigned int UVset;						// 1, 2, 3, 4... (l'UVset 0 non va inserito, non è necessario l'uvChooser). MA Exporter al momento supporta solo UVset 0 e 1
		string mesh_name;
	};

public:
	string name;								// Nome dell'oggetto texture (ad esempio "PRAGUE3_ZONE2_TEXTURE5")
	string filename;							// Nome e percorso del file della texture (ad esempio "D:\AoD\C++\PARIS2_1_Z00_34.dds")
	vector <uvChooser> UV_mesh_list;			// Aggiungere solo le mesh con UVset diverso da 0
	float Exposure = 0;
	bool AlphaIsLuminance = false;
	
	int UVChannel = 1;	// 1 o 2. DA RIMUOVERE!!!!!!
};


class Locator {
public:
	string name;						// Nome dell'oggetto locator (ad esempio "PRAGUE3_ROOM_5_NODE_2")
	string parent;						// Nome del gruppo/oggetto da cui dipende
	string FBX_parent = "0";			// hashID del gruppo/oggetto da cui dipende
	string layer;						// Nome layer di appartenenza, se vuoto non appartiene ad alcun layer (solo file Maya ASCII)
	bool translate_flag = false;
	bool rotate_flag = false;
	bool scale_flag = false;
	float tX = 0;						// Traslazione X
	float tY = 0;						// Traslazione Y
	float tZ = 0;						// Traslazione Z
	float rX = 0;						// Rotazione X
	float rY = 0;						// Rotazione Y
	float rZ = 0;						// Rotazione Z
	float sX = 1;						// Scalatura X
	float sY = 1;						// Scalatura Y
	float sZ = 1;						// Scalatura Z
};


class Light {
public:
	string name;						// Nome della luce (ad esempio "PRAGUE3_ROOM_5_LIGHT_2")
	string parent;						// Nome del gruppo/oggetto da cui dipende
	string FBX_parent = "0";			// hashID del gruppo/oggetto da cui dipende
	string layer;						// Nome layer di appartenenza, se vuoto non appartiene ad alcun layer (solo file Maya ASCII)
	string type = "Point";				// Tipo di luce (Point/Ambient/Spot)
	bool translate_flag = false;
	bool rotate_flag = false;
	bool scale_flag = false;
	float tX = 0;						// Traslazione X
	float tY = 0;						// Traslazione Y
	float tZ = 0;						// Traslazione Z
	float rX = 0;						// Rotazione X
	float rY = 0;						// Rotazione Y
	float rZ = 0;						// Rotazione Z
	float sX = 1;						// Scalatura X
	float sY = 1;						// Scalatura Y
	float sZ = 1;						// Scalatura Z
	float Intensity = 100;				// Intensità luce
	float R = 1;						// Intensità colore rosso (0-1)
	float G = 1;						// Intensità colore verde (0-1)
	float B = 1;						// Intensità colore blu (0-1)
	float Decay_Near_Start = 0;			// Di regola è sempre 0 in AoD
	float Decay_Near_End = 0;			// Per luce più diffusa in Arnolds impostare come Attenuation_Far_End
	float Decay_Far_Start = 0;			// Dove la luce inizia a sbiadirsi
	float Decay_Far_End = 0;			// Distanza massima raggiunta dalla sfera di luce
};



class Camera {
public:
	string name;					// Nome della telecamera (ad esempio "IG_9_15")
	string parent;					// Nome del gruppo/oggetto da cui dipende
	string FBX_parent = "0";		// hashID del gruppo/oggetto da cui dipende
	string layer;					// Nome layer di appartenenza, se vuoto non appartiene ad alcun layer (solo file Maya ASCII)
	bool translate_flag = false;
	bool rotate_flag = false;
	bool scale_flag = false;
	float tX = 0;					// Traslazione X
	float tY = 0;					// Traslazione Y
	float tZ = 0;					// Traslazione Z
	float rX = 0;					// Rotazione X
	float rY = 0;					// Rotazione Y
	float rZ = 0;					// Rotazione Z
	float sX = 1;					// Scalatura X
	float sY = 1;					// Scalatura Y
	float sZ = 1;					// Scalatura Z
	float hfa = 2.367f;				// Horizontal Film Aperture
	float vfa = 1.33f;				// Vertical Film Aperture
	float fl = 35;					// Focal Lenght
	float coi = 45.453f;			// Center Of Interest
	float ncp = 10;					// Near clip plane
	float fcp = 1000000;			// Far clip plane
};


class Transform {
public:
	string name;					// Nome del gruppo
	string parent;					// Nome del gruppo/oggetto da cui dipende 
	string FBX_parent = "0";		// hashID del gruppo/oggetto da cui dipende
	string layer;					// Nome layer di appartenenza, se vuoto non appartiene ad alcun layer (solo file Maya ASCII)
	bool translate_flag = false;
	bool rotate_flag = false;
	bool scale_flag = false;
	float tX = 0;					// Traslazione X
	float tY = 0;					// Traslazione Y
	float tZ = 0;					// Traslazione Z
	float rX = 0;					// Rotazione X
	float rY = 0;					// Rotazione Y
	float rZ = 0;					// Rotazione Z
	float sX = 1;					// Scalatura X
	float sY = 1;					// Scalatura Y
	float sZ = 1;					// Scalatura Z
};


class Mesh {
public:
	string name;					// Nome del modello geometrico
	string parent;					// Nome del gruppo/oggetto da cui dipende
	string FBX_parent = "0";		// hashID del gruppo/oggetto da cui dipende
	string layer;					// Nome layer di appartenenza, se vuoto non appartiene ad alcun layer (solo file Maya ASCII)
	string material_name;			// Nome del materiale associato alla mesh
	unsigned int nV;
	bool uv_set1_flag = true;
	bool uv_set2_flag = true;
	bool normals_flag = true;
	bool tangents_flag = true;
	bool binormals_flag = true;
	bool vcolors_flag = true;
	bool doublesided = true;
	vector <float> X;
    vector <float> Y;
    vector <float> Z;
	vector <float> U1;
    vector <float> V1;
	vector <float> U2;
    vector <float> V2;
    vector <float> Xn;
    vector <float> Yn;
    vector <float> Zn;
    vector <float> Xtg;
    vector <float> Ytg;
    vector <float> Ztg;
    vector <float> Xbn;
    vector <float> Ybn;
    vector <float> Zbn;
    vector <float> R;
    vector <float> G;
    vector <float> B;
	vector <float> A;
	vector <Face> Face;		// utilizzato da FBX (tris e quads) e MA (solo tris)
};