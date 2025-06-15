#include "stdafx.h"
#include "Classes.h"
#include "TRAOD/CLZ/CLZ_struct.h"
#include "ZLIB/zlib.h"


bool Decompress_CLZ ()
{
	CLZ_HEADER clz_header;

	SetCurrentDirectory(AOD_IO.folder_clzgmx_lpwstr);												// Imposta la cartella in cui si trova il file CLZ
	ifstream clz(AOD_IO.file_clzgmx, std::ios::binary);

	if (!clz.is_open())							// Se il file non viene aperto la funzione termina
	{
		msg(msg::TGT::CONS, msg::TYP::FATAL) << AOD_IO.file_clzgmx << " not found.";
		return false;
	}

	unsigned int compressed_size, decompressed_size;
	clz.seekg(0, clz.end);
	compressed_size = (unsigned int)clz.tellg() - 4;												// Calcolo dimensione dati compressi
	clz.seekg(0);
	clz.read(reinterpret_cast<char*>(&decompressed_size), sizeof(clz_header.decompressed_size));	// Lettura dimensione file di output decompresso
	msg(msg::TGT::FILE, msg::TYP::LOG) << "Compressed data size: " << compressed_size + 4 << " bytes. " <<
										  "Decompressed data size: " << decompressed_size << " bytes. " <<
										  "Compression ratio: " << (float)(compressed_size + 4) / (float)decompressed_size * 100 << "%";
	char* compressed = new char[compressed_size];													// Array di input per dati compressi
	char* decompressed = new char[decompressed_size];												// Array di output per dati decompressi
	clz.read(compressed, compressed_size);															// Lettura dati compressi
    
	// Impostazione parametri zlib
	z_stream data_stream;
	data_stream.zalloc = (alloc_func)0;
	data_stream.zfree = (free_func)0;
	data_stream.opaque = (voidpf)0;
	data_stream.next_in  = (Bytef*)compressed;			// Dati compressi
	data_stream.avail_in = compressed_size;				// Dimensione dati compressi
	data_stream.next_out = (Bytef*)decompressed;		// Dati decompressi (ancora vuoto)
	data_stream.avail_out = decompressed_size;			// Dimensione dati decompressi
 
	// Decompressione
	inflateInit(&data_stream);      
	inflate(&data_stream, Z_NO_FLUSH);
	inflateEnd(&data_stream);

	// Creazione file di output
	AOD_IO.file_clzgmx = AOD_IO.file_clzgmx.substr(0, AOD_IO.file_clzgmx.size() - 4);
	ofstream out;
	out.open(AOD_IO.file_clzgmx, ofstream::binary);
	out.write(decompressed, decompressed_size);
	out.close();
	delete[] compressed;
	delete[] decompressed;
	return true;
}