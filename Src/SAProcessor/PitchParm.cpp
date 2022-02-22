#include "pch.h"
#include "sa_process.h"
#include "sa_p_pitch.h"
#include "sa_p_grappl.h"
#include "sa_p_melogram.h"
#include "sa_p_custompitch.h"
#include "sa_p_smoothedpitch.h"
#include "pitchparm.h"
#include "ObjectIStream.h"
#include "ObjectOStream.h"

static LPCSTR psz_parse            = "parse";
static LPCSTR psz_breakWidth       = "break_ms";
static LPCSTR psz_phraseBreakWidth = "phrase_break_ms";
static LPCSTR psz_minThreshold     = "minThreshold";
static LPCSTR psz_maxThreshold     = "maxThreshold";
static LPCSTR psz_pitch             = "pitch";
static LPCSTR psz_pitchRangeMode    = "RangeMode";
static LPCSTR psz_pitchScaleMode    = "ScaleMode";
static LPCSTR psz_pitchUpperBound   = "UpperBound";
static LPCSTR psz_pitchLowerBound   = "LowerBound";
static LPCSTR psz_pitchUseCepMedianFilter  = "UseCepMedianFilter";
static LPCSTR psz_pitchCepMedianFilterSize = "CepMedianFilterSize";
// RLJ 09/26/2000: Bug GPI-01
static LPCSTR psz_pitchManualUpper   = "ManualUpper";
static LPCSTR psz_pitchManualLower   = "ManualLower";

void CPitchParm::WriteProperties(CObjectOStream & obs) {
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

BOOL CPitchParm::ReadProperties(CObjectIStream & obs) {
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_pitch)) {
        return FALSE;
    }

    int nTemp;
    while (!obs.bAtEnd()) {
        if (obs.bReadInteger(psz_pitchRangeMode, nRangeMode));
        else if (obs.bReadInteger(psz_pitchScaleMode, nScaleMode));
        else if (obs.bReadInteger(psz_pitchUpperBound, nUpperBound));
        else if (obs.bReadInteger(psz_pitchLowerBound, nLowerBound));
        else if (obs.bReadBool(psz_pitchUseCepMedianFilter, bUseCepMedianFilter));
        else if (obs.bReadInteger(psz_pitchCepMedianFilterSize, nTemp)) {
            nCepMedianFilterSize = (BYTE)nTemp;
        }

        // RLJ 09/26/2000: Bug GPI-01
        else if (obs.bReadInteger(psz_pitchManualUpper, nManualPitchUpper));
        else if (obs.bReadInteger(psz_pitchManualLower, nManualPitchLower));

        else if (obs.bEnd(psz_pitch)) {
            break;
        }
    }

    return TRUE;
}

void CPitchParm::Init() {
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

void CPitchParm::GetAutoRange(Model * pModel, CProcess * pGrappl, int & nUpperBound, int & nLowerBound) {

    CPitchParm temp;
    temp.Init();

    // If auto pitch is not done look for alternates
    if (!pGrappl->IsDataReady()) {
        pGrappl = pModel->GetSmoothedPitch();
    }
    if (!pGrappl->IsDataReady()) {
        pGrappl = pModel->GetPitch();
    }
    if (!pGrappl->IsDataReady()) {
        pGrappl = pModel->GetCustomPitch();
    }
    if (pGrappl->IsDataReady() && !pGrappl->IsStatusFlag(PROCESS_NO_PITCH)) {
        // auto range mode
        int nMinData = pGrappl->GetMinValue();
        int nMaxData = pGrappl->GetMaxValue();

        nMinData /= PRECISION_MULTIPLIER;
        nMaxData /= PRECISION_MULTIPLIER;

        // make sure the legend scale spans at least an octave
        double fSpan = (double)nMaxData / nMinData;
        if (fSpan < 2.0) {
            nMinData = (short)((double)(nMinData + nMaxData) / 3.0);
            nMaxData = (short)(2 * nMinData);
        }

        // Add a little space to the top/bottom
        nMaxData = int(nMaxData * 1.05);
        nMinData = int(nMinData / 1.05);
        if (nMinData < 0) {
            nMinData = 0;
        }

        temp.nUpperBound = nMaxData;
        temp.nLowerBound = nMinData;
    }

    nUpperBound = temp.nUpperBound;
    nLowerBound = temp.nLowerBound;
}
