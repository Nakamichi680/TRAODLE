#pragma once
#include <map>
#include "TRAOD/SCX/SCX_Struct.h"


// Opcodes della macchina virtuale Small 2.x (stessa numerazione di Pawn)
enum AMX_OP {
	OP_NONE, OP_LOAD_PRI, OP_LOAD_ALT, OP_LOAD_S_PRI, OP_LOAD_S_ALT, OP_LREF_PRI, OP_LREF_ALT, OP_LREF_S_PRI, OP_LREF_S_ALT, OP_LOAD_I,
	OP_LODB_I, OP_CONST_PRI, OP_CONST_ALT, OP_ADDR_PRI, OP_ADDR_ALT, OP_STOR_PRI, OP_STOR_ALT, OP_STOR_S_PRI, OP_STOR_S_ALT, OP_SREF_PRI,
	OP_SREF_ALT, OP_SREF_S_PRI, OP_SREF_S_ALT, OP_STOR_I, OP_STRB_I, OP_LIDX, OP_LIDX_B, OP_IDXADDR, OP_IDXADDR_B, OP_ALIGN_PRI,
	OP_ALIGN_ALT, OP_LCTRL, OP_SCTRL, OP_MOVE_PRI, OP_MOVE_ALT, OP_XCHG, OP_PUSH_PRI, OP_PUSH_ALT, OP_PUSH_R, OP_PUSH_C,
	OP_PUSH, OP_PUSH_S, OP_POP_PRI, OP_POP_ALT, OP_STACK, OP_HEAP, OP_PROC, OP_RET, OP_RETN, OP_CALL,
	OP_CALL_PRI, OP_JUMP, OP_JREL, OP_JZER, OP_JNZ, OP_JEQ, OP_JNEQ, OP_JLESS, OP_JLEQ, OP_JGRTR,
	OP_JGEQ, OP_JSLESS, OP_JSLEQ, OP_JSGRTR, OP_JSGEQ, OP_SHL, OP_SHR, OP_SSHR, OP_SHL_C_PRI, OP_SHL_C_ALT,
	OP_SHR_C_PRI, OP_SHR_C_ALT, OP_SMUL, OP_SDIV, OP_SDIV_ALT, OP_UMUL, OP_UDIV, OP_UDIV_ALT, OP_ADD, OP_SUB,
	OP_SUB_ALT, OP_AND, OP_OR, OP_XOR, OP_NOT, OP_NEG, OP_INVERT, OP_ADD_C, OP_SMUL_C, OP_ZERO_PRI,
	OP_ZERO_ALT, OP_ZERO, OP_ZERO_S, OP_SIGN_PRI, OP_SIGN_ALT, OP_EQ, OP_NEQ, OP_LESS, OP_LEQ, OP_GRTR,
	OP_GEQ, OP_SLESS, OP_SLEQ, OP_SGRTR, OP_SGEQ, OP_EQ_C_PRI, OP_EQ_C_ALT, OP_INC_PRI, OP_INC_ALT, OP_INC,
	OP_INC_S, OP_INC_I, OP_DEC_PRI, OP_DEC_ALT, OP_DEC, OP_DEC_S, OP_DEC_I, OP_MOVS, OP_CMPS, OP_FILL,
	OP_HALT, OP_BOUNDS, OP_SYSREQ_PRI, OP_SYSREQ_C, OP_FILE, OP_LINE, OP_SYMBOL, OP_SRANGE, OP_JUMP_PRI, OP_SWITCH,
	OP_CASETBL, OP_SWAP_PRI, OP_SWAP_ALT, OP_PUSHADDR, OP_NOP, OP_SYSREQ_D, OP_SYMTAG, OP_BREAK,
	OP_NUM_OPCODES
};


struct AMX_OPCODE_INFO
{
	const char *name;
	int nParams;				// Numero di parametri (-1 = numero variabile)
};

extern const AMX_OPCODE_INFO AMX_Opcodes[OP_NUM_OPCODES];


class AMX_INSTRUCTION {
public:
	uint32_t address;			// Indirizzo in bytes relativo all'inizio del blocco codice
	int opcode;
	vector <int32_t> param;
};


class AMX_SYMBOL {
public:
	uint32_t address;
	string name;
};


// Script AMX letto e decodificato in memoria
class AMX_SCRIPT {
public:
	AMX_HEADER header;
	vector <AMX_SYMBOL> publics, natives, libraries, pubvars, tags;
	vector <int32_t> image;					// Codice + dati espansi (in celle da 4 bytes), a partire da header.cod
	vector <AMX_INSTRUCTION> code;			// Istruzioni decodificate
	vector <int> address_to_instruction;	// Indirizzo/4 -> indice in code (-1 se l'indirizzo non e' l'inizio di un'istruzione)
	uint32_t code_size = 0;					// Dimensione del blocco codice in bytes
	uint32_t data_size = 0;					// Dimensione del blocco dati in bytes

	int FindInstruction (uint32_t address) const			// Ritorna l'indice dell'istruzione all'indirizzo indicato. Se l'indirizzo coincide con la fine del codice ritorna code.size(), altrimenti -1
	{
		if (address == code_size)
			return (int)code.size();
		if (address % 4 || address / 4 >= address_to_instruction.size())
			return -1;
		return address_to_instruction[address / 4];
	}
	bool IsDataAddress (int32_t address) const
	{
		return address >= 0 && (uint32_t)address < data_size && address % 4 == 0;
	}
	int32_t GetDataCell (int32_t address) const
	{
		return IsDataAddress(address) ? image[(code_size + address) / 4] : 0;
	}
	bool ReadString (int32_t address, string &s) const;		// Legge una stringa (packed o unpacked) dal blocco dati. Ritorna false se i dati non sembrano una stringa
};


bool Export_SCX (string filename);

bool AMX_Load (const char *data, uint32_t size, AMX_SCRIPT &amx);

bool AMX_Disassemble (const AMX_SCRIPT &amx, string filename);

bool AMX_Decompile (const AMX_SCRIPT &amx, string filename);

string AMX_RenderNumber (int32_t value);

map <uint32_t, string> AMX_FunctionNames (const AMX_SCRIPT &amx);

void SCX_CollectHashNames ();				// Raccoglie i nomi degli hash dai file del livello (SCX_HashNames.cpp)

string AMX_HashName (int32_t value);		// Nome dell'hash indicato, "" se sconosciuto
