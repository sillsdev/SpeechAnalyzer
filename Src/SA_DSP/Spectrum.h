#ifndef SPECTRUM_H

#define SPECTRUM_H

#include "dspTypes.h"
#include "Error.h"
#include "Signal.h"
#include "Scale.h"
#include "Formants.h"
#include "fft.h"



#include "DSPWins.h"

#define MAX_FRAME_DUR    2.0F       //maximum frame duration (in sec) allowed


struct SSpectrumSettings {
    short nScaleSelect;             //scale, either LINEAR or DB
    uint16 wLength;                 //number of spectral values to calculate
    float fSmoothFreq;              //max frequency allowed for smoothing
    float fFFTRadius;               //radius for evaluating FFT in z-plane
    int32 nWindowType;              //DSP window switch:  ON = apply window
};

struct SSpectrumParms {
    uint16 wSpectLength;
    float fFreqScale;
    float fdBRef;
    float * pfRawSpectrum;
    float * pfSmoothSpectrum;
    uint16 wNumFormants;
    SFormantValues * pstFormant;
};

class CSpectrum {
public:
    static char * Copyright(void);
    static float Version(void);
    static dspError_t CreateObject(CSpectrum ** ppoSpectrum,
                                   SSpectrumSettings & stSpectSetting, SSigParms & stSignalParm,
                                   uint16 wFFTLength = MAX_FFT_LENGTH);  //must be a power of 2
    SSpectrumParms GetSpectParms(void);


    //dspError_t GetMetrics(uint16 *pwFreqCnt, float *pfFreqScale, uint16 *pwWinLen);
    //dspError_t GetRange (float *pMin, float *pMax);
    ~CSpectrum();

private:
    static dspError_t ValidateSignalParms(SSigParms & stSignalParms);
    static dspError_t ValidateSettings(SSpectrumSettings & stSpectSetting);
    CSpectrum(SSpectrumSettings & stSpectSetting,
              SSigParms & stSignalParm, CDspWin & poDSPWindow,
              uint16 wFFTLength, float * pfFFTBuffer,
              float * pfRawSpectrum, float * pfSmoothSpectrum);
    dspError_t Preprocess(uint8 * pubFrame);
    dspError_t Preprocess(int8 * psbFrame);
    dspError_t Preprocess(short * psFrame);
    dspError_t CalcPwrFFT(void);
    dspError_t GetPwrSpectrum(void);
    void      *     m_pFrameStart;
    uint32           m_dwFrameLength;
    uint16          m_wSmpRate;
    int8           m_sbSmpFormat;
    uint16          m_wSpectLength;
    short           m_nScaleSelect;
    float           m_fSmoothFreq;
    float           m_fSpectCalcRadius;
    CDspWin          m_oDSPWindow;
    uint16          m_wFFTLength;
    float     *     m_pfFFTBuffer;
    float     *     m_pfRawSpectrum;
    float     *     m_pfSmoothSpectrum;
    uint16          m_wNumFormants;
    SFormantValues  m_pstFormant[MAX_NUM_FORMANTS+1];   //pitch and formants 1, 2, etc.
    uint16          m_wMinPitchPeriod;
    uint16          m_wMaxPitchPeriod;
    double             m_dMaxRawPower;
    float           m_fPwrDbRef[3];
    bool m_bReplicate;
};

#endif
