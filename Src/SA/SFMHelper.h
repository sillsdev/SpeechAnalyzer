#ifndef SFMHELPER_H
#define SFMHELPER_H

#include "SaString.h"
#include "objectostream.h"
#include "TranscriptionHelper.h"
#include <list>
#include <map>
#include <streambuf>

using std::stringstream;
using std::wistringstream;

class CSFMHelper {
public:
    static wstring ToLower(wstring in);
    static bool IsSFM(wistringstream & strm);
    static bool IsMultiRecordSFM(wistringstream & stream, CSaString & marker);
    static bool IsColumnarSFM(wistringstream & stream);
    static TranscriptionDataMap ImportMultiRecordSFM(wistringstream & stream, CSaString & syncMarker, MarkerList & markers, bool addTag);
    static TranscriptionDataMap ImportColumnarSFM(wistringstream & stream);
    static TranscriptionDataMap ImportSFM(CSaString & filename);
    static bool IsTag(LPCTSTR text);
    static vector<wstring> FilterBlankLines(vector<wstring> & input);
    static bool IsPhonemic(LPCTSTR text, size_t length);
    static bool IsPhonetic(LPCTSTR text, size_t length);
    static bool IsOrthographic(LPCTSTR text, size_t length);
    static bool IsGloss(LPCTSTR text, size_t length);
    static bool IsGlossNat(LPCTSTR text, size_t length);
    static bool IsRef(LPCTSTR text, size_t length);

    static const CSaString ExtractTabField(const CSaString & szLine, const int nField);

private:
    static void BalanceDataMap(TranscriptionDataMap & map, CSaString & marker);
};

#endif
