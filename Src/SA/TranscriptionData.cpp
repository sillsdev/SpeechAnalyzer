#include "Stdafx.h"
#include "TranscriptionData.h"

CTranscriptionData::CTranscriptionData() {
    m_bPhonetic = false;
    m_bPhonemic = false;
    m_bOrthographic = false;
    m_bGloss = false;
    m_bGlossNat = false;
}

void CTranscriptionData::ResetContent() {
    m_szPrimary = L"";
    m_MarkerDefs.clear();
    m_Markers.clear();
    m_TranscriptionData.clear();
    m_bPhonetic = false;
    m_bPhonemic = false;
    m_bOrthographic = false;
    m_bGloss = false;
    m_bGlossNat = false;
}

MarkerList & CTranscriptionData::GetMarkerList( EAnnotation type) {
	CSaString marker = m_MarkerDefs[type];
	return m_TranscriptionData[marker];
}

CString CTranscriptionData::GetValue( EAnnotation type, int index) {
	CSaString marker = m_MarkerDefs[type];
	if (marker.GetLength()==0) return L"";
	MarkerList & list = m_TranscriptionData[marker];
	MarkerList::iterator mit = list.begin();
	int i=0;
	while (mit!=list.end()) {
		if (i==index) {
			return *mit;
		}
		mit++;
		i++;
	}
	return L"";
}
