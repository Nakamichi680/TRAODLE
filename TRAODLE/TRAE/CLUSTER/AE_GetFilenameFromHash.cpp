#include "stdafx.h"
#include "Classes.h"


string AE_GetFilenameFromHash (int hash, char* buffer)
{
	unsigned short Magic1;
	string Magic2 = "fil";
	string first4bytes = "fill";
	memcpy(&Magic1, buffer, 2);
	memcpy(&Magic2, buffer + 4, 3);
	memcpy(&first4bytes, buffer, 4);
	stringstream searchfailed;
	
	if (Magic1 == 1 && Magic2 == "CLU")
	{
		searchfailed << std::hex << hash << ".CLU";
		return AE_IO.AddFileToClusterList(searchfailed.str(), AEFileType::CLU);
	}
	if (Magic1 == 3 && Magic2 == "FNT")
	{
		searchfailed << std::hex << hash << ".FNT";
		return AE_IO.AddFileToClusterList(searchfailed.str(), AEFileType::FNT);
	}
	if (Magic1 == 9 && Magic2 == "TXS")
	{
		searchfailed << std::hex << hash << ".TXS";
		return AE_IO.AddFileToClusterList(searchfailed.str(), AEFileType::TXS);
	}
	if ((Magic1 == 0x0E || Magic1 == 0x13) && Magic2 == "SEQ")
	{
		searchfailed << std::hex << hash << ".SEQ";
		return AE_IO.AddFileToClusterList(searchfailed.str(), AEFileType::SEQ);
	}
	if (Magic1 == 0x24 && Magic2 == "AST")
	{
		searchfailed << std::hex << hash << ".AST";
		return AE_IO.AddFileToClusterList(searchfailed.str(), AEFileType::AST);
	}
	if ((Magic1 == 0x5A || Magic1 == 0x5C) && Magic2 == "MDC")
	{
		searchfailed << std::hex << hash << ".MDC";
		return AE_IO.AddFileToClusterList(searchfailed.str(), AEFileType::MDC);
	}
	if ((Magic1 == 0x8F || Magic1 == 0x93 || Magic1 == 0x96) && Magic2 == "LVC")
	{
		searchfailed << std::hex << hash << ".LVC";
		return AE_IO.AddFileToClusterList(searchfailed.str(), AEFileType::LVC);
	}
	if (first4bytes == "AWAD")
	{
		searchfailed << std::hex << hash << ".AWAD";
		return AE_IO.AddFileToClusterList(searchfailed.str(), AEFileType::AWAD);
	}
	if (first4bytes == "‰PNG")
	{
		searchfailed << std::hex << hash << ".PNG";
		return AE_IO.AddFileToClusterList(searchfailed.str(), AEFileType::UNKNOWN);
	}
	if (first4bytes == "LINK")
	{
		searchfailed << std::hex << hash << ".LINK";
		return AE_IO.AddFileToClusterList(searchfailed.str(), AEFileType::LINK);
	}
	searchfailed << std::hex << hash;
	return AE_IO.AddFileToClusterList(searchfailed.str(), AEFileType::UNKNOWN);
}