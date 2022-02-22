#ifndef SASTRING_H_
#define SASTRING_H_

#include "objectistream.h"

class CSaString : public CString {
public:
    CSaString() : CString() {};
    CSaString(TCHAR ch, int nRepeat = 1) : CString(ch, nRepeat) {};
    CSaString(const CSaString& stringSrc) : CString(stringSrc) {};
    CSaString(const CString& stringSrc) : CString(stringSrc) {};
    CSaString(LPCSTR psz) : CString(psz) {};
    CSaString(LPCWSTR psz) : CString(psz) {};
    CSaString(const unsigned char* psz) : CString(psz) {};

	// return temporary utf8 copy of current string
    virtual std::string utf8() const;
	// set contents of string from utf8 source
	virtual void setUtf8(LPCSTR pUtf8); 
};

bool ReadStreamString(CObjectIStream & stream, CSaString pszMarker, CSaString & szResult);
bool ReadStreamString(CObjectIStream & stream, LPCSTR pszMarker, CSaString & szResult);

#endif