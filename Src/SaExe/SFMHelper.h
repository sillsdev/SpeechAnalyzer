#ifndef SFMHELPER_H
#define SFMHELPER_H

#include "CSaString.h"
#include "Settings\obstream.h"
#include "TranscriptionHelper.h"
#include <list>
#include <map>
#include <string>
#include <vector>

using std::wstring;
using std::vector;

class CSFMHelper
{
public:
    static bool IsSFM(CSaString & filename);
    static bool IsMultiRecordSFM(CSaString & filename, CSaString & marker);
	static bool IsColumnarSFM( LPCTSTR filename);
    static TranscriptionDataMap ImportMultiRecordSFM(CSaString & filename, CSaString & syncMarker, MarkerList & markers, bool addTag);
	static TranscriptionDataMap ImportColumnarSFM( LPCTSTR filename);
    static TranscriptionDataMap ImportSFM(CSaString & filename);
	static bool IsTag( LPCTSTR text);
	static vector<wstring> FilterBlankLines( vector<wstring> & input);
	static bool IsPhonemic( LPCTSTR text, size_t length);
	static bool IsPhonetic( LPCTSTR text, size_t length);
	static bool IsOrthographic( LPCTSTR text, size_t length);
	static bool IsGloss( LPCTSTR text, size_t length);

	static const CSaString ExtractTabField(const CSaString & szLine, const int nField);

private:
    static void BalanceDataMap(TranscriptionDataMap & map, CSaString & marker);
};

#endif
