#include "stdafx.h"
#include "SpectroParm.h"
#include "DSP/SPECTGRM.H"

static const char * psz_spectroA             = "spectrogramA";
static const char * psz_spectroB             = "spectrogramB";
static const char * psz_Resolution           = "nResolution3";
static const char * psz_ResolutionOld        = "nResolution";
static const char * psz_wavelet              = "Wavelet";               // ARH 8/1/01 Added for Wavelet scalogram graph
static const char * psz_Color                = "nColor";
static const char * psz_ShowF1               = "bShowF1";
static const char * psz_ShowF2               = "bShowF2";
static const char * psz_ShowF3               = "bShowF3";
static const char * psz_ShowF4               = "bShowF4";
static const char * psz_ShowF5andUp          = "bShowF5andUp";
static const char * psz_SmoothFormantTracks  = "bSmoothFormantTracks";
static const char * psz_Frequency            = "nFrequency";
static const char * psz_MaxThreshold         = "nMaxThreshold";
static const char * psz_MinThreshold         = "nMinThreshold";
static const char * psz_Overlay              = "nOverlay";
static const char * psz_ShowPitch            = "bShowPitch";

void CSpectroParm::WritePropertiesA(Object_ostream & obs) {
    WriteProperties(psz_spectroA, obs);
}

void CSpectroParm::WritePropertiesB(Object_ostream & obs) {
    WriteProperties(psz_spectroB, obs);
}

// Write spectroParm properties to *.psa file.
void CSpectroParm::WriteProperties(const char * pszMarker, Object_ostream & obs) {
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

BOOL CSpectroParm::ReadPropertiesA(Object_istream & obs) {
    return ReadProperties(psz_spectroA, obs);
}

BOOL CSpectroParm::ReadPropertiesB(Object_istream & obs) {
    return ReadProperties(psz_spectroB, obs);
}

// Read spectroParm properties from *.psa file.
BOOL CSpectroParm::ReadProperties(const char * pszMarker, Object_istream & obs) {
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(pszMarker)) {
        return FALSE;
    }

    while (!obs.bAtEnd()) {
        if (obs.bReadInteger(psz_Resolution, nResolution));
        // This is temporary until *.psa files are built with this version
        else if (obs.bReadInteger(psz_ResolutionOld, nResolution)) {
            nResolution *= 2;    // map 0->0 & 1->2
        } else if (obs.bReadInteger(psz_Color, nColor));
        else if (obs.bReadBool(psz_ShowF1, bShowF1));
        else if (obs.bReadBool(psz_ShowF2, bShowF2));
        else if (obs.bReadBool(psz_ShowF3, bShowF3));
        else if (obs.bReadBool(psz_ShowF4, bShowF4));
        else if (obs.bReadBool(psz_ShowF5andUp, bShowF5andUp));
        else if (obs.bReadBool(psz_SmoothFormantTracks, bSmoothFormantTracks));
        else if (obs.bReadInteger(psz_Frequency, nFrequency));
        else if (obs.bReadInteger(psz_MaxThreshold, nMaxThreshold));
        else if (obs.bReadInteger(psz_MinThreshold, nMinThreshold));
        else if (obs.bReadInteger(psz_Overlay, nOverlay));
        else if (obs.bReadBool(psz_ShowPitch, bShowPitch));
        else if (obs.bEnd(pszMarker)) {
            break;
        }
    }
    return TRUE;
}

const float CSpectroParm::DspWinBandwidth[] = {(float)NARROW_BW, (float)MEDIUM_BW, (float)WIDE_BW};

void CSpectroParm::Init() {
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

