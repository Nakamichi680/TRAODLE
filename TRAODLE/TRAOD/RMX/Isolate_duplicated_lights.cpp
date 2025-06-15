#include "stdafx.h"
#include "MA/MA_Classes.h"


unsigned int Isolate_duplicated_lights (MA_EXPORT &MA)
{
	unsigned int nDuplicatedLights = 0;

	for (unsigned int l = 0; l < MA.Light.size(); l++)
	{
		for (unsigned int i = 0; i < l; i++)
		{
			if (MA.Light[i].tX == MA.Light[l].tX &&
				MA.Light[i].tY == MA.Light[l].tY &&
				MA.Light[i].tZ == MA.Light[l].tZ &&
				MA.Light[i].rX == MA.Light[l].rX &&
				MA.Light[i].rY == MA.Light[l].rY &&
				MA.Light[i].rZ == MA.Light[l].rZ &&
				MA.Light[i].sX == MA.Light[l].sX &&
				MA.Light[i].sY == MA.Light[l].sY &&
				MA.Light[i].sZ == MA.Light[l].sZ &&
				MA.Light[i].Intensity == MA.Light[l].Intensity &&
				MA.Light[i].R == MA.Light[l].R &&
				MA.Light[i].G == MA.Light[l].G &&
				MA.Light[i].B == MA.Light[l].B &&
				MA.Light[i].Decay_Near_Start == MA.Light[l].Decay_Near_Start &&
				MA.Light[i].Decay_Near_End == MA.Light[l].Decay_Near_End &&
				MA.Light[i].Decay_Far_Start == MA.Light[l].Decay_Far_Start &&
				MA.Light[i].Decay_Far_End == MA.Light[l].Decay_Far_End)
			{
				MA.Light[l].layer = AOD_IO.levelname;
				MA.Light[l].layer.append("_duplicated_lights");
				nDuplicatedLights++;
				break;
			}
		}
	}

	if (nDuplicatedLights > 0)						// Aggiunta layer per luci ridondanti
	{
		Layer templayer;
		templayer.name = AOD_IO.levelname;
		templayer.name.append("_duplicated_lights");
		templayer.Label_ARGB = 0xFF999900;
		templayer.Visible = false;
		MA.Layer.push_back(templayer);
	}

	return nDuplicatedLights;
}