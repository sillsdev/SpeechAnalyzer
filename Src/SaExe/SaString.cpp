/////////////////////////////////////////////////////////////////////////////
// CSaString.cpp:
// Implementation of the CSaString class.
// Author: Steve MacLean
// copyright 2005 JAARS Inc. SIL
//
// Revision History
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "SaString.h"
#include "UnicodeString.h"
#include "ZGraph\toolkit.h"

std::string CSaString::utf8() const
{

    CUtf8String result(CUtf16String(*this));
    return result.getUtf8();
}

void CSaString::setUtf8(LPCSTR pUtf8)
{

    *this = CUtf16String(CUtf8String(pUtf8)).getUtf16().c_str();
}


double ParseUtf8(const CSaString & sz, INT * att, double X, double Y)
{

    std::string szUtf8 = sz.utf8();
    return parse(szUtf8.c_str(), att, X, Y);
}

LONG mmioWriteUtf8(HMMIO hmmio, const CSaString & str, LONG cch)
{

    std::string szUtf8 = str.utf8();
    return mmioWrite(hmmio, szUtf8.c_str(), cch);
}

LONG mmioReadUtf8(HMMIO hmmio, CSaString & str, LONG cch)
{

    char * strUtf8 = new char[cch];
    LONG result = mmioRead(hmmio, strUtf8, cch);

    str.setUtf8(strUtf8);
    delete strUtf8;

    return result;
}

bool ReadStreamString(CObjectIStream & stream, CSaString pszMarker, CSaString & szResult)
{
    return ReadStreamString(stream, pszMarker.utf8().c_str(), szResult);
}

bool ReadStreamString(CObjectIStream & stream, LPCSTR pszMarker, CSaString & szResult)
{
	size_t size = stream.GetBufferSize()+1;
	LPSTR buffer = new char[size];
	try
	{
		memset( buffer, 0, size);
		BOOL result = stream.bReadString(pszMarker, buffer, size);
		if (result)
		{
			szResult.setUtf8(buffer);
		}
		delete [] buffer;
		buffer = NULL;
	    return result;
	}
	catch(...)
	{
		if (buffer!=NULL)
		{
			delete [] buffer;
			buffer = NULL;
		}
	}
	return FALSE;
}
