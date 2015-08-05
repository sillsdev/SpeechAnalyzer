#ifndef WAVERESAMPLER_H
#define WAVERESAMPLER_H

#include <stdio.h>
#include <tchar.h>
#include <mmsystem.h>
#include "errors.h"
#include "mmreg.h"
#include "msacm.h"
#include <vector>

using std::vector;

#pragma comment( lib, "winmm" )
#pragma comment( lib, "msacm32.lib")

__interface IProgressUpdate {
    void SetProgress(int value);
    int GetProgress();
};

/**
* the DSP logic for this class was borrowed from AudioCon.dll
*/
class CWaveResampler {

public:
    enum ECONVERT {
        EC_SUCCESS      = 0,
        EC_NOWAVE       =-1,
        EC_NOFMT        =-2,
        EC_FMTERROR     =-3,
        EC_NODATA       =-4,
        EC_READFAIL     =-5,
        EC_WRONGFORMAT  =-6,
        EC_CONVERTFORMATFAIL=-7,
        EC_WRITEFAIL    =-8,
        EC_NOTSUPPORTED =-9,
        EC_SOFTWARE     =-10,
        EC_TOOLARGE     =-11,
        EC_USERABORT    =-12
    };

    ECONVERT Resample(LPCTSTR infilename, LPCTSTR outfilename, IProgressUpdate & update);
    ECONVERT Resample(LPCTSTR infilename, LPCTSTR outfilename, DWORD targetSamplesPerSec, IProgressUpdate & update);

private:
    double Limit(double val);
    long Round(double val);
    long ConvBitSize(unsigned long in, int bps);
    unsigned long GCD(unsigned long m, unsigned long n);
    unsigned long LCM(unsigned long m, unsigned long n);
    double BessI0(float x);

    // filter processing functions
    void CalculateCoefficients(DWORD nSamplesPerSec,
                               WORD wBitsPerSample,
                               double *& coeffs,
                               size_t & coeffsLen);
    void Func(size_t bufferLen,
              double * buffer,
              size_t coeffsLen,
              double * coeffs,
              size_t upSmpFactor,
              size_t dwnSmpFactor,
              vector<double> & datal,
              IProgressUpdate & progressUpdater);
};

#endif
