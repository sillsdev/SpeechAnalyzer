#pragma once
#include <streambuf>
#include <sstream>
#include "SaString.h"
#include "array_ptr.h"

using std::wstringstream;
using std::streampos;

/**
* a class that reads in a wchar_t stream of sfm markers from a string stream
*/
class CStringStream {
public:
    CStringStream(LPCTSTR data);
    ~CStringStream();

    void Clear();
    void GetLine(LPTSTR buffer, size_t size);
    int Peek();
    void Rewind();                          // skip back to the beginning and reset
    void SeekG(streampos pos);

    streampos TellG();

    bool bAtEnd();                          // True if at eof
    bool bAtBackslash();                    // True if at a backslash
    bool bAtBeginMarker();                  // True if at any begin marker
    bool bAtBeginOrEndMarker();             // True if at any begin or end marker
    bool bAtBeginMarker(LPCTSTR pszMarker, LPCTSTR pszName = NULL); // True if at the desired begin marker followed by the requested name
    bool bAtEndMarker(LPCTSTR pszMarker);    // True if at the desired end marker
    bool bFail();

    bool bReadBeginMarker(LPCTSTR pszMarker,  LPTSTR psName, size_t size); // Read the desired begin marker (pszMarker with a plus sign)
    bool bReadBeginMarker(LPCTSTR pszMarker);                // Read the desired begin marker (pszMarker with a plus sign)
    bool bReadEndMarker(LPCTSTR pszMarker);                  // Read the desired end marker (pszMarker with a minus sign)
    void SkipToEndMarker(LPCTSTR pszMarker);
    bool bReadBeginMarkerWithQualifier(LPCTSTR pszMarker, LPCTSTR pszQualifier); // Read the desired begin marker with a qualifier
    bool bReadEndMarkerWithQualifier(LPCTSTR pszMarker, LPCTSTR pszQualifier); // Read the desired end marker with a qualifier

    bool bReadString(LPCTSTR pszMarker, LPTSTR szResult, size_t len); // Read a string

    bool bReadBool(LPCTSTR pszMarker, bool & b);             // Read a boolean. The presence of the marker means TRUE
    bool bReadCOLORREF(LPCTSTR pszMarker, COLORREF & rgb);   // Read a set of three rgb color numbers
    bool bReadInteger(LPCTSTR pszMarker, int & i);           // Read an integer
    bool bReadDouble(LPCTSTR pszMarker, double & i);
    bool bReadUInt(LPCTSTR pszMarker, UINT & ui);            // Read an unsigned integer (unsigned short)
    bool bReadNumberOf(LPCTSTR pszMarker, unsigned long & num);   // Read a long unsigned integer
    bool bReadDWord(LPCTSTR pszMarker, DWORD & dw);          // Read a DWORD (long unsigned integer)

    bool bEnd(LPCTSTR pszMarker);  // Read given end marker, stop any begin marker, for error recovery on objects with unrecognized names AB 1-18-95
    bool bEndWithQualifier(LPCTSTR pszMarker, LPCTSTR pszQualifier); // same as bEnd() but with additional qualifier
    void ReadMarkedString(); // Generic eat of one marker and its string

    void PeekMarkedString(LPCTSTR * ppszMarker, LPTSTR pszString, size_t len, bool bTrimWhiteSpace = true);

    bool ReadStreamString(LPCTSTR pszMarker, CSaString & szResult);

protected:
    // Read any marked string
    void ReadMarkedString(LPCTSTR * ppszMarker, LPCTSTR * ppszString, bool bTrimWhiteSpace = true);
    bool bReadString(LPCTSTR pszMarker, LPCTSTR * s);
    void UnReadMarkedString(); // Store most recently read marked string for next read

private:
    bool Shw_bAtWhiteSpace(LPCTSTR psz);
    void ReadMarkedLine(LPCTSTR * ppszMarker, LPCTSTR * ppszString); // Read any marked line
    void ReadLine();            // Read one line

    wstringstream m_ios;        // iostream from which input is read
    array_ptr<wchar_t> string_buffer;
    LPTSTR m_pszEnd;            // end of string (i.e. its null) where next line is read
    bool m_bUnRead;             // buffer contains a marked string which has been "unread"
    LPTSTR m_pszMarker;         // beginning of unread marker
    LPTSTR m_pszString;         // beginning of unread string
    wchar_t m_chEndOfLine;      // delimiter for the get-one-line function

    bool bReadMarker(wchar_t cFirstChar, LPCTSTR pszMarker); // low level read begin or end marker

};

