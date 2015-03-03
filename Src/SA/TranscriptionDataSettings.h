#ifndef TRANSCRIPTIONDATASETTINGS_H
#define TRANSCRIPTIONDATASETTINGS_H

#include "SaString.h"
#include "TranscriptionHelper.h"
#include "TranscriptionData.h"

class CTranscriptionDataSettings {
public:
    bool m_bPhoneticModified;
    bool m_bPhonemicModified;
    bool m_bOrthographicModified;
    bool m_bGlossModified;
    bool m_bGlossNatModified;
    bool m_bReferenceModified;

    int m_nAlignBy;
    int m_nSegmentBy;

    bool m_bUseReference;
    bool m_bReference;
    bool m_bOrthographic;
    bool m_bPhonemic;
    bool m_bPhonetic;
    bool m_bGloss;
    bool m_bGlossNat;

    CSaString m_szReference;
    CSaString m_szPhonetic;
    CSaString m_szPhonemic;
    CSaString m_szOrthographic;
    CSaString m_szGloss;
    CSaString m_szGlossNat;

    CTranscriptionData m_TranscriptionData;
};

#endif
