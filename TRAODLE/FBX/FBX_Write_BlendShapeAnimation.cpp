/*------------------------------------------------------------------------------------------------------------------
Scrittura di un'animazione dei pesi dei blend shapes come take FBX: AnimationStack + AnimationLayer, e per ogni mesh e
target un AnimationCurveNode "DeformPercent" collegato al BlendShapeChannel (vedi FBX_Write_BlendShape) con la sua
AnimationCurve (chiavi lineari, peso * 100). Le chiavi uguali alla precedente e alla successiva vengono omesse.
Il tempo FBX e' in unita' di 1/46186158000 di secondo: a 30 fps (TimeMode 6) un frame vale 1539538600.
INPUT: BlendShapeAnimation anim
OUTPUT: FBX_EXPORT &FBX
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "FBX/FBX_Classes.h"


static const unsigned long long FBX_FRAME_30FPS = 1539538600ULL;


void FBX_Write_BlendShapeAnimation (const BlendShapeAnimation &anim, FBX_EXPORT &FBX)
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

	for (unsigned int m = 0; m < anim.meshes.size(); m++)
		for (unsigned int t = 0; t < anim.targets.size() && t < anim.weight.size(); t++)
		{
			const vector <float> &w = anim.weight[t];
			string key = anim.name + "|" + anim.meshes[m] + "|" + anim.targets[t];
			string node_id = hashID(key, "AnimCurveNode");
			string curve_id = hashID(key, "AnimCurve");
			string channel_id = hashID(anim.meshes[m] + "|" + anim.targets[t], "BlendShapeChannel");

			vector <unsigned int> frames;					// Chiavi necessarie (interpolazione lineare)
			for (unsigned int f = 0; f < w.size(); f++)
				if (f == 0 || f + 1 == w.size() || w[f] != w[f - 1] || w[f] != w[f + 1])
					frames.push_back(f);

			FBX.FBX_Count.AnimationCurveNode++;
			out << "	AnimationCurveNode: " << node_id << ", \"AnimCurveNode::DeformPercent\", \"\" {\n";
			out << "		Properties70:  {\n";
			out << "			P: \"d|DeformPercent\", \"Number\", \"\", \"A\"," << (w.empty() ? 0 : w[0] * 100) << "\n";
			out << "		}\n";
			out << "	}\n";
			FBX.FBX_Count.AnimationCurve++;
			out << "	AnimationCurve: " << curve_id << ", \"AnimCurve::\", \"\" {\n";
			out << "		Default: 0\n";
			out << "		KeyVer: 4009\n";
			out << "		KeyTime: *" << frames.size() << " {\n";
			out << "			a: ";
			for (unsigned int k = 0; k < frames.size(); k++)
				out << (k ? "," : "") << ((k && k % 20 == 0) ? "\n" : "") << frames[k] * FBX_FRAME_30FPS;
			out << "\n		}\n";
			out << "		KeyValueFloat: *" << frames.size() << " {\n";
			out << "			a: ";
			for (unsigned int k = 0; k < frames.size(); k++)
				out << (k ? "," : "") << ((k && k % 20 == 0) ? "\n" : "") << w[frames[k]] * 100;
			out << "\n		}\n";
			out << "		KeyAttrFlags: *1 {\n";							// Tutte le chiavi lineari
			out << "			a: 1028\n";
			out << "		}\n";
			out << "		KeyAttrDataFloat: *4 {\n";
			out << "			a: 0,0,218434821,0\n";
			out << "		}\n";
			out << "		KeyAttrRefCount: *1 {\n";
			out << "			a: " << frames.size() << "\n";
			out << "		}\n";
			out << "	}\n";

			conn << "	;AnimCurveNode::DeformPercent, AnimLayer::BaseLayer\n";
			conn << "	C: \"OO\"," << node_id << "," << layer_id << "\n\n";
			conn << "	;AnimCurveNode::DeformPercent, SubDeformer::" << anim.targets[t] << "\n";
			conn << "	C: \"OP\"," << node_id << "," << channel_id << ", \"DeformPercent\"\n\n";
			conn << "	;AnimCurve::, AnimCurveNode::DeformPercent\n";
			conn << "	C: \"OP\"," << curve_id << "," << node_id << ", \"d|DeformPercent\"\n\n";
		}
	FBX.FBX_Properties << out.str();
	FBX.FBX_Connections << conn.str();
}


/*------------------------------------------------------------------------------------------------------------------
Sezione Takes: elenco delle animazioni (necessaria per alcuni importatori)
------------------------------------------------------------------------------------------------------------------*/
string FBX_Write_Takes (const FBX_EXPORT &FBX)
{
	vector <pair <string, unsigned int>> takes;			// Nome e numero di frames di ogni take
	for (const BlendShapeAnimation &anim : FBX.BlendShapeAnimation)
		takes.push_back({anim.name, anim.nFrames});
	for (const SkeletalAnimation &anim : FBX.SkeletalAnimation)
		takes.push_back({anim.name, anim.nFrames});
	if (takes.empty())
		return "";
	stringstream out;
	out << "\n; Takes section\n";
	out << ";----------------------------------------------------\n\n";
	out << "Takes:  {\n";
	out << "	Current: \"" << takes[0].first << "\"\n";
	for (const pair <string, unsigned int> &take : takes)
	{
		unsigned long long stop = (unsigned long long)(take.second > 0 ? take.second - 1 : 0) * FBX_FRAME_30FPS;
		out << "	Take: \"" << take.first << "\" {\n";
		out << "		FileName: \"" << take.first << ".tak\"\n";
		out << "		LocalTime: 0," << stop << "\n";
		out << "		ReferenceTime: 0," << stop << "\n";
		out << "	}\n";
	}
	out << "}\n";
	return out.str();
}
