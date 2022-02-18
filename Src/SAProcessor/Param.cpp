/////////////////////////////////////////////////////////////////////////////
// param.cpp:
// Implementation of the CParseParm, CSegmentParm, and CPitchParm classes.
// Author: Steve MacLean
// copyright 2000 JAARS Inc. SIL
//
// Revision History
// 1.5Test11.1A
//      RLJ Added CPitchParm, CSpectrumParm classes.
// 06/07/2000
//      RLJ Added CRecordingParm class
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "Process.h"
#include "sa_p_pitch.h"
#include "sa_p_grappl.h"
#include "sa_p_melogram.h"
#include "sa_p_custompitch.h"
#include "sa_p_smoothedpitch.h"
#include "param.h"

static LPCSTR psz_parse            = "parse";
static LPCSTR psz_breakWidth       = "break_ms";
static LPCSTR psz_phraseBreakWidth = "phrase_break_ms";
static LPCSTR psz_minThreshold     = "minThreshold";
static LPCSTR psz_maxThreshold     = "maxThreshold";

CFormantTrackerOptions formantTrackerOptions;

void CParseParm::WriteProperties(ObjectOStream & obs) {
    obs.WriteBeginMarker(psz_parse);

    // write out properties
    obs.WriteInteger(psz_breakWidth, (int)(fBreakWidth * 1000.0 + 0.5));
    obs.WriteInteger(psz_phraseBreakWidth, (int)(fPhraseBreakWidth * 1000.0 + 0.5));
    obs.WriteInteger(psz_maxThreshold, nMaxThreshold);
    obs.WriteInteger(psz_minThreshold, nMinThreshold);

    obs.WriteEndMarker(psz_parse);
}

BOOL CParseParm::ReadProperties(ObjectIStream & obs) {
    if ((!obs.bAtBackslash()) || (!obs.bReadBeginMarker(psz_parse))) {
        return FALSE;
    }

    int nTemp;
    while (!obs.bAtEnd()) {
        if (obs.bReadInteger(psz_breakWidth, nTemp)) {
            fBreakWidth = (float) nTemp/1000.0f;
        } else if (obs.bReadInteger(psz_phraseBreakWidth, nTemp)) {
            fPhraseBreakWidth = (float) nTemp/1000.0f;
        } else if (obs.bReadInteger(psz_maxThreshold, nMaxThreshold)) {
        } else if (obs.bReadInteger(psz_minThreshold, nMinThreshold)) {
        } else if (obs.bEnd(psz_parse)) {
            break;
        }
    }
    return TRUE;
}

void CParseParm::Init() {
    fBreakWidth = (float) 0.025;
    fPhraseBreakWidth = (float) 0.5;
    nMaxThreshold = 10;
    nMinThreshold = 5;
    nParseMode = 2;
}

static LPCSTR psz_segment   = "segment";
static LPCSTR psz_segmentWidth   = "segment_ms";
static LPCSTR psz_minChange   = "minChange";
static LPCSTR psz_minZeroCrossing   = "minZeroCrossing";

void CSegmentParm::WriteProperties(ObjectOStream & obs) {
    obs.WriteBeginMarker(psz_segment);

    // write out properties
    obs.WriteInteger(psz_segmentWidth, (int)(fSegmentWidth * 1000.0 + 0.5));
    obs.WriteInteger(psz_minChange, nChThreshold);
    obs.WriteInteger(psz_minZeroCrossing, nZCThreshold);

    obs.WriteEndMarker(psz_segment);
}

BOOL CSegmentParm::ReadProperties(ObjectIStream & obs) {
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_segment)) {
        return FALSE;
    }

    int nTemp;
    while (!obs.bAtEnd()) {
        if (obs.bReadInteger(psz_segmentWidth, nTemp)) {
            fSegmentWidth = (float)nTemp/1000.0f;
        } else if (obs.bReadInteger(psz_minChange, nChThreshold));
        else if (obs.bReadInteger(psz_minZeroCrossing, nZCThreshold));
        else if (obs.bEnd(psz_segment)) {
            break;
        }
    }

    return TRUE;
}

void CSegmentParm::Init() {
    fSegmentWidth = (float) 0.020;
    nChThreshold = 17;
    nZCThreshold = 50;
    nSegmentMode = 0;
    bKeepSegments = TRUE;
}

void CUttParm::Init(int nBitsPerSample) {
    nMinFreq = CECIL_PITCH_MINFREQ;
    nMaxFreq = CECIL_PITCH_MAXFREQ;
    nCritLoud = CECIL_PITCH_VOICING << (nBitsPerSample - 8);
    nMaxChange = CECIL_PITCH_CHANGE;
    nMinGroup = CECIL_PITCH_GROUP;
    nMaxInterp = CECIL_PITCH_INTERPOL;
}

int CUttParm::TruncatedCritLoud(int nBitsPerSample) const {
    if (nCritLoud<128) {
        return nCritLoud;
    }

    int nResult=nCritLoud;

    if (nBitsPerSample > 8) {
        if (nResult >= 256) {
            nResult >>= nBitsPerSample-8;
        }
    }
    // 99.9% of max counts is the largest legal value internally
    if (nResult > (int)(0.999 * 128 * 16)) {
        nResult = (int)(0.999 * 128 * 16);
    }
    if (nResult < 0) {
        nResult = 0;
    }

    return nResult;
}

static LPCSTR psz_music             = "music";
static LPCSTR psz_calcRangeMode    = "CalcRangeMode";
static LPCSTR psz_calcUpperBound   = "CalcUpperBound";
static LPCSTR psz_calcLowerBound   = "CalcLowerBound";
static LPCSTR psz_calcManualUpper   = "ManualCalcUpper";
static LPCSTR psz_calcManualLower   = "ManualCalcLower";
static LPCSTR psz_pitchRangeMode = "RangeMode";
static LPCSTR psz_pitchScaleMode = "ScaleMode";
static LPCSTR psz_pitchUpperBound = "UpperBound";
static LPCSTR psz_pitchLowerBound = "LowerBound";
static LPCSTR psz_pitchUseCepMedianFilter = "UseCepMedianFilter";
static LPCSTR psz_pitchCepMedianFilterSize = "CepMedianFilterSize";

// RLJ 09/26/2000: Bug GPI-01
static LPCSTR psz_pitchManualUpper = "ManualUpper";
static LPCSTR psz_pitchManualLower = "ManualLower";

void CMusicParm::WriteProperties(ObjectOStream & obs) {
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

BOOL CMusicParm::ReadProperties(ObjectIStream & obs) {
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_music)) {
        return FALSE;
    }

    while (!obs.bAtEnd()) {
        //    if ( obs.bReadInteger(psz_pitchRangeMode, nRangeMode) );
        //    else
        if (obs.bReadInteger(psz_pitchUpperBound, nUpperBound));
        else if (obs.bReadInteger(psz_pitchLowerBound, nLowerBound));

        else if (obs.bReadInteger(psz_pitchManualUpper, nManualUpper));
        else if (obs.bReadInteger(psz_pitchManualLower, nManualLower));
        //    if ( obs.bReadInteger(psz_calcRangeMode, nCalcRangeMode) );
        else if (obs.bReadInteger(psz_calcUpperBound, nCalcUpperBound));
        else if (obs.bReadInteger(psz_calcLowerBound, nCalcLowerBound));
        else if (obs.bReadInteger(psz_calcManualUpper, nManualCalcUpper));
        else if (obs.bReadInteger(psz_calcManualLower, nManualCalcLower));

        else if (obs.bEnd(psz_music)) {
            break;
        }
    }

    return TRUE;
}

void CMusicParm::Init() {
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

void CMusicParm::GetAutoRange(Model* pModel, CProcessMelogram* pMelogram, int & nUpperBound, int & nLowerBound) {

	CMusicParm temp;
    temp.Init();

    if (pMelogram->IsDataReady() && !pMelogram->IsStatusFlag(PROCESS_NO_PITCH)) {
        int nLowerBound2 = (int)floor((double)pMelogram->GetMinValue() / 100.0);
        int nUpperBound2 = (int)ceil((double)pMelogram->GetMaxValue() / 100.0);

        // make sure the legend scale spans at least an octave
        int nRange = (nUpperBound2 - nLowerBound2);
        if (nRange < 12) {
            nLowerBound2 = (int)(((double)(nLowerBound2 + nUpperBound2) / 2.0) - 6.);
            nUpperBound2 = (int)(nLowerBound2 + 12);
        }
        temp.nUpperBound = nUpperBound2;
        temp.nLowerBound = nLowerBound2;
    }

    nUpperBound = temp.nUpperBound;
    nLowerBound = temp.nLowerBound;
}

static LPCSTR psz_intensity = "intensity";
static LPCSTR psz_intensityScaleMode = "ScaleMode";

CIntensityParm::CIntensityParm() {
    Init();
}

void CIntensityParm::WriteProperties(ObjectOStream & obs) {
    obs.WriteBeginMarker(psz_intensity);
    // write out properties
    obs.WriteInteger(psz_intensityScaleMode,  nScaleMode);
    obs.WriteEndMarker(psz_intensity);
}

BOOL CIntensityParm::ReadProperties(ObjectIStream & obs) {
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_intensity)) {
        return FALSE;
    }

    while (!obs.bAtEnd()) {
        if (obs.bReadInteger(psz_intensityScaleMode, nScaleMode));
        else if (obs.bEnd(psz_intensity)) {
            break;
        }
    }

    return TRUE;
}

void CIntensityParm::Init() {
    nScaleMode = 1;
}

static const char * psz_wavelet              = "Wavelet";               // ARH 8/1/01 Added for Wavelet scalogram graph
static const char * psz_MaxThreshold         = "nMaxThreshold";
static const char * psz_MinThreshold         = "nMinThreshold";
static const char * psz_Resolution           = "nResolution3";
static const char * psz_Color                = "nColor";
static const char * psz_Frequency            = "nFrequency";


// ARH 8/1/01 Added for wavelet scalogram graph
void CWaveletParm::WriteProperties(ObjectOStream & obs)
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
BOOL CWaveletParm::ReadProperties(ObjectIStream & obs)
// Read waveletParm properties from *.psa file.
{
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_wavelet)) {
        return FALSE;
    }

    while (!obs.bAtEnd()) {
        if (obs.bReadInteger(psz_Resolution, nResolution));
        else if (obs.bReadInteger(psz_Color, nColor));
        else if (obs.bReadInteger(psz_Frequency, nFrequency));
        else if (obs.bReadInteger(psz_MaxThreshold, nMaxThreshold));
        else if (obs.bReadInteger(psz_MinThreshold, nMinThreshold));
        else if (obs.bEnd(psz_wavelet)) {
            break;
        }
    }
    return TRUE;
}

// ARH 8/1/01 Added for wavelet scalogram graph
void CWaveletParm::Init() {
    nResolution = 1;
    nColor = 0;
    nFrequency = 3000;
    nMinThreshold = 0;
    nMaxThreshold = 234;
    bSmoothSpectra = TRUE;
}

static LPCSTR psz_formantcharts        = "formantcharts";
static LPCSTR psz_FromLpcSpectrum      = "bFromLpcSpectrum";
static LPCSTR psz_FromCepstralSpectrum = "bFromCepstralSpectrum";
static LPCSTR psz_TrackFormants        = "bTrackFormants";
static LPCSTR psz_SmoothFormants       = "bSmoothFormants";
static LPCSTR psz_MelScale             = "bMelScale";
static LPCSTR psz_spectrum = "spectrum";

// Write CFormantParm properties to *.psa file.
void CFormantParm::WriteProperties(ObjectOStream & obs) {
    obs.WriteBeginMarker(psz_formantcharts);

    // write out properties
    obs.WriteInteger(psz_FromLpcSpectrum, bFromLpcSpectrum);
    obs.WriteInteger(psz_FromCepstralSpectrum, bFromCepstralSpectrum);
    obs.WriteInteger(psz_TrackFormants, bTrackFormants);
    obs.WriteInteger(psz_SmoothFormants, bSmoothFormants);
    obs.WriteInteger(psz_MelScale, bMelScale);
    obs.WriteEndMarker(psz_formantcharts);
}

// Read CFormantParm properties from *.psa file.
BOOL CFormantParm::ReadProperties(ObjectIStream & obs) {
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_spectrum)) {
        return FALSE;
    }

    while (!obs.bAtEnd()) {
        if (obs.bReadInteger(psz_FromLpcSpectrum, bFromLpcSpectrum));
        else if (obs.bReadInteger(psz_FromCepstralSpectrum, bFromCepstralSpectrum));
        else if (obs.bReadInteger(psz_TrackFormants, bTrackFormants));
        else if (obs.bReadInteger(psz_SmoothFormants, bSmoothFormants));
        else if (obs.bReadInteger(psz_MelScale, bMelScale));
        else if (obs.bEnd(psz_formantcharts)) {
            break;
        }
    }
    return TRUE;
}

void CFormantParm::Init() {
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

static LPCSTR psz_Recording = "Recording";
static LPCSTR psz_Rate      = "Rate";
static LPCSTR psz_Bits      = "Bits";
static LPCSTR psz_Highpass  = "Highpass";
static LPCSTR psz_Mode      = "Mode";

void CRecordingParm::WriteProperties(ObjectOStream & obs)
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

BOOL CRecordingParm::ReadProperties(ObjectIStream & obs)
// Read spectrumParm properties from *.psa file.
{
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_Recording)) {
        return FALSE;
    }

    while (!obs.bAtEnd()) {
        if (obs.bReadInteger(psz_Rate, nRate));
        else if (obs.bReadInteger(psz_Bits, nBits));
        else if (obs.bReadInteger(psz_Highpass, bHighpass));
        else if (obs.bReadInteger(psz_Mode, nMode));
        else if (obs.bEnd(psz_Recording)) {
            break;
        }
    }
    return TRUE;
}

void CRecordingParm::Init() {
    nRate = -1;
    nBits = -1;
    bHighpass = TRUE;
    nMode = -1;
}
