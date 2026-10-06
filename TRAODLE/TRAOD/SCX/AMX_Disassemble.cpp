#include "stdafx.h"
#include <map>
#include <set>
#include "TRAOD/SCX/SCX_Functions.h"


const AMX_OPCODE_INFO AMX_Opcodes[OP_NUM_OPCODES] = {
	{"none", 0}, {"load.pri", 1}, {"load.alt", 1}, {"load.s.pri", 1}, {"load.s.alt", 1}, {"lref.pri", 1}, {"lref.alt", 1}, {"lref.s.pri", 1}, {"lref.s.alt", 1}, {"load.i", 0},
	{"lodb.i", 1}, {"const.pri", 1}, {"const.alt", 1}, {"addr.pri", 1}, {"addr.alt", 1}, {"stor.pri", 1}, {"stor.alt", 1}, {"stor.s.pri", 1}, {"stor.s.alt", 1}, {"sref.pri", 1},
	{"sref.alt", 1}, {"sref.s.pri", 1}, {"sref.s.alt", 1}, {"stor.i", 0}, {"strb.i", 1}, {"lidx", 0}, {"lidx.b", 1}, {"idxaddr", 0}, {"idxaddr.b", 1}, {"align.pri", 1},
	{"align.alt", 1}, {"lctrl", 1}, {"sctrl", 1}, {"move.pri", 0}, {"move.alt", 0}, {"xchg", 0}, {"push.pri", 0}, {"push.alt", 0}, {"push.r", 1}, {"push.c", 1},
	{"push", 1}, {"push.s", 1}, {"pop.pri", 0}, {"pop.alt", 0}, {"stack", 1}, {"heap", 1}, {"proc", 0}, {"ret", 0}, {"retn", 0}, {"call", 1},
	{"call.pri", 0}, {"jump", 1}, {"jrel", 1}, {"jzer", 1}, {"jnz", 1}, {"jeq", 1}, {"jneq", 1}, {"jless", 1}, {"jleq", 1}, {"jgrtr", 1},
	{"jgeq", 1}, {"jsless", 1}, {"jsleq", 1}, {"jsgrtr", 1}, {"jsgeq", 1}, {"shl", 0}, {"shr", 0}, {"sshr", 0}, {"shl.c.pri", 1}, {"shl.c.alt", 1},
	{"shr.c.pri", 1}, {"shr.c.alt", 1}, {"smul", 0}, {"sdiv", 0}, {"sdiv.alt", 0}, {"umul", 0}, {"udiv", 0}, {"udiv.alt", 0}, {"add", 0}, {"sub", 0},
	{"sub.alt", 0}, {"and", 0}, {"or", 0}, {"xor", 0}, {"not", 0}, {"neg", 0}, {"invert", 0}, {"add.c", 1}, {"smul.c", 1}, {"zero.pri", 0},
	{"zero.alt", 0}, {"zero", 1}, {"zero.s", 1}, {"sign.pri", 0}, {"sign.alt", 0}, {"eq", 0}, {"neq", 0}, {"less", 0}, {"leq", 0}, {"grtr", 0},
	{"geq", 0}, {"sless", 0}, {"sleq", 0}, {"sgrtr", 0}, {"sgeq", 0}, {"eq.c.pri", 1}, {"eq.c.alt", 1}, {"inc.pri", 0}, {"inc.alt", 0}, {"inc", 1},
	{"inc.s", 1}, {"inc.i", 0}, {"dec.pri", 0}, {"dec.alt", 0}, {"dec", 1}, {"dec.s", 1}, {"dec.i", 0}, {"movs", 1}, {"cmps", 1}, {"fill", 1},
	{"halt", 1}, {"bounds", 1}, {"sysreq.pri", 0}, {"sysreq.c", 1}, {"file", -1}, {"line", 2}, {"symbol", -1}, {"srange", 2}, {"jump.pri", 0}, {"switch", 1},
	{"casetbl", -1}, {"swap.pri", 0}, {"swap.alt", 0}, {"push.adr", 1}, {"nop", 0}, {"sysreq.d", 1}, {"symtag", 1}, {"break", 0}
};


static string Hex (uint32_t value, int width)
{
	stringstream ss;
	ss << hex << uppercase << setw(width) << setfill('0') << value;
	return ss.str();
}


/*------------------------------------------------------------------------------------------------------------------
Rappresentazione testuale di una costante. I valori grandi che corrispondono ad un float "semplice" (al massimo 6 cifre
significative) vengono scritti come float, gli altri valori grandi (es. hash) in esadecimale.
------------------------------------------------------------------------------------------------------------------*/
string AMX_RenderNumber (int32_t value)
{
	if (value >= 0x01000000 || value <= -0x01000000)
	{
		float f;
		memcpy(&f, &value, sizeof(f));
		if (isfinite(f) && fabs(f) >= 1e-4f && fabs(f) < 1e7f)
		{
			stringstream ss;
			ss << setprecision(6) << f;
			if (strtof(ss.str().c_str(), NULL) == f)			// Il valore e' esprimibile con al massimo 6 cifre significative
				for (int d = 1; d <= 12; d++)					// Scrittura in notazione fissa con il minimo numero di decimali
				{
					stringstream fx;
					fx << fixed << setprecision(d) << f;
					if (strtof(fx.str().c_str(), NULL) == f)
						return fx.str();
				}
		}
		return "0x" + Hex((uint32_t)value, 8);
	}
	return to_string(value);
}


bool AMX_SCRIPT::ReadString (int32_t address, string &s) const
{
	s.clear();
	if (!IsDataAddress(address))
		return false;
	if ((uint32_t)GetDataCell(address) > 0xFFFFFF)		// Stringa packed: 4 caratteri per cella, a partire dal byte piu' significativo
	{
		for (int32_t p = address; IsDataAddress(p); p += 4)
		{
			uint32_t cell = GetDataCell(p);
			for (int shift = 24; shift >= 0; shift -= 8)
			{
				unsigned char c = (cell >> shift) & 0xFF;
				if (c == 0)
					return s.size() >= 2;
				if (c < 32 || c > 126)
					return false;
				s.push_back(c);
			}
		}
		return false;
	}
	for (int32_t p = address; IsDataAddress(p); p += 4)	// Stringa unpacked: 1 carattere per cella
	{
		int32_t cell = GetDataCell(p);
		if (cell == 0)
			return s.size() >= 2;
		if (cell < 32 || cell > 126)
			return false;
		s.push_back((char)cell);
	}
	return false;
}


/*------------------------------------------------------------------------------------------------------------------
Legge un file AMX (Small 2.x) dalla memoria: header, tabelle dei simboli, espansione di codice e dati compressi
e decodifica delle istruzioni.
------------------------------------------------------------------------------------------------------------------*/
bool AMX_Load (const char *data, uint32_t size, AMX_SCRIPT &amx)
{
	if (size < sizeof(AMX_HEADER))
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "AMX block too small (" << size << " bytes).";
		return false;
	}
	memcpy(&amx.header, data, sizeof(AMX_HEADER));
	const AMX_HEADER &h = amx.header;
	if (h.magic != 0xF1E0)
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Invalid AMX magic 0x" << Hex(h.magic, 4) << ".";
		return false;
	}
	if (h.file_version != 6)
		msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << "Unexpected AMX file version " << (int)h.file_version << ". Output may be wrong.";
	if ((uint32_t)h.size > size || h.defsize < 8 || h.cod < (int32_t)sizeof(AMX_HEADER) || h.cod > h.size || h.dat < h.cod || h.hea < h.dat ||
		h.publics > h.natives || h.natives > h.libraries || h.libraries > h.pubvars || h.pubvars > h.tags || h.tags > h.cod || (h.dat - h.cod) % 4 || (h.hea - h.dat) % 4)
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Invalid AMX header.";
		return false;
	}

	// Lettura tabelle dei simboli
	auto ReadTable = [&](int32_t from, int32_t to, vector <AMX_SYMBOL> &table)
	{
		for (int32_t p = from; p + h.defsize <= to; p += h.defsize)
		{
			AMX_SYMBOL symbol;
			memcpy(&symbol.address, data + p, sizeof(symbol.address));
			const char *name = data + p + 4;
			unsigned int len = 0;
			while (len < (unsigned int)h.defsize - 4 && name[len])
				len++;
			symbol.name = string(name, len);
			table.push_back(symbol);
		}
	};
	ReadTable(h.publics, h.natives, amx.publics);
	ReadTable(h.natives, h.libraries, amx.natives);
	ReadTable(h.libraries, h.pubvars, amx.libraries);
	ReadTable(h.pubvars, h.tags, amx.pubvars);
	ReadTable(h.tags, h.cod, amx.tags);

	// Espansione codice e dati
	amx.code_size = h.dat - h.cod;
	amx.data_size = h.hea - h.dat;
	unsigned int nCells = (h.hea - h.cod) / 4;
	amx.image.clear();
	amx.image.reserve(nCells);
	if (h.flags & 0x0004)					// AMX_FLAG_COMPACT: ogni cella e' codificata in gruppi di 7 bit, dal piu' significativo. Il bit 0x80 indica che la cella continua nel byte successivo, il bit 0x40 del primo byte e' il segno
	{
		int32_t p = h.cod;
		while (p < h.size && amx.image.size() < nCells)
		{
			uint32_t cell = ((unsigned char)data[p] & 0x40) ? 0xFFFFFFFF : 0;
			unsigned char b;
			do {
				b = (unsigned char)data[p++];
				cell = (cell << 7) | (b & 0x7F);
			} while ((b & 0x80) && p < h.size);
			amx.image.push_back((int32_t)cell);
		}
	}
	else
		for (int32_t p = h.cod; p + 4 <= h.size && amx.image.size() < nCells; p += 4)
		{
			int32_t cell;
			memcpy(&cell, data + p, sizeof(cell));
			amx.image.push_back(cell);
		}
	if (amx.image.size() < nCells)
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << "AMX code/data shorter than expected (" << amx.image.size() << " of " << nCells << " cells).";
		amx.image.resize(nCells, 0);
	}

	// Decodifica istruzioni
	unsigned int nCodeCells = amx.code_size / 4;
	amx.code.clear();
	amx.address_to_instruction.assign(nCodeCells, -1);
	unsigned int i = 0;
	while (i < nCodeCells)
	{
		AMX_INSTRUCTION ins;
		ins.address = i * 4;
		ins.opcode = amx.image[i];
		int nParams = 0;
		if (ins.opcode == OP_CASETBL)
			nParams = (i + 1 < nCodeCells) ? 2 * amx.image[i + 1] + 2 : 0;
		else if (ins.opcode == OP_FILE || ins.opcode == OP_SYMBOL)
			nParams = (i + 1 < nCodeCells) ? 1 + amx.image[i + 1] / 4 : 0;
		else if (ins.opcode > OP_NONE && ins.opcode < OP_NUM_OPCODES)
			nParams = AMX_Opcodes[ins.opcode].nParams;
		nParams = max(0, min(nParams, (int)(nCodeCells - i - 1)));
		ins.param.assign(amx.image.begin() + i + 1, amx.image.begin() + i + 1 + nParams);
		amx.address_to_instruction[i] = (int)amx.code.size();
		amx.code.push_back(ins);
		i += 1 + nParams;
	}
	return true;
}


/*------------------------------------------------------------------------------------------------------------------
Nomi delle funzioni: le funzioni pubbliche hanno il nome contenuto nella tabella publics, le altre (destinazioni di
istruzioni call o proc senza nome) vengono chiamate func_XXXX dove XXXX e' l'indirizzo
------------------------------------------------------------------------------------------------------------------*/
map <uint32_t, string> AMX_FunctionNames (const AMX_SCRIPT &amx)
{
	map <uint32_t, string> names;
	for (unsigned int i = 0; i < amx.publics.size(); i++)
		names[amx.publics[i].address] = amx.publics[i].name;
	for (unsigned int i = 0; i < amx.code.size(); i++)
	{
		uint32_t address = 0xFFFFFFFF;
		if (amx.code[i].opcode == OP_PROC)
			address = amx.code[i].address;
		else if (amx.code[i].opcode == OP_CALL && !amx.code[i].param.empty())
			address = amx.code[i].param[0];
		if (address != 0xFFFFFFFF && !names.count(address))
			names[address] = "func_" + Hex(address, 4);
	}
	return names;
}


static bool IsJump (int opcode)
{
	return opcode == OP_JUMP || (opcode >= OP_JZER && opcode <= OP_JSGEQ);
}


bool AMX_Disassemble (const AMX_SCRIPT &amx, string filename)
{
	ofstream out(filename);
	if (!out.is_open())
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Unable to create " << filename;
		return false;
	}
	const AMX_HEADER &h = amx.header;
	map <uint32_t, string> functions = AMX_FunctionNames(amx);

	// Etichette di salto
	set <uint32_t> labels;
	for (unsigned int i = 0; i < amx.code.size(); i++)
	{
		const AMX_INSTRUCTION &ins = amx.code[i];
		if ((IsJump(ins.opcode) || ins.opcode == OP_SWITCH) && !ins.param.empty())
			labels.insert(ins.param[0]);
		if (ins.opcode == OP_CASETBL && ins.param.size() >= 2)
		{
			labels.insert(ins.param[1]);							// default
			for (unsigned int p = 2; p + 1 < ins.param.size(); p += 2)
				labels.insert(ins.param[p + 1]);					// case
		}
	}

	out << "; " << filename << "\n";
	out << "; Small 2.x AMX - file version " << (int)h.file_version << ", required VM version " << (int)h.amx_version << ", flags 0x" << Hex(h.flags, 4) << "\n";
	out << "; size " << h.size << ", code " << amx.code_size << " bytes, data " << amx.data_size << " bytes, stack/heap " << h.stp - h.hea << " bytes\n";
	if (h.cip >= 0)
		out << "; main at " << Hex(h.cip, 4) << "\n";
	out << ";\n; Publics\n";
	for (unsigned int i = 0; i < amx.publics.size(); i++)
		out << ";   " << Hex(amx.publics[i].address, 4) << "  " << amx.publics[i].name << "\n";
	out << ";\n; Natives\n";
	for (unsigned int i = 0; i < amx.natives.size(); i++)
		out << ";   " << setw(3) << setfill(' ') << i << "  " << amx.natives[i].name << "\n";
	if (!amx.pubvars.empty())
	{
		out << ";\n; Public variables\n";
		for (unsigned int i = 0; i < amx.pubvars.size(); i++)
			out << ";   " << Hex(amx.pubvars[i].address, 4) << "  " << amx.pubvars[i].name << "\n";
	}
	if (!amx.tags.empty())
	{
		out << ";\n; Tags\n";
		for (unsigned int i = 0; i < amx.tags.size(); i++)
			out << ";   " << Hex(amx.tags[i].address, 8) << "  " << amx.tags[i].name << "\n";
	}

	// Codice
	out << "\n\n.CODE\n";
	for (unsigned int i = 0; i < amx.code.size(); i++)
	{
		const AMX_INSTRUCTION &ins = amx.code[i];
		if (ins.opcode == OP_PROC && functions.count(ins.address))
			out << "\n; ------------------------------------------------------------------\n" << functions[ins.address] << ":\n";
		else if (labels.count(ins.address))
			out << "L_" << Hex(ins.address, 4) << ":\n";

		string name = (ins.opcode >= 0 && ins.opcode < OP_NUM_OPCODES) ? AMX_Opcodes[ins.opcode].name : "??? (" + to_string(ins.opcode) + ")";
		stringstream line;
		line << "    " << Hex(ins.address, 4) << "  " << left << setw(12) << setfill(' ') << name;
		string comment;
		if (ins.opcode == OP_CASETBL)
		{
			out << line.str() << "\n";
			if (ins.param.size() >= 2)
			{
				out << "                  default -> L_" << Hex(ins.param[1], 4) << "\n";
				for (unsigned int p = 2; p + 1 < ins.param.size(); p += 2)
					out << "                  case " << setw(11) << ins.param[p] << " -> L_" << Hex(ins.param[p + 1], 4) << "\n";
			}
			continue;
		}
		for (unsigned int p = 0; p < ins.param.size(); p++)
		{
			if (p)
				line << ", ";
			if ((IsJump(ins.opcode) || ins.opcode == OP_SWITCH) && p == 0)
				line << "L_" << Hex(ins.param[p], 4);
			else if (ins.opcode == OP_CALL && p == 0)
				line << (functions.count(ins.param[p]) ? functions[ins.param[p]] : "L_" + Hex(ins.param[p], 4));
			else
				line << ins.param[p];
		}
		if (ins.opcode == OP_SYSREQ_C && !ins.param.empty())
			comment = (ins.param[0] >= 0 && (unsigned int)ins.param[0] < amx.natives.size()) ? amx.natives[ins.param[0]].name : "unknown native";
		else if ((ins.opcode == OP_PUSH_C || ins.opcode == OP_CONST_PRI || ins.opcode == OP_CONST_ALT) && !ins.param.empty())
		{
			string s;
			if (amx.ReadString(ins.param[0], s))
				comment = "\"" + s + "\"";
			else if (!AMX_HashName(ins.param[0]).empty())
				comment = AMX_HashName(ins.param[0]);
			else if (AMX_RenderNumber(ins.param[0]) != to_string(ins.param[0]))
				comment = AMX_RenderNumber(ins.param[0]);
		}
		out << line.str();
		if (!comment.empty())
			out << string(max(1, 40 - (int)line.str().size()), ' ') << "; " << comment;
		out << "\n";
	}

	// Dati
	out << "\n\n.DATA\n";
	for (uint32_t a = 0; a < amx.data_size; a += 16)
	{
		out << "    " << Hex(a, 4) << " ";
		string render;
		for (uint32_t c = a; c < a + 16 && c < amx.data_size; c += 4)
		{
			out << " " << Hex(amx.GetDataCell(c), 8);
			string name = AMX_HashName(amx.GetDataCell(c));
			render += "  " + (name.empty() ? AMX_RenderNumber(amx.GetDataCell(c)) : name);
		}
		out << "    ;" << render << "\n";
	}
	return true;
}
