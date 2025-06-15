/*------------------------------------------------------------------------------------------------------------------
Generatore ID per nodi del file FBX. Effettua l'hash della stringa in ingresso e restituisce un numero lungo 8 byte
INPUT: string input, string type
OUTPUT: string
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"


string hashID (string name, string type)
{
	if (name.empty())
		return name;
	string hash;
	unsigned long long temp1;
	stringstream output;
	type.append(name);
	hash = picosha2::hash256_hex_string(type);
	hash = hash.substr(0,14);						// La stringa viene accorciata a 7 bytes
	temp1 = stoull(hash, nullptr, 16);
	output << temp1;
	return output.str();
}
