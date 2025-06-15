#include "stdafx.h"
#include "MA/MA_Classes.h"


unsigned int Isolate_duplicated_water (MA_EXPORT &MA)
{
	unsigned int nDuplicatedWater = 0;

	for (unsigned int p = 0; p < MA.PolyPlane.size(); p++)
	{
		for (unsigned int i = 0; i < p; i++)
		{
			if (MA.PolyPlane[i].name.find("WATER") != string::npos &&
				MA.PolyPlane[i].tX == MA.PolyPlane[p].tX &&
				MA.PolyPlane[i].tY == MA.PolyPlane[p].tY &&
				MA.PolyPlane[i].tZ == MA.PolyPlane[p].tZ &&
				MA.PolyPlane[i].rX == MA.PolyPlane[p].rX &&
				MA.PolyPlane[i].rY == MA.PolyPlane[p].rY &&
				MA.PolyPlane[i].rZ == MA.PolyPlane[p].rZ &&
				MA.PolyPlane[i].sX == MA.PolyPlane[p].sX &&
				MA.PolyPlane[i].sY == MA.PolyPlane[p].sY &&
				MA.PolyPlane[i].sZ == MA.PolyPlane[p].sZ &&
				MA.PolyPlane[i].rpX == MA.PolyPlane[p].rpX &&
				MA.PolyPlane[i].rpY == MA.PolyPlane[p].rpY &&
				MA.PolyPlane[i].rpZ == MA.PolyPlane[p].rpZ &&
				MA.PolyPlane[i].spX == MA.PolyPlane[p].spX &&
				MA.PolyPlane[i].spY == MA.PolyPlane[p].spY &&
				MA.PolyPlane[i].spZ == MA.PolyPlane[p].spZ &&
				MA.PolyPlane[i].Width == MA.PolyPlane[p].Width &&
				MA.PolyPlane[i].Height == MA.PolyPlane[p].Height &&
				MA.PolyPlane[i].Subdiv_Width == MA.PolyPlane[p].Subdiv_Width &&
				MA.PolyPlane[i].Subdiv_Height == MA.PolyPlane[p].Subdiv_Height)
			{
				MA.PolyPlane[p].layer = AOD_IO.levelname;
				MA.PolyPlane[p].layer.append("_duplicated_water");
				nDuplicatedWater++;
				break;
			}
		}
	}

	for (unsigned int b = 0; b < MA.BossWave.size(); b++)
	{
		for (unsigned int i = 0; i < b; i++)
		{
			if (MA.BossWave[i].name.find("WATER") != string::npos &&
				MA.BossWave[i].tX == MA.BossWave[b].tX &&
				MA.BossWave[i].tY == MA.BossWave[b].tY &&
				MA.BossWave[i].tZ == MA.BossWave[b].tZ &&
				MA.BossWave[i].rX == MA.BossWave[b].rX &&
				MA.BossWave[i].rY == MA.BossWave[b].rY &&
				MA.BossWave[i].rZ == MA.BossWave[b].rZ &&
				MA.BossWave[i].sX == MA.BossWave[b].sX &&
				MA.BossWave[i].sY == MA.BossWave[b].sY &&
				MA.BossWave[i].sZ == MA.BossWave[b].sZ &&
				MA.BossWave[i].rpX == MA.BossWave[b].rpX &&
				MA.BossWave[i].rpY == MA.BossWave[b].rpY &&
				MA.BossWave[i].rpZ == MA.BossWave[b].rpZ &&
				MA.BossWave[i].spX == MA.BossWave[b].spX &&
				MA.BossWave[i].spY == MA.BossWave[b].spY &&
				MA.BossWave[i].spZ == MA.BossWave[b].spZ &&
				MA.BossWave[i].patchSizeX == MA.BossWave[b].patchSizeX &&
				MA.BossWave[i].patchSizeZ == MA.BossWave[b].patchSizeZ &&
				MA.BossWave[i].waveHeight == MA.BossWave[b].waveHeight &&
				MA.BossWave[i].windSpeed == MA.BossWave[b].windSpeed &&
				MA.BossWave[i].oceanDepth == MA.BossWave[b].oceanDepth)
			{
				MA.BossWave[b].layer = AOD_IO.levelname;
				MA.BossWave[b].layer.append("_duplicated_water");
				break;
			}
		}
	}

	if (nDuplicatedWater > 0)						// Aggiunta layer per luci ridondanti
	{
		Layer templayer;
		templayer.name = AOD_IO.levelname;
		templayer.name.append("_duplicated_water");
		templayer.Label_ARGB = 0x207B7BFF;
		templayer.Visible = false;
		MA.Layer.push_back(templayer);
	}

	return nDuplicatedWater;
}