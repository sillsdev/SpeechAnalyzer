#include "Stdafx.h"
#include "TextHelper.h"
#include "Settings/OBSTREAM.H"
#include "resource.h"
#include "AppDefs.h"
#include <iostream>

using std::vector;
using std::ifstream;
using std::streampos;
using std::stringstream;
using std::wstringstream;

bool ReadFileIntoBuffer( LPCTSTR filename, string & unknown) 
{
	ifstream file( filename, std::ios::binary);
    if (!file)
    {
        // file failed to open
        return false;
    }

    // determine size
    file.seekg(0, std::ios::end);
    streampos length = file.tellg();
    file.seekg(0, std::ios::beg);

	unknown = "";
    char c;
    while (file.get(c))
    {
		unknown.push_back(c);
    }

	TRACE("file length=%d\n",unknown.length());
    file.close();
	return true;
}

bool IsUTF8( string & unknown)
{
    //EF BB BF      UTF-8
	if (unknown.length()<3)
    {
        return false;
    }
	unsigned char c1 = unknown[0];
	unsigned char c2 = unknown[1];
	unsigned char c3 = unknown[2];
    bool result = ((c1==0xef) && (c2==0xbb) && (c3==0xbf));
	TRACE("stream %s UTF8\n",((result)?"is":"is not"));
    return result;
}

bool IsUTF16BE( string & unknown)
{
    //FE FF         UTF-16, big-endian
    if (unknown.length()<2)
    {
        return false;
    }
	unsigned char c1 = unknown[0];
	unsigned char c2 = unknown[1];
    if ((c1==0xfe)&&(c2==0xff))
    {
		TRACE("stream is big-endian UTF16\n");
        return true;
    }
	TRACE("stream is not big-endian UTF16\n");
    return false;
}

bool IsUTF16LE( string & unknown)
{
    //FF FE         UTF-16, little-endian
    if (unknown.length()<2)
    {
        return false;
    }
	unsigned char c1 = unknown[0];
	unsigned char c2 = unknown[1];
    if ((c1==0xff)&&(c2==0xfe))
    {
		TRACE("stream is little-endian UTF16\n");
        return true;
    }
	TRACE("stream is not little-endian UTF16\n");
    return false;
}

bool IsASCII( string & unknown)
{
	for (int i=0; i<unknown.length(); i++)
    {
		unsigned char c = unknown[i];
        if (c>0x7f)
        {
			TRACE("stream is not ASCII\n");
            return false;
        }
    }
	TRACE("stream is ASCII\n");
    return true;
}

/**
* converts UTF16 big endian buffer to a UTF16 array
* if the function returns true, obuffer will be a pointer to the newly created array
* the caller is responsible for deleting buffer.
*/
bool ConvertUTF16BEToUTF16( string & unknown, wstring & obuffer) {

	// remove BOM
	unknown.erase(unknown.begin());
	unknown.erase(unknown.begin());

	// swap the bytes
	for (int i=0;i<unknown.length();) {
		unsigned c1 = unknown[i];
		unsigned c2 = unknown[i+1];
		unknown[i] = c2;
		unknown[i+1] = c1;
		i+=2;
	}

	obuffer = L"";
	obuffer.resize(unknown.length()/2);
	memcpy(&obuffer[0],unknown.data(),unknown.length());

	return (obuffer.length()!=0);
}

/**
* converts UTF16 little endian buffer to a UTF16 array
* if the function returns true, obuffer will be a pointer to the newly created array
* the caller is responsible for deleting buffer.
*/
bool ConvertUTF16LEToUTF16( string & unknown, wstring & obuffer) {

	// remove BOM
	unknown.erase(unknown.begin());
	unknown.erase(unknown.begin());

	obuffer = L"";
	obuffer.resize(unknown.length()/2);
	memcpy(&obuffer[0],unknown.data(),unknown.length());

	return (obuffer.length()!=0);
}

/**
* converts buffer to a UTF16 array
* if the function returns true, obuffer will be a pointer to the newly created array
* the caller is responsible for deleting buffer.
*/
bool ConvertUTF8ToUTF16( string & unknown, wstring & obuffer) {

	// remove the BOM
	unknown.erase(unknown.begin());
	unknown.erase(unknown.begin());
	unknown.erase(unknown.begin());
	obuffer=L"";
    int length2 = MultiByteToWideChar(CP_UTF8,0,unknown.data(),unknown.length(),NULL,0);
	if (length2==0) return false;
	obuffer.resize(length2);
    length2 = MultiByteToWideChar(CP_UTF8,0,unknown.data(),unknown.length(),&obuffer[0],obuffer.length());
	return (length2!=0);
}

/**
* converts buffer to a UTF16 array
* if the function returns true, obuffer will be a pointer to the newly created array
* the caller is responsible for deleting buffer.
*/
bool ConvertASCIIToUTF16( string & unknown, wstring & obuffer) {

	obuffer=L"";
    int length2 = MultiByteToWideChar(CP_ACP,0,unknown.data(),unknown.length(),NULL,0);
	if (length2==0) return false;
	obuffer.resize(length2);
    length2 = MultiByteToWideChar(CP_ACP,0,unknown.data(),unknown.length(),&obuffer[0],obuffer.length());
	return (length2!=0);
}

bool CheckEncoding( LPCTSTR filename, bool display) {

	string unknown;
	if (!ReadFileIntoBuffer( filename, unknown)) {
		if (display) {
			CString msg;
			msg.FormatMessage(IDS_ERROR_FILENAME,filename);
			AfxMessageBox(msg, MB_OK|MB_ICONEXCLAMATION, 0);
		}
		return false;
	}

	if (IsUTF8( unknown)) {
		return true;
	} else if (IsUTF16BE(unknown)) {
		if (display) {
			AfxMessageBox(IDS_WARN_CONVERT, MB_OK|MB_ICONEXCLAMATION);
		}
		return true;
	} else if (IsUTF16LE(unknown)) {
		if (display) {
			AfxMessageBox(IDS_WARN_CONVERT, MB_OK|MB_ICONEXCLAMATION);
		}
		return true;
	} else if (IsASCII(unknown)) {
		return true;
	}
	return true;
}

bool ConvertFileToUTF16( LPCTSTR filename, wstring & obuffer) {

	string unknown;
	if (!ReadFileIntoBuffer( filename, unknown)) {
		return false;
	}
	if (IsUTF8( unknown)) {
		return ConvertUTF8ToUTF16( unknown, obuffer);
	} else if (IsUTF16BE(unknown)) {
		return ConvertUTF16BEToUTF16( unknown, obuffer);
	} else if (IsUTF16LE(unknown)) {
		return ConvertUTF16LEToUTF16( unknown, obuffer);
	} else if (IsASCII(unknown)) {
		return ConvertASCIIToUTF16( unknown, obuffer);
	}
	// assume UTF8
	return ConvertUTF8ToUTF16( unknown, obuffer);
}

vector<wstring> TokenizeBufferToLines( wstring buffer) {

    // now tokenize the buffer
    vector<wstring> lines;

	wstring line;
    for (size_t i=0; i<buffer.length(); i++)
    {
        wchar_t c = buffer[i];
        if ((c==0x0d)||(c==0x0a))
        {
            if (line.size()>0)
            {
                lines.push_back(line);
            }
            line.clear();
        }
        else
        {
            line.push_back(c);
        }
    }
	return lines;
}

vector<wstring> TokenizeLineToTokens( wstring & line, wchar_t token) {

    // now tokenize the buffer
    vector<wstring> lines;

	wstring sub;
    for (size_t i=0; i<line.length(); i++)
    {
        wchar_t c = line[i];
        if (c==token)
        {
            lines.push_back(sub);
            sub.clear();
        }
        else
        {
            sub.push_back(c);
        }
    }
	if (sub.size()>0) 
	{
		lines.push_back(sub);
	}
	return lines;
}

bool ContainsTabs(wchar_t * buffer, int length)
{
    for (int i=0; i<length; i++)
    {
        if (buffer[i]==0x09)
        {
            return true;
        }
    }
    return false;
}

bool ContainsPoundSigns(wchar_t * buffer, int length)
{
    for (int i=0; i<length; i++)
    {
        if (buffer[i]==WORD_DELIMITER)
        {
            return true;
        }
    }
    return false;
}

bool ContainsUnderscore(wchar_t * buffer, int length)
{
    for (int i=0; i<length; i++)
    {
        if (buffer[i]=='_')
        {
            return true;
        }
    }
    return false;
}

/**
* attempt to pull in a tab delimited file, the reference column only
*/
TranscriptionDataMap AttemptTabDelimitedRefOnly(const vector<wstring> & lines, const MarkerList & markers)
{

    TranscriptionDataMap map;

    if (markers.size()!=1)
    {
        TRACE(L"Skipping TabDelimitedRefOnly because there's more than one column\n");
        return map;
    }

    // now tokenize the lines
    vector<vector<wstring>> list2;
    vector<wstring> tokens;
    wstring token;
    size_t min = 100000;
    size_t max = 0;
    for (vector<wstring>::const_iterator it = lines.begin(); it!=lines.end(); it++)
    {
        wstring line = *it;
        for (size_t i=0; i<line.size(); i++)
        {
            wchar_t c = line[i];
            if (line[i]==0x09)
            {
                tokens.push_back(token);
                token.clear();
            }
            else
            {
                token.push_back(c);
            }
        }
        tokens.push_back(token);
        token.clear();

        min = (tokens.size()<min)?tokens.size():min;
        max = (tokens.size()>max)?tokens.size():max;
        list2.push_back(tokens);
        tokens.clear();
    }

    // count the tokens if the lengths are the same then this is a simple list...
    // and the length matches the marker size, this is as easy as it gets!
    if (min!=max)
    {
        TRACE(L"Skipping TabDelimitedRefOnly because there's a discrepancy in the list\n");
        return map;
    }

    for (vector<vector<wstring>>::iterator it = list2.begin(); it!=list2.end(); it++)
    {
        vector<wstring> tokens = *it;
        vector<wstring>::iterator it2 = tokens.begin();
        MarkerList::const_iterator it3 = markers.begin();
        while (it3!=markers.end())
        {
            CSaString marker = *it3;
            wstring token = *it2;
            map[marker].push_back(CSaString(token.c_str()));
            it2++;
            it3++;
        }
    }
    return map;
}

TranscriptionDataMap AttemptTabDelimited(vector<wstring> lines, MarkerList markers)
{

    TranscriptionDataMap map;

    // now tokenize the lines
    vector<vector<wstring>> list2;
    vector<wstring> tokens;
    wstring token;
    size_t min = 100000;
    size_t max = 0;
    for (vector<wstring>::iterator it = lines.begin(); it!=lines.end(); it++)
    {
        wstring line = *it;
        for (size_t i=0; i<line.size(); i++)
        {
            wchar_t c = line[i];
            if (line[i]==0x09)
            {
                tokens.push_back(token);
                token.clear();
            }
            else
            {
                token.push_back(c);
            }
        }
        tokens.push_back(token);
        token.clear();

        min = (tokens.size()<min)?tokens.size():min;
        max = (tokens.size()>max)?tokens.size():max;
        list2.push_back(tokens);
        tokens.clear();
    }

    // count the tokens if the lengths are the same then this is a simple list...
    // and the length matches the marker size, this is as easy as it gets!
    if (min!=max)
    {
        TRACE(L"Skipping TabDelimited because there's a discrepancy in the list\n");
        return map;
    }
    if (min!=markers.size())
    {
        TRACE(L"Skipping TabDelimited because a line is missing data\n");
        return map;
    }

    for (vector<vector<wstring>>::iterator it = list2.begin(); it!=list2.end(); it++)
    {
        vector<wstring> tokens = *it;
        vector<wstring>::iterator it2 = tokens.begin();
        MarkerList::const_iterator it3 = markers.begin();
        while (it3!=markers.end())
        {
            CSaString marker = *it3;
            wstring token = *it2;
            map[marker].push_back(CSaString(token.c_str()));
            it2++;
            it3++;
        }
    }
    return map;
}

TranscriptionDataMap AttemptWhitespaceDelimited(vector<wstring> lines, MarkerList markers)
{

    TranscriptionDataMap map;

    // now tokenize the lines
    vector<vector<wstring>> list2;
    vector<wstring> tokens;
    wstring token;
    size_t min = 100000;
    size_t max = 0;
    for (vector<wstring>::iterator it = lines.begin(); it!=lines.end(); it++)
    {
        wstring line = *it;
        for (size_t i=0; i<line.size(); i++)
        {
            wchar_t c = line[i];
            if ((line[i]==0x20)||(line[i]==0x09))
            {
                tokens.push_back(token);
                token.clear();
            }
            else
            {
                token.push_back(c);
            }
        }
        tokens.push_back(token);
        token.clear();

        min = (tokens.size()<min)?tokens.size():min;
        max = (tokens.size()>max)?tokens.size():max;
        list2.push_back(tokens);
        tokens.clear();
    }

    // count the tokens if the lengths are the same then this is a simple list...
    // and the length matches the marker size, this is as easy as it gets!
    if (min!=max)
    {
        TRACE(L"Skipping TabDelimitedRefOnly because there's missing data\n");
        return map;
    }
    if (min!=markers.size())
    {
        TRACE(L"Skipping TabDelimitedRefOnly because there's a discrepancy in the list\n");
        return map;
    }

    for (vector<vector<wstring>>::iterator it = list2.begin(); it!=list2.end(); it++)
    {
        vector<wstring> tokens = *it;
        vector<wstring>::iterator it2 = tokens.begin();
        MarkerList::const_iterator it3 = markers.begin();
        while (it3!=markers.end())
        {
            CSaString marker = *it3;
            wstring token = *it2;
            map[marker].push_back(CSaString(token.c_str()));
            it2++;
            it3++;
        }
    }
    return map;
}

TranscriptionDataMap AttemptTwoMarkerWhitespaceDelimited(vector<wstring> lines, MarkerList markers)
{

    TranscriptionDataMap map;

    if (markers.size()!=2)
    {
        TRACE(L"Skipping AttemptTwoMarkerWhitespaceDelimited because marker size != 2\n");
        return map;
    }

    // now tokenize the lines
    vector<vector<wstring>> list2;
    vector<wstring> tokens;
    wstring token;
    size_t min = 100000;
    size_t max = 0;
    // only delimit between the first and second token
    for (vector<wstring>::iterator it = lines.begin(); it!=lines.end(); it++)
    {
        bool first = true;
        wstring line = *it;
        for (size_t i=0; i<line.size(); i++)
        {
            wchar_t c = line[i];
            if ((first)&&((line[i]==0x20)||(line[i]==0x09)))
            {
                tokens.push_back(token);
                token.clear();
                first = false;
            }
            else
            {
                token.push_back(c);
            }
        }
        tokens.push_back(token);
        token.clear();

        min = (tokens.size()<min)?tokens.size():min;
        max = (tokens.size()>max)?tokens.size():max;
        list2.push_back(tokens);
        tokens.clear();
    }

    // we are only expecting two tokens per line
    if (min!=markers.size())
    {
        TRACE("Skipping AttemptTwoMarkerWhitespaceDelimited because there's missing data\n");
        return map;
    }
    if (max<markers.size())
    {
        return map;
    }

    for (vector<vector<wstring>>::iterator it = list2.begin(); it!=list2.end(); it++)
    {
        vector<wstring> tokens = *it;
        vector<wstring>::iterator it2 = tokens.begin();
        MarkerList::const_iterator it3 = markers.begin();
        while (it3!=markers.end())
        {
            CSaString marker = *it3;
            wstring token = *it2;
            map[marker].push_back(CSaString(token.c_str()));
            it2++;
            it3++;
        }
    }
    return map;
}

/**
* addTag - true if we should add 'Tag' to the beginning of reference markers
*/
bool CTextHelper::ImportText(const CSaString & filename,
                             const CSaString & sync,
                             const MarkerList & markers,
                             TranscriptionDataMap & map,
                             bool addTag)
{
	wstring obuffer;
	if (!ConvertFileToUTF16( filename, obuffer))
	{
		return false;
	}

	vector<wstring> lines = TokenizeBufferToLines( obuffer);

    CString tag;
    tag.LoadStringW(IDS_AUTO_REF_TAG);

    map = AttemptTabDelimitedRefOnly(lines,markers);
    if (map.size()>0)
    {
        if (addTag)
        {
            map[sync].push_front(tag);
        }
        return true;
    }
    map = AttemptTabDelimited(lines,markers);
    if (map.size()>0)
    {
        if (addTag)
        {
            map[sync].push_front(tag);
        }
        return true;
    }
    map = AttemptWhitespaceDelimited(lines,markers);
    if (map.size()>0)
    {
        if (addTag)
        {
            map[sync].push_front(tag);
        }
        return true;
    }
    map = AttemptTwoMarkerWhitespaceDelimited(lines,markers);
    if (map.size()>0)
    {
        if (addTag)
        {
            map[sync].push_front(tag);
        }
        return true;
    }

    for (MarkerList::const_iterator it=markers.begin(); it!=markers.end(); it++)
    {
        map[*it].push_back("");
    }

    return false;
}

vector<string> Tokenize(const string & str, const string & delimiters)
{

    vector<string> tokens;
    string::size_type delimPos = 0;
    string::size_type tokenPos = 0;
    string::size_type pos = 0;

    if (str.length()<1)
    {
        return tokens;
    }
    while (1)
    {
        delimPos = str.find_first_of(delimiters, pos);
        tokenPos = str.find_first_not_of(delimiters, pos);

        if (string::npos != delimPos)
        {
            if (string::npos != tokenPos)
            {
                if (tokenPos<delimPos)
                {
                    tokens.push_back(str.substr(pos,delimPos-pos));
                }
                else
                {
                    tokens.push_back("");
                }
            }
            else
            {
                tokens.push_back("");
            }
            pos = delimPos+1;
        }
        else
        {
            if (string::npos != tokenPos)
            {
                tokens.push_back(str.substr(pos));
            }
            else
            {
                tokens.push_back("");
            }
            break;
        }
    }
    return tokens;
}

