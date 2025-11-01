#include "stdafx.h"
#include "MA/MA_Classes.h"


// Calcola la camera ottimale per inquadrare tutta la scena
Camera MA_CalculatePerspCamera (const vector<Mesh>& meshes,
								const vector<Transform>& transforms,
								float azimuthDeg = 45.0f,			// Angolo orizzontale
								float elevationDeg = 30.0f,			// Angolo verticale
								float distanceMultiplier = 2.5f)	// Moltiplicatore distanza) 
{
	Camera cam;
	cam.name = "persp";

	// 1. CALCOLA BOUNDING BOX DELLA SCENA
	float minX = numeric_limits<float>::max();
	float minY = numeric_limits<float>::max();
	float minZ = numeric_limits<float>::max();
	float maxX = numeric_limits<float>::lowest();
	float maxY = numeric_limits<float>::lowest();
	float maxZ = numeric_limits<float>::lowest();

	bool hasVertices = false;

	for (const Mesh& mesh : meshes)
	{
		string parent = mesh.parent;
		vector <Transform>::const_iterator it;
		float transformX(0), transformY(0), transformZ(0);
		it = find_if(transforms.begin(), transforms.end(), [&parent](const Transform& t) {return t.name == parent;});
		if (it != transforms.end())
		{
			int t = distance(transforms.begin(), it);
			transformX = transforms[t].tX;
			transformY = transforms[t].tY;
			transformZ = transforms[t].tZ;
		}

		for (size_t i = 0; i < mesh.X.size(); i++) {
			minX = min(minX, mesh.X[i] + transformX);
			minY = min(minY, mesh.Y[i] + transformY);
			minZ = min(minZ, mesh.Z[i] + transformZ);
			maxX = max(maxX, mesh.X[i] + transformX);
			maxY = max(maxY, mesh.Y[i] + transformY);
			maxZ = max(maxZ, mesh.Z[i] + transformZ);
			hasVertices = true;
		}
	}

	// Se non ci sono vertici, ritorna camera di default
	if (!hasVertices) {
		cam.tX = 2;
		cam.tY = -3;
		cam.tZ = 1;
		cam.rX = 74.5f;
		cam.rY = 0;
		cam.rZ = 33.69f;
		cam.coi = 3.742f;
		cam.fl = 35;
		cam.hfa = 1.417f;
		cam.vfa = 0.945f;
		cam.translate_flag = true;
		cam.rotate_flag = true;
		return cam;
	}

	// 2. CALCOLA CENTRO E DIMENSIONI DEL BOUNDING BOX
	float centerX = (minX + maxX) * 0.5f;
	float centerY = (minY + maxY) * 0.5f;
	float centerZ = (minZ + maxZ) * 0.5f;

	float sizeX = maxX - minX;
	float sizeY = maxY - minY;
	float sizeZ = maxZ - minZ;

	// Raggio della sfera che contiene il bounding box
	float boundingSphereRadius = sqrt(sizeX * sizeX + sizeY * sizeY + sizeZ * sizeZ) * 0.5f;

	// 3. CENTER OF INTEREST (punto verso cui guarda la camera)
	cam.coi = boundingSphereRadius * distanceMultiplier;  // Distanza dal COI alla camera

	// 4. POSIZIONAMENTO CAMERA
	// Angoli per una bella vista isometrica
	float azimuthRad = azimuthDeg * M_PI / 180.0f;
	float elevationRad = elevationDeg * M_PI / 180.0f;

	// Calcola la distanza necessaria per inquadrare tutta la scena
	// Fattore moltiplicativo per dare un po' di margine
	float dist = boundingSphereRadius * distanceMultiplier;

	// Posizione della camera in coordinate sferiche rispetto al centro
	float camX = centerX + dist *cos(elevationRad) * cos(azimuthRad);
	float camY = centerY + dist *cos(elevationRad) * sin(azimuthRad);
	float camZ = centerZ + dist *sin(elevationRad);

	cam.tX = camX;
	cam.tY = camY;
	cam.tZ = camZ;
	cam.translate_flag = true;

	// 5. ROTAZIONE CAMERA (punta verso il centro della scena)
	// Calcola vettore dalla camera al centro
	float dirX = centerX - camX;
	float dirY = centerY - camY;
	float dirZ = centerZ - camZ;

	float dirLen = sqrt(dirX * dirX + dirY * dirY + dirZ * dirZ);
	if (dirLen > 0.0001f) {
		dirX /= dirLen;
		dirY /= dirLen;
		dirZ /= dirLen;
	}

	// Calcola rotazioni Euler (XYZ order) per guardare verso il centro
	// rotZ (yaw) - rotazione attorno all'asse Z
	cam.rZ = atan2(dirY, dirX) * 180.0f / M_PI - 90;

	// rotX (pitch) - rotazione attorno all'asse X
	cam.rX = asin(dirZ) * 180.0f / M_PI + 90;

	// rotY (roll) - manteniamo a 0 per una camera dritta
	cam.rY = 0;

	cam.rotate_flag = true;

	// 6. CALCOLA FILM APERTURE
	// Aspect ratio 16:9
	float aspectRatio = 16.0f / 9.0f;

	// Film aperture standard in pollici (per Maya)
	cam.hfa = 1.417f;  // pollici (circa 36mm in fotografia)
	cam.vfa = cam.hfa / aspectRatio;
	cam.fl = 35.0f;

	// 7. CLIP PLANES
	cam.ncp = dist * 0.001f;         // Near clip al 0.1% della distanza
	cam.fcp = dist * 10.0f;          // Far clip al 1000% della distanza

	// Assicurati che i clip planes siano ragionevoli
	cam.ncp = max(0.1f, cam.ncp);
	cam.fcp = max(cam.ncp * 2.0f, cam.fcp);

	return cam;
}