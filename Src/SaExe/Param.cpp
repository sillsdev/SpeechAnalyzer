/////////////////////////////////////////////////////////////////////////////
// param.cpp:
// Implementation of the ParseParm, SegmentParm, and PitchParm classes.
// Author: Steve MacLean
// copyright 2000 JAARS Inc. SIL
//
// Revision History
// 1.5Test11.1A
//      RLJ Added PitchParm, SpectrumParm, SpectroParm classes.
// 06/07/2000
//      RLJ Added RecordingParm class
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "settings\obstream.h"
#include "dsp\spectgrm.h"
#include "sa_doc.h"
#include "Process\sa_proc.h"
#include "Process\sa_p_gra.h"
#include "Process\sa_p_mel.h"
#include "Process\sa_p_cpi.h"
#include "Process\sa_p_pit.h"
#include "Process\sa_p_spi.h"

static const char* psz_parse   = "parse";
static const char* psz_breakWidth   = "break_ms";
static const char* psz_minThreshold   = "minThreshold";
static const char* psz_maxThreshold   = "maxThreshold";

void ParseParm::WriteProperties(Object_ostream& obs)
{
	obs.WriteBeginMarker(psz_parse);

	// write out properties
	obs.WriteInteger(psz_breakWidth, (int)(fBreakWidth * 1000.0 + 0.5));
	obs.WriteInteger(psz_maxThreshold, nMaxThreshold);
	obs.WriteInteger(psz_minThreshold, nMinThreshold);

	obs.WriteEndMarker(psz_parse);
}

BOOL ParseParm::ReadProperties(Object_istream& obs)
{
	if ( !obs.bAtBackslash() || !obs.bReadBeginMarker(psz_parse) )
	{
		return FALSE;
	}

	int nTemp;

	while ( !obs.bAtEnd() )
	{
		if ( obs.bReadInteger(psz_breakWidth, nTemp) )
			fBreakWidth = (float) nTemp/1000.0f;
		else if ( obs.bReadInteger(psz_maxThreshold, nMaxThreshold) );
		else if ( obs.bReadInteger(psz_minThreshold, nMinThreshold) );
		else if ( obs.bEnd(psz_parse) )
			break;
	}

	return TRUE;
}

void ParseParm::Init()
{
	fBreakWidth = (float) 0.025;  // CLW 1.07a
	nMaxThreshold = 10;
	nMinThreshold = 5;
	nParseMode = 2;
	bKeepParse = TRUE;
}

static const char* psz_segment   = "segment";
static const char* psz_segmentWidth   = "segment_ms";
static const char* psz_minChange   = "minChange";
static const char* psz_minZeroCrossing   = "minZeroCrossing";

void SegmentParm::WriteProperties(Object_ostream& obs)
{
	obs.WriteBeginMarker(psz_segment);

	// write out properties
	obs.WriteInteger(psz_segmentWidth, (int)(fSegmentWidth * 1000.0 + 0.5));
	obs.WriteInteger(psz_minChange, nChThreshold);
	obs.WriteInteger(psz_minZeroCrossing, nZCThreshold);

	obs.WriteEndMarker(psz_segment);
}

BOOL SegmentParm::ReadProperties(Object_istream& obs)
{
	if ( !obs.bAtBackslash() || !obs.bReadBeginMarker(psz_segment) )
	{
		return FALSE;
	}

	int nTemp;
	while ( !obs.bAtEnd() )
	{
		if ( obs.bReadInteger(psz_segmentWidth, nTemp) )
			fSegmentWidth = (float)nTemp/1000.0f;
		else if ( obs.bReadInteger(psz_minChange, nChThreshold) );
		else if ( obs.bReadInteger(psz_minZeroCrossing, nZCThreshold) );
		else if ( obs.bEnd(psz_segment) )
			break;
	}

	return TRUE;
}

void SegmentParm::Init()
{
	fSegmentWidth = (float) 0.020;
	nChThreshold = 17;
	nZCThreshold = 50;
	nSegmentMode = 0;
	bKeepSegments = TRUE;
}


static const char* psz_pitch             = "pitch";
static const char* psz_pitchRangeMode    = "RangeMode";
static const char* psz_pitchScaleMode    = "ScaleMode";
static const char* psz_pitchUpperBound   = "UpperBound";
static const char* psz_pitchLowerBound   = "LowerBound";
static const char* psz_pitchUseCepMedianFilter  = "UseCepMedianFilter";
static const char* psz_pitchCepMedianFilterSize = "CepMedianFilterSize";

// RLJ 09/26/2000: Bug GPI-01
static const char* psz_pitchManualUpper   = "ManualUpper";
static const char* psz_pitchManualLower   = "ManualLower";

void PitchParm::WriteProperties(Object_ostream& obs)
{
	obs.WriteBeginMarker(psz_pitch);

	// write out properties
	obs.WriteInteger(psz_pitchRangeMode,  nRangeMode);
	obs.WriteInteger(psz_pitchScaleMode,  nScaleMode);
	obs.WriteInteger(psz_pitchUpperBound, nUpperBound);
	obs.WriteInteger(psz_pitchLowerBound, nLowerBound);
	obs.WriteBool(psz_pitchUseCepMedianFilter, bUseCepMedianFilter);
	obs.WriteInteger(psz_pitchCepMedianFilterSize, ((int)nCepMedianFilterSize));

	// RLJ 09/26/2000: Bug GPI-01
	obs.WriteInteger(psz_pitchManualUpper, nManualPitchUpper);
	obs.WriteInteger(psz_pitchManualLower, nManualPitchLower);

	obs.WriteEndMarker(psz_pitch);
}

BOOL PitchParm::ReadProperties(Object_istream& obs)
{
	if ( !obs.bAtBackslash() || !obs.bReadBeginMarker(psz_pitch) )
	{
		return FALSE;
	}

	int nTemp;
	while ( !obs.bAtEnd() )
	{
		if ( obs.bReadInteger(psz_pitchRangeMode, nRangeMode) );
		else if ( obs.bReadInteger(psz_pitchScaleMode, nScaleMode) );
		else if ( obs.bReadInteger(psz_pitchUpperBound, nUpperBound) );
		else if ( obs.bReadInteger(psz_pitchLowerBound, nLowerBound) );
		else if ( obs.bReadBool(psz_pitchUseCepMedianFilter, bUseCepMedianFilter) );
		else if ( obs.bReadInteger(psz_pitchCepMedianFilterSize, nTemp) )
			nCepMedianFilterSize = (BYTE)nTemp;

		// RLJ 09/26/2000: Bug GPI-01
		else if ( obs.bReadInteger(psz_pitchManualUpper, nManualPitchUpper) );
		else if ( obs.bReadInteger(psz_pitchManualLower, nManualPitchLower) );

		else if ( obs.bEnd(psz_pitch) )
			break;
	}

	return TRUE;
}

void PitchParm::Init()
{
	nRangeMode = 0;
	// RLJ, 09/21/2000: Bug GPI-01
	//    nScaleMode = 1; // Default scaling = LINEAR
	nScaleMode = 0; // Default scaling = LOGARITHMIC
	nUpperBound = CECIL_PITCH_MAXFREQ;
	nLowerBound = CECIL_PITCH_MINFREQ;
	bUseCepMedianFilter = TRUE;
	nCepMedianFilterSize = 7;

	// RLJ 09/26/2000: Bug GPI-01
	nManualPitchUpper = nUpperBound;
	nManualPitchLower = nLowerBound;
}

void PitchParm::GetAutoRange(CSaDoc *pDoc, int &nUpperBound, int &nLowerBound)
{
	PitchParm temp;
	temp.Init();

	CDataProcess * pGrappl = (CProcessGrappl*)pDoc->GetGrappl(); // get pointer to grappl object
	// If auto pitch is not done look for alternates
	if(!pGrappl->IsDataReady())
		pGrappl = pDoc->GetSmoothedPitch();
	if(!pGrappl->IsDataReady())
		pGrappl = pDoc->GetPitch();
	if(!pGrappl->IsDataReady())
		pGrappl = pDoc->GetCustomPitch();
	if(pGrappl->IsDataReady() && !pGrappl->IsStatusFlag(PROCESS_NO_PITCH))
	{
		// auto range mode
		int nMinData = pGrappl->GetMinValue();
		int nMaxData = pGrappl->GetMaxValue();

		nMinData /= PRECISION_MULTIPLIER;
		nMaxData /= PRECISION_MULTIPLIER;

		// make sure the legend scale spans at least an octave
		double fSpan = (double)nMaxData / nMinData;
		if (fSpan < 2.0)
		{
			nMinData = (short)((double)(nMinData + nMaxData) / 3.0);
			nMaxData = (short)(2 * nMinData);
		}

		// Add a little space to the top/bottom
		nMaxData = int(nMaxData * 1.05);
		nMinData = int(nMinData / 1.05);
		if(nMinData < 0)
			nMinData = 0;

		temp.nUpperBound = nMaxData;
		temp.nLowerBound = nMinData;
	}

	nUpperBound = temp.nUpperBound;
	nLowerBound = temp.nLowerBound;
}

void UttParm::Init(int nBitsPerSample)
{
	nMinFreq = CECIL_PITCH_MINFREQ;
	nMaxFreq = CECIL_PITCH_MAXFREQ;
	nCritLoud = CECIL_PITCH_VOICING << (nBitsPerSample - 8);
	nMaxChange = CECIL_PITCH_CHANGE;
	nMinGroup = CECIL_PITCH_GROUP;
	nMaxInterp = CECIL_PITCH_INTERPOL;
}

int UttParm::TruncatedCritLoud(int nBitsPerSample) const
{
	if(nCritLoud<128) return nCritLoud;

	int nResult=nCritLoud;

	if (nBitsPerSample > 8)
	{
		if (nResult >= 256)
		{
			nResult >>= nBitsPerSample-8;
		}
	}
	// 99.9% of max counts is the largest legal value internally
	if (nResult > (int)(0.999 * 128 * 16))
	{
		nResult = (int)(0.999 * 128 * 16);
	}
	if (nResult < 0)
	{
		nResult = 0;
	}

	return nResult;
}


static const char* psz_music             = "music";
static const char* psz_calcRangeMode    = "CalcRangeMode";
static const char* psz_calcUpperBound   = "CalcUpperBound";
static const char* psz_calcLowerBound   = "CalcLowerBound";
static const char* psz_calcManualUpper   = "ManualCalcUpper";
static const char* psz_calcManualLower   = "ManualCalcLower";

void MusicParm::WriteProperties(Object_ostream& obs)
{
	obs.WriteBeginMarker(psz_music);

	// write out properties
	//  obs.WriteInteger(psz_pitchRangeMode,  nRangeMode);
	obs.WriteInteger(psz_pitchUpperBound, nUpperBound);
	obs.WriteInteger(psz_pitchLowerBound, nLowerBound);

	// RLJ 09/26/2000: Bug GPI-01
	obs.WriteInteger(psz_pitchManualUpper, nManualUpper);
	obs.WriteInteger(psz_pitchManualLower, nManualLower);

	//  obs.WriteInteger(psz_calcRangeMode,  nCalcRangeMode);
	obs.WriteInteger(psz_calcUpperBound, nCalcUpperBound);
	obs.WriteInteger(psz_calcLowerBound, nCalcLowerBound);
	obs.WriteInteger(psz_calcManualUpper, nManualCalcUpper);
	obs.WriteInteger(psz_calcManualLower, nManualCalcLower);

	obs.WriteEndMarker(psz_music);
}

BOOL MusicParm::ReadProperties(Object_istream& obs)
{
	if ( !obs.bAtBackslash() || !obs.bReadBeginMarker(psz_music) )
	{
		return FALSE;
	}

	while ( !obs.bAtEnd() )
	{
		//    if ( obs.bReadInteger(psz_pitchRangeMode, nRangeMode) );
		//    else 
		if ( obs.bReadInteger(psz_pitchUpperBound, nUpperBound) );
		else if ( obs.bReadInteger(psz_pitchLowerBound, nLowerBound) );

		else if ( obs.bReadInteger(psz_pitchManualUpper, nManualUpper) );
		else if ( obs.bReadInteger(psz_pitchManualLower, nManualLower) );
		//    if ( obs.bReadInteger(psz_calcRangeMode, nCalcRangeMode) );
		else if ( obs.bReadInteger(psz_calcUpperBound, nCalcUpperBound) );
		else if ( obs.bReadInteger(psz_calcLowerBound, nCalcLowerBound) );
		else if ( obs.bReadInteger(psz_calcManualUpper, nManualCalcUpper) );
		else if ( obs.bReadInteger(psz_calcManualLower, nManualCalcLower) );

		else if ( obs.bEnd(psz_music) )
			break;
	}

	return TRUE;
}

void MusicParm::Init()
{
	nRangeMode = 0;
	nUpperBound = 70;
	nLowerBound = 44;

	nManualUpper = nUpperBound;
	nManualLower = nLowerBound;

	nCalcRangeMode = 0;
	nCalcUpperBound = 84;
	nCalcLowerBound = 28;

	nManualCalcUpper = nCalcUpperBound;
	nManualCalcLower = nCalcLowerBound;
}

void MusicParm::GetAutoRange(CSaDoc *pDoc, int &nUpperBound, int &nLowerBound)
{
	MusicParm temp;
	temp.Init();

	CProcessMelogram* pMelogram = (CProcessMelogram*)pDoc->GetMelogram(); // get pointer to melogram object

	if(pMelogram->IsDataReady() && !pMelogram->IsStatusFlag(PROCESS_NO_PITCH))
	{
		int nLowerBound = (int)floor((double)pMelogram->GetMinValue() / 100.0);
		int nUpperBound = (int)ceil((double)pMelogram->GetMaxValue() / 100.0);

		// make sure the legend scale spans at least an octave
		int nRange = (nUpperBound - nLowerBound);
		if (nRange < 12)
		{
			nLowerBound = (int)(((double)(nLowerBound + nUpperBound) / 2.0) - 6.);
			nUpperBound = (int)(nLowerBound + 12);
		}
		temp.nUpperBound = nUpperBound;
		temp.nLowerBound = nLowerBound;
	}

	nUpperBound = temp.nUpperBound;
	nLowerBound = temp.nLowerBound;
}

static const char* psz_intensity = "intensity";
static const char* psz_intensityScaleMode = "ScaleMode";

void IntensityParm::WriteProperties(Object_ostream& obs)
{
	obs.WriteBeginMarker(psz_intensity);

	// write out properties
	obs.WriteInteger(psz_intensityScaleMode,  nScaleMode);

	obs.WriteEndMarker(psz_intensity);
}

BOOL IntensityParm::ReadProperties(Object_istream& obs)
{
	if ( !obs.bAtBackslash() || !obs.bReadBeginMarker(psz_intensity) )
	{
		return FALSE;
	}

	while ( !obs.bAtEnd() )
	{
		if ( obs.bReadInteger(psz_intensityScaleMode, nScaleMode) );
		else if ( obs.bEnd(psz_intensity) )
			break;
	}

	return TRUE;
}

void IntensityParm::Init()
{
	nScaleMode = 1;
}

static const char* psz_spectroA             = "spectrogramA";
static const char* psz_spectroB             = "spectrogramB";
static const char* psz_Resolution           = "nResolution3";
static const char* psz_ResolutionOld        = "nResolution";
static const char* psz_wavelet              = "Wavelet";				// ARH 8/1/01 Added for Wavelet scalogram graph
static const char* psz_Color                = "nColor";
static const char* psz_ShowF1               = "bShowF1";
static const char* psz_ShowF2               = "bShowF2";
static const char* psz_ShowF3               = "bShowF3";
static const char* psz_ShowF4               = "bShowF4";
static const char* psz_ShowF5andUp          = "bShowF5andUp";
static const char* psz_SmoothFormantTracks  = "bSmoothFormantTracks";
static const char* psz_Frequency            = "nFrequency";
static const char* psz_MaxThreshold         = "nMaxThreshold";
static const char* psz_MinThreshold         = "nMinThreshold";
static const char* psz_Overlay              = "nOverlay";
static const char* psz_ShowPitch            =  "bShowPitch";

void SpectroParm::WritePropertiesA(Object_ostream& obs)
{
	WriteProperties(psz_spectroA, obs);
}

void SpectroParm::WritePropertiesB(Object_ostream& obs)
{
	WriteProperties(psz_spectroB, obs);
}

// Write spectroParm properties to *.psa file.
void SpectroParm::WriteProperties(const char * pszMarker, Object_ostream& obs)
{
	obs.WriteBeginMarker(pszMarker);

	// write out properties
	obs.WriteInteger(psz_Resolution, nResolution);
	obs.WriteInteger(psz_Color, nColor);
	obs.WriteBool(psz_ShowF1, bShowF1);
	obs.WriteBool(psz_ShowF2, bShowF2);
	obs.WriteBool(psz_ShowF3, bShowF3);
	obs.WriteBool(psz_ShowF4, bShowF4);
	obs.WriteBool(psz_ShowF5andUp, bShowF5andUp);
	obs.WriteBool(psz_SmoothFormantTracks, bSmoothFormantTracks);
	obs.WriteInteger(psz_Frequency, nFrequency);
	obs.WriteInteger(psz_MaxThreshold, nMaxThreshold);
	obs.WriteInteger(psz_MinThreshold, nMinThreshold);
	obs.WriteInteger(psz_Overlay, nOverlay);
	obs.WriteBool(psz_ShowPitch, bShowPitch);

	obs.WriteEndMarker(pszMarker);
}

BOOL SpectroParm::ReadPropertiesA(Object_istream& obs)
{
	return ReadProperties(psz_spectroA, obs);
}

BOOL SpectroParm::ReadPropertiesB(Object_istream& obs)
{
	return ReadProperties(psz_spectroB, obs);
}

// Read spectroParm properties from *.psa file.
BOOL SpectroParm::ReadProperties(const char * pszMarker, Object_istream& obs)
{
	if ( !obs.bAtBackslash() || !obs.bReadBeginMarker(pszMarker) )
	{
		return FALSE;
	}

	while ( !obs.bAtEnd() )
	{
		if ( obs.bReadInteger(psz_Resolution, nResolution) );
		// This is temporary until *.psa files are built with this version
		else if ( obs.bReadInteger(psz_ResolutionOld, nResolution) )
			nResolution *= 2;  // map 0->0 & 1->2
		else if ( obs.bReadInteger(psz_Color, nColor) );
		else if ( obs.bReadBool(psz_ShowF1, bShowF1) );
		else if ( obs.bReadBool(psz_ShowF2, bShowF2) );
		else if ( obs.bReadBool(psz_ShowF3, bShowF3) );
		else if ( obs.bReadBool(psz_ShowF4, bShowF4) );
		else if ( obs.bReadBool(psz_ShowF5andUp, bShowF5andUp) );
		else if ( obs.bReadBool(psz_SmoothFormantTracks, bSmoothFormantTracks) );
		else if ( obs.bReadInteger(psz_Frequency, nFrequency) );
		else if ( obs.bReadInteger(psz_MaxThreshold, nMaxThreshold) );
		else if ( obs.bReadInteger(psz_MinThreshold, nMinThreshold) );
		else if ( obs.bReadInteger(psz_Overlay, nOverlay) );
		else if ( obs.bReadBool(psz_ShowPitch, bShowPitch) );
		else if ( obs.bEnd(pszMarker) )
			break;
	}
	return TRUE;
}

const float SpectroParm::DspWinBandwidth[] = {(float)NARROW_BW, (float)MEDIUM_BW, (float)WIDE_BW};

void SpectroParm::Init()
{
	nResolution = 2;
	nColor = 1; // Monochrome
	nOverlay = 0;
	bShowF1 = FALSE;
	bShowF2 = FALSE;
	bShowF3 = FALSE;
	bShowF4 = FALSE;
	bShowF5andUp = FALSE;
	bSmoothFormantTracks = TRUE;
	nFrequency = 3200;
	nMinThreshold = 61;
	nMaxThreshold = 209;
	nNumberFormants = 3;
	bSmoothSpectra = TRUE;
	bFormantColor = TRUE;
	bShowPitch = FALSE;
}



// ARH 8/1/01 Added for wavelet scalogram graph
void WaveletParm::WriteProperties(Object_ostream& obs)
// Write waveletParm(0) properties to *.psa file.
{
	obs.WriteBeginMarker(psz_wavelet);

	// write out properties
	obs.WriteInteger(psz_Resolution, nResolution);
	obs.WriteInteger(psz_Color, nColor);
	obs.WriteInteger(psz_Frequency, nFrequency);
	obs.WriteInteger(psz_MaxThreshold, nMaxThreshold);
	obs.WriteInteger(psz_MinThreshold, nMinThreshold);

	obs.WriteEndMarker(psz_wavelet);
}


// ARH 8/1/01 Added for wavelet scalogram graph
BOOL WaveletParm::ReadProperties(Object_istream& obs)
// Read waveletParm properties from *.psa file.
{
	if ( !obs.bAtBackslash() || !obs.bReadBeginMarker(psz_wavelet) )
	{
		return FALSE;
	}

	while ( !obs.bAtEnd() )
	{
		if ( obs.bReadInteger(psz_Resolution, nResolution) );
		else if ( obs.bReadInteger(psz_Color, nColor) );
		else if ( obs.bReadInteger(psz_Frequency, nFrequency) );
		else if ( obs.bReadInteger(psz_MaxThreshold, nMaxThreshold) );
		else if ( obs.bReadInteger(psz_MinThreshold, nMinThreshold) );
		else if ( obs.bEnd(psz_wavelet) )
			break;
	}
	return TRUE;
}


// ARH 8/1/01 Added for wavelet scalogram graph
void WaveletParm::Init()
{
	nResolution = 1;
	nColor = 0;
	nFrequency = 3000;
	nMinThreshold = 0;
	nMaxThreshold = 234;
	bSmoothSpectra = TRUE;
}




static const char* psz_spectrum       = "spectrum";
static const char* psz_ScaleMode      = "nScaleMode";
static const char* psz_PwrUpperBound  = "nPwrUpperBound";
static const char* psz_PwrLowerBound  = "nPwrLowerBound";
static const char* psz_FreqUpperBound = "nFreqUpperBound";
static const char* psz_FreqLowerBound = "nFreqLowerBound";
static const char* psz_FreqRange      = "nFreqRange";
static const char* psz_FreqScaleRange = "nFreqScaleRange";
static const char* psz_SmoothLevel    = "nSmoothLevel";
static const char* psz_PeakSharpFac   = "nPeakSharpFac";
static const char* psz_ShowLpcSpectrum = "bShowLpcSpectrum";
static const char* psz_ShowCepSpectrum = "bShowCepSpectrum";
static const char* psz_ShowFormantFreq = "bShowFormantFreq";
static const char* psz_ShowFormantBandwidth = "bShowFormantBandwidth";
static const char* psz_ShowFormantPower = "bShowFormantPower";

void SpectrumParm::WriteProperties(Object_ostream& obs)
// Write spectrumParm properties to *.psa file.
{
	obs.WriteBeginMarker(psz_spectrum);

	// write out properties
	obs.WriteInteger(psz_ScaleMode, nScaleMode);
	obs.WriteInteger(psz_PwrUpperBound, nPwrUpperBound);
	obs.WriteInteger(psz_PwrLowerBound, nPwrLowerBound);
	obs.WriteInteger(psz_FreqUpperBound, nFreqUpperBound);
	obs.WriteInteger(psz_FreqLowerBound, nFreqLowerBound);
	obs.WriteInteger(psz_FreqScaleRange, nFreqScaleRange);
	obs.WriteInteger(psz_SmoothLevel, nSmoothLevel);
	obs.WriteInteger(psz_PeakSharpFac, nPeakSharpFac);
	obs.WriteInteger(psz_ShowLpcSpectrum, bShowLpcSpectrum);
	obs.WriteInteger(psz_ShowCepSpectrum, bShowCepSpectrum);
	obs.WriteInteger(psz_ShowFormantFreq, bShowFormantFreq);
	obs.WriteInteger(psz_ShowFormantBandwidth, bShowFormantBandwidth);
	obs.WriteInteger(psz_ShowFormantPower, bShowFormantPower);

	obs.WriteEndMarker(psz_spectrum);
}


BOOL SpectrumParm::ReadProperties(Object_istream& obs)
// Read spectrumParm properties from *.psa file.
{
	if ( !obs.bAtBackslash() || !obs.bReadBeginMarker(psz_spectrum) )
	{
		return FALSE;
	}

	while ( !obs.bAtEnd() )
	{
		if ( obs.bReadInteger(psz_ScaleMode, nScaleMode) );
		else if ( obs.bReadInteger(psz_PwrUpperBound, nPwrUpperBound) );
		else if ( obs.bReadInteger(psz_PwrLowerBound, nPwrLowerBound) );
		else if ( obs.bReadInteger(psz_FreqUpperBound, nFreqUpperBound) );
		else if ( obs.bReadInteger(psz_FreqLowerBound, nFreqLowerBound) );
		else if ( obs.bReadInteger(psz_FreqRange, nFreqScaleRange) )
			nFreqScaleRange = nFreqScaleRange ? 0 : 1; // translate old settings to new setting
		else if ( obs.bReadInteger(psz_FreqScaleRange, nFreqScaleRange) );
		else if ( obs.bReadInteger(psz_MinThreshold, nSmoothLevel) );
		else if ( obs.bReadInteger(psz_SmoothLevel, nSmoothLevel) );
		else if ( obs.bReadInteger(psz_PeakSharpFac, nPeakSharpFac) );
		else if ( obs.bReadInteger(psz_ShowLpcSpectrum, bShowLpcSpectrum) );
		else if ( obs.bReadInteger(psz_ShowCepSpectrum, bShowCepSpectrum) );
		else if ( obs.bReadInteger(psz_ShowFormantFreq, bShowFormantFreq) );
		else if ( obs.bReadInteger(psz_ShowFormantBandwidth, bShowFormantBandwidth) );
		else if ( obs.bReadInteger(psz_ShowFormantPower, bShowFormantPower) );
		else if ( obs.bEnd(psz_spectrum) )
			break;
	}
	return TRUE;
}

void SpectrumParm::Init()
{
	const int SamplesPerSec = 22050;
	nScaleMode = 0;
	nPwrUpperBound = 10;
	nPwrLowerBound = -110;
	nFreqUpperBound = SamplesPerSec / 2;
	nFreqLowerBound = 0;
	nFreqScaleRange = 2; // third scale
	nSmoothLevel = 5;
	nPeakSharpFac = 0;
	cWindow = ResearchSettings.m_cWindow;  // since cursor alignment defaults to zero crossing
	bShowLpcSpectrum = TRUE;
	bShowCepSpectrum = FALSE;
	bShowFormantFreq = TRUE;
	bShowFormantBandwidth = FALSE;
	bShowFormantPower = FALSE;
}


static const char* psz_formantcharts        = "formantcharts";
static const char* psz_FromLpcSpectrum      = "bFromLpcSpectrum";
static const char* psz_FromCepstralSpectrum = "bFromCepstralSpectrum";
static const char* psz_TrackFormants        = "bTrackFormants";
static const char* psz_SmoothFormants       = "bSmoothFormants";
static const char* psz_MelScale             = "bMelScale";

void FormantParm::WriteProperties(Object_ostream& obs)
// Write FormantParm properties to *.psa file.
{
	obs.WriteBeginMarker(psz_formantcharts);

	// write out properties
	obs.WriteInteger(psz_FromLpcSpectrum, bFromLpcSpectrum);
	obs.WriteInteger(psz_FromCepstralSpectrum, bFromCepstralSpectrum);
	obs.WriteInteger(psz_TrackFormants, bTrackFormants);
	obs.WriteInteger(psz_SmoothFormants, bSmoothFormants);
	obs.WriteInteger(psz_MelScale, bMelScale);
	obs.WriteEndMarker(psz_formantcharts);
}


BOOL FormantParm::ReadProperties(Object_istream& obs)
// Read FormantParm properties from *.psa file.
{
	if ( !obs.bAtBackslash() || !obs.bReadBeginMarker(psz_spectrum) )
	{
		return FALSE;
	}

	while ( !obs.bAtEnd() )
	{
		if ( obs.bReadInteger(psz_FromLpcSpectrum, bFromLpcSpectrum) );
		else if ( obs.bReadInteger(psz_FromCepstralSpectrum, bFromCepstralSpectrum) );
		else if ( obs.bReadInteger(psz_TrackFormants, bTrackFormants) );
		else if ( obs.bReadInteger(psz_SmoothFormants, bSmoothFormants) );
		else if ( obs.bReadInteger(psz_MelScale, bMelScale) );
		else if ( obs.bEnd(psz_formantcharts) )
			break;
	}
	return TRUE;
}

void FormantParm::Init()
{
	/*
	bFromLpcSpectrum = FALSE;
	bFromCepstralSpectrum = TRUE;
	*/
	bFromLpcSpectrum = TRUE;
	bFromCepstralSpectrum = FALSE;
	bTrackFormants = TRUE;
	bSmoothFormants = FALSE;
	bMelScale = FALSE;
}

static const char* psz_Recording = "Recording";
static const char* psz_Rate      = "Rate";
static const char* psz_Bits      = "Bits";
static const char* psz_Highpass  = "Highpass";
static const char* psz_Mode      = "Mode";

void RecordingParm::WriteProperties(Object_ostream& obs)
// Write spectrumParm properties to *.psa file.
{
	obs.WriteBeginMarker(psz_Recording);

	// write out properties
	obs.WriteInteger(psz_Rate, nRate);
	obs.WriteInteger(psz_Bits, nBits);
	obs.WriteInteger(psz_Highpass, bHighpass);
	obs.WriteInteger(psz_Mode, nMode);

	obs.WriteEndMarker(psz_Recording);
}

BOOL RecordingParm::ReadProperties(Object_istream& obs)
// Read spectrumParm properties from *.psa file.
{
	if ( !obs.bAtBackslash() || !obs.bReadBeginMarker(psz_Recording) )
	{
		return FALSE;
	}

	while ( !obs.bAtEnd() )
	{
		if ( obs.bReadInteger(psz_Rate, nRate) );
		else if ( obs.bReadInteger(psz_Bits, nBits) );
		else if ( obs.bReadInteger(psz_Highpass, bHighpass) );
		else if ( obs.bReadInteger(psz_Mode, nMode) );
		else if ( obs.bEnd(psz_Recording) )
			break;
	}
	return TRUE;
}

void RecordingParm::Init()
{
	nRate = -1;
	nBits = -1;
	bHighpass = TRUE;
	nMode = -1;
}

