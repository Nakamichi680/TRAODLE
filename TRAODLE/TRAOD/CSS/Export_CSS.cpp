#include "stdafx.h"
#include "Classes.h"
#include "TRAOD/CSS/CSS_Functions.h"


static string CSS_Time (uint32_t frame)					// Tempo mm:ss.mmm di un frame a 30 fps
{
	uint32_t ms = (uint32_t)((uint64_t)frame * 1000 / 30);
	char buf[32];
	snprintf(buf, sizeof(buf), "%02u:%02u.%03u", ms / 60000, (ms / 1000) % 60, ms % 1000);
	return buf;
}


/*------------------------------------------------------------------------------------------------------------------
Esporta i sottotitoli di un filmato (file CSS, vedi CSS_Struct.h) in un file di testo nella cartella
\NOMELIVELLO\Subtitles (<FILMATO>.txt): per ogni sottotitolo frame di inizio, durata e fine (30 fps, frame 0 = inizio
del filmato, come le animazioni e la camera esportate), gli stessi tempi in minuti e secondi e la chiave del testo nel
database dei testi del gioco (il testo non e' contenuto nei file del livello).
------------------------------------------------------------------------------------------------------------------*/
bool Export_CSS (string filename)
{
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Reading " << filename;
	SetCurrentDirectory(AOD_IO.folder_level_lpwstr);			// \NOMELIVELLO
	vector <uint8_t> data;
	{
		ifstream cssfile(filename, std::ios::binary);
		if (!cssfile.is_open())
		{
			msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " not found.";
			return false;
		}
		data.assign(istreambuf_iterator<char>(cssfile), istreambuf_iterator<char>());
	}
	CSS_HEADER header;
	if (data.size() < sizeof(header))
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " is not a valid CSS file.";
		return false;
	}
	memcpy(&header, data.data(), sizeof(header));
	if (header.Magic != CSS_MAGIC || sizeof(CSS_HEADER) + (size_t)header.nLines * sizeof(CSS_LINE) > data.size())
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << filename << " is not a valid CSS file.";
		return false;
	}

	string name = filename.substr(0, filename.find('.'));
	stringstream out;
	out << "// Subtitles of cutscene " << name << " (" << filename << "): " << header.nLines << " lines\n";
	out << "// Frames at 30 fps, frame 0 = start of the cutscene (same timeline as the exported animations and camera).\n";
	out << "// Length = length of the audio clip of the line. Text ID = key of the text in the game text database.\n\n";

	// Colonne allineate a destra: l'ultimo carattere di ogni valore e' sotto l'ultimo carattere dell'intestazione
	static const char *titles[] = {"N", "Start", "Length", "End", "Start (time)", "End (time)", "Text ID"};
	static const int widths[] = {4, 9, 9, 9, 15, 15, 13};
	auto Row = [&](const vector <string> &cells) -> string
	{
		stringstream row;
		for (unsigned int c = 0; c < cells.size(); c++)
			row << right << setw(widths[c]) << cells[c];
		return row.str();
	};
	out << Row(vector <string> (begin(titles), end(titles))) << "\n";
	for (uint32_t l = 0; l < header.nLines; l++)
	{
		CSS_LINE line;
		memcpy(&line, &data[sizeof(CSS_HEADER) + l * sizeof(CSS_LINE)], sizeof(line));
		char id[16];
		snprintf(id, sizeof(id), "0x%08X", line.TextID);
		out << Row({to_string(l + 1), to_string(line.StartFrame), to_string(line.nFrames), to_string(line.StartFrame + line.nFrames),
					CSS_Time(line.StartFrame), CSS_Time(line.StartFrame + line.nFrames), id});
		if (line.TextID == 0)
			out << "   (no text assigned)";
		if (line.Flags)
			out << "   flags 0x" << hex << uppercase << line.Flags << dec;
		out << "\n";
	}
	if (header.Flags1 || header.Flags2)
		out << "\n// File flags: 0x" << hex << uppercase << header.Flags1 << " 0x" << header.Flags2 << dec << "\n";

	// File di testo nella cartella \NOMELIVELLO\Subtitles
	string folder = AOD_IO.folder_level + "Subtitles";
	CreateDirectoryA(folder.c_str(), NULL);
	if (!SetCurrentDirectoryA(folder.c_str()))
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Unable to access folder " << folder << ". Subtitles not exported.";
		return false;
	}
	ofstream txt(name + ".txt");
	if (!txt.is_open())
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Unable to create " << name << ".txt";
		return false;
	}
	txt << out.str();
	txt.close();
	msg(msg::TGT::FILE_CONS, msg::TYP::LOG) << "Output: Subtitles\\" << name << ".txt (" << header.nLines << " subtitles)";
	return true;
}
