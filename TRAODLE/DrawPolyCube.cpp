/*------------------------------------------------------------------------------------------------------------------
Dati due vertici di un bounding box, restituisce un PolyCube per Maya
INPUT: string name, string parent, string layer, string material_name, Vec3 BBmin, Vec3 BBmax, bool visible
OUTPUT: PolyCube output
------------------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Classes.h"
#include "MA/MA_Classes.h"


PolyCube DrawPolyCube (string name, string parent, string layer, string material_name, Vec3 BBmin, Vec3 BBmax, bool visible, unsigned int VC_ARGB)
{
	PolyCube output;
	output.name = name;
	output.parent = parent;
	output.layer = layer;
	output.material_name = material_name;
	output.Visible = visible;
	output.VC_ARGB = VC_ARGB;

	// Calcola le dimensioni del box
	Vec3 size = BBmax - BBmin;

	// La scala è semplicemente la dimensione del box
	output.scale_flag = true;
	output.sX = size.x;
	output.sY = size.y;
	output.sZ = size.z;

	// Il centro del box è la traslazione necessaria
	// (il box unitario è centrato in (0, 0, 0))
	output.translate_flag = true;
	output.tX = BBmin.x + size.x * 0.5f;
	output.tY = BBmin.y + size.y * 0.5f;
	output.tZ = BBmin.z + size.z * 0.5f;

	return output;
}