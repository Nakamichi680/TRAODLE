#include "stdafx.h"
#include "MA/MA_Classes.h"


/*------------------------------------------------------------------------------------------------------------------
Scrittura di una curva NURBS aperta e non razionale. Con grado 1 la curva e' una spezzata che passa esattamente per
tutti i punti (usata ad esempio per disegnare gli archi del grafo dei waypoint).
------------------------------------------------------------------------------------------------------------------*/
void MA_Write_NurbsCurve (unsigned int n, MA_EXPORT &MA)
{
	const NurbsCurve &curve = MA.NurbsCurve[n];
	unsigned int nPoints = curve.Points.size();
	unsigned int degree = curve.Degree;
	if (nPoints <= degree)						// Una curva di grado d richiede almeno d + 1 punti
		return;

	stringstream out;
	out << setprecision(7);
	out << "createNode transform -n \"" << curve.name << "\"";
	if (curve.parent.size() > 0)
		out << " -p \"" << curve.parent << "\"";
	out << ";\n";
	out << "createNode nurbsCurve -n \"" << curve.name << "Shape\" -p \"" << curve.name << "\";\n";
	out << "	setAttr -k off \".v\";\n";
	out << "	setAttr \".cc\" -type \"nurbsCurve\" \n";
	out << "		" << degree << " " << nPoints - degree << " 0 no 3\n";		// Grado, numero di span, forma (0 = aperta), razionale, dimensione
	// Vettore dei nodi: per una curva aperta i nodi estremi sono ripetuti "grado" volte
	unsigned int nKnots = nPoints + degree - 1;
	out << "		" << nKnots;
	for (unsigned int k = 0; k < nKnots; k++)
	{
		int knot = (int)k - (int)degree + 1;
		out << " " << max(0, min(knot, (int)(nPoints - degree)));
	}
	out << "\n";
	out << "		" << nPoints << "\n";
	for (unsigned int p = 0; p < nPoints; p++)
		out << "		" << curve.Points[p].x << " " << curve.Points[p].y << " " << curve.Points[p].z << "\n";
	out << "		;\n";
	MA.MA_Nodes << out.str();

	if (curve.layer.size() > 0)
		MA.MA_Connections << "connectAttr \"" << curve.layer << ".di\" \"" << curve.name << ".do\";\n";
}
