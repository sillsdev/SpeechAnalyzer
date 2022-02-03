#ifndef SPECTROPARM_H
#define SPECTROPARM_H

#include <dspWins.h>

class CSpectroParm {                 // spectrogram parameters
public:
    int  nResolution;       // resolution of display
    int  nOverlay;          // formants overlay mode
    int  nFrequency;        // frequency range to calculate
    int  nMinThreshold;     // minimum threshold
    int  nMaxThreshold;     // maximum threshold
    BOOL bShowF1;           // show/hide formants (TRUE = show)
    BOOL bShowF2;
    BOOL bShowF3;
    BOOL bShowF4;
    BOOL bShowF5andUp;
    BOOL bSmoothFormantTracks;  // TRUE = smooth formant tracks before displaying them
    BOOL bSmoothSpectra;    // TRUE = smooth spectra
    BOOL bFormantColor;
    BOOL bShowPitch;

    void WritePropertiesA(ObjectOStream & obs);
    void WritePropertiesB(ObjectOStream & obs);
    BOOL ReadPropertiesA(ObjectIStream & obs);
    BOOL ReadPropertiesB(ObjectIStream & obs);

    void Init();
    static const float DspWinBandwidth[3];
    static float Bandwidth(int nDspMode) {
        return nDspMode > 2 ? 0 : DspWinBandwidth[nDspMode];
    }
    float Bandwidth() const {
        return Bandwidth(nResolution);
    }
    void SetShowFormants(bool value) {
        bShowFormants = value;
    }
    bool GetShowFormants() const {
        return bShowFormants;
    }
    void SetColor(int color) {
        nColor = color;
    }
    int GetColor() const {
        return nColor;
    }

private:
    void WriteProperties(LPCSTR pszMarker, ObjectOStream & obs);
    BOOL ReadProperties(LPCSTR pszMarker, ObjectIStream & obs);

    bool bShowFormants;     // true if picture or any of the formants should be displayed
    int  nColor;            // color of display
};
#endif
