/////////////////////////////////////////////////////////////////////////////
// param.h:
// Interface of the CParseParm, CSegmentParm, and CPitchParm classes.
// Author: Steve MacLean
// copyright 2000 JAARS Inc. SIL
//
// Revision History
// 1.5Test11.1A
//      RLJ Changed CPitchParm and CSpectrumParm from
//            structures in appdefs.h to classes here..
// 06/07/2000
//      RLJ Added CRecordingParm class
/////////////////////////////////////////////////////////////////////////////
#ifndef _PARAM_H
#define _PARAM_H

#include "SA_DSP.h"

class ObjectIStream;
class CProcessMelogram;

class CParseParm {              // parsing parameters
public:
    void WriteProperties(ObjectOStream & obs);
    BOOL ReadProperties(ObjectIStream & obs);
    void Init();
    float fBreakWidth;          // minimum width of break (ms) for words
    float fPhraseBreakWidth;    // minimum width of break (ms) for phrases
    int nMaxThreshold;          // minimum threshold (%)
    int nMinThreshold;          // maximum threshold (%)
    int nParseMode;             // parsing mode
};

class CSegmentParm {            // segmenting parameters
public:
    float fSegmentWidth;        // minimum width of peak (ms)
    int nChThreshold;           // minimum Change threshold (%)
    int nZCThreshold;           // minimum Zero Crossing threshold (# of crossings)
    int nSegmentMode;           // segmenting mode
    BOOL bKeepSegments;         // TRUE if existing segments to keep

    void WriteProperties(ObjectOStream & obs);
    BOOL ReadProperties(ObjectIStream & obs);

    void Init();
};

class CSaDoc;
class CPitchParm {                  // pitch parameters
public:
    void WriteProperties(ObjectOStream & obs);
    BOOL ReadProperties(ObjectIStream & obs);
    void Init();
    static void GetAutoRange(CProcessGrappl* pGrappl, int & nUpperBound, int & nLowerBound);

    int nRangeMode;                 // frequency range display mode
    int nScaleMode;                 // scale display mode
    int nUpperBound;                // upper frequency display boundary
    int nLowerBound;                // lower frequency display boundary
    BOOL bUseCepMedianFilter;       // TRUE if the CepstralPitch graph..
    // ..should use the Median Filter.
    BYTE nCepMedianFilterSize;      // number of points to do the..
    // ..median filter over

    // RLJ 09/26/2000: Bug GPI-01
    int nManualPitchUpper;          // temporary location to save manual pitch upper boundary
    int nManualPitchLower;          // temporary location to save manual pitch lower boundary
};

class CUttParm {
public:
    void Init(int nBitsPerSample);
    int TruncatedCritLoud(int nBitsPerSample) const;

    unsigned int nMinFreq;      // min freq for calculation
    unsigned int nMaxFreq;      // max freq for calculation
    int nCritLoud;              // threshold for voicing
    int nMaxChange;             // max freq % change for continuity
    int nMinGroup;              // minimum significant cluster size
    int nMaxInterp;             // maximum gap for interpolation
};                        // RIFF file header utterance parameters

class CMusicParm {              // pitch parameters
public:
    int nRangeMode;             // frequency range display mode
    int nUpperBound;            // upper frequency display boundary
    int nLowerBound;            // lower frequency display boundary

    int nManualUpper;           // temporary location to save manual upper boundary
    int nManualLower;           // temporary location to save manual lower boundary

    int nCalcRangeMode;         // frequency range display mode
    int nCalcUpperBound;        // upper frequency display boundary
    int nCalcLowerBound;        // lower frequency display boundary

    int nManualCalcUpper;       // temporary location to save manual upper boundary
    int nManualCalcLower;       // temporary location to save manual lower boundary

    void WriteProperties(ObjectOStream & obs);
    BOOL ReadProperties(ObjectIStream & obs);

    void Init();
    static void GetAutoRange(CProcessMelogram* pMelogram, int & nUpperBound, int & nLowerBound);
};

class CIntensityParm {          // pitch parameters
public:
    CIntensityParm();
    void WriteProperties(ObjectOStream & obs);
    BOOL ReadProperties(ObjectIStream & obs);
    void Init();

    int nScaleMode;             // 0 - dB, 1 - Percent
};

// ARH 8/1/01 - Added for wavelet scalogram graph
class CWaveletParm {                 // Wavelet parameters
public:
    void WriteProperties(ObjectOStream & obs);
    BOOL ReadProperties(ObjectIStream & obs);
    void Init();

    int nResolution;       // resolution of display
    int nColor;            // color of display
    int nFrequency;        // frequency range to calculate
    int nMinThreshold;     // minimum threshold
    int nMaxThreshold;     // maximum threshold
    BOOL bSmoothSpectra;    // TRUE = smooth spectra
};


class CSpectrumParm { // spectrum parameters
public:
    int nScaleMode;             // scale display mode
    int nPwrUpperBound;         // upper power display boundary
    int nPwrLowerBound;         // lower power display boundary
    int nFreqUpperBound;        // upper frequency display boundary
    int nFreqLowerBound;        // lower frequency display boundary
    int nFreqScaleRange;        // frequency range: 0 = full scale, 1 = half scale, 2 = third scale, 3 = quarter scale
    int nSmoothLevel;           // level to control spectral smoothing
    int nPeakSharpFac;          // factor to control sharpening of formant peaks
    CWindowSettings cWindow;    // DSP Window settings
    BOOL bShowLpcSpectrum;      // TRUE = show LPC-smoothed spectrum
    BOOL bShowCepSpectrum;      // TRUE = show cepstrally-smoothed spectrum
    BOOL bShowFormantFreq;      // TRUE = show formant frequencies
    BOOL bShowFormantBandwidth;  // TRUE = show formant bandwidths
    BOOL bShowFormantPower;     // TRUE = show formant powers

    void WriteProperties(ObjectOStream & obs);
    BOOL ReadProperties(ObjectIStream & obs);

    void Init();
};

class CFormantParm { // formant chart parameters
public:
    CFormantParm() {
        Init();
    }
    BOOL bFromLpcSpectrum;       // TRUE = formants extracted from LPC spectrum
    BOOL bFromCepstralSpectrum;  // TRUE = formants extracted from cepstrally-smoothed spectrum
    BOOL bTrackFormants;         // TRUE = track formants during animation
    BOOL bSmoothFormants;        // TRUE = smooth formants after animation
    BOOL bMelScale;              // TRUE = use mel-scale for grids

    void WriteProperties(ObjectOStream & obs);
    BOOL ReadProperties(ObjectIStream & obs);

    void Init();
};

class CRecordingParm { // Recording parameters
public:
    int nRate;          // Sampling Rate code (0=11KHz, 1=22KHz, 2=44KHz)
    int nBits;          // Sample data width in bits
    BOOL bHighpass;     // User 70KHz highpass filter (0=FALSE, 1=TRUE)
    int nMode;          // Recording mode (0=mono, 1=stereo)

    void WriteProperties(ObjectOStream & obs);
    BOOL ReadProperties(ObjectIStream & obs);

    void Init();
};


// Graph Parameter.cpp for implementation
class CResearchSettings {
public:
    CResearchSettings() {
        Init();
    }

    void Init();

    BOOL m_bSpectrogramConnectFormants;
    BOOL m_bSpectrogramContrastEnhance;
    BOOL m_bShowHilbertTransform;
    BOOL m_bShowInstantaneousPower;
    int m_nSpectrumLpcMethod;
    int m_nSpectrumLpcOrderFsMult;
    int m_nSpectrumLpcOrderExtra;
    int m_nSpectrumLpcOrderAuxMax;
    int m_nLpcCepstralSmooth;
    int m_nLpcCepstralSharp;
    CWindowSettings m_cWindow;
};

// Graph Parameter.cpp for implementation
class CFormantTrackerOptions {
public:
    CFormantTrackerOptions() {
        Init();
    }

    void Init();
    BOOL operator!=(const CFormantTrackerOptions &) const;

    int m_nWindowType;
    double m_dUpdateRate;
    double m_dWindowBandwidth;
    double m_dDtfMinBandwidth;
    double m_dAzfMinBandwidth;
    BOOL m_bAzfAddConjugateZeroes;
    BOOL m_bAzfMostRecent;
    BOOL m_bAzfDiscardLpcBandwidth;
    BOOL m_bLowPass;
    BOOL m_bShowOriginalFormantTracks;
};

#endif // _PARAM_H
