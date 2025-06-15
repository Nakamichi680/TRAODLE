#pragma once
#include "concol.h"


class msg {
public:
	enum class TGT {FILE, CONS, FILE_CONS};
	enum class TYP {OVR, LOG, DBG, WARN, ERR, FATAL};
	msg (const char* function_caller, TGT output_target, TYP message_type) : first(true), caller(function_caller), output(output_target), type(message_type) {}
	template <typename T> msg& operator<<(const T &text)
	{
		if (!first)									// Questo blocco viene eseguito solo per i dati che seguono il secondo "<<"
		{
			if (output == TGT::FILE || output == TGT::FILE_CONS || type == TYP::FATAL)
				msg_file_stream << text;
			if (output == TGT::CONS || output == TGT::FILE_CONS || type == TYP::FATAL)
				cout << text;
			return *this;
		}
		else
			first = false;

		// Preparazione stringa "[NOMECHIAMANTE]"
		string aligned_caller = " [";
		aligned_caller.append(caller);
		aligned_caller.append("] : ");

		// Preparazione stringa "HH:MM:SS"
		time_t rawtime;
		struct tm * timeinfo;
		char timestring[12];
		time(&rawtime);
		timeinfo = localtime (&rawtime);
		strftime (timestring, 12, "%T   ", timeinfo);

		// Output messaggio
		switch(type)
		{
		case TYP::OVR:												// L'utente ha il pieno controllo sulla stringa mostrata a schermo e scritta su file
			if (output == TGT::FILE || output == TGT::FILE_CONS)
				msg_file_stream << text;
			if (output == TGT::CONS || output == TGT::FILE_CONS)
				cout << text;
			break;

		case TYP::LOG:												// Info di logging. Ora e nome funzione chiamante sono riportate solo nel file di testo
			if (output == TGT::FILE || output == TGT::FILE_CONS)
			msg_file_stream << endl << timestring << "LOG --------" << right << setw(37) << setfill('-') << aligned_caller << setfill(' ') << text;
			if (output == TGT::CONS || output == TGT::FILE_CONS)
				cout << endl << text;
			break;

		case TYP::DBG:												// Info di debugging. L'ora è riportata solo nel file di testo. Colore del testo: celeste
			if (output == TGT::FILE || output == TGT::FILE_CONS)
			msg_file_stream << endl << timestring << "DEBUG ------" << right << setw(37) << setfill('-') << aligned_caller << setfill(' ') << text;
			if (output == TGT::CONS || output == TGT::FILE_CONS)
				cout << endl << aqua << "DEBUG" << dark_white << " : " << text;
			break;

		case TYP::WARN:												// Avvisi generici. L'ora è riportata solo nel file di testo. Colore del testo: giallo
			if (output == TGT::FILE || output == TGT::FILE_CONS)
			msg_file_stream << endl << timestring << "WARNING ----" << right << setw(37) << setfill('-') << aligned_caller << setfill(' ') << text;
			if (output == TGT::CONS || output == TGT::FILE_CONS)
				cout << endl << yellow << "WARNING" << dark_white << " : " << text;
			break;

		case TYP::ERR:												// Messaggi di errore. L'ora è riportata solo nel file di testo. Colore del testo: viola
			if (output == TGT::FILE || output == TGT::FILE_CONS)
			msg_file_stream << endl << timestring << "ERROR ------" << right << setw(37) << setfill('-') << aligned_caller << setfill(' ') << text;
			if (output == TGT::CONS || output == TGT::FILE_CONS)
				cout << endl << purple << "ERROR" << dark_white << " : " << text;
			break;

		case TYP::FATAL:											// Errori fatali, il programma termina. L'ora è riportata solo nel file di testo. Colore del testo: rosso
			msg_file_stream << endl << timestring << "FATAL ERROR " << right << setw(37) << setfill('-') << aligned_caller << setfill(' ') << text;
			cout << endl << red << "FATAL ERROR" << dark_white << " : " << text;
		}
		return *this;
	}
	typedef ostream& (*STRFUNC)(ostream&);
	msg& operator<< (STRFUNC func)
	{
		func(cout);
		func(msg_file_stream);
		return *this;
	}
private:
	bool first;
	const char* caller;
	TGT output = TGT::FILE;
	TYP type = TYP::LOG;
};


#define msg(output_target, message_type) msg(__FUNCTION__, output_target, message_type)


void msgInit ();		// Inizializza l'interfaccia di output su console e su file