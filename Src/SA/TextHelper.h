#ifndef TEXTHELPER_H
#define TEXTHELPER_H

#include "SaString.h"
#include "TranscriptionHelper.h"

using std::vector;
using std::streampos;
using std::stringstream;
using std::wstringstream;
using std::wistringstream;

class CTextHelper {
public:
    static bool ImportText(wistringstream & stream, const CSaString & sync, const MarkerList & markers, TranscriptionDataMap & map, bool addTag);
private:
    static vector<string> Tokenize(const string & str, const string & delimiters);
};

extern vector<wstring> TokenizeBufferToLines(wistringstream & stream);
extern vector<wstring> TokenizeLineToTokens(wstring & line, wchar_t token);

#endif

