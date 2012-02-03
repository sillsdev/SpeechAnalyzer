/////////////////////////////////////////////////////////////////////////////
// param.h:
// Interface of the ParseParm, SegmentParm, and PitchParm classes.
// Author: Steve MacLean
// copyright 2000 JAARS Inc. SIL
//
// Revision History
// 1.5Test11.1A
//      RLJ Changed PitchParm, SpectroParm, and SpectrumParm from
//            structures in appdefs.h to classes here..
// 06/07/2000
//      RLJ Added RecordingParm class
/////////////////////////////////////////////////////////////////////////////
#ifndef _PARAM_H

#include "dsp\dspWins.h"
#define _PARAM_H

class Object_ostream;
class Object_istream;

class ParseParm                   // parsing parameters
{
public:
	float        fBreakWidth;       // minimum width of break (ms)
	int          nMaxThreshold;     // minimum threshold (%)
	int          nMinThreshold;     // maximum threshold (%)
	int          nParseMode;        // parsing mode
	BOOL         bKeepParse;        // TRUE if existing parse to keep

	void WriteProperties(Object_ostream& obs);
	BOOL ReadProperties(Object_istream& obs);

	void Init();
};

class SegmentParm                 // segmenting parameters
{
public:
	float        fSegmentWidth;     // minimum width of peak (ms)
	int          nChThreshold;      // minimum Change threshold (%)
	int          nZCThreshold;      // minimum Zero Crossing threshold (# of crossings)
	int          nSegmentMode;      // segmenting mode
	BOOL         bKeepSegments;     // TRUE if existing segments to keep

	void WriteProperties(Object_ostream& obs);
	BOOL ReadProperties(Object_istream& obs);

	void Init();
};


class CSaDoc;
class PitchParm                        // pitch parameters
{
public:
	int          nRangeMode;           // frequency range display mode
	int          nScaleMode;           // scale display mode
	int          nUpperBound;          // upper frequency display boundary
	int          nLowerBound;          // lower frequency display boundary
	BOOL         bUseCepMedianFilter;  // TRUE if the CepstralPitch graph..
	// ..should use the Median Filter.
	BYTE         nCepMedianFilterSize; // number of points to do the..
	// ..median filter over

	// RLJ 09/26/2000: Bug GPI-01
	int          nManualPitchUpper; // temporary location to save manual pitch upper boundary
	int          nManualPitchLower; // temporary location to save manual pitch lower boundary

	void WriteProperties(Object_ostream& obs);
	BOOL ReadProperties(Object_istream& obs);

	void Init();
	static void GetAutoRange(CSaDoc *pDoc, int &nUpperBound, int &nLowerBound);
};

class UttParm
{
public:
	unsigned int nMinFreq;          // min freq for calculation
	unsigned int nMaxFreq;          // max freq for calculation
	int          nCritLoud;         // threshold for voicing
	int          nMaxChange;        // max freq % change for continuity
	int          nMinGroup;         // minimum significant cluster size
	int          nMaxInterp;        // maximum gap for interpolation

	void Init(int nBitsPerSample);
	int TruncatedCritLoud(int nBitsPerSample) const;
};                        // RIFF file header utterance parameters

class MusicParm                        // pitch parameters
{
public:
	int          nRangeMode;           // frequency range display mode
	int          nUpperBound;          // upper frequency display boundary
	int          nLowerBound;          // lower frequency display boundary

	int          nManualUpper;         // temporary location to save manual upper boundary
	int          nManualLower;         // temporary location to save manual lower boundary

	int          nCalcRangeMode;           // frequency range display mode
	int          nCalcUpperBound;          // upper frequency display boundary
	int          nCalcLowerBound;          // lower frequency display boundary

	int          nManualCalcUpper;         // temporary location to save manual upper boundary
	int          nManualCalcLower;         // temporary location to save manual lower boundary

	void WriteProperties(Object_ostream& obs);
	BOOL ReadProperties(Object_istream& obs);

	void Init();
	static void GetAutoRange(CSaDoc *pDoc, int &nUpperBound, int &nLowerBound);
};

class IntensityParm                        // pitch parameters
{
public:
	IntensityParm() { Init();}

	int          nScaleMode;             // 0 - dB, 1 - Percent

	void WriteProperties(Object_ostream& obs);
	BOOL ReadProperties(Object_istream& obs);

	void Init();
};

class SpectroParm                   // spectrogram parameters
{
public:
	int  nResolution;       // resolution of display
	int  nColor;            // color of display
	int  nOverlay;          // formants overlay mode
	int  nFrequency;        // frequency range to calculate
	int  nMinThreshold;     // minimum threshold
	int  nMaxThreshold;     // maximum threshold
	int  nNumberFormants;   // number of calculated/displayed formants
	BOOL bShowF1;           // show/hide formants (TRUE = show)
	BOOL bShowF2;
	BOOL bShowF3;
	BOOL bShowF4;
	BOOL bShowF5andUp;
	BOOL bSmoothFormantTracks;  // TRUE = smooth formant tracks before displaying them
	BOOL bSmoothSpectra;    // TRUE = smooth spectra
	BOOL bFormantColor;
	BOOL bShowPitch;

	void WritePropertiesA(Object_ostream& obs);
	void WritePropertiesB(Object_ostream& obs);
	BOOL ReadPropertiesA(Object_istream& obs);
	BOOL ReadPropertiesB(Object_istream& obs);

	void Init();
	static const float DspWinBandwidth[3];
	static float Bandwidth(int nDspMode) { return nDspMode > 2 ? 0 : DspWinBandwidth[nDspMode];}
	float Bandwidth() const { return Bandwidth(nResolution);}

private:
	void WriteProperties(const char* pszMarker, Object_ostream& obs);
	BOOL ReadProperties(const char* pszMarker, Object_istream& obs);
};


// ARH 8/1/01 - Added for wavelet scalogram graph
class WaveletParm                   // Wavelet parameters
{
public:
	int  nResolution;       // resolution of display
	int  nColor;            // color of display
	int  nFrequency;        // frequency range to calculate
	int  nMinThreshold;     // minimum threshold
	int  nMaxThreshold;     // maximum threshold
	BOOL bSmoothSpectra;    // TRUE = smooth spectra

	void WriteProperties(Object_ostream& obs);
	BOOL ReadProperties(Object_istream& obs);

	void Init();
};



class SpectrumParm  // spectrum parameters
{
public:
	int          nScaleMode;        // scale display mode
	int          nPwrUpperBound;    // upper power display boundary
	int          nPwrLowerBound;    // lower power display boundary
	int          nFreqUpperBound;   // upper frequency display boundary
	int          nFreqLowerBound;   // lower frequency display boundary
	int          nFreqScaleRange;   // frequency range: 0 = full scale, 1 = half scale, 2 = third scale, 3 = quarter scale
	int          nSmoothLevel;      // level to control spectral smoothing
	int          nPeakSharpFac;     // factor to control sharpening of formant peaks
	CWindowSettings cWindow;        // DSP Window settings
	BOOL         bShowLpcSpectrum;  // TRUE = show LPC-smoothed spectrum
	BOOL         bShowCepSpectrum;  // TRUE = show cepstrally-smoothed spectrum
	BOOL         bShowFormantFreq;  // TRUE = show formant frequencies
	BOOL         bShowFormantBandwidth;  // TRUE = show formant bandwidths
	BOOL         bShowFormantPower; // TRUE = show formant powers

	void WriteProperties(Object_ostream& obs);
	BOOL ReadProperties(Object_istream& obs);

	void Init();
};

class FormantParm  // formant chart parameters
{
public:
	FormantParm() { Init();}
	BOOL         bFromLpcSpectrum;       // TRUE = formants extracted from LPC spectrum
	BOOL         bFromCepstralSpectrum;  // TRUE = formants extracted from cepstrally-smoothed spectrum
	BOOL         bTrackFormants;         // TRUE = track formants during animation
	BOOL         bSmoothFormants;        // TRUE = smooth formants after animation
	BOOL         bMelScale;              // TRUE = use mel-scale for grids

	void WriteProperties(Object_ostream& obs);
	BOOL ReadProperties(Object_istream& obs);

	void Init();
};

class RecordingParm  // Recording parameters
{
public:
	int     nRate;      // Sampling Rate code (0=11KHz, 1=22KHz, 2=44KHz)
	int     nBits;      // Sample data width in bits
	BOOL    bHighpass;  // User 70KHz highpass filter (0=FALSE, 1=TRUE)
	int     nMode;      // Recording mode (0=mono, 1=stereo)

	void WriteProperties(Object_ostream& obs);
	BOOL ReadProperties(Object_istream& obs);

	void Init();
};


// Graph Parameter.cpp for implementation
class CResearchSettings
{
public:
	CResearchSettings() { Init();}

	void Init();

	BOOL	m_bSpectrogramConnectFormants;
	BOOL	m_bSpectrogramContrastEnhance;
	BOOL  m_bShowHilbertTransform;
	BOOL  m_bShowInstantaneousPower;
	int		m_nSpectrumLpcMethod;
	int		m_nSpectrumLpcOrderFsMult;
	int		m_nSpectrumLpcOrderExtra;
	int		m_nSpectrumLpcOrderAuxMax;
	int   m_nLpcCepstralSmooth;
	int   m_nLpcCepstralSharp;  
	CWindowSettings m_cWindow;
};

extern CResearchSettings ResearchSettings;


// Graph Parameter.cpp for implementation
class CFormantTrackerOptions
{
public:
	CFormantTrackerOptions() { Init();}

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

extern CFormantTrackerOptions FormantTrackerOptions;
#endif // _PARAM_H



