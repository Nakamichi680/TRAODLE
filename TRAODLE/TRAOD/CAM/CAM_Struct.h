#pragma once


struct CAM_HEADER				// Size: 24 bytes
{
	uint32_t MAGIC1;			// "HDER"
	uint32_t Unknown1;
	uint32_t Unknown2;
	uint32_t nFrames;
	uint32_t MAGIC2;			// "TRAX"
	uint32_t Cuts_offset;		// Puntatore all'inizio del blocco Cuts (aggiungere a questo valore 24 per la dimensione dell'header)
};
	
	
struct CAM_POINT_FROM
{
	float Data;					// Prima tutte le X, poi tutte le Y, poi tutte le Z
};


struct CAM_LOOK_AT
{
	float Data;					// Prima tutte le X, poi tutte le Y, poi tutte le Z
};


struct CAM_ROLL
{
	float Data;					// Valori di rotazione lungo l'asse Y
};


struct CAM_FOV
{
	float Data;					// Field of View
};