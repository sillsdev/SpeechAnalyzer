#include "stdafx.h"
#include "SFMHelper.h"
#include <iterator>
#include "IUtf8String.h"

static const char * IMPORT_END = "import";
static const wchar_t * EMPTY = L"";

using std::list;
using std::map;

bool CSFMHelper::IsSFM(  CSaString & filename) {

	Object_istream stream(filename);
	if ( stream.getIos().fail()) {
		return false;
	}
	if (!stream.bAtBackslash()) {
		stream.SkipBOM();
	}
	if (!stream.bAtBackslash()) {
		return false;
	}
	return true;
}

/**
* we will just count markers - if there's multiple of one type
* then it's a multirecord file
*
*/
bool CSFMHelper::IsMultiRecordSFM( CSaString & filename, CSaString & marker) {
	
	int count = 0;
	CSaString buffer;

	Object_istream stream(filename);
	if ( stream.getIos().fail()) {
		return false;
	}
	if (!stream.bAtBackslash()) {
		stream.SkipBOM();
	}
	if (!stream.bAtBackslash()) {
		return false;
	}

	while (!stream.bAtEnd()) {
		if (stream.bReadString( marker, &buffer)) {
			count++;
		} else {
			stream.bEnd(IMPORT_END);
		}
	}
	return (count>1);
}

/**
* Read in a SFM data stream.
* Parse out all types defined in the markers list.
* Whenever syncMarker is encountered, the list will be balanced for consistency
* Return a map using key marker as a key and the list of value for that marker
*/
TranscriptionDataMap CSFMHelper::ImportMultiRecordSFM(  CSaString & filename, CSaString & syncMarker, MarkerList & markers, bool /*addTag*/) {

	TranscriptionDataMap result;

	Object_istream stream(filename);
	if ( stream.getIos().fail()) {
		return result;
	}
	if (!stream.bAtBackslash()) {
		stream.SkipBOM();
	}
	if (!stream.bAtBackslash()) {
		return result;
	}

	/**
	* we will build up lists of each of the imported types,
	* then we will build the output after the data
	* has been completely read
	*/
	while (!stream.bAtEnd()) {
		MarkerList::const_iterator it = markers.begin();
		bool found = false;
		for (MarkerList::iterator it = markers.begin();it != markers.end();it++) {
			CSaString buffer;
			CSaString marker = *it;
			if (stream.bReadString( marker, &buffer)) {
				result[marker].push_back(buffer);
				// when see the sync marker, balance the other entries.
				if (marker.Compare(syncMarker)==0) {
					BalanceDataMap( result, syncMarker);
				}
				found=true;
				break;
			}
		}
		if (!found) {
			//we are at string that doesn't match - skip over it
			stream.bEnd(IMPORT_END);
		}
	}

	BalanceDataMap( result, syncMarker);

	return result;
}

void CSFMHelper::BalanceDataMap( TranscriptionDataMap & map, CSaString & marker) {
	
	if (map[marker].size()>0) {
		TranscriptionDataMap::size_type length = map[marker].size()-1;
		for (TranscriptionDataMap::iterator pos=map.begin();pos!=map.end();pos++) {
			if (marker.Compare(pos->first)!=0) {
				while (pos->second.size()<length) {
					pos->second.push_back(EMPTY);
				}
			}
		}
	}
}

TranscriptionDataMap CSFMHelper::ImportSFM( CSaString & /*filename*/) {
	TranscriptionDataMap map;
	return map;
}
