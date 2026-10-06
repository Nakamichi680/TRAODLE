#include "stdafx.h"
#include "Classes.h"
#include "MA/MA_Functions.h"
#include "FBX/FBX_Functions.h"
#include "TRAOD/CHR/CHR_Functions.h"


/*------------------------------------------------------------------------------------------------------------------
Esporta un file CHR (personaggio) nei formati FBX e MA nella cartella \NOMELIVELLO\Characters: scheletro (joints),
mesh in posa di bind con skinning, blend shapes del volto (dai file TMT del livello, vedi TMT_Struct.h), materiali (solo
MA) e textures. Le animazioni facciali (file TMS e ".3") sono scritte come takes del file FBX; per Maya sono contenute
nei file MA delle animazioni dei dialoghi e dei filmati (Export_CAL).
I file CHR degli oggetti animati (magic "NODE") hanno un formato diverso, non ancora supportato: vengono saltati.
------------------------------------------------------------------------------------------------------------------*/
bool Export_CHR (string filename)
{
	FBX_EXPORT FBX;
	MA_EXPORT MA;
	CHR_HEADER chr_header;
	CHR_Skeleton skeleton;

	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Reading " << filename;
	SetCurrentDirectory(AOD_IO.folder_level_lpwstr);			// \NOMELIVELLO
	ifstream chrfile(filename, std::ios::binary);
	if (!chrfile.is_open())
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " not found.";
		return false;
	}
	chrfile.read(reinterpret_cast<char*>(&chr_header), sizeof(chr_header));
	if (!chrfile)
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " is not a valid CHR file.";
		return false;
	}
	if (chr_header.CHR_MAGIC == 0x45444F4E)						// "NODE": oggetto animato
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << filename << " is an animated object (NODE format): not supported yet. Skipped.";
		return true;
	}
	if (chr_header.CHR_MAGIC != 0 || chr_header.HeaderSize != sizeof(CHR_HEADER) || chr_header.nBONES == 0 || chr_header.nBONES > 255)
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " is not a valid CHR file.";
		return false;
	}

	// Gruppo principale del personaggio
	string chrname = filename.substr(0, filename.find(".CHR"));
	map <uint32_t, CHR_Morph> morphs = CHR_Read_Morphs(chrname);		// Blend shapes del volto (file TMT del livello)
	Transform group;
	group.name = chrname;
	FBX.Group.push_back(group);
	MA.Transform.push_back(group);

	if (!CHR_Read_Skeleton(chrfile, chr_header, chrname, skeleton, FBX, MA))		// Lettura scheletro
		return false;
	if (!CHR_Read_Materials(chrfile, chr_header, chrname, MA))						// Lettura materiali
		return false;
	SetCurrentDirectory(AOD_IO.folder_characters_lpwstr);
	if (!CHR_Read_Textures(chrfile, chr_header, chrname, MA))						// Esportazione textures
		return false;
	if (!CHR_Read_Meshes(chrfile, chr_header, chrname, skeleton, morphs, FBX, MA))	// Lettura mesh (con i blend shapes del volto)
		return false;
	for (const pair <const uint32_t, CHR_Morph> &m : morphs)		// TMT del personaggio senza la mesh del volto nel CHR
		if (!m.second.used && m.second.name.compare(0, chrname.size(), chrname) == 0)
			msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << m.second.name << ".TMT: face mesh not found in " << filename << ". Blend shapes not exported.";

	// Animazioni facciali (file TMS e ".3" del livello) dei blend shapes applicati
	SetCurrentDirectory(AOD_IO.folder_level_lpwstr);			// \NOMELIVELLO (file estratti dal GMX)
	for (const pair <const uint32_t, CHR_Morph> &m : morphs)
		if (m.second.used)
		{
			vector <BlendShapeAnimation> anims = CHR_Read_FacialAnimations(m.second, chrname);
			FBX.BlendShapeAnimation.insert(FBX.BlendShapeAnimation.end(), anims.begin(), anims.end());
		}

	SetCurrentDirectory(AOD_IO.folder_characters_lpwstr);		// \NOMELIVELLO\Characters
	FBX_Export(chrname, FBX);									// Esportazione file FBX (animazioni facciali come takes)
	MA_Export(chrname, MA);										// Esportazione file MA

	return true;
}
