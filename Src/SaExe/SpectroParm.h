#ifndef SPECTROPARM_H
#define SPECTROPARM_H

#include "dsp\dspWins.h"
#include "objectostream.h"
#include "objectistream.h"

class CSpectroParm                   // spectrogram parameters
{
public:
    int  nResolution;       // resolution of display
    int  nColor;            // color of display
    int  nOverlay;          // formants overlay mode
    int  nFrequency;        // frequency range to calculate
    int  nMinThreshold;     // minimum threshold
    int  nMaxThreshold;     // maximum threshold
	BOOL bShowFormants;		// true if picture or any of the formants should be displayed
    BOOL bShowF1;           // show/hide formants (TRUE = show)
    BOOL bShowF2;
    BOOL bShowF3;
    BOOL bShowF4;
    BOOL bShowF5andUp;
    BOOL bSmoothFormantTracks;  // TRUE = smooth formant tracks before displaying them
    BOOL bSmoothSpectra;    // TRUE = smooth spectra
    BOOL bFormantColor;
    BOOL bShowPitch;

    void WritePropertiesA(CObjectOStream & obs);
    void WritePropertiesB(CObjectOStream & obs);
    BOOL ReadPropertiesA(CObjectIStream & obs);
    BOOL ReadPropertiesB(CObjectIStream & obs);

    void Init();
    static const float DspWinBandwidth[3];
    static float Bandwidth(int nDspMode)
    {
        return nDspMode > 2 ? 0 : DspWinBandwidth[nDspMode];
    }
    float Bandwidth() const
    {
        return Bandwidth(nResolution);
    }

private:
    void WriteProperties(LPCSTR pszMarker, CObjectOStream & obs);
    BOOL ReadProperties(LPCSTR pszMarker, CObjectIStream & obs);
};
#endif
