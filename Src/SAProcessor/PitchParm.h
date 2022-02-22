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
#ifndef _PITCHPARAM_H
#define _PITCHPARAM_H

#include "SA_DSP.h"

class CPitchParm {                  // pitch parameters
public:
    void WriteProperties(CObjectOStream & obs);
    BOOL ReadProperties(CObjectIStream & obs);
    void Init();
    static void GetAutoRange(Model* pModel, CProcess * pGrappl, int & nUpperBound, int & nLowerBound);

    int nRangeMode;                 // frequency range display mode
    int nScaleMode;                 // scale display mode
    int nUpperBound;                // upper frequency display boundary
    int nLowerBound;                // lower frequency display boundary
    BOOL bUseCepMedianFilter;       // TRUE if the CepstralPitch graph..
    // ..should use the Median Filter.
    BYTE nCepMedianFilterSize;      // number of points to do the..
    // ..median filter over

    // RLJ 09/26/2000: Bug GPI-01
    int nManualPitchUpper;          // temporary location to save manual pitch upper boundary
    int nManualPitchLower;          // temporary location to save manual pitch lower boundary
};

#endif
