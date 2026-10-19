#include "stdafx.h"
#include <functional>
#include <map>
#include <algorithm>
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
	// I vertici delle mesh sono nello spazio del gruppo padre: la posizione nel mondo si ottiene con tutta la catena dei
	// gruppi (traslazione, rotazione XYZ, scala; i pivot sono ignorati). Il box usato per inquadrare la scena e' quello
	// del gruppo principale di vertici su ogni asse (MainCluster), cosi' gli oggetti lontani (es. oggetti dei filmati
	// posizionati all'origine, Items sulla griglia accanto al livello) non spostano la telecamera; il box completo serve
	// per il far clip plane.
	struct Affine { double m[3][4]; };
	map <string, size_t> transform_index;
	for (size_t t = 0; t < transforms.size(); t++)
		transform_index[transforms[t].name] = t;
	map <string, Affine> world_cache;
	function <Affine (const string &, int)> World = [&](const string &name, int depth) -> Affine
	{
		Affine identity = {{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}}};
		map <string, size_t>::const_iterator it = transform_index.find(name);
		if (name.empty() || it == transform_index.end() || depth > 64)
			return identity;
		map <string, Affine>::const_iterator cached = world_cache.find(name);
		if (cached != world_cache.end())
			return cached->second;
		const Transform &t = transforms[it->second];
		const double D = M_PI / 180.0;
		double cx = cos(t.rX * D), sx = sin(t.rX * D), cy = cos(t.rY * D), sy = sin(t.rY * D), cz = cos(t.rZ * D), sz = sin(t.rZ * D);
		double R[3][3] = {{cy * cz, sx * sy * cz - cx * sz, cx * sy * cz + sx * sz},		// Rz * Ry * Rx (ordine XYZ di Maya)
						  {cy * sz, sx * sy * sz + cx * cz, cx * sy * sz - sx * cz},
						  {-sy, sx * cy, cx * cy}};
		double S[3] = {t.sX, t.sY, t.sZ}, T[3] = {t.tX, t.tY, t.tZ};
		Affine local;
		for (int r = 0; r < 3; r++)
		{
			for (int c = 0; c < 3; c++)
				local.m[r][c] = R[r][c] * S[c];
			local.m[r][3] = T[r];
		}
		Affine parent = World(t.parent, depth + 1), out;
		for (int r = 0; r < 3; r++)
		{
			for (int c = 0; c < 4; c++)
				out.m[r][c] = parent.m[r][0] * local.m[0][c] + parent.m[r][1] * local.m[1][c] + parent.m[r][2] * local.m[2][c];
			out.m[r][3] += parent.m[r][3];
		}
		world_cache[name] = out;
		return out;
	};

	float minX = numeric_limits<float>::max();
	float minY = numeric_limits<float>::max();
	float minZ = numeric_limits<float>::max();
	float maxX = numeric_limits<float>::lowest();
	float maxY = numeric_limits<float>::lowest();
	float maxZ = numeric_limits<float>::lowest();
	vector <float> allX, allY, allZ;							// Vertici nel mondo (al massimo circa 2 milioni)
	size_t nVertices = 0;
	for (const Mesh& mesh : meshes)
		nVertices += mesh.X.size();
	size_t stride = nVertices / 2000000 + 1;

	bool hasVertices = false;
	size_t counter = 0;
	for (const Mesh& mesh : meshes)
	{
		Affine w = World(mesh.parent, 0);
		for (size_t i = 0; i < mesh.X.size() && i < mesh.Y.size() && i < mesh.Z.size(); i++) {
			float x = (float)(w.m[0][0] * mesh.X[i] + w.m[0][1] * mesh.Y[i] + w.m[0][2] * mesh.Z[i] + w.m[0][3]);
			float y = (float)(w.m[1][0] * mesh.X[i] + w.m[1][1] * mesh.Y[i] + w.m[1][2] * mesh.Z[i] + w.m[1][3]);
			float z = (float)(w.m[2][0] * mesh.X[i] + w.m[2][1] * mesh.Y[i] + w.m[2][2] * mesh.Z[i] + w.m[2][3]);
			minX = min(minX, x);	minY = min(minY, y);	minZ = min(minZ, z);
			maxX = max(maxX, x);	maxY = max(maxY, y);	maxZ = max(maxZ, z);
			if (counter++ % stride == 0)
			{
				allX.push_back(x);	allY.push_back(y);	allZ.push_back(z);
			}
			hasVertices = true;
		}
	}
	// Raggio della sfera che contiene tutta la scena (per il far clip plane)
	float fullRadius = sqrt((maxX - minX) * (maxX - minX) + (maxY - minY) * (maxY - minY) + (maxZ - minZ) * (maxZ - minZ)) * 0.5f;
	// Su ogni asse i vertici vengono divisi in gruppi separati da intervalli vuoti piu' ampi del 3% dell'estensione totale:
	// il box usa il gruppo con piu' vertici (il livello), senza gli oggetti lontani
	auto MainCluster = [](vector <float> &v, float &lo, float &hi)
	{
		sort(v.begin(), v.end());
		float gap = (v.back() - v.front()) * 0.03f;
		size_t best_start = 0, best_end = 0, start = 0;
		for (size_t i = 1; i <= v.size(); i++)
			if (i == v.size() || v[i] - v[i - 1] > gap)
			{
				if (i - start > best_end - best_start)
				{
					best_start = start;
					best_end = i;
				}
				start = i;
			}
		lo = v[best_start];
		hi = v[best_end - 1];
	};
	if (allX.size() >= 100)										// Box senza i vertici isolati
	{
		MainCluster(allX, minX, maxX);
		MainCluster(allY, minY, maxY);
		MainCluster(allZ, minZ, maxZ);
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
	// Far clip: tutta la scena (anche i vertici isolati) vista dalla posizione iniziale con margine, anche spostando la
	// telecamera. Near clip piccolo per poter girare dentro le stanze: 1/100000 del far clip (precisione dello Z-buffer)
	cam.fcp = (dist + fullRadius) * 2.0f;
	cam.ncp = cam.fcp * 0.00001f;

	// Assicurati che i clip planes siano ragionevoli
	cam.ncp = max(0.1f, cam.ncp);
	cam.fcp = max(cam.ncp * 2.0f, cam.fcp);

	return cam;
}