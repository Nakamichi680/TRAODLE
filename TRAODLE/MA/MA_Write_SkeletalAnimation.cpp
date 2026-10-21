/*------------------------------------------------------------------------------------------------------------------
Scrittura di un'animazione scheletrica in un file Maya ASCII a parte (<anim.name>.ma, nella cartella corrente), che
carica il personaggio come riferimento (reference_path, namespace = anim.name) e ne anima i joints ed il gruppo
principale con curve lineari a 30 fps (animCurveTL per le traslazioni, animCurveTA per le rotazioni in gradi,
animCurveTU per le scalature). Le connessioni verso i nodi del riferimento sono scritte come "reference edits" del
nodo reference (placeHolderList): una connectAttr diretta non funzionerebbe perche' il riferimento viene caricato dopo la
lettura del file.
Le chiavi vengono ridotte con mathCurveKeys (scarto massimo 0.001 per traslazioni e rotazioni, 0.00001 per le scalature).
Se facial non e' nullptr vengono animati anche i pesi del blend shape del volto (filmati: animazione facciale dell'attore).
Se targets non e' nullptr i target dei blend shapes del personaggio vengono sostituiti con quelli delle mesh indicate
(filmati: blend shapes del TMT del filmato).
Gli eventi dell'animazione (anim.events, anim.event_text: suoni, animazioni sovrapposte, effetti, ...) vanno sul nodo
<namespace>_EVENTS del file (non nel riferimento): un attributo intero per tipo di evento con chiavi a gradini e
attributi di testo.
Namespace, nodo reference e nomi delle curve dipendono dal nome dell'animazione: piu' animazioni dello stesso personaggio
(es. STRAHOVA_1_CS_6_2 e STRAHOVA_2_CS_6_2, due attori con lo stesso CHR) caricate nella stessa scena restano personaggi
distinti invece di animare lo stesso riferimento.
INPUT: SkeletalAnimation anim, string reference_path, const BlendShapeAnimation *facial, const vector <Mesh> *targets
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "MA/MA_Classes.h"
#include "MATH/math.h"


void MA_Write_SkeletalAnimationFile (const SkeletalAnimation &anim, string reference_path, const BlendShapeAnimation *facial, const vector <Mesh> *targets)
{
	string ns = anim.name;											// Namespace unico per animazione
	for (char &c : ns)
		if (!isalnum((unsigned char)c) && c != '_')
			c = '_';
	string rn = ns + "RN";
	unsigned int last = anim.nFrames > 0 ? anim.nFrames - 1 : 0;

	// Curve: una per ogni canale animato
	struct Curve { string name, type, plug; const vector <float> *v; };
	vector <Curve> curves;
	for (const NodeAnimation &node : anim.nodes)
	{
		const struct { const char *attr, *type; const vector <float> *v; } channels[] = {
			{"translateX", "animCurveTL", &node.tX}, {"translateY", "animCurveTL", &node.tY}, {"translateZ", "animCurveTL", &node.tZ},
			{"rotateX", "animCurveTA", &node.rX}, {"rotateY", "animCurveTA", &node.rY}, {"rotateZ", "animCurveTA", &node.rZ},
			{"scaleX", "animCurveTU", &node.sX}, {"scaleY", "animCurveTU", &node.sY}, {"scaleZ", "animCurveTU", &node.sZ},
			{"visibility", "animCurveTU", &node.vis}};											// Oggetti agganciati (chiavi MAKE_VISIBLE / INVISIBLE)
		for (const auto &c : channels)
			if (!c.v->empty())
				curves.push_back({ns + "_" + node.node + "_" + c.attr, c.type, ns + ":" + node.node + "." + c.attr, c.v});
	}
	if (facial)													// Pesi dei blend shapes del volto (filmati)
	{
		for (unsigned int m = 0; m < facial->meshes.size(); m++)
			for (unsigned int t = 0; t < facial->targets.size() && t < facial->weight.size(); t++)
				curves.push_back({ns + "_" + facial->meshes[m] + "_" + facial->targets[t], "animCurveTU", ns + ":" + facial->meshes[m] + "_blendShape.weight[" + to_string(t) + "]", &facial->weight[t]});
		if (facial->nFrames > anim.nFrames)
			last = facial->nFrames - 1;
	}

	// Filmati: target del blend shape del volto sostituiti con quelli del TMT del filmato (reference edits "setAttr" dei
	// nodi blendShape del personaggio: inputTargetItem[6000].inputPointsTarget / inputComponentsTarget, vedi MA_Write_BlendShape)
	vector <string> edits;
	if (targets)
		for (const Mesh &mesh : *targets)
			for (unsigned int t = 0; t < mesh.BlendShape.size(); t++)
			{
				const BlendShapeTarget &target = mesh.BlendShape[t];
				vector <unsigned int> indexes;						// Vertici spostati dal target
				for (unsigned int v = 0; v < target.dX.size(); v++)
					if (target.dX[v] != 0 || target.dY[v] != 0 || target.dZ[v] != 0)
						indexes.push_back(v);
				string node = "\"" + ns + ":" + mesh.name + "_blendShape\"";
				string item = "it[0].itg[" + to_string(t) + "].iti[6000]";
				string points = "2 " + node + " \"" + item + ".ipt\" \" -type \\\"pointArray\\\" " + to_string(indexes.size());
				string components = "2 " + node + " \"" + item + ".ict\" \" -type \\\"componentList\\\" " + to_string(indexes.size());
				char buf[96];
				for (unsigned int i : indexes)
				{
					snprintf(buf, sizeof(buf), " %.9g %.9g %.9g 1", target.dX[i], target.dY[i], target.dZ[i]);
					points += buf;
					components += " \\\"vtx[" + to_string(i) + "]\\\"";
				}
				edits.push_back(points + "\"");
				edits.push_back(components + "\"");
			}

	ofstream out(anim.name + ".ma");
	if (!out.is_open())
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Unable to create " << anim.name << ".ma";
		return;
	}
	out << setprecision(7);
	out << "//Maya ASCII 2016 scene\n";
	out << "//Name: " << anim.name << ".ma\n";
	out << "//Skeletal animation exported by TRAODLE\n";
	out << "requires maya \"2016\";\n";
	out << "currentUnit -l centimeter -a degree -t ntsc;\n";
	out << "file -rdi 1 -ns \"" << ns << "\" -rfn \"" << rn << "\" -typ \"mayaAscii\" \"" << reference_path << "\";\n";
	out << "file -r -ns \"" << ns << "\" -dr 1 -rfn \"" << rn << "\" -typ \"mayaAscii\" \"" << reference_path << "\";\n";
	out << "createNode reference -n \"" << rn << "\";\n";
	out << "	setAttr -s " << curves.size() << " \".phl\";\n";
	for (unsigned int c = 0; c < curves.size(); c++)
		out << "	setAttr \".phl[" << c + 1 << "]\" 0;\n";
	out << "	setAttr \".ed\" -type \"dataReferenceEdits\" \n";
	out << "		\"" << rn << "\"\n";
	out << "		\"" << rn << "\" 0\n";
	out << "		\"" << rn << "\" " << edits.size() + curves.size();
	for (const string &edit : edits)
		out << "\n		" << edit;
	for (unsigned int c = 0; c < curves.size(); c++)
		out << "\n		5 4 \"" << rn << "\" \"" << curves[c].plug << "\" \"" << rn << ".placeHolderList[" << c + 1 << "]\" \"\"";
	out << ";\n";
	out << "lockNode -l 1 ;\n";

	for (const Curve &curve : curves)
	{
		const vector <float> &v = *curve.v;
		float tolerance = (curve.type == "animCurveTU") ? 1e-5f : 1e-3f;	// Molto sotto la quantizzazione dei dati CAL
		vector <unsigned int> frames = mathCurveKeys(v, tolerance);		// Chiavi necessarie (interpolazione lineare)
		string keys;													// snprintf: molto piu' veloce degli stream
		char buf[64];
		for (unsigned int k = 0; k < frames.size(); k++)
		{
			snprintf(buf, sizeof(buf), "%s%u %.7g", (k % 8 == 0) ? "\n\t\t" : " ", frames[k], v[frames[k]]);
			keys += buf;
		}
		out << "createNode " << curve.type << " -n \"" << curve.name << "\";\n";
		out << "	setAttr \".tan\" 2;\n";								// Tangenti lineari
		out << "	setAttr \".wgt\" no;\n";
		out << "	setAttr -s " << frames.size() << " \".ktv[0" << (frames.size() > 1 ? ":" + to_string(frames.size() - 1) : string()) << "]\"" << keys << ";\n";
	}
	if (!anim.events.empty() || !anim.event_text.empty())			// Eventi dell'animazione
	{
		string ev = ns + "_EVENTS";
		out << "createNode transform -n \"" << ev << "\";\n";
		for (const auto &e : anim.events)
			out << "	addAttr -ci true -k true -sn \"" << e.first << "\" -ln \"" << e.first << "\" -at \"long\";\n";
		for (const auto &t : anim.event_text)
		{
			string value;
			for (char c : t.second)
			{
				if (c == '"' || c == '\\')
					value.push_back('\\');
				if (c == '\n')
					value += "\\n";
				else
					value.push_back(c);
			}
			out << "	addAttr -ci true -sn \"" << t.first << "\" -ln \"" << t.first << "\" -dt \"string\";\n";
			out << "	setAttr \"." << t.first << "\" -type \"string\" \"" << value << "\";\n";
		}
		for (const auto &e : anim.events)
		{
			out << "createNode animCurveTU -n \"" << ev << "_" << e.first << "\";\n";
			out << "	setAttr \".tan\" 18;\n";
			out << "	setAttr \".wgt\" no;\n";
			out << "	setAttr -s " << e.second.size() << " \".ktv[0" << (e.second.size() > 1 ? ":" + to_string(e.second.size() - 1) : string()) << "]\"";
			for (unsigned int k = 0; k < e.second.size(); k++)
				out << ((k % 8 == 0) ? "\n\t\t" : " ") << e.second[k].first << " " << setprecision(10) << e.second[k].second << setprecision(7);
			out << ";\n";
			out << "	setAttr -s " << e.second.size() << " \".kot[0" << (e.second.size() > 1 ? ":" + to_string(e.second.size() - 1) : string()) << "]\"";
			for (unsigned int k = 0; k < e.second.size(); k++)
				out << " 5";											// Tangenti a gradini
			out << ";\n";
		}
	}
	out << "createNode script -n \"sceneConfigurationScriptNode\";\n";		// Intervallo della timeline
	out << "	setAttr \".b\" -type \"string\" \"playbackOptions -min 0 -max " << last << " -ast 0 -aet " << last << " \";\n";
	out << "	setAttr \".st\" 6;\n";
	for (unsigned int c = 0; c < curves.size(); c++)
		out << "connectAttr \"" << curves[c].name << ".o\" \"" << rn << ".phl[" << c + 1 << "]\";\n";
	for (const auto &e : anim.events)
		out << "connectAttr \"" << ns << "_EVENTS_" << e.first << ".o\" \"" << ns << "_EVENTS." << e.first << "\";\n";
	out << "// End of " << anim.name << ".ma\n";
	out.close();
	msg(msg::TGT::FILE, msg::TYP::LOG) << "Output filename: " << anim.name << ".ma";
}
