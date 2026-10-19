#ifndef _ANIMAPI_H_
#define _ANIMAPI_H_

#include "maths.h"
#include "typedefs.h"

//----------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------

#define GAME_ANIMATION_VERSION		17


#define ANIMATION_PLAYBACK_DEFINED
#define ANIMOBJECT_DEFINED


#define ANIMCTRL_MAXOVERLAY		2



#define MAX_MORPH_MESHES 		4
#define MAX_MORPH_SEQUENCES		512
#define MAX_MORPH_TARGETS 		7




//----------------------------------------------------------------------------------------------------

typedef unsigned char	BYTE;

//----------------------------------------------------------------------------------------------------

#define	CHAR_NAME_LEN	64

// Animation Track Order


enum
{
    AT_TX,
	AT_TY,
	AT_TZ,
	AT_RX,
	AT_RY,
	AT_RZ,
	AT_SX,
	AT_SY,
	AT_SZ,
	AT_RW,
	AT_TOTAL,
};


#if defined( _GAME_PC_)
	#include "sys_memory.h"

	#if defined( _DEBUG)
		#define		memalloc(a)					sysAllocMem_CallerInfo(a, __FILE__, __LINE__)
	#else
		#define		memalloc(a)					sysAllocMem(a)
	#endif
	#define		memrealloc(a,b)				sysReAllocMem(a,b)
	#define		memclear(a,b)				sysClearMem(a,b)
	#define		memfree(a)					sysFreeMem(a)
	#define		memcopy(a,b,c)				sysCopyMem(a,b,c)
#elif defined WIN32 && !defined WORLDEDIT
	#define		memalloc(a)					malloc(a)
	#define		memrealloc(a,b)				realloc(a,b)
	#define		memclear(a,b)				memset(a,0,b)
	#define		memfree(a)					free(a)
	#define		memcopy(a,b,c)				memcpy(a,b,c)
#else

	#include "sys_draw.h"
	#include "sys_memory.h"

	#define		memalloc(a)					sysAllocMem(a)
	#define		memrealloc(a,b)				sysReAllocMem(a,b)
	#define		memclear(a,b)				sysClearMem(a,b)
	#define		memfree(a)					sysFreeMem(a)
	#define		memcopy(a,b,c)				sysCopyMem(a,b,c)

#endif

#ifdef __cplusplus
extern "C" {
#endif

//#define		TARGET_FPS					60.0f
//#define		TARGET_MS					(1000.0f/TARGET_FPS)
//#define		TARGET_MS_SQUARED			(TARGET_MS * TARGET_MS)

//#define		ANIM_DEFAULT_FPS			30.0f
//#define		ANIM_DEFAULT_MSPERFRAME		(1000.0f/ANIM_DEFAULT_FPS)
//#define		ANIM_DEFAULT_MSBLENDTIME	(2.0f*ANIM_DEFAULT_MSPERFRAME)


#define		ANIM_ONESECOND				30.0f
#define		ANIM_FPS					30.0f



float		APB_FrameToMS(float a);
float		APB_MSToFrame(float a);
void		AnimApiInit(float fps);

extern float gAnimPlayFPS;
extern float gAnimTargetMS;
extern float gAnimTargetMSSquared;
extern float gAnimDefaultMS;
extern float gAnimDefaultMSBlend;



#define		ANIM_EMPTY		0
#define		ANIM_BEZIER		(1<<0)
#define		ANIM_CONST		(1<<1)
#define		ANIM_KEYS		(1<<2)
#define		ANIM_LASTBLOCK	(1<<3)

#define		ANIM_RSCALE		(16383.0f)
#define		ANIM_TSCALE		(4.0f)
#define		ANIM_SSCALE		(255.0f)


#define		ANIM_SETRSCALE		(16383.0f)
#define		ANIM_SETTSCALE		(1.0f)
#define		ANIM_SETSSCALE		(255.0f)


#define		ANIM_VTSCALE	(32.0f)
#define		ANIM_VRSCALE	(512.0f)


enum
{
	TRACK_KEY_CODE_MAKE_INVISIBLE=0,
	TRACK_KEY_CODE_MAKE_VISIBLE,
	TRACK_KEY_CODE_FIRE_AUDIO_EVENT,
	TRACK_KEY_CODE_FIRE_ANIM_TRIGGER,
	TRACK_KEY_CODE_FIRE_MORPH_TRIGGER,
	TRACK_KEY_CODE_FIRE_FXEVENT_ON,
	TRACK_KEY_CODE_FIRE_FXEVENT_OFF,
	TRACK_KEY_CODE_FIRE_FXEVENT_ONESHOT,
};


//----------------------------------------------------------------------------------------------------

typedef struct
{
	uint16 frame;
	uint8 word;
	uint8 size;
}ANIMBLOCK_HEADER;

// Animation Track

#define ANIMTRACK_HASTX		1
#define ANIMTRACK_HASTY		2
#define ANIMTRACK_HASTZ		4
#define ANIMTRACK_HASRX		8
#define ANIMTRACK_HASRY		16
#define ANIMTRACK_HASRZ		32
#define ANIMTRACK_HASSX		64
#define ANIMTRACK_HASSY		128
#define ANIMTRACK_HASSZ		256
#define ANIMTRACK_HASRW		512
#define	ANIMTRACK_HASSSC	1024


	// DLH - Added as the game is also a WIN32 application
#if defined( WIN32) && !defined( _GAME_PC_)
	#define ANIMATION_TRACK_USE_ANIMSIZE
#endif

typedef struct
{
	uint32	flags;						// Animation Track Flags
	uint8**	animblock;					// Animation Tracks

	#if defined( ANIMATION_TRACK_USE_ANIMSIZE)
		uint32*	animsize;					// Animation Track Size
	#endif
}ANIMATION_TRACK;


typedef struct
{
	uint32	flags;						// Animation Track Flags
	uint8**	animblock;					// Animation Tracks
}ANIMATION_EXPORTTRACK;

typedef struct
{
	ANIMATION_EXPORTTRACK* tracks;			// Animation Tracks
	ANIMATION_EXPORTTRACK  vtrack;			// Velocity Track
}ANIMATION_EXPORTDATA;



// Blend Ranges

typedef struct
{
	float msStart;
	float msEnd;
}FRAMERANGE;


typedef struct
{
	uint32		flags;
	FRAMERANGE	source;
	FRAMERANGE	dest;
	float		msBlendTime;
}BLENDCLIP;


typedef struct
{
	uint32		destID;
	uint32		nClips;
	BLENDCLIP*  BlendClips;
}ANIMATION_BLENDRANGE;


typedef struct
{
	uint32	destID;
	uint32	LLinkID;
	uint32	RLinkID;
}ANIMATION_LINK;



typedef struct
{
	// probably pack these into a single long
	uint32 time;
	uint32 type;
	uint32 data;
} TRACK_KEY;


// Animation

typedef struct
{
	ANIMATION_TRACK* tracks;			// Animation Tracks
	ANIMATION_TRACK  vtrack;			// Velocity Track
}ANIMATION_DATA;




// Node Structure For Animated Objects
/*
typedef struct
{
	uint32	flags;
	char	name[CHAR_NAME_LEN];
	uint32	boneID;
	uint32  boneidx;
	uint32  matrixIdx;

	uint32	pad;

//*** Remove This
	MATRIX	rotateOrient;
	VECTOR	rotateTranslate;
	VECTOR	rotatePivot;
	VECTOR  scalePivot;
	VECTOR	scaleTranslate;
//***


	VECTOR	rotate;
	VECTOR  translate;
	VECTOR	scale;

	MATRIX  pre;
	MATRIX  mid;
	MATRIX  post;

}NODE;
*/

#define FXNODE_MIRROR		(1<<0)
#define FXNODE_ACTIVE		(1<<1)

typedef struct
{
	VECTOR	rotate;
	VECTOR  translate;
	VECTOR	scale;

	uint32	flags;
	char	name[CHAR_NAME_LEN];
	uint32	boneID;

	uint32	argb;
	uint32	type;

	float	speed;
	float	emit;
	float	random;
	float	u0;
	float	u1;
	float	u2;
	float	u3;
	uint32	Message;			// Message + 1 (so 0 is no message)

}FXNODE;





typedef struct
{
	char name[CHAR_NAME_LEN];				// Animation Name (Debug Only)
	ANIMATION_DATA			*data;
	ANIMATION_BLENDRANGE	*BlendRanges;	// Blend Ranges
	ANIMATION_LINK			*Links;			// Links
	TRACK_KEY				*keys;			// Track Keys
	uint32					*ParentTracks;	// Mesh Indices For Parents
	uint32					*ParentIdx;		// Track Index For Parent
	FXNODE					*FXNodes;		// FX Nodes


	uint32 flags;							// Animation flags
	uint32 animID;							// Animation ID (Hash Of Name)
	uint32 altanimID;						// Alternative Anim (ie Mirror)


	uint32 nFrames;							// Number of Animation Frames
	uint32 nTracks;							// Number Of Blend Ranges
	uint32 nLinkRanges;						// Number Of Animation Tracks;
	uint32 nBlendRanges;					// Number Of Link Ranges
	uint32 nKeys;							// Number Of Keys
	uint32 nParentTracks;					// Number Of Parented Object Tracks
	uint32 nFXNodes;						// Number Of FX Nodes

}ANIMATION;




// Animation Links

typedef struct
{
	uint32  sourceID;
	uint32  destID;
	uint32	nlinks;
	uint32* links;
}ANIMATION_LINKPATH;

typedef struct
{
	uint32 nEntries;
	ANIMATION_LINKPATH* animlink;
}ANIMATION_LINKPATHTABLE;

// Animation List

typedef struct _ANIMATION_LIST
{
	uint32					nEntries;				// Number Of Filled In Entries
	uint32					nAllocEntries;			// Number Of Allocated Entries
	ANIMATION**				animation;				// Entries
	ANIMATION_LINKPATHTABLE	linktable;				// Animation Links
	uint32					uData;					// User Data - Currently used to hold audio object id - as set in anim editor

}ANIMATION_LIST;



//extern ANIMATION_LIST* pAPB_CurrentAnimationList;


#define	BONEFLAG_MATRIXPUSH		(1<<0)
#define BONEFLAG_MATRIXPOP		(1<<1)
#define BONEFLAG_OVERRIDEANIM	(1<<2)
#define BONEFLAG_NOTRACK		(1<<3)
#define BONEFLAG_BEGINDYNAMIC	(1<<4)
#define BONEFLAG_DYNAMIC		(1<<5)
#define BONEFLAG_HASBOUND		(1<<6)
#define BONEFLAG_BOUNDSPHERE	(1<<7)

#define BONEFLAG_OVERLAY_CH0 	(1<<8)
#define BONEFLAG_OVERLAY_CH1 	(1<<9)

#define BONEFLAG_REMOVABLE		(1<<10)

#define	BONEFLAG_BLENDTOANIM	(1<<11)
#define	BONEFLAG_BLENDFROMANIM	(1<<12)


#define BONETYPE_DYNAMIC		(1<<0)
#define BONETYPE_LEFT			(1<<1)
#define BONETYPE_RIGHT			(1<<2)
#define BONETYPE_DYNAMICBEGIN	(1<<3)


typedef struct ANIMATION_PLAYBACK_STRUCT ANIMATION_PLAYBACK;
typedef struct ANIMOBJECT_STRUCT ANIMOBJECT;
typedef struct BONECONTROL_STRUCT BONECONTROL;
typedef struct HIERACHY_STRUCT HIERACHY;

#define MESHFLAG_VISIBLE		(1<<0)
#define MESHFLAG_CLOTH_OVERRIDE	(1<<1)
#define MESHFLAG_ANIMATED		(1<<2)


typedef struct
{
	uint8* AnimBlock;					// track
	uint32 AnimSize;					// size
}
MORPH_SEQUENCE_TRACK;


// vertex or normal record - used in updating mesh
typedef struct
{
	uint32 Index;
	float x, y, z;
}
MORPH_RECORD;

// this structure represents a base object
// the base object has one or more targets that it can blend
// with to generate the final output
typedef struct
{
	uint32 Id;
	uint32 BaseId;
	uint32 NumTargets;
	uint32 NumVertices;
	float* Vertices;
	uint32 pad;
}
MORPH_MESH;


// this is a sequence or morphing animation
//
// one or more targets will be controlled by this
// sequence
typedef struct
{
	uint32 hash;
	
	uint32 Id;								// Sequence ID (Hash of Name)

	uint32 TargetId;						// Sequence ID (Hash of Name)
	MORPH_MESH* MorphMesh;

	uint32 NumFrames;						// Number of frames

	MORPH_SEQUENCE_TRACK* Envelope;			// Envelope Track

	uint32 NumTracks;						// Number of tracks
	MORPH_SEQUENCE_TRACK* Tracks;			// Target blend tracks
		}
MORPH_SEQUENCE;


// current morphing information
// similar to ANIMATION above.
//
// this structure guides how the model morphs
// this structure is used only to update the MORPH_TARGET factors
typedef struct
{
	// current time
	float Time;

	// sequence to execute; sequence will update all targets it is associated
	// with
	MORPH_SEQUENCE* Sequence;

	float Factors[MAX_MORPH_TARGETS];
}
MORPH;

///////////////////////////////////////////////////////////////////////////////
// Mesh structure

typedef struct {
#ifdef PS2
	QUADWORD DmaTag;
#else
	uint32	DmaTag[4];
#endif
	uint32	meshID;
	uint32	flags;
	uint32	animidx;
	uint32	matlidx;

	uint16	bone;
	uint16	morph_dma_buffer;
	uint16	*morph_lookup;
	uint32	*morph_dma[2];

	MATRIX	tm;

	VECTOR	min;
	VECTOR	max;

} MESHPS2;

#if defined(WIN32) || defined(_GAME_PC_)

	typedef struct
	{
		char   meshName[32];
		uint32 meshID;
		uint32 flags;
		uint32 bone;
		uint32 animidx;

		uint32 matlidx;
	//	uint32 tpageidx;	// Remove

		uint32  nstrips;
		uint32* striplens;
		uint16* strips;

		uint32 nverts;
		uint32 nuvs;
		uint32 nnormals;

		float* uvs;
		float* normals;
		float* verts;

	//	MORPH_MESH* morph_mesh;
		uint16* morph_lookup;
		uint32 morph_dma_buffer;
		uint32* morph_dma[2];

		MATRIX tm;

		VECTOR min;
		VECTOR max;

	}MESH;

#else

	typedef MESHPS2 MESH;

#endif


typedef struct
{
	VECTOR origin;
	float  radius;
	float pad[3];
}COLLIDESPHERE;

// Bone Structure

typedef struct
{
	uint32  flags;						// Flags
	char	name[CHAR_NAME_LEN];		// Bone Name (Debug Only)
	uint32  boneID;						// Bone ID (Hash Of Name)
	uint32	boneidx;
	uint32  matrixIdx;

	MATRIX bind;						///************
	MATRIX iabsbind;					//
	MATRIX pre;							//	Work In Progress
	MATRIX post;						//
//	MATRIX dynskin;						///************

	COLLIDESPHERE sphere;
	void    *physics_flags;				// RT
	uint32  uParentBone;

	uint32  pad[2];
	VECTOR  vpad[4];

}BONE;


typedef struct
{
	uint32	flags;
	char	name[CHAR_NAME_LEN];
	uint32	boneID;
	uint32  boneidx;
	uint32  matrixIdx;

//*** Remove This
	MATRIX  pre;
	MATRIX  mid;
	MATRIX  post;
	MATRIX	rotateOrient;

	VECTOR	rotateTranslate;
	VECTOR	rotatePivot;

	VECTOR  scalePivot;
	
	VECTOR	scaleTranslate;
//***

	VECTOR	rotate;
	VECTOR  translate;
	VECTOR	scale;

}NODE;





// Animation Playback
#define		APBFLAG_BLEND					(1<<0)
#define		APBFLAG_MIRROR					(1<<1)
#define		APBFLAG_PLAY					(1<<2)
#define		APBFLAG_NOVELOCITY				(1<<3)
#define		APBFLAG_NODESTADVANCE			(1<<4)
#define		APBFLAG_LOOP					(1<<5)
#define     APBFLAG_NOSOURCEADVANCE			(1<<6)
#define		APBFLAG_BLENDWAIT				(1<<7)
#define		APBFLAG_FINISHED				(1<<8)
#define		APBFLAG_LINK					(1<<9)
#define		APBFLAG_OVERLAY					(1<<10)
#define		APBFLAG_OVERLAYBLEND			(1<<11)
#define		APBFLAG_LOOPED					(1<<12)
#define		APBFLAG_BLENDCOMPLETE			(1<<13)
#define		APBFLAG_FORCEBLEND				(1<<14)
#define		APBFLAG_XFADEVELOCITY			(1<<15)
#define		APBFLAG_NOLINK					(1<<16)
#define		APBFLAG_LINKWAIT				(1<<17)
#define		APBFLAG_DELAYBLEND				(1<<18)
#define		APBFLAG_ANIMATEDSET				(1<<19)
#define		APBFLAG_OVERLAYBLENDCOMPLETE	(1<<20)
#define		APBFLAG_NOBLENDWAIT				(1<<21)
#define		APBFLAG_TERMINATOR				(1<<22)
#define		APBFLAG_FOLLOWLINK				(1<<23)
#define		APBFLAG_NOFOLLOWLINK			(1<<24)
#define		APBFLAG_NOBLEND					(1<<25)
#define		APBFLAG_HIDEOVERLAY				(1<<26)
#define		APBFLAG_NOALTERNATEANIM			(1<<27)
#define		APBFLAG_ENDOVERLAY				(1<<28)
#define		APBFLAG_TRIG_OVERLAY			(1<<29)
#define		APBFLAG_CONTROLBLEND			(1<<30)
#define		APBFLAG_FORCEUPDATE				(1<<31)

#define		APBFLAG_ALLOWSAMEANIMBLEND		(1<<31)



typedef struct
{
	VECTOR rot;
	VECTOR tran;
	VECTOR scale;
}FRAMEDATA;


typedef struct
{
	ANIMBLOCK_HEADER *pTrack;
	ANIMBLOCK_HEADER *pLast;
	int begin;
	int size;
}FRAMEINFO;



typedef struct
{
	uint32		flags;		// Flags
	float		time;		// MS Time
//	float		prevtime;	// Previous Time
	ANIMATION*	anim;		// Source Animation
	int16		frame0;		// Key Frame 0
	int16		frame1;		// Key Frame 1
	float		tval;		// t Value;

	int16		prevframe0; // Previous Frame
	int16		prevframe1;
	float		prevtval;

	uint32		loopcnt;	// Loop counter

	FRAMEINFO*  pFrameInfo;

	float		msperframe;

}ANIMATION_POSITIONINFO;


typedef struct
{
	float	   msBlend;									// ms Total Blend Time
	float	   msBlendTime;								// ms Current Blend Time
	float	   pmsBlendTime;							// ms Previous Blend Time
}BLEND_CONTROL;


typedef struct
{
	uint32 flags;
	ANIMATION_POSITIONINFO source;
	ANIMATION_POSITIONINFO dest;
	BLEND_CONTROL		   blend;
}ANIMATION_CONTROL;


struct BONECONTROL_STRUCT
{
	uint32 flags;
	void(*ControlBone)(ANIMOBJECT*,BONECONTROL*,MATRIX*,MATRIX*,VECTOR*); // User Defined Anim Control Function
	uint32* ControlData;
};



struct ANIMATION_PLAYBACK_STRUCT
{
	uint32	   flags;									// PlayBack Flags
//	uint8*	   boneflags;								// Bone Flags
	float	   msPerFrame;								// ms Per Frame

	BONECONTROL* bonecontrol;

	uint32    trigger_bones[4];


	ANIMATION_CONTROL	overlay[ANIMCTRL_MAXOVERLAY];
	ANIMATION_CONTROL	animation;


	ANIMATION_BLENDRANGE*	blendrange;					// Active BlendRange
	ANIMATION_LINK*			linkrange;					// Active Link
	ANIMATION_LINKPATH*		linkpath;					// Active Link Path
	uint32					linkpathidx;				// Active Path Idx


	void(*BeginFrameTick)(ANIMOBJECT*);											// User Defined Begin Animation Frame
	void(*ProcessBone)(ANIMOBJECT*,BONE* bone,MATRIX* prev,MATRIX* current, int iBone, int iParentBone);	// User Defined Process Bone
	void(*EndFrameTick)(ANIMOBJECT*);											// User Defined End Animation Frame

	struct ANIMOBJECT_STRUCT* pao;

	int			state;

	VECTOR		position;
	VECTOR		rotation;
	VECTOR		scale;
	
	VECTOR		vscaler;

	VECTOR      OldPosition;
	VECTOR      PositionDelta;

	VECTOR		velocity;
	VECTOR		rotvelocity;

	VECTOR		gravity;
	VECTOR		hip;

};



typedef struct
{
	int w,h;
	char* data;
	char *clut;
}TEXTURE;


typedef struct
{
	uint16 flags;
	uint16 tidx;
}MATERIAL;


#define	SKIN_SOLID	0
#define	SKIN_CADD	1
#define	SKIN_CSUB	2



typedef struct
{
	char	name[CHAR_NAME_LEN];
	uint32	skinID;

	int32	matidx;

	int8*	skin[3];
	uint32	skinsize[3];
//	int8*	skincoloradd;
//	uint32	skincoloraddsize;
//	int8*	skincolorsub;
//	uint32	skincolorsubsize;

	uint32	nmesh;
	MESH*	mesh;

	uint32		nHierachy;
	HIERACHY*	Hierachy;
	int      nTextures;
	TEXTURE* texture;

	int nMaterial;
	MATERIAL* pMaterial;

	void* dma[3];
	void* extradma[3];
	void* tpage;


	char* Shadow;

	#if defined( _GAME_PC_)
			// DLH - GamePC draw data
		void* psys_d3d_skin;
	#endif
}SKIN;



#define		HIERACHYFLAGS_HASMIRROR		(1<<0)
#define		HIERACHYFLAGS_HASCOLLISION	(1<<1)




// Hierachy Structure

struct HIERACHY_STRUCT
{
	uint32	flags;		// Hierachy Flags
	uint32	nNodes;		// Number Of Bones

	union
	{
		BONE*	boneptr;	// Bones in Hierachy Order
		NODE*	nodeptr;	// Nodes In Hierachy Order
	}nodes;

	union
	{
		BONE**	bone;	// Sorted Ptr List For Bone Searh
		NODE**  node;
	}nodeRefs;

	uint32*	bonemap;	// Bone Mappings For Mirroring Hierachy
};


#define AO_NOHIERACHY		(1<<0)

#define AO_ANIM_ADDVEL	(1<<15)

#define AO_RAGDOLL_INIT_WAIT (1<<1) // wait for event from death anim
#define AO_RAGDOLL_INIT_POS	 (1<<2) // snap shot pos
#define AO_RAGDOLL_INIT_VEL	 (1<<3) // calc velocities

#define AO_RAGDOLL			 (1<<4) // in ragdoll mode
#define AO_RAGDOLL_STATIC	 (1<<5) // in ragdoll mode but static - only updates draw matrice if on screen

#define AO_RAGDOLL_BLEND		(1<<6) //take hit blend
#define AO_RAGDOLL_BLEND_FINISH (1<<7) 

#define AO_RAGDOLL_TRACK (1<<8)

#define AO_RAGDOLL_RESTORE (1<<9)

#define AO_RAGDOLL_MASK (AO_RAGDOLL_RESTORE|AO_RAGDOLL_TRACK|AO_RAGDOLL_STATIC|AO_RAGDOLL_BLEND|AO_RAGDOLL_BLEND_FINISH|AO_RAGDOLL|AO_RAGDOLL_INIT_VEL|AO_RAGDOLL_INIT_POS)

#define	AO_VISIBLE				(1<<0)
#define AO_SHOOTLEFT			(1<<16)
#define AO_SHOOTRIGHT			(1<<17)
#define AO_SHOOTBOTH			(1<<18)
#define	AO_CLIPPED				(1<<19)
#define	AO_NOADVANCE			(1<<24)
#define AO_LODANIMATION			(1<<25)
#define	AO_PORTALCLIPPED		(1<<26)
#define AO_VALIDMATRICES		(1<<27)
#define AO_ANIMATE				(1<<28)
#define AO_CUTSCENE				(1<<29)
#define AO_USECURRENTMATRIX		(1<<30)						// AD: Don't load unit matrix at start of AO_Animate

#define STATICFXNODE 0x7eda3bfa


#define	AEFLAG_ISFOOTSTEP	(1<<24)
#define	AEFLAG_ISLEFTRIGHT	(1<<25)
#define	AEFLAG_ISFRONTBACK	(1<<26)


#define	MAX_FXLIST		512

typedef struct 
{
	uint32		flags;
	ANIMOBJECT* pao;
	FXNODE*		pFXNode;
}ANIMFX;

extern uint32 nFXList;
extern ANIMFX FXList[MAX_FXLIST];

void	animProcessFXList();


// Animated Object Structure

struct ANIMOBJECT_STRUCT
{
	uint32 flags;
	int32 userdata;
	void* userptr;
	int32 audioInstanceID;
	int32 csdata;

	VECTOR	min;		// Bounding Box min
	VECTOR  max;		// Bounding Box max

	HIERACHY* hierachy;
	ANIMATION_LIST		*pAnimationList;
	ANIMATION_PLAYBACK* apb;

	void* matrices;
	uint32 nMatrices;

	uint32  nskin;
	SKIN**  skin;

	SKIN*	selectedskin;
	SKIN*	mipskin;

	MATRIX	Light;		// CC : 05:12:02 : Current light matrices for interpolation
	MATRIX	Colour;     //

	uint32* meshflags;

	// Shadow Stuff
	int16	shadowcalced ;
	int16	shadownrtris ;
	int16	shadownredges ;
	int16	*shadowvertcounts ;
	VECTOR	*shadowverts ;
	void	*shadowindex ;
	void	*shadowedges ;

	// Physics stuff

	void* dynamic_char; 
	#if 1 //ISME(RT) //this is needed for ragdoll to AnimObject collision
		int physics_flags[16];
	#endif

	#if defined( PS2)
		QUADWORD* cloth_mesh_DmaTags;
	#endif

//	void * ragdoll;
//	void * ponytail;

//  RT - might keep
//	ANIMOBJECT_VELINFO animVelInfo[ANIMOBJECT_VELINFO_NUM];
//	void * havokChar;						// Editor safe.  This will actually point to a structure that has it's first member
											// of type HAVOKCHARHEADER (see HavokCharacter.h)

    struct PLAYERCONTROL_STRUCT* Control;

	// morph playback

	MORPH Morph;
	uint32 NumMorphMeshes;
	MORPH_MESH* MorphMeshes[MAX_MORPH_MESHES];

	uint32 NumMorphSequences;
	MORPH_SEQUENCE* MorphSequences[MAX_MORPH_SEQUENCES];

	#if defined( _GAME_PC_)
			// DLH - GamePC draw data
		void* psys_draw_instchar;
		void* psys_draw_cloth_list;
	#endif

	ANIMATION*  pStaticFXAnim;

	VECTOR target;
	uint32 id;
};


//----------------------------------------------------------------------------------------------------
// Function Prototypes
//----------------------------------------------------------------------------------------------------

unsigned int STRING_GetHashValue(char *string);


//----------------------------------------------------------------------------------------------------

BONE*			BONE_CreateList			 (int nEntries);
void			BONE_SetName			 (BONE* bone,char* name);
int				BONE_GetTypeFromName	 (BONE* bone);
int*			BONE_CreateMirrorMap	 (int nEntries,BONE* bones);



extern void(*FXHandler)(ANIMOBJECT*,FXNODE* node,uint32 flags);
extern void(*FXHandlerImmediate)(ANIMOBJECT*,FXNODE* node,uint32 flags);
extern void(*BoneControlBlend)(ANIMOBJECT*,BONECONTROL*,MATRIX*,MATRIX*,VECTOR*);


//----------------------------------------------------------------------------------------------------


void			HR_SetInitialise(HIERACHY* phr,int nBones,NODE* pBones);

void			HR_Initialise(HIERACHY* phr,int nBones,BONE* pBones,int* pBoneMirrorMap);
void			HR_Destroy(HIERACHY** phr);

void			HR_CreateBoneRef(HIERACHY* phr);
void			HR_CreateNodeRef(HIERACHY* phr);


BONE*			HR_FindBone(HIERACHY* phr,unsigned int BoneID);
int				HR_GetBoneIndex(HIERACHY * phr, unsigned int boneID);
void			HR_GetBoneBindMatrix(MATRIX * m, HIERACHY * phr, int boneIndex);

NODE*			HR_FindNode(HIERACHY* phr,unsigned int boneID);


//----------------------------------------------------------------------------------------------------

void			AO_Initialise(ANIMOBJECT* pao);
void			AO_FrameTick(ANIMOBJECT* pao);
void			AO_AnimateHierachy(ANIMOBJECT* pao);
void			AO_AnimateSet(ANIMOBJECT* pao);
void			AO_Animate(ANIMOBJECT* pao);
void			AO_SetSkinData(ANIMOBJECT* pao,char* name);
void			AO_ApplyVelocity(ANIMOBJECT* pao);
void			AO_Destroy(ANIMOBJECT **pao);
BOOL			AO_SetParentedObjectVisibility(ANIMOBJECT *pao, int nParentedObject, int state);

void			AO_SetPositionAbs(ANIMOBJECT* pao,float x,float y,float z);
void			AO_SetPositionRel(ANIMOBJECT* pao,float x,float y,float z);
void			AO_SetRotationAbs(ANIMOBJECT* pao,float x,float y,float z);

void			AO_SetPositionLocal(ANIMOBJECT* pao);

void			AO_DestroySkin(ANIMOBJECT* pao,uint32 skinID);

void			AO_InitMeshFlags(ANIMOBJECT* pao);

MESH*			AO_FindMesh(ANIMOBJECT* pao,uint32 meshID);


//----------------------------------------------------------------------------------------------------

void			APB_Initialise(ANIMOBJECT* pao);

//float			APB_FrameToMS(float frame);
//float			APB_MSToFrame(float ms);

float			APB_SetMSPerFrame(ANIMATION_PLAYBACK* papb,float ms);
float			APB_GetMSPerFrame(ANIMATION_PLAYBACK* papb);
float			APB_SetBlendTime(ANIMATION_PLAYBACK* papb,float ms);

float			APB_GetSourceMSTime(ANIMATION_PLAYBACK* papb);
float			APB_GetSourceFrameTime(ANIMATION_PLAYBACK* papb);


void			APB_SetFunctions(ANIMATION_PLAYBACK* papb,void(*BeginFrameTick)(ANIMOBJECT*),void(*ProcessBone)(ANIMOBJECT*,BONE* bone,MATRIX* prev,MATRIX* current, int iBone, int iPrevBone),void(*EndFrameTick)(ANIMOBJECT*));

void			APB_PlayAnimation(ANIMATION_PLAYBACK* papb,unsigned int animID,float startTime,int flags);
BOOL			APB_BlendToAnimation(ANIMATION_PLAYBACK* papb,unsigned int animID,float startTime,float blendtime,int flags);


BOOL			APB_OverlayAnimation(ANIMATION_PLAYBACK* papb,unsigned int idx,unsigned int animID,float startTime,float blendtime,int flags);
BOOL			APB_BlendFromOverlay(ANIMATION_PLAYBACK* papb,unsigned int idx,float mstime);

void			APB_DestToSource(ANIMATION_PLAYBACK* papb);

void			APB_StopAnimation(ANIMATION_PLAYBACK* papb);
void			APB_StartAnimation(ANIMATION_PLAYBACK* papb);
void			APB_PauseAnimation(ANIMATION_PLAYBACK* pab);


void			APB_AudioEvents(ANIMOBJECT*);

float			  APB_BezierIdx(short* p,float t,int a,int b,int c,int d);
ANIMBLOCK_HEADER* APB_GetTrackValue(uint8* s,int frame,float* dest,float mul);

BOOL			APB_IsPlayingSpecifiedAnimation(ANIMATION_PLAYBACK *papb, unsigned int animID);


void			APB_GetVelocityOffset(ANIMATION* anim,int frame,VECTOR* t,VECTOR* r);
void			APB_SetAnimationFrame(ANIMOBJECT* pao,int frame);
void			APB_InitBoneFlags(ANIMATION_PLAYBACK* papb,HIERACHY* phr);
void			APB_GetAnimationVelocity(ANIMOBJECT* pao);

void			APB_ApplyVelocity(ANIMATION_PLAYBACK* papb,VECTOR* v,VECTOR* r);
void			APB_ApplyVelocity_General(ANIMATION_PLAYBACK *papb, VECTOR *rotation, VECTOR *v, VECTOR *r, VECTOR *result_velocity);

void			APB_GetFrames(ANIMATION_PLAYBACK* papb,ANIMATION_POSITIONINFO* pos);

void			APB_InitFrames(ANIMATION_PLAYBACK* pao,ANIMATION_POSITIONINFO* pos,float startime);

void			APB_EvalDestination(ANIMATION_PLAYBACK* pao,uint32 destid,float start,float blend,uint32 flags,VECTOR* eval);

void			APB_GetCurrentVelocity(ANIMOBJECT* pao,VECTOR* rv,VECTOR* rr);
void			APB_Visibility(ANIMOBJECT* pao);

BOOL			APB_HideOverlay(ANIMATION_PLAYBACK* papb,unsigned int idx,float msblend);
BOOL			APB_ShowOverlay(ANIMATION_PLAYBACK* papb,unsigned int idx,float msblend);



BOOL			AO_BoundingBoxCollide(ANIMOBJECT *pa, ANIMOBJECT *pb);
int				AO_SphereCollide(ANIMOBJECT* pao,VECTOR* pos,float radius);
int32			AO_BoxSphereCollide(ANIMOBJECT* pao,uint32 hash,ANIMOBJECT* player);

//----------------------------------------------------------------------------------------------------

void			AL_Initialise(ANIMATION_LIST*,int nInitialEntries);
void			AL_Destroy(ANIMATION_LIST**);
void			AL_DestroyContent(ANIMATION_LIST*);


ANIMATION_LIST*	AL_ActivateAnimationList(ANIMATION_LIST*);


int				AL_GetAnimationListCnt(ANIMATION_LIST*);
int				AL_AddAnimation(ANIMATION_LIST*,ANIMATION*);
void			AL_SortAnimationList(ANIMATION_LIST*);
ANIMATION*		AL_FindAnimation(ANIMOBJECT* pao,unsigned int animID);
void			AL_DeleteAnimation(ANIMATION_LIST* pal,unsigned int animID);
ANIMATION_LIST* AL_SetAnimationList(ANIMOBJECT* pao,ANIMATION_LIST* pal);

void			AL_SortBlendList(ANIMATION* pa);
void			AL_SortLinkPathList(ANIMATION_LIST* pal);

ANIMATION_LINKPATH*		AL_CreateLink(ANIMATION_LIST* pal,uint32 sourceID,uint32 destID,uint32 nEntries);
void					AL_AddLinkEntry(ANIMATION_LINKPATH* link,uint32 idx,uint32 hashID);
void					AL_DestroyLinkTable(ANIMATION_LIST* pal);
ANIMATION_LINKPATH*		AL_CheckForLinkPath(ANIMOBJECT* pao,uint32 sourceID,uint32 destID);

ANIMATION* AL_FindAnimationFromList(ANIMATION_LIST* pal,unsigned int animID);

void					AL_AddBlendRange(ANIMATION_LIST*,unsigned int animID,ANIMATION_BLENDRANGE*);
ANIMATION_BLENDRANGE*	AL_FindBlendRange(ANIMATION* anim,unsigned int animID);
int						APB_InBlendRange(ANIMATION_CONTROL *pos,ANIMATION_BLENDRANGE* pabr);
void					APB_SetupRangeBlend(ANIMATION_PLAYBACK *papb,ANIMATION_BLENDRANGE* pabr,int ridx,int flags);

void APB_GetAnimFrames(ANIMOBJECT* pao);
void APB_ClearOverlayTriggers(ANIMATION_PLAYBACK* papb);
void APB_ClearAllOverlays(ANIMOBJECT* pao);

// used in editor
void AL_ClearBlendRanges(ANIMATION_LIST*pal,unsigned int animID);


//void AT_GetFrameData(ANIMATION_TRACK* track,int frame0,VECTOR *rot,VECTOR *tran,VECTOR *scale);

void AT_GetFrameData(ANIMATION_POSITIONINFO* pos,int idx,ANIMATION_TRACK* track,int frame0,int frame1,FRAMEDATA* fd0,FRAMEDATA* fd1);
void AT_GetSetFrameData(ANIMATION_TRACK* track,int frame0,VECTOR *rot,VECTOR *tran,VECTOR *scale);
void AT_GetSetFrameData2(ANIMATION_POSITIONINFO* pos,int boneidx,ANIMATION_TRACK* track,int frame0,int frame1,FRAMEDATA* fd0,FRAMEDATA* fd1);


void SetBit(unsigned char* ptr,int bit,int flag);
int  CheckBit(unsigned char* ptr,int bit);

void AO_GetBoneSphere(ANIMOBJECT* pao,int idx,VECTOR* pos);

void APB_ClearAllCache(ANIMOBJECT* pao);

void APB_ClearPositionInfo(ANIMATION_POSITIONINFO* data);

int dbgAnimInfo(ANIMOBJECT* pao);


void MPH_AddMorphMesh(ANIMOBJECT* pao, MORPH_MESH* base);
void MPH_RemoveMorphMesh(ANIMOBJECT* pao, MORPH_MESH* mesh);
MORPH_MESH* MPH_GetMorphMesh(ANIMOBJECT* pao, uint32 hash);
MORPH_MESH* MPH_GetMorphBase(ANIMOBJECT* pao, uint32 hash);
void MPH_AddMorphSequence(ANIMOBJECT* pao, MORPH_SEQUENCE* sequence);
void MPH_RemoveMorphSequence(ANIMOBJECT* pao, MORPH_SEQUENCE* sequence);
MORPH_SEQUENCE* MPH_GetMorphSequence(ANIMOBJECT* pao, uint32 hash);
void MPH_PlayMorphSequence(ANIMOBJECT* pao, MORPH_SEQUENCE* sequence);
void MPH_FrameTick(ANIMOBJECT *ao);

//----------------------------------------------------------------------------------------------------




//----------------------------------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif
