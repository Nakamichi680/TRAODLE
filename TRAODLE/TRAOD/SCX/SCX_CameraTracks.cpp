#include "stdafx.h"
#include "Classes.h"
#include "TRAOD/SCX/SCX_Functions.h"
#include "FBX/FBX_Functions.h"
#include "MA/MA_Functions.h"
#include "TRAOD/CAM/CAM_Functions.h"


/*------------------------------------------------------------------------------------------------------------------
Sequenze animate della telecamera contenute negli script (es. PARIS4 z4_lights__corrcam, PRAGUE3 biodome_meshes__flyby).
Lo script, avviato da un trigger, a ogni frame chiama Camera_SetScriptCamera(traccia, frame) e si ferma all'ultimo
frame (_break_mayatrack). La traccia ("maya track") e' un array 2D di float nel blocco dati dello script:
	cella base + k: offset (relativo alla cella stessa) della riga k, poi le righe da 8 float:
		posizione X Y Z, target X Y Z, FOV (gradi), roll (gradi)
	Le righe sono campionate a 60 fps: il gioco legge un record di 64 bytes (2 righe) per frame e usa solo la prima riga,
	interpolando linearmente tra il frame corrente e il successivo [SRC: lCamera_SetScriptCamera].
	Il primo argomento passato dallo script e' l'indirizzo della prima riga: base + mem[base].
Ogni traccia viene esportata come le telecamere dei filmati (CAM_BuildAnimatedCamera) in \NOMELIVELLO\Cameras\<script>.MA
/ .FBX (piu' tracce nello stesso script: <script>_<n>). Alla fine la cartella corrente torna quella degli script.
------------------------------------------------------------------------------------------------------------------*/
bool SCX_ExportCameraTracks (const AMX_SCRIPT &amx, string scriptname)
{
	int native = -1;
	for (size_t n = 0; n < amx.natives.size(); n++)
		if (amx.natives[n].name == "Camera_SetScriptCamera")
			native = (int)n;
	if (native < 0)
		return true;

	set <int32_t> bases;										// Indirizzi base degli array delle tracce
	for (size_t i = 0; i < amx.code.size(); i++)
	{
		const AMX_INSTRUCTION &ins = amx.code[i];
		if (string(AMX_Opcodes[ins.opcode].name) != "sysreq.c" || ins.param.empty() || ins.param[0] != native)
			continue;
		for (size_t k = i > 12 ? i - 12 : 0; k < i; k++)		// Costante caricata poco prima della chiamata
		{
			string op = AMX_Opcodes[amx.code[k].opcode].name;
			if ((op != "const.pri" && op != "const.alt" && op != "push.c") || amx.code[k].param.empty())
				continue;
			int32_t base = amx.code[k].param[0];
			if (!amx.IsDataAddress(base) || !amx.IsDataAddress(base + 4))
				continue;
			int32_t row0 = base + amx.GetDataCell(base);
			int32_t row1 = base + 4 + amx.GetDataCell(base + 4);
			if (row0 <= base || row1 - row0 != 32 || !amx.IsDataAddress(row0))		// Righe da 8 celle
				continue;
			bases.insert(base);
		}
	}
	if (bases.empty())
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << scriptname << " calls Camera_SetScriptCamera but no camera track was found.";
		return true;
	}

	bool ok = true;
	unsigned int index = 0;
	for (int32_t base : bases)
	{
		int32_t row0 = base + amx.GetDataCell(base);
		uint32_t nRows = (uint32_t)(row0 - base) / 4;
		while (nRows > 0 && !amx.IsDataAddress(row0 + 32 * (int32_t)nRows - 4))
			nRows--;
		unsigned int nFrames = (nRows + 1) / 2;					// Un frame ogni 2 righe (60 -> 30 fps)
		vector <float> Xfrom(nFrames), Yfrom(nFrames), Zfrom(nFrames), Xat(nFrames), Yat(nFrames), Zat(nFrames), Roll(nFrames), Fov(nFrames);
		for (unsigned int f = 0; f < nFrames; f++)
		{
			float v[8];
			for (int c = 0; c < 8; c++)
			{
				int32_t cell = amx.GetDataCell(row0 + 64 * (int32_t)f + 4 * c);
				memcpy(&v[c], &cell, 4);
			}
			Xfrom[f] = v[0];	Yfrom[f] = v[1];	Zfrom[f] = v[2];
			Xat[f] = v[3];		Yat[f] = v[4];		Zat[f] = v[5];
			Fov[f] = v[6];		Roll[f] = v[7];
		}
		string name = bases.size() > 1 ? scriptname + "_" + to_string(index) : scriptname;
		index++;
		FBX_EXPORT FBX;
		MA_EXPORT MA;
		if (!CAM_BuildAnimatedCamera(name, Xfrom, Yfrom, Zfrom, Xat, Yat, Zat, Roll, Fov, FBX, MA))
		{
			ok = false;
			continue;
		}
		CreateDirectory(AOD_IO.folder_cameras_lpwstr, NULL);
		SetCurrentDirectory(AOD_IO.folder_cameras_lpwstr);		// \NOMELIVELLO\Cameras
		FBX_Export(name, FBX);
		MA_Export(name, MA);
		SetCurrentDirectory(AOD_IO.folder_scx_lpwstr);
		msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Camera track: " << nFrames << " frames, output filename: " << AOD_IO.folder_cameras << name << ".MA";
	}
	return ok;
}
