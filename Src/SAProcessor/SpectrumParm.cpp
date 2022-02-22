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
#include "sa_process.h"
#include "sa_p_pitch.h"
#include "sa_p_grappl.h"
#include "sa_p_melogram.h"
#include "sa_p_custompitch.h"
#include "sa_p_smoothedpitch.h"
#include "spectrumparm.h"
#include "ObjectIStream.h"
#include "ObjectOStream.h"
#include "ResearchSettings.h"

static LPCSTR psz_spectrum       = "spectrum";
static LPCSTR psz_ScaleMode      = "nScaleMode";
static LPCSTR psz_PwrUpperBound  = "nPwrUpperBound";
static LPCSTR psz_PwrLowerBound  = "nPwrLowerBound";
static LPCSTR psz_FreqUpperBound = "nFreqUpperBound";
static LPCSTR psz_FreqLowerBound = "nFreqLowerBound";
static LPCSTR psz_FreqRange      = "nFreqRange";
static LPCSTR psz_FreqScaleRange = "nFreqScaleRange";
static LPCSTR psz_SmoothLevel    = "nSmoothLevel";
static LPCSTR psz_PeakSharpFac   = "nPeakSharpFac";
static LPCSTR psz_ShowLpcSpectrum = "bShowLpcSpectrum";
static LPCSTR psz_ShowCepSpectrum = "bShowCepSpectrum";
static LPCSTR psz_ShowFormantFreq = "bShowFormantFreq";
static LPCSTR psz_ShowFormantBandwidth = "bShowFormantBandwidth";
static LPCSTR psz_ShowFormantPower = "bShowFormantPower";
static const char* psz_MinThreshold = "nMinThreshold";

// Write spectrumParm properties to *.psa file.
void CSpectrumParm::WriteProperties(CObjectOStream & obs) {
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

// Read spectrumParm properties from *.psa file.
BOOL CSpectrumParm::ReadProperties(CObjectIStream & obs) {
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_spectrum)) {
        return FALSE;
    }

    while (!obs.bAtEnd()) {
        if (obs.bReadInteger(psz_ScaleMode, nScaleMode));
        else if (obs.bReadInteger(psz_PwrUpperBound, nPwrUpperBound));
        else if (obs.bReadInteger(psz_PwrLowerBound, nPwrLowerBound));
        else if (obs.bReadInteger(psz_FreqUpperBound, nFreqUpperBound));
        else if (obs.bReadInteger(psz_FreqLowerBound, nFreqLowerBound));
        else if (obs.bReadInteger(psz_FreqRange, nFreqScaleRange)) {
            nFreqScaleRange = nFreqScaleRange ? 0 : 1;    // translate old settings to new setting
        } else if (obs.bReadInteger(psz_FreqScaleRange, nFreqScaleRange));
        else if (obs.bReadInteger(psz_MinThreshold, nSmoothLevel));
        else if (obs.bReadInteger(psz_SmoothLevel, nSmoothLevel));
        else if (obs.bReadInteger(psz_PeakSharpFac, nPeakSharpFac));
        else if (obs.bReadInteger(psz_ShowLpcSpectrum, bShowLpcSpectrum));
        else if (obs.bReadInteger(psz_ShowCepSpectrum, bShowCepSpectrum));
        else if (obs.bReadInteger(psz_ShowFormantFreq, bShowFormantFreq));
        else if (obs.bReadInteger(psz_ShowFormantBandwidth, bShowFormantBandwidth));
        else if (obs.bReadInteger(psz_ShowFormantPower, bShowFormantPower));
        else if (obs.bEnd(psz_spectrum)) {
            break;
        }
    }
    return TRUE;
}

void CSpectrumParm::Init(App & app) {
    const int SamplesPerSec = 22050;
    nScaleMode = 0;
    nPwrUpperBound = 10;
    nPwrLowerBound = -110;
    nFreqUpperBound = SamplesPerSec / 2;
    nFreqLowerBound = 0;
    nFreqScaleRange = 2; // third scale
    nSmoothLevel = 5;
    nPeakSharpFac = 0;
    bShowLpcSpectrum = TRUE;
    bShowCepSpectrum = FALSE;
    bShowFormantFreq = TRUE;
    bShowFormantBandwidth = FALSE;
    bShowFormantPower = FALSE;
    window = app.GetResearchSettings().GetWindow();
}
