////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimLib\hierachy.h"
#include "AnimLib\skin.h"
#include "morph.h"
#include "baseclass.h"
#include "cfitcurve.h"
#include "resource.h"
#include "AnimLib\AnimAPI.h"
#include "general.h"
#include "AnimLib\animation.h"
#include "CGlobal.h"

////////////////////////////////////////////////////////////////////////////////

#define MORPH_ENVELOPE_TRACK_SCALE 8192.0f
#define MORPH_WEIGHT_TRACK_SCALE 8192.0f

////////////////////////////////////////////////////////////////////////////////

const int MORPH_RECORD_VERSION = 1;
const int MORPH_VERSION = 1;
const int MORPH_BASE_VERSION = 1;
const int MORPH_TRACK_VERSION = 1;
const int MORPH_BLEND_VERSION = 1;
const int MORPH_DATA_VERSION = 1;

CMorphData* CMorphData::MorphData = NULL;
float Frame = 0;

////////////////////////////////////////////////////////////////////////////////

extern short* FindBezier(short* fdata, short* p, int nBeziers, float* frame);
extern int ChunkRead_MRPH(char *chunkadr, int chunklen);
extern int ChunkRead_MRAN(char *chunkadr, int chunklen);

////////////////////////////////////////////////////////////////////////////////
// CMorphTrack
////////////////////////////////////////////////////////////////////////////////

CMorphTrack::CMorphTrack(int count)
{
	TrackValues = NULL;
	NumTrackValues = 0;

	CurveData = NULL;
	FrameData = NULL;

	NumCurves = 0;
	AnimBlock = NULL;
	AnimSize = 0;

	if (count != 0)
	{
		SetNumTrackValues(0);
	}
}

//------------------------------------------------------------------------------

CMorphTrack::~CMorphTrack()
{
	if (CurveData) free(CurveData);
	if (FrameData) free(FrameData);
	if (AnimBlock) free(AnimBlock);

	if (TrackValues) delete[] TrackValues;
}

//------------------------------------------------------------------------------

int CMorphTrack::GetNumTrackValues()
{
	return NumTrackValues;
}

//------------------------------------------------------------------------------

void CMorphTrack::SetNumTrackValues(int count)
{
	if (TrackValues)
	{
		delete[] TrackValues;
		TrackValues = NULL;
	}

	if (count)
	{
		TrackValues = new float[count];
	}

	NumTrackValues = count;
}

//------------------------------------------------------------------------------

float CMorphTrack::GetTrackValue(int index)
{
	if ((index >=  0) && (index < NumTrackValues)) return TrackValues[index];

	return 0;
}

//------------------------------------------------------------------------------

#define USE_STREAM

#ifndef USE_STREAM

extern float Bezier(short* p, float t, float s);
extern short* FindBezier(short* fdata, short* p, int nBeziers, float* frame);

float CMorphTrack::GetTrackValue(float time)
{
	short *curve;

	curve = FindBezier(FrameData, CurveData, NumCurves, &time);
	
	return Bezier(curve, time, MORPH_ENVELOPE_TRACK_SCALE);
}

#else

float CMorphTrack::GetTrackValue(float time)
{
	float data;

	APB_GetTrackValue(AnimBlock, (int)time, &data, 1.0f / MORPH_ENVELOPE_TRACK_SCALE);	

	return data;
}

#endif

//------------------------------------------------------------------------------

void CMorphTrack::SetTrackValue(int index, float value)
{
	if ((index < 0) || (index >=  NumTrackValues)) return;
	
	TrackValues[index] = value;
}

//------------------------------------------------------------------------------

float* CMorphTrack::GetTrackValues()
{
	return TrackValues;
}

//------------------------------------------------------------------------------

void CMorphTrack::Serialize(CArchive &ar)
{
	int i;

	if (ar.IsStoring())
	{
		ar << MORPH_TRACK_VERSION;

		ar << GetPackageID();

		ar << AnimSize;

		for (i = 0; i < AnimSize; i++)
		{
			ar << AnimBlock[i];
		}

		ar << GetNumTrackValues();

		for (i = 0; i < GetNumTrackValues(); i++)
		{
			ar << GetTrackValue(i);
		}
	}
	else
	{
		int StoredVersion;
		int PackageID;
		int temp;

		ar >> StoredVersion;

		ar >> PackageID;
		SetPackageID(PackageID);

		ar >> AnimSize;

		if (AnimSize)
		{
			AnimBlock = (uint8*)malloc(sizeof(char) * AnimSize);

			for (i = 0; i < AnimSize; i++)
			{
				ar >> AnimBlock[i];
			}
		}

		ar >> temp;
		SetNumTrackValues(temp);

		for (i = 0; i < GetNumTrackValues(); i++)
		{
			ar >> temp;
			SetTrackValue(i, (float) temp);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// CMorphData
////////////////////////////////////////////////////////////////////////////////

CMorphData::CMorphData()
{
	MorphMeshes.RemoveAll();
	MorphSequences.RemoveAll();
}

//------------------------------------------------------------------------------

CMorphData::~CMorphData()
{
	Clear();
}

//------------------------------------------------------------------------------

void CMorphData::Clear()
{
	int i;

	for (i = 0; i < MorphMeshes.GetSize(); i++)
	{
		delete MorphMeshes[i];
	}

	MorphMeshes.RemoveAll();

	for (i = 0; i < MorphSequences.GetSize(); i++)
	{
		delete MorphSequences[i];
	}

	MorphSequences.RemoveAll();
}

//------------------------------------------------------------------------------

int CMorphData::GetNumMorphMeshes()
{
	return MorphMeshes.GetSize();
}

//------------------------------------------------------------------------------

int CMorphData::AddMorphMesh(CMorphMesh* morph_mesh)
{
	return MorphMeshes.Add(morph_mesh);
}

//------------------------------------------------------------------------------

void CMorphData::RemoveMorphMesh(CMorphMesh* morph_mesh)
{
	int i;

	for (i = MorphMeshes.GetSize() - 1; i >= 0; i--)
	{
		if (MorphMeshes[i] == morph_mesh) MorphMeshes.RemoveAt(i);
	}
}

//------------------------------------------------------------------------------

CMorphMesh* CMorphData::GetMorphMesh(int index)
{
	return MorphMeshes[index];
}

//------------------------------------------------------------------------------

CMorphMesh* CMorphData::GetMorphMesh(const char* name)
{
	int i;

	for (i = 0; i < MorphMeshes.GetSize(); i++)
	{
		if (!stricmp(MorphMeshes[i]->GetName(), name)) return MorphMeshes[i];
	}

	return NULL;
}

//------------------------------------------------------------------------------

CMorphMesh* CMorphData::GetMorphMeshUnique(const char* name)
{
	int i;

	for (i = 0; i < MorphMeshes.GetSize(); i++)
	{
		if (!stricmp(MorphMeshes[i]->GetUniqueName(), name)) return MorphMeshes[i];
	}

	return NULL;
}

//------------------------------------------------------------------------------

int CMorphData::GetNumMorphSequences()
{
	return MorphSequences.GetSize();
}

//------------------------------------------------------------------------------

int CMorphData::AddMorphSequence(CMorphSequence* morph_sequence)
{
	return MorphSequences.Add(morph_sequence);
}

//------------------------------------------------------------------------------

void CMorphData::RemoveMorphSequence(CMorphSequence* morph_sequence)
{
	int i;

	for (i = MorphSequences.GetSize() - 1; i >= 0; i--)
	{
		if (MorphSequences[i] == morph_sequence) MorphSequences.RemoveAt(i);
	}
}

//------------------------------------------------------------------------------

CMorphSequence* CMorphData::GetMorphSequence(int index)
{
	return MorphSequences[index];
}

//------------------------------------------------------------------------------

CMorphSequence* CMorphData::GetMorphSequence(const char* name)
{
	int i;

	for (i = 0; i < MorphSequences.GetSize(); i++)
	{
		if (!stricmp(MorphSequences[i]->GetName(), name)) return MorphSequences[i];
	}

	return NULL;
}

//------------------------------------------------------------------------------

bool CMorphData::Load(const char* filename, const char* file)
{
	static RCHUNK ReadChunks[] =
	{
		{ CHUNKID("MRPH"), ChunkRead_MRPH },
		{ CHUNKID("MRAN"), ChunkRead_MRAN },
		{ 0, 0 }
	};

	char* data;
	int file_length;
	bool result;

	data = (char*)LoadFile(filename, NULL, file_length);
	if (!data) return false;
	
	InitChunkReader();
	
	strcpy(Filename, file);
	strcpy(FullPathName, filename);

	result = (ChunksRecurse(data, file_length, ReadChunks) != 0);

	free(data);

	return result;
}

//------------------------------------------------------------------------------

void CMorphData::Serialize(CArchive &ar)
{
	int i;

	if (ar.IsStoring())
	{
		ar << MORPH_DATA_VERSION;
		ar << GetPackageID();

		ar << GetNumMorphMeshes();

		for (i = 0; i < GetNumMorphMeshes(); i++)
		{
			GetMorphMesh(i)->Serialize(ar);
		}

		ar << GetNumMorphSequences();

		for (i = 0; i < GetNumMorphSequences(); i++)
		{
			GetMorphSequence(i)->Serialize(ar);
		}
	}
	else
	{
		int StoredVersion;
		int count;
		int PackageID;
		int i;

		Clear();

		ar >> StoredVersion;

		ar >> PackageID;
		SetPackageID(PackageID);

		ar >> count;

		for (i = 0; i < count; i++)
		{
			CMorphMesh *morph_mesh = new CMorphMesh;

			morph_mesh->Serialize(ar);
			AddMorphMesh(morph_mesh);
		}

		ar >> count;

		for (i = 0; i < count; i++)
		{
			CMorphSequence *sequence = new CMorphSequence;

			sequence->Serialize(ar);
			AddMorphSequence(sequence);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

void Morph_DeleteSequence(MORPH_SEQUENCE* sequence)
{
	if (sequence->Envelope) delete[] sequence->Envelope;

	for (uint32 n = 0; n < sequence->NumTracks; n++)
	{
		delete[] sequence->Tracks[n].AnimBlock;
	}

	if (sequence->MorphMesh)
	{
		Morph_DeleteMesh(sequence->MorphMesh);
		sequence->MorphMesh = NULL;
	}

	delete sequence;
}

////////////////////////////////////////////////////////////////////////////////

void Morph_DeleteMesh(MORPH_MESH *mesh)
{
	if (mesh->Vertices)
	{
		delete[] mesh->Vertices;
		mesh->Vertices = NULL;
	}

	delete mesh;
}

////////////////////////////////////////////////////////////////////////////////

void Morph_RemoveMorphData(ANIMOBJECT *aobj)
{
	uint32 i;

	for (i = 0; i < aobj->NumMorphMeshes; i++)
	{
		Morph_DeleteMesh(aobj->MorphMeshes[i]);
	}

	aobj->NumMorphMeshes = 0;

	for (i = 0; i < aobj->NumMorphSequences; i++)
	{
		Morph_DeleteSequence(aobj->MorphSequences[i]);
	}

	aobj->NumMorphSequences = 0;

	aobj->Morph.Sequence = NULL;
}

////////////////////////////////////////////////////////////////////////////////

int ChunkRead_MRPH(char *chunkadr, int chunklen)
{
	int num_morph_meshes;
	int length, dummy;
	int i;

	length = 0;
	
	ChunkReadInt(num_morph_meshes, chunkadr, length);

	for (i = 0; i < num_morph_meshes; i++)
	{
		CMorphMesh* morph_mesh = new CMorphMesh();
		int num_targets;
		int num_vertices;
		int num_normals;
		char name[256];

		strcpy(morph_mesh->Filename, CMorphData::MorphData->Filename);
		strcpy(morph_mesh->FullPathName, CMorphData::MorphData->FullPathName);

		// Read in the mesh's name

		ChunkReadString(name, &dummy, &chunkadr, &length);
		morph_mesh->SetName(name);
		morph_mesh->GetBase().SetName(name);

		// Read in the number of vertices

		ChunkReadInt(num_vertices, chunkadr, length);
		morph_mesh->GetBase().SetNumVertices(num_vertices);

		for (i = 0; i < num_vertices; i++)
		{
			float x, y, z;

			ChunkReadFloat(x, chunkadr, length);
			ChunkReadFloat(y, chunkadr, length);
			ChunkReadFloat(z, chunkadr, length);

			morph_mesh->GetBase().Vertex(i) = CMorphTgtRecord(x, y, z);
		}

		// Read in the number of normals

		ChunkReadInt(num_normals, chunkadr, length);
		morph_mesh->GetBase().SetNumNormals(num_normals);

		for (i = 0; i < num_normals; i++)
		{
			float x, y, z;

			ChunkReadFloat(x, chunkadr, length);
			ChunkReadFloat(y, chunkadr, length);
			ChunkReadFloat(z, chunkadr, length);

			morph_mesh->GetBase().Normal(i) = CMorphTgtRecord(x, y, z);
		}

		// Read in the number of target meshes

		ChunkReadInt(num_targets, chunkadr, length);
		
		// Read in the target meshes

		for (i = 0; i < num_targets; i++)
		{
			int j;
			
			CMorphTgt *target = new CMorphTgt;
			target->SetNumVertices(num_vertices);
			target->SetNumNormals(num_normals);

			// Read in the name

			ChunkReadString(name, &dummy, &chunkadr, &length);
			target->SetName(name);

			// Read in the vertices

			for (j = 0; j < num_vertices; j++)
			{
				float x, y, z;

				ChunkReadFloat(x, chunkadr, length);
				ChunkReadFloat(y, chunkadr, length);
				ChunkReadFloat(z, chunkadr, length);

				target->Vertex(j) = CMorphTgtRecord(x, y, z);
			}

			// Read in the normals

			for (j = 0; j < num_normals; j++)
			{
				float x, y, z;

				ChunkReadFloat(x, chunkadr, length);
				ChunkReadFloat(y, chunkadr, length);
				ChunkReadFloat(z, chunkadr, length);

				target->Normal(j) = CMorphTgtRecord(x, y, z);
			}

			// Add this target to the mesh

			morph_mesh->AddTarget(target);
		}

		// Add mesh to morph data

		CMorphData::MorphData->AddMorphMesh(morph_mesh);

		// Fix up morph mesh for current character/skin


		// Find Matching Skin
			
		CCharacter * pC;
		pC = GetDocument()->GetCharacter();		

		CSkin* pSkin;
		pSkin = GetDocument()->GetCharacter()->skin;
		
		POSITION pos;
		pos = GetDocument()->GetData()->SkinList.GetHeadPosition();

		CString mn = morph_mesh->Filename;
		mn = mn.Left(mn.Find("."));

		while (pos)
		{
			CSkin* ps;
			CString sn;

			ps = GetDocument()->GetData()->SkinList.GetAt(pos);

			sn = ps->fname;
			sn = sn.Left(sn.Find("."));
			sn.MakeUpper();
			mn.MakeUpper();

			if(sn == mn)
			{
				pSkin = ps;
				break;
			}
			
			GetDocument()->GetData()->SkinList.GetNext(pos);
		}

		morph_mesh->Transform(pSkin);
	}

	return (length + CHUNK_HEADER_SIZE);
}


////////////////////////////////////////////////////////////////////////////////

int	ChunkRead_MRAN(char *chunkadr, int chunklen)
{
	int length = 0;
	int num_objects, num_targets, num_frames, temp;
	int i, j, k;
	char name[256];

	ChunkReadInt(num_objects, chunkadr, length);

	for (i = 0; i < num_objects; i++)
	{
		CMorphSequence *morph_sequence = new CMorphSequence;

		strcpy(morph_sequence->Filename, CMorphData::MorphData->Filename);
		strcpy(morph_sequence->FullPathName, CMorphData::MorphData->FullPathName);

		{
			char drive[ _MAX_DRIVE];
			char dir[ _MAX_DIR];
			char name[ _MAX_FNAME];
			char ext[ _MAX_EXT];
			_splitpath(CMorphData::MorphData->Filename,drive, dir, name, ext);
			morph_sequence->SetName(name);
		}


		ChunkReadString(name, &temp, &chunkadr, &length);
		morph_sequence->SetBaseName(name);

		ChunkReadInt(num_frames, chunkadr, length);	
		morph_sequence->GetEnvelopeTrack()->SetNumTrackValues(num_frames);

		ChunkReadInt(temp, chunkadr, length);

		if (temp)
		{
			for (j = 0; j < num_frames; j++)
			{
				float value;

				ChunkReadFloat(value, chunkadr, length);
				morph_sequence->GetEnvelopeTrack()->SetTrackValue(j, value);
			}
		}

		ChunkReadInt(num_targets, chunkadr, length);

		for (j = 0; j < num_targets; j++)
		{
			CMorphTrack* track = new CMorphTrack();

			track->SetNumTrackValues(num_frames);

			ChunkReadString(name, &temp, &chunkadr, &length);

			ChunkReadInt(temp, chunkadr, length);

			if (temp)
			{
				for (k = 0; k < num_frames; k++)
				{
					float value;

					ChunkReadFloat(value, chunkadr, length);
					track->SetTrackValue(k, value);
				}
			}
			else
			{
				for (k = 0; k < num_frames; k++) track->SetTrackValue(k, 0.0f);
			}

			morph_sequence->AddTarget(name, track);
		}

		const char* unique_name = morph_sequence->GetUniqueName();

		morph_sequence->SetMorphMesh(CMorphData::MorphData->GetMorphMeshUnique(unique_name));

		if (!morph_sequence->GetMorphMesh())
		{
			delete morph_sequence;
			return (chunklen + CHUNK_HEADER_SIZE);
		}

		morph_sequence->ConvertTracks();

		CMorphData::MorphData->AddMorphSequence(morph_sequence);
	}

	return (length + CHUNK_HEADER_SIZE);
}

////////////////////////////////////////////////////////////////////////////////
// Some helper functions
////////////////////////////////////////////////////////////////////////////////

static inline char* CopyString(const char* string)
{
	char* output;

	if (!string) return NULL;

	output = new char[strlen(string) + 1];
	if (!output) return NULL;

	strcpy(output, string);

	return output;
}

//------------------------------------------------------------------------------

static inline void DeleteString(char*& string)
{
	if (string) delete[] string;

	string = NULL;
}

//------------------------------------------------------------------------------

static inline void WriteString(CArchive& ar, const char* string)
{
	if (string)
	{
		int length = strlen(string);

		ar << length;
		ar.Write(string, length);
	}
	else
	{
		ar << 0;
	}
}

//------------------------------------------------------------------------------

static inline char* ReadString(CArchive& ar)
{
	char* output;
	int length;

	ar >> length;

	if (!length) return NULL;

	output = new char[length + 1];
	ar.Read(output, length);
	output[length] = '\0';

	return output;
}

//------------------------------------------------------------------------------

template<class T>
static inline T* CopyArray(const T* array, int length)
{
	if (!length) return NULL;

	T* output = new T[length];

	if (array) memcpy(output, array, sizeof(T) * length);

	return output;
}

//------------------------------------------------------------------------------

template<class T>
static inline void DeleteArray(T*& array)
{
	if (array) delete[] array;

	array = NULL;
}

//------------------------------------------------------------------------------

template<class T>
static inline void CopyCArray(CArray<T*, T*>& dest, const CArray<T*, T*>& source)
{
	int i;

	dest.RemoveAll();

	for (i = 0; i < source.GetSize(); i++)
	{
		dest.Add(new T(*source[i]));
	}
}

//------------------------------------------------------------------------------

template<class T>
static inline void DeleteCArray(CArray<T*, T*>& array)
{
	int i;

	for (i = 0; i < array.GetSize(); i++)
	{
		if (array[i]) delete array[i];
	}

	array.RemoveAll();
}

////////////////////////////////////////////////////////////////////////////////

CMorphTgtRecord::CMorphTgtRecord()
{
	x = y = z = 0.0f;
}

//------------------------------------------------------------------------------

CMorphTgtRecord::CMorphTgtRecord(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

//------------------------------------------------------------------------------

CMorphTgtRecord::CMorphTgtRecord(const CMorphTgtRecord& record)
{
	x = record.x;
	y = record.y;
	z = record.z;
}

//------------------------------------------------------------------------------

void CMorphTgtRecord::Clear()
{
	x = y = z = 0.0f;
}

//------------------------------------------------------------------------------

CMorphTgtRecord CMorphTgtRecord::operator=(const CMorphTgtRecord& record)
{
	x = record.x;
	y = record.y;
	z = record.z;

	return *this;
}

//------------------------------------------------------------------------------

void CMorphTgtRecord::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << x << y << z;
	}
	else
	{
		ar >> x >> y >> z;
	}
}

////////////////////////////////////////////////////////////////////////////////

CMorphTgt::CMorphTgt()
{
	Name = NULL;

	NumVertices = 0;
	Vertices = NULL;

	NumNormals = 0;
	Normals = NULL;
}

//------------------------------------------------------------------------------

CMorphTgt::CMorphTgt(const CMorphTgt& target)
{
	Name = CopyString(target.Name);

	NumVertices = target.NumVertices;
	Vertices = CopyArray<CMorphTgtRecord>(target.Vertices, NumVertices);

	NumNormals = target.NumNormals;
	Normals = CopyArray<CMorphTgtRecord>(target.Normals, NumNormals);
}

//------------------------------------------------------------------------------

CMorphTgt::~CMorphTgt()
{
	Clear();
}

//------------------------------------------------------------------------------

void CMorphTgt::Clear()
{
	DeleteString(Name);

	NumVertices = 0;
	DeleteArray<CMorphTgtRecord>(Vertices);

	NumNormals = 0;
	DeleteArray<CMorphTgtRecord>(Normals);
}

//------------------------------------------------------------------------------

CMorphTgt CMorphTgt::operator=(const CMorphTgt& target)
{
	Clear();

	Name = CopyString(target.Name);

	NumVertices = target.NumVertices;
	Vertices = CopyArray<CMorphTgtRecord>(target.Vertices, NumVertices);

	NumNormals = target.NumNormals;
	Normals = CopyArray<CMorphTgtRecord>(target.Normals, NumNormals);

	return *this;
}

//------------------------------------------------------------------------------

void CMorphTgt::SetName(const char* name)
{
	DeleteString(Name);

	Name = CopyString(name);
}
//------------------------------------------------------------------------------

const char* CMorphTgt::GetName()
{
	return Name;
}

//------------------------------------------------------------------------------

void CMorphTgt::SetNumVertices(int count)
{
	DeleteArray<CMorphTgtRecord>(Vertices);

	NumVertices = count;
	Vertices = CopyArray<CMorphTgtRecord>(NULL, NumVertices);
}

//------------------------------------------------------------------------------

int CMorphTgt::GetNumVertices()
{
	return NumVertices;
}

//------------------------------------------------------------------------------

const CMorphTgtRecord& CMorphTgt::Vertex(int index) const
{
	return Vertices[index];
}

//------------------------------------------------------------------------------

CMorphTgtRecord& CMorphTgt::Vertex(int index)
{
	return Vertices[index];
}

//------------------------------------------------------------------------------

void CMorphTgt::SetNumNormals(int count)
{
	DeleteArray<CMorphTgtRecord>(Normals);

	NumNormals = count;
	Normals = CopyArray<CMorphTgtRecord>(NULL, NumNormals);
}

//------------------------------------------------------------------------------

int CMorphTgt::GetNumNormals()
{
	return NumNormals;
}

//------------------------------------------------------------------------------

const CMorphTgtRecord& CMorphTgt::Normal(int index) const
{
	return Normals[index];
}

//------------------------------------------------------------------------------

CMorphTgtRecord& CMorphTgt::Normal(int index)
{
	return Normals[index];
}

//------------------------------------------------------------------------------

void CMorphTgt::Serialize(CArchive& ar)
{
	int i;

	if (ar.IsStoring())
	{
		WriteString(ar, Name);

		ar << NumVertices;
		for (i = 0; i < NumVertices; i++) Vertices[i].Serialize(ar);

		ar << NumNormals;
		for (i = 0; i < NumNormals; i++) Normals[i].Serialize(ar);
	}
	else
	{
		Clear();

		Name = ReadString(ar);

		ar >> NumVertices;
		for (i = 0; i < NumVertices; i++) Vertices[i].Serialize(ar);

		ar >> NumNormals;
		for (i = 0; i < NumNormals; i++) Normals[i].Serialize(ar);
	}
}

////////////////////////////////////////////////////////////////////////////////

CMorphVertex::CMorphVertex()
{
	vx = vy = vz = 0.0f;
	nx = ny = nz = 0.0f;
	u = v = 0.0f;
}

//------------------------------------------------------------------------------

CMorphVertex::CMorphVertex(float vx, float vy, float vz, float nx, float ny, float nz, float u, float v)
{
	this->vx = vx;
	this->vy = vy;
	this->vz = vz;
	this->nx = nx;
	this->ny = ny;
	this->nz = nz;
	this->u = u;
	this->v = v;
}

//------------------------------------------------------------------------------

CMorphVertex::CMorphVertex(const CMorphVertex& vertex)
{
	vx = vertex.vx;
	vy = vertex.vy;
	vz = vertex.vz;
	nx = vertex.nx;
	ny = vertex.ny;
	nz = vertex.nz;
	u = vertex.u;
	v = vertex.v;
}

//------------------------------------------------------------------------------

void CMorphVertex::Clear()
{
	vx = vy = vz = 0.0f;
	nx = ny = nz = 0.0f;
	u = v = 0.0f;
}

//------------------------------------------------------------------------------

CMorphVertex CMorphVertex::operator=(const CMorphVertex& vertex)
{
	vx = vertex.vx;
	vy = vertex.vy;
	vz = vertex.vz;
	nx = vertex.nx;
	ny = vertex.ny;
	nz = vertex.nz;
	u = vertex.u;
	v = vertex.v;

	return *this;
}

//------------------------------------------------------------------------------

void CMorphVertex::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << vx << vy << vz;
		ar << nx << ny << nz;
		ar << u << v;
	}
	else
	{
		ar >> vx >> vy >> vz;
		ar >> nx >> ny >> nz;
		ar >> u >> v;
	}
}

////////////////////////////////////////////////////////////////////////////////

CMorphVertexArray::CMorphVertexArray(int count)
{
	if (count < 0) count = 0;

	NumVertices = count;

	Vertices = CopyArray<CMorphVertex>(NULL, NumVertices);
}

//------------------------------------------------------------------------------

CMorphVertexArray::CMorphVertexArray(const CMorphVertexArray& array)
{
	NumVertices = array.NumVertices;

	Vertices = CopyArray<CMorphVertex>(array.Vertices, NumVertices);
}

//------------------------------------------------------------------------------

CMorphVertexArray::~CMorphVertexArray()
{
	Clear();
}

//------------------------------------------------------------------------------

void CMorphVertexArray::Clear()
{
	if (NumVertices)
	{
		delete[] Vertices;
		NumVertices = 0;
	}
}

//------------------------------------------------------------------------------

CMorphVertexArray CMorphVertexArray::operator=(const CMorphVertexArray& array)
{
	Clear();

	NumVertices = array.NumVertices;

	Vertices = CopyArray<CMorphVertex>(array.Vertices, NumVertices);

	return *this;
}

//------------------------------------------------------------------------------

int CMorphVertexArray::GetNumVertices()
{
	return NumVertices;
}


//------------------------------------------------------------------------------
CMorphVertex* CMorphVertexArray::GetVertices()
{
	return Vertices;
}

//------------------------------------------------------------------------------

const CMorphVertex& CMorphVertexArray::operator[](int index) const
{
	return Vertices[index];
}

//------------------------------------------------------------------------------

CMorphVertex& CMorphVertexArray::operator[](int index)
{
	return Vertices[index];
}

//------------------------------------------------------------------------------

void CMorphVertexArray::Serialize(CArchive& ar)
{
	int i;

	if (ar.IsStoring())
	{
		ar << NumVertices;
		for (i = 0; i < NumVertices; i++) Vertices[i].Serialize(ar);
	}
	else
	{
		Clear();

		ar >> NumVertices;
		for (i = 0; i < NumVertices; i++) Vertices[i].Serialize(ar);
	}
}

////////////////////////////////////////////////////////////////////////////////

CMorphMesh::CMorphMesh()
{
	Name = NULL;
	UniqueName = NULL;

	BaseVertices = CMorphVertexArray(0);
	TargetVertices.RemoveAll();

	Base.Clear();
	Targets.RemoveAll();

	strcpy(Filename, "");
	strcpy(FullPathName, "");
}

//------------------------------------------------------------------------------

CMorphMesh::CMorphMesh(const CMorphMesh& mesh)
{
	Name = CopyString(mesh.Name);
	UniqueName = CopyString(mesh.UniqueName);

	BaseVertices = mesh.BaseVertices;
	CopyCArray<CMorphVertexArray>(TargetVertices, mesh.TargetVertices);

	Base = mesh.Base;
	CopyCArray<CMorphTgt>(Targets, mesh.Targets);
}

//------------------------------------------------------------------------------

CMorphMesh::~CMorphMesh()
{
	Clear();
}

//------------------------------------------------------------------------------

void CMorphMesh::Clear()
{
	DeleteString(Name);
	DeleteString(UniqueName);

	BaseVertices = CMorphVertexArray(0);
	DeleteCArray<CMorphVertexArray>(TargetVertices);

	Base.Clear();
	DeleteCArray<CMorphTgt>(Targets);
}

//------------------------------------------------------------------------------

CMorphMesh CMorphMesh::operator=(const CMorphMesh& mesh)
{
	Clear();

	Name = CopyString(mesh.Name);
	UniqueName = CopyString(mesh.UniqueName);

	BaseVertices = mesh.BaseVertices;
	CopyCArray<CMorphVertexArray>(TargetVertices, mesh.TargetVertices);

	Base = mesh.Base;
	CopyCArray<CMorphTgt>(Targets, mesh.Targets);

	return *this;
}

//------------------------------------------------------------------------------

void CMorphMesh::SetName(const char* name)
{
	DeleteString(Name);
	Name = CopyString(name);
}

//------------------------------------------------------------------------------

const char* CMorphMesh::GetName()
{
	return Name;
}

//------------------------------------------------------------------------------

const char* CMorphMesh::GetUniqueName()
{
	if (UniqueName) return UniqueName;

	// Build up a unique name from the list of meshes used in this morph mesh
	// e.g. if the base is LARA_FACE and the rest are LARA_OOHH, LARA_AAHH and
	// LARA_EEKK, then the final name will be LARA_FACE_OOHH_AAHH_EEKK, a pretty
	// unique name used to identify this morph set.

	CString unique(Name);
	int i;

	for (i = 0; i < Targets.GetSize(); i++)
	{
		CString name(Targets[i]->GetName());

		int index = name.Find('_');
		if (index != -1) name = name.Mid(index);

		unique += name;
	}

	UniqueName = CopyString(unique);

	return UniqueName;
}

//------------------------------------------------------------------------------

const CMorphVertexArray& CMorphMesh::GetBaseVertices() const
{
	return BaseVertices;
}

//------------------------------------------------------------------------------

CMorphVertexArray& CMorphMesh::GetBaseVertices()
{
	return BaseVertices;
}

//------------------------------------------------------------------------------

const CMorphVertexArray& CMorphMesh::GetTargetVertices(int index) const
{
	return *TargetVertices[index];
}

//------------------------------------------------------------------------------

CMorphVertexArray& CMorphMesh::GetTargetVertices(int index)
{
	return *TargetVertices[index];
}

//------------------------------------------------------------------------------

const CMorphTgt& CMorphMesh::GetBase() const
{
	return Base;
}

//------------------------------------------------------------------------------

CMorphTgt& CMorphMesh::GetBase()
{
	return Base;
}

//------------------------------------------------------------------------------

int CMorphMesh::AddTarget(CMorphTgt* target)
{
	DeleteString(UniqueName); // Delete the unique name as it is now not valid

	return Targets.Add(target);
}

//------------------------------------------------------------------------------

const CMorphTgt& CMorphMesh::GetTarget(int index) const
{
	return *Targets[index];
}

//------------------------------------------------------------------------------

CMorphTgt& CMorphMesh::GetTarget(int index)
{
	return *Targets[index];
}

//------------------------------------------------------------------------------

void CMorphMesh::Serialize(CArchive& ar)
{
	int i;

	if (ar.IsStoring())
	{
		WriteString(ar, Name);
		WriteString(ar, UniqueName);

		BaseVertices.Serialize(ar);

		ar << TargetVertices.GetSize();
		for (i = 0; i < TargetVertices.GetSize(); i++) TargetVertices[i]->Serialize(ar);

		Base.Serialize(ar);

		ar << Targets.GetSize();
		for (i = 0; i < Targets.GetSize(); i++) Targets[i]->Serialize(ar);
	}
	else
	{
		Clear();

		Name = ReadString(ar);
		UniqueName = ReadString(ar);

		BaseVertices.Serialize(ar);

		int count;

		ar >> count;
		for (i = 0; i < count; i++)
		{
			CMorphVertexArray* array = new CMorphVertexArray();

			array->Serialize(ar);

			TargetVertices.Add(array);
		}

		Base.Serialize(ar);

		ar >> count;
		for (i = 0; i < count; i++)
		{
			CMorphTgt* array = new CMorphTgt();

			array->Serialize(ar);

			Targets.Add(array);
		}
	}
}

//------------------------------------------------------------------------------

void CMorphMesh::TextSerialize(CTextSave& io)
{
	if (io.IsStoring())
	{
		io.BeginChunk("Morph Target");

		io << io.Label("Version") << 1;
		io << io.Label("FileName") << this->FullPathName;

		io.EndChunk();
	}
	else
	{
	}
}

//------------------------------------------------------------------------------

void CMorphMesh::Transform(class CSkin* skin)
{
	int i, j, k;

	if (!skin) return;

	unsigned int hash = STRING_GetHashValue((char*)Base.GetName());

	for (i = 0; i < skin->nParented; i++)
	{
		if (skin->parentedobjs[i].meshhash != hash) continue;

		CBone* bone = skin->Skeleton->GetBone(skin->parentedobjs[i].bonehash);

		if (!bone)
		{
			Log("***** Error Bone Not Found");
			continue;
		}

		CMeshInfo* tm = skin->Mesh->GetMesh(skin->parentedobjs[i].meshhash);

		if (!tm->vertex_indices)
		{
			Log("***** Error No remapping indices");
			continue;
		}

		int num_vertices = tm->nverts;

		BaseVertices = CMorphVertexArray(num_vertices);

		for (j = 0; j < num_vertices; j++)
		{
			CMorphTgtRecord& vertex_record = Base.Vertex(tm->vertex_indices[j].iv);
			CMorphTgtRecord& normal_record = Base.Normal(tm->vertex_indices[j].in);
			float u = tm->uvs[j * 2 + 0];
			float v = tm->uvs[j * 2 + 1];

			CVector vertex(vertex_record.x, vertex_record.y, vertex_record.z);
			vertex = vertex  * tm->matrix * bone->iabsbindmatrix();

			BaseVertices[j] = CMorphVertex(vertex.x, vertex.y, vertex.z, normal_record.x, normal_record.y, normal_record.z, u, v);
		}

		DeleteCArray<CMorphVertexArray>(TargetVertices);

		for (j = 0; j < Targets.GetSize(); j++)
		{
			CMorphVertexArray* target = new CMorphVertexArray(num_vertices);

			for (k = 0; k < num_vertices; k++)
			{
				CMorphTgtRecord& vertex_record = Targets[j]->Vertex(tm->vertex_indices[k].iv);
				CMorphTgtRecord& normal_record = Targets[j]->Normal(tm->vertex_indices[k].in);
				float u = tm->uvs[k * 2 + 0];
				float v = tm->uvs[k * 2 + 1];

				CVector vertex(vertex_record.x, vertex_record.y, vertex_record.z);

				vertex = vertex  * tm->matrix * bone->iabsbindmatrix();

				(*target)[k] = CMorphVertex(vertex.x, vertex.y, vertex.z, normal_record.x, normal_record.y, normal_record.z, u, v);
			}

			TargetVertices.Add(target);
		}

		break;
	}
}

//------------------------------------------------------------------------------

void CMorphMesh::ExportToGame(void **data)
{
	MORPH_MESH *base;
	CMorphVertex* base_vertices;
	CMorphVertex* vertices;
	int i, j;

	base = new MORPH_MESH;
	base->Id = STRING_GetHashValue((char*)GetUniqueName());
	base->BaseId = STRING_GetHashValue((char*)GetName());

	int num_vertices = BaseVertices.GetNumVertices();
	int num_targets = TargetVertices.GetSize();

	base->NumVertices = num_vertices;
	base->NumTargets = num_targets;
	base->Vertices = new float[num_vertices * 8 * (1 + base->NumTargets)];

	base_vertices = BaseVertices.GetVertices();
	float* vertex_ptr = base->Vertices;

	for (i = 0; i < num_vertices; i++)
	{
		vertex_ptr[0] = base_vertices[i].vx;
		vertex_ptr[1] = base_vertices[i].vy;
		vertex_ptr[2] = base_vertices[i].vz;
		vertex_ptr[3] = base_vertices[i].nx;
		vertex_ptr[4] = base_vertices[i].ny;
		vertex_ptr[5] = base_vertices[i].nz;
		vertex_ptr[6] = base_vertices[i].u;
		vertex_ptr[7] = base_vertices[i].v;

		vertex_ptr += 8;

		for (j = 0; j < num_targets; j++, vertex_ptr += 8)
		{
			vertices = TargetVertices[j]->GetVertices();

			vertex_ptr[0] = vertices[i].vx - base_vertices[i].vx;
			vertex_ptr[1] = vertices[i].vy - base_vertices[i].vy;
			vertex_ptr[2] = vertices[i].vz - base_vertices[i].vz;
			vertex_ptr[3] = vertices[i].nx - base_vertices[i].nx;
			vertex_ptr[4] = vertices[i].ny - base_vertices[i].ny;
			vertex_ptr[5] = vertices[i].nz - base_vertices[i].nz;
			vertex_ptr[6] = vertices[i].u;
			vertex_ptr[7] = vertices[i].v;
		}
	}

	*data = base;
}

////////////////////////////////////////////////////////////////////////////////

CMorphSequence::CMorphSequence()
{
	Name = NULL;
	BaseName = NULL;
	UniqueName = NULL;
	MorphMesh = NULL;

	EnvelopeTrack.SetNumTrackValues(0);

	TargetNames.RemoveAll();
	TargetTracks.RemoveAll();
}

//------------------------------------------------------------------------------

CMorphSequence::~CMorphSequence()
{
	Clear();
}

//------------------------------------------------------------------------------

void CMorphSequence::Clear()
{
	DeleteString(Name);
	DeleteString(BaseName);
	DeleteString(UniqueName);
	MorphMesh = NULL;

	EnvelopeTrack.SetNumTrackValues(0);

	TargetNames.RemoveAll();

	for (int i = 0; i < this->TargetTracks.GetSize(); i++)
	{
		delete TargetTracks[i];
	}

	TargetTracks.RemoveAll();
}

//------------------------------------------------------------------------------

void CMorphSequence::SetName(const char* name)
{
	DeleteString(Name);
	Name = CopyString(name);
}

//------------------------------------------------------------------------------

const char* CMorphSequence::GetName()
{
	return Name;
}

//------------------------------------------------------------------------------

void CMorphSequence::SetBaseName(const char* name)
{
	DeleteString(BaseName);
	BaseName = CopyString(name);
}

//------------------------------------------------------------------------------

const char* CMorphSequence::GetBaseName()
{
	return BaseName;
}

//------------------------------------------------------------------------------

const char* CMorphSequence::GetUniqueName()
{
	if (UniqueName) return UniqueName;

	// Build up a unique name from the list of meshes used in this morph mesh
	// e.g. if the base is LARA_FACE and the rest are LARA_OOHH, LARA_AAHH and
	// LARA_EEKK, then the final name will be LARA_FACEOOHHAAHHEEKK, a pretty
	// unique name used to identify this morph set.

	CString unique(BaseName);
	int i;

	for (i = 0; i < TargetNames.GetSize(); i++)
	{
		CString name(TargetNames[i]);

		int index = name.Find('_');
		if (index != -1) name = name.Mid(index);
		else name = "_" + name;

		unique += name;
	}

	UniqueName = CopyString(unique);

	return UniqueName;
}

//------------------------------------------------------------------------------

void CMorphSequence::SetMorphMesh(CMorphMesh* morph_mesh)
{
	MorphMesh = morph_mesh;
}

//------------------------------------------------------------------------------

CMorphMesh* CMorphSequence::GetMorphMesh()
{
	return MorphMesh;
}

//------------------------------------------------------------------------------

int CMorphSequence::GetNumFrames()
{
	return EnvelopeTrack.GetNumTrackValues();
}

//------------------------------------------------------------------------------

int CMorphSequence::GetNumTargets()
{
	return TargetTracks.GetSize();
}

//------------------------------------------------------------------------------

int CMorphSequence::AddTarget(const char* name, CMorphTrack* target)
{
	char* name_copy = CopyString(name);
	TargetNames.Add(name_copy);
	return TargetTracks.Add(target);
}

//------------------------------------------------------------------------------

CMorphTrack* CMorphSequence::GetTargetTrack(int index)
{
	return TargetTracks[index];
}

//------------------------------------------------------------------------------

CMorphTrack* CMorphSequence::GetEnvelopeTrack()
{
	return &EnvelopeTrack;
}

//------------------------------------------------------------------------------

void CMorphSequence::ConvertTracks()
{
	int i, j;

	int size = 0;

	int num_frames = GetNumFrames();
	int num_targets = GetNumTargets();

	// setup points on curve

	VECTORXY *xy = new VECTORXY[num_frames];

	for (i = 0; i < num_frames; i++)
	{
		xy[i].x = (float)i;
		xy[i].y = GetEnvelopeTrack()->GetTrackValue(i);
	}

	// Fit envelope curve

	CFitCurve* envelope_curve = new CFitCurve;
	CMorphTrack* envelope_track = GetEnvelopeTrack();

	envelope_track->NumCurves = envelope_curve->GenerateCurve(xy, num_frames, &envelope_track->CurveData, &envelope_track->FrameData);	
	envelope_track->AnimBlock = CreateAnimStream(envelope_track->NumCurves, envelope_track->FrameData, envelope_track->CurveData, envelope_track->GetTrackValues(), &envelope_track->AnimSize, MORPH_ENVELOPE_TRACK_SCALE);

	delete envelope_curve;

	size +=  envelope_track->AnimSize;

	// Fit wieght curves

	for (i = 0; i < num_targets; i++)
	{
		// setup points on curve
		
		for (j = 0; j < num_frames; j++)
		{
			xy[j].x = (float)j;
			xy[j].y = GetTargetTrack(i)->GetTrackValue(j);
		}
	
		CFitCurve* weight_curve = new CFitCurve;
		CMorphTrack* weight_track = GetTargetTrack(i);

		weight_track->NumCurves = weight_curve->GenerateCurve(xy, num_frames, &weight_track->CurveData, &weight_track->FrameData);	
		weight_track->AnimBlock = CreateAnimStream(weight_track->NumCurves, weight_track->FrameData, weight_track->CurveData, weight_track->GetTrackValues(), &weight_track->AnimSize, MORPH_WEIGHT_TRACK_SCALE);

		delete weight_curve;
		
		size +=  weight_track->AnimSize;
	}

	delete[] xy;
}

//------------------------------------------------------------------------------

void CMorphSequence::Serialize(CArchive &ar)
{
}

//------------------------------------------------------------------------------

void CMorphSequence::TextSerialize(CTextSave& io)
{
	if (io.IsStoring())
	{
		io.BeginChunk("Morph Sequence");

		io << io.Label("Version") << 1;
		io << io.Label("FileName") << this->FullPathName;

		io.EndChunk();
	}
	else
	{
	}
}

//------------------------------------------------------------------------------

void CMorphSequence::ExportToGame(void **data)
{
	MORPH_SEQUENCE *sequence;
	int i;

	int num_frames = GetNumFrames();
	int num_targets = GetNumTargets();

	sequence = new MORPH_SEQUENCE;

	sequence->Id = STRING_GetHashValue((char*)GetName());

	sequence->TargetId = STRING_GetHashValue((char*)GetUniqueName());
	MorphMesh->ExportToGame((void **)&sequence->MorphMesh);

	sequence->NumFrames = num_frames;
	sequence->NumTracks = num_targets;

	// setup envelope track
	sequence->Envelope = new MORPH_SEQUENCE_TRACK;

	CMorphTrack *track = GetEnvelopeTrack();
	sequence->Envelope->AnimBlock = new unsigned char[track->AnimSize];
	sequence->Envelope->AnimSize = track->AnimSize;
	memcpy(sequence->Envelope->AnimBlock, track->AnimBlock, track->AnimSize);

	// setup tracks
	sequence->Tracks = new MORPH_SEQUENCE_TRACK[num_targets];

	for (i = 0; i < GetNumTargets(); i++)
	{
		CMorphTrack *track = this->GetTargetTrack(i);

		sequence->Tracks[i].AnimBlock = new unsigned char[track->AnimSize];
		sequence->Tracks[i].AnimSize = track->AnimSize;

		memcpy(sequence->Tracks[i].AnimBlock, track->AnimBlock, track->AnimSize);
	}

	*data = sequence;
}

////////////////////////////////////////////////////////////////////////////////
