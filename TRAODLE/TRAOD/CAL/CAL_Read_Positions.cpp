#include "stdafx.h"
#include "Classes.h"
#include "TRAOD/CAL/CAL_Functions.h"


/*------------------------------------------------------------------------------------------------------------------
Legge tutti i file POS del livello (vedi POS_Struct.h) dalla cartella corrente (\NOMELIVELLO) e restituisce le posizioni
del joint radice di ogni attore, indicizzate per hash dell'animazione (= CAL_ANIMATION.animID).
------------------------------------------------------------------------------------------------------------------*/
map <uint32_t, CAL_ActorPositions> CAL_Read_Positions ()
{
	map <uint32_t, CAL_ActorPositions> positions;
	for (unsigned int i = 0; i < AOD_IO.gmxfiles.size(); i++)
	{
		if (AOD_IO.gmxfiles[i].type != AoDFileType::POS)
			continue;
		const string &filename = AOD_IO.gmxfiles[i].name;
		ifstream posfile(filename, std::ios::binary);
		vector <uint8_t> data((istreambuf_iterator<char>(posfile)), istreambuf_iterator<char>());
		auto U32 = [&](size_t offset, uint32_t &value) -> bool
		{
			if (offset + 4 > data.size())
				return false;
			memcpy(&value, &data[offset], 4);
			return true;
		};
		POS_HEADER header;
		if (data.size() < sizeof(header))
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << filename << " is not a valid POS file.";
			continue;
		}
		memcpy(&header, data.data(), sizeof(header));
		if (header.Magic != POS_MAGIC || header.nActors > 1000)
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << filename << " is not a valid POS file.";
			continue;
		}
		size_t p = sizeof(POS_HEADER);
		bool ok = true;
		for (unsigned int a = 0; a < header.nActors && ok; a++)
		{
			CAL_ActorPositions actor;
			actor.posfile = filename;
			uint32_t len, nameHash, animHash, tmsHash, nFrames;
			ok = U32(p, len) && p + 4 + len <= data.size();
			if (!ok)
				break;
			actor.actor.assign((const char*)&data[p + 4], strnlen((const char*)&data[p + 4], len));
			p += 4 + len;
			ok = U32(p, nameHash) && U32(p + 4, animHash) && U32(p + 8, tmsHash) && U32(p + 12, len) && p + 16 + len <= data.size();
			if (!ok)
				break;
			actor.tms.assign((const char*)&data[p + 16], strnlen((const char*)&data[p + 16], len));
			p += 16 + len;
			ok = U32(p, nFrames) && nFrames < 1000000 && p + 4 + sizeof(POS_FRAME) * ((size_t)nFrames + 1) <= data.size();
			if (!ok)
				break;
			p += 4;
			actor.frames.resize(nFrames + 1);
			memcpy(actor.frames.data(), &data[p], sizeof(POS_FRAME) * actor.frames.size());
			p += sizeof(POS_FRAME) * actor.frames.size();
			positions[animHash] = actor;
		}
		if (!ok)
			msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << filename << ": invalid actor data.";
	}
	return positions;
}
