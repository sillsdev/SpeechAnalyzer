#include "stdafx.h"
#include "SpectroParm.h"
#include "DSP/SPECTGRM.H"

static LPCSTR psz_spectroA             = "spectrogramA";
static LPCSTR psz_spectroB             = "spectrogramB";
static LPCSTR psz_Resolution           = "nResolution3";
static LPCSTR psz_ResolutionOld        = "nResolution";
static LPCSTR psz_wavelet              = "Wavelet";               // ARH 8/1/01 Added for Wavelet scalogram graph
static LPCSTR psz_Color                = "nColor";
static LPCSTR psz_ShowPitch            = "bShowPitch";
static LPCSTR psz_ShowF1               = "bShowF1";
static LPCSTR psz_ShowF2               = "bShowF2";
static LPCSTR psz_ShowF3               = "bShowF3";
static LPCSTR psz_ShowF4               = "bShowF4";
static LPCSTR psz_ShowFormants         = "bShowFormants";
static LPCSTR psz_ShowF5andUp          = "bShowF5andUp";
static LPCSTR psz_SmoothFormantTracks  = "bSmoothFormantTracks";
static LPCSTR psz_Frequency            = "nFrequency";
static LPCSTR psz_MaxThreshold         = "nMaxThreshold";
static LPCSTR psz_MinThreshold         = "nMinThreshold";
static LPCSTR psz_Overlay              = "nOverlay";

void CSpectroParm::WritePropertiesA(CObjectOStream & obs)
{
    WriteProperties(psz_spectroA, obs);
}

void CSpectroParm::WritePropertiesB(CObjectOStream & obs)
{
    WriteProperties(psz_spectroB, obs);
}

// Write spectroParm properties to *.psa file.
void CSpectroParm::WriteProperties(LPCSTR pszMarker, CObjectOStream & obs)
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
	obs.WriteBool(psz_ShowFormants, bShowFormants);

    obs.WriteEndMarker(pszMarker);
}

BOOL CSpectroParm::ReadPropertiesA(CObjectIStream & obs)
{
    return ReadProperties(psz_spectroA, obs);
}

BOOL CSpectroParm::ReadPropertiesB(CObjectIStream & obs)
{
    return ReadProperties(psz_spectroB, obs);
}

// Read spectroParm properties from *.psa file.
BOOL CSpectroParm::ReadProperties(LPCSTR pszMarker, CObjectIStream & obs)
{
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(pszMarker))
    {
        return FALSE;
    }

	bool showFormants = false;

    while (!obs.bAtEnd())
    {
        if (obs.bReadInteger(psz_Resolution, nResolution));
        // This is temporary until *.psa files are built with this version
        else if (obs.bReadInteger(psz_ResolutionOld, nResolution))
        {
            nResolution *= 2;    // map 0->0 & 1->2
        }
        else if (obs.bReadInteger(psz_Color, nColor));
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
		else if (obs.bReadBool(psz_ShowFormants, bShowFormants)) showFormants=true;
        else if (obs.bEnd(pszMarker))
        {
            break;
        }
    }

	/*
	* model old behaviour.  Previously if any of the items were checked, than formants were shown.
	* this has now changed.
	* if the global is off, nothing is shown - but the formant booleans and the pitch boolean will retain
	* their previous setting.
	* - so - if we don't find it in the settings file, we will assume this is a 'old' settings file, and
	* and use the settings for the formant and pitch booleans to set the 'showFormants' member.
	*/
	if ((!showFormants) && ((bShowF1) || (bShowF2) || (bShowF3) || (bShowF4) || (bShowF5andUp) || (bShowPitch))) {
		bShowFormants = TRUE;
	}
    return TRUE;
}

const float CSpectroParm::DspWinBandwidth[] = {(float)NARROW_BW, (float)MEDIUM_BW, (float)WIDE_BW};

void CSpectroParm::Init()
{
    nResolution = 2;
    nColor = 1; // Monochrome
    nOverlay = 0;
    bShowF1 = TRUE;
    bShowF2 = TRUE;
    bShowF3 = TRUE;
    bShowF4 = FALSE;
	bShowFormants = FALSE;
    bShowF5andUp = FALSE;
    bSmoothFormantTracks = TRUE;
    nFrequency = 3200;
    nMinThreshold = 61;
    nMaxThreshold = 209;
    bSmoothSpectra = TRUE;
    bFormantColor = TRUE;
    bShowPitch = FALSE;
}

