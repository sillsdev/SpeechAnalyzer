/////////////////////////////////////////////////////////////////////////////
// obstream.h
// Interface for textual iostreams of objects (1995-07-22)
// copyright 1995 JAARS Inc. SIL
//
// Revision History
// 1.5Test8.2
//        SDM Added WriteWindowPlacement(const char*,WINDOWPLACEMENT&)
/////////////////////////////////////////////////////////////////////////////
#ifndef OBJECTISTREAM_H
#define OBJECTISTREAM_H

#include <fstream>
#include "array_ptr.h"

using std::ifstream;  // iostream
using std::ofstream;  // iostream
using std::streampos;

// ==========================================================================

class CObjectIStream
{
public:
    CObjectIStream( LPCSTR filename);
    ~CObjectIStream();

	void Clear();
	void Close();
	void GetLine( char * buffer, size_t size);
	int Peek();
    void Rewind();                          // skip back to the beginning and reset
    void SkipBOM();                         // Skip opening BOM, if present
	void SeekG( streampos pos);

	streampos TellG();

	bool bAtEnd();                          // True if at eof
    bool bAtBackslash();                    // True if at a backslash
    bool bAtBeginMarker();                  // True if at any begin marker
    bool bAtBeginOrEndMarker();             // True if at any begin or end marker
    bool bAtBeginMarker(LPCSTR pszMarker, LPCSTR pszName = NULL); // True if at the desired begin marker followed by the requested name
    bool bAtEndMarker(LPCSTR pszMarker);    // True if at the desired end marker
	bool bFail();
	
    bool bReadBeginMarker(LPCSTR pszMarker,  LPSTR psName, size_t size); // Read the desired begin marker (pszMarker with a plus sign)
    bool bReadBeginMarker(LPCSTR pszMarker);                // Read the desired begin marker (pszMarker with a plus sign)
    bool bReadEndMarker(LPCSTR pszMarker);                  // Read the desired end marker (pszMarker with a minus sign)
    void SkipToEndMarker(LPCSTR pszMarker);
    bool bReadBeginMarkerWithQualifier(LPCSTR pszMarker, LPCSTR pszQualifier); // Read the desired begin marker with a qualifier
    bool bReadEndMarkerWithQualifier(LPCSTR pszMarker, LPCSTR pszQualifier); // Read the desired end marker with a qualifier

    bool bReadString(LPCSTR pszMarker, LPSTR szResult, size_t len); // Read a string

    bool bReadBool(LPCSTR pszMarker, BOOL & b);             // Read a boolean. The presence of the marker means TRUE
    bool bReadCOLORREF(LPCSTR pszMarker, COLORREF & rgb);   // Read a set of three rgb color numbers
    bool bReadInteger(LPCSTR pszMarker, int & i);           // Read an integer
    bool bReadDouble(LPCSTR pszMarker, double & i);
    bool bReadUInt(LPCSTR pszMarker, UINT & ui);            // Read an unsigned integer (unsigned short)
    bool bReadNumberOf(LPCSTR pszMarker, unsigned long & num);   // Read a long unsigned integer
    bool bReadDWord(LPCSTR pszMarker, DWORD & dw);          // Read a DWORD (long unsigned integer)

    bool bEnd(LPCSTR pszMarker);  // Read given end marker, stop any begin marker, for error recovery on objects with unrecognized names AB 1-18-95
    bool bEndWithQualifier(LPCSTR pszMarker, LPCSTR pszQualifier); // same as bEnd() but with additional qualifier
    void ReadMarkedString(); // Generic eat of one marker and its string

    void PeekMarkedString(LPCSTR * ppszMarker, LPSTR pszString, size_t len, BOOL bTrimWhiteSpace = TRUE);

#ifndef NO_INTERFACE
    bool bReadWindowPlacement(LPCSTR pszMarker, WINDOWPLACEMENT & wpl);
#endif  // not NO_INTERFACE

	size_t GetBufferSize() const;

protected:
    // Read any marked string
    void ReadMarkedString(LPCSTR * ppszMarker, LPCSTR * ppszString, BOOL bTrimWhiteSpace = TRUE);
    bool bReadString(LPCSTR pszMarker, LPCSTR * s);
    void peekMarkedString(LPCSTR * ppszMarker, LPCSTR * ppszString, BOOL bTrimWhiteSpace = TRUE);
    void UnReadMarkedString(); // Store most recently read marked string for next read

private:
	bool Shw_bAtWhiteSpace(const char * psz);
    void ReadMarkedLine(LPCSTR * ppszMarker, LPCSTR * ppszString); // Read any marked line
    void ReadLine();				// Read one line

    ifstream m_ios;					// iostream from which input is read
	array_ptr<char> string_buffer;	// input buffer for a marked string
    char * m_pszEnd;				// end of string (i.e. its null) where next line is read
    bool m_bUnRead;					// buffer contains a marked string which has been "unread"
    char * m_pszMarker;				// beginning of unread marker
    char * m_pszString;				// beginning of unread string
    char m_chEndOfLine;				// delimiter for the get-one-line function

    bool bReadMarker(char cFirstChar, LPCSTR pszMarker); // low level read begin or end marker

};

#endif
