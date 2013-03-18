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
#include "CSaString.h"
#include "UnicodeString.h"
#include "ZGraph\toolkit.h"

std::string CSaString::utf8() const {

    CUtf8String result(CUtf16String(*this));
    return result.getUtf8();
}

void CSaString::setUtf8(const char * pUtf8) {

    *this = CUtf16String(CUtf8String(pUtf8)).getUtf16().c_str();
}


double parseUtf8(const IUtf8String & sz, INT * att, double X, double Y) {

    std::string szUtf8 = sz.utf8();
    return parse(szUtf8.c_str(), att, X, Y);
}

LONG mmioWriteUtf8(HMMIO hmmio, const IUtf8String & str, LONG cch) {

    std::string szUtf8 = str.utf8();
    return mmioWrite(hmmio, szUtf8.c_str(), cch);
}

LONG mmioReadUtf8(HMMIO hmmio, IUtf8String & str, LONG cch) {

    char * strUtf8 = new char[cch];
    LONG result = mmioRead(hmmio, strUtf8, cch);

    str.setUtf8(strUtf8);
    delete strUtf8;

    return result;
}

