#ifndef SPECTGRM_H
#define SPECTGRM_H

#include "Error.h"
#include "dspTypes.h"
#include "Signal.h"
#include "DSPWins.h"
#include "fft.h"
#include "MathX.h"
#include "Formants.h"

#define UNDEF      0

enum EBANDWIDTH {NARROWBAND=-2, WIDEBAND=-1};
#define NARROW_BW   45.F
#define MEDIUM_BW  172.F
#define WIDE_BW    300.F

enum ESPECTROGRAM_LOC {PENDING=0, END_OF_SPGM=1, END_FMNT_CALC=1};

#define DB_MIN              0
#define DB_MAX            233  //!!was 234
#define FMNT_TRACK        234
#define BLANK             235
#define NA                 -1

#define CHIRP_DECAY         0.96F

enum ESPECTROGRAM_STATE {SPGM_NOT_READY=0, SPGM_PENDING=1, SPGM_CALC=2, FORMANTS_CALC=3}; //dspError_t definitions

struct SSpectrogramSettings
{
    SSpectrogramSettings()
    {
        windowType = CWindowSettings::kGaussian;
    }
    float LwrFreq;				//lowest frequency to calculate in Hz
    float UprFreq;				//highest frequency to calculate in Hz
    float Bandwidth;			//spectral resolution in Hz
    uint16 FreqCnt;				//number of frequency samples from low to high
    uint16 FFTLength;			//desired length of FFT
    uint32 SpectCnt;			//number of spectra to calculate
    bool preEmphSw;				//pre-emphasis switch
    bool fmntTrackSw;			//formant tracking switch
    uint16 NumFmnt;				//number of formants to track
    uint32 SigBlkOffset;		//number of samples offset into speech data
    uint32 SigBlkLength;		//number of speech samples over which spectra will be calculated
    uint16 SpectBatchLength;	//number of spectra to be calculated at one time
    CWindowSettings::Type windowType;
};


class CSpectrogram
{
public:
    static char * Copyright(void);
    static float Version(void);
    static dspError_t CreateObject(CSpectrogram ** Spgm, SSpectrogramSettings SpgmSetting, SSigParms Signal);
    //dspError_t ChangeSettings(SSpectrogramSettings New);
    dspError_t GetMetrics(float * FreqScale,
                          float * SpectScale, float * SpectRes, float * SpectIntv,
                          uint8 * MaxPwr);
    dspError_t Generate(void);
    dspError_t ReadPower(short * Power, uint16 SpgmX, uint16 SpgmY);
    static dspError_t CalcPower(float * PowerInDb, float fFrequency, float fDspWinBandwidth, CWindowSettings::Type, SSigParms Signal, bool bPreEmphasis, float DbRef = 0.F);
    static dspError_t CalcPower(float * PowerInDb, float Frequency, int32 nBandwidthSelect, CWindowSettings::Type, SSigParms Signal, bool bPreEmphasis);
    uint8 * ReadPowerSlice(dspError_t * dspError_t, uint16 SpgmX);
    dspError_t ReadFormants(SFormantFreq * Freq, uint16 SpgmX);
    CDspWin & GetWidebandWindow();
    CDspWin & GetNarrowbandWindow();
    dspError_t CalcFormants(void);
    ~CSpectrogram();
    bool IsAliased();

	uint8  * m_ScreenData;

private:
    static dspError_t ValidateSignalParms(SSigParms Signal);
    static dspError_t ValidateSettings(SSpectrogramSettings Setting);
    CSpectrogram(SSpectrogramSettings SpgmSetting, uint8 * SpgmData, SFormantFreq * FmntData,
                CDspWin & Window, CDspWin & NBWindow,
                SSigParms Signal, uint8 * ScreenData);
    dspError_t CalcSpectra(uint8 * BlockStart, uint32 NumSpect);
    dspError_t CalcSpectra(short * BlockStart, uint32 NumSpect);
    dspError_t PreProc(short * Frame, CDspWin & Window);
    dspError_t PreProc(uint8 * Frame, CDspWin & Window);
    dspError_t PwrFFT(uint8 * PwrSpect);
    void BlankSpectrum(uint8 * Spect);  //!!Validate Spect?
    void BlankFmntSet(SFormantFreq * Fmnt);
    dspError_t SeekFormants(uint8 * BlockStart, uint32 NumFmntSets);
    dspError_t SeekFormants(short * BlockStart, uint32 NumFmntSets);
    dspError_t GetFormants(SFormantFreq * F);

    bool m_Aliased;
    SFormantFreq * m_FmntData;
    bool m_PreEmphSw;
    bool m_FmntTrackSw;
    uint16 m_NumFmnt;
    void * m_SigStart;
    uint32 m_SigLength;
    int8 m_SmpDataFmt;
    uint32 m_SmpRate;
    uint32 m_SigBlkOffset;
    uint32 m_SigBlkLength;
    uint8 * m_SpgmData;
    uint16 m_SpgmHgt;
    float m_SpectIntv;
    uint32 m_SpectCnt;
    uint16 m_SpectLen;
    float m_SpectRes;
    float m_SpectScale;
    uint16 m_FFTLen;
    float m_FreqScale;
    float m_LwrFreq;
    float m_UprFreq;
    uint32 m_SpectToProc;
    uint32 m_FmntSetToProc;
    uint16 m_SpectBatchLength;
    CDspWin m_Window;
    CDspWin m_NBWindow;
    float * m_WinFrame;
    uint16 m_MinPitchPeriod;
    uint16 m_MaxPitchPeriod;
    uint8 m_MaxPwr;
    uint8 m_Status;
};

#endif


