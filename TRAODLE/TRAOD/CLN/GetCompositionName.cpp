#include "stdafx.h"


string GetCompositionName (unsigned short compositionID)
{
	string out = "Stone1";
	switch (compositionID)
	{
	case 0:
		out = "Stone1";
		break;
	case 1:
		out = "Stone2";
		break;
	case 2:
		out = "Stone3";
		break;
	case 3:
		out = "Marble1";
		break;
	case 4:
		out = "Marble2";
		break;
	case 5:
		out = "Marble3";
		break;
	case 6:
		out = "Gravel1";
		break;
	case 7:
		out = "Gravel2";
		break;
	case 8:
		out = "Gravel3";
		break;
	case 9:
		out = "Sand1";
		break;
	case 10:
		out = "Sand2";
		break;
	case 11:
		out = "Sand3";
		break;
	case 12:
		out = "Mud1";
		break;
	case 13:
		out = "Mud2";
		break;
	case 14:
		out = "Mud3";
		break;
	case 15:
		out = "Grass1";
		break;
	case 16:
		out = "Grass2";
		break;
	case 17:
		out = "Grass3";
		break;
	case 18:
		out = "Tall_Grass1";
		break;
	case 19:
		out = "Tall_Grass2";
		break;
	case 20:
		out = "Tall_Grass3";
		break;
	case 21:
		out = "Carpet1";
		break;
	case 22:
		out = "Carpet2";
		break;
	case 23:
		out = "Carpet3";
		break;
	case 24:
		out = "Wood1";
		break;
	case 25:
		out = "Wood2";
		break;
	case 26:
		out = "Wood3";
		break;
	case 27:
		out = "Creaky_Wood1";
		break;
	case 28:
		out = "Creaky_Wood2";
		break;
	case 29:
		out = "Creaky_Wood3";
		break;
	case 30:
		out = "Metal1";
		break;
	case 31:
		out = "Metal2";
		break;
	case 32:
		out = "Metal3";
		break;
	case 33:
		out = "Creaky_Metal1";
		break;
	case 34:
		out = "Creaky_Metal2";
		break;
	case 35:
		out = "Creaky_Metal3";
		break;
	case 36:
		out = "Rock1";
		break;
	case 37:
		out = "Rock2";
		break;
	case 38:
		out = "Rock3";
		break;
	case 39:
		out = "Unsafe_Rock1";
		break;
	case 40:
		out = "Unsafe_Rock2";
		break;
	case 41:
		out = "Unsafe_Rock3";
		break;
	case 42:
		out = "Water_Puddle1";
		break;
	case 43:
		out = "Water_Puddle2";
		break;
	case 44:
		out = "Water_Puddle3";
		break;
	case 45:
		out = "Wet_Wood";
		break;
	case 46:
		out = "Wet_Metal";
		break;
	case 47:
		out = "Wet_Stone";
		break;
	case 48:
		out = "Snow1";
		break;
	case 49:
		out = "Snow2";
		break;
	case 50:
		out = "Snow3";
		break;
	case 51:
		out = "Rubber";
		break;
	case 52:
		out = "Skip";
		break;
	case 53:
		out = "Glass_Panel";
		break;
	case 54:
		out = "Ice";
		break;
	case 55:
		out = "Wet_Gravel";
		break;
	case 56:
		out = "Wet_Carpet";
		break;
	case 57:
		out = "Wet_Grass";
		break;
	case 58:
		out = "Wet_Sand";
		break;
	case 59:
		out = "Wet_Rubber";
		break;
	case 60:
		out = "Wet_Skip";
		break;
	default:
		out = "Stone1";
		break;
	}
	return out;
}