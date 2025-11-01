#include "stdafx.h"


string CLN_GetAttributeName (unsigned int composition, unsigned int mapping, unsigned int bitattrib)
{
	unsigned int DEADLY = 0b000000000001;			// bit 0
	unsigned int HARMFUL = 0b000000000010;			// bit 1
	unsigned int CLIMBABLE = 0b000000000100;		// bit 2
	unsigned int MONKEY_SWING = 0b000000001000;		// bit 3
	unsigned int SHOT_PERMEABLE = 0b000000010000;	// bit 4
	unsigned int VIEW_PERMEABLE = 0b000000100000;	// bit 5
	unsigned int WALL_CLIMB = 0b000001000000;		// bit 6
	unsigned int STAIRS = 0b000010000000;			// bit 7
	unsigned int SLIDE = 0b000100000000;			// bit 8
	unsigned int NO_SLIDE = 0b001000000000;			// bit 9
	unsigned int CAMERA_PERMEABLE = 0b010000000000; // bit 10
	unsigned int NO_GRAB = 0b100000000000;			// bit 11

	string out = "";
	string composition_string = "_";
	switch (composition)
	{
	case 0:
	case 1:
	case 2:
	case 51:
	case 52:
	case 53:
	case 54:
		composition_string.append("stone");
		break;
	case 3:
	case 4:
	case 5:
		composition_string.append("marble");
		break;
	case 6:
	case 7:
	case 8:
		composition_string.append("gravel");
		break;
	case 9:
	case 10:
	case 11:
		composition_string.append("sand");
		break;
	case 12:
	case 13:
	case 14:
		composition_string.append("mud");
		break;
	case 15:
	case 16:
	case 17:
		composition_string.append("grass");
		break;
	case 18:
	case 19:
	case 20:
		composition_string.append("tallgrass");
		break;
	case 21:
	case 22:
	case 23:
		composition_string.append("carpet");
		break;
	case 24:
	case 25:
	case 26:
		composition_string.append("wood");
		break;
	case 27:
	case 28:
	case 29:
		composition_string.append("creakywood");
		break;
	case 30:
	case 31:
	case 32:
		composition_string.append("metal");
		break;
	case 33:
	case 34:
	case 35:
		composition_string.append("creakymetal");
		break;
	case 36:
	case 37:
	case 38:
		composition_string.append("rock");
		break;
	case 39:
	case 40:
	case 41:
		composition_string.append("unsaferock");
		break;
	case 42:
	case 43:
	case 44:
		composition_string.append("waterpuddle");
		break;
	case 45:
		composition_string.append("wetwood");
		break;
	case 46:
		composition_string.append("wetmetal");
		break;
	case 47:
	case 59:
	case 60:
		composition_string.append("wetstone");
		break;
	case 48:
	case 49:
	case 50:
		composition_string.append("snow");
		break;
	case 55:
		composition_string.append("wetgravel");
		break;
	case 56:
		composition_string.append("wetcarpet");
		break;
	case 57:
		composition_string.append("wetgrass");
		break;
	case 58:
		composition_string.append("wetsand");
		break;
	default:
		composition_string.append("stone");
		break;
	}



	switch (mapping)
	{
	case 0:				// Mapping: WALL
		out.assign("wall");
		if (bitattrib & CAMERA_PERMEABLE)		// Bitattrib: CAMERA_PERMEABLE
			out.append("_CAMERAP");
		if (bitattrib & WALL_CLIMB)				// Bitattrib: WALL_CLIMB
			out.append("_WALLCLIMB");
		if (bitattrib & VIEW_PERMEABLE)			// Bitattrib: VIEW_PERMEABLE
			out.append("_VIEWP");
		if (bitattrib & SHOT_PERMEABLE)			// Bitattrib: SHOT_PERMEABLE
			out.append("_SHOTP");
		if (bitattrib & CLIMBABLE)				// Bitattrib: CLIMBABLE
			out.append("_CLIMBABLE");
		break;
	case 1:				// Mapping: FLOOR
		out.assign("floor");
		if (bitattrib & DEADLY)					// Bitattrib: DEADLY
		{
			out.append("_DEADLY");
			break;
		}
		out.append(composition_string);
		if (bitattrib & NO_GRAB)				// Bitattrib: NO_GRAB
			out.append("_NOGRAB");
		if (bitattrib & CAMERA_PERMEABLE)		// Bitattrib: CAMERA_PERMEABLE
			out.append("_CAMERAP");
		if (bitattrib & NO_SLIDE)				// Bitattrib: NO_SLIDE
			out.append("_NOSLIDE");
		if (bitattrib & SLIDE)					// Bitattrib: SLIDE
			out.append("_SLIDE");
		if (bitattrib & STAIRS)					// Bitattrib: STAIRS
			out.append("_STAIRS");
		if (bitattrib & VIEW_PERMEABLE)			// Bitattrib: VIEW_PERMEABLE
			out.append("_VIEWP");
		if (bitattrib & SHOT_PERMEABLE)			// Bitattrib: SHOT_PERMEABLE
			out.append("_SHOTP");
		if (bitattrib & HARMFUL)				// Bitattrib: HARMFUL
			out.append("_HARMFUL");
		break;
	case 2:				// Mapping: CEILING
		out.assign("ceiling");
		if (bitattrib & WALL_CLIMB)				// Bitattrib: WALL_CLIMB
			out.append("_WALLCLIMB");
		if (bitattrib & VIEW_PERMEABLE)			// Bitattrib: VIEW_PERMEABLE
			out.append("_VIEWP");
		if (bitattrib & SHOT_PERMEABLE)			// Bitattrib: SHOT_PERMEABLE
			out.append("_SHOTP");
		if (bitattrib & MONKEY_SWING)			// Bitattrib: MONKEY_SWING
			out.append("_MONKEYSWING");
		if (bitattrib & CLIMBABLE)				// Bitattrib: CLIMBABLE
			out.append("_CLIMBABLE");
		break;
	case 3:
		if (bitattrib & MONKEY_SWING)			// Bitattrib: MONKEY_SWING
			out.assign("ceiling_MONKEYSWING");
		else
			if (bitattrib & CLIMBABLE)			// Bitattrib: CLIMBABLE
				out.assign("wall_CLIMBABLE");
			else
			{
				out.assign("floor");
				out.append(composition_string);
			}
	}

	return out;
}