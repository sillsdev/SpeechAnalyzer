/**************************************************************************\
* MODULE:       PEAKPICK.CPP                                               *
* COPYRIGHT:    SUMMER INSTITUTE OF LINGUISTICS, 2000                      *
* AUTHOR:       ALEC K. EPTING                                             *
*                                                                          *
*                                                                          *
* DESCRIPTION:                                                             *
*                                                                          *
* Class to pick peaks from signal data.                                    *
*                                                                          *
*                                                                          *
* TYPICAL APPLICATION:                                                     *
*                                                                          *
* For speech, to find formant frequencies in an LPC or cepstrally-         *
* smoothed spectrum.                                                       *
*                                                                          *
*                                                                          *
* PUBLIC MEMBER VARIABLES:                                                 *
*                                                                          *
*                                                                          *
* PUBLIC MEMBER FUNCTIONS:                                                 *
*                                                                          *
* Note: all arguments are passed using the C calling convention.           *
*                                                                          *
* Copyright                                                                *
*   Function to retrieve pointer to copyright notice                       *
*   Arguments:    None                                                     *
*   Return value: pointer to null-terminated string containing             *
*                 copyright notice                                         *
*                                                                          *
* Version                                                                  *
*   Function to return version number of class                             *
*   Arguments:     None                                                    *
*   Return value:  Version number in 4-byte floating point format          *
*                                                                          *
* CreateObject                                                             *
*   Function to validate parameters, construct an object, and initialize   *
*   it.                                                                    *
*                                                                          *
*   Arguments:                                                             *
*     ppPeakPicker,       address to contain pointer to peak picker        *
*                         object                                           *
*     nMaxNumPeaks        maximum number of peaks to find                  *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     DONE                      no fatal errors                            *
*     INVALID_NUM_PEAKS         fatal error: invalid number of sections    *
*                                            specified for peak picker     *
*     INVALID_PARM_PTR          fatal error: invalid address specified for *
*                                            pointer to peak picker        *
*     OUT_OF_MEMORY             fatal error: insufficient memory for       *
*                                            buffers                       *
*                                                                          *
* GetPeakLocations                                                         *
*   Function to find peaks                                                 *
*                                                                          *
*   Arguments:                                                             *
*     ppPeakLoc           address to contain pointer to peak locations     *
*                         each of which is an uint32 index into     *
*                         the signal buffer                                *
*     pPeakCount          address to number of peaks found, pointer to     *
*                         uint32                                    *
*     pSignal             pointer to signal buffer                         *
*     dwLength            number of samples in signal buffer               *
*     nMethod             method for finding peaks                         *
*                                                                          *
*   Return value:  pointersigned integer, defined in ERRCODES.H            *
*     INVALID_SIG_PTR     fatal error:  null pointer to signal buffer      *
*     INVALID_SIG_LEN     fatal error:  zero length                        *
*     INVALID_METHOD      fatal error:  undefined method                   *
*                                                                          *
*                                                                          *
* TYPICAL CALLING SEQUENCE:                                                *
*          :                                                               *
*   #include "peakpick.h"                                                  *
*          :                                                               *
*   CPeakPicker *pPeakPicker;                                              *
*   dspError_t Err = CPeakPicker::CreateObject(&pPeakPicker, MAX_NUM_FORMANTS);   *
*   if (Err) return(Code());                                               *
*                                                                          *
*   uint32 *pPeakLoc;                                                       *
*   uint32 dwFormantCount;                                                  *
*   dspError_t Err = pPeakPicker->GetPeakLocations(&pPeakLoc, &dwFormantCount,    *
*             m_LpcParm.Model.pPwrSpectrum, m_wFFTLength/2, PARABOLIC);    *
*   if (Err) return(Code());                                               *
*   for (uint16 i = 1; i <= dwFormantCount; i++)                           *
*        m_LpcParm.Model.Formant[i].FrequencyInHertz =                     *
*                                   (float)(pPeakLoc[i]*dFFTScale);        *
*   for ( ; i <= MAX_NUM_FORMANTS; i++)                                    *
*        m_LpcParm.Model.Formant[i].FrequencyInHz = 0.F;                   *
*                                                                          *
*   delete pPeakPicker;                                                    *
*                                                                          *
*                                                                          *
* TEST DRIVER:                                                             *
*                                                                          *
*                                                                          *
* COMPILER:         Microsoft Visual C++ version 1.52                      *
* COMPILE OPTIONS:  /nologo /f- /G3 /W3 /Zi /AL /YX /Od /D "_DEBUG" /I     *
*                   "c:\msvc\asapdsp" /FR /GA /Fd"SA.PDB" /Fp"SA.PCH"      *
* LINK OPTIONS:     /NOLOGO /LIB:"lafxcwd" /LIB:"oldnames" /LIB:"libw"     *
*                   /LIB:"llibcew" /NOD /PACKC:61440 /STACK:10240          *
*                   /ALIGN:16 /ONERROR:NOEXE /CO /LIB:"commdlg.lib"        *
*                   /LIB:"mmsystem.lib" /LIB:"olecli.lib"                  *
*                   /LIB:"olesvr.lib" /LIB:"shell.lib"                     *
*                                                                          *
* TO DO:            1. Other methods.                                      *
*                                                                          *
* CHANGE HISTORY:                                                          *
*   Date     By             Description                                    *
* 09/16/00  AKE    Initial coding and debug.                               *
\**************************************************************************/
#include "stddsp.h"

#define PEAKPICK_CPP
#define COPYRIGHT_DATE  "2000"
#define VERSION_NUMBER  "0.1"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <malloc.h>
#include "PeakPick.h"


////////////////////////////////////////////////////////////////////////////////////////
// Class function to return copyright notice.                                         //
////////////////////////////////////////////////////////////////////////////////////////
char * CPeakPicker::Copyright(void) {
    static char Notice[] = {"Peak Picker, Version " VERSION_NUMBER "\n"
                            "Copyright (c) " COPYRIGHT_DATE " by Summer Institute of Linguistics. "
                            "All rights reserved.\n"
                           };
    return(Notice);
}

////////////////////////////////////////////////////////////////////////////////////////
// Class function to return version of class.                                         //
////////////////////////////////////////////////////////////////////////////////////////
float CPeakPicker::Version(void) {
    return((float)atof(VERSION_NUMBER));
}


////////////////////////////////////////////////////////////////////////////////////////
// Class function to validate Peak picker parameters and construct object.            //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CPeakPicker::CreateObject(CPeakPicker ** ppPeakPicker, uint32 dwMaxNumBumps) {
    if (!ppPeakPicker) {
        return(Code(INVALID_PARM_PTR));
    }
    *ppPeakPicker = NULL;
    if (!dwMaxNumBumps) {
        return(Code(INVALID_NUM_PEAKS));
    }
    SBumpTableEntry * BumpTable = (SBumpTableEntry *)malloc((int32)dwMaxNumBumps * sizeof(*BumpTable));
    if (!BumpTable) {
        return(Code(OUT_OF_MEMORY));
    }
    CCurveFitting * pCurveFitter;
    dspError_t Err = CCurveFitting::CreateObject(&pCurveFitter, PARABOLIC);
    if (Err) {
        return(Err);
    }
    *ppPeakPicker = new CPeakPicker(BumpTable, dwMaxNumBumps, pCurveFitter);
    if (!*ppPeakPicker) {
        return(Code(OUT_OF_MEMORY));
    }
    return(DONE);
}

////////////////////////////////////////////////////////////////////////////////////////
// Peak picker object constructor.                                                    //
////////////////////////////////////////////////////////////////////////////////////////
CPeakPicker::CPeakPicker(uint32 dwMaxNumBumps) {
    m_dwMaxNumBumps = dwMaxNumBumps;
    if (m_dwMaxNumBumps) {
        m_BumpTable = (SBumpTableEntry *)malloc((int32)dwMaxNumBumps * sizeof(*m_BumpTable));
    }
    CCurveFitting::CreateObject(&m_pParabola, PARABOLIC);
}

CPeakPicker::CPeakPicker(SBumpTableEntry BumpTable[], uint32 dwMaxNumBumps, CCurveFitting * pCurveFitter) {
    m_BumpTable = BumpTable;
    m_dwMaxNumBumps = dwMaxNumBumps;
    m_pParabola = pCurveFitter;
}

dspError_t CPeakPicker::ValidateObject() {
    if (!m_pParabola || !m_BumpTable) {
        return(Code(OUT_OF_MEMORY));
    }
    return(DONE);
}
////////////////////////////////////////////////////////////////////////////////////////
// Peak picker object destructor.                                                     //
////////////////////////////////////////////////////////////////////////////////////////
CPeakPicker::~CPeakPicker() {
    free(m_BumpTable);
    delete m_pParabola;
}


////////////////////////////////////////////////////////////////////////////////////////
// Object function to get peak locations and their amplitudes.                        //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CPeakPicker::GetPeaks(SBumpTableEntry * pPeakTable[], uint32 * pPeakCount,
                                 float * pSignal, uint32 dwSignalLength) {
    if (!pPeakCount) {
        return(Code(INVALID_PARM_PTR));
    }
    *pPeakCount = 0;
    if (!pPeakTable) {
        return(Code(INVALID_PARM_PTR));
    }
    if (!pSignal) {
        return(Code(INVALID_SIG_DATA_PTR));
    }
    if (!dwSignalLength) {
        return(Code(INVALID_SIG_LEN));
    }


    float * pEnd = pSignal + dwSignalLength - 1;
    m_dwBumpCount = 0;
    for (float * y = pSignal+1; y < pEnd; y++) {
        if (y[0]>y[-1] && y[0]>=y[1]) {
            dspError_t Err = m_pParabola->FitCurve(y-1, 3);
            if (Err) {
                return(Err);
            }
            float FirstDerivRoot;
            Err = m_pParabola->FindFirstDerivRoot(&FirstDerivRoot);
            if (!Err) {
                m_BumpTable[m_dwBumpCount].Distance = (float)(y-1 - pSignal) + FirstDerivRoot;
                float PeakValue;
                Err = m_pParabola->GetValue(&PeakValue, FirstDerivRoot);
                if (Err) {
                    return(Err);
                }
                m_BumpTable[m_dwBumpCount].Amplitude = PeakValue;
                if (++m_dwBumpCount == m_dwMaxNumBumps) {
                    break;
                }
            }
        }
    }

    for (uint32 i = m_dwBumpCount; i < m_dwMaxNumBumps; i++) {
        m_BumpTable[i].Distance = (float)UNDEFINED_DATA;
        m_BumpTable[i].Amplitude = (float)UNDEFINED_DATA;
    }

    *pPeakCount = m_dwBumpCount;
    *pPeakTable = m_BumpTable;

    return(DONE);

}

////////////////////////////////////////////////////////////////////////////////////////
// Object function to get bumps in curve.                                             //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CPeakPicker::GetBumps(SBumpTableEntry ** ppBumpTable, uint32 * pBumpCount, float * pSignal, uint32 dwSignalLength) {
    if (!pBumpCount) {
        return(Code(INVALID_PARM_PTR));
    }
    *pBumpCount = 0;
    if (!ppBumpTable) {
        return(Code(INVALID_PARM_PTR));
    }
    if (!pSignal) {
        return(Code(INVALID_SIG_DATA_PTR));
    }
    if (dwSignalLength < 3) {
        return(Code(INVALID_SIG_LEN));
    }



    // If the curve slope starts off negative but is concave (third point lower than first, second higher than
    // first and third), scan through data until a peak is found or the curve turns convex. The former case accounts
    // for an initial peak, while the latter case flushes out data to prevent the following bump detector code
    // from falsely triggering on a concave curve which starts off on a downslope, then turns convex.
    const float * y = pSignal;
    float * pEnd = pSignal + dwSignalLength - 1;


    dspError_t Err = m_pParabola->FitCurve(y, 3);   // fit the next 3 data points to a parabola
    if (Err) {
        return(Err);
    }

    float Slope;
    Err = m_pParabola->CalcFirstDerivative(&Slope, y+1);
    if (Err) {
        return(Err);
    }
    bool bFalling = (Slope < 0.F);   //!!<=0

    float SecondDeriv;
    Err = m_pParabola->CalcSecondDerivative(&SecondDeriv, y+1);
    if (Err) {
        return(Err);
    }
    bool bConcave = (SecondDeriv < -CURVATURE_THD);

    m_dwBumpCount = 0;
    if (bFalling) {
        m_BumpTable[m_dwBumpCount].Distance = 0.F;     // default peak assumed to be at 0
        m_BumpTable[m_dwBumpCount].Amplitude = y[0];
        y++;

        if (bConcave) {
            float FirstDerivRoot;
            dspError_t Err = m_pParabola->FindFirstDerivRoot(&FirstDerivRoot);
            if (!Err) {
                if (FirstDerivRoot > 0.F) {
                    m_BumpTable[m_dwBumpCount].Distance = FirstDerivRoot;   // override default because peak actually to right of 0
                    Err = m_pParabola->GetValue(&m_BumpTable[m_dwBumpCount].Amplitude, FirstDerivRoot);
                    if (Err) {
                        return(Err);
                    }
                }
            }
            while (++y < pEnd) {
                // flush (ignore) data until curve turns convex
                dspError_t Err = m_pParabola->FitCurve(y-1, 3);   // fit the next 3 data points to a parabola
                if (Err) {
                    return(Err);
                }
                Err = m_pParabola->CalcSecondDerivative(&SecondDeriv, y);
                if (Err) {
                    return(Err);
                }
                bool bConvex = (SecondDeriv > -CURVATURE_THD);
                if (bConvex) {
                    break;
                }
            }
        }
        m_dwBumpCount++;
    }


    // Now look for bumps.
    SBumpTableEntry Bump;
    const float * pStart = y;

    for (; m_dwBumpCount < m_dwMaxNumBumps; m_dwBumpCount++) {
        dspError_t RetValue = FindNextBump(&Bump, &y, pEnd);
        if (RetValue == BUMP_NOT_FOUND) {
            break;
        }
        if (RetValue != DONE) {
            return(RetValue);
        }

        m_BumpTable[m_dwBumpCount].Distance = (float)(pStart - pSignal) + Bump.Distance;  // compute offset from beginning of signal
        m_BumpTable[m_dwBumpCount].Amplitude = Bump.Amplitude;
        pStart = y;
    }

    for (uint32 i = m_dwBumpCount; i < m_dwMaxNumBumps; i++) {
        m_BumpTable[i].Distance = (float)UNDEFINED_DATA;
        m_BumpTable[i].Amplitude = (float)UNDEFINED_DATA;
    }

    *pBumpCount = m_dwBumpCount;
    *ppBumpTable = m_BumpTable;

    return(DONE);
}



////////////////////////////////////////////////////////////////////////////////////////
// Object function to find the next peak, searching [start..end].                     //
// Returns NULL if there is no peak to be found.                                      //
// start is set to where it stopped looking so you can call this                      //
// again.                                                                             //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CPeakPicker::FindNextBump(SBumpTableEntry * pBump, const float ** ppStart, const float * const pEnd) {
    ASSERT(pBump != NULL);
    ASSERT(ppStart);
    ASSERT(*ppStart);

    const float * y = *ppStart;
    float  SecondDeriv;
    dspError_t    Err;

    // For each triple of points, go until we first get to a reasonably concave portion.
    while (++y < pEnd) {
        // Check to see if curve is sufficiently concave.
        Err = m_pParabola->FitCurve(y-1, 3);    // fit 3 data points, starting at y-1, to parabola
        if (Err) {
            return(Err);
        }
        Err = m_pParabola->CalcSecondDerivative(&SecondDeriv, y); // compute second derivative of parabola at y
        if (Err) {
            return(Err);
        }
        bool bConcave = (SecondDeriv < -CURVATURE_THD);
        if (bConcave) {
            break;
        }
    }

    // Continue until a peak is found or the curve turns convex.  Record the point whose derivative is
    // closest to 0 along the way.
    pBump->Distance = (float)UNDEFINED_DATA;
    float MinFirstDeriv = FLT_MAX;
    const float * pMinFirstDeriv=NULL;
    bool  bConvex = false;
    while (y < pEnd) {
        // Find flattest spot along curve based on slope (first derivative).
        float FirstDeriv;
        Err = m_pParabola->CalcFirstDerivative(&FirstDeriv, y);
        if (Err) {
            return(Err);
        }

        if (fabs(FirstDeriv) < MinFirstDeriv) {
            MinFirstDeriv = (float)fabs((float)FirstDeriv);
            pMinFirstDeriv = y;
        }

        // Just in case the data points never turn convex, check to see if a peak occurs between y-1 and y
        // in the parabola and save the location and amplitude if it does.
        float FirstDerivRoot;
        dspError_t Err = m_pParabola->FindFirstDerivRoot(&FirstDerivRoot);
        if (!Err) {
            if (FirstDerivRoot >= 0.F && FirstDerivRoot < 1.F) {
                // peak found
                pBump->Distance = (float)(y-1 - *ppStart) + FirstDerivRoot;
                Err = m_pParabola->GetValue(&pBump->Amplitude, FirstDerivRoot);
                if (Err) {
                    return(Err);
                }
            }
        }

        // Fit a parabola to next data point and adjacent points on either side.
        Err = m_pParabola->FitCurve(++y-1, 3);
        if (Err) {
            return(Err);
        }

        // Check to see if curve turns sufficiently convex.
        Err = m_pParabola->CalcSecondDerivative(&SecondDeriv, y);
        if (Err) {
            return(Err);
        }
        bConvex = (SecondDeriv > CURVATURE_THD);
        if (bConvex) {
            break;
        }
    }

    if (bConvex && pBump->Distance == (float)UNDEFINED_DATA) {
        pBump->Distance = (float)(pMinFirstDeriv - *ppStart);
        pBump->Amplitude = *pMinFirstDeriv;
    }
    *ppStart = y;

    if (pBump->Distance == UNDEFINED_DATA) {
        return(BUMP_NOT_FOUND);    //!!Code?
    }
    return(DONE);
}

/***************/

// Pseudo-templates for data structures
#define First float
#define Second uint32
#define tpair pair_float_uint
#include "tpair.h"
#include "tpair.hpp"
#undef tpair
#undef First
#undef Second

#define Type pair_float_uint
#define CTPQueue pq_pair_float_uint
#define CTVector vector_pair_float_uint
#include "tvector.h"
#include "tvector.hpp"
#include "tpq.h"
#include "tpq.hpp"
#undef Type
#undef CTPQueue
#undef CTVector

#define Type uint32
#define CTPQueue pq_uint
#define CTVector vector_uint
#include "tvector.h"
#include "tvector.hpp"
#include "tpq.h"
#include "tpq.hpp"
#undef Type
#undef CTPQueue
#undef CTVector

// Fit a parabola p[0] + p[1] x + p[2] x^2 to the points
// (-1,y[0]), (0,y[1]), (1,y[2]).
static void FitParabola(float p[3], const float y[3]) {
    // y[1] == p[0]
    // y[2] == p[0] + p[1] + p[2]
    // y[0] == p[0] - p[1] + p[2]
    p[0] = y[1];
    p[1] = (y[2]-y[0])/2.0f;
    p[2] = (y[2]+y[0])/2.0f - p[0];
}

// Look for the next peak, searching [first..last).
// Return NULL if there is no peak to be found.
// first is set to where it stopped looking so you can call this
// again.
static const float *
FindNextPeak(const float *& first, const float * const last,
             const float concavityThreshold) {
    assert(first != NULL);
    assert(last != NULL);
    assert(first < last);

    // Handle degenerate cases:

    // If only one or two points, not enough for a peak.
    if (last - first < 2) {
        return NULL;
    }

    // Handle usual cases.
    float bestDerivative = FLT_MAX;
    const float * bestLocation = NULL;
    const float * y;
    float p[3];

    // For each triple of points
    // go until we first get to a reasonably concave portion.
    y = first;

    while (y+2 < last) {
        // Fit a parabola to the points at y, y+1, y+2
        FitParabola(p, y);

        // The second derivative at y+1 is given by 2*p[2]
        if (p[2]*2.f < concavityThreshold) {
            break;
        }

        y++;
    }

    // Now that we're on the peak, go until we fall off.
    // Record the point whose derivative is closest to 0.
    while (y < last) {
        FitParabola(p, y);
        // If we've passed to a region of convexity, stop
        if (p[2]*2.f > concavityThreshold) {
            break;
        }

        // The derivative at y+1 is given by p[1].
        // If we've found a flatter spot, save it.
        if (float(fabs(p[1])) < bestDerivative) {
            bestDerivative = float(fabs(p[1]));
            bestLocation = y+1;
        }

        y++;
    }

    if (y+2 < last) {
        // Means we found a complete peak
        assert(bestDerivative < FLT_MAX);
        first = y+1;
        return bestLocation;
    } else {
        // Ran out of peaks
        first = last;
        return NULL;
    }
}

// Return a pointer to the highest peak in [first..last)
// or NULL if there is no peak.
const float *
FindHighestPeak(const float * first, const float * const last,
                const float concavityThreshold) {
    const float * peak = NULL;
    const float * q = NULL;
    float foo, bar;

    for (;;) {
        // q -> a peak or NULL, first -> stopping place
        q = FindNextPeak(first, last, concavityThreshold);
        if (q == NULL) {
            break;
        }
        if (peak == NULL) {
            peak = q;
        }
        if (*q > *peak) {
            peak = q;
        }
        foo = *peak;
        bar = *q;
    }

    return peak;
}

uint32
FindHighEnergyPeaks(float * const peakTable, const float * const first,
                    const float * const last, const uint32 maxNumPeaks,
                    const float concavityThreshold) {
    // Find all peaks
    // Put them in a priority queue based on energy
    // Extract <= maxNumPeaks from the first priority queue
    // Put the indices in a second priority queue of just indices
    // Extract all of those in reverse order and place in peakTable
    // Return total number of peaks stored
    // Indices are based on distance to first

    pair_float_uint point;
    pq_pair_float_uint powerQ;
    pq_uint indexQ;
    const float * peakLocation = NULL;
    const float * mark = first;
    uint32 numPeaks = 0, i;

    // Find all peaks
    for (;;) {
        // Remember, mark is moved by FindNextPeak
        peakLocation = FindNextPeak(mark, last, concavityThreshold);
        // Stop if no more peaks
        if (peakLocation == NULL) {
            break;
        }
        point.second = peakLocation - first;
        point.first = *peakLocation;
        powerQ.push(point);
    }

    // Take enough tall ones to sort
    while (numPeaks <= maxNumPeaks && !powerQ.empty()) {
        indexQ.push(powerQ.top().second);
        powerQ.pop();
        numPeaks++;
    }

    // Store them in reverse order
    i = numPeaks;
    while (i > 0) {
        i--;
        peakTable[i] = float(indexQ.top());
        indexQ.pop();
    }

    return numPeaks;
}

