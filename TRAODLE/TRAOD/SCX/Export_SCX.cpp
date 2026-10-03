#include "stdafx.h"
#include "Classes.h"
#include "TRAOD/SCX/SCX_Functions.h"


/*------------------------------------------------------------------------------------------------------------------
Esporta gli script contenuti in un file SCX. Per ogni script vengono salvati nella cartella \NOMELIVELLO\SCX:
 - il file AMX originale (.amx)
 - il disassemblato (.asm)
 - lo pseudo-codice Pawn decompilato (.p)
------------------------------------------------------------------------------------------------------------------*/
bool Export_SCX (string filename)
{
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Reading " << filename;
	SetCurrentDirectory(AOD_IO.folder_level_lpwstr);			// \NOMELIVELLO
	ifstream scxfile(filename, std::ios::binary | std::ios::ate);
	if (!scxfile.is_open())
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " not found.";
		return false;
	}
	streamoff filesize = scxfile.tellg();
	vector <char> data((size_t)filesize);
	scxfile.seekg(0);
	scxfile.read(data.data(), filesize);
	scxfile.close();

	SCX_HEADER scx_header;
	if (filesize < (streamoff)sizeof(SCX_HEADER))
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " is not a valid SCX file.";
		return false;
	}
	memcpy(&scx_header, data.data(), sizeof(SCX_HEADER));
	if (scx_header.MAGIC != 0x102)
		msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << filename << ": unexpected header value " << scx_header.MAGIC << ".";
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Number of scripts: " << scx_header.nScriptFiles;

	SetCurrentDirectory(AOD_IO.folder_scx_lpwstr);				// \NOMELIVELLO\SCX
	string basename = filename.substr(0, filename.find(".SCX"));
	bool result = true;
	streamoff pos = sizeof(SCX_HEADER);
	for (unsigned int s = 0; s < scx_header.nScriptFiles; s++)
	{
		// Lettura entry
		SCX_ENTRY scx_entry;
		pos = (pos + 3) & ~3;									// Le entry sono allineate a 4 bytes
		if (pos + (streamoff)sizeof(SCX_ENTRY) > filesize)
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Unexpected end of file reading script " << s << ".";
			return false;
		}
		memcpy(&scx_entry, data.data() + pos, sizeof(SCX_ENTRY));
		pos = (pos + sizeof(SCX_ENTRY) + 15) & ~15;				// Il blocco dati e' allineato a 16 bytes
		if (pos + scx_entry.Size > filesize)
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Script " << s << " exceeds the end of file.";
			return false;
		}

		stringstream ssname;
		ssname << basename << "_SCRIPT_" << setw(2) << setfill('0') << s << "_" << hex << uppercase << setw(8) << scx_entry.Hash;
		string scriptname = ssname.str();

		// Lettura AMX
		AMX_SCRIPT amx;
		if (!AMX_Load(data.data() + pos, scx_entry.Size, amx))
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << scriptname << " skipped.";
			result = false;
			pos += scx_entry.Size;
			continue;
		}
		msg(msg::TGT::FILE, msg::TYP::LOG) << scriptname << ": " << amx.publics.size() << " publics, " << amx.natives.size() << " natives, " << amx.code.size() << " instructions.";
		if (scx_entry.Size != (uint32_t)(amx.header.size + amx.header.stp))		// Il blocco contiene sempre l'AMX seguito da stp bytes di spazzatura
			msg(msg::TGT::FILE, msg::TYP::WARN) << scriptname << ": unexpected block size " << scx_entry.Size << " (AMX " << amx.header.size << " + stp " << amx.header.stp << ").";

		// Salvataggio AMX originale
		ofstream amxfile(scriptname + ".amx", std::ios::binary);
		amxfile.write(data.data() + pos, amx.header.size);
		amxfile.close();
		msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Output filename: " << scriptname << ".amx";

		if (!AMX_Disassemble(amx, scriptname + ".asm"))
			result = false;
		else
			msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Output filename: " << scriptname << ".asm";

		if (!AMX_Decompile(amx, scriptname + ".p"))
			result = false;
		else
			msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Output filename: " << scriptname << ".p";

		pos += scx_entry.Size;
	}
	return result;
}
