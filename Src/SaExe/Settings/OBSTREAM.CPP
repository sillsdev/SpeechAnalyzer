/////////////////////////////////////////////////////////////////////////////
// obstream.cpp
// Implementation of textual iostreams of objects (1995-07-22)
// copyright 1995 JAARS Inc. SIL
//
// Revision History
// 1.5Test8.2
//        SDM Added WriteWindowPlacement(const char*,WINDOWPLACEMENT&)
/////////////////////////////////////////////////////////////////////////////
#include "stdSettings.h"
#include <windows.h>
#include <fstream>
#include "..\IUtf8String.h"
#include "obstream.h"

using std::ofstream;
using std::ifstream;
using std::ios;

#include "patch.h"  // Shw_bAtWhiteSpace, Shw_pszOtherWhiteSpace
#include "defs.h"

// **************************************************************************

Object_ostream::Object_ostream(const IUtf8String &filename) : m_ios()
{
	std::string szUtf8 = filename.utf8();

	m_ios.open(szUtf8.c_str());
}

void Object_ostream::WriteBeginMarker(const char* pszMarker, const IUtf8String &szName)
{
	std::string sName = szName.utf8();
	WriteBeginMarker(pszMarker, sName.c_str());
}

void Object_ostream::WriteBeginMarker(const char* pszMarker, const char* pszName)
{
	ASSERT( pszName );
	//  ASSERT( *pszName );
	m_ios << "\\+" << pszMarker << ' ' << pszName << '\n';
}

void Object_ostream::WriteEndMarker(const char* pszMarker)
{
	m_ios << "\\-" << pszMarker << '\n';
}

void Object_ostream::WriteBeginMarkerWithQualifier(const char* pszMarker, const char* pszQualifier,
												   const char* pszName)
{
	if ( !pszQualifier )
	{
		WriteBeginMarker( pszMarker, pszName );
		return;
	}

	ASSERT( pszName );
	m_ios << "\\+" << pszMarker << pszQualifier << ' ' << pszName << '\n';
}

void Object_ostream::WriteEndMarkerWithQualifier(const char* pszMarker, const char* pszQualifier)
{
	if ( !pszQualifier )
	{
		WriteEndMarker( pszMarker );
		return;
	}

	m_ios << "\\-" << pszMarker << pszQualifier << '\n';
}


void Object_ostream::WriteString(const char* pszMarker, const char* pszQualifier,
								 const char* psz)
{
	ASSERT( psz );
	if ( !*psz )
		return;

	m_ios << '\\' << pszMarker << pszQualifier << ' ';
	WriteContents(psz);
	m_ios << '\n';
}

void Object_ostream::WriteString(const char* pszMarker, const IUtf8String &sz)
{
	std::string theString = sz.utf8();
	const char* psz = theString.c_str();
	WriteString(pszMarker, psz);
}

void Object_ostream::WriteString(const char* pszMarker, const char* psz)
{
	ASSERT( psz );
	if ( !*psz )
		return;

	m_ios << '\\' << pszMarker << ' ';
	WriteContents(psz);
	m_ios << '\n';
}

void Object_ostream::WriteContents(const char* pszContents)
{
	const char* psz = pszContents;
	ASSERT( psz );
	const char* pszBackslashAtBOL = NULL;
	while ( (pszBackslashAtBOL = strstr(psz, "\n\\")) != NULL )
	{
		pszBackslashAtBOL += 1;  // Point to the backslash;
		// Write the substring up to and including the newline
		m_ios.write(psz, pszBackslashAtBOL - psz);
		m_ios << ' ';  // Write a space before the backslash
		psz = pszBackslashAtBOL;
		ASSERT( *psz == '\\' );
	}
	m_ios << psz;  // Write the rest of the field
}

void Object_ostream::WriteBool(const char* pszMarker, BOOL b)
{
	m_ios << '\\' << pszMarker << ' ' << ((b) ? 1 : 0) << '\n';
}

void Object_ostream::WriteCOLORREF(const char* pszMarker, COLORREF rgb)
{
	m_ios << '\\' << pszMarker << ' ' <<
		(int)GetRValue(rgb) << ',' << (int)GetGValue(rgb) << ',' << (int)GetBValue(rgb) << '\n';
}

void Object_ostream::WriteInteger(const char* pszMarker, int i, const char* pszComment)
{
	m_ios << '\\' << pszMarker << ' ' << i;
	if(pszComment)
		m_ios << " // " << pszComment;
	m_ios << '\n';
}


void Object_ostream::WriteDouble(const char* pszMarker, double i)
{
	int oldPrecision = m_ios.precision(20); // Use as many digits as necessary, 20 is the most we should see
	long oldFlags = m_ios.flags();
	m_ios.unsetf(ios::showpoint);
	m_ios << '\\' << pszMarker << ' ' << i << '\n';

	m_ios.precision(oldPrecision);
	oldFlags = m_ios.flags(oldFlags);
}


void Object_ostream::WriteUInt(const char* pszMarker, UINT u, const IUtf8String &szComment)
{
	std::string sComment = szComment.utf8();
	WriteUInt(pszMarker, u, sComment.c_str());
}

void Object_ostream::WriteUInt(const char* pszMarker, UINT u, const char* pszComment)
{
	m_ios << '\\' << pszMarker << ' ' << u;
	if(pszComment)
		m_ios << " //" << pszComment;
	m_ios << '\n';
}


void Object_ostream::WriteNewline()
{
	m_ios << '\n';
}

void Object_ostream::WriteWindowPlacement(const char* pszMarker, WINDOWPLACEMENT& wpl)
{
	char* pszState = "max";
	switch ( wpl.showCmd )
	{
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
		ASSERT( FALSE );
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


// ==========================================================================

static const Length maxsizMString = 32767;

Object_istream::Object_istream(const IUtf8String &filename) : m_ios()
{
	std::string szUtf8 = filename.utf8();

	m_ios.open(szUtf8.c_str());

	m_pszMStringBuf = new char[maxsizMString];
	m_pszEnd = NULL;
	m_pszMarker = NULL;
	m_pszString = NULL;
	m_bUnRead = FALSE;
	m_chEndOfLine = '\n';  // Default for Mac and Windows
}

Object_istream::~Object_istream()
{
	delete [] m_pszMStringBuf;
}


void Object_istream::SkipBOM()
{
	char nextChar;

	for (int i = 0; i < 3; i++)
	{
		if (bAtBackslash())
			return;
		else
			m_ios.get(nextChar);
	}
}

BOOL Object_istream::bAtEnd()
{
	// MRP: TO DO!!! Check whether the following (and bReadField) is right...
	// 1. If marker or contents at EOF with no newline;
	// 2. If explicit Ctrl-Z (code 26) in file.

	// Have read to end if no "unread" field, and all chars have been read
	// from the stream. NOTE: Because no attempt has yet been made actually
	// to get chars from beyond the end of the stream, eof() will NOT be true.
	return ( !m_bUnRead && m_ios.peek() == EOF );
}

BOOL Object_istream::bAtBackslash()
{
	// Stream represents Standard Format if a field has already been read
	// successfully, or if the first char of the stream is a backslash.
	return ( m_bUnRead || m_ios.peek() == '\\' );
}

BOOL Object_istream::bAtBeginMarker()
{
	const char* pszMarkerRead = NULL;
	const char* pszStringRead = NULL;
	ReadMarkedString(&pszMarkerRead, &pszStringRead);

	BOOL bBeginMarker = ( *pszMarkerRead == '+' );
	UnReadMarkedString();

	return bBeginMarker;
}

BOOL Object_istream::bAtBeginOrEndMarker()
{
	const char* pszMarkerRead = NULL;
	const char* pszStringRead = NULL;
	ReadMarkedString(&pszMarkerRead, &pszStringRead);

	BOOL bBeginOrEndMarker = ( *pszMarkerRead == '+' || *pszMarkerRead == '-' );
	UnReadMarkedString();

	return bBeginOrEndMarker;
}

BOOL Object_istream::bAtBeginMarker(const char* pszMarker, const char* pszName )
{
	const char* pszMarkerRead = NULL;
	const char* pszStringRead = NULL;
	if ( !bReadBeginMarker(pszMarker) )
		return FALSE;

	UnReadMarkedString();
	ReadMarkedString(&pszMarkerRead, &pszStringRead);

	BOOL b = !pszName || !strcmp( pszName, pszStringRead ); // If name given, it must match
	UnReadMarkedString();
	return b;
}

BOOL Object_istream::bAtEndMarker(const char* pszMarker)
{
	if ( !bReadEndMarker(pszMarker) )
		return FALSE;

	UnReadMarkedString();
	return TRUE;
}


BOOL Object_istream::bReadBeginMarker(const char* pszMarker, IUtf8String *psName)
{
	const char* pszMarkerRead = NULL;
	const char* pszStringRead = NULL;
	ReadMarkedString(&pszMarkerRead, &pszStringRead);

	BOOL bBeginMarker = (*pszMarkerRead == '+') && bEqual(pszMarker, pszMarkerRead+1);
	if ( !bBeginMarker )
	{
		UnReadMarkedString();
		return FALSE;
	}

	psName->setUtf8(pszStringRead);
	return TRUE;
}

BOOL Object_istream::bReadMarker(char cFirstChar, const char* pszMarker)
{
	const char* pszMarkerRead = NULL;
	const char* pszStringRead = NULL;
	ReadMarkedString(&pszMarkerRead, &pszStringRead);

	BOOL bMarkerFound = (*pszMarkerRead == cFirstChar) && bEqual(pszMarker, pszMarkerRead+1);
	if ( !bMarkerFound )
	{
		UnReadMarkedString();
		return FALSE;
	}

	return TRUE;
}

#ifdef MOVED_TO_HFILE_FOR_INLINING__BJY_1_25_96

BOOL Object_istream::bReadBeginMarker(const char* pszMarker)
{
	return bReadMarker('+', pszMarker);
}

BOOL Object_istream::bReadEndMarker(const char* pszMarker)
{
	return bReadMarker('-', pszMarker);
}

#endif

#define MAXMKRSIZE 50

BOOL Object_istream::bReadBeginMarkerWithQualifier(const char* pszMarker, const char* pszQualifier)
{
	if ( !pszQualifier )
		return bReadBeginMarker( pszMarker );

	char buf[MAXMKRSIZE];
	ASSERT( strlen(pszMarker) + strlen(pszQualifier) + 1 < MAXMKRSIZE );
	strcpy_s( buf, _countof(buf), pszMarker );
	strcat_s( buf, _countof(buf), pszQualifier );
	return bReadMarker('+', buf);
}

BOOL Object_istream::bReadEndMarkerWithQualifier(const char* pszMarker, const char* pszQualifier)
{
	if ( !pszQualifier )
		return bReadEndMarker( pszMarker );

	char buf[MAXMKRSIZE];
	ASSERT( strlen(pszMarker) + strlen(pszQualifier) + 1 < MAXMKRSIZE );
	strcpy_s( buf, _countof(buf), pszMarker );
	strcat_s( buf, _countof(buf), pszQualifier );
	return bReadMarker('-', buf);
}

BOOL Object_istream::bReadString(const IUtf8String &szMarker, IUtf8String* s)
{
	std::string szUtf8 = szMarker.utf8();

	return bReadString(szUtf8.c_str(), s);
}

BOOL Object_istream::bReadString(const char* pszMarker, IUtf8String* s)
{
	const char * read;

	BOOL result = bReadString(pszMarker, &read);

	if(result)
		s->setUtf8(read);

	return result;
}

BOOL Object_istream::bReadString(const char* pszMarker, const char** s)
{
	const char* pszMarkerRead;
	const char* pszStringRead = NULL;
	ReadMarkedString(&pszMarkerRead, &pszStringRead);

	if ( !bEqual(pszMarker, pszMarkerRead) )
	{
		UnReadMarkedString();
		return FALSE;
	}

	*s = pszStringRead;
	return TRUE;
}

BOOL Object_istream::bReadBool(const char* pszMarker, BOOL& b)
{
	const char* s;
	if ( !bReadString(pszMarker, &s) )
		return FALSE;

	b = (atoi(s) != 0);
	return TRUE;
}

BOOL Object_istream::bReadCOLORREF(const char* pszMarker, COLORREF& rgb)
{
	const char* s;
	if ( !bReadString(pszMarker, &s) )
		return FALSE;

	short int r,g,b;
	// 1997-05-17 MRP: sscanf not defined in DLLs -- reimplement
	if ( sscanf_s(s, "%hd,%hd,%hd", &r, &g, &b) == 0 )
		return FALSE;

	rgb = RGB(r,g,b);
	return TRUE;
}

BOOL Object_istream::bReadInteger(const char* pszMarker, int& i)
{
	const char* s;
	if ( !bReadString(pszMarker, &s) )
		return FALSE;

	i = atoi(s);
	return TRUE;
}



BOOL Object_istream::bReadDouble(const char* pszMarker, double& i)
{
	const char* s;
	if ( !bReadString(pszMarker, &s) )
		return FALSE;

	i = atof(s);
	return TRUE;
}



BOOL Object_istream::bReadNumberOf(const char* pszMarker, NumberOf& num)
{
	const char* s;
	if ( !bReadString(pszMarker, &s) )
		return FALSE;

	ASSERT( sizeof(unsigned long) == sizeof(NumberOf) );
	if ( sscanf_s(s, "%lu", &num) == 0 )
		return FALSE;

	return TRUE;
}



void Object_istream::peekMarkedString(const char** ppszMarker, IUtf8String* ppszString,BOOL bTrimWhiteSpace)
{
	const char* read = 0;
	peekMarkedString(ppszMarker, &read,bTrimWhiteSpace);

	ppszString->setUtf8(read);
}

void Object_istream::ReadMarkedString() // Generic eat of one marker and its string
{
	const char* pszMarkerRead;
	const char* pszStringRead = NULL;
	ReadMarkedString(&pszMarkerRead, &pszStringRead);
}

BOOL Object_istream::bEnd( const char* pszMarker ) // Read given end marker, stop any begin marker, for error recovery on objects with unrecognized names AB 1-18-95
{
	if ( bReadEndMarker( pszMarker ) ) // If desired end marker, return true for successfully found end
		return TRUE;
	else if ( bAtBeginMarker() ) // If somebody else's begin marker, return true for found end
		return TRUE;
	else // Else, unrecognized marker, eat marker and return false for not at end yet
	{
		const char* pszMarkerRead = NULL;
		const char* pszStringRead = NULL;
		ReadMarkedString(&pszMarkerRead, &pszStringRead);
		return FALSE;
	}
}

BOOL Object_istream::bEndWithQualifier( const char* pszMarker, const char* pszQualifier )
{
	if ( !pszQualifier )
		return bEnd( pszMarker );

	char buf[MAXMKRSIZE];
	ASSERT( strlen(pszMarker) + strlen(pszQualifier) + 1 < MAXMKRSIZE );
	strcpy_s( buf, _countof(buf), pszMarker );
	strcat_s( buf, _countof(buf), pszQualifier );
	return bEnd( buf );
}

#ifndef NO_INTERFACE
BOOL Object_istream::bReadWindowPlacement(const char* pszMarker, WINDOWPLACEMENT& wpl)
{
	const char* sPlacement;
	if ( !bReadString(pszMarker, &sPlacement) )
		return FALSE;

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

	if ( nItems != 9 )
		return FALSE;

	if ( bEqual(pszShowState, "normal") )
		wpl.showCmd = SW_SHOWNORMAL;
	else if ( bEqual(pszShowState, "min") )
		wpl.showCmd = SW_SHOWMINIMIZED;
	else if ( bEqual(pszShowState, "max") )
		wpl.showCmd = SW_SHOWMAXIMIZED;
	else
		wpl.showCmd = SW_SHOWNORMAL;;

	return TRUE;
}
#endif  // not NO_INTERFACE


void Object_istream::ReadMarkedString(const char** ppszMarker,
									  const char** ppszString, BOOL bTrimWhiteSpace)
{
	if ( m_bUnRead )
	{
		*ppszMarker = m_pszMarker;
		*ppszString = m_pszString;
		m_bUnRead = FALSE;
		return;
	}

	ReadMarkedLine(ppszMarker, ppszString);

	// Read any additional lines of the marked string
	int ich;
	while ( ((ich = m_ios.peek()) != EOF) && (ich != '\\') )
	{
		*m_pszEnd++ = '\n';
		ReadLine();
	}

	if ( bTrimWhiteSpace )
	{
		// 1995-10-02 MRP: Clean this up and separate out for use of
		// settings properties but *not* Standard Format data fields.
		// Trim white space from the beginning and end of the string
		for ( ; Shw_bAtWhiteSpace(m_pszString); m_pszString += 1 )
		{
			if (*m_pszString == 0)
				break;
		}
		if ( *m_pszString )  // string contains non-ws
		{
			char* psz = m_pszString + strlen(m_pszString) - 1;  // last char
			for ( ; Shw_bAtWhiteSpace(psz); psz -= 1 )
			{
				ASSERT( psz != m_pszString );  // have not moved past beginning
				*psz = '\0';  // remove last char
			}
		}
	}

	*ppszString = m_pszString;
}

void Object_istream::UnReadMarkedString()
{
	ASSERT( !m_bUnRead );
	m_bUnRead = TRUE;
}


void Object_istream::ReadMarkedLine(const char** ppszMarker, const char** ppszString)
{
	ASSERT( !m_bUnRead );
	char* psz = m_pszEnd = m_pszMStringBuf;
	*psz = '\0';
	ReadLine();

	// Caller is responsible to not to read to end, and also to check that
	// the file is indeed Standard Format (i.e. it begins with a backslash)
	ASSERT( *psz == '\\' );
	psz += 1; // move past the backslash

	m_pszMarker = psz;
	Length lenMarker = strcspn(psz, " \t\n");
	// 1996-11-04 MRP: This temporary patch will skip the rest of a field
	// that contains a backslash at the beginning of one of its lines.
	// The real fix is to use the read-line-ahead approach in sfstream.cpp
	// ASSERT( lenMarker != 0 ); // RECOVERY
	psz += lenMarker;

	if ( *psz == ' ' )
	{
		// A space delimiting the marker is not considered part of the string
		*psz = '\0'; // so the marker's null termination can take its place
		psz += 1;
	}
	else
	{
		// 1995-10-04 MRP: At first I thought this shifting was redundant
		// if no white space followed the marker. If the field consisted
		// of just that one line that would be true. If, however, the
		// field's content follows in additional lines we must do it.
		//
		// Move the marker left one position (covering the backslash)
		m_pszMarker = m_pszMStringBuf;
		memcpy(m_pszMarker, m_pszMStringBuf + 1, lenMarker);
		ASSERT( m_pszMarker + lenMarker == psz - 1 );
		m_pszMarker[lenMarker] = '\0'; // making a place for its null
	}

	*ppszMarker = m_pszMarker; // beginning of marker
	*ppszString = m_pszString = psz; // beginning of string
}

void Object_istream::ReadLine()
{
	ASSERT( !m_bUnRead );
	// 1995-04-12 MRP: use get in order to detect line longer than buffer
	Length sizRemaining = maxsizMString - (m_pszEnd - m_pszMStringBuf);
	m_ios.get(m_pszEnd, sizRemaining, m_chEndOfLine);
	m_ios.clear(); // if get encounters an empty line it will set fail bit, must be cleared
	size_t lenLine = strlen(m_pszEnd);
	m_pszEnd += lenLine;
	int ichNext = m_ios.get();
	if ( (ichNext != m_chEndOfLine) && (ichNext != EOF) )
		ASSERT( FALSE ); // at end of input buffer-- RECOVERY
}

// **************************************************************************

BOOL Object_istream::bReadUInt(const char* pszMarker, UINT& ui) // Read an unsigned integer (unsigned short)
{
	const char* s;
	if ( !bReadString(pszMarker, &s) )
		return FALSE;

	if ( sscanf_s(s, "%hu", &ui) == 0 )
		return FALSE;

	return TRUE;
}

void Object_istream::SkipToEndMarker(const char* pszMarker)
{
	while ( !bAtEnd() )
	{
		if ( bReadEndMarker(pszMarker) )
		{
			break;
		}
		ReadMarkedString();  // Skip unexpected field
	}
}
