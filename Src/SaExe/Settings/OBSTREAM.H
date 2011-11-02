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

class IUtf8String;

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
class Object_ostream // Hungarian: obs
{
public:
	Object_ostream(const IUtf8String &filename);
	~Object_ostream() {} // Destructor

	ofstream& getIos() {return m_ios;};

	void WriteBeginMarker(const char* pszMarker, const IUtf8String &szName); // Write a begin marker with an optional name
	void WriteBeginMarker(const char* pszMarker, const char* pszName = ""); // Write a begin marker with an optional name
	void WriteEndMarker(const char* pszMarker); // Write an end marker
	void WriteBeginMarkerWithQualifier(const char* pszMarker, // Write a begin marker with qualifier
		const char* pszQualifier, const char* pszName = "");
	void WriteEndMarkerWithQualifier(const char* pszMarker, const char* pszQualifier); // Write an end marker with qualifier

	void WriteString(const char* pszMarker, const IUtf8String &sz); // Write string
	void WriteString(const char* pszMarker, const char* pszQualifier, const IUtf8String &sz); // Write marker with qualifier appended and string
	void WriteString(const char* pszMarker, const char* psz); // Write string
	void WriteString(const char* pszMarker, const char* pszQualifier, const char* psz); // Write marker with qualifier appended and string
	void WriteBool(const char* pszMarker, BOOL b); // Write marker if boolean is true
	void WriteCOLORREF(const char* pszMarker, COLORREF rgb); // Write a set of three rgb color numbers
	void WriteInteger(const char* pszMarker, int i, const char* pszComment = NULL); // Write an integer
	void WriteInteger(const char* pszMarker, int i, const IUtf8String &szComment); // Write an integer
	void WriteDouble(const char* pszMarker, double i); // Write an integer
	void WriteUInt(const char* pszMarker, UINT u, const char* pszComment = NULL); // Write a uint
	void WriteUInt(const char* pszMarker, UINT u, const IUtf8String &szComment); // Write a uint

	void WriteNewline(); // Write a blank line, used to separate sections of the file for readability

#ifndef NO_INTERFACE
	void WriteWindowPlacement(const char* pszMarker, WINDOWPLACEMENT& wpl); //SDM 1.5Test8.2
#endif  // not NO_INTERFACE

private:
	ofstream m_ios;  // iostream to which output is written
	void WriteContents(const char* pszContents);

	// Write the field's contents NOT including a newline.
	// "Protect" backslashes at the beginning of lines
	// by writing a space before them.
}; // class Object_ostream


// ==========================================================================

class Object_istream // Hungarian: obs
{
public:
	Object_istream(const IUtf8String &filename);
	~Object_istream(); // Destructor

	ifstream& getIos() {return m_ios;};

	void SkipBOM(); // Skip opening BOM, if present
	BOOL bAtEnd(); // True if at eof
	BOOL bAtBackslash(); // True if at a backslash
	BOOL bAtBeginMarker(); // True if at any begin marker
	BOOL bAtBeginOrEndMarker(); // True if at any begin or end marker
	BOOL bAtBeginMarker(const char* pszMarker, const char* pszName = NULL ); // True if at the desired begin marker followed by the requested name
	BOOL bAtBeginMarker(const char* pszMarker, IUtf8String *pszName); // True if at the desired begin marker followed by the requested name
	BOOL bAtEndMarker(const char* pszMarker); // True if at the desired end marker

	BOOL bReadBeginMarker(const char* pszMarker, IUtf8String *pszName); // Read the desired begin marker (pszMarker with a plus sign)
	BOOL bReadBeginMarker(const char* pszMarker) // Read the desired begin marker (pszMarker with a plus sign)
	{ return bReadMarker('+', pszMarker); }
	BOOL bReadEndMarker(const char* pszMarker) // Read the desired end marker (pszMarker with a minus sign)
	{ return bReadMarker('-', pszMarker); }
	void SkipToEndMarker(const char* pszMarker);
	BOOL bReadBeginMarkerWithQualifier(const char* pszMarker, const char* pszQualifier); // Read the desired begin marker with a qualifier
	BOOL bReadEndMarkerWithQualifier(const char* pszMarker, const char* pszQualifier); // Read the desired end marker with a qualifier

	BOOL bReadString(const char* pszMarker, const char* pszQualifier, IUtf8String* ps); // Read a string with both a marker and a qualifier before it
	BOOL bReadString(const char* pszMarker, IUtf8String* ps); // Read a string
	BOOL bReadString(const IUtf8String& pszMarker, IUtf8String* ps); // Read a string
	BOOL bReadBool(const char* pszMarker, BOOL& b); // Read a boolean. The presence of the marker means TRUE
	BOOL bReadCOLORREF(const char* pszMarker, COLORREF& rgb); // Read a set of three rgb color numbers
	BOOL bReadInteger(const char* pszMarker, int& i); // Read an integer
	BOOL bReadDouble(const char* pszMarker, double& i);
	BOOL bReadUInt(const char* pszMarker, UINT& ui); // Read an unsigned integer (unsigned short)
	BOOL bReadNumberOf(const char* pszMarker, NumberOf& num); // Read a long unsigned integer
	BOOL bReadDWord(const char* pszMarker, DWORD& dw) // Read a DWORD (long unsigned integer)
	{ return bReadNumberOf(pszMarker, dw); }

	BOOL bEnd( const char* pszMarker ); // Read given end marker, stop any begin marker, for error recovery on objects with unrecognized names AB 1-18-95
	BOOL bEndWithQualifier( const char* pszMarker, const char* pszQualifier ); // same as bEnd() but with additional qualifier
	void ReadMarkedString(); // Generic eat of one marker and its string

	void peekMarkedString(const char** ppszMarker, IUtf8String* ps,BOOL bTrimWhiteSpace = TRUE);

#ifndef NO_INTERFACE
	BOOL bReadWindowPlacement(const char* pszMarker, WINDOWPLACEMENT& wpl);
#endif  // not NO_INTERFACE


protected:
	void ReadMarkedString(const char** ppszMarker, const char** ppszString,
		BOOL bTrimWhiteSpace = TRUE); // Read any marked string
	void peekMarkedString(const char** ppszMarker, const char** ppszString,BOOL bTrimWhiteSpace = TRUE)
	{ ReadMarkedString(ppszMarker, ppszString,bTrimWhiteSpace); // Read any marked string

	UnReadMarkedString(); }// Store most recently read marked string for next read
	BOOL Object_istream::bReadString(const char* pszMarker, const char** s);
	void UnReadMarkedString(); // Store most recently read marked string for next read

private:
	void ReadMarkedLine(const char** ppszMarker, const char** ppszString); // Read any marked line
	void ReadLine(); // Read one line

	ifstream m_ios;  // iostream from which input is read
	char* m_pszMStringBuf; // input buffer for a marked string
	char* m_pszEnd; // end of string (i.e. its null) where next line is read
	BOOL m_bUnRead; // buffer contains a marked string which has been "unread"
	char* m_pszMarker; // beginning of unread marker
	char* m_pszString; // beginning of unread string
	char m_chEndOfLine;  // delimiter for the get-one-line function

	BOOL bReadMarker(char cFirstChar, const char* pszMarker); // low level read begin or end marker

}; // class Object_istream

#endif // OBSTREAM_H
