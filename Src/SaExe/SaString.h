#ifndef SASTRING_H_
#define SASTRING_H_

#include <afx.h>
#include "objectistream.h"
#include <string>
#include <MMSystem.h>

class CSaString : public CString
{
public:
    CSaString() : CString()
    {
    }
    CSaString(TCHAR ch, int nRepeat = 1) : CString(ch, nRepeat)
    {
    }
    CSaString(const CSaString & stringSrc) : CString(stringSrc)
    {
    }
    CSaString(const CString & stringSrc) : CString(stringSrc)
    {
    }
    CSaString(LPCSTR psz) : CString(psz)
    {
    }
    CSaString(LPCWSTR psz) : CString(psz)
    {
    }
    CSaString(const unsigned char * psz) : CString(psz)
    {
    }

    virtual std::string utf8() const; // return temporary utf8 copy of current string
    virtual void setUtf8(LPCSTR pUtf8); // set contents of string from utf8 source
};

class CSaStringArray : public CStringArray
{
public:
};

double ParseUtf8(const CSaString &, INT * att, double X, double Y = 0.0);
LONG mmioWriteUtf8(HMMIO hmmio, const CSaString & str, LONG cch = -1);
LONG mmioReadUtf8(HMMIO hmmio, CSaString & str, LONG cch);
bool ReadStreamString(CObjectIStream & stream, CSaString pszMarker, CSaString & szResult);
bool ReadStreamString(CObjectIStream & stream, LPCSTR pszMarker, CSaString & szResult);

#endif