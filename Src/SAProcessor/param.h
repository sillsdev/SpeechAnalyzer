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

class CObjectOStream;
class CObjectIStream;
class CProcessMelogram;

class CParseParm {              // parsing parameters
public:
    void WriteProperties(CObjectOStream & obs);
    BOOL ReadProperties(CObjectIStream & obs);
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

    void WriteProperties(CObjectOStream & obs);
    BOOL ReadProperties(CObjectIStream & obs);

    void Init();
};

class CSaDoc;

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

    void WriteProperties(CObjectOStream & obs);
    BOOL ReadProperties(CObjectIStream & obs);

    void Init();
    static void GetAutoRange(Model* pModel, CProcessMelogram* pMelogram, int & nUpperBound, int & nLowerBound);
};

class CIntensityParm {          // pitch parameters
public:
    CIntensityParm();
    void WriteProperties(CObjectOStream & obs);
    BOOL ReadProperties(CObjectIStream & obs);
    void Init();

    int nScaleMode;             // 0 - dB, 1 - Percent
};

// ARH 8/1/01 - Added for wavelet scalogram graph
class CWaveletParm {                 // Wavelet parameters
public:
    void WriteProperties(CObjectOStream & obs);
    BOOL ReadProperties(CObjectIStream & obs);
    void Init();

    int nResolution;       // resolution of display
    int nColor;            // color of display
    int nFrequency;        // frequency range to calculate
    int nMinThreshold;     // minimum threshold
    int nMaxThreshold;     // maximum threshold
    BOOL bSmoothSpectra;    // TRUE = smooth spectra
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

    void WriteProperties(CObjectOStream & obs);
    BOOL ReadProperties(CObjectIStream & obs);

    void Init();
};

class CRecordingParm { // Recording parameters
public:
    int nRate;          // Sampling Rate code (0=11KHz, 1=22KHz, 2=44KHz)
    int nBits;          // Sample data width in bits
    BOOL bHighpass;     // User 70KHz highpass filter (0=FALSE, 1=TRUE)
    int nMode;          // Recording mode (0=mono, 1=stereo)

    void WriteProperties(CObjectOStream & obs);
    BOOL ReadProperties(CObjectIStream & obs);

    void Init();
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
