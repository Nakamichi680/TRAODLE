#pragma once
#include "Classes.h"


string nodeID (string name, unsigned int node_type, unsigned int value1 = 0, unsigned int value2 = 0, unsigned int value3 = 0);
string hashID (string name, string type);


class AnimationCurveData {
public:
    vector <unsigned long long> KeyTime;				// Contiene i tempi
    vector <float> KeyValueFloat;						// Contiene i valori per ogni KeyTime
    vector <int> KeyAttrFlags;							// Contiene la tipologia di ogni keyframe
    vector <int> KeyAttrDataFloat;						// Contiene i valori delle derivate dei keyframes
};


class AnimationCurveNode {
public:
	string name;
	string parent;
	string FBX_parent;
    bool tX_flag = false;								// True se presente la traslazione lungo la coordinata X
    bool tY_flag = false;								// True se presente la traslazione lungo la coordinata Y
    bool tZ_flag = false;								// True se presente la traslazione lungo la coordinata Z
    bool rX_flag = false;								// True se presente la rotazione lungo la coordinata X
    bool rY_flag = false;								// True se presente la rotazione lungo la coordinata Y
    bool rZ_flag = false;								// True se presente la rotazione lungo la coordinata Z
    bool sX_flag = false;								// True se presente la scalatura lungo la coordinata X
    bool sY_flag = false;								// True se presente la scalatura lungo la coordinata Y
    bool sZ_flag = false;								// True se presente la scalatura lungo la coordinata Z
    // Classi contenenti i valori definitivi per il file FBX
    AnimationCurveData tX;
    AnimationCurveData tY;
    AnimationCurveData tZ;
    AnimationCurveData rX;
    AnimationCurveData rY;
    AnimationCurveData rZ;
    AnimationCurveData sX;
    AnimationCurveData sY;
    AnimationCurveData sZ;
	// Valori di traslazione di base dell'oggetto
	float Xtrasl = 0;
	float Ytrasl = 0;
	float Ztrasl = 0;

    string Properties_Text (string Animation_name) {		// Questa funzione prepara l'AnimationCurveNode
        stringstream temp, temp2;
		temp2 << Animation_name << "_" << name;
        if (tX_flag || tY_flag || tZ_flag)
        {
            temp << "	AnimationCurveNode: " << hashID(temp2.str(), "AnimCurveNodeT") << ", \"AnimCurveNode::T\", \"\" {\n";
            temp << "		Properties70:  {\n";
            temp << "			P: \"d|X\", \"Number\", \"\", \"A\"," << Xtrasl << "\n";
            temp << "			P: \"d|Y\", \"Number\", \"\", \"A\"," << Ytrasl << "\n";
            temp << "			P: \"d|Z\", \"Number\", \"\", \"A\"," << Ztrasl << "\n";
            temp << "		}\n";
            temp << "	}\n";
        }
        if (rX_flag || rY_flag || rZ_flag)
        {
            temp << "	AnimationCurveNode: " << hashID(temp2.str(), "AnimCurveNodeR") << ", \"AnimCurveNode::R\", \"\" {\n";
            temp << "		Properties70:  {\n";
            temp << "			P: \"d|X\", \"Number\", \"\", \"A\",0\n";
            temp << "			P: \"d|Y\", \"Number\", \"\", \"A\",0\n";
            temp << "			P: \"d|Z\", \"Number\", \"\", \"A\",0\n";
            temp << "		}\n";
            temp << "	}\n";
        }
        if (sX_flag || sY_flag || sZ_flag)
        {
            temp << "	AnimationCurveNode: " << hashID(temp2.str(), "AnimCurveNodeS") << ", \"AnimCurveNode::S\", \"\" {\n";
            temp << "		Properties70:  {\n";
            temp << "			P: \"d|X\", \"Number\", \"\", \"A\",1\n";
            temp << "			P: \"d|Y\", \"Number\", \"\", \"A\",1\n";
            temp << "			P: \"d|Z\", \"Number\", \"\", \"A\",1\n";
            temp << "		}\n";
            temp << "	}\n";
        }
        return temp.str();
    };

	string Connections_Text (string Animation_name) {		// Questa funzione scrive le Connections
		stringstream temp, temp2;
		temp2 << Animation_name << "_" << name;
		if (tX_flag || tY_flag || tZ_flag)
		{
			temp << "	;AnimCurveNode::T, AnimLayer::BaseLayer\n";
			temp << "	C: \"OO\"," << hashID(temp2.str(), "AnimCurveNodeT") << "," << hashID(Animation_name, "AnimLayer") << "\n\n";	// AnimationCurveNode t, AnimationLayer
			temp << "	;AnimCurveNode::T, Model::" << parent << "\n";
			temp << "	C: \"OP\"," << hashID(temp2.str(), "AnimCurveNodeT") << "," << FBX_parent << ", \"Lcl Translation\"\n\n";		// AnimationCurveNode t, Bone/oggetto
		}
		if (rX_flag || rY_flag || rZ_flag)
		{
			temp << "	;AnimCurveNode::R, AnimLayer::BaseLayer\n";
			temp << "	C: \"OO\"," << hashID(temp2.str(), "AnimCurveNodeR") << "," << hashID(Animation_name, "AnimLayer") << "\n\n";	// AnimationCurveNode r, AnimationLayer
			temp << "	;AnimCurveNode::R, Model::" << parent << "\n";
			temp << "	C: \"OP\"," << hashID(temp2.str(), "AnimCurveNodeR") << "," << FBX_parent << ", \"Lcl Rotation\"\n\n";			// AnimationCurveNode r, Bone/oggetto
		}
		if (sX_flag || sY_flag || sZ_flag)
		{
			temp << "	;AnimCurveNode::S, AnimLayer::BaseLayer\n";
			temp << "	C: \"OO\"," << hashID(temp2.str(), "AnimCurveNodeS") << "," << hashID(Animation_name, "AnimLayer") << "\n\n";	// AnimationCurveNode s, AnimationLayer
			temp << "	;AnimCurveNode::S, Model::" << parent << "\n";
			temp << "	C: \"OP\"," << hashID(temp2.str(), "AnimCurveNodeS") << "," << FBX_parent << ", \"Lcl Scaling\"\n\n";			// AnimationCurveNode s, Bone/oggetto
		}
		return temp.str();
	};
};


class Animation {
public:
	string name;										// Il nome dell'animazione
    unsigned int nJoints;								// Numero di joints
    unsigned int nFrames;								// Durata in frames dell'animazione
	vector <AnimationCurveNode> Curvenode;				// Ogni elemento del vettore è una joint
};


class FBX_COUNT_CLASS {
public:
    unsigned int AnimationStack = 0;
    unsigned int AnimationLayer = 0;
    unsigned int AnimationCurveNode = 0;
    unsigned int AnimationCurve = 0;
    unsigned int Model = 0;
    unsigned int Geometry = 0;
    unsigned int Material = 0;
    unsigned int Texture = 0;
    unsigned int NodeAttribute = 0;
    unsigned int Pose = 0;
    unsigned int BindPose = 0;
    unsigned int Deformer = 0;
};


class FBX_EXPORT {
public:
	FBX_COUNT_CLASS FBX_Count;

	char UpAxis = 'z';
	vector <Transform> Group;
	vector <Mesh> Geometry;
	vector <Locator> Locator;
	vector <Camera> Camera;
	vector <Light> Light;
	vector <Animation> Animation;
	vector <Joint> Joint;

	stringstream FBX_Header;
	stringstream FBX_Definitions;
	stringstream FBX_Properties;
	stringstream FBX_Connections;

	void Clear () {
		FBX_Header.str("");
		FBX_Definitions.str("");
		FBX_Properties.str("");
		FBX_Connections.str("");
		FBX_Count = FBX_COUNT_CLASS();
	};

	void Init () {
		FBX_Properties << "; Object properties\n";
		FBX_Properties << ";------------------------------------------------------------------\n\n";
		FBX_Properties << "Objects:  {\n";
		FBX_Connections << "; Object connections\n";
		FBX_Connections << ";------------------------------------------------------------------\n\n";
		FBX_Connections << "Connections:  {\n";
	};

	void Close () {
		FBX_Properties << "}\n\n";
		FBX_Connections << "}";
	};
};