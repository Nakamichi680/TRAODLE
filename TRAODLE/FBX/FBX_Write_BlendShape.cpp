/*------------------------------------------------------------------------------------------------------------------
Scrittura dei blend shapes di una mesh: Deformer BlendShape collegato alla geometria, un SubDeformer BlendShapeChannel
per ogni target e una Geometry Shape con gli offset dei vertici che si spostano (Indexes + Vertices).
Connessioni: Shape -> BlendShapeChannel -> BlendShape -> Geometry.
INPUT: Mesh input
OUTPUT: FBX_EXPORT &FBX
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "FBX/FBX_Classes.h"


void FBX_Write_BlendShape (const Mesh &input, FBX_EXPORT &FBX)
{
	stringstream out, conn;
	out << setprecision(9);
	string blend_id = hashID(input.name, "BlendShape");

	FBX.FBX_Count.Deformer++;
	out << "	Deformer: " << blend_id << ", \"Deformer::" << input.name << "_blendShape\", \"BlendShape\" {\n";
	out << "		Version: 100\n";
	out << "	}\n";
	conn << "	;Deformer::" << input.name << "_blendShape, Geometry::\n";
	conn << "	C: \"OO\"," << blend_id << "," << hashID(input.name, "Geometry") << "\n\n";

	for (unsigned int t = 0; t < input.BlendShape.size(); t++)
	{
		const BlendShapeTarget &target = input.BlendShape[t];
		string channel_id = hashID(input.name + "|" + target.name, "BlendShapeChannel");
		string shape_id = hashID(input.name + "|" + target.name, "Shape");

		// Vertici spostati dal target (almeno uno, per avere una geometria valida)
		vector <unsigned int> indexes;
		for (unsigned int v = 0; v < target.dX.size(); v++)
			if (target.dX[v] != 0 || target.dY[v] != 0 || target.dZ[v] != 0)
				indexes.push_back(v);
		if (indexes.empty())
			indexes.push_back(0);

		FBX.FBX_Count.Deformer++;
		out << "	Deformer: " << channel_id << ", \"SubDeformer::" << target.name << "\", \"BlendShapeChannel\" {\n";
		out << "		Version: 100\n";
		out << "		Properties70:  {\n";							// Proprieta' animabile (necessaria per le animazioni dei pesi)
		out << "			P: \"DeformPercent\", \"Number\", \"\", \"A+\",0\n";
		out << "		}\n";
		out << "		DeformPercent: 0\n";
		out << "		FullWeights: *1 {\n";
		out << "			a: 100\n";
		out << "		}\n";
		out << "	}\n";

		FBX.FBX_Count.Geometry++;
		out << "	Geometry: " << shape_id << ", \"Geometry::" << input.name << "_" << target.name << "\", \"Shape\" {\n";
		out << "		Version: 100\n";
		out << "		Indexes: *" << indexes.size() << " {\n";
		out << "			a: ";
		for (unsigned int i = 0; i < indexes.size(); i++)
			out << (i ? "," : "") << ((i && i % 30 == 0) ? "\n" : "") << indexes[i];
		out << "\n		}\n";
		out << "		Vertices: *" << indexes.size() * 3 << " {\n";
		out << "			a: ";
		for (unsigned int i = 0; i < indexes.size(); i++)
			out << (i ? "," : "") << ((i && i % 10 == 0) ? "\n" : "") << target.dX[indexes[i]] << "," << target.dY[indexes[i]] << "," << target.dZ[indexes[i]];
		out << "\n		}\n";
		out << "	}\n";

		conn << "	;SubDeformer::" << target.name << ", Deformer::" << input.name << "_blendShape\n";
		conn << "	C: \"OO\"," << channel_id << "," << blend_id << "\n\n";
		conn << "	;Geometry::" << input.name << "_" << target.name << ", SubDeformer::" << target.name << "\n";
		conn << "	C: \"OO\"," << shape_id << "," << channel_id << "\n\n";
	}
	FBX.FBX_Properties << out.str();
	FBX.FBX_Connections << conn.str();
}
