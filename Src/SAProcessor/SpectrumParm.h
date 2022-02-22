/////////////////////////////////////////////////////////////////////////////
// param.h:
// Interface of the CParseParm, CSegmentParm, and CPitchParm classes.
// Author: Steve MacLean
// copyright 2000 JAARS Inc. SIL
//
// Revision History
// 1.5Test11.1A
//      RLJ Changed CPitchParm and CSpectrumParm from
//            structures in appdefs.h to classes here..
// 06/07/2000
//      RLJ Added CRecordingParm class
/////////////////////////////////////////////////////////////////////////////
#ifndef _SPECTRUM_PARM_H
#define _SPECTRUM_PARM_H

#include "SA_DSP.h"

// spectrum parameters
class CSpectrumParm { 

public:
    void Init(App & app);
    void WriteProperties(CObjectOStream& obs);
    BOOL ReadProperties(CObjectIStream& obs);
    CWindowSettings getWindow() { return window; }
    void setWindow(CWindowSettings val) { window = val; }

    int nScaleMode;             // scale display mode
    int nPwrUpperBound;         // upper power display boundary
    int nPwrLowerBound;         // lower power display boundary
    int nFreqUpperBound;        // upper frequency display boundary
    int nFreqLowerBound;        // lower frequency display boundary
    int nFreqScaleRange;        // frequency range: 0 = full scale, 1 = half scale, 2 = third scale, 3 = quarter scale
    int nSmoothLevel;           // level to control spectral smoothing
    int nPeakSharpFac;          // factor to control sharpening of formant peaks
    BOOL bShowLpcSpectrum;      // TRUE = show LPC-smoothed spectrum
    BOOL bShowCepSpectrum;      // TRUE = show cepstrally-smoothed spectrum
    BOOL bShowFormantFreq;      // TRUE = show formant frequencies
    BOOL bShowFormantBandwidth; // TRUE = show formant bandwidths
    BOOL bShowFormantPower;     // TRUE = show formant powers

private:
    CWindowSettings window;     // DSP Window settings
};

#endif
