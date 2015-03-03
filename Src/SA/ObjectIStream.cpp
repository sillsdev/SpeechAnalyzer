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
#include "objectistream.h"
#include <string.h>
#include <assert.h>
#include "FileUtils.h"

using std::ofstream;
using std::ifstream;
using std::ios;

#define maxsizMString 32000

// ==========================================================================
CObjectIStream::CObjectIStream(LPCSTR filename) :
    m_ios(),
    string_buffer() {
    m_ios.open(filename);
    DWORD file_length = FileUtils::GetFileSize(filename);
    string_buffer.reallocate(file_length+1);
    m_pszEnd = NULL;
    m_pszMarker = NULL;
    m_pszString = NULL;
    m_bUnRead = FALSE;
    m_chEndOfLine = '\n';  // Default for Mac and Windows
}

CObjectIStream::~CObjectIStream() {
}

size_t CObjectIStream::GetBufferSize() const {
    return string_buffer.size();
}

void CObjectIStream::SkipBOM() {
    char nextChar;

    for (int i = 0; i < 3; i++) {
        if (bAtBackslash()) {
            return;
        } else {
            m_ios.get(nextChar);
        }
    }
}

bool CObjectIStream::bAtEnd() {
    // MRP: TO DO!!! Check whether the following (and bReadField) is right...
    // 1. If marker or contents at EOF with no newline;
    // 2. If explicit Ctrl-Z (code 26) in file.

    // Have read to end if no "unread" field, and all chars have been read
    // from the stream. NOTE: Because no attempt has yet been made actually
    // to get chars from beyond the end of the stream, eof() will NOT be true.
    return ((!m_bUnRead) && (m_ios.peek() == EOF));
}

bool CObjectIStream::bAtBackslash() {
    // Stream represents Standard Format if a field has already been read
    // successfully, or if the first char of the stream is a backslash.
    return ((m_bUnRead) || (m_ios.peek() == '\\'));
}

bool CObjectIStream::bAtBeginMarker() {

    LPCSTR pszMarkerRead = NULL;
    LPCSTR pszStringRead = NULL;
    ReadMarkedString(&pszMarkerRead, &pszStringRead);
    bool bBeginMarker = (*pszMarkerRead == '+');
    UnReadMarkedString();
    return bBeginMarker;
}

bool CObjectIStream::bAtBeginOrEndMarker() {
    LPCSTR pszMarkerRead = NULL;
    LPCSTR pszStringRead = NULL;
    ReadMarkedString(&pszMarkerRead, &pszStringRead);

    bool bBeginOrEndMarker = (*pszMarkerRead == '+' || *pszMarkerRead == '-');
    UnReadMarkedString();

    return bBeginOrEndMarker;
}

bool CObjectIStream::bAtBeginMarker(LPCSTR pszMarker, LPCSTR pszName) {

    LPCSTR pszMarkerRead = NULL;
    LPCSTR pszStringRead = NULL;
    if (!bReadBeginMarker(pszMarker)) {
        return FALSE;
    }
    UnReadMarkedString();
    ReadMarkedString(&pszMarkerRead, &pszStringRead);
    bool b = ((!pszName) || (strcmp(pszName, pszStringRead)==0));   // If name given, it must match
    UnReadMarkedString();
    return b;
}

bool CObjectIStream::bAtEndMarker(LPCSTR pszMarker) {
    if (!bReadEndMarker(pszMarker)) {
        return false;
    }

    UnReadMarkedString();
    return true;
}


bool CObjectIStream::bReadBeginMarker(LPCSTR pszMarker, LPSTR psName, size_t size) {

    LPCSTR pszMarkerRead = NULL;
    LPCSTR pszStringRead = NULL;
    ReadMarkedString(&pszMarkerRead, &pszStringRead);

    BOOL bBeginMarker = (*pszMarkerRead == '+') && (strcmp(pszMarker, pszMarkerRead+1)==0);
    if (!bBeginMarker) {
        UnReadMarkedString();
        return FALSE;
    }

    memset(psName,0,size);
    strcpy_s(psName, size, pszStringRead);
    return TRUE;
}

bool CObjectIStream::bReadMarker(char cFirstChar, LPCSTR pszMarker) {
    LPCSTR pszMarkerRead = NULL;
    LPCSTR pszStringRead = NULL;
    ReadMarkedString(&pszMarkerRead, &pszStringRead);

    BOOL bMarkerFound = (*pszMarkerRead == cFirstChar) && (strcmp(pszMarker, pszMarkerRead+1)==0);
    if (!bMarkerFound) {
        UnReadMarkedString();
        return FALSE;
    }

    return TRUE;
}

#define MAXMKRSIZE 50

bool CObjectIStream::bReadBeginMarkerWithQualifier(LPCSTR pszMarker, LPCSTR pszQualifier) {
    if (!pszQualifier) {
        return bReadBeginMarker(pszMarker);
    }

    char buf[MAXMKRSIZE];
    assert(strlen(pszMarker) + strlen(pszQualifier) + 1 < MAXMKRSIZE);
    strcpy_s(buf, _countof(buf), pszMarker);
    strcat_s(buf, _countof(buf), pszQualifier);
    return bReadMarker('+', buf);
}

bool CObjectIStream::bReadEndMarkerWithQualifier(LPCSTR pszMarker, LPCSTR pszQualifier) {
    if (!pszQualifier) {
        return bReadEndMarker(pszMarker);
    }

    char buf[MAXMKRSIZE];
    assert(strlen(pszMarker) + strlen(pszQualifier) + 1 < MAXMKRSIZE);
    strcpy_s(buf, _countof(buf), pszMarker);
    strcat_s(buf, _countof(buf), pszQualifier);
    return bReadMarker('-', buf);
}

bool CObjectIStream::bReadString(LPCSTR pszMarker, LPSTR szResult, size_t size) {
    LPCSTR read = NULL;
    bool result = bReadString(pszMarker, &read);
    if (result) {
        size_t sz = strlen(read);
        strcpy_s(szResult, size, read);
    }
    return result;
}

bool CObjectIStream::bReadString(LPCSTR pszMarker, LPCSTR * s) {
    LPCSTR pszMarkerRead = NULL;
    LPCSTR pszStringRead = NULL;
    ReadMarkedString(&pszMarkerRead, &pszStringRead);

    if (strcmp(pszMarker, pszMarkerRead)!=0) {
        UnReadMarkedString();
        return false;
    }

    *s = pszStringRead;
    return true;
}

bool CObjectIStream::bReadBool(LPCSTR pszMarker, BOOL & b) {
    LPCSTR s = NULL;
    if (!bReadString(pszMarker, &s)) {
        return FALSE;
    }

    b = (atoi(s) != 0);
    return TRUE;
}

bool CObjectIStream::bReadCOLORREF(LPCSTR pszMarker, COLORREF & rgb) {
    LPCSTR s = NULL;
    if (!bReadString(pszMarker, &s)) {
        return FALSE;
    }

    short int r = 0;
    short int g = 0;
    short int b = 0;
    // 1997-05-17 MRP: sscanf not defined in DLLs -- reimplement
    if (sscanf_s(s, "%hd,%hd,%hd", &r, &g, &b) == 0) {
        return FALSE;
    }

    rgb = RGB(r,g,b);
    return TRUE;
}

bool CObjectIStream::bReadInteger(LPCSTR pszMarker, int & i) {
    LPCSTR s = NULL;
    if (!bReadString(pszMarker, &s)) {
        return FALSE;
    }

    i = atoi(s);
    return TRUE;
}

bool CObjectIStream::bReadDouble(LPCSTR pszMarker, double & i) {
    LPCSTR s = NULL;
    if (!bReadString(pszMarker, &s)) {
        return FALSE;
    }

    i = atof(s);
    return TRUE;
}

bool CObjectIStream::bReadNumberOf(LPCSTR pszMarker, unsigned long & num) {
    LPCSTR s = NULL;
    if (!bReadString(pszMarker, &s)) {
        return FALSE;
    }

    if (sscanf_s(s, "%lu", &num) == 0) {
        return FALSE;
    }

    return TRUE;
}

void CObjectIStream::ReadMarkedString() { // Generic eat of one marker and its string
    LPCSTR pszMarkerRead = NULL;
    LPCSTR pszStringRead = NULL;
    ReadMarkedString(&pszMarkerRead, &pszStringRead);
}

// Read given end marker, stop any begin marker, for error recovery on objects with unrecognized names AB 1-18-95
bool CObjectIStream::bEnd(LPCSTR pszMarker) {
    if (bReadEndMarker(pszMarker)) {   // If desired end marker, return true for successfully found end
        return TRUE;
    } else if (bAtBeginMarker()) { // If somebody else's begin marker, return true for found end
        return TRUE;
    } else { // Else, unrecognized marker, eat marker and return false for not at end yet
        LPCSTR pszMarkerRead = NULL;
        LPCSTR pszStringRead = NULL;
        ReadMarkedString(&pszMarkerRead, &pszStringRead);
        return FALSE;
    }
}

bool CObjectIStream::bEndWithQualifier(LPCSTR pszMarker, LPCSTR pszQualifier) {
    if (!pszQualifier) {
        return bEnd(pszMarker);
    }

    char buf[MAXMKRSIZE];
    assert(strlen(pszMarker) + strlen(pszQualifier) + 1 < MAXMKRSIZE);
    strcpy_s(buf, _countof(buf), pszMarker);
    strcat_s(buf, _countof(buf), pszQualifier);
    return bEnd(buf);
}

#ifndef NO_INTERFACE
bool CObjectIStream::bReadWindowPlacement(LPCSTR pszMarker, WINDOWPLACEMENT & wpl) {
    LPCSTR sPlacement = NULL;
    if (!bReadString(pszMarker, &sPlacement)) {
        return FALSE;
    }

    wpl.length = sizeof(wpl);
    wpl.flags = WPF_SETMINPOSITION;
    char pszShowState[7];
    int nItems = sscanf_s(sPlacement, "%s %d %d %d %d %d %d %d %d",
                          pszShowState,
                          _countof(pszShowState),
                          &wpl.ptMinPosition.x,
                          &wpl.ptMinPosition.y,
                          &wpl.ptMaxPosition.x,
                          &wpl.ptMaxPosition.y,
                          &wpl.rcNormalPosition.left,
                          &wpl.rcNormalPosition.top,
                          &wpl.rcNormalPosition.right,
                          &wpl.rcNormalPosition.bottom);

    if (nItems != 9) {
        return FALSE;
    }

    if (strcmp(pszShowState, "normal")==0) {
        wpl.showCmd = SW_SHOWNORMAL;
    } else if (strcmp(pszShowState, "min")==0) {
        wpl.showCmd = SW_SHOWMINIMIZED;
    } else if (strcmp(pszShowState, "max")==0) {
        wpl.showCmd = SW_SHOWMAXIMIZED;
    } else {
        wpl.showCmd = SW_SHOWNORMAL;
    };

    return TRUE;
}
#endif  // not NO_INTERFACE

void CObjectIStream::ReadMarkedString(LPCSTR * ppszMarker, LPCSTR * ppszString, BOOL bTrimWhiteSpace) {
    if (m_bUnRead) {
        *ppszMarker = m_pszMarker;
        *ppszString = m_pszString;
        m_bUnRead = FALSE;
        return;
    }

    ReadMarkedLine(ppszMarker, ppszString);

    // Read any additional lines of the marked string
    int ich;
    while (((ich = m_ios.peek()) != EOF) && (ich != '\\')) {
        *m_pszEnd++ = '\n';
        ReadLine();
    }

    if (bTrimWhiteSpace) {
        // 1995-10-02 MRP: Clean this up and separate out for use of
        // settings properties but *not* Standard Format data fields.
        // Trim white space from the beginning and end of the string
        for (; Shw_bAtWhiteSpace(m_pszString); m_pszString += 1) {
            if (*m_pszString == 0) {
                break;
            }
        }
        if (*m_pszString) {  // string contains non-ws
            char * psz = m_pszString + strlen(m_pszString) - 1; // last char
            for (; Shw_bAtWhiteSpace(psz); psz -= 1) {
                assert(psz != m_pszString);    // have not moved past beginning
                *psz = '\0';  // remove last char
            }
        }
    }

    *ppszString = m_pszString;
}

void CObjectIStream::UnReadMarkedString() {
    assert(!m_bUnRead);
    m_bUnRead = TRUE;
}


void CObjectIStream::ReadMarkedLine(LPCSTR * ppszMarker, LPCSTR * ppszString) {
    assert(!m_bUnRead);
    char * psz = m_pszEnd = string_buffer.get();
    *psz = '\0';
    ReadLine();

    // Caller is responsible to not to read to end, and also to check that
    // the file is indeed Standard Format (i.e. it begins with a backslash)
    assert(*psz == '\\');
    psz += 1; // move past the backslash

    m_pszMarker = psz;
    size_t lenMarker = strcspn(psz, " \t\n");
    // 1996-11-04 MRP: This temporary patch will skip the rest of a field
    // that contains a backslash at the beginning of one of its lines.
    // The real fix is to use the read-line-ahead approach in sfstream.cpp
    // ASSERT( lenMarker != 0 ); // RECOVERY
    psz += lenMarker;

    if (*psz == ' ') {
        // A space delimiting the marker is not considered part of the string
        *psz = '\0'; // so the marker's null termination can take its place
        psz += 1;
    } else {
        // 1995-10-04 MRP: At first I thought this shifting was redundant
        // if no white space followed the marker. If the field consisted
        // of just that one line that would be true. If, however, the
        // field's content follows in additional lines we must do it.
        //
        // Move the marker left one position (covering the backslash)
        m_pszMarker = string_buffer.get();
        memcpy(m_pszMarker, string_buffer.get() + 1, lenMarker);
        assert(m_pszMarker + lenMarker == psz - 1);
        m_pszMarker[lenMarker] = '\0'; // making a place for its null
    }

    *ppszMarker = m_pszMarker; // beginning of marker
    *ppszString = m_pszString = psz; // beginning of string
}

void CObjectIStream::ReadLine() {
    assert(!m_bUnRead);
    // 1995-04-12 MRP: use get in order to detect line longer than buffer
    size_t sizRemaining = maxsizMString - (m_pszEnd - string_buffer.get());
    m_ios.get(m_pszEnd, sizRemaining, m_chEndOfLine);
    m_ios.clear(); // if get encounters an empty line it will set fail bit, must be cleared
    size_t lenLine = strlen(m_pszEnd);
    m_pszEnd += lenLine;
    int ichNext = m_ios.get();
    if ((ichNext != m_chEndOfLine) && (ichNext != std::char_traits<char>::eof())) {
        assert(FALSE);    // at end of input buffer-- RECOVERY
    }
}

// **************************************************************************

bool CObjectIStream::bReadUInt(LPCSTR pszMarker, UINT & ui) { // Read an unsigned integer (unsigned short)
    LPCSTR s;
    if (!bReadString(pszMarker, &s)) {
        return false;
    }

    if (sscanf_s(s, "%hu", &ui) == 0) {
        return false;
    }

    return true;
}

void CObjectIStream::SkipToEndMarker(LPCSTR pszMarker) {
    while (!bAtEnd()) {
        if (bReadEndMarker(pszMarker)) {
            break;
        }
        ReadMarkedString();  // Skip unexpected field
    }
}

void CObjectIStream::Rewind() {
    m_ios.clear();
    m_ios.seekg(0);
    m_ios.clear();
    // reset all internal data
    size_t length = string_buffer.size();
    string_buffer.reallocate(length);
    m_pszEnd = NULL;
    m_pszMarker = NULL;
    m_pszString = NULL;
    m_bUnRead = FALSE;
    m_chEndOfLine = '\n';  // Default for Mac and Windows
}

bool CObjectIStream::bReadBeginMarker(LPCSTR pszMarker) {
    // Read the desired begin marker (pszMarker with a plus sign)
    return bReadMarker('+', pszMarker);
}

bool CObjectIStream::bReadEndMarker(LPCSTR pszMarker) {
    // Read the desired end marker (pszMarker with a minus sign)
    return bReadMarker('-', pszMarker);
}

bool CObjectIStream::bReadDWord(LPCSTR pszMarker, DWORD & dw) {
    // Read a DWORD (long unsigned integer)
    return bReadNumberOf(pszMarker, dw);
}

void CObjectIStream::PeekMarkedString(LPCSTR * ppszMarker, LPSTR pszString, size_t len, BOOL bTrimWhiteSpace) {
    LPCSTR read = 0;
    peekMarkedString(ppszMarker, &read, bTrimWhiteSpace);
    strcpy_s(pszString, len, read);
}

void CObjectIStream::peekMarkedString(LPCSTR * ppszMarker, LPCSTR * ppszString, BOOL bTrimWhiteSpace) {
    // Read any marked string
    ReadMarkedString(ppszMarker, ppszString, bTrimWhiteSpace);
    // Store most recently read marked string for next read
    UnReadMarkedString();
}

bool CObjectIStream::bFail() {
    return m_ios.fail();
}

void CObjectIStream::Close() {
    m_ios.close();
}

void CObjectIStream::GetLine(char * buffer, size_t size) {
    m_ios.getline(buffer, size);
}

int CObjectIStream::Peek() {
    return m_ios.peek();
}

streampos CObjectIStream::TellG() {
    return m_ios.tellg();
}

void CObjectIStream::Clear() {
    return m_ios.clear();
}

void CObjectIStream::SeekG(streampos pos) {
    m_ios.seekg(pos);
}

bool CObjectIStream::Shw_bAtWhiteSpace(const char * psz) {
    return (strchr(" \t\n", *psz) != NULL);
}

