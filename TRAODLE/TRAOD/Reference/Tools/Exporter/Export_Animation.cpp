//***************************************************************************
//
//
//  Export_Animation.cpp 05/28/02
//
//***************************************************************************
#include "Export_Animation.h"
#include "Exporter_Utility.h"
#include "Export_Camera.h"
#include "GDBServer/gdbglobal.h"


#define HIP_VELOCITY		0
#define	ORIGIN_VELOCITY		1

#pragma warning(disable:4018)	// get rid of signed / unsigned comparison error
#pragma warning(disable:4244)	// get rid of float - double conversion error

#define GetValue(object,attr,value)	{MPlug plug(object,attr); plug.getValue(value);}

void ObjectAdjustJointArrayDynamic(MObjectArray &joints,MIntArray &children);
void ObjectWriteAnimation(MString filename,MObjectArray&joints,int bf,int ef,MString &originname,MObject,bool);
void WriteTransformHierachy(MObject &origin,MString &filename);


#define MORPHS


int 	undocnt;
char 	file_ext[] = {"trx"};
int 	nFuckedRooms;

bool bCutscene = false;
MObject gCOrigin;


void ExportAnimation(MString filename, int ab_SkipDialog)
{
	MObject origin;
	int i;

	if(FindNode(origin,"ORIGIN",MFn::kTransform))
	{

		Log(LTINFO,"Found the origin node...");

		MIntArray types;
		MObjectArray joints;
		types.append(MFn::kJoint);

		GetChildObjects(origin,joints,types,true);

		MIntArray childcounts;

		// Remove The Dynamic Joints As Well As End Joints
		AdjustJointArrayDynamic(joints,childcounts);

		if (!ab_SkipDialog) 
		{
			// Call Setup Dialog For Bake Etc...
			if(!DoAnimDialog(joints[0],origin))
				return;
		}

		// Create Array Of Locators To Output
		MObjectArray objects;

		FindNodesByType(objects,MFn::kPluginLocatorNode,ANIMSPLITTER_ID);

		// First Bake The Whole Animation Range From All The Splitters
		int begin =  10000000;
		int end   = -10000000;

		int plappy = joints.length();

		for(i=0;i<objects.length();i++)
		{
			MObject beginframe;
			MObject endframe;
			int bf;
			int ef;

			FindNodeAttribute(objects[i],"beginFrame" ,beginframe);
			FindNodeAttribute(objects[i],"endFrame"	  ,endframe);

			MPlug bfplug(objects[i],beginframe);
			bfplug.getValue(bf);

			MPlug efplug(objects[i],endframe);
			efplug.getValue(ef);

			if(bf < begin) begin = bf;
			if(ef > end  ) end 	 = ef;
		}

		// Do Animation Bake
		ExecuteMayaCommand("bakeResults -simulation true -t \"%d:%d\" -hierarchy below -sampleBy 1 -disableImplicitControl true -preserveOutsideKeys true -sparseAnimCurveBake false -controlPoints false -shape true {\"%s\"}",
							(int)begin,
							(int)end,
							GetNodeName(origin).asChar());

		ExecuteMayaCommand("doEnableNodeItems false all");

		// If Export Selected Locate It
		// Clear The Array And Add As First Element
		if(!bExportAll)
		{
			int i;
			for(i=0;i<objects.length();i++)
			{
				MObject cname;
				MString clipname;
				FindNodeAttribute(objects[i],"clipName",cname);
				MPlug p(objects[i],cname);
				p.getValue(clipname);
				clipname.toUpperCase();
				if(clipname == selected)
				{
					MObject t;
					t = objects[i];
					objects.clear();
					objects.append(t);
					break;
				}
			}
		}

		// Get The Character Name
		MObjectArray objs;
		MObject cname,expdir;
		MString characterName;
		MString dir;
		FindNodesByType(objs,MFn::kPluginLocatorNode,ANIMLOCATOR_ID);

		FindNodeAttribute(objs[0],"exportDirectory",expdir);
		MPlug plg(objs[0],expdir);
		plg.getValue(dir);

		FindNodeAttribute(objs[0],ANIMORIGIN_CNAME,cname);
		MPlug p(objs[0],cname);
		p.getValue(characterName);


		//-----------------------------------------------------------------------------------
		//
		// Get Child heirachies
		//
		MObjectArray l_ChildHierachys;
		MObjectArray l_ChildHierachyParents;
		GetChildHierachys(origin, l_ChildHierachys, l_ChildHierachyParents);

		for(i=0;i<objects.length();i++)
		{
			MString msfile;
			int bf;
			int ef;

			MObject tx, ty, tz, rz;
			MObject removehiptrans;
			MObject removezrot, vo;
			MObject snaporigin, loop, link;
			bool val;

			MObject beginframe;
			MObject endframe;

			//-----------------------------------------------------------------------------------
			//
			// Create The Filename
			//
			msfile  = dir + "\\" + characterName;

			MObject cname;
			MString clipname;

			FindNodeAttribute(objects[i],"clipName",cname);
			MPlug p(objects[i],cname);
			p.getValue(clipname);

			msfile += "_"+clipname+".mea";

			Log(LTINFO,"%s",msfile.asChar());


			FindNodeAttribute(objects[i],"beginFrame",beginframe);
			FindNodeAttribute(objects[i],"endFrame",endframe);

			MPlug bfplug(objects[i],beginframe);
			bfplug.getValue(bf);

			MPlug efplug(objects[i],endframe);
			efplug.getValue(ef);

			//-----------------------------------------------------------------------------------
			//
			// Setup VeloTracks variable
			//
			FindNodeAttribute( objects[i], "exportTranslateX"	, tx		 );
			FindNodeAttribute( objects[i], "exportTranslateY"	, ty		 );
			FindNodeAttribute( objects[i], "exportTranslateZ"	, tz		 );
			FindNodeAttribute( objects[i], "exportRotateZ"		, rz		 );
			FindNodeAttribute( objects[i], "zOrientation"		, removezrot );
			FindNodeAttribute( objects[i], "velocityFromOrigin"	, vo		 );
			FindNodeAttribute( objects[i], "loopingAnim"		, loop		 );
			FindNodeAttribute( objects[i], "linkAnim"			, link		 );
			VeloTracks = 0;

			GetValue( objects[i], tx, val); if ( val ) VeloTracks |= ANIM_HASTX;
			GetValue( objects[i], ty, val); if ( val ) VeloTracks |= ANIM_HASTY;
			GetValue( objects[i], tz, val); if ( val ) VeloTracks |= ANIM_HASTZ;
			GetValue( objects[i], rz, val); if ( val ) VeloTracks |= ANIM_HASRZ;

			GetValue( objects[i], loop		 , bLooped		   );
			GetValue( objects[i], link		 , bLink		   );
			GetValue( objects[i], vo		 , bVelocityOrigin );
			GetValue( objects[i], removezrot , ZRotRemove	   );


			//-----------------------------------------------------------------------------------
			//
			// Open Animation File for writing
			//
			const char* plapy = msfile.asChar();

			ChunkOpen((char*)msfile.asChar());
			{
				//-----------------------------------------------------------------------------------
				//
				// recurse for child heirachys
				//
//				int j;
//				int numHierachys;

				//-----------------------------------------------------------------------------------
				//
				// Write out Bone Joint hierachy
				//
				WriteAnimation((char*)msfile.asChar(),joints,bf,ef,clipname);


				//-----------------------------------------------------------------------------------
				//
				// Write Child Hierachys Count
				//
/*				ChunkBegin(CHUNKID("HCNT"));
				{
					numHierachys = l_ChildHierachys.length();
					ChunkWriteInt(&numHierachys);
				}
				ChunkClose();

				//-----------------------------------------------------------------------------------
				//
				// Write out child heirachy Chunks
				//
				GetChildHierachys(origin, l_ChildHierachys, l_ChildHierachyParents);
				for ( j = 0 ; j < numHierachys ; j++ )
				{
					MObjectArray l_NodeList;

					GetChildHierachyNodes(l_ChildHierachys[j], l_NodeList);
					WriteChildHierachyChunk(l_NodeList, l_ChildHierachyParents[j], bf, ef);
				}*/
			}
			ChunkClose();

		
		  }

		MString command;
		MStringArray result;

		command.clear();
		command = "undo";

		MGlobal::executeCommand(command,result,true,true);
		MGlobal::executeCommand(command,result,true,true);

		Hip 	= MObject::kNullObj;
		Origin 	= MObject::kNullObj;
	}
	else
	{
		Log(LTERROR,"Unable to find origin");
	}
}

void ExportAnimationForObject(MObject object,int bf,int ef,bool bhip)
{
	int flag;
	MObject txobj;
	MObject tyobj;
	MObject tzobj;
	MObject rxobj;
	MObject ryobj;
	MObject rzobj;
	MObject sxobj;
	MObject syobj;
	MObject szobj;
	bool bTranslate = true;

	MString name;
	name = GetNodeName(object);

	Log(0,"%s",name.asChar());

	ChunkWriteString((char*)name.asChar());

//	const char * dappy = name.asChar();

	flag = 0;

	if(GetAnimCurveFor(object,MString("translateX"),txobj))	flag|=ANIM_HASTX;
	if(GetAnimCurveFor(object,MString("translateY"),tyobj))	flag|=ANIM_HASTY;
	if(GetAnimCurveFor(object,MString("translateZ"),tzobj))	flag|=ANIM_HASTZ;

	if(GetAnimCurveFor(object,MString("rotateX"),rxobj))	flag|=ANIM_HASRX;
	if(GetAnimCurveFor(object,MString("rotateY"),ryobj))	flag|=ANIM_HASRY;
	if(GetAnimCurveFor(object,MString("rotateZ"),rzobj))	flag|=ANIM_HASRZ;
	if(GetAnimCurveFor(object,MString("scaleX"),sxobj))		flag|=ANIM_HASSX;
	if(GetAnimCurveFor(object,MString("scaleY"),syobj))		flag|=ANIM_HASSY;
	if(GetAnimCurveFor(object,MString("scaleZ"),szobj))		flag|=ANIM_HASSZ;

	bool value = true;
	GetAttributeValue(object,"segmentScaleCompensate",value);
	if(value)
		flag |= ANIM_HASSSC;


//	segmentScaleCompensate

	float vtx,vty,vtz;
	vtx = vty = vtz = 0;

	ChunkWriteInt(&flag);


	if(flag&ANIM_HASTX)	SampleCurve(txobj,bf,ef,0);
	if(flag&ANIM_HASTY)	SampleCurve(tyobj,bf,ef,0);
	if(flag&ANIM_HASTZ)	SampleCurve(tzobj,bf,ef,0);


	if(flag&ANIM_HASRX)	SampleCurve(rxobj,bf,ef,0);
	if(flag&ANIM_HASRY)	SampleCurve(ryobj,bf,ef,0);

	if(bCutscene && bhip)
	{
		MObject orzobj;
		GetAnimCurveFor(gCOrigin,MString("rotateZ"),orzobj);
		if(flag&ANIM_HASRZ)	SampleCurveAdd(rzobj,bf,ef,orzobj);
	}
	else 
		if(flag&ANIM_HASRZ)	SampleCurve(rzobj,bf,ef,0);


	if(flag&ANIM_HASSX)	SampleCurve(sxobj,bf,ef,0);
	if(flag&ANIM_HASSY)	SampleCurve(syobj,bf,ef,0);
	if(flag&ANIM_HASSZ)	SampleCurve(szobj,bf,ef,0);
}




void GetFXNodes(MObject joint,MObjectArray &objects)
{
	MObjectArray nodes;
	MIntArray types;
	int j;

	nodes.clear();
	types.clear();
	types.append(MFn::kTransform);

	GetChildObjects(joint, nodes, types, false);

	objects.clear();
	types.clear();
	types.append(MFn::kPluginLocatorNode);

	for(j=0;j<(int)nodes.length();j++) GetChildObjects(nodes[j],objects, types, true);
}




void ExportFXLocator(MObject &object,int bf,int ef,int nFrames)
{

	Log(LTINFO,"-------> %s",GetNodeName(object).asChar());

	// Add Mesh Name To Object List
	objectnames.append(GetNodeName(object));

	// Now Get The Transform Node For This Object
	MObject transform;
	GetTransform(object,transform);
	MFnTransform tf(transform);
	float x,y,z,w;

	ChunkWriteString((char*)GetNodeName(transform).asChar());

	Log(0,"%s",tf.fullPathName().asChar()) ;

	cout << GetNodeName(transform).asChar() << endl;


	MEulerRotation r;
	tf.getRotation(r);

	Log(LTINFO,"Rotate      %8.3f %8.3f %8.3f",RTOD(r.x),RTOD(r.y),RTOD(r.z));
	x = (float)r.x;
	y = (float)r.y;
	z = (float)r.z;
	ChunkWriteFloatTriple(&x,&y,&z);

	MVector t;
	t = tf.translation(MSpace::kTransform);
	Log(LTINFO,"Translate   %8.3f %8.3f %8.3f",t.x,t.y,t.z);
	x = (float)t.x;
	y = (float)t.y;
	z = (float)t.z;
	ChunkWriteFloatTriple(&x,&y,&z);

	double scale[3];
	tf.getScale(scale);
	Log(LTINFO,"scale       %8.3f %8.3f %8.3f",scale[0],scale[1],scale[2]);
	x = (float)scale[0];
	y = (float)scale[1];
	z = (float)scale[2];
	ChunkWriteFloatTriple(&x,&y,&z);

	MPoint sp,rp;
	MVector rpt;
	MVector spt;
	sp = tf.scalePivot(MSpace::kTransform);
	x =(float) sp.x;
	y =(float)sp.y;
	z =(float) sp.z;
	ChunkWriteFloatTriple(&x,&y,&z);

	spt = tf.scalePivotTranslation(MSpace::kTransform);
	x = (float)spt.x;
	y = (float)spt.y;
	z =(float) spt.z;
	ChunkWriteFloatTriple(&x,&y,&z);

	rp = tf.rotatePivot(MSpace::kTransform);
	x = (float)rp.x;
	y = (float)rp.y;
	z = (float)rp.z;
	ChunkWriteFloatTriple(&x,&y,&z);

	rpt = tf.rotatePivotTranslation(MSpace::kTransform);
	x = (float)rpt.x;
	y = (float)rpt.y;
	z = (float)rpt.z;
	ChunkWriteFloatTriple(&x,&y,&z);

	Log(LTINFO,"RotatePivot %8.3f %8.3f %8.3f",rp.x,rp.y,rp.z);
	Log(LTINFO,"RP Trans    %8.3f %8.3f %8.3f",rpt.x,rpt.y,rpt.z);

	Log(LTINFO,"ScalePivot  %8.3f %8.3f %8.3f",sp.x,sp.y,sp.z);
	Log(LTINFO,"SP Trans    %8.3f %8.3f %8.3f",spt.x,spt.y,spt.z);
				
	MQuaternion ro;
	ro = tf.rotateOrientation(MSpace::kTransform);
	x =  ro.x;
	y =  ro.y;
	z =  ro.z;
	w =  ro.w;
	ChunkWriteFloatTriple(&x,&y,&z);
	ChunkWriteFloat(&w);

	// Export User Data
	
	MObject FXType;
	MObject	FXMsg;
	MObject RGB;
	MObject Transparency;	
	MObject emitRate;	
	MObject speed;
	MObject random;
	MObject user0,user1,user2,user3;
	

	int		fx,msg;
//	float	rgb[3];
	float	u0,u1,u2,u3;
	float	alpha,emit,vel,ran;
	
	FindNodeAttribute(object,"FXType",FXType);
	FindNodeAttribute(object,"Message",FXMsg);
	FindNodeAttribute(object,"RGB",RGB);
	FindNodeAttribute(object,"Transparency",Transparency);
	FindNodeAttribute(object,"emitRate",emitRate);
	FindNodeAttribute(object,"speed",speed);
	FindNodeAttribute(object,"random",random);
	FindNodeAttribute(object,"user0",user0);
	FindNodeAttribute(object,"user1",user1);
	FindNodeAttribute(object,"user2",user2);
	FindNodeAttribute(object,"user3",user3);

	GetValue(object,FXType,			fx);
	
	// Get Key Value 
	
	list<GDBFxType*>::iterator i = g_GameDB.m_lFxTypes.begin();
	
	int idx = 0;

	for(;i != g_GameDB.m_lFxTypes.end();i++)
	{	
		GDBFxType *pFX = *i;
		if(idx == fx)
		{
			fx = pFX->m_nKey;
			break;
		}
		idx++;
	}



	MObject obj;
	MPlug cp(object,RGB);
	cp.getValue(obj);
	MFnNumericData data(obj);
	float cr,cg,cb;
	data.getData(cr,cg,cb);
	
	cr = cr * 255.0f;
	cg = cg * 255.0f;
	cb = cb * 255.0f;
	GetValue(object,Transparency,	alpha);
	
	alpha = alpha * 255.0f;
	
	GetValue(object,FXMsg,			msg);
	GetValue(object,emitRate,		emit);
	GetValue(object,speed,			vel);
	GetValue(object,random,			ran);
	GetValue(object,user0,			u0);
	GetValue(object,user1,			u1);
	GetValue(object,user2,			u2);
	GetValue(object,user3,			u3);

	int color; 

	color = (((int)alpha)<<24)|(((int)cr)<<16)|(((int)cg)<<8)|((int)cb);
	
	ChunkWriteInt(&fx);
	ChunkWriteInt(&color);
	ChunkWriteFloat(&emit);
	ChunkWriteFloat(&vel);
	ChunkWriteFloat(&ran);
	ChunkWriteFloat(&u0);
	ChunkWriteFloat(&u1);
	ChunkWriteFloat(&u2);
	ChunkWriteInt  (&msg);				// Was user3

	// Export Visibility Track For FXNode

	MObject node,vis;	

	ChunkWriteInt(&nFrames);

	GetTransform(object,node);

	// write name
//	MFnDependencyNode depNode(node);

	cout << "Number Of Frames " << nFrames << endl;
//	cout << "ef - bf " << ef-bf << endl;

	if(GetAnimCurveFor(node,MString("visibility"),vis))
	{
		// no need to sample curve, visibility is either on or off.
		SampleCurve(vis,bf,ef,0);
	}
	else
	{
		float pad = 0.0f;
		
		for(int i=0;i<nFrames;i++)
			ChunkWriteFloat(&pad);
	}


}




void WriteAnimation(MString filename,MObjectArray&joints,int bf,int ef,MString &originname,bool bWriteBind)
{
	MTime startFrame;
	MTime endFrame ;
	MTime currentFrame;

	startFrame.setUnit(MTime::kNTSCFrame);
	endFrame.setUnit(MTime::kNTSCFrame);
	currentFrame.setUnit(MTime::kNTSCFrame);
	int numframes;
	int numobjs;
	int i,j;
	MObject velocity;
	MString gameOrigin;
	
	if(originname.length())
		gameOrigin = originname + "_gameOrigin";
		

	startFrame.setValue(bf);
	endFrame.setValue(ef);

	Log(LTINFO,"Exporting %s",gameOrigin.asChar());

	numobjs = joints.length();

	Log(LTINFO,"Found %d joints from origin",joints.length());

	numframes = 0;

	for(currentFrame=startFrame;currentFrame<endFrame;currentFrame+=1.0f)
		numframes++;

	numframes++;

	Log(LTINFO,"Number Of Frames %d",numframes);

	// Write Out BindMatrix Per Bone

	ChunkBegin(CHUNKID("VERS"));

		int version = ANIMATION_VERSION;
		ChunkWriteInt(&version);

	ChunkEnd();

	if(bWriteBind)
	{
		ChunkBegin(CHUNKID("BIND"));
		{
			ChunkWriteInt(&numobjs);
			for(i=0;i<joints.length();i++)
			{
				MString name = GetNodeName(joints[i]);
				ChunkWriteString((char*)name.asChar());

				MMatrix matrix,pre,post;
				float fmatrix[4][4];
				GetRestMatrix(joints[i],matrix,pre,post);

				matrix.get(fmatrix);
				for(int j=0;j<4;j++)
					for(int k=0;k<4;k++)
						ChunkWriteFloat(&fmatrix[j][k]);
			
				pre.get(fmatrix);
				for(j=0;j<4;j++)
					for(int k=0;k<4;k++)
						ChunkWriteFloat(&fmatrix[j][k]);
			
				post.get(fmatrix);
				for(j=0;j<4;j++)
					for(int k=0;k<4;k++)
						ChunkWriteFloat(&fmatrix[j][k]);
			}
		}
		ChunkEnd();
	}

	ChunkBegin(CHUNKID("ANIM"));
	{
		ChunkWriteInt(&numframes);
		ChunkWriteInt(&numobjs);
		ChunkWriteInt(&ZRotRemove);

		int pad = 0;

		ChunkWriteInt(&bVelocityOrigin);
		ChunkWriteInt(&pad);
		ChunkWriteInt(&pad);


		ChunkWriteInt(&bLooped);
		ChunkWriteInt(&bLink);

		ExportAnimationForObject(joints[0],bf,ef,true);
		for( i = 1 ; i < (int)joints.length() ; i++ )
			ExportAnimationForObject(joints[i],bf,ef,false);

	}
	ChunkEnd();

	// Velocity chunk
	
	if(gameOrigin.length())
	{
		ChunkBegin(CHUNKID("VELO"));
		{
			ChunkWriteInt(&numframes);
			ChunkWriteInt((int*)&VeloTracks);

			MObject vcube;
			const char * plappety = gameOrigin.asChar();
			FindNode(vcube,(char*)gameOrigin.asChar(),MFn::kTransform);
			ExportAnimationForObject(vcube,bf,ef,true);
		}
		ChunkEnd();
	}
	else
	{
		ChunkBegin(CHUNKID("VELO"));
		{
			ChunkWriteInt(&numframes);
			ChunkWriteInt((int*)&VeloTracks);
			ExportAnimationForObject(joints[0],bf,ef,true);
		}
		ChunkEnd();
	}




	// Get Total Number Parented ( == CHILD ) Objects
	numobjs = 0;
	for(i=0;i<(int)joints.length();i++)
	{
		MObjectArray objects;
		GetParentedMeshes(joints[i],objects);
		numobjs += objects.length();
	}

	//-----------------------------------------------------------------------------------
	//
	// General Node Visibility
	//
	if(numobjs)
	{
		ChunkBegin(CHUNKID("PMAN"));
		{
			ChunkWriteInt(&numobjs);

			// Now Get Parented ( == CHILD ) Objects
			for(i=0;i<(int)joints.length();i++)
			{
				int cnt;

				MObjectArray objects;
				GetParentedMeshes( joints[i], objects );

				cnt = objects.length();

				if(cnt)
				{
					ChunkWriteInt(&i);
					ChunkWriteInt(&cnt);
					for(j=0;j<cnt;j++)
					{
						// Get Parent Transform
						MObject node;
						GetTransform(objects[j],node);
						ExportAnimationForObject(node,bf,ef,false);
					}
				}
			}
		}
		ChunkEnd();
	}

	Log(0,"---");

	//-----------------------------------------------------------------------------------
	//
	// General Node Visibility
	//
	if(numobjs)
	{
		ChunkBegin(CHUNKID("PVIS"));
		{
			ChunkWriteInt(&numobjs);

			// Now Get Child Objects
			for( i = 0 ; i < (int)joints.length(); i++ )
			{
				int cnt;
				MObjectArray objects;

				GetParentedMeshes(joints[i],objects);

				cnt = objects.length();
				ChunkWriteInt(&cnt);

				if(cnt)
				{
					ChunkWriteInt(&i);
					ChunkWriteInt(&cnt);

					for(j=0;j<cnt;j++)
					{
						MObject vis;
						MObject node;
						MString name;

						// Get Parent Transform
						GetTransform(objects[j],node);

						// write name
						name = GetNodeName(node);
						Log(0,"%s",name.asChar());

						MFnDependencyNode depNode(node);
						if(GetAnimCurveFor(node,MString("visibility"),vis))
						{
							int marker = 1;

							Log(0,"***");
							ChunkWriteInt(&marker);
							ChunkWriteString((char*)name.asChar());

							// no need to sample curve, visibility is either on or off.
							SampleCurve(vis,bf,ef,0);
						}
						else
						{
							int dummyMarker = 0;
							ChunkWriteInt(&dummyMarker);
						}
					}
				}
			}
		}
		ChunkEnd();
	}

	// FX Nodes

	// Get Total Number FX Child Objects

	numobjs = 0;
	for(i=0;i<(int)joints.length();i++)
	{
		MObjectArray objects;
		GetFXNodes(joints[i],objects);
		numobjs += objects.length();
	}
	
	if(numobjs)
	{
		ChunkBegin(CHUNKID("FXPT"));
		{
			ChunkWriteInt(&numobjs);

			// Now Get Parented ( == CHILD ) Objects
			for(i=0;i<(int)joints.length();i++)
			{
				int cnt;

				MObjectArray objects;
				GetFXNodes( joints[i], objects );

				cnt = objects.length();

				if(cnt)
				{
					MString name;
					name = GetNodeName(joints[i]);
					
					ChunkWriteString((char*) name.asChar(),true);
									
					cout << "*** " << name.asChar() << endl;

					ChunkWriteInt(&i);
					ChunkWriteInt(&cnt);
					
					
					for(j=0;j<cnt;j++)
					{
						// Get Parent Transform
						MObject node;
						GetTransform(objects[j],node);
//						ExportAnimationForObject(node,bf,ef,false);
						
						ExportFXLocator(objects[j],bf,ef,numframes);

					}
				}
			}
		}
		ChunkEnd();
	}

	//-----------------------------------------------------------------------------------
	//
	// Output Extra Information
	//
	int pad = 0;
	ChunkBegin(CHUNKID("AINF"));
	{
		// Output Anim Info File
		MFileIO file;
		MString name;
		name = file.currentFile();

		ChunkWriteString((char*)name.asChar(),false);
		ChunkWriteInt(&pad);
	}
	ChunkEnd();
}

bool DoAnimDialog(MObject hip,MObject origin)
{
	Hip = hip;
	Origin = origin;
	VeloTracks = 0;

	if(DialogBox(MhInstPlugin,MAKEINTRESOURCE(IDD_EXPORTANIM),hWndMaya,BakeAnimCallBack)) return true;

	return false;
}

void BakeAnimation(int start,int end)
{
	MObject hip,origin;
	MString node;
	MVector HipTranslate;
	MString hipname;
	MString originname;
//	MCommandResult result;

	hip = Hip;
	origin = Origin;

	node = GetNodeName(hip);

	MTime startframe;
	MTime endframe;

	startframe.setValue(start);
	endframe.setValue(end);

	undocnt = 0;

	// Velocity From The Hip
	if(VeloSource == HIP_VELOCITY)
	{
		double hipz,hipx,hipy;
		double originz,originx,originy;

		hipname    = GetNodeName(hip);
		originname = GetNodeName(origin);

		// Set Animation Timer To Start Of Anim
		ExecuteMayaCommand("currentTime %d",start);

		// Get HIP Height Above The Ground Plane
		ExecuteMayaCommand(hipx,"getAttr \"%s.translateX\"",hipname.asChar());
		ExecuteMayaCommand(hipy,"getAttr \"%s.translateY\"",hipname.asChar());
		ExecuteMayaCommand(hipz,"getAttr \"%s.translateZ\"",hipname.asChar());


		// Get The ORIGIN Height Above The Ground Plane For Current Animation
		MString SelectedTracks = "{";
		SelectedTracks += "\"";
		SelectedTracks += hipname;
		SelectedTracks += ".translateX";
		SelectedTracks += "\"";
		SelectedTracks += "}";

		ExecuteMayaCommand("cutKey -t \"%d:%d\" -float \":\"  -hierarchy none -controlPoints 0 -shape 1 %s",
							(int)0 ,
							(int)10000,
							(char*) SelectedTracks.asChar()
							);

		ExecuteMayaCommand("pasteKey -t %d -float 0 -option insert -copies 1 -connect 1 -timeOffset 0 -floatOffset 0 -valueOffset 0 {\"%s\"}",0,originname.asChar());

		SelectedTracks = "{";
		SelectedTracks += "\"";
		SelectedTracks += hipname;
		SelectedTracks += ".translateY";
		SelectedTracks += "\"";
		SelectedTracks += "}";

		ExecuteMayaCommand("cutKey -t \"%d:%d\" -float \":\"  -hierarchy none -controlPoints 0 -shape 1 %s",
							(int)0 ,
							(int)10000,
							(char*) SelectedTracks.asChar()
							);

		ExecuteMayaCommand("pasteKey -t %d -float 0 -option insert -copies 1 -connect 1 -timeOffset 0 -floatOffset 0 -valueOffset 0 {\"%s\"}",0,originname.asChar());

		SelectedTracks = "{";
		SelectedTracks += "\"";
		SelectedTracks += hipname;
		SelectedTracks += ".translateZ";
		SelectedTracks += "\"";
		SelectedTracks += "}";

		ExecuteMayaCommand("cutKey -t \"%d:%d\" -float \":\"  -hierarchy none -controlPoints 0 -shape 1 %s",
							(int)0 ,
							(int)10000,
							(char*) SelectedTracks.asChar()
							);

		ExecuteMayaCommand("pasteKey -t %d -float 0 -option insert -copies 1 -connect 1 -timeOffset 0 -floatOffset 0 -valueOffset 0 {\"%s\"}",0,originname.asChar());

		ExecuteMayaCommand(originx,"getAttr \"%s.translateX\"",originname.asChar());
		ExecuteMayaCommand(originy,"getAttr \"%s.translateY\"",originname.asChar());
		ExecuteMayaCommand(originz,"getAttr \"%s.translateZ\"",originname.asChar());

		ExecuteMayaCommand("undo");
		ExecuteMayaCommand("undo");
		ExecuteMayaCommand("undo");
		ExecuteMayaCommand("undo");
		ExecuteMayaCommand("undo");
		ExecuteMayaCommand("undo");

		HipOffsetX = hipx - originx;
		HipOffsetY = hipy - originy;
		HipOffsetZ = hipz - originz;

		undocnt += 1;

		// Move The Selected Tracks From The HIP to The Origin
		char* strings[9] = {".translateX",".translateY",".translateZ",
							".rotateX",".rotateY",".rotateZ",
							".scaleX","scaleY","scaleZ"};

		if(VeloTracks)
		{
			MString SelectedTracks 	= "{";
			bool 	bComma 			= false;
			long 	tflag 			= VeloTracks;

			for(int i=0;i<9;i++)
			{
				if(tflag&1)
				{
					if(bComma) SelectedTracks += ",";

					SelectedTracks += "\"";
					SelectedTracks += hipname;
					SelectedTracks += strings[i];
					SelectedTracks += "\"";

					bComma = true;
				}

				tflag>>=1;
			}
			SelectedTracks += "}";

			// Cut The Keys From The Hip
			ExecuteMayaCommand("cutKey -t \"%d:%d\" -float \":\"  -hierarchy none -controlPoints 0 -shape 1 %s",
								(int)0 ,
								(int)10000,
								(char*) SelectedTracks.asChar()
								);

			// Paste The Keys To The Origin
			ExecuteMayaCommand("pasteKey -t %d -float 0 -option insert -copies 1 -connect 1 -timeOffset 0 -floatOffset 0 -valueOffset 0 {\"%s\"}",0,originname.asChar());

			undocnt += 2;
		}

		// Translate The Z Track
		if(VeloTracks&ANIM_HASTZ)
		{
			ExecuteMayaCommand("select -r %s",originname.asChar());
			ExecuteMayaCommand("selectKey -add -k %s_translateZ",originname.asChar());
			ExecuteMayaCommand("keyframe -e -iub true -r -o over -vc %f %s_translateZ",-hipz,originname.asChar());
			undocnt += 3;
		}
		else if(bMoveOrigin)
		{
			ExecuteMayaCommand("select -r %s",hipname.asChar());
			ExecuteMayaCommand("selectKey -add -k %s_translateZ",hipname.asChar());
			ExecuteMayaCommand("keyframe -e -iub true -r -o over -vc %f %s_translateZ",-hipz + HipOffsetZ,hipname.asChar());
			undocnt += 3;
		}
	}
}

bool UndoBakeAnim()
{
	MString command;
	MStringArray result;
	int i;
	command.clear();
	command = "undo";

	for(i=0;i<undocnt;i++)
		MGlobal::executeCommand(command,result,true,true);

	return true;
}

CALLBACK BakeAnimCallBack(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	int i;

	HWND hVs;
	HWND hTX;
	HWND hTY;
	HWND hTZ;
	HWND hRX;
	HWND hRY;
	HWND hRZ;
	HWND hSX;
	HWND hSY;
	HWND hSZ;
	HWND hAnimClip;
	MObjectArray objects;
	HWND hExportTo;
	HWND hExportAll;

	hExportTo  = GetDlgItem(hWnd,IDC_DIR);
	hExportAll = GetDlgItem(hWnd,IDC_EXPORTALL);

	hVs = GetDlgItem(hWnd,IDC_VELOCITYSOURCE);
	hAnimClip = GetDlgItem(hWnd,IDC_ANIMCLIP);

	hTX = GetDlgItem(hWnd,IDC_TX);
	hTY = GetDlgItem(hWnd,IDC_TY);
	hTZ = GetDlgItem(hWnd,IDC_TZ);

	hRX = GetDlgItem(hWnd,IDC_RX);
	hRY = GetDlgItem(hWnd,IDC_RY);
	hRZ = GetDlgItem(hWnd,IDC_RZ);

	hSX = GetDlgItem(hWnd,IDC_SX);
	hSY = GetDlgItem(hWnd,IDC_SY);
	hSZ = GetDlgItem(hWnd,IDC_SZ);


	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			bExportAll = false;

			// Now Find All animSplitter Locators
			FindNodesByType(objects,MFn::kPluginLocatorNode,ANIMSPLITTER_ID);

			for(i=0;i<objects.length();i++)
			{
				MObject cname;
				MString clipname;

				FindNodeAttribute(objects[i],"clipName",cname);
				MPlug p(objects[i],cname);
				p.getValue(clipname);

				clipname.toUpperCase();

				SendMessage(hAnimClip,CB_ADDSTRING,0,(LPARAM) clipname.asChar());
				SendMessage(hAnimClip,CB_SETCURSEL,0,0);
			}

			MObjectArray objects;
			MObject origin;
			MObject expdir;
			MString dir;
			FindNodesByType(objects,MFn::kPluginLocatorNode,ANIMLOCATOR_ID);
			origin = objects[0];
			FindNodeAttribute(origin,"exportDirectory",expdir);
			MPlug plg(origin,expdir);
			plg.getValue(dir);

			SetWindowText(hExportTo,dir.asChar());
		}
		break;

		case WM_COMMAND:
		{
			if(LOWORD(wParam)==IDOK)
			{
				char string[256];
				int idx;

				VeloSource = SendMessage(hVs,CB_GETCURSEL,0,0);

				idx = SendMessage(hAnimClip,CB_GETCURSEL,0,0);
				SendMessage(hAnimClip,CB_GETLBTEXT,idx,(LPARAM)string);
				selected = string;

				EndDialog(hWnd,1);
				break;
			}

			if(LOWORD(wParam)==IDCANCEL)
			{
				EndDialog(hWnd,0);
				break;
			}

			if(LOWORD(wParam)==IDC_EXPORTTO)
			{
				MObjectArray objects;
				MObject origin;
				MObject expdir;
				MString dir;
				char directory[MAX_PATH];

				FindNodesByType(objects,MFn::kPluginLocatorNode,ANIMLOCATOR_ID);
				origin = objects[0];
				FindNodeAttribute(origin,"exportDirectory",expdir);
				MPlug plg(origin,expdir);
				plg.getValue(dir);

				strcpy(directory,dir.asChar());

				if(BrowseForDirectory(hWnd,"Export Animation Clips To",directory))
				{
					plg.setValue(directory);
					SetWindowText(hExportTo,directory);
				}

			}

			if(LOWORD(wParam)==IDC_EXPORTALL)
			{
				if(SendMessage(hExportAll,BM_GETCHECK,0,0)==BST_CHECKED)
					bExportAll = true;
				else
					bExportAll = false;
			}
		}
	}

	return false;
}

void AdjustJointArrayDynamic(MObjectArray &joints,MIntArray &children)
{
	// Scan Joint Array And Remove All Joints With No Children
	// Create Child Array First

	unsigned int i;

	children.clear();

	for(i=0;i<joints.length();i++)
		children.append(GetNumTransformChildren(joints[i]));

	for(i=0;i<joints.length();i++)
	{
		// Remove Any Dynamic Joints Totally For The Minute

		MString name;
		MString key = "DYNAMIC";
		GetNodeName(joints[i],name);
		name.toUpperCase();
		MString sub;

		if(key == name.substring(name.length()-key.length(),name.length()))
		{
			Log(0,"Removed **** %s",name.asChar());

			joints.remove(i);
			children[i-1]--;
			children.remove(i);
			i=0;
			continue;
		}

		key = "DYNAMICBEGIN";

		if(key == name.substring(name.length()-key.length(),name.length()))
		{
			Log(0,"Removed **** %s",name.asChar());

			joints.remove(i);
			children[i-1]--;
			children.remove(i);
			i=0;
			continue;
		}

		// Remove Unused End Joints
		if(GetNumTransformChildren(joints[i])==0)
		{
			joints.remove(i);
			children[i-1]--;
			children.remove(i);
			i=0;
			continue;
		}
	}

	for(i=0;i<children.length();i++)
	{
		Log(0,"%s %d",GetNodeName(joints[i]).asChar(),children[i]);
	}

}

void WriteChildHierachyChunk(MObjectArray& a_Origin, MObject& a_Parent, int an_StartFrame, int an_EndFrame)
{
	MString l_name 		 = GetNodeName(a_Parent);
	int 	l_ChildCount = a_Origin.length();

	ChunkBegin(CHUNKID("CHIE"));
	{
		ChunkWriteString((char*)l_name.asChar());
		ChunkWriteInt(&l_ChildCount);

		ExportAnimationForObject(a_Origin[0], an_StartFrame, an_EndFrame, false);
		for ( int i = 1 ; i < l_ChildCount  ; i++ ) {
			ExportAnimationForObject(a_Origin[i], an_StartFrame, an_EndFrame, false);
		}
	}
	ChunkEnd();
}






#define		EXPORTPATH	"Z:\\NEXTGEN\\CUTSCENES\\"
#define		CHARPATH	"Z:\\NEXTGEN\\GAMECHARS\\"



unsigned int STRING_GetHashValue(const char *s)
{
	int h,l,step;
	l = strlen(s);
	h = l;
	step = (l>>6)+1;
	for(;l>0;l-=step)
		h = h^((h<<5)+(h>>2)+(unsigned char)*(s++));
  	return h;
}






void SampleCutCurve(MObject &x,MObject &y,MObject &z,MObject &vis,int bf,int ef,MVector &offset)
{
	MTime startFrame;
	MTime endFrame;
	MTime currentFrame;
 	startFrame.setUnit(MTime::kNTSCFrame);
 	endFrame.setUnit(MTime::kNTSCFrame);
 	currentFrame.setUnit(MTime::kNTSCFrame);
	float value;

	startFrame.setValue(bf);
	endFrame.setValue(ef);

	MFnAnimCurve xc(x);
	MFnAnimCurve yc(y);
	MFnAnimCurve zc(z);	
	MFnAnimCurve vs(vis);

	for(currentFrame=startFrame;currentFrame<=endFrame;currentFrame+=1.0f)
	{
		value = (float) xc.evaluate(currentFrame);
		value += offset.x;
		
		ChunkWriteFloat(&value);
		value = (float) yc.evaluate(currentFrame);
		value += offset.y;
		
		ChunkWriteFloat(&value);
		value = (float) zc.evaluate(currentFrame);
		value += offset.z;
		
		ChunkWriteFloat(&value);
	
		if(!(vis.isNull()))
			value = (float) vs.evaluate(currentFrame);
		else
			value = 1.0f;


		ChunkWriteFloat(&value);
	
	}
}




void ExportObjectCutscenePositions(MObject origin,MObject object,int bf,int ef,MVector &offset)
{
	MObject txobj;
	MObject tyobj;
	MObject tzobj;
	
	int nFrames;

	nFrames = ef - bf;
	
	ChunkWriteInt(&nFrames);

	GetAnimCurveFor(object,MString("translateX"),txobj);
	GetAnimCurveFor(object,MString("translateY"),tyobj);
	GetAnimCurveFor(object,MString("translateZ"),tzobj);
	
	MObject vis;
	vis = MObject::kNullObj;
	GetAnimCurveFor(origin,MString("visibility"),vis);
	SampleCutCurve(txobj,tyobj,tzobj,vis,bf,ef,offset);
}




void SampleCutCurve1(MObject &x,MObject &y,MObject &z,MObject &vis,int bf,int ef,MVector &offset,MObject &tx,MObject &ty,MObject &tz,MObject &rz,MObject &hrz)
{
	MTime startFrame;
	MTime endFrame;
	MTime currentFrame;
 	startFrame.setUnit(MTime::kNTSCFrame);
 	endFrame.setUnit(MTime::kNTSCFrame);
 	currentFrame.setUnit(MTime::kNTSCFrame);
	float xval;
	float yval;
	float zval;
	float value;

	startFrame.setValue(bf);
	endFrame.setValue(ef);

	MFnAnimCurve xc(x);
	MFnAnimCurve yc(y);
	MFnAnimCurve zc(z);	
	MFnAnimCurve vs(vis);

	MFnAnimCurve otx(tx);
	MFnAnimCurve oty(ty);
	MFnAnimCurve otz(tz);	
	MFnAnimCurve orz(rz);
	MFnAnimCurve ohrz(hrz);


	for(currentFrame=startFrame;currentFrame<=endFrame;currentFrame+=1.0f)
	{
		float rotz;

		rotz = 0.0f;

		if(orz.type()!=MFn::kInvalid)
			rotz = orz.evaluate(currentFrame);

//		if(ohrz.type()!=MFn::kInvalid)
//			rotz += ohrz.evaluate(currentFrame);

		xval = (float) xc.evaluate(currentFrame);
		yval = (float) yc.evaluate(currentFrame);
		zval = (float) zc.evaluate(currentFrame);

		float x;
		x = xval;
	
		xval = ( cos(rotz) * x)  - (sin(rotz) * yval);
		yval = ( sin(rotz) * x)  + (cos(rotz) * yval);

		
		xval += offset.x;
		yval += offset.y;
		zval += offset.z;
		
		if(otx.type()!=MFn::kInvalid)
			xval += otx.evaluate(currentFrame);
		if(oty.type()!=MFn::kInvalid)
			yval += oty.evaluate(currentFrame);
		if(otz.type()!=MFn::kInvalid)
			zval += otz.evaluate(currentFrame);
	
		ChunkWriteFloat(&xval);
		ChunkWriteFloat(&yval);
		ChunkWriteFloat(&zval);

		if(!(vis.isNull()))
			value = (float) vs.evaluate(currentFrame);
		else
			value = 1.0f;

		ChunkWriteFloat(&value);
	}
}


void ExportCharCutscenePositions(MObject origin,MObject object,int bf,int ef,MVector &offset)
{
	MObject txobj;
	MObject tyobj;
	MObject tzobj;

	MObject otxobj;
	MObject otyobj;
	MObject otzobj;
	MObject orzobj;
	MObject orzhip;

	int nFrames;

	nFrames = ef - bf;
	
	ChunkWriteInt(&nFrames);

	GetAnimCurveFor(object,MString("translateX"),txobj);
	GetAnimCurveFor(object,MString("translateY"),tyobj);
	GetAnimCurveFor(object,MString("translateZ"),tzobj);
	GetAnimCurveFor(object,MString("rotateZ"),orzhip);



	GetAnimCurveFor(origin,MString("translateX"),otxobj);
	GetAnimCurveFor(origin,MString("translateY"),otyobj);
	GetAnimCurveFor(origin,MString("translateZ"),otzobj);
	GetAnimCurveFor(origin,MString("rotateZ"),orzobj);
	

	MObject vis;
	vis = MObject::kNullObj;
	GetAnimCurveFor(origin,MString("visibility"),vis);
	SampleCutCurve1(txobj,tyobj,tzobj,vis,bf,ef,offset,otxobj,otyobj,otzobj,orzobj,orzhip);
}









void ReadLine(FILE* fp,char* string)
{
	int idx = 0;

	while(!feof(fp))
	{
		fread(&string[idx],1,1,fp);
		
		if(string[idx] == '\n')
		{
			char trim[512];
			
			string[idx] = 0;
				
			for(int i=0;i<strlen(string);i++)
			{
				if(string[i] != 32)
					break;
			}
			
			strcpy(trim,&string[i]);			
			strcpy(string,trim);			
			break;
		}
		
		idx++;
	}
}






void GetANMFile(MString &cname,MString &file)
{
	FILE* fp;
	char Filename[MAX_PATH];
	
	sprintf(Filename,"%s\\%s\\%s.prj",CHARPATH,cname.asChar(),cname.asChar());

	fp = fopen(Filename,"rt");
	
	if(fp)
	{
		char string[256];
		cout << "Opened : " << Filename << endl;

		while(!feof(fp))
		{
			
			fscanf(fp,"%s\n",string);

			if(!strcmp(string,"$fname"))
				break;
		}


		ReadLine(fp,string);

		fclose(fp);

		file = string;
	}
	else
	{
		cout << "ERROR : " << Filename << " Not Found " << endl;
	}
}



void GetHierachyPath(MString &anmFile,MString &hier,MString &skin)
{
	FILE *fp;

	fp = fopen(anmFile.asChar(),"rt");

	if(fp)
	{
		char string[256];
		cout << "Opened : " << anmFile << endl;

		while(!feof(fp))
		{
			fscanf(fp,"%s\n",string);

			if(!strcmp(string,"$fullPath"))
				break;
		}

		ReadLine(fp,string);

		hier =  string;


		while(!feof(fp))
		{
			fscanf(fp,"%s\n",string);

			if(!strcmp(string,"$fullpath"))
				break;
		}

		ReadLine(fp,string);

		skin = string;

		fclose(fp);
	}
	else
	{
		cout << "ERROR : " << anmFile << " Not Found " << endl;
	}
}







void GetMorphTargetPath(MString &anmFile,MString &morph)
{
	FILE *fp;

	fp = fopen(anmFile.asChar(),"rt");

	if(fp)
	{
		char string[256];
		cout << "Opened : " << anmFile << endl;

		while(!feof(fp))
		{
			ReadLine(fp,string);

			if(!strcmp(string,"<Morph Target>"))
				break;
		}

		if(!feof(fp))
		{
			ReadLine(fp,string);      // Skip Version
			fscanf(fp,"%s\n",string); // Skip $fname
			ReadLine(fp,string);	  // Get Path


			morph = string;

			cout << "Morph Target ::  " << morph << endl;
		}	
		else
			cout << "WARNING : No Morph Target In Project" << endl;

		fclose(fp);
		
	}
	else
	{
		cout << "ERROR : " << anmFile << " Not Found " << endl;
	}
}





















int GetTransformChildren(MObject &obj)
{
	int count=0;
	MStatus status;

	MFnDagNode tn( obj, &status );

	for(int i=0;i<(signed)tn.childCount();i++)
	{
		MObject kid=tn.child(i);

		if(kid.apiType()==MFn::kTransform)
			count++;
	}

	return count;
}




bool GetBlendShapes(MObjectArray &blendShapes,MString actname)
{
	MString name;
	MStatus status;

	blendShapes.clear();

	GetObjectArray(blendShapes,MFn::kBlendShape);

	// Filter Blend Shapes

	int l;
	for(l=0;l<blendShapes.length();l++)
	{

		GetNodeName(blendShapes[l],name);

		MObjectArray baseObjects;
		MFnBlendShapeDeformer blend(blendShapes[l],&status);
		blend.getBaseObjects(baseObjects);
		MObject base_node;
		MObject base_object = baseObjects[0];
    
		if (base_object.apiType() != MFn::kTransform)
			GetTransform(base_object, base_node);
		else 
			base_node = base_object;

		GetNodeName(base_node, name);

		// Check Against Character name
		
//		cout << name.substring(0,actname.length()-1).asChar() << " ==  " << actname.asChar() << endl;
/*
		MObject object = blendShapes[l];
		MFnDependencyNode depNode(object);
		MObject weightAttribute;
		weightAttribute = depNode.attribute("weight");
		MFnAttribute weightAttr(weightAttribute);

		MPlug plg(object,weightAttribute);

		int nElements = plg.numElements();
		int nActualElements = 0;

		MString* lookup_name = new MString[nElements];
		int* lookup_table = new int[nElements];

		MIntArray weightIndexList;
		blend.weightIndexList(weightIndexList);

		for (int j=0; j<plg.numElements(); j++)
		{
			MObject weight;
			MPlug elementPlug = plg.elementByLogicalIndex(j);

			MObjectArray targetObjects;

			blend.getTargets ( baseObjects[0], weightIndexList[j], targetObjects);

			if (targetObjects.length())
			{
				MObject node;

				if (targetObjects[0].apiType()!=MFn::kTransform) GetTransform(targetObjects[0], node);
				else node = targetObjects[0];

				GetNodeName(node, name);

				Log(LTINFO,"-%s", name.asChar());

				lookup_name[nActualElements] = name;
				lookup_table[nActualElements] = j;

				nActualElements++;
			}
		}
*/
/*
		if(nActualElements != 7)
		{
			cout << "ERROR : Blendshapes Are Incorrect " << name << endl;

		}
*/

		cout << "-----" << actname << " " << name << endl;

		if((name.substring(0,actname.length()-1) != actname) /*|| (blend.numWeights() != 7) || (nActualElements!=7)*/)
		{
			blendShapes.remove(l);
			l--;
			continue;
		}

//		cout << "BlendShape " << name.asChar() << endl;
	}

//	blendShapes.clear();
//	return false;

	if(blendShapes.length()) return true;

	return false;
}



bool GetMorphTargets(MObjectArray &objects,MString &filename);

extern bool bQuiet;

void RemoveObjects(MObjectArray& joints, MIntArray& remove_types);

void ExportCutScene()
{
	MObject			toplevel;
	MObjectArray	actors;
	MObjectArray	props;
	MStringArray anmFiles;
	MString cutscene;
	char AnimFile[MAX_PATH];
	char AnimName[MAX_PATH];
	MVector csorigin;
	FILE* outfp;

	bQuiet = true;
	bCutscene = true;


	// Get All Top Level Nodes
	
	toplevel = MObject::kNullObj;

	MStatus status;
	MItDag dagIterator( MItDag::kBreadthFirst, MFn::kTransform, &status);

	cout << "*** Find Cutscene Origin ***" << endl;

	for (;!dagIterator.isDone();dagIterator.next())
	{
        MString name;
		MString origin;

		MDagPath dagPath;
        status = dagIterator.getPath(dagPath);
        if ( !status )
		{
            status.perror("MItDag::getPath");
            continue;
        }

		// Break On Top Level
		
		if(dagIterator.depth() > 1)
			break;

		MFnDagNode dagNode(dagPath, &status);
	
		// Find "ORIGIN_CUTSCENENAME"

		name   = dagNode.name();
		origin = name.substring(0,5).toUpperCase();

		if(origin == "ORIGIN")
		{
			cout << dagNode.name() << ": " << dagNode.typeName() << " " << dagIterator.depth() << endl;
				
			toplevel = dagPath.node();
						
			break;
		}
	}
	
	// Check We Have Found The Top Level Node

	if(toplevel != MObject::kNullObj)
	{
		// Now Pick Up All The Character Origins
		
		MString name;
		MString origin;
		MStatus status;
		MItDag dagIterator;

		// Get The Origin Offset (For Those MAPS that have moved)

		MFnTransform tf(toplevel);
		csorigin = tf.translation(MSpace::kTransform);

		cout << "*** Find Actor Origins ***" << endl;
				
		dagIterator.reset(toplevel,MItDag::kBreadthFirst, MFn::kTransform);

		for (;!dagIterator.isDone();dagIterator.next())
		{
			MDagPath dagPath;
			status = dagIterator.getPath(dagPath);

			if ( !status )
			{
	            status.perror("MItDag::getPath");
				continue;
			}

			MFnDagNode dagNode(dagPath, &status);

			// Break On Second Level

			if(dagIterator.depth() < 1)
				continue;

			if(dagIterator.depth() > 1)
				break;
			
			// Find "ORIGIN_CHARNAME"

			name   = dagNode.name();
			origin = name.substring(0,5).toUpperCase();

			if(origin == "ORIGIN")
			{
				cout << dagNode.name() << ": " << dagNode.typeName() << " " << dagIterator.depth() << endl;
				
				actors.append(dagPath.node());
				
				continue;
			
			}
		}
	}
	else
		cout << "Error : Unable To Find Top Level Node" << endl;


	// Now Lets Output The Various Files..
	// Assumes The Scene is PreBaked.

	if(toplevel != MObject::kNullObj)
	{
		char Filename[MAX_PATH];
		int i,j;

		GetNodeName(toplevel,cutscene);
		
		cutscene = cutscene.substring(7,cutscene.length());

		cout << "** Output Data **" << endl;

		sprintf(Filename,"%s\\%s",EXPORTPATH,cutscene.asChar());
			
		CreateDirectory(Filename,NULL);


		sprintf(Filename,"%s\\%s\\%s.out",EXPORTPATH,cutscene.asChar(),cutscene.asChar());

		outfp = fopen(Filename,"w+");

		// Export The Camera
		
		sprintf(Filename,"%s\\%s\\%s.cam",EXPORTPATH,cutscene.asChar(),cutscene.asChar());


		MString file = Filename;

		ExportCamera(file);

		// Do All Actors First

		// Get The Frame Range

		int bf,ef;
		MString name;
				
		MGlobal::executeCommand("playbackOptions -q -min", bf);
		MGlobal::executeCommand("playbackOptions -q -max", ef);

		cout << bf << " " << ef << endl;
	
		// Create Asset Text File

		// Export Actor Positions 
		
		sprintf(Filename,"%s\\%s\\%s.pos",EXPORTPATH,cutscene.asChar(),cutscene.asChar());
		
		ChunkOpen(Filename);

		ChunkBegin(CHUNKID("CPOS"));
		
		int nActors = actors.length();

		ChunkWriteInt(&nActors);

		for(i=0;i<actors.length();i++)
		{
			MStringArray split;
			MString cname;
			GetNodeName(actors[i],cname);

			cname = cname.substring(7,cname.length());

			cout << cname << endl;

			MString temp;

			temp = "__" + cname;
			temp.toLowerCase();

			cout << temp << endl;
			
			ChunkWriteString((char*)temp.asChar(),FALSE);
			

			int HashID = STRING_GetHashValue((char*)temp.asChar());
			ChunkWriteInt(&HashID);
			
			MIntArray types;
			MObjectArray joints;
			types.append(MFn::kJoint);
			GetChildObjects(actors[i],joints,types,true);

			// Export Animation Hash Value

			GetNodeName(actors[i],name);
			name = name.substring(7,name.length());

			sprintf(Filename,"%s_%s",name.asChar(),cutscene.asChar());			

			strupr(Filename);

			cout << Filename << endl;

			HashID = STRING_GetHashValue(Filename);

			ChunkWriteInt(&HashID);

		
			MObjectArray blendShapes;
		
			MString sname;
			split.clear();
			name.split('_',split);
			
			sname = split[0];
			for(int l=1;l<split.length()-1;l++)
			{
				sname += "_";
				sname += split[l];	
			}
			
			cout << "*** " << sname << endl;

			if(GetBlendShapes(blendShapes,sname))
			{
				sprintf(Filename,"%s_%s",sname.asChar(),cutscene.asChar());
				
				HashID = STRING_GetHashValue(Filename);
				ChunkWriteInt(&HashID);

				sprintf(Filename,"%s_%s.tms",sname.asChar(),cutscene.asChar());
				cout << Filename << endl;

				ChunkWriteString(Filename,true);
			}
			else
			{
				int pad = 0;
				ChunkWriteInt(&pad);
				ChunkWriteString("",true);
			
			}
			
			// Now Get The Transform Node For This Object

			ExportCharCutscenePositions(actors[i],joints[0],bf,ef,csorigin);
		
		}

		ChunkEnd();

		ChunkClose();
	
		// Create Dummy Projects For Exporting Each Of The Characters

		MIntArray Flags;
		MObjectArray cactors;

		for(i=0;i<actors.length();i++)
			Flags.append(0);

		cout << "**************************************" << endl;
		
		


		for(i=0;i<actors.length();i++)
		{
			MStringArray names;
			MString cname;
			MStringArray split;
			MString sname;

			names.clear();
			cactors.clear();

			if(!Flags[i])
			{
				split.clear();
				GetNodeName(actors[i],cname);
				cname.split('_',split);
				
				sname = split[1];

				for(int l=2;l<split.length()-1;l++)
				{
					sname += "_";
					sname += split[l];	
				}

				cactors.append(actors[i]);

				Flags[i] = true;

				// Find all Matching Actors
				
				for(j=0;j<actors.length();j++)
				{
					if(!Flags[j])
					{
						MString cname,mname;
						MStringArray csplit;
						csplit.clear();
						GetNodeName(actors[j],cname);
						cname.split('_',csplit);

						mname = csplit[1];

						for(int l=2;l<csplit.length()-1;l++)
						{
							mname += "_";
							mname += csplit[l];	
						}


						if(mname == /*split[1]*/ sname)
						{
							cactors.append(actors[j]);
							Flags[j] = true;	
						}
					}
				}


				cout << "*** Create Project ***" << endl;
				
				

				for(j=0;j<cactors.length();j++)
				{
					MString name;
					MStringArray split;
					MString anmFile;
					MString hieFile;
					MString skinFile;
					char AnmFile[MAX_PATH];

					GetNodeName(cactors[j],name);
					cout << name.asChar() << endl;
				
					// Find Anm File From Character Name				
					
					name.split('_',split);

					MString mname;


					mname = split[1];

					for(int l=2;l<split.length()-1;l++)
					{
						mname += "_";
						mname += split[l];	
					}

					GetANMFile(mname,anmFile);
		
					cout << anmFile.asChar() << endl;
				
					GetHierachyPath(anmFile,hieFile,skinFile);

					cout << hieFile.asChar() << endl;

					MString actname = split[1];

					
					fprintf(outfp,"%s_%s.cal\n",mname.asChar(),cutscene.asChar());

					
					// Create A Project File

					sprintf(Filename,"%s\\%s\\%s.prj",EXPORTPATH,cutscene.asChar(),split[1].asChar());					
					sprintf(AnmFile,"%s\\%s\\%s.anm",EXPORTPATH,cutscene.asChar(),split[1].asChar());	
					
					
					FILE* fp;

					// Create An Anm File
			
					sprintf(Filename,"%s\\%s\\%s.anm",EXPORTPATH,cutscene.asChar(),mname.asChar()/*split[1].asChar()*/);					
					
					if(!j)
						anmFiles.append(Filename);

					fp = fopen(Filename,"wt+");
					fprintf(fp,"<%s>\n    $Version 2\n    <Hierachy>\n        $Version 1\n        $fullPath %s\n    <End>\n    <Skins>\n        $nSkins 1\n",Filename,hieFile.asChar());
					fprintf(fp,"        <Skin>\n");
					fprintf(fp,"            $Version 1\n");
					fprintf(fp,"            $fullpath %s\n",skinFile.asChar());
					fprintf(fp,"        <End>\n");
					fprintf(fp,"    <End>\n");
					fprintf(fp,"    <Animations>\n");

					fprintf(fp,"        $nAnims %d\n",cactors.length());		
								
					for(int k=0;k<cactors.length();k++)
					{
						MString cname;
						MStringArray split;
						
						GetNodeName(cactors[k],cname);
						cname = cname.substring(7,cname.length());
						sprintf(Filename,"%s\\%s\\%s_%s.mea",EXPORTPATH,cutscene.asChar(),cname.asChar(),cutscene.asChar());
		
						sprintf(AnimName,"%s_%s",cname.asChar(),cutscene.asChar());

						GetNodeName(cactors[k],cname);
						cname.split('_',split);
						
						sprintf(AnimFile,"%s",Filename);
						
						strupr(AnimFile);
						strupr(AnimName);
						

						fprintf(fp,"        <Animation>\n");
						fprintf(fp,"            $Version 5\n");
						fprintf(fp,"            $exportfile %s\n",AnimFile);
						fprintf(fp,"            $fname %s\n",AnimName);
						fprintf(fp,"            $alt\n");
						fprintf(fp,"            $flags 0\n");
						fprintf(fp,"            $LH 0\n");
						fprintf(fp,"            $RH 0\n");
						fprintf(fp,"        <End>\n");
					}
	
					fprintf(fp,"    <End>\n");		
								
					fprintf(fp,"    <Scenery>\n        $nScenes 0\n    <End>\n");		
					fprintf(fp,"    <Blend Clips>\n        $nBlendClips 0\n    <End>\n");		
					fprintf(fp,"    <TreeView>\n        $nItems 0\n    <End>\n");		

					
					MString morph;
					MObjectArray blendShapes;
//					GetMorphTargetPath(anmFile,morph);

#ifdef MORPHS
					cout << ">>> " << actname << endl;
					
					GetBlendShapes(blendShapes,actname);
#else
					blendShapes.clear();  //****************
#endif					
					if(blendShapes.length())
					{
						char buffer[256];
						sprintf(buffer,"%s\\%s\\%s_%s.mtg",EXPORTPATH,cutscene.asChar(),actname.asChar(),cutscene.asChar());
						morph = buffer;
						if (!GetMorphTargets(blendShapes,morph)) morph = "";
					}
					
					if(morph.length())
					{
						fprintf(fp,"    <Morph Targets>\n");
						fprintf(fp,"        $NumMorphMeshes 1\n");
						fprintf(fp,"        <Morph Target>\n");
						fprintf(fp,"            $Version 1\n");
						fprintf(fp,"            $Filename %s\n",morph.asChar());
						fprintf(fp,"        <End>\n");
						fprintf(fp,"    <End>\n");
					}
					else
					{
						fprintf(fp,"    <Morph Targets>\n        $NumMorphMeshes 0\n    <End>\n");		
					}
					

					// Get The Morph Sequences

					bool bMorph = false;

					sprintf(Filename,"%s\\%s\\%s_%s.msq",EXPORTPATH,cutscene.asChar(),actname.asChar(),cutscene.asChar());

					fprintf(outfp,"%s_%s.tmt\n",actname.asChar(),cutscene.asChar());


#ifdef MORPHS
					GetBlendShapes(blendShapes,actname);
#endif					
					if(blendShapes.length())
					{
						if(GetMorphSequence(blendShapes,Filename,true))
						{
							cout << Filename << endl;
						
							char out[256];

							sprintf(out,"%s_%s.tms",actname.asChar(),cutscene.asChar());
							
							fprintf(outfp,"%s\n",out);

							bMorph = true;
						}
					}
					
//					bMorph = false;			//*********************


					if(bMorph)
					{
						fprintf(fp,"    <Morph Sequences>\n");
						fprintf(fp,"        $NumMorphSequences 1\n");
						fprintf(fp,"        <Morph Sequence>\n");
						fprintf(fp,"            $Version 1\n");
						fprintf(fp,"            $Filename %s\n",Filename);
						fprintf(fp,"        <End>\n");
						fprintf(fp,"    <End>\n");
					}
					else
					{
						fprintf(fp,"    <Morph Sequences>\n        $NumMorphSequences 0\n    <End>\n");		
					}
					
					fprintf(fp,"<End>\n");		
					fclose(fp);
				}
			}

		}

		cout << "**************************************" << endl;

		
		for(i=0;i<actors.length();i++)
		{
			MString null;

			GetNodeName(actors[i],name);
			
			name = name.substring(7,name.length());
				
			name.toUpperCase();

			sprintf(Filename,"%s\\%s\\%s_%s.mea",EXPORTPATH,cutscene.asChar(),name.asChar(),cutscene.asChar());
		
			strupr(Filename);

			cout << Filename << endl;

			MStringArray split;
			MString cname;
			split.clear();
			GetNodeName(actors[i],cname);
			cname.split('_',split);

			MIntArray types;
			MObjectArray joints;
			types.append(MFn::kJoint);
			GetChildObjects(actors[i],joints,types,true);
			MIntArray childcounts;
			AdjustJointArrayDynamic(joints,childcounts);

			ChunkOpen((char*)Filename);
			
			gCOrigin = actors[i];

			VeloTracks  = 0;
			WriteAnimation(null,joints,bf,ef,null,false);

			ChunkClose();
		}

		
		gCOrigin = MObject::kNullObj;

		// Now Do The Props
		
		// Find The PROPS Group 

		MObject props;
		
		dagIterator.reset(toplevel,MItDag::kBreadthFirst, MFn::kTransform);

		for (;!dagIterator.isDone();dagIterator.next())
		{
		    MString name;
			MString origin;

			MDagPath dagPath;
			status = dagIterator.getPath(dagPath);
			if ( !status )
			{
				status.perror("MItDag::getPath");
				continue;
			}

			// Break On Top Level
		
			if(dagIterator.depth() > 1)
				break;

			MFnDagNode dagNode(dagPath, &status);
	
			// Find "ORIGIN_CUTSCENENAME"

			name   = dagNode.name();
			origin = name.substring(0,7).toUpperCase();

			if(origin == "OBJECTS")
			{
				cout << dagNode.name() << ": " << dagNode.typeName() << " " << dagIterator.depth() << endl;
				
				props = dagPath.node();
							
				break;
			}
		}
	
		

		// Create Each Object Track 

		MIntArray types;
		MObjectArray nodes;
		MIntArray childcounts;

		types.append(MFn::kTransform);
		GetChildObjects(props,nodes,types,false);

		

		for(j=0;j<nodes.length();j++)
		{
			MString file;
			MString oname;
			MObjectArray onodes;

			GetNodeName(nodes[j],oname);

			sprintf(Filename,"%s\\%s\\%s_%s.trk",EXPORTPATH,cutscene.asChar(),oname.asChar(),cutscene.asChar());
			
			file = Filename;
			
			cout << file << endl;
			
			WriteTransformHierachy(nodes[j],file);

			sprintf(Filename,"%s\\%s\\%s_%s.mea",EXPORTPATH,cutscene.asChar(),oname.asChar(),cutscene.asChar());
			
			ChunkOpen((char*)Filename);		
					
			VeloTracks  = 0;

			MString t = "OBJECTS";

			onodes.clear();

			GetChildObjects(nodes[j],onodes,types,true);

			// Remove Any Locator Nodes
			
			MIntArray remove_types;
			remove_types.append(MFn::kPluginLocatorNode);
			RemoveObjects(onodes,remove_types);
		
			cout << "~~~~~~~~~~~~~~~" << endl;
			cout << onodes.length() << endl;

			ObjectWriteAnimation("",onodes,bf,ef,t,onodes[0] , true);

			ChunkClose();
		}


		// Write ANM Files For Objects
		
		for(j=0;j<nodes.length();j++)
		{
			MString oname;
			char trkfile[MAX_PATH];
			FILE* fp;

			GetNodeName(nodes[j],oname);
	
			sprintf(Filename,"%s\\%s\\%s.anm",EXPORTPATH,cutscene.asChar(),oname.asChar());					
			
			anmFiles.append(Filename);
			
			sprintf(trkfile,"%s\\%s\\%s_%s.trk",EXPORTPATH,cutscene.asChar(),oname.asChar(),cutscene.asChar());

			fp = fopen(Filename,"wt+");
			fprintf(fp,"<%s>\n    $Version 2\n    <Hierachy>\n        $Version 1\n        $fullPath %s\n    <End>\n    <Skins>\n        $nSkins 0\n",Filename,trkfile);
			fprintf(fp,"    <End>\n");
			fprintf(fp,"    <Animations>\n");
			fprintf(fp,"        $nAnims %d\n",1);		

			sprintf(AnimFile,"%s\\%s\\%s_%s.mea",EXPORTPATH,cutscene.asChar(),oname.asChar(),cutscene.asChar());
			sprintf(AnimName,"%s_%s",oname.asChar(),cutscene.asChar());

			fprintf(fp,"        <Animation>\n");
			fprintf(fp,"            $Version 5\n");
			fprintf(fp,"            $exportfile %s\n",AnimFile);
			fprintf(fp,"            $fname %s\n",AnimName);
			fprintf(fp,"            $alt\n");
			fprintf(fp,"            $flags 0\n");
			fprintf(fp,"            $LH 0\n");
			fprintf(fp,"            $RH 0\n");
			fprintf(fp,"        <End>\n");




			fprintf(fp,"    <End>\n");		
			fprintf(fp,"    <Scenery>\n        $nScenes 0\n    <End>\n");		
			fprintf(fp,"    <Blend Clips>\n        $nBlendClips 0\n    <End>\n");		
			fprintf(fp,"    <TreeView>\n        $nItems 0\n    <End>\n");		
			fprintf(fp,"    <Morph Targets>\n        $NumMorphMeshes 0\n    <End>\n");
			fprintf(fp,"    <Morph Sequences>\n        $NumMorphSequences 0\n    <End>\n");		
			fprintf(fp,"<End>\n");
			fclose(fp);
		}



		// Export PROP Positions 
		
		sprintf(Filename,"%s\\%s\\%s.xxx",EXPORTPATH,cutscene.asChar(),cutscene.asChar());
		
		ChunkOpen(Filename);

		ChunkBegin(CHUNKID("PPOS"));
		
		int nProps = nodes.length();


//		sprintf(Filename,"OBJECTS_%s",cutscene.asChar());
//		ChunkWriteString(Filename);

		ChunkWriteInt(&nProps);

		for(i=0;i<nodes.length();i++)
		{
			MStringArray split;
			MString cname;
			MObject shape;


			// Write Anim Filename
			GetNodeName(nodes[i],cname);
			sprintf(Filename,"%s",cname.asChar());			

			ChunkWriteString(Filename,true);
			


			MFnDagNode dnode(nodes[i]); 
			shape = dnode.child(0);
			GetNodeName(shape,cname);
				


			sprintf(Filename,"objects__%s",cname.asChar());
			strlwr(Filename);
			cout << Filename << endl;
			ChunkWriteString(Filename,FALSE);

			
			int HashID = STRING_GetHashValue(Filename);
			ChunkWriteInt(&HashID);
			
			// Export Animation Hash Value

			GetNodeName(nodes[i],cname);
			
			sprintf(Filename,"%s_%s",cname.asChar(),cutscene.asChar());			

			strupr(Filename);

			cout << Filename << endl;


			fprintf(outfp,"%s_%s.cal\n",cname.asChar(),cutscene.asChar());
			fprintf(outfp,"%s_%s.chr\n",cname.asChar(),cutscene.asChar());

//			sprintf(Filename,"%s_%s",cname.asChar(),cutscene.asChar());

			HashID = STRING_GetHashValue(Filename);

			ChunkWriteInt(&HashID);

			// Now Get The Transform Node For This Object

			ExportObjectCutscenePositions(shape,shape,bf,ef,csorigin);
		
		}

		ChunkEnd();

		ChunkClose();


		// Now Write Out The Project File The CutScene
		
		sprintf(Filename,"%s\\%s\\%s.prj",EXPORTPATH,cutscene.asChar(),cutscene.asChar());					
		
		FILE* fp;
		fp = fopen(Filename,"wt+");
		
		fprintf(fp,"<%s>\n",Filename);
		fprintf(fp,"    $Version 1\n");
		fprintf(fp,"    $Version 1\n");
		fprintf(fp,"    $HierachyCount %d\n",anmFiles.length());

		for(i=0;i<anmFiles.length();i++)
		{
			fprintf(fp,"    <HierachyChunk>\n");
			fprintf(fp,"        $fname %s\n",anmFiles[i].asChar());
			fprintf(fp,"    <End>\n");
		}

		fprintf(fp,"<End>\n");
		
		fclose(fp);

		fclose(outfp);


	}

	bQuiet = false;
	bCutscene = false;
	gCOrigin = MObject::kNullObj;
}







