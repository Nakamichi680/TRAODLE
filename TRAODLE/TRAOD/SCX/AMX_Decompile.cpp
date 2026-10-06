#include "stdafx.h"
#include <map>
#include <set>
#include <memory>
#include "TRAOD/SCX/SCX_Functions.h"
#include "hash_Functions.h"


/*------------------------------------------------------------------------------------------------------------------
Decompilatore AMX (Small 2.x) -> pseudo-codice Pawn.

Il codice di ogni funzione viene eseguito simbolicamente: i registri PRI e ALT e lo stack contengono espressioni invece
di valori. Le istruzioni con effetti collaterali (assegnazioni, chiamate il cui risultato non viene usato, ...) generano
righe di output. Le strutture di controllo vengono ricostruite riconoscendo gli schemi generati dal compilatore:
 - if / if-else / operatore ternario (salto condizionato in avanti, eventuale jump finale sopra al ramo else)
 - && e || (valore 0/1 materializzato: ... jzer F ... const.pri 1, jump E, F: zero.pri, E:)
 - while (salto all'indietro incondizionato), do-while (salto all'indietro condizionato),
   while con test in fondo (jump in avanti alla condizione + salto condizionato all'indietro)
 - switch (switch + casetbl)
Tutto cio' che non viene riconosciuto viene scritto con goto ed etichette.
------------------------------------------------------------------------------------------------------------------*/

namespace {

enum PREC { P_LOWEST, P_ASSIGN, P_TERNARY, P_LOR, P_LAND, P_BOR, P_BXOR, P_BAND, P_EQ, P_REL, P_SHIFT, P_ADD, P_MUL, P_UNARY, P_ATOM };

struct Expr;
typedef shared_ptr <Expr> ExprP;

struct Expr {
	string text;
	int prec = P_ATOM;
	bool isConst = false;
	int32_t value = 0;
	int32_t constBase = -1;			// Se la costante e' un indirizzo nel blocco dati calcolato come base + offset: base
	bool sideEffect = false;		// Chiamata a funzione o ++/--: se il valore non viene usato va scritta come istruzione
	bool used = false;
	string var;						// Nome della variabile, se l'espressione e' la lettura di una variabile
	bool isAddress = false;			// L'espressione e' l'indirizzo di una variabile o di un elemento di un array
	string base;					// Nome della variabile/array
	bool baseIsArray = false;
	ExprP index;					// Indice dinamico (in celle)
	int32_t offset = 0;				// Indice costante (in celle)
	bool isHeap = false;			// Cella temporanea sull'heap (passaggio di costanti per riferimento)
	ExprP heapValue;
	string cmpOp;					// Se l'espressione e' un confronto: operatore e operandi
	ExprP left, right;
	ExprP notInner;					// Se l'espressione e' una negazione (!x): x
	ExprP fcmpA, fcmpB;				// Se l'espressione e' floatcmp(a, b): a e b
};


string Hex (uint32_t value, int width)
{
	stringstream ss;
	ss << hex << uppercase << setw(width) << setfill('0') << value;
	return ss.str();
}


/*------------------------------------------------------------------------------------------------------------------
Costanti con nome: se il valore e' l'hash di un nome noto (SCX_HashNames.cpp) viene scritto il nome, dichiarato in testa
al file come costante. Se il nome non e' un identificatore valido o il valore non e' esattamente il suo hash (copie
rinominate delle animazioni) viene scritto il numero seguito dal nome in commento.
------------------------------------------------------------------------------------------------------------------*/
set <uint32_t> usedNames;				// Hash il cui nome e' stato usato come costante


bool IsIdentifier (const string &s)
{
	if (s.empty() || isdigit((unsigned char)s[0]))
		return false;
	for (unsigned char c : s)
		if (!isalnum(c) && c != '_')
			return false;
	return true;
}


string RenderValue (int32_t value)
{
	string name = AMX_HashName(value);
	if (name.empty())
		return AMX_RenderNumber(value);
	if (IsIdentifier(name) && (uint32_t)GetHashValue(name.c_str()) == (uint32_t)value)
	{
		usedNames.insert((uint32_t)value);
		return name;
	}
	return "0x" + Hex((uint32_t)value, 8) + " /* " + name + " */";
}


ExprP Atom (const string &text, int prec = P_ATOM)
{
	ExprP e = make_shared <Expr> ();
	e->text = text;
	e->prec = prec;
	return e;
}


ExprP Const (int32_t value)
{
	string text = RenderValue(value);
	ExprP e = Atom(text, text[0] == '-' ? P_UNARY : P_ATOM);
	e->isConst = true;
	e->value = value;
	return e;
}


ExprP Var (const string &name)
{
	ExprP e = Atom(name);
	e->var = name;
	return e;
}


ExprP Unknown ()
{
	return Atom("_unknown");
}


ExprP Use (ExprP e)
{
	if (!e)
		e = Unknown();
	e->used = true;
	return e;
}


string Paren (const ExprP &e, int prec)
{
	return e->prec < prec ? "(" + e->text + ")" : e->text;
}


ExprP Bin (const string &op, int prec, ExprP a, ExprP b)
{
	a = Use(a);
	b = Use(b);
	ExprP e = Atom(Paren(a, prec) + " " + op + " " + Paren(b, prec + 1), prec);
	e->sideEffect = a->sideEffect || b->sideEffect;
	return e;
}


ExprP Unary (const string &op, ExprP a)
{
	a = Use(a);
	ExprP e = Atom(op + Paren(a, P_UNARY), P_UNARY);
	e->sideEffect = a->sideEffect;
	return e;
}


string InvertOp (const string &op)
{
	if (op == "==") return "!=";
	if (op == "!=") return "==";
	if (op == "<") return ">=";
	if (op == ">=") return "<";
	if (op == ">") return "<=";
	return ">";						// "<="
}


ExprP Cmp (const string &op, ExprP a, ExprP b)
{
	a = Use(a);
	b = Use(b);
	if (a->fcmpA && b->isConst && b->value == 0)			// floatcmp(x, y) op 0  ->  x op y
		return Cmp(op, a->fcmpA, a->fcmpB);
	if (b->fcmpA && a->isConst && a->value == 0)			// 0 op floatcmp(x, y)  ->  y op x
		return Cmp(op, b->fcmpB, b->fcmpA);
	int prec = (op == "==" || op == "!=") ? P_EQ : P_REL;
	ExprP e = Atom(Paren(a, prec) + " " + op + " " + Paren(b, prec + 1), prec);
	e->cmpOp = op;
	e->left = a;
	e->right = b;
	e->sideEffect = a->sideEffect || b->sideEffect;
	return e;
}


ExprP Not (ExprP e)
{
	e = Use(e);
	if (!e->cmpOp.empty())
		return Cmp(InvertOp(e->cmpOp), e->left, e->right);
	if (e->notInner)
		return e->notInner;
	if (e->isConst)
		return Const(e->value == 0 ? 1 : 0);
	ExprP n = Unary("!", e);
	n->notInner = e;
	return n;
}


ExprP Logic (const string &op, const vector <ExprP> &operands)
{
	int prec = (op == "&&") ? P_LAND : P_LOR;
	string text;
	bool sideEffect = false;
	for (unsigned int i = 0; i < operands.size(); i++)
	{
		ExprP o = Use(operands[i]);
		text += (i ? " " + op + " " : "") + Paren(o, prec + 1);
		sideEffect = sideEffect || o->sideEffect;
	}
	ExprP e = Atom(text, prec);
	e->sideEffect = sideEffect;
	return e;
}


ExprP Ternary (ExprP c, ExprP a, ExprP b)
{
	c = Use(c);
	a = Use(a);
	b = Use(b);
	ExprP e = Atom(Paren(c, P_TERNARY + 1) + " ? " + Paren(a, P_TERNARY + 1) + " : " + Paren(b, P_TERNARY), P_TERNARY);
	e->sideEffect = c->sideEffect || a->sideEffect || b->sideEffect;
	return e;
}


string AddressText (const ExprP &a)			// Testo dell'indirizzo passato come argomento (es. "arr", "arr[2]", "x")
{
	if (!a->index && a->offset == 0)
		return a->base;
	string idx;
	if (a->index)
		idx = a->offset ? Paren(a->index, P_ADD) + " + " + to_string(a->offset) : a->index->text;
	else
		idx = to_string(a->offset);
	return a->base + "[" + idx + "]";
}


ExprP MakeAddress (const string &base, bool isArray, ExprP index, int32_t offset)
{
	ExprP e = make_shared <Expr> ();
	e->isAddress = true;
	e->base = base;
	e->baseIsArray = isArray;
	e->index = index;
	e->offset = offset;
	e->text = AddressText(e);
	return e;
}


string Quote (const string &s)
{
	string q = "\"";
	for (unsigned int i = 0; i < s.size(); i++)
	{
		if (s[i] == '"' || s[i] == '\\')
			q.push_back('\\');
		q.push_back(s[i]);
	}
	return q + "\"";
}


struct Slot {									// Elemento dello stack (variabile locale o argomento in attesa di una chiamata)
	ExprP val;									// Valore inserito con push (nullo se allocato con stack)
	int line = -1;								// Riga riservata per l'eventuale dichiarazione della variabile
	bool declared = false;
	int cells = 1;
};


struct State {
	ExprP pri, alt;
	vector <Slot> stack;
	int credit = 0;								// Bytes da ignorare alla prossima istruzione stack (argomenti gia' rimossi da sysreq)
	bool terminated = false;					// L'ultima istruzione era un salto o un return: il codice seguente e' raggiungibile solo tramite etichette
};


struct Line {
	int indent;
	string text;
	bool active;
};


struct Context {								// Ciclo o switch in corso di decompilazione
	bool isSwitch;
	uint32_t cont;								// Destinazione di continue (0xFFFFFFFF se assente)
	uint32_t exit;								// Destinazione di break
};


struct Function {
	uint32_t address;
	string name;
	bool isPublic;
	int maxArg;
	set <int> refArgs;
	vector <Line> lines;
};


class Decompiler {
public:
	Decompiler (const AMX_SCRIPT &a) : amx(a), code(a.code) {}
	bool Run (string filename);

private:
	const AMX_SCRIPT &amx;
	const vector <AMX_INSTRUCTION> &code;
	map <uint32_t, string> funcNames;
	set <uint32_t> publicAddresses;
	set <uint32_t> allTargets;					// Tutte le destinazioni di salto dello script
	map <int, int> nativeArgCount;
	map <uint32_t, int> funcArgCount;
	set <int32_t> globals;
	set <int32_t> globalArrays;					// Variabili globali usate come array
	map <int32_t, int> globalSizes;				// Dimensione (in celle) delle variabili globali copiate con movs

	// Stato della funzione in corso di decompilazione
	vector <Line> out;
	vector <Context> ctx;
	set <int> activeHeads;						// Indici delle istruzioni che sono testa di un ciclo in corso di decompilazione
	set <uint32_t> gotoTargets, newGotoTargets, labelsEmitted;
	map <int, ExprP> breakLines;				// Righe "if (cond) break;" con la relativa condizione
	bool emitLabels = false;
	int funcEnd = 0;
	int maxArg = -1;
	set <int> refArgs;

	uint32_t AddrOf (int i) const { return i < (int)code.size() ? code[i].address : amx.code_size; }
	static bool IsCondJump (int op) { return op >= OP_JZER && op <= OP_JSGEQ; }
	static bool IsJump (int op) { return op == OP_JUMP || IsCondJump(op); }

	int AddLine (int indent, const string &text) { out.push_back({indent, text, true}); return (int)out.size() - 1; }
	int AddPlaceholder (int indent) { out.push_back({indent, "", false}); return (int)out.size() - 1; }
	void SetLine (int line, const string &text) { if (line >= 0 && line < (int)out.size()) { out[line].text = text; out[line].active = true; } }
	bool HasActive (int from, int to) const { for (int i = from; i < to && i < (int)out.size(); i++) if (out[i].active) return true; return false; }
	int FirstActive (int from) const { for (int i = from; i < (int)out.size(); i++) if (out[i].active) return i; return -1; }

	string GlobalName (int32_t address);
	string SlotName (int base) const { return "var" + to_string(-base / 4); }
	ExprP FrameAddress (State &st, int32_t offset);
	void DeclareSlot (State &st, int k, int base);
	string Lvalue (ExprP address);
	string ArrayRef (ExprP address, int cells);
	ExprP Indexed (ExprP base, ExprP index);
	string ArgText (ExprP a, const string &callee, int position);
	struct StringVotes {
		int valid = 0;							// Costanti che puntano all'inizio di una stringa valida
		int total = 0;							// Costanti totali
		bool proven = false;					// Almeno una costante diversa da 0 punta ad una stringa: l'argomento e' sicuramente una stringa
		bool zero = false;						// Almeno una costante vale 0
	};
	map <pair <string, int>, StringVotes> stringVotes;	// (funzione, posizione argomento) -> voti
	bool collectVotes = false;
	bool IsStringConst (int32_t value, string &s) const;

	void FlushReg (ExprP &r, int indent);
	void FlushAll (State &st, int indent) { FlushReg(st.pri, indent); FlushReg(st.alt, indent); }
	string FlushText (State &st);
	void SetPri (State &st, ExprP e, int indent) { FlushReg(st.pri, indent); st.pri = e; }
	void SetAlt (State &st, ExprP e, int indent) { FlushReg(st.alt, indent); st.alt = e; }
	void Push (State &st, ExprP e, int indent) { Slot s; s.val = e; s.line = AddPlaceholder(indent); st.stack.push_back(s); }
	ExprP PopValue (State &st, int indent);
	void PopCells (State &st, int cells, int indent);
	vector <ExprP> PopArgs (State &st, int &nbytes, int indent);
	void Store (const string &target, ExprP value, int indent);
	void StoreFrame (State &st, int32_t offset, ExprP value, int indent);
	void IncDec (State &st, const string &target, const string &op, int indent);
	ExprP JumpCond (State &st, int op);
	string BreakContinue (uint32_t target);

	void Block (int from, int to, State &st, int indent);
	int Instruction (int i, int to, State &st, int indent);
	int CondJump (int i, int to, State &st, int indent);
	int Jump (int i, int to, State &st, int indent);
	int TryLogical (int i, int to, State &st, int indent);
	int Loop (int i, int j, State &st, int indent);
	int BottomLoop (int i, int t, int k, State &st, int indent);
	int TryFor (int i, int t, int to, State &st, int indent);
	int Switch (int i, int to, State &st, int indent);
	void DecompileFunction (int first, int end, Function &f);
};


string Decompiler::GlobalName (int32_t address)
{
	for (unsigned int i = 0; i < amx.pubvars.size(); i++)
		if ((int32_t)amx.pubvars[i].address == address)
			return amx.pubvars[i].name;
	globals.insert(address);
	return "g_" + Hex(address, 4);
}


ExprP Decompiler::FrameAddress (State &st, int32_t offset)
{
	if (offset >= 12)							// Argomento della funzione
	{
		int n = (offset - 12) / 4;
		maxArg = max(maxArg, n);
		return MakeAddress("arg" + to_string(n), false, nullptr, 0);
	}
	if (offset == 8)
		return MakeAddress("_argbytes", false, nullptr, 0);
	if (offset >= 0)
		return MakeAddress("_frame" + to_string(offset), false, nullptr, 0);
	int cum = 0;
	for (unsigned int k = 0; k < st.stack.size(); k++)
	{
		int top = -cum * 4;
		cum += st.stack[k].cells;
		int base = -cum * 4;
		if (offset >= base && offset < top)
		{
			DeclareSlot(st, k, base);
			return MakeAddress(SlotName(base), st.stack[k].cells > 1, nullptr, (offset - base) / 4);
		}
	}
	return MakeAddress("var" + to_string(-offset / 4), false, nullptr, 0);
}


void Decompiler::DeclareSlot (State &st, int k, int base)
{
	Slot &s = st.stack[k];
	if (s.declared)
		return;
	s.declared = true;
	string name = SlotName(base);
	if (s.cells > 1)
		SetLine(s.line, "new " + name + "[" + to_string(s.cells) + "];");
	else if (!s.val || (s.val->isConst && s.val->value == 0))
		SetLine(s.line, "new " + name + ";");
	else
		SetLine(s.line, "new " + name + " = " + Use(s.val)->text + ";");
}


string Decompiler::Lvalue (ExprP a)					// Nome della cella di memoria all'indirizzo indicato
{
	a = Use(a);
	if (a->isAddress)
	{
		if (!a->index && a->offset == 0)
			return a->baseIsArray ? a->base + "[0]" : a->base;
		return AddressText(a);
	}
	if (a->isConst)
	{
		if (a->constBase >= 0 && a->value != a->constBase)
		{
			globalArrays.insert(a->constBase);
			return GlobalName(a->constBase) + "[" + to_string((a->value - a->constBase) / 4) + "]";
		}
		return GlobalName(a->value);
	}
	return "_mem[" + a->text + "]";
}


string Decompiler::ArrayRef (ExprP a, int cells)		// Nome di un intero array (destinazione/sorgente di movs)
{
	a = Use(a);
	if (a->isAddress)
		return AddressText(a);
	if (a->isConst)
	{
		int32_t base = a->constBase >= 0 ? a->constBase : a->value;
		if (a->value == base)
		{
			globalSizes[base] = max(globalSizes[base], cells);
			return GlobalName(base);
		}
		globalArrays.insert(base);
		return GlobalName(base) + "[" + to_string((a->value - base) / 4) + "]";
	}
	return "_mem[" + a->text + "]";
}


ExprP Decompiler::Indexed (ExprP base, ExprP index)		// Indirizzo dell'elemento index dell'array base
{
	base = Use(base);
	index = Use(index);
	if (base->isAddress && !base->index)
	{
		if (index->isConst)
			return MakeAddress(base->base, true, nullptr, base->offset + index->value);
		return MakeAddress(base->base, true, index, base->offset);
	}
	if (base->isConst)
	{
		int32_t b = base->constBase >= 0 ? base->constBase : base->value;
		int32_t offset = (base->value - b) / 4;
		globalArrays.insert(b);
		return MakeAddress(GlobalName(b), true, index->isConst ? nullptr : index, offset + (index->isConst ? index->value : 0));
	}
	return MakeAddress("_mem[" + base->text + "]", true, index->isConst ? nullptr : index, index->isConst ? index->value : 0);
}


/*------------------------------------------------------------------------------------------------------------------
Testo di un argomento di una chiamata. Una costante puo' essere sia un numero che l'indirizzo di una stringa nel blocco
dati (i voti vengono raccolti in un passaggio preliminare). Viene scritta come stringa solo se:
 - in tutte le chiamate alla stessa funzione l'argomento in quella posizione e' una costante che punta all'inizio di
   una stringa valida
 - se il valore e' 0 (indirizzo della prima stringa, ma anche l'intero piu' comune): la posizione e' dimostrata essere
   una stringa (riceve almeno una volta un indirizzo diverso da 0), oppure nessuna posizione dimostrata usa gia'
   l'indirizzo 0 come stringa
------------------------------------------------------------------------------------------------------------------*/
bool Decompiler::IsStringConst (int32_t value, string &s) const
{
	if (!amx.ReadString(value, s))
		return false;
	return value == 0 || (amx.GetDataCell(value - 4) & 0xFF) == 0;		// La cella precedente deve terminare con 0 (fine della stringa precedente)
}


string Decompiler::ArgText (ExprP a, const string &callee, int position)
{
	a = Use(a);
	if (a->isHeap)
		return a->heapValue ? a->heapValue->text : "_heap";
	if (a->isConst)
	{
		string s;
		bool valid = IsStringConst(a->value, s);
		StringVotes &votes = stringVotes[make_pair(callee, position)];
		if (collectVotes)
		{
			votes.valid += valid ? 1 : 0;
			votes.total++;
			votes.proven = votes.proven || (valid && a->value != 0);
			votes.zero = votes.zero || a->value == 0;
		}
		else if (valid && votes.valid == votes.total)
		{
			if (a->value != 0 || votes.proven)
				return Quote(s);
			bool zeroClaimed = false;			// L'indirizzo 0 e' gia' usato come stringa da una posizione dimostrata
			for (map <pair <string, int>, StringVotes>::const_iterator it = stringVotes.begin(); it != stringVotes.end() && !zeroClaimed; ++it)
				zeroClaimed = it->second.proven && it->second.zero && it->second.valid == it->second.total;
			if (!zeroClaimed)
				return Quote(s);
		}
	}
	return a->text;
}


void Decompiler::FlushReg (ExprP &r, int indent)
{
	if (r && r->sideEffect && !r->used)
	{
		r->used = true;
		AddLine(indent, r->text + ";");
	}
}


string Decompiler::FlushText (State &st)
{
	string text;
	ExprP regs[2] = {st.pri, st.alt};
	for (int k = 0; k < 2; k++)
		if (regs[k] && regs[k]->sideEffect && !regs[k]->used)
		{
			regs[k]->used = true;
			text += (text.empty() ? "" : " ") + regs[k]->text + ";";
		}
	return text;
}


ExprP Decompiler::PopValue (State &st, int indent)
{
	if (st.stack.empty())
		return Unknown();
	Slot &s = st.stack.back();
	ExprP v = s.val ? s.val : Unknown();
	if (s.cells > 1)
		s.cells--;
	else
		st.stack.pop_back();
	return v;
}


void Decompiler::PopCells (State &st, int cells, int indent)
{
	while (cells > 0 && !st.stack.empty())
	{
		Slot &s = st.stack.back();
		if (!s.declared && s.val && !s.val->used)	// Valore inserito e mai usato: era una variabile locale (es. new x = f();)
		{
			int cum = 0;
			for (unsigned int k = 0; k < st.stack.size(); k++)
				cum += st.stack[k].cells;
			DeclareSlot(st, (int)st.stack.size() - 1, -cum * 4);
		}
		if (s.cells <= cells)
		{
			cells -= s.cells;
			st.stack.pop_back();
		}
		else
		{
			s.cells -= cells;
			cells = 0;
		}
	}
}


vector <ExprP> Decompiler::PopArgs (State &st, int &nbytes, int indent)
{
	ExprP count = PopValue(st, indent);
	nbytes = count->isConst ? count->value : 0;
	vector <ExprP> args;
	for (int k = 0; k < nbytes / 4; k++)
		args.push_back(PopValue(st, indent));
	return args;
}


void Decompiler::Store (const string &target, ExprP value, int indent)
{
	value = Use(value);
	if (value->isHeap && value->heapValue)
		value = value->heapValue;
	// x = x + 1 -> x++,  x = x + y -> x += y
	string prefix = target + " + ";
	if (value->text == prefix + "1" && value->prec == P_ADD)
		AddLine(indent, target + "++;");
	else if (value->text == target + " - 1" && value->prec == P_ADD)
		AddLine(indent, target + "--;");
	else
		AddLine(indent, target + " = " + value->text + ";");
}


void Decompiler::StoreFrame (State &st, int32_t offset, ExprP value, int indent)
{
	if (offset < 0)
	{
		int cum = 0;
		for (unsigned int k = 0; k < st.stack.size(); k++)
		{
			int top = -cum * 4;
			cum += st.stack[k].cells;
			int base = -cum * 4;
			if (offset >= base && offset < top)
			{
				Slot &s = st.stack[k];
				if (!s.declared && !s.val && s.cells == 1)		// Dichiarazione con inizializzazione: stack -4 / stor.s.pri
				{
					s.declared = true;
					value = Use(value);
					if (value->isConst && value->value == 0)
						AddLine(indent, "new " + SlotName(base) + ";");
					else
						AddLine(indent, "new " + SlotName(base) + " = " + value->text + ";");
					return;
				}
				break;
			}
		}
	}
	Store(Lvalue(FrameAddress(st, offset)), value, indent);
}


void Decompiler::IncDec (State &st, const string &target, const string &op, int indent)
{
	if (st.pri && !st.pri->used && st.pri->var == target)		// Valore letto prima dell'incremento: x++ usato in un'espressione
	{
		ExprP e = Atom(target + op);
		e->sideEffect = true;
		st.pri = e;
		return;
	}
	AddLine(indent, target + op + ";");
}


ExprP Decompiler::JumpCond (State &st, int op)			// Condizione per cui il salto viene eseguito
{
	ExprP pri = st.pri ? st.pri : Unknown();
	ExprP alt = st.alt ? st.alt : Unknown();
	switch (op)
	{
	case OP_JZER:	return Not(pri);
	case OP_JNZ:	return Use(pri);
	case OP_JEQ:	return Cmp("==", alt, pri);
	case OP_JNEQ:	return Cmp("!=", alt, pri);
	case OP_JLESS: case OP_JSLESS:	return Cmp(">", alt, pri);		// pri < alt
	case OP_JLEQ: case OP_JSLEQ:	return Cmp(">=", alt, pri);		// pri <= alt
	case OP_JGRTR: case OP_JSGRTR:	return Cmp("<", alt, pri);		// pri > alt
	case OP_JGEQ: case OP_JSGEQ:	return Cmp("<=", alt, pri);		// pri >= alt
	}
	return Atom("true");
}


string Decompiler::BreakContinue (uint32_t target)		// "break", "continue", "switchexit" o "" se il salto non e' riconducibile al ciclo/switch corrente
{
	for (int k = (int)ctx.size() - 1; k >= 0; k--)
	{
		if (ctx[k].isSwitch)
		{
			if (target == ctx[k].exit)
				return "switchexit";
			continue;
		}
		if (target == ctx[k].exit)
			return "break";
		if (target == ctx[k].cont)
			return "continue";
		break;
	}
	return "";
}


void Decompiler::Block (int from, int to, State &st, int indent)
{
	int i = from;
	while (i < to)
	{
		uint32_t address = code[i].address;
		if (allTargets.count(address))
			st.terminated = false;
		else if (st.terminated)				// Codice irraggiungibile (es. jump dopo un return)
		{
			i++;
			continue;
		}
		if (emitLabels && gotoTargets.count(address) && !labelsEmitted.count(address))
		{
			FlushAll(st, indent);
			AddLine(indent, "L_" + Hex(address, 4) + ":");
			labelsEmitted.insert(address);
		}
		if (!activeHeads.count(i))			// Ricerca dell'ultimo salto all'indietro verso questa istruzione: testa di un ciclo
		{
			int j = -1;
			for (int k = to - 1; k >= i && j < 0; k--)
				if (IsJump(code[k].opcode) && !code[k].param.empty() && (uint32_t)code[k].param[0] == address)
					j = k;
			if (j >= 0)
			{
				i = Loop(i, j, st, indent);
				continue;
			}
		}
		i = Instruction(i, to, st, indent);
	}
}


int Decompiler::Loop (int i, int j, State &st, int indent)
{
	activeHeads.insert(i);
	FlushAll(st, indent);
	uint32_t exitA = AddrOf(j + 1);
	State body = st;
	body.pri = body.alt = nullptr;
	body.terminated = false;
	if (code[j].opcode == OP_JUMP)				// while
	{
		int header = AddLine(indent, "while (true) {");
		int bodyStart = (int)out.size();
		ctx.push_back({false, code[i].address, exitA});
		Block(i, j, body, indent + 1);
		FlushAll(body, indent + 1);
		ctx.pop_back();
		AddLine(indent, "}");
		int first = FirstActive(bodyStart);					// while (true) { if (x) break; ... }  ->  while (!x) { ... }
		if (first >= 0 && breakLines.count(first))
		{
			out[header].text = "while (" + Not(breakLines[first])->text + ") {";
			out[first].active = false;
		}
	}
	else										// do-while
	{
		AddLine(indent, "do {");
		ctx.push_back({false, 0xFFFFFFFF, exitA});
		Block(i, j, body, indent + 1);
		ctx.pop_back();
		ExprP cond = JumpCond(body, code[j].opcode);
		FlushAll(body, indent + 1);
		AddLine(indent, "} while (" + cond->text + ");");
	}
	activeHeads.erase(i);
	st.pri = st.alt = nullptr;
	return j + 1;
}


int Decompiler::BottomLoop (int i, int t, int k, State &st, int indent)		// jump T / body / T: condizione / salto condizionato a body
{
	activeHeads.insert(i + 1);
	FlushAll(st, indent);
	State cond = st;
	cond.pri = cond.alt = nullptr;
	int condStart = (int)out.size();
	Block(t, k, cond, indent);
	ExprP c = JumpCond(cond, code[k].opcode);
	bool condStatements = HasActive(condStart, (int)out.size());
	AddLine(indent, "while (" + c->text + ") {");
	ctx.push_back({false, code[t].address, AddrOf(k + 1)});
	State body = st;
	body.pri = body.alt = nullptr;
	body.terminated = false;
	Block(i + 1, t, body, indent + 1);
	FlushAll(body, indent + 1);
	if (condStatements)						// Le istruzioni della condizione vanno ripetute alla fine di ogni iterazione
	{
		State cond2 = st;
		Block(t, k, cond2, indent + 1);
	}
	ctx.pop_back();
	AddLine(indent, "}");
	activeHeads.erase(i + 1);
	st.pri = st.alt = nullptr;
	return k + 1;
}


/*------------------------------------------------------------------------------------------------------------------
Ciclo for:		jump T / I: incremento / T: condizione / salto condizionato a E / corpo / jump I / E:
La condizione e l'incremento vengono decompilati in righe temporanee che vengono poi rimosse e riscritte
nell'intestazione del for. Se la condizione contiene istruzioni o l'incremento contiene strutture di controllo
lo schema non viene applicato.
------------------------------------------------------------------------------------------------------------------*/
int Decompiler::TryFor (int i, int t, int to, State &st, int indent)
{
	uint32_t incrA = AddrOf(i + 1);
	int j = -1;
	for (int k = to - 1; k > t && j < 0; k--)
		if (code[k].opcode == OP_JUMP && (uint32_t)code[k].param[0] == incrA)
			j = k;
	if (j < 0)
		return -1;
	uint32_t exitA = AddrOf(j + 1);
	int k = -1;
	for (int m = t; m < j && k < 0; m++)
		if (IsCondJump(code[m].opcode) && (uint32_t)code[m].param[0] == exitA)
			k = m;
	int mark = (int)out.size();
	auto Discard = [&]() { out.resize(mark); breakLines.erase(breakLines.lower_bound(mark), breakLines.end()); };

	// Condizione
	string cond;
	int bodyStart = t;
	if (k >= 0)
	{
		State sc = st;
		sc.pri = sc.alt = nullptr;
		Block(t, k, sc, indent);
		ExprP J = JumpCond(sc, code[k].opcode);
		FlushAll(sc, indent);
		bool statements = HasActive(mark, (int)out.size());
		Discard();
		if (statements)
			return -1;
		cond = Not(J)->text;
		bodyStart = k + 1;
	}

	// Incremento
	State si = st;
	si.pri = si.alt = nullptr;
	Block(i + 1, t, si, indent);
	FlushAll(si, indent);
	string incr;
	for (int l = mark; l < (int)out.size(); l++)
		if (out[l].active)
		{
			const string &s = out[l].text;
			if (s.empty() || s.back() != ';' || s.find_first_of("{}") != string::npos)
			{
				Discard();
				return -1;
			}
			incr += (incr.empty() ? "" : ", ") + s.substr(0, s.size() - 1);
		}
	Discard();

	activeHeads.insert(i + 1);
	AddLine(indent, "for (; " + cond + "; " + incr + ") {");
	ctx.push_back({false, incrA, exitA});
	State body = st;
	body.pri = body.alt = nullptr;
	body.terminated = false;
	Block(bodyStart, j, body, indent + 1);
	FlushAll(body, indent + 1);
	ctx.pop_back();
	AddLine(indent, "}");
	activeHeads.erase(i + 1);
	st.pri = st.alt = nullptr;
	return j + 1;
}


int Decompiler::TryLogical (int i, int to, State &st, int indent)
{
	uint32_t F = code[i].param[0];
	int L = amx.FindInstruction(F);
	if (L <= i + 2 || L + 1 > to || L >= (int)code.size())
		return -1;
	int p = L - 1, q = L - 2;
	if (code[p].opcode != OP_JUMP || amx.FindInstruction(code[p].param[0]) != L + 1)
		return -1;
	auto IsOne = [&](int k) { return code[k].opcode == OP_CONST_PRI && code[k].param[0] == 1; };
	auto IsZero = [&](int k) { return code[k].opcode == OP_ZERO_PRI || (code[k].opcode == OP_CONST_PRI && code[k].param[0] == 0); };
	bool isAnd;
	if (IsOne(q) && IsZero(L))
		isAnd = true;
	else if (IsZero(q) && IsOne(L))
		isAnd = false;
	else
		return -1;
	vector <int> splits(1, i);
	for (int m = i + 1; m < q; m++)
		if (IsCondJump(code[m].opcode) && (uint32_t)code[m].param[0] == F)
			splits.push_back(m);
	if (splits.back() != q - 1)
		return -1;

	vector <ExprP> operands;
	ExprP J = JumpCond(st, code[i].opcode);
	operands.push_back(isAnd ? Not(J) : J);
	for (unsigned int k = 1; k < splits.size(); k++)
	{
		State s = st;
		s.pri = s.alt = nullptr;
		Block(splits[k - 1] + 1, splits[k], s, indent);
		J = JumpCond(s, code[splits[k]].opcode);
		operands.push_back(isAnd ? Not(J) : J);
	}
	st.pri = Logic(isAnd ? "&&" : "||", operands);
	st.alt = nullptr;
	return L + 1;
}


int Decompiler::CondJump (int i, int to, State &st, int indent)
{
	int r = TryLogical(i, to, st, indent);
	if (r >= 0)
		return r;
	uint32_t T = code[i].param[0];
	ExprP J = JumpCond(st, code[i].opcode);
	FlushAll(st, indent);
	string bc = BreakContinue(T);
	if (bc == "break" || bc == "continue")
	{
		int line = AddLine(indent, "if (" + J->text + ") " + bc + ";");
		if (bc == "break")
			breakLines[line] = J;
		st.pri = st.alt = nullptr;
		return i + 1;
	}
	int t = amx.FindInstruction(T);
	if (T > code[i].address && t >= 0 && t <= to)
	{
		ExprP c = Not(J);
		int p = t - 1;
		if (p > i && code[p].opcode == OP_JUMP)
		{
			uint32_t E = code[p].param[0];
			int e = amx.FindInstruction(E);
			if (E > T && e >= 0 && e <= to && BreakContinue(E).empty())			// if-else
			{
				int header = AddLine(indent, "if (" + c->text + ") {");
				State s1 = st;
				s1.terminated = false;
				int start1 = (int)out.size();
				Block(i + 1, p, s1, indent + 1);
				int ph1 = AddPlaceholder(indent + 1);
				int middle = AddLine(indent, "} else {");
				State s2 = st;
				s2.terminated = false;
				int start2 = (int)out.size();
				Block(t, e, s2, indent + 1);
				int ph2 = AddPlaceholder(indent + 1);
				int end = AddLine(indent, "}");
				if (!HasActive(start1, ph1) && !HasActive(start2, ph2) && s1.pri && s2.pri && s1.stack.size() == st.stack.size() && s2.stack.size() == st.stack.size()
					&& !s1.terminated && !s2.terminated)
				{
					out[header].active = out[middle].active = out[end].active = false;		// Nessuna istruzione nei due rami: operatore ternario
					st.pri = Ternary(c, s1.pri, s2.pri);
					st.alt = nullptr;
					return e;
				}
				string f1 = FlushText(s1), f2 = FlushText(s2);
				if (!f1.empty())
					SetLine(ph1, f1);
				if (!f2.empty())
					SetLine(ph2, f2);
				if (!HasActive(start2, end))		// Ramo else vuoto
					out[middle].text = "}", out[end].active = false;
				else								// else { if (...) { ... } }  ->  else if (...) { ... }
				{
					int first = FirstActive(start2), last = -1;
					for (int l = end - 1; l > first && last < 0; l--)
						if (out[l].active)
							last = l;
					bool chain = first >= 0 && last > first && out[first].indent == indent + 1 && out[first].text.compare(0, 4, "if (") == 0 &&
						out[last].indent == indent + 1 && out[last].text == "}";
					for (int l = first + 1; chain && l < last; l++)
						if (out[l].active && out[l].indent <= indent + 1 && out[l].text.compare(0, 6, "} else") != 0)
							chain = false;
					if (chain)
					{
						out[middle].text = "} else " + out[first].text;
						out[first].active = false;
						for (int l = first + 1; l <= last; l++)
							out[l].indent--;
						out[last].active = false;
					}
				}
				st.stack = s1.terminated ? s2.stack : s1.stack;
				st.terminated = s1.terminated && s2.terminated;
				st.pri = st.alt = nullptr;
				return e;
			}
		}
		AddLine(indent, "if (" + c->text + ") {");								// if
		State s1 = st;
		s1.terminated = false;
		Block(i + 1, t, s1, indent + 1);
		FlushAll(s1, indent + 1);
		AddLine(indent, "}");
		if (!s1.terminated)
			st.stack = s1.stack;
		st.pri = st.alt = nullptr;
		return t;
	}
	AddLine(indent, "if (" + J->text + ") goto L_" + Hex(T, 4) + ";");
	newGotoTargets.insert(T);
	st.pri = st.alt = nullptr;
	return i + 1;
}


int Decompiler::Jump (int i, int to, State &st, int indent)
{
	uint32_t T = code[i].param[0];
	FlushAll(st, indent);
	int t = amx.FindInstruction(T);
	if (t == i + 1)													// Salto all'istruzione successiva (es. else vuoto)
		return i + 1;
	if (T > code[i].address && t > i + 1 && t < to)					// while con il test in fondo
		for (int k = to - 1; k >= t; k--)
			if (IsCondJump(code[k].opcode) && (uint32_t)code[k].param[0] == AddrOf(i + 1))
				return BottomLoop(i, t, k, st, indent);
	if (T > code[i].address && t > i + 1 && t <= to)				// Salto sopra codice irraggiungibile (nessuna istruzione intermedia e' destinazione di un salto)
	{
		bool reachable = false;
		for (int k = i + 1; k < t && !reachable; k++)
			reachable = allTargets.count(code[k].address) > 0;
		if (!reachable)
		{
			AddLine(indent, "// unreachable code removed (" + Hex(code[i + 1].address, 4) + "-" + Hex(code[t - 1].address, 4) + ")");
			return t;
		}
	}
	if (T > code[i].address && t > i + 1 && t < to)					// for
	{
		int r = TryFor(i, t, to, st, indent);
		if (r >= 0)
			return r;
	}
	string bc = BreakContinue(T);
	if (bc == "break" || bc == "continue")
		AddLine(indent, bc + ";");
	else if (bc.empty())
	{
		AddLine(indent, "goto L_" + Hex(T, 4) + ";");
		newGotoTargets.insert(T);
	}
	st.terminated = true;
	return i + 1;
}


int Decompiler::Switch (int i, int to, State &st, int indent)
{
	int k = amx.FindInstruction(code[i].param[0]);
	if (k <= i || k >= to || code[k].opcode != OP_CASETBL || code[k].param.size() < 2)
	{
		AddLine(indent, "/* unsupported switch */ goto L_" + Hex(code[i].param[0], 4) + ";");
		newGotoTargets.insert(code[i].param[0]);
		return i + 1;
	}
	ExprP v = Use(st.pri);
	FlushAll(st, indent);
	const vector <int32_t> &P = code[k].param;
	uint32_t def = P[1];
	uint32_t exitA = AddrOf(k + 1);
	map <uint32_t, vector <int32_t>> cases;
	set <uint32_t> starts;
	for (unsigned int p = 2; p + 1 < P.size(); p += 2)
	{
		cases[P[p + 1]].push_back(P[p]);
		starts.insert(P[p + 1]);
	}
	if (def != exitA)
		starts.insert(def);

	AddLine(indent, "switch (" + v->text + ") {");
	ctx.push_back({true, 0xFFFFFFFF, exitA});
	for (set <uint32_t>::iterator it = starts.begin(); it != starts.end(); ++it)
	{
		set <uint32_t>::iterator next = it;
		++next;
		uint32_t end = (next == starts.end()) ? code[k].address : *next;
		int a = amx.FindInstruction(*it), b = amx.FindInstruction(end);
		if (a < 0 || b < 0 || a > b || b > k)
			continue;
		if (cases.count(*it))
		{
			string label = "case ";
			for (unsigned int c = 0; c < cases[*it].size(); c++)
				label += (c ? ", " : "") + RenderValue(cases[*it][c]);
			AddLine(indent + 1, label + ":");
		}
		if (*it == def)
			AddLine(indent + 1, "default:");
		out.back().text += " {";
		State sc = st;
		sc.pri = sc.alt = nullptr;
		sc.terminated = false;
		Block(a, b, sc, indent + 2);
		FlushAll(sc, indent + 2);
		AddLine(indent + 1, "}");
	}
	ctx.pop_back();
	AddLine(indent, "}");
	st.pri = st.alt = nullptr;
	return k + 1;
}


int Decompiler::Instruction (int i, int to, State &st, int indent)
{
	const AMX_INSTRUCTION &ins = code[i];
	int op = ins.opcode;
	int32_t p0 = ins.param.empty() ? 0 : ins.param[0];
	if (IsCondJump(op))
		return CondJump(i, to, st, indent);
	switch (op)
	{
	case OP_JUMP:		return Jump(i, to, st, indent);
	case OP_SWITCH:		return Switch(i, to, st, indent);

	// Letture
	case OP_LOAD_PRI: case OP_LREF_PRI:		SetPri(st, Var(GlobalName(p0)), indent); break;
	case OP_LOAD_ALT: case OP_LREF_ALT:		SetAlt(st, Var(GlobalName(p0)), indent); break;
	case OP_LOAD_S_PRI:		SetPri(st, Var(Lvalue(FrameAddress(st, p0))), indent); break;
	case OP_LOAD_S_ALT:		SetAlt(st, Var(Lvalue(FrameAddress(st, p0))), indent); break;
	case OP_LREF_S_PRI:		if (p0 >= 12) refArgs.insert((p0 - 12) / 4); SetPri(st, Var(Lvalue(FrameAddress(st, p0))), indent); break;
	case OP_LREF_S_ALT:		if (p0 >= 12) refArgs.insert((p0 - 12) / 4); SetAlt(st, Var(Lvalue(FrameAddress(st, p0))), indent); break;
	case OP_LOAD_I: case OP_LODB_I:			{ ExprP a = st.pri; SetPri(st, Var(Lvalue(a)), indent); break; }
	case OP_CONST_PRI:		SetPri(st, Const(p0), indent); break;
	case OP_CONST_ALT:		SetAlt(st, Const(p0), indent); break;
	case OP_ADDR_PRI:		SetPri(st, FrameAddress(st, p0), indent); break;
	case OP_ADDR_ALT:		SetAlt(st, FrameAddress(st, p0), indent); break;
	case OP_LIDX: case OP_LIDX_B:			{ ExprP a = Indexed(st.alt, st.pri); SetPri(st, Var(Lvalue(a)), indent); break; }
	case OP_IDXADDR: case OP_IDXADDR_B:		{ ExprP a = Indexed(st.alt, st.pri); SetPri(st, a, indent); break; }
	case OP_LCTRL:			SetPri(st, Atom("_lctrl(" + to_string(p0) + ")"), indent); break;

	// Scritture
	case OP_STOR_PRI: case OP_SREF_PRI:		Store(GlobalName(p0), st.pri, indent); break;
	case OP_STOR_ALT: case OP_SREF_ALT:		Store(GlobalName(p0), st.alt, indent); break;
	case OP_STOR_S_PRI:		StoreFrame(st, p0, st.pri, indent); break;
	case OP_STOR_S_ALT:		StoreFrame(st, p0, st.alt, indent); break;
	case OP_SREF_S_PRI:		if (p0 >= 12) refArgs.insert((p0 - 12) / 4); StoreFrame(st, p0, st.pri, indent); break;
	case OP_SREF_S_ALT:		if (p0 >= 12) refArgs.insert((p0 - 12) / 4); StoreFrame(st, p0, st.alt, indent); break;
	case OP_STOR_I: case OP_STRB_I:
		if (st.alt && st.alt->isHeap)
			st.alt->heapValue = Use(st.pri);
		else
			Store(Lvalue(st.alt), st.pri, indent);
		break;
	case OP_ZERO:			Store(GlobalName(p0), Const(0), indent); break;
	case OP_ZERO_S:			StoreFrame(st, p0, Const(0), indent); break;
	case OP_SCTRL:			AddLine(indent, "_sctrl(" + to_string(p0) + ", " + Use(st.pri)->text + ");"); break;
	case OP_MOVS:			AddLine(indent, ArrayRef(st.alt, p0 / 4) + " = " + ArrayRef(st.pri, p0 / 4) + ";"); break;
	case OP_FILL:
		if (!st.pri || !st.pri->isConst || st.pri->value != 0)
			AddLine(indent, "_fill(" + Use(st.alt)->text + ", " + Use(st.pri)->text + ", " + to_string(p0 / 4) + ");");
		break;

	// Registri e stack
	case OP_MOVE_PRI:		SetPri(st, st.alt, indent); break;
	case OP_MOVE_ALT:		SetAlt(st, st.pri, indent); break;
	case OP_XCHG:			swap(st.pri, st.alt); break;
	case OP_PUSH_PRI:		Push(st, Use(st.pri), indent); break;
	case OP_PUSH_ALT:		Push(st, Use(st.alt), indent); break;
	case OP_PUSH_R:			for (int k = 0; k < p0; k++) Push(st, Use(st.pri), indent); break;
	case OP_PUSH_C:			Push(st, Const(p0), indent); break;
	case OP_PUSH:			Push(st, Var(GlobalName(p0)), indent); break;
	case OP_PUSH_S:			Push(st, Var(Lvalue(FrameAddress(st, p0))), indent); break;
	case OP_PUSHADDR:		Push(st, FrameAddress(st, p0), indent); break;
	case OP_POP_PRI:		SetPri(st, PopValue(st, indent), indent); break;
	case OP_POP_ALT:		SetAlt(st, PopValue(st, indent), indent); break;
	case OP_SWAP_PRI:		if (!st.stack.empty()) swap(st.stack.back().val, st.pri); break;
	case OP_SWAP_ALT:		if (!st.stack.empty()) swap(st.stack.back().val, st.alt); break;
	case OP_STACK:
		if (p0 < 0)
		{
			Slot s;
			s.cells = -p0 / 4;
			s.line = AddPlaceholder(indent);
			st.stack.push_back(s);
			if (s.cells > 1)						// Array locale: dichiarazione immediata
			{
				int cum = 0;
				for (unsigned int k = 0; k < st.stack.size(); k++)
					cum += st.stack[k].cells;
				DeclareSlot(st, (int)st.stack.size() - 1, -cum * 4);
			}
		}
		else
		{
			int n = p0;
			int c = min(n, st.credit);
			n -= c;
			st.credit -= c;
			PopCells(st, n / 4, indent);
		}
		break;
	case OP_HEAP:			{ ExprP h = Atom("_heap"); h->isHeap = true; SetAlt(st, h, indent); break; }

	// Chiamate
	case OP_CALL:
	{
		int nbytes;
		vector <ExprP> args = PopArgs(st, nbytes, indent);
		funcArgCount[p0] = max(funcArgCount[p0], (int)args.size());
		string text = (funcNames.count(p0) ? funcNames[p0] : "func_" + Hex(p0, 4)) + "(";
		for (unsigned int k = 0; k < args.size(); k++)
			text += (k ? ", " : "") + ArgText(args[k], "F" + to_string(p0), k);
		ExprP e = Atom(text + ")");
		e->sideEffect = true;
		SetPri(st, e, indent);
		break;
	}
	case OP_SYSREQ_C: case OP_SYSREQ_D:
	{
		int nbytes;
		vector <ExprP> args = PopArgs(st, nbytes, indent);
		st.credit += nbytes + 4;
		string name = (p0 >= 0 && (unsigned int)p0 < amx.natives.size()) ? amx.natives[p0].name : "native_" + to_string(p0);
		nativeArgCount[p0] = max(nativeArgCount[p0], (int)args.size());
		ExprP e;
		if (args.size() == 2 && (name == "floatadd" || name == "floatsub"))
			e = Bin(name == "floatadd" ? "+" : "-", P_ADD, args[0], args[1]);
		else if (args.size() == 2 && (name == "floatmul" || name == "floatdiv"))
			e = Bin(name == "floatmul" ? "*" : "/", P_MUL, args[0], args[1]);
		else
		{
			string text = name + "(";
			for (unsigned int k = 0; k < args.size(); k++)
				text += (k ? ", " : "") + ArgText(args[k], "N" + to_string(p0), k);
			e = Atom(text + ")");
			if (name == "floatcmp" && args.size() == 2)
			{
				e->fcmpA = args[0];
				e->fcmpB = args[1];
			}
			else
				e->sideEffect = true;
		}
		SetPri(st, e, indent);
		break;
	}
	case OP_SYSREQ_PRI:		{ ExprP e = Atom("_sysreq(" + Use(st.pri)->text + ")"); e->sideEffect = true; SetPri(st, e, indent); break; }
	case OP_CALL_PRI:		{ ExprP e = Atom("_call(" + Use(st.pri)->text + ")"); e->sideEffect = true; SetPri(st, e, indent); break; }
	case OP_RET: case OP_RETN:
	{
		FlushReg(st.alt, indent);
		bool last = true;
		for (int k = i + 1; k < funcEnd; k++)
			if (code[k].opcode != OP_NOP)
				last = false;
		if (!st.pri)
			AddLine(indent, "return;");
		else if (!(last && st.pri->isConst && st.pri->value == 0))
			AddLine(indent, "return " + Use(st.pri)->text + ";");
		st.terminated = true;
		break;
	}
	case OP_HALT:			AddLine(indent, "_halt(" + to_string(p0) + ");"); st.terminated = true; break;
	case OP_JREL: case OP_JUMP_PRI:	AddLine(indent, "/* " + string(AMX_Opcodes[op].name) + " */"); break;

	// Aritmetica
	case OP_SHL:			SetPri(st, Bin("<<", P_SHIFT, st.pri, st.alt), indent); break;
	case OP_SHR:			SetPri(st, Bin(">>>", P_SHIFT, st.pri, st.alt), indent); break;
	case OP_SSHR:			SetPri(st, Bin(">>", P_SHIFT, st.pri, st.alt), indent); break;
	case OP_SHL_C_PRI:		SetPri(st, Bin("<<", P_SHIFT, st.pri, Const(p0)), indent); break;
	case OP_SHL_C_ALT:		SetAlt(st, Bin("<<", P_SHIFT, st.alt, Const(p0)), indent); break;
	case OP_SHR_C_PRI:		SetPri(st, Bin(">>>", P_SHIFT, st.pri, Const(p0)), indent); break;
	case OP_SHR_C_ALT:		SetAlt(st, Bin(">>>", P_SHIFT, st.alt, Const(p0)), indent); break;
	case OP_SMUL: case OP_UMUL:		SetPri(st, Bin("*", P_MUL, st.alt, st.pri), indent); break;
	case OP_SDIV: case OP_UDIV:		{ ExprP a = st.pri, b = st.alt; st.pri = Bin("/", P_MUL, a, b); st.alt = Bin("%", P_MUL, a, b); break; }
	case OP_SDIV_ALT: case OP_UDIV_ALT:	{ ExprP a = st.alt, b = st.pri; st.pri = Bin("/", P_MUL, a, b); st.alt = Bin("%", P_MUL, a, b); break; }
	case OP_ADD:
	{
		// Array a 2 dimensioni: indirizzo di arr[i] + valore di arr[i] (offset della riga) = indirizzo della riga arr[i]
		ExprP a = st.alt, v = st.pri;
		if (a && v && !a->isAddress && v->isAddress)
			swap(a, v);
		if (a && v && a->isAddress && !v->var.empty() && v->var == Lvalue(a))
		{
			Use(v);
			SetPri(st, MakeAddress(Lvalue(a), true, nullptr, 0), indent);
		}
		else
			SetPri(st, Bin("+", P_ADD, st.alt, st.pri), indent);
		break;
	}
	case OP_SUB:			SetPri(st, Bin("-", P_ADD, st.pri, st.alt), indent); break;
	case OP_SUB_ALT:		SetPri(st, Bin("-", P_ADD, st.alt, st.pri), indent); break;
	case OP_AND:			SetPri(st, Bin("&", P_BAND, st.alt, st.pri), indent); break;
	case OP_OR:				SetPri(st, Bin("|", P_BOR, st.alt, st.pri), indent); break;
	case OP_XOR:			SetPri(st, Bin("^", P_BXOR, st.alt, st.pri), indent); break;
	case OP_NOT:			SetPri(st, Not(st.pri), indent); break;
	case OP_NEG:			SetPri(st, Unary("-", st.pri), indent); break;
	case OP_INVERT:			SetPri(st, Unary("~", st.pri), indent); break;
	case OP_ADD_C:
		if (st.pri && st.pri->isAddress && !st.pri->index && p0 % 4 == 0)		// Indirizzo + costante: elemento successivo dell'array
		{
			ExprP a = Use(st.pri);
			SetPri(st, MakeAddress(a->base, true, nullptr, a->offset + p0 / 4), indent);
		}
		else if (st.pri && st.pri->isConst)		// Costante + costante: se la prima e' un indirizzo nel blocco dati il risultato e' un elemento dello stesso array
		{
			ExprP a = Use(st.pri);
			ExprP e = Const(a->value + p0);
			e->constBase = a->constBase >= 0 ? a->constBase : (amx.IsDataAddress(a->value) ? a->value : -1);
			SetPri(st, e, indent);
		}
		else if (p0 < 0)
			SetPri(st, Bin("-", P_ADD, st.pri, Const(-p0)), indent);
		else
			SetPri(st, Bin("+", P_ADD, st.pri, Const(p0)), indent);
		break;
	case OP_SMUL_C:			SetPri(st, Bin("*", P_MUL, st.pri, Const(p0)), indent); break;
	case OP_ZERO_PRI:		SetPri(st, Const(0), indent); break;
	case OP_ZERO_ALT:		SetAlt(st, Const(0), indent); break;
	case OP_EQ:				SetPri(st, Cmp("==", st.alt, st.pri), indent); break;
	case OP_NEQ:			SetPri(st, Cmp("!=", st.alt, st.pri), indent); break;
	case OP_LESS: case OP_SLESS:	SetPri(st, Cmp(">", st.alt, st.pri), indent); break;		// pri < alt
	case OP_LEQ: case OP_SLEQ:		SetPri(st, Cmp(">=", st.alt, st.pri), indent); break;
	case OP_GRTR: case OP_SGRTR:	SetPri(st, Cmp("<", st.alt, st.pri), indent); break;
	case OP_GEQ: case OP_SGEQ:		SetPri(st, Cmp("<=", st.alt, st.pri), indent); break;
	case OP_EQ_C_PRI:		SetPri(st, Cmp("==", st.pri, Const(p0)), indent); break;
	case OP_EQ_C_ALT:		SetPri(st, Cmp("==", st.alt, Const(p0)), indent); break;
	case OP_INC_PRI:		SetPri(st, Bin("+", P_ADD, st.pri, Const(1)), indent); break;
	case OP_INC_ALT:		SetAlt(st, Bin("+", P_ADD, st.alt, Const(1)), indent); break;
	case OP_DEC_PRI:		SetPri(st, Bin("-", P_ADD, st.pri, Const(1)), indent); break;
	case OP_DEC_ALT:		SetAlt(st, Bin("-", P_ADD, st.alt, Const(1)), indent); break;
	case OP_INC:			IncDec(st, GlobalName(p0), "++", indent); break;
	case OP_DEC:			IncDec(st, GlobalName(p0), "--", indent); break;
	case OP_INC_S:			IncDec(st, Lvalue(FrameAddress(st, p0)), "++", indent); break;
	case OP_DEC_S:			IncDec(st, Lvalue(FrameAddress(st, p0)), "--", indent); break;
	case OP_INC_I:			{ string t = Lvalue(st.pri); st.pri = nullptr; IncDec(st, t, "++", indent); break; }
	case OP_DEC_I:			{ string t = Lvalue(st.pri); st.pri = nullptr; IncDec(st, t, "--", indent); break; }
	case OP_CMPS:			SetPri(st, Atom("_cmps(" + Use(st.alt)->text + ", " + Use(st.pri)->text + ", " + to_string(p0) + ")"), indent); break;

	// Istruzioni senza effetto sul codice decompilato
	case OP_PROC: case OP_NOP: case OP_ALIGN_PRI: case OP_ALIGN_ALT: case OP_BOUNDS: case OP_SIGN_PRI: case OP_SIGN_ALT:
	case OP_FILE: case OP_LINE: case OP_SYMBOL: case OP_SRANGE: case OP_SYMTAG: case OP_BREAK: case OP_CASETBL:
		break;

	default:
		AddLine(indent, "/* unknown opcode " + to_string(op) + " */");
		break;
	}
	return i + 1;
}


void Decompiler::DecompileFunction (int first, int end, Function &f)
{
	funcEnd = end;
	gotoTargets.clear();
	for (int pass = 0; pass < 2; pass++)		// Primo passaggio: raccolta delle destinazioni dei goto. Secondo passaggio: output con le etichette
	{
		emitLabels = (pass == 1);
		out.clear();
		ctx.clear();
		activeHeads.clear();
		breakLines.clear();
		labelsEmitted.clear();
		newGotoTargets.clear();
		maxArg = -1;
		refArgs.clear();
		State st;
		Block(first + 1, end, st, 1);
		FlushAll(st, 1);
		gotoTargets = newGotoTargets;
	}
	f.maxArg = maxArg;
	f.refArgs = refArgs;
	f.lines = out;
}


bool Decompiler::Run (string filename)
{
	funcNames = AMX_FunctionNames(amx);
	usedNames.clear();
	for (unsigned int i = 0; i < amx.publics.size(); i++)
		publicAddresses.insert(amx.publics[i].address);
	for (unsigned int i = 0; i < code.size(); i++)
	{
		const AMX_INSTRUCTION &ins = code[i];
		if ((IsJump(ins.opcode) || ins.opcode == OP_SWITCH) && !ins.param.empty())
			allTargets.insert(ins.param[0]);
		if (ins.opcode == OP_CASETBL && ins.param.size() >= 2)
		{
			allTargets.insert(ins.param[1]);
			for (unsigned int p = 2; p + 1 < ins.param.size(); p += 2)
				allTargets.insert(ins.param[p + 1]);
		}
	}

	// Decompilazione delle funzioni. Il primo passaggio serve solo a raccogliere i voti sugli argomenti di tipo stringa
	vector <Function> functions;
	for (int pass = 0; pass < 2; pass++)
	{
		collectVotes = (pass == 0);
		functions.clear();
		for (unsigned int i = 0; i < code.size(); i++)
		{
			if (code[i].opcode != OP_PROC)
				continue;
			unsigned int end = i + 1;
			while (end < code.size() && code[end].opcode != OP_PROC)
				end++;
			Function f;
			f.address = code[i].address;
			f.name = funcNames.count(f.address) ? funcNames[f.address] : "func_" + Hex(f.address, 4);
			f.isPublic = publicAddresses.count(f.address) > 0;
			DecompileFunction(i, end, f);
			functions.push_back(f);
		}
	}

	ofstream file(filename);
	if (!file.is_open())
	{
		msg(msg::TGT::FILE_CONS, msg::TYP::ERR) << "Unable to create " << filename;
		return false;
	}
	file << "// " << filename << "\n";
	file << "// Pseudo-code decompiled by TRAODLE from a Small 2.x AMX script. Variable names are generated:\n";
	file << "//   g_XXXX = global variable at data address XXXX, argN = function argument, varN = local variable,\n";
	file << "//   _xxx = constructs with no direct Pawn equivalent.\n";
	file << "// Arguments are listed in call order; the native prototypes below only show the number of arguments used.\n\n";

	if (!amx.natives.empty())
	{
		for (unsigned int i = 0; i < amx.natives.size(); i++)
		{
			file << "native " << amx.natives[i].name << "(";
			int n = nativeArgCount.count(i) ? nativeArgCount[i] : 0;
			for (int k = 0; k < n; k++)
				file << (k ? ", " : "") << "a" << k;
			file << ");\n";
		}
		file << "\n";
	}

	// Variabili globali (preparate prima della scrittura: anche i valori iniziali possono usare costanti con nome)
	stringstream gl;
	if (!globals.empty())
	{
		for (set <int32_t>::iterator it = globals.begin(); it != globals.end(); ++it)
		{
			string s;
			set <int32_t>::iterator next = it;
			++next;
			int cells = 1;							// Dimensione: da movs, altrimenti per gli array fino alla variabile successiva
			if (globalSizes.count(*it))
				cells = globalSizes[*it];
			else if (globalArrays.count(*it))
				cells = ((next == globals.end() ? (int32_t)amx.data_size : *next) - *it) / 4;
			if (amx.ReadString(*it, s))
				gl << "new g_" << Hex(*it, 4) << "[] = " << Quote(s) << ";\n";
			else if (cells > 1)
			{
				gl << "new g_" << Hex(*it, 4) << "[" << cells << "] = {";
				for (int c = 0; c < cells; c++)
					gl << (c ? "," : "") << (c % 8 == 0 ? "\n\t" : " ") << RenderValue(amx.GetDataCell(*it + c * 4));
				gl << "\n};\n";
			}
			else
				gl << "new g_" << Hex(*it, 4) << " = " << RenderValue(amx.GetDataCell(*it)) << ";\n";
		}
		gl << "\n";
	}

	// Costanti con nome: hash di animazioni, bones, personaggi e file del livello
	if (!usedNames.empty())
	{
		file << "// Hashed names (GetHashValue) found in the level files\n";
		for (uint32_t h : usedNames)
			file << "const " << AMX_HashName(h) << " = 0x" << Hex(h, 8) << ";\n";
		file << "\n";
	}

	file << gl.str();

	for (unsigned int i = 0; i < functions.size(); i++)
	{
		const Function &f = functions[i];
		int nArgs = max(f.maxArg + 1, funcArgCount.count(f.address) ? funcArgCount[f.address] : 0);
		file << "\n" << (f.isPublic ? "public " : "") << f.name << "(";
		for (int k = 0; k < nArgs; k++)
			file << (k ? ", " : "") << (f.refArgs.count(k) ? "&" : "") << "arg" << k;
		file << ")\n{\n";
		for (unsigned int l = 0; l < f.lines.size(); l++)
			if (f.lines[l].active)
				file << string(f.lines[l].indent, '\t') << f.lines[l].text << "\n";
		file << "}\n";
	}
	return true;
}

}		// namespace


bool AMX_Decompile (const AMX_SCRIPT &amx, string filename)
{
	Decompiler decompiler(amx);
	return decompiler.Run(filename);
}
