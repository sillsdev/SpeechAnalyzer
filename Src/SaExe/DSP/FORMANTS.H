#ifndef FORMANTS_H
#define FORMANTS_H

#include "Signal.h"

using std::string;

#define AUTO_SET            0.F
#define UNVOICED            0.F
#define MIN_PITCH           50
#define MAX_PITCH           500
#define MIN_PITCHPEAK_THD   0.015F
// NOTE! This value can be NO LESS than 4!
// The vowel processing code hardcodes access into elements F[0] through F[4]
#define MAX_NUM_FORMANTS    30
#define LOG_MAG_NA          -1.F

#define NUM_EXTRA_PEAKS_ALLOWED  MAX_NUM_FORMANTS

struct SFormantFreq
{
    float F[MAX_NUM_FORMANTS+1];  //F[0] = pitch, F[1]... = formant frequencies

	string Dump()
	{
		string result;
		for (int i=0;i<_countof(F);i++)
		{
			char buffer[128];
			sprintf_s(buffer,_countof(buffer),"%f, ",F[i]);
			result.append(buffer);
		}
		return result;
	}
};

struct SFormantValues
{
    float FrequencyInHertz;
    float BandwidthInHertz;
    float PowerInDecibels;
};

struct SRange
{
    float Low;
    float High;
};

#include "dspTypes.h"
#include "Error.h"
#include "PeakPick.h"

class CFormantPicker: public CPeakPicker
{
public:
    static char * Copyright(void);
    static float Version(void);
    static dspError_t CreateObject(CFormantPicker ** ppFormant, uint16 nMaxNumFormants, short nSource);
    dspError_t PickFormants(SFormantValues ** ppFormantTable, uint16 * pFormantCount,
                            float * pSmoothSpectrum, uint16 nSmoothSpectrumLength,
                            double dSpectralResolution, float fPitch);
    static SRange FormantRange(int32 nSource, uint32 nFormantIndex);
    static bool FormantInRange(int32 nSource, uint16 nFormantIndex, float fFormantFrequency);
    static bool BandwidthInRange(int32 /* nSource */, uint16 /* nFormantIndex */, float fFormantBandwidth);
    ~CFormantPicker();
private:
    CFormantPicker(SFormantValues * FormantTable, uint16 nMaxNumFormants, short nSource = UNKNOWN_HUMAN);
    SFormantValues * m_FormantTable;
    uint16 m_nMaxNumFormants;
    uint16 m_nFormantCount;
    short m_nSource;
};

#endif //FORMANTS_H
