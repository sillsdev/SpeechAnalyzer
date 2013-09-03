/////////////////////////////////////////////////////////////////////////////
// obstream.h
// Interface for textual iostreams of objects (1995-07-22)
// copyright 1995 JAARS Inc. SIL
//
// Revision History
// 1.5Test8.2
//        SDM Added WriteWindowPlacement(const char*,WINDOWPLACEMENT&)
/////////////////////////////////////////////////////////////////////////////
#ifndef OBSTREAM_H
#define OBSTREAM_H

using std::ifstream;  // iostream
using std::ofstream;  // iostream

typedef unsigned long NumberOf;  // Hungarian: num
// Count of the number of some kind of objects.


/* Documentation and comments by Alan B 1-18-96
These routines are used for persistence, for reading and writing settings files.
They are designed to make it easy to load and save the variables of an object.
An object provides a WriteProperties function to write its member variables and
bReadProperties routine to read its member variables.
See bottom of file for samples.
*/
// **************************************************************************
class CObjectOStream
{
public:
    CObjectOStream(LPCSTR filename);
    ~CObjectOStream();

    ofstream & getIos();
    void WriteBeginMarker(LPCSTR pszMarker, LPCSTR pszName = "");   // Write a begin marker with an optional name
    void WriteEndMarker(LPCSTR pszMarker);                          // Write an end marker
    void WriteBeginMarkerWithQualifier(LPCSTR pszMarker,            // Write a begin marker with qualifier
                                       LPCSTR pszQualifier,
                                       LPCSTR pszName = "");
    void WriteEndMarkerWithQualifier(LPCSTR pszMarker, LPCSTR pszQualifier); // Write an end marker with qualifier
    void WriteString(LPCSTR pszMarker, LPCSTR psz);                 // Write string
    void WriteString(LPCSTR pszMarker, LPCSTR pszQualifier, LPCSTR psz); // Write marker with qualifier appended and string
    void WriteBool(LPCSTR pszMarker, BOOL b);                       // Write marker if boolean is true
    void WriteCOLORREF(LPCSTR pszMarker, COLORREF rgb);             // Write a set of three rgb color numbers
    void WriteInteger(LPCSTR pszMarker, int i, LPCSTR pszComment = NULL); // Write an integer
    void WriteDouble(LPCSTR pszMarker, double i);                   // Write an integer
    void WriteUInt(LPCSTR pszMarker, UINT u, LPCSTR pszComment = NULL); // Write a uint

    void WriteNewline(); // Write a blank line, used to separate sections of the file for readability

#ifndef NO_INTERFACE
    void WriteWindowPlacement(LPCSTR pszMarker, WINDOWPLACEMENT & wpl); //SDM 1.5Test8.2
#endif  // not NO_INTERFACE

private:
    ofstream m_ios;  // iostream to which output is written
    void WriteContents(LPCSTR pszContents);

    // Write the field's contents NOT including a newline.
    // "Protect" backslashes at the beginning of lines
    // by writing a space before them.
};

// ==========================================================================

class CObjectIStream
{
public:
    CObjectIStream(LPCSTR filename);
    ~CObjectIStream();

    ifstream & getIos();
    void Rewind();                          // skip back to the beginning and reset
    void SkipBOM();                         // Skip opening BOM, if present
    BOOL bAtEnd();                          // True if at eof
    BOOL bAtBackslash();                    // True if at a backslash
    BOOL bAtBeginMarker();                  // True if at any begin marker
    BOOL bAtBeginOrEndMarker();             // True if at any begin or end marker
    BOOL bAtBeginMarker(LPCSTR pszMarker, LPCSTR pszName = NULL); // True if at the desired begin marker followed by the requested name
    BOOL bAtEndMarker(LPCSTR pszMarker);    // True if at the desired end marker

    BOOL bReadBeginMarker(LPCSTR pszMarker,  LPSTR psName, size_t size); // Read the desired begin marker (pszMarker with a plus sign)
    BOOL bReadBeginMarker(LPCSTR pszMarker);                // Read the desired begin marker (pszMarker with a plus sign)
    BOOL bReadEndMarker(LPCSTR pszMarker);                  // Read the desired end marker (pszMarker with a minus sign)
    void SkipToEndMarker(LPCSTR pszMarker);
    BOOL bReadBeginMarkerWithQualifier(LPCSTR pszMarker, LPCSTR pszQualifier); // Read the desired begin marker with a qualifier
    BOOL bReadEndMarkerWithQualifier(LPCSTR pszMarker, LPCSTR pszQualifier); // Read the desired end marker with a qualifier

    BOOL bReadString(LPCSTR pszMarker, LPSTR szResult, size_t len); // Read a string

    BOOL bReadBool(LPCSTR pszMarker, BOOL & b);             // Read a boolean. The presence of the marker means TRUE
    BOOL bReadCOLORREF(LPCSTR pszMarker, COLORREF & rgb);   // Read a set of three rgb color numbers
    BOOL bReadInteger(LPCSTR pszMarker, int & i);           // Read an integer
    BOOL bReadDouble(LPCSTR pszMarker, double & i);
    BOOL bReadUInt(LPCSTR pszMarker, UINT & ui);            // Read an unsigned integer (unsigned short)
    BOOL bReadNumberOf(LPCSTR pszMarker, NumberOf & num);   // Read a long unsigned integer
    BOOL bReadDWord(LPCSTR pszMarker, DWORD & dw);          // Read a DWORD (long unsigned integer)

    BOOL bEnd(LPCSTR pszMarker);  // Read given end marker, stop any begin marker, for error recovery on objects with unrecognized names AB 1-18-95
    BOOL bEndWithQualifier(LPCSTR pszMarker, LPCSTR pszQualifier); // same as bEnd() but with additional qualifier
    void ReadMarkedString(); // Generic eat of one marker and its string

    void PeekMarkedString(LPCSTR * ppszMarker, LPSTR pszString, size_t len, BOOL bTrimWhiteSpace = TRUE);

#ifndef NO_INTERFACE
    BOOL bReadWindowPlacement(LPCSTR pszMarker, WINDOWPLACEMENT & wpl);
#endif  // not NO_INTERFACE

	size_t GetBufferSize();

protected:
    // Read any marked string
    void ReadMarkedString(LPCSTR * ppszMarker, LPCSTR * ppszString, BOOL bTrimWhiteSpace = TRUE);
    BOOL bReadString(LPCSTR pszMarker, LPCSTR * s);
    void peekMarkedString(LPCSTR * ppszMarker, LPCSTR * ppszString, BOOL bTrimWhiteSpace = TRUE);
    void UnReadMarkedString(); // Store most recently read marked string for next read

private:
    void ReadMarkedLine(LPCSTR * ppszMarker, LPCSTR * ppszString); // Read any marked line
    void ReadLine();        // Read one line

    ifstream m_ios;         // iostream from which input is read
    char * m_pszMStringBuf; // input buffer for a marked string
    char * m_pszEnd;        // end of string (i.e. its null) where next line is read
    BOOL m_bUnRead;         // buffer contains a marked string which has been "unread"
    char * m_pszMarker;     // beginning of unread marker
    char * m_pszString;     // beginning of unread string
    char m_chEndOfLine;     // delimiter for the get-one-line function

    BOOL bReadMarker(char cFirstChar, LPCSTR pszMarker); // low level read begin or end marker

};

#endif
