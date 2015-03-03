/**************************************************************************\
* MODULE:       LPC.CPP                                                    *
* COPYRIGHT:    SUMMER INSTITUTE OF LINGUISTICS, 1996                      *
* AUTHOR:       ALEC K. EPTING                                             *
*                                                                          *
*                                                                          *
* DESCRIPTION:                                                             *
*                                                                          *
* Class to generate an all-pole Linear Predictive Coding (LPC) model       *
* for a frame of sample data.                                              *
*                                                                          *
* TYPICAL APPLICATION:                                                     *
*                                                                          *
* For speech, used to model the shape of the vocal tract.  If the frame    *
* is located just after the point of glottal closure, the speech signal    *
* is assumed to be the result of reverberation within the vocal tract.     *
* Accordingly, a set of optimal linear predictive coefficients may be      *
* found such that predicted outputs, based on a weighted sum of past       *
* outputs, closely resemble the actual signal in the least squared sense.  *
* From these coefficients, cross sectional areas of concatenated fixed-    *
* length tubes, approximating the vocal tract shape, may be derived.       *
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
*   Function to validate LPC settings, construct an LPC object, and        *
*   initialize it.                                                         *
*                                                                          *
*   Arguments:                                                             *
*     ppLpcObject,        address to contain pointer to LPC object         *
*     LpcSetting,         structure containing specifications for          *
*                         LPC method to be applied (as defined in LPC.H),  *
*                         order of the model, number of samples in         *
*                         the frame, and parameter sets to calculate       *
*     Signal              structure containing pointer to signal data,     *
*                         number of samples, sample format, and            *
*                         sampling rate (not used)                         *
*     FFTLength           number of points in FFT, unsigned short,         *
*                         defaults to MAX_FFT_LENGTH                       *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     DONE                      no fatal errors                            *
*     INVALID_FRAME_LEN         fatal error: invalid no. of samples to     *
*                                            process specified             *
*     INVALID_NUM_STAGES        fatal error: invalid number of sections    *
*                                            specified for model           *
*     INVALID_PARM_PTR          fatal error: invalid address specified for *
*                                            pointer to LPC object         *
*     INVALID_SIG_DATA_PTR      fatal error: invalid pointer to signal     *
*                                            data specified                *
*     INVALID_SIG_LEN           fatal error: invalid number of signal      *
*                                            samples specified             *
*     UNSUPP_SMP_DATA_FMT       fatal error: unsupported sample data       *
*                                            format                        *
*     OUT_OF_MEMORY             fatal error: insufficient memory for       *
*                                            buffers                       *
*     INVALID_LPC_METHOD        fatal error: invalid method specifed for   *
*                                            LPC analysis                  *
*     INVALID_FFT_LENGTH        fatal error: invalid FFT length specified, *
*                                            must be a power of 2 less     *
*                                            than or equal to              *
*                                            MAX_FFT_LENGTH                *
*                                                                          *
* GetLpcModel                                                              *
*   Function to compute LPC model parameters.                              *
*                                                                          *
*   Arguments:                                                             *
*     ppLpcModel          address to contain pointer to LPC model          *
*                         structure, as defined in LPC.H                   *
*     pFrame              pointer to sample data frame, cast as unsigned   *
*                         byte, short, or void type;  if not void, type    *
*                         must match data format in Signal structure       *
*                         under Setup, otherwise assumes that format       *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     INVALID_FRAME_PTR   fatal error:  frame pointer not pointing into    *
*                                       signal buffer                      *
*     INVALID_FRAME_LEN   fatal error:  frame extends beyond end of        *
*                                       signal buffer                      *
*                                                                          *
*                                                                          *
* GetPowerSpectrum                                                         *
*   Function to produce an LPC power spectrum of a given size.             *
*                                                                          *
*   Note:  The Lpc model will already contain a high resolution power      *
*          spectrum on a linear power scale if POWER_SPECTRUM was          *
*          requested when the object was constructed.  This function       *
*          merely consolidates the spectral values into the specified      *
*          number of frequency bins.                                       *
*                                                                          *
*   Arguments:                                                             *
*     wSpecLength         unsigned short word containing number of         *
*                         frequency bands (points) the power spectrum is   *
*                         to contain.                                      *
*     nScaleSelect        unsigned integer specifying LINEAR or DB         *
*                         power scale.                                     *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     DONE                      no fatal errors                            *
*     INVALID_SPECT_LENGTH      fatal error:  no bands or too many         *
*                                             specified                    *
*     INVALID_SCALE             fatal error:  invalid scale specified,     *
*                                             must be DB (default) or      *
*                                             LINEAR                       *
*     OUT_OF_SEQUENCE           fatal error:  invalid calling sequence,    *
*                                             must specify POWER_SPECTRUM  *
*                                             when creating object and     *
*                                             then call GetLpcModel first  *
*                                                                          *
*                                                                          *
* GetDecibelPowerRef                                                       *
*   Function to retun reference power level in decibels.                   *
*   Arguments:            None                                             *
*   Return value:         Reference power in decibels.                     *
*                                                                          *
*                                                                          *
* PRACTICAL CONSIDERATIONS:                                                *
*                                                                          *
*   1. Area function estimation                                            *
*      a. Number of sections should be approximately equal to twice the    *
*         length of the vocal tract (typically 17 cm for adult male)       *
*         times the sampling rate divided by the speed of sound            *
*         (34000 cm/sec).  See Markel and Gray, _Linear Prediction of      *
*         Speech_, Springer-Verlag, (c) 1982, p. 78.                       *
*      b. Valid estimates are obtained only for voiced non-nasal speech    *
*         preprocessed to eliminate the glottal shape and lip              *
*         radiation characterisitics (Markel and Gray, p. 79).             *
*      c. Analysis frame duration should be in the range of 20 - 25 msec   *
*         (Markel and Gray, p. 79).                                        *
*      d. The covariance method relies on ensuring stability by            *
*         reciprocating any poles outside the unit circle in the z-plane.  *
*         If this occurs, then the resulting area estimates may not bear   *
*         any physical relationship to the actual vocal tract area         *
*         functions.  Since the autocorrelation method is guaranteed to    *
*         be stable, no such problem arises.  However, when no pole        *
*         reciprocation is required, the covariance method may yield       *
*         more accurate results than the autocorrelation method (Markel    *
*         and Gray, p. 79).                                                *
*   2. Spectral Smoothing                                                  *
*      a. LPC tends to fit the peaks of relatively high energy resonance   *
*         behavior more accurately than the valleys of low energy          *
*         behavior (Markel and Gray, p. 136).  For this reason, LPC is     *
*         well-suited for formant frequency estimation on non-nasalized    *
*         voiced speech.                                                   *
*      b. The autocorrelation method, unlike the covariance method,        *
*         requires that several pitch periods be contained within the      *
*         analysis window (Markel and Gray, p. 152).                       *
*      c. Formant estimation is likely to be good for a male speaker with  *
*         low fundamental when the vowel is uttered forcefully, ensuring   *
*         glottal closure.  Conversely, conditions leading to possibly     *
*         unreliable results are high fundamental frequency, insufficient  *
*         interval of glottal closure, and utterances of low intensity     *
*         since low subglottal pressure will cause the volume velocity     *
*         waveform to appear almost sinusoidal (Markel and Gray, p. 152).  *
*      d. Order of model, as a rule of thumb, should be equal to           *
*         sampling frequency in kHz plus four or five (Markel and Gray,    *
*         p. 154).                                                         *
*      e. Smoothed spectra for unvoiced sounds can be performed with a     *
*         lower order model, typically around 4 (Markel and Gray, p. 156). *
*      f. Analysis frame should be 15 - 20 msec in duration for most       *
*         vowels (except possibly glides, which may require the use of     *
*         a shorter analysis frame).  This value is a compromise based     *
*         on the desire to produce stable spectral estimates, while at the *
*         same time minimimizing the effect of averaging on a time-varying *
*         signal.  The placement of the frame with respect to the pitch    *
*         period will not substantially affect the results of either the   *
*         covariance or the autocorrelation method unless the analysis is  *
*         done over a much shorter frame duration using the covariance     *
*         method.  For unvoiced speech, particularly plosives, the         *
*         duration should be much shorter than 15 - 20 msec, typically 10  *
*         msec or so. (Markel and Gray, pp. 156 - 157)                     *
*      g. Generally, windowing is recommended for both methods when the    *
*         frame duration spans more than 1 pitch period.  For less than    *
*         a pitch period, when the covariance technique is employed, a     *
*         window should not be applied. (Markel and Gray, p. 157)          *
*      h. Pre-emphasis is advised if the spectral properties of the vocal  *
*         tract, without the effects of the glottal waveform and lip       *
*         radiation characteristics, are to be estimated.  It may be       *
*         applied before or after windowing.  Pre-emphasis is not          *
*         recommended for unvoiced speech.  An adaptive pre-emphasis       *
*         filter based on autocorrelation values can account for both.     *
*         (Markel and Gray, p. 158)                                        *
*                                                                          *
*                                                                          *
* TYPICAL CALLING SEQUENCE:                                                *
*          :                                                               *
*   #include "ASAPDSP.h"                                                   *
*          :                                                               *
*   SSigParms Signal;                                                      *
*   SLPCSettings LpcSetting;                                               *
*   CLinPredCoding *pLpcObject;                                            *
*   SLPCModel pLpcModel;                                                   *
*   dspError_t Err;                                                        *
*                                                                          *
*   Signal.Start = (void *)pSpeech;                                        *
*   Signal.Length = nSamples;                                              *
*   Signal.SmpDataFmt = PCM_2SSHORT;                                       *
*   Signal.SmpRate = 22050;          //not used by LPC class               *
*   LpcSetting.nMethod = COVAR_LATTICE;                                    *
*   LpcSetting.nOrder = 20;         //20th order predictor                 *
*   LpcSetting.Process.Flags = REFL_COEFF | PRED_COEFF | NORM_CROSS_SECT | *
*                              PRE_EMPHASIS;                               *
*   LpcSetting.nFrameLen = (USHORT)(Signal.SmpRate * .020)   //20 msec     *
*   USHORT nFrameInterval = LpcSetting.nFrameLen - LpcSetting.nOrder;      *
*   Err = CLinPredCoding::CreateObject(&pLpcObject, LpcSetting, Signal);   *
*   if (Err) return(Err);                                                  *
*   for (short *pFrame = (short *)Signal.Start;                            *
*        pFrame <= (short *)Signal.Start + Signal.Length -                 *
*                  LpcSetting.nFrameLen;                                   *
*        pFrame += nFrameInterval)                                         *
*       {                                                                  *
*        Err = LpcObject->GetLpcModel(&pLpcModel, pFrame);                 *
*        if (Err) return(Err);                                             *
*        for (USHORT i = 0; i < LpcSetting.nNormCrossSectAreas; i++)       *
*             PlotArea(pLpcModel->pNormCrossSectArea[i]);                  *
*       }                                                                  *
*                :                                                         *
*   delete pLpcObject;                                                     *
*                                                                          *
*                                                                          *
* TEST DRIVER:                                                             *
*   Main (no arguments or return value); available when compiled for       *
*   QuickWin with TEST_LPC defined (/D"TEST_LPC" compile option).          *
*                                                                          *
*                                                                          *
* COMPILER:         Microsoft Visual C++ version 1.0                       *
* COMPILE OPTIONS:  /nologo /f- /G3 /W3 /Zi /AL /YX /Od /D "_DEBUG" /I     *
*                   "c:\msvc\asapdsp" /FR /GA /Fd"SA.PDB" /Fp"SA.PCH"      *
* LINK OPTIONS:     /NOLOGO /LIB:"lafxcwd" /LIB:"oldnames" /LIB:"libw"     *
*                   /LIB:"llibcew" /NOD /PACKC:61440 /STACK:10240          *
*                   /ALIGN:16 /ONERROR:NOEXE /CO /LIB:"commdlg.lib"        *
*                   /LIB:"mmsystem.lib" /LIB:"olecli.lib"                  *
*                   /LIB:"olesvr.lib" /LIB:"shell.lib"                     *
*                                                                          *
* TO DO:            1. Complete derivation of other LPC parameter sets.    *
*                   2. Code autocorrelation and lattice methods.           *
*                   3. Allow unlimited size for frame buffer.              *
*                                                                          *
* CHANGE HISTORY:                                                          *
*   Date     By             Description                                    *
* 10/14/96  AKE    Initial coding and debug.                               *
* 11/09/96  AKE    Add process flag to specify parameter sets to           *
*                  calculate.                                              *
* 11/16/96  AKE    Converted covariance matrix and other parameters to     *
*                  double floating point.                                  *
* 11/18/97  AKE    Changed Setup to CreateObject.                          *
* 11/04/99  AKE    Fixed overflow in energy and mean-squared error         *
*                  calculations.  Improved accuracy in pre-emphasis        *
*                  routine by using floating point.  Reduced minimum       *
*                  frame size required.  Changed Hungarian notation        *
*                  for double from f to d.  Version 0.2.                   *
* 06/23/00  AKE    Set default object pointer to NULL.  Check frame        *
*                  length to ensure it does not exceed 1 memory segment.   *
* 06/30/00  AKE    Added CalcPowerSpectrum.  Optimized use of memory       *
*                  by allocating only when corresponding parameter         *
*                  is requested.  Computed model gain.                     *
* 07/08/00  AKE    Added GetPowerSpectrum.  Version 0.3.                   *
* 07/13/00  AKE    Added optional FFT length specification in constructor. *
*                  Version 0.4.                                            *
* 08/12/00  AKE    Added formant estimation code.                          *
* 08/12/00  AKE    Added means to sharpen peaks in power spectrum.         *
* 09/21/00  AKE    Replaced trend follower peak picker with a bump         *
*                  detector based on parabolic curve fitting.              *
* 12/14/00  AKE    Returned formant log magnitudes as well as frequencies  *
*                  in LPC model.                                           *
* 12/18/00  AKE    Added function to remove DC bias.  Version 0.5.         *
* 04/03/01  AKE    Used peak picker to seed root finder since LPC poles    *
*                  are not really parabolic.  Also calculated bandwidths.  *
*                  Version 0.6.                                            *
* 07/22/01  AKE    Added conjugate seeds.  Version 0.7.                    *
* 07/23/01  AKE    Fixed bugs in seeding.                                  *
\**************************************************************************/
#include "stdafx.h"
#define LPC_CPP
#define COPYRIGHT_DATE  "2001"
#define VERSION_NUMBER  "0.7"
#include <search.h>
#include <math.h>
#include "Lpc.h"
#include "appdefs.h"

static float fDbPowerRef[3] = {0.F, LPC_8BIT_DB_PWR_REF, LPC_16BIT_DB_PWR_REF};

////////////////////////////////////////////////////////////////////////////////////////
// Class function to return copyright notice.                                         //
////////////////////////////////////////////////////////////////////////////////////////
char * CLinPredCoding::Copyright(void) {
    static char Notice[] = {"Linear Predictive Coding Model, Version " VERSION_NUMBER "\n"
                            "Copyright (c) " COPYRIGHT_DATE " by Summer Institute of Linguistics. "
                            "All rights reserved.\n"
                           };
    return(Notice);
}

////////////////////////////////////////////////////////////////////////////////////////
// Class function to return version of class.                                         //
////////////////////////////////////////////////////////////////////////////////////////
float CLinPredCoding::Version(void) {
    return((float)atof(VERSION_NUMBER));
}


////////////////////////////////////////////////////////////////////////////////////////
// Macro definitions.                                                                 //
////////////////////////////////////////////////////////////////////////////////////////
#define RetMemErr  { FreeLpcMem(LpcParm); return(Code(OUT_OF_MEMORY)); }


////////////////////////////////////////////////////////////////////////////////////////
// Class function to validate LPC settings and construct object.                      //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CLinPredCoding::CreateObject(CLinPredCoding ** ppLpcObject, SLPCSettings & LpcSetting,
                                        SSigParms & Signal, USHORT wFFTLength) {
    if (!ppLpcObject) {
        return(Code(INVALID_PARM_PTR));
    }
    *ppLpcObject = NULL;

    if (!Signal.Start) {
        return(Code(INVALID_SIG_DATA_PTR));
    }
    if (!Signal.Length) {
        return(Code(INVALID_SIG_LEN));
    }
    if (Signal.SmpRate < 1) {
        return(Code(INVALID_SMP_RATE));
    }
    if (Signal.SmpDataFmt != PCM_UBYTE && Signal.SmpDataFmt != PCM_2SSHORT) {
        return(Code(UNSUPP_SMP_DATA_FMT));
    }
    if (wFFTLength > MAX_FFT_LENGTH ||
            !IsSingleBitOn(wFFTLength)) { //must be power of 2 for FFT
        return(Code(INVALID_FFT_LENGTH));
    }

    SLPCParms LpcParm;
    LpcParm.ppCovarMatrix = NULL;
    LpcParm.pScratchArray = NULL;
    LpcParm.Model.pFrameBfr = NULL;
    LpcParm.Model.pWinFrameBfr = NULL;
    LpcParm.Model.pReflCoeff = NULL;
    LpcParm.Model.pPredCoeff = NULL;
    LpcParm.Model.pLpcCoeff = NULL;
    LpcParm.Model.pCorrValue = NULL;
    LpcParm.Model.pLogAreaRatio = NULL;
    LpcParm.Model.pNormCrossSectArea = NULL;
    LpcParm.Model.pPole = NULL;
    LpcParm.Model.pPredValue = NULL;
    LpcParm.Model.pResidual = NULL;
    LpcParm.Model.pPwrSpectrum = NULL;

    if (!LpcSetting.nFrameLen || (long)LpcSetting.nFrameLen > 32768) { //limit to max number of 16-bit samples in Lpc model frame buffer
        return(Code(INVALID_FRAME_LEN));
    }
    LpcParm.Model.nFrameLen = LpcSetting.nFrameLen;
    //if (!LpcSetting.nOrder || (LpcSetting.nFrameLen < 2*(USHORT)LpcSetting.nOrder + 1) ||
    if (!LpcSetting.nOrder || (LpcSetting.nFrameLen < (USHORT)(LpcSetting.nOrder + 1)) ||
            (LpcSetting.nOrder > 890)) { //CovarMatrix limited to 1 memory segment
        return(Code(INVALID_NUM_STAGES));
    }
    LpcParm.Model.nOrder = (USHORT)LpcSetting.nOrder;
    LpcParm.Process.Flags = LpcSetting.Process.Flags;
    if (LpcParm.Process.ParmSet.bPwrSpectrum) {
        if (LpcSetting.fFFTRadius <= 0.F || LpcSetting.fFFTRadius > 1.F) {
            return(Code(INVALID_RESOLUTION));
        }
        LpcParm.fFFTRadius = LpcSetting.fFFTRadius;
    }
    LpcParm.dClosurePhase = LpcSetting.dClosurePhase;
    LpcParm.dPitch = LpcSetting.dPitch;
    if (LpcParm.dPitch != 0) {
        if ((LpcSetting.nFrameLen - LpcSetting.nOrder)/(Signal.SmpRate/LpcParm.dPitch)*2*PI - LpcParm.dClosurePhase < PI) {
            LpcParm.dPitch = 0;
        } else if (LpcParm.dClosurePhase < 0 || LpcParm.dClosurePhase >= 2*PI) {
            return(Code(INVALID_PARM));
        }
    }
    USHORT NumRowPointers = (USHORT)(LpcParm.Model.nOrder + 1);
    USHORT NumColumns = (USHORT)(LpcParm.Model.nOrder + 1);
    ULONG NumMatrixElem = ((LpcParm.Model.nOrder+1)*(LpcParm.Model.nOrder+1));
    USHORT i;

    switch (LpcSetting.nMethod) {
    case LPC_COVAR_LATTICE:
    case LPC_AUTOCOR:
    case LPC_CEPSTRAL:
        LpcParm.ppCovarMatrix = (double **)new double[NumRowPointers + NumMatrixElem];
        //allocate space for row pointers and covariance values -
        //assumes each row pointer occupies no more space than
        //a matrix element
        if (!LpcParm.ppCovarMatrix) {
            return(Code(OUT_OF_MEMORY));
        }

        //Build table of row pointers into covariance matrix.  This will allow the dynamically
        //allocated memory to be addressed in the same way as a static two-dimensional matrix.
        LpcParm.ppCovarMatrix[0] = (double *)(LpcParm.ppCovarMatrix + NumRowPointers);
        for (i = 1; i <= LpcSetting.nOrder; i++) {
            LpcParm.ppCovarMatrix[i] = LpcParm.ppCovarMatrix[i-1] + NumColumns;
        }
        LpcParm.pScratchArray = new double[LpcSetting.nOrder];
        if (!LpcParm.pScratchArray) {
            RetMemErr;
        }
        break;
    default:
        return(Code(INVALID_LPC_METHOD));
    }

    LpcParm.Model.nMethod = (unsigned char) LpcSetting.nMethod;
    LpcParm.Model.pFrameBfr = (short *)new short[LpcParm.Model.nFrameLen];
    if (!LpcParm.Model.pFrameBfr) {
        RetMemErr;
    }
    LpcParm.Model.pWinFrameBfr = (short *)new short[LpcParm.Model.nFrameLen];
    if (!LpcParm.Model.pWinFrameBfr) {
        RetMemErr;
    }
    LpcParm.Model.pReflCoeff = (double *)new double[LpcParm.Model.nOrder];
    if (!LpcParm.Model.pReflCoeff) {
        RetMemErr;
    }
    LpcParm.Model.pPredCoeff = (double *)new double[LpcParm.Model.nOrder+1];
    if (!LpcParm.Model.pPredCoeff) {
        RetMemErr;
    }
    LpcParm.Model.pLpcCoeff = (double *)new double[LpcParm.Model.nOrder+1];
    if (!LpcParm.Model.pLpcCoeff) {
        RetMemErr;
    }
    LpcParm.Model.pCorrValue = (double *)new double[LpcParm.Model.nOrder+1];
    if (!LpcParm.Model.pCorrValue) {
        RetMemErr;
    }


    if (LpcParm.Process.ParmSet.bLogAreaRatios) {
        LpcParm.Model.pLogAreaRatio = (double *)new double[LpcParm.Model.nOrder];
        if (!LpcParm.Model.pLogAreaRatio) {
            RetMemErr;
        }
    }
    if (LpcParm.Process.ParmSet.bNormCrossSectAreas) {
        LpcParm.Model.pNormCrossSectArea = (double *)new double[LpcParm.Model.nOrder+1];
        if (!LpcParm.Model.pNormCrossSectArea) {
            RetMemErr;
        }
    }
    // Clear out formant array.
    for (i = 0; i < sizeof(LpcParm.Model.Formant)/sizeof(*LpcParm.Model.Formant); i++) {
        LpcParm.Model.Formant[i].FrequencyInHertz = (float)UNDEFINED_DATA;
        LpcParm.Model.Formant[i].BandwidthInHertz = (float)UNDEFINED_DATA;
        LpcParm.Model.Formant[i].PowerInDecibels = FLT_MAX_NEG;
    }
    if (LpcParm.Process.ParmSet.bFormants) {
        LpcParm.Model.pPole = (SComplexPolarFloat *)new SComplexPolarFloat[LpcParm.Model.nOrder];
        if (!LpcParm.Model.pPole) {
            RetMemErr;
        }
        // Clear out pole array.
        for (i = 0; i < LpcParm.Model.nOrder; i++) {
            LpcParm.Model.pPole[i].mag = (float)UNDEFINED_DATA;
            LpcParm.Model.pPole[i].phase = (float)UNDEFINED_DATA;
        }
        // Set formant frequency 0 to pitch frequency, assuming length of signal is 1 pitch period.
        LpcParm.Model.Formant[0].FrequencyInHertz = (float)Signal.SmpRate / (float)Signal.Length;
        LpcParm.Model.Formant[0].PowerInDecibels = FLT_MAX_NEG;
        if (!LpcParm.Process.ParmSet.bPwrSpectrum) {
            LpcParm.Process.ParmSet.bPwrSpectrum;    // need power spectrum
        }
        // for peak picking in
        // case root finding fails
    }

    if (LpcParm.Process.ParmSet.bMeanSqPredError) {
        LpcParm.Process.ParmSet.bResidual = TRUE;
    }

    if (LpcParm.Process.ParmSet.bResidual) {
        LpcParm.Process.ParmSet.bPredSignal = TRUE;
        LpcParm.Model.pResidual = (double *)new double[LpcParm.Model.nFrameLen-LpcParm.Model.nOrder];
        if (!LpcParm.Model.pResidual) {
            RetMemErr;
        }
    }
    if (LpcParm.Process.ParmSet.bPredSignal) {
        LpcParm.Model.pPredValue = (double *)new double[LpcParm.Model.nFrameLen-LpcParm.Model.nOrder];
        //LpcParm.Model.pPredValue = (short *)new short[LpcParm.Model.nFrameLen];
        if (!LpcParm.Model.pPredValue) {
            RetMemErr;
        }
    }
    if (LpcParm.Process.ParmSet.bPwrSpectrum) {
        // Allocate memory for power spectrum.  Since the signal is real, the FFT will return
        // half as many complex floating point numbers as signal samples, representing the real
        // and imaginary parts of the transform (which can be converted to magnitude and phase)
        // equally spaced in frequency across the signal bandwidth.  The first complex number
        // is unique, however; since the transform at 0 Hz and the signal bandwidth frequency
        // have no imaginary parts, their energies are packed in the first complex number, with
        // the real part representing the energy at 0 Hz and the imaginary part representing the
        // energy at the signal bandwidth frequency.  Additional memory for 2 floating point numbers
        // is reserved for conversion of the transform data to both linear and log power spectra
        // (each one will occupy half the FFT length plus 1 for the energy at the signal bandwidth
        // frequency).
        LpcParm.Model.pPwrSpectrum = (float *)new float[wFFTLength+2];
        if (!LpcParm.Model.pPwrSpectrum) {
            RetMemErr;
        }
    }

    *ppLpcObject = new CLinPredCoding(LpcParm, Signal, wFFTLength);
    if (!*ppLpcObject) {
        RetMemErr;
    }
    return(DONE);
}

////////////////////////////////////////////////////////////////////////////////////////
// LPC object constructor.                                                            //
////////////////////////////////////////////////////////////////////////////////////////
CLinPredCoding::CLinPredCoding(SLPCParms & LpcParm, SSigParms & Signal, USHORT wFFTLength) {
    //Copy LPC and signal parameters into object member variables.
    m_Signal = Signal;
    m_LpcParm = LpcParm;
    m_wFFTLength = wFFTLength;
}

////////////////////////////////////////////////////////////////////////////////////////
// LPC object destructor.                                                             //
////////////////////////////////////////////////////////////////////////////////////////
CLinPredCoding::~CLinPredCoding() {
    FreeLpcMem(m_LpcParm);
}


////////////////////////////////////////////////////////////////////////////////////////
// Class function to free memory allocated for LPC parameters.                        //
////////////////////////////////////////////////////////////////////////////////////////
void CLinPredCoding::FreeLpcMem(SLPCParms & LpcParm) {
    if (LpcParm.ppCovarMatrix) {
        delete [] LpcParm.ppCovarMatrix;
    }
    if (LpcParm.pScratchArray) {
        delete [] LpcParm.pScratchArray;
    }
    if (LpcParm.Model.pFrameBfr) {
        delete [] LpcParm.Model.pFrameBfr;
    }
    if (LpcParm.Model.pWinFrameBfr) {
        delete [] LpcParm.Model.pWinFrameBfr;
    }
    if (LpcParm.Model.pReflCoeff) {
        delete [] LpcParm.Model.pReflCoeff;
    }
    if (LpcParm.Model.pPredCoeff) {
        delete [] LpcParm.Model.pPredCoeff;
    }
    if (LpcParm.Model.pLpcCoeff) {
        delete [] LpcParm.Model.pLpcCoeff;
    }
    if (LpcParm.Model.pCorrValue) {
        delete [] LpcParm.Model.pCorrValue;
    }
    if (LpcParm.Model.pLogAreaRatio) {
        delete [] LpcParm.Model.pLogAreaRatio;
    }
    if (LpcParm.Model.pNormCrossSectArea) {
        delete [] LpcParm.Model.pNormCrossSectArea;
    }
    if (LpcParm.Model.pPole) {
        delete [] LpcParm.Model.pPole;
    }
    if (LpcParm.Model.pPredValue) {
        delete [] LpcParm.Model.pPredValue;
    }
    if (LpcParm.Model.pResidual) {
        delete [] LpcParm.Model.pResidual;
    }
    if (LpcParm.Model.pPwrSpectrum) {
        delete [] LpcParm.Model.pPwrSpectrum;
    }
}


////////////////////////////////////////////////////////////////////////////////////////
// Object function to calculate LPC model parameters.                                 //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CLinPredCoding::GetLpcModel(SLPCModel ** ppLpcModel, uint8 * pFrame) { //for 8-bit unsigned data
    if (!ppLpcModel) {
        return(Code(INVALID_PARM_PTR));
    }
    *ppLpcModel = NULL;
    if (m_Signal.SmpDataFmt != PCM_UBYTE) {
        return(Code(INVALID_SMP_DATA_FMT));
    }
    if (pFrame < (uint8 *)m_Signal.Start ||
            pFrame > (uint8 *)m_Signal.Start + m_Signal.Length) {
        return(Code(INVALID_FRAME_PTR));
    }
    if (pFrame + m_LpcParm.Model.nFrameLen > (uint8 *)m_Signal.Start + m_Signal.Length) {
        return(Code(INVALID_FRAME_LEN));
    }

    m_LpcParm.Model.nReflCoeffs = 0;
    m_LpcParm.Model.nPredCoeffs = 0;
    m_LpcParm.Model.nLpcCoeffs = 0;
    m_LpcParm.Model.dGain = 0.;
    m_LpcParm.Model.nCorrValues = 0;
    m_LpcParm.Model.nLogAreaRatios = 0;
    m_LpcParm.Model.nNormCrossSectAreas = 0;
    m_LpcParm.Model.nPoles = 0;
    m_LpcParm.Model.nPredValues = 0;
    m_LpcParm.Model.nResiduals = 0;
    m_LpcParm.Model.nSpectValues = 0;
    m_LpcParm.Model.nFormants = 0;


    if (m_LpcParm.Process.ParmSet.bPreEmphasis) {
        PreEmphasize(pFrame);
    } else {
        Transfer(pFrame);
    }

    if (m_LpcParm.Process.ParmSet.bNoDcBias) {
        RemoveDcBias();
    }

    if (m_LpcParm.Process.ParmSet.bWindowSignal) {
        ApplyWindow();
    }

    switch (m_LpcParm.Model.nMethod) {
    case LPC_COVAR_LATTICE:
    case LPC_AUTOCOR:
    case LPC_CEPSTRAL:
        CalcCovarMatrix(m_LpcParm.Model.nMethod);
        CalcReflCoeff();
        break;
    }

    if (m_LpcParm.Process.ParmSet.bEnergy) {
        CalcEnergy();
    }
    if (m_LpcParm.Process.ParmSet.bPredSignal) {
        CalcPredSignal();
    }
    if (m_LpcParm.Process.ParmSet.bResidual) {
        CalcResidual();
    }
    if (m_LpcParm.Process.ParmSet.bMeanSqPredError) {
        CalcMeanSqError();
    }
    if (m_LpcParm.Process.ParmSet.bNormCrossSectAreas) {
        CalcCrossSectAreas();
    }
    if (m_LpcParm.Process.ParmSet.bPwrSpectrum) {
        CalcPowerSpectrum();
    }
    if (m_LpcParm.Process.ParmSet.bFormants) {
        CalcFormants();    //must follow power spectrum calculation
    }

    *ppLpcModel = &m_LpcParm.Model;
    return(DONE);
}

dspError_t CLinPredCoding::GetLpcModel(SLPCModel ** ppLpcModel, short * pFrame) { //for 16-bit signed data
    if (!ppLpcModel) {
        return(Code(INVALID_PARM_PTR));
    }
    if (m_Signal.SmpDataFmt != PCM_2SSHORT) {
        return(Code(INVALID_SMP_DATA_FMT));
    }
    if (pFrame < (short *)m_Signal.Start ||
            pFrame > (short *)m_Signal.Start + m_Signal.Length) {
        return(Code(INVALID_FRAME_PTR));
    }
    if (pFrame + m_LpcParm.Model.nFrameLen > (short *)m_Signal.Start + m_Signal.Length) {
        return(Code(INVALID_FRAME_LEN));
    }

    m_LpcParm.Model.nReflCoeffs = 0;
    m_LpcParm.Model.nPredCoeffs = 0;
    m_LpcParm.Model.nCorrValues = 0;
    m_LpcParm.Model.nLogAreaRatios = 0;
    m_LpcParm.Model.nNormCrossSectAreas = 0;
    m_LpcParm.Model.nPoles = 0;
    m_LpcParm.Model.nPredValues = 0;
    m_LpcParm.Model.nResiduals = 0;
    m_LpcParm.Model.nSpectValues = 0;
    m_LpcParm.Model.nFormants = 0;

    if (m_LpcParm.Process.ParmSet.bPreEmphasis) {
        PreEmphasize(pFrame);
    } else {
        Transfer(pFrame);
    }

    if (m_LpcParm.Process.ParmSet.bNoDcBias) {
        RemoveDcBias();
    }

    if (m_LpcParm.Process.ParmSet.bWindowSignal) {
        ApplyWindow();
    }

    switch (m_LpcParm.Model.nMethod) {
    case LPC_COVAR_LATTICE:
    case LPC_AUTOCOR:
    case LPC_CEPSTRAL:
        CalcCovarMatrix(m_LpcParm.Model.nMethod);
        CalcReflCoeff();
        break;
    }

    if (m_LpcParm.Process.ParmSet.bEnergy) {
        CalcEnergy();
    }
    if (m_LpcParm.Process.ParmSet.bPredSignal) {
        CalcPredSignal();
    }
    if (m_LpcParm.Process.ParmSet.bResidual) {
        CalcResidual();
    }
    if (m_LpcParm.Process.ParmSet.bMeanSqPredError) {
        CalcMeanSqError();
    }
    if (m_LpcParm.Process.ParmSet.bNormCrossSectAreas) {
        CalcCrossSectAreas();
    }
    if (m_LpcParm.Process.ParmSet.bPwrSpectrum) {
        CalcPowerSpectrum();
    }
    if (m_LpcParm.Process.ParmSet.bFormants) {
        CalcFormants();    //must follow power spectrum calculation
    }

    *ppLpcModel = &m_LpcParm.Model;
    return(DONE);
}


dspError_t CLinPredCoding::GetLpcModel(SLPCModel ** ppLpcModel, void * pFrame) {
    dspError_t Err = Code(INVALID_FRAME_PTR);

    switch (m_Signal.SmpDataFmt) {
    case PCM_UBYTE:
        Err = GetLpcModel(ppLpcModel, (uint8 *)pFrame);
        break;
    case PCM_2SSHORT:
        Err = GetLpcModel(ppLpcModel, (short *)pFrame);
        break;
    }

    return(Err);
}

////////////////////////////////////////////////////////////////////////////////////////
// Object function to pre-emphasize frame sample data.  For speech, this removes      //
// the effect of the glottal pulse shape and radiation from the lips on the signal.   //
////////////////////////////////////////////////////////////////////////////////////////
#define DECAY  0.95
void CLinPredCoding::PreEmphasize(uint8 * pFrame) {
    if (pFrame == (uint8 *)m_Signal.Start) {
        m_LpcParm.Model.pFrameBfr[0] = ((short)(pFrame[0] - 128));
    } else {
        m_LpcParm.Model.pFrameBfr[0] = (short)Round((double)((short)(pFrame[0] - 128)) - DECAY*(double)((short)(pFrame[-1]-128)));
    }
    for (USHORT i = 1; i < m_LpcParm.Model.nFrameLen; i++) {
        m_LpcParm.Model.pFrameBfr[i] = (short)((((short)(pFrame[i] - 128))) - (short)Round(DECAY*(double)(((short)(pFrame[i-1]-128)))));
    }
    return;
}

void CLinPredCoding::PreEmphasize(short * pFrame) {
    if (pFrame == (short *)m_Signal.Start) {
        m_LpcParm.Model.pFrameBfr[0] = pFrame[0];
    } else {
        m_LpcParm.Model.pFrameBfr[0] = (short)Round((double)pFrame[0] - DECAY*(double)pFrame[-1]);
    }
    for (USHORT i = 1; i < m_LpcParm.Model.nFrameLen; i++) {
        m_LpcParm.Model.pFrameBfr[i] = (short)Round((double)pFrame[i] - DECAY*(double)pFrame[i-1]);
    }
    return;
}

void CLinPredCoding::Transfer(uint8 * pFrame) {
    for (USHORT i = 0; i < m_LpcParm.Model.nFrameLen; i++) {
        m_LpcParm.Model.pFrameBfr[i] = (short)(pFrame[i] - 128);
    }
}

void CLinPredCoding::Transfer(short * pFrame) {
    for (USHORT i = 0; i < m_LpcParm.Model.nFrameLen; i++) {
        m_LpcParm.Model.pFrameBfr[i] = pFrame[i];
    }
}

////////////////////////////////////////////////////////////////////////////////////////
// Object function to window frame of sample data.                                    //
////////////////////////////////////////////////////////////////////////////////////////
void CLinPredCoding::ApplyWindow() {
    double dTotalInput = 0;
    double dTotalWindow = 0.25;
    CDspWin cWindow = CDspWin::FromLength(m_LpcParm.Model.nFrameLen, m_Signal.SmpRate, ResearchSettings.m_cWindow.m_nType);
    const double * Window = cWindow.WindowDouble();

    for (USHORT i = 0; i < m_LpcParm.Model.nFrameLen; i++) {
        double value = m_LpcParm.Model.pFrameBfr[i] * Window[i];
        dTotalInput += m_LpcParm.Model.pFrameBfr[i] * m_LpcParm.Model.pFrameBfr[i];
        dTotalWindow += value*value;
    }

    double scale = sqrt(dTotalInput/dTotalWindow);
    for (USHORT i = 0; i < m_LpcParm.Model.nFrameLen; i++) {
        double value = m_LpcParm.Model.pFrameBfr[i] * Window[i];
        m_LpcParm.Model.pWinFrameBfr[i] = (short) Round(value * scale);
    }
}

////////////////////////////////////////////////////////////////////////////////////////
// Object function to remove DC bias from frame sample data.                          //
////////////////////////////////////////////////////////////////////////////////////////
void CLinPredCoding::RemoveDcBias() {
    double dSum=0.;
    for (USHORT i = 0; i < m_LpcParm.Model.nFrameLen; i++) {
        dSum += (double)m_LpcParm.Model.pFrameBfr[i];
    }
    double dAverage = (double)(dSum / (double)m_LpcParm.Model.nFrameLen);
    for (USHORT i = 0; i < m_LpcParm.Model.nFrameLen; i++) {
        double dData = Round((double)m_LpcParm.Model.pFrameBfr[i] - dAverage);
        if (dData > 32767.) {
            dData = 32767.;
        } else if (dData < -32768.) {
            dData = -32768.;
        } else {
            m_LpcParm.Model.pFrameBfr[i] = (short)dData;
        }
    }
}


////////////////////////////////////////////////////////////////////////////////////////
// Object function to calculate the covariance matrix required for the covariance     //
// method of LPC analysis.                                                            //
////////////////////////////////////////////////////////////////////////////////////////
void CLinPredCoding::CalcCovarMatrix(USHORT nMethod) {
    /*-----------------------------------------------------------------*\
    |  This function calculates the covariance matrix for a frame of    |
    |  speech samples.  Unlike the autocorrelation method, it requires  |
    |  no windowing of the signal.  The algorithm first calculates the  |
    |  top row of the matrix using the standard multiply and add        |
    |  technique, then exploits the symmetry property of the            |
    |  covariance of a signal with itself and a corresponding recursive |
    |  relation to derive the other rows.  The function is adapted from |
    |  _Programs for Digital Signal Processing_, IEEE Press (c) 1979,   |
    |  p. 4.2-14.                                                       |
    |                                                                   |
    |                                                                   |
    |               0  1  2  ..col.. nOrder                             |
    |        0     [ \  \  \                 ]                          |
    |        1     [   \  \  \               ]    recursive relation    |
    |        2     [     \  \  \             ]    proceeds along        |
    |        :     [       \  \  \           ]    diagonals             |
    |              [         \  \  \         ]                          |
    |        :     [           \  \  \       ]                          |
    |       row    [ this area   \  \  \     ]                          |
    |        :     [ is mirror     \  \  \   ]                          |
    |              [ image of area   \  \    ]                          |
    |              [ across this       \  \  ]                          |
    |        :     [ diagonal ---------> \   ]                          |
    |     nOrder   [                       \ ]                          |
    |                                                                   |
    |                                                                   |
    |                                                                   |
    |                              SYMMETRY PROPERTY                    |
    |                                                                   |
    |                                                                   |
    |        Covar(0,1):                                                |
    |                         0     nOrder             nFrameLen-1      |
    |      row shift = 0      |-------+----------------------| <--Frame |
    |                                                                   |
    |      column shift = 1    |-------+----------------------|         |
    |                                 |<--multiply and add-->|          |
    |                                          in here                  |
    |                                                                   |
    |        Covar(1,0):                                                |
    |                          0     nOrder             nFrameLen-1     |
    |      row shift = 1       |-------+----------------------|         |
    |                                                                   |
    |      column shift = 0   |-------+----------------------|          |
    |                                 |<--multiply and add-->|          |
    |                                          in here                  |
    |                                                                   |
    |                         Covar(0,1) and Covar(1,0) produce same    |
    |                         results                                   |
    |                                                                   |
    |                                                                   |
    |                                                                   |
    |                               RECURSIVE RELATION                  |
    |                                                                   |
    |                                                                   |
    |        Covar(0,1):                                                |
    |                         0     nOrder             nFrameLen-1      |
    |      row shift = 0      |-------+----------------------|          |
    |                                                                   |
    |      column shift = 1    |-------+----------------------|         |
    |                                 |<--multiply and add-->|          |
    |                                          in here                  |
    |                                                                   |
    |        Covar(1,2):                                                |
    |                         0     nOrder             nFrameLen-1      |
    |      row shift = 1       |-------+----------------------|         |
    |                                                                   |
    |      column shift = 2     |-------+----------------------|        |
    |                                 A                       A         |
    |                                 |   use Covar(0,1)      |         |
    |                              but add this  and  subtract this     |
    |                                                                   |
    \*-----------------------------------------------------------------*/
    unsigned int row, col, smp;

    short * pWinFrame = m_LpcParm.Model.pFrameBfr;

    if (m_LpcParm.Process.ParmSet.bWindowSignal) {
        pWinFrame = m_LpcParm.Model.pWinFrameBfr;    // Use windowed signal for analysis
    }

    int subFrames = 1;
    int nClosureBegin = 0;
    int nClosureLen = m_LpcParm.Model.nFrameLen;
    int nPitchLen = 0;

    if (m_LpcParm.dClosurePhase != 0 && m_LpcParm.dPitch != 0) {
        nPitchLen = int(m_Signal.SmpRate/m_LpcParm.dPitch);
        nClosureLen = nPitchLen/2;
        nClosureBegin = int(m_LpcParm.dClosurePhase/(2*PI)*m_Signal.SmpRate/m_LpcParm.dPitch + 1);
        subFrames = (m_LpcParm.Model.nFrameLen - nClosureBegin + nPitchLen - nClosureLen)/nPitchLen;
    }

    switch (nMethod) {
    case LPC_COVAR_LATTICE: {
        for (col = 0; col <= m_LpcParm.Model.nOrder; col++) {
            m_LpcParm.ppCovarMatrix[0][col] = 0;

            for (int subFrame = 0; subFrame < subFrames; subFrame++) {
                int begin = nClosureBegin + subFrame*nPitchLen + m_LpcParm.Model.nOrder;
                unsigned int length = nClosureLen - m_LpcParm.Model.nOrder;
                for (smp = 0; smp < length; smp++)
                    m_LpcParm.ppCovarMatrix[0][col] += (double)pWinFrame[smp+begin] *
                                                       (double)pWinFrame[smp+begin-col];
            }
        }

        for (row = 1; row <= m_LpcParm.Model.nOrder; row++) {
            for (col = 0; col < row; col++) {
                m_LpcParm.ppCovarMatrix[row][col] = m_LpcParm.ppCovarMatrix[col][row];
            }

            for (int subFrame = 0; subFrame < subFrames; subFrame++) {
                int begin = nClosureBegin + subFrame*nPitchLen + m_LpcParm.Model.nOrder;
                unsigned int length = nClosureLen - m_LpcParm.Model.nOrder;
                for (col = row; col <= m_LpcParm.Model.nOrder; col++)
                    m_LpcParm.ppCovarMatrix[row][col] = m_LpcParm.ppCovarMatrix[row-1][col-1] +
                                                        (double)pWinFrame[begin-row]*
                                                        (double)pWinFrame[begin-col] -
                                                        (double)pWinFrame[begin+length-row]*
                                                        (double)pWinFrame[begin+length-col];
            }
        }
    }
    break;
    case LPC_AUTOCOR: {
        // Autocorrelation pads the signal with zeros (on both ends)
        // error is minimized over all time with including the zero pad
        // This means the entire signal is used and the rows do not need to be adjusted
        for (col = 0; col <= m_LpcParm.Model.nOrder; col++) {
            m_LpcParm.ppCovarMatrix[0][col] = 0;
            for (int subFrame = 0; subFrame < subFrames; subFrame++) {
                int begin = nClosureBegin + subFrame*nPitchLen;
                unsigned int length = nClosureLen;
                // The entire signal is used (zero padded)
                for (smp = col; smp < length; smp++) {
                    m_LpcParm.ppCovarMatrix[0][col] += (double)pWinFrame[smp+begin] * (double)pWinFrame[smp+begin-col];
                }
            }
            m_LpcParm.ppCovarMatrix[0][col] *= nClosureLen/(nClosureLen - col);
        }

        for (row = 1; row <= m_LpcParm.Model.nOrder; row++) {
            // each row is just a rotation of the reflected coeffients
            for (col = 0; col < row; col++) {
                m_LpcParm.ppCovarMatrix[row][col] = m_LpcParm.ppCovarMatrix[col][row];
            }

            for (col = row; col <= m_LpcParm.Model.nOrder; col++) {
                m_LpcParm.ppCovarMatrix[row][col] = m_LpcParm.ppCovarMatrix[row-1][col-1];
            }
        }
    }
    break;
    case LPC_CEPSTRAL: {
        int nFFTLength = 128;
        using std::vector;

        while (nFFTLength < m_LpcParm.Model.nOrder*6 || nFFTLength < nClosureLen*6) {
            nFFTLength *= 2;
        }

        vector<float> buffer;

        buffer.assign(nFFTLength,0);

        for (smp = 0; smp < unsigned(nClosureLen); smp++) {
            buffer[smp] = pWinFrame[smp];
        }

        // Calculate FFT on real data.
        rfft2f(&buffer[0],nFFTLength,FORWARD);

        // Compute the log magnitude of the spectrum.
        int i;

        // Reference location where complex spectral values will be returned.
        SComplexRectFloat * pfSpectCoeff = (SComplexRectFloat *)&buffer[0];

        double    dPower = pfSpectCoeff[0].real*pfSpectCoeff[0].real;

        pfSpectCoeff[0].real = (dPower == 0.) ? MIN_LOG_MAG : (float)(0.5*log10(dPower));

        for (i = 1; i < nFFTLength/2; i++) {
            dPower = pfSpectCoeff[i].real*pfSpectCoeff[i].real +
                     pfSpectCoeff[i].imag*pfSpectCoeff[i].imag;

            pfSpectCoeff[i].real = (dPower == 0.) ? MIN_LOG_MAG : (float)(0.5*log10(dPower));
            pfSpectCoeff[i].imag = 0.;
        }

        dPower = pfSpectCoeff[0].imag*pfSpectCoeff[0].imag;

        pfSpectCoeff[0].imag = (dPower == 0.) ? MIN_LOG_MAG : (float)(0.5*log10(dPower));


        // Calculate cepstrum from log spectrum by taking inverse FFT.  As a real-to-complex FFT,
        // the function will assume the real part of the spectral array is even while the imaginary
        // part is odd.  Since the imaginary part has been forced to zero, producing a real spectral
        // array, the inverse FFT (cepstrum) will be both real and even.
        rfft2f(&buffer[0],nFFTLength,INVERSE);  //!!cos transform for 1st few cepstral coeff and
        //cos transform for pitch peaks may be faster

        float * pfCepstralCoeff  = &buffer[0];

        // Remove excitation characteristic from high time portion
        double d2Pitch = ResearchSettings.m_nLpcCepstralSmooth != -1 ? 2*ResearchSettings.m_nLpcCepstralSmooth : 0.5*m_Signal.SmpRate/m_LpcParm.Model.nOrder;
        int nSmoothPeriod = (int)(m_Signal.SmpRate/d2Pitch + 0.5);

        // Multiply low time cesptral coefficients by growing exponential to sharpen formant
        // peaks.
        double fSpectSharpRadius = 1/(1 - ResearchSettings.m_nLpcCepstralSharp/200.);
        double r = fSpectSharpRadius;
        for (i = 1; i < nSmoothPeriod*2; i++, r*=fSpectSharpRadius) {
            pfCepstralCoeff[i] = pfCepstralCoeff[nFFTLength-i] = float(pfCepstralCoeff[i]*r);
        }

        const double power = 4.; // Theoretical is 2.0-6.0 traditional is infinity (approximately a rectangle)

        for (i = 1; i < nSmoothPeriod*2; i++) {
            double snr = double(nSmoothPeriod*2-i)/i;
            double weight = 1./(1. + pow(snr,-power));
            pfCepstralCoeff[i] = pfCepstralCoeff[nFFTLength-i] = float(weight*(pfCepstralCoeff[i]));     //!!i > 0
        }

        for (i = nSmoothPeriod*2; i <= nFFTLength/2; i++) {
            pfCepstralCoeff[i] = pfCepstralCoeff[nFFTLength-i] = 0.;     //!!i > 0
        }

        // Compute FFT to get cepstrally smoothed spectrum.
        rfft2f(&buffer[0],nFFTLength,FORWARD);  //!!cosine transform faster?

        // Prepare to calculate the AutoCorrelation
        for (i = 1; i < nFFTLength/2; i++) {
            pfSpectCoeff[i].real = float(pow(10,2.*pfSpectCoeff[i].real));
            pfSpectCoeff[i].imag = 0.;
        }
        pfSpectCoeff[0].real = float(pow(10,2.*pfSpectCoeff[0].real));
        pfSpectCoeff[0].imag = float(pow(10,2.*pfSpectCoeff[0].imag));

        // Calculate the autocorrelation coefficients
        rfft2f(&buffer[0],nFFTLength,INVERSE);  //to get autocorrelation coefficients of cepstrally smoothed signal

        for (col = 0; col <= m_LpcParm.Model.nOrder; col++) {
            m_LpcParm.ppCovarMatrix[0][col] = buffer[col];
        }

        for (row = 1; row <= m_LpcParm.Model.nOrder; row++) {
            // each row is just a rotation of the reflected coeffients
            for (col = 0; col < row; col++) {
                m_LpcParm.ppCovarMatrix[row][col] = m_LpcParm.ppCovarMatrix[col][row];
            }

            for (col = row; col <= m_LpcParm.Model.nOrder; col++) {
                m_LpcParm.ppCovarMatrix[row][col] = m_LpcParm.ppCovarMatrix[row-1][col-1];
            }
        }
    }
    break;
    }
    return;
}


////////////////////////////////////////////////////////////////////////////////////////
// Object function to calculate reflection coefficients for the LPC model.            //
////////////////////////////////////////////////////////////////////////////////////////
void CLinPredCoding::CalcReflCoeff(void) {
    if (m_LpcParm.Model.nMethod == LPC_COVAR_LATTICE) {
        double fplusb, c, sum1, sum3, sum4, sum6, sum7, sum9, y;
        USHORT k, kp1, km1, i, j, ip1, km2, kmj, kmi;

        m_LpcParm.Model.pLpcCoeff[0] = 1.;

        for (k = 0; k < m_LpcParm.Model.nOrder; k++) {
            if (k == 0) {
                fplusb = m_LpcParm.ppCovarMatrix[0][0] + m_LpcParm.ppCovarMatrix[1][1];
                c = m_LpcParm.ppCovarMatrix[0][1];
                m_LpcParm.Model.pReflCoeff[k] = 0.;
                if (c != 0.) {
                    m_LpcParm.Model.pReflCoeff[k] = -2. * c/fplusb;
                }
                m_LpcParm.Model.pLpcCoeff[k+1] = m_LpcParm.Model.pReflCoeff[k];
                m_LpcParm.Model.dGain = m_LpcParm.ppCovarMatrix[0][0]*(1.-m_LpcParm.Model.pReflCoeff[0]*m_LpcParm.Model.pReflCoeff[0]);
            } else {
                kp1 = (USHORT)(k + 1);
                km1 = (USHORT)(k - 1);
                sum1 = sum3 = sum4 = sum6 = 0.;
                for (i = 0; i <= km1; i++) {
                    ip1= (USHORT)(i + 1);
                    m_LpcParm.pScratchArray[i] = m_LpcParm.Model.pLpcCoeff[i+1];
                    kmi = (USHORT)(k - i);
                    sum1 += m_LpcParm.Model.pLpcCoeff[i+1]*(m_LpcParm.ppCovarMatrix[0][ip1]+m_LpcParm.ppCovarMatrix[kp1][kmi]);
                    sum3 += m_LpcParm.Model.pLpcCoeff[i+1]*(m_LpcParm.ppCovarMatrix[0][kmi]+m_LpcParm.ppCovarMatrix[ip1][kp1]);
                    y = m_LpcParm.Model.pLpcCoeff[i+1]*m_LpcParm.Model.pLpcCoeff[i+1];
                    sum4 += y*(m_LpcParm.ppCovarMatrix[ip1][ip1] + m_LpcParm.ppCovarMatrix[kmi][kmi]);
                    sum6 += y*m_LpcParm.ppCovarMatrix[ip1][kmi];
                }
                sum7 = sum9 = 0.;
                if (k != 1) {
                    km2 = (USHORT)(k - 2);
                    for (i = 0; i <= km2; i++) {
                        ip1 = (USHORT)(i + 1);
                        kmi = (USHORT)(k - i);
                        for (j = ip1; j <= km1; j++) {
                            y = m_LpcParm.Model.pLpcCoeff[i+1]*m_LpcParm.Model.pLpcCoeff[j+1];
                            kmj = (USHORT)(k - j);
                            sum7 += y*(m_LpcParm.ppCovarMatrix[ip1][j+1] + m_LpcParm.ppCovarMatrix[kmi][kmj]);
                            sum9 += y*(m_LpcParm.ppCovarMatrix[ip1][kmj] + m_LpcParm.ppCovarMatrix[j+1][kmi]);
                        }
                    }
                }
                fplusb = m_LpcParm.ppCovarMatrix[0][0] + m_LpcParm.ppCovarMatrix[kp1][kp1] +
                         2.*(sum1+sum7) + sum4;
                c = m_LpcParm.ppCovarMatrix[0][kp1] + sum3 + sum6 + sum9;
                m_LpcParm.Model.pReflCoeff[k] = 0.;
                if (c != 0.) {
                    m_LpcParm.Model.pReflCoeff[k] = (-2. * c)/fplusb;
                }
                m_LpcParm.Model.dGain *= (1.-m_LpcParm.Model.pReflCoeff[k]*m_LpcParm.Model.pReflCoeff[k]);
                for (i = 0; i <= km1; i++) {
                    kmi = (USHORT)(k - i);
                    m_LpcParm.Model.pLpcCoeff[i+1] = m_LpcParm.pScratchArray[i] +
                                                     m_LpcParm.Model.pReflCoeff[k] * m_LpcParm.pScratchArray[kmi-1];
                }
                m_LpcParm.Model.pLpcCoeff[k+1] = m_LpcParm.Model.pReflCoeff[k];
            }

            m_LpcParm.Model.dMethodError = fplusb * (1. - m_LpcParm.Model.pReflCoeff[k] *
                                           m_LpcParm.Model.pReflCoeff[k]);
        }
        m_LpcParm.Model.dMethodError /= (2.*m_LpcParm.ppCovarMatrix[0][0]);

        m_LpcParm.Model.nReflCoeffs = m_LpcParm.Model.nOrder;
        m_LpcParm.Model.nLpcCoeffs = (USHORT)(m_LpcParm.Model.nOrder + 1);
        m_LpcParm.Model.nPredCoeffs = (USHORT)(m_LpcParm.Model.nOrder + 1);
        m_LpcParm.Model.pPredCoeff[0] = 1.;
        for (i = 1; i < m_LpcParm.Model.nPredCoeffs; i++) {
            m_LpcParm.Model.pPredCoeff[i] = -m_LpcParm.Model.pLpcCoeff[i];
        }

        m_LpcParm.Model.dGain = sqrt(m_LpcParm.Model.dGain);
    } else {
        // Since we are using autocorrelation method we can use simpler Levinson Recursion method
        // This method is order N^2 as opposed to N^3 for COVAR method

        double & e = m_LpcParm.Model.dMethodError;
        e = m_LpcParm.ppCovarMatrix[0][0];

        m_LpcParm.Model.pPredCoeff[0] = 1.;

        for (int i=1; i<=m_LpcParm.Model.nOrder; i++) {
            double ki = m_LpcParm.ppCovarMatrix[0][i];

            for (int j=1; j < i; j++) {
                ki -= m_LpcParm.Model.pPredCoeff[j]*m_LpcParm.ppCovarMatrix[0][i - j];
            }

            ki /= e;

            m_LpcParm.Model.pReflCoeff[i-1] = -ki;  // reflection coefficient is negative of ki (PARCOR)
            m_LpcParm.Model.pPredCoeff[i] = ki;

            for (int j = 1; j <= i; j++) {
                m_LpcParm.pScratchArray[j-1] = m_LpcParm.Model.pPredCoeff[j];
            }

            for (int j = 1; j < i; j++) {
                m_LpcParm.Model.pPredCoeff[j] = m_LpcParm.pScratchArray[j-1] - ki*m_LpcParm.pScratchArray[i-j-1];
            }

            e *= (1 - ki*ki);
        }

        m_LpcParm.Model.nReflCoeffs = m_LpcParm.Model.nOrder;
        m_LpcParm.Model.nLpcCoeffs = (USHORT)(m_LpcParm.Model.nOrder + 1);
        m_LpcParm.Model.nPredCoeffs = (USHORT)(m_LpcParm.Model.nOrder + 1);

        m_LpcParm.Model.pLpcCoeff[0] = 1.;
        for (int i = 1; i < m_LpcParm.Model.nLpcCoeffs; i++) {
            m_LpcParm.Model.pLpcCoeff[i] = -m_LpcParm.Model.pPredCoeff[i];
        }

        m_LpcParm.Model.dGain = m_LpcParm.ppCovarMatrix[0][0];
        for (int i=1; i<=m_LpcParm.Model.nOrder; i++) {
            m_LpcParm.Model.dGain -= m_LpcParm.Model.pPredCoeff[i]*m_LpcParm.ppCovarMatrix[0][i];
        }

        m_LpcParm.Model.dGain = sqrt(m_LpcParm.Model.dGain);
    }

    return;
}

////////////////////////////////////////////////////////////////////////////////////////
// Object functions to derive other LPC model parameter sets from a given set.        //
////////////////////////////////////////////////////////////////////////////////////////
void CLinPredCoding::CalcCrossSectAreas(void) {
    if (m_LpcParm.Model.nReflCoeffs) {
        m_LpcParm.Model.pNormCrossSectArea[m_LpcParm.Model.nOrder] = 1.;
        for (short i = (short)(m_LpcParm.Model.nOrder-1); i >= 0; i--)
            m_LpcParm.Model.pNormCrossSectArea[i] = m_LpcParm.Model.pNormCrossSectArea[i+1]*
                                                    (1. - m_LpcParm.Model.pReflCoeff[i])/
                                                    (1. + m_LpcParm.Model.pReflCoeff[i]); //!!divide by 0?
        m_LpcParm.Model.nNormCrossSectAreas = (USHORT)(m_LpcParm.Model.nOrder + 1);
    }
}

extern "C" int cdecl Compare(const void * pPole1, const void * pPole2) {
    // Sort by increasing phase.
    double dPhaseDiff = ((SComplexPolarFloat *)pPole1)->phase - ((SComplexPolarFloat *)pPole2)->phase;
    if (dPhaseDiff > 0.) {
        return(GREATER);
    }
    if (dPhaseDiff < 0.) {
        return(LESS);
    }
    // Then by increasing magnitude.
    double dMagDiff = ((SComplexPolarFloat *)pPole1)->mag - ((SComplexPolarFloat *)pPole2)->mag;
    if (dMagDiff > 0.) {
        return(GREATER);
    }
    if (dMagDiff < 0.) {
        return(LESS);
    }
    return(EQUAL);
}

using std::complex;
typedef complex<double> CDBL;
dspError_t CLinPredCoding::CalcPoles(void) {
    const double TwoPi = 2.*PI;
    // Seed the root finder with formant frequency estimates.  Starting with formant frequency
    // estimates tends to keep the root finder from settling on a local minimum instead of a true zero,
    // in addition to expediting the entire process.
    int nSeeds = 0;
    CDBL * pSeed = NULL;
    if (m_LpcParm.Model.nFormants) {
        pSeed = new CDBL[2*m_LpcParm.Model.nFormants];
    }
    if (pSeed) {
        // seed root finder with formant frequency estimates
        double BW = 100.; // Use 100 Hz as the Bandwidth seed
        double InvMagnitude = exp(BW*PI/m_Signal.SmpRate); // See derivation in CalcFormants
        for (USHORT i = 0; i < m_LpcParm.Model.nFormants; i++) {
            double PhaseAngle = TwoPi * (double)m_LpcParm.Model.Formant[i+1].FrequencyInHertz / (double)m_Signal.SmpRate;
            pSeed[nSeeds++] = InvMagnitude*CDBL(cos(PhaseAngle),sin(PhaseAngle));
            if (PhaseAngle != 0. && PhaseAngle != PI) {
                pSeed[nSeeds++] = InvMagnitude*CDBL(cos(PhaseAngle),-sin(PhaseAngle));    // conjugate
            }
        }
    }

    // Get poles for z^-1 polynomial.
    bool bRefine = TRUE;
    bool bPolish = TRUE;

#ifdef DUMP_COEFF
    FILE * CoeffDump = fopen("LpcCoeff.txt", "w");
    for (USHORT n = 0; n < m_LpcParm.Model.nLpcCoeffs; n++) {
        fprintf(CoeffDump, "%f ", m_LpcParm.Model.pLpcCoeff[n]);
    }
    fclose(CoeffDump);
#endif

    CDBL * pInvRoots = new CDBL[m_LpcParm.Model.nLpcCoeffs-1];

    dspError_t Err = roots(m_LpcParm.Model.pLpcCoeff, m_LpcParm.Model.nLpcCoeffs-1, pInvRoots, pSeed, nSeeds, bRefine, bPolish);
    if (pSeed) {
        delete [] pSeed;
    }
    if (Err) {
        return(Err);
    }
    m_LpcParm.Model.nPoles = m_LpcParm.Model.nLpcCoeffs - 1;
    // Invert to get poles for z polynomial.
    for (USHORT i = 0; i < m_LpcParm.Model.nPoles; i++) {
        double dRootMagnitude  = std::abs(pInvRoots[i]);
        double dRootPhase = atan2(pInvRoots[i].imag(), pInvRoots[i].real());
        if (dRootMagnitude == 0.) {
            m_LpcParm.Model.pPole[i].mag = (double)UNDEFINED_DATA;
            m_LpcParm.Model.pPole[i].phase = (double)UNDEFINED_DATA;
        } else {
            m_LpcParm.Model.pPole[i].mag = float(1/dRootMagnitude);
            m_LpcParm.Model.pPole[i].phase = float(-dRootPhase);
            if (m_LpcParm.Model.pPole[i].phase < 0.) {
                m_LpcParm.Model.pPole[i].phase += float(TwoPi);    // shift negative phase poles to upper half of table
            }
        }
    }

    delete [] pInvRoots;

    // Sort by pole phase, then by magnitude, in ascending order.
    qsort((void *)m_LpcParm.Model.pPole, (size_t)m_LpcParm.Model.nPoles, sizeof(*m_LpcParm.Model.pPole), Compare);
    return(DONE);
}

struct SFormantAssessment {
    int index;  // link to unsorted data
    BOOL bValidFrequency;
    BOOL bShadow;
    double dSecondDerivative;
    double dLocalAvgSecondDerivative;
    double dLocalizedSecondDerivative;
};

static BOOL AssessBest(SFormantAssessment & param1, SFormantAssessment & param2) {
    if (!param1.bValidFrequency < !param2.bValidFrequency) {
        return TRUE;
    }
    if (!param1.bValidFrequency > !param2.bValidFrequency) {
        return FALSE;
    }

    if (param1.bShadow < param2.bShadow) {
        return TRUE;
    }
    if (param1.bShadow > param2.bShadow) {
        return FALSE;
    }

    if (param1.dLocalizedSecondDerivative < param2.dLocalizedSecondDerivative) {
        return TRUE;
    }
    if (param1.dLocalizedSecondDerivative > param2.dLocalizedSecondDerivative) {
        return FALSE;
    }

    if (param1.dSecondDerivative < param2.dSecondDerivative) {
        return TRUE;
    }

    return FALSE;
}

static BOOL frequencyLess(SFormantValues & param1, SFormantValues & param2) {
    if (param1.FrequencyInHertz < param2.FrequencyInHertz) {
        return TRUE;
    }

    return FALSE;
}

using std::vector;
using std::sort;

void CLinPredCoding::CalcFormants(void) {
    // First estimate formant frequencies by finding bumps in the log spectrum.
    USHORT nMaxFormants = sizeof(m_LpcParm.Model.Formant)/sizeof(*m_LpcParm.Model.Formant) - 1;
    float * LogPwrSpectrum = m_LpcParm.Model.pPwrSpectrum + (m_wFFTLength/2 + 1);
    m_LpcParm.Model.nFormants = 0;
    CFormantPicker * pFormantPicker;
    dspError_t Err = CFormantPicker::CreateObject(&pFormantPicker, nMaxFormants, short(m_Signal.Source));
    if (!Err) {
        // Select formants and store them in the LPC model array.
        //SFormantValues *Formant;
        //USHORT nFormants;
        USHORT wSpectLength = (USHORT)(m_wFFTLength/2 + 1);
        double dFFTScale = (double)m_Signal.SmpRate / (double)m_wFFTLength;

        //dspError_t Err = pFormantPicker->PickFormants(&Formant, &nFormants, (float *)LogPwrSpectrum, wSpectLength,
        //                                       dFFTScale, m_LpcParm.Model.Formant[0].FrequencyInHertz);
        ULONG dwBumpCount;
        SBumpTableEntry * BumpTable;
        dspError_t Err = pFormantPicker->GetBumps(&BumpTable, (uint32 *)&dwBumpCount, (float *)LogPwrSpectrum, (ULONG)wSpectLength);
        USHORT nFormants = (USHORT)min(dwBumpCount, (ULONG)nMaxFormants);


#ifdef DUMP_BUMPS
        FILE * BumpsDump = fopen("bumps.txt", "w");
#endif
        if (!Err) {
            for (m_LpcParm.Model.nFormants = 0; m_LpcParm.Model.nFormants < nFormants; m_LpcParm.Model.nFormants++) {
                //m_LpcParm.Model.Formant[m_LpcParm.Model.nFormants+1] = Formant[m_LpcParm.Model.nFormants+1];
                m_LpcParm.Model.Formant[m_LpcParm.Model.nFormants+1].FrequencyInHertz = (float)(BumpTable[m_LpcParm.Model.nFormants+1].Distance *
                        dFFTScale);
                m_LpcParm.Model.Formant[m_LpcParm.Model.nFormants+1].BandwidthInHertz = (float)UNDEFINED_DATA;
                m_LpcParm.Model.Formant[m_LpcParm.Model.nFormants+1].PowerInDecibels = FLT_MAX_NEG;
#ifdef DUMP_BUMPS
                fprintf(BumpsDump, "F%d: %f Hz    %f\n", m_LpcParm.Model.nFormants+1, Formant[m_LpcParm.Model.nFormants+1].FrequencyInHertz, Formant[m_LpcParm.Model.nFormants+1].PowerInDecibels);
#endif
            }
        }
#ifdef DUMP_BUMPS
        fclose(BumpsDump);
#endif
        if (pFormantPicker) {
            delete pFormantPicker;
        }

        // Clear out the rest of the formant array.
        for (USHORT i = (USHORT)(m_LpcParm.Model.nFormants + 1); i <= nMaxFormants; i++) {
            m_LpcParm.Model.Formant[i].FrequencyInHertz = (float)UNDEFINED_DATA;
            m_LpcParm.Model.Formant[i].BandwidthInHertz = (float)UNDEFINED_DATA;
            m_LpcParm.Model.Formant[i].PowerInDecibels = FLT_MAX_NEG;
        }
    }

    // Then refine the formant frequency estimates with the pole frequencies.  Calculate the
    // bandwidths as well.
    Err = CalcPoles();
    if (Err) {
        return;    // return with formant estimates from spectrum if pole calculation fails
    }

    double TwoPi = 2.*PI;
    const double MinHalfPwrRadius = (6. - sqrt(30.)) / 2.;  // minimum magnitude required for a half power bandwidth
    vector<SFormantValues> unfilteredFormants;
    unfilteredFormants.reserve((unsigned int)(m_LpcParm.Model.nPoles)); // reserve space to prevent reallocation

    for (USHORT i = 0; i < m_LpcParm.Model.nPoles; i++) {
        SFormantValues formant;
        if (m_LpcParm.Model.pPole[i].phase > PI) {
            break;    // The poles are sorted by phase we do not need both complex conjugate pairs
        }
        if (m_LpcParm.Model.pPole[i].phase != (double)UNDEFINED_DATA) {
            //
            // The following bandwidth formula is based on the frequency at which the power drops by 50% from
            // that at the pole frequency in the z domain.  The problem with calculating this in the z domain
            // is that the pole frequency response is not bandlimited and is therefore aliased.  Hence, the
            // formula should be based on the s-plane (analog world) definition and then mapped to the z domain.
            // The side effect will be that the calculated bandwidths will be narrower that what can be measured
            // from the spectrum plot.
            //
            //formant.BandwidthInHertz = (2.*fabs(acos((4.*m_LpcParm.Model.pPole[i].mag-m_LpcParm.Model.pPole[i].mag*m_LpcParm.Model.pPole[i].mag-1.)/
            //                           (2.*m_LpcParm.Model.pPole[i].mag)))*(double)m_Signal.SmpRate/TwoPi);
            //
            // Now the pole frequency response magnitude in the s-plane is given by
            //
            //                                          1
            //                           |H(jw)| = ----------
            //                                      |jw - s0|
            //
            // i.e., 1 / distance on the jw axis to the pole.
            //
            // It will be maximum when the distance is minimum.  This occurs when w = Im(s0) which we will call w0.
            // It will fall 3 dB, i.e. 1/sqrt(2), when the distance from w0 along the jw axis equals the distance from
            // w0 to the pole, forming a triangle whose sides have proportions 1:1:sqrt(2).  The bandwidth wb0 is defined
            // to be twice this distance
            //
            //                                  wb0 = 2 * (-Re(s0));
            //
            //                                               jw
            //                                                ^
            //                                                |
            //                                                w----
            //                                                |  ^
            //                                                |  | distance from w to s0 is longer by a factor of sqrt(2)
            //                                                |  | when this distance equals the distance from w0 to s0
            //                                                |  v
            //                                      s0 <----->w0---
            //                                                |
            //                                                |
            //                                                |           s-plane
            //                                                |
            //                                                |
            //                       -------------------------+---------------------------->
            //                                                |
            //                                                |
            //
            //
            // Thus s0 = Re(s0) + jIm(s0) = -wb0/2 + j*w0 = -2*pi*fb0/2 + j*2*pi*f0
            //
            // Mapping to z-plane
            //
            //             s0*T     s0/fs    (-2*pi*fb0/2 + j*2*pi*f0)/fs    -pi*fb0/fs    j*2*pi*f0/fs
            //       z0 = e     = e       = e                             = e           * e
            //
            //                -pi*fb0/fs     j*2*pi*f0/fs
            //      |z0| =  |e           |*|e            |
            //                -pi*fb0/fs
            //      |z0| =   e            *  1
            //                 -pi*fb0/fs
            //   ln |z0| = ln e
            //
            //   fb0 = -fs * ln |z0| / pi
            formant.FrequencyInHertz = float((m_LpcParm.Model.pPole[i].phase / TwoPi) * m_Signal.SmpRate);

            if (m_LpcParm.Model.pPole[i].mag < MinHalfPwrRadius) {
                formant.BandwidthInHertz = (float)(-(double)m_Signal.SmpRate * log(MinHalfPwrRadius) / PI);
            } else {
                formant.BandwidthInHertz = (float)(-(double)m_Signal.SmpRate * log(m_LpcParm.Model.pPole[i].mag) / PI);
            }

            //!!de-emphasis
            USHORT k = (USHORT)Round((double)formant.FrequencyInHertz/((double)m_Signal.SmpRate/(double)m_wFFTLength));
            formant.PowerInDecibels = 10.F*LogPwrSpectrum[k];

            unfilteredFormants.push_back(formant);
        }
    }

    int j = 1;
    if (unfilteredFormants.size()) {
        // So far what we have are all the LPC poles in the frequency domain
        // We need to pick out the LPC poles which correspond to formants
        SFormantAssessment defaultAssessment;

        defaultAssessment.index = 0;
        defaultAssessment.bValidFrequency = TRUE;
        defaultAssessment.bShadow = FALSE;
        defaultAssessment.dSecondDerivative = 0;
        defaultAssessment.dLocalAvgSecondDerivative = 0;
        defaultAssessment.dLocalizedSecondDerivative = 0;

        vector<SFormantAssessment> assessment;
        assessment.assign(unfilteredFormants.size(),defaultAssessment);

        const int maxK = m_wFFTLength/2 -1;
        USHORT deltaK = (USHORT)Round((double)50/((double)m_Signal.SmpRate/(double)m_wFFTLength));

        for (USHORT i = 0; i < unfilteredFormants.size(); i++) {
            assessment[i].index = i; // save index so later we can sort and keep track of original data

            SFormantValues & formant = unfilteredFormants[i];

            // Delete if real pole (not resonant)
            if (Round(formant.FrequencyInHertz) == 0 || Round(formant.FrequencyInHertz) == Round(m_Signal.SmpRate/2.)) {
                assessment[i].bValidFrequency = FALSE;
            }

            // Delete if in shadow of more dominant pole (previous)
            if (i > 0 && formant.BandwidthInHertz > unfilteredFormants[i-1].BandwidthInHertz
                    && formant.FrequencyInHertz < unfilteredFormants[i-1].FrequencyInHertz + formant.BandwidthInHertz/2) {
                assessment[i].bShadow = TRUE;
            }

            // Delete if in shadow of more dominant pole (next)
            if (i < unfilteredFormants.size() - 1 && formant.BandwidthInHertz > unfilteredFormants[i+1].BandwidthInHertz
                    && formant.FrequencyInHertz + formant.BandwidthInHertz/2 > unfilteredFormants[i+1].FrequencyInHertz) {
                assessment[i].bShadow = TRUE;
            }

            // measure second derivative of the log spectrum
            USHORT kCenter = (USHORT)Round((double)formant.FrequencyInHertz/((double)m_Signal.SmpRate/(double)m_wFFTLength));
            double dSecondDerivative = 0;
            for (int k = kCenter - deltaK; k < kCenter + deltaK; k++) {
                dSecondDerivative += (LogPwrSpectrum[max(k-deltaK, 0)] - 2*LogPwrSpectrum[min(max(k,0),maxK)] + LogPwrSpectrum[min(k+deltaK, maxK)])/(10+abs(k-kCenter));
            }

            assessment[i].dSecondDerivative = dSecondDerivative;
        }

        // Make a second pass to localize SecondDerivatives
        double dSumSecondDerivatives = 0;
        double dLowerFrequency = unfilteredFormants[0].FrequencyInHertz;
        double dUpperFrequency = 0;
        ULONG nLowerIndex = 0;
        ULONG nUpperIndex = 0;
        for (USHORT i = 0; i < unfilteredFormants.size(); i++) {
            SFormantValues & formant = unfilteredFormants[i];
            while (dUpperFrequency < formant.FrequencyInHertz + 2500) {
                if (nUpperIndex < unfilteredFormants.size()) {
                    dSumSecondDerivatives +=  assessment[nUpperIndex++].dSecondDerivative;
                }
                if (nUpperIndex < unfilteredFormants.size()) {
                    dUpperFrequency = unfilteredFormants[nUpperIndex].FrequencyInHertz;
                } else {
                    dUpperFrequency = m_Signal.SmpRate + 3000;
                }
            }

            while (dLowerFrequency < formant.FrequencyInHertz - 2500) {
                dSumSecondDerivatives -=  assessment[nLowerIndex++].dSecondDerivative;
                dLowerFrequency = unfilteredFormants[nLowerIndex].FrequencyInHertz;
            }

            int n = nUpperIndex - nLowerIndex;

            assessment[i].dLocalAvgSecondDerivative = n ? dSumSecondDerivatives/n : 0;
            assessment[i].dLocalizedSecondDerivative = assessment[i].dSecondDerivative - assessment[i].dLocalAvgSecondDerivative/2;
        }

        sort(assessment.begin(), assessment.end(), AssessBest);

        int nDesiredFormants = int(Round(m_Signal.SmpRate/2000.));

        for (USHORT i = 0; i < nDesiredFormants && i < assessment.size(); i++) {
            SFormantAssessment & assess = assessment[i];
            SFormantValues & formant = unfilteredFormants[assess.index];

            if (j < nMaxFormants) {
                m_LpcParm.Model.Formant[j++] = formant;
            }
        }
    }

    m_LpcParm.Model.nFormants = (USHORT)(j-1);

    sort(&m_LpcParm.Model.Formant[1],&m_LpcParm.Model.Formant[j], frequencyLess);

    // Clear out the rest of the formant array.
    for (USHORT i = (USHORT)(m_LpcParm.Model.nFormants + 1); i <= nMaxFormants; i++) {
        m_LpcParm.Model.Formant[i].FrequencyInHertz = (float)UNDEFINED_DATA;
        m_LpcParm.Model.Formant[i].BandwidthInHertz = (float)UNDEFINED_DATA;
        m_LpcParm.Model.Formant[i].PowerInDecibels = FLT_MAX_NEG;
    }

}

void CLinPredCoding::CalcEnergy(void) {
    switch (m_LpcParm.Model.nMethod) {
    case LPC_COVAR_LATTICE:
        m_LpcParm.Model.dMeanEnergy = m_LpcParm.ppCovarMatrix[0][0]/(m_LpcParm.Model.nFrameLen - m_LpcParm.Model.nOrder);
        break;
    case LPC_AUTOCOR:
        m_LpcParm.Model.dMeanEnergy = m_LpcParm.ppCovarMatrix[0][0]/m_LpcParm.Model.nFrameLen;
        break;
    default:
        m_LpcParm.Model.dMeanEnergy = 0.;
        for (USHORT i = m_LpcParm.Model.nOrder; i < m_LpcParm.Model.nFrameLen; i++) {
            m_LpcParm.Model.dMeanEnergy += (double)m_LpcParm.Model.pFrameBfr[i]*(double)m_LpcParm.Model.pFrameBfr[i];
        }
        m_LpcParm.Model.dMeanEnergy /= (m_LpcParm.Model.nFrameLen - m_LpcParm.Model.nOrder);
    }
}


void CLinPredCoding::CalcPredSignal(void) {
    m_LpcParm.Model.nPredValues = (USHORT)(m_LpcParm.Model.nFrameLen - m_LpcParm.Model.nOrder);
    for (USHORT i = 0; i < m_LpcParm.Model.nPredValues; i++) {
        double fPredValue = 0.;
        for (USHORT j = 1; j <= m_LpcParm.Model.nOrder; j++)
            fPredValue += m_LpcParm.Model.pPredCoeff[j]*
                          (double)m_LpcParm.Model.pFrameBfr[i+m_LpcParm.Model.nOrder-j];


        m_LpcParm.Model.pPredValue[i] = (fPredValue);
    }
}


void CLinPredCoding::CalcResidual(void) {
    if (!m_LpcParm.Model.nPredValues) {
        CalcPredSignal();
    }

    m_LpcParm.Model.nResiduals = m_LpcParm.Model.nPredValues;
    for (USHORT i = 0; i < m_LpcParm.Model.nResiduals; i++) {
        m_LpcParm.Model.pResidual[i] = m_LpcParm.Model.pFrameBfr[m_LpcParm.Model.nOrder+i] -
                                       m_LpcParm.Model.pPredValue[i];  //!!overflow?
    }
}


void CLinPredCoding::CalcMeanSqError(void) {
    if (!m_LpcParm.Model.nResiduals) {
        CalcResidual();
    }

    m_LpcParm.Model.dMeanSqPredError = 0.;
    m_LpcParm.Model.dMeanQuadPredError = 0.;
    m_LpcParm.Model.dPeakError = 0.;
    for (USHORT i = 0; i < m_LpcParm.Model.nResiduals; i++) {
        double error =
            (double)m_LpcParm.Model.pResidual[i]*m_LpcParm.Model.pResidual[i];
        m_LpcParm.Model.dMeanSqPredError += error;
        m_LpcParm.Model.dMeanQuadPredError += error*error;
        if (error > m_LpcParm.Model.dPeakError) {
            m_LpcParm.Model.dPeakError = error;
        }
    }
    m_LpcParm.Model.dMeanSqPredError /= (double)m_LpcParm.Model.nResiduals; //!!divide by 0?
    m_LpcParm.Model.dMeanQuadPredError /= (double)m_LpcParm.Model.nResiduals; //!!divide by 0?
}


////////////////////////////////////////////////////////////////////////////////////////
// Object functions to generate a power spectrum.                                     //
////////////////////////////////////////////////////////////////////////////////////////
void CLinPredCoding::CalcPowerSpectrum(void) {
    // Point to buffer which first contains LPC input data, then FFT results, and finally power spectrum.
    float * LpcCoeff = (float *)m_LpcParm.Model.pPwrSpectrum;
    SComplexRectFloat * FFT = (SComplexRectFloat *)m_LpcParm.Model.pPwrSpectrum;

    // Load buffer with negative LPC predictor coefficients.  The denominator of the LPC model transfer function contains
    // a power series in z with negative predictor coefficients.  Except for the upper limit, the power series has the same
    // form as the DFT.  The upper limit is extended to the range of the DFT by setting coefficients beyond the order of
    // the model to 0.
    LpcCoeff[0] = (float)m_LpcParm.Model.pPredCoeff[0];
    float fRadius = m_LpcParm.fFFTRadius;

    USHORT i;
    for (i=1; i <= m_LpcParm.Model.nOrder; i++) {
        LpcCoeff[i] = -(float)m_LpcParm.Model.pPredCoeff[i] / fRadius;
        fRadius *= m_LpcParm.fFFTRadius;  // pre-condition to sharpen peaks
    }
    for (; i < m_wFFTLength; i++) {
        LpcCoeff[i] = 0.F;
    }

    // Perform an FFT to get the the denominator of the transfer function.
    rfft2f(LpcCoeff, m_wFFTLength, FORWARD);

    // Compute the magnitude squared of the FFT results, reciprocate, and multiply by the LPC model gain squared
    // to get the power spectrum.
    double    dGainSquared = m_LpcParm.Model.dGain * m_LpcParm.Model.dGain;
    double    dPower = (FFT[0].real==0.F) ? FLT_MAX : (dGainSquared / ((double)FFT[0].real*(double)FFT[0].real));
    double    dLastPower = (FFT[0].imag==0.F) ? FLT_MAX : (dGainSquared / ((double)FFT[0].imag*(double)FFT[0].imag));

    if (m_LpcParm.Process.ParmSet.bPreEmphasis) {
        dLastPower /= (1. + 2.*DECAY + DECAY*DECAY);
        dPower /= (1. - 2.*DECAY + DECAY*DECAY);
    }

    m_LpcParm.Model.pPwrSpectrum[0] = (float)dPower;
    USHORT wSpectLength = (USHORT)(m_wFFTLength/2 + 1);
    for (i = 1; i < wSpectLength-1; i++) {
        double dMagSquared = (double)FFT[i].real*(double)FFT[i].real + (double)FFT[i].imag*(double)FFT[i].imag;
        dPower = (dMagSquared==0.) ? FLT_MAX : (dGainSquared / (double)dMagSquared);
        if (m_LpcParm.Process.ParmSet.bPreEmphasis) {
            dPower /= (1. - 2.*DECAY*cos(2.*PI*i/(double)m_wFFTLength) + DECAY*DECAY);    // de-emphasize the FFT results
        }
        // to achieve overall rolloff from
        // glottal pulse and lip radiation
        m_LpcParm.Model.pPwrSpectrum[i] = (float)dPower;
    }

    m_LpcParm.Model.pPwrSpectrum[i++] = (float)dLastPower;
    m_LpcParm.Model.nSpectValues = i;

    /*
    double dFFTScale = (double)m_Signal.SmpRate / (double)m_wFFTLength;
    USHORT nMaxFormants = sizeof(m_LpcParm.Model.Formant)/sizeof(*m_LpcParm.Model.Formant) - 1;
    */

    // Point to storage block for log version of power spectrum, to be located just above linear version.  Linear version
    // must be retained to consolidate power within each band for plotting purposes.  Log version is required to find
    // "hidden" formants which evidently appear as bumps only in the log spectrum.
    float * pLogPwrSpectrum = m_LpcParm.Model.pPwrSpectrum + wSpectLength;

    // Construct a formant picker object, which selects peaks or bumps in the log spectrum which fall within acceptable
    // ranges for each formant.
    // Compute log power spectrum from linear spectrum.
    for (i = 0; i < wSpectLength; i++) {
        pLogPwrSpectrum[i] = (m_LpcParm.Model.pPwrSpectrum[i] == 0.F) ? MIN_LOG_PWR / 10.F:(float)log10(m_LpcParm.Model.pPwrSpectrum[i]);
    }
}

dspError_t CLinPredCoding::GetPowerSpectrum(USHORT wSpectLength, int nScaleSelect) {
    // Validate parameters.
    if (!wSpectLength || wSpectLength > m_wFFTLength / 2) {
        return(Code(INVALID_SPECT_LEN));    // limit to half FFT length
    }
    if (nScaleSelect != LINEAR && nScaleSelect != DB) {
        return(Code(INVALID_SCALE));    // allow linear or log scales only
    }

    // Check calling sequence.
    if (!m_LpcParm.Process.ParmSet.bPwrSpectrum) {
        return(Code(OUT_OF_SEQUENCE));
    }

    // Average the power for all points in each frequency band.
    double dSpectScale = (double)m_Signal.SmpRate/(2.*(double)(wSpectLength-1));
    double dFFTScale = (double)m_Signal.SmpRate/(double)m_wFFTLength;
    short  nBandStart = -1;
    double    dSpectBand = 0.5*dSpectScale;
    double    dFFTFreq = 0.*dFFTScale;
    double    dPower = 0.;
    double    dAccumPower = dPower;

    short j = 0;
    short i;
    for (i=0; i < (short)m_LpcParm.Model.nSpectValues; i++) {
        dAccumPower += m_LpcParm.Model.pPwrSpectrum[i];
        dFFTFreq += dFFTScale;
        if (dFFTFreq > dSpectBand) {
            m_LpcParm.Model.pPwrSpectrum[j] = (float)(dAccumPower / (double)(i - nBandStart));
            if (nScaleSelect == DB)
                m_LpcParm.Model.pPwrSpectrum[j] = (m_LpcParm.Model.pPwrSpectrum[j] == 0.F) ? MIN_LOG_PWR :
                                                  (float)(10.*log10(m_LpcParm.Model.pPwrSpectrum[j]))
                                                  - GetDecibelPowerRef(); //adjust to plot in range (found empirically)
            dAccumPower = 0.;
            nBandStart = i;
            dSpectBand += dSpectScale;
            j++;
        }
    }

    m_LpcParm.Model.pPwrSpectrum[j] = (float)(dAccumPower / (double)(i - nBandStart));
    if (nScaleSelect == DB) m_LpcParm.Model.pPwrSpectrum[j] = (m_LpcParm.Model.pPwrSpectrum[j] == 0.F) ? MIN_LOG_PWR :
                (float)(10.*log10(m_LpcParm.Model.pPwrSpectrum[j]))
                - GetDecibelPowerRef(); //adjust to plot in range (found empirically)

    m_LpcParm.Model.nSpectValues = (USHORT)(j + 1);

#ifdef DUMP_SPECTRUM
    FILE * SpectrumDump = fopen("LpcSpect.txt", "w");
    float MaxSpectValue = 0.F;
    for (i = 0; i < m_LpcParm.Model.nSpectValues; i++) {
        if (MaxSpectValue < m_LpcParm.Model.pPwrSpectrum[i]) {
            maxSpec = m_LpcParm.Model.pPwrSpectrum[i];
        }
        fprintf(SpectrumDump, "%f ", m_LpcParm.Model.pPwrSpectrum[i]);
    }
    fclose(SpectrumDump);
#endif

    return(DONE);
}

float CLinPredCoding::GetDecibelPowerRef() {
    return(fDbPowerRef[abs(m_Signal.SmpDataFmt)]);
}


#ifdef TEST_LPC
#include <stdio.h>

void main(void) {
    static short Sig[] = {-16,-18,-21,-24,-27,-30,-31,-33,
                          -36,-39,-42,-44,-46,-49,-51,-55,
                          -58,-61,-64,-67,-71,-73,-68,-53,
                          -38,-36,-31,-13,  6, 15, 19, 35
                         };

    SSigParms Signal;
    SLPCSettings LpcSetting;
    CLinPredCoding * pLpcObject;
    SLPCModel * pLpcModel;
    dspError_t Err;

    Signal.Start = (void *)Sig;
    Signal.Length = sizeof(Sig)/sizeof(*Sig);
    Signal.SmpDataFmt = PCM_2SSHORT;
    Signal.SmpRate = 22050;
    LpcSetting.nMethod = COVAR_LATTICE;
    LpcSetting.nOrder = 4;
    LpcSetting.Process.Flags = REFL_COEFF | PRED_COEFF | NORM_CROSS_SECT |
                               PRED_SIGNAL | MEAN_SQ_ERR | RESIDUAL;
    LpcSetting.nFrameLen = (USHORT)Signal.Length;
    Err = CLinPredCoding::CreateObject(&pLpcObject, LpcSetting, Signal);
    if (Err) {
        printf("Error %lx\n occurred\n", Err);
        return;
    }
    Err = pLpcObject->GetLpcModel(&pLpcModel, Sig);
    if (Err) {
        printf("Error %lx\n occurred\n", Err);
        delete pLpcObject;
        return;
    }

    USHORT i;
    for (i = 0; i < pLpcModel->nPredCoeffs; i++) {
        printf("PredCoeff[%u] = %6.6f\n", i, pLpcModel->pPredCoeff[i]);
    }
    printf("\n");

    for (i = 0; i < pLpcModel->nReflCoeffs; i++) {
        printf("ReflCoeff[%u] = %6.6f\n", i, pLpcModel->pReflCoeff[i]);
    }
    printf("\n");

    printf("Gain = %f\n", pLpcModel->dGain);
    printf("\n");

    for (i = 0; i < pLpcModel->nNormCrossSectAreas; i++) {
        printf("NormCrossSectArea[%u] = %6.6f\n", i, pLpcModel->pNormCrossSectArea[i]);
    }

    printf("\nMethod Error = %f\n", pLpcModel->dMethodError);

    printf("Mean squared prediction error = %6.6f\n", pLpcModel->dMeanSqPredError);


    ////////////////////////////////////////////////////////////////////////////////////////
    //                                 TEST RESULTS                                       //
    //                                                                                    //
    // PredCoeff[0] = 1.000000                                                            //
    // PredCoeff[1] = 1.981905                                                            //
    // PredCoeff[2] = -1.845783                                                           //
    // PredCoeff[3] = 1.601839                                                            //
    // PredCoeff[4] = -0.774958                                                           //
    //                                                                                    //
    // ReflCoeff[0] = -0.985770                                                           //
    // ReflCoeff[1] = 0.754381                                                            //
    // ReflCoeff[2] = -0.165097                                                           //
    // ReflCoeff[3] = 0.774958                                                            //
    //                                                                                    //
    // Gain =                                                                             //
    //                                                                                    //
    // NormCrossSectArea[0] = 0.289298                                                    //
    // NormCrossSectArea[1] = 40.369996                                                   //
    // NormCrossSectArea[2] = 5.651936                                                    //
    // NormCrossSectArea[3] = 7.887214                                                    //
    // NormCrossSectArea[4] = 1.000000                                                    //
    //                                                                                    //
    // Method Error = 0.003935                                                            //
    // Mean squared prediction error = 10.678571                                          //
    //                                                                                    //
    ////////////////////////////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////////////////////////

    delete pLpcObject;
    return;
}

#endif
