//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//
// AnimAPI	(c) 2001 Core Design Ltd
//
//----------------------------------------------------------------------------------------------------
//
// History :: Created 24-09-01 Richard Flower
//
//
//
//
// 25-10-02 - Not much else happened!
//
//
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
#include <stdlib.h>
#include "typedefs.h"
#include <float.h>

#include "AnimAPI.h"
#include <string.h>

#include "assert.h"

//#include "dbg.h"

#include "CoreAudio.h"
#include "stdio.h"

#ifdef WIN32
	#ifndef _GAME_PC_
		#define USE_COREAUDIO
	#endif
#endif


#ifdef USE_COREAUDIO
	extern void CueAudioEvent(TRACK_KEY* key,ANIMOBJECT* animObj);
	extern void UpdateAudioEvent(TRACK_KEY* key,ANIMOBJECT* animObj);
	extern void CueEditorSpeech(unsigned int  morphHashID);
#else
	#define CueAudioEvent(_a, _b)
	#define UpdateAudioEvent(_a, _b)
	#define CueEditorSpeech(_a)
#endif


#if defined(PS2) || defined(_GAME_PC_)
	#include "PhysicsSystem.h"
	#include "..\..\map\map_collideAPI.h"
	#include "sys_draw.h"
	#include "sys_input.h"
	#include "dbg.h"
	#include "..\..\control\player_control.h"


	void	mapAnimEventHandler (ANIMOBJECT *pAO,FXNODE *pFX);
#endif

void debugLastEvalPos();


#ifdef PS2
#undef assert
#define assert(a) ASSERT(a)
#endif

#ifndef ASSERT
#define ASSERT(a)
#endif


void(*FXHandler)(ANIMOBJECT*,FXNODE* node,uint32 bFlags) = NULL;
void(*FXHandlerImmediate)(ANIMOBJECT*,FXNODE* node,uint32 bFlags) = NULL;
void(*BoneControlBlend)(ANIMOBJECT*,BONECONTROL*,MATRIX*,MATRIX*,VECTOR*);


#if defined( PS2) || defined( _GAME_PC_)
extern DRAWMATRIX* pMatrices;
#endif

//----------------------------------------------------------------------------------------------------

float		gAnimPlayFPS;
float		gAnimTargetMS;
float		gAnimTargetMSSquared;
float		gAnimDefaultMS;
float		gAnimDefaultMSBlend;

float		APB_FrameToMS(float a)		{return (a * gAnimDefaultMS);}
float		APB_MSToFrame(float a)		{return (a / gAnimDefaultMS);}

void AnimApiInit(float fps)
{
	gAnimPlayFPS		 = fps;
	gAnimTargetMS		 = (ANIM_ONESECOND/gAnimPlayFPS);
	gAnimTargetMSSquared = gAnimTargetMS * gAnimTargetMS;
	gAnimDefaultMS		 = (ANIM_ONESECOND/ANIM_FPS);
	gAnimDefaultMSBlend  = 2.0f * gAnimDefaultMS;

	BoneControlBlend = NULL;
}



//----------------------------------------------------------------------------------------------------

#if defined( PS2) || defined( _GAME_PC_)

#include "core/game.h"

#else

#define	BORDER

#endif



//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

/*
	Function	:	STRING_GetHashValue
	Useage		:	Return Hash Value Of String
*/

unsigned int STRING_GetHashValue(char *s)
{
	int h,l,step;
	l = strlen(s);
	h = l;
	step = (l>>6)+1;
	for(;l>0;l-=step)
		h = h^((h<<5)+(h>>2)+(unsigned char)*(s++));
  	return h;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

/*
	Function	:	APB_Initialise
	Useage		:	Initialise ANIMATION_PLAYBACK Structure
*/

void APB_Initialise(ANIMOBJECT* pao)
{
	assert(pao!=NULL);

	memclear(pao->apb,sizeof(ANIMATION_PLAYBACK));

	pao->apb->pao = pao;

	pao->apb->animation.blend.msBlend			= gAnimDefaultMSBlend;
	pao->apb->msPerFrame						= gAnimDefaultMS/(gAnimPlayFPS/ANIM_FPS);

	pao->apb->scale.x = 1.0f;
	pao->apb->scale.y = 1.0f;
	pao->apb->scale.z = 1.0f;

	pao->apb->vscaler.x = 1.0f;
	pao->apb->vscaler.y = 1.0f;
	pao->apb->vscaler.z = 1.0f;


	mathCopyVector(&pao->apb->position, &pao->apb->OldPosition);
	pao->apb->PositionDelta.x = 0.0f;
	pao->apb->PositionDelta.y = 0.0f;
	pao->apb->PositionDelta.z = 0.0f;
	pao->apb->PositionDelta.w = 0.0f;


	pao->min.x = pao->min.y = pao->min.z = -1024.0f;
	pao->max.x = pao->max.y = pao->max.z = 1024.0f;
}



/*
	Function	:	APB_InitBoneFlags
	Useage		:	Initialise Bone Flags From Hierachy
*/

void APB_InitBoneFlags(ANIMATION_PLAYBACK* papb,HIERACHY* phr)
{
	unsigned int boneStack[64], stackptr, prevbone;
	unsigned int i;

	// Allocate Track Cache Ptrs

	papb->animation.source.pFrameInfo = memalloc(phr->nNodes * sizeof(FRAMEINFO) * AT_TOTAL);
	memclear(papb->animation.source.pFrameInfo,phr->nNodes * sizeof(FRAMEINFO) * AT_TOTAL);

	papb->animation.dest.pFrameInfo = memalloc(phr->nNodes * sizeof(FRAMEINFO) * AT_TOTAL);
	memclear(papb->animation.dest.pFrameInfo,phr->nNodes * sizeof(FRAMEINFO) * AT_TOTAL);

	if(!(papb->flags&APBFLAG_ANIMATEDSET))
	{
		for(i=0;i<ANIMCTRL_MAXOVERLAY;i++)
		{
			papb->overlay[i].source.pFrameInfo = memalloc(phr->nNodes * sizeof(FRAMEINFO) * AT_TOTAL);
			memclear(papb->overlay[i].source.pFrameInfo,phr->nNodes * sizeof(FRAMEINFO) * AT_TOTAL);

			papb->overlay[i].dest.pFrameInfo = memalloc(phr->nNodes * sizeof(FRAMEINFO) * AT_TOTAL);
			memclear(papb->overlay[i].dest.pFrameInfo,phr->nNodes * sizeof(FRAMEINFO) * AT_TOTAL);
		}
	}


	papb->bonecontrol = memalloc(phr->nNodes * sizeof(BONECONTROL));

	for(i=0;i<phr->nNodes;i++)
	{
		papb->bonecontrol[i].flags = phr->nodes.boneptr[i].flags;
		papb->bonecontrol[i].ControlBone = 0;
		papb->bonecontrol[i].ControlData = 0;
	}


	if(!(papb->flags&APBFLAG_ANIMATEDSET))
	{
		stackptr=0;
		prevbone = 0xffffffff;

		for(i=0;i<phr->nNodes;i++)
		{

			if (phr->nodes.boneptr[i].flags & BONEFLAG_MATRIXPOP)
			{
				stackptr--;
				prevbone = boneStack[stackptr];
			}

			if (phr->nodes.boneptr[i].flags & BONEFLAG_MATRIXPUSH)
			{
				boneStack[stackptr+1] = boneStack[stackptr];
				stackptr++;
			}

			phr->nodes.boneptr[i].boneidx = i;

			boneStack[stackptr] = i;

			phr->nodes.boneptr[i].uParentBone = prevbone;

			prevbone = boneStack[stackptr];
		}
	}
	else
	{
		for(i=0;i<phr->nNodes;i++)
			phr->nodes.nodeptr[i].boneidx = i;
	}
}




/*
	Function	:	APB_SetFunctions
	Useage		:	Set The User Functions For Playing Animations
*/


void APB_SetFunctions(ANIMATION_PLAYBACK* papb,void(*BeginFrameTick)(ANIMOBJECT*),void(*ProcessBone)(ANIMOBJECT* pao,BONE* bone,MATRIX* prev,MATRIX* current,int,int),void(*EndFrameTick)(ANIMOBJECT*))
{
	assert(papb!=NULL);

	papb->BeginFrameTick  = BeginFrameTick;
	papb->EndFrameTick	  = EndFrameTick;
	papb->ProcessBone	  = ProcessBone;
}




/*
	Function	:	APB_FrameToMS
	Useage		:	Convert Frame Number To Milliseconds
*/

//float APB_FrameToMS(float frame)
//{
	//return frame * ANIM_DEFAULT_MS;
//}


/*
	Function	:	APB_MSToFrame
	Useage		:	Convert Milliseconds To Frame Number
*/

//float APB_MSToFrame(float ms)
//{
	//return ms / ANIM_DEFAULT_MS;
//}


/*
	Function	:	APB_SetMSPerFrame
	Useage		:	Set Milliseconds Per Frame
*/

float APB_SetMSPerFrame(ANIMATION_PLAYBACK* papb,float ms)
{
	float msPerFrame;
	assert(papb!=NULL);
	msPerFrame = papb->msPerFrame;
	papb->msPerFrame = ms;
	return msPerFrame;
}


/*
	Function	:	APB_GetMSPerFrame
	Useage		:	Get The Milliseconds Per Frame
*/

float APB_GetMSPerFrame(ANIMATION_PLAYBACK* papb)
{
	assert(papb!=NULL);
	return papb->msPerFrame;
}


/*
	Function	:	APB_SetBlendTime
	Useage		:	Set The Animation Transition BlendTime
*/

float APB_SetBlendTime(ANIMATION_PLAYBACK* papb,float ms)
{
	float msBlend = papb->animation.blend.msBlend;
	assert(papb!=NULL);
	papb->animation.blend.msBlend = ms;
	return msBlend;
}


/*
	Function	:	APB_BezierIdx
	Useage		:	Fetch Point On Bezier Track Curve
*/

float APB_BezierIdx(short* p,float t,int a,int b,int c,int d)
{
  float ab,bc,cd,abbc,bccd;
  float point;

  ab    = p[a] + ((p[b] - p[a]) * t);
  bc    = p[b] + ((p[c] - p[b]) * t);
  cd    = p[c] + ((p[d] - p[c]) * t);

  abbc  = ab + ((bc - ab) * t);
  bccd  = bc + ((cd - bc) * t);

  point = (abbc + ((bccd - abbc) * t));

  return point;
}



/*
	Function	:	APB_GetTrackValue
	Useage		:	Fetch Value For Frame On Track
*/

ANIMBLOCK_HEADER* APB_GetTrackValue(uint8* s,int frame,float* dest,float mul)
{
	short* d;
	ANIMBLOCK_HEADER* pab;
	int begin,end;

	pab = (ANIMBLOCK_HEADER*) s;

	begin = 0;

	if(!(pab->word&ANIM_LASTBLOCK))
	{
		while(1)
		{
			end = pab->frame;

			if(frame <= end)
				break;

			begin = end;

			s += pab->size;

			pab = (ANIMBLOCK_HEADER*) s;
		}
	}
	else
	{
		end = pab->frame;
	}

	pab = (ANIMBLOCK_HEADER*) s;

	d = (short*) (s + sizeof(ANIMBLOCK_HEADER));

	if(pab->word&ANIM_CONST)
	{
		*(dest) = *(d) * mul;
	}
	else if(pab->word&ANIM_KEYS)
	{
		*(dest) = *(d+(frame-begin)) * mul;
	}
	else if(pab->word&ANIM_BEZIER)
	{
		float t;

		t = (float) (frame - begin)/(float)(end - begin);

		if(pab->size == 10)
			*(dest) = APB_BezierIdx(d,t,-3,0,1,2) * mul;
		else
			*(dest) = APB_BezierIdx(d,t,0,1,2,3) * mul;
	}

	return pab;
}




/*
	Function	:	APB_GetTrackValue
	Useage		:	Fetch Value For Frame On Track
*/

ANIMBLOCK_HEADER* APB_GetTrackValue2(FRAMEINFO* pFinf,int frame,float* dest,float mul)
{
	short* d;
	uint8* s = NULL;
	ANIMBLOCK_HEADER* pab = NULL;
	int begin,end = 0;
	int size = 0;

	s = (uint8*) pFinf->pLast;

	if(s)
	{
		begin = pFinf->begin;

		if(frame < begin)
		{
			s = NULL;
		}
		else
		{
			end = pFinf->pLast->frame;
			pab = (ANIMBLOCK_HEADER*) s;
		}
	}

	if(!s || frame > pFinf->pLast->frame)
	{

		if(!s)
		{
			s = (uint8*) pFinf->pTrack;
			pab = pFinf->pTrack;
		}

		begin = 0;

		if(!(pab->word&ANIM_LASTBLOCK))
		{
			while(1)
			{
				end = pab->frame;

				if(frame <= end)
					break;

				begin = end;

				s += pab->size;
				size = pab->size;

				pab = (ANIMBLOCK_HEADER*) s;
			}
		}
		else
		{
			end = pab->frame;
		}
	}

	pFinf->pLast = pab;
	pFinf->begin = begin;
	pFinf->size  = size;

	d = (short*) (s + sizeof(ANIMBLOCK_HEADER));

	if(pab->word&ANIM_CONST)
	{
		*(dest) = *(d) * mul;
	}
	else if(pab->word&ANIM_KEYS)
	{
		*(dest) = *(d+(frame-begin)) * mul;
	}
	else if(pab->word&ANIM_BEZIER)
	{
		float t;

		t = (float) (frame - begin)/(float)(end - begin);

		if(pab->size == 10)
			*(dest) = APB_BezierIdx(d,t,-3,0,1,2) * mul;
		else
			*(dest) = APB_BezierIdx(d,t,0,1,2,3) * mul;
	}

	return pab;
}




void APB_CheckFrames(ANIMATION_PLAYBACK* papb,ANIMATION_POSITIONINFO* pos)
{
	float currentframe;
	int f0,f1;
	float t;
	int nFrames;

	currentframe = APB_MSToFrame(pos->time);

	nFrames	 = pos->anim->nFrames;

	f0 = (int) currentframe;
	f1 = f0 + 1;

	if(pos->anim->flags&APBFLAG_LOOP)
		nFrames--;
	else
	{
		if(f1 > nFrames-1)
			f1 = nFrames-1;
		if(f0 < 0)
			f0 = 0;
	}

	// ++ IF LOOPING

	if(pos->anim->flags&APBFLAG_LOOP)
	{
		if(f1 > nFrames)
			f1 -= nFrames;

		if(currentframe < 0.0f)
		{
			currentframe += nFrames;
			f0 = (int) currentframe;
			f1 = f0 + 1;

			if(f1 > (int) nFrames)
				f1 -= (int) nFrames;
		}
		else if(f0 >= nFrames)
		{
			currentframe -= nFrames;

			f0 = (int) currentframe;
			f1 = f0 + 1;

			if(f1 > (int) nFrames)
				f1 -= (int) nFrames;
		}
	}
	else
	{
		int cmp;

		cmp = (int) (currentframe); //+0.5f);

		if(cmp >= nFrames-1)
		{
			currentframe = (float) nFrames-1;
			f0 = nFrames-1;
			f1 = nFrames-1;
		}

		if(f1 <= 0)
		{
			currentframe = 0;
			f0 = 0;
			f1 = 0;
		}

	}

	t = currentframe - f0;

//	pos->prevtime = pos->time;
	pos->time   = APB_FrameToMS((currentframe));
	pos->frame0 = f0;
	pos->frame1 = f1;
	pos->tval   = t;
}




void APB_InitFrames(ANIMATION_PLAYBACK* papb,ANIMATION_POSITIONINFO* pos,float startime)
{
	float currentframe;

	pos->msperframe = gAnimTargetMS;

//	pos->prevtime = -1.0f;
	pos->time = startime;

	currentframe = APB_MSToFrame(pos->time);

	pos->frame0 = (int) currentframe;
	pos->frame1 = pos->frame0 + 1;
	pos->tval   = currentframe - pos->frame0;

	APB_CheckFrames(papb,pos);

//	pos->tval   = 0.0f;

	pos->prevframe0 = pos->frame0;// - 1;
	pos->prevframe1 = pos->frame1;// - 1;
	pos->prevtval   = pos->tval - mathFabs(gAnimTargetMSSquared / papb->msPerFrame); //papb->msPerFrame; //0.5f;

	if(pos->prevtval < 0.0f)
	{
		pos->prevtval += 1.0f;
		pos->prevframe0 = pos->frame0 - 1;
		pos->prevframe1 = pos->frame1 - 1;
	}


	if(pos->prevframe0 < 0)
	{
		if(pos->anim->flags&APBFLAG_LOOP)
		{
			pos->prevframe0 = (short) pos->anim->nFrames-2;
			pos->prevframe1 = pos->prevframe0 + 1;
			pos->prevtval   = mathFabs(gAnimTargetMSSquared / papb->msPerFrame); //papb->msPerFrame; //0.5f;
		}
	}

	if(papb->pao->hierachy)
	{
		if(pos->pFrameInfo)
		{
			memclear(pos->pFrameInfo,sizeof(FRAMEINFO) * papb->pao->hierachy->nNodes * AT_TOTAL);
		}
	}

	pos->flags &= ~APBFLAG_FINISHED;
}





/*
	Function	:	APB_PlayAnimation
	Useage		:	Play The Specified Animation
*/

void APB_PlayAnimation(ANIMATION_PLAYBACK* papb,unsigned int animID,float startTime,int flags)
{
	ANIMATION* animation;

	assert(papb != NULL);
	assert(animID != 0);

//	if(papb->animation.source.anim)
//	{
//		if(papb->animation.source.flags&APBFLAG_LOCK)
//			return;
//	}

	animation = AL_FindAnimation(papb->pao,animID);

	if(!animation)
	{
//		dbgLog(XX,0,"Attempt To Play Invalid Animation %x",animID);
		return;
	}

//	assert(animation != NULL);

	papb->animation.source.loopcnt = 0;
	papb->animation.source.anim   = animation;
	papb->animation.source.flags  = flags;

	APB_InitFrames(papb,&papb->animation.source,startTime);

	papb->flags &= (~APBFLAG_BLEND);
	papb->flags &= (~APBFLAG_BLENDWAIT);
	papb->flags &= (~APBFLAG_LINKWAIT);
	papb->flags &= (~APBFLAG_DELAYBLEND);
	papb->flags &= (~APBFLAG_FOLLOWLINK);
	papb->flags &= (~APBFLAG_CONTROLBLEND);

	papb->animation.dest.anim = NULL;

//	dbgLog("Play %s %f",animation->name,startTime);
}





/*
	Function	:	APB_IsPlayingSpecifiedAnimation
	Usage		:	return true if the Animation specified by animID is the animation currently playing,
					or if blending is active, is the animation that is being blended to.
					false indicates that animation is not being played, or for a blend is not going to be
					blended to.
*/

BOOL APB_IsPlayingSpecifiedAnimation(ANIMATION_PLAYBACK *papb, unsigned int animID)
{
	ANIMATION* animation;

	animation = AL_FindAnimation(papb->pao,animID);

	// is blend active?
	if (papb->flags&APBFLAG_BLEND)
	{
		// yes

		if (papb->animation.dest.anim == animation)
			return TRUE;
	}
	else
	{
		// no
		if (papb->animation.source.anim == animation)
			return TRUE;
	}

	return FALSE;
}





void APB_GetFrameRange(ANIMATION_CONTROL*pos,float start,float end,float* dist)
{
	float mstime;

	*dist = 0;

	mstime = APB_FrameToMS((float)pos->source.anim->nFrames);

	if(start <= end)
		*dist = end - start;

	if(start > end)
		*dist = (mstime - start) + end;

	if(!*dist)
		*dist = 1.0f;
}




void APB_GetDistanceToStartOfBlend(ANIMATION_CONTROL* pos,float start,float end,int* dist)
{
	int frame;
	int nframes;

	nframes = pos->source.anim->nFrames;
	frame = pos->source.frame0;

	*dist = 0;

	if(frame <= start)
		*dist = (int) (start - frame);

	if(frame >= start)
		*dist = (int) (start + (nframes-frame));
}



int APB_FindClosestBlendStart(ANIMATION_CONTROL *pos,ANIMATION_BLENDRANGE* pabr)
{
	uint32 i;
	float mstime;

	int distance = 0;
	int dist;

	if(!pabr) return -1;

	mstime = pos->source.time;

	for(i=0;i<pabr->nClips;i++)
	{
		float start;
		float end;

		start = APB_FrameToMS(pabr->BlendClips[i].source.msStart);
		end   = APB_FrameToMS(pabr->BlendClips[i].source.msEnd);

		APB_GetDistanceToStartOfBlend(pos,start,end,&dist);

		if(i)
		{
			if(dist < distance)
				distance = dist;
		}
		else
			distance = dist;

	}

	return distance;
}


/*
	Function	:	APB_InBlendRange
	Useage		:	Check Blend Range
*/

#define		EPSILON_FRIG	1.0f


int APB_InBlendRange(ANIMATION_CONTROL *pos,ANIMATION_BLENDRANGE* pabr)
{
	uint32 i;
	float mstime;

	if(!pabr) return -1;

	mstime = pos->source.time;

	for(i=0;i<pabr->nClips;i++)
	{
		float start;
		float end;
		float size;

		start = APB_FrameToMS(pabr->BlendClips[i].source.msStart);
		end   = APB_FrameToMS(pabr->BlendClips[i].source.msEnd);

		APB_GetFrameRange(pos,start,end,&size);

		if(start == end)
		{
			if(mstime >= start - EPSILON_FRIG || mstime <= end + EPSILON_FRIG)
			{
				return i;
			}
		}

		if(start < end)
		{
			if((mstime + EPSILON_FRIG) >= start && (mstime - EPSILON_FRIG) <= end)
				return i;
			continue;
		}

		if(end < start)
		{
			if(((mstime + EPSILON_FRIG) >= start) || ((mstime - EPSILON_FRIG) <= end))
				return i;
			continue;

		}
	}

	return -1;
}







/*
	Function	:	APB_SetupRangeBlend
	Useage		:	Setup A Range Blend
*/

void APB_SetupRangeBlend(ANIMATION_PLAYBACK *papb,ANIMATION_BLENDRANGE* pabr,int ridx,int flags)
{
	float sd,dd;
	float sstart,send;
	float dstart,dend;

	// Setup The Blend

	papb->animation.dest.flags  = flags;
	papb->animation.dest.anim   = AL_FindAnimation(papb->pao,pabr->destID);

//	if(papb->animation.dest.anim->animID == STRING_GetHashValue("LARA_RUN"))
//		dbgLog(1,TRUE,"Range Blend - %s",papb->animation.dest.anim->name);

//	if(papb->animation.dest.anim->animID == STRING_GetHashValue("LARA_RUN") &&
//		papb->animation.source.anim->animID == STRING_GetHashValue("LARA_RUN_TURN180_MIRROR"))
//	{
//		dbgLog(1,TRUE,"HERE");
//	}

	// Calc Destination Blend Time

	sstart = pabr->BlendClips[ridx].source.msStart;
	send   = pabr->BlendClips[ridx].source.msEnd;
	dstart = pabr->BlendClips[ridx].dest.msStart;
	dend   = pabr->BlendClips[ridx].dest.msEnd;

	APB_GetFrameRange(&papb->animation,sstart,send,&sd);
	APB_GetFrameRange(&papb->animation,dstart,dend,&dd);

//	dbgLog("%f %f %f %f",APB_MSToFrame(sstart),APB_MSToFrame(send),APB_MSToFrame(dstart),APB_MSToFrame(dend));

	if (papb->animation.source.time >= sstart)
		papb->animation.dest.time = dstart + ((papb->animation.source.time - sstart) * (dd/sd));
	else
		papb->animation.dest.time = dstart;

	papb->animation.blend.msBlend   = pabr->BlendClips[ridx].msBlendTime;

	papb->animation.blend.msBlendTime  = 0;
	papb->animation.blend.pmsBlendTime = 0;

//	dbgLog("Blend Range %s Time %f",papb->animation.dest.anim->name,APB_MSToFrame(papb->animation.dest.time));

	APB_InitFrames(papb,&papb->animation.dest,papb->animation.dest.time);

	// Set Blend Flags

	papb->flags &= (~APBFLAG_BLENDWAIT);
	papb->flags |= APBFLAG_BLEND;
	papb->flags |= pabr->BlendClips[ridx].flags;
}





BOOL APB_OverlayAnimation(ANIMATION_PLAYBACK* papb,unsigned int idx,unsigned int animID,float startTime,float blendtime,int flags)
{
	ANIMATION* animation;

	if(papb->overlay[idx].flags&APBFLAG_ENDOVERLAY)
		return FALSE;

	if(papb->overlay[idx].flags&APBFLAG_HIDEOVERLAY)
		return FALSE;


	animation = AL_FindAnimation(papb->pao,animID);

	if(!animation)
	{
		#if defined( PS2) || defined( _GAME_PC_)
			dbgLog(1,0,"APB_OverlayAnimation : Attempt To Play Invalid Anim %x",animID);
		#endif
		return FALSE;
	}

	papb->overlay[idx].source.loopcnt = 0;

	if(!(papb->overlay[idx].flags&APBFLAG_OVERLAY))
	{
		papb->overlay[idx].flags |= APBFLAG_OVERLAY;

		// Set The Current Animation As The Source For The Overlay

		papb->overlay[idx].source.anim   = papb->animation.source.anim;
		papb->overlay[idx].source.time   = papb->animation.source.time;
		papb->overlay[idx].source.flags  = papb->animation.source.flags;
		papb->overlay[idx].source.msperframe = papb->animation.source.msperframe;

		APB_InitFrames(papb,&papb->overlay[idx].source,papb->animation.source.time);
	}

	papb->overlay[idx].flags &= ~(APBFLAG_OVERLAYBLENDCOMPLETE|APBFLAG_OVERLAYBLEND);
	papb->overlay[idx].source.flags &= ~APBFLAG_FINISHED;


	// Set The Destination as The Requested Overlay Anim

	papb->overlay[idx].dest.anim   = animation;
	papb->overlay[idx].dest.time   = startTime;
	papb->overlay[idx].dest.flags  = flags;
	papb->overlay[idx].dest.msperframe  = papb->msPerFrame;

	APB_InitFrames(papb,&papb->overlay[idx].dest,startTime);

	// Set The Blend Flag

	papb->overlay[idx].blend.msBlend      = blendtime;
	papb->overlay[idx].blend.msBlendTime  = 0;
	papb->overlay[idx].blend.pmsBlendTime = 0;
	papb->overlay[idx].flags |= APBFLAG_OVERLAYBLEND;

	APB_GetFrames(papb,&papb->overlay[idx].source);
	APB_GetFrames(papb,&papb->overlay[idx].dest);

	return TRUE;
}





void APB_PlayOverlay(ANIMATION_PLAYBACK* papb,unsigned int idx,unsigned int animID,float startTime,int flags)
{
	ANIMATION* animation;
//	float currentframe;

	papb->overlay[idx].flags &= ~APBFLAG_OVERLAYBLEND;
	papb->overlay[idx].flags |= APBFLAG_OVERLAY;

	animation = AL_FindAnimation(papb->pao,animID);

	// Set The Destination as The Requested Overlay Anim

	papb->overlay[idx].source.loopcnt = 0;

	papb->overlay[idx].source.anim   = animation;
	papb->overlay[idx].source.time   = startTime;
	papb->overlay[idx].source.flags  = flags;

//	currentframe = APB_MSToFrame(papb->overlay.source.time);

//	papb->overlay.source.frame0 = (int) currentframe;
//	papb->overlay.source.frame1 = papb->overlay.source.frame0 + 1;
//	papb->overlay.source.tval   = currentframe - papb->overlay.source.frame0;

//	papb->overlay.source.prevframe0 = 0;
//	papb->overlay.source.prevframe1 = 0;
//	papb->overlay.source.prevtval   = 0;

	APB_InitFrames(papb,&papb->overlay[idx].source,startTime);


}



void APB_ClearPositionInfo(ANIMATION_POSITIONINFO* data)
{
	FRAMEINFO* p;
	p = data->pFrameInfo;
	memclear(data,sizeof(ANIMATION_POSITIONINFO));
	data->pFrameInfo = p;
}


void APB_ClearAllCache(ANIMOBJECT* pao)
{
	memclear(pao->apb->animation.source.pFrameInfo,sizeof(FRAMEINFO) * pao->hierachy->nNodes * AT_TOTAL);
	memclear(pao->apb->animation.dest.pFrameInfo  ,sizeof(FRAMEINFO) * pao->hierachy->nNodes * AT_TOTAL);

	if(!(pao->apb->flags&APBFLAG_ANIMATEDSET))
	{
		if(pao->apb->overlay[0].source.pFrameInfo)
			memclear(pao->apb->overlay[0].source.pFrameInfo,sizeof(FRAMEINFO) * pao->hierachy->nNodes * AT_TOTAL);

		if(pao->apb->overlay[0].dest.pFrameInfo)
			memclear(pao->apb->overlay[0].dest.pFrameInfo,sizeof(FRAMEINFO)   * pao->hierachy->nNodes * AT_TOTAL);

		if(pao->apb->overlay[1].source.pFrameInfo)
			memclear(pao->apb->overlay[1].source.pFrameInfo,sizeof(FRAMEINFO) * pao->hierachy->nNodes * AT_TOTAL);

		if(pao->apb->overlay[1].dest.pFrameInfo)
			memclear(pao->apb->overlay[1].dest.pFrameInfo,sizeof(FRAMEINFO)   * pao->hierachy->nNodes * AT_TOTAL);
	}
}



void APB_DestToSource(ANIMATION_PLAYBACK* papb)
{
	BOOL bDestToSource = TRUE;
	FRAMEINFO* p;

	if (!(papb->flags & APBFLAG_BLENDWAIT))
	{

		if(papb->flags&APBFLAG_CONTROLBLEND)
		{
			if(papb->animation.blend.msBlendTime < (papb->animation.blend.msBlend/2.0f))
				bDestToSource = FALSE;
		}

		if(bDestToSource)
		{
			ASSERT(papb->animation.dest.anim);

			p = papb->animation.source.pFrameInfo;

			memcopy(&papb->animation.source,&papb->animation.dest,sizeof(ANIMATION_POSITIONINFO));

			papb->animation.source.pFrameInfo = p;

			if(papb->pao->hierachy)
			{
				if(papb->animation.source.pFrameInfo)
					memclear(papb->animation.source.pFrameInfo,sizeof(FRAMEINFO) * papb->pao->hierachy->nNodes * AT_TOTAL);
			}
		}
	}

	papb->flags &= (~APBFLAG_CONTROLBLEND);
	papb->flags &= (~APBFLAG_BLEND);
	papb->flags &= (~APBFLAG_BLENDWAIT);
	papb->flags &= (~APBFLAG_XFADEVELOCITY);

	papb->animation.source.flags = 0;

	// Reset The LoopCnt

	papb->animation.source.loopcnt = 0;

//	p = papb->animation.dest.pFrameInfo;
//	memclear(&papb->animation.dest,sizeof(ANIMATION_POSITIONINFO));
//	papb->animation.dest.pFrameInfo = p;

	if(bDestToSource)
		APB_ClearPositionInfo(&papb->animation.dest);


	if(papb->pao->hierachy)
	{
		if(papb->animation.source.pFrameInfo)
			memclear(papb->animation.dest.pFrameInfo,sizeof(FRAMEINFO) * papb->pao->hierachy->nNodes * AT_TOTAL);
	}

}







ANIMATION_LINK* AL_FindBlendLink(ANIMATION* anim,unsigned int animID)
{
	// ToDo : Make Binary Cut

	uint32 i;

	for(i=0;i<anim->nLinkRanges;i++)
	{
		if(anim->Links[i].destID == animID)
			return &anim->Links[i];
	}

	return NULL;
}










void APB_SetOverlayTriggers(ANIMATION_PLAYBACK* papb)
{
	if (!(papb->flags & APBFLAG_TRIG_OVERLAY))
	{
		uint32 i;

		for(i = 0; i < 128; i++)
		{
			if (papb->trigger_bones[i >> 5] & (1 << (i & 31)))
			{
				papb->bonecontrol[i].ControlBone = NULL;
				papb->bonecontrol[i].flags |= BONEFLAG_OVERRIDEANIM | BONEFLAG_OVERLAY_CH0;
			}
		}

		papb->flags |= APBFLAG_TRIG_OVERLAY;
	}
}

void APB_ClearOverlayTriggers(ANIMATION_PLAYBACK* papb)
{
	if (papb->flags & APBFLAG_TRIG_OVERLAY)
	{
		uint32 i;

		for(i = 0; i < 128; i++)
		{
			if (papb->trigger_bones[i >> 5] & (1 << (i & 31)))
			{
				papb->bonecontrol[i].ControlBone = NULL;
				papb->bonecontrol[i].flags &= ~(BONEFLAG_OVERRIDEANIM | BONEFLAG_OVERLAY_CH0);
			}
		}

		papb->overlay[0].flags&=~(APBFLAG_OVERLAY|APBFLAG_OVERLAYBLEND);
		papb->overlay[1].flags&=~(APBFLAG_OVERLAY|APBFLAG_OVERLAYBLEND);
		papb->flags &= ~APBFLAG_TRIG_OVERLAY;
	}
}

/*
	Function	:	APB_BlendToAnimation
	Useage		:	Blend To The Specified Animation
*/

BOOL APB_BlendToAnimation(ANIMATION_PLAYBACK* papb,unsigned int animID,float startTime,float blendtime,int flags)
{
	ANIMATION* animation;
	ANIMATION_BLENDRANGE* pabr = NULL;
	ANIMATION_LINK* pabl = NULL;
	ANIMATION_LINKPATH* animlink = NULL;

	assert(papb != NULL);
	assert(animID != 0);

//	APB_ClearOverlayTriggers(papb);

	if(!(flags&APBFLAG_FORCEBLEND) && (!(papb->flags&APBFLAG_CONTROLBLEND)))
	{
//		if(papb->animation.source.flags&APBFLAG_LOCK)
//			return FALSE;

		if(papb->flags&APBFLAG_BLEND ||
		   papb->flags&APBFLAG_BLENDWAIT ||
		   papb->flags&APBFLAG_LINKWAIT ||
		   papb->flags&APBFLAG_DELAYBLEND)
			return FALSE;
	}
	else
	{
		if(papb->flags&APBFLAG_BLEND || papb->flags&APBFLAG_BLENDWAIT || papb->flags&APBFLAG_CONTROLBLEND)
			APB_DestToSource(papb);

		papb->flags &= ~APBFLAG_DELAYBLEND;
		papb->flags &= ~APBFLAG_FOLLOWLINK;
		papb->flags &= ~APBFLAG_NOALTERNATEANIM;
		papb->flags &= ~APBFLAG_LINKWAIT;
	}

	papb->flags &= ~APBFLAG_BLENDCOMPLETE;

	if(flags & APBFLAG_ALLOWSAMEANIMBLEND)
	{
		// allow a blend to the same animation as the source anim
		flags &= ~APBFLAG_ALLOWSAMEANIMBLEND;
	}
	else if(papb->animation.source.anim)
	{
		if(papb->animation.source.anim->animID == animID)
		{
			if(papb->animation.source.flags&APBFLAG_FINISHED)
			{
				APB_InitFrames(papb,&papb->animation.source,startTime);
			}
			else
			{
				papb->animation.source.flags |= flags;
				return TRUE;
			}
		}
	}

	animation = AL_FindAnimation(papb->pao,animID);

	if(!animation) return FALSE;


	// Check For A Link

	if(papb->animation.source.anim)
	{
		if(!(flags&APBFLAG_NOFOLLOWLINK))
		{
			pabl = AL_FindBlendLink(papb->animation.source.anim,animID);

			if(pabl)
			{
				int ridx = 0;

	//			dbgLog(1,TRUE,"Link Blend - %s",animation->name);

				if(pabl->LLinkID)
				{
					pabr = AL_FindBlendRange(papb->animation.source.anim,pabl->LLinkID);
					ridx = APB_InBlendRange(&papb->animation,pabr);
				}

				if(pabl->RLinkID && ridx == -1)
				{
					pabr = AL_FindBlendRange(papb->animation.source.anim,pabl->RLinkID);
					ridx = APB_InBlendRange(&papb->animation,pabr);
				}

				if(ridx != -1)
				{
					APB_SetupRangeBlend(papb,pabr,ridx,flags);
					papb->flags |= APBFLAG_DELAYBLEND;
					papb->linkrange = pabl;
					return TRUE;
				}
				else
				{
					papb->flags |= APBFLAG_LINKWAIT;
					papb->linkrange = pabl;
					return TRUE;
				}
			}
		}
	}



	// Check For An Animation Link Path Between The Anims


	if (!(flags & APBFLAG_NOFOLLOWLINK))
	{
		ANIMATION* pa = animation;

		if(!(papb->flags&APBFLAG_FOLLOWLINK))
		{
			if((animlink = AL_CheckForLinkPath(papb->pao,papb->animation.source.anim->animID,animID)))
			{
				if(animlink->nlinks)
				{
					animID = animlink->links[0];

					animation = AL_FindAnimation(papb->pao,animID);

					if(animation)
					{
						papb->flags		   |= APBFLAG_FOLLOWLINK;
						papb->flags		   |= flags&APBFLAG_NOALTERNATEANIM;
						papb->linkpath		= animlink;
						papb->linkpathidx	= 1;
					}
					else
						animation = pa;	// For Editor If LINKPATHS have changed...
				}
			}
		}
	}



	// Check For Alternate Animation
	if(!(flags&APBFLAG_NOALTERNATEANIM))
	{
		if(animation->altanimID)
		{
			ANIMATION* altanimation;
			ANIMATION_BLENDRANGE* pabr0;
			ANIMATION_BLENDRANGE* pabr1;

			altanimation = AL_FindAnimation(papb->pao,animation->altanimID);

			pabr0 = AL_FindBlendRange(papb->animation.source.anim,animID);
			pabr1 = AL_FindBlendRange(papb->animation.source.anim,animation->altanimID);

			if(!pabr0 && pabr1)
			{
				animation = altanimation;			// In Alternate Animation Range
				animID    = altanimation->animID;
			}
			else if(pabr0 && pabr1)
			{
				int ridx;

				// Check If In A Range Then Select

				ridx = APB_InBlendRange(&papb->animation,pabr0);

				if(ridx == -1)
				{
					ridx = APB_InBlendRange(&papb->animation,pabr1);

					if(ridx != -1)
					{
						animation = altanimation;			// In Alternate Animation Range
						animID    = altanimation->animID;

//						dbgLog(TRUE,FALSE,"%s",animation->name);

					}
					else
					{
						int d0,d1;

						// Not In either Range So Find The Closest One

						d0 = APB_FindClosestBlendStart(&papb->animation,pabr0);
						d1 = APB_FindClosestBlendStart(&papb->animation,pabr1);

						if(d1 < d0)
						{
							animation = altanimation;			// In Alternate Animation Range
							animID    = altanimation->animID;

//							dbgLog(TRUE,FALSE,"%s",animation->name);
						}
					}
				}
			}
		}

	}




	// Check For A Range

	if(!(flags&APBFLAG_NOBLEND))
	{
		if(papb->animation.source.anim)
			pabr = AL_FindBlendRange(papb->animation.source.anim,animID);
	}

	// Ensure The Blend Ranges Have Not Been Passed
	if(pabr && (!(papb->animation.source.anim->flags&APBFLAG_LOOP)))
	{
		uint32 i;
		BOOL bFlag = TRUE;

		for(i=0;i<pabr->nClips;i++)
		{
			// Check If Current time is before ranges
			if(papb->animation.source.time <= pabr->BlendClips[i].source.msEnd)
			{
				bFlag = FALSE;
				break;
			}
		}

		// If still set Anim has past The Ranges so clear it

		if(bFlag)
			pabr = NULL;
	}

	if(pabr == NULL)
	{
		animation = AL_FindAnimation(papb->pao,animID);

		if(animation == NULL)
			return FALSE;

//		assert(animation != NULL);

		if(blendtime == 0.0f)
			blendtime = 1.0f;

		// No Range So Blend Anyway

		papb->animation.dest.flags  = flags;
		papb->animation.dest.anim   = animation;
		papb->animation.blend.msBlend     = blendtime;
		papb->animation.blend.msBlendTime  = 0;
		papb->animation.blend.pmsBlendTime = 0;


		APB_InitFrames(papb,&papb->animation.dest,startTime);

//		papb->animation.dest.time   = startTime;
//		papb->animation.dest.prevframe0 = 0;
//		papb->animation.dest.prevframe1 = 0;
//		papb->animation.dest.prevtval = 0;

		// Set Blend Flag

		papb->flags |= APBFLAG_BLEND;
		papb->flags |= APBFLAG_XFADEVELOCITY;
	}
	else
	{
		int ridx;

		ridx = APB_InBlendRange(&papb->animation,pabr);

		if(ridx != -1)
		{
			APB_SetupRangeBlend(papb,pabr,ridx,flags);
		}
		else
		{
			if(!(flags&APBFLAG_NOBLENDWAIT))
			{
				// Got To Wait Until In Range
				papb->flags |= APBFLAG_BLENDWAIT;
				papb->blendrange = pabr;
			}
			else
				return FALSE;
		}

	}

	return TRUE;
}





void APB_GetFrames(ANIMATION_PLAYBACK* papb,ANIMATION_POSITIONINFO* pos)
{
	float currentframe;
	int f0,f1;
	float t;
	int nFrames;

	pos->prevframe0  = pos->frame0;
	pos->prevframe1  = pos->frame1;
	pos->prevtval    = pos->tval;

	currentframe = APB_MSToFrame(pos->time);

	nFrames	 = pos->anim->nFrames;

	f0 = (int) currentframe;
	f1 = f0 + 1;

	if(pos->anim->flags&APBFLAG_LOOP)
		nFrames--;
	else
	{
		if(f1 > nFrames-1)
			f1 = nFrames-1;
		if(f0 < 0)
			f0 = 0;
	}

	// ++ IF LOOPING

	if(pos->anim->flags&APBFLAG_LOOP)
	{
		pos->flags &= ~APBFLAG_LOOPED;

		if(f1 > nFrames)
			f1 -= nFrames;

		if(currentframe < 0.0f)
		{
			currentframe += nFrames;
			f0 = (int) currentframe;
			f1 = f0 + 1;

			if(f1 > (int) nFrames)
				f1 -= (int) nFrames;
		}
		else if(f0 >= nFrames)
		{
//			pos->flags &= ~APBFLAG_LOCK;

			currentframe -= nFrames;

			f0 = (int) currentframe;
			f1 = f0 + 1;

			if(f1 > (int) nFrames)
				f1 -= (int) nFrames;

			pos->flags |= APBFLAG_LOOPED;		// Set Looped Flag
	 		pos->loopcnt++;						// Increase Loop Counter
		}
	}
	else
	{
		int cmp;

		cmp = (int) (currentframe);

		if(cmp >= nFrames-1)
		{
			currentframe = (float) nFrames-1;
			f0 = nFrames-1;
			f1 = nFrames-1;

			pos->flags |=  APBFLAG_FINISHED;
//			pos->flags &= ~APBFLAG_LOCK;
		}

		if(f1 <= 0)
		{
			currentframe = 0;
			f0 = 0;
			f1 = 0;
			pos->flags |= APBFLAG_FINISHED;
//			pos->flags &= ~APBFLAG_LOCK;
		}

	}

	t = currentframe - f0;

	pos->time   = APB_FrameToMS((currentframe));
	pos->frame0 = f0;
	pos->frame1 = f1;
	pos->tval   = t;

}





/*
	Function	:	APB_StopAnimation
	Useage		:	Stop The Animation
*/

void APB_StopAnimation(ANIMATION_PLAYBACK* pab)
{
	pab->flags &= ~APBFLAG_PLAY;
	pab->animation.source.flags &= ~APBFLAG_FINISHED;
	mathUnitVector(&pab->velocity);

	if(pab->flags&APBFLAG_ANIMATEDSET)
	{
		pab->flags = APBFLAG_ANIMATEDSET;
	}
	else
		pab->flags = 0;

}



void APB_PauseAnimation(ANIMATION_PLAYBACK* pab)
{
	pab->flags &= ~APBFLAG_PLAY;
	mathUnitVector(&pab->velocity);
}





/*
	Function	:	APB_StartAnimation
	Useage		:	Start The Animation
*/

void APB_StartAnimation(ANIMATION_PLAYBACK* pab)
{
	if(pab->animation.source.anim)
		pab->flags |= APBFLAG_PLAY;
}




//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------


/*
	Function	:	AL_Initialise
	Useage		:	Initialise Animation List
*/

void AL_Initialise(ANIMATION_LIST* pal,int nInitialEntries)
{
	assert(pal!=NULL);

	pal->nEntries  = 0;
	pal->animation = NULL;

	pal->linktable.nEntries = 0;
	pal->linktable.animlink = NULL;

	if(nInitialEntries)
	{
		pal->nAllocEntries = nInitialEntries;
		pal->animation     = (ANIMATION**) memalloc(sizeof(ANIMATION*) * nInitialEntries);

		assert(pal->animation != NULL);
	}
	else
		pal->nAllocEntries = 0;
}


void AL_DestroyLinkTable(ANIMATION_LIST* pal)
{
	uint32 i;

	if(!pal) return;

	for(i=0;i<pal->linktable.nEntries;i++)
		memfree(pal->linktable.animlink[i].links);

	memfree(pal->linktable.animlink);

	pal->linktable.animlink = NULL;
	pal->linktable.nEntries = 0;
}



ANIMATION_LINKPATH* AL_CreateLink(ANIMATION_LIST* pal,uint32 sourceID,uint32 destID,uint32 nEntries)
{
	int idx;

	idx = pal->linktable.nEntries++;
	pal->linktable.animlink = memrealloc(pal->linktable.animlink,pal->linktable.nEntries*sizeof(ANIMATION_LINKPATH));

	pal->linktable.animlink[idx].nlinks = nEntries;
	pal->linktable.animlink[idx].links  = memalloc(nEntries*sizeof(uint32));

	pal->linktable.animlink[idx].sourceID = sourceID;
	pal->linktable.animlink[idx].destID   = destID;

	return &pal->linktable.animlink[idx];
}


void AL_AddLinkEntry(ANIMATION_LINKPATH* link,uint32 idx,uint32 hashID)
{
	link->links[idx] = hashID;
}




void AL_SortLinkPathList(ANIMATION_LIST* pal)
{
	uint32 i;
	int exit;

	assert(pal!=NULL);

	do
	{
		exit = TRUE;

		for(i=1;i<pal->linktable.nEntries;i++)
		{
			uint64 v0,v1;

			v0 = ((uint64)pal->linktable.animlink[i].sourceID<<32)   | pal->linktable.animlink[i].destID;
			v1 = ((uint64)pal->linktable.animlink[i-1].sourceID<<32) | pal->linktable.animlink[i-1].destID;

			if(v0 < v1)
			{
				ANIMATION_LINKPATH tabr;
				memcpy(&tabr,&pal->linktable.animlink[i],sizeof(ANIMATION_LINKPATH));
				memcpy(&pal->linktable.animlink[i],&pal->linktable.animlink[i-1],sizeof(ANIMATION_LINKPATH));
				memcpy(&pal->linktable.animlink[i-1],&tabr,sizeof(ANIMATION_LINKPATH));
				exit = FALSE;
			}
		}
	}
	while(!exit);
}





ANIMATION_LINKPATH* AL_CheckForLinkPath(ANIMOBJECT* pao,uint32 sourceID,uint32 destID)
{
	//** ToDo : Binary Cut & Sort Entries

#if !defined(PS2) && !defined(_GAME_PC_)


	uint32 i;
	ANIMATION_LIST* pal;
	pal = pao->pAnimationList;
	for(i=0;i<pal->linktable.nEntries;i++)
	{
		if(pal->linktable.animlink[i].sourceID == sourceID &&
		   pal->linktable.animlink[i].destID   == destID)
		   return &pal->linktable.animlink[i];
	}

#else
	int bottom;
	int top;
	int mid = 0;
	int found = FALSE;
	uint64 value0;
	uint64 value1;

	ANIMATION_LIST* pal = pao->pAnimationList;

	pal = pao->pAnimationList;

//	AL_SortLinkPathList(pal);

	bottom = 0;
	top    = pal->linktable.nEntries-1;

	value0 = ((uint64)sourceID<<32) | destID;

	while((top>=bottom) && !found)
	{
		mid = (top + bottom)>>1;

		value1 = ((uint64)pal->linktable.animlink[mid].sourceID<<32) | pal->linktable.animlink[mid].destID;

		if(value0 == value1)
			found = TRUE;
		else if(value0 > value1)
			bottom = mid + 1;
		else
			top = mid - 1;
	}

	if(found)
		return &pal->linktable.animlink[mid];
#endif

	return NULL;
}






void ANIMATION_Destroy(ANIMATION **ppAnimation)
{
	ANIMATION *pAnimation = *ppAnimation;

	{
		uint32 itrack;
		for( itrack=0; itrack < pAnimation->nTracks; itrack += 1)
		{
			int k;
			int idx;

			idx = 0;

			for(k=0;k<AT_TOTAL;k++)
			{
				if(pAnimation->data->tracks[itrack].flags&0x1)
				{
					void *p = pAnimation->data->tracks[itrack].animblock[idx];

					if( p != NULL)
					{
						memfree( p);


					}

					idx++;
				}


				pAnimation->data->tracks[itrack].flags>>=1;
			}

			memfree(pAnimation->data->tracks[itrack].animblock);

#if defined( ANIMATION_TRACK_USE_ANIMSIZE)
			memfree(pAnimation->data->tracks[itrack].animsize);
#endif

		}
	}

	{
		int k;
		for(k=0;k<6;k++)
		{
			if(pAnimation->data->vtrack.animblock[k] != NULL)
			{
				memfree( pAnimation->data->vtrack.animblock[k]);
			}
		}

		memfree(pAnimation->data->vtrack.animblock);

#if defined( ANIMATION_TRACK_USE_ANIMSIZE)
		memfree(pAnimation->data->vtrack.animsize);
#endif

	}

	memfree(pAnimation->data->tracks);

	{
//		uint32 ilink;
//		for( ilink = 0; ilink < pAnimation->nLinkRanges; ilink += 1)
//		{
//			memfree( pAnimation->Links[ ilink]);
//		}
	}

	if( pAnimation->Links != NULL)
	{
		memfree(pAnimation->Links);
	}
	if( pAnimation->keys != NULL)
	{
		memfree(pAnimation->keys);
	}

	memfree(pAnimation);
	*ppAnimation = NULL;
}


/*
	Function	:	AL_Destroy
	Useage		:	Free Animation List
*/

void AL_DestroyContent(ANIMATION_LIST* pal)
{
	if(pal)
	{
		ANIMATION_LIST *al = pal;
		uint32 i;

		AL_DestroyLinkTable(pal);

		for (i=0; i<al->nEntries; i++)
		{
			AL_ClearBlendRanges(al, al->animation[i]->animID);
		}

		for (i=0; i<al->nEntries; i++)
		{
			ANIMATION_Destroy(&al->animation[i]);
		}

		if( al->animation != NULL)
		{
			memfree(al->animation);
			al->animation = NULL;
		}
	}
}

/*
	Function	:	AL_Destroy
	Useage		:	Free Animation List
*/

void AL_Destroy(ANIMATION_LIST** pal)
{
	if(*pal)
	{
		ANIMATION_LIST *al = *pal;

		AL_DestroyContent( al);

		memfree(*pal);
		*pal = NULL;
	}
}





/*
	Function	:	AL_ActivateAnimationList
	Useage		:	Set The Active Animation List
*/

ANIMATION_LIST* AL_SetAnimationList(ANIMOBJECT* pao,ANIMATION_LIST* pal)
{
	ANIMATION_LIST* cpal;

//	assert(pal!=NULL);

	cpal = pao->pAnimationList;

	pao->pAnimationList = pal;

	return cpal;
}









/*
	Function	:	AL_AddAnimation
	Useage		:	Add Animation To Animation List
*/

int AL_AddAnimation(ANIMATION_LIST* pal,ANIMATION* pa)
{
	assert(pal!=NULL);
	assert(pa!=NULL);

	pal->nEntries++;

	if(pal->nEntries > pal->nAllocEntries)
	{
		// Add A New Entry
		pal->animation = (ANIMATION**) memrealloc(pal->animation,pal->nEntries*sizeof(ANIMATION*));
		pal->nAllocEntries++;

		assert(pal->animation != NULL);
	}

	// Store The Animation

	pal->animation[pal->nEntries-1] = pa;

	return pal->nEntries;
}




/*
	Function	:	AL_SortBlendList
	Useage		:	Sort The Animation Blend List
*/

void AL_SortBlendList(ANIMATION* pa)
{
	uint32 i;
	int exit;

	assert(pa!=NULL);

	do
	{
		exit = TRUE;

		for(i=1;i<pa->nBlendRanges;i++)
		{
			if(pa->BlendRanges[i].destID < pa->BlendRanges[i-1].destID)
			{
				ANIMATION_BLENDRANGE tabr;
//				tabr = pa->BlendRanges[i];
//				pa->BlendRanges[i]   = pa->BlendRanges[i-1];
//				pa->BlendRanges[i-1] = tabr;

				memcpy(&tabr,&pa->BlendRanges[i],sizeof(ANIMATION_BLENDRANGE));
				memcpy(&pa->BlendRanges[i],&pa->BlendRanges[i-1],sizeof(ANIMATION_BLENDRANGE));
				memcpy(&pa->BlendRanges[i-1],&tabr,sizeof(ANIMATION_BLENDRANGE));


				exit = FALSE;
			}
		}
	}
	while(!exit);
}




/*
	Function	:	AL_AddBlendRange
	Useage		:	Add Animation Blend Range
*/

void AL_AddBlendRange(ANIMATION_LIST*pal,unsigned int animID,ANIMATION_BLENDRANGE* abr)
{
	ANIMATION* pa = NULL;

	assert(pal!=NULL);
	assert(abr!=NULL);

	// Find The Animation

	pa = AL_FindAnimationFromList(pal,animID);

	assert(pa!=NULL);

	// Add A New Entry

	pa->BlendRanges = (ANIMATION_BLENDRANGE*) memrealloc(pa->BlendRanges,(pa->nBlendRanges+1)*sizeof(ANIMATION_BLENDRANGE));

	assert(pa->BlendRanges != NULL);




//	pa->BlendRanges[pa->nBlendRanges] = abr;

	memcpy(&pa->BlendRanges[pa->nBlendRanges],abr,sizeof(ANIMATION_BLENDRANGE));

	pa->nBlendRanges++;

	// Sort The Blend Range List

	AL_SortBlendList(pa);
}

/*
	Function	:	AL_ClearBlendRange
	Useage		:	Remove any animation Blend Ranges
*/

void AL_ClearBlendRanges(ANIMATION_LIST*pal,unsigned int animID)
{
//	uint32 i;
	ANIMATION* pa = NULL;

	assert(pal!=NULL);

	// Find The Animation

	pa = AL_FindAnimationFromList(pal,animID);

	if (pa==NULL)
		return;

	assert(pa!=NULL);

//	for (i=0; i<pa->nBlendRanges; i++)
//	{
//		memfree(pa->BlendRanges[i]->BlendClips);
//		memfree(pa->BlendRanges[i]);
//	}

	if (pa->BlendRanges!=NULL)
		memfree(pa->BlendRanges);

	pa->BlendRanges = NULL;
	pa->nBlendRanges = 0;
}




ANIMATION_BLENDRANGE* AL_FindBlendRange(ANIMATION* anim,unsigned int animID)
{
	int bottom;
	int top;
	int mid = 0;
	int found = FALSE;
	unsigned int cmpid;

	if(!anim->nBlendRanges) return NULL;

	bottom = 0;
	top    = anim->nBlendRanges-1;

	while((top>=bottom) && !found)
	{
		mid = (top + bottom)>>1;

		cmpid = anim->BlendRanges[mid].destID;

		if(cmpid == animID)
			found = TRUE;
		else if(animID > cmpid)
			bottom = mid + 1;
		else
			top = mid - 1;
	}

	if(found)
		return &anim->BlendRanges[mid];

	return NULL;
}





/*
	Function	:	AL_SortAnimationList
	Useage		:	Sort The Animation List By animID
*/

void AL_SortAnimationList(ANIMATION_LIST* pal)
{
	uint32 i;
	int exit;

	assert(pal!=NULL);

	do
	{
		exit = TRUE;

		for(i=1;i<pal->nEntries;i++)
		{
			if(pal->animation[i]->animID < pal->animation[i-1]->animID)
			{
				ANIMATION* tap;
				tap = pal->animation[i];
				pal->animation[i]   = pal->animation[i-1];
				pal->animation[i-1] = tap;
				exit = FALSE;
			}
		}
	}
	while(!exit);
}


/*
	Function	:	AL_FindAnimation
	Useage		:	Return Animation Pointer From ID
*/


ANIMATION* AL_FindAnimation(ANIMOBJECT* pao,unsigned int animID)
{
	int bottom;
	int top;
	int mid = 0;
	int found = FALSE;
	unsigned int cmpid;
	ANIMATION_LIST* pal = pao->pAnimationList;

	bottom = 0;
	top    = pal->nEntries-1;

	while((top>=bottom) && !found)
	{
		mid = (top + bottom)>>1;

		cmpid = pal->animation[mid]->animID;

		if(cmpid == animID)
			found = TRUE;
		else if(animID > cmpid)
			bottom = mid + 1;
		else
			top = mid - 1;
	}

	if(found)
		return pal->animation[mid];

//	#if ( defined( PS2) || defined( _GAME_PC_) ) && !ISME(RT)
//		dbgLog(TRUE,TRUE,"AL_FindAnimation : Attempted To Find Invalid Anim %x",animID);
//	#endif
//	assert(0);

	return NULL;
}













/*
	Function	:	AL_FindAnimationFromList
	Useage		:	Return Animation Pointer From ID
*/

ANIMATION* AL_FindAnimationFromList(ANIMATION_LIST* pal,unsigned int animID)
{
	int bottom;
	int top;
	int mid = 0;
	int found = FALSE;
	unsigned int cmpid;

	bottom = 0;
	top    = pal->nEntries-1;

	while((top>=bottom) && !found)
	{
		mid = (top + bottom)>>1;

		cmpid = pal->animation[mid]->animID;

		if(cmpid == animID)
			found = TRUE;
		else if(animID > cmpid)
			bottom = mid + 1;
		else
			top = mid - 1;
	}

	if(found)
		return pal->animation[mid];

	#if defined( PS2) || defined( _GAME_PC_)
//		dbgLog(TRUE,TRUE,"Attempted To Play Invalid Anim %x",animID);
	#endif
//	assert(0);

	return NULL;
}







/*
	Function	:	AL_DeleteAnimation
	Useage		:	Add Animation To Animation List
*/

void AL_DeleteAnimation(ANIMATION_LIST* pal,unsigned int animID)
{
	uint32 i;

	for(i=0;i<pal->nEntries;i++)
	{
		if(pal->animation[i]->animID == animID)
			break;
	}

	i++;

	for(;i<pal->nEntries;i++)
		pal->animation[i-1] = pal->animation[i];

	pal->nEntries--;
}


//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

/*
	Function	:	HR_Initialise
	Useage		:	Initialise HIERACHY Structure
*/

void HR_Initialise(HIERACHY* phr,int nBones,BONE* pBones,int* pBoneMap)
{
	uint32 i;
	uint32 boneStack[64];
	uint32 stackptr;

	assert(phr!=NULL);
	assert(pBones!=NULL);

	phr->flags  = 0;

	if(pBoneMap)
	{
		phr->bonemap = pBoneMap;
		phr->flags  |= HIERACHYFLAGS_HASMIRROR;
	}

	phr->nNodes			= nBones;
	phr->nodes.boneptr  = pBones;

	HR_CreateBoneRef(phr);

	//
	// Setup the bone parents members for physics stuff.
	//
	stackptr=0;
	for(i=0;i<phr->nNodes;i++)
	{
		// Setup the parent member for this bone
		if (stackptr==0)
		{
			phr->nodes.boneptr[i].uParentBone = 0xffffffff;
		}
		else
		{
			phr->nodes.boneptr[i].uParentBone = boneStack[stackptr-1];
		}

		// Handle our bone stack.
		if (phr->nodes.boneptr[i].flags & BONEFLAG_MATRIXPUSH)
		{
			boneStack[stackptr] = i;
			stackptr++;
		}

		if (phr->nodes.boneptr[i].flags & BONEFLAG_MATRIXPOP)
		{
			stackptr--;
		}
	};

}




void HR_SetInitialise(HIERACHY* phr,int nBones,NODE* pBones)
{
	assert(phr!=NULL);
	assert(pBones!=NULL);

	phr->flags  = 0;
	phr->nNodes			= nBones;
	phr->nodes.nodeptr  = pBones;
	phr->bonemap = 0;
	HR_CreateNodeRef(phr);
}





/*
	Function	: HR_Destroy
	Useage		: Free Up The Hierachy
*/

void HR_Destroy(HIERACHY** phr)
{
	HIERACHY* h;

	h = *(phr);

	if(h)
	{
		memfree(h->bonemap);
		memfree(h->nodeRefs.bone);
		memfree(h->nodes.boneptr);
		memfree(*phr);

		*phr = NULL;
	}
}



/*
	Function	:	HR_CreateBoneRef
	Useage		:	Create And Sort Bone Reference List
*/

void HR_CreateBoneRef(HIERACHY* phr)
{
	uint32 i;
	int exit;

	assert(phr!=NULL);

	phr->nodeRefs.bone = (BONE**) memalloc(phr->nNodes*(sizeof(BONE*)));

	assert(phr->nodeRefs.bone!=NULL);

	// Fill List With Pointers

	for(i=0;i<phr->nNodes;i++)
		phr->nodeRefs.bone[i] = &phr->nodes.boneptr[i];

	// Sort Ptr List

	do
	{
		exit = TRUE;

		for(i=1;i<phr->nNodes;i++)
		{
			if(phr->nodeRefs.bone[i]->boneID < phr->nodeRefs.bone[i-1]->boneID)
			{
				BONE* tbp;
				tbp = phr->nodeRefs.bone[i];
				phr->nodeRefs.bone[i]   = phr->nodeRefs.bone[i-1];
				phr->nodeRefs.bone[i-1] = tbp;
				exit = FALSE;
			}
		}
	}

	while(!exit);
}





void HR_CreateNodeRef(HIERACHY* phr)
{
	uint32 i;
	int exit;

	assert(phr!=NULL);

	phr->nodeRefs.node = (NODE**) memalloc(phr->nNodes*(sizeof(BONE*)));

	assert(phr->nodeRefs.node!=NULL);

	// Fill List With Pointers

	for(i=0;i<phr->nNodes;i++)
		phr->nodeRefs.node[i] = &phr->nodes.nodeptr[i];

	// Sort Ptr List

	do
	{
		exit = TRUE;

		for(i=1;i<phr->nNodes;i++)
		{
			if(phr->nodeRefs.node[i]->boneID < phr->nodeRefs.node[i-1]->boneID)
			{
				NODE* tbp;
				tbp = phr->nodeRefs.node[i];
				phr->nodeRefs.node[i]   = phr->nodeRefs.node[i-1];
				phr->nodeRefs.node[i-1] = tbp;
				exit = FALSE;
			}
		}
	}

	while(!exit);
}



NODE* HR_FindNode(HIERACHY* phr,unsigned int boneID)
{
	int bottom;
	int top;
	int mid = 0;
	int found = FALSE;
	unsigned int cmpid;

	bottom = 0;
	top    = phr->nNodes-1;

	while((top>=bottom) && !found)
	{
		mid = (top + bottom)>>1;

		cmpid = phr->nodeRefs.node[mid]->boneID;

		if(cmpid == boneID)
			found = TRUE;
		else if(boneID > cmpid)
			bottom = mid + 1;
		else
			top = mid - 1;
	}

	if(found)
		return phr->nodeRefs.node[mid];

//	assert(0);

	return NULL;
}

















/*
	Function	:	HR_FindBone
	Useage		:	Find And Return Bone* By ID
*/

BONE* HR_FindBone(HIERACHY* phr,unsigned int boneID)
{
	int bottom;
	int top;
	int mid = 0;
	int found = FALSE;
	unsigned int cmpid;

	bottom = 0;
	top    = phr->nNodes-1;

	while((top>=bottom) && !found)
	{
		mid = (top + bottom)>>1;

		cmpid = phr->nodeRefs.bone[mid]->boneID;

		if(cmpid == boneID)
			found = TRUE;
		else if(boneID > cmpid)
			bottom = mid + 1;
		else
			top = mid - 1;
	}

	if(found)
		return phr->nodeRefs.bone[mid];

//	assert(0);

	return NULL;
}





/*
	Function	:	HR_GetBoneIndex
	Useage		:	Find And Return index By ID
*/
int HR_GetBoneIndex(HIERACHY * phr, unsigned int boneID)
{
	BONE * b;

	b = HR_FindBone(phr, boneID);
	if (b==NULL)
		return -1;

	return (int)(b - phr->nodes.boneptr);
};




void HR_GetBoneBindMatrix(MATRIX * m, HIERACHY * phr, int boneIndex)
{
int iNumPushed=0, iPop, iBone;
MATRIX bind;
BONE * cbone;

	mathPushMatrix();											// Save the stack
	iNumPushed++;

	mathLoadUnitMatrix();

	cbone = phr->nodes.boneptr;

	for(iBone=0; iBone <= boneIndex; iBone++)
	{
		bind = cbone->bind;										// Bind matrix for the current bone

		// Pop the matrix for the current hierachy leaf off the stack.
		if (cbone->flags & BONEFLAG_MATRIXPOP)
		{
			mathPopMatrix();
			iNumPushed--;
		}

		// Push the matrix for the current hierachy leaf onto the stack.
		if (cbone->flags & BONEFLAG_MATRIXPUSH)
		{
			mathPushMatrix();
			iNumPushed++;
		}

		// Multiply the matrix onto the current leaf accumulator.
		mathMulMatrix(&bind);
		cbone++;
	}

	// The top matrix on the stack should now be the final matrix for the bone, so
	// retrieve it.
	mathStoreMatrix(m);


	// Because we may not have went to the end of a leaf, there may still be one or more matrices
	// still pushed onto the stack (including the ones before the start of the loop). So get rid of them.
	for(iPop=0; iPop < iNumPushed; iPop++)
		mathPopMatrix();

}




//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

/*
	Function	:	BONE_CreateList
	Useage		:	Create A Bonelist
*/

BONE* BONE_CreateList(int nEntries)
{
	BONE* bones;

	assert(nEntries!=0);

	bones = (BONE*) memalloc(sizeof(BONE)*nEntries);

	assert(bones!=NULL);

	memclear(bones,sizeof(BONE)*nEntries);

	return bones;
}


/*
	Function	:	BONE_SetName
	Useage		:	Set The Bones Name and Setup Bone Based On Bone Type
*/

void BONE_SetName(BONE* bone,char* name)
{
	int bonetype;

	assert(bone!=NULL);
	assert(name!=NULL);
	assert(strlen(name) < 32);

	strcpy(bone->name,name);
	bone->boneID = STRING_GetHashValue(name);
	bonetype = BONE_GetTypeFromName(bone);

	if(bonetype&BONETYPE_DYNAMIC)
	{
		bone->flags |= BONEFLAG_NOTRACK|BONEFLAG_DYNAMIC;

		if(bonetype&BONETYPE_DYNAMICBEGIN)
			bone->flags |= BONEFLAG_BEGINDYNAMIC;
	}
}



/*
	Function	:	BONE_GetTypeFromName
	Useage		:	Determine The Bone Type From The Name

	(BONENAME)_(BONEFLAGS)_(L/R)
*/

int BONE_GetTypeFromName(BONE* bone)
{
	int bonetype;
	char *name;
	char *flag;
	char *lr;
	char seps[] = "_";
	char namecopy[CHAR_NAME_LEN];

	assert(bone!=NULL);

	bonetype = 0;

	strcpy(namecopy,bone->name);

	// (BONENAME)
	name = strtok(namecopy,seps);

	// (BONEFLAGS)
	flag = strtok(NULL,seps);

	// (L/R)
	lr   = strtok(NULL,seps);

	if(!lr) lr = flag;

	if(lr)
	{
		if(!strcmp(lr,"L"))
			bonetype |= BONETYPE_LEFT;
		else if(!strcmp(lr,"R"))
			bonetype |= BONETYPE_RIGHT;

		if(!strcmp(flag,"DYNAMIC"))
			bonetype |= BONETYPE_DYNAMIC;
		if(!strcmp(flag,"DYNAMICBEGIN"))
			bonetype |= BONETYPE_DYNAMIC|BONETYPE_DYNAMICBEGIN;



	}

	return bonetype;
}




/*
	Function	:	BONE_CreateMirrorMap
	Useage		:	Create An Index Mirror Of Bones
*/

int* BONE_CreateMirrorMap(int nEntries,BONE* bones)
{
	int* bonemap;
	int  i,j,idx0,idx1;
	int  bonetype;

	bonemap = (int*) memalloc(nEntries*sizeof(int));

	assert(bonemap != NULL);

	idx0 = -1;

	for(i=0;i<nEntries;i++)
	{
		char name[CHAR_NAME_LEN];

		if(bones[i].flags&BONEFLAG_NOTRACK)
			continue;

		idx0++;

		strcpy(name,bones[i].name);

		bonemap[idx0] = idx0;

		bonetype = BONE_GetTypeFromName(&bones[i]);

		if(bonetype&BONETYPE_RIGHT)
			name[strlen(name)-1] = 'L';

		if(bonetype&BONETYPE_LEFT)
			name[strlen(name)-1] = 'R';

		// Now Find Corresponding Bone

		idx1 = -1;

		for(j=0;j<nEntries;j++)
		{
			if(bones[j].flags&BONEFLAG_NOTRACK)
				continue;

			idx1++;

			if(!strcmp(name,bones[j].name))
			{
//				Log("%s - %s",bones[i].name,bones[j].name);

				bonemap[idx0] = idx1;
				break;
			}
		}
	}

	return bonemap;
}



//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

void AT_GetFrameData(ANIMATION_POSITIONINFO* pos,int boneidx,ANIMATION_TRACK* track,int frame0,int frame1,FRAMEDATA* fd0,FRAMEDATA* fd1)
{
	int idx = 0;
	FRAMEINFO* finf;

	// Clear Variables

	mathUnitVector(&fd0->rot);
	mathUnitVector(&fd1->rot);
	fd0->rot.w = 1.0f;
	fd1->rot.w = 1.0f;
	mathUnitVector(&fd0->tran);
	mathUnitVector(&fd1->tran);
	fd0->scale.x = fd0->scale.y = fd0->scale.z = 1.0f;
	fd1->scale.x = fd1->scale.y = fd1->scale.z = 1.0f;

	// Translation

	if(track->flags&ANIMTRACK_HASTX)
	{
		finf = &pos->pFrameInfo[(boneidx * AT_TOTAL) + idx];
		if(!finf->pTrack) finf->pTrack = (ANIMBLOCK_HEADER*) track->animblock[idx];
		APB_GetTrackValue2(finf,frame0,&fd0->tran.x,1.0f/ANIM_TSCALE);
		APB_GetTrackValue2(finf,frame1,&fd1->tran.x,1.0f/ANIM_TSCALE);
		idx++;
	}

	if(track->flags&ANIMTRACK_HASTY)
	{
		finf = &pos->pFrameInfo[(boneidx * AT_TOTAL) + idx];
		if(!finf->pTrack) finf->pTrack = (ANIMBLOCK_HEADER*) track->animblock[idx];
		APB_GetTrackValue2(finf,frame0,&fd0->tran.y,1.0f/ANIM_TSCALE);
		APB_GetTrackValue2(finf,frame1,&fd1->tran.y,1.0f/ANIM_TSCALE);
		idx++;
	}

	if(track->flags&ANIMTRACK_HASTZ)
	{
		finf = &pos->pFrameInfo[(boneidx * AT_TOTAL) + idx];
		if(!finf->pTrack) finf->pTrack = (ANIMBLOCK_HEADER*) track->animblock[idx];
		APB_GetTrackValue2(finf,frame0,&fd0->tran.z,1.0f/ANIM_TSCALE);
		APB_GetTrackValue2(finf,frame1,&fd1->tran.z,1.0f/ANIM_TSCALE);
		idx++;
	}

	// Quaternion Rotation XYZ

	if(track->flags&ANIMTRACK_HASRX)
	{
		finf = &pos->pFrameInfo[(boneidx * AT_TOTAL) + idx];
		if(!finf->pTrack) finf->pTrack = (ANIMBLOCK_HEADER*) track->animblock[idx];
		APB_GetTrackValue2(finf,frame0,&fd0->rot.x,1.0f/ANIM_RSCALE);
		APB_GetTrackValue2(finf,frame1,&fd1->rot.x,1.0f/ANIM_RSCALE);
		idx++;
	}

	if(track->flags&ANIMTRACK_HASRY)
	{
		finf =  &pos->pFrameInfo[(boneidx * AT_TOTAL) + idx];
		if(!finf->pTrack) finf->pTrack = (ANIMBLOCK_HEADER*) track->animblock[idx];
		APB_GetTrackValue2(finf,frame0,&fd0->rot.y,1.0f/ANIM_RSCALE);
		APB_GetTrackValue2(finf,frame1,&fd1->rot.y,1.0f/ANIM_RSCALE);
		idx++;
	}

	if(track->flags&ANIMTRACK_HASRZ)
	{
		finf = &pos->pFrameInfo[(boneidx * AT_TOTAL) + idx];
		if(!finf->pTrack) finf->pTrack = (ANIMBLOCK_HEADER*) track->animblock[idx];
		APB_GetTrackValue2(finf,frame0,&fd0->rot.z,1.0f/ANIM_RSCALE);
		APB_GetTrackValue2(finf,frame1,&fd1->rot.z,1.0f/ANIM_RSCALE);
		idx++;
	}

	// Scale

	if(track->flags&ANIMTRACK_HASSX)
	{
		finf =  &pos->pFrameInfo[(boneidx * AT_TOTAL) + idx];
		if(!finf->pTrack) finf->pTrack = (ANIMBLOCK_HEADER*) track->animblock[idx];
		APB_GetTrackValue2(finf,frame0,&fd0->scale.x,1.0f/ANIM_SSCALE);
		APB_GetTrackValue2(finf,frame1,&fd1->scale.x,1.0f/ANIM_SSCALE);
		idx++;
	}

	if(track->flags&ANIMTRACK_HASSY)
	{
		finf =  &pos->pFrameInfo[(boneidx * AT_TOTAL) + idx];
		if(!finf->pTrack) finf->pTrack = (ANIMBLOCK_HEADER*) track->animblock[idx];
		APB_GetTrackValue2(finf,frame0,&fd0->scale.y,1.0f/ANIM_SSCALE);
		APB_GetTrackValue2(finf,frame1,&fd1->scale.y,1.0f/ANIM_SSCALE);
		idx++;
	}

	if(track->flags&ANIMTRACK_HASSZ)
	{
		finf =  &pos->pFrameInfo[(boneidx * AT_TOTAL) + idx];
		if(!finf->pTrack) finf->pTrack = (ANIMBLOCK_HEADER*) track->animblock[idx];
		APB_GetTrackValue2(finf,frame0,&fd0->scale.z,1.0f/ANIM_SSCALE);
		APB_GetTrackValue2(finf,frame1,&fd1->scale.z,1.0f/ANIM_SSCALE);
		idx++;
	}

	// Quaternion Rotation W

	if(track->flags&ANIMTRACK_HASRW)
	{
		finf = &pos->pFrameInfo[(boneidx * AT_TOTAL) + idx];
		if(!finf->pTrack) finf->pTrack = (ANIMBLOCK_HEADER*) track->animblock[idx];
		APB_GetTrackValue2(finf,frame0,&fd0->rot.w,1.0f/ANIM_RSCALE);
		APB_GetTrackValue2(finf,frame1,&fd1->rot.w,1.0f/ANIM_RSCALE);
		idx++;
	}

}























void AT_GetFrameDataVelocity(ANIMATION_TRACK* track,int frame0,VECTOR *tran,VECTOR *rot)
{
	int idx = 0;

	// Clear Variables

	mathUnitVector(tran);
	mathUnitVector(rot);

	// Translation

	if(track->flags&ANIMTRACK_HASTX)
		APB_GetTrackValue(track->animblock[idx++],frame0,&tran->x,1.0f/ANIM_VTSCALE);

	if(track->flags&ANIMTRACK_HASTY)
		APB_GetTrackValue(track->animblock[idx++],frame0,&tran->y,1.0f/ANIM_VTSCALE);

	if(track->flags&ANIMTRACK_HASTZ)
		APB_GetTrackValue(track->animblock[idx++],frame0,&tran->z,1.0f/ANIM_VTSCALE);

	if(track->flags&ANIMTRACK_HASRZ)
		APB_GetTrackValue(track->animblock[idx++],frame0,&rot->z,1.0f/ANIM_VRSCALE);
}





void AT_GetSetFrameData(ANIMATION_TRACK* track,int frame0,VECTOR *rot,VECTOR *tran,VECTOR *scale)
{
	// Clear Variables
	int idx = 0;
	mathUnitVector(rot);
	mathUnitVector(tran);
	scale->x = scale->y = scale->z = 1.0f;

	// Translation

	if(track->flags&ANIMTRACK_HASTX)
		APB_GetTrackValue(track->animblock[idx++],frame0,&tran->x,1.0f/ANIM_SETTSCALE);

	if(track->flags&ANIMTRACK_HASTY)
		APB_GetTrackValue(track->animblock[idx++],frame0,&tran->y,1.0f/ANIM_SETTSCALE);

	if(track->flags&ANIMTRACK_HASTZ)
		APB_GetTrackValue(track->animblock[idx++],frame0,&tran->z,1.0f/ANIM_SETTSCALE);

	// Quaternion Rotation

	if(track->flags&ANIMTRACK_HASRX)
		APB_GetTrackValue(track->animblock[idx++],frame0,&rot->x,1.0f/ANIM_SETRSCALE);

	if(track->flags&ANIMTRACK_HASRY)
		APB_GetTrackValue(track->animblock[idx++],frame0,&rot->y,1.0f/ANIM_SETRSCALE);

	if(track->flags&ANIMTRACK_HASRZ)
		APB_GetTrackValue(track->animblock[idx++],frame0,&rot->z,1.0f/ANIM_SETRSCALE);

	// Scale

	if(track->flags&ANIMTRACK_HASSX)
		APB_GetTrackValue(track->animblock[idx++],frame0,&scale->x,1.0f/ANIM_SETSSCALE);

	if(track->flags&ANIMTRACK_HASSY)
		APB_GetTrackValue(track->animblock[idx++],frame0,&scale->y,1.0f/ANIM_SETSSCALE);

	if(track->flags&ANIMTRACK_HASSZ)
		APB_GetTrackValue(track->animblock[idx++],frame0,&scale->z,1.0f/ANIM_SETSSCALE);

	if(track->flags&ANIMTRACK_HASRW)
		APB_GetTrackValue(track->animblock[idx++],frame0,&rot->w,1.0f/ANIM_SETRSCALE);
}




void AT_GetSetFrameData2(ANIMATION_POSITIONINFO* pos,int boneidx,ANIMATION_TRACK* track,int frame0,int frame1,FRAMEDATA* fd0,FRAMEDATA* fd1)
{
	int idx = 0;
	FRAMEINFO* finf;

	// Clear Variables

	mathUnitVector(&fd0->rot);
	mathUnitVector(&fd1->rot);
	fd0->rot.w = 1.0f;
	fd1->rot.w = 1.0f;
	mathUnitVector(&fd0->tran);
	mathUnitVector(&fd1->tran);
	fd0->scale.x = fd0->scale.y = fd0->scale.z = 1.0f;
	fd1->scale.x = fd1->scale.y = fd1->scale.z = 1.0f;

	// Translation

	if(track->flags&ANIMTRACK_HASTX)
	{
		finf = &pos->pFrameInfo[(boneidx * AT_TOTAL) + idx];
		if(!finf->pTrack) finf->pTrack = (ANIMBLOCK_HEADER*) track->animblock[idx];
		APB_GetTrackValue2(finf,frame0,&fd0->tran.x,1.0f/ANIM_SETTSCALE);
		APB_GetTrackValue2(finf,frame1,&fd1->tran.x,1.0f/ANIM_SETTSCALE);
		idx++;
	}

	if(track->flags&ANIMTRACK_HASTY)
	{
		finf = &pos->pFrameInfo[(boneidx * AT_TOTAL) + idx];
		if(!finf->pTrack) finf->pTrack = (ANIMBLOCK_HEADER*) track->animblock[idx];
		APB_GetTrackValue2(finf,frame0,&fd0->tran.y,1.0f/ANIM_SETTSCALE);
		APB_GetTrackValue2(finf,frame1,&fd1->tran.y,1.0f/ANIM_SETTSCALE);
		idx++;
	}

	if(track->flags&ANIMTRACK_HASTZ)
	{
		finf = &pos->pFrameInfo[(boneidx * AT_TOTAL) + idx];
		if(!finf->pTrack) finf->pTrack = (ANIMBLOCK_HEADER*) track->animblock[idx];
		APB_GetTrackValue2(finf,frame0,&fd0->tran.z,1.0f/ANIM_SETTSCALE);
		APB_GetTrackValue2(finf,frame1,&fd1->tran.z,1.0f/ANIM_SETTSCALE);
		idx++;
	}

	// Quaternion Rotation XYZ

	if(track->flags&ANIMTRACK_HASRX)
	{
		finf = &pos->pFrameInfo[(boneidx * AT_TOTAL) + idx];
		if(!finf->pTrack) finf->pTrack = (ANIMBLOCK_HEADER*) track->animblock[idx];
		APB_GetTrackValue2(finf,frame0,&fd0->rot.x,1.0f/ANIM_SETRSCALE);
		APB_GetTrackValue2(finf,frame1,&fd1->rot.x,1.0f/ANIM_SETRSCALE);
		idx++;
	}

	if(track->flags&ANIMTRACK_HASRY)
	{
		finf =  &pos->pFrameInfo[(boneidx * AT_TOTAL) + idx];
		if(!finf->pTrack) finf->pTrack = (ANIMBLOCK_HEADER*) track->animblock[idx];
		APB_GetTrackValue2(finf,frame0,&fd0->rot.y,1.0f/ANIM_SETRSCALE);
		APB_GetTrackValue2(finf,frame1,&fd1->rot.y,1.0f/ANIM_SETRSCALE);
		idx++;
	}

	if(track->flags&ANIMTRACK_HASRZ)
	{
		finf = &pos->pFrameInfo[(boneidx * AT_TOTAL) + idx];
		if(!finf->pTrack) finf->pTrack = (ANIMBLOCK_HEADER*) track->animblock[idx];
		APB_GetTrackValue2(finf,frame0,&fd0->rot.z,1.0f/ANIM_SETRSCALE);
		APB_GetTrackValue2(finf,frame1,&fd1->rot.z,1.0f/ANIM_SETRSCALE);
		idx++;
	}

	// Scale

	if(track->flags&ANIMTRACK_HASSX)
	{
		finf =  &pos->pFrameInfo[(boneidx * AT_TOTAL) + idx];
		if(!finf->pTrack) finf->pTrack = (ANIMBLOCK_HEADER*) track->animblock[idx];
		APB_GetTrackValue2(finf,frame0,&fd0->scale.x,1.0f/ANIM_SETSSCALE);
		APB_GetTrackValue2(finf,frame1,&fd1->scale.x,1.0f/ANIM_SETSSCALE);
		idx++;
	}

	if(track->flags&ANIMTRACK_HASSY)
	{
		finf =  &pos->pFrameInfo[(boneidx * AT_TOTAL) + idx];
		if(!finf->pTrack) finf->pTrack = (ANIMBLOCK_HEADER*) track->animblock[idx];
		APB_GetTrackValue2(finf,frame0,&fd0->scale.y,1.0f/ANIM_SETSSCALE);
		APB_GetTrackValue2(finf,frame1,&fd1->scale.y,1.0f/ANIM_SETSSCALE);
		idx++;
	}

	if(track->flags&ANIMTRACK_HASSZ)
	{
		finf =  &pos->pFrameInfo[(boneidx * AT_TOTAL) + idx];
		if(!finf->pTrack) finf->pTrack = (ANIMBLOCK_HEADER*) track->animblock[idx];
		APB_GetTrackValue2(finf,frame0,&fd0->scale.z,1.0f/ANIM_SETSSCALE);
		APB_GetTrackValue2(finf,frame1,&fd1->scale.z,1.0f/ANIM_SETSSCALE);
		idx++;
	}

	// Quaternion Rotation W

	if(track->flags&ANIMTRACK_HASRW)
	{
		finf = &pos->pFrameInfo[(boneidx * AT_TOTAL) + idx];
		if(!finf->pTrack) finf->pTrack = (ANIMBLOCK_HEADER*) track->animblock[idx];
		APB_GetTrackValue2(finf,frame0,&fd0->rot.w,1.0f/ANIM_SETRSCALE);
		APB_GetTrackValue2(finf,frame1,&fd1->rot.w,1.0f/ANIM_SETRSCALE);
		idx++;
	}

}







//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

/*
	Function	:	AO_Intialise
	Useage		:	Intialise ANIMOBJECT
*/


void AO_Initialise(ANIMOBJECT* pao)
{
	assert(pao!=NULL);
	pao->apb      = NULL;
	pao->userdata = 0;
	pao->hierachy = NULL;
	pao->selectedskin = NULL;
	mathUnitVector(&pao->target);

	pao->apb = (ANIMATION_PLAYBACK*) memalloc(sizeof(ANIMATION_PLAYBACK));

	APB_Initialise(pao);

	pao->apb->pao = pao;

	pao->flags = AO_VISIBLE|AO_ANIMATE;

	#if defined( _GAME_PC_)
		pao->psys_draw_cloth_list = NULL;
	#endif
}


/*
	Function	:	AO_Destroy
	Useage		:	Free ANIMOBJECT
*/

void AO_Destroy(ANIMOBJECT** pao)
{
	ANIMOBJECT *ao;
	uint32 i;

	assert(pao!=NULL);

	ao = *pao;

	if (ao->apb)
	{
		memfree(ao->apb->bonecontrol);
		memfree(ao->apb);
		ao->apb = NULL;
	}

	if (ao->hierachy)
		HR_Destroy(&ao->hierachy);

	for (i=0; i<ao->nskin; i++)
	{
		memfree(ao->skin[i]->mesh);
	}

	MPH_RemoveMorphMesh(ao, NULL);
	MPH_RemoveMorphSequence(ao, NULL);

	memfree(ao->skin);
	memfree(ao);

	*pao = NULL;

	#if defined( _GAME_PC_)
		ASSERT( NULL == ao->psys_draw_cloth_list);
	#endif
}


/*
	Function	:	AO_SetPositionRel
	Useage		:	Set AO Position Relative
*/


void AO_SetPositionRel(ANIMOBJECT* pao,float x,float y,float z)
{
	pao->apb->position.x += x;
	pao->apb->position.y += y;
	pao->apb->position.z += z;
}


/*
	Function	:	AO_SetPositionAbs
	Useage		:	Set AO Position Absolute
*/


void AO_SetPositionAbs(ANIMOBJECT* pao,float x,float y,float z)
{
	pao->apb->position.x = x;
	pao->apb->position.y = y;
	pao->apb->position.z = z;
}



void AO_SetRotationAbs(ANIMOBJECT* pao,float x,float y,float z)
{
	pao->apb->rotation.x = x;
	pao->apb->rotation.y = y;
	pao->apb->rotation.z = z;
}



void AO_SetPositionLocal(ANIMOBJECT* pao)
{
	VECTOR v;

	mathPushMatrix();
	mathLoadUnitMatrix();
	mathRotZ(pao->apb->rotation.z);
	mathRotY(pao->apb->rotation.y);
	mathRotX(pao->apb->rotation.x);
	mathRot(&pao->apb->velocity,&v);
	mathPopMatrix();

	AO_SetPositionRel(pao,v.x,v.y,v.z);
}



/*
	Function	:	AO_SetSkinData
	Useage		:	Set Up The Skin Data Ptr
*/

void AO_SetSkinData(ANIMOBJECT* pao,char* name)
{
	uint32 id;
	uint32 i;

	assert(pao!=NULL);

	if(name == NULL)
	{
		pao->selectedskin = NULL;
		return;
	}

	// Set The Current Skin

	id = STRING_GetHashValue(name);

	for(i=0;i<pao->nskin;i++)
	{
		if(pao->skin[i]->skinID == id)
		{
			pao->selectedskin = pao->skin[i];
			break;
		}
	}

	if(pao->meshflags)
		memfree(pao->meshflags);

	AO_InitMeshFlags(pao);

}






MESH* AO_FindMesh(ANIMOBJECT* pao,uint32 meshID)
{
	uint32 i;

	if(pao->selectedskin)
	{
		for(i=0;i<pao->selectedskin->nmesh;i++)
		{
			if(pao->selectedskin->mesh[i].meshID == (uint32) meshID)
				return &pao->selectedskin->mesh[i];
		}
	}

	return NULL;
}






/*
	Function	:	AO_SetParentedObjectVisibility
	Useage		:	Set the visibility state of the parent object refered by nParentedObject
*/
BOOL AO_SetParentedObjectVisibility(ANIMOBJECT *pao, int nMeshID, int state)
{
	uint32 i;
	BOOL bFound = FALSE;

	assert(pao!=NULL);

#if defined(PS2) || defined (_GAME_PC_)

	if(pao->apb->flags&APBFLAG_ANIMATEDSET)
	{
		NODE* node;
		node = HR_FindNode(pao->hierachy,nMeshID);
		if(node)
		{
			if (state)
				pao->meshflags[node->boneidx] |=MESHFLAG_VISIBLE;
			else
				pao->meshflags[node->boneidx] &=~MESHFLAG_VISIBLE;
		}

		/*
		for(i=0;i<pao->hierachy->nNodes;i++)
		{
			if(pao->hierachy->nodes.nodeptr[i].boneID == (uint32) nMeshID)
			{
				if (state)
					pao->meshflags[i] |=MESHFLAG_VISIBLE;
				else
					pao->meshflags[i] &=~MESHFLAG_VISIBLE;

				bFound = TRUE;

				break;
			}
		}*/
	}
	else
#endif
	{
		if(pao->selectedskin)
		{
			for(i=0;i<pao->selectedskin->nmesh;i++)
			{
				if(pao->selectedskin->mesh[i].meshID == (uint32) nMeshID)
				{
					if (state)
						pao->meshflags[i] |=MESHFLAG_VISIBLE;
					else
						pao->meshflags[i] &=~MESHFLAG_VISIBLE;

					bFound = TRUE;

					break;
				}
			}
		}

	}

	return bFound;

}




BOOL APB_BlendFromOverlay(ANIMATION_PLAYBACK* papb,unsigned int idx,float msblend)
{
	FRAMEINFO* p;

	if(papb->overlay[idx].flags&APBFLAG_ENDOVERLAY)
		return FALSE;

// this fixes stephane's shit
	if (!(papb->overlay[idx].flags & APBFLAG_OVERLAY))
		return FALSE;

	// Now Blend From Overlay Animation To Current Source Animation

	papb->overlay[idx].source.flags &= ~APBFLAG_FINISHED;

	// Set The Destination as The Requested Overlay Anim

	p = papb->overlay[idx].dest.pFrameInfo;

	memcopy(&papb->overlay[idx].dest,&papb->animation.source,sizeof(ANIMATION_POSITIONINFO));

	papb->overlay[idx].dest.pFrameInfo = p;


	papb->overlay[idx].blend.msBlend      = msblend;
	papb->overlay[idx].blend.msBlendTime  = 0;
	papb->overlay[idx].blend.pmsBlendTime = 0;

	papb->overlay[idx].flags |= APBFLAG_OVERLAYBLEND|APBFLAG_ENDOVERLAY;

	if(papb->overlay[idx].dest.pFrameInfo)
		memclear(papb->overlay[idx].dest.pFrameInfo,sizeof(FRAMEINFO) * papb->pao->hierachy->nNodes * AT_TOTAL);

	if(papb->overlay[idx].source.pFrameInfo)
		memclear(papb->overlay[idx].source.pFrameInfo,sizeof(FRAMEINFO) * papb->pao->hierachy->nNodes * AT_TOTAL);

	return TRUE;
}






BOOL APB_HideOverlay(ANIMATION_PLAYBACK* papb,unsigned int idx,float msblend)
{
	FRAMEINFO* p;

	if(papb->overlay[idx].flags&APBFLAG_HIDEOVERLAY)
		return FALSE;

	if(papb->overlay[idx].flags&APBFLAG_OVERLAYBLEND)
		return FALSE;

	if(papb->overlay[idx].flags&APBFLAG_ENDOVERLAY)
		return FALSE;

// this fixes stephane's shit
	if (!(papb->overlay[idx].flags & APBFLAG_OVERLAY))
		return FALSE;

	// Now Blend From Overlay Animation To Current Source Animation

	papb->overlay[idx].source.flags &= ~APBFLAG_FINISHED;

	// Set The Destination as The Requested Overlay Anim

	p = papb->overlay[idx].dest.pFrameInfo;

	memcopy(&papb->overlay[idx].dest,&papb->animation.source,sizeof(ANIMATION_POSITIONINFO));

	papb->overlay[idx].dest.pFrameInfo = p;


	papb->overlay[idx].blend.msBlend      = msblend;
	papb->overlay[idx].blend.msBlendTime  = 0;
	papb->overlay[idx].blend.pmsBlendTime = 0;

	papb->overlay[idx].flags |= APBFLAG_OVERLAYBLEND|APBFLAG_HIDEOVERLAY;

	if(papb->overlay[idx].dest.pFrameInfo)
		memclear(papb->overlay[idx].dest.pFrameInfo,sizeof(FRAMEINFO) * papb->pao->hierachy->nNodes * AT_TOTAL);

	if(papb->overlay[idx].source.pFrameInfo)
		memclear(papb->overlay[idx].source.pFrameInfo,sizeof(FRAMEINFO) * papb->pao->hierachy->nNodes * AT_TOTAL);

	return TRUE;
}



BOOL APB_ShowOverlay(ANIMATION_PLAYBACK* papb,unsigned int idx,float msblend)
{
	if (!(papb->overlay[idx].flags & APBFLAG_OVERLAY))
		return FALSE;
	if (!(papb->overlay[idx].flags & APBFLAG_HIDEOVERLAY))
		return FALSE;

	papb->overlay[idx].flags &= ~APBFLAG_HIDEOVERLAY;
	papb->overlay[idx].flags &= ~APBFLAG_OVERLAY;
	APB_OverlayAnimation(papb,idx,papb->overlay[idx].source.anim->animID,papb->overlay[idx].source.time,msblend,0);

	return TRUE;
}







#ifdef PS2
inline ANIMATION_CONTROL* AO_CurrentAC(ANIMATION_PLAYBACK* papb,int node)
#else
ANIMATION_CONTROL* AO_CurrentAC(ANIMATION_PLAYBACK* papb,int node)
#endif
{
	// Check For Overlayed Animation

	if ((papb->overlay[0].flags & APBFLAG_OVERLAY) && ((papb->bonecontrol[node].flags & (BONEFLAG_OVERRIDEANIM|BONEFLAG_OVERLAY_CH0)) == (BONEFLAG_OVERRIDEANIM|BONEFLAG_OVERLAY_CH0)))
		return &papb->overlay[0];
	else if ((papb->overlay[1].flags & APBFLAG_OVERLAY) && ((papb->bonecontrol[node].flags & (BONEFLAG_OVERRIDEANIM|BONEFLAG_OVERLAY_CH1)) == (BONEFLAG_OVERRIDEANIM|BONEFLAG_OVERLAY_CH1)))
		return &papb->overlay[1];
	else
		return &papb->animation;
}





#ifdef PS2


inline void mathVectorInterpAdd(VECTOR* a,VECTOR* b,float t,VECTOR* c,VECTOR* d)
{
	asm("
		lqc2			vf1,0x00(%0)
		lqc2			vf2,0x00(%1)
		vsub.xyzw		vf3,vf2,vf1
		mfc1			$8, %2
		qmtc2    		$8, vf4
		vmulx.xyzw		vf3,vf3,vf4
		vadd.xyzw		vf3,vf3,vf1
		lqc2			vf1,0x00(%3)
		vadd.xyzw		vf3,vf3,vf1
		sqc2			vf3,0x00(%4)

	": : "r" (a), "r" (b), "rf" (t) , "r" (c) , "r" (d): "$8");
}


inline void mathVectorOneOver(VECTOR *s, VECTOR *d)
{
	asm __volatile__("

		lqc2			vf01,0x00(%0)

		vdiv	  		q,vf00w,vf01x
		vmaxw			vf02,vf00,vf00w
		vwaitq
		vmulq.x			vf03,vf02x,Q
		vdiv	  		q,vf00w,vf01y
		vwaitq
		vmulq.y			vf03,vf02y,Q
		vdiv	  		q,vf00w,vf01z
		vwaitq
		vmulq.z			vf03,vf02z,Q
#		vdiv	  		q,vf00w,vf01w
#		vwaitq
#		vmulq.w			vf03,vf02w,Q

		sqc2			vf03,0x00(%1)

	": : "r"(s), "r"(d) );
}


#endif







#if (!defined(PS2)) && (!defined(_GAME_PC_))

/*
	Function	:	AO_AnimateHierachy
	Useage		:	Animate The Hierachy
*/

void AO_AnimateHierachy(ANIMOBJECT* pao)
{
	uint32 i;
	uint32 trackidx;
	HIERACHY* phr;
	ANIMATION_PLAYBACK* papb;
	VECTOR inversescale;
	MATRIX previousMatrix;
	MATRIX currentMatrix;
	MATRIX m;
	uint32 joints;
	uint32 jointd;
	FRAMEDATA fd0;
	FRAMEDATA fd1;
	VECTOR qs,ts,ss;
	VECTOR qd,td,sd;
	BOOL bNoLOD;

	float tval;
	int pbone = 0;
	int bstackptr = 0;
	int bonestack[16];
	ANIMATION_CONTROL* pac = NULL;

	assert(pao!=NULL);

	papb = pao->apb;

	assert(papb != NULL);

	phr = pao->hierachy;

	assert(phr!=NULL);

	// Save Stack

	mathPushMatrix();

	// Intialise Inverse Scale

	inversescale.x = inversescale.y = inversescale.z = 1.0f;

	mathLoadVector(&inversescale);

	// Intialise Track Index

	trackidx = -1;

	// Loop Through All The Bones

	for(i=0;i<phr->nNodes;i++)
	{
		// Check Flags For PushPop
		if(papb->bonecontrol[i].flags&BONEFLAG_MATRIXPOP)
		{
			mathPopMatrix();
			bstackptr -- ;
			mathStoreVector(&inversescale);
			mathPopVector();
			mathStoreMatrix(&previousMatrix);
			pbone = bonestack[bstackptr];
		}

		if(papb->bonecontrol[i].flags&BONEFLAG_MATRIXPUSH)
		{
			mathPushMatrix();
			mathPushVector();
			mathLoadVector(&inversescale);
			bonestack[bstackptr+1] = bonestack[bstackptr];
			bstackptr++;
		}

		// Increase Track Index If Bone Has Animation Track

		if(!(papb->bonecontrol[i].flags&BONEFLAG_NOTRACK))
			trackidx++;

		pac = AO_CurrentAC(papb,i);



		bNoLOD = TRUE;

		if(!(pao->flags&AO_CUTSCENE))
		{
			if(pao->flags&AO_LODANIMATION)
			{
				if(phr->nodes.boneptr[i].flags&BONEFLAG_REMOVABLE)
					bNoLOD = FALSE;
			}
		}

		if(bNoLOD)
		{

			if(pac->source.anim && (papb->bonecontrol[i].flags&BONEFLAG_DYNAMIC)==0 )
			{
				BOOL bBlend;

				// Check Mirror Flag And Setup Joint Indices

				if(pac->source.anim->flags&APBFLAG_MIRROR)
					joints = phr->bonemap[trackidx];
				else
					joints = trackidx;


				if(papb->flags&APBFLAG_BLEND)
				{
					bBlend = TRUE;

					if(pac == &papb->overlay[0])
						if(!(pac->flags&APBFLAG_OVERLAYBLEND))
							bBlend = FALSE;

					if(pac == &papb->overlay[1])
						if(!(pac->flags&APBFLAG_OVERLAYBLEND))
							bBlend = FALSE;
				}
				else if(papb->overlay[0].flags&APBFLAG_OVERLAYBLEND
				&& ((papb->bonecontrol[i].flags & (BONEFLAG_OVERRIDEANIM|BONEFLAG_OVERLAY_CH0)) == (BONEFLAG_OVERRIDEANIM|BONEFLAG_OVERLAY_CH0)))
					bBlend = TRUE;
				else if(papb->overlay[1].flags&APBFLAG_OVERLAYBLEND
				&& ((papb->bonecontrol[i].flags & (BONEFLAG_OVERRIDEANIM|BONEFLAG_OVERLAY_CH1)) == (BONEFLAG_OVERRIDEANIM|BONEFLAG_OVERLAY_CH1)))
					bBlend = TRUE;
				else
					bBlend = FALSE;

				if(pac == &papb->overlay[0])
				{
					if(papb->overlay[0].flags&APBFLAG_ENDOVERLAY)
					{
						if(papb->bonecontrol[i].flags&BONEFLAG_OVERLAY_CH0)
							bBlend = TRUE;
					}
				}

				if(pac == &papb->overlay[1])
				{
					if(papb->overlay[1].flags&APBFLAG_ENDOVERLAY)
					{
						if(papb->bonecontrol[i].flags&BONEFLAG_OVERLAY_CH1)
							bBlend = TRUE;
					}
				}

				if(bBlend)
				{
					if(pac->dest.anim->flags&APBFLAG_MIRROR)
						jointd = phr->bonemap[trackidx];
					else
						jointd = trackidx;

					// Blending Between Animations

					// Get Source Frame

					AT_GetFrameData(&pac->source,i,&pac->source.anim->data->tracks[joints],pac->source.frame0,pac->source.frame1,&fd0,&fd1);

		//				dbgLog("Blend Source %d %d %f",pac->source.frame0,pac->source.frame1,pac->source.tval);

					if(pac->source.anim->flags&APBFLAG_MIRROR)
					{
						fd0.tran.x = -fd0.tran.x;
						fd1.tran.x = -fd1.tran.x;
						fd0.rot.x  = -fd0.rot.x;
						fd0.rot.w  = -fd0.rot.w;
						fd1.rot.x  = -fd1.rot.x;
						fd1.rot.w  = -fd1.rot.w;
					}

					// Linear Blend Translate & Scale

					mathVectorInterp(&fd0.tran,&fd1.tran,pac->source.tval,&ts);
					mathVectorInterp(&fd0.scale,&fd1.scale,pac->source.tval,&ss);

					// Slerp Quaternion Rotation

					mathQuatSlerp(&fd0.rot,&fd1.rot,pac->source.tval,&qs);

					// Get Destination Frame

					AT_GetFrameData(&pac->dest,i,&pac->dest.anim->data->tracks[jointd],pac->dest.frame0,pac->dest.frame1,&fd0,&fd1);

		//				dbgLog("Blend Dest %d %d %f",pac->dest.frame0,pac->dest.frame1,pac->dest.tval);

					if(pac->dest.anim->flags&APBFLAG_MIRROR)
					{
						fd0.tran.x = -fd0.tran.x;
						fd1.tran.x = -fd1.tran.x;
						fd0.rot.x  = -fd0.rot.x;
						fd0.rot.w  = -fd0.rot.w;
						fd1.rot.x  = -fd1.rot.x;
						fd1.rot.w  = -fd1.rot.w;
					}

					// Linear Blend Translate & Scale

					mathVectorInterp(&fd0.tran,&fd1.tran,pac->dest.tval,&td);
					mathVectorInterp(&fd0.scale,&fd1.scale,pac->dest.tval,&sd);

					// Slerp Quaternion Rotation

					mathQuatSlerp(&fd0.rot,&fd1.rot,pac->dest.tval,&qd);

					// Assign tval For Blend

					tval = pac->blend.msBlendTime/pac->blend.msBlend;

					tval = (1.0f - mathCos(M_PI*tval)) / 2.0f;


				}
				else
				{
					// Blending On The Same Animation

					AT_GetFrameData(&pac->source,i,&pac->source.anim->data->tracks[joints],pac->source.frame0,pac->source.frame1,&fd0,&fd1);

					tval = pac->source.tval;

					// Mirror

					if(pac->source.anim->flags&APBFLAG_MIRROR)
					{
						fd0.tran.x = -fd0.tran.x;
						fd1.tran.x = -fd1.tran.x;
						fd0.rot.x  = -fd0.rot.x;
						fd0.rot.w  = -fd0.rot.w;
						fd1.rot.x  = -fd1.rot.x;
						fd1.rot.w  = -fd1.rot.w;
					}

					mathCopyVector(&fd0.tran,&ts);
					mathCopyVector(&fd0.rot,&qs);
					mathCopyVector(&fd0.scale,&ss);

					mathCopyVector(&fd1.tran,&td);
					mathCopyVector(&fd1.rot,&qd);
					mathCopyVector(&fd1.scale,&sd);
				}


		#if defined( PS2) || defined( _GAME_PC_)
				mathVectorInterpAdd(&ts,&td,tval,(VECTOR*) &phr->nodes.boneptr[i].bind.m30,&ts);
		#else
				// Linear Blend The Translation
				mathVectorInterp(&ts,&td,tval,&ts);
				// Offset By The Bone Translation
				mathVectorAdd(&ts,(VECTOR*) &phr->nodes.boneptr[i].bind.m30,&ts);
		#endif

				// Apply Translation

				mathTranslate(&ts);

				// Scale By Parent Inverse Scale

				if(pac->source.anim->data->tracks[joints].flags&ANIMTRACK_HASSSC)
					mathScale(&inversescale);


				// Slerp Quaternion Rotation
				// And Convert To Matrix

				mathQuatSlerp(&qs,&qd,tval,&qs);
				mathQuatToMatrix(&qs,&m);

				if(papb->bonecontrol[i].flags&BONEFLAG_OVERRIDEANIM)
				{
					// Call User Function For Animation Channel Override
					// Function Returns Matrix On Stack

					if(papb->bonecontrol[i].ControlBone)
						papb->bonecontrol[i].ControlBone(pao,&papb->bonecontrol[i],&previousMatrix,&m,&qs);
					else
						mathMulMatrix(&m);
				}
				else
					mathMulMatrix(&m);

				// Blend Scale
				mathVectorInterp(&ss,&sd,tval,&ss);

				// Apply Scale

				mathScale(&ss);


				// Store The Inverse Scale For Children

				inversescale.x = 1.0f / ss.x;
				inversescale.y = 1.0f / ss.y;
				inversescale.z = 1.0f / ss.z;
			}
			else
			{
				// No Anim So Use Bind Translation
				mathMulMatrix(&phr->nodes.boneptr[i].bind);
			}

		}
		else
		{
			mathMulMatrix(&phr->nodes.boneptr[i].bind);
		}



		bonestack[bstackptr] = i;

		// Only On First Pass

		if(i==0)
		{
			mathStoreMatrix(&previousMatrix);
			pbone = bonestack[bstackptr];

			// Store Off The Hip Position
			mathPushMatrix();
#if !defined( PS2) && !defined( _GAME_PC_)
			mathMulMatrixPost(&gmathScreen2World);
#endif
			mathGetTranslate(&papb->hip);
			mathPopMatrix();

		}

		// Call User Function For Processing Bone

		mathStoreMatrix(&currentMatrix);

		papb->ProcessBone(pao,&phr->nodes.boneptr[i],&previousMatrix,&currentMatrix, i, pbone);

		// Store Current Matrix To Previous

		mathStoreMatrix(&previousMatrix);

		pbone = bonestack[bstackptr];

	}

	mathPopMatrix();
}

#else

//**********************************************************************************************************************
//**********************************************************************************************************************


#ifdef PS2
	#define SCRATCHPAD	0x70000000
	uint8* FALLBACK_SCRATCHPAD[32*1024];
#else
	uint8* FALLBACK_SCRATCHPAD[32*1024];
	uint8* SCRATCHPAD[16*1024];
#endif

VECTOR gInverseScale[128];

#if defined( _GAME_PC_)
void mathVectorOneOver( VECTOR* pscale, VECTOR* pinversescale)
{
	pinversescale->x = 0.0f != pscale->x ? 1.0f / pscale->x : 0;
	pinversescale->y = 0.0f != pscale->y ? 1.0f / pscale->y : 0;
	pinversescale->z = 0.0f != pscale->z ? 1.0f / pscale->z : 0;
	pinversescale->w = 0.0f;
}
#endif

/*
	Function	:	AO_AnimateHierachy
	Useage		:	Animate The Hierachy
*/

void AO_AnimateHierachy(ANIMOBJECT* pao)
{
	uint32 i;
	uint32 trackidx;
	HIERACHY* phr;
	ANIMATION_PLAYBACK* papb;
	VECTOR inversescale;
	MATRIX previousMatrix;
	MATRIX currentMatrix;
	MATRIX m;
	uint32 joints = 0;
	uint32 jointd = 0;
	MATRIX* matrices;
	float tval;
	ANIMATION_CONTROL* pac = NULL;
	FRAMEDATA* pfd0;
	FRAMEDATA* pfd1;
	FRAMEDATA* pfds;
	FRAMEDATA* pfdd;
	FRAMEDATA* pfdf;
	uint32 nOverlay;
	BOOL bLodAnimation;
	uint32 size;

	assert(pao!=NULL);

	papb = pao->apb;

	assert(papb != NULL);

	phr = pao->hierachy;

	assert(phr!=NULL);

	// Save Stack

	mathPushMatrix();

	// Intialise Inverse Scale

	inversescale.x = inversescale.y = inversescale.z = 1.0f;

	mathLoadVector(&inversescale);

	// Intialise Track Index

	trackidx = -1;
#if 0
	matrices = (MATRIX*) SCRATCHPAD;
	pfd0	 = (FRAMEDATA*)  ((SCRATCHPAD + ((phr->nNodes)*sizeof(MATRIX))));
	pfd1	 = (FRAMEDATA*)  ((SCRATCHPAD + ((phr->nNodes)*sizeof(MATRIX))) + (sizeof(FRAMEDATA)));
	pfds	 = (FRAMEDATA*)  ((SCRATCHPAD + ((phr->nNodes)*sizeof(MATRIX))) + (sizeof(FRAMEDATA) * 2));
	pfdd	 = (FRAMEDATA*)  ((SCRATCHPAD + ((phr->nNodes)*sizeof(MATRIX))) + (sizeof(FRAMEDATA) * 3) + (sizeof(FRAMEDATA) * phr->nNodes));
	pfdf	 = (FRAMEDATA*)  ((SCRATCHPAD + ((phr->nNodes)*sizeof(MATRIX))) + (sizeof(FRAMEDATA) * 4) + (sizeof(FRAMEDATA) * phr->nNodes * 2));


	size =  (((phr->nNodes)*sizeof(MATRIX)) + (sizeof(FRAMEDATA) * 4) + (sizeof(FRAMEDATA) * phr->nNodes * 2));
	size += sizeof(FRAMEDATA)*phr->nNodes;

	if(size > (16*1024))
#endif
	{
		matrices = (MATRIX*) FALLBACK_SCRATCHPAD;
		pfd0	 = (FRAMEDATA*)  ((FALLBACK_SCRATCHPAD + ((phr->nNodes)*sizeof(MATRIX))));
		pfd1	 = (FRAMEDATA*)  ((FALLBACK_SCRATCHPAD + ((phr->nNodes)*sizeof(MATRIX))) + (sizeof(FRAMEDATA)));
		pfds	 = (FRAMEDATA*)  ((FALLBACK_SCRATCHPAD + ((phr->nNodes)*sizeof(MATRIX))) + (sizeof(FRAMEDATA) * 2));
		pfdd	 = (FRAMEDATA*)  ((FALLBACK_SCRATCHPAD + ((phr->nNodes)*sizeof(MATRIX))) + (sizeof(FRAMEDATA) * 3) + (sizeof(FRAMEDATA) * phr->nNodes));
		pfdf	 = (FRAMEDATA*)  ((FALLBACK_SCRATCHPAD + ((phr->nNodes)*sizeof(MATRIX))) + (sizeof(FRAMEDATA) * 4) + (sizeof(FRAMEDATA) * phr->nNodes * 2));
	}


	BORDER(0x007f00);

	// Loop Through All The Bones

	pac = &papb->animation;


	// Initialise t

	if(papb->flags&APBFLAG_BLEND)
	{
		// Assign tval For Blend
		tval = pac->blend.msBlendTime/pac->blend.msBlend;
		tval = (1.0f - mathCos(M_PI*tval)) / 2.0f;
	}
	else
		tval = pac->source.tval;


	for(i=0;i<phr->nNodes;i++)
	{
		mathLoadUnitMatrix();

		// Check Flags For PushPop

		if(papb->bonecontrol[i].flags&BONEFLAG_MATRIXPOP)
		{
			mathStoreVector(&inversescale);
			mathPopVector();
		}

		if(papb->bonecontrol[i].flags&BONEFLAG_MATRIXPUSH)
		{
			mathPushVector();
			mathLoadVector(&inversescale);
		}

		// Increase Track Index If Bone Has Animation Track

		if(!(papb->bonecontrol[i].flags&BONEFLAG_NOTRACK)) trackidx++;


		bLodAnimation = FALSE;

		if(!(pao->flags&AO_CUTSCENE))
		{
			if(pao->flags&AO_LODANIMATION)
			{
				if(phr->nodes.boneptr[i].flags&BONEFLAG_REMOVABLE)
					bLodAnimation = TRUE;
			}
		}

		if(pac->source.anim && (!(papb->bonecontrol[i].flags&BONEFLAG_DYNAMIC)) && (!bLodAnimation))
		{
			// Check Mirror Flag And Setup Joint Indices

			if(pac->source.anim->flags&APBFLAG_MIRROR)
				joints = phr->bonemap[trackidx];
			else
				joints = trackidx;

			if(papb->flags&APBFLAG_BLEND)
			{
				if(pac->dest.anim->flags&APBFLAG_MIRROR)
					jointd = phr->bonemap[trackidx];
				else
					jointd = trackidx;

				// Blending Between Animations

				// Get Source Frame

				AT_GetFrameData(&pac->source,i,&pac->source.anim->data->tracks[joints],pac->source.frame0,pac->source.frame1,pfd0,pfd1);

				// Linear Blend Translate & Scale / Slerp Rotation

				mathVectorInterp(&pfd0->tran,&pfd1->tran,pac->source.tval,&pfds[i].tran);
				mathVectorInterp(&pfd0->scale,&pfd1->scale,pac->source.tval,&pfds[i].scale);
				mathQuatSlerp(&pfd0->rot,&pfd1->rot,pac->source.tval,&pfds[i].rot);

				if(pac->source.anim->flags&APBFLAG_MIRROR)
				{
					pfds[i].tran.x = -pfds[i].tran.x;
					pfds[i].rot.x  = -pfds[i].rot.x;
					pfds[i].rot.w  = -pfds[i].rot.w;
				}

				// Get Destination Frame

				AT_GetFrameData(&pac->dest,i,&pac->dest.anim->data->tracks[jointd],pac->dest.frame0,pac->dest.frame1,pfd0,pfd1);

				// Linear Blend Translate & Scale / Slerp Rotation

				mathVectorInterp(&pfd0->tran,&pfd1->tran,pac->dest.tval,&pfdd[i].tran);
				mathVectorInterp(&pfd0->scale,&pfd1->scale,pac->dest.tval,&pfdd[i].scale);
				mathQuatSlerp(&pfd0->rot,&pfd1->rot,pac->dest.tval,&pfdd[i].rot);

				if(pac->dest.anim->flags&APBFLAG_MIRROR)
				{
					pfdd[i].tran.x = -pfdd[i].tran.x;
					pfdd[i].rot.w  = -pfdd[i].rot.w;
					pfdd[i].rot.x  = -pfdd[i].rot.x;
				}

			}
			else
			{
				// Blending On The Same Animation

				AT_GetFrameData(&pac->source,i,&pac->source.anim->data->tracks[joints],pac->source.frame0,pac->source.frame1,&pfds[i],&pfdd[i]);

				// Mirror

				if(pac->source.anim->flags&APBFLAG_MIRROR)
				{
					pfds[i].tran.x = -pfds[i].tran.x;
					pfds[i].rot.x  = -pfds[i].rot.x;
					pfds[i].rot.w  = -pfds[i].rot.w;
					pfdd[i].tran.x = -pfdd[i].tran.x;
					pfdd[i].rot.w  = -pfdd[i].rot.w;
					pfdd[i].rot.x  = -pfdd[i].rot.x;
				}
			}

			// Linear Blend The Translation
			mathVectorInterpAdd(&pfds[i].tran,&pfdd[i].tran,tval,(VECTOR*) &phr->nodes.boneptr[i].bind.m30,&pfdf[i].tran);

			// Apply Translation

			mathTranslate(&pfdf[i].tran);

			// Scale By Parent Inverse Scale If Has Segment Scale Compensation

			if(pac->source.anim->data->tracks[joints].flags&ANIMTRACK_HASSSC)
				mathScale(&inversescale);

			mathCopyVector(&inversescale,&gInverseScale[i]);

			// Slerp Quaternion Rotation
			// And Convert To Matrix

			mathQuatSlerp(&pfds[i].rot,&pfdd[i].rot,tval,&pfdf[i].rot);
			mathQuatToMatrix(&pfdf[i].rot,&m);

			mathMulMatrix(&m);

			// Blend Scale

			mathVectorInterp(&pfds[i].scale,&pfdd[i].scale,tval,&pfdf[i].scale);

			// Apply Scale

			mathScale(&pfdf[i].scale);

			// Store The Inverse Scale For Children

			
			mathVectorOneOver(&pfdf[i].scale,&inversescale);

		}
		else
		{
			VECTOR v;

			// No Anim So Use Bind Translation

			mathMulMatrix(&phr->nodes.boneptr[i].bind);
		}

		mathStoreMatrix(&matrices[i]);
	}

	mathPopMatrix();



//BORDER(0x0000ff);

	// Process Overlay Channels

	for(nOverlay = 0; nOverlay < ANIMCTRL_MAXOVERLAY ; nOverlay++)
	{
		uint32 flags[ANIMCTRL_MAXOVERLAY] = {BONEFLAG_OVERLAY_CH0,BONEFLAG_OVERLAY_CH1};

		if((papb->overlay[nOverlay].flags&APBFLAG_HIDEOVERLAY) && (!(papb->overlay[nOverlay].flags&APBFLAG_OVERLAYBLEND)))
		#if defined( PS2)
			goto SkipOverlay;
		#else
			continue;
		#endif

		if(papb->overlay[nOverlay].flags&APBFLAG_OVERLAY)
		{
			FRAMEDATA* pSourceFD;
			FRAMEDATA* pDestFD;
			FRAMEDATA  fd0;
			FRAMEDATA  fd1;

			pSourceFD = pfds;
			pDestFD   = pfdd;

			mathPushMatrix();

			trackidx = -1;

			pac = &papb->overlay[nOverlay];

			if((pac->flags&APBFLAG_OVERLAYBLEND) || (pac->flags&APBFLAG_ENDOVERLAY))
			{
				// Assign tval For Blend
				tval = pac->blend.msBlendTime/pac->blend.msBlend;
				tval = (1.0f - mathCos(M_PI*tval)) / 2.0f;
			}
			else
				tval = pac->source.tval;

			for(i=0;i<phr->nNodes;i++)
			{
				pfds = &fd0;
				pfdd = &fd1;

				mathLoadUnitMatrix();

				// Check Flags For PushPop

				if(papb->bonecontrol[i].flags&BONEFLAG_MATRIXPOP)
				{
					mathStoreVector(&inversescale);
					mathPopVector();
				}

				if(papb->bonecontrol[i].flags&BONEFLAG_MATRIXPUSH)
				{
					mathPushVector();
					mathLoadVector(&inversescale);
				}

				// Increase Track Index If Bone Has Animation Track

				if(!(papb->bonecontrol[i].flags&BONEFLAG_NOTRACK)) trackidx++;

				if((papb->bonecontrol[i].flags&flags[nOverlay]) && (papb->bonecontrol[i].flags&BONEFLAG_OVERRIDEANIM))
				{
					// Check Mirror Flag And Setup Joint Indices

					if(pac->source.anim->flags&APBFLAG_MIRROR)
						joints = phr->bonemap[trackidx];
					else
						joints = trackidx;

					if((pac->flags&APBFLAG_OVERLAYBLEND) || (pac->flags&APBFLAG_ENDOVERLAY))
					{
						if(pac->dest.anim->flags&APBFLAG_MIRROR)
							jointd = phr->bonemap[trackidx];
						else
							jointd = trackidx;

						// Blending Between Animations

						// Get Source Frame

						if((pac->source.anim != papb->animation.source.anim))
						{
							AT_GetFrameData(&pac->source,i,&pac->source.anim->data->tracks[joints],pac->source.frame0,pac->source.frame1,pfd0,pfd1);

							// Linear Blend Translate & Scale / Slerp Rotation

							mathVectorInterp(&pfd0->tran,&pfd1->tran,pac->source.tval,&pfds->tran);
							mathVectorInterp(&pfd0->scale,&pfd1->scale,pac->source.tval,&pfds->scale);
							mathQuatSlerp(&pfd0->rot,&pfd1->rot,pac->source.tval,&pfds->rot);

							if(pac->source.anim->flags&APBFLAG_MIRROR)
							{
								pfds->tran.x = -pfds->tran.x;
								pfds->rot.x  = -pfds->rot.x;
								pfds->rot.w  = -pfds->rot.w;
							}
						}
						else
						{
							pfds = &pfdf[i];
							mathVectorSub((VECTOR*) &pfds->tran,(VECTOR*) &phr->nodes.boneptr[i].bind.m30,&pfds->tran);
						}

						if((pac->flags&APBFLAG_ENDOVERLAY || pac->flags&APBFLAG_HIDEOVERLAY) &&
							(pac->source.anim != papb->animation.source.anim))
						{
							pfdd = &pfdf[i];
							mathVectorSub(&pfdd->tran,(VECTOR*)&phr->nodes.boneptr[i].bind.m30,&pfdd->tran);
						}
						else
						{
							// Get Destination Frame

							AT_GetFrameData(&pac->dest,i,&pac->dest.anim->data->tracks[jointd],pac->dest.frame0,pac->dest.frame1,pfd0,pfd1);

							// Linear Blend Translate & Scale / Slerp Rotation

							mathVectorInterp(&pfd0->tran,&pfd1->tran,pac->dest.tval,&pfdd->tran);
							mathVectorInterp(&pfd0->scale,&pfd1->scale,pac->dest.tval,&pfdd->scale);
							mathQuatSlerp(&pfd0->rot,&pfd1->rot,pac->dest.tval,&pfdd->rot);

							if(pac->dest.anim->flags&APBFLAG_MIRROR)
							{
								pfdd->tran.x = -pfdd->tran.x;
								pfdd->rot.x  = -pfdd->rot.x;
								pfdd->rot.w  = -pfdd->rot.w;
							}

						}
					}
					else
					{
						// Blending On The Same Animation

						AT_GetFrameData(&pac->source,i,&pac->source.anim->data->tracks[joints],pac->source.frame0,pac->source.frame1,pfds,pfdd);

						// Mirror

						if(pac->source.anim->flags&APBFLAG_MIRROR)
						{
							pfds->tran.x = -pfds->tran.x;
							pfds->rot.x  = -pfds->rot.x;
							pfds->rot.w  = -pfds->rot.w;

							pfdd->tran.x = -pfdd->tran.x;
							pfdd->rot.x  = -pfdd->rot.x;
							pfdd->rot.w  = -pfdd->rot.w;
						}
					}

					// Linear Blend The Translation

					mathVectorInterpAdd(&pfds->tran,&pfdd->tran,tval,(VECTOR*) &phr->nodes.boneptr[i].bind.m30,&pfdf[i].tran);

					// Apply Translation

					mathTranslate(&pfdf[i].tran);

					// Scale By Parent Inverse Scale If Has Segment Scale Compensation

					if(pac->source.anim->data->tracks[joints].flags&ANIMTRACK_HASSSC)
						mathScale(&inversescale);

					mathCopyVector(&inversescale,&gInverseScale[i]);


					// Slerp Quaternion Rotation
					// And Convert To Matrix

					mathQuatSlerp(&pfds->rot,&pfdd->rot,tval,&pfdf[i].rot);
					mathQuatToMatrix(&pfdf[i].rot,&m);

					mathMulMatrix(&m);

					// Blend Scale

					mathVectorInterp(&pfds->scale,&pfdd->scale,tval,&pfdf[i].scale);

					// Apply Scale

					mathScale(&pfdf[i].scale);

					// Store The Inverse Scale For Children

					mathVectorOneOver(&pfdf[i].scale,&inversescale);

					mathStoreMatrix(&matrices[i]);
				}

			}

			mathPopMatrix();
		}


//	BORDER(0xff0000);


#if defined( PS2)
SkipOverlay:
#endif

	}

	// Process The Final Matrices

	mathCopyMatrix(&matrices[0],&previousMatrix);

	mathPushMatrix();
	mathMulMatrix(&matrices[0]);
	mathGetTranslate(&papb->hip);
	mathPopMatrix();

	for(i=0;i<phr->nNodes;i++)
	{
		if(papb->bonecontrol[i].flags&BONEFLAG_MATRIXPOP)
		{
			mathPopMatrix();
			mathStoreMatrix(&previousMatrix);
		}

		if(papb->bonecontrol[i].flags&BONEFLAG_MATRIXPUSH)
			mathPushMatrix();


		if((papb->overlay[0].flags&APBFLAG_HIDEOVERLAY) || (!pac->source.anim))
			mathMulMatrix(&matrices[i]);
		else if(papb->bonecontrol[i].flags&BONEFLAG_OVERRIDEANIM)
		{
			mathTranslate(&pfdf[i].tran);

			if(pac->source.anim->data->tracks[joints].flags&ANIMTRACK_HASSSC)
				mathScale(&gInverseScale[i]);

			mathQuatToMatrix(&pfdf[i].rot,&m);

			if(papb->bonecontrol[i].ControlBone)
				papb->bonecontrol[i].ControlBone(pao,&papb->bonecontrol[i],&previousMatrix,&m,&pfdf[i].rot);
			else
				mathMulMatrix(&m);

			mathScale(&pfdf[i].scale);
		}
		else
			mathMulMatrix(&matrices[i]);

		mathStoreMatrix(&currentMatrix);

		papb->ProcessBone(pao,&phr->nodes.boneptr[i],&previousMatrix,&currentMatrix, 0, 0);

		mathStoreMatrix(&previousMatrix);
	}



//	memset(SCRATCHPAD,0xff,16*1024);



BORDER(0);

}


#endif













































void AO_AnimateItems(ANIMOBJECT* pao,ANIMATION_CONTROL* pac)
{
	uint32 i,j;
	int strackidx;
	int dtrackidx;
	MATRIX previousMatrix;
	MATRIX currentMatrix;
	MATRIX m;
	ANIMATION_PLAYBACK* papb;

	if(pao->selectedskin)
	{
		papb = pao->apb;
	//	pac  = &papb->animation;

		for(i=0;i<pao->selectedskin->nHierachy;i++)
		{
			mathPushMatrix();

#if defined( WIN32) && !defined( _GAME_PC_)
			mathLoadUnitMatrix();
			mathUnitMatrix(&previousMatrix);
#else
			{
			uint32 k;
			DRAWMATRIX* dm;
			for(k=0;k<pao->hierachy->nNodes;k++)
			{
				if(pao->hierachy->nodes.boneptr[k].boneID == pao->selectedskin->Hierachy[i].flags)
					break;
			}

			dm = (DRAWMATRIX*) pao->matrices;
			mathLoadMatrix((MATRIX*)&dm[k]);
			mathStoreMatrix(&previousMatrix);
			}
#endif

			for(j=0;j<pao->selectedskin->Hierachy[i].nNodes;j++)
			{
				NODE* node;
				VECTOR t0,t1;
				VECTOR s0,s1;
				VECTOR q0,q1;
				uint32 k;

				node = &pao->selectedskin->Hierachy[i].nodes.nodeptr[j];

				strackidx = 0;
				dtrackidx = 0;

//				if(!(pao->flags&AO_LODANIMATION))
				{
					if(pac->source.anim)
					{
						for(k=0;k<pac->source.anim->nParentTracks;k++)
						{
							if(pac->source.anim->ParentTracks[k] == node->boneID)
								strackidx = pac->source.anim->ParentIdx[k];
						}
					}

					if(papb->flags&APBFLAG_BLEND)
					{
						if(pac->dest.anim)
						{
							for(k=0;k<pac->dest.anim->nParentTracks;k++)
							{
								if(pac->dest.anim->ParentTracks[k] == node->boneID)
									dtrackidx = pac->dest.anim->ParentIdx[k];
							}
						}

					}

				}

				if(node->flags&BONEFLAG_MATRIXPOP)
					mathPopMatrix();

				if(node->flags&BONEFLAG_MATRIXPUSH)
					mathPushMatrix();


				if(strackidx)
				{
					float tval;

					// Blend Animated Items.. Only Do These At 30fps...

					AT_GetSetFrameData(&pac->source.anim->data->tracks[strackidx],pac->source.frame0,&q0,&t0,&s0);

					if(papb->flags&APBFLAG_BLEND && dtrackidx)
					{
						tval = pac->blend.msBlendTime/pac->blend.msBlend;
						tval = (1.0f - mathCos(M_PI*tval)) / 2.0f;

//						if(dtrackidx)
//						{
							AT_GetSetFrameData(&pac->dest.anim->data->tracks[dtrackidx],pac->dest.frame0,&q1,&t1,&s1);
//						}
//						else
//						{
//							mathCopyVector(&node->rotate,&q0);
//							mathCopyVector(&node->scale,&s0);
//							mathCopyVector(&node->translate,&t0);
//						}

						mathVectorInterp(&t0,&t1,tval,&t0);
						mathVectorInterp(&s0,&s1,tval,&s0);
						mathQuatSlerp(&q0,&q1,tval,&q0);
					}
				}
				else
				{
					mathCopyVector(&node->rotate,&q0);
					mathCopyVector(&node->scale,&s0);
					mathCopyVector(&node->translate,&t0);
				}

				mathTranslate(&t0);

				mathMulMatrix(&node->pre);

				mathQuatToMatrix(&q0,&m);

				mathMulMatrix(&m);

				mathMulMatrix(&node->mid);
				mathScale(&s0);
				mathMulMatrix(&node->post);

				mathStoreMatrix(&currentMatrix);

				papb->ProcessBone(pao,(BONE*)&node,&previousMatrix,&currentMatrix,0,0);

				mathStoreMatrix(&previousMatrix);
			}

			mathPopMatrix();
		}

	}
}


void AO_InitMeshFlags(ANIMOBJECT* pao)
{
	uint32 i;

	if(pao->apb->flags&APBFLAG_ANIMATEDSET)
	{
		pao->meshflags = (uint32*) memalloc(sizeof(uint32)*pao->hierachy->nNodes);

		pao->nMatrices = pao->hierachy->nNodes;
		pao->matrices  = (MATRIX*) memalloc(sizeof(MATRIX)*pao->hierachy->nNodes);
		for(i=0;i<pao->hierachy->nNodes;i++)
			pao->meshflags[i] = MESHFLAG_VISIBLE;
	}
	else
	{
		pao->meshflags = (uint32*) memalloc(sizeof(uint32)*pao->selectedskin->nmesh);

		for(i=0;i<pao->selectedskin->nmesh;i++)
			pao->meshflags[i] = pao->selectedskin->mesh[i].flags;
	}
}




#if defined(PS2) || defined(_GAME_PC_)

enum
{
	BBXMIN = 0,
	BBXMAX,

	BBYMIN,
	BBYMAX,

	BBZMIN,
	BBZMAX
};


void AO_GetBoneSphere(ANIMOBJECT* pao,int idx,VECTOR* pos)
{
	MATRIX m;
	GetWorldBoneMatrix(pao,idx,&m);
	mathPushMatrix();
	mathLoadMatrix(&m);
	mathTranslate(&pao->hierachy->nodes.boneptr[idx].sphere.origin);
	mathUnitVector(pos);
	mathRotTrans(pos,pos);
	mathPopMatrix();
}


BOOL AO_BoundingBoxCollide(ANIMOBJECT *pa, ANIMOBJECT *pb)
{
	VECTOR box_a_min, box_a_max;
	VECTOR box_b_min, box_b_max;

	mathVectorAdd(&pa->apb->position, &pa->min, &box_a_min);
	mathVectorAdd(&pa->apb->position, &pa->max, &box_a_max);
	mathVectorAdd(&pb->apb->position, &pb->min, &box_b_min);
	mathVectorAdd(&pb->apb->position, &pb->max, &box_b_max);

	if (box_a_min.x > box_b_max.x || box_a_max.x < box_b_min.x ||
		box_a_min.y > box_b_max.y || box_a_max.y < box_b_min.y ||
		box_a_min.z > box_b_max.z || box_a_max.z < box_b_min.z)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


int AO_SphereCollide(ANIMOBJECT* pao,VECTOR* pos,float radius)
{
	HIERACHY* phr;
	uint32 i;

	phr = pao->hierachy;

//	sysDrawSphere(pos,radius,0xff00ffff);

	for(i=0;i<phr->nNodes;i++)
	{
		if(phr->nodes.boneptr[i].flags&BONEFLAG_BOUNDSPHERE)
		{
			VECTOR pos1;

			AO_GetBoneSphere(pao,i,&pos1);

//			sysDrawSphere(&pos1,phr->nodes.boneptr[i].sphere.radius,0xff00ff00);

			if(mapSphereSphereCollide(pos,radius, &pos1,phr->nodes.boneptr[i].sphere.radius))
				return phr->nodes.boneptr[i].boneID;
		}
	}

	return 0;
}





int32 AO_BoxSphereCollide(ANIMOBJECT* pao,uint32 hash,ANIMOBJECT* player)
{
	MESH* pMesh;
	uint32 i;
	int32 ret = -2;

	pMesh = AO_FindMesh(pao,hash);

	if(pMesh)
	{
		MATRIX dm;
		BONE* bone = NULL;
		DRAWMATRIX* pm;

		ret = -1;

		pm = pao->matrices;
		mathCopyMatrix(&pm[pMesh->bone].Trans1,&dm);

#ifdef PS2

//		mathPushMatrix();
//		mathLoadMatrix(&dm);
//		mathMulMatrixPost(&gmathCameraMatrix);
//		sysDrawBoxMinMax(&pMesh->min,&pMesh->max,0xffff0000);
//		mathPopMatrix();
#endif
		for(i=0;i<player->hierachy->nNodes;i++)
		{
			MATRIX dm0;
			VECTOR v;

			bone = &player->hierachy->nodes.boneptr[i];

			pm = player->matrices;

			mathUnitVector(&v);

			mathCopyMatrix(&pm[i].Trans1,&dm0);

			mathPushMatrix();
			mathLoadMatrix(&dm0);
			mathTranslate(&bone->sphere.origin);
			mathRotTrans(&v,&v);
			mathPopMatrix();

			if(mapBoxSphereCollide2(&pMesh->min,&dm,&v,bone->sphere.radius))
			{
#ifdef PS2
//				mathPushMatrix();
//				mathLoadMatrix(&gmathCameraMatrix);
//				sysDrawSphere(&v,bone->sphere.radius,0xffff0000);
//				mathPopMatrix();
#endif
				return i;
			}
		}
	}


	return ret;
}



#define HEAD 0x733c37


#define HEAD 0x733c37

// BEST BE CAREFUL HERE!

void AO_UpdateBounds(ANIMOBJECT* pao)
{
	uint32 i;
	HIERACHY* phr;
	BOOL bFirst = FALSE;
	BOOL bFoundTarget = FALSE;

	if(!pao->matrices) return;

	phr = pao->hierachy;



	for(i=0;i<phr->nNodes;i++)
	{
		// Now Update The Collision Bounding Box & Sphere World Position

		if(phr->nodes.boneptr[i].boneID == HEAD)
		{
			VECTOR pos;
			DRAWMATRIX* dm = (DRAWMATRIX*) pao->matrices;
			mathUnitVector(&pos);
			mathPushMatrix();
			mathLoadMatrix((MATRIX*)&dm[i]);
			mathRotTrans(&pos,&pao->target);
			mathPopMatrix();
			bFoundTarget = TRUE;
		}


		if(phr->nodes.boneptr[i].flags&BONEFLAG_BOUNDSPHERE || (phr->nodes.boneptr[i].flags&BONEFLAG_DYNAMIC && pao->dynamic_char))
		{
			VECTOR pos;
			float radius;
			DRAWMATRIX* dm = (DRAWMATRIX*) pao->matrices;

			if(phr->nodes.boneptr[i].flags&BONEFLAG_DYNAMIC)
			{
				phr->nodes.boneptr[i].sphere.radius = 32.0f;
				mathUnitVector(&phr->nodes.boneptr[i].sphere.origin);
			}

			radius = phr->nodes.boneptr[i].sphere.radius;

			mathUnitVector(&pos);

			mathPushMatrix();
			mathLoadMatrix((MATRIX*)&dm[i]);

//			mathPushMatrix();
//			mathTranslatef(phr->nodes.boneptr[i].sphere.origin.x,phr->nodes.boneptr[i].sphere.origin.y,phr->nodes.boneptr[i].sphere.origin.z);
//			sysDrawSphere(&pos,radius,0xff00ff00);
//			mathPopMatrix();

//			mathMulMatrixPost(&gmathInverseCameraMatrix);
			mathTranslate(&phr->nodes.boneptr[i].sphere.origin);
			mathRotTrans(&pos,&pos);

//			mathCopyVector(&pos,&phr->nodes.boneptr[i].sphere.world);

			if(!bFirst)
			{
				pao->min.x = pos.x - radius;
				pao->min.y = pos.y - radius;
				pao->min.z = pos.z - radius;

				pao->max.x = pos.x + radius;
				pao->max.y = pos.y + radius;
				pao->max.z = pos.z + radius;
			}
			else
			{
				if(pos.x + radius > pao->max.x) pao->max.x = pos.x + radius;
				if(pos.y + radius > pao->max.y) pao->max.y = pos.y + radius;
				if(pos.z + radius > pao->max.z) pao->max.z = pos.z + radius;

				if(pos.x - radius < pao->min.x) pao->min.x = pos.x - radius;
				if(pos.y - radius < pao->min.y) pao->min.y = pos.y - radius;
				if(pos.z - radius < pao->min.z) pao->min.z = pos.z - radius;
			}

			bFirst = TRUE;

			mathPopMatrix();

		}
/*		else
		{
			if(!i)
			{
				VECTOR pos;
				DRAWMATRIX* dm = (DRAWMATRIX*) pao->matrices;

				mathUnitVector(&pos);
				mathPushMatrix();
				mathLoadMatrix((MATRIX*)&dm[i]);
				mathRotTrans(&pos,&pos);
				mathPopMatrix();

				pao->min.x = pos.x - 256.0f;
				pao->min.y = pos.y - 256.0f;
				pao->min.z = pos.z - 512.0f;

				pao->max.x = pos.x + 256.0f;
				pao->max.y = pos.y + 256.0f;
				pao->max.z = pos.z + 512.0f;
			}
		}*/

	}

#if 1
	if(pao->selectedskin)
	{

		for(i=0;i<pao->selectedskin->nmesh;i++)
		{
			VECTOR v0,v1;
			int j;
			DRAWMATRIX* dm = (DRAWMATRIX*) pao->matrices;
			float pBox[6];


			if(!(pao->meshflags[i]&MESHFLAG_VISIBLE))
				continue;


			if(pao->selectedskin->mesh[i].flags&MESHFLAG_ANIMATED)
				mathLoadMatrix((MATRIX*)&dm[pao->hierachy->nNodes + pao->selectedskin->mesh[i].animidx].Trans1);
			else
				mathLoadMatrix((MATRIX*)&dm[pao->selectedskin->mesh[i].bone].Trans1);


			pBox[BBXMIN] = pao->selectedskin->mesh[i].min.x;
			pBox[BBYMIN] = pao->selectedskin->mesh[i].min.y;
			pBox[BBZMIN] = pao->selectedskin->mesh[i].min.z;

			pBox[BBXMAX] = pao->selectedskin->mesh[i].max.x;
			pBox[BBYMAX] = pao->selectedskin->mesh[i].max.y;
			pBox[BBZMAX] = pao->selectedskin->mesh[i].max.z;

			for	( j=0;  j<8 ; j++ )
			{
				VECTOR v;

				v.x = pBox[BBXMIN+ (j& 1)   ];
				v.y = pBox[BBYMIN+((j>>1)&1)];
				v.z = pBox[BBZMIN+((j>>2)&1)];

				mathRotTrans(&v,&v);

				if(v.x > pao->max.x) pao->max.x = v.x;
				if(v.y > pao->max.y) pao->max.y = v.y;
				if(v.z > pao->max.z) pao->max.z = v.z;

				if(v.x < pao->min.x) pao->min.x = v.x;
				if(v.y < pao->min.y) pao->min.y = v.y;
				if(v.z < pao->min.z) pao->min.z = v.z;
			}
#if 0
			mathPushMatrix();
			mathMulMatrixPost(&gmathCameraMatrix);
			sysDrawBoxMinMax(&pao->selectedskin->mesh[i].min,&pao->selectedskin->mesh[i].max,0xffff00ff);
			mathPopMatrix();
#endif

		}
	}
#endif

	if(!bFoundTarget)
	{
		mathVectorAdd(&pao->max, &pao->min, &pao->target);
		mathVectorScale(&pao->target, 0.5f, &pao->target);
		//mathVectorAdd(&pao->apb->position, &pao->target, &pao->target);
	}

	mathVectorSub(&pao->min, &pao->apb->position, &pao->min);
	mathVectorSub(&pao->max, &pao->apb->position, &pao->max);
}

#endif








/*
	Function	:	AO_Animate
	Useage		:	Animate The Object
*/

void AO_Animate(ANIMOBJECT* pao)
{
	ANIMATION_PLAYBACK* papb;

	assert(pao!=NULL);
	assert(pao->apb!=NULL);

	papb = pao->apb;

	pao->flags &= ~AO_SHOOTLEFT;
	pao->flags &= ~AO_SHOOTBOTH;
	pao->flags &= ~AO_SHOOTRIGHT;

//	if(papb->flags&APBFLAG_ANIMATEDSET)
//		return;


	mathPushMatrix();

	#if defined( PS2) || defined( _GAME_PC_)
	if	(!(pao->flags & AO_USECURRENTMATRIX))
	{
		mathLoadUnitMatrix();
	}
	#endif

	mathTranslate(&papb->position);
	mathRotZ(DTOR(papb->rotation.z));
	mathRotY(DTOR(papb->rotation.y));
	mathRotX(DTOR(papb->rotation.x));

	mathScale(&papb->scale);

	// Call Begin Frame

	if(papb->BeginFrameTick)
		papb->BeginFrameTick(pao);


	// Do Hierachy Animation

	if(papb->flags&APBFLAG_ANIMATEDSET)
	{

#if defined( PS2)
		BORDER(0x003f00);
#endif
		AO_AnimateSet(pao);

#if defined(PS2)
		BORDER (0xff0000ff);
#endif

	}
	else
	{
//		#if defined( PS2) || defined( _GAME_PC_)
//			if(papb->flags&APBFLAG_CUTSCENE)
//				AO_AnimateCutscene();
//			else
#ifdef PHYSICS
//			if ( (pao==gmapPlayerAO) || (!(pao->flags & (AO_RAGDOLL|AO_RAGDOLL_STATIC))) )
			if ( (!(pao->flags & (AO_RAGDOLL|AO_RAGDOLL_STATIC))) )
#endif
			{

				#if defined( PS2)
				BORDER(0x00ff00);
				#endif

				AO_AnimateHierachy(pao);

				#if defined(PS2)
				BORDER(0x007f00);
				#endif

				if (!(pao->flags & (AO_RAGDOLL_TRACK|AO_RAGDOLL_BLEND|AO_RAGDOLL_BLEND_FINISH)))
				{
					AO_AnimateItems(pao,&pao->apb->animation);
				}
			}
	}

#if defined( PS2)
	BORDER(0x00ffff);
#endif

//#ifndef _WIN32
#if !defined(_WIN32) || defined(_GAME_PC_)

#if 1 && ISME(RT) //bit of a hack to test anims without scripted movement
	if (pao->flags&AO_ANIM_ADDVEL)
	{
		pao->apb->position.x+=pao->apb->velocity.x;
		pao->apb->position.y+=pao->apb->velocity.y;
		pao->apb->position.z+=pao->apb->velocity.z;
	}
#endif

#ifdef PHYSICS
	if (pao->dynamic_char!=NULL)
	{
		extern void *update_char_dynamics(void*);
		update_char_dynamics(pao->dynamic_char);
	}

	#if defined( PS2)
	BORDER(0xffffff);
	#endif

	if ((pao->flags & (AO_RAGDOLL_TRACK|AO_RAGDOLL|AO_RAGDOLL_STATIC|AO_RAGDOLL_BLEND|AO_RAGDOLL_BLEND_FINISH)))
	{
	#if defined( PS2)
		extern void calcCharLights(ANIMOBJECT* pao);

//		if (!(pao->flags & (AO_RAGDOLL_TRACK)))
//		if (!(pao->flags & (AO_RAGDOLL_STATIC|AO_RAGDOLL_TRACK|AO_RAGDOLL_BLEND|AO_RAGDOLL_BLEND_FINISH)))

		if (!(pao->flags & (AO_RAGDOLL_TRACK|AO_RAGDOLL_BLEND|AO_RAGDOLL_BLEND_FINISH)))
			calcCharLights(pao);
	#endif

		if (!(pao->flags & (AO_RAGDOLL_TRACK|AO_RAGDOLL_BLEND|AO_RAGDOLL_BLEND_FINISH)))
		{
			pMatrices+=pao->hierachy->nNodes;
		}

		AO_AnimateItems(pao,&pao->apb->animation);
	}
#endif //physics

#endif

#if defined( PS2)
	BORDER(0);
#endif

#if defined( PS2) || defined( _GAME_PC_)

	if(!(papb->flags&APBFLAG_ANIMATEDSET))
		AO_UpdateBounds(pao);
#endif

//	BORDER(0);

	// Call User End Frame

	if(papb->EndFrameTick)
		papb->EndFrameTick(pao);

	mathPopMatrix();


	pao->flags |= AO_VALIDMATRICES;
}




void APB_GetDeltaVelocity(ANIMATION* anim,int frame,float ft,int pframe,float pft,VECTOR* vd,VECTOR* rvd)
{
	VECTOR ts;
	VECTOR td;
	VECTOR rs;
	VECTOR rd;

	float dt;
	int i;


	mathUnitVector(&ts);
	mathUnitVector(&td);
	mathUnitVector(&rs);
	mathUnitVector(&rd);


	if(pframe == frame)
	{
		dt = ft - pft;

		AT_GetFrameDataVelocity(&anim->data->vtrack,frame,&ts,&rs);

		ts.x = ts.x * dt;
		ts.y = ts.y * dt;
		ts.z = ts.z * dt;

		rs.x = rs.x * dt;
		rs.y = rs.y * dt;
		rs.z = rs.z * dt;
	}
	else
	{
		AT_GetFrameDataVelocity(&anim->data->vtrack,pframe,&ts,&rs);
		AT_GetFrameDataVelocity(&anim->data->vtrack,frame,&td,&rd);

		dt = 1.0f - pft;

		ts.x = ts.x * dt;
		ts.y = ts.y * dt;
		ts.z = ts.z * dt;

		rs.x = rs.x * dt;
		rs.y = rs.y * dt;
		rs.z = rs.z * dt;

		dt = ft;

		ts.x += td.x * dt;
		ts.y += td.y * dt;
		ts.z += td.z * dt;

		rs.x += rd.x * dt;
		rs.y += rd.y * dt;
		rs.z += rd.z * dt;

		for(i=pframe+1;i<frame;i++)
		{
			AT_GetFrameDataVelocity(&anim->data->vtrack,i,&td,&rd);

			ts.x += td.x;
			ts.y += td.y;
			ts.z += td.z;

			rs.x += rd.x;
			rs.y += rd.y;
			rs.z += rd.z;
		}
	}

	vd->x = ts.x;
	vd->y = ts.y;
	vd->z = ts.z;
	vd->w = ts.w;

	rvd->x = rs.x;
	rvd->y = rs.y;
	rvd->z = rs.z;
}




void APB_GetVelocity(ANIMATION_PLAYBACK* papb,ANIMATION_POSITIONINFO* pos,VECTOR* velocity,VECTOR* rvelocity)
{
	int frame;
	int pframe;
	float vmul;
	float ft;
	float pft;

	mathUnitVector(velocity);
	mathUnitVector(rvelocity);

//	if(pos->vframe0 == (pos->anim->nFrames-1) && !pos->loopcnt)
//		return;

	if(/*papb->msPerFrame > 0*/ (pos->frame0 >= pos->prevframe0) || ((pos->frame0 < pos->prevframe0) && pos->flags&APBFLAG_LOOPED))
	{
		frame  = pos->frame0;
		pframe = pos->prevframe0;
		ft     = pos->tval;
		pft    = pos->prevtval;
		vmul = 1.0f;

	}
	else
	{
		frame  = pos->prevframe0;
		pframe = pos->frame0;
		ft     = pos->prevtval;
		pft    = pos->tval;
		vmul = -1.0f;
	}

	APB_GetDeltaVelocity(pos->anim,frame,ft,pframe,pft,velocity,rvelocity);

	velocity->x *= vmul;
	velocity->y *= vmul;
	velocity->z *= vmul;

	rvelocity->x *= vmul;
	rvelocity->y *= vmul;
	rvelocity->z *= vmul;

	if(pos->anim->flags&APBFLAG_MIRROR)
	{
		velocity->x  = -velocity->x;
		rvelocity->z = -rvelocity->z;
	}
}


void APB_ApplyVelocity(ANIMATION_PLAYBACK* papb,VECTOR* v,VECTOR* r)
{
	if (r->x || r->y || r->z)
	{
		if (papb->rotation.x == 0.0f && papb->rotation.y == 0.0f)
		{
			papb->rotation.x += r->x;
			papb->rotation.y += r->y;
			papb->rotation.z += r->z;
		}
		else
		{
			// correction for applying rotation when player has x or y base rotation already
			// i.e. in swimming or crawling PDC 3/6/2003
			MATRIX m;
			VECTOR rot;

			mathPushMatrix();
			mathLoadUnitMatrix();
			mathRotZ(DTOR(papb->rotation.z));
			mathRotY(DTOR(papb->rotation.y));
			mathRotX(DTOR(papb->rotation.x));
			mathRotZ(DTOR(r->z));
			mathRotY(DTOR(r->y));
			mathRotX(DTOR(r->x));
			mathStoreMatrix(&m);
			mathPopMatrix();

			mathMatrixGetAngles(&m, &rot);
			papb->rotation.x = RTOD(rot.x);
			papb->rotation.y = RTOD(rot.y);
			papb->rotation.z = RTOD(rot.z);
		}
	}

	v->x *= papb->vscaler.x;
	v->y *= papb->vscaler.y;
	v->z *= papb->vscaler.z;

	mathPushMatrix();
	mathLoadUnitMatrix();
	mathRotZ(DTOR(papb->rotation.z));
	mathRotY(DTOR(papb->rotation.y));
	mathRotX(DTOR(papb->rotation.x));
	mathScale(&papb->scale);
	mathRot(v,&papb->velocity);
	mathPopMatrix();

	mathCopyVector(r,&papb->rotvelocity);
}


void APB_ApplyVelocity_General(ANIMATION_PLAYBACK *papb, VECTOR *rotation, VECTOR *v, VECTOR *r, VECTOR *result_velocity)
{
	mathVectorAdd(rotation, r, rotation);

	mathPushMatrix();
	mathLoadUnitMatrix();
	mathRotZ(DTOR(rotation->z));
	mathRotY(DTOR(rotation->y));
	mathRotX(DTOR(rotation->x));
	mathScale(&papb->scale);
	mathRot(v,result_velocity);
	mathPopMatrix();

	mathCopyVector(r,&papb->rotvelocity);
}


float APB_GetSourceMSTime(ANIMATION_PLAYBACK* papb)
{
	return papb->animation.source.time;
}


float APB_GetSourceFrameTime(ANIMATION_PLAYBACK* papb)
{
	return APB_MSToFrame(papb->animation.source.time);
}



void APB_GetVelocityOffset(ANIMATION* anim,int frame,VECTOR* t,VECTOR* r)
{
	VECTOR td,rd;
	int i;

	mathUnitVector(t);
	mathUnitVector(r);

//	if(!(anim->flags&APBFLAG_LOOP))
//		frame++;

	for(i=0;i<frame;i++)
	{
		AT_GetFrameDataVelocity(&anim->data->vtrack,i,&td,&rd);

		if(anim->flags&APBFLAG_MIRROR)
		{
			td.x = -td.x;
			rd.z = -rd.z;
		}

		r->x += rd.x;
		r->y += rd.y;
		r->z += rd.z;

		mathPushMatrix();
		mathLoadUnitMatrix();
		mathRotZ(DTOR(r->z));
		mathRotY(DTOR(r->y));
		mathRotX(DTOR(r->x));
		mathRot(&td,&td);
		mathPopMatrix();

		t->x += td.x;
		t->y += td.y;
		t->z += td.z;
	}

}



void APB_SetAnimationFrame(ANIMOBJECT* pao,int frame)
{
	if(pao)
	{
		if(pao->apb->animation.source.anim)
		{
			pao->apb->animation.source.time = APB_FrameToMS((float) frame);
	//		pao->apb->animation.source.tval = 0.5f;
			pao->apb->flags |= APBFLAG_PLAY;
			APB_InitFrames(pao->apb,&pao->apb->animation.source,pao->apb->animation.source.time);
	//		AO_FrameTick(pao);
			pao->apb->flags &= ~APBFLAG_PLAY;
		}
	}
}




void APB_CheckBlendWait(ANIMOBJECT* pao)
{
	ANIMATION_PLAYBACK* papb;
	papb = pao->apb;


	if(papb->flags&APBFLAG_BLENDWAIT)
	{
		int ridx;

		ridx = APB_InBlendRange(&pao->apb->animation,pao->apb->blendrange);

		if(ridx != -1)
		{
			APB_SetupRangeBlend(pao->apb,pao->apb->blendrange,ridx,papb->animation.source.flags);
		}
	}
}



void APB_CheckLinkWait(ANIMOBJECT* pao)
{
	ANIMATION_PLAYBACK* papb;
	papb = pao->apb;


	if(papb->flags&APBFLAG_LINKWAIT)
	{
		int ridx = 0;
		ANIMATION_LINK* pabl;
		ANIMATION_BLENDRANGE* pabr = NULL;
		pabl = papb->linkrange;

		// Check Blend Ranges For Left & Right

		if(pabl->LLinkID)
		{
			pabr = AL_FindBlendRange(papb->animation.source.anim,pabl->LLinkID);
			ridx = APB_InBlendRange(&papb->animation,pabr);
		}

		if(pabl->RLinkID && ridx == -1)
		{
			pabr = AL_FindBlendRange(papb->animation.source.anim,pabl->RLinkID);
			ridx = APB_InBlendRange(&papb->animation,pabr);
		}

		if(ridx != -1)
		{
			APB_SetupRangeBlend(papb,pabr,ridx,papb->animation.source.flags);
			papb->flags &= ~(APBFLAG_LINKWAIT);
			papb->flags |= APBFLAG_DELAYBLEND;
		}
	}
}


void APB_CheckDelayBlend(ANIMOBJECT* pao)
{
	ANIMATION_PLAYBACK* papb;
	papb = pao->apb;


	if(papb->flags&APBFLAG_DELAYBLEND)
	{
		if(papb->flags&APBFLAG_BLENDCOMPLETE)
		{
			papb->flags &= ~(APBFLAG_DELAYBLEND);

//			dbgLog(1,1,"Blend To Delayed Blend");

			APB_BlendToAnimation(papb,papb->linkrange->destID,0,0,0);
		}
	}
}


void APB_GetCurrentVelocity(ANIMOBJECT* pao,VECTOR* rv,VECTOR* rr)
{
	float l0,l1;


	ANIMATION_PLAYBACK* papb;
	papb = pao->apb;



	// Apply Animation Velocity

	if(papb->flags&APBFLAG_PLAY)
	{
		VECTOR v0,v1,r0,r1;

		if(papb->flags&APBFLAG_BLEND)
		{
			float tval = 0.0f;
			APB_GetVelocity(papb,&papb->animation.source,&v0,&r0);
			APB_GetVelocity(papb,&papb->animation.dest,&v1,&r1);

			if(papb->animation.blend.msBlend)
				tval = papb->animation.blend.msBlendTime/papb->animation.blend.msBlend;
/*
			{

				l0 = mathVectorLength(&v0);
				l1 = mathVectorLength(&v1);


				if(l1 > 0.01f)
				{
					dbgLog(1,0,"v0 %.2f v1 %.2f",l0,l1);
					APB_GetVelocity(papb,&papb->animation.dest,&v1,&r1);
				}
			}
*/

			// Automate Velocity Blending
			// Source & Dest Have Velocity Then X Fade
			// Source Only Use Source
			// Dest Only Use Dest

			if(papb->flags&APBFLAG_XFADEVELOCITY)
			{

				if(papb->animation.source.anim->data->vtrack.flags &&
					papb->animation.dest.anim->data->vtrack.flags)
				{

//					tval = (1.0f - mathCos(M_PI*tval)) / 2.0f;

					mathVectorInterp(&v0,&v1,tval,&v0);
					mathVectorInterp(&r0,&r1,tval,&r0);
				}
				else if((!papb->animation.source.anim->data->vtrack.flags) &&
				   papb->animation.dest.anim->data->vtrack.flags)
				{
					mathCopyVector(&v1,&v0);
					mathCopyVector(&r1,&r0);
				}

			}
			else
			{
				mathCopyVector(&v1,&v0);
				mathCopyVector(&r1,&r0);
			}

		}
		else
		{
			APB_GetVelocity(papb,&papb->animation.source,&v0,&r0);
		}

		mathCopyVector(&v0,rv);
		mathCopyVector(&r0,rr);


	//	l0 = mathVectorLength(&v0);

	//	if(l0 > 0.01f)
	//		dbgLog(1,0,"%.2f ",l0);


	}
	else
	{
		mathUnitVector(rv);
		mathUnitVector(rr);
	}


}


void APB_GetAnimationVelocity(ANIMOBJECT* pao)
{
	ANIMATION_PLAYBACK* papb;
	VECTOR v0,r0;
	papb = pao->apb;

//		totalz += v0.z;
//		Log("Frame 0 %d Time %.2f TVal %.2f Z %.2f ",papb->animation.source.frame0,papb->animation.source.time,papb->animation.source.tval,v0.z);

	APB_GetCurrentVelocity(pao,&v0,&r0);
	APB_ApplyVelocity(papb,&v0,&r0);

/*
		{
		VECTOR h,t,r;
		mathCopyVector(&papb->position,&t);
		mathCopyVector(&papb->rotation,&r);
		mathCopyVector(&papb->hip,&h);
		Log("%d) Pos %f %f %f  Rot %f %f %f Hip %f %f %f",papb->animation.source.frame0,t.x,t.y,t.z,r.x,r.y,r.z,h.x,h.y,h.z);
		}
*/

}


void APB_ClearOverlayCH0(ANIMOBJECT* pao)
{
	uint32 i;

	pao->apb->overlay[0].flags&=~(APBFLAG_OVERLAY|APBFLAG_ENDOVERLAY|APBFLAG_OVERLAYBLEND|APBFLAG_HIDEOVERLAY);

	for(i=0;i<pao->hierachy->nNodes;i++)
		pao->apb->bonecontrol[i].flags &= ~(/*BONEFLAG_OVERRIDEANIM|*/BONEFLAG_OVERLAY_CH0);

	memclear(pao->apb->overlay[0].source.pFrameInfo,pao->hierachy->nNodes * sizeof(FRAMEINFO) * AT_TOTAL);
	memclear(pao->apb->overlay[0].dest.pFrameInfo,pao->hierachy->nNodes* sizeof(FRAMEINFO) * AT_TOTAL);

	APB_ClearPositionInfo(&pao->apb->overlay[0].source);
	APB_ClearPositionInfo(&pao->apb->overlay[0].dest);
}


void APB_ClearOverlayCH1(ANIMOBJECT* pao)
{
	uint32 i;

	pao->apb->overlay[1].flags&=~(APBFLAG_OVERLAY|APBFLAG_ENDOVERLAY|APBFLAG_OVERLAYBLEND|APBFLAG_HIDEOVERLAY);

	for(i=0;i<pao->hierachy->nNodes;i++)
		pao->apb->bonecontrol[i].flags &= ~(/*BONEFLAG_OVERRIDEANIM|*/BONEFLAG_OVERLAY_CH1);

	memclear(pao->apb->overlay[1].source.pFrameInfo,pao->hierachy->nNodes * sizeof(FRAMEINFO) * AT_TOTAL);
	memclear(pao->apb->overlay[1].dest.pFrameInfo,pao->hierachy->nNodes* sizeof(FRAMEINFO) * AT_TOTAL);

	APB_ClearPositionInfo(&pao->apb->overlay[1].source);
	APB_ClearPositionInfo(&pao->apb->overlay[1].dest);
}


void APB_ClearAllOverlays(ANIMOBJECT* pao)
{
	APB_ClearOverlayCH0(pao);
	APB_ClearOverlayCH1(pao);
}



void APB_CheckBlendStatus(ANIMOBJECT* pao)
{
	ANIMATION_PLAYBACK* papb;
	papb = pao->apb;

	// Animation Tick

	if(papb->flags&APBFLAG_BLEND || papb->overlay[0].flags&APBFLAG_OVERLAYBLEND || papb->overlay[1].flags&APBFLAG_OVERLAYBLEND)
	{
		// Check If Blend Achieved

		if(papb->flags&APBFLAG_BLEND)
		{
			// Increase BlendTime

			papb->animation.blend.pmsBlendTime = papb->animation.blend.msBlendTime;

			if(!(papb->flags&APBFLAG_CONTROLBLEND))
				papb->animation.blend.msBlendTime += mathFabs(gAnimTargetMSSquared / papb->msPerFrame);


			if(papb->animation.blend.msBlendTime > papb->animation.blend.msBlend)
			{
				// Now Make Blend Animation The Source Animation

//				dbgLog(1,1,"Blend Complete");

				papb->flags |= APBFLAG_BLENDCOMPLETE;

				APB_DestToSource(papb);
			}
			else if(papb->animation.blend.msBlendTime <= FLT_EPSILON)
			{
//				papb->flags |= APBFLAG_BLENDCOMPLETE;

//				memclear(&papb->animation.dest,sizeof(ANIMATION_POSITIONINFO));
				APB_ClearPositionInfo(&papb->animation.dest);

				papb->flags &= ~APBFLAG_BLEND;
				papb->flags &= ~APBFLAG_CONTROLBLEND;
			}

		}

		if(papb->overlay[0].flags&APBFLAG_OVERLAYBLEND)
		{
			float add;

			add = mathFabs(gAnimTargetMSSquared / papb->msPerFrame);

			// Increase BlendTime

			papb->overlay[0].blend.pmsBlendTime = papb->overlay[0].blend.msBlendTime;
			papb->overlay[0].blend.msBlendTime += mathFabs(gAnimTargetMSSquared / papb->msPerFrame);

			if(papb->overlay[0].blend.msBlendTime >= papb->overlay[0].blend.msBlend)
			{
				papb->overlay[0].flags |= APBFLAG_OVERLAYBLENDCOMPLETE;

//				printf("** Blend Complete \n");

				if(papb->overlay[0].flags&APBFLAG_ENDOVERLAY)
				{
//##				papb->overlay[0].source.flags |= APBFLAG_FINISHED;

/*					papb->overlay[0].flags&=~(APBFLAG_OVERLAY|APBFLAG_ENDOVERLAY|APBFLAG_OVERLAYBLEND);

					// Clear The Bones
					{
						uint32 i;
						for(i=0;i<papb->pao->hierachy->nNodes;i++)
							papb->bonecontrol[i].flags &= ~(BONEFLAG_OVERRIDEANIM|BONEFLAG_OVERLAY_CH0);
					}

					memclear(papb->animation.source.pFrameInfo,papb->pao->hierachy->nNodes * sizeof(FRAMEINFO) * AT_TOTAL);
					memclear(papb->animation.dest.pFrameInfo,papb->pao->hierachy->nNodes* sizeof(FRAMEINFO) * AT_TOTAL);
*/
					APB_ClearOverlayCH0(papb->pao);

					APB_ClearOverlayTriggers(papb);

//					printf("** Overlay Cleared\n");

				}
				else
				{
					if(!(papb->overlay[0].flags&APBFLAG_HIDEOVERLAY))
					{
//						printf("** Play Overlay \n");
						APB_PlayOverlay(papb,0,papb->overlay[0].dest.anim->animID,papb->overlay[0].dest.time,papb->overlay[0].dest.flags);
					}
					else
						papb->overlay[0].flags &= ~APBFLAG_OVERLAYBLEND;
				}

			}

		}

		if(papb->overlay[1].flags&APBFLAG_OVERLAYBLEND)
		{
			// Increase BlendTime

			papb->overlay[1].blend.pmsBlendTime = papb->overlay[1].blend.msBlendTime;
			papb->overlay[1].blend.msBlendTime += mathFabs(gAnimTargetMSSquared / papb->msPerFrame);

			if(papb->overlay[1].blend.msBlendTime >= papb->overlay[1].blend.msBlend)
			{
				papb->overlay[1].flags |= APBFLAG_OVERLAYBLENDCOMPLETE;

				if(papb->overlay[1].flags&APBFLAG_ENDOVERLAY)
				{
/*
					papb->overlay[1].flags&=~(APBFLAG_OVERLAY|APBFLAG_ENDOVERLAY|APBFLAG_OVERLAYBLEND);
//					papb->overlay[1].source.flags |= APBFLAG_FINISHED;

					// Clear The Bones
					{
						uint32 i;
						for(i=0;i<papb->pao->hierachy->nNodes;i++)
							papb->bonecontrol[i].flags &= ~(BONEFLAG_OVERRIDEANIM|BONEFLAG_OVERLAY_CH1);
					}

					memclear(papb->animation.source.pFrameInfo,papb->pao->hierachy->nNodes * sizeof(FRAMEINFO) * AT_TOTAL);
					memclear(papb->animation.dest.pFrameInfo,papb->pao->hierachy->nNodes* sizeof(FRAMEINFO) * AT_TOTAL);
*/
					APB_ClearOverlayCH1(papb->pao);

				}
				else
				{
					if(!(papb->overlay[1].flags&APBFLAG_HIDEOVERLAY))
					{
						APB_PlayOverlay(papb,1,papb->overlay[1].dest.anim->animID,papb->overlay[1].dest.time,papb->overlay[1].dest.flags);
					}
					else
						papb->overlay[1].flags &= ~APBFLAG_OVERLAYBLEND;

				}

			}

		}

	}

}




void APB_GetAnimFrames(ANIMOBJECT* pao)
{
	ANIMATION_PLAYBACK* papb;
	papb = pao->apb;

	// Set Up The Frames To Process

	if(pao->apb->animation.source.anim)
		APB_GetFrames(pao->apb,&pao->apb->animation.source);

	if(papb->flags&APBFLAG_BLEND)
		APB_GetFrames(pao->apb,&pao->apb->animation.dest);


	if(papb->overlay[0].flags&APBFLAG_OVERLAY)
	{
		APB_GetFrames(pao->apb,&pao->apb->overlay[0].source);
		if(papb->overlay[0].flags&APBFLAG_OVERLAYBLEND)
			APB_GetFrames(pao->apb,&pao->apb->overlay[0].dest);
	}

	if(papb->overlay[1].flags&APBFLAG_OVERLAY)
	{
		APB_GetFrames(pao->apb,&pao->apb->overlay[1].source);
		if(papb->overlay[1].flags&APBFLAG_OVERLAYBLEND)
			APB_GetFrames(pao->apb,&pao->apb->overlay[1].dest);
	}



}





void APB_Advance(ANIMOBJECT* pao)
{
	ANIMATION_PLAYBACK* papb;
	float tmul;
	papb = pao->apb;

	tmul = 	mathFabs(gAnimTargetMS/papb->msPerFrame);

	if(papb->flags&APBFLAG_PLAY)
	{
		float timestep;

		timestep = (gAnimTargetMSSquared / papb->animation.source.msperframe)*tmul;

		papb->animation.source.time += timestep;

		// Check If Past Last Frame On Non Looping Anims

		if(!(papb->animation.source.anim->flags&APBFLAG_LOOP))
		{
			if(papb->animation.source.time > APB_FrameToMS((float)(papb->animation.source.anim->nFrames-1)))
			{
				if(!(papb->flags&APBFLAG_NOSOURCEADVANCE))
					papb->animation.source.time = APB_FrameToMS((float)(papb->animation.source.anim->nFrames-1));

				papb->animation.source.flags |=  APBFLAG_FINISHED;
			}
		}

		if(papb->flags&APBFLAG_BLEND)
		{
			timestep = (gAnimTargetMSSquared / papb->animation.dest.msperframe)*tmul;

			if(!(papb->flags&APBFLAG_NODESTADVANCE))
				papb->animation.dest.time += timestep;
		}

		if(papb->overlay[0].flags&APBFLAG_OVERLAY)
		{
			timestep = (gAnimTargetMSSquared / papb->overlay[0].source.msperframe)*tmul;

			if(!(papb->overlay[0].flags&APBFLAG_HIDEOVERLAY))
				papb->overlay[0].source.time += timestep;

			timestep = (gAnimTargetMSSquared / papb->overlay[0].dest.msperframe)*tmul;

			if(papb->overlay[0].flags&APBFLAG_OVERLAYBLEND)
				papb->overlay[0].dest.time += timestep;
		}


		if(papb->overlay[1].flags&APBFLAG_OVERLAY)
		{
			timestep = (gAnimTargetMSSquared / papb->overlay[1].source.msperframe)*tmul;


			if(!(papb->overlay[1].flags&APBFLAG_HIDEOVERLAY))
				papb->overlay[1].source.time += timestep;

			timestep = (gAnimTargetMSSquared / papb->overlay[1].dest.msperframe)*tmul;

			if(papb->overlay[1].flags&APBFLAG_OVERLAYBLEND)
				papb->overlay[1].dest.time += timestep;
		}
	}
}





void APB_Visibility(ANIMOBJECT* pao)
{
	uint32 i;
	ANIMATION_PLAYBACK* papb;
	papb = pao->apb;

	BORDER(0x00007f);

	// Process Visibility Of Parented Meshes

	if (papb->animation.source.anim)
	{
		for (i=0; i<papb->animation.source.anim->nKeys; i++)
		{
			if (papb->animation.source.anim->keys[i].type==TRACK_KEY_CODE_MAKE_INVISIBLE ||
				papb->animation.source.anim->keys[i].type==TRACK_KEY_CODE_MAKE_VISIBLE)
			{
#if defined( PS2) || defined( _GAME_PC_)
				if ((short) papb->animation.source.anim->keys[i].time==pao->apb->animation.source.frame0)
#else
				if (papb->animation.source.anim->keys[i].time<=APB_MSToFrame(pao->apb->animation.source.time))
#endif
				{
					int visibility = 1;

					if (papb->animation.source.anim->keys[i].type==TRACK_KEY_CODE_MAKE_INVISIBLE)
						visibility = 0;

					AO_SetParentedObjectVisibility(pao,papb->animation.source.anim->keys[i].data,visibility);
				}
			}
		}
	}


	if(papb->overlay[0].flags&APBFLAG_OVERLAY)
	{
		if (papb->overlay[0].source.anim)
		{
			for (i=0; i<papb->overlay[0].source.anim->nKeys; i++)
			{
				if (papb->overlay[0].source.anim->keys[i].time<=APB_MSToFrame(pao->apb->overlay[0].source.time))
				{
					int visibility;

					if (papb->overlay[0].source.anim->keys[i].type==TRACK_KEY_CODE_MAKE_INVISIBLE)
						visibility = 0;
					else
						visibility = 1;

					AO_SetParentedObjectVisibility(pao,papb->overlay[0].source.anim->keys[i].data,visibility);
				}
			}
		}

	}


	if(papb->overlay[1].flags&APBFLAG_OVERLAY)
	{
		if (papb->overlay[1].source.anim)
		{
			for (i=0; i<papb->overlay[1].source.anim->nKeys; i++)
			{
				if (papb->overlay[1].source.anim->keys[i].time<=APB_MSToFrame(pao->apb->overlay[1].source.time))
				{
					int visibility;

					if (papb->overlay[1].source.anim->keys[i].type==TRACK_KEY_CODE_MAKE_INVISIBLE)
						visibility = 0;
					else
						visibility = 1;

					AO_SetParentedObjectVisibility(pao,papb->overlay[1].source.anim->keys[i].data,visibility);
				}
			}
		}

	}

	BORDER(0);

}


uint32 nFXList;
ANIMFX FXList[MAX_FXLIST];




void APB_ProcessFXTriggers(ANIMOBJECT* pao,ANIMATION_POSITIONINFO* pos,uint32 bCheckFlags)
{
	uint32 i;
	ANIMATION* anim;

	BORDER(0xff0000);

	if(pos->anim)
	{
		for (i=0; i<pos->anim->nKeys; i++)
		{
			if (pos->anim->keys[i].type==TRACK_KEY_CODE_FIRE_FXEVENT_ON ||
				pos->anim->keys[i].type==TRACK_KEY_CODE_FIRE_FXEVENT_OFF ||
				pos->anim->keys[i].type==TRACK_KEY_CODE_FIRE_FXEVENT_ONESHOT
				)
			{
				if ((short)pos->anim->keys[i].time <= (pos->frame0))
				{
					FXNODE* pFX;
					uint32 bFlags = 0;
					uint32 bone;
					uint32 bValid = TRUE;

					pFX = &pos->anim->FXNodes[pos->anim->keys[i].data];

					if(pos->anim->flags&APBFLAG_MIRROR)
					{
						bFlags |= FXNODE_MIRROR;
						bone = (pFX->boneID>>16)&0xffff;
					}
					else
						bone = (pFX->boneID)&0xffff;

					if(bCheckFlags)
					{
						if(!(pao->apb->bonecontrol[bone].flags&bCheckFlags))
							bValid = FALSE;
					}

					if(bValid)
					{
						if(pos->anim->keys[i].type == TRACK_KEY_CODE_FIRE_FXEVENT_ON)
							pFX->flags |= FXNODE_ACTIVE;
						else if(pos->anim->keys[i].type == TRACK_KEY_CODE_FIRE_FXEVENT_ONESHOT)
						{
							if(((short)pos->anim->keys[i].time == pos->frame0) && ((short)pos->anim->keys[i].time != pos->prevframe0))
								pFX->flags |= FXNODE_ACTIVE;
							else
								pFX->flags &= ~FXNODE_ACTIVE;
						}
						else
							pFX->flags &= ~FXNODE_ACTIVE;
					}
					else
						pFX->flags &= ~FXNODE_ACTIVE;
				}
			}
		}

		if(FXHandler)
		{
			for(i=0;i<pos->anim->nFXNodes;i++)
			{
				uint32 bFlags = 0;

				if(pos->anim->FXNodes[i].flags&FXNODE_ACTIVE)
				{
					if(pos->anim->flags&APBFLAG_MIRROR)
						bFlags |= FXNODE_MIRROR;


#if defined(PS2) || defined(_GAME_PC_)
					FXList[nFXList].pao     = pao;
					FXList[nFXList].flags   = bFlags;
					FXList[nFXList].pFXNode = &pos->anim->FXNodes[i];
					nFXList++;

					ASSERT(nFXList < MAX_FXLIST);

					if(FXHandlerImmediate && !pos->anim->FXNodes[i].Message)
					{
						FXHandlerImmediate(pao,&pos->anim->FXNodes[i],bFlags);
					}
					else
					{
						mapAnimEventHandler (pao,pos->anim->FXNodes + i);
					}
#else
					FXHandler(pao,&pos->anim->FXNodes[i],bFlags);
#endif
				}
			}

		}

		// Now Check Static FX Nodes On Animation

		anim =	pao->pStaticFXAnim;

		if(anim)
		{
			for (i=0; i< anim->nFXNodes; i++)
			{
				FXNODE* pFX;
				uint32 bone;
				uint32 bValid = TRUE;
				uint32 bFlags = 0;

				// Check If Current Anim Is Mirrored

				pFX = &anim->FXNodes[i];

				if(pos->anim->flags&APBFLAG_MIRROR)
				{
					bFlags |= FXNODE_MIRROR;
					bone = (pFX->boneID>>16)&0xffff;
				}
				else
					bone = (pFX->boneID)&0xffff;

				pFX->flags |= FXNODE_ACTIVE;
			}

			for(i=0;i<anim->nFXNodes;i++)
			{
				uint32 bFlags = 0;

				if(anim->FXNodes[i].flags&FXNODE_ACTIVE)
				{
					if(pos->anim->flags&APBFLAG_MIRROR)
						bFlags |= FXNODE_MIRROR;

#if defined(PS2) || defined(_GAME_PC_)
					FXList[nFXList].pao     = pao;
					FXList[nFXList].flags   = bFlags;
					FXList[nFXList].pFXNode = &anim->FXNodes[i];
					nFXList++;

					ASSERT(nFXList < MAX_FXLIST);

					if(FXHandlerImmediate)
						FXHandlerImmediate(pao,&anim->FXNodes[i],bFlags);
#endif
				}
			}

		}

	}

	BORDER(0x0);

}


#if defined(PS2) || defined(_GAME_PC_)

void	animProcessFXList()
{
	uint32 i;

	for(i=0;i<nFXList;i++)
	{
		if	(!FXList[i].pFXNode->Message)
		{
			FXHandler(FXList[i].pao,FXList[i].pFXNode,FXList[i].flags);
		}
		else
		{
			mapAnimEventHandler (FXList[i].pao,FXList[i].pFXNode);
		}
	}
}

#endif

















void APB_FXTriggers(ANIMOBJECT* pao)
{
	ANIMATION_PLAYBACK* papb;
	papb = pao->apb;

	if(papb->animation.flags&APBFLAG_BLEND)
		APB_ProcessFXTriggers(pao,&papb->animation.dest,0);
	else
		APB_ProcessFXTriggers(pao,&papb->animation.source,0);


	if(!(papb->overlay[0].flags&APBFLAG_HIDEOVERLAY))
	{
		if(papb->overlay[0].flags&APBFLAG_OVERLAY)
		{
			if(papb->overlay[0].flags&APBFLAG_OVERLAYBLEND)
				APB_ProcessFXTriggers(pao,&papb->overlay[0].dest,BONEFLAG_OVERLAY_CH0);
			else
				APB_ProcessFXTriggers(pao,&papb->overlay[0].source,BONEFLAG_OVERLAY_CH0);
		}
	}


	if(papb->overlay[1].flags&APBFLAG_OVERLAY)
	{
		if(papb->overlay[1].flags&APBFLAG_OVERLAYBLEND)
			APB_ProcessFXTriggers(pao,&papb->overlay[1].dest,BONEFLAG_OVERLAY_CH1);
		else
			APB_ProcessFXTriggers(pao,&papb->overlay[1].source,BONEFLAG_OVERLAY_CH1);
	}
}





void APB_AudioEvents(ANIMOBJECT* pao)
{
	uint32 i;
	ANIMATION_PLAYBACK* papb;
	uint32 overlayIndex;
	BOOL bSource = TRUE;
	BOOL bDest = TRUE;
	papb = pao->apb;

	if(papb->flags&APBFLAG_CONTROLBLEND)
	{
		if(papb->animation.blend.msBlendTime > papb->animation.blend.msBlend/2.0f)
			bSource = FALSE;
		else
			bDest = FALSE;
	}


	if (papb->animation.source.anim && bSource)
	{
		for (i=0; i<papb->animation.source.anim->nKeys; i++)
		{
			TRACK_KEY* key=&papb->animation.source.anim->keys[i];

			if (key->type==TRACK_KEY_CODE_FIRE_AUDIO_EVENT)
			{
				short time=(short)(key->time&0xFFFF); // Mask off time's lower 16 bits

				if (time==pao->apb->animation.source.frame0 &&
					time!=pao->apb->animation.source.prevframe0) // Is this key's time between the current frame markers?
				{
					if(key->time&AEFLAG_ISFOOTSTEP)
						FXHandler(pao,NULL,(key->time>>25)&0x3);
				}


				// Update Audio Events
				if (key->time&0x80000000) // Update flag is set
					UpdateAudioEvent(key,pao);

				// Cue Audio Events
				if ((key->time&0x40000000)==0)
				{
					if (time==pao->apb->animation.source.frame0 &&
						time!=pao->apb->animation.source.prevframe0) // Is this key's time between the current frame markers?
					{
						CueAudioEvent(key,pao);
					}
				}
			}
		}
	}


	if (papb->flags&APBFLAG_BLEND && bDest)
	{
		for (i=0; i<papb->animation.dest.anim->nKeys; i++)
		{
			TRACK_KEY* key=&papb->animation.dest.anim->keys[i];

			if (key->type==TRACK_KEY_CODE_FIRE_AUDIO_EVENT)
			{
				short time=(short)(key->time&0xFFFF); // Mask off time's lower 16 bits

				if (time==pao->apb->animation.dest.frame0 &&
					time!=pao->apb->animation.dest.prevframe0) // Is this key's time between the current frame markers?
				{
					if(key->time&AEFLAG_ISFOOTSTEP)
						FXHandler(pao,NULL,(key->time>>25)&0x3);
				}

				// Update Audio Events
				if (key->time&0x80000000) // Update flag is set
					UpdateAudioEvent(key,pao);

				// Cue Audio Events
				if ((key->time&0x40000000)==0)
				{

					if (time==pao->apb->animation.dest.frame0 &&
						time!=pao->apb->animation.dest.prevframe0) // Is this key's time between the current frame markers?
					{
						CueAudioEvent(key,pao);
					}
				}
			}
		}
	}

	for (overlayIndex=0;overlayIndex<2;overlayIndex++)
	{
		if(papb->overlay[overlayIndex].flags&APBFLAG_OVERLAY)
		{
			if (papb->overlay[overlayIndex].source.anim)
			{
				for (i=0; i<papb->overlay[overlayIndex].source.anim->nKeys; i++)
				{
					if (papb->overlay[overlayIndex].source.anim->keys[i].type==TRACK_KEY_CODE_FIRE_AUDIO_EVENT)
					{
						short time=(short)(papb->overlay[overlayIndex].source.anim->keys[i].time&0xFFFF);
						if (time==pao->apb->overlay[overlayIndex].source.frame0 &&
							time!=pao->apb->overlay[overlayIndex].source.prevframe0)
						{
							CueAudioEvent(&papb->overlay[overlayIndex].source.anim->keys[i],pao);
						}
					}
				}
			}

		}
	}

	for (overlayIndex=0;overlayIndex<2;overlayIndex++)
	{
		if(papb->overlay[overlayIndex].flags&APBFLAG_OVERLAY)
		{
			if (papb->overlay[overlayIndex].dest.anim)
			{
				for (i=0; i<papb->overlay[overlayIndex].dest.anim->nKeys; i++)
				{
					if (papb->overlay[overlayIndex].dest.anim->keys[i].type==TRACK_KEY_CODE_FIRE_AUDIO_EVENT)
					{
						short time=(short)(papb->overlay[overlayIndex].dest.anim->keys[i].time&0xFFFF);
						if (time==pao->apb->overlay[overlayIndex].dest.frame0 &&
							time!=pao->apb->overlay[overlayIndex].dest.prevframe0)
						{
							CueAudioEvent(&papb->overlay[overlayIndex].dest.anim->keys[i],pao);
						}
					}
				}
			}

		}
	}

}


void APB_AnimTriggers(ANIMOBJECT* pao)
{
	uint32 i, j;
	ANIMATION_PLAYBACK* papb;
	papb = pao->apb;

	if (papb->animation.source.anim)
	{
		for (i = 0; i < papb->animation.source.anim->nKeys; i++)
		{
			if ((papb->animation.source.anim->keys[i].time == (unsigned short) pao->apb->animation.source.frame0) && (papb->animation.source.anim->keys[i].time != (unsigned short) pao->apb->animation.source.prevframe0))
			{
				switch (papb->animation.source.anim->keys[i].type)
				{
					case TRACK_KEY_CODE_FIRE_ANIM_TRIGGER:
					{
						pao->apb->trigger_bones[0] = 0;
						pao->apb->trigger_bones[1] = 0;
						pao->apb->trigger_bones[2] = 0;
						pao->apb->trigger_bones[3] = 0;
						for(j = 7; j < pao->hierachy->nNodes; j++) pao->apb->trigger_bones[j >> 5] |= 1 << (j & 31);

						APB_SetOverlayTriggers(pao->apb);
						APB_OverlayAnimation(pao->apb, 0, papb->animation.source.anim->keys[i].data, 0, 8.0f, 0);
					}
					break;

					case TRACK_KEY_CODE_FIRE_MORPH_TRIGGER:
					{
						MORPH_SEQUENCE* sequence = MPH_GetMorphSequence(pao, papb->animation.source.anim->keys[i].data);
						if (sequence)
						{
							CueEditorSpeech(papb->animation.source.anim->keys[i].data);
							MPH_PlayMorphSequence(pao, sequence);

						}
					}
					break;
				}
			}
		}
	}

	if (papb->animation.dest.anim)
	{
		for (i = 0; i < papb->animation.dest.anim->nKeys; i++)
		{
			if ((papb->animation.dest.anim->keys[i].time == (unsigned short) pao->apb->animation.dest.frame0) && (papb->animation.dest.anim->keys[i].time != (unsigned short) pao->apb->animation.dest.prevframe0))
			{
				switch (papb->animation.dest.anim->keys[i].type)
				{
					case TRACK_KEY_CODE_FIRE_ANIM_TRIGGER:
					{
						pao->apb->trigger_bones[0] = 0;
						pao->apb->trigger_bones[1] = 0;
						pao->apb->trigger_bones[2] = 0;
						pao->apb->trigger_bones[3] = 0;
						for(j = 7; j < pao->hierachy->nNodes; j++) pao->apb->trigger_bones[j >> 5] |= 1 << (j & 31);

						APB_SetOverlayTriggers(pao->apb);
						APB_OverlayAnimation(pao->apb, 0, papb->animation.dest.anim->keys[i].data, 0, 4.0f, 0);
					}
					break;

					case TRACK_KEY_CODE_FIRE_MORPH_TRIGGER:
					{
						MORPH_SEQUENCE* sequence = MPH_GetMorphSequence(pao, papb->animation.dest.anim->keys[i].data);
						if (sequence)
						{
							CueEditorSpeech(papb->animation.dest.anim->keys[i].data);
							MPH_PlayMorphSequence(pao, sequence);
						}
					}
					break;
				}
			}
		}
	}
}



void APB_CheckLinkPath(ANIMOBJECT* pao)
{
	uint32 animID, altanimID;

	if(pao->apb->flags&APBFLAG_FOLLOWLINK)
	{
		if(pao->apb->linkpathidx >= pao->apb->linkpath->nlinks)
		{
			uint32 sourceID, blendID;
			ANIMATION *animation;

			animID = pao->apb->linkpath->destID;

			APB_BlendToAnimation(pao->apb, animID, 0, gAnimDefaultMSBlend, 0);

			animation = AL_FindAnimation(pao, animID);
			altanimID = ((animation) ? (animation->altanimID) : 0);

			sourceID = pao->apb->animation.source.anim->animID;

			if(pao->apb->flags&APBFLAG_BLEND)
				blendID = pao->apb->animation.dest.anim->animID;
			else if(pao->apb->flags&APBFLAG_BLENDWAIT)
				blendID = pao->apb->blendrange->destID;
			else
				blendID = 0;

			if (sourceID == animID || sourceID == altanimID || (blendID && (blendID == animID || blendID == altanimID)))
			{
				pao->apb->flags &= ~APBFLAG_FOLLOWLINK;
				pao->apb->flags &= ~APBFLAG_NOALTERNATEANIM;
			}
		}
		else
		{
			animID = pao->apb->linkpath->links[pao->apb->linkpathidx];
			if(APB_BlendToAnimation(pao->apb,animID,0,0,0))
				pao->apb->linkpathidx++;
		}
	}
}




/*
	Function	:	AO_FrameTick
	Useage		:	Update The Current Animation Frame
*/

void AO_FrameTick(ANIMOBJECT* pao)
{
	ANIMATION_PLAYBACK* papb;
	papb = pao->apb;
	assert(pao!=NULL);
	assert(pao->apb!=NULL);


#if defined(PS2) || defined(_GAME_PC_)
	if(!(pao->apb->flags&APBFLAG_PLAY))
		return;
#endif

	MPH_FrameTick(pao);
	APB_CheckBlendWait(pao);
	APB_CheckLinkWait(pao);
	APB_CheckDelayBlend(pao);
	APB_CheckBlendStatus(pao);

	APB_CheckLinkPath(pao);

	if(!(pao->flags&AO_NOADVANCE))
		APB_Advance(pao);

	APB_Visibility(pao);
	APB_FXTriggers(pao);
	APB_AudioEvents(pao);
	APB_AnimTriggers(pao);

	APB_GetAnimFrames(pao);

	if(!(pao->apb->flags&APBFLAG_ANIMATEDSET))
	{
		if(!(pao->apb->flags&APBFLAG_NOVELOCITY))
			APB_GetAnimationVelocity(pao);
	}

	// Update deltas
	mathVectorSub(&pao->apb->position, &pao->apb->OldPosition, &pao->apb->PositionDelta);
	mathCopyVector(&pao->apb->position, &pao->apb->OldPosition);


	if(pao->flags&AO_CUTSCENE)
	{
		VECTOR a,b;
		int f0,f1;
		float t;
		f0 = ((pao->apb->animation.source.frame0)*4);
		f1 = ((pao->apb->animation.source.frame1)*4);
		t  =  pao->apb->animation.source.tval;

		a.x = ((float*)pao->csdata)[f0+0];
		a.y = ((float*)pao->csdata)[f0+1];
		a.z = ((float*)pao->csdata)[f0+2];

//		printf("%d %.2f %.2f %.2f %.2f %.2f %s\n",f0>>2,pao->apb->animation.source.time,t,a.x,a.y,a.z,pao->apb->animation.source.anim->name);

		b.x = ((float*)pao->csdata)[f1+0];
		b.y = ((float*)pao->csdata)[f1+1];
		b.z = ((float*)pao->csdata)[f1+2];

		mathVectorInterp(&a,&b,t,&pao->apb->position);

/*
		{
		VECTOR v;
		mathUnitVector(&v);
		mathPushMatrix();
		mathLoadMatrix(&gmathCameraMatrix);
		mathTranslate(&pao->apb->position);
		sysDrawSphere(&v,128.0f,0xff0000);
		mathPopMatrix();
		}
*/

//		mathCopyVector(&a,&pao->apb->position);

		if(((float*)(pao->csdata))[f0+3])
		{
			pao->flags |= AO_VISIBLE;
		}
		else
			pao->flags &= ~AO_VISIBLE;
	}
}





void AO_AnimateSet(ANIMOBJECT* pao)
{
	uint32 i;
	HIERACHY* phr;
//	VECTOR t0,t1;
//	VECTOR s0,s1;
	VECTOR q0,q1;
	MATRIX previousMatrix,m;
	MATRIX *matrices = pao->matrices;
	FRAMEDATA fd0;
	FRAMEDATA fd1;
	FRAMEDATA* pfd0;
	FRAMEDATA* pfd1;

#ifdef PS2
	pfd0     = (FRAMEDATA*) (0x70000000);
	pfd1     = (FRAMEDATA*) (0x70000000 + sizeof(FRAMEDATA));
	matrices = (MATRIX*)    (0x70000000 + (sizeof(FRAMEDATA)*2));
#else
	pfd0 = &fd0;
	pfd1 = &fd1;
	matrices = pao->matrices;
#endif

	#if defined(PS2)
			BORDER(0x003F00);
	#endif

	phr = pao->hierachy;

	assert(phr != NULL);

	if(!pao->apb->animation.source.anim)
		return;

	mathPushMatrix();
	mathUnitMatrix(&previousMatrix);

#if defined(PS2) || defined(_GAME_PC_)

	if( !(pao->apb->flags&APBFLAG_PLAY) && !(pao->apb->flags&APBFLAG_FORCEUPDATE))			// AD: Added force update as a one shot state
	{
		matrices = pao->matrices;

		for(i=0;i<phr->nNodes;i++)
		{
			mathLoadMatrix(&matrices[i]);
			pao->apb->ProcessBone(pao,(BONE*) &phr->nodes.nodeptr[i],NULL,&matrices[i],i,0);
		}
	}
	else
#endif
	{
		pao->min.x = FLT_MAX;
		pao->min.y = FLT_MAX;
		pao->min.z = FLT_MAX;
		pao->max.x = -FLT_MAX;
		pao->max.y = -FLT_MAX;
		pao->max.z = -FLT_MAX;

		pao->apb->flags &= ~APBFLAG_FORCEUPDATE;		// AD: See above...

		for(i=0;i<phr->nNodes;i++)
		{
			if(phr->nodes.nodeptr[i].flags & BONEFLAG_MATRIXPOP)  mathPopMatrix();
			if(phr->nodes.nodeptr[i].flags & BONEFLAG_MATRIXPUSH) mathPushMatrix();

			mathStoreMatrix(&previousMatrix);

//			if(pao->apb->animation.source.anim)
			{
//				AT_GetSetFrameData(&pao->apb->animation.source.anim->data->tracks[i],pao->apb->animation.source.frame0,&q0,&t0,&s0);
//				AT_GetSetFrameData(&pao->apb->animation.source.anim->data->tracks[i],pao->apb->animation.source.frame1,&q1,&t1,&s1);

				AT_GetSetFrameData2(&pao->apb->animation.source,i,&pao->apb->animation.source.anim->data->tracks[i],pao->apb->animation.source.frame0,pao->apb->animation.source.frame1,pfd0,pfd1);

//				mathVectorInterp(&t0,&t1,pao->apb->animation.source.tval,&t0);
//				mathVectorInterp(&s0,&s1,pao->apb->animation.source.tval,&s0);
//				mathQuatSlerp(&q0,&q1,pao->apb->animation.source.tval,&q0);

				mathVectorInterp(&pfd0->tran,&pfd1->tran,pao->apb->animation.source.tval,&pfd0->tran);
				mathVectorInterp(&pfd0->scale,&pfd1->scale,pao->apb->animation.source.tval,&pfd0->scale);
				mathQuatSlerp(&pfd0->rot,&pfd1->rot,pao->apb->animation.source.tval,&pfd0->rot);

//				mathTranslate(&t0);

				mathTranslate(&pfd0->tran);

				mathMulMatrix(&phr->nodes.nodeptr[i].pre);

//				mathQuatToMatrix(&q0,&m);

				mathQuatToMatrix(&pfd0->rot,&m);

				if(pao->apb->bonecontrol[i].ControlBone)
				{
					mathPushMatrix();
					mathLoadMatrix(&previousMatrix);
					mathMulMatrix(&phr->nodes.nodeptr[i].pre);
					mathStoreMatrix(&previousMatrix);
					mathPopMatrix();
					pao->apb->bonecontrol[i].ControlBone(pao,&pao->apb->bonecontrol[i],&previousMatrix,&m,&q0);
				}
				else
					mathMulMatrix(&m);


				mathMulMatrix(&phr->nodes.nodeptr[i].mid);
				mathScale(&pfd0->scale);
				mathMulMatrix(&phr->nodes.nodeptr[i].post);
			}
//			else
//			{
//				mathMulMatrix(&phr->nodes.nodeptr[i].pre);
//				mathMulMatrix(&phr->nodes.nodeptr[i].mid);
//				mathMulMatrix(&phr->nodes.nodeptr[i].post);
//			}

			mathStoreMatrix(&matrices[i]);
		}

	#if defined(PS2)
		BORDER(0xffffff);
	#endif

		for(i=0;i<phr->nNodes;i++)
		{
			mathLoadMatrix(&matrices[i]);
			pao->apb->ProcessBone(pao,(BONE*) &phr->nodes.nodeptr[i],NULL,&matrices[i],i,0);
		}

#ifdef PS2
		sysCopyScratchMem128(pao->matrices, matrices, phr->nNodes * 4);
#endif


	#if defined(PS2)
		BORDER(0xff0000ff);
	#endif

	}

	mathPopMatrix();
}








void AO_DestroySkin(ANIMOBJECT* pao,uint32 skinID)
{
	uint32 i,j;

	for(i=0;i<pao->nskin;i++)
	{
		if(pao->skin[i]->skinID == skinID)
		{
			if(pao->selectedskin)
			{
				if(pao->selectedskin->skinID == skinID)
					pao->selectedskin = NULL;
			}



			// ** ToDo Free Skin In Here!



			for(j=i;j<pao->nskin-1;j++)
				pao->skin[j] = pao->skin[j+1];

			pao->nskin --;
		}
	}
}





/*

#define		APBFLAG_BLEND					(1<<0)
#define		APBFLAG_MIRROR					(1<<1)
#define		APBFLAG_PLAY					(1<<2)
#define		APBFLAG_LOCK					(1<<3)
#define		APBFLAG_FORCEPLAY				(1<<4)
#define		APBFLAG_LOOP					(1<<5)
#define     APBFLAG_APPLYROT				(1<<6)
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
*/


#if defined( PS2) || defined( _GAME_PC_)

#define FH	19

#define APBFLAG_TOTAL	32


char* APBFlag_Table[APBFLAG_TOTAL] =
{
	"APBFLAG_BLEND",
	"APBFLAG_MIRROR",
	"APBFLAG_PLAY",
	"APBFLAG_NOVELOCITY",
	"APBFLAG_NODESTADVANCE",
	"APBFLAG_LOOP",
	"APBFLAG_NOSOURCEADVANCE",
	"APBFLAG_BLENDWAIT",
	"APBFLAG_FINISHED",
	"APBFLAG_LINK",
	"APBFLAG_OVERLAY",
	"APBFLAG_OVERLAYBLEND",
	"APBFLAG_LOOPED",
	"APBFLAG_BLENDCOMPLETE",
	"APBFLAG_FORCEBLEND",
	"APBFLAG_XFADEVELOCITY",
	"APBFLAG_NOLINK",
	"APBFLAG_LINKWAIT",
	"APBFLAG_DELAYBLEND",
	"APBFLAG_ANIMATEDSET",
	"APBFLAG_OVERLAYBLENDCOMPLETE",
	"APBFLAG_NOBLENDWAIT",
	"APBFLAG_TERMINATOR",
	"APBFLAG_FOLLOWLINK",
	"APBFLAG_NOFOLLOWLINK",
	"APBFLAG_NOBLEND",
	"APBFLAG_HIDEOVERLAY",
	"APBFLAG_NOALTERNATEANIM",
	"APBFLAG_ENDOVERLAY",
	"APBFLAG_TRIG_OVERLAY",
	"APBFLAG_CONTROLBLEND",
	"APBFLAG_FORCEUPDATE",
};



void dbgShowAPBFlags(int x,int *y,uint32 flags)
{
	int i;

	for(i=0;i<APBFLAG_TOTAL;i++)
	{
		if(flags&(1<<i))
		{
			*y+=FH;
			conDrawString(0,*y,APBFlag_Table[i]);

		}
	}
}



int dbgAnimInfo(ANIMOBJECT* pao)
{
	int y;
	ANIMATION* sanim;
	ANIMATION* danim;

	if(!pao->apb)
		return 0 ;

	debugLastEvalPos();

	sanim = pao->apb->animation.source.anim;
	danim = pao->apb->animation.dest.anim;

	y = 0;
	conDrawStringF(0,y,       "Source    : %s ",sanim->name);

	y += FH;

	conDrawStringF(0,y,       "Frame     : %d - %f %f",pao->apb->animation.source.frame0,pao->apb->animation.source.time,pao->apb->msPerFrame);

	y += FH;
	conDrawStringF(0,y,       "OSource    : %s ",pao->apb->overlay[0].source.anim);
	y += FH;
	conDrawStringF(0,y,       "ODest    : %s ",pao->apb->overlay[0].dest.anim);

	y += FH;
	conDrawStringF(0,y,       "Frame     : %d - %f %f",pao->apb->overlay[0].source.frame0,pao->apb->overlay[0].source.time,pao->apb->msPerFrame);

	if(pao->apb->flags&APBFLAG_BLEND)
	{
		conDrawStringF(0,y+FH,"Blend     : %s",danim->name);
	}
	else if(pao->apb->flags&APBFLAG_BLENDWAIT)
	{
		uint32 i;

		danim = AL_FindAnimation(pao,pao->apb->blendrange->destID);
		conDrawStringF(0,y+FH,"BlendWait : %s",danim->name);

		y+= FH;

		for(i=0;i<pao->apb->blendrange->nClips;i++)
		{
			conDrawStringF(0,y+FH,"%.2f %.2f %.2f",pao->apb->blendrange->BlendClips[i].source.msStart,
													pao->apb->blendrange->BlendClips[i].source.msEnd,
													pao->apb->blendrange->BlendClips[i].msBlendTime);
			y+= FH;

		}
	}
	else if(pao->apb->flags&APBFLAG_LINKWAIT)
	{
		danim = AL_FindAnimation(pao,pao->apb->linkrange->destID);
		conDrawStringF(0,y+FH,"LinkWait  : %s",danim->name);
	}
	else
		conDrawStringF(0,y+FH,"Dest      : None",danim->name);



	if(pao->apb->animation.source.flags&APBFLAG_LOOPED)
	{
		y+=FH;
		conDrawStringF(0,y+FH,"SOURCE LOOPED");
	}


	y+=FH;
	dbgShowAPBFlags(0,&y,pao->apb->flags);


//	showState(pao);


	return 0;
}

#endif

void MPH_AddMorphMesh(ANIMOBJECT* pao, MORPH_MESH* mesh)
{
	ASSERT(pao);
	ASSERT(pao->NumMorphMeshes < MAX_MORPH_MESHES);

	pao->MorphMeshes[pao->NumMorphMeshes++] = mesh;
}

void MPH_RemoveMorphMesh(ANIMOBJECT* pao, MORPH_MESH* mesh)
{
	uint32 i, j;

	ASSERT(pao);

	if (mesh)
	{
		for (i = 0; i < pao->NumMorphMeshes; i++)
		{
			if (pao->MorphMeshes[pao->NumMorphMeshes++] == mesh) break;
		}

		if (i == pao->NumMorphMeshes) return;

		memfree((void*)((uint32)mesh - 8));

		pao->NumMorphMeshes--;

		for (j = i; j < pao->NumMorphMeshes; j++)
		{
			pao->MorphMeshes[j] = pao->MorphMeshes[j + 1];
		}
	}
	else
	{
		for (i = 0; i < pao->NumMorphMeshes; i++)
		{
			memfree(pao->MorphMeshes[i]);
		}

		pao->NumMorphMeshes = 0;
	}
}

MORPH_MESH* MPH_GetMorphMesh(ANIMOBJECT* pao, uint32 hash)
{
	uint32 i;

	ASSERT(pao);

	for (i = 0; i < pao->NumMorphMeshes; i++)
	{
		if (pao->MorphMeshes[i]->Id == hash) return pao->MorphMeshes[i];
	}

	return NULL;
}

void MPH_AddMorphSequence(ANIMOBJECT* pao, MORPH_SEQUENCE* sequence)
{
	ASSERT(pao);
	ASSERT(pao->NumMorphSequences < MAX_MORPH_SEQUENCES);

	pao->MorphSequences[pao->NumMorphSequences++] = sequence;
}

void MPH_RemoveMorphSequence(ANIMOBJECT* pao, MORPH_SEQUENCE* sequence)
{
	uint32 i, j;

	ASSERT(pao);

	if (sequence)
	{
		for (i = 0; i < pao->NumMorphSequences; i++)
		{
			if (pao->MorphSequences[pao->NumMorphSequences++] == sequence) break;
		}

		if (i == pao->NumMorphSequences) return;

		memfree(sequence);
			// DLH - Added to stop the code accessing the morph after it has been deleted
		if( pao->Morph.Sequence == sequence)
		{
			pao->Morph.Sequence = NULL;
		}

		pao->NumMorphSequences--;

		for (j = i; j < pao->NumMorphSequences; j++)
		{
			pao->MorphSequences[j] = pao->MorphSequences[j + 1];
		}
	}
	else
	{
		for (i = 0; i < pao->NumMorphSequences; i++)
		{
			memfree(pao->MorphSequences[i]);
			pao->MorphSequences[i] = NULL;
		}

			// DLH - Added to stop the code accessing the morph after it has been deleted
		pao->Morph.Sequence = NULL;
		pao->NumMorphSequences = 0;
	}
}

MORPH_SEQUENCE* MPH_GetMorphSequence(ANIMOBJECT* pao, uint32 hash)
{
	uint32 i;

	ASSERT(pao);

	for (i = 0; i < pao->NumMorphSequences; i++)
	{
		if (pao->MorphSequences[i]->Id == hash) return pao->MorphSequences[i];
	}

	return NULL;
}

void MPH_PlayMorphSequence(ANIMOBJECT* pao, MORPH_SEQUENCE* sequence)
{
	pao->Morph.Sequence = sequence;
	pao->Morph.Time = 0.0f;
}

void MPH_FrameTick(ANIMOBJECT *ao)
{
	float add;
	int frame;
	unsigned int i;

	MORPH* morph = &ao->Morph;

	add = gAnimTargetMSSquared /ao->apb->msPerFrame;

	frame = (int)APB_MSToFrame(morph->Time);

	if (morph->Sequence)
	{
		MORPH_SEQUENCE *sequence = morph->Sequence;
		float total_time = APB_FrameToMS((float)sequence->NumFrames);
 		float envelope;

		// get envelope scale
		APB_GetTrackValue(sequence->Envelope->AnimBlock, frame, &envelope, 1.0f/8192.0f);

		// setup factors for blending
		for (i = 0; i < sequence->NumTracks; i++)
		{
			float factor;

			if(i==MAX_MORPH_TARGETS)
				break;

			APB_GetTrackValue(sequence->Tracks[i].AnimBlock, frame, &factor, 1.0f/8192.0f);

			morph->Factors[i] = factor * envelope;
		}

		morph->Time += add;


//		if(gameGetState() != GAME_STATE_CINEMATIC)
		{
			if (morph->Time >= total_time)
			{
				morph->Time = 0.0f;
				morph->Factors[0] = 0.0f;
				morph->Factors[1] = 0.0f;
				morph->Factors[2] = 0.0f;
				morph->Factors[3] = 0.0f;
				morph->Factors[4] = 0.0f;
				morph->Factors[5] = 0.0f;
				morph->Factors[6] = 0.0f;
				morph->Sequence = NULL;
			}
		}
	}
}



/*
	Function	:	AO_FakeFrameTick
	Useage		:	Update The Fake Animation Frame
*/

void AO_FakeFrameTick(ANIMOBJECT* pao)
{
	APB_CheckBlendWait(pao);
	APB_CheckLinkWait(pao);
	APB_CheckDelayBlend(pao);
	APB_CheckBlendStatus(pao);
	APB_CheckLinkPath(pao);
	APB_Advance(pao);
	APB_GetAnimFrames(pao);
	APB_GetAnimationVelocity(pao);
}


#if (defined( PS2) || defined( _GAME_PC_)) && !ISME(CC)
	#define _APB_DRAWDEBUG
#endif

#define MAX_LOOKAHEAD 80

static VECTOR pos;
static VECTOR rot;
static VECTOR dist;

void debugLastEvalPos()
{
	mathPushMatrix();
	mathTranslate(&pos);
	mathRotZ(DTOR(rot.z));
	#if defined( _APB_DRAWDEBUG)
		sysDrawCross(&dist,64,0xff0000);
	#endif
	mathPopMatrix();
}

void APB_EvalDestination(ANIMATION_PLAYBACK* papb,uint32 destid,float start,float blend,uint32 flags,VECTOR* eval)
{
	ANIMATION_PLAYBACK apb;
	ANIMOBJECT ao;
	BOOL bRetVal;

#ifdef PS2
	BORDER(0xff8080);
#endif

	if((!papb->animation.source.anim) || (destid == papb->animation.source.anim->animID))
	{
		mathUnitVector(eval);
		eval->w = -1;
		return;
	}

	ao.apb = &apb;
	ao.Morph.Sequence = NULL;
	ao.selectedskin = NULL;
	ao.pAnimationList = papb->pao->pAnimationList;

	APB_Initialise(&ao);
	memcopy(&apb.animation,&papb->animation,sizeof(ANIMATION_CONTROL));
	apb.pao = &ao;
	apb.flags = APBFLAG_PLAY; //papb->flags;// | APBFLAG_FORCEBLEND;
	apb.msPerFrame = gAnimTargetMS/4.0f; // 0.125f;   //papb->msPerFrame;
	apb.pao->hierachy = 0;

	bRetVal = APB_BlendToAnimation(&apb,destid,start,blend,flags);

	if(!bRetVal)
	{
		mathUnitVector(eval);
		eval->w = -1;
		return;
	}

	#if defined( _APB_DRAWDEBUG)
//		sysDrawCross(&papb->position,64,0xff0000);
	#endif

	while(1)
	{
		AO_FakeFrameTick(&ao);


		if(apb.animation.source.anim->animID == destid)
			break;

		mathVectorAdd(&apb.velocity,&apb.position,&apb.position);


		if(apb.position.w > MAX_LOOKAHEAD)
		{
			apb.position.w = -1;
			break;
		}
	}

	mathCopyVector(&apb.position,eval);
/*
	{
	VECTOR v;
	mathUnitVector(&v);

	mathPushMatrix();
	mathTranslate(&papb->position);
	mathRotZ(DTOR(papb->rotation.z));
	#if defined( _APB_DRAWDEBUG)
		sysDrawAddWorldLine(&v,&apb.position,0xffffffff);
	#endif
	mathPopMatrix();

	mathPushMatrix();
	mathTranslate(&papb->position);
	mathRotZ(DTOR(papb->rotation.z));
	#if defined( _APB_DRAWDEBUG)
		sysDrawCross(&apb.position,64,0xff0000);
	#endif
	mathPopMatrix();

	mathCopyVector(&papb->position,&pos);
	mathCopyVector(&papb->rotation,&rot);
	mathCopyVector(&apb.position,&dist);
	}
*/

#ifdef PS2
	BORDER(0x0);
#endif


//	printf("** %.2f %.2f %.2f in %.2f\n",eval->x,eval->y,eval->z,eval->w);
//	APB_EvalDestination2(papb,destid,start,blend,flags,eval);

	if(apb.animation.source.anim->animID != destid)
	{
		ANIMATION* pa;
		pa = AL_FindAnimation(&ao,destid);

		if(pa)
		{
			printf("Unable To Reach %s From %s in %d Frames\n",pa->name,papb->animation.source.anim->name,MAX_LOOKAHEAD);
		}
		else
			printf("Error : Trying To Reach Invalid Anim\n");
	}
}















