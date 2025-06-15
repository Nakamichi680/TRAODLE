#include "stdafx.h"
#include "MA/MA_Classes.h"

void MA_Write_DefaultNodes (MA_EXPORT &MA)
{
	stringstream out;
	unsigned int Max_frame = 0;
	for (unsigned int a = 0; a < MA.Animation.size(); a++)			// Ricerca dell'animazione più lunga (per impostare lo slider di Maya su tale valore)
		for (unsigned int b = 0; b < MA.Animation[a].size(); b++)
			if (MA.Animation[a][b].nFrames > Max_frame)
				Max_frame = MA.Animation[a][b].nFrames;
	if (Max_frame == 0)
		Max_frame = 120;

	// Calcolo posizione telecamera Persp	DA IMPLEMENTARE!!!!
	//for (MA.Mesh.size())
	
	// TELECAMERE
	out << "createNode transform -s -n \"persp\";\n";
	out << "	setAttr \".v\" no;\n";
	if (MA.UpAxis == 'y')
	{
		out << "	setAttr \".t\" -type \"double3\" 200 100 0;\n";
		out << "	setAttr \".r\" -type \"double3\" -28 45 0;\n";
	}
	if (MA.UpAxis == 'z')
	{
		out << "	setAttr \".t\" -type \"double3\" 10000 10000 10000;\n";
		out << "	setAttr \".r\" -type \"double3\" 62 0 -44;\n";
	}
	out << "createNode camera -s -n \"perspShape\" -p \"persp\";\n";
	out << "	setAttr -k off \".v\" no;\n";
	out << "	setAttr \".fl\" 18;\n";
	out << "	setAttr \".ncp\" " << MA.NearClipPlane << ";\n";
	out << "	setAttr \".fcp\" " << MA.FarClipPlane << ";\n";
	out << "	setAttr \".imn\" -type \"string\" \"persp\";\n";
	out << "	setAttr \".den\" -type \"string\" \"persp_depth\";\n";
	out << "	setAttr \".man\" -type \"string\" \"persp_mask\";\n";
	out << "	setAttr \".hc\" -type \"string\" \"viewSet -p %camera\";\n";
	out << "createNode transform -s -n \"top\";\n";
	out << "	setAttr \".v\" no;\n";
	if (MA.UpAxis == 'y')
	{
		out << "	setAttr \".t\" -type \"double3\" 100 1000 -100;\n";
		out << "	setAttr \".r\" -type \"double3\" -90 0 0;\n";
	}
	if (MA.UpAxis == 'z')
		out << "	setAttr \".t\" -type \"double3\" 0 0 1000;\n";
	out << "createNode camera -s -n \"topShape\" -p \"top\";\n";
	out << "	setAttr -k off \".v\" no;\n";
	out << "	setAttr \".rnd\" no;\n";
	out << "	setAttr \".ncp\" 1;\n";
	out << "	setAttr \".fcp\" 1000000;\n";
	out << "	setAttr \".imn\" -type \"string\" \"top\";\n";
	out << "	setAttr \".den\" -type \"string\" \"top_depth\";\n";
	out << "	setAttr \".man\" -type \"string\" \"top_mask\";\n";
	out << "	setAttr \".hc\" -type \"string\" \"viewSet -t %camera\";\n";
	out << "	setAttr \".o\" yes;\n";
	out << "createNode transform -s -n \"front\";\n";
	out << "	setAttr \".v\" no;\n";
	if (MA.UpAxis == 'y')
		out << "	setAttr \".t\" -type \"double3\" 100 0 1000;\n";
	if (MA.UpAxis == 'z')
	{
		out << "	setAttr \".t\" -type \"double3\" 0 -1000 0;\n";
		out << "	setAttr \".r\" -type \"double3\" 90 0 0;\n";
	}
	out << "createNode camera -s -n \"frontShape\" -p \"front\";\n";
	out << "	setAttr -k off \".v\" no;\n";
	out << "	setAttr \".rnd\" no;\n";
	out << "	setAttr \".ncp\" 1;\n";
	out << "	setAttr \".fcp\" 1000000;\n";
	out << "	setAttr \".imn\" -type \"string\" \"front\";\n";
	out << "	setAttr \".den\" -type \"string\" \"front_depth\";\n";
	out << "	setAttr \".man\" -type \"string\" \"front_mask\";\n";
	out << "	setAttr \".hc\" -type \"string\" \"viewSet -f %camera\";\n";
	out << "	setAttr \".o\" yes;\n";
	out << "createNode transform -s -n \"side\";\n";
	out << "	setAttr \".v\" no;\n";
	if (MA.UpAxis == 'y')
	{
		out << "	setAttr \".t\" -type \"double3\" 1000 0 -100;\n";
		out << "	setAttr \".r\" -type \"double3\" 0 90 0;\n";
	}
	if (MA.UpAxis == 'z')
	{
		out << "	setAttr \".t\" -type \"double3\" 1000 0 0;\n";
		out << "	setAttr \".r\" -type \"double3\" 90 0 90;\n";
	}
	out << "createNode camera -s -n \"sideShape\" -p \"side\";\n";
	out << "	setAttr -k off \".v\" no;\n";
	out << "	setAttr \".rnd\" no;\n";
	out << "	setAttr \".ncp\" 1;\n";
	out << "	setAttr \".fcp\" 1000000;\n";
	out << "	setAttr \".imn\" -type \"string\" \"side\";\n";
	out << "	setAttr \".den\" -type \"string\" \"side_depth\";\n";
	out << "	setAttr \".man\" -type \"string\" \"side_mask\";\n";
	out << "	setAttr \".hc\" -type \"string\" \"viewSet -s %camera\";\n";
	out << "	setAttr \".o\" yes;\n";

	// LIGHT LINKER
	out << "createNode lightLinker -s -n \"lightLinker1\";\n";
	out << "	setAttr -s " << MA.Material.size() + 2 << " \".lnk\";\n";
	out << "	setAttr -s " << MA.Material.size() + 2 << " \".slnk\";\n";

	// RENDER LAYER
	out << "createNode displayLayerManager -n \"layerManager\";\n";
	if (MA.Layer.size() > 0)
	{
		out << "	setAttr \".cdl\" 2;\n";
		if (MA.Layer.size() == 1)
			out << "	setAttr -s " << MA.Layer.size() + 1 << "\".dli[1]\" 1;\n";
		else
		{
			out << "	setAttr -s " << MA.Layer.size() + 1 << "\".dli[1:" << MA.Layer.size() << "]\" ";
			for (unsigned int i = 0; i < MA.Layer.size(); i++)
				out << " " << i + 1;
			out << ";\n";
		}
		out << "	setAttr -s " << MA.Layer.size() + 1 << "\".dli\";\n";
	}
	out << "createNode displayLayer -n \"defaultLayer\";\n";
	out << "createNode renderLayerManager -n \"renderLayerManager\";\n";
	out << "createNode renderLayer -n \"defaultRenderLayer\";\n";
	out << "	setAttr \".g\" yes;\n";

	// VALORI MIN/MAX SLIDER ANIMAZIONE
	out << "createNode script -n \"sceneConfigurationScriptNode\";\n";
	out << "	setAttr \".b\" -type \"string\" \"playbackOptions -min 1 -max " << Max_frame << " -ast 1 -aet " << Max_frame << " \";\n";
	out << "	setAttr \".st\" 6;\n";
	MA.MA_Nodes << out.str();
	out.str("");

	// Select
	out << "select -ne :time1;\n";
	out << "	setAttr \".o\" 1;\n";
	out << "	setAttr \".unw\" 1;\n";
	out << "select -ne :hardwareRenderingGlobals;\n";
	out << "	setAttr \".lm\" 4;\n";						// Lighting Mode = Full Ambient
	out << "	setAttr \".otfna\" -type \"stringArray\" 22 \"NURBS Curves\" \"NURBS Surfaces\" \"Polygons\" \"Subdiv Surface\" \"Particles\" \"Particle Instance\" \"Fluids\" \"Strokes\" \"Image Planes\" \"UI\" \"Lights\" \"Cameras\" \"Locators\" \"Joints\" \"IK Handles\" \"Deformers\" \"Motion Trails\" \"Components\" \"Hair Systems\" \"Follicles\" \"Misc. UI\" \"Ornaments\";\n";
	out << "	setAttr \".otfva\" -type \"Int32Array\" 22 0 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0;\n";
	out << "	setAttr \".ta\" 3;\n";						// Transparency algorithm = Depth Peeling
	out << "	setAttr \".tq\" 1;\n";						// Transparency quality = 1.000
	out << "	setAttr \".etmr\" no;\n";					// Enable maximum texture resolution = false
	out << "	setAttr \".tmr\" 4096;\n";
	out << "	setAttr \".fprt\" yes;\n";					// Floating point RT
	out << "select -ne :defaultRenderingList1;\n";
	out << "select -ne :initialShadingGroup;\n";
	out << "	setAttr \".ro\" yes;\n";
	out << "select -ne :initialParticleSE;\n";
	out << "	setAttr \".ro\" yes;\n";
	out << "select -ne :defaultLightSet;\n";
	MA.MA_Select << out.str();
	out.str("");

	// Relationship
	out << "relationship \"link\" \":lightLinker1\" \":initialShadingGroup.message\" \":defaultLightSet.message\";\n";
	out << "relationship \"link\" \":lightLinker1\" \":initialParticleSE.message\" \":defaultLightSet.message\";\n";
	out << "relationship \"shadowLink\" \":lightLinker1\" \":initialShadingGroup.message\" \":defaultLightSet.message\";\n";
	out << "relationship \"shadowLink\" \":lightLinker1\" \":initialParticleSE.message\" \":defaultLightSet.message\";\n";
	MA.MA_Relationships << out.str();
	out.str("");

	// ConnectAttr
	out << "connectAttr \"layerManager.dli[0]\" \"defaultLayer.id\";\n";
	out << "connectAttr \"renderLayerManager.rlmi[0]\" \"defaultRenderLayer.rlid\";\n";
	out << "connectAttr \"defaultRenderLayer.msg\" \":defaultRenderingList1.r\" -na;\n";
	MA.MA_Connections << out.str();
}