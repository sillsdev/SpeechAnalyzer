#ifndef TRANSCRIPTIONHELPER_H
#define TRANSCRIPTIONHELPER_H

#include "CSaString.h"

typedef std::list<CSaString> MarkerList;
typedef std::map<int,CSaString> MarkerDefList;
typedef std::map<CSaString,MarkerList> TranscriptionDataMap;

class CTranscriptionHelper {
public:
	static CSaString Render( class CTranscriptionData & td);
};

#endif