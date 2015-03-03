#ifndef PEAKPICK_H
#define PEAKPICK_H

#include "dspTypes.h"
#include "Error.h"
#include "CurveFit.h"

/***********************************************************************************/
/*               Peak Picking Methods                                              */
/***********************************************************************************/
enum EPEAK_METHODS {TREND_FOLLOWING};

/***********************************************************************************/
/*               Trend Following Technique                                         */
/***********************************************************************************/
enum EPEAK_FOLLOWING {UNKNOWN_TREND=-1, RISING_TREND=0, LEVEL_TREND=1, FALLING_TREND=2};


/***********************************************************************************/
/*               Parabolic Fitting Technique                                       */
/***********************************************************************************/
#define CURVATURE_THD  0.F

struct SBumpTableEntry {
    float Distance;
    float Amplitude;
};

class CPeakPicker {
public:
    static char * Copyright(void);
    static float Version(void);
    static dspError_t CreateObject(CPeakPicker ** ppPeakPicker, uint32 dwMaxNumBumps);
    dspError_t GetPeaks(SBumpTableEntry * BumpTable[], uint32 * pBumpCount,
                        float * pSignal, uint32 dwSignalLength);
    dspError_t GetBumps(SBumpTableEntry ** ppBumpTable, uint32 * pBumpCount,
                        float * pSignal, uint32 dwSignalLength);
    ~CPeakPicker();
protected:
    CPeakPicker(uint32 dwMaxNumBumps);
    dspError_t ValidateObject();
    CPeakPicker(SBumpTableEntry BumpTable[], uint32 dwMaxNumBumps, CCurveFitting * pCurveFitter);
    dspError_t FindNextBump(SBumpTableEntry * pBump, const float ** ppStart, const float * const pEnd);
    CCurveFitting * m_pParabola;
    SBumpTableEntry * m_BumpTable;
    uint32 m_dwBumpCount;
    uint32 m_dwMaxNumBumps;
};

// Return a pointer to the highest peak in [first..last)
// or NULL if there is no peak.
const float * FindHighestPeak(const float * first, const float * const last,
                              const float concavityThreshold);

// Look for the highest energy peaks and store their locations in peakTable.
// maxNumPeaks limits the maximum number of peaks to find, [first..last)
// is the data range to look through, and concavityThreshold is the parameter
// used by the peak picking algorithm to determine where bumps are.
// Return the number of peaks found and stored, which is <= maxNumPeaks.
// The peak indices are stored as floating point rather than integer values
// to make it easier to convert them to frequencies.
uint32
FindHighEnergyPeaks(float * const peakTable, const float * const first,
                    const float * const last, const uint32 maxNumPeaks,
                    const float concavityThreshold);
#endif // PEAKPICK_H