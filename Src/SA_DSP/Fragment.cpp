/*****************************************************************************\
* MODULE:       FRAGMENT.CPP                                                  *
* COPYRIGHT:    SUMMER INSTITUTE OF LINGUISTICS, 1997 - 2000                  *
* AUTHOR:       ALEC K. EPTING                                                *
*                                                                             *
*                                                                             *
* DESCRIPTION:                                                                *
*                                                                             *
* Class to divide waveform into fragments of single pitch periods for         *
* voiced speech and short sections for unvoiced speech.                       *
*                                                                             *
* The speech waveform is smoothed with a low pass filter having an adaptive   *
* cutoff frequency based on the local pre-calculated pitch value if voiced    *
* or a default value if unvoiced.  As soon as a zero crossing in the filtered *
* waveform is followed by a peak which exceeds a certain threshold, the       *
* speech data itself is scanned for a zero crossing within the filter window, *
* first to the left of the location corresponding to the zero crossing in the *
* filtered waveform, then if not found, to the right.  For the left scan,     *
* if there is a peak along the way, it must be at least 50% of the filtered   *
* peak.  The pitch period derived from this procedure is then checked to      *
* ensure it is no more than 125% of the local pre-calculated pitch if it is   *
* voiced.  If this criterion is met, the speech waveform is fragmented at     *
* this point.  For a voiced fragment, the peak threshold for the next         *
* zero crossing in the filtered waveform is set to 50% of the current         *
* filtered peak value; otherwise, it is set to zero.                          *
*                                                                             *
*                                                                             *
* PUBLIC MEMBER VARIABLES:                                                    *
*                                                                             *
*                                                                             *
* PUBLIC MEMBER FUNCTIONS:                                                    *
*                                                                             *
* Note: all arguments are passed using the C calling convention.              *
*                                                                             *
* Copyright                                                                   *
*   Function to return character pointer to copyright notice.                 *
*                                                                             *
*   Arguments:                                                                *
*     None                                                                    *
*                                                                             *
* Version                                                                     *
*   Function to return version of class as floating point number.             *
*                                                                             *
*   Arguments:                                                                *
*     None                                                                    *
*                                                                             *
* CreateObject                                                                *
*   Function to construct waveform fragment object.                           *
*                                                                             *
*   Arguments:                                                                *
*     ppoFragment          address to contain pointer to fragment object      *
*     pstFragParm         address to contain pointer to fragment             *
*                          parameters                                         *
*     stSigParm            reference to structure containing signal           *
*                          sampling rate, sample format, pointer to signal    *
*                          buffer, and number of samples in frame             *
*     stPitchParm          reference to structure containing auto pitch       *
*                          parameters                                         *
*                                                                             *
*   Return value:  4-byte signed integer, defined in ERRCODES.H               *
*     DONE                      no fatal errors                               *
*     INVALID_PARM_PTR          fatal error: NULL pointer specified           *
*     UNSUPP_SMP_RATE           fatal error: unsupported sampling rate        *
*                                  specified                                  *
*     UNSUPP_SMP_DATA_FMT       fatal error: unsupported sample data          *
*                                  format specified                           *
*     INVALID_SIG_LEN           fatal error: no signal length in samples      *
*                                  specified                                  *
*     OUT_OF_MEMORY             fatal error: insufficient memory to           *
*                                  create object                              *
*     INVALID_PITCH_CONTOUR_LEN fatal error: no length specified for          *
*                                  pitch contour                              *
*     INVALID_SMP_INTERVAL      fatal error: no sample interval specified     *
*     INVALID_SCALE_FACTOR      fatal error: no scale factor specified        *
*                                                                             *
*                                                                             *
* Fragment                                                                    *
*   Function to divide speech waveform into sections.                         *
*                                                                             *
*                                                                             *
*   Arguments:                                                                *
*     None                                                                    *
*   Return value:                                                             *
*     dwFragCnt            number of waveform fragments                       *
*                                                                             *
*                                                                             *
* TYPICAL CALLING SEQUENCE:                                                   *
*          :                                                                  *
*   #include "Fragment.h"                                                     *
*          :                                                                  *
*   SSigParms stFrameParm;                                                    *
*   stFrameParm.Length = dwFrameSize / wSmpSize;                              *
*   stFrameParm.SmpRate = pDoc->GetSamplesPerSec();                           *
*   if (wSmpSize == 1) stFrameParm.SmpDataFmt = PCM_UBYTE;                    *
*   else stFrameParm.SmpDataFmt = PCM_2SSHORT;                                *
*   stFrameParm.Start = (void *)new char[dwFrameSize];                        *
*          :                                                                  *
*   (load frame buffer)                                                       *
*          :                                                                  *
*   SPitchParms stPitchParm;                                                  *
*          :                                                                  *
*   stPitchParm.psContour = (short *)new short[stPitchParm.dwContourLength];  *
*          :                                                                  *
*   (load pitch contour buffer)                                               *
*          :                                                                  *
*   CFragment *poFragment;                                                    *
*   SFragParms *pstFragParm;                                                  *
*   dspError_t Err = CFragment::CreateObject(&poFragment, , &pstFragParm,     *
*                                     stFrameParm, stPitchParm);              *
*   if (Err) return;                                                          *
*   poFragment->Fragment();                                                   *
*   delete poFragment;                                                        *
*          :                                                                  *
*                                                                             *
*                                                                             *
* COMPILER:         Microsoft Visual C++ version 1.52                         *
* COMPILE OPTIONS:  /nologo /G3 /W3 /Zi /AL /YX /D "_DEBUG"                   *
*                   /I "c:\msvc\dsp" /GA /Fd"SA.PDB" /Fp"SA.PCH"              *
*                                                                             *
* LINK OPTIONS:     /NOLOGO /LIB:"lafxcwd" /LIB:"oldnames" /LIB:"libw"        *
*                   /LIB:"llibcew" /NOD /NOE                                  *
*                   /PACKC:61440 /STACK:10120 /SEG:256 /ALIGN:64              *
*                   /ONERROR:NOEXE /CO /LIB:"commdlg.lib"                     *
*                   /LIB:"mmsystem.lib" /LIB:"shell.lib"                      *
*                                                                             *
* TO DO:                                                                      *
*                                                                             *
*                                                                             *
* CHANGE HISTORY:                                                             *
*   Date     By             Description                                       *
* 09/03/97  AKE     Coverted from experimental C version.                     *
* 11/07/97  AKE     Released as version 1.0 for SA version 1.00 and 1.01.     *
* 11/20/97  AKE     Used DEFAULT_CUTOFF instead of MAX_PITCH to size fragment *
*                   buffer.  Released as version 1.1 for SA version 1.02.     *
* 12/09/97  AKE     Dumped pitch only after checking for valid pitch index.   *
*                   Limited fragment length to value based on DEFAULT_CUTOFF. *
*                   Removed wBandwidth.  Released as version 1.2 for SA       *
*                   version 1.03.                                             *
* 09/23/99  AKE     Revamped code to fragment based on sharpest waveform      *
*                   reversal.                                                 *
* 11/16/99  AKE     For waveform callback, requested buffer load at maximum   *
*                   half filter length before first sample to process.        *
* 06/23/00  AKE     Set default object pointer to NULL.                       *
* 07/28/00  AKE     Set filter width previous value on re-entry to process    *
*                   routine.  Version 2.1.                                    *
* 06/07/01  AKE     Set minimum fragment width when no zero crossing found.   *
*                   Version 2.2.                                              *
\*****************************************************************************/
#include "stddsp.h"

#define FRAGMENT_CPP
#define COPYRIGHT_DATE  "1997-2000"
#define VERSION_NUMBER  "2.2"

//#define DUMP
#ifdef DUMP
#include <stdio.h>
#endif
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include "Fragment.h"

////////////////////////////////////////////////////////////////////////////////////////
// Class function to return copyright notice.                                         //
////////////////////////////////////////////////////////////////////////////////////////
char * CFragment::Copyright(void) {
    static char Notice[] = {"Fragment Version " VERSION_NUMBER "\n"
                            "Copyright (c) " COPYRIGHT_DATE " by Summer Institute of Linguistics. "
                            "All rights reserved.\n"
                           };
    return (Notice);
}

////////////////////////////////////////////////////////////////////////////////////////
// Class function to return version of class.                                         //
////////////////////////////////////////////////////////////////////////////////////////
float CFragment::Version(void) {
    return ((float)atof(VERSION_NUMBER));
}

#undef COPYRIGHT_DATE
#undef VERSION_NUMBER

////////////////////////////////////////////////////////////////////////////////////////
// Class function to construct fragment object if parameters are valid.               //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CFragment::CreateObject(CFragment ** ppoFragment, SFragParms * pstFragParmBfr, uint32 dwFragParmBfrLength,
                                   SSigParms & stSigParm, void * pWaveBfr, uint32 dwWaveBfrLength,
                                   SPitchParms & stPitchParm, short * psPitchBfr, uint32 dwPitchBfrLength) {
// Validate parameters passed.
    if (!ppoFragment) {
        return (Code(INVALID_PARM_PTR));   //address to return object pointer must not be NULL
    }
    *ppoFragment = NULL;

    if (!pstFragParmBfr) {
        return (Code(INVALID_PARM_PTR));   //pointer to fragment buffer must not be NULL
    }
    if (!dwFragParmBfrLength) {
        return (Code(INVALID_PARM));   //fragment buffer length must not be zero
    }
    dspError_t Err = ValidateSignalParms(stSigParm);                   //check signal parameters
    if (Err) {
        return (Err);
    }
    if (!pWaveBfr) {
        return (Code(INVALID_PARM_PTR));
    }
    if (!dwWaveBfrLength) {
        return (Code(INVALID_PARM));
    }

    Err = ValidatePitchParms(stPitchParm);                      //check pitch parameters
    if (Err) {
        return (Err);
    }
    if (!psPitchBfr) {
        return (Code(INVALID_PARM_PTR));
    }
    if (!dwPitchBfrLength) {
        return (Code(INVALID_PARM));
    }


// Construct filter object.
    *ppoFragment = new CFragment(stSigParm, pWaveBfr, dwWaveBfrLength, stPitchParm, psPitchBfr, dwPitchBfrLength,
                                 pstFragParmBfr, dwFragParmBfrLength);
    if (!*ppoFragment) {
        return (Code(OUT_OF_MEMORY));
    }


    return (DONE);
}


////////////////////////////////////////////////////////////////////////////////////////
// Class function to validate signal parameters.                                      //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CFragment::ValidateSignalParms(SSigParms & stSigParm) {
    if (!stSigParm.Length) {
        return (Code(INVALID_SIG_LEN));
    }

    if (stSigParm.SmpRate < 1) {           //ensure signal sampled at valid
        return (Code(UNSUPP_SMP_RATE));   //  Microsoft Windows frequencies.
    }

    if (stSigParm.SmpDataFmt != PCM_UBYTE &&   //sample data format should be unsigned byte PCM or
            stSigParm.SmpDataFmt != PCM_2SSHORT) { //  2's complement signed 16-bit PCM
        return (Code(UNSUPP_SMP_DATA_FMT));
    }

    return (DONE);
}


////////////////////////////////////////////////////////////////////////////////////////
// Class function to validate pitch parameters.                                       //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CFragment::ValidatePitchParms(SPitchParms & stPitchParm) {
    if (!stPitchParm.dwContourLength) {
        return (Code(INVALID_PITCH_CONTOUR_LEN));
    }
    if (!stPitchParm.wSmpIntvl) {
        return (Code(INVALID_SMP_INTERVAL));
    }
    if (!stPitchParm.wScaleFac) {
        return (Code(INVALID_SCALE_FACTOR));
    }
    if (stPitchParm.wCalcRangeMin >= stPitchParm.wCalcRangeMax) {
        return (Code(INVALID_RANGE));
    }

    return (DONE);
}


////////////////////////////////////////////////////////////////////////////////////////
// Fragment object constructor.                                                       //
////////////////////////////////////////////////////////////////////////////////////////
CFragment::CFragment(SSigParms & stSigParm, void * pWaveBfr, uint32 dwWaveBfrLength,
                     SPitchParms & stPitchParm, short * psPitchBfr, uint32 dwPitchBfrLength,
                     SFragParms * pstFragParmBfr, uint32 dwFragParmBfrLength) {
// Update object member variables.
    m_pWaveBfr = pWaveBfr;
    m_dwWaveBfrLength = dwWaveBfrLength;
    m_dwWaveBlock = 0;
    m_dwWaveIndex = 0;
    m_dwSigLength = stSigParm.Length;
    m_wSmpRate = stSigParm.SmpRate;
    m_sbSmpFormat = stSigParm.SmpDataFmt;
    m_psPitchBfr = psPitchBfr;
    m_dwPitchBfrLength = dwPitchBfrLength;
    m_dwPitchBlock = 0;
    m_wPitchScaleFac = stPitchParm.wScaleFac;
    m_wPitchCalcIntvl = stPitchParm.wSmpIntvl;
    m_dwPitchDataLength = stPitchParm.dwContourLength;
    m_pstFragParmBfr = pstFragParmBfr;
    m_dwFragBfrLength = dwFragParmBfrLength;
    m_dwFragBfrIndex = 0;
    m_dwFragCount = 0;
    m_dwWaveFragStart = 0;
    m_dwFltrPeakIndex = 0;
    m_lFltrPeakThd = 0;
    m_lFltrPeak = 0;       //!!needed?
    m_dwWaveCrossing = 0;
    m_nMaxSmpValue = 0;  //!!define
    m_nMinSmpValue = 0;   //!!define
    m_dwMaxRange = 0;
    m_dwMaxRangeCrossing = 0;
    m_bUnvoiced = true;
    m_dwFragBlockLength = 0;


//  Since the output will be smoothed, the filtering operation may be expedited by
//  calculating the moving sum over a downsampled version of the waveform.  This
//  can be implemented later if speed becomes an issue.
//uint16 wFltrIntv = pDocument->m_nSamplesPerSec/nDefaultCutoff;

//  Calculate shortest pitch period allowed.
    m_wMinPitchPeriod = (uint16)((m_wSmpRate + (stPitchParm.wCalcRangeMax>>1)) / stPitchParm.wCalcRangeMax);
//  Calculate length of moving sum window for lowest pitch expected.
    m_wMaxPitchPeriod = (uint16)((m_wSmpRate + (stPitchParm.wCalcRangeMin>>1)) / stPitchParm.wCalcRangeMin);
    m_wMaxFltrLen = (int16)((m_wSmpRate + stPitchParm.wCalcRangeMin)/(2 * stPitchParm.wCalcRangeMin)    //filter cutoff at twice
                            //min pitch expected
                            + 2); //to allow previous sum to be initialized
    //if filter length is maximum

    m_wMaxFltrLen |= 1;                              //force filter window length odd
    //to ensure a sample at the center
    m_wMaxFltrHalf = (uint16)(m_wMaxFltrLen / 2);    //number of samples to either side of center
}

////////////////////////////////////////////////////////////////////////////////////////
// Fragment object destructor.                                                        //
////////////////////////////////////////////////////////////////////////////////////////
CFragment::~CFragment() {

}

////////////////////////////////////////////////////////////////////////////////////////
// Fragment routine.                                                                  //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CFragment::Fragment(void) {
    switch (m_sbSmpFormat) {
    case PCM_UBYTE:
        return (Process((uint8 *)m_pWaveBfr));
    case PCM_2SSHORT:
        return (Process((short *)m_pWaveBfr));
    default:
        return (0);
    }
}


////////////////////////////////////////////////////////////////////////////////////////
// Process routine.                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CFragment::Process(uint8 * pubWaveBfr) {

//  Set moving average filter parameters.
    uint16 wFltrLen;      // number of samples in filter window
    uint16 wFltrHalf;     // number of samples to left (or right) of center sample in filter window
    short nPitch;         // current scaled pitch

    uint8 * pLead,                //pointer to next sample to be included in moving sum
          *pTrail;                //pointer to oldest sample to be removed from moving sum

    if (m_dwWaveIndex == 0) {  //!!what if buffer too small? initialize
        // initialize filter parms
        nPitch = m_psPitchBfr[0];                                                       //set current pitch to smoothed pitch value
        // in pitch buffer
        if (nPitch > 0) {                                                               //if voiced,
            nPitch = (short)((nPitch + (short)(m_wPitchScaleFac>>1))/
                             (short)m_wPitchScaleFac);                                  // scale and round value
            //  to nearest whole number
            wFltrLen = (uint16)((m_wSmpRate + (uint16)nPitch) / (uint16)(nPitch<<1));   // size filter length to
            //  pass fundamental,
            //  suppress even harmonics
            //  and attenuate odd
        } else wFltrLen = (uint16)((m_wSmpRate + (DEFAULT_CUTOFF>>1)) /                 // otherwise, use length
                                       DEFAULT_CUTOFF);                                   // closest to default cutoff

        wFltrLen |= 1;                                                                  // force length odd to ensure sample at center
        wFltrHalf = (uint16)(wFltrLen / 2);


        pLead = pTrail = pubWaveBfr;                                                    // initialize filter edge pointers
        m_lCurrSum = 0;
        for (uint16 i = 0; i <= wFltrHalf; i++) {
            m_lCurrSum += (int32)(*pLead++-128);                                        // initialize filter sum
        }


        m_nPrevPitch = nPitch;
        m_wPrevFltrHalf = wFltrHalf;
        m_dwWaveIndex++;
        /***************************** DEBUG ONLY *************************************/
#ifdef DUMP
        m_hWaveDump = fopen("Waveform.txt", "w");
        m_hPitchDump = fopen("Pitch.txt", "w");
        m_hFragDump = fopen("Fragments.txt", "w");
        m_hFltrDump = fopen("AdaptFtr.txt", "w");

        fprintf(m_hPitchDump, "%d ", __max(0, (m_psPitchBfr[0] + (m_wPitchScaleFac>>1))/m_wPitchScaleFac));
        fprintf(m_hFltrDump, "%ld ", (m_lCurrSum + (int32)wFltrHalf) / wFltrLen);
#endif
        /******************************************************************************/
    } else {
        // restore filter parms
        nPitch = m_nPrevPitch;                                                 //set current pitch to previous value
        if (nPitch > 0) {                                                      //if voiced,
            nPitch = (short)((nPitch + (short)(m_wPitchScaleFac>>1))/
                             (short)m_wPitchScaleFac);                                    // scale and round value
            //  to nearest whole number
            wFltrLen = (uint16)((m_wSmpRate +
                                 (uint16)nPitch) / (uint16)(nPitch<<1));                   // size filter length to
            //  pass fundamental,
            //  suppress even harmonics
            //  and attenuate odd
        } else wFltrLen = (uint16)((m_wSmpRate + (DEFAULT_CUTOFF>>1)) /         //otherwise, use length
                                       DEFAULT_CUTOFF);                                        // closest to default cutoff

        wFltrLen |= 1;                                                          //force length odd to ensure sample at center
        wFltrHalf = (uint16)(wFltrLen / 2);
        pLead = pubWaveBfr + (m_dwWaveIndex + wFltrHalf - m_dwWaveBlock);       //set leading edge pointer
        pTrail = pubWaveBfr + (m_dwWaveIndex - wFltrHalf - 1 - m_dwWaveBlock);  //set trailing edge pointer
    }

    int32 lCurrFltrOut = (m_lCurrSum + (int32)wFltrHalf) / wFltrLen;  //calculate the current filter output


//  Fragment waveform.
    uint16 wWaveFragLen=0;        //length of fragment in samples
    uint32 dwFragMagSum;         //sum of sample magnitudes within waveform fragment

    do {
        // Reload waveform buffer if sample to be requested is out of range.
        if (m_dwWaveIndex + wFltrHalf >= m_dwWaveBlock + m_dwWaveBfrLength) {
            m_dwWaveBlock = __min(m_dwWaveFragStart, m_dwWaveIndex - (uint32)m_wMaxFltrHalf - 1);   //!!what if < 0
            return (WAVE_BUFFER_CALLBACK);
        }

        // Reload pitch buffer if data value requested could be out of range.
        uint32 dwPitchIndex = (m_dwWaveIndex + m_wMaxFltrHalf) / m_wPitchCalcIntvl;
        if (dwPitchIndex  >= m_dwPitchBlock + m_dwPitchBfrLength) {
            m_dwPitchBlock = (m_dwWaveFragStart + 1) / (uint32)m_wPitchCalcIntvl ;     //!!initialize
            return (PITCH_BUFFER_CALLBACK);
        }
        dwPitchIndex = m_dwWaveIndex / (uint32)m_wPitchCalcIntvl;      // map to current waveform same to
        // pitch calculation scale

        // Save previous sum to detect a zero crossing.
        m_lPrevSum = m_lCurrSum;

        uint32 j = dwPitchIndex - m_dwPitchBlock;
        if (dwPitchIndex < m_dwPitchDataLength) {                    //!!needed?
            // Adapt filter to current pitch if pitch has changed.
            nPitch = m_psPitchBfr[j];

            if (nPitch != m_nPrevPitch) {                        //if pitch has changed,
                if (nPitch > 0) {                                // if voiced,
                    // scale and round to nearest whole number
                    nPitch = (short)((nPitch + (short)(m_wPitchScaleFac>>1)) / (short)m_wPitchScaleFac);
                    // round filter length to value which passes fundamental, supresses even harmonics, and attenuates odd harmonics
                    wFltrLen = (uint16)((m_wSmpRate + (uint16)nPitch) / (uint16)(nPitch<<1));
                } else {
                    // otherwise, use length closest to default cutoff
                    wFltrLen = (uint16)((m_wSmpRate + (DEFAULT_CUTOFF>>1)) / DEFAULT_CUTOFF);
                }

                wFltrLen |= 1;                                       // force length odd

                wFltrHalf = (uint16)(wFltrLen / 2);

                if (wFltrHalf > m_wPrevFltrHalf) {                   // if filter length has increased,
                    if (m_dwWaveIndex + wFltrHalf - m_dwWaveBlock >= m_dwWaveBfrLength) { //if it will extend beyond
                        // end of waveform buffer
                        m_dwWaveBlock = __min(m_dwWaveFragStart, m_dwWaveIndex - (uint32)m_wMaxFltrHalf - 1);   //!!what if < 0
                        return (WAVE_BUFFER_CALLBACK);                                   // return for reload
                    }

                    for (uint16 i = 0; i < wFltrHalf - m_wPrevFltrHalf; i++) { //update sum for
                        //expanded filter length
                        if (m_dwWaveIndex  > (uint32)(m_wPrevFltrHalf + 1 + i)) {
                            m_lCurrSum += (int32)(*--pTrail-128);
                        }
                        if (m_dwWaveIndex < m_dwSigLength - (uint32)(m_wPrevFltrHalf + i)) {
                            m_lCurrSum += (int32)(*pLead++-128);
                        }
                    }
                } else                                               // otherwise, filter length has decreased
                    for (uint16 i = 0; i < m_wPrevFltrHalf - wFltrHalf; i++) { //update sum for
                        //reduced length
                        if (m_dwWaveIndex > (uint32)(m_wPrevFltrHalf - i)) {
                            m_lCurrSum -= (int32)(*pTrail++-128);
                        }
                        if (m_dwWaveIndex <= m_dwSigLength - (uint32)(m_wPrevFltrHalf - i)) { //!!<?
                            m_lCurrSum -= (int32)(*--pLead-128);
                        }
                    }

                m_wPrevFltrHalf = wFltrHalf;
                m_nPrevPitch = m_psPitchBfr[j];
            }
            /***************************** DEBUG ONLY *************************************/
#ifdef DUMP
            fprintf(m_hPitchDump, "%d ", __max(0, (m_psPitchBfr[j] + (m_wPitchScaleFac>>1))/m_wPitchScaleFac));
#endif
            /******************************************************************************/
        }

        // Update filter parameters and calculate filter output.
        if (m_dwWaveIndex > wFltrHalf) {
            m_lCurrSum -= (int32)(*pTrail++-128);
        }
        if (m_dwWaveIndex < m_dwSigLength - wFltrHalf) {
            m_lCurrSum += (int32)(*pLead++-128);
        }

        m_lPrevFltrOut = lCurrFltrOut;
        lCurrFltrOut = (m_lCurrSum + (int32)wFltrHalf) / (int32)wFltrLen;

        /***************************** DEBUG ONLY *************************************/
#ifdef DUMP
        fprintf(m_hFltrDump, "%ld ", lCurrFltrOut);
#endif
        /******************************************************************************/

        // If current sample being processed is beyond beginning of the fragment,
        if (m_dwWaveIndex > m_dwWaveFragStart) {    //!!needed?
            short nSmpValue = (short)(pubWaveBfr[m_dwWaveIndex-m_dwWaveBlock]-128);
            if (nSmpValue >= 0) {
                //  If negative to positive zero crossing, save location.
                if (m_dwWaveIndex > 0 &&  //!!replace this
                        (short)(pubWaveBfr[m_dwWaveIndex-1-m_dwWaveBlock]-128) < 0) {
                    m_dwWaveCrossing = m_dwWaveIndex;
                }

                //  Record maximum sample value.
                if (nSmpValue > m_nMaxSmpValue) {
                    m_nMaxSmpValue = nSmpValue;
                }
            }

            else {
                //  If positive to negative zero crossing, compute previous range of
                //  negative to positive wave reversal.
                if (m_dwWaveIndex > 0 &&  //!!replace this
                        (short)(pubWaveBfr[m_dwWaveIndex-1-m_dwWaveBlock]-128) >= 0) {
                    uint32 dwSmpRange = (uint32)((int32)m_nMaxSmpValue - (int32)m_nMinSmpValue);
                    if (dwSmpRange > m_dwMaxRange) {
                        //!!need to ensure transition through zero is smooth
                        m_dwMaxRange = dwSmpRange;
                        m_dwMaxRangeCrossing = m_dwWaveCrossing;
                    }

                    //  Reset max and min values.
                    m_nMinSmpValue = 0;
                    m_nMaxSmpValue = 0;

                }

                //  Record minimum sample value.
                if (nSmpValue <= m_nMinSmpValue) {
                    m_nMinSmpValue = nSmpValue;
                }
            }


            if (lCurrFltrOut >= 0) {
                // Reset peak location and value at negative to positive zero crossing in the filtered
                // waveform.
                if (m_lPrevFltrOut < 0) {
                    m_dwFltrPeakIndex = 0;
                    m_lFltrPeak = 0;
                }

                // Record location and value of first positive peak if it meets or exceeds the peak threshold.
                if (m_lPrevFltrOut >= lCurrFltrOut && m_lPrevFltrOut >= m_lFltrPeakThd) {
                    m_dwFltrPeakIndex = m_dwWaveIndex - 1;
                    m_lFltrPeak = m_lPrevFltrOut;
                }
            }

            // Check if peak following a zero crossing in filtered waveform is above a certain threshold
            // and the waveform is negative.  Scanning for the beginning of the fragment under these
            // conditions ensures a strong peak has occurred and the full range from maximum negative to
            // positive, indicating a sharp reversal, has been found in the waveform.
            if (m_dwFltrPeakIndex && m_lPrevFltrOut >= 0 && lCurrFltrOut < 0) {

                //  Update value and location of sharpest reversal.
                uint32 dwSmpRange = (uint32)((int32)m_nMaxSmpValue - (int32)m_nMinSmpValue);
                if (dwSmpRange > m_dwMaxRange) {
                    m_dwMaxRange = dwSmpRange;
                    m_dwMaxRangeCrossing = m_dwWaveCrossing;
                }

                //  Reset max and min sample values.
                m_nMinSmpValue = 0;
                m_nMaxSmpValue = 0;

                if (m_dwMaxRangeCrossing) {
                    m_dwWaveCrossing = m_dwMaxRangeCrossing;

                    //  Search for a zero crossing which corresponds to the smooth pitch period starting
                    //  with the zero crossing which occurred during the sharpest reversal of the waveform.
                    do {
                        //  Get pre-computed pitch estimate
                        short nPitchUpperBound;
                        short nSmoothPitch = m_psPitchBfr[m_dwWaveCrossing/m_wPitchCalcIntvl - m_dwPitchBlock];

                        if (nSmoothPitch > 0 && !m_bUnvoiced)     //if smooth pitch value available
                            //  and waveform is not
                            //  transitioning from unvoiced
                            //  fragment
                        {
                            nPitchUpperBound = (short)((nSmoothPitch + (m_wPitchScaleFac>>1)) /
                                                       m_wPitchScaleFac);      //round pitch to nearest
                            //  whole number
                            nPitchUpperBound = (short)(nPitchUpperBound + (nPitchUpperBound>>2));  //set upper bound to
                            //  25% above nominal
                        } else {
                            nPitchUpperBound = DEFAULT_CUTOFF;    //otherwise limit
                        }
                        //  to default filter cutoff

                        //  Mark fragment if calculated pitch of fragment after rounding
                        //  is less than or equal to 125% of nominal smooth pitch at zero crossing.
                        wWaveFragLen = (uint16)(m_dwWaveCrossing - m_dwWaveFragStart);                                        //!!Len to Length
                        short nFragPitch = (short)((m_wSmpRate + (wWaveFragLen>>1))/wWaveFragLen);
                        if (nFragPitch <= nPitchUpperBound) {
                            m_lFltrPeakThd = 0;

                            if (nSmoothPitch > 0) {
                                m_bUnvoiced = false;
                            } else {
                                m_bUnvoiced = true;
                            }

                            m_pstFragParmBfr[m_dwFragBfrIndex].dwOffset = m_dwWaveFragStart;                                                  //!!buffering
                            m_pstFragParmBfr[m_dwFragBfrIndex].wLength = wWaveFragLen;
                            m_pstFragParmBfr[m_dwFragBfrIndex].nPitch = nSmoothPitch;
                            dwFragMagSum = 0;
                            for (uint16 i = 0; i < wWaveFragLen; i++) {
                                dwFragMagSum += (uint32)abs((short)(pubWaveBfr[m_dwWaveFragStart+(uint32)i-m_dwWaveBlock]-128));
                                /***************************** DEBUG ONLY *************************************/
#ifdef DUMP
                                fprintf(m_hWaveDump, "%d ", (short)(pubWaveBfr[m_dwWaveFragStart+i-m_dwWaveBlock]-128));
                                if (i == 0) {
                                    fprintf(m_hFragDump, "32767 ");
                                } else {
                                    fprintf(m_hFragDump, "0 ");
                                }
#endif
                                /******************************************************************************/

                            }
                            m_pstFragParmBfr[m_dwFragBfrIndex++].wAvgMag = (uint16)((uint16)(dwFragMagSum +
                                    (uint32)(wWaveFragLen>>1)) /
                                    wWaveFragLen);    //!!precedence

                            m_dwWaveFragStart += (uint32)wWaveFragLen;

                            break;
                        }

                        //  The fragment ending at that zero crossing produced too high a pitch value, so
                        //  continue to the next zero crossing.
                        for (m_dwWaveCrossing++; m_dwWaveCrossing <= m_dwFltrPeakIndex; m_dwWaveCrossing++)
                            if ((short)(pubWaveBfr[m_dwWaveCrossing-1-m_dwWaveBlock]-128) < 0 &&
                                    (short)(pubWaveBfr[m_dwWaveCrossing-m_dwWaveBlock]-128) >= 0) {
                                break;
                            }

                    } while (m_dwWaveCrossing <= m_dwFltrPeakIndex);

                    m_dwMaxRange = 0;
                    m_dwMaxRangeCrossing = 0;
                    m_dwWaveCrossing = 0;
                }


                //!!what if no crossings found, m_dwWaveCrossing = 0
            }


            // If no zero crossings occurred in filtered waveform, truncate fragment
            // at nearest zero crossing to the left, if possible.  Otherwise truncate
            // at current position.
            else if (m_dwWaveIndex >= m_dwWaveFragStart + (uint32)m_wMaxPitchPeriod) {
                m_bUnvoiced = true;
                m_dwWaveCrossing = 0;
                uint16 wMinFragLength;
                if (nPitch > 0) {
                    wMinFragLength = m_wMinPitchPeriod;
                } else {
                    wMinFragLength = wFltrHalf;
                }

                for (short k = 0;
                        (m_dwWaveIndex-k) > m_dwWaveFragStart + wMinFragLength;
                        k++)   //search left
                    if ((short)(pubWaveBfr[m_dwWaveIndex-k-1-m_dwWaveBlock]-128) < 0 &&
                            (short)(pubWaveBfr[m_dwWaveIndex-k-m_dwWaveBlock]-128) >= 0) {
                        //zero crossing found
                        m_dwWaveCrossing = m_dwWaveIndex - k;
                        wWaveFragLen = (uint16)(m_dwWaveCrossing - m_dwWaveFragStart);
                        break;
                    }

                if (!m_dwWaveCrossing) {
                    wWaveFragLen = (uint16)(m_dwWaveIndex - m_dwWaveFragStart);    //chop here since no zero crossing found
                }

                m_pstFragParmBfr[m_dwFragBfrIndex].dwOffset = m_dwWaveFragStart;
                m_pstFragParmBfr[m_dwFragBfrIndex].wLength = wWaveFragLen;
                m_pstFragParmBfr[m_dwFragBfrIndex].nPitch = m_psPitchBfr[(m_dwWaveFragStart+wWaveFragLen-1)/
                        m_wPitchCalcIntvl-m_dwPitchBlock];
                dwFragMagSum = 0;
                for (uint16 i = 0; i < wWaveFragLen; i++) {
                    dwFragMagSum += (uint32)abs((short)(pubWaveBfr[m_dwWaveFragStart+i-m_dwWaveBlock]-128));
                    /***************************** DEBUG ONLY *************************************/
#ifdef DUMP
                    fprintf(m_hWaveDump, "%d ", (short)(pubWaveBfr[m_dwWaveFragStart+i-m_dwWaveBlock]-128));
                    if (i == 0) {
                        fprintf(m_hFragDump, "32767 ");
                    } else {
                        fprintf(m_hFragDump, "0 ");
                    }
#endif
                    /******************************************************************************/
                }
                m_pstFragParmBfr[m_dwFragBfrIndex++].wAvgMag = (uint16)((dwFragMagSum +
                        (uint32)(wWaveFragLen>>1)) /
                        wWaveFragLen);   //!!precedence
                m_dwWaveFragStart += (uint32)wWaveFragLen;
                m_dwFltrPeakIndex = 0;
                m_dwMaxRange = 0;
                m_dwMaxRangeCrossing = 0;
                m_dwWaveCrossing = 0;
            }

        }

        //  If voiced, set peak threshold
        if (m_psPitchBfr[j] > 0) {
            //  to 50% of first filtered peak in the pitch period.
            if (!m_lFltrPeakThd && m_dwFltrPeakIndex) {
                m_lFltrPeakThd = m_lFltrPeak>>1;
            }
        }
        //  Otherwise, do not use a threshold.
        else {
            m_lFltrPeakThd = 0;
        }

        // Check if fragment parameter buffer is full.
        m_dwWaveIndex++;
        if (m_dwFragBfrIndex == m_dwFragBfrLength) {
            m_dwFragBlockLength = m_dwFragBfrIndex;
            m_dwFragCount += m_dwFragBlockLength;
            if (m_dwWaveIndex == m_dwSigLength) {
                return (DONE);
            } else {
                m_dwFragBfrIndex = 0;
                return (FRAG_BUFFER_FULL);
            }
        }

    } while (m_dwWaveIndex < m_dwSigLength);


//  Append remaining fragment from last zero crossing.
    wWaveFragLen = (uint16)(m_dwSigLength - m_dwWaveFragStart);
    if (wWaveFragLen > 1) {
        m_pstFragParmBfr[m_dwFragBfrIndex].dwOffset = m_dwWaveFragStart;
        m_pstFragParmBfr[m_dwFragBfrIndex].wLength = wWaveFragLen;
        m_pstFragParmBfr[m_dwFragBfrIndex].nPitch = m_psPitchBfr[m_dwPitchDataLength-1-m_dwPitchBlock];
        dwFragMagSum = 0;
        for (uint16 i = 0; i < wWaveFragLen; i++) {
            dwFragMagSum += (uint32)abs((short)(pubWaveBfr[m_dwWaveFragStart+i-m_dwWaveBlock]-128));
            /***************************** DEBUG ONLY *************************************/
#ifdef DUMP
            fprintf(m_hWaveDump, "%d ", (short)(pubWaveBfr[m_dwWaveFragStart+i-m_dwWaveBlock]-128));
            if (i == 0) {
                fprintf(m_hFragDump, "32767 ");
            } else {
                fprintf(m_hFragDump, "0 ");
            }
#endif
            /******************************************************************************/

        }
        m_pstFragParmBfr[m_dwFragBfrIndex++].wAvgMag = (uint16)((dwFragMagSum + (uint32)(wWaveFragLen>>1)) /
                wWaveFragLen);
        m_dwFragBlockLength = m_dwFragBfrIndex;
        m_dwFragCount += m_dwFragBlockLength;
    }

    /***************************** DEBUG ONLY *************************************/
#ifdef DUMP
    fclose(m_hWaveDump);
    fclose(m_hPitchDump);
    fclose(m_hFragDump);
    fclose(m_hFltrDump);
#endif
    /******************************************************************************/

    return (DONE);
}

dspError_t CFragment::Process(short * psWaveBfr) {

//  Set moving average filter parameters.
    uint16 wFltrLen;      // number of samples in filter window
    uint16 wFltrHalf;     // number of samples to left (or right) of center sample in filter window
    short nPitch;         // current scaled pitch

    short * pLead,                //pointer to next sample to be included in moving sum
          *pTrail;                //pointer to oldest sample to be removed from moving sum

    if (m_dwWaveIndex == 0) {  //!!what if buffer too small? initialize
        // initialize filter parms
        nPitch = m_psPitchBfr[0];                                              //set current pitch to smoothed pitch value
        // in pitch buffer
        if (nPitch > 0) {                                                      //if voiced,
            nPitch = (short)((nPitch + (short)(m_wPitchScaleFac>>1))/
                             (short)m_wPitchScaleFac);                               // scale and round value
            //  to nearest whole number
            wFltrLen = (uint16)((m_wSmpRate +
                                 (uint16)nPitch) / (uint16)(nPitch<<1));                   // size filter length to
            //  pass fundamental,
            //  suppress even harmonics
            //  and attenuate odd
        } else wFltrLen = (uint16)((m_wSmpRate + (DEFAULT_CUTOFF>>1)) /        //otherwise, use length
                                       DEFAULT_CUTOFF);                              // closest to default cutoff

        wFltrLen |= 1;                                                         //force length odd to ensure sample at center
        wFltrHalf = (uint16)(wFltrLen / 2);


        pLead = pTrail = psWaveBfr;                                            //initialize filter edge pointers
        m_lCurrSum = 0;
        for (uint16 i = 0; i <= wFltrHalf; i++) {
            m_lCurrSum += (int32)*pLead++;    //initialize filter sum
        }


        m_nPrevPitch = nPitch;
        m_wPrevFltrHalf = wFltrHalf;
        m_dwWaveIndex++;
        /***************************** DEBUG ONLY *************************************/
#ifdef DUMP
        m_hWaveDump = fopen("Waveform.txt", "w");
        m_hPitchDump = fopen("Pitch.txt", "w");
        m_hFragDump = fopen("Fragments.txt", "w");
        m_hFltrDump = fopen("AdaptFtr.txt", "w");

        fprintf(m_hPitchDump, "%d ", __max(0, (m_psPitchBfr[0] + (m_wPitchScaleFac>>1))/m_wPitchScaleFac));
        fprintf(m_hFltrDump, "%ld ", (m_lCurrSum + (int32)wFltrHalf) / wFltrLen);
#endif
        /******************************************************************************/
    } else {
        // restore filter parms
        nPitch = m_nPrevPitch;                                                 //set current pitch to previous value
        if (nPitch > 0) {                                                      //if voiced,
            nPitch = (short)((nPitch + (short)(m_wPitchScaleFac>>1))/
                             (short)m_wPitchScaleFac);                                   // scale and round value
            //  to nearest whole number
            wFltrLen = (uint16)((m_wSmpRate +
                                 (uint16)nPitch) / (uint16)(nPitch<<1));                   // size filter length to
            //  pass fundamental,
            //  suppress even harmonics
            //  and attenuate odd
        } else wFltrLen = (uint16)((m_wSmpRate + (DEFAULT_CUTOFF>>1)) /         //otherwise, use length
                                       DEFAULT_CUTOFF);                               // closest to default cutoff

        wFltrLen |= 1;                                                          //force length odd to ensure sample at center
        wFltrHalf = (uint16)(wFltrLen / 2);
        pLead = psWaveBfr + (m_dwWaveIndex + wFltrHalf - m_dwWaveBlock);        //set leading edge pointer
        pTrail = psWaveBfr + (m_dwWaveIndex - wFltrHalf - 1 - m_dwWaveBlock);   //set trailing edge pointer
    }

    int32 lCurrFltrOut = (m_lCurrSum + (int32)wFltrHalf) / wFltrLen;  //calculate the current filter output


//  Fragment waveform.
    uint16 wWaveFragLen=0;        //length of fragment in samples
    uint32 dwFragMagSum=0;         //sum of sample magnitudes within waveform fragment

    do {
        // Reload waveform buffer if sample to be requested is out of range.
        if (m_dwWaveIndex + wFltrHalf >= m_dwWaveBlock + m_dwWaveBfrLength) {
            m_dwWaveBlock = __min(m_dwWaveFragStart, m_dwWaveIndex - (uint32)m_wMaxFltrHalf - 1);   //!!what if < 0
            return (WAVE_BUFFER_CALLBACK);
        }

        // Reload pitch buffer if data value requested could be out of range.
        uint32 dwPitchIndex = (m_dwWaveIndex + m_wMaxFltrHalf) / m_wPitchCalcIntvl;
        if (dwPitchIndex  >= m_dwPitchBlock + m_dwPitchBfrLength) {
            m_dwPitchBlock = (m_dwWaveFragStart + 1) / (uint32)m_wPitchCalcIntvl ;     //!!initialize
            return (PITCH_BUFFER_CALLBACK);
        }
        dwPitchIndex = m_dwWaveIndex / (uint32)m_wPitchCalcIntvl;     // map to current waveform same to
        // pitch calculation scale

        // Save previous sum to detect a zero crossing.
        m_lPrevSum = m_lCurrSum;

        uint32 j = dwPitchIndex - m_dwPitchBlock;
        if (dwPitchIndex < m_dwPitchDataLength) {                     //!!needed?
            // Adapt filter to current pitch if pitch has changed.
            nPitch = m_psPitchBfr[j];

            if (nPitch != m_nPrevPitch) {                            //if pitch has changed,
                if (nPitch > 0) {                                    // if voiced,
                    nPitch = (short)((nPitch + (short)(m_wPitchScaleFac>>1)) /
                                     (short)m_wPitchScaleFac);           //  scale and round to nearest
                    //  whole number
                    wFltrLen = (uint16)((m_wSmpRate + (uint16)nPitch) /
                                        (uint16)(nPitch<<1));        //  round filter length to value
                    //  which passes fundamental,
                    //  supresses even harmonics, and
                    //  attenuates odd harmonics
                } else
                    wFltrLen = (uint16)((m_wSmpRate + (DEFAULT_CUTOFF>>1)) /
                                        DEFAULT_CUTOFF);              //  otherwise, use length closest
                //  to default cutoff

                wFltrLen |= 1;                                       // force length odd

                wFltrHalf = (short)(wFltrLen / 2);

                if (wFltrHalf > m_wPrevFltrHalf) {                   // if filter length has increased,
                    if (m_dwWaveIndex + wFltrHalf - m_dwWaveBlock >= m_dwWaveBfrLength) { //if it will extend beyond
                        // end of waveform buffer
                        m_dwWaveBlock = __min(m_dwWaveFragStart, m_dwWaveIndex - (uint32)m_wMaxFltrHalf - 1);   //!!what if < 0
                        return (WAVE_BUFFER_CALLBACK);                                   // return for reload
                    }

                    for (uint16 i = 0; i < wFltrHalf - m_wPrevFltrHalf; i++) { //update sum for
                        //expanded filter length
                        if (m_dwWaveIndex  > (uint32)(m_wPrevFltrHalf + 1 + i)) {
                            m_lCurrSum += (int32)*--pTrail;
                        }
                        if (m_dwWaveIndex < m_dwSigLength - (uint32)(m_wPrevFltrHalf + i)) {
                            m_lCurrSum += (int32)*pLead++;
                        }
                    }
                } else                                               // otherwise, filter length has decreased
                    for (uint16 i = 0; i < m_wPrevFltrHalf - wFltrHalf; i++) { //update sum for
                        //reduced length
                        if (m_dwWaveIndex > (uint32)(m_wPrevFltrHalf - i)) {
                            m_lCurrSum -= (int32)*pTrail++;
                        }
                        if (m_dwWaveIndex <= m_dwSigLength - (uint32)(m_wPrevFltrHalf - i)) { //!!<?
                            m_lCurrSum -= (int32)*--pLead;
                        }
                    }

                m_wPrevFltrHalf = wFltrHalf;
                m_nPrevPitch = m_psPitchBfr[j];
            }
            /***************************** DEBUG ONLY *************************************/
#ifdef DUMP
            fprintf(m_hPitchDump, "%d ", __max(0, (m_psPitchBfr[j] + (m_wPitchScaleFac>>1))/m_wPitchScaleFac));
#endif
            /******************************************************************************/
        }

        // Update filter parameters and calculate filter output.
        if (m_dwWaveIndex > wFltrHalf) {
            m_lCurrSum -= (int32)*pTrail++;
        }
        if (m_dwWaveIndex < m_dwSigLength - wFltrHalf) {
            m_lCurrSum += (int32)*pLead++;
        }

        m_lPrevFltrOut = lCurrFltrOut;
        lCurrFltrOut = (m_lCurrSum + (int32)wFltrHalf) / (int32)wFltrLen;

        /***************************** DEBUG ONLY *************************************/
#ifdef DUMP
        fprintf(m_hFltrDump, "%ld ", lCurrFltrOut);
#endif
        /******************************************************************************/
#ifdef DUMP
#define SAMPLE  8236
        if (m_dwWaveIndex == SAMPLE) {
            bool breakpoint = true;
        }
#endif

        // If current sample being processed is beyond beginning of the fragment,
        if (m_dwWaveIndex > m_dwWaveFragStart) {    //!!needed?
            short nSmpValue = psWaveBfr[m_dwWaveIndex-m_dwWaveBlock];
            if (nSmpValue >= 0) {
                //  If negative to positive zero crossing, save location.
                if (m_dwWaveIndex > 0 &&  //!!replace this
                        psWaveBfr[m_dwWaveIndex-1-m_dwWaveBlock] < 0) {
                    m_dwWaveCrossing = m_dwWaveIndex;
                }

                //  Record maximum sample value.
                if (nSmpValue > m_nMaxSmpValue) {
                    m_nMaxSmpValue = nSmpValue;
                }
            }

            else {
                //  If positive to negative zero crossing, compute previous range of
                //  negative to positive wave reversal.
                if (m_dwWaveIndex > 0 &&  //!!replace this
                        psWaveBfr[m_dwWaveIndex-1-m_dwWaveBlock] >= 0) {
                    uint32 dwSmpRange = (uint32)((int32)m_nMaxSmpValue - (int32)m_nMinSmpValue);
                    if (dwSmpRange > m_dwMaxRange) {
                        //!!need to ensure transition through zero is smooth
                        m_dwMaxRange = dwSmpRange;
                        m_dwMaxRangeCrossing = m_dwWaveCrossing;
                    }

                    //  Reset max and min values.
                    m_nMinSmpValue = 0;
                    m_nMaxSmpValue = 0;

                }

                //  Record minimum sample value.
                if (nSmpValue <= m_nMinSmpValue) {
                    m_nMinSmpValue = nSmpValue;
                }
            }


            if (lCurrFltrOut >= 0) {
                // Reset peak location and value at negative to positive zero crossing in the filtered
                // waveform.
                if (m_lPrevFltrOut < 0) {
                    m_dwFltrPeakIndex = 0;
                    m_lFltrPeak = 0;
                }

                // Record location and value of first positive peak if it meets or exceeds the peak threshold.
                if (m_lPrevFltrOut >= lCurrFltrOut && m_lPrevFltrOut >= m_lFltrPeakThd) {
                    m_dwFltrPeakIndex = m_dwWaveIndex - 1;
                    m_lFltrPeak = m_lPrevFltrOut;
                }
            }

            // Check if peak following a zero crossing in filtered waveform is above a certain threshold
            // and the waveform is negative.  Scanning for the beginning of the fragment under these
            // conditions ensures a strong peak has occurred and the full range from maximum negative to
            // positive, indicating a sharp reversal, has been found in the waveform.
            if (m_dwFltrPeakIndex && m_lPrevFltrOut >= 0 && lCurrFltrOut < 0) {

                //  Update value and location of sharpest reversal.
                uint32 dwSmpRange = (uint32)((int32)m_nMaxSmpValue - (int32)m_nMinSmpValue);
                if (dwSmpRange > m_dwMaxRange) {
                    m_dwMaxRange = dwSmpRange;
                    m_dwMaxRangeCrossing = m_dwWaveCrossing;
                }

                //  Reset max and min sample values.
                m_nMinSmpValue = 0;
                m_nMaxSmpValue = 0;

                if (m_dwMaxRangeCrossing) {
                    m_dwWaveCrossing = m_dwMaxRangeCrossing;

                    //  Search for a zero crossing which corresponds to the smooth pitch period starting
                    //  with the zero crossing which occurred during the sharpest reversal of the waveform.
                    do {
                        //  Get pre-computed pitch estimate
                        short nPitchUpperBound;
                        short nSmoothPitch = m_psPitchBfr[m_dwWaveCrossing/m_wPitchCalcIntvl - m_dwPitchBlock];

                        if (nSmoothPitch > 0 && !m_bUnvoiced)     //if smooth pitch value available
                            //  and waveform is not
                            //  transitioning from unvoiced
                            //  fragment
                        {
                            nPitchUpperBound = (short)((nSmoothPitch + (m_wPitchScaleFac>>1)) /
                                                       m_wPitchScaleFac);      //round pitch to nearest
                            //  whole number
                            nPitchUpperBound = (short)(nPitchUpperBound + (nPitchUpperBound>>2));  //set upper bound to
                            //  25% above nominal
                        } else {
                            nPitchUpperBound = DEFAULT_CUTOFF;    //otherwise limit
                        }
                        //  to default filter cutoff

                        //  Mark fragment if calculated pitch of fragment after rounding
                        //  is less than or equal to 125% of nominal smooth pitch at zero crossing.
                        wWaveFragLen = (uint16)(m_dwWaveCrossing - m_dwWaveFragStart);                                        //!!Len to Length
                        short nFragPitch = (short)((m_wSmpRate + (wWaveFragLen>>1))/wWaveFragLen);
                        if (nFragPitch <= nPitchUpperBound) {
                            m_lFltrPeakThd = 0;

                            if (nSmoothPitch > 0) {
                                m_bUnvoiced = false;
                            } else {
                                m_bUnvoiced = true;
                            }

                            m_pstFragParmBfr[m_dwFragBfrIndex].dwOffset = m_dwWaveFragStart;                                                  //!!buffering
                            m_pstFragParmBfr[m_dwFragBfrIndex].wLength = wWaveFragLen;
                            m_pstFragParmBfr[m_dwFragBfrIndex].nPitch = nSmoothPitch;
                            dwFragMagSum = 0;
                            for (uint16 i = 0; i < wWaveFragLen; i++) {
                                dwFragMagSum += (uint32)abs(psWaveBfr[m_dwWaveFragStart+(uint32)i-m_dwWaveBlock]);
                                /***************************** DEBUG ONLY *************************************/
#ifdef DUMP
                                fprintf(m_hWaveDump, "%d ", psWaveBfr[m_dwWaveFragStart+i-m_dwWaveBlock]);
                                if (i == 0) {
                                    fprintf(m_hFragDump, "32767 ");
                                } else {
                                    fprintf(m_hFragDump, "0 ");
                                }
#endif
                                /******************************************************************************/

                            }
                            m_pstFragParmBfr[m_dwFragBfrIndex++].wAvgMag = (uint16)((dwFragMagSum +
                                    (uint32)(wWaveFragLen>>1)) /
                                    wWaveFragLen);    //!!precedence

                            m_dwWaveFragStart += (uint32)wWaveFragLen;

                            break;
                        }

                        //  The fragment ending at that zero crossing produced too high a pitch value, so
                        //  continue to the next zero crossing.
                        for (m_dwWaveCrossing++; m_dwWaveCrossing <= m_dwFltrPeakIndex; m_dwWaveCrossing++)
                            if (psWaveBfr[m_dwWaveCrossing-1-m_dwWaveBlock] < 0 &&
                                    psWaveBfr[m_dwWaveCrossing-m_dwWaveBlock] >= 0) {
                                break;
                            }

                    } while (m_dwWaveCrossing <= m_dwFltrPeakIndex);

                    m_dwMaxRange = 0;
                    m_dwMaxRangeCrossing = 0;
                    m_dwWaveCrossing = 0;
                }


                //!!what if no crossings found, m_dwWaveCrossing = 0
            }


            // If no zero crossings occurred in filtered waveform, truncate fragment
            // at nearest zero crossing to the left, if possible.  Otherwise truncate
            // at current position.
            else if (m_dwWaveIndex >= m_dwWaveFragStart + (uint32)m_wMaxPitchPeriod) {
                m_bUnvoiced = true;
                m_dwWaveCrossing = 0;
                uint16 wMinFragLength;
                if (nPitch > 0) {
                    wMinFragLength = m_wMinPitchPeriod;
                } else {
                    wMinFragLength = wFltrHalf;
                }

                for (short k = 0;
                        (m_dwWaveIndex-k) > m_dwWaveFragStart + wMinFragLength;
                        k++)   //search left
                    if (psWaveBfr[m_dwWaveIndex-k-1-m_dwWaveBlock] < 0 &&
                            psWaveBfr[m_dwWaveIndex-k-m_dwWaveBlock] >= 0) {
                        //zero crossing found
                        m_dwWaveCrossing = m_dwWaveIndex - k;
                        wWaveFragLen = (uint16)(m_dwWaveCrossing - m_dwWaveFragStart);
                        break;
                    }

                if (!m_dwWaveCrossing) {
                    wWaveFragLen = (uint16)(m_dwWaveIndex - m_dwWaveFragStart);    //chop here since no zero crossing found
                }

                m_pstFragParmBfr[m_dwFragBfrIndex].dwOffset = m_dwWaveFragStart;
                m_pstFragParmBfr[m_dwFragBfrIndex].wLength = wWaveFragLen;
                m_pstFragParmBfr[m_dwFragBfrIndex].nPitch = m_psPitchBfr[(m_dwWaveFragStart+wWaveFragLen-1)/
                        m_wPitchCalcIntvl-m_dwPitchBlock];
                dwFragMagSum = 0;
                for (uint16 i = 0; i < wWaveFragLen; i++) {
                    dwFragMagSum += (uint32)abs(psWaveBfr[m_dwWaveFragStart+i-m_dwWaveBlock]);
                    /***************************** DEBUG ONLY *************************************/
#ifdef DUMP
                    fprintf(m_hWaveDump, "%d ", psWaveBfr[m_dwWaveFragStart+i-m_dwWaveBlock]);
                    if (i == 0) {
                        fprintf(m_hFragDump, "32767 ");
                    } else {
                        fprintf(m_hFragDump, "0 ");
                    }
#endif
                    /******************************************************************************/
                }
                m_pstFragParmBfr[m_dwFragBfrIndex++].wAvgMag = (uint16)((dwFragMagSum +
                        (uint32)(wWaveFragLen>>1)) /
                        wWaveFragLen);   //!!precedence
                m_dwWaveFragStart += (uint32)wWaveFragLen;
                m_dwFltrPeakIndex = 0;
                m_dwMaxRange = 0;
                m_dwMaxRangeCrossing = 0;
                m_dwWaveCrossing = 0;
            }

        }

        //  If voiced, set peak threshold
        if (m_psPitchBfr[j] > 0) {
            //  to 50% of first filtered peak in the pitch period.
            if (!m_lFltrPeakThd && m_dwFltrPeakIndex) {
                m_lFltrPeakThd = m_lFltrPeak>>1;
            }
        }
        //  Otherwise, do not use a threshold.
        else {
            m_lFltrPeakThd = 0;
        }

        // Check if fragment parameter buffer is full.
        m_dwWaveIndex++;
        if (m_dwFragBfrIndex == m_dwFragBfrLength) {
            m_dwFragBlockLength = m_dwFragBfrIndex;
            m_dwFragCount += m_dwFragBlockLength;
            if (m_dwWaveIndex == m_dwSigLength) {
                return (DONE);
            } else {
                m_dwFragBfrIndex = 0;
                return (FRAG_BUFFER_FULL);
            }
        }

    } while (m_dwWaveIndex < m_dwSigLength);


//  Append remaining fragment from last zero crossing.
    wWaveFragLen = (uint16)(m_dwSigLength - m_dwWaveFragStart);
    if (wWaveFragLen > 1) {
        m_pstFragParmBfr[m_dwFragBfrIndex].dwOffset = m_dwWaveFragStart;
        m_pstFragParmBfr[m_dwFragBfrIndex].wLength = wWaveFragLen;
        m_pstFragParmBfr[m_dwFragBfrIndex].nPitch = m_psPitchBfr[m_dwPitchDataLength-1-m_dwPitchBlock];
        dwFragMagSum = 0;
        for (uint16 i = 0; i < wWaveFragLen; i++) {
            dwFragMagSum += (uint32)abs(psWaveBfr[m_dwWaveFragStart+i-m_dwWaveBlock]);
            /***************************** DEBUG ONLY *************************************/
#ifdef DUMP
            fprintf(m_hWaveDump, "%d ", psWaveBfr[m_dwWaveFragStart+i-m_dwWaveBlock]);
            if (i == 0) {
                fprintf(m_hFragDump, "32767 ");
            } else {
                fprintf(m_hFragDump, "0 ");
            }
#endif
            /******************************************************************************/

        }
        m_pstFragParmBfr[m_dwFragBfrIndex++].wAvgMag = (uint16)((dwFragMagSum + (uint32)(wWaveFragLen>>1)) / wWaveFragLen);
        m_dwFragBlockLength = m_dwFragBfrIndex;
        m_dwFragCount += m_dwFragBlockLength;
    }

    /***************************** DEBUG ONLY *************************************/
#ifdef DUMP
    fclose(m_hWaveDump);
    fclose(m_hPitchDump);
    fclose(m_hFragDump);
    fclose(m_hFltrDump);
#endif
    /******************************************************************************/

    return (DONE);
}

uint32 CFragment::GetWaveBlockIndex() {
    return m_dwWaveBlock;
}
uint32 CFragment::GetPitchBlockIndex() {
    return m_dwPitchBlock;
}
uint32 CFragment::GetFragmentBlockLength() {
    return m_dwFragBlockLength;
}
uint32 CFragment::GetFragmentCount() {
    return m_dwFragCount;
}

