#include "stdafx.h"


bool SaveTGAResource (int resourceID, string filename)
{
	HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(resourceID), RT_RCDATA);
	if (!hRes)
		return false;

	HGLOBAL hData = LoadResource(NULL, hRes);
	if (!hData)
		return false;

	void* pData = LockResource(hData);
	DWORD size = SizeofResource(NULL, hRes);

	ofstream out(filename, std::ios::binary);
	if (!out)
		return false;

	out.write(static_cast<const char*>(pData), size);
	out.close();
	return true;
}