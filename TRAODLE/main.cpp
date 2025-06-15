#include <iostream>
#include <fstream>
#include <ostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <string>
#include <stdint.h>
#include <stdio.h>
#include <cstdlib>
#include <conio.h>
#include <stdlib.h>
#include <ctime>
#include <ctype.h>
#define MAX 256
#include "windows.h"
#include "dir.h"
#include "Hash_generator.h"
#include "GMX_Struct.h"
#include "ZONE_Struct.h"
#include "RMX_Struct.h"
#include "String_list.h"
#include "ZONE_Exporter.h"


using namespace std;

int main(int argc, char **argv)
{
GMX_HEADER gmx_header;
GMX_LIST gmx_list;

system("cls");

cout << "TOMB RAIDER: THE ANGEL OF DARKNESS GMX UNPACKER by Nakamichi680\n\n";
cout << "For any question, feel free to contact me on:\nhttp://www.aspidetr.com/forum/\nor\nhttp://www.tombraiderforums.com/\n\n\n\n";


if (argv[1] != NULL)                                // Controlla se il nome del file del livello è stato inserito
{
    char levelname[MAX];                            // Stringa contenente il nome del livello
    char GMX[MAX];
    string input = argv[1];
    string temp;
    if (input.find("\\") == string::npos)
		temp = input;
    else
    {
        temp = input.substr(input.find_last_of("\\") + 1);
    }
    //cout << temp << endl;
    unsigned int sz = temp.size();
    transform(temp.begin(), temp.end(), temp.begin(), ::toupper);		// Converte in maiuscolo il nome del file in ingresso
    //cout << temp << endl;
    if (temp[sz-4] == '.' && temp[sz-3] == 'G' && temp[sz-2] == 'M' && temp[sz-1] == 'X')
    {
        temp = temp.substr(0, sz-4);
        //cout << "yes: " << temp;
    }

    char extension[MAX];
    strcpy(extension, ".GMX");
    strcpy(levelname, temp.c_str());                     // Copia il nome del livello dall'input del prompt dei comandi in "levelname"
//cout << levelname;
    //for (int i=0; i < strlen(levelname); i++)              // Questo ciclo for converte le lettere minuscole in maiuscole
        //levelname[i] = toupper(levelname[i]);

    strcpy(GMX, levelname);                         // Copia levelname in GMX
    strcat(GMX, extension);                         // Aggiunge l'estensione ".GMX"

    //cout << temp << endl << GMX << endl << levelname << endl << extension;

    ifstream gmxfile(GMX, std::ios::binary);        // Apre il file GMX

    if (gmxfile.is_open())      // Se il file è presente esegue il programma
        {
        gmxfile.read(reinterpret_cast<unsigned char*>(&gmx_header.GMX_ID), sizeof(gmx_header.GMX_ID));              // Legge l'ID del file

        if (gmx_header.GMX_ID == 1080452710)        // Controlla se il file GMX è valido
            {

            cout << "Reading GMX file...   \n\n";

            gmxfile.read(reinterpret_cast<unsigned char*>(&gmx_header.nFiles), sizeof(gmx_header.nFiles));
            gmxfile.seekg(2, ios_base::cur);
            int h_filenames[gmx_header.nFiles];
            int fileoffsets[gmx_header.nFiles];
            int filesizes[gmx_header.nFiles];

            for (int i=0; i<gmx_header.nFiles; i++)
                {
                gmxfile.read(reinterpret_cast<unsigned char*>(&h_filenames[i]), sizeof(gmx_list.Filename));         // Legge tutti i nomi dei files e li salva in un array
                gmxfile.read(reinterpret_cast<unsigned char*>(&fileoffsets[i]), sizeof(gmx_list.Offset));           // Legge tutti gli offsets dei files e li salva in un array
                gmxfile.read(reinterpret_cast<unsigned char*>(&filesizes[i]), sizeof(gmx_list.Size));               // Legge tutte le dimensioni dei files e le salva in un array
                }

            int num_zones = Get_num_zones(levelname, h_filenames, gmx_header.nFiles);       // Ricava il numero di zone contenute nel GMX

            char main_folder[MAX];
            GetCurrentDirectory(MAX, main_folder);                                          // Legge il percorso attuale del file
            //cout << main_folder;
            //getch();
            strcat(main_folder, "\\");                                                      // Aggiunge "\"
            strcat(main_folder, levelname);                                                 // Aggiunge il nome del livello
            CreateDirectory(main_folder, NULL);                                             // Crea la directory "....\nomelivello"
            SetCurrentDirectory(main_folder);                                               // Imposta "....\nomelivello" come cartella di lavoro

            for (int n=0; n<gmx_header.nFiles; n++)                                                         // Ad ogni ciclo estrae un file
                {
                gmxfile.seekg(fileoffsets[n] + 2048);                                                       // Posiziona lo stream all'inizio dei dati del file da estrarre
                ofstream file;
                file.open(Get_filename(levelname, h_filenames[n], num_zones).c_str(), ofstream::binary);    // Crea il file con il nome restituito da Get_filename
                cout << n+1 << ") " << Get_filename(levelname, h_filenames[n], num_zones).c_str() << endl;
                char* buffer = new char[filesizes[n]];                                                      // Buffer di lettura
                gmxfile.read(buffer, filesizes[n]);                                                         // Legge il gmx e mette nel buffer
                file.write(buffer, filesizes[n]);                                                           // Copia il buffer nel file di destinazione
                delete[] buffer;                                                                            // Elimina il buffer
                file.close();
                //cout << Get_filename(levelname, h_filenames[n], num_zones) << endl;
                }

            gmxfile.close();
            //cout << "DONE                 " << gmx_header.nFiles << " files found.\n\n\n";
            cout << "\nOperation completed. " << gmx_header.nFiles << " files found. Press any key to exit. \n";
            getch();
            //cout << "Reading RMX file... DONE\n\n";
            //for (int n=0; n<num_zones; n++)
                //zone_exporter(levelname, main_folder, n, num_zones);

            }
        else
        {
            cout << GMX << " is not a valid TR AoD level. Press any key to exit." << endl;          // Se il file GMX non è un file valido il programma termina
            getch();
        }
        }
    else
        {
        cout << GMX << " not found. Press any key to exit." << endl;                               // Se il nome del file è sbagliato il programma termina
        getch();
        }
    }
    else
    {
        cout << "USAGE\n";
        cout << "- Command Prompt:\n";
        cout << "     SYNTAX:   unGMX filename\n";
        cout << "               unGMX filename.GMX\n\n";
        cout << "- File Explorer:\n";
        cout << "     Drag and drop the GMX file into unGMX.EXE\n\n\n";
        cout << "Press any key to exit\n";
        getch();
    }

	//system("PAUSE");


return 0;
}
