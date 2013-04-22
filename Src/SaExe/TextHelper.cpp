#include "Stdafx.h"
#include "TextHelper.h"
#include "Settings/OBSTREAM.H"
#include "resource.h"
#include "AppDefs.h"
#include <iostream>

using std::vector;
using std::ifstream;
using std::streampos;

bool ReadFileIntoBuffer( LPCTSTR filename, char ** buffer, streampos & length)
{
	length = 0;
	*buffer = NULL;

    ifstream file( filename, std::ios::binary);
    if (!file)
    {
        // file failed to open
        return false;
    }

    // determine size
    file.seekg(0, std::ios::end);
    length = file.tellg();
    file.seekg(0, std::ios::beg);

	// read the file completely
    *buffer = new char[length];
	char * ptr = *buffer;
    int i=0;
    char c;
    while (file.get(c))
    {
        ptr[i++] = c;
    }
    file.close();
	return true;
}

bool IsUTF8( BYTE * buffer, std::ios::pos_type length)
{
    //EF BB BF      UTF-8
    if (length<3)
    {
        return false;
    }
    bool result = ((buffer[0]==0xef)&(buffer[1]==0xbb)&(buffer[2]==0xbf));
    return result;
}

bool IsUTF16(BYTE * buffer, std::ios::pos_type length)
{
    //FE FF         UTF-16, big-endian
    //FF FE         UTF-16, little-endian
    if (length<2)
    {
        return false;
    }
    if ((buffer[0]==0xfe)&&(buffer[1]==0xff))
    {
        return true;
    }
    if ((buffer[0]==0xff)&&(buffer[1]==0xfe))
    {
        return true;
    }
    return false;
}

bool IsUTF32(BYTE * buffer, std::ios::pos_type length)
{
    //00 00 FE FF       UTF-32, big-endian
    //FF FE 00 00       UTF-32, little-endian
    if (length<4)
    {
        return false;
    }
    if ((buffer[0]==0)&&(buffer[1]==0)&&(buffer[2]==0xfe)&&(buffer[3]==0xff))
    {
        return true;
    }
    if ((buffer[0]==0xff)&&(buffer[1]==0xfe)&&(buffer[2]==0x00)&&(buffer[3]==0x00))
    {
        return true;
    }
    return false;
}

bool IsASCII(BYTE * buffer, std::ios::pos_type length)
{
    for (int i=0; i<length; i++)
    {
        if (buffer[i]>0x7f)
        {
            return false;
        }
    }
    return true;
}

/**
* converts buffer to a UTF16 array
* if the function returns true, obuffer will be a pointer to the newly created array
* the caller is responsible for deleting buffer.
*/
bool ConvertBufferToUTF16( char * buffer, size_t length, wchar_t ** obuffer, size_t & length2) {

    *obuffer = NULL;
    length2 = 0;

    if (IsUTF8((BYTE *)buffer,length))
    {
        length2 = MultiByteToWideChar(CP_UTF8,0,buffer,length,NULL,0);
		if (length2==0) return false;
        *obuffer = new wchar_t[length2];
        int result = MultiByteToWideChar(CP_UTF8,0,buffer,length,*obuffer,length2);
		if (result==0) {
			delete [] *obuffer;
			*obuffer = NULL;
			length2 = 0;
		}
		return (result!=0);
    }
    
	if (IsUTF16((BYTE *)buffer,length))
    {
        // just recast the pointer?
		length2 = length/2;
		*obuffer = new wchar_t[length2];
		memcpy( obuffer, buffer, length);
    }
    
	if (IsUTF32((BYTE *)buffer,length))
    {
        // not supported yet
        return false;
    }
    
	if (IsASCII((BYTE *)buffer,length))
    {
        // assume ascii
        length2 = MultiByteToWideChar(CP_ACP,0,buffer,length,NULL,0);
		if (length2==0) return false;
        *obuffer = new wchar_t[length2];
        int result = MultiByteToWideChar(CP_ACP,0,buffer,length,*obuffer,length2);
		if (result==0) {
			delete [] *obuffer;
			*obuffer = NULL;
			length2 = 0;
		}
		return (result!=0);
    }

	// assume ascii
    length2 = MultiByteToWideChar(CP_ACP,0,buffer,length,NULL,0);
	if (length2==0) return false;
    *obuffer = new wchar_t[length2];
    int result = MultiByteToWideChar(CP_ACP,0,buffer,length,*obuffer,length2);
	if (result==0) {
		delete [] *obuffer;
		*obuffer = NULL;
		length2 = 0;
	}
	return (result!=0);
}

vector<wstring> TokenizeBufferToLines( wchar_t * buffer, size_t start, size_t length) {

    // now tokenize the buffer
    vector<wstring> lines;

	wstring line;
    for (size_t i=start; i<length; i++)
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
	size_t length2 = 0;
	wchar_t * obuffer = NULL;
	{
		streampos length = 0;
		char * buffer = NULL;
		if (!ReadFileIntoBuffer( filename, &buffer, length)) return false;

		if (!ConvertBufferToUTF16( buffer, length, &obuffer, length2))
		{
			delete [] buffer;
			return false;
		}

		delete [] buffer;
	}

    size_t start = 0;
    if ((obuffer[0]==0xfeff)||(obuffer[0]==0xfffe))
    {
        start = 1;
    }

	vector<wstring> lines = TokenizeBufferToLines( obuffer, start, length2);

    delete [] obuffer;
	obuffer = NULL;
	length2 = 0;

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

