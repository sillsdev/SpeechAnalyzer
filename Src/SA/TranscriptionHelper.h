#ifndef TRANSCRIPTIONHELPER_H
#define TRANSCRIPTIONHELPER_H

#include "SaString.h"

typedef std::list<CSaString> MarkerList;
typedef std::map<EAnnotation,CSaString> MarkerDefList;
typedef std::map<CSaString,MarkerList> TranscriptionDataMap;

class CTranscriptionHelper {
public:
    static CSaString Render(class CTranscriptionData & td);
};

#endif
