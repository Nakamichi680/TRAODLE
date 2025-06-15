#pragma once
#include "Classes.h"
enum class LayerDisplayType {
	Normal,			// L'oggetto è renderizzato e può essere selezionato
	Reference,		// L'oggetto è renderizzato e non può essere selezionato
	Template		// L'oggetto è in wireframe e non può essere selezionato
};


class Layer {
public:
	string name;											// Nome del layer
	unsigned int Label_ARGB = 0xFFFFFFFF;					// Colore etichetta (la trasparenza viene ignorata)
	bool Visible = true;									// Controlla la visibilità del layer
	LayerDisplayType Type = LayerDisplayType::Normal;		// Controlla se la selezione del layer è bloccata						
};


class NurbsSurface {
public:
	string name;					// Nome del nurbs
	string parent;					// Nome del gruppo/oggetto da cui dipende
	string layer;					// Nome layer di appartenenza, se vuoto non appartiene ad alcun layer (solo file Maya ASCII)
	string Type;					// Tipologia di nurbs
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
	unsigned int Sections = 8;			// Numero di settori (linee divisorie verticali)
	unsigned int Spans = 6;				// Numero di spans (linee divisorie orizzontali)
	float Radius = 1;					// Raggio della sfera
};


class PolyPlane {
public:
	string name;
	string parent;						// Nome del gruppo/oggetto da cui dipende
	string layer;						// Nome layer di appartenenza, se vuoto non appartiene ad alcun layer (solo file Maya ASCII)
	string material_name;				// Nome del materiale associato al PolyPlane
	bool translate_flag = false;
	bool rotate_flag = false;
	bool scale_flag = false;
	bool rotate_pivot_flag = false;
	bool scale_pivot_flag = false;
	bool Visible = true;
	float tX = 0;						// Traslazione X
	float tY = 0;						// Traslazione Y
	float tZ = 0;						// Traslazione Z
	float rX = 0;						// Rotazione X
	float rY = 0;						// Rotazione Y
	float rZ = 0;						// Rotazione Z
	float sX = 1;						// Scalatura X
	float sY = 1;						// Scalatura Y
	float sZ = 1;						// Scalatura Z
	float rpX = 0;						// Rotate pivot X
	float rpY = 0;						// Rotate pivot Y
	float rpZ = 0;						// Rotate pivot Z
	float spX = 0;						// Scale pivot X
	float spY = 0;						// Scale pivot Y
	float spZ = 0;						// Scale pivot Z
	float Width = 10;					// Larghezza
	float Height = 10;					// Altezza
	unsigned int Subdiv_Width = 10;		// Numero suddivisioni larghezza
	unsigned int Subdiv_Height = 10;	// Numero suddivisioni altezza
};


class BossWave {
public:
	string name;
	string parent;						// Nome del gruppo/oggetto da cui dipende
	string layer;						// Nome layer di appartenenza, se vuoto non appartiene ad alcun layer (solo file Maya ASCII)
	string inputmesh_name;				// Nome della mesh a cui viene applicata l'onda
	string material_name;				// Nome del materiale associato alla BossWave
	bool translate_flag = false;
	bool rotate_flag = false;
	bool scale_flag = false;
	bool rotate_pivot_flag = false;
	bool scale_pivot_flag = false;
	float tX = 0;						// Traslazione X
	float tY = 0;						// Traslazione Y
	float tZ = 0;						// Traslazione Z
	float rX = 0;						// Rotazione X
	float rY = 0;						// Rotazione Y
	float rZ = 0;						// Rotazione Z
	float sX = 1;						// Scalatura X
	float sY = 1;						// Scalatura Y
	float sZ = 1;						// Scalatura Z
	float rpX = 0;						// Rotate pivot X
	float rpY = 0;						// Rotate pivot Y
	float rpZ = 0;						// Rotate pivot Z
	float spX = 0;						// Scale pivot X
	float spY = 0;						// Scale pivot Y
	float spZ = 0;						// Scale pivot Z
	float patchSizeX = 200;
	float patchSizeZ = 200;
	float spaceScale = 1;
	float waveHeight = 1;
	float windSpeed = 4;
	float oceanDepth = 10000;
};


class MA_KeyTimeValue {
public:
	vector <unsigned int> Time;
	vector <float> Value;
};


class MA_animCurve {
public:
	string name;
	string parent;
	unsigned int nFrames;
	bool translateX_flag = false;
	bool translateY_flag = false;
	bool translateZ_flag = false;
	bool rotateX_flag = false;
	bool rotateY_flag = false;
	bool rotateZ_flag = false;
	bool scaleX_flag = false;
	bool scaleY_flag = false;
	bool scaleZ_flag = false;
	bool focalLength_flag = false;
	bool centerOfInterest_flag = false;
	MA_KeyTimeValue tX;
	MA_KeyTimeValue tY;
	MA_KeyTimeValue tZ;
	MA_KeyTimeValue rX;
	MA_KeyTimeValue rY;
	MA_KeyTimeValue rZ;
	MA_KeyTimeValue sX;
	MA_KeyTimeValue sY;
	MA_KeyTimeValue sZ;
	MA_KeyTimeValue fl;
	MA_KeyTimeValue coi;
};


class MA_EXPORT {
public:
	char UpAxis = 'z';
	float NearClipPlane = 2;
	float FarClipPlane = 1000000;
	vector <Texture> Texture;
	vector <Material> Material;
	vector <Layer> Layer;
	vector <Transform> Transform;
	vector <Mesh> Mesh;
	vector <Light> Light;
	vector <Camera> Camera;
	vector <Locator> Locator;
	vector <NurbsSurface> NurbsSurface;
	vector <Joint> Joint;
	vector < vector <MA_animCurve> > Animation;
	vector <PolyPlane> PolyPlane;
	vector <BossWave> BossWave;

	stringstream MA_Header;
	stringstream MA_Nodes;
	stringstream MA_Select;
	stringstream MA_Connections;
	stringstream MA_Relationships;
};