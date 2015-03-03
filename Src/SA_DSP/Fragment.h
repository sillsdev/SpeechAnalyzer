#ifndef FRAGMENT_H
#define FRAGMENT_H

#include "dspTypes.h"
#include "Error.h"
#include "Signal.h"


//  Define default cutoff frequency for moving sum filter.  This filter is used
//  to locate pitch periods by passing the fundamental frequency in the waveform
//  while removing or attenuating harmonics.  The default value is assumed during
//  unvoiced portions of the waveform.
#define DEFAULT_CUTOFF  1000


#pragma pack(1)
struct SFragParms {
    uint32 dwOffset;                    // sample offset into waveform
    uint16 wLength;                     // length of fragment in samples
    short  nPitch;                      // pitch value for fragment
    uint16 wAvgMag;                     // average magnitude of samples within fragment
};
#pragma pack()

struct SPitchParms {
    uint16      wSmpIntvl;              // calculation interval in waveform samples
    uint16      wScaleFac;              // scale factor
    uint16      wCalcRangeMin;          // min pitch allowed
    uint16      wCalcRangeMax;          // max pitch allowed
    uint32      dwContourLength;        // number of pitch data values, including unvoiced portions
    short    *   psContour;             // pointer to pitch data
};

class CFragment {
public:
    static char * Copyright(void);
    static float Version(void);
    static dspError_t CreateObject(CFragment ** ppoFragment, SFragParms * pstFragParmBfr, uint32 dwFragParmBfrLength,
                                   SSigParms & stSigParm, void * pWaveBfr, uint32 dwWaveBfrLength,
                                   SPitchParms & stPitchParm, short * pPitchBfr, uint32 dwPitchBfrLength);
    dspError_t Fragment(void);
    ~CFragment();
    uint32 GetWaveBlockIndex();
    uint32 GetPitchBlockIndex();
    uint32 GetFragmentBlockLength();
    uint32 GetFragmentCount();

private:
    static dspError_t ValidateSignalParms(SSigParms & stSignal);
    static dspError_t ValidatePitchParms(SPitchParms & stPitchParm);

    CFragment(SSigParms & stSigParm, void * pWaveBfr, uint32 dwWaveBfrLength,
              SPitchParms & stPitchParm, short * psPitchBfr, uint32 dwPitchBfrLength,
              SFragParms * pstFragParmBfr, uint32 dwFragParmBfrLength);

    dspError_t Process(uint8 * pbWaveBfr);
    dspError_t Process(short * psWaveBfr);
    uint32 m_dwSigLength;
    uint32 m_wSmpRate;
    int8 m_sbSmpFormat;
    void * m_pWaveBfr;
    uint32 m_dwWaveBfrLength;
    uint32 m_dwWaveIndex;
    uint32 m_dwWaveBlock;
    uint32 m_dwWaveCrossing;
    short m_nMinSmpValue;
    short m_nMaxSmpValue;
    uint32 m_dwMaxRange;
    uint32 m_dwMaxRangeCrossing;
    bool m_bUnvoiced;
    uint16 m_wPitchScaleFac;
    uint16 m_wPitchCalcIntvl;
    uint16 m_wMinPitchPeriod;
    uint16 m_wMaxPitchPeriod;
    uint32 m_dwPitchDataLength;
    uint32 m_dwPitch;
    short m_nPrevPitch;
    short * m_psPitchBfr;
    uint32 m_dwPitchBfrLength;
    uint32 m_dwPitchBlock;
    uint16 m_wMaxFltrLen;
    uint16 m_wMaxFltrHalf;
    uint16 m_wPrevFltrHalf;
    int32 m_lPrevFltrOut;
    int32 m_lCurrSum;
    int32 m_lPrevSum;
    uint32 m_dwFltrPeakIndex;
    int32 m_lFltrPeak;
    int32 m_lFltrPeakThd;
    SFragParms * m_pstFragParmBfr;
    uint32 m_dwFragBfrLength;
    uint32 m_dwFragBfrIndex;
    uint32 m_dwFragBlockLength;
    uint32 m_dwFragCount;
    uint32 m_dwWaveFragStart;
    /***************************** DEBUG ONLY *************************************/
#ifdef DUMP
    FILE * m_hWaveDump;
    FILE * m_hPitchDump;
    FILE * m_hFragDump;
    FILE * m_hFltrDump;
#endif
    /******************************************************************************/
};

#endif
