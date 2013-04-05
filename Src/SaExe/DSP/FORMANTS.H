#ifndef FORMANTS_H
#define FORMANTS_H

#include "Signal.h"

#define AUTO_SET            0.F
#define UNVOICED            0.F
#define MIN_PITCH           50
#define MAX_PITCH           500
#define MIN_PITCHPEAK_THD   0.015F
#define MAX_NUM_FORMANTS    30
#define LOG_MAG_NA          -1.F

#define NUM_EXTRA_PEAKS_ALLOWED  MAX_NUM_FORMANTS

typedef struct
{
    float F[MAX_NUM_FORMANTS+1];  //F[0] = pitch, F[1]... = formant frequencies
} FORMANT_FREQ;

typedef struct
{
    float FrequencyInHertz;
    float BandwidthInHertz;
    float PowerInDecibels;
} FORMANT_VALUES;

typedef struct
{
    float Low;
    float High;
} RANGE;


#include "dspTypes.h"
#include "Error.h"
#include "PeakPick.h"

class CFormantPicker: public CPeakPicker
{
public:
    static char * Copyright(void);
    static float Version(void);
    static dspError_t CreateObject(CFormantPicker ** ppFormant, uint16 nMaxNumFormants, short nSource);
    dspError_t PickFormants(FORMANT_VALUES ** ppFormantTable, uint16 * pFormantCount,
                            float * pSmoothSpectrum, uint16 nSmoothSpectrumLength,
                            double dSpectralResolution, float fPitch);
    static RANGE FormantRange(int32 nSource, uint32 nFormantIndex);
    static bool FormantInRange(int32 nSource, uint16 nFormantIndex, float fFormantFrequency);
    static bool BandwidthInRange(int32 /* nSource */, uint16 /* nFormantIndex */, float fFormantBandwidth);
    ~CFormantPicker();
private:
    CFormantPicker(FORMANT_VALUES * FormantTable, uint16 nMaxNumFormants, short nSource = UNKNOWN_HUMAN);
    FORMANT_VALUES * m_FormantTable;
    uint16 m_nMaxNumFormants;
    uint16 m_nFormantCount;
    short m_nSource;
};

#endif //FORMANTS_H
