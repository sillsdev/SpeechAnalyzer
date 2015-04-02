#ifndef TRANSCRIPTION_DATA_H
#define TRANSCRIPTION_DATA_H

#include "AppDefs.h"
#include "TranscriptionHelper.h"

class CTranscriptionData {
public:
    CTranscriptionData();

	MarkerList & GetMarkerList( EAnnotation type);
    CString GetValue( EAnnotation type, int index);

	void ResetContent();

    // the primary tag
    CSaString m_szPrimary;
    // a link between a SFM marker and it's type
    MarkerDefList m_MarkerDefs;
    // the list of SFM markers for this map
    MarkerList m_Markers;
    // a map between the markers and the strings
    TranscriptionDataMap m_TranscriptionData;
    bool m_bPhonetic;
    bool m_bPhonemic;
    bool m_bOrthographic;
    bool m_bGloss;
    bool m_bGlossNat;
};

#endif
