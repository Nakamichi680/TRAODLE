#include "stdafx.h"

#include "mathlib\cmaths.h"
#include "AnimLib\animapi.h"
#include "animation.h"
#include "AnimLib\hierachy.h"
#include "Animation EditorView.h"
#include "editortogame.h"
#include "stripe interface\\stripper.h"
#include "morph.h"
#include "AnimLib\magic\MgcCont3DCapsule.h"

#include "wglext.h"


extern CTexture* gSkinTexture;

#include "CGlobal.h"
#include "CBlendClip.h"

int nMatrices;
MATRIX Matrices[256];
extern int envmapid;

void fxAnimTriggerHandler(ANIMOBJECT* pao,FXNODE* pFX,uint32 flags);


//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

static int gBlendIdx = 0;
static int gnRanges = 0;

void DrawFXNodes(ANIMOBJECT* ao,FXNODE* node,uint32 flags);


void ExportBlendRange(ANIMATION* pa,CBlendClip* pbc)
{
	int i,j;
	CBlendRange range;

//	pa->BlendRanges[gBlendIdx]		       = (ANIMATION_BLENDRANGE*) malloc(sizeof(ANIMATION_BLENDRANGE));
	pa->BlendRanges[gBlendIdx].destID     = pbc->to->hashid;
	pa->BlendRanges[gBlendIdx].nClips     = pbc->TotalUsedRanges();
	pa->BlendRanges[gBlendIdx].BlendClips = (BLENDCLIP*) malloc(pa->BlendRanges[gBlendIdx].nClips * sizeof(BLENDCLIP));

	j = 0;

	for(i=0;i<8;i++)
	{
		range = pbc->ranges[i];
		
		if(!pbc->IsRangeUnused(i))
		{
			pa->BlendRanges[gBlendIdx].BlendClips[j].flags           = range.flags;
			pa->BlendRanges[gBlendIdx].BlendClips[j].dest.msStart    = APB_FrameToMS(range.toRange.start);
			pa->BlendRanges[gBlendIdx].BlendClips[j].dest.msEnd      = APB_FrameToMS(range.toRange.end);
			pa->BlendRanges[gBlendIdx].BlendClips[j].source.msStart  = APB_FrameToMS(range.fromRange.start);
			pa->BlendRanges[gBlendIdx].BlendClips[j].source.msEnd    = APB_FrameToMS(range.fromRange.end);
			pa->BlendRanges[gBlendIdx].BlendClips[j].msBlendTime     = range.BlendSpeed;
			j++;
		}
	}

	gBlendIdx++;
}


void RemoveBlendClips(ANIMOBJECT* pao)
{
	uint32 i;
	ANIMATION_LIST* pal;

	pal = pao->pAnimationList;

	if(!pal) return;
	
	for(i=0;i<pal->nEntries;i++)
	{
		if(pal->animation[i]->nBlendRanges)
		{
//			for(j=0;j<pal->animation[i]->nBlendRanges;j++)
//				free(pal->animation[i]->BlendRanges[j]->BlendClips);
	
			free(pal->animation[i]->BlendRanges);
			
			pal->animation[i]->nBlendRanges = 0;
			pal->animation[i]->BlendRanges  = NULL;
		}

		if(pal->animation[i]->nLinkRanges)
		{
			
			free(pal->animation[i]->Links);
			
			pal->animation[i]->Links = NULL;
			pal->animation[i]->nLinkRanges = 0;
		}
	}
}



// Export All Blend Clips To Game

int GetBlendClipCount(unsigned int animid,const CList <CBlendClip*,CBlendClip*>& BlendList);
int GetLinkCount(unsigned int animid,const CList <CBlendClip*,CBlendClip*>& BlendList);

void E2G_BlendClips()
{
	POSITION pos;
	POSITION animpos;
	ANIMATION_LIST* pal;
	CAnimationEditorDoc* doc;
	doc = GetDocument();


	pal = doc->GetCharacter()->animObject->pAnimationList;

	if(!pal) return;
	if(!pal->animation) return;

	animpos = doc->GetData()->AnimationList.GetHeadPosition();

	RemoveBlendClips(doc->GetCharacter()->animObject);


	while(animpos)
	{
		int nRanges; 
		CAnimation *anim;
		ANIMATION* pa;
		
		anim = doc->GetData()->AnimationList.GetAt(animpos);
		pa   = AL_FindAnimation(doc->GetCharacter()->animObject,anim->hashid);
		
		// Export Blend Ranges
		
		nRanges = GetBlendClipCount(anim->hashid,doc->GetData()->BlendClips);
		
		pa->nBlendRanges = nRanges;

		if(nRanges)
			pa->BlendRanges  = (ANIMATION_BLENDRANGE*) malloc(pa->nBlendRanges * sizeof(ANIMATION_BLENDRANGE));
		else
			pa->BlendRanges  = NULL;

		pos = doc->GetData()->BlendClips.GetHeadPosition();
		
		gBlendIdx = 0;
		
		while (pos)
		{
			CBlendClip *pbc;
			CBlendClipLink* pbl;

			pbc = doc->GetData()->BlendClips.GetAt(pos);

			if(pbc->mn_BlendClipType == BlendClip_Standard)
			{
				if(pa->animID == pbc->from->hashid && pbc->TotalUsedRanges())
					ExportBlendRange(pa,pbc);
			}

			if(pbc->mn_BlendClipType == BlendClip_Link)
			{
				pbl = (CBlendClipLink*) pbc;
				
				if(pbl->LeftFrom.from->hashid == pa->animID && pbl->LeftFrom.TotalUsedRanges())
					ExportBlendRange(pa,&pbl->LeftFrom);
					
				if(pbl->LeftTo.from->hashid == pa->animID && pbl->LeftTo.TotalUsedRanges())
					ExportBlendRange(pa,&pbl->LeftTo);
				
				if(pbl->RightFrom.from->hashid == pa->animID && pbl->RightFrom.TotalUsedRanges())
					ExportBlendRange(pa,&pbl->RightFrom);
					
				if(pbl->RightTo.from->hashid == pa->animID && pbl->RightTo.TotalUsedRanges())
					ExportBlendRange(pa,&pbl->RightTo);
			}
			
			doc->GetData()->BlendClips.GetNext(pos);
		}

		AL_SortBlendList(pa);

		// Export Link Table
	
		pa->nLinkRanges = GetLinkCount(pa->animID,doc->GetData()->BlendClips);	
		pa->Links       = NULL;

		int idx = 0;

		if(pa->nLinkRanges)
		{
			pa->Links = (ANIMATION_LINK*) malloc(pa->nLinkRanges * sizeof(ANIMATION_LINK));
					
			pos = doc->GetData()->BlendClips.GetHeadPosition();

			while(pos)
			{
				CBlendClip *pbc;
				CBlendClipLink* pbl;

				pbc = doc->GetData()->BlendClips.GetAt(pos);

				if(pbc->mn_BlendClipType == BlendClip_Link)
				{
					pbl = (CBlendClipLink*) pbc;
			
					if(pa->animID == pbc->from->hashid && (pbl->LeftFrom.TotalUsedRanges() || pbl->RightFrom.TotalUsedRanges()))
					{
				
//						if(pbl->LeftFrom.TotalUsedRanges())
//							Log("%s (%s) %s",pbl->from->fname,pbl->LeftFrom.to->fname,pbc->to->fname);
				
//						if(pbl->RightFrom.TotalUsedRanges())
//							Log("%s (%s) %s",pbl->from->fname,pbl->RightFrom.to->fname,pbc->to->fname);

//						pa->Links[idx]  = (ANIMATION_LINK*) malloc(sizeof(ANIMATION_LINK));

						pa->Links[idx].destID  = pbc->to->hashid;
						pa->Links[idx].LLinkID = pbl->LeftFrom.to->hashid;
						pa->Links[idx].RLinkID = pbl->RightFrom.to->hashid;

						idx++;
					
					}
				}
		
				doc->GetData()->BlendClips.GetNext(pos);
			}
		}

		// Next Animation

		doc->GetData()->AnimationList.GetNext(animpos);
	}
}








//----------------------------------------------------------------------------------------------------

void DynamicSet::ExportToGame(void**pResult)
{
	int i,j;
	ANIMATION* animation;

	animation = (ANIMATION *)malloc(sizeof(ANIMATION));

	animation->flags   = 0;

	CString file;
	file = fname;
	file = "CUTSCENE_"+file.Left(file.Find("."));
	file.MakeUpper();
	this->fname = file;


	strcpy(animation->name,(LPCSTR) fname);
	animation->animID  = STRING_GetHashValue(animation->name);
	animation->nFrames = this->mn_FrameCount;
	animation->nTracks = this->mn_NodeCount;

	animation->data->tracks  = (ANIMATION_TRACK *)malloc(sizeof(ANIMATION_TRACK)*this->mn_NodeCount);

	// ---------- TEMP

	animation->keys = NULL;
	animation->nBlendRanges = 0;
	animation->BlendRanges = NULL;
	animation->nLinkRanges = 0;
	animation->Links = NULL;
	animation->flags = 0;

	for(j=0;j<this->mn_NodeCount;j++)
	{
		for(i=0;i<AT_TOTAL;i++)
		{
			animation->data->tracks[j].flags		  = this->mp_Tracks[j].flag;
			animation->data->tracks[j].animblock[i] = this->mp_Tracks[j].animblock[i];
			animation->data->tracks[j].animsize[i]  = this->mp_Tracks[j].animsize[i];
		}
	}

	animation->nKeys = 0;


	*(pResult) = animation;
}



/*
	Function	:	E2G_Animations
	Useage		:	Send Animation To The Game Animation List
*/

void E2G_DynamicSet(DynamicSet* animation)
{
	ANIMATION* anim = NULL;

	if(!GetDocument()->GetCharacter()->pAnimationList)
	{
		GetDocument()->GetCharacter()->pAnimationList = (ANIMATION_LIST *)malloc(sizeof(ANIMATION_LIST));

		AL_Initialise(GetDocument()->GetCharacter()->pAnimationList,0);
	}

	animation->ExportToGame((void**)&anim);

	// Initialise Blend Ranges

	anim->nBlendRanges = 0;
	anim->BlendRanges  = NULL;

	// Add Single Animation

	AL_AddAnimation(GetDocument()->GetCharacter()->pAnimationList,anim);

	// Sort List

	AL_SortAnimationList(GetDocument()->GetCharacter()->pAnimationList);

	// Set The Animation List

	AL_SetAnimationList(GetDocument()->GetCharacter()->animObject,GetDocument()->GetCharacter()->pAnimationList);



	animation->pGameData  = anim;
}



extern int nHandBones;
extern char HandTable[14][20];


void CAnimation::ExportToGame(void**pResult)
{
	int i,j;
	ANIMATION* animation;

	animation = (ANIMATION *)malloc(sizeof(ANIMATION));

	memset(animation,0,sizeof(ANIMATION));

	animation->flags   = 0;
	strcpy(animation->name,(LPCSTR) fname);
	animation->animID  = STRING_GetHashValue(animation->name);
	
	animation->nFrames = nFrames;
	animation->nTracks = nBones + this->nParents;

	animation->data = (ANIMATION_DATA*) malloc(sizeof(ANIMATION_DATA));

	animation->data->tracks  = (ANIMATION_TRACK *)malloc(sizeof(ANIMATION_TRACK)*animation->nTracks);

	// ---------- TEMP

	animation->keys = NULL;
	animation->nKeys = 0;
	if (nKeys!=0)
	{
		animation->keys = (TRACK_KEY *)malloc(sizeof(TRACK_KEY)*nKeys);
		animation->nKeys = nKeys;
	}
	for (i=0; i<nKeys; i++)
	{
		animation->keys[i].data = this->pKeys[i].data;
		animation->keys[i].type = this->pKeys[i].type;
		animation->keys[i].time = this->pKeys[i].time;
	}

	// ---------- TEMP

	animation->nBlendRanges = 0;
	animation->BlendRanges = NULL;

	animation->nLinkRanges = 0;
	animation->Links = NULL;
	animation->flags = this->Flags;

	int bidx = 0;

	bool bPose;

	for(j=0;j<nBones;j++)
	{
		int flag = 0;
		int cnt = 0;
		int idx;
	
		if(gRemoveHandData)
		{
			CString name = this->bones[j].name;
			
			bPose = false;

			for(int k=0;k<nHandBones;k++)
			{
				CString left  = HandTable[k];
				CString right = HandTable[k];
				left  += "_L";
				right += "_R";

				if(name.Find(left,0) != -1)
					bPose = true;
					
				if(name.Find(right,0) != -1)
					bPose = true;
			}

			if(bPose) 
				continue;
			
		}
		
		flag = Tracks[j].flag;
		animation->data->tracks[bidx].flags = flag;

		for(i=0;i<AT_TOTAL;i++)
		{
			cnt += flag&1;
			flag >>= 1;
		}

		animation->data->tracks[bidx].animblock = (uint8**) malloc(sizeof(uint8*)*cnt);
		animation->data->tracks[bidx].animsize  = (uint32*) malloc(sizeof(uint32)*cnt);

		idx = 0;
		
		for(i=0;i<AT_TOTAL;i++)
		{
			if(Tracks[j].flag&(1<<i))
			{				
				animation->data->tracks[bidx].animblock[idx] = Tracks[j].animblock[idx];
				animation->data->tracks[bidx].animsize[idx]  = Tracks[j].animsize[idx];
				idx++;
			}
		}
	
		bidx++;
	}
	
	animation->nTracks = bidx + this->nParents;
	

	// Now Add The Parent Tracks On

	animation->nParentTracks = 0;
	animation->ParentTracks = NULL;
	animation->ParentIdx = NULL;

	if(nParents)
	{
		int off = bidx;

		animation->nParentTracks = nParents;
		animation->ParentTracks = (uint32*) malloc(sizeof(uint32)*nParents);
		animation->ParentIdx = (uint32*) malloc(sizeof(uint32)*nParents);

		for(j=0;j<nParents;j++)
		{
			int flag = 0;
			int cnt = 0;
			int idx;

			flag = ParentTracks[j].flag;
			animation->data->tracks[j+off].flags = ParentTracks[j].flag;

			for(i=0;i<AT_TOTAL;i++)
			{
				cnt += flag&1;
				flag >>= 1;
			}
	
			if(cnt)
			{
				animation->data->tracks[j+off].animblock = (uint8**) malloc(sizeof(uint8*)*cnt);
				animation->data->tracks[j+off].animsize  = (uint32*) malloc(sizeof(uint32)*cnt);
				
				animation->ParentTracks[j] = ParentTracks[j].id; // Store The Mesh ID
				animation->ParentIdx[j] = j+off;

				idx = 0;
						
				for(i=0;i<AT_TOTAL;i++)
				{
					if(ParentTracks[j].flag&(1<<i))
					{				
						animation->data->tracks[j+off].animblock[idx] = ParentTracks[j].animblock[idx];
						animation->data->tracks[j+off].animsize[idx]  = ParentTracks[j].animsize[idx];
						idx++;
					}
				}

			}
		}
	}
		
	// Now Export The FXNodes
	
	animation->nFXNodes = this->nFxNodes;
	animation->FXNodes  = (FXNODE*) malloc(sizeof(FXNODE)*this->nFxNodes);

	for(i=0;i<this->nFxNodes;i++)
	{
		strcpy(animation->FXNodes[i].name,(char*)(LPCSTR)this->FxNodes[i].name);
		
		BONE* bone = HR_FindBone(GetDocument()->GetData()->m_Character.animObject->hierachy,this->FxNodes[i].bonehash);
		
		
		if(GetDocument()->GetData()->bAnimatedSet)
		{	
			NODE* node;

			node  = (NODE*) bone;
			animation->FXNodes[i].boneID  = node->boneidx;
		}
		else
			animation->FXNodes[i].boneID = bone->boneidx;
		
		if(GetDocument()->GetData()->m_Character.animObject->hierachy->bonemap)
			animation->FXNodes[i].boneID |= (GetDocument()->GetData()->m_Character.animObject->hierachy->bonemap[bone->boneidx]<<16);
		
		mathCopyVector((VECTOR*)&this->FxNodes[i].r,&animation->FXNodes[i].rotate);
		mathCopyVector((VECTOR*)&this->FxNodes[i].t,&animation->FXNodes[i].translate);
		mathCopyVector((VECTOR*)&this->FxNodes[i].s,&animation->FXNodes[i].scale);
		
		// Export User Values
	
		animation->FXNodes[i].type		= this->FxNodes[i].fxType;	
		animation->FXNodes[i].argb		= this->FxNodes[i].argb;	
										
		animation->FXNodes[i].emit		= this->FxNodes[i].emit;	
		animation->FXNodes[i].speed		= this->FxNodes[i].speed;	
		animation->FXNodes[i].random	= this->FxNodes[i].rand;	
		animation->FXNodes[i].u0		= this->FxNodes[i].u0;	
		animation->FXNodes[i].u1		= this->FxNodes[i].u1;	
		animation->FXNodes[i].u2		= this->FxNodes[i].u2;	
		animation->FXNodes[i].u3		= this->FxNodes[i].u3;	
		animation->FXNodes[i].Message	= this->FxNodes[i].fxMsg;
	}

	*(pResult) = animation;
}


//----------------------------------------------------------------------------------------------------

void CMeshInfo::ExportToGame(void*pResult)
{
	MESH* pm;
	pm = (MESH*) pResult;
	int i;
	CStripMesh* stripmesh;
	stripmesh = new CStripMesh;

	stripmesh->SetNumVerts(nverts);

	for(i=0;i<nverts;i++)
		stripmesh->SetVertex(i,verts[(i*3)+0],verts[(i*3)+1],verts[(i*3)+2]);

	stripmesh->SetNumNormals(nnormals);

	for(i=0;i<nnormals;i++)
		stripmesh->SetNormal(i,normals[(i*3)+0],normals[(i*3)+1],normals[(i*3)+2]);

	stripmesh->SetNumUVs(nuvs);

	for(i=0;i<nuvs;i++)
		stripmesh->SetUV(i,uvs[(i*2)+0],uvs[(i*2)+1]);

	// Create The Mesh To Strip

	stripmesh->SetNumTris(ntris);

	for(i=0;i<ntris;i++)
	{
		stripmesh->SetVertIndex(i,iverts[(i*3)+0],iverts[(i*3)+1],iverts[(i*3)+2]);
		stripmesh->SetNormalIndex(i,inormals[(i*3)+0],inormals[(i*3)+1],inormals[(i*3)+2]);
		stripmesh->SetUVIndex(i,iuvs[(i*3)+0],iuvs[(i*3)+1],iuvs[(i*3)+2]);
	}

	CStripeInterface Stripper;
	Stripper.Strip(stripmesh);

	pm->nstrips   = stripmesh->nStrips;
	pm->strips    = (unsigned short*) stripmesh->StripData;
	pm->striplens = (unsigned long*) stripmesh->StripLengths;

	pm->nverts   = nverts;
	pm->nuvs     = nuvs;
	pm->nnormals = nnormals;

	pm->normals = normals;
	pm->uvs     = uvs;
	pm->verts   = verts;

//	pm->tpageidx = this->mesh->Materials[this->imaterials[0]].TChannels[0].idx;
	pm->matlidx  = this->imaterials[0];

	pm->min = this->min;
	pm->max = this->max;

	this->pStripMesh = stripmesh;

}

//-----------------------------------------------------------------------------------------------------


void CSkin::ExportToGame(void*pResult)
{
	SKIN* ps;
	int i,j,k;

	ps = (SKIN*) pResult;

	ps->texture   = (TEXTURE*) malloc(this->Mesh->nTextures*sizeof(TEXTURE));
	ps->nTextures = this->Mesh->nTextures;

	for(i=0;i<this->Mesh->nTextures;i++)
	{
		ps->texture[i].data = this->Mesh->Textures[i].data;
		ps->texture[i].w    = this->Mesh->Textures[i].w;
		ps->texture[i].h    = this->Mesh->Textures[i].h;
	}

	ps->tpage = (void*) this->textureidx;
	ps->matidx = this->matidx;

	strcpy(ps->name,this->fname);
	ps->skinID = STRING_GetHashValue(ps->name);

	ps->skin[SKIN_SOLID]	 = (int8*) this->SkinSolid;
	ps->skinsize[SKIN_SOLID] = this->SkinSolidSize;


	// Parented Meshes

	ps->nmesh = this->nParented;
	ps->mesh  = (MESH *)malloc(sizeof(MESH)*this->nParented);

	for(i=0;i<(int)ps->nmesh;i++)
	{
		// Find The Mesh

		ps->mesh[i].meshID = 0;

		for(j=0;j<this->Mesh->nMesh;j++)
		{
			unsigned long meshid;

			meshid = STRING_GetHashValue(this->Mesh->meshs[j].name);

			if(this->parentedobjs[i].meshhash == meshid)
			{
				Log("Parented : %d  = %s",i,this->Mesh->meshs[j].name);

				this->Mesh->meshs[j].ExportToGame(&ps->mesh[i]);

				strcpy(ps->mesh[i].meshName,this->Mesh->meshs[j].name);

				ps->mesh[i].meshID = meshid;

				ps->mesh[i].flags = this->parentedobjs[i].flags;

				mathCopyMatrix((MATRIX*)this->Mesh->meshs[j].matrix.m,&ps->mesh[i].tm);

				// Find Bone

				for(k=0;k<this->Skeleton->nBones;k++)
				{
					if(this->Skeleton->Bones[k].bonehash == this->parentedobjs[i].bonehash)
					{
						ps->mesh[i].bone = k; // Store Bone Number

						Log("---->> Bone : %s",this->Skeleton->Bones[k].name);

						ps->mesh[i].flags |= MESHFLAG_VISIBLE;

						break;
					}
				}

				break;
			}
		}
		
		if (ps->mesh[i].meshID==0)
		{
			Log("**** Error: %d",i);
		}
	}

	// Export The Materials To Game

	ps->nMaterial  = this->Mesh->nMaterials;
	ps->pMaterial = (MATERIAL*) malloc(sizeof(MATERIAL)*this->Mesh->nMaterials);

	for(i=0;i<ps->nMaterial;i++)
	{
		ps->pMaterial[i].flags = (this->Mesh->Materials[i].flags>>16) | (this->Mesh->Materials[i].flags&0xff);
		ps->pMaterial[i].tidx  = this->Mesh->Materials[i].TChannels[0].idx;
	}

	
	// Get Animation IDX prior To Sorting
	
	int idx = 0;

	for(i=0;i<(int)ps->nmesh;i++)
	{
		if(ps->mesh[i].flags&MESHFLAG_ANIMATED)
		{
			ps->mesh[i].animidx = idx;
			idx++;
		}
		else
			ps->mesh[i].animidx = -1;
	}


	// Sort The Mesh Based On Material

	int exit;
	do
	{
		exit = TRUE;

		for(i=1;i<(int)ps->nmesh;i++)
		{
			if(ps->pMaterial[ps->mesh[i].matlidx].flags < ps->pMaterial[ps->mesh[i-1].matlidx].flags)
			{
				MESH mesh;
				memcpy(&mesh,&ps->mesh[i],sizeof(MESH));
				memcpy(&ps->mesh[i],&ps->mesh[i-1],sizeof(MESH));
				memcpy(&ps->mesh[i-1],&mesh,sizeof(MESH));
				exit = FALSE;
			}
		}
	}
	while(!exit);


	for(i=0;i<(int)ps->nmesh;i++)
	{
		Log("%s",ps->mesh[i].meshName);
	}


	// Create Hierachys
	
	ps->nHierachy = this->nHierachy;
	
	ps->Hierachy  = (HIERACHY*) malloc(sizeof(HIERACHY)*this->nHierachy);

	idx = 0;


	for(k=0;k<this->nHierachy;k++)
	{
		int n;

		memset(&ps->Hierachy[k],0,sizeof(HIERACHY));

		ps->Hierachy[k].nNodes			= this->Hierachy[k].nBones;
		ps->Hierachy[k].flags			= STRING_GetHashValue((char*) (LPCSTR) this->Hierachy[k].fname);
		ps->Hierachy[k].nodes.nodeptr   = (NODE*) malloc(sizeof(NODE)*this->Hierachy[k].nBones);


		for(n=0;n<(int)ps->Hierachy[k].nNodes;n++)
		{
			ps->Hierachy[k].nodes.nodeptr[n].flags = this->Hierachy[k].Bones[n].pushpop;
			strcpy(ps->Hierachy[k].nodes.nodeptr[n].name,(LPCSTR)this->Hierachy[k].Bones[n].name);
		
/*
			char name[255];
			int id;
			CString sn = ps->Hierachy[k].nodes.nodeptr[n].name;		
			if(sn.Left(8) == "ANIMATE_")
			{
				sn = sn.Mid(8);
				strcpy(name,(LPCSTR)sn);
			}
			
			id = STRING_GetHashValue(name);

*/
			
			for(i=0;i<this->nParented;i++)
			{
				if(this->parentedobjs[i].meshhash == STRING_GetHashValue((char*)(LPCSTR)this->Hierachy[k].Bones[n].name))
				{
					idx = i;
					break;
				}
			}
					

			ps->Hierachy[k].nodes.nodeptr[n].boneID = STRING_GetHashValue(ps->Hierachy[k].nodes.nodeptr[n].name);

			
//			ps->Hierachy[k].nodes.nodeptr[n].boneID = id;



			mathCopyVector((VECTOR*)&this->parentedobjs[idx].rp,&ps->Hierachy[k].nodes.nodeptr[n].rotatePivot);
			mathCopyVector((VECTOR*)&this->parentedobjs[idx].rt,&ps->Hierachy[k].nodes.nodeptr[n].rotateTranslate);
			mathCopyVector((VECTOR*)&this->parentedobjs[idx].sp,&ps->Hierachy[k].nodes.nodeptr[n].scalePivot);
			mathCopyVector((VECTOR*)&this->parentedobjs[idx].st,&ps->Hierachy[k].nodes.nodeptr[n].scaleTranslate);
			CMatrix rom;
			rom = this->parentedobjs[idx].ro.ToMatrix();
			mathCopyMatrix((MATRIX*)&rom,&ps->Hierachy[k].nodes.nodeptr[n].rotateOrient);

			mathCopyVector((VECTOR*)&this->parentedobjs[idx].t,&ps->Hierachy[k].nodes.nodeptr[n].translate);
			mathCopyVector((VECTOR*)&this->parentedobjs[idx].s,&ps->Hierachy[k].nodes.nodeptr[n].scale);

			CMatrix m;
			m.RotZ(this->parentedobjs[idx].r.z);
			m.RotY(this->parentedobjs[idx].r.y);
			m.RotX(this->parentedobjs[idx].r.x);

			mathCopyVector((VECTOR*)&m.ToQuaternion().v,&ps->Hierachy[k].nodes.nodeptr[n].rotate);
		
		
			mathPushMatrix();
			mathLoadUnitMatrix();
			mathTranslate(&ps->Hierachy[k].nodes.nodeptr[n].rotateTranslate);
			mathTranslate(&ps->Hierachy[k].nodes.nodeptr[n].rotatePivot);
			mathStoreMatrix(&ps->Hierachy[k].nodes.nodeptr[n].pre);		
			mathLoadUnitMatrix();
			
			mathMulMatrix(&ps->Hierachy[k].nodes.nodeptr[n].rotateOrient);

			VECTOR v;

			mathCopyVector(&ps->Hierachy[k].nodes.nodeptr[n].rotatePivot,&v);
			v.x = -v.x;
			v.y = -v.y;
			v.z = -v.z;
			v.w = 1.0f;
			mathTranslate(&v);

			mathTranslate(&ps->Hierachy[k].nodes.nodeptr[n].scaleTranslate);
			mathTranslate(&ps->Hierachy[k].nodes.nodeptr[n].scalePivot);

			mathStoreMatrix(&ps->Hierachy[k].nodes.nodeptr[n].mid);
			
			
			mathLoadUnitMatrix();
					
			mathCopyVector(&ps->Hierachy[k].nodes.nodeptr[n].scalePivot,&v);
			v.x = -v.x;
			v.y = -v.y;
			v.z = -v.z;
			v.w = 1.0f;
			mathTranslate(&v);
			
			mathStoreMatrix(&ps->Hierachy[k].nodes.nodeptr[n].post);
				
			
			mathPopMatrix();
		
		}


		// Find First Parented Mesh In Hierachy And Store Bone Number
/*		
		for(n=0;n<ps->nmesh;n++)
		{
			if(ps->Hierachy[k].nodes.nodeptr[0].boneID == ps->mesh[n].meshID)
			{
				ps->Hierachy[k].bonemap = (unsigned long*) ps->mesh[n].bone;
			}
		}*/
	}	
}


//-----------------------------------------------------------------------------------------------------


void CBone::ExportToGame(void*pResult)
{
	CMatrix m;
	CVector v;
	BONE* pb;

	pb = (BONE*) pResult;

	pb->flags = pushpop | this->Flags;

	// Get Bone Name (Excluding Character Name)

	CString bonename;
	bonename = name.GetBuffer(32) + name.Find("_", 0)+1;
	BONE_SetName(pb,(char*)(LPCSTR)bonename);

	// Get Translation From BindMatrix

	m = bindmatrix();

	mathCopyMatrix((MATRIX*)&m,&pb->bind);
	mathCopyMatrix((MATRIX*)&iabsbindmatrix(),&pb->iabsbind);
	mathCopyMatrix((MATRIX*)&pre(),&pb->pre);
	mathCopyMatrix((MATRIX*)&post(),&pb->post);

	pb->sphere.radius = this->sphere.radius;
	pb->sphere.origin.x = this->sphere.position.x;
	pb->sphere.origin.y = this->sphere.position.y;
	pb->sphere.origin.z = this->sphere.position.z;
	pb->sphere.origin.w = 1.0f;
}



//----------------------------------------------------------------------------------------------------


void CHierachy::ExportToGame(void**pResult)
{
	HIERACHY* phr;
	BONE* pBoneList;
	int* pBoneMirror;
	int i;

	phr = (HIERACHY *)malloc(sizeof(HIERACHY));

	phr->flags = this->flags;

	// Export All The Bones

	pBoneList = BONE_CreateList(nBones);

	for(i=0;i<nBones;i++)
		Bones[i].ExportToGame((void*)&pBoneList[i]);

	// Create Bone Mirror Map

	pBoneMirror = BONE_CreateMirrorMap(nBones,pBoneList);

	// Now Intialise The Hierachy

	HR_Initialise(phr,nBones,pBoneList,pBoneMirror);


	*(pResult) = phr;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

/*
	Function	:	E2G_Hierachy
	Useage		:	Export Hierachy To Game
*/

ANIMOBJECT* E2G_Hierachy(CHierachy* hierachy)
{
	ANIMOBJECT* pao = NULL;
	HIERACHY*   phr = NULL;

	pao = (ANIMOBJECT *)malloc(sizeof(ANIMOBJECT));
	memset(pao, 0, sizeof(ANIMOBJECT));

	AO_Initialise(pao);

	hierachy->ExportToGame((void**)&phr);

	pao->hierachy = phr;

	pao->skin = NULL;
	pao->nskin = 0;
	pao->Morph.Sequence = NULL;

	APB_Initialise(pao);
	APB_InitBoneFlags(pao->apb,phr);


	// Setup The Frame Functions

	APB_SetFunctions(pao->apb,&SkinCharBeginFrame,&SkinCharProcessBone,&SkinCharEndFrame);

	FXHandler = fxAnimTriggerHandler;

	return pao;
}









void CPropHierachy::ExportToGame(void**pResult)
{
	HIERACHY* phr;
	NODE* pBoneList;
	int i;

	phr = (HIERACHY *)malloc(sizeof(HIERACHY));

	phr->flags = this->flags;

	// Export All The Bones

	pBoneList = (NODE*) malloc(sizeof(NODE)*this->nBones);

	for(i=0;i<nBones;i++)
	{
		pBoneList[i].flags = this->Bones[i].Flags;
		strcpy(pBoneList[i].name,this->Bones[i].name);
		pBoneList[i].boneID = STRING_GetHashValue((char*) (LPCSTR) this->Bones[i].name);
		mathCopyVector((VECTOR*)&this->Bones[i].rotatePivot,&pBoneList[i].rotatePivot);
		mathCopyVector((VECTOR*)&this->Bones[i].scalePivot,&pBoneList[i].scalePivot);
		mathCopyVector((VECTOR*)&this->Bones[i].rotateTranslate,&pBoneList[i].rotateTranslate);
		mathCopyVector((VECTOR*)&this->Bones[i].scaleTranslate,&pBoneList[i].scaleTranslate);
		mathCopyMatrix((MATRIX*)&this->Bones[i].rotateOrient,&pBoneList[i].rotateOrient);

		mathPushMatrix();
		mathLoadUnitMatrix();
		mathTranslate(&pBoneList[i].rotateTranslate);
		mathTranslate(&pBoneList[i].rotatePivot);
		mathStoreMatrix(&pBoneList[i].pre);		
		mathLoadUnitMatrix();
		
		mathMulMatrix(&pBoneList[i].rotateOrient);

		VECTOR v;

		mathCopyVector(&pBoneList[i].rotatePivot,&v);
		v.x = -v.x;
		v.y = -v.y;
		v.z = -v.z;
		v.w = 1.0f;
		mathTranslate(&v);

		mathTranslate(&pBoneList[i].scaleTranslate);
		mathTranslate(&pBoneList[i].scalePivot);

		mathStoreMatrix(&pBoneList[i].mid);
		
		
		mathLoadUnitMatrix();
				
		mathCopyVector(&pBoneList[i].scalePivot,&v);
		v.x = -v.x;
		v.y = -v.y;
		v.z = -v.z;
		v.w = 1.0f;
		mathTranslate(&v);
		
		mathStoreMatrix(&pBoneList[i].post);
			
		
		mathPopMatrix();
	
	
	
	
	
	
	
	
	
	
	
	
	
	}

	// Now Intialise The Hierachy

	HR_SetInitialise(phr,nBones,pBoneList);

	*(pResult) = phr;
}






/*
	Function	:	E2G_SetHierachy
	Useage		:	Export Set Hierachy To Game
*/

ANIMOBJECT* E2G_SetHierachy(CPropHierachy* hierachy)
{
	ANIMOBJECT* pao = NULL;
	HIERACHY*   phr = NULL;

	pao = (ANIMOBJECT *)malloc(sizeof(ANIMOBJECT));
	memset(pao, 0, sizeof(ANIMOBJECT));

	AO_Initialise(pao);

	hierachy->ExportToGame((void**)&phr);

	pao->hierachy = phr;

	pao->skin = NULL;
	pao->nskin = 0;
	pao->Morph.Sequence = NULL;
	pao->apb = (ANIMATION_PLAYBACK *)malloc(sizeof(ANIMATION_PLAYBACK));

/*
	// Export The Meshes To The Skin Section

	SKIN* ps;

	ps = new SKIN;

	pao->skin  = (SKIN**) malloc(sizeof(SKIN*));
	pao->nskin = 1;
	pao->skin[0] = ps;
	pao->skin[0]->skindata = NULL;


	ps->nmesh = hierachy->ds->mn_NodeCount;

	ps->mesh  = (MESH*) malloc(sizeof(MESH) * hierachy->ds->mn_NodeCount);


	for(int i=0;i<ps->nmesh;i++)
	{
		hierachy->ds->mp_Mesh->meshs[i].ExportToGame(&ps->mesh[i]);
		strcpy(ps->mesh[i].meshName,hierachy->ds->mp_Mesh->meshs[i].shapename);
		ps->mesh[i].meshID = STRING_GetHashValue((char*)(LPCSTR)hierachy->ds->mp_Mesh->meshs[i].shapename);
	}

	pao->selectedskin = ps;
*/

	APB_Initialise(pao);

	pao->apb->flags |= APBFLAG_ANIMATEDSET;

	// Setup The Frame Functions

	APB_SetFunctions(pao->apb,&SetBeginFrame,&SetProcessNode,&SetEndFrame);

	CAnimationEditorDoc* doc;

	doc = GetDocument();

	APB_InitBoneFlags(pao->apb,phr);

	AO_InitMeshFlags(pao);


/*
	ANIMATION* anim;

	anim = AL_FindAnimation(pAPB_CurrentAnimationList,STRING_GetHashValue((char*)(LPCSTR)hierachy->ds->fname));

	anim->flags |= APBFLAG_LOOP;


	APB_PlayAnimation(pao->apb,STRING_GetHashValue((char*)(LPCSTR)hierachy->ds->fname),0,0);
*/
	return pao;
}





/*
	Function	:	E2G_Animations
	Useage		:	Send Animation To The Game Animation List
*/


void E2G_Animation(CAnimation* animation)
{
	ANIMATION* anim = NULL;
	int i;

	if(!GetDocument()->GetCharacter()->pAnimationList)
	{
		GetDocument()->GetCharacter()->pAnimationList = (ANIMATION_LIST *)malloc(sizeof(ANIMATION_LIST));

		AL_Initialise(GetDocument()->GetCharacter()->pAnimationList,0);
	}

	animation->ExportToGame((void**)&anim);

	// Export The Velocity Track

	anim->data->vtrack.animblock = (uint8**) malloc(sizeof(uint8*)*AT_TOTAL);
	anim->data->vtrack.animsize =  (uint32*) malloc(sizeof(uint32)*AT_TOTAL);


	for(i=0;i<AT_TOTAL;i++)
	{
		anim->data->vtrack.flags        = animation->VelocityTrack.flag;
		anim->data->vtrack.animblock[i] = animation->VelocityTrack.animblock[i];
		anim->data->vtrack.animsize[i]  = animation->VelocityTrack.animsize[i];
	}

	// Initialise Blend Ranges

	anim->nBlendRanges = 0;
	anim->BlendRanges  = NULL;

	// Add Single Animation

	AL_AddAnimation(GetDocument()->GetCharacter()->pAnimationList,anim);

	// Sort List

	AL_SortAnimationList(GetDocument()->GetCharacter()->pAnimationList);

	// Set The Animation List

	AL_SetAnimationList(GetDocument()->GetCharacter()->animObject,GetDocument()->GetCharacter()->pAnimationList);

	animation->pGameData  = anim;

//	mathCopyVector(&animation->errorCorrection,&anim->errorCorrection);
}



void E2G_Skin(ANIMOBJECT* pao,CSkin* skin)
{
	SKIN* ps;

	ps = (SKIN*) malloc(sizeof(SKIN));

	skin->ExportToGame((void*)ps);

	pao->nskin++;
	pao->skin = (SKIN**) memrealloc(pao->skin,pao->nskin*sizeof(SKIN*));
	pao->skin[pao->nskin-1] = ps;

	pao->skin[pao->nskin-1]->skin[SKIN_SOLID] = (int8*) skin->SkinSolid;
	pao->skin[pao->nskin-1]->skinsize[SKIN_SOLID] = skin->SkinSolidSize;

	pao->skin[pao->nskin-1]->skin[SKIN_CADD] = (int8*) skin->SkinColorAdd;
	pao->skin[pao->nskin-1]->skinsize[SKIN_CADD] = skin->SkinColorAddSize;

	pao->skin[pao->nskin-1]->skin[SKIN_CSUB] = (int8*) skin->SkinColorSub;
	pao->skin[pao->nskin-1]->skinsize[SKIN_CSUB] = skin->SkinColorSubSize;


//	ClearMorphOnSkin(ps);
}


#if 0
void E2G_MorphTarget(ANIMOBJECT* pao, CMorphTarget *target)
{
	int i;

	MORPH_TARGET* gamemorphtarget = NULL;

	target->ExportToGame(&gamemorphtarget);
}
#endif


//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

volatile int8* g_skindata;			// Global Skin Data Ptr

VECTOR Light;


float verts[MAX_VERTS];
float everts[MAX_VERTS];



void DrawExtraSkinBits(int DrawFlags)
{
	int i;
	float* pv;
	pv = everts;

	int nStrips;
	int k;

	glLoadIdentity();

	ReadShort(nStrips,g_skindata);

	int nTotalEverts = 0;

	for(k=0;k<nStrips;k++)
	{
		int nverts;

		ReadShort(nverts,g_skindata);

		glBegin(GL_TRIANGLE_STRIP);

		float r,g,b;

		r = g = b = 1.0f;

		if(DrawFlags&GLVIEWFLAG_SHOWSTRIPS)
		{
			r = (rand()&255)/255.0f;
			g = (rand()&255)/255.0f;
			b = (rand()&255)/255.0f;
		}

		for(i=0;i<nverts;i++)
		{
			int a;
			float u,v;

			ReadShort(a,g_skindata);
			ReadWait(u,g_skindata);
			ReadWait(v,g_skindata);

			glTexCoord2f(u,v);
			glColor3f(r*pv[a+4],g*pv[a+5],b*pv[a+6]);
			glVertex4f(pv[a+0],pv[a+1],pv[a+2],pv[a+3]);
		}


		glEnd();

		nTotalEverts += nverts;
	}


}




void DrawExtraSkinBitsEnvMap(int DrawFlags)
{
	int i;
	float* pv;
	pv = everts;

	int nStrips;
	int k;

	glLoadIdentity();

	ReadShort(nStrips,g_skindata);

	int nTotalEverts = 0;

	for(k=0;k<nStrips;k++)
	{
		int nverts;

		ReadShort(nverts,g_skindata);

		glBegin(GL_TRIANGLE_STRIP);

		float r,g,b;

		r = g = b = 1.0f;

		if(DrawFlags&GLVIEWFLAG_SHOWSTRIPS)
		{
			r = (rand()&255)/255.0f;
			g = (rand()&255)/255.0f;
			b = (rand()&255)/255.0f;
		}

		for(i=0;i<nverts;i++)
		{
			int a;
			float u,v;

			ReadShort(a,g_skindata);
			ReadWait(u,g_skindata);
			ReadWait(v,g_skindata);

			glTexCoord2f(pv[a+4],pv[a+5]);
			glColor3f(pv[a+6],pv[a+6],pv[a+6]);
			glVertex4f(pv[a+0],pv[a+1],pv[a+2],pv[a+3]);
		}


		glEnd();

		nTotalEverts += nverts;
	}


}




void DrawSkinBonePair(MATRIX *prev,MATRIX *current,BONE* cbone,BONE* pbone,int DrawFlags)
{
	int nStrips;
	int i,k;
	float pad;
	int uwoff = 0;
	int	woff = 0;
	float dota,dotb;
	float* pv  = verts;

	VECTOR lv,lvp;

	glLoadIdentity();

	mathPushMatrix();

	mathLoadMatrix(current);
	mathMulMatrix(&cbone->iabsbind);
	mathMulMatrixPost(&gmathInversePerspMatrix);
	mathMulMatrixPost(&gmathInverseCameraMatrix);
	mathTransposeMatrix();
	mathRot(&Light,&lv);

	mathLoadMatrix(prev);

	mathMulMatrix(&pbone->iabsbind);
	mathMulMatrixPost(&gmathInversePerspMatrix);
	mathMulMatrixPost(&gmathInverseCameraMatrix);
	mathTransposeMatrix();

	mathRot(&Light,&lvp);

	mathVectorNormalise(&lv,&lv);
	mathVectorNormalise(&lvp,&lvp);

	mathPopMatrix();

	int cnt;

	// First Transform All Verts & Normals With No Weights

	ReadShort(cnt,g_skindata);

	for(i=0;i<cnt;i++)
	{
		VECTOR v;

		// Verts

		ReadVert(v.x,g_skindata);
		ReadVert(v.y,g_skindata);
		ReadVert(v.z,g_skindata);

		mathRotTrans2(current,&v,&v);

		verts[uwoff+0] = v.x;
		verts[uwoff+1] = v.y;
		verts[uwoff+2] = v.z;
		verts[uwoff+3] = v.w;

		// Normals

		ReadNorm(v.x,g_skindata);
		ReadNorm(v.y,g_skindata);
		ReadNorm(v.z,g_skindata);
		ReadNorm(pad,g_skindata);

		dota = mathVectorDot(&lv,&v);

		if(dota < 0)
			dota = 0;

		if(!(DrawFlags&GLVIEWFLAG_SHOWLIGHT))
			dota = 1.0f;

		if(DrawFlags&GLVIEWFLAG_SHOWSEGMENTS)
		{
			verts[uwoff+4] = 0;
			verts[uwoff+5] = dota;
			verts[uwoff+6] = 0;
		}
		else
			verts[uwoff+4] = verts[uwoff+5] = verts[uwoff+6] = dota;

		uwoff+=8;
	}

	// unweighted mesh

	ReadShort(nStrips,g_skindata);

	for(k=0;k<nStrips;k++)
	{
		int nverts;

		ReadShort(nverts,g_skindata);

		glBegin(GL_TRIANGLE_STRIP);

		float r,g,b;

		r = g = b = 1.0f;

		if(DrawFlags&GLVIEWFLAG_SHOWSTRIPS)
		{
			r = (rand()&255)/255.0f;
			g = (rand()&255)/255.0f;
			b = (rand()&255)/255.0f;
		}

		for(i=0;i<nverts;i++)
		{
			int a;
			float u,v;

			ReadShort(a,g_skindata);
			ReadWait(u,g_skindata);
			ReadWait(v,g_skindata);

			glTexCoord2f(u,v);

			glColor3f(r*pv[a+4],g*pv[a+5],b*pv[a+6]);

			glVertex4f(pv[a+0],pv[a+1],pv[a+2],pv[a+3]);

		}

		glEnd();
	}

	// Now Transform All Weighted Verts & Normals On This Bone

	ReadShort(cnt,g_skindata);

	for(i=0;i<cnt;i++)
	{
		VECTOR v1,v2;
		float w1,w2;

		ReadVert(v1.x,g_skindata);
		ReadVert(v1.y,g_skindata);
		ReadVert(v1.z,g_skindata);
		ReadWait(w1,g_skindata);

		ReadVert(v2.x,g_skindata);
		ReadVert(v2.y,g_skindata);
		ReadVert(v2.z,g_skindata);
		ReadWait(w2,g_skindata);


		mathRotTrans2(prev,&v1,&v1);
		mathRotTrans2(current,&v2,&v2);


		verts[woff+0] = (v1.x * w1) + (v2.x * w2);
		verts[woff+1] = (v1.y * w1) + (v2.y * w2);
		verts[woff+2] = (v1.z * w1) + (v2.z * w2);
		verts[woff+3] = (v1.w * w1) + (v2.w * w2);

		ReadNorm(v1.x,g_skindata);
		ReadNorm(v1.y,g_skindata);
		ReadNorm(v1.z,g_skindata);
		ReadNorm(pad,g_skindata);

		dota = mathVectorDot(&lv,&v1);
		dota *= w2;

		dotb = mathVectorDot(&lvp,&v1);
		dotb *= w1;

		if(dota < 0) dota = 0;
		if(dotb < 0) dotb = 0;

		if(!(DrawFlags&GLVIEWFLAG_SHOWLIGHT))
			dota = dotb = 0.5f;

		if(DrawFlags&GLVIEWFLAG_SHOWSEGMENTS)
		{
			verts[woff+4] = 0;
			verts[woff+5] = 0;
			verts[woff+6] = dota+dotb;
		}
		else
			verts[woff+4] = verts[woff+5] = verts[woff+6] = dota+dotb;


		woff+=8;
	}

	if(woff > MAX_VERTS)
		woff = woff;

	// weighted mesh

	ReadShort(nStrips,g_skindata);

	for(k=0;k<nStrips;k++)
	{
		int nverts;

		ReadShort(nverts,g_skindata);

		glBegin(GL_TRIANGLE_STRIP);

		float r,g,b;

		r = g = b = 1.0f;

		if(DrawFlags&GLVIEWFLAG_SHOWSTRIPS)
		{
			r = (rand()&255)/255.0f;
			g = (rand()&255)/255.0f;
			b = (rand()&255)/255.0f;
		}

		for(i=0;i<nverts;i++)
		{
			int a;
			float u,v;

			ReadShort(a,g_skindata);
			ReadWait(u,g_skindata);
			ReadWait(v,g_skindata);
			glTexCoord2f(u,v);
			glColor3f(r*pv[a+4],g*pv[a+5],b*pv[a+6]);
			glVertex4f(pv[a+0],pv[a+1],pv[a+2],pv[a+3]);
		}

		glEnd();
	}

	ReadShort(cnt,g_skindata);

	// Transform any Extra Verts

	for(i=0;i<cnt;i++)
	{
		VECTOR v1;
		float w1;
		short fidx;

		ReadVert(v1.x,g_skindata);
		ReadVert(v1.y,g_skindata);
		ReadVert(v1.z,g_skindata);
		ReadWait(w1,g_skindata);
		ReadShort(fidx,g_skindata);

		mathRotTrans2(current,&v1,&v1);

		everts[fidx+0] += (v1.x * w1);
		everts[fidx+1] += (v1.y * w1);
		everts[fidx+2] += (v1.z * w1);
		everts[fidx+3] += (v1.w * w1);

		if(fidx > MAX_VERTS)
			fidx = fidx;

		ReadNorm(v1.x,g_skindata);
		ReadNorm(v1.y,g_skindata);
		ReadNorm(v1.z,g_skindata);
		ReadNorm(pad,g_skindata);

		dota = mathVectorDot(&lv,&v1);
		dota *= w1;


		if(!(DrawFlags&GLVIEWFLAG_SHOWLIGHT))
			dota = 1.0f;

		if(dota < 0) dota = 0;

		if(DrawFlags&GLVIEWFLAG_SHOWSEGMENTS)
		{
			everts[fidx+4] += dota;
			everts[fidx+5] = 0;
			everts[fidx+6] = 0;
		}
		else
		{
			everts[fidx+4] += dota;
			everts[fidx+5] += dota;
			everts[fidx+6] += dota;
		}
	}
}





void DrawSkinBonePairEnvMap(MATRIX *prev,MATRIX *current,BONE* cbone,BONE* pbone,int DrawFlags)
{
	int nStrips;
	int i,k;
	float pad;
	int uwoff = 0;
	int	woff = 0;
	float* pv  = verts;

	VECTOR lv,lvp;

	glLoadIdentity();

	mathPushMatrix();

	mathLoadMatrix(current);
	mathMulMatrix(&cbone->iabsbind);
	mathMulMatrixPost(&gmathInversePerspMatrix);
	mathMulMatrixPost(&gmathInverseCameraMatrix);
	mathTransposeMatrix();
	mathRot(&Light,&lv);

	mathLoadMatrix(prev);

	mathMulMatrix(&pbone->iabsbind);
	mathMulMatrixPost(&gmathInversePerspMatrix);
	mathMulMatrixPost(&gmathInverseCameraMatrix);
	mathTransposeMatrix();

	mathRot(&Light,&lvp);

	mathVectorNormalise(&lv,&lv);
	mathVectorNormalise(&lvp,&lvp);

	mathPopMatrix();

	int cnt;

	// First Transform All Verts & Normals With No Weights

	ReadShort(cnt,g_skindata);

	for(i=0;i<cnt;i++)
	{
		VECTOR v;

		// Verts

		ReadVert(v.x,g_skindata);
		ReadVert(v.y,g_skindata);
		ReadVert(v.z,g_skindata);

		mathRotTrans2(current,&v,&v);

		verts[uwoff+0] = v.x;
		verts[uwoff+1] = v.y;
		verts[uwoff+2] = v.z;
		verts[uwoff+3] = v.w;

		// Normals

		ReadNorm(v.x,g_skindata);
		ReadNorm(v.y,g_skindata);
		ReadNorm(v.z,g_skindata);
		ReadNorm(pad,g_skindata);

		mathRot(&v,&v);
		
		verts[uwoff+4] = v.x;	// uv
		verts[uwoff+5] = v.y;	
		verts[uwoff+6] = 1.0f;	// Strength
		
		uwoff+=8;
	}

	// unweighted mesh

	ReadShort(nStrips,g_skindata);

	for(k=0;k<nStrips;k++)
	{
		int nverts;

		ReadShort(nverts,g_skindata);

		glBegin(GL_TRIANGLE_STRIP);

		float r,g,b;

		r = g = b = 1.0f;

		if(DrawFlags&GLVIEWFLAG_SHOWSTRIPS)
		{
			r = (rand()&255)/255.0f;
			g = (rand()&255)/255.0f;
			b = (rand()&255)/255.0f;
		}

		for(i=0;i<nverts;i++)
		{
			int a;
			float u,v;

			ReadShort(a,g_skindata);
			ReadWait(u,g_skindata);
			ReadWait(v,g_skindata);
		
			
			glTexCoord2f(pv[a+4],pv[a+5]);

			glColor3f(pv[a+6],pv[a+6],pv[a+6]);

			glVertex4f(pv[a+0],pv[a+1],pv[a+2],pv[a+3]);

		}

		glEnd();
	}

	// Now Transform All Weighted Verts & Normals On This Bone

	ReadShort(cnt,g_skindata);

	for(i=0;i<cnt;i++)
	{
		VECTOR v1,v2;
		float w1,w2;

		ReadVert(v1.x,g_skindata);
		ReadVert(v1.y,g_skindata);
		ReadVert(v1.z,g_skindata);
		ReadWait(w1,g_skindata);

		ReadVert(v2.x,g_skindata);
		ReadVert(v2.y,g_skindata);
		ReadVert(v2.z,g_skindata);
		ReadWait(w2,g_skindata);


		mathRotTrans2(prev,&v1,&v1);
		mathRotTrans2(current,&v2,&v2);


		verts[woff+0] = (v1.x * w1) + (v2.x * w2);
		verts[woff+1] = (v1.y * w1) + (v2.y * w2);
		verts[woff+2] = (v1.z * w1) + (v2.z * w2);
		verts[woff+3] = (v1.w * w1) + (v2.w * w2);

		ReadNorm(v1.x,g_skindata);
		ReadNorm(v1.y,g_skindata);
		ReadNorm(v1.z,g_skindata);
		ReadNorm(pad,g_skindata);


		mathRot(&v1,&v1);

		verts[woff+4] = v1.x;
		verts[woff+4] = v1.y;
		verts[woff+4] = 1.0f;
		
		woff+=8;
	}

	if(woff > MAX_VERTS)
		woff = woff;

	// weighted mesh

	ReadShort(nStrips,g_skindata);

	for(k=0;k<nStrips;k++)
	{
		int nverts;

		ReadShort(nverts,g_skindata);

		glBegin(GL_TRIANGLE_STRIP);

		float r,g,b;

		r = g = b = 1.0f;

		if(DrawFlags&GLVIEWFLAG_SHOWSTRIPS)
		{
			r = (rand()&255)/255.0f;
			g = (rand()&255)/255.0f;
			b = (rand()&255)/255.0f;
		}

		for(i=0;i<nverts;i++)
		{
			int a;
			float u,v;

			ReadShort(a,g_skindata);
			ReadWait(u,g_skindata);
			ReadWait(v,g_skindata);
						
			glTexCoord2f(pv[a+4],pv[a+5]);
			glColor3f(pv[a+6],pv[a+6],pv[a+6]);
			glVertex4f(pv[a+0],pv[a+1],pv[a+2],pv[a+3]);
		}

		glEnd();
	}

	ReadShort(cnt,g_skindata);

	// Transform any Extra Verts

	for(i=0;i<cnt;i++)
	{
		VECTOR v1;
		float w1;
		short fidx;

		ReadVert(v1.x,g_skindata);
		ReadVert(v1.y,g_skindata);
		ReadVert(v1.z,g_skindata);
		ReadWait(w1,g_skindata);
		ReadShort(fidx,g_skindata);

		mathRotTrans2(current,&v1,&v1);

		everts[fidx+0] += (v1.x * w1);
		everts[fidx+1] += (v1.y * w1);
		everts[fidx+2] += (v1.z * w1);
		everts[fidx+3] += (v1.w * w1);

		if(fidx > MAX_VERTS)
			fidx = fidx;

		ReadNorm(v1.x,g_skindata);
		ReadNorm(v1.y,g_skindata);
		ReadNorm(v1.z,g_skindata);
		ReadNorm(pad,g_skindata);

		mathRot(&v1,&v1);

		everts[fidx+4] = v1.x;
		everts[fidx+5] = v1.y;
		everts[fidx+6] = 1.0f;
	}
}


void DrawWireBone(MATRIX* previousMatrix,MATRIX* currentMatrix)
{
	VECTOR a,b;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_TEXTURE_2D);


	mathPushMatrix();
	mathLoadMatrix(previousMatrix);

	VECTOR p1,p2,p3,p4;
	VECTOR v1,v2,v3,up;

	up.x = 0;
	up.y = 0;
	up.z = 1.0f;

	v1.x = previousMatrix->m02;
	v1.y = previousMatrix->m12;
	v1.z = previousMatrix->m22;

	mathVectorCross(&up,&v1,&v2);
	mathVectorCross(&v2,&v1,&v3);

	mathVectorNormalise(&v2,&v2);
	mathVectorNormalise(&v3,&v3);

	mathVectorScale(&v3,-2.0f,&a);
	mathRotTrans(&a,&p1);

	mathVectorScale(&v2,-2.0f,&a);
	mathRotTrans(&a,&p2);

	mathVectorScale(&v3,2.0f,&a);
	mathRotTrans(&a,&p3);

	mathVectorScale(&v2,2.0f,&a);
	mathRotTrans(&a,&p4);

	mathLoadMatrix(currentMatrix);
	b.x = b.y = b.z = 0; b.w = 1.0f;
	mathRotTrans(&b,&b);
	mathPopMatrix();

	glLoadIdentity();
	glBegin(GL_LINES);
	glColor3f(0,0,0.5f);

	glVertex4f(p1.x,p1.y,p1.z,p1.w);
	glVertex4f(b.x,b.y,b.z,b.w);

	glVertex4f(p2.x,p2.y,p2.z,p2.w);
	glVertex4f(b.x,b.y,b.z,b.w);

	glVertex4f(p3.x,p3.y,p3.z,p3.w);
	glVertex4f(b.x,b.y,b.z,b.w);

	glVertex4f(p4.x,p4.y,p4.z,p4.w);
	glVertex4f(b.x,b.y,b.z,b.w);

	glVertex4f(p1.x,p1.y,p1.z,p1.w);
	glVertex4f(p2.x,p2.y,p2.z,p2.w);

	glVertex4f(p2.x,p2.y,p2.z,p2.w);
	glVertex4f(p3.x,p3.y,p3.z,p3.w);

	glVertex4f(p3.x,p3.y,p3.z,p3.w);
	glVertex4f(p4.x,p4.y,p4.z,p4.w);

	glVertex4f(p4.x,p4.y,p4.z,p4.w);
	glVertex4f(p1.x,p1.y,p1.z,p1.w);

	glEnd();

	glPopAttrib();
}




void SkinCharBeginFrame(ANIMOBJECT* pao)
{
	ANIMOBJECT_EXT* m_ext;

	m_ext = (ANIMOBJECT_EXT*) pao->userdata;

	int DrawFlags = m_ext->DrawFlags;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_TEXTURE_2D);
	VECTOR a;
	glLoadIdentity();
	glBegin(GL_LINES);
	glColor3f(0,1.0f,0);
	a.x = -50.0f; a.y = 0.0f; a.z = 0.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
	a.x =  50.0f; a.y = 0.0f; a.z = 0.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
	a.x = 0.0f; a.y = -50.0f; a.z = 0.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
	a.x = 0.0f; a.y =  50.0f; a.z = 0.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
	a.x = 0.0f; a.y = 0.0f; a.z = -50.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
	a.x = 0.0f; a.y = 0.0f; a.z =  50.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
	glEnd();

	glPopAttrib();

	// Clear The Extra Vert Buffer


	// Initialise Data Ptr


	glPushAttrib(GL_ENABLE_BIT);

	if(m_ext->TextureID >= 0)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,m_ext->TextureID);
	}


	if(DrawFlags&GLVIEWFLAG_SHOWSEGMENTS || DrawFlags&GLVIEWFLAG_SHOWSTRIPS)
	{
		glDisable(GL_TEXTURE_2D);
	}

	srand(256);

//	glEnable(GL_CULL_FACE);


	if(DrawFlags&GLVIEWFLAG_DRAWWIREFRAME)
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

	nMatrices = 0;

}







void SkinCharProcessBone(ANIMOBJECT* pao,BONE* bone,MATRIX* previousMatrix,MATRIX* currentMatrix, int iBone, int iParentBone)
{
	ANIMOBJECT_EXT* m_ext;

	m_ext = (ANIMOBJECT_EXT*) pao->userdata;

	if(m_ext->DrawFlags&GLVIEWFLAG_DRAWAXIS)
		DrawAxis();

	mathCopyMatrix(currentMatrix,&Matrices[nMatrices++]);

	if(bone->boneID == STRING_GetHashValue("HIP"))
	{
		VECTOR hippos;
		mathUnitVector(&hippos);
		mathPushMatrix();
		mathLoadMatrix(currentMatrix);
		mathMulMatrixPost(&gmathInversePerspMatrix);
		mathMulMatrixPost(&gmathInverseCameraMatrix);
		mathRotTrans(&hippos,&hippos);

		m_ext->ctarget->z = hippos.z;

		mathPopMatrix();
	}
}




void DrawHierachy(ANIMOBJECT* pao)
{
	MATRIX previousMatrix;
	MATRIX currentMatrix;

	ANIMOBJECT_EXT* m_ext;
	m_ext = (ANIMOBJECT_EXT*) pao->userdata;

	unsigned int i;

	mathPushMatrix();

	for(i=0;i<pao->hierachy->nNodes;i++)
	{
		if(pao->hierachy->nodes.boneptr[i].flags&BONEFLAG_MATRIXPOP)
		{
			mathPopMatrix();
			mathStoreMatrix(&previousMatrix);
		}

		if(pao->hierachy->nodes.boneptr[i].flags&BONEFLAG_MATRIXPUSH)
			mathPushMatrix();

		if(i==0)
			mathStoreMatrix(&previousMatrix);

		mathLoadMatrix(&Matrices[i]);
		mathCopyMatrix(mathCurrentMatrix(),&currentMatrix);

		if(i)
			DrawWireBone(&previousMatrix,&currentMatrix);

		mathStoreMatrix(&previousMatrix);
	}

	mathPopMatrix();

}



void DrawFXNodes2(ANIMOBJECT* ao)
{
	uint32 i;
	
	// Draw FX Nodes

	if(ao->apb->animation.source.anim)
	{
		for (i=0; i<ao->apb->animation.source.anim->nFXNodes; i++)
		{
			FXNODE* pFX = &ao->apb->animation.source.anim->FXNodes[i];

			if(pFX->flags&FXNODE_ACTIVE)
				DrawFXNodes(ao,pFX,0);
		}
	}
		
}



void DrawFXNodes(ANIMOBJECT* ao,FXNODE* node,uint32 flags)
{
	FXNODE* pFX = node;

	mathPushMatrix();

	uint32 bone;
	
	VECTOR t,r;
	mathCopyVector(&pFX->translate,&t);
	mathCopyVector(&pFX->rotate,&r);

	MATRIX m;
	
	if(ao->apb->animation.source.anim->flags&APBFLAG_MIRROR)
	{
		VECTOR q;

		mathPushMatrix();
		mathLoadUnitMatrix();
		mathRotZ(r.z);			
		mathRotY(r.y);			
		mathRotX(r.x);			
		mathStoreMatrix(&m);	
		mathPopMatrix();

		mathMatrixToQuat(&m,&q);
		q.x = -q.x;
		q.w = -q.w;
		mathQuatToMatrix(&q,&m);
						
		bone = (pFX->boneID>>16)&0xffff;
		t.x = -t.x;
	}	
	else
	{
		bone = pFX->boneID&0xffff;
	
		mathPushMatrix();
		mathLoadUnitMatrix();
		mathRotZ(r.z);			
		mathRotY(r.y);			
		mathRotX(r.x);			
		mathStoreMatrix(&m);	
		mathPopMatrix();
	}

	
	mathLoadMatrix(&Matrices[bone]);
	mathTranslate(&t);
	mathMulMatrix(&m);
		
	mathScalef(pFX->scale.x,pFX->scale.y,pFX->scale.z);


//	LoadGLMatrix();

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_TEXTURE_2D);
	VECTOR a;
	glLoadIdentity();
	glBegin(GL_LINES);
	glColor3f(1.0f,0.0f,0);
	a.x = -15.0f; a.y = 0.0f; a.z = 0.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
	a.x =  15.0f; a.y = 0.0f; a.z = 0.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);

	glColor3f(0.0f,1.0f,0);
	a.x = 0.0f; a.y = -15.0f; a.z = 0.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
	a.x = 0.0f; a.y =  15.0f; a.z = 0.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
	
	glColor3f(0.0f,0.0f,1.0);
	a.x = 0.0f; a.y = 0.0f; a.z = 0.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
	a.x = 0.0f; a.y = 0.0f; a.z =  15.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
	glEnd();
	
	glPopAttrib();
	mathPopMatrix();
}




extern void glPutGroundPlane(int,int);


void DrawBox(VECTOR*min,VECTOR*max)
{
	glBegin(GL_LINES);
	glVertex3f(min->x,min->y,min->z);
	glVertex3f(max->x,min->y,min->z);

	glVertex3f(max->x,min->y,min->z);
	glVertex3f(max->x,max->y,min->z);

	glVertex3f(max->x,max->y,min->z);
	glVertex3f(min->x,max->y,min->z);

	glVertex3f(min->x,max->y,min->z);
	glVertex3f(min->x,min->y,min->z);


	glVertex3f(min->x,min->y,max->z);
	glVertex3f(max->x,min->y,max->z);

	glVertex3f(max->x,min->y,max->z);
	glVertex3f(max->x,max->y,max->z);

	glVertex3f(max->x,max->y,max->z);
	glVertex3f(min->x,max->y,max->z);

	glVertex3f(min->x,max->y,max->z);
	glVertex3f(min->x,min->y,max->z);


	glVertex3f(min->x,min->y,min->z);
	glVertex3f(min->x,min->y,max->z);

	glVertex3f(max->x,min->y,min->z);
	glVertex3f(max->x,min->y,max->z);


	glVertex3f(max->x,max->y,min->z);
	glVertex3f(max->x,max->y,max->z);

	glVertex3f(min->x,max->y,min->z);
	glVertex3f(min->x,max->y,max->z);

	glEnd();
}














enum
{
	BBXMIN = 0,
	BBXMAX,

	BBYMIN,
	BBYMAX,

	BBZMIN,
	BBZMAX
};



void DrawCollisionSpheres(ANIMOBJECT* pao)
{
	MATRIX previousMatrix;
	MATRIX currentMatrix;
	VECTOR min,max;

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_TEXTURE_2D);


	min.x = 32768;
	min.y = 32768;
	min.z = 32768;

	max.x = -32768;
	max.y = -32768;
	max.z = -32768;


	ANIMOBJECT_EXT* m_ext;
	m_ext = (ANIMOBJECT_EXT*) pao->userdata;

	unsigned int i;

	mathPushMatrix();

	for(i=0;i<pao->hierachy->nNodes;i++)
	{
		if(pao->hierachy->nodes.boneptr[i].flags&BONEFLAG_MATRIXPOP)
		{
			mathPopMatrix();
			mathStoreMatrix(&previousMatrix);
		}

		if(pao->hierachy->nodes.boneptr[i].flags&BONEFLAG_MATRIXPUSH)
			mathPushMatrix();

		if(i==0)
			mathStoreMatrix(&previousMatrix);

		mathLoadMatrix(&Matrices[i]);
		mathCopyMatrix(mathCurrentMatrix(),&currentMatrix);


		if(pao->hierachy->nodes.boneptr[i].flags&BONEFLAG_BOUNDSPHERE)
		{
			glPushMatrix();
			LoadGLMatrix();
			DrawSphere(pao->hierachy->nodes.boneptr[i].sphere.radius,(CVector*)&pao->hierachy->nodes.boneptr[i].sphere.origin,1.0f,0.0f,0.0f);
			glPopMatrix();
		}

		if(pao->hierachy->nodes.boneptr[i].flags&BONEFLAG_HASBOUND && pao->hierachy->nodes.boneptr[i].flags&BONEFLAG_BOUNDSPHERE)
		{
			VECTOR pos;
			float radius;

			radius = pao->hierachy->nodes.boneptr[i].sphere.radius;

			pos.x = pos.y = pos.z = 0;
			pos.w = 1.0f;

			mathPushMatrix();
			mathMulMatrixPost(&gmathScreen2World);
				
			mathTranslate(&pao->hierachy->nodes.boneptr[i].sphere.origin);
			mathRotTrans(&pos,&pos);
			mathVectorSub(&pos,&pao->apb->position,&pos);

			if(pos.x + radius > max.x) max.x = pos.x + radius;
			if(pos.y + radius > max.y) max.y = pos.y + radius;
			if(pos.z + radius > max.z) max.z = pos.z + radius;

			if(pos.x - radius < min.x) min.x = pos.x - radius;
			if(pos.y - radius < min.y) min.y = pos.y - radius;
			if(pos.z - radius < min.z) min.z = pos.z - radius;

			mathPopMatrix();
		}

		mathStoreMatrix(&previousMatrix);
	}

	mathPopMatrix();


	if(pao->selectedskin)
	{

		for(i=0;i<pao->selectedskin->nmesh;i++)
		{
			VECTOR v0,v1;
			int j;


			if(!(pao->meshflags[i]&MESHFLAG_VISIBLE))
				continue;

			mathLoadMatrix(&Matrices[pao->selectedskin->mesh[i].bone]);

			if(pao->selectedskin->mesh[i].flags&MESHFLAG_ANIMATED)
			{
//				float rot[3];
//				mathGetRotation(&Matrices[pao->hierachy->nNodes + pao->selectedskin->mesh[i].animidx],rot);
				mathMulMatrix(&Matrices[pao->hierachy->nNodes + pao->selectedskin->mesh[i].animidx]);			
			}
			
/*			
			mathPushMatrix();
			mathMulMatrixPost(&gmathScreen2World);
			mathRotTrans(&pao->selectedskin->mesh[i].min,&v0);
			mathRotTrans(&pao->selectedskin->mesh[i].max,&v1);
		
			mathVectorSub(&v0,&pao->apb->position,&v0);
			mathVectorSub(&v1,&pao->apb->position,&v1);
			mathPopMatrix();
*/

			float pBox[6];

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

				mathPushMatrix();
				
				
				mathMulMatrixPost(&gmathScreen2World);
				mathRotTrans(&v,&v);
				mathVectorSub(&v,&pao->apb->position,&v);
				mathPopMatrix();


				if(v.x > max.x) max.x = v.x;
				if(v.y > max.y) max.y = v.y;
				if(v.z > max.z) max.z = v.z;

				if(v.x < min.x) min.x = v.x;
				if(v.y < min.y) min.y = v.y;
				if(v.z < min.z) min.z = v.z;
			}

			glPushMatrix();
			LoadGLMatrix();
			glColor3f(1.0,1.0,0);
			DrawBox(&pao->selectedskin->mesh[i].min,&pao->selectedskin->mesh[i].max);
			glPopMatrix();
		}


	}

	mathPushMatrix();

	mathLoadMatrix(&gmathWorld2Screen);

	mathTranslatef(pao->apb->position.x,pao->apb->position.y,pao->apb->position.z);
//	mathRotZ(pao->apb->rotation.z);
//	mathRotY(pao->apb->rotation.y);
//	mathRotX(pao->apb->rotation.x);

	glPushMatrix();
	LoadGLMatrix();

	glColor3f(0,0,1.0);
	
	DrawBox(&min,&max);
	glPopMatrix();
	mathPopMatrix();

	glPopAttrib();

}




extern void mathGetRotation(MATRIX* m,float xyz[3]);


void DrawSkin(ANIMOBJECT* pao)
{
	MATRIX previousMatrix;
	MATRIX currentMatrix;
	ANIMOBJECT_EXT* m_ext;
	unsigned int i;
	int bonestack[256];
	int pbs = 0;
	int bone = 0;
	BONE* cbone;
	BONE* pbone;

	memset(&everts,0,MAX_VERTS*sizeof(float));

	memset(bonestack,0,sizeof(int)*256);

	m_ext = (ANIMOBJECT_EXT*) pao->userdata;

	glBindTexture(GL_TEXTURE_2D,m_ext->TextureID);


	mathPushMatrix();

	for(i=0;i<pao->hierachy->nNodes;i++)
	{
		if(pao->hierachy->nodes.boneptr[i].flags&BONEFLAG_MATRIXPOP)
		{
			mathPopMatrix();
			mathStoreMatrix(&previousMatrix);

			pbs--;
			bone = bonestack[pbs];
		}

		if(pao->hierachy->nodes.boneptr[i].flags&BONEFLAG_MATRIXPUSH)
		{
			mathPushMatrix();

			bonestack[pbs] = bone;
			pbs++;
		}

		cbone  = &pao->hierachy->nodes.boneptr[i];
		pbone  = &pao->hierachy->nodes.boneptr[bone];

		if(i==0)
		{
			mathStoreMatrix(&previousMatrix);
			bone = i;
		}

		mathLoadMatrix(&Matrices[i]);
		mathCopyMatrix(mathCurrentMatrix(),&currentMatrix);

		DrawSkinBonePair(&previousMatrix,&currentMatrix,cbone,pbone,m_ext->DrawFlags);

		mathStoreMatrix(&previousMatrix);

		bone = i;
	}

	mathPopMatrix();

	DrawExtraSkinBits(m_ext->DrawFlags);
}










void DrawSkinEnvMap(ANIMOBJECT* pao)
{
	MATRIX previousMatrix;
	MATRIX currentMatrix;
	ANIMOBJECT_EXT* m_ext;
	unsigned int i;
	int bonestack[256];
	int pbs = 0;
	int bone = 0;
	BONE* cbone;
	BONE* pbone;

	memset(&everts,0,MAX_VERTS*sizeof(float));

	memset(bonestack,0,sizeof(int)*256);

	m_ext = (ANIMOBJECT_EXT*) pao->userdata;

	glBindTexture(GL_TEXTURE_2D,envmapid);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_ONE);
	glDepthMask(FALSE);

	mathPushMatrix();

	for(i=0;i<pao->hierachy->nNodes;i++)
	{
		if(pao->hierachy->nodes.boneptr[i].flags&BONEFLAG_MATRIXPOP)
		{
			mathPopMatrix();
			mathStoreMatrix(&previousMatrix);

			pbs--;
			bone = bonestack[pbs];
		}

		if(pao->hierachy->nodes.boneptr[i].flags&BONEFLAG_MATRIXPUSH)
		{
			mathPushMatrix();

			bonestack[pbs] = bone;
			pbs++;
		}

		cbone  = &pao->hierachy->nodes.boneptr[i];
		pbone  = &pao->hierachy->nodes.boneptr[bone];

		if(i==0)
		{
			mathStoreMatrix(&previousMatrix);
			bone = i;
		}

		mathLoadMatrix(&Matrices[i]);
		mathCopyMatrix(mathCurrentMatrix(),&currentMatrix);

		DrawSkinBonePairEnvMap(&previousMatrix,&currentMatrix,cbone,pbone,m_ext->DrawFlags);

		mathStoreMatrix(&previousMatrix);

		bone = i;
	}

	mathPopMatrix();

	DrawExtraSkinBitsEnvMap(m_ext->DrawFlags);
}







// more gamelike playback

void AdvanceMorph(ANIMOBJECT *ao, MORPH *morph)
{
	float add;
	int Frame;
	add = (1000.0f/gAnimPlayFPS) / (gAnimPlayFPS/(1000.0F/ao->apb->msPerFrame));

	add = gAnimTargetMSSquared /ao->apb->msPerFrame;

	Frame = (int) APB_MSToFrame(morph->Time);

	if (morph->Sequence)
	{
		MORPH_SEQUENCE *sequence = morph->Sequence;
		float Envelope;
		unsigned int i;

		APB_GetTrackValue(sequence->Envelope->AnimBlock, Frame, &Envelope, 1.0f / 8192.0f);

		for (i = 0; i < sequence->NumTracks; i++)
		{
			float Factor;

			APB_GetTrackValue(sequence->Tracks[i].AnimBlock, Frame, &Factor, 1.0f / 8192.0f);
			Factor *= Envelope;

			morph->Factors[i] = Factor;
		}

		morph->Time+=add;

		float MaxTime;

		MaxTime = (float)APB_FrameToMS((float)sequence->NumFrames);

		if (morph->Time >= MaxTime)
		{
			morph->Time -= MaxTime;
		}
	}
}

void PerformMorph(MESH *mesh, MORPH *morph)
{
	MORPH_MESH* morph_mesh = morph->Sequence->MorphMesh;
	float *source_vertices = morph_mesh->Vertices;

	for (unsigned int i = 0; i < morph_mesh->NumVertices; i++)
	{
		mesh->verts[i * 3 + 0] = source_vertices[0];
		mesh->verts[i * 3 + 1] = source_vertices[1];
		mesh->verts[i * 3 + 2] = source_vertices[2];
		mesh->normals[i * 3 + 0] = source_vertices[3];
		mesh->normals[i * 3 + 1] = source_vertices[4];
		mesh->normals[i * 3 + 2] = source_vertices[5];
		mesh->uvs[i * 2 + 0] = source_vertices[6];
		mesh->uvs[i * 2 + 1] = source_vertices[7];

		source_vertices += 8;

		for (unsigned int t = 0; t < 7; t++, source_vertices += 8)
		{
			if (morph->Factors[t] == 0.0f) continue;

			mesh->verts[i * 3 + 0] += source_vertices[0] * morph->Factors[t];
			mesh->verts[i * 3 + 1] += source_vertices[1] * morph->Factors[t];
			mesh->verts[i * 3 + 2] += source_vertices[2] * morph->Factors[t];
//			mesh->normals[i * 3 + 0] += source_vertices[3] * morph->Factors[t];
//			mesh->normals[i * 3 + 1] += source_vertices[4] * morph->Factors[t];
//			mesh->normals[i * 3 + 2] += source_vertices[5] * morph->Factors[t];
		}

		for (;t < morph_mesh->NumTargets; t++) source_vertices += 8;
	}
}


extern void DrawLocator();



void DrawParented(ANIMOBJECT* ao)
{
	unsigned int i,j,k;
	short* data;
	float* uv;
	float* vert;
	float* normal;
	GLuint id;
	VECTOR lv;
	ANIMOBJECT_EXT* m_ext;

	m_ext = (ANIMOBJECT_EXT*) ao->userdata;

	mathPushMatrix();

	id = 0;
		


	for(i=0;i<ao->selectedskin->nmesh;i++)
	{
		if(!(ao->meshflags[i]&MESHFLAG_VISIBLE))
			continue;

		MATERIAL* mat;
		mat = &ao->selectedskin->pMaterial[ao->selectedskin->mesh[i].matlidx];
		TEXTURE* texture;
		texture = &ao->selectedskin->texture[mat->tidx];

		// Set The Texture

		if(mat->flags&1)	// Add
		{	
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE,GL_ONE);
			glDepthMask(FALSE);
		}
		else if(mat->flags&2) // Sub
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
			glDepthMask(FALSE);
		}
		else
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_DEPTH_TEST);
			glDepthMask(TRUE);
		}

		if(id > 0)
			glDeleteTextures(1,&id);

		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		glGenTextures(1,(GLuint*)&id);
		glBindTexture(GL_TEXTURE_2D,(GLuint)id);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,texture->w,texture->h,0,GL_RGBA,GL_UNSIGNED_BYTE,(void*)texture->data);

		if (ao->Morph.Sequence)
		{
			if (ao->Morph.Sequence->MorphMesh->BaseId == ao->selectedskin->mesh[i].meshID)
				PerformMorph(&ao->selectedskin->mesh[i], &ao->Morph);
		}

		uv     = ao->selectedskin->mesh[i].uvs;
		vert   = ao->selectedskin->mesh[i].verts;
		normal = ao->selectedskin->mesh[i].normals;

		data = (short*) ao->selectedskin->mesh[i].strips;

		mathLoadMatrix(&Matrices[ao->selectedskin->mesh[i].bone]);

		mathPushMatrix();
		mathMulMatrixPost(&gmathInversePerspMatrix);
		mathMulMatrixPost(&gmathInverseCameraMatrix);
		mathTransposeMatrix();
		mathRot(&Light,&lv);
		mathPopMatrix();
				
		if(ao->selectedskin->mesh[i].flags&MESHFLAG_ANIMATED)
		{
			float rot[3];
			mathGetRotation(&Matrices[ao->hierachy->nNodes + ao->selectedskin->mesh[i].animidx],rot);
			mathMulMatrix(&Matrices[ao->hierachy->nNodes + ao->selectedskin->mesh[i].animidx]);
			
//			idx++;
		}


		LoadGLMatrix();

		for(j=0;j<ao->selectedskin->mesh[i].nstrips;j++)
		{
			unsigned int striplen;

			glBegin(GL_TRIANGLE_STRIP);

			striplen = ao->selectedskin->mesh[i].striplens[j];

			float r,g,b;

			r = g = b = 1.0f;

			if(m_ext->DrawFlags&GLVIEWFLAG_SHOWSTRIPS)
			{
				r = (rand()&255)/255.0f;
				g = (rand()&255)/255.0f;
				b = (rand()&255)/255.0f;
			}

			for(k=0;k<striplen;k++)
			{
				int vi;
				int ti;
				int ni;
				VECTOR v;
				VECTOR n;


				vi = *(data++);
				ti = *(data++);
				ni = *(data++);

				v.x = vert[(vi*3)+0];
				v.z = vert[(vi*3)+2];
				v.y = vert[(vi*3)+1];
				v.w = 1.0f;

				// Adjust Verts Same Resolution As Skin!

				short va,vb,vc;

				va = short(v.x * 16.0f);
				vb = short(v.y * 16.0f);
				vc = short(v.z * 16.0f);

				v.x = float(va) / 16.0f;
				v.y = float(vb) / 16.0f;
				v.z = float(vc) / 16.0f;

//				mathRotTrans(&v,&v);

				n.x = normal[(ni*3)+0];
				n.y = normal[(ni*3)+1];
				n.z = normal[(ni*3)+2];

				float dot = mathVectorDot(&lv,&n);


				glTexCoord2f(uv[(ti*2)+0],uv[(ti*2)+1]);
			
				if(mat->flags&1)
				{
					if(dot < 0) dot = -dot;					

				}
				else if(dot < 0) dot = 0; //-dot;
				
				
				glColor3f(r*dot,g*dot,b*dot);

				
				glVertex4f(v.x,v.y,v.z,v.w);

			}

			glEnd();
		}
	
		if((mat->flags>>8)&0xff)
		{
			float intensity = (mat->flags>>8)/255.0f;

			// Enviroment Map
			
			uv     = ao->selectedskin->mesh[i].uvs;
			vert   = ao->selectedskin->mesh[i].verts;
			normal = ao->selectedskin->mesh[i].normals;

			data = (short*) ao->selectedskin->mesh[i].strips;

			glBindTexture(GL_TEXTURE_2D,envmapid);
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE,GL_ONE);
			glDepthMask(FALSE);

			for(j=0;j<ao->selectedskin->mesh[i].nstrips;j++)
			{
				unsigned int striplen;

				glBegin(GL_TRIANGLE_STRIP);

				striplen = ao->selectedskin->mesh[i].striplens[j];

				float r,g,b;

				r = g = b = intensity;

				if(m_ext->DrawFlags&GLVIEWFLAG_SHOWSTRIPS)
				{
					r = (rand()&255)/255.0f;
					g = (rand()&255)/255.0f;
					b = (rand()&255)/255.0f;
				}

				for(k=0;k<striplen;k++)
				{
					int vi;
					int ti;
					int ni;
					VECTOR v;
					VECTOR n;


					vi = *(data++);
					ti = *(data++);
					ni = *(data++);

					v.x = vert[(vi*3)+0];
					v.z = vert[(vi*3)+2];
					v.y = vert[(vi*3)+1];
					v.w = 1.0f;

					// Adjust Verts Same Resolution As Skin!

					short va,vb,vc;

					va = short(v.x * 16.0f);
					vb = short(v.y * 16.0f);
					vc = short(v.z * 16.0f);

					v.x = float(va) / 16.0f;
					v.y = float(vb) / 16.0f;
					v.z = float(vc) / 16.0f;

	//				mathRotTrans(&v,&v);

					n.x = normal[(ni*3)+0];
					n.y = normal[(ni*3)+1];
					n.z = normal[(ni*3)+2];

					float dot = mathVectorDot(&lv,&n);

					mathPushMatrix();
					mathRot(&n,&n);
					mathPopMatrix();
				

	//				glTexCoord2f(uv[(ti*2)+0],uv[(ti*2)+1]);
					glTexCoord2f(n.x,n.y);

					if(mat->flags&1)
					{
						if(dot < 0) dot = -dot;					

					}
					else if(dot < 0) dot = 0; //-dot;
					
					
					glColor3f(r*dot,g*dot,b*dot);

					
					glVertex4f(v.x,v.y,v.z,v.w);

				}

				glEnd();
			}

		}

	}

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB,GL_MODULATE);

	mathPopMatrix();

	if(id > 0)
		glDeleteTextures(1,&id);

	glDepthMask(TRUE);


}




void ProcessFeet(ANIMOBJECT* pao)
{
	uint32 i;
	ANIMATION_PLAYBACK* papb;
	uint32 overlayIndex;
	BOOL bSource = TRUE;
	BOOL bDest = TRUE;
	papb = pao->apb;

	if (papb->animation.source.anim)
	{
		for (i=0; i<papb->animation.source.anim->nKeys; i++)
		{
			TRACK_KEY* key=&papb->animation.source.anim->keys[i];

			if (key->type==TRACK_KEY_CODE_FIRE_AUDIO_EVENT)
			{
				short time=(short)(key->time&0xFFFF); // Mask off time's lower 16 bits			
				
				if (time==pao->apb->animation.source.frame0) // Is this key's time between the current frame markers?
				{
					if(key->time&AEFLAG_ISFOOTSTEP)
						FXHandler(pao,NULL,(key->time>>25)&0x3);				
				}
			}				
		}
	}
}



void SkinCharEndFrame(ANIMOBJECT* pao)
{
	ANIMOBJECT_EXT* m_ext;
	m_ext = (ANIMOBJECT_EXT*) pao->userdata;

	Light.x = -gmathCameraMatrix.m02;
	Light.y = -gmathCameraMatrix.m12;
	Light.z = -gmathCameraMatrix.m22;

	// Draw Wireframe

	if(!pao->selectedskin)
		DrawHierachy(pao);

	ProcessFeet(pao);

	DrawFXNodes2(pao);

	if(m_ext->DrawFlags&GLVIEWFLAG_DRAWSPHERES)
		DrawCollisionSpheres(pao);

	// Draw Skin Parts And Extras

	if(pao->selectedskin)
	{
		MATERIAL* mat;
		mat = &pao->selectedskin->pMaterial[pao->selectedskin->matidx];
		
		// Solid 

		glEnable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(TRUE);
		glAlphaFunc(GL_GREATER,0.5);

		g_skindata = (int8*) pao->selectedskin->skin[SKIN_SOLID];

		if(g_skindata)
			DrawSkin(pao);

		// Draw Parented Items Now Before Skinned Color Add/Sub
		
		DrawParented(pao);

		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		glDepthMask(FALSE);
		
		g_skindata = (int8*) pao->selectedskin->skin[SKIN_CADD];
		if(g_skindata)
			DrawSkin(pao);
	
		glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
		g_skindata = (int8*) pao->selectedskin->skin[SKIN_CSUB];
		if(g_skindata)
			DrawSkin(pao);
	

		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(TRUE);
	}

	// Pop GL Attrib Pushed On BeginFrame

	glPopAttrib();
}




void SetBeginFrame(ANIMOBJECT* pao)
{
	ANIMOBJECT_EXT* m_ext;
	m_ext = (ANIMOBJECT_EXT*) pao->userdata;
	int DrawFlags = m_ext->DrawFlags;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_TEXTURE_2D);
	VECTOR a;
	glLoadIdentity();
	glBegin(GL_LINES);
	glColor3f(0,1.0f,0);
	a.x = -50.0f; a.y = 0.0f; a.z = 0.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
	a.x =  50.0f; a.y = 0.0f; a.z = 0.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
	a.x = 0.0f; a.y = -50.0f; a.z = 0.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
	a.x = 0.0f; a.y =  50.0f; a.z = 0.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
	a.x = 0.0f; a.y = 0.0f; a.z = -50.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
	a.x = 0.0f; a.y = 0.0f; a.z =  50.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
	glEnd();

	glPopAttrib();

	glPushAttrib(GL_ENABLE_BIT);

	if(m_ext->TextureID >= 0)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,m_ext->TextureID);
	}

	if(DrawFlags&GLVIEWFLAG_SHOWSEGMENTS || DrawFlags&GLVIEWFLAG_SHOWSTRIPS)
	{
		glDisable(GL_TEXTURE_2D);
	}

	srand(256);

	if(DrawFlags&GLVIEWFLAG_DRAWWIREFRAME)
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

	Light.x = -gmathCameraMatrix.m02;
	Light.y = -gmathCameraMatrix.m12;
	Light.z = -gmathCameraMatrix.m22;

	nMatrices = 0;

}






void SetProcessNode(ANIMOBJECT* pao,BONE* bone,MATRIX* previousMatrix,MATRIX* currentMatrix, int iBone, int iParentBone)
{
	ANIMOBJECT_EXT* m_ext;

	m_ext = (ANIMOBJECT_EXT*) pao->userdata;

	unsigned int i,j,k;
	short* data;
	float* uv;
	float* vert;
	float* normal;
	GLuint id;

	VECTOR lv;

	mathCopyMatrix(currentMatrix,&Matrices[nMatrices++]);

	i = iBone;

	m_ext = (ANIMOBJECT_EXT*) pao->userdata;


	id = 0;

	if(pao->selectedskin)
	{
		for(j=0;j<pao->selectedskin->nmesh;j++)
		{
			if(bone->boneID == pao->selectedskin->mesh[j].meshID)
			{
				i = j;
				break;
			}
	
		}

		if(j >= pao->selectedskin->nmesh)
			return;
	}
	else
	{
		mathPushMatrix();
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_TEXTURE_2D);
		VECTOR a;
		glLoadIdentity();
		glBegin(GL_LINES);
		glColor3f(0,1.0f,0);
		a.x = -50.0f; a.y = 0.0f; a.z = 0.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
		a.x =  50.0f; a.y = 0.0f; a.z = 0.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
		a.x = 0.0f; a.y = -50.0f; a.z = 0.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
		a.x = 0.0f; a.y =  50.0f; a.z = 0.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
		a.x = 0.0f; a.y = 0.0f; a.z = -50.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
		a.x = 0.0f; a.y = 0.0f; a.z =  50.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
		glEnd();
		mathPopMatrix();
	
		return;
	}

	if(!(pao->meshflags[i]&MESHFLAG_VISIBLE))
			return;


	CPropHierachy* hierachy;
	hierachy = (CPropHierachy*) GetDocument()->GetData()->m_Character.hierachy;

	if(hierachy->ds->mp_Mesh[0].nTextures)
	{
		CTexture* texture;
		texture = &hierachy->ds->mp_Mesh[0].Textures[0];

		if(texture->textureID > 0) glDeleteTextures(1,&texture->textureID);
			
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glGenTextures(1, (GLuint*)&texture->textureID);
		glBindTexture(GL_TEXTURE_2D, (GLuint)texture->textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)texture->data);

		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		
	}
	
	mathPushMatrix();

	uv     = pao->selectedskin->mesh[i].uvs;
	vert   = pao->selectedskin->mesh[i].verts;
	normal = pao->selectedskin->mesh[i].normals;

	data = (short*) pao->selectedskin->mesh[i].strips;

	mathPushMatrix();

	mathMulMatrixPost(&gmathInversePerspMatrix);
	mathMulMatrixPost(&gmathInverseCameraMatrix);
	mathTransposeMatrix();
	mathRot(&Light,&lv);

	mathPopMatrix();

	for(j=0;j<pao->selectedskin->mesh[i].nstrips;j++)
	{
		unsigned int striplen;

		glBegin(GL_TRIANGLE_STRIP);

		striplen = pao->selectedskin->mesh[i].striplens[j];

		float r,g,b;

		r = g = b = 1.0f;

		if(m_ext->DrawFlags&GLVIEWFLAG_SHOWSTRIPS)
		{
			r = (rand()&255)/255.0f;
			g = (rand()&255)/255.0f;
			b = (rand()&255)/255.0f;
		}

		for(k=0;k<striplen;k++)
		{
			int vi;
			int ti;
			int ni;
			VECTOR v;
			VECTOR n;


			vi = *(data++);
			ti = *(data++);
			ni = *(data++);

			v.x = vert[(vi*3)+0];
			v.z = vert[(vi*3)+2];
			v.y = vert[(vi*3)+1];
			v.w = 1.0f;

			mathRotTrans(&v,&v);

			n.x = normal[(ni*3)+0];
			n.y = normal[(ni*3)+1];
			n.z = normal[(ni*3)+2];

			float dot = mathVectorDot(&lv,&n);

			if(dot < 0) dot = 0;

			glTexCoord2f(uv[(ti*2)+0],uv[(ti*2)+1]);
			glColor3f(r*dot,g*dot,b*dot);
			glVertex4f(v.x,v.y,v.z,v.w);

		}

		glEnd();
	}

//	if(id > 0)
//		glDeleteTextures(1,&id);

	mathPopMatrix();

}




void SetEndFrame(ANIMOBJECT* pao)
{
	DrawFXNodes2(pao);
}




#define	FXFLAG_FOOTRIGHT	(1<<0)
#define FXFLAG_FOOTFRONT	(1<<1)



void fxAnimTriggerHandler(ANIMOBJECT* pao,FXNODE* pFX,uint32 flags)
{
	char* bones[4] = {{"TOE_L"},{"TOE_R"},{"PALM_L"},{"PALM_R"}};
		
	// FootStep Process
	
	if(pFX == NULL)
	{	
		BONE* bone;

		if(!(flags&FXFLAG_FOOTFRONT))
		{
			if(flags&FXFLAG_FOOTRIGHT)
				bone = HR_FindBone(pao->hierachy,STRING_GetHashValue(bones[1]));
			else
				bone = HR_FindBone(pao->hierachy,STRING_GetHashValue(bones[0]));
		}
		else
		{
			if(flags&FXFLAG_FOOTRIGHT)
				bone = HR_FindBone(pao->hierachy,STRING_GetHashValue(bones[3]));
			else
				bone = HR_FindBone(pao->hierachy,STRING_GetHashValue(bones[2]));
		}

		if(bone)
		{
			mathPushMatrix();
			mathLoadMatrix(&Matrices[bone->boneidx]);

			LoadGLMatrix();

			glPushAttrib(GL_ALL_ATTRIB_BITS);
			glDisable(GL_TEXTURE_2D);
			VECTOR a;
			glLoadIdentity();

			glBegin(GL_LINES);
			glColor3f(1.0f,0.0f,0);
			a.x = -15.0f; a.y = 0.0f; a.z = 0.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
			a.x =  15.0f; a.y = 0.0f; a.z = 0.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);

			glColor3f(0.0f,1.0f,0);
			a.x = 0.0f; a.y = -15.0f; a.z = 0.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
			a.x = 0.0f; a.y =  15.0f; a.z = 0.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
			
			glColor3f(0.0f,0.0f,1.0);
			a.x = 0.0f; a.y = 0.0f; a.z = 0.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
			a.x = 0.0f; a.y = 0.0f; a.z =  15.0f; a.w = 0.0f; mathRotTrans(&a,&a); glVertex4f(a.x,a.y,a.z,a.w);
			glEnd();
			

			glPopAttrib();

			mathPopMatrix();
		}		
	}
}





