#ifndef SASTRING_H_
#define SASTRING_H_

#include "objectistream.h"

class CSaString : public CString {
public:
    CSaString();
    CSaString(TCHAR ch, int nRepeat = 1);
    CSaString(const CSaString & stringSrc);
    CSaString(const CString & stringSrc);
    CSaString(LPCSTR psz);
    CSaString(LPCWSTR psz);
    CSaString(const unsigned char * psz);

	// return temporary utf8 copy of current string
    virtual std::string utf8() const;
	// set contents of string from utf8 source
	virtual void setUtf8(LPCSTR pUtf8); 
};

double ParseUtf8(const CSaString &, INT * att, double X, double Y = 0.0);
bool ReadStreamString(CObjectIStream & stream, CSaString pszMarker, CSaString & szResult);
bool ReadStreamString(CObjectIStream & stream, LPCSTR pszMarker, CSaString & szResult);

#endif