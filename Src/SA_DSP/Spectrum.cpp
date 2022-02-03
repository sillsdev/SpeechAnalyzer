/**************************************************************************\
* MODULE:       SPECTRUM.CPP                                               *
* COPYRIGHT:    SUMMER INSTITUTE OF LINGUISTICS, 1997                      *
* AUTHOR:       ALEC K. EPTING                                             *
*                                                                          *
*                                                                          *
* DESCRIPTION:                                                             *
*                                                                          *
* Class to produce both raw and smoothed power spectra from 8 or 16-bit    *
* sample data.                                                             *
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
*   Function to return character pointer to copyright notice.              *
*                                                                          *
*   Arguments:                                                             *
*     None                                                                 *
*                                                                          *
* Version                                                                  *
*   Function to return version of class as floating point number.          *
*                                                                          *
*   Arguments:                                                             *
*     None                                                                 *
*                                                                          *
* CreateObject                                                             *
*   Function to construct spectrum object.                                 *
*                                                                          *
*   Arguments:                                                             *
*     ppoSpectrum,        address to contain pointer to spectrum object    *
*     stSpectSetting,     reference to structure containing specifications *
*                         for spectrum calculation, such as selection of   *
*                         intensity scale (linear or dB), number of        *
*                         spectral points to calculate, smoothing          *
*                         frequency, peak resolution factor, and DSP       *
*                         analysis window switch.  Smoothing frequency of  *
*                         AUTO_SET signifies frequency is to be set        *
*                         automatically.                                   *
*     stFrameParm         reference to structure containing signal         *
*                         sampling rate, sample format, pointer to frame   *
*                         buffer, and number of samples in frame           *
*     FFTLength           number of points in FFT, unsigned short,         *
*                         defaults to MAX_FFT_LENGTH                       *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     DONE                      no fatal errors                            *
*     INVALID_PARM_PTR          fatal error: invalid pointer specified     *
*     INVALID_SPECT_LEN         fatal error: invalid number of spectral    *
*                                  points specified                        *
*     INVALID_SCALE             fatal error: invalid scale specified       *
*     INVALID_FREQ              fatal error: invalid smoothing frequency   *
*                                  specified                               *
*     INVALID_RESOLUTION        fatal error: invalid peak resolution       *
*                                  factor specified                        *
*     UNSUPP_SMP_RATE           fatal error: unsupported sampling rate     *
*                                  specified                               *
*     UNSUPP_SMP_DATA_FMT       fatal error: unsupported sample data       *
*                                  format specified                        *
*     INVALID_SIG_LEN           fatal error: invalid signal length         *
*                                  in samples specified                    *
*     OUT_OF_MEMORY             fatal error: insufficient memory available *
*     INVALID_FFT_LENGTH        fatal error: invalid FFT length specified, *
*                                            must be a power of 2 less     *
*                                            than or equal to              *
*                                            MAX_FFT_LENGTH                *
*                                                                          *
*                                                                          *
* GetSpectParms                                                            *
*   Function to retrieve spectral parameters.                              *
*                                                                          *
*   Arguments:                                                             *
*     None                                                                 *
*                                                                          *
*   Return value:  structure containing number of spectral values          *
*                  calculated, the frequency scale, pointers to the raw    *
*                  and smoothed spectral data, the number of formants, and *
*                  a pointer to the cepstrally-derived formant locations   *
*                  (in Hz) and their log magnitude value (in dB).          *
*                  Formant 0 corresponds to the cepstrally-derived pitch.  *
*                  The spectral data is stored in 4-byte floating point    *
*                  format.  The structure also contains the reference      *
*                  value used in the dB calculation, in 4-byte floating    *
*                  point format.                                           *
*                                                                          *
*                                                                          *
* TYPICAL CALLING SEQUENCE:                                                *
*          :                                                               *
*   #include "spectrum.h"                                                  *
*          :                                                               *
*   SSigParms stFrameParm;                                                 *
*   stFrameParm.Length = dwFrameSize / wSmpSize;                           *
*   stFrameParm.SmpRate = pModel->GetSamplesPerSec();                        *
*   if (wSmpSize == 1) stFrameParm.SmpDataFmt = PCM_UBYTE;                 *
*   else stFrameParm.SmpDataFmt = PCM_2SSHORT;                             *
*   stFrameParm.Start = (void *) new char[dwFrameSize];                    *
*          :                                                               *
*   (load frame buffer)                                                    *
*          :                                                               *
*   SSpectrumSettings stSpectSetting;                                      *
*   stSpectSetting.wLength = nScreenWidth;                                 *
*   stSpectSetting.nScaleSelect = DB;                                      *
*   stSpectSetting.fSmoothFreq = 500.F;                                    *
*   stSpectSetting.fFFTRadius = 1.F;                                       *
*   stSpectSetting.bDSPWinSw = ON;                                         *
*                                                                          *
*   dspError_t Err = CSpectrum::CreateObject(&poSpectrum, stSpectSetting,  *
*                                     stFrameParm);                        *
*   if (Err) return;                                                       *
*                                                                          *
*   SSpectrumParms stSpectParm = poSpectrum->GetSpectParms();                 *
*          :                                                               *
*   delete poSpectrum;                                                     *
*                                                                          *
*                                                                          *
* COMPILER:         Microsoft Visual C++ version 1.52                      *
* COMPILE OPTIONS:  /nologo /G3 /W3 /Zi /AL /YX /D "_DEBUG"                *
*                   /I "c:\msvc\dsp" /GA /Fd"SA.PDB" /Fp"SA.PCH"           *
* LINK OPTIONS:     /NOLOGO /LIB:"lafxcwd" /LIB:"oldnames" /LIB:"libw"     *
*                   /LIB:"llibcew" /LIB:"dsp\fft86e" /NOD /NOE             *
*                   /PACKC:61440 /STACK:10120 /SEG:256 /ALIGN:64           *
*                   /ONERROR:NOEXE /CO /LIB:"commdlg.lib"                  *
*                   /LIB:"mmsystem.lib" /LIB:"shell.lib"                   *
*                                                                          *
* TO DO:            1. Fix FFT routine to return correctly scaled values   *
*                      (half what they are now and with 1/N in the         *
*                      forward transform).                                 *
*                   2. Scale frame data to SPL levels.                     *
*                   3. Pre-emphasize?                                      *
*                   4. Return spectral parameters by reference.            *
*                                                                          *
*                                                                          *
* CHANGE HISTORY:                                                          *
*   Date     By             Description                                    *
* 03/31/97  AKE    Initial coding.                                         *
* 05/21/97  AKE    Debugged and running.                                   *
* 05/26/97  AKE    Added provision for preprocessing without DSP window.   *
* 06/25/97  AKE    Added code to calculate pitch and locate formants.      *
* 07/31/97  AKE    Added support for 44.1 and 48 kHz sampling rates.       *
*                  Adjusted spectral dB power to SPL levels for speech.    *
* 08/01/97  AKE    Fixed uint8 preprocessing.                              *
* 06/23/00  AKE    Set default object pointer to NULL.                     *
* 07/13/00  AKE    Added dB ref to spectral parms. Added optional FFT      *
*                  length specification in constructor.  Version 0.2.      *
* 08/12/00  AKE    Fixed formant frequency calculation errors in peak      *
*                  picker for cepstrally smoothed spectrum.  Version 0.3.  *
* 08/26/00  AKE    Changed from MIN_LOG_PWR to MIN_LOG_MAG.  Version 0.4.  *
* 09/06/01  AKE    Reduced power reading at 0 and Nyquist/2 by half and    *
*                  doubled power at other frequencies.  Discovered FFT     *
*                  returns values twice what they should be.  Version 0.5. *
\**************************************************************************/
#include "stddsp.h"

#define SPECTRUM_CPP
#define COPYRIGHT_DATE  "2000"
#define VERSION_NUMBER  "0.5"

#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <math.h>
#include "MathX.h"
#include "Spectrum.h"
#include "FFT.h"
#include "PeakPick.h"
#include <limits>

////////////////////////////////////////////////////////////////////////////////////////
// Class function to return copyright notice.                                         //
////////////////////////////////////////////////////////////////////////////////////////
char * CSpectrum::Copyright(void) {
    static char Notice[] = {"Spectrum Version " VERSION_NUMBER "\n"
                            "Copyright (c) " COPYRIGHT_DATE " by Summer Institute of Linguistics. "
                            "All rights reserved.\n"
                           };
    return(Notice);
}

////////////////////////////////////////////////////////////////////////////////////////
// Class function to return version of class.                                         //
////////////////////////////////////////////////////////////////////////////////////////
float CSpectrum::Version(void) {
    return((float)atof(VERSION_NUMBER));
}

#undef COPYRIGHT_DATE
#undef VERSION_NUMBER

////////////////////////////////////////////////////////////////////////////////////////
// Class function to construct spectrum object if parameters are valid.               //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CSpectrum::CreateObject(CSpectrum ** ppoSpectrum, SSpectrumSettings & stSpectSetting, SSigParms & stFrameParm, uint16 wFFTLength) {
// Validate requested spectrum settings and signal parameters.
    if (!ppoSpectrum) {
        return(Code(INVALID_PARM_PTR));    //address of pointer to spectrum object
    }
    //  must not be NULL
    *ppoSpectrum = NULL;
    dspError_t Err = ValidateSettings(stSpectSetting);        //check settings
    if (Err) {
        return(Err);
    }
    Err = ValidateSignalParms(stFrameParm);            //check signal parameters
    if (Err) {
        return(Err);
    }
    if (stSpectSetting.fSmoothFreq >= (float)stFrameParm.SmpRate/2.F) {
        return(Code(INVALID_FREQ));
    }
    if (wFFTLength > MAX_FFT_LENGTH ||
            !IsSingleBitOn(wFFTLength)) { //must be power of 2 for FFT
        return(Code(INVALID_FFT_LENGTH));
    }
    if (stSpectSetting.wLength == 0 || stSpectSetting.wLength > wFFTLength/2) {
        return(Code(INVALID_SPECT_LEN));    //requested length must not be zero or greater than
    }
    //FFT length which may cause divide by zero

// Allocate memory to contain raw and smoothed power spectra.
    float * pfRawSpectrum = new float[stSpectSetting.wLength];
    if (!pfRawSpectrum) {
        return(Code(OUT_OF_MEMORY));
    }
    float * pfSmoothSpectrum = new float[stSpectSetting.wLength];
    if (!pfSmoothSpectrum) {
        delete [] pfRawSpectrum;
        return(Code(OUT_OF_MEMORY));
    }

// Allocate memory to contain FFT values.
    float * pfFFTBuffer = new float[wFFTLength];
    if (!pfFFTBuffer) {
        delete [] pfRawSpectrum;
        delete [] pfSmoothSpectrum;
        return(Code(OUT_OF_MEMORY));
    }

// Calculate window for frame data.
    CDspWin oDSPWindow = CDspWin::FromLength((uint16)stFrameParm.Length, stFrameParm.SmpRate, stSpectSetting.nWindowType);

// Construct spectrum object.
    CSpectrum * poSpectrum = new CSpectrum(stSpectSetting, stFrameParm, oDSPWindow,
                                           wFFTLength, pfFFTBuffer, pfRawSpectrum, pfSmoothSpectrum);
    if (!poSpectrum) {
        delete [] pfRawSpectrum;
        delete [] pfSmoothSpectrum;
        delete [] pfFFTBuffer;
        return(Code(OUT_OF_MEMORY));
    }

    *ppoSpectrum = poSpectrum;

    return(DONE);
}


////////////////////////////////////////////////////////////////////////////////////////
// Class function to validate signal parameters.                                      //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CSpectrum::ValidateSignalParms(SSigParms & stFrame) {
    if (!stFrame.Start) {
        return(Code(INVALID_PARM_PTR));
    }
    if (!stFrame.Length) {
        return(Code(INVALID_SIG_LEN));
    }

    if (stFrame.SmpRate < 1) {
        return(Code(UNSUPP_SMP_RATE));
    }

    if (stFrame.SmpDataFmt != PCM_UBYTE &&   //sample data format should be unsigned byte PCM, or
            stFrame.SmpDataFmt != PCM_2SBYTE &&  //  2's complement signed byte PCM, or
            stFrame.SmpDataFmt != PCM_2SSHORT) { //  2's complement signed 16-bit PCM
        return(Code(UNSUPP_SMP_DATA_FMT));
    }

    return(DONE);
}

////////////////////////////////////////////////////////////////////////////////////////
// Class function to validate requested spectrum settings.                            //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CSpectrum::ValidateSettings(SSpectrumSettings & stSpectSetting) {
    if (stSpectSetting.nScaleSelect != LINEAR &&
            stSpectSetting.nScaleSelect != DB) {
        return(Code(INVALID_SCALE));    //requested scale must be linear or decibel
    }

    if (stSpectSetting.fSmoothFreq < 0.F) {
        return(Code(INVALID_FREQ));    //smoothing freq must not be negative
    }

    if (stSpectSetting.fFFTRadius <= 0.F ||
            stSpectSetting.fFFTRadius > 1.F) {
        return(Code(INVALID_RESOLUTION));    //requested peak resolution must be > 0 and <= 1
    }

    return(DONE);
}

////////////////////////////////////////////////////////////////////////////////////////
// Spectrum object constructor.                                                       //
////////////////////////////////////////////////////////////////////////////////////////
CSpectrum::CSpectrum(SSpectrumSettings & stSpectSetting,
                     SSigParms & stFrameParm, CDspWin & oDSPWindow,
                     uint16 wFFTLength, float * pfFFTBuffer,
                     float * pfRawSpectrum, float * pfSmoothSpectrum) : m_oDSPWindow(oDSPWindow) {
// Update object member variables.
    m_bReplicate = false;
    m_pFrameStart = stFrameParm.Start;
    m_dwFrameLength = stFrameParm.Length;
    m_wSmpRate = stFrameParm.SmpRate;
    m_sbSmpFormat = stFrameParm.SmpDataFmt;
    m_wSpectLength = stSpectSetting.wLength;
    m_nScaleSelect = stSpectSetting.nScaleSelect;
    m_fSmoothFreq = stSpectSetting.fSmoothFreq;
    m_fSpectCalcRadius = stSpectSetting.fFFTRadius;
    m_wFFTLength = wFFTLength;
    m_pfFFTBuffer = pfFFTBuffer;
    m_pfRawSpectrum = pfRawSpectrum;
    m_pfSmoothSpectrum = pfSmoothSpectrum;
    m_wNumFormants = MAX_NUM_FORMANTS;
    m_wMaxPitchPeriod = (uint16)((float)m_wSmpRate/(float)MIN_PITCH + 0.5F);
    m_wMinPitchPeriod = (uint16)((float)m_wSmpRate/(float)MAX_PITCH + 0.5F);
    m_dMaxRawPower = -1.;
// Since the recording level is not controlled by SA, SPL cannot be determined exactly.  Thus, the 0 dB reference
// level is instead taken to be the top of the green range for the recorder VU LED meter.  This is
// 70.7% of full scale for the raw waveform.  Now the power of sine wave of amplitude A is A*A/2.  If A is
// 70.7% of full scale (i.e. sqrt(2)*F/2, where F is the fullscale value), then the power is F*F/4.
// Now the power from the FFT is N*N*A*A/2 = N*N*F*F/4, when N is the length of the FFT.  So the reference
// decibel level is
//
//                                      dB    =  10*log(N*N*F*F/4)
//                                        ref
//
// 3 dB is added to account for the FFT returning values twice what they should be
//!!Fix this
    m_fPwrDbRef[1] = (float)(10.*log10((double)m_wFFTLength*(double)m_wFFTLength*128.*128./4.)) + 3.F;
    m_fPwrDbRef[2] = (float)(10.*log10((double)m_wFFTLength*(double)m_wFFTLength*32768.*32768./4.)) + 3.F;

    GetPwrSpectrum();
}


////////////////////////////////////////////////////////////////////////////////////////
// Spectrum object destructor.                                                        //
////////////////////////////////////////////////////////////////////////////////////////
CSpectrum::~CSpectrum() {
// Free allocated memory
    delete [] m_pfRawSpectrum;
    delete [] m_pfSmoothSpectrum;
    delete [] m_pfFFTBuffer;
}

////////////////////////////////////////////////////////////////////////////////////////
// Public object function to retrieve spectral parameters.                            //
////////////////////////////////////////////////////////////////////////////////////////
SSpectrumParms CSpectrum::GetSpectParms(void) {
    SSpectrumParms stSpectParm;

    stSpectParm.wSpectLength = m_wSpectLength;
    stSpectParm.fFreqScale = (float)((double)m_wSmpRate / (double)m_wSpectLength);
    stSpectParm.pfRawSpectrum = m_pfRawSpectrum;
    stSpectParm.pfSmoothSpectrum = m_pfSmoothSpectrum;
    stSpectParm.wNumFormants = m_wNumFormants;
    stSpectParm.pstFormant = m_pstFormant;
    stSpectParm.fdBRef = m_fPwrDbRef[abs(m_sbSmpFormat)];

    return stSpectParm;
}


////////////////////////////////////////////////////////////////////////////////////////
// Private object function to calculate raw and smoothed spectra.                     //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CSpectrum::GetPwrSpectrum(void) {
    dspError_t Err;

    switch (m_sbSmpFormat) {
    case PCM_UBYTE:
        Err = Preprocess((uint8 *)m_pFrameStart);
        if (Err) {
            return(Err);
        }
        break;
    case PCM_2SBYTE:
        Err = Preprocess((int8 *)m_pFrameStart);
        if (Err) {
            return(Err);
        }
        break;
    case PCM_2SSHORT:
        Err = Preprocess((short *)m_pFrameStart);
        if (Err) {
            return(Err);
        }
        break;
    }
    Err = CalcPwrFFT();
    if (Err) {
        return(Err);
    }

    return(DONE);
}

////////////////////////////////////////////////////////////////////////////////////////
// Private object functions to prepare data for spectral calculation.                 //
////////////////////////////////////////////////////////////////////////////////////////

// Experimentally determined weight to keep energy in windowed data similar to energy in
// unwindowed.  Windowed response varied from 0.5 to 1.0 with an average response of
// 0.74 and a SD of 0.14.  (tested with Hann window)
static double WindowWeight = 1./0.74;

dspError_t CSpectrum::Preprocess(uint8 * pubFrame) {
    uint16 i;
    uint32 j;
    const float * WData = m_oDSPWindow.WindowFloat();
    uint32 dwWinLength = m_oDSPWindow.Length();

    double dFftScale = WindowWeight*double(m_wFFTLength)/CDspWin::CalcEquivalentLength(m_dwFrameLength, CDspWin::kHanning, m_oDSPWindow.Type());
    for (i = 0; i < m_wFFTLength; i++) {
        m_pfFFTBuffer[i] = 0.F;
        for (j = (uint32)i; (j < m_dwFrameLength) && (j < dwWinLength); j += (uint32)m_wFFTLength) {
            m_pfFFTBuffer[i] += WData[j] * (float)((int8)(pubFrame[j] - 128) * dFftScale);
        }
        // pre-alias to prevent overlapping bands
    }

    return(DONE);
}

dspError_t CSpectrum::Preprocess(int8 * psbFrame) {
    uint16 i;
    uint32 j;
    const float * WData = m_oDSPWindow.WindowFloat();
    uint32 dwWinLength = m_oDSPWindow.Length();

    double dFftScale = WindowWeight*double(m_wFFTLength)/m_dwFrameLength;
    for (i = 0; i < m_wFFTLength; i++) {
        m_pfFFTBuffer[i] = 0.F;
        for (j = (uint32)i; (j < m_dwFrameLength) && (j < dwWinLength); j += (uint32)m_wFFTLength) {
            m_pfFFTBuffer[i] += WData[j] * (float)(psbFrame[j] * dFftScale);
        }
        // pre-alias to prevent overlapping bands
    }

    return(DONE);
}

dspError_t CSpectrum::Preprocess(short * psFrame) {
    uint16 i;
    uint32 j;
    const float * WData = m_oDSPWindow.WindowFloat();
    uint32 dwWinLength = m_oDSPWindow.Length();

    double dFftScale = WindowWeight*double(m_wFFTLength)/m_dwFrameLength;
    for (i = 0; i < m_wFFTLength; i++) {
        m_pfFFTBuffer[i] = 0.F;
        for (j = (uint32)i; j < m_dwFrameLength, j < dwWinLength; j += (uint32)m_wFFTLength) {
            m_pfFFTBuffer[i] += WData[j] * (float)(psFrame[j] * dFftScale);
        }
        // pre-alias to prevent overlapping bands
    }

    return(DONE);
}


////////////////////////////////////////////////////////////////////////////////////////
// Private object function to compute spectra from pre-processed sample data.         //
// The power must be calculated over positive and negative frequencies.  Since a      //
// real signal has a frequency spectrum with even symmetry, the power within a        //
// frequency range may be simply doubled (except at 0 Hz and half the Nyquist         //
// frequency).  For the particular FFT used here, the values returned are double what //
// they should be, so the power is 4 times the theoretical value.  Half of this is    //
// compensated for by the need to accumulate power over positive and negative         //
// frequencies; the other half is taken care of in the dB reference values.           //
// !!Fix FFT so that it returns proper values, also include 1/N scale factor in the   //
// forward transform.                                                                 //
////////////////////////////////////////////////////////////////////////////////////////
#ifdef FFT_DUMP
#include <stdio.h>
#endif


dspError_t CSpectrum::CalcPwrFFT(void) {
    // Reference location where complex spectral values will be returned.
    SComplexRectFloat * pfSpectCoeff = (SComplexRectFloat *)m_pfFFTBuffer;

    // Calculate FFT on real data.
    rfft2f(m_pfFFTBuffer,m_wFFTLength,FORWARD);

    // Compute the log magnitude of the spectrum.  Average power for each frequency point
    // across adjacent bands.
    uint16 i, j = 0;
    double dSpectScale = (double)m_wSmpRate/(2.*(double)(m_wSpectLength-1));
    double dFFTScale = (double)m_wSmpRate/(double)m_wFFTLength;

    uint16 wBandStart = 0;
    double    dSpectBand = 0.5*dSpectScale;
    double    dFFTFreq = dFFTScale;

    double    dPower = pfSpectCoeff[0].real*pfSpectCoeff[0].real;
    double    dAccumPower = 0.5*dPower;

#ifdef FFT_DUMP
    FILE * FFTDump = fopen("fftdump.txt", "w");
    fprintf(FFTDump, "%f %f\n", pfSpectCoeff[0].real, 0.);
#endif

    double dTotalRawEnergy = 0;
    pfSpectCoeff[0].real = (dPower == 0.) ? MIN_LOG_MAG : (float)(0.5*log10(dPower));

    if (dFFTFreq > dSpectBand) {
        m_pfRawSpectrum[j++] = (float)dAccumPower;
        dTotalRawEnergy += dAccumPower;
        dAccumPower = 0.;
        dSpectBand += dSpectScale;
    }
    for (i = 1; i < m_wFFTLength/2; i++) {
        dPower = pfSpectCoeff[i].real*pfSpectCoeff[i].real +
                 pfSpectCoeff[i].imag*pfSpectCoeff[i].imag;
#ifdef FFT_DUMP
        fprintf(FFTDump, "%f %f\n", pfSpectCoeff[i].real, pfSpectCoeff[i].imag);
#endif
        dAccumPower += dPower;
        dFFTFreq += dFFTScale;
        if (dFFTFreq > dSpectBand) {
            m_pfRawSpectrum[j++] = (float)(dAccumPower / (double)(i - wBandStart));
            dTotalRawEnergy += dAccumPower;
            dAccumPower = 0.;
            wBandStart = i;
            dSpectBand += dSpectScale;
        }

        pfSpectCoeff[i].real = (dPower == 0.) ? MIN_LOG_MAG : (float)(0.5*log10(dPower));

        pfSpectCoeff[i].imag = 0.F;
    }

    dPower = pfSpectCoeff[0].imag*pfSpectCoeff[0].imag;

#ifdef FFT_DUMP
    fprintf(FFTDump, "%f %f\n", 0., pfSpectCoeff[0].imag);
    fclose(FFTDump);
#endif

    pfSpectCoeff[0].imag = (dPower == 0.) ? MIN_LOG_MAG : (float)(0.5*log10(dPower));

    dAccumPower += 0.5*dPower;
    dTotalRawEnergy += 0.5*dAccumPower;
    m_pfRawSpectrum[j] = (float)(dAccumPower / (double)(++i - wBandStart));

    // Calculate cepstrum from log spectrum by taking inverse FFT.  As a real-to-complex FFT,
    // the function will assume the real part of the spectral array is even while the imaginary
    // part is odd.  Since the imaginary part has been forced to zero, producing a real spectral
    // array, the inverse FFT (cepstrum) will be both real and even.
    rfft2f((float *)pfSpectCoeff,m_wFFTLength,INVERSE);  //!!cos transform for 1st few cepstral coeff and
    //cos transform for pitch peaks may be faster
#define pfCepstralCoeff  m_pfFFTBuffer

    short nHiPeakLoc =   UNKNOWN_TREND;
    float fHiPeakVal =   MIN_LOG_MAG;

    short nTrend[3] = {UNKNOWN_TREND, UNKNOWN_TREND, UNKNOWN_TREND};  //!!preset Trend[0] and Trend[1]
    short nDirection;
    short nPeakLoc;
    uint16 wCorner = 0;
    for (i = (uint16)(m_wMaxPitchPeriod-1); i >= m_wMinPitchPeriod-1; i--) {  //!!>1 and <SpectLen
        if (pfCepstralCoeff[i] > pfCepstralCoeff[i+1]) {
            nDirection = FALLING_TREND;
        } else if (pfCepstralCoeff[i] < pfCepstralCoeff[i+1]) {
            nDirection = RISING_TREND;
        } else {
            nDirection = LEVEL_TREND;
        }
        if (nDirection != nTrend[0]) {
            nTrend[2] = nTrend[1];
            nTrend[1] = nTrend[0];
            nTrend[0] = nDirection;
            if (nTrend[0] == LEVEL_TREND) {
                wCorner = (uint16)(i + 1);
            } else if (nTrend[0] == RISING_TREND && nTrend[1] == FALLING_TREND) {
                nPeakLoc = (short)(i + 1);
                if (pfCepstralCoeff[nPeakLoc] > fHiPeakVal) {
                    fHiPeakVal = pfCepstralCoeff[nPeakLoc];
                    nHiPeakLoc = nPeakLoc;
                }
            } else if (nTrend[0] == RISING_TREND  &&  nTrend[1] == LEVEL_TREND  &&  nTrend[2] == FALLING_TREND) {
                nPeakLoc = (short)((i + 1 + wCorner)/2);                   //!!PeakLoc float?
                if (pfCepstralCoeff[nPeakLoc] > fHiPeakVal) {
                    fHiPeakVal = pfCepstralCoeff[nPeakLoc];
                    nHiPeakLoc = nPeakLoc;
                }
            }
        }
    }

    // If voiced, save pitch period.
    short & nPitchPeakLoc = nHiPeakLoc;
    float & fPitchPeakVal = fHiPeakVal;

    if (fPitchPeakVal > MIN_PITCHPEAK_THD) { //!!MIN_PEAK_THD > FLT_MIN_NEG
        m_pstFormant[0].FrequencyInHertz = (float)m_wSmpRate/(float)nPitchPeakLoc;
    } else {
        m_pstFormant[0].FrequencyInHertz = UNVOICED;
    }
    m_pstFormant[0].PowerInDecibels = LOG_MAG_NA;

    if (m_fSmoothFreq == AUTO_SET && m_pstFormant[0].FrequencyInHertz != UNVOICED) {
        m_fSmoothFreq = m_pstFormant[0].FrequencyInHertz;
    }
    uint16 wSmoothPeriod = (uint16)((float)m_wSmpRate/m_fSmoothFreq + 0.5F);

    // Divide low time cesptral coefficients by decaying exponential to sharpen formant
    // peaks.
    double r = m_fSpectCalcRadius;
    for (i = 1; i < wSmoothPeriod*2; i++, r*=m_fSpectCalcRadius) {
        pfCepstralCoeff[i] = pfCepstralCoeff[m_wFFTLength-i] = (float)(pfCepstralCoeff[i]/r);
    }

    // Remove excitation characteristic from high time portion
    const double power = 4.; // Theoretical is 2.0-6.0 traditional is infinity (approximately a rectangle)

    for (i = 1; i < wSmoothPeriod*2; i++) {
        double snr = double(wSmoothPeriod*2-i)/i;
        double weight = 1./(1. + pow(snr,-power));
        pfCepstralCoeff[i] = pfCepstralCoeff[m_wFFTLength-i] = float(weight*pfCepstralCoeff[i]);     //!!i > 0
    }

    for (i = wSmoothPeriod*2; i <= m_wFFTLength/2; i++) {
        pfCepstralCoeff[i] = pfCepstralCoeff[m_wFFTLength-i] = 0.F;     //!!i > 0
    }

    // Compute FFT to get cepstrally smoothed spectrum.
    rfft2f(pfCepstralCoeff,m_wFFTLength,FORWARD);  //!!cosine transform faster?

    // Pick peaks from cepstrally-smoothed spectrum.  Calculate average power within the
    // band for each point in the smoothed spectrum.
    dAccumPower = pow(10.,2.*(double)pfSpectCoeff[0].real);  //undo log magnitude

    wBandStart = 0;
    dSpectBand = 0.5*dSpectScale;
    dFFTFreq = dFFTScale;
    uint16 k = 0;
    double dTotalCepEnergy = 0.;
    if (dFFTFreq > dSpectBand) {
        m_pfSmoothSpectrum[k++] = (float)dAccumPower;
        // Don't count DC...
        // dTotalCepEnergy += dAccumPower;
        dAccumPower = 0.;
        dSpectBand += dSpectScale;
    }
    nTrend[0] = nTrend[1] = nTrend[2] = UNKNOWN_TREND;
    for (i = 1, j = 1; i < m_wFFTLength/2; i++) {
        if (j <= m_wNumFormants) {
            if (pfSpectCoeff[i].real > pfSpectCoeff[i-1].real) {
                nDirection = RISING_TREND;
            } else if (pfSpectCoeff[i].real < pfSpectCoeff[i-1].real) {
                nDirection = FALLING_TREND;
            } else {
                nDirection = LEVEL_TREND;
            }
            if (nDirection != nTrend[0]) {
                nTrend[2] = nTrend[1];
                nTrend[1] = nTrend[0];
                nTrend[0] = nDirection;
                if (nTrend[0] == LEVEL_TREND) {
                    wCorner = (uint16)(i - 1);
                } else if (nTrend[1] == RISING_TREND && nTrend[0] == FALLING_TREND) {
                    m_pstFormant[j].FrequencyInHertz = (float)((i - 1)*dFFTScale);
                    m_pstFormant[j++].PowerInDecibels = (float)pfSpectCoeff[i-1].real;
                } else if ((nTrend[2] == RISING_TREND && nTrend[1] == LEVEL_TREND && nTrend[0] == FALLING_TREND) ||
                           (nTrend[2] == FALLING_TREND && nTrend[1] == LEVEL_TREND && nTrend[0] == FALLING_TREND) ||
                           (nTrend[2] == RISING_TREND && nTrend[1] == LEVEL_TREND && nTrend[0] == RISING_TREND)) {
                    uint16 wMidPoint = (uint16)((i - 1 + wCorner)/2);
                    m_pstFormant[j].FrequencyInHertz = (float)(wMidPoint*dFFTScale);
                    m_pstFormant[j++].PowerInDecibels = (float)pfSpectCoeff[wMidPoint].real;
                }
            }
        }

        dAccumPower += pow(10.,2.*(double)pfSpectCoeff[i].real);

        dFFTFreq += dFFTScale;
        if (dFFTFreq > dSpectBand) {
            m_pfSmoothSpectrum[k++] = (float)(dAccumPower / (double)(i - wBandStart));
            dTotalCepEnergy += dAccumPower;
            dAccumPower = 0.;
            wBandStart = i;
            dSpectBand += dSpectScale;
        }
    }

    for (m_wNumFormants = j; j <= MAX_NUM_FORMANTS; j++) { //clear out rest of formant positions
        m_pstFormant[j].FrequencyInHertz = 0.F;
        m_pstFormant[j].PowerInDecibels = 0.F;
    }

    dAccumPower += pow(10.,2.*(double)pfSpectCoeff[0].imag);

    dTotalCepEnergy += dAccumPower;
    m_pfSmoothSpectrum[k] = (float)(dAccumPower / (double)(++i - wBandStart));


    //!!Select largest peak within 0 to 1000 Hz, 500 to 2000 Hz, 1500 to 3000 Hz, etc. heuristic goes here.

    // Convert spectral values to dB if requested.
    if (m_nScaleSelect == DB) {
        uint16 wSmpSize = (uint16)abs(m_sbSmpFormat);
        double dCepScaleDB = 10*log10(dTotalRawEnergy/dTotalCepEnergy);

        for (i = 0; i < m_wSpectLength; i++) {
            m_pfRawSpectrum[i] = (m_pfRawSpectrum[i]==0.F) ? MIN_LOG_MAG :
                                 (float)(10.*log10((double)m_pfRawSpectrum[i])) -
                                 m_fPwrDbRef[wSmpSize];
            m_pfSmoothSpectrum[i] = (m_pfSmoothSpectrum[i]==0.F) ? MIN_LOG_MAG :
                                    (float)(10.*log10((double)m_pfSmoothSpectrum[i]) -
                                            m_fPwrDbRef[wSmpSize] + dCepScaleDB);
        }
    } else {
        double dCepScale = dTotalRawEnergy/dTotalCepEnergy;
        for (i = 0; i < m_wSpectLength; i++) {
            m_pfSmoothSpectrum[i] *= float(dCepScale);
        }
    }


    return(DONE);
}
