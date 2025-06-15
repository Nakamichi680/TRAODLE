#include "stdafx.h"
#include "MA/MA_Classes.h"

string MA_Write_Animation_Subroutine (MA_KeyTimeValue *Data)
{
	stringstream out;
	unsigned int n1 = 0;
	unsigned int n2 = 249;
	unsigned int i = 0;

	out << "	setAttr \".tan\" 18;\n";
	out << "	setAttr \".wgt\" no;\n";
	if (Data->Time.size() > 250)
		out << "	setAttr -s " << Data->Time.size() << "\".ktv\";\n";
	do
	{
		n2 = min(Data->Time.size() - 1, n2);
		out << "	setAttr ";
		if (Data->Time.size() <= 250)
			out << "-s " << n2 + 1 << " ";
		out << "\".ktv[" << n1 << ":" << n2 << "]\" ";
		for (i = n1; i <= n2; i++)
		{
			out << " " << Data->Time[i] << " " << Data->Value[i];
			if ((i + 1) % 5 == 0 && i != n2)
				out << "\n		";
		}
		out << ";\n";
		n1 += 250;
		n2 += 250;
	} while (i < Data->Time.size());

	return out.str();
}
