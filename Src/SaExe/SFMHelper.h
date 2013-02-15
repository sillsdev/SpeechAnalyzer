#ifndef SFMHELPER_H
#define SFMHELPER_H

#include "CSaString.h"
#include "Settings\obstream.h"
#include "TranscriptionHelper.h"
#include <list>
#include <map>

class CSFMHelper
{
public:
	static bool IsSFM( CSaString & filename);
	static bool IsMultiRecordSFM( CSaString & filename, CSaString & marker);
	static TranscriptionDataMap ImportMultiRecordSFM( CSaString & filename, CSaString & syncMarker, MarkerList & markers, bool addTag);
	static TranscriptionDataMap ImportSFM( CSaString & filename);

private:
	static void BalanceDataMap( TranscriptionDataMap & map, CSaString & marker);
};

#endif
