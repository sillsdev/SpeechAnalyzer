#ifndef TEXTHELPER_H
#define TEXTHELPER_H

#include "CSaString.h"
#include "TranscriptionHelper.h"
#include <vector>
#include <string>

using std::vector;
using std::string;

class CTextHelper {
public:
	static bool ImportText( const CSaString & filename, const CSaString & sync, const MarkerList & markers, TranscriptionDataMap & map);
private:
	static vector<string> Tokenize( const string & str, const string & delimiters);
};

#endif

