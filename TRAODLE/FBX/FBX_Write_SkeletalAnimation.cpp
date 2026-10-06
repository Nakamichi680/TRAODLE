/*------------------------------------------------------------------------------------------------------------------
Scrittura di un'animazione scheletrica come take FBX: AnimationStack + AnimationLayer, e per ogni nodo animato (joint
o gruppo) un AnimationCurveNode "T", "R" o "S" collegato a "Lcl Translation", "Lcl Rotation" o "Lcl Scaling" del
Model, con un'AnimationCurve per asse (chiavi lineari, 30 fps). I canali di un gruppo senza curva usano il valore 0
(rotazioni e traslazioni) o 1 (scalature). Le chiavi vengono ridotte con mathCurveKeys (scarto massimo 0.001
per traslazioni e rotazioni, 0.00001 per le scalature).
INPUT: SkeletalAnimation anim, set <string> animated (gruppi di canali animati in almeno una take del file)
OUTPUT: FBX_EXPORT &FBX
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "FBX/FBX_Classes.h"
#include "MATH/math.h"
#include <set>


static const unsigned long long FBX_FRAME_30FPS = 1539538600ULL;


void FBX_Write_SkeletalAnimation (const SkeletalAnimation &anim, const set <string> &animated, FBX_EXPORT &FBX)
{
	stringstream out, conn;
	out << setprecision(7);
	string stack_id = hashID(anim.name, "AnimStack");
	string layer_id = hashID(anim.name, "AnimLayer");
	unsigned long long stop = (unsigned long long)(anim.nFrames > 0 ? anim.nFrames - 1 : 0) * FBX_FRAME_30FPS;

	FBX.FBX_Count.AnimationStack++;
	out << "	AnimationStack: " << stack_id << ", \"AnimStack::" << anim.name << "\", \"\" {\n";
	out << "		Properties70:  {\n";
	out << "			P: \"LocalStop\", \"KTime\", \"Time\", \"\"," << stop << "\n";
	out << "			P: \"ReferenceStop\", \"KTime\", \"Time\", \"\"," << stop << "\n";
	out << "		}\n";
	out << "	}\n";
	FBX.FBX_Count.AnimationLayer++;
	out << "	AnimationLayer: " << layer_id << ", \"AnimLayer::BaseLayer\", \"\" {\n";
	out << "	}\n";
	conn << "	;AnimLayer::BaseLayer, AnimStack::" << anim.name << "\n";
	conn << "	C: \"OO\"," << layer_id << "," << stack_id << "\n\n";

	// Con piu' takes nello stesso file, Maya assegna ai canali senza curve nella take importata i valori di un'altra take
	// (la prima). Per questo ogni take riceve una curva ad una chiave (valore statico) per ogni gruppo di canali (T, R,
	// S) che e' animato in almeno un'altra take del file (animated: "nodo|T", "nodo|R", "nodo|S").
	vector <NodeAnimation> nodes = anim.nodes;
	if (FBX.SkeletalAnimation.size() > 1)
	{
		auto AddStatic = [&](const string &name, bool joint, float t[3], float r[3], float s[3])
		{
			NodeAnimation *node = nullptr;
			for (NodeAnimation &n : nodes)
				if (n.node == name && n.joint == joint)
					node = &n;
			bool needT = animated.count(name + "|T") && (!node || node->tX.empty());
			bool needR = animated.count(name + "|R") && (!node || node->rX.empty());
			bool needS = animated.count(name + "|S") && (!node || node->sX.empty());
			if (!needT && !needR && !needS)
				return;
			if (!node)
			{
				nodes.push_back(NodeAnimation());
				node = &nodes.back();
				node->node = name;
				node->joint = joint;
			}
			if (needT)
				node->tX = {t[0]}, node->tY = {t[1]}, node->tZ = {t[2]};
			if (needR)
				node->rX = {r[0]}, node->rY = {r[1]}, node->rZ = {r[2]};
			if (needS)
				node->sX = {s[0]}, node->sY = {s[1]}, node->sZ = {s[2]};
		};
		for (const Joint &j : FBX.Joint)
		{
			float t[3] = {j.tX, j.tY, j.tZ}, r[3] = {j.rX, j.rY, j.rZ}, s[3] = {j.sX, j.sY, j.sZ};
			AddStatic(j.name, true, t, r, s);
		}
		for (const Transform &g : FBX.Group)
		{
			float t[3] = {g.tX, g.tY, g.tZ}, r[3] = {g.rX, g.rY, g.rZ}, s[3] = {g.sX, g.sY, g.sZ};
			AddStatic(g.name, false, t, r, s);
		}
	}

	for (const NodeAnimation &node : nodes)
	{
		string model_id = node.joint ? hashID(node.node, "Joint") : hashID(node.node, "Group");
		const struct { const char *type, *property; float def, tolerance; const vector <float> *v[3]; } groups[] = {
			{"T", "Lcl Translation", 0, 1e-3f, {&node.tX, &node.tY, &node.tZ}},
			{"R", "Lcl Rotation", 0, 1e-3f, {&node.rX, &node.rY, &node.rZ}},
			{"S", "Lcl Scaling", 1, 1e-5f, {&node.sX, &node.sY, &node.sZ}}};
		for (const auto &g : groups)
		{
			if (g.v[0]->empty() && g.v[1]->empty() && g.v[2]->empty())
				continue;
			string key = anim.name + "|" + node.node + "|" + g.type;
			string node_id = hashID(key, "AnimCurveNode");
			const char *axis[3] = {"X", "Y", "Z"};

			FBX.FBX_Count.AnimationCurveNode++;
			out << "	AnimationCurveNode: " << node_id << ", \"AnimCurveNode::" << g.type << "\", \"\" {\n";
			out << "		Properties70:  {\n";
			for (unsigned int a = 0; a < 3; a++)
				out << "			P: \"d|" << axis[a] << "\", \"Number\", \"\", \"A\"," << (g.v[a]->empty() ? g.def : (*g.v[a])[0]) << "\n";
			out << "		}\n";
			out << "	}\n";
			conn << "	;AnimCurveNode::" << g.type << ", AnimLayer::BaseLayer\n";
			conn << "	C: \"OO\"," << node_id << "," << layer_id << "\n\n";
			conn << "	;AnimCurveNode::" << g.type << ", Model::" << node.node << "\n";
			conn << "	C: \"OP\"," << node_id << "," << model_id << ", \"" << g.property << "\"\n\n";

			for (unsigned int a = 0; a < 3; a++)
			{
				if (g.v[a]->empty())
					continue;
				const vector <float> &v = *g.v[a];
				string curve_id = hashID(key + "|" + axis[a], "AnimCurve");
				vector <unsigned int> frames = mathCurveKeys(v, g.tolerance);		// Chiavi necessarie (interpolazione lineare)
				string times, values;										// snprintf: molto piu' veloce degli stream
				char buf[64];
				for (unsigned int k = 0; k < frames.size(); k++)
				{
					snprintf(buf, sizeof(buf), "%s%s%llu", k ? "," : "", (k && k % 20 == 0) ? "\n" : "", frames[k] * FBX_FRAME_30FPS);
					times += buf;
					snprintf(buf, sizeof(buf), "%s%s%.7g", k ? "," : "", (k && k % 20 == 0) ? "\n" : "", v[frames[k]]);
					values += buf;
				}

				FBX.FBX_Count.AnimationCurve++;
				out << "	AnimationCurve: " << curve_id << ", \"AnimCurve::\", \"\" {\n";
				out << "		Default: 0\n";
				out << "		KeyVer: 4009\n";
				out << "		KeyTime: *" << frames.size() << " {\n";
				out << "			a: " << times << "\n";
				out << "		}\n";
				out << "		KeyValueFloat: *" << frames.size() << " {\n";
				out << "			a: " << values << "\n";
				out << "		}\n";
				out << "		KeyAttrFlags: *1 {\n";						// Tutte le chiavi lineari
				out << "			a: 1028\n";
				out << "		}\n";
				out << "		KeyAttrDataFloat: *4 {\n";
				out << "			a: 0,0,218434821,0\n";
				out << "		}\n";
				out << "		KeyAttrRefCount: *1 {\n";
				out << "			a: " << frames.size() << "\n";
				out << "		}\n";
				out << "	}\n";
				conn << "	;AnimCurve::, AnimCurveNode::" << g.type << "\n";
				conn << "	C: \"OP\"," << curve_id << "," << node_id << ", \"d|" << axis[a] << "\"\n\n";
			}
		}
	}
	FBX.FBX_Properties << out.str();
	FBX.FBX_Connections << conn.str();
}
