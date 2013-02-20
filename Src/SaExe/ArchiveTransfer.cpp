
#include "stdafx.h"
#include "ArchiveTransfer.h"
#include "CSaString.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

short int ArchiveTransfer::tShortInt(CArchive& ar, short int x)
{
	short int ret = 0;
	CSaString shortInt;

	if (ar.IsStoring())
	{
		swprintf_s(shortInt.GetBuffer(100),100,_T("%d"),x);
		shortInt.ReleaseBuffer();
		ar << shortInt;
	}
	else
	{
		ar >> shortInt;
		swscanf_s(shortInt.GetBuffer(100),_T("%d"),&ret);
		shortInt.ReleaseBuffer();
	}

	return ret;
}

int ArchiveTransfer::tInt(CArchive& ar, int x)
{
	int ret = 0;
	CSaString szInt;

	if (ar.IsStoring())
	{
		swprintf_s(szInt.GetBuffer(100),100,_T("%d"),x);
		szInt.ReleaseBuffer();
		ar << szInt;
	}
	else
	{
		ar >> szInt;
		swscanf_s(szInt.GetBuffer(100),_T("%d"),&ret);
		szInt.ReleaseBuffer();
	}

	return ret;
}

DWORD ArchiveTransfer::tDWORD(CArchive& ar, DWORD x)
{
	DWORD ret = 0;
	CSaString dWord;

	if (ar.IsStoring())
	{
		swprintf_s(dWord.GetBuffer(100),100,_T("%lu"),x);
		dWord.ReleaseBuffer();
		ar << dWord;
	}
	else
	{
		ar >> dWord;
		swscanf_s(dWord.GetBuffer(100),_T("%lu"),&ret);
		dWord.ReleaseBuffer();
	}

	return ret;
}

