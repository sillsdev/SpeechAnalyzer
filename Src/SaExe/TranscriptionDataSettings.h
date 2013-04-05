#ifndef TRANSCRIPTIONDATASETTINGS_H
#define TRANSCRIPTIONDATASETTINGS_H

#include "SaString.h"
#include "TranscriptionHelper.h"
#include "TranscriptionData.h"

class CTranscriptionDataSettings
{
public:
    bool m_bPhoneticModified;
    bool m_bPhonemicModified;
    bool m_bOrthographicModified;
    bool m_bGlossModified;
    int m_nAlignBy;
    int m_nSegmentBy;
    bool m_bUseReference;
    bool m_bReference;
    bool m_bOrthographic;
    bool m_bPhonemic;
    bool m_bPhonetic;
    bool m_bGloss;
    CSaString m_szPhonetic;
    CSaString m_szPhonemic;
    CSaString m_szOrthographic;
    CSaString m_szGloss;
    CTranscriptionData m_TranscriptionData;
};

#endif
