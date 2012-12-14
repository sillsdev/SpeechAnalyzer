#include "Stdafx.h"
#include "TranscriptionData.h"

CTranscriptionData::CTranscriptionData()
{
	m_bPhonetic = false;
	m_bPhonemic = false;
	m_bOrthographic = false;
	m_bGloss = false;
}

void CTranscriptionData::ResetContent()
{
	m_szPrimary = L"";
	m_MarkerDefs.clear();
	m_Markers.clear();
	m_TranscriptionData.clear();
	m_bPhonetic = false;
	m_bPhonemic = false;
	m_bOrthographic = false;
	m_bGloss = false;
}
