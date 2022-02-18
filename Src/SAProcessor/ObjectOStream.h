/////////////////////////////////////////////////////////////////////////////
// obstream.h
// Interface for textual iostreams of objects (1995-07-22)
// copyright 1995 JAARS Inc. SIL
//
// Revision History
// 1.5Test8.2
//        SDM Added WriteWindowPlacement(const char*,WINDOWPLACEMENT&)
/////////////////////////////////////////////////////////////////////////////
#ifndef OBJECTOSTREAM_H
#define OBJECTOSTREAM_H

using std::ifstream;  // iostream
using std::ofstream;  // iostream
using std::streampos;

/* Documentation and comments by Alan B 1-18-96
These routines are used for persistence, for reading and writing settings files.
They are designed to make it easy to load and save the variables of an object.
An object provides a WriteProperties function to write its member variables and
bReadProperties routine to read its member variables.
See bottom of file for samples.
*/
// **************************************************************************
class CObjectOStream {
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

#endif
