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

std::string CSaString::utf8() const {

    CUtf8String result(CUtf16String(*this));
    return result.getUtf8();
}

void CSaString::setUtf8(LPCSTR pUtf8) {
    *this = CUtf16String(CUtf8String(pUtf8)).getUtf16().c_str();
}


double ParseUtf8(const CSaString & sz, INT * att, double X, double Y) {
    std::string szUtf8 = sz.utf8();
    return parse(szUtf8.c_str(), att, X, Y);
}

bool ReadStreamString(CObjectIStream & stream, CSaString pszMarker, CSaString & szResult) {
    return ReadStreamString(stream, pszMarker.utf8().c_str(), szResult);
}

bool ReadStreamString(CObjectIStream & stream, LPCSTR pszMarker, CSaString & szResult) {
    size_t size = stream.GetBufferSize()+1;
    array_ptr<char> buffer(size);
    bool result = stream.bReadString(pszMarker, buffer.get(), buffer.size());
    if (result) {
        szResult.setUtf8(buffer.get());
    }
    return result;
}

CSaString::CSaString() : CString() {
}

CSaString::CSaString(TCHAR ch, int nRepeat) : CString(ch, nRepeat) {
}

CSaString::CSaString(const CSaString & stringSrc) : CString(stringSrc) {
}

CSaString::CSaString(const CString & stringSrc) : CString(stringSrc) {
}

CSaString::CSaString(LPCSTR psz) : CString(psz) {
}

CSaString::CSaString(LPCWSTR psz) : CString(psz) {
}

CSaString::CSaString(const unsigned char * psz) : CString(psz) {
}
