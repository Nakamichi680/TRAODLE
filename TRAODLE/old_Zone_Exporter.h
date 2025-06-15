#ifndef ZONE_EXPORTER_H_INCLUDED
#define ZONE_EXPORTER_H_INCLUDED

using namespace std;

void zone_exporter(char levelname[MAX], char main_folder[MAX], int zone_number, int num_zones)
{
RMX_HEADER rmx_header;
RMX_OFFSETS rmx_offsets;
RMX_ROOM rmx_room;

ZONE_HEADER header;
ZONE_TEXTURE_HEADER texture_header;
ZONE_TEXTURE_MATERIALS texture_materials;
ZONE_TEXTURE_LIST texture_list;
ZONE_MESH_HEADER mesh_header;
ZONE_MESH_ROOM_HEADER mesh_room_header;
ZONE_MESH_OBJECT_HEADER mesh_object_header;
ZONE_MESH_VERTEX mesh_vertex;
ZONE_MESH_STRIP mesh_strip;
ZONE_MESH_ELEMENT mesh_element;

cout << "-------------------------------------------------------------------\n";
cout << "EXPORTING ZONE " << zone_number << " (" << (zone_number+1) << "/" << num_zones << ")" << endl << endl;

SetCurrentDirectory(main_folder);
char zone_folder[MAX], RMX[MAX], ZONE[MAX], FBX[MAX];
strcpy(zone_folder, main_folder);
strcat(zone_folder, "\\");
stringstream Znn;
Znn << "Z";
if (zone_number<10)
    Znn << "0";
Znn << zone_number;                        // Questo stringstream contiene "Z&&" dove && è il numero della zona
strcat(zone_folder, Znn.str().c_str());    // zone_folder contiene il nome della directory dove verrà creato il file FBX
CreateDirectory(zone_folder, NULL);         // Crea la directory Z&&

strcpy(RMX, levelname);                     // Crea il nome del file RMX
strcat(RMX, ".RMX");

strcpy(ZONE, levelname);                    // Crea il nome del file ZONE
strcat(ZONE, ".");
strcat(ZONE, Znn.str().c_str());

strcpy(FBX, levelname);                     // Crea il nome del file FBX
strcat(FBX, "_");
strcat(FBX, Znn.str().c_str());
strcat(FBX, ".FBX");

ifstream rmxfile(RMX, std::ios::binary);
ifstream zonefile(ZONE, std::ios::binary);

SetCurrentDirectory(zone_folder);

ofstream out;
out.open(FBX);




// LETTURA FILE RMX

rmxfile.seekg(16);
rmxfile.read(reinterpret_cast<unsigned char*>(&rmx_header.nROOMS), sizeof(rmx_header.nROOMS));

int c_offsets[rmx_header.nROOMS];
int c_R_IDs[rmx_header.nROOMS];
float c_R_Xt[rmx_header.nROOMS];
float c_R_Yt[rmx_header.nROOMS];
float c_R_Zt[rmx_header.nROOMS];

for (int r_rmx=0; r_rmx<rmx_header.nROOMS; r_rmx++)
    {
    rmxfile.read(reinterpret_cast<unsigned char*>(&c_offsets[r_rmx]), sizeof(rmx_offsets.OFFSET));        // Memorizza gli offsets di ogni stanza nel file RMX
    }

for (int r_rmx=0; r_rmx<rmx_header.nROOMS; r_rmx++)
    {
    rmxfile.seekg(c_offsets[r_rmx] + 100);
    rmxfile.read(reinterpret_cast<unsigned char*>(&c_R_IDs[r_rmx]), sizeof(rmx_room.Room_ID));        // Mette in un array gli ID di ogni stanza del RMX
    rmxfile.seekg(8, ios_base::cur);
    rmxfile.read(reinterpret_cast<unsigned char*>(&c_R_Xt[r_rmx]), sizeof(rmx_room.Room_Xt));         // Mette in un array le coordinate Xtranslation di ogni stanza del RMX
    rmxfile.read(reinterpret_cast<unsigned char*>(&c_R_Yt[r_rmx]), sizeof(rmx_room.Room_Yt));         // Mette in un array le coordinate Ytranslation di ogni stanza del RMX
    rmxfile.read(reinterpret_cast<unsigned char*>(&c_R_Zt[r_rmx]), sizeof(rmx_room.Room_Zt));         // Mette in un array le coordinate Ztranslation di ogni stanza del RMX
    }

rmxfile.close();

cout << "   Reading ZONE file:\n";


// LETTURA FILE ZONE

zonefile.seekg(4);       // Salta i primi 4 byte dell'ID
zonefile.read(reinterpret_cast<unsigned char*>(&header.TEXTURE_PTR), sizeof(header.TEXTURE_PTR));
zonefile.seekg(4, ios_base::cur);       // Salta UNKNOWN_PTR
zonefile.read(reinterpret_cast<unsigned char*>(&header.MESH_PTR), sizeof(header.MESH_PTR));
zonefile.read(reinterpret_cast<unsigned char*>(&header.EOF_PTR), sizeof(header.EOF_PTR));


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Esportazione textures. Innanzitutto viene letto l'header

zonefile.seekg(header.TEXTURE_PTR);
zonefile.read(reinterpret_cast<unsigned char*>(&texture_header.nMaterials), sizeof(texture_header.nMaterials));        // Lettura numero materiali
zonefile.seekg(4, ios_base::cur);       // Salta Unknown1
zonefile.read(reinterpret_cast<unsigned char*>(&texture_header.nTextures), sizeof(texture_header.nTextures));          // Lettura numero textures
zonefile.seekg(4, ios_base::cur);       // Salta Unknown2


///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////      CREAZIONE LISTA MATERIALI      ///////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

cout << "       Reading materials... ";

int arrDiffuse[texture_header.nMaterials];
int arrShadow[texture_header.nMaterials];
int arrBump[texture_header.nMaterials];
int Material_Count(0);
for (int m=0; m<texture_header.nMaterials; m++)
    {
    zonefile.seekg(8, ios_base::cur);
    int MaterialExist;
    zonefile.read(reinterpret_cast<unsigned char*>(&MaterialExist), sizeof(texture_materials.DiffuseID));        // Lettura ID diffuse map
    if (MaterialExist != -1)
        {
        arrDiffuse[m] = MaterialExist;                                                                           // Scrittura ID diffuse map
        zonefile.read(reinterpret_cast<unsigned char*>(&arrShadow[m]), sizeof(texture_materials.ShadowMapID));   // Scrittura ID shadow map
        zonefile.read(reinterpret_cast<unsigned char*>(&arrBump[m]), sizeof(texture_materials.BumpMapID));       // Scrittura ID bump map
        zonefile.seekg(4, ios_base::cur);
        Material_Count++;
        //cout << m << ") " << arrDiffuse[m] << " / " << arrShadow[m] << " / " << arrBump[m] << endl;
        }
    else
        {
        zonefile.seekg(12, ios_base::cur);
        arrDiffuse[m] = -1;
        }
    }

int realMaterialsCount(0);
for (int i=0; i<texture_header.nMaterials; i++)
    {
    if (arrDiffuse[i] != -1)
        realMaterialsCount++;
    }
cout << "DONE         " << realMaterialsCount << " materials found.\n       Extracting textures... ";

//////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////      CREAZIONE TEXTURES DDS O BMP     ///////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////


zonefile.seekg(24*texture_header.nMaterials + header.TEXTURE_PTR + 16);    // Posiziona la lettura all'inizio del blocco indice textures

int DXT[texture_header.nTextures];
int ColourBumpShadow[texture_header.nTextures];
int Mips[texture_header.nTextures];
int Xsize[texture_header.nTextures];
int Ysize[texture_header.nTextures];
int RAWsize[texture_header.nTextures];

for (int T=0; T<texture_header.nTextures; T++)
    {
    zonefile.read(reinterpret_cast<unsigned char*>(&DXT[T]), sizeof(texture_list.DXT));         // DXT1 o DXT3
    zonefile.read(reinterpret_cast<unsigned char*>(&ColourBumpShadow[T]), sizeof(texture_list.ColourBumpShadow));         // 1 = shadow map, 2 = colour map, 4 = bump map
    zonefile.seekg(8, ios_base::cur);        // Salta Unknown1/2
    zonefile.read(reinterpret_cast<unsigned char*>(&Mips[T]), sizeof(texture_list.Mips));            // MIPS
    zonefile.read(reinterpret_cast<unsigned char*>(&Xsize[T]), sizeof(texture_list.Xsize));          // Dimensioni texture asse X
    zonefile.read(reinterpret_cast<unsigned char*>(&Ysize[T]), sizeof(texture_list.Ysize));          // Dimensioni texture asse Y
    zonefile.read(reinterpret_cast<unsigned char*>(&RAWsize[T]), sizeof(texture_list.RAWsize));      // Dimensioni texture in bytes
    zonefile.seekg(8, ios_base::cur);        // Salta Unknown3/4
    //cout << DXT[T] << endl;
    //cout << T << "-----" << Mips[T] << " - " << Xsize[T] << " - " << Ysize[T] << " - " << RAWsize[T] << endl;
    }

for(int T=0; T<texture_header.nTextures; T++)
        {
        if (DXT[T] == 827611204 || DXT[T] == 861165636)                     // SCRITTURA TEXTURE DDS
            {
            string FILENAME;                    // Il nome della texture
            stringstream INCREMENT_NAME;
            INCREMENT_NAME << levelname << "_" << Znn.str() << "_" << T << ".dds";
            FILENAME = INCREMENT_NAME.str();
            ofstream dds;
            dds.open(FILENAME.c_str(), ofstream::binary);

            dds << char(0x44) << char(0x44) << char(0x53) << char(0x20) << char(0x7c) << char(0x00) << char(0x00) << char(0x00) << char(0x07) << char(0x10) << char(0x0a) << char(0x00);
            dds.write(reinterpret_cast<const char *>(&Xsize[T]), 4);        // DIMENSIONI DEI LATI DELLE TEXTURE
            dds.write(reinterpret_cast<const char *>(&Ysize[T]), 4);        // DIMENSIONI DEI LATI DELLE TEXTURE
            for (int h=0; h<8; h++)
                dds << char(0x00);
            dds.write(reinterpret_cast<const char *>(&Mips[T]), 4);         // MIPMAPS
            for (int h=0; h<44; h++)
                dds << char(0x00);
            dds << char(0x20) << char(0x00) << char(0x00) << char(0x00) << char(0x04) << char(0x00) << char(0x00) << char(0x00);
            dds.write(reinterpret_cast<const char *>(&DXT[T]), 4);;         // DXT1/DXT3
            for (int h=0; h<20; h++)
            dds << char(0x00);
            dds << char(0x08) << char(0x10) << char(0x40);
            for (int h=0; h<17; h++)
            dds << char(0x00);
            char* buffer = new char[RAWsize[T]];                             // Buffer di lettura
            zonefile.read(buffer, RAWsize[T]);                              // Legge i dati raw della texture e li mette nel buffer
            dds.write(buffer, RAWsize[T]);                                  // Copia il buffer nel file di destinazione
            delete[] buffer;
            dds.close();
            }
        else
            if (DXT[T] == 21)                                       // SCRITTURA TEXTURE BMP
                {
                string FILENAME;                    // Il nome della texture
                stringstream INCREMENT_NAME;
                INCREMENT_NAME << levelname << "_" << Znn.str() << "_" << T << ".bmp";
                FILENAME = INCREMENT_NAME.str();
                ofstream bmp;
                bmp.open(FILENAME.c_str(), ofstream::binary);

                bmp << char(0x42) << char(0x4d);                // Intestazione BMP
                int BMP_size(RAWsize[T] + 54);
                bmp.write(reinterpret_cast<const char *>(&BMP_size), 4);        // Dimensione file BMP
                bmp << char(0x00) << char(0x00) << char(0x00) << char(0x00) << char(0x36) << char(0x00) << char(0x00) << char(0x00) << char(0x28) << char(0x00) << char(0x00) << char(0x00);
                bmp.write(reinterpret_cast<const char *>(&Xsize[T]), 4);        // DIMENSIONI DEI LATI DELLE TEXTURE
                bmp.write(reinterpret_cast<const char *>(&Ysize[T]), 4);        // DIMENSIONI DEI LATI DELLE TEXTURE
                bmp << char(0x01) << char(0x00) << char(0x20) << char(0x00) << char(0x00) << char(0x00) << char(0x00) << char(0x00);
                bmp.write(reinterpret_cast<const char *>(&RAWsize[T]), 4);        // Dimensioni dati RAW texture
                for (int h=0; h<16; h++)
                    bmp << char(0x00);
                char* buffer = new char[RAWsize[T]];                             // Buffer di lettura
                zonefile.read(buffer, RAWsize[T]);                              // Legge i dati raw della texture e li mette nel buffer
                bmp.write(buffer, RAWsize[T]);                                  // Copia il buffer nel file di destinazione
                delete[] buffer;
                bmp.close();
                }
            else
                zonefile.seekg(RAWsize[T], ios_base::cur);
        }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cout << "DONE       " << texture_header.nTextures << " textures found.\n       Reading rooms... ";

////////////////////////////////////////////////////////////
// CALCOLO NUMERO LAYERED TEXTURE

int Layered_Count(0);
for (int m=0; m<Material_Count; m++)
    {
    if (arrDiffuse[m] >= 0 && arrDiffuse[m] < 999 && arrShadow[m] >= 0 && arrShadow[m] < 999)
        Layered_Count++;
    }
//cout << Layered_Count << endl;

// CALCOLO NUMERO TEXTURE BUMP MAPPING / REFLECTION MAP
int Bump_Count(0);
int bump_reflection_counter[texture_header.nTextures] = {};
for (int c=0; c<Material_Count; c++)
    if (arrBump[c] != -1 && bump_reflection_counter[arrBump[c]] == 0)
        {
        Bump_Count++;
        bump_reflection_counter[arrBump[c]]++;
        }
//cout << Bump_Count << endl;
// CALCOLO NUMERO TEXTURE SHADOW MAP
int SHMAP_Count(0);
int shadow_counter[Material_Count] = {};
for (int c=0; c<Material_Count; c++)
    if (arrShadow[c] != -1 && shadow_counter[arrShadow[c]] == 0)
        {
        SHMAP_Count++;
        shadow_counter[arrShadow[c]]++;
        }
//cout << SHMAP_Count;

////////////////////////////////////////////////////////////


// Si passa al gruppo mesh 1


zonefile.seekg(header.MESH_PTR);
zonefile.read(reinterpret_cast<unsigned char*>(&mesh_header.nRooms), sizeof(mesh_header.nRooms));        // Lettura numero stanze (r)

int R_IDs[mesh_header.nRooms];          // Tutti gli ID
int R_SIZE[mesh_header.nRooms];         // Tutte le dimensioni delle stanze
int R_nV[mesh_header.nRooms];           // Tutti i numeri di vertici delle stanze
int R_nINDX[mesh_header.nRooms];        // Tutti i numeri di indici delle stanze
int R_nEL[mesh_header.nRooms];          // Tutti i numeri di elementi delle stanze

// vettori del blocco VERTICI STANZE
vector< vector<float> > arrX (mesh_header.nRooms);
vector< vector<float> > arrY(mesh_header.nRooms);
vector< vector<float> > arrZ(mesh_header.nRooms);
vector< vector<float> > arrU1(mesh_header.nRooms);
vector< vector<float> > arrV1(mesh_header.nRooms);
vector< vector<float> > arrU2(mesh_header.nRooms);
vector< vector<float> > arrV2(mesh_header.nRooms);
vector< vector<uint8_t> > arrXn(mesh_header.nRooms);
vector< vector<uint8_t> > arrYn(mesh_header.nRooms);
vector< vector<uint8_t> > arrZn(mesh_header.nRooms);
vector< vector<uint8_t> > arrVc_red(mesh_header.nRooms);
vector< vector<uint8_t> > arrVc_green(mesh_header.nRooms);
vector< vector<uint8_t> > arrVc_blue(mesh_header.nRooms);

// vettore del blocco TRIANGLE STRIP
vector< vector<uint16_t> > arrIndex(mesh_header.nRooms);

// vettori del blocco ELEMENTI STANZE
vector< vector<uint32_t> > arrEl_Ind(mesh_header.nRooms);
vector< vector<uint32_t> > arrOffset(mesh_header.nRooms);
vector< vector<uint32_t> > arrMat_Ref(mesh_header.nRooms);


for(int r=0; r<mesh_header.nRooms; r++) // Questo macroblocco "for" legge tutti i dati di ogni stanza. "r" è il contatore delle stanze
    {
    zonefile.read(reinterpret_cast<unsigned char*>(&R_IDs[r]), sizeof(mesh_room_header.RoomID));         // ID stanza
    zonefile.read(reinterpret_cast<unsigned char*>(&R_SIZE[r]), sizeof(mesh_room_header.Room_size));     // dimensioni stanza bytes
    zonefile.seekg(8, ios_base::cur);       // Salta i due unknown1/2
    zonefile.read(reinterpret_cast<unsigned char*>(&R_nV[r]), sizeof(mesh_room_header.nVertices));       // numero vertici
    zonefile.seekg(4, ios_base::cur);       // Salta unknown3
    zonefile.read(reinterpret_cast<unsigned char*>(&R_nINDX[r]), sizeof(mesh_room_header.nIndices));     // numero indici nel triangle strip
    zonefile.seekg(4, ios_base::cur);       // Salta unknown4
    zonefile.read(reinterpret_cast<unsigned char*>(&R_nEL[r]), sizeof(mesh_room_header.nElements));      // numero elementi
    zonefile.seekg(20, ios_base::cur);       // Salta unknown5/6/7/8/9. Header stanza finito, la posizione di lettura è pronta per leggere i vertici


            for (int v=0; v<R_nV[r]; v++)       // Blocco lettura VERTICI
                {
                arrX[r].resize(v+1);
                arrY[r].resize(v+1);
                arrZ[r].resize(v+1);
                arrU1[r].resize(v+1);
                arrV1[r].resize(v+1);
                arrU2[r].resize(v+1);
                arrV2[r].resize(v+1);
                arrXn[r].resize(v+1);
                arrYn[r].resize(v+1);
                arrZn[r].resize(v+1);
                arrVc_red[r].resize(v+1);
                arrVc_green[r].resize(v+1);
                arrVc_blue[r].resize(v+1);

                zonefile.read(reinterpret_cast<unsigned char*>(&arrX[r][v]), sizeof(mesh_vertex.X));                 // Legge e memorizza nel vettore la coordinata X
                zonefile.read(reinterpret_cast<unsigned char*>(&arrY[r][v]), sizeof(mesh_vertex.Y));                 // Legge e memorizza nel vettore la coordinata Y
                zonefile.read(reinterpret_cast<unsigned char*>(&arrZ[r][v]), sizeof(mesh_vertex.Z));                 // Legge e memorizza nel vettore la coordinata Z

                zonefile.read(reinterpret_cast<unsigned char*>(&arrU1[r][v]), sizeof(mesh_vertex.U1));               // UV
                zonefile.read(reinterpret_cast<unsigned char*>(&arrV1[r][v]), sizeof(mesh_vertex.V1));               // UV
                zonefile.read(reinterpret_cast<unsigned char*>(&arrU2[r][v]), sizeof(mesh_vertex.U2));               // UV shadow map
                zonefile.read(reinterpret_cast<unsigned char*>(&arrV2[r][v]), sizeof(mesh_vertex.V2));               // UV shadow map

                zonefile.read(reinterpret_cast<unsigned char*>(&arrXn[r][v]), sizeof(mesh_vertex.Xn));               // Vertex normal
                zonefile.read(reinterpret_cast<unsigned char*>(&arrYn[r][v]), sizeof(mesh_vertex.Yn));               // Vertex normal
                zonefile.read(reinterpret_cast<unsigned char*>(&arrZn[r][v]), sizeof(mesh_vertex.Zn));               // Vertex normal

                zonefile.seekg(6, ios_base::cur);        // Salta Unknown 1-6 (coordinate da scoprire per bump ed altro)

                zonefile.read(reinterpret_cast<unsigned char*>(&arrVc_red[r][v]), sizeof(mesh_vertex.VC_red));       // Vertex color
                zonefile.read(reinterpret_cast<unsigned char*>(&arrVc_green[r][v]), sizeof(mesh_vertex.VC_green));   // Vertex color
                zonefile.read(reinterpret_cast<unsigned char*>(&arrVc_blue[r][v]), sizeof(mesh_vertex.VC_blue));     // Vertex color
                }

            for (int s=0; s<R_nINDX[r]; s++)    // Blocco lettura TRIANGLE STRIP
                {
                arrIndex[r].resize(s+1);
                zonefile.read(reinterpret_cast<unsigned char*>(&arrIndex[r][s]), sizeof(mesh_strip.Index));
                }

            for (int el=0; el<R_nEL[r]; el++)    // Blocco lettura ELEMENTI
                {
                arrEl_Ind[r].resize(el+1);
                arrOffset[r].resize(el+1);
                arrMat_Ref[r].resize(el+1);

                zonefile.seekg(4, ios_base::cur);        // Salta nElement_Triangles (non serve a niente per l'esportazione)
                zonefile.read(reinterpret_cast<unsigned char*>(&arrEl_Ind[r][el]), sizeof(mesh_element.nElement_Indices));
                zonefile.read(reinterpret_cast<unsigned char*>(&arrOffset[r][el]), sizeof(mesh_element.Offset));
                zonefile.read(reinterpret_cast<unsigned char*>(&arrMat_Ref[r][el]), sizeof(mesh_element.Material_Ref));
                zonefile.seekg(48, ios_base::cur);
                }
    }

cout << "DONE             " << mesh_header.nRooms << " room";
if (mesh_header.nRooms != 1)
    cout << "s";
cout << " found.\n       Creating faces... ";


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IN QUESTA PORZIONE VIENE MANIPOLATO IL TRIANGLE STRIP E CREATO IL VETTORE CONTENENTE TUTTE LE FACCE(1) DI TUTTI GLI ELEMENTI(2) DI TUTTE LE STANZE(3) (MATRICE 3D)

vector < vector<uint16_t> > INDXmin(mesh_header.nRooms);
vector < vector<uint16_t> > INDXmax(mesh_header.nRooms);
vector < vector < vector<int16_t> > > FBX_FACES(mesh_header.nRooms);       // VETTORE TRIDIMENSIONALE "STANZE" x "ELEMENTI" x "FACCE"

for (int r=0; r<mesh_header.nRooms; r++)        // Ogni ciclo è una stanza
    {
    vector < vector<uint16_t> > Split_strip(R_nEL[r]);      // Questo vettore contiene gli strips solamente divisi per elementi

    // Divisione strip per elementi

    for (int el=0; el<R_nEL[r]; el++)           // Ogni ciclo è un elemento
        {
        for (int I=0; I<arrEl_Ind[r][el]; I++)                      // Questo "for" legge ogni indice per elemento e lo memorizza in Split_strip
            {
            Split_strip[el].resize(I+1);
            Split_strip[el][I] = arrIndex[r][arrOffset[r][el]+I];   // Split_strip[numero elemento][posizione INDX vertice]
            }
        INDXmin[r].resize(el+1);
        INDXmax[r].resize(el+1);
        INDXmin[r][el] = *min_element(Split_strip[el].begin(), Split_strip[el].end());      // Memorizza l'indice del vertice minore dell'elemento
        INDXmax[r][el] = *max_element(Split_strip[el].begin(), Split_strip[el].end());      // Memorizza l'indice del vertice maggiore dell'elemento

        for (int I=0; I<arrEl_Ind[r][el]; I++)              // Questo "for" prende i valori gli indici di ogni strip e li fa cominciare da 0, sottraendo da ognuno l'indice minimo INDXmin[r][el]
            Split_strip[el][I] = Split_strip[el][I] - INDXmin[r][el];
        }

    for (int el=0; el<R_nEL[r]; el++)       // Ogni ciclo fa un elemento della stanza
        {
        FBX_FACES[r].resize(el+1);          // Aggiunge uno slot per il nuovo elemento
        if (arrOffset[r][el]%2 == 0)        // Verifica se il punto d'inizio di ogni frammento dello strip è pari o dispari
            {
            int P=0;
            for (int I=0; I<(arrEl_Ind[r][el] -2); I++)
                {
                if (Split_strip[el][I] != Split_strip[el][I+1] && Split_strip[el][I] != Split_strip[el][I+2] && Split_strip[el][I+1] != Split_strip[el][I+2])       // Esclude le facce nulle
                    {
                    if (I%2 == 0)       // Se pari
                        {
                        FBX_FACES[r][el].resize(P+3);
                        FBX_FACES[r][el][P] = Split_strip[el][I+2];
                        P++;
                        FBX_FACES[r][el][P] = Split_strip[el][I+1];
                        P++;
                        FBX_FACES[r][el][P] = Split_strip[el][I] ^ -1;      // XOR
                        P++;
                        }
                    else                // Se dispari
                        {
                        FBX_FACES[r][el].resize(P+3);
                        FBX_FACES[r][el][P] = Split_strip[el][I];
                        P++;
                        FBX_FACES[r][el][P] = Split_strip[el][I+1];
                        P++;
                        FBX_FACES[r][el][P] = Split_strip[el][I+2] ^ -1;    // XOR
                        P++;
                        }
                    }
                }
            }
        else
            {
            int P=0;
            for (int I=0; I<(arrEl_Ind[r][el] -2); I++)
                {
                if (Split_strip[el][I] != Split_strip[el][I+1] && Split_strip[el][I] != Split_strip[el][I+2] && Split_strip[el][I+1] != Split_strip[el][I+2])
                    {
                    if (I%2 == 0)
                        {
                        FBX_FACES[r][el].resize(P+3);
                        FBX_FACES[r][el][P] = Split_strip[el][I];
                        P++;
                        FBX_FACES[r][el][P] = Split_strip[el][I+1];
                        P++;
                        FBX_FACES[r][el][P] = Split_strip[el][I+2] ^ -1;
                        P++;
                        }
                    else
                        {
                        FBX_FACES[r][el].resize(P+3);
                        FBX_FACES[r][el][P] = Split_strip[el][I+2];
                        P++;
                        FBX_FACES[r][el][P] = Split_strip[el][I+1];
                        P++;
                        FBX_FACES[r][el][P] = Split_strip[el][I] ^ -1;
                        P++;
                        }
                    }
                }
            }
        }
    Split_strip.clear();
    }

// FINE BLOCCO MANIPOLAZIONE TRIANGLE STRIP. LA MATRICE 3D SI CHIAMA "FBX_FACES[numero stanza][numero elemento][indice vertice]"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Calcolo dei valori di traslazione delle stanze. Viene effettuato un confronto di ID tra zone e rmx e
float R_Xt[mesh_header.nRooms];
float R_Yt[mesh_header.nRooms];
float R_Zt[mesh_header.nRooms];
int c_R_counter(0);
for (int r=0; r<mesh_header.nRooms;)
    {
        if (R_IDs[r] == c_R_IDs[c_R_counter])       // Il programma cerca tra tutti gli ID del rmx quello corrispondente al zone
            {
            R_Xt[r] = c_R_Xt[c_R_counter];          // Array contenente tutte le coordinate di traslazione delle stanze lungo l'asse X
            R_Yt[r] = c_R_Yt[c_R_counter];          // Array contenente tutte le coordinate di traslazione delle stanze lungo l'asse Y
            R_Zt[r] = c_R_Zt[c_R_counter];          // Array contenente tutte le coordinate di traslazione delle stanze lungo l'asse Z
            r++;
            c_R_counter = 0;
            }
        else c_R_counter++;
    }


// Calcolo Model_Count e Geometry_Count per header file FBX
int Model_Count(0);
int Geometry_Count(0);
for (int r=0; r<mesh_header.nRooms; r++)
    Geometry_Count= R_nEL[r] + Geometry_Count;              // Numero di elementi totale
Model_Count = Geometry_Count + mesh_header.nRooms;          // Numero di elementi totale + numero stanze


// Generazione colori per le stanze nel file FBX
srand (static_cast <unsigned> (time(0)));
float Red[mesh_header.nRooms], Green[mesh_header.nRooms], Blue[mesh_header.nRooms];
float LO(0.2);
float HI(1);
for (int r=0; r<mesh_header.nRooms; r++)
    {
    Red[r] = LO + static_cast <float> (rand()) / ( static_cast <float> (RAND_MAX/(HI-LO)));
    Green[r] = LO + static_cast <float> (rand()) / ( static_cast <float> (RAND_MAX/(HI-LO)));
    Blue[r] = LO + static_cast <float> (rand()) / ( static_cast <float> (RAND_MAX/(HI-LO)));
    }


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INIZIO PARTE SCRITTURA FILE DI OUTPUT

cout << "DONE\n\n   Writing FBX file:";

// Intestazione file
out << "; FBX 7.1.0 project file\n; Copyright (C) 1997-2010 Autodesk Inc. and/or its licensors.\n; All rights reserved.\n; ----------------------------------------------------\n\n";

// Header file FBX
out << "FBXHeaderExtension:  {\n    FBXHeaderVersion: 1003\n    FBXVersion: 7100\n    Creator: \"FBX SDK/FBX Plugins version 2011.3.1\"\n    SceneInfo: \"SceneInfo::GlobalInfo\", \"UserData\" {";
out << "\n        Type: \"UserData\"\n        Version: 100\n        MetaData:  {\n            Version: 100\n            Title: \"\"\n            Subject: \"\"\n            Author: \"\"\n";
out << "            Keywords: \"\"\n            Revision: \"\"\n            Comment: \"\"\n        }\n\n    }\n}";
out << "\nGlobalSettings:  {\n    Version: 1000\n    Properties70:  {\n        P: \"UpAxis\", \"int\", \"Integer\", \"\",2\n        P: \"UpAxisSign\", \"int\", \"Integer\", \"\",1\n        P: \"FrontAxis\", \"int\", \"Integer\", \"\",1\n        P: \"FrontAxisSign\", \"int\", \"Integer\", \"\",-1\n        P: \"CoordAxis\", \"int\", \"Integer\", \"\",0\n";
out << "        P: \"CoordAxisSign\", \"int\", \"Integer\", \"\",1\n        P: \"OriginalUpAxis\", \"int\", \"Integer\", \"\",2\n        P: \"OriginalUpAxisSign\", \"int\", \"Integer\", \"\",1\n        P: \"UnitScaleFactor\", \"double\", \"Number\", \"\",2.54\n        P: \"OriginalUnitScaleFactor\", \"double\", \"Number\", \"\",2.54\n        P: \"AmbientColor\", \"ColorRGB\", \"Color\", \"\",0,0,0\n";
out << "        P: \"DefaultCamera\", \"KString\", \"\", \"\", \"Producer Perspective\"\n        P: \"TimeMode\", \"enum\", \"\", \"\",6\n        P: \"TimeSpanStart\", \"KTime\", \"Time\", \"\",0\n        P: \"TimeSpanStop\", \"KTime\", \"Time\", \"\",153953860000\n    }\n}\n";

// Object definitions
out << "; Object definitions\n;------------------------------------------------------------------\n\nDefinitions:  {\n    Version: 100\n    Count: 21\n    ObjectType: \"Model\" {\n        Count: ";
out << Model_Count;
out << "\n    }\n    ObjectType: \"Geometry\" {\n        Count: ";
out << Geometry_Count << "\n" << "    }\n    ObjectType: \"Material\" {\n        Count: " << Material_Count;
out << "\n        PropertyTemplate: \"KFbxSurfacePhong\" {\n            Properties70:  {\n                P: \"ShadingModel\", \"KString\", \"\", \"\", \"Phong\"\n";
out << "                P: \"MultiLayer\", \"bool\", \"\", \"\",0\n                P: \"EmissiveColor\", \"ColorRGB\", \"Color\", \"\",0,0,0\n                P: \"EmissiveFactor\", \"double\", \"Number\", \"\",1\n";
out << "                P: \"AmbientColor\", \"ColorRGB\", \"Color\", \"\",0.2,0.2,0.2\n                P: \"AmbientFactor\", \"double\", \"Number\", \"\",1\n                P: \"DiffuseColor\", \"ColorRGB\", \"Color\", \"\",0.8,0.8,0.8\n";
out << "                P: \"DiffuseFactor\", \"double\", \"Number\", \"\",1\n                P: \"Bump\", \"Vector3D\", \"Vector\", \"\",0,0,0\n                P: \"NormalMap\", \"Vector3D\", \"Vector\", \"\",0,0,0\n";
out << "                P: \"BumpFactor\", \"double\", \"Number\", \"\",1\n                P: \"TransparentColor\", \"ColorRGB\", \"Color\", \"\",0,0,0\n                P: \"TransparencyFactor\", \"double\", \"Number\", \"\",0\n";
out << "                P: \"DisplacementColor\", \"ColorRGB\", \"Color\", \"\",0,0,0\n                P: \"DisplacementFactor\", \"double\", \"Number\", \"\",1\n                P: \"SpecularColor\", \"ColorRGB\", \"Color\", \"\",0.2,0.2,0.2\n";
out << "                P: \"SpecularFactor\", \"double\", \"Number\", \"\",1\n                P: \"ShininessExponent\", \"double\", \"Number\", \"\",20\n                P: \"ReflectionColor\", \"ColorRGB\", \"Color\", \"\",0,0,0\n";
out << "                P: \"ReflectionFactor\", \"double\", \"Number\", \"\",1\n            }\n        }\n    }\n    ObjectType: \"LayeredTexture\" {\n        Count: " << Layered_Count << "\n";
out << "         PropertyTemplate: \"KFbxLayeredTexture\" {\n            Properties70:  {\n                P: \"TextureTypeUse\", \"enum\", \"\", \"\",0\n                P: \"Texture alpha\", \"Number\", \"\", \"A+\",1\n";
out << "                P: \"CurrentMappingType\", \"enum\", \"\", \"\",0\n                P: \"WrapModeU\", \"enum\", \"\", \"\",0\n                P: \"WrapModeV\", \"enum\", \"\", \"\",0\n                P: \"UVSwap\", \"bool\", \"\", \"\",0\n";
out << "                P: \"Translation\", \"Vector\", \"\", \"A+\",0,0,0\n                P: \"Rotation\", \"Vector\", \"\", \"A+\",0,0,0\n                P: \"Scaling\", \"Vector\", \"\", \"A+\",1,1,1\n                P: \"TextureRotationPivot\", \"Vector3D\", \"Vector\", \"\",0,0,0\n";
out << "                P: \"TextureScalingPivot\", \"Vector3D\", \"Vector\", \"\",0,0,0\n                P: \"UseMaterial\", \"bool\", \"\", \"\",0\n                P: \"UseMipMap\", \"bool\", \"\", \"\",0\n                P: \"CurrentTextureBlendMode\", \"enum\", \"\", \"\",1\n";
out << "                P: \"UVSet\", \"KString\", \"\", \"\", \"default\"\n            }\n        }\n    }\n    ObjectType: \"Texture\" {\n        Count: ";
out << Material_Count + Layered_Count + Bump_Count + SHMAP_Count;
out << "\n        PropertyTemplate: \"KFbxTexture\" {\n            Properties70:  {\n                P: \"TextureTypeUse\", \"enum\", \"\", \"\",0\n                P: \"Texture alpha\", \"Number\", \"\", \"A+\",1\n                P: \"CurrentMappingType\", \"enum\", \"\", \"\",0\n";
out << "                P: \"WrapModeU\", \"enum\", \"\", \"\",0\n                P: \"WrapModeV\", \"enum\", \"\", \"\",0\n                P: \"UVSwap\", \"bool\", \"\", \"\",0\n                P: \"Translation\", \"Vector\", \"\", \"A+\",0,0,0\n";
out << "                P: \"Rotation\", \"Vector\", \"\", \"A+\",0,0,0\n                P: \"Scaling\", \"Vector\", \"\", \"A+\",1,1,1\n                P: \"TextureRotationPivot\", \"Vector3D\", \"Vector\", \"\",0,0,0\n                P: \"TextureScalingPivot\", \"Vector3D\", \"Vector\", \"\",0,0,0\n";
out << "                P: \"UseMaterial\", \"bool\", \"\", \"\",0\n                P: \"UseMipMap\", \"bool\", \"\", \"\",0\n                P: \"CurrentTextureBlendMode\", \"enum\", \"\", \"\",1\n                P: \"UVSet\", \"KString\", \"\", \"\", \"default\"\n            }\n        }\n    }\n}\n\n";


cout << "\n       Writing rooms... ";


// Object properties
out << "; Object properties\n;------------------------------------------------------------------\n\nObjects:  {\n";

for (int r=0; r<mesh_header.nRooms; r++)
    {
    out << "    Model: " << "9" <<  r << r << ", \"Model::ROOM_" << r << "\", \"Null\" {\n        Version: 232\n        Properties70:  {\n            P: \"InheritType\", \"enum\", \"\", \"\",1\n";
    out << "            P: \"ScalingMin\", \"Vector3D\", \"Vector\", \"\",1,1,1\n            P: \"DefaultAttributeIndex\", \"int\", \"Integer\", \"\",0\n";
    out << "            P: \"Lcl Translation\", \"Lcl Translation\", \"\", \"A+\"," << R_Xt[r] << "," << R_Yt[r] << "," << R_Zt[r] << "\n        }\n";
    out << "        MultiLayer: 0\n        MultiTake: 1\n        Shading: T\n        Culling: \"CullingOff\"\n    }\n";

    for (int el=0; el<R_nEL[r]; el++)
        {
        out << "    Model: " << "9" << r << r << "9" << el << el << ", \"Model::ROOM_" << r << "_OBJ_" << el << "\", \"Mesh\" {\n        Version: 232\n        Properties70:  {\n";
        out << "            P: \"InheritType\", \"enum\", \"\", \"\",1\n            P: \"ScalingMin\", \"Vector3D\", \"Vector\", \"\",1,1,1\n";
        out << "            P: \"DefaultAttributeIndex\", \"int\", \"Integer\", \"\",0\n            P: \"Lcl Translation\", \"Lcl Translation\", \"\", \"A+\",0,0,0\n        }\n";
        out << "        MultiLayer: 0\n        MultiTake: 1\n        Shading: T\n        Culling: \"CullingOff\"\n    }\n";

        out << "    Geometry: " << "9" << r << r << "0" << el << el << ", \"Geometry::\", \"Mesh\" {\n        Properties70:  {\n            P: \"Color\", \"ColorRGB\", \"Color\", \"\",";
        out << Red[r] << "," << Green[r] << "," << Blue[r] << "\n" << "}" << "\n";
        out << "        Vertices: *" << (INDXmax[r][el] - INDXmin[r][el] + 1) * 3 << " {";      // Numero di vertici x 3
        out << "\n" << "a: ";

        /////////////////////////////////////////////////////
        // Scrive i vertici
        for (int v=0; v<(INDXmax[r][el] - INDXmin[r][el] + 1); v++)
            {
            out << arrX[r][INDXmin[r][el]+v] << "," << arrY[r][INDXmin[r][el]+v] << "," << arrZ[r][INDXmin[r][el]+v];
            if (INDXmax[r][el] - INDXmin[r][el] == v) out << "\n";
            else out << ",\n";
            }
        ////////////////////////////////////////////////////

        out << "}\n        PolygonVertexIndex: *";
        out << FBX_FACES[r][el].size() << " {";         // Numero di indici di vertici
        out << "\n" << "a: ";

        ////////////////////////////////////////////////////
        // Scrive le facce
        for (int P=0; P<FBX_FACES[r][el].size(); P++)
            {
            out << FBX_FACES[r][el][P];
            if (FBX_FACES[r][el].size() - P != 1)
                {
                out << ",";
                if ((P+1)%30 == 0 and P != 0)  out << "\n";
                }
            else out << "\n";
            }
        ////////////////////////////////////////////////////

        out << "}\n        GeometryVersion: 124\n        LayerElementNormal: 0 {\n            Version: 101\n            Name: \"\"\n            MappingInformationType: \"ByVertice\"\n";
        out << "            ReferenceInformationType: \"Direct\"\n            Normals: *" << (INDXmax[r][el] - INDXmin[r][el] + 1) * 3 << " {";         // Numero di normals x 3
        out << "\n" << "a: ";

        /////////////////////////////////////////////////////
        // Scrive i normals
        for (int n=0; n<(INDXmax[r][el] - INDXmin[r][el] + 1); n++)
            {
            float Xn, Yn, Zn;
            Xn = unsigned(arrXn[r][INDXmin[r][el]+n]);
            Xn = (Xn - 128) / 127;
            Yn = unsigned(arrYn[r][INDXmin[r][el]+n]);
            Yn = (Yn - 128) / 127;
            Zn = unsigned(arrZn[r][INDXmin[r][el]+n]);
            Zn = (Zn - 128) / 127;
            out << Xn << "," << Yn << "," << Zn;
            if (INDXmax[r][el] - INDXmin[r][el] == n) out << "\n";
            else out << ",\n";
            }
        ////////////////////////////////////////////////////

        out << "}\n        }\n        LayerElementColor: 0 {\n            Version: 101\n            Name: \"\"\n            MappingInformationType: \"ByVertice\"\n";
        out << "            ReferenceInformationType: \"Direct\"\n            Colors: *" << (INDXmax[r][el] - INDXmin[r][el] + 1) * 4 << " {";          // Numero di vertex colors x 4
        out << "\n" << "a: ";

        /////////////////////////////////////////////////////
        // Scrive i vertex colors
        for (int c=0; c<(INDXmax[r][el] - INDXmin[r][el] + 1); c++)
            {
            float vcR, vcG, vcB;
            vcR = unsigned(arrVc_red[r][INDXmin[r][el]+c]);
            vcR = vcR / 255;
            vcG = unsigned(arrVc_green[r][INDXmin[r][el]+c]);
            vcG = vcG / 255;
            vcB = unsigned(arrVc_blue[r][INDXmin[r][el]+c]);
            vcB = vcB / 255;
            out << vcR << "," << vcG << "," << vcB << ",1";
            if (INDXmax[r][el] - INDXmin[r][el] == c) out << "\n";
            else out << ",\n";
            }
        ////////////////////////////////////////////////////

        out << "}\n        }\n        LayerElementUV: 0 {\n            Version: 101\n            Name: \"UVChannel_1\"\n            MappingInformationType: \"ByVertice\"\n            ReferenceInformationType: \"Direct\"\n";
        out << "            UV: *" << (INDXmax[r][el] - INDXmin[r][el] + 1) * 2 << " {\na: ";       // Numero di UV DIFFUSE/BUMP x 2

        /////////////////////////////////////////////////////
        // Scrive gli UV DIFFUSE/BUMP

        for (int uv=0; uv<(INDXmax[r][el] - INDXmin[r][el] + 1); uv++)
            {
            out << arrU1[r][INDXmin[r][el]+uv] << "," << arrV1[r][INDXmin[r][el]+uv];
            if (INDXmax[r][el] - INDXmin[r][el] == uv) out << "\n";
            else out << ",\n";
            }
        ////////////////////////////////////////////////////

        out << "}\n        }\n";

        /////////////////////////////////////////////////////
        // SE PRESENTI scrive gli UV SHADOW MAP

        if (arrShadow[arrMat_Ref[r][el]] != -1)
            {
            out << "        LayerElementUV: 1 {\n            Version: 101\n            Name: \"UVChannel_2\"\n            MappingInformationType: \"ByVertice\"\n            ReferenceInformationType: \"Direct\"\n";
            out << "            UV: *" << (INDXmax[r][el] - INDXmin[r][el] + 1) * 2 << " {\na: ";       // Numero di UV SHADOW MAP x 2

            /////////////////////////////////////////////////////
            // Scrive gli UV SHADOW MAP

            for (int uv=0; uv<(INDXmax[r][el] - INDXmin[r][el] + 1); uv++)
                {
                out << arrU2[r][INDXmin[r][el]+uv] << "," << arrV2[r][INDXmin[r][el]+uv];
                if (INDXmax[r][el] - INDXmin[r][el] == uv) out << "\n";
                else out << ",\n";
                }
            ////////////////////////////////////////////////////

            out << "}\n        }\n";
            };

        out << "        LayerElementMaterial: 0 {\n";
        out << "            Version: 101\n";
        out << "            Name: \"\"\n";
        out << "            MappingInformationType: \"AllSame\"\n";
        out << "            ReferenceInformationType: \"IndexToDirect\"\n";
        out << "            Materials: *1 {\n";
        out << "a: 0\n";
        out << "            }\n";
        out << "        }\n";
        out << "        Layer: 0 {\n";
        out << "            Version: 100\n";
        out << "            LayerElement:  {\n";
        out << "                Type: \"LayerElementNormal\"\n";
        out << "                TypedIndex: 0\n";
        out << "            }\n";
        out << "            LayerElement:  {\n";
        out << "                Type: \"LayerElementColor\"\n";
        out << "                TypedIndex: 0\n";
        out << "            }\n";
        out << "            LayerElement:  {\n";
        out << "                Type: \"LayerElementMaterial\"\n";
        out << "                TypedIndex: 0\n";
        out << "            }\n";
        out << "            LayerElement:  {\n";
        out << "                Type: \"LayerElementUV\"\n";
        out << "                TypedIndex: 0\n";
        out << "            }\n";
        out << "        }\n";

        if (arrShadow[arrMat_Ref[r][el]] != -1)
            {
            out << "        Layer: 1 {\n            Version: 100\n            LayerElement:  {\n                Type: \"LayerElementUV\"\n                TypedIndex: 1\n            }\n        }\n";

            }
        out << "\n    }\n";
        }
    out << "\n";
    }

cout << "DONE\n       Writing materials... ";

for (int m=0; m<Material_Count; m++)
    {
    out << "    Material: 8" << m << m;
    out << ", \"Material::Material_" << m << "\", \"\" {\n        Version: 102\n        ShadingModel: \"phong\"\n        MultiLayer: 0\n        Properties70:  {\n";
    out << "            P: \"ShadingModel\", \"KString\", \"\", \"\", \"phong\"\n            P: \"EmissiveFactor\", \"double\", \"Number\", \"\",0\n            P: \"AmbientColor\", \"ColorRGB\", \"Color\", \"\",0.588235318660736,0.588235318660736,0.588235318660736\n";
    out << "            P: \"DiffuseColor\", \"ColorRGB\", \"Color\", \"\",0.588235318660736,0.588235318660736,0.588235318660736\n            P: \"TransparentColor\", \"ColorRGB\", \"Color\", \"\",1,1,1\n            P: \"SpecularColor\", \"ColorRGB\", \"Color\", \"\",0.899999976158142,0.899999976158142,0.899999976158142\n";
    out << "            P: \"SpecularFactor\", \"double\", \"Number\", \"\",0\n            P: \"ShininessExponent\", \"double\", \"Number\", \"\",1.99999991737042\n            P: \"Enabled\", \"Bool\", \"\", \"A+U\",0\n            P: \"EffectList\", \"Integer\", \"\", \"A+U\",0,0,0\n";
    out << "            P: \"dxStdMat\", \"Bool\", \"\", \"A+U\",0\n            P: \"SaveFXFile\", \"KString\", \"\", \"U\", \"\"\n            P: \"Emissive\", \"Vector3D\", \"Vector\", \"\",0,0,0\n            P: \"Ambient\", \"Vector3D\", \"Vector\", \"\",0.588235318660736,0.588235318660736,0.588235318660736\n";
    out << "            P: \"Diffuse\", \"Vector3D\", \"Vector\", \"\",0.588235318660736,0.588235318660736,0.588235318660736\n            P: \"Specular\", \"Vector3D\", \"Vector\", \"\",0,0,0\n            P: \"Shininess\", \"double\", \"Number\", \"\",1.99999991737042\n";
    out << "            P: \"Opacity\", \"double\", \"Number\", \"\",1\n            P: \"Reflectivity\", \"double\", \"Number\", \"\",0\n        }\n    }\n";
    }

int texture_counter[texture_header.nTextures][6] = {}; // colonna 0: alpha, colonna 1: diffuse map, colonna 2: diffuse+shadow map, colonna 3: shadow map, colonna 4: bump map, colonna 5: specular map

for (int m=0; m<Material_Count; m++)
    {
    if (texture_counter[arrDiffuse[m]][0] == 0)
        {
        if (arrDiffuse[m] >= 0 && arrDiffuse[m] < 999 && DXT[arrDiffuse[m]] == 861165636)
            {
            out << "    Texture: 1" << arrDiffuse[m] << arrDiffuse[m] <<", \"Texture::Opacity Map #" << arrDiffuse[m] <<"\", \"\" {\n";
            out << "        Type: \"TextureVideoClip\"\n";
            out << "        Version: 202\n";
            out << "        TextureName: \"Texture::Opacity Map #" << arrDiffuse[m] << "\"\n";
            out << "        Properties70:  {\n";
            out << "            P: \"UseMaterial\", \"bool\", \"\", \"\",1\n";
            out << "            P: \"UVSet\", \"KString\", \"\", \"\", \"UVChannel_1\"\n";
            out << "        }\n";
            out << "        FileName: \"" << zone_folder << "\\" << levelname << "_" << Znn.str() << "_" << arrDiffuse[m] << ".tif\"\n";
            out << "        ModelUVTranslation: 0,0\n";
            out << "        ModelUVScaling: 1,1\n";
            out << "        Texture_Alpha_Source: \"Alpha_Black\"\n";
            out << "        Cropping: 0,0,0,0\n    }\n";
            texture_counter[arrDiffuse[m]][0]++;
            }
        }

    if (texture_counter[arrDiffuse[m]][1] == 0)
        {
        if (arrDiffuse[m] >= 0 && arrDiffuse[m] < 999 && arrShadow[m] == -1)
            {
            out << "    Texture: 7" << arrDiffuse[m] << arrDiffuse[m] << ", \"Texture::Diffuse map #" << arrDiffuse[m] <<"\", \"\" {\n";
            out << "        Type: \"TextureVideoClip\"\n";
            out << "        Version: 202\n";
            out << "        TextureName: \"Texture::Diffuse map #" << arrDiffuse[m] << "\"\n";
            out << "        Properties70:  {\n";
            out << "            P: \"UseMaterial\", \"bool\", \"\", \"\",1\n";
            out << "            P: \"UVSet\", \"KString\", \"\", \"\", \"UVChannel_1\"\n";
            out << "        }\n";
            out << "        FileName: \"" << zone_folder << "\\" << levelname << "_" << Znn.str() << "_" << arrDiffuse[m] << ".tif\"\n";
            out << "        ModelUVTranslation: 0,0\n";
            out << "        ModelUVScaling: 1,1\n";
            out << "        Texture_Alpha_Source: \"None\"\n";
            out << "        Cropping: 0,0,0,0\n    }\n";
            texture_counter[arrDiffuse[m]][1]++;
            }
        }

    if (texture_counter[arrDiffuse[m]][2] == 0)
        {
        if (arrDiffuse[m] >= 0 && arrDiffuse[m] < 999 && arrShadow[m] >=0 && arrShadow[m] < 999)
            {
            out << "    Texture: 6" << arrDiffuse[m] << arrDiffuse[m] << ", \"Texture::Composite_Diffuse map #" << arrDiffuse[m] << "\", \"\" {\n";
            out << "        Type: \"TextureVideoClip\"\n";
            out << "        Version: 202\n";
            out << "        TextureName: \"Texture::Composite_Diffuse map #" << arrDiffuse[m] << "\"\n";
            out << "        Properties70:  {\n";
            out << "            P: \"UseMaterial\", \"bool\", \"\", \"\",1\n";
            out << "            P: \"CurrentTextureBlendMode\", \"enum\", \"\", \"\",4\n";
            out << "            P: \"UVSet\", \"KString\", \"\", \"\", \"UVChannel_1\"\n";
            out << "        }\n";
            out << "        FileName: \"" << zone_folder << "\\" << levelname << "_" << Znn.str() << "_" << arrDiffuse[m] << ".tif\"\n";
            out << "        ModelUVTranslation: 0,0\n";
            out << "        ModelUVScaling: 1,1\n";
            out << "        Texture_Alpha_Source: \"None\"\n";
            out << "        Cropping: 0,0,0,0\n    }\n";
            texture_counter[arrDiffuse[m]][2]++;
            }
        }

    if (texture_counter[arrShadow[m]][3] == 0)
        {
        if (arrDiffuse[m] >= 0 && arrDiffuse[m] < 999 && arrShadow[m] >=0 && arrShadow[m] < 999)
            {
            out << "    Texture: 4" << arrShadow[m] << arrShadow[m] << ", \"Texture::Shadow map #" << arrShadow[m] << "\", \"\" {\n";
            out << "        Type: \"TextureVideoClip\"\n";
            out << "        Version: 202\n";
            out << "        TextureName: \"Texture::Shadow map #" << arrShadow[m] << "\"\n";
            out << "        Properties70:  {\n";
            out << "            P: \"Texture alpha\", \"Number\", \"\", \"A+\",0.8\n";
            out << "            P: \"UseMaterial\", \"bool\", \"\", \"\",1\n";
            out << "            P: \"CurrentTextureBlendMode\", \"enum\", \"\", \"\",2\n";
            out << "            P: \"UVSet\", \"KString\", \"\", \"\", \"UVChannel_2\"\n";
            out << "        }\n";
            out << "        FileName: \"" << zone_folder << "\\" << levelname << "_" << Znn.str() << "_" << arrShadow[m] << ".tif\"\n";
            out << "        ModelUVTranslation: 0,0\n";
            out << "        ModelUVScaling: 1,1\n";
            out << "        Texture_Alpha_Source: \"None\"\n";
            out << "        Cropping: 0,0,0,0\n    }\n";
            texture_counter[arrShadow[m]][3]++;
            }
        }

    if (texture_counter[arrBump[m]][4] == 0)
        {
        if (arrDiffuse[m] >= 0 && arrDiffuse[m] < 999 && arrBump[m] >=0 && arrBump[m] < 999 && DXT[arrBump[m]] == 827611204)
            {
            out << "    Texture: 3" << arrBump[m] << arrBump[m] << ", \"Texture::Bump map #" << arrBump[m] << "\", \"\" {\n";
            out << "        Type: \"TextureVideoClip\"\n";
            out << "        Version: 202\n";
            out << "        TextureName: \"Texture::Map #" << arrBump[m] << "\"\n";
            out << "        Properties70:  {\n";
            out << "            P: \"UseMaterial\", \"bool\", \"\", \"\",1\n";
            out << "            P: \"UVSet\", \"KString\", \"\", \"\", \"UVChannel_1\"\n";
            out << "        }\n";
            out << "        FileName: \"" << zone_folder << "\\" << levelname << "_" << Znn.str() << "_" << arrBump[m] << ".tif\"\n";
            out << "        ModelUVTranslation: 0,0\n";
            out << "        ModelUVScaling: 1,1\n";
            out << "        Texture_Alpha_Source: \"Alpha_Black\"\n";
            out << "        Cropping: 0,0,0,0\n    }\n";
            texture_counter[arrBump[m]][4]++;
            }
        }

    if (texture_counter[arrBump[m]][5] == 0)
        {
        if (arrDiffuse[m] >= 0 && arrDiffuse[m] < 999 && arrBump[m] >=0 && arrBump[m] < 999 && DXT[arrBump[m]] == 861165636)
            {
            out << "    Texture: 2" << arrBump[m] << arrBump[m] << ", \"Texture::Reflection map #" << arrBump[m] << "\", \"\" {\n";
            out << "        Type: \"TextureVideoClip\"\n";
            out << "        Version: 202\n";
            out << "        TextureName: \"Texture::Map #" << arrBump[m] << "\"\n";
            out << "        Properties70:  {\n";
            out << "            P: \"3dsMax\", \"Compound\", \"\", \"\"\n";
            out << "            P: \"3dsMax|ClassIDa\", \"int\", \"Integer\", \"\",592\n";
            out << "            P: \"3dsMax|ClassIDb\", \"int\", \"Integer\", \"\",0\n";
            out << "            P: \"3dsMax|SuperClassID\", \"int\", \"Integer\", \"\",3088\n";
            out << "            P: \"3dsMax|parameters\", \"Compound\", \"\", \"\"\n";
            out << "            P: \"3dsMax|parameters|size\", \"Integer\", \"\", \"A\",100\n";
            out << "            P: \"3dsMax|parameters|blur\", \"Float\", \"\", \"A\",1\n";
            out << "            P: \"3dsMax|parameters|blurOffset\", \"Float\", \"\", \"A\",0\n";
            out << "            P: \"3dsMax|parameters|near\", \"Float\", \"\", \"A\",0\n";
            out << "            P: \"3dsMax|parameters|far\", \"Float\", \"\", \"A\",500\n";
            out << "            P: \"3dsMax|parameters|source\", \"Integer\", \"\", \"A\",0\n";
            out << "            P: \"3dsMax|parameters|useAtmosphericMap\", \"Bool\", \"\", \"A\",1\n";
            out << "            P: \"3dsMax|parameters|apply\", \"Bool\", \"\", \"A\",1\n";
            out << "            P: \"3dsMax|parameters|frametype\", \"Integer\", \"\", \"A\",0\n";
            out << "            P: \"3dsMax|parameters|nthframe\", \"Integer\", \"\", \"A\",1\n";
            out << "            P: \"3dsMax|parameters|bitmapName\", \"KString\", \"\", \"\", \"\"\n";
            out << "            P: \"3dsMax|parameters|bitmapName|[0]\", \"KString\", \"\", \"\", \"\"\n";
            out << "            P: \"3dsMax|parameters|bitmapName|[1]\", \"KString\", \"\", \"\", \"\"\n";
            out << "            P: \"3dsMax|parameters|bitmapName|[2]\", \"KString\", \"\", \"\", \"\"\n";
            out << "            P: \"3dsMax|parameters|bitmapName|[3]\", \"KString\", \"\", \"\", \"\"\n";
            out << "            P: \"3dsMax|parameters|bitmapName|[4]\", \"KString\", \"\", \"\", \"\"\n";
            out << "            P: \"3dsMax|parameters|bitmapName|[5]\", \"KString\", \"\", \"\", \"\"\n";
            out << "            P: \"3dsMax|parameters|outputname\", \"KString\", \"\", \"A\", \"\"\n";
            out << "        }\n";
            out << "        FileName: \"\"\n";
            out << "        ModelUVTranslation: 0,0\n";
            out << "        ModelUVScaling: 1,1\n";
            out << "        Texture_Alpha_Source: \"None\"\n";
            out << "        Cropping: 0,0,0,0\n    }\n";
            }
        }

    if (arrDiffuse[m] >= 0 && arrDiffuse[m] < 999 && arrShadow[m] >= 0 && arrShadow[m] < 999)
        {
        out << "    LayeredTexture: 5" << m << m << ", \"LayeredTexture::Diffuse+Shadow\", \"\" {\n        LayeredTexture: 100\n        BlendModes: 4,2\n    }\n";
        }
    }


cout << "DONE\n       Creating connections... ";

out << "}\n\n; Object connections\n;------------------------------------------------------------\n\nConnections:  {\n";


for (int r=0; r<mesh_header.nRooms; r++)
    {
    out << "    C: \"OO\"," << "9" << r << r << ",0\n";      // Stanza,0

    for (int el=0; el<R_nEL[r]; el++)
        {
        out << "    C: \"OO\"," << "9" << r << r << "9" << el << el << "," << "9" << r << r << "\n";         // Null,Stanza
        out << "    C: \"OO\"," << "9" << r << r << "0" << el << el << "," << "9" << r << r << "9" << el << el << "\n";     // Oggetto,Null
        out << "    C: \"OO\"," << "8" << arrMat_Ref[r][el] << arrMat_Ref[r][el] << "," << "9" << r << r << "9" << el << el << "\n";    // Materiale,Null
        }
    }
for (int m=0; m<Material_Count; m++)
    {
    out << "\n";
    if (arrDiffuse[m] >= 0 && arrDiffuse[m] < 999 && DXT[arrDiffuse[m]] == 861165636)
        out << "    C: \"OP\"," << "1" << arrDiffuse[m] << arrDiffuse[m] << "," << "8" << m << m << ", \"TransparentColor\"\n";

    if (arrDiffuse[m] >= 0 && arrDiffuse[m] < 999 && arrShadow[m] == -1)
        out << "    C: \"OP\"," << "7" << arrDiffuse[m] << arrDiffuse[m] << "," << "8" << m << m << ", \"DiffuseColor\"\n";

    if (arrDiffuse[m] >= 0 && arrDiffuse[m] < 999 && arrShadow[m] >= 0 && arrShadow[m] < 999)
        {
        out << "    C: \"OP\"," << "5" << m << m << "," << "8" << m << m <<", \"DiffuseColor\"\n";
        out << "    C: \"OO\"," << "6" << arrDiffuse[m] << arrDiffuse[m] << "," << "5" << m << m << "\n";
        out << "    C: \"OO\"," << "4" << arrShadow[m] << arrShadow[m] << "," << "5" << m << m << "\n";
        }

    if (arrDiffuse[m] >= 0 && arrDiffuse[m] < 999 && arrBump[m] >=0 && arrBump[m] < 999 && DXT[arrBump[m]] == 827611204)
        out << "    C: \"OP\"," << "3" << arrBump[m] << arrBump[m] << "," << "8" << m << m << ", \"Bump\"\n";

    if (arrDiffuse[m] >= 0 && arrDiffuse[m] < 999 && arrBump[m] >=0 && arrBump[m] < 999 && DXT[arrBump[m]] == 861165636)
        out << "    C: \"OP\"," << "2" << arrBump[m] << arrBump[m] << "," << "8" << m << m << ", \"ReflectionColor\"\n";
    }
out << "}";
out.close();
zonefile.close();

cout << "DONE\n\nZONE " << zone_number << " SUCCESSFULLY EXPORTED.\n";
cout << "-------------------------------------------------------------------\n";


}


#endif // ZONE_EXPORTER_H_INCLUDED
