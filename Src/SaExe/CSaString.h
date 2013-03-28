#ifndef _CSaString_H_

#include "IUtf8String.h"

#define _CSaString_H_


class CSaString : public CString, public IUtf8String
{
public:
    CSaString() : CString()
    {
        ;
    }
    CSaString(TCHAR ch, int nRepeat = 1) : CString(ch, nRepeat)
    {
        ;
    }
    CSaString(const CSaString & stringSrc) : CString(stringSrc)
    {
        ;
    }
    CSaString(const CString & stringSrc) : CString(stringSrc)
    {
        ;
    }
    CSaString(LPCSTR psz) : CString(psz)
    {
        ;
    }
    CSaString(LPCWSTR psz) : CString(psz)
    {
        ;
    }
    CSaString(const unsigned char * psz) : CString(psz)
    {
        ;
    }

    virtual std::string utf8() const; // return temporary utf8 copy of current string
    virtual void setUtf8(const char * pUtf8); // set contents of string from utf8 source
};

class CSaStringArray : public CStringArray
{
public:
};

double  parseUtf8(const IUtf8String &, INT * att, double X, double Y = 0.0);

LONG mmioWriteUtf8(HMMIO hmmio, const IUtf8String & str, LONG cch = -1);
LONG mmioReadUtf8(HMMIO hmmio, IUtf8String & str, LONG cch);



#endif