#include "stdafx.h"


string Reverb_preset (unsigned int ReverbID)
{
	stringstream out;
	out << ReverbID;
	if (ReverbID > 18)
		out << " - None";

	switch (ReverbID)
	{
	case(0):
		out << " - Default";
		break;
	case(1):
		out << " - Room small";
		break;
	case(2):
		out << " - Room medium";
		break;
	case(3):
		out << " - Room carpeted";
		break;
	case(4):
		out << " - Pipe medium";
		break;
	case(5):
		out << " - Pipe small";
		break;
	case(6):
		out << " - Hall large";
		break;
	case(7):
		out << " - Hall medium";
		break;
	case(8):
		out << " - Outside alley";
		break;
	case(9):
		out << " - Outside street";
		break;
	case(10):
		out << " - Small corridor";
		break;
	case(11):
		out << " - Large corridor";
		break;
	case(12):
		out << " - Underwater";
		break;
	case(13):
		out << " - Large pipe";
		break;
	case(14):
		out << " - Concrete sewer";
		break;
	case(15):
		out << " - Medium tiled room";
		break;
	case(16):
		out << " - Stairwell";
		break;
	case(17):
		out << " - Cave large";
		break;
	case(18):
		out << " - Cave medium";
	}
	return out.str();
}