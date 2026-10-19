//+============================================================-//
//
// map_msg.h
//
// 03/02 - AD - Created
//
// Map node system messages and events - shared between game
// and script
//
//+============================================================-//

#if !defined(__MAP_MSG_H)
#define		 __MAP_MSG_H

//+============================================================-//
// Node messages + mapping
//+============================================================-//

// VERY VERY VERY IMPORTANT!!!! Do -->*NOT*<-- insert new messages in the middle of the list. They MUST go at the end...


// Messages before NM_MAXOVERRIDES can have script override functions , messages after cannot

//		Message							Maps to
enum
{
		NM_INIT = 0,					// "OnInit"				One time node init
		NM_USERINIT,					// "OnUserInit"			Extra init functions for scripts...
		NM_DAMAGE,						// "OnDamage"			Receives damage
		NM_DESTROY,						// "OnDestroy"			Destroyed (TODO: Suss interaction with OnDamage)
		NM_ACTORCOLLIDE,				// "OnActorCollide"		Actor collision - triggers use this too!
		NM_ACTIVATE,					// "OnActivate"			Becomes active - game state (as opposed to visual / process state)
		NM_UPDATE,						// "OnUpdate"			Every frame

		NM_MAXOVERRIDES,

		NM_CHARENTRY,					// None					Character enters box (for cameras, at the mo)
		NM_CHAREXIT,					// None					Character leaves box

		NM_ANIMATE,
		NM_DRAW,
		NM_RELEASE,

		NM_FXTRIG,						// None					FX trigger event
		NM_LADDER,						// None					Ladder trigger event

		NM_INITRESOURCE,				// None					Initialize resources for something....
		NM_PREUPDATE,					// None					Pre update message
		NM_POSTUPDATE,					// None					Post update message

		NM_WINDOW,						// None					Window trigger event

		NM_NUDGEPOSX,					// None					Nudge node in direction
		NM_NUDGEPOSY,					// None
		NM_NUDGENEGX,					// None					Nudge node in direction
		NM_NUDGENEGY,					// None

		NM_CONVERSATIONSTART,			// None					Converstion Starts (for cameras, at the mo)
		NM_CONVERSATIONEND,				// None					Conversation Ends

		NM_OPEN,						// None					Open trigger event

		NM_CAMERAENTRY,					// None					Camera enters box
		NM_CAMERAEXIT,					// None					Camera exits box

		NM_OBJCOLLIDE,					// None					Something has hit an object
		NM_PICKUP,						// None					Something has hit a pickup
		NM_REMOVE,						// None					Remove the object from the world

		NM_SETOBJECTFLAGS,				// None					Set the specified ObjFlags bits of a MAP_OBJNODE (OF_XXX)
		NM_CLEAROBJECTFLAGS,			// None					Clear the specified flags

		NM_PLAYERCOLLIDE,				// None					Player collides

		NM_BGCOLLIDE,					// None					Someting collides with background

		NM_STARTTIMER,					// None					Start node timer (not implemented yet)
		NM_STARTTIMERRANGE,				// None					Start node timer with min max random range
		NM_TIMEREVENT,					// None					Node timer times out (not implemented)

		NM_PIPESWING,					// None					Pipeswing trigger (like ladder trigger)
		NM_WALLCLIMB,					// None					Wall climb trigger (like ladder trigger and pipeswing trigger!)

		NM_DISPATCHEVENT,				// None					Dispatch an event group (for triggers only!)

		NM_STARTPAUSETIMER,				// None					Start a pause timer , and hold the event list pointer until the timer times out
		NM_STARTPAUSETIMERRANGE,		// None					As above, but with min max random range

		NM_ENDPAUSELIST,				// None					A paused list has finished (can also be used to halt paused list)

		NM_PLAYEROPEN,					// None					Pre open event - player must get into position first!
		NM_CLOSE,						// None					Close event
		NM_ANIMFINISH,					// None					Anim finish event

		NM_PUZZLECLEAR,					// None					Puzzle cleared event
		NM_USEPUZZLE_A,					// None					Puzzle item used on node
		NM_USEPUZZLE_B,					// None					Puzzle item used on node
		NM_USEPUZZLE_C,					// None					Puzzle item used on node
		NM_USEPUZZLE_D,					// None					Puzzle item used on node

		NM_UNLOCK,						// None					Guess
		NM_PLY_OBJEVENT,				// None					Object event signals player control

		NM_SWITCHON,					// None					As OPEN , PLAYEROPEN , CLOSE
		NM_SWITCHOFF,					// None
		NM_PLAYERSWITCHON,				// None

		NM_SWITCHEDON,					// None					Sent to object when switch on / off sequence finishes
		NM_SWITCHEDOFF,					// None

		NM_UPGRADE_UPPERBODY,			// None					Upgrades - uParam is ABSOLUTE upgrade level
		NM_UPGRADE_LOWERBODY,
		NM_UPGRADE_GRIP,
		NM_UPGRADE_JUMP,

		NM_TAKE,						// None					Take (replaces search, as search is combined with open)
		NM_TAKEFINISH,					// None					Take has finished (on NM_ANIMFINISH)

		NM_GIVEITEM,					// None					Give item to player
		NM_PLAYERUNLOCK,				// None					Make player do unlock anim

		NM_DEACTIVATE,					// None					Clear NF_ACTIVE flag

		NM_OPENFINISH,					// None					Open has finished (on NM_ANIMFINISH)
		NM_CLOSEFINISH,					// None					Close has finished (on NM_ANIMFINISHED)

		NM_VOICEPROMPT,					// None					Voice prompt

		NM_STARTWATERRAISE,				// None					Start water node surface raising
		NM_STARTWATERLOWER,				// None					Start water node surface lowering
		NM_STOPWATER,					// None					Stop  water node surface
		NM_SHOWWATER,					// None					Make water visible
		NM_HIDEWATER,					// None					Make water invisible

		NM_SETPUZZLECOUNT,				// None					Set the puzzle count on an object

		NM_FLIPROOM_RESET,				// None					Flip to 'initial' room
		NM_FLIPROOM0,					// None					Flip to first flip map
		NM_FLIPROOM1,					// None					And so on...
		NM_FLIPROOM2,					// None
		NM_FLIPROOM3,					// None

		NM_PLAYANIM,					// None					Play named anim

		NM_SETROTX,						// None					Set angles , integer values in degrees
		NM_SETROTY,						// None
		NM_SETROTZ,						// None

		NM_USER0,						// None					Custom user messges - use them for owt!
		NM_USER1,						// None
		NM_USER2,						// None
		NM_USER3,						// None
		NM_USER4,						// None
		NM_USER5,						// None
		NM_USER6,						// None
		NM_USER7,						// None

		NM_DAMAGESTAGE,					// None

		NM_LIFTPLATFORM,				// None					Character / node is on moving platform trigger
		NM_TIMEDACTIVATE,				// None					Activate a node for a set time

		NM_PLAYANIM_SLOT0,				// None					Play anims in door / switch slot 0 -> 3 (maps to some of the open anims)
		NM_PLAYANIM_SLOT1,				// None
		NM_PLAYANIM_SLOT2,				// None
		NM_PLAYANIM_SLOT3,				// None

		NM_DEFLECT,						// None					Deflect a node
		NM_ENDLEVEL,					// None					End the level

		NM_ANIMEVENT0,					// None					Animation event
		NM_ANIMEVENT1,					// None
		NM_ANIMEVENT2,					// None
		NM_ANIMEVENT3,					// None
		NM_ANIMEVENT4,					// None
		NM_ANIMEVENT5,					// None
		NM_ANIMEVENT6,					// None
		NM_ANIMEVENT7,					// None

		NM_CUEMUSIC,					// None					Cue audio

		NM_SAVESTATUS,					// ?					Save status (save game)
		NM_LOADSTATUS,					// ?					Load status (load game)

		NM_ANIMFINISH_SLOT0,			// None					Sent when anim on slot 0-3 finishes
		NM_ANIMFINISH_SLOT1,			// None					
		NM_ANIMFINISH_SLOT2,			// None					
		NM_ANIMFINISH_SLOT3,			// None					

		NM_PUSHEVENT,					// None					Sent to object during push/pull stuff 

		NM_TIMEDREMOVE,					// None					Remove object on time out
		NM_ACTIVATEONTIMER,				// None					Activate object on time out

		NM_CUTSCENE,					// None
		NM_HEADLOOK,					// None
		NM_LEDGESHIMMY,					// None

		NM_STARTAUDIOEVENT,				// None
		NM_STOPAUDIOEVENT,				// None
		NM_AUDIOEVENTFADE,				// None					Fade to uParam volume

		NM_LOCKPLAYER,					// None					Lock player control
		NM_UNLOCKPLAYER,				// None
		NM_HARMPLAYER,					// None

		NM_ZONESWITCH,					// None					Normal zone switch (same as charentry -> zonemanager)
		NM_ZONESWITCH_DOOR,				// None					As above, but on door...

		NM_DISABLETRIGGER,				// None					Disable action triggers
		NM_ENABLETRIGGER,				// None					Enable	   "      "

		NM_HIDEOBJECT,					// None					Hide + disable process on object
		NM_SHOWOBJECT,					// None					
		NM_CUTSCENE_END,				// None					Sent to cutscene trigger when a cutscene ends

		NM_SETXVEL,						// None					Set object velocities
		NM_SETYVEL,						// 
		NM_SETZVEL,						// 

		NM_SETVELDECAY,					// None					Set velocity decay rate
		NM_LIFTSIGNAL,					// None					Signal lift area

		NM_INCREMENT_UVAR,				// None					Increment uParam user value
		NM_BLAST,						// None					Radial blast
		NM_PLAYANIMSLOT_UVAR,			// None					Play anim in slot referenced by userVar[uParam]
		NM_PLAYANIMSLOT,				// None					Play anim in slot referenced by uParam

		NM_ANIMFINISH_SLOT4,			// None					Sent when anim on slot 4-11 finishes
		NM_ANIMFINISH_SLOT5,			// None					
		NM_ANIMFINISH_SLOT6,			// None					
		NM_ANIMFINISH_SLOT7,			// None					
		NM_ANIMFINISH_SLOT8,			// None					
		NM_ANIMFINISH_SLOT9,			// None					
		NM_ANIMFINISH_SLOT10,			// None					
		NM_ANIMFINISH_SLOT11,			// None					

		NM_CLEAR_UVAR,					// None					Clear userVariable[uParam]
		NM_INVALIDATE_UVAR,				// None					Set userVariable[uParam] to -1

		NM_INVISIBLE,					// None					Send to character, make it invisible (to the game LOGIC - not to the eye...)

		NM_DIRECT_ANIMEVENT0,			// None					Animation event - sent directly to pre set target node
		NM_DIRECT_ANIMEVENT1,			// None
		NM_DIRECT_ANIMEVENT2,			// None
		NM_DIRECT_ANIMEVENT3,			// None
		NM_DIRECT_ANIMEVENT4,			// None
		NM_DIRECT_ANIMEVENT5,			// None
		NM_DIRECT_ANIMEVENT6,			// None
		NM_DIRECT_ANIMEVENT7,			// None

		NM_PLAYANIMSLOT_UVAR_OFFSET,	// None					16:16 parameter - uVar:Offset - plays animslot userVar[uParam] + offset
		NM_WARP_PLAYER,					// None					Warp player to another node pos

		NM_SETWATER_AMP,				// None					Set amplitude on dynamic water
		NM_SETWATER_FREQ,				// None					Set frequency on dynamic water

		NM_AUDIO_DEACTIVATE_CHAIN,		// None					Something nigel asked for...

		NM_DECREMENT_UVAR,				// None					Decrement uParam user value
		NM_UVAR_MAX,					// None					Clip a uvar to max value (16:16 uVar:value)
		NM_UVAR_MIN,					// None					Clip a uvar to min value (16:16 uVar:value)

		NM_SETNODEFLAGS,				// None
		NM_CLEARNODEFLAGS,				// None

		NM_PRECUTSCENE,					// None					Sent to a trigger just before cutscene plays
		NM_INSTANT_CLOSE,				// None					Send to a door - close instantly

		NM_DIARY_ENTRY,					// None

		NM_MAXX
};

// Text defs are done like this to keep them in the same file - it's worldedit only, so it doesn't really matter

#if defined (WORLDEDIT)

#if !defined(__MM_MSGDEFS)
#define		 __MM_MSGDEFS

struct	MessageDef
{
	char	*Str;
	bool	bValid;
};

static MessageDef mapMsgDefs[NM_MAXX] =
{
	{"NM_INIT"				,true},
	{"NM_USERINIT"			,true},
	{"NM_DAMAGE"			,true},
	{"NM_DESTROY"			,true},
	{"NM_ACTORCOLLIDE"		,true},
	{"NM_ACTIVATE"			,true},
	{"NM_UPDATE"			,true},

	{"--Delimiter--"		,false},

	{"NM_CHARENTRY"			,true},
	{"NM_CHAREXIT"			,true},

	{"NM_ANIMATE"			,false},
	{"NM_DRAW"				,false},
	{"NM_RELEASE"			,false},

	{"NM_FXTRIG"			,true},
	{"NM_LADDER"			,true},

	{"NM_INITRESOURCE"		,false},
	{"NM_PREUPDATE"	  		,false},
	{"NM_POSTUPDATE"  		,false},

	{"NM_WINDOW"			,true },

	{"NM_NUDGEPOSX"			,true },
	{"NM_NUDGEPOSY"			,true },
	{"NM_NUDGENEGX"			,true },
	{"NM_NUDGENEGY"			,true },

	{"NM_CONVERSATIONSTART"	,false},
	{"NM_CONVERSATIONEND"	,false},

	{"NM_OPEN"				,true},

	{"NM_CAMERAENTRY"		,true},
	{"NM_CAMERAEXIT"		,true},

	{"NM_OBJCOLLIDE"		,false},
	{"NM_PICKUP"			,false},
	{"NM_REMOVE"			,false},

	{"NM_SETOBJECTFLAGS"	,false},
	{"NM_CLEAROBJECTFLAGS"	,false},

	{"NM_PLAYERCOLLIDE"		,false},

	{"NM_BGCOLLIDE"			,false},

	{"NM_STARTTIMER"		,false},
	{"NM_STARTTIMERRANGE"	,false},
	{"NM_TIMEREVENT"		,false},

	{"NM_PIPESWING"			,false},
	{"NM_WALLCLIMB"			,false},

	{"NM_DISPATCHEVENT"		,false},

	{"NM_STARTPAUSETIMER"		,false},
	{"NM_STARTPAUSETIMERRANGE"	,false},

	{"NM_ENDPAUSELIST"			,false},

	{"NM_PLAYEROPEN"			,true},
	{"NM_CLOSE"					,true},
	{"NM_ANIMFINISH"			,false},

	{"NM_PUZZLECLEAR"			,false},
	{"NM_USEPUZZLE_A"			,false},
	{"NM_USEPUZZLE_B"			,false},
	{"NM_USEPUZZLE_C"			,false},
	{"NM_USEPUZZLE_D"   		,false},

	{"NM_UNLOCK"   				,false},
	{"NM_PLY_OBJEVENT"			,false},

	{"NM_SWITCHON"				,false},
	{"NM_SWITCHOFF"				,false},
	{"NM_PLAYERSWITCHON"		,false},

	{"NM_SWITCHEDON"			,false},
	{"NM_SWITCHEDOFF"			,false},

	{"NM_UPGRADE_UPPERBODY"		,false},
	{"NM_UPGRADE_LOWERBODY"		,false},
	{"NM_UPGRADE_GRIP"			,false},
	{"NM_UPGRADE_JUMP"			,false},

	{"NM_TAKE"					,false},
	{"NM_TAKEFINISH"			,false},

	{"NM_GIVEITEM"				,false},
	{"NM_PLAYERUNLOCK"			,false},

	{"NM_DEACTIVATE"			,false},

	{"NM_OPENFINISH"	 		,false},
	{"NM_CLOSEFINISH"	 		,false},

	{"NM_VOICEPROMPT"			,false},

	{"NM_STARTWATERRAISE"		,false},
	{"NM_STARTWATERLOWER"		,false},
	{"NM_STOPWATER"				,false},
	{"NM_SHOWWATER"				,false},
	{"NM_HIDEWATER"				,false},

	{"NM_SETPUZZLECOUNT"		,false},

	{"NM_FLIPROOM_RESET"		,false},
	{"NM_FLIPROOM0"				,false},
	{"NM_FLIPROOM1"   			,false},
	{"NM_FLIPROOM2"	   			,false},
	{"NM_FLIPROOM3"	   			,false},

	{"NM_PLAYANIM"	   			,false},

	{"NM_SETROTX"	   			,false},
	{"NM_SETROTY"	   			,false},
	{"NM_SETROTZ"	   			,false},

	{"NM_USER0"	   			,false},
	{"NM_USER1"	   			,false},
	{"NM_USER2"	   			,false},
	{"NM_USER3"	   			,false},
	{"NM_USER4"	   			,false},
	{"NM_USER5"	   			,false},
	{"NM_USER6"	   			,false},
	{"NM_USER7"	   			,false},

	{"NM_DAMAGESTAGE"	   	,false},

	{"NM_LIFTPLATFORM"	   	,false},
	{"NM_TIMEDACTIVATE"	   	,false},

	{"NM_PLAYANIM_SLOT0",false},
	{"NM_PLAYANIM_SLOT1",false},
	{"NM_PLAYANIM_SLOT2",false},
	{"NM_PLAYANIM_SLOT3",false},

	{"NM_DEFLECT"	,false},
	{"NM_ENDLEVEL"	,false},

	{"NM_ANIMEVENT0", false},
	{"NM_ANIMEVENT1", false},
	{"NM_ANIMEVENT2", false},
	{"NM_ANIMEVENT3", false},
	{"NM_ANIMEVENT4", false},
	{"NM_ANIMEVENT5", false},
	{"NM_ANIMEVENT6", false},
	{"NM_ANIMEVENT7", false},

	{"NM_CUEMUSIC",   false},

	{"NM_SAVESTATUS",   false},
	{"NM_LOADSTATUS",   false},

	{"NM_ANIMFINISH_SLOT0",false},
	{"NM_ANIMFINISH_SLOT1",false},
	{"NM_ANIMFINISH_SLOT2",false},
	{"NM_ANIMFINISH_SLOT3",false},

	{"NM_PUSHEVENT"		  ,false},

	{"NM_TIMEDREMOVE"	  ,false},
	{"NM_ACTIVATEONTIMER" ,false},

	{"NM_CUTSCENE" ,false},
	{"NM_HEADLOOK" ,false},
	{"NM_LEDGESHIMMY" ,false},

	{"NM_STARTAUDIOEVENT",false},
	{"NM_STOPAUDIOEVENT" ,false},
	{"NM_AUDIOEVENTFADE" ,false},

	{"NM_LOCKPLAYER"	,false},
	{"NM_UNLOCKPLAYER"	,false},
	{"NM_HARMPLAYER"	,false},

	{"NM_ZONESWITCH"		,false},
	{"NM_ZONESWITCH_DOOR"	,false},

	{"NM_DISABLETRIGGER"	,false},
	{"NM_ENABLETRIGGER"		,false},

	{"NM_HIDEOBJECT" ,false},
	{"NM_SHOWOBJECT" ,false},

	{"NM_CUTSCENE_END" ,false},

	{"NM_SETXVEL"	  ,false},
	{"NM_SETYVEL"	  ,false},
	{"NM_SETZVEL"	  ,false},
	{"NM_SETVELDECAY" ,false},

	{"NM_LIFTSIGNAL" ,false},

	{"NM_INCREMENT_UVAR" ,false},
	{"NM_BLAST"			 ,false},

	{"NM_PLAYANIMSLOT_UVAR",false},
	{"NM_PLAYANIMSLOT"     ,false},

	{"NM_ANIMFINISH_SLOT4",false},
	{"NM_ANIMFINISH_SLOT5",false},
	{"NM_ANIMFINISH_SLOT6",false},
	{"NM_ANIMFINISH_SLOT7",false},
	{"NM_ANIMFINISH_SLOT8",false},
	{"NM_ANIMFINISH_SLOT9",false},
	{"NM_ANIMFINISH_SLOT10",false},
	{"NM_ANIMFINISH_SLOT11",false},

	{"NM_CLEAR_UVAR",false},
	{"NM_INVALIDATE_UVAR",false},

	{"NM_INVISIBLE",false},

	{"NM_DIRECT_ANIMEVENT0",false},
	{"NM_DIRECT_ANIMEVENT1",false},
	{"NM_DIRECT_ANIMEVENT2",false},
	{"NM_DIRECT_ANIMEVENT3",false},
	{"NM_DIRECT_ANIMEVENT4",false},
	{"NM_DIRECT_ANIMEVENT5",false},
	{"NM_DIRECT_ANIMEVENT6",false},
	{"NM_DIRECT_ANIMEVENT7",false},

	{"NM_PLAYANIMSLOT_UVAR_OFFSET",false},
	{"NM_WARP_PLAYER"			  ,false},

	{"NM_SETWATER_AMP"	,false},
	{"NM_SETWATER_FREQ"	,false},

	{"NM_AUDIO_DEACTIVATE_CHAIN",false},

	{"NM_DECREMENT_UVAR",false},
	{"NM_UVAR_MAX"		,false},
	{"NM_UVAR_MIN"		,false},	 

	{"NM_SETNODEFLAGS"	,false},
	{"NM_CLEARNODEFLAGS",false},

	{"NM_PRECUTSCENE"	,false},

	{"NM_INSTANT_CLOSE"	,false},
	{"NM_DIARY_ENTRY"	,false},
};

#endif
#endif

#endif	// __MAP_MSG_H

