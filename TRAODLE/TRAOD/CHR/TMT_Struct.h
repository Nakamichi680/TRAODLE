#pragma once


/*==================================================================================================================
	FORMATO TMT - Tomb Raider: The Angel of Darkness (blend shapes del volto dei personaggi)

	Fonti: [SRC] struttura MORPH_MESH della libreria di animazione (Tools\Libs\AnimLib\AnimAPI.h) e
		   CMorphMesh::ExportToGame (Tools\AnimEdit\Morph.cpp); [OK] verificato su tutti i 129 file TMT dei livelli.

	STRUTTURA DEL FILE
		TMT_HEADER (32 bytes)
		TMT_VERTEX x nVertices x (1 + nTargets)			Per ogni vertice: il record base seguito da un record per target
		padding fino a multiplo di 2048 bytes

	COLLEGAMENTO CON IL CHR [OK]
		BaseId e' l'ID (hash del nome, es. "LARA_FACE") di una MESH2 del CHR del personaggio: la mesh del volto, agganciata
		alla bone HEAD. I vertici sono nello stesso ordine e nello stesso spazio (quello della bone) della MESH2: il record
		base coincide con il vertice della MESH2 (in float invece che int16 / 16).
		Verificato su tutti i 21 volti distinti (Lara, Kurtis, Carvier, Karel, Bouchard, Muller, Eck, ...): la MESH2 BaseId
		ha sempre flags 1, bone HEAD e lo stesso numero di vertici del TMT. Attenzione: le MESH2 hanno vertici da 19 bytes,
		quindi i loro header non sono allineati a 4 bytes. Un TMT si applica solo al CHR che contiene la mesh (es. LARA.CHR).

	TARGET [SRC][OK]
		Nei record dei target posizione e normale sono OFFSET da sommare al record base; le UV sono ricopiate dalla base.
		Il file non contiene i nomi dei target (nTargets e' sempre 7).
		Le animazioni facciali (file TMS / MPHS, MORPH_SEQUENCE) modificano i pesi dei target: il loro TargetId e'
		TMT_HEADER.Id.
		Piu' TMT possono descrivere lo stesso volto (stesso BaseId) [OK]: con lo stesso Id (es. LARA.TMT e LARA_IG_1_24.TMT,
		vertici leggermente diversi, fino a 0.7) o con un Id diverso e gli stessi target in un altro ordine (LARA_CS_1_22D.TMT:
		target 3, 6, 7 = target 7, 3, 6 di LARA.TMT). Le sequenze del secondo tipo vengono rimappate (CHR_Morph.remap).
==================================================================================================================*/


struct TMT_HEADER				// SIZE: 32 bytes
{
	uint32_t Magic;						// [OK] "MPHB"
	uint32_t Size;						// [OK] Dimensione del file senza il padding finale: 32 + nVertices * (1 + nTargets) * 32
	uint32_t Id;						// [SRC] Hash del nome unico del morph [MORPH_MESH.Id]
	uint32_t BaseId;					// [SRC][OK] Hash del nome della mesh base (MESH2 del CHR) [MORPH_MESH.BaseId]
	uint32_t nTargets;					// [SRC][OK] Numero di target [NumTargets], sempre 7
	uint32_t nVertices;					// [SRC][OK] Numero di vertici [NumVertices]
	uint32_t VERTICES_PTR;				// [OK] Offset dei vertici dall'offset 8 (sempre 24) [Vertices]
	uint32_t pad;						// [SRC] Sempre 0
};
static_assert(sizeof(TMT_HEADER) == 32, "TMT_HEADER deve essere 32 bytes");


struct TMT_VERTEX				// SIZE: 32 bytes
{
	float X, Y, Z;						// [OK] Posizione (base) oppure offset della posizione (target)
	float Xn, Yn, Zn;					// [OK] Normale (base) oppure offset della normale (target)
	float U, V;							// [OK] UV (uguali in tutti i record del vertice)
};
static_assert(sizeof(TMT_VERTEX) == 32, "TMT_VERTEX deve essere 32 bytes");


#define TMT_MAGIC 0x4248504D			// "MPHB"
