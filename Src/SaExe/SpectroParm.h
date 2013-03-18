#ifndef SPECTROPARM_H
#define SPECTROPARM_H

#include "dsp\dspWins.h"
#include "settings\obstream.h"

class CSpectroParm {                 // spectrogram parameters
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

    void WritePropertiesA(Object_ostream & obs);
    void WritePropertiesB(Object_ostream & obs);
    BOOL ReadPropertiesA(Object_istream & obs);
    BOOL ReadPropertiesB(Object_istream & obs);

    void Init();
    static const float DspWinBandwidth[3];
    static float Bandwidth(int nDspMode) {
        return nDspMode > 2 ? 0 : DspWinBandwidth[nDspMode];
    }
    float Bandwidth() const {
        return Bandwidth(nResolution);
    }

private:
    void WriteProperties(const char * pszMarker, Object_ostream & obs);
    BOOL ReadProperties(const char * pszMarker, Object_istream & obs);
};
#endif
