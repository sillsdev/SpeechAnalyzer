#ifndef WAVEFORMGENERATORSETTINGS_H
#define WAVEFORMGENERATORSETTINGS_H

#include "objectistream.h"
#include "objectostream.h"

class CWaveformGeneratorSettings {

public:
    CWaveformGeneratorSettings();
    virtual void WriteProperties(CObjectOStream & obs);
    virtual BOOL ReadProperties(CObjectIStream & obs);
    BOOL Synthesize(LPCTSTR szFileName);
    void SynthesizeSamples(BPTR pTargetData, DWORD dwDataPos, DWORD dwBufferSize);

    // enables for seven signal generators in order
    BOOL m_bSinusoid1;
    BOOL m_bSinusoid2;
    BOOL m_bSinusoid3;
    BOOL m_bComb;
    BOOL m_bSquareWave;
    BOOL m_bTriangle;
    BOOL m_bSawtooth;
    double m_dAmplitude[7];
    double m_dFrequency[7];
    double m_dPhase[7];
    // special cases for discontinutities
    int m_nHandleDiscontinuities;
    int m_nHarmonics;
    PCMWAVEFORMAT pcm;
    double m_fFileLength;
};

#endif
