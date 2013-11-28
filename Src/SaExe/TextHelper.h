#ifndef TEXTHELPER_H
#define TEXTHELPER_H

#include "SaString.h"
#include "TranscriptionHelper.h"
#include <vector>

using std::vector;
using std::streampos;
using std::stringstream;
using std::wstringstream;

class CTextHelper
{
public:
    static bool ImportText(const CSaString & filename, const CSaString & sync, const MarkerList & markers, TranscriptionDataMap & map, bool addTag);
private:
    static vector<string> Tokenize(const string & str, const string & delimiters);
};

extern bool CheckEncoding( LPCTSTR filename, bool display);
extern bool ConvertFileToUTF16( LPCTSTR filename, wstring & obuffer);
extern vector<wstring> TokenizeBufferToLines( wstring buffer);
extern vector<wstring> TokenizeLineToTokens( wstring & line, wchar_t token);

#endif

