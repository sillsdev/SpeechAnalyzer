/////////////////////////////////////////////////////////////////////////////
// obstream.cpp
// Implementation of textual iostreams of objects (1995-07-22)
// copyright 1995 JAARS Inc. SIL
//
// Revision History
// 1.5Test8.2
//        SDM Added WriteWindowPlacement(const char*,WINDOWPLACEMENT&)
/////////////////////////////////////////////////////////////////////////////
#include "Stdafx.h"
#include <windows.h>
#include <fstream>
#include "objectostream.h"
#include <string.h>
#include <assert.h>

using std::ofstream;
using std::ifstream;
using std::ios;

CObjectOStream::CObjectOStream(LPCSTR filename) : m_ios() {
    m_ios.open(filename);
}

CObjectOStream::~CObjectOStream() {
}

ofstream & CObjectOStream::getIos() {
    return m_ios;
}

void CObjectOStream::WriteBeginMarker(LPCSTR pszMarker, LPCSTR pszName) {
    assert(pszName);
    m_ios << "\\+" << pszMarker << ' ' << pszName << '\n';
}

void CObjectOStream::WriteEndMarker(LPCSTR pszMarker) {
    m_ios << "\\-" << pszMarker << '\n';
}

void CObjectOStream::WriteBeginMarkerWithQualifier(LPCSTR pszMarker, LPCSTR pszQualifier, LPCSTR pszName) {
    if (!pszQualifier) {
        WriteBeginMarker(pszMarker, pszName);
        return;
    }
    assert(pszName);
    m_ios << "\\+" << pszMarker << pszQualifier << ' ' << pszName << '\n';
}

void CObjectOStream::WriteEndMarkerWithQualifier(LPCSTR pszMarker, LPCSTR pszQualifier) {
    if (!pszQualifier) {
        WriteEndMarker(pszMarker);
        return;
    }

    m_ios << "\\-" << pszMarker << pszQualifier << '\n';
}

void CObjectOStream::WriteString(LPCSTR pszMarker, LPCSTR pszQualifier, LPCSTR psz) {
    assert(psz);
    if (!*psz) {
        return;
    }

    m_ios << '\\' << pszMarker << pszQualifier << ' ';
    WriteContents(psz);
    m_ios << '\n';
}

void CObjectOStream::WriteString(LPCSTR pszMarker, LPCSTR psz) {
    assert(psz);
    if (!*psz) {
        return;
    }

    m_ios << '\\' << pszMarker << ' ';
    WriteContents(psz);
    m_ios << '\n';
}

void CObjectOStream::WriteContents(LPCSTR pszContents) {
    LPCSTR psz = pszContents;
    assert(psz);
    LPCSTR pszBackslashAtBOL = NULL;
    while ((pszBackslashAtBOL = strstr(psz, "\n\\")) != NULL) {
        pszBackslashAtBOL += 1;  // Point to the backslash;
        // Write the substring up to and including the newline
        m_ios.write(psz, pszBackslashAtBOL - psz);
        m_ios << ' ';  // Write a space before the backslash
        psz = pszBackslashAtBOL;
        assert(*psz == '\\');
    }
    m_ios << psz;  // Write the rest of the field
}

void CObjectOStream::WriteBool(LPCSTR pszMarker, BOOL b) {
    m_ios << '\\' << pszMarker << ' ' << ((b) ? 1 : 0) << '\n';
}

void CObjectOStream::WriteCOLORREF(LPCSTR pszMarker, COLORREF rgb) {
    m_ios << '\\' << pszMarker << ' ' <<
          (int)GetRValue(rgb) << ',' << (int)GetGValue(rgb) << ',' << (int)GetBValue(rgb) << '\n';
}

void CObjectOStream::WriteInteger(LPCSTR pszMarker, int i, LPCSTR pszComment) {
    m_ios << '\\' << pszMarker << ' ' << i;
    if (pszComment) {
        m_ios << " // " << pszComment;
    }
    m_ios << '\n';
}

void CObjectOStream::WriteDouble(LPCSTR pszMarker, double i) {

    // Use as many digits as necessary, 20 is the most we should see
    std::streamsize oldPrecision = m_ios.precision(20);
    long oldFlags = m_ios.flags();
    m_ios.unsetf(ios::showpoint);
    m_ios << '\\' << pszMarker << ' ' << i << '\n';

    m_ios.precision(oldPrecision);
    oldFlags = m_ios.flags(oldFlags);
}

void CObjectOStream::WriteUInt(LPCSTR pszMarker, UINT u, LPCSTR pszComment) {
    m_ios << '\\' << pszMarker << ' ' << u;
    if (pszComment) {
        m_ios << " //" << pszComment;
    }
    m_ios << '\n';
}

void CObjectOStream::WriteNewline() {
    m_ios << '\n';
}

void CObjectOStream::WriteWindowPlacement(LPCSTR pszMarker, WINDOWPLACEMENT & wpl) {
    char * pszState = "max";
    switch (wpl.showCmd) {
    case SW_SHOWNORMAL:
        pszState = "normal";
        break;
    case SW_SHOWMINIMIZED:
        pszState = "min";
        break;
    case SW_SHOWMAXIMIZED:
        pszState = "max";
        break;
    default:
        assert(FALSE);
        break;
    }
    char pszPlacement[103]; // 6 + 8*(1+11) + 1
    sprintf_s(pszPlacement, _countof(pszPlacement), "%-6s %4d %4d %4d %4d %4d %4d %4d %4d",
              pszState,
              wpl.ptMinPosition.x, wpl.ptMinPosition.y,
              wpl.ptMaxPosition.x, wpl.ptMaxPosition.y,
              wpl.rcNormalPosition.left, wpl.rcNormalPosition.top,
              wpl.rcNormalPosition.right, wpl.rcNormalPosition.bottom);
    WriteString(pszMarker, pszPlacement);
}


