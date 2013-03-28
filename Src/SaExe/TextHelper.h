#ifndef TEXTHELPER_H
#define TEXTHELPER_H

#include "CSaString.h"
#include "TranscriptionHelper.h"
#include <vector>
#include <string>

using std::vector;
using std::string;
using std::streampos;
using std::wstring;

class CTextHelper
{
public:
    static bool ImportText(const CSaString & filename, const CSaString & sync, const MarkerList & markers, TranscriptionDataMap & map, bool addTag);
private:
    static vector<string> Tokenize(const string & str, const string & delimiters);
};

extern bool ReadFileIntoBuffer( LPCTSTR filename, char ** buffer, std::ios::pos_type & length);
extern bool ConvertBufferToUTF16( char * buffer, size_t length, wchar_t ** obuffer, size_t & len);
extern vector<wstring> TokenizeBufferToLines( wchar_t * buffer, size_t start, size_t length);
extern vector<wstring> TokenizeLineToTokens( wstring & line, wchar_t token);

#endif

