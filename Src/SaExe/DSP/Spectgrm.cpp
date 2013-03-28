/**************************************************************************\
* MODULE:       SPECTGRM.CPP                                               *
* COPYRIGHT:    SUMMER INSTITUTE OF LINGUISTICS, 1996                      *
* AUTHOR:       ALEC K. EPTING                                             *
*                                                                          *
*                                                                          *
* DESCRIPTION:                                                             *
*                                                                          *
* Class to generate power spectrogram and formant tracks from 8 or 16-bit  *
* sample data.                                                             *
*                                                                          *
*                                                                          *
* PUBLIC MEMBER VARIABLES:                                                 *
*   ScreenData            pointer to batch of spectra arranged left-       *
*                         to-right, top-to-bottom, in unsigned byte        *
*                         format, ready to be displayed via BitBlt         *
*                                                                          *
*                                                                          *
* PUBLIC MEMBER FUNCTIONS:                                                 *
*                                                                          *
* Note: all arguments are passed using the C calling convention.           *
*                                                                          *
* CreateObject                                                             *
*   Function to initialize spectrogram.                                    *
*                                                                          *
*   Arguments:                                                             *
*     Spgm,               address to contain pointer to spectrogram object *
*     SpgmSetting,        structure containing specifications for          *
*                         spectrogram dimensions, spectral resolution,     *
*                         signal block to process, and frequency display   *
*                         range, including options to request pre-emphasis *
*                         and formant tracking                             *
*     Signal              structure containing pointer to signal data,     *
*                         number of samples, sample format, and            *
*                         sampling rate                                    *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     DONE                      no fatal errors                            *
*     INVALID_FREQ              fatal error: invalid frequency range       *
*                                            specified                     *
*     INVALID_BANDWIDTH         fatal error: invalid spectral resolution   *
*                                            specified                     *
*     INVALID_PREEMPH_OPTION    fatal error: invalid pre-emphasis option   *
*                                            specified                     *
*     INVALID_FMNTTRACK_OPTION  fatal error: invalid formant tracking      *
*                                            option specified              *
*     INVALID_NUM_SPECTRA       fatal error: invalid no. of spectra to     *
*                                            compute specified             *
*     INVALID_BATCH_SIZE        fatal error: invalid no. of spectra per    *
*                                            batch specified               *
*     INVALID_BLOCK_LEN         fatal error: invalid no. of samples to     *
*                                            process specified             *
*     UNSUPP_SMP_RATE           fatal error: unsupported sampling rate     *
*     INVALID_SIG_DATA_PTR      fatal error: invalid pointer to signal     *
*                                            data specified                *
*     INVALID_SIG_LEN           fatal error: invalid number of signal      *
*                                            samples specified             *
*     UNSUPP_SMP_DATA_FMT       fatal error: unsupported sample data       *
*                                            format                        *
*     OUT_OF_MEMORY             fatal error: insufficient memory for       *
*                                            buffers                       *
*                                                                          *
*                                                                          *
* ChangeSettings                                                           *
*   Function to change initial settings.                                   *
*                                                                          *
*   Arguments:                                                             *
*     SpgmSetting,        structure containing new specifications for      *
*                         spectrogram dimensions, signal block, frequency  *
*                         range, spectral resolution, pre-emphasis, and    *
*                         formant tracking                                 *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     DONE                    no fatal errors                              *
*     INVALID_FREQ            fatal error: invalid frequency range         *
*                                          specified                       *
*     INVALID_BANDWIDTH       fatal error: invalid spectral resolution     *
*                                          specified                       *
*     INVALID_PREEMPH_OPTION  fatal error: invalid pre-emphasis option     *
*                                          specified                       *
*     INVALID_NUM_SPECTRA     fatal error: invalid no. of spectra to       *
*                                          compute specified               *
*     INVALID_BATCH_SIZE      fatal error: invalid no. of spectra per      *
*                                          batch specified                 *
*     INVALID_BLOCK_LEN       fatal error: invalid no. of samples to       *
*                                          process specified               *
*     OUT_OF_MEMORY           fatal error: insufficient memory for buffers *
*                                                                          *
* GetMetrics                                                               *
*   Function to get spectrogram parameters.                                *
*                                                                          *
*   Arguments:                                                             *
*     FreqScale,          address to contain plot frequency scale in Hz    *
*     SpectScale,         address to contain internally-used FFT scale     *
*                         in Hz                                            *
*     SpectRes,           address to contain spectral resolution           *
*                           (bandwidth) in Hz                              *
*     SpectIntv,          address to contain spectral sampling interval    *
*                           (in number of samples)                         *
*     MaxPwr              address to contain maximum spectral power level  *
*                           so                                         *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     DONE                no fatal errors                                  *
*     INVALID_PARM_PTR    fatal error:  invalid pointer to parameter       *
*                                       specified                          *
*                                                                          *
* Generate                                                                 *
*   Function to generate spectrogram and formant tracks, if requested.     *
*                                                                          *
*   Arguments:                                                             *
*     None                                                                 *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     NOT_READY           fatal error:  spectrogram not properly set up    *
*                                       or function calling sequence dspError_t   *
*     PENDING             message:      batch processed, ready to run      *
*                                       next batch                         *
*     END_OF_SPGM         message:      spectrogram complete               *
*                                                                          *
*                                                                          *
* ReadPower                                                                *
*   Function to retrieve power reading (in db) from specified spectrogram  *
*   coordinates.                                                           *
*                                                                          *
*   Arguments:                                                             *
*     Power               address to contain power reading; power          *
*                         values inside spectrogram plot range from DB_MIN *
*                         to DB_MAX as defined in SPECTGRM.H, outside the  *
*                         value is set to NA                               *
*     SpgmX               offset from left side of spectrogram window pane *
*     SpgmY               offset from top of spectrogram window pane       *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     DONE                no fatal errors                                  *
*     INVALID_PARM_PTR    fatal error:  invalid pointer to parameter       *
*                                       specified                          *
*     OUT_OF_SEQUENCE     fatal error:  function called before spectrogram *
*                                       generated                          *
*     INVALID_SPGM_COORD  one or both coordinates outside pane             *
*                                                                          *
*                                                                          *
* ReadFormants                                                             *
*   Function to retrieve pitch and formant frequencies at the specified    *
*   spectrogram coordinate.                                                *
*                                                                          *
*   Arguments:                                                             *
*     Freq                address of structure to contain pitch and        *
*                         formant frequencies. If pitch (F[0]) is set to   *
*                         (float)NA the formant info is not available      *
*                         usually because there is insufficient signal     *
*                         data to process; if set to (float)UNVOICED, the  *
*                         frame is deemed to be unvoiced. All other values *
*                         signify a voiced frame, and thus valid formant   *
*                         frequencies.  Formants are taken to be           *
*                         frequencies at which peaks or plateaus occur in  *
*                         the smoothed spectrum.  The frequencies are      *
*                         assigned to F[1], F[2], ... F[10] in order,      *
*                         lowest to  highest.  Any unused members of the   *
*                         F array are set to 0.F.                          *
*     SpgmX               offset from left side of spectrogram window pane *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     DONE                no fatal errors                                  *
*     INVALID_PARM_PTR    fatal error:  invalid pointer to parameter       *
*                                       specified                          *
*     OUT_OF_SEQUENCE     fatal error:  function called before spectrogram *
*                                       generated                          *
*     INVALID_SPGM_COORD  one or both coordinates outside pane             *
*                                                                          *
*                                                                          *
* CalcFormants                                                             *
*   Function to calculate pitch and formant frequencies for block of       *
*   speech specified in SpgmSetting.                                       *
*                                                                          *
*   Arguments:                                                             *
*     None                                                                 *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     OUT_OF_SEQUENCE     fatal error:  function called before spectrogram *
*                                       generated                          *
*     PENDING             message:      batch processed, ready to run      *
*                                       next batch                         *
*     END_FMNT_TRK        message:      formant tracking complete          *
*                                                                          *
* IsAliased                                                                *
*   Function to return boolean flag indicating if screen data is aliased   *
*                                                                          *
*   Arguments:                                                             *
*     None                                                                 *
*                                                                          *
*   Return value:  boolean flag (0 = false, 1 = true)                      *
*                                                                          *
*                                                                          *
*                                                                          *
* TYPICAL CALLING SEQUENCE:                                                *
*          :                                                               *
*   #include "ASAPDSP.h"                                                   *
*          :                                                               *
*   SPGM_SETTINGS SpgmSetting;                                             *
*   SIG_PARMS Signal;                                                      *
*   dspError_t dspError_t;                                                               *
*                                                                          *
*   SpgmSetting.LwrFreq = (float)MinDisplayFrequency;                      *
*   SpgmSetting.UprFreq = (float)MaxDisplayFrequency;                      *
*   SpgmSetting.FreqCnt = (uint16)nPlotYDivisions;                         *
*   if (SpecResolution == FINE) SpgmSetting.Bandwidth = NARROW_BW;         *
*   else SpgmSetting.Bandwidth = WIDE_BW;                                  *
*   SpgmSetting.SpectCnt = nPlotXDivisions;                                *
*   SpgmSetting.PreEmphSw = true;                                            *
*   if (FormantTracking == YES) SpgmSetting.FmntTrackSw = true             *
*   else SpgmSetting.FmntTrackSw = false;                                  *
*   SpgmSetting.SigBlkOffset = SpeechBlockStart/SampleByteSize;            *
*   SpgmSetting.SigBlkLength = SpeechBlockByteSize/SampleByteSize;         *
*   SpgmSetting.SpectBatchLength = 50;                                     *
*                                                                          *
*   if (!m_Spectrogram)                                                    *
*      {                                                                   *
*       Signal.Start = pSpeechSamples;                                     *
*       Signal.Length = SpeechByteSize/SampleByteSize;                     *
*       Signal.SmpRate = (uint16)nSamplesPerSecond;                        *
*       if (SampleByteSize == 1) Signal.SmpDataFmt = PCM_UBYTE;            *
*       else Signal.SmpDataFmt = PCM_2SSHORT;                              *
*                                                                          *
*       dspError_t = Spectrogram::Setup(&m_Spectrogram, SpgmSetting, Signal);     *
*       if (dspError_t) return(dspError_t);                                *
*      }                                                                   *
*   else                                                                   *
*      {                                                                   *
*       dspError_t = m_Spectrogram->ChangeSettings(SpgmSetting);           *
*       if (dspError_t) return(dspError_t);                                *
*      }                                                                   *
*                                                                          *
*   BITMAP BitmapParms;                                                    *
*   BitmapParms.bmType = 0;                                                *
*   BitmapParms.bmWidth = SpgmSetting.SpectBatchLength;                    *
*   BitmapParms.bmWidthBytes = BitmapParms.bmWidth +                       *
*                              (BitmapParms.bmWidth % 2);                  *
*   BitmapParms.bmHeight = (int32)nPlotYDivisions;                         *
*   BitmapParms.bmPlanes = 1;                                              *
*   BitmapParms.bmBitsPixel = 8;                                           *
*   BitmapParms.bmBits = m_Spectrogram->ScreenData;                        *
*                                                                          *
*   CDC dcMem;                                                             *
*   if (!dcMem.CreateCompatibleDC(pdc)) return(CREATE_DC_ERR);             *
*                                                                          *
*   dspError_t dspError_t;                                                 *
*   for (uint16 PlotX = 0;                                                 *
*        PlotX < nPlotXDivisions;                                          *
*        PlotX += SpgmSetting.SpectBatchLength)                            *
*       {                                                                  *
*        dspError_t = m_Spectrogram->Generate();                           *
*        if (dspError_t < DONE) return(dspError_t);                        *
*        if (nPlotXDivisions - PlotX < SpgmSetting.SpectBatchLength)       *
*           {                                                              *
*            BitmapParms.bmWidth = (int32)((uint16)nPlotXDivisions - PlotX); *
*            BitmapParms.bmWidthBytes = BitmapParms.bmWidth +              *
*                                       (BitmapParms.bmWidth % 2);         *
*           }                                                              *
*                                                                          *
*        CBitmap *SpgmBitmap = new CBitmap;                                *
*        if (!SpgmBitmap->CreateBitmapIndirect(&BitmapParms))              *
*            return(CREATE_BM_ERR);                                        *
*                                                                          *
*        CBitmap *OldBitmap = dcMem.SelectObject(SpgmBitmap);              *
*        if (!OldBitmap) return(SELECT_BM_ERR);                            *
*        pdc->BitBlt(rect.left + PlotX, rect.top,                          *
*                    BitmapParms.bmWidth, (int32)nPlotYDivisions,          *
*                    &dcMem, 0, 0, SRCCOPY);                               *
*        dcMem.SelectObject(OldBitmap);                                    *
*        delete SpgmBitmap;                                                *
*       }                                                                  *
*                                                                          *
*   float FreqScale, SpectralScale;                                        *
*   uint32 SpectralInterval;                                               *
*   uint8 MaxPower;                                                        *
*   dspError_t = m_Spectrogram->GetMetrics(&FreqScale, &SpectralScale,     *
*                                   &SpgmSetting.Bandwidth,                *
*                                   &SpectralInterval, &MaxPower);         *
*   if (dspError_t) return(dspError_t);                                    *
*                                                                          *
*   short Power;                                                           *
*   dspError_t = ReadPower(&Power, MouseX, MouseY);                        *
*   if (dspError_t) return(dspError_t);                                    *
*                                                                          *
*   FORMANT_FREQ Freq;                                                     *
*   dspError_t = ReadFormants(&Freq, MouseX);                              *
*   if (dspError_t) return(dspError_t);                                    *
*                                                                          *
*                                                                          *
* COMPILER:         Microsoft Visual C++ version 1.0                       *
* COMPILE OPTIONS:  /nologo /f- /G3 /W3 /Zi /AL /YX /Od /D "_DEBUG" /I     *
*                   "c:\msvc\asapdsp" /FR /GA /Fd"SA.PDB" /Fp"SA.PCH"      *
* LINK OPTIONS:     /NOLOGO /LIB:"lafxcwd" /LIB:"oldnames" /LIB:"libw"     *
*                   /LIB:"llibcew" /LIB:"c:\msvc\asapdsp\fft86e" /NOD      *
*                   /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE     *
*                   /CO /LIB:"commdlg.lib" /LIB:"mmsystem.lib"             *
*                   /LIB:"olecli.lib" /LIB:"olesvr.lib" /LIB:"shell.lib"   *
*                                                                          *
* TO DO:            1. Store spectra lwr freq to upr freq.                 *
*                   2. Do not store entire spectrogram,                    *
*                      get power and formants on request.                  *
*                   3. License FFT routine and include copyright notice.   *
*                                                                          *
* CHANGE HISTORY:                                                          *
*   Date     By             Description                                    *
* 11/28/95  AKE    Initial coding.                                         *
* 03/14/96  AKE    Spectrogram debugged and running.                       *
* 04/25/96  AKE    Formant tracking debugged and running.                  *
* 05/08/96  AKE    Added functions to read power and formant frequencies.  *
* 10/08/96  UR     Small modifications for use in SA V2.00 marked by ##    *
* 04/09/97  AKE    Fixed problem of using NB window length for blanking    *
*                  out spectra when formant tracking not enabled.          *
* 08/01/97  AKE    Added support for 44.1 and 48 kHz sampling rates.       *
* 06/23/00  AKE    Set default object pointer to NULL.                     *
* 07/06/00  WGM    Improved formant tracking.                              *
* 07/21/00  AKE    Fixed validation for number of formants requested.      *
* 08/26/00  AKE    Changed FLT_MAX_NEG to MIN_LOG_PWR.  Version 0.2.       *
* 04/27/01  AKE    Change spectrum interval calculation. Version 0.3.      *
\**************************************************************************/
#include "stddsp.h"

#define SPECTGRM_CPP
#define COPYRIGHT_DATE  "2000"
#define VERSION_NUMBER  "0.3"

#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <assert.h>
#include "MathX.h"
#include "Scale.h"
#include "DspWins.h"
#include "PeakPick.h"
#include "Spectgrm.h"


#ifdef DEBUG
#include <stdio.h>
FILE * PitchDump;
FILE * CepstraDump;
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Class function to return copyright notice.                                         //
////////////////////////////////////////////////////////////////////////////////////////
char * Spectrogram::Copyright(void)
{
    static char Banner[] = {"Spectrogram Version " VERSION_NUMBER "\n"
                            "Copyright (c) " COPYRIGHT_DATE " by Summer Institute of Linguistics. "
                            "All rights reserved.\n"
                           };
    return(Banner);
}

////////////////////////////////////////////////////////////////////////////////////////
// Class function to return version of class.                                         //
////////////////////////////////////////////////////////////////////////////////////////
float Spectrogram::Version(void)
{
    return((float)atof(VERSION_NUMBER));
}

////////////////////////////////////////////////////////////////////////////////////////
// Class function to construct spectrogram object if parameters are valid.            //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t Spectrogram::CreateObject(Spectrogram ** Spgm, SPGM_SETTINGS SpgmSetting, SIG_PARMS Signal)
{
    // Validate requested spectrogram settings and signal parameters.
    if (!Spgm)
    {
        return(Code(INVALID_PARM_PTR));    //address of pointer to spectrogram object
    }
    //  must not be NULL
    *Spgm = NULL;
    dspError_t dspError_t = ValidateSettings(SpgmSetting);    //check settings
    if (dspError_t)
    {
        return(dspError_t);
    }
    dspError_t = ValidateSignalParms(Signal);          //check signal parameters
    if (dspError_t)
    {
        return(dspError_t);
    }
    if (SpgmSetting.UprFreq > (float)Signal.SmpRate/2.F)   //upper frequency requested for
    {
        return(Code(INVALID_FREQ));    //  spectrogram must not be
    }
    //  greater than signal bandwidth
    if (SpgmSetting.SigBlkOffset + SpgmSetting.SigBlkLength > Signal.Length)
    {
        return(Code(INVALID_BLOCK_LEN));    //block to calculate must not go beyond end of signal data
    }
    if (SpgmSetting.SpectCnt > Signal.Length)
    {
        return(Code(INVALID_NUM_SPECTRA));    //number of spectra must not exceed signal length
    }


    // Calculate window for frame data from specified bandwidth.
    DspWin Window = DspWin::FromBandwidth(SpgmSetting.Bandwidth,Signal.SmpRate, SpgmSetting.windowType);

    // Create narrowband window for formant tracking.
    DspWin NBWindow = DspWin::FromBandwidth(NARROW_BW,Signal.SmpRate, SpgmSetting.windowType);

    // Allocate memory for spectrogram based on number of points to display.
    int32 Size = (int32)SpgmSetting.FreqCnt * (int32)SpgmSetting.SpectCnt;
    uint8 * SpgmData = (uint8 *)malloc(Size * sizeof(uint8));
    if (!SpgmData)
    {
        return(Code(OUT_OF_MEMORY));
    }

    // Allocate memory for formant data.
    FORMANT_FREQ * FmntData = (FORMANT_FREQ *)malloc((uint16)SpgmSetting.SpectCnt *
                              sizeof(FORMANT_FREQ));
    if (!FmntData)
    {
        free(SpgmData);
        return(Code(OUT_OF_MEMORY));
    }

#ifdef SPECTGRM_SCREEN
    // Allocate buffer for transferring batch of spectra to screen.
    Size = (int32)(SpgmSetting.SpectBatchLength + (SpgmSetting.SpectBatchLength % 2)) *
           (int32)SpgmSetting.FreqCnt;
    uint8 * ScreenData = new uint8[Size];
    if (!ScreenData)
    {
        free(FmntData);
        free(SpgmData);
        return(Code(OUT_OF_MEMORY));
    }
#else
    uint8 * ScreenData = NULL;
#endif

    // Construct spectrogram object.
    *Spgm = new Spectrogram(SpgmSetting, SpgmData, FmntData, Window, NBWindow, Signal, ScreenData);
    if (!*Spgm)
    {
        //!!put in function
        delete [] ScreenData;
        free(FmntData);
        free(SpgmData);
        return(Code(OUT_OF_MEMORY));
    }

    return(DONE);
}


////////////////////////////////////////////////////////////////////////////////////////
// Class function to validate signal parameters.                                      //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t Spectrogram::ValidateSignalParms(SIG_PARMS Signal)
{
    if (Signal.SmpRate < 1)
    {
        return(Code(UNSUPP_SMP_RATE));
    }
    if (!Signal.Start)
    {
        return(Code(INVALID_SIG_DATA_PTR));    //signal data starting address
    }
    //  must not be NULL
    if (!Signal.Length)
    {
        return(Code(INVALID_SIG_LEN));    //number of samples must not be
    }
    //  NULL
    if (Signal.SmpDataFmt != PCM_UBYTE &&   //sample data format should be unsigned byte PCM, or
            Signal.SmpDataFmt != PCM_2SSHORT)   //  2's complement signed 16-bit PCM
    {
        return(Code(UNSUPP_SMP_DATA_FMT));
    }
    return(DONE);
}

////////////////////////////////////////////////////////////////////////////////////////
// Class function to validate requested spectrogram settings.                         //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t Spectrogram::ValidateSettings(SPGM_SETTINGS Setting)
{
    if (Setting.LwrFreq > Setting.UprFreq)   //lower frequency of spectra must be less than
    {
        return(Code(INVALID_FREQ));    //  upper
    }
    if (!Setting.FreqCnt)
    {
        return(Code(INVALID_SPGM_HGT));    //requested number of points in
    }
    //  spectra must not be zero
    if (!Setting.Bandwidth || Setting.Bandwidth > Setting.UprFreq)
    {
        return(Code(INVALID_BANDWIDTH));    //requested spectra resolution
    }
    //  must not be zero or greater
    //  than signal bandwidth
    if (Setting.PreEmphSw != false && Setting.PreEmphSw != true)   //requested pre-emphasis can
    {
        return(Code(INVALID_PREEMPH_OPTION));    //  only be false or true
    }
    if (Setting.FmntTrackSw != false && Setting.FmntTrackSw != true)   //requested formant tracking
    {
        return(Code(INVALID_FMNTTRACK_OPTION));    //  can only be false or true
    }
    if (Setting.FmntTrackSw == true &&
            (Setting.NumFmnt > MAX_NUM_FORMANTS || Setting.NumFmnt < 0))   //can't exceed limit
    {
        return(Code(INVALID_FMNTTRACK_OPTION));
    }
    if (!Setting.SpectCnt)
    {
        return(Code(INVALID_NUM_SPECTRA));    //requested number of spectra
    }
    //to generate must not be zero
    if (!Setting.SpectBatchLength || Setting.SpectBatchLength > Setting.SpectCnt)
    {
        return(Code(INVALID_BATCH_SIZE));    //batch size to process must not be zero or greater
    }
    //  than number of spectra
    if (!Setting.SigBlkLength)
    {
        return(Code(INVALID_BLOCK_LEN));    //block size of spectra to
    }
    //  display must not be zero
    return(DONE);
}



////////////////////////////////////////////////////////////////////////////////////////
// Spectrogram object constructor.                                                    //
////////////////////////////////////////////////////////////////////////////////////////
Spectrogram::Spectrogram(SPGM_SETTINGS SpgmSetting, uint8 * SpgmData, FORMANT_FREQ * FmntData,
                         DspWin & Window, DspWin & NBWindow, SIG_PARMS Signal, uint8 * ScreenData) : m_Window(Window), m_NBWindow(NBWindow)
{
    // Update object member variables.
    m_ScreenData = ScreenData;
    m_SpectRes = (float)Window.Bandwidth();
    m_LwrFreq = SpgmSetting.LwrFreq;
    m_UprFreq = SpgmSetting.UprFreq;
    m_FreqScale = (SpgmSetting.UprFreq - SpgmSetting.LwrFreq)/
                  (float)(SpgmSetting.FreqCnt - 1);
    m_PreEmphSw = SpgmSetting.PreEmphSw;
    m_FmntTrackSw = SpgmSetting.FmntTrackSw;
    m_NumFmnt = SpgmSetting.NumFmnt;
    m_SigStart = Signal.Start;
    m_SigLength = Signal.Length;
    m_SigBlkOffset = SpgmSetting.SigBlkOffset;
    m_SigBlkLength = SpgmSetting.SigBlkLength;
    m_SmpDataFmt = Signal.SmpDataFmt;
    m_SmpRate = Signal.SmpRate;
    m_SpectCnt = SpgmSetting.SpectCnt;
    m_SpectLen = uint16(SpgmSetting.FFTLength/2);
    m_FFTLen = uint16(2*m_SpectLen);
    m_SpectScale = (float)Signal.SmpRate / (float)m_FFTLen;
    m_SpectIntv = (float)SpgmSetting.SigBlkLength/(float)SpgmSetting.SpectCnt;
    m_SpgmData = SpgmData;
    m_SpgmHgt = SpgmSetting.FreqCnt;
    m_FmntData = FmntData;
    m_SpectBatchLength = SpgmSetting.SpectBatchLength;
    m_SpectToProc = 0;
    m_FmntSetToProc = 0;
    m_MaxPwr = 0;
    m_MaxPitchPeriod = (uint16)((float)m_SmpRate/(float)MIN_PITCH + 0.5F);
    m_MinPitchPeriod = (uint16)((float)m_SmpRate/(float)MAX_PITCH + 0.5F);
    if (m_SpectRes < 0.5F*(float)m_SmpRate/m_SpectIntv)
    {
        m_Aliased = false;
    }
    else
    {
        m_Aliased = true;
    }

    m_Status = SPGM_PENDING;
#ifdef DEBUG
    PitchDump = fopen("Pitch.dat", "w");
    CepstraDump = fopen("Cepstra.dat", "w");
#endif
    m_WinFrame = new float[2*m_SpectLen];
}


////////////////////////////////////////////////////////////////////////////////////////
// Spectrogram object destructor.                                                     //
////////////////////////////////////////////////////////////////////////////////////////
Spectrogram::~Spectrogram()
{
    // Deallocate all buffers.
    if (m_ScreenData)
    {
        delete [] m_ScreenData;
    }
    if (m_SpgmData)
    {
        free(m_SpgmData);
    }
    if (m_FmntData)
    {
        free(m_FmntData);
    }
    if (m_WinFrame)
    {
        delete [] m_WinFrame;
    }
#ifdef DEBUG
    fclose(PitchDump);
    fclose(CepstraDump);
#endif
}


////////////////////////////////////////////////////////////////////////////////////////
// Object function to retrieve various spectrogram parameters.                        //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t Spectrogram::GetMetrics(float * FreqScale,
                                   float * SpectScale, float * SpectRes, float * SpectIntv,
                                   uint8 * MaxPwr)
{
    // Ensure pointers are not null and set corresponding member variables.
    if (!FreqScale)
    {
        return(Code(INVALID_PARM_PTR));
    }
    *FreqScale = m_FreqScale;
    if (!SpectScale)
    {
        return(Code(INVALID_PARM_PTR));
    }
    *SpectScale = m_SpectScale;
    if (!SpectRes)
    {
        return(Code(INVALID_PARM_PTR));
    }
    *SpectRes = m_SpectScale;
    if (!SpectIntv)
    {
        return(Code(INVALID_PARM_PTR));
    }
    *SpectIntv = m_SpectIntv;
    if (!MaxPwr)
    {
        return(Code(INVALID_PARM_PTR));
    }
    *MaxPwr = m_MaxPwr;

    return(DONE);
}

////////////////////////////////////////////////////////////////////////////////////////
// Object function to return power at specified spectrogram coordinates.              //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t Spectrogram::ReadPower(short * Power, uint16 SpgmX, uint16 SpgmY)
{
    if (!Power)
    {
        return(Code(INVALID_PARM_PTR));
    }
    if (m_Status < SPGM_CALC)
    {
        *Power = NA;
        return(Code(OUT_OF_SEQUENCE));
    }

    if (SpgmX >= m_SpectCnt || SpgmY >= m_SpgmHgt)
    {
        return(Code(INVALID_SPGM_COORD));
    }
    *Power = (short)*(m_SpgmData + (uint32)SpgmX * (uint32)m_SpgmHgt + SpgmY);
    if (*Power == BLANK)
    {
        *Power = NA;
    }
    return(DONE);
}

////////////////////////////////////////////////////////////////////////////////////////
// Object function to return a pointer to a power slice at specified spectrogram coordinates. ##             //
////////////////////////////////////////////////////////////////////////////////////////
uint8 * Spectrogram::ReadPowerSlice(dspError_t * dspError_t, uint16 SpgmX)
{
    if (!dspError_t)
    {
        return NULL;
    }
    if (m_Status < SPGM_CALC)
    {
        *dspError_t = Code(OUT_OF_SEQUENCE);
        return NULL;
    }
    if (SpgmX >= m_SpectCnt)
    {
        *dspError_t = Code(INVALID_SPGM_COORD);
        return NULL;
    }
    *dspError_t = DONE;
    return (m_SpgmData + (uint32)SpgmX * (uint32)m_SpgmHgt);
}

////////////////////////////////////////////////////////////////////////////////////////
// Object function to return pitch and formant frequencies for specified spectrogram  //
// coordinates.                                                                       //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t Spectrogram::ReadFormants(FORMANT_FREQ * Freq, uint16 SpgmX)
{
    if (!Freq)
    {
        return(Code(INVALID_PARM_PTR));
    }
    if (m_Status < FORMANTS_CALC)
    {
        Freq->F[0] = (float)NA;
        return(Code(OUT_OF_SEQUENCE));
    }
    if (SpgmX >= m_SpectCnt)
    {
        return(Code(INVALID_SPGM_COORD));
    }
    *Freq = m_FmntData[SpgmX];

    return(DONE);
}


////////////////////////////////////////////////////////////////////////////////////////
// Object function to generate spectrogram.                                           //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t Spectrogram::Generate(void)
{
    // Ensure call is valid.
    if (m_Status == SPGM_NOT_READY)
    {
        return(Code(NOT_READY));    //if not ready, exit
    }
    if (m_SpectToProc == m_SpectCnt)                                //if already done, exit
    {
        return((int32)END_OF_SPGM);
    }

    // Set number of spectra to compute.
    uint32 NumSpect;
    if (m_SpectToProc+(uint32)m_SpectBatchLength > m_SpectCnt)
    {
        NumSpect = m_SpectCnt - m_SpectToProc;    //if last batch, calculate no. of spectra left
    }
    else
    {
        NumSpect = (uint32)m_SpectBatchLength;    //otherwise, set to default value
    }


    // Call function to calculate spectra based on sample data format.
    dspError_t dspError_t;
    switch (m_SmpDataFmt)
    {
    case PCM_2SSHORT:     // 2's complement short (2-byte) PCM
        dspError_t = CalcSpectra((short *)m_SigStart+m_SigBlkOffset, NumSpect);
        if (dspError_t)
        {
            return(dspError_t);
        }
        break;
    case PCM_UBYTE:       // unsigned byte
        dspError_t = CalcSpectra((uint8 *)m_SigStart+m_SigBlkOffset, NumSpect);
        if (dspError_t)
        {
            return(dspError_t);
        }
        break;
    }


    // Copy spectra to the screen buffer, transposing in the process, to
    // permit high speed data transfer to the plotting window via BitBlt.

    /*## #define PALETTE_OFFSET  10                            //offset into palette
    uint8 * p = ScreenData;
    uint8 * s = SpgmData + SpectToProc*SpgmHgt;
    uint16 i, j;
    for (i = 0; i < SpgmHgt; i++)                  //top to bottom
    {
    uint8 * q = &s[i];
    for (j = 0; j < NumSpect; j++)            //left to right
    {
    *p++ = PALETTE_OFFSET + *q;
    q += SpgmHgt;
    }
    if (NumSpect % 2) *p++ = PALETTE_OFFSET + BLANK;  //append blank column -
    //  required by BitBlt
    }
    dspError_t = SPGM_CALC;

    // Superimpose formant tracks if feature is enabled.

    if (FmntTrackSw == true)
    {
    FORMANT_FREQ * Frame = (FORMANT_FREQ *)FmntData + SpectToProc;
    for (i = 0; i < NumSpect; i++)
    {
    #ifdef DEBUG
    if (Frame[i].F[0] != (float)NA) fprintf(PitchDump, "%6.2f ", Frame[i].F[0]);
    else fprintf(PitchDump, "0.00 ");
    #endif

    if (Frame[i].F[0] == 0.F || Frame[i].F[0] == (float)NA) continue; //skip if unvoiced
    //or blank frame
    for (j = 1; j <= MAX_NUM_FORMANTS; j++)
    {
    if (Frame[i].F[j] < LwrFreq) continue;
    if (Frame[i].F[j] == 0.F || Frame[i].F[j] > UprFreq) break;
    *(ScreenData + i + (NumSpect + (NumSpect%2))* (SpgmHgt -
    (uint16)((Frame[i].F[j]-LwrFreq)/FreqScale + 0.5F))) =
    PALETTE_OFFSET + FMNT_TRACK;
    }        //!!use built-in gray?
    }
    */     m_Status = FORMANTS_CALC;
    //##    }


    // Advance index to next spectrum to process.
    // m_SpectToProc += NumSpect;

    // Return.
    if (m_SpectToProc != m_SpectCnt)
    {
        return(PENDING);    //indicate spectrogram still incomplete
    }
    return((int32)END_OF_SPGM);                     //indicate spectrogram done and include
    //  maximum power calculated in least
    //  significant byte of return value
}

////////////////////////////////////////////////////////////////////////////////////////
// Private object function to blank out beginning and end of spectrogram for          //
// synchronizing it with the acoustic waveform.                                       //
////////////////////////////////////////////////////////////////////////////////////////
void Spectrogram::BlankSpectrum(uint8 * Spect)
{
    for (uint16 i = 0; i < m_SpgmHgt; i++)
    {
        Spect[i] = BLANK;
    }
    return;
}

////////////////////////////////////////////////////////////////////////////////////////
// Private object functions to calculate spectra based on sample data format.         //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t Spectrogram::CalcSpectra(short * BlockStart, uint32 NumSpect)
{
    // Calculate addresses for first and last spectra.
    uint8 * FirstSpect = m_SpgmData + m_SpectToProc*m_SpgmHgt;
    uint8 * LastSpect = FirstSpect + (NumSpect-1)*m_SpgmHgt;

    // Calculate address for first set of formants.
    FORMANT_FREQ * FirstFmntSet = m_FmntData + m_SpectToProc;

    int32 WinMargin = m_Window.Length() / 2;   // margin available to left and right of frame
    if (m_FmntTrackSw == true && m_NBWindow.Length() > m_Window.Length())
    {
        WinMargin = m_NBWindow.Length() / 2;
    }
    short * SigEnd = (short *)m_SigStart + m_SigLength - abs(m_SmpDataFmt);


    // Generate spectra.
    FORMANT_FREQ * FmntSet = FirstFmntSet;
    for (uint8 * Spect = FirstSpect;
            Spect <= LastSpect;
            Spect += m_SpgmHgt, m_SpectToProc++)
    {

        //   Calculate address for beginning of sample data frame.  Spectrum will be computed
        //   at the center of the frame.
        short * MidFrame = BlockStart + (uint32)((float)m_SpectToProc*m_SpectIntv + 0.5F);

        //   If frame data invalid, emit blank spectra and formant tracks, if formant tracking
        //   enabled.
        if ((MidFrame < (short *)m_SigStart + WinMargin) ||
                (MidFrame > SigEnd - WinMargin))
        {
            BlankSpectrum(Spect);
            if (m_FmntTrackSw == true)
            {
                BlankFmntSet(FmntSet);
                FmntSet++;
            }
        }

        //   Otherwise, preprocess frame and compute power spectrum.
        else
        {
            dspError_t dspError_t = PreProc(MidFrame - m_Window.Length()/2, m_Window);
            //apply appropriate window for requested
            //  pre-emphasis and spectral resolution
            if (dspError_t)
            {
                return(dspError_t);
            }
            dspError_t = PwrFFT(Spect);                     //calculate spectral values in dB
            if (dspError_t)
            {
                return(dspError_t);
            }
            if (m_FmntTrackSw == true)                   //if formant tracking enabled,
            {
                if (m_SpectRes != NARROW_BW)           //  if windowed frame not narrowband,
                {
                    dspError_t = PreProc(MidFrame - m_NBWindow.Length()/2, m_NBWindow);  //    apply narrowband window
                    if (dspError_t)
                    {
                        return(dspError_t);
                    }
                    rfft2f(m_WinFrame,m_FFTLen,FORWARD); //    and calculate spectral values
                }
                dspError_t = GetFormants(FmntSet);          //  find the formants if any
                if (dspError_t)
                {
                    return(dspError_t);
                }
                FmntSet++;                           //  point to next location to store formants
            }
        }
    }

    return(DONE);
}


////////////////////////////////////////////////////////////////////////////////////////
// Object function to calculate spectra for sample data in unsigned byte format.      //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t Spectrogram::CalcSpectra(uint8 * BlockStart, uint32 NumSpect)
{
    // Calculate addresses for first and last spectra.
    uint8 * FirstSpect = m_SpgmData + m_SpectToProc*m_SpgmHgt;
    uint8 * LastSpect = FirstSpect + (NumSpect-1)*m_SpgmHgt;

    // Calculate address for first set of formants.
    FORMANT_FREQ * FirstFmntSet = m_FmntData + m_SpectToProc;

    int32 WinMargin = m_Window.Length() / 2;   // margin available to left and right of frame
    if (m_FmntTrackSw == true && m_NBWindow.Length() > m_Window.Length())
    {
        WinMargin = m_NBWindow.Length() / 2;
    }
    uint8 * SigEnd = (uint8 *)m_SigStart + m_SigLength - abs(m_SmpDataFmt);


    // Generate spectra.
    FORMANT_FREQ * FmntSet = FirstFmntSet;
    for (uint8 * Spect = FirstSpect;
            Spect <= LastSpect;
            Spect += m_SpgmHgt, m_SpectToProc++)
    {

        //   Calculate address for beginning of sample data frame.  Spectrum will be computed
        //   at the center of the frame.
        uint8 * MidFrame = BlockStart + (uint32)((float)m_SpectToProc*m_SpectIntv + 0.5F);

        //   If frame data invalid, emit blank spectra and formant tracks, if formant tracking
        //   enabled.
        if ((MidFrame < (uint8 *)m_SigStart + WinMargin) ||
                (MidFrame > SigEnd - WinMargin))
        {
            BlankSpectrum(Spect);
            if (m_FmntTrackSw == true)
            {
                BlankFmntSet(FmntSet);
                FmntSet++;
            }
        }

        //   Otherwise, preprocess frame and compute power spectrum.
        else
        {
            dspError_t dspError_t = PreProc(MidFrame - m_Window.Length()/2, m_Window);
            //apply appropriate window for requested
            //  pre-emphasis and spectral resolution
            if (dspError_t)
            {
                return(dspError_t);
            }
            dspError_t = PwrFFT(Spect);                     //calculate spectral values in dB
            if (dspError_t)
            {
                return(dspError_t);
            }
            if (m_FmntTrackSw == true)                   //if formant tracking enabled,
            {
                if (m_SpectRes != NARROW_BW)           //  if windowed frame not narrowband,
                {
                    dspError_t = PreProc(MidFrame - m_NBWindow.Length()/2, m_NBWindow);  //    apply narrowband window
                    if (dspError_t)
                    {
                        return(dspError_t);
                    }
                    rfft2f(m_WinFrame,m_FFTLen,FORWARD); //    and calculate spectral values
                }
                dspError_t = GetFormants(FmntSet);          //  find the formants if any
                if (dspError_t)
                {
                    return(dspError_t);
                }
                FmntSet++;                           //  point to next location to store formants
            }
        }
    }

    return(DONE);
}


////////////////////////////////////////////////////////////////////////////////////////
// Object function to pre-process frame of signed 16-bit sample data.                 //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t Spectrogram::PreProc(short * Frame, DspWin & Window)
{
    uint16 i;
    int32 j;

    const float * WData = Window.WindowFloat();

    // If pre-emphasis requested, take first difference of sample data, then apply
    // window.
    if (m_PreEmphSw == true)
    {

        //  Initialize windowed frame value with first difference.
        if (Frame == m_SigStart)   //first difference does not apply to first sample in signal
        {
            m_WinFrame[0] = WData[0] * Frame[0];    //produces slight error if
        }
        //  first sample of signal nonzero
        else
        {
            m_WinFrame[0] = WData[0] * ((float)Frame[0] - (float)Frame[-1]);
        }

        //   Pre-alias in the time domain, if necessary, to keep number of points required in the
        //   FFT to a minimum without undersampling in the frequency domain.  This will implement
        //   a bank of filters.
        for (j = (int32)m_FFTLen; j < Window.Length(); j += m_FFTLen)
        {
            m_WinFrame[0] += WData[j] * ((float)Frame[j] - (float)Frame[j-1]);
        }
        for (i = 1; i < m_FFTLen; i++)
            for (m_WinFrame[i] = 0.F, j = (int32)i; j < m_Window.Length(); j += m_FFTLen)
            {
                m_WinFrame[i] += WData[j] * ((float)Frame[j] - (float)Frame[j-1]);
            }
    }

    // Otherwise, process without taking the first difference.  If necessary, pre-alias
    // as in the pre-emphasis case.
    else for (i = 0; i < m_FFTLen; i++)
            for (m_WinFrame[i] = 0.F, j = (int32)i; j < Window.Length(); j += m_FFTLen)
            {
                m_WinFrame[i] += WData[j] * (float)Frame[j];
            }
    return(DONE);
}


////////////////////////////////////////////////////////////////////////////////////////
// Object function to pre-process frame of unsigned byte sample data.                 //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t Spectrogram::PreProc(uint8 * Frame, DspWin & Window)
{
#define SBFrame(i)  (int8)(Frame[i] - 128)  //macro to convert from unsigned to signed byte

    uint16 i;
    int32 j;
    const float * WData = Window.WindowFloat();

    // If pre-emphasis requested, take first difference of sample data, then apply
    // window.
    if (m_PreEmphSw == true)
    {

        //  Initialize windowed frame value with first difference.
        if (Frame == m_SigStart)
        {
            m_WinFrame[0] = WData[0] * (float)SBFrame(0);    //first difference does not
        }
        //  apply to first sample in signal
        //produces slight error if first
        //  sample of signal non-zero
        else
        {
            m_WinFrame[0] = WData[0] * ((float)SBFrame(0) - (float)SBFrame(-1));
        }

        //   Pre-alias in the time domain, if necessary, to keep number of points required in the
        //   FFT to a minimum without undersampling in the frequency domain.  This will implement
        //   a bank of filters.
        for (j = (int32)m_FFTLen; j < Window.Length(); j += m_FFTLen)
        {
            m_WinFrame[0] += WData[j] * ((float)SBFrame(j) - (float)SBFrame(j-1));
        }
        for (i = 1; i < m_FFTLen; i++)
            for (m_WinFrame[i] = 0.F, j = (int32)i; j < Window.Length(); j += m_FFTLen)
            {
                m_WinFrame[i] += WData[j] * ((float)SBFrame(j) - (float)SBFrame(j-1));
            }
    }

    // Otherwise, process without taking the first difference.  If necessary, pre-alias
    // as in the pre-emphasis case.
    else for (i = 0; i < m_FFTLen; i++)
            for (m_WinFrame[i] = 0.F, j = (int32)i; j < Window.Length(); j += m_FFTLen)
            {
                m_WinFrame[i] += WData[j] * (float)SBFrame(j);
            }
    return(DONE);

#undef SBFrame
}


#define ENCODE_DPOWER_AS_DB(dBVal, dPower) \
{ \
    if ( dPower <= dMinPower) \
    dBVal = dDbMin;  \
   else \
{ \
    dBVal = 10.*log10(dPower); \
    if (dBVal > dDbMax) \
    dBVal = dDbMax; \
} \
}

////////////////////////////////////////////////////////////////////////////////////////
// Object function to compute power spectrum using a Fast Fourier Transform.          //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t Spectrogram::PwrFFT(uint8 * PwrSpect)
{
    typedef struct
    {
        double Max;    // max expected dB value
        double Min;    // min expected dB value
        double Scale;  // resolution = (Max - Min)/233., for colors ranging from
        //    0 to 233 (234 is reserved for formant tracks,
        //    235 for blank spectra)
    } DB_PARMS;

    static const DB_PARMS db[2][3] =    //determined experimentally
    {
        {   {0.,0.,0.},  //unused
            {77.75, 19.5, 0.25}, //8-bit sample data, no pre-emphasis
            {127.67, 69.42, 0.25}
        }, //16-bit data, no pre-emphasis
        {   {0.,0.,0.},  //unused
            //{71.64, -23.625, 0.25}, //8-bit sample data with pre-emphasis
            {56.5, -1.75, 0.25}, //8-bit sample data with pre-emphasis
            {104.75, 46.5, 0.25}
        }  //16-bit sample data with pre-emphasis
    };
    double dBVal;

    // Reference location where complex spectral values will be returned.
    COMPLEX_RECT_FLOAT * SpectCoeff = (COMPLEX_RECT_FLOAT *)m_WinFrame;

    // Calculate FFT on real data.

    rfft2f(m_WinFrame,m_FFTLen,FORWARD);

    // Determine number of bytes per sample from data format.
    uint8 SmpByteSize = (uint8)abs(m_SmpDataFmt);

    // Calculate power for spectral locations to be displayed, aligning them with nearest FFT
    // points.  Some error is introduced here, but true interpolation would be too costly.

    if (m_SpectLen + 1 == m_SpgmHgt && m_FreqScale == m_SpectScale)
    {
        const double dDbMin = db[m_PreEmphSw][SmpByteSize].Min;
        const double dDbMax = db[m_PreEmphSw][SmpByteSize].Max;
        const double dDbScale = 1./db[m_PreEmphSw][SmpByteSize].Scale;
        const double dMinPower = pow(10, dDbMin/10.);

        int32 i, j = 0;
        double dPower = SpectCoeff[0].imag * SpectCoeff[0].imag;

        ENCODE_DPOWER_AS_DB(dBVal, dPower);

        PwrSpect[j] = (uint8)((dBVal-dDbMin) * dDbScale + 0.5);
        m_MaxPwr = __max(m_MaxPwr , PwrSpect[j]);  //update max power computed for this spectrogram

        for (j = 1, i = m_SpectLen - j; j < m_SpectLen; i--, j++)
        {
            dPower = (SpectCoeff[i].real*SpectCoeff[i].real +
                      SpectCoeff[i].imag*SpectCoeff[i].imag);

            ENCODE_DPOWER_AS_DB(dBVal, dPower);

            PwrSpect[j] = (uint8)((dBVal-dDbMin) * dDbScale + 0.5);
            m_MaxPwr = __max(m_MaxPwr , PwrSpect[j]);  //update max power computed for this spectrogram
        }

        dPower = SpectCoeff[0].real * SpectCoeff[0].real;

        ENCODE_DPOWER_AS_DB(dBVal, dPower);

        PwrSpect[j] = (uint8)((dBVal-dDbMin) * dDbScale + 0.5);
        m_MaxPwr = __max(m_MaxPwr , PwrSpect[j]);  //update max power computed for this spectrogram

    }
    else
    {
        const double dDbMin = db[m_PreEmphSw][SmpByteSize].Min;
        const double dDbMax = db[m_PreEmphSw][SmpByteSize].Max;
        const double dDbScale = 1./db[m_PreEmphSw][SmpByteSize].Scale;
        const double dMinPower = pow(10, dDbMin/10.);

        uint16 i, j = 0;
        float Freq = m_UprFreq;

        double dPower = SpectCoeff[0].imag * SpectCoeff[0].imag;
        for (; j < m_SpgmHgt; Freq -= m_FreqScale, j++)
        {
            i = (uint16)(Freq/m_SpectScale + 0.5F);  //set to nearest FFT point

            if (i != m_SpectLen)
            {
                break;
            }
            ENCODE_DPOWER_AS_DB(dBVal, dPower);

            PwrSpect[j] = (uint8)((dBVal-dDbMin) * dDbScale + 0.5);
            m_MaxPwr = __max(m_MaxPwr , PwrSpect[j]);  //update max power computed for this spectrogram
        }

        for (; j < m_SpgmHgt; Freq -= m_FreqScale, j++)
        {
            i = (uint16)(Freq/m_SpectScale + 0.5F);  //set to nearest FFT point
            if (i==0)
            {
                break;
            }
            dPower = (SpectCoeff[i].real*SpectCoeff[i].real +
                      SpectCoeff[i].imag*SpectCoeff[i].imag);

            ENCODE_DPOWER_AS_DB(dBVal, dPower);

            PwrSpect[j] = (uint8)((dBVal-dDbMin) * dDbScale + 0.5);
            m_MaxPwr = __max(m_MaxPwr , PwrSpect[j]);  //update max power computed for this spectrogram
        }

        for (; j < m_SpgmHgt; Freq -= m_FreqScale, j++)
        {
            i = 0;  //set to nearest FFT point
            dPower = SpectCoeff[0].real * SpectCoeff[0].real;

            ENCODE_DPOWER_AS_DB(dBVal, dPower);

            PwrSpect[j] = (uint8)((dBVal-dDbMin) * dDbScale + 0.5);
            m_MaxPwr = __max(m_MaxPwr , PwrSpect[j]);  //update max power computed for this spectrogram
        }
    }

    return(DONE);
}

////////////////////////////////////////////////////////////////////////////////////////
// Object function to track formants from spectral data.                              //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t Spectrogram::CalcFormants(void)
{
    // Ensure call is valid.
    if (m_Status == SPGM_NOT_READY)
    {
        return(Code(OUT_OF_SEQUENCE));    //if not ready, exit
    }
    if (m_FmntSetToProc == m_SpectCnt)                                   //if already done, exit
    {
        return((int32)END_FMNT_CALC);
    }

    // Set number of formant sets to compute.
    uint32 NumFmntSets;
    if (m_FmntSetToProc+(uint32)m_SpectBatchLength > m_SpectCnt)
    {
        NumFmntSets = m_SpectCnt - m_FmntSetToProc;    //if last batch, calculate no. of
    }
    //  formant sets left
    else
    {
        NumFmntSets = (uint32)m_SpectBatchLength;    //otherwise, set to default value
    }


    // Call function to search for formants based on sample data format.
    dspError_t dspError_t = 0;
    switch (m_SmpDataFmt)
    {
    case PCM_2SSHORT:     // 2's complement short (2-byte) PCM
        dspError_t = SeekFormants((short *)m_SigStart+m_SigBlkOffset, NumFmntSets);
        if (dspError_t)
        {
            return(dspError_t);
        }
        break;
    case PCM_UBYTE:       // unsigned byte
        dspError_t = SeekFormants((uint8 *)m_SigStart+m_SigBlkOffset, NumFmntSets);
        if (dspError_t)
        {
            return(dspError_t);
        }
        break;
    }

    // Return.
    if (m_FmntSetToProc != m_SpectCnt)
    {
        return(PENDING);    //indicate formant tracking still incomplete
    }
    return((int32)END_FMNT_CALC);                     //indicate formant tracking done
}

////////////////////////////////////////////////////////////////////////////////////////
// Private object function to search for formants in acoustic waveform                //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t Spectrogram::SeekFormants(short * BlockStart, uint32 NumFmntSets)
{
    // Calculate addresses where first and last formant sets will be stored.
    FORMANT_FREQ * FirstFmntSet = m_FmntData + m_FmntSetToProc;
    FORMANT_FREQ * LastFmntSet = FirstFmntSet + NumFmntSets - 1;

    // Generate formant sets.
    for (FORMANT_FREQ * FmntSet = FirstFmntSet;
            FmntSet <= LastFmntSet;
            FmntSet++, m_FmntSetToProc++)
    {
        //   Calculate address for beginning of sample data frame.  Formants will be computed
        //   at the center of the frame.
        short * Frame = BlockStart + (uint32)(m_FmntSetToProc*m_SpectIntv+0.5F) - (uint32)m_NBWindow.Length()/2;

        //   If frame data invalid, emit blank formant sets.
        if (Frame < m_SigStart ||
                Frame > (short *)m_SigStart + m_SigLength - m_NBWindow.Length())
        {
            BlankFmntSet(FmntSet);
        }

        //   Otherwise, preprocess frame and compute power spectrum.
        else
        {
            dspError_t dspError_t = PreProc(Frame, m_NBWindow);   //  apply narrowband window
            if (dspError_t)
            {
                return(dspError_t);
            }
            rfft2f(m_WinFrame,m_FFTLen,FORWARD);  //  and calculate spectral values
            dspError_t = GetFormants(FmntSet);       //  find the formants if any
            if (dspError_t)
            {
                return(dspError_t);
            }
        }
    }

    return(DONE);
}

////////////////////////////////////////////////////////////////////////////////////////
// Private object function to search for formants in acoustic waveform                //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t Spectrogram::SeekFormants(uint8 * BlockStart, uint32 NumFmntSets)
{
    // Calculate addresses where first and last formant sets will be stored.
    FORMANT_FREQ * FirstFmntSet = m_FmntData + m_FmntSetToProc;
    FORMANT_FREQ * LastFmntSet = FirstFmntSet + NumFmntSets - 1;

    // Generate formant sets.
    for (FORMANT_FREQ * FmntSet = FirstFmntSet;
            FmntSet <= LastFmntSet;
            FmntSet++, m_FmntSetToProc++)
    {
        //   Calculate address for beginning of sample data frame.  Formants will be computed
        //   at the center of the frame.
        uint8 * Frame = BlockStart + (uint32)(m_FmntSetToProc*m_SpectIntv+0.5F) - (uint32)m_NBWindow.Length()/2;

        //   If frame data invalid, emit blank formant sets.
        if (Frame < m_SigStart ||
                Frame > (uint8 *)m_SigStart + m_SigLength - m_NBWindow.Length())
        {
            BlankFmntSet(FmntSet);
        }

        //   Otherwise, preprocess frame and compute power spectrum.
        else
        {
            dspError_t dspError_t = PreProc(Frame, m_NBWindow);   //  apply narrowband window
            if (dspError_t)
            {
                return(dspError_t);
            }
            rfft2f(m_WinFrame,m_FFTLen,FORWARD);  //  and calculate spectral values
            dspError_t = GetFormants(FmntSet);       //  find the formants if any
            if (dspError_t)
            {
                return(dspError_t);
            }
        }
    }

    return(DONE);
}

////////////////////////////////////////////////////////////////////////////////////////
// Private object function to blank out formant array for synchronization with        //
// the acoustic waveform.                                                             //
////////////////////////////////////////////////////////////////////////////////////////
void Spectrogram::BlankFmntSet(FORMANT_FREQ * Fmnt)
{
    for (uint16 i = 0; i <= MAX_NUM_FORMANTS; i++)
    {
        Fmnt->F[i] = (float)NA;
    }
    return;
}

////////////////////////////////////////////////////////////////////////////////////////
// Private object function to extract formants from spectral data.                    //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t Spectrogram::GetFormants(FORMANT_FREQ * fmnt)
{
    // Check for valid state.

    COMPLEX_RECT_FLOAT * spectCoeff = (COMPLEX_RECT_FLOAT *)m_WinFrame;
    uint32 i;

#define logMagSpectrum spectCoeff


    // Cepstrum smoothing
    // Compute log of FFT magnitude and store in real portion of complex array, zeroing out
    // imaginary portion.

    //!!log instead of log10?
    logMagSpectrum[0].real = (spectCoeff[0].real == 0.F) ? MIN_LOG_PWR :
                             (float)log10(fabs((double)spectCoeff[0].real));  //value at 0

    logMagSpectrum[0].imag = (spectCoeff[0].imag == 0.F) ? MIN_LOG_PWR :
                             (float)log10(fabs((double)spectCoeff[0].imag));  //value at SpectLen

    for (i = 1; i < m_SpectLen; i++)
    {
        logMagSpectrum[i].real = (spectCoeff[i].real == 0.F && spectCoeff[i].imag == 0.F) ?
                                 MIN_LOG_PWR : 0.5F*(float)log10(spectCoeff[i].real*
                                         spectCoeff[i].real + spectCoeff[i].imag*spectCoeff[i].imag);
        logMagSpectrum[i].imag = 0.F;
    }


    // Calculate cepstrum from log spectrum by taking inverse FFT.  As a real-to-complex FFT,
    // the function will assume the real part of the spectral array is even while the imaginary
    // part is odd.  Since the imaginary part has been forced to zero, producing a real spectral
    // array, the inverse FFT (cepstrum) will be both real and even.

    //!!cos transform for 1st few cepstral coeff and
    //cos transform for pitch peaks may be faster
    rfft2f((float *)logMagSpectrum,m_FFTLen,INVERSE);

    // Check for voicing by examining peak levels over a typical pitch period
    // range (20 ms down to 2 ms) in the high time portion of the cepstrum.

    // float *cepstralCoeff = (float *)logMagSpectrum;
#define cepstralCoeff m_WinFrame

    uint32 hiPeakLoc = 0;
    float hiPeakVal =   MIN_LOG_PWR;
    const float * hiPeakPtr = NULL;

    hiPeakPtr = FindHighestPeak(cepstralCoeff + m_MinPitchPeriod,
                                cepstralCoeff + m_MaxPitchPeriod, 0);
    if (hiPeakPtr != NULL)
    {
        hiPeakLoc = hiPeakPtr - cepstralCoeff;
        hiPeakVal = *hiPeakPtr;
        assert(hiPeakLoc < m_MaxPitchPeriod);
        assert(hiPeakLoc >= m_MinPitchPeriod);
        assert(hiPeakVal == cepstralCoeff[hiPeakLoc]);
    }

    // If the high peak is high enough, then this sound is voiced
    // and we record its time as the pitch period and calculate
    // the pitch frequency from it.  If it's too low, this sound
    // is unvoiced.
    if (hiPeakVal > MIN_PITCHPEAK_THD)   //!!MIN_PEAK_THD > FLT_MIN_NEG
    {
        fmnt->F[0] = (float)m_SmpRate/(float)hiPeakLoc;
    }
    else
    {
        fmnt->F[0] = UNVOICED;
    }

    // Remove excitation characteristic from high time portion.
    for (i = m_MinPitchPeriod; i <= m_SpectLen; i++)   //!!start at zero crossing?
    {
        cepstralCoeff[i] = cepstralCoeff[m_FFTLen-i] = 0.F;    //!!i > 0
    }

    // Compute FFT to get cepstrally smoothed spectrum.
    rfft2f(cepstralCoeff, m_FFTLen, FORWARD);  //!!cosine transform faster?

    // Pick peaks from cepstrally smoothed spectrum.

    // Create a magnitude spectrum from the cepstrum-smoothed spectrum.
    // Remember, the output of rfft2f is this funny thing with an exceptional
    // first element:
    // {{Re coeff[0], Re coeff[last]}, {Re coeff[1], Im coeff[1]}, ...}

    float * magnitudeSpectrum = new float[m_SpectLen+1];
    float x, y;

    magnitudeSpectrum[0] = spectCoeff[0].real*spectCoeff[0].real;
    magnitudeSpectrum[m_SpectLen] = spectCoeff[0].imag*spectCoeff[0].imag;

    for (i = 1; i < m_SpectLen; i++)
    {
        x = spectCoeff[i].real;
        y = spectCoeff[i].imag;
        magnitudeSpectrum[i] = x*x + y*y;
    }

    // Only look for formants up to 5000 Hz.
    const uint32 maxInterestingIndex = (uint32)(5000.0f/m_SpectScale);
    assert(maxInterestingIndex < m_SpectLen);

    uint32 numPeaksFound = FindHighEnergyPeaks(fmnt->F + 1,
                           magnitudeSpectrum, magnitudeSpectrum + maxInterestingIndex, m_NumFmnt, 0);

    delete [] magnitudeSpectrum;

    // Convert from indices to frequencies
    for (i = 1; i <= numPeaksFound; i++)
    {
        fmnt->F[i] *= m_SpectScale;
    }

    // Put zeros in slots for missing peaks.
    while (i < MAX_NUM_FORMANTS)
    {
        fmnt->F[i] = 0;
        i++;
    }

    return(DONE);
}

dspError_t Spectrogram::CalcPower(float * PowerInDb, float Frequency, float DspWinBandwidth, CWindowSettings::Type windowType, SIG_PARMS Signal, bool bPreEmphasis, float DbRef)
{

    // Calculate window for frame data from specified bandwidth.
    DspWin Window = DspWin::FromBandwidth(DspWinBandwidth,Signal.SmpRate, windowType);
    const float * WData = Window.WindowFloat();

    double FrameSample;  //!!double
    double RadianAngle = 2.*PI*(double)Frequency/(double)Signal.SmpRate;
    double CosTransform = 0., SinTransform = 0.;
    int32 n;

    //!!frame length less than window length?
    //!!doesnt average across pixels

    switch (Signal.SmpDataFmt)
    {
    case PCM_UBYTE:
        CosTransform = (double)((short)(((uint8 *)Signal.Start)[0] - 128))*WData[0];
        for (n = 1; n < Window.Length(); n++)
        {
            FrameSample = (double)((short)(((uint8 *)Signal.Start)[n] - 128) -
                                   bPreEmphasis*((short)(((uint8 *)Signal.Start)[n-1] - 128)))*
                          WData[n];  //!!check

            CosTransform += FrameSample * cos(RadianAngle*n);
            SinTransform += FrameSample * sin(RadianAngle*n);
        }

        break;
    case PCM_2SSHORT:
        CosTransform = (double)((short *)Signal.Start)[0]*WData[0];
        for (n = 1; n < Window.Length(); n++)
        {
            FrameSample = (double)(((short *)Signal.Start)[n] - bPreEmphasis*((short *)Signal.Start)[n-1])*
                          WData[n];
            CosTransform += FrameSample * cos(RadianAngle*n);
            SinTransform += FrameSample * sin(RadianAngle*n);
        }

        break;
    }
    *PowerInDb = (float)(10.*log10((CosTransform*CosTransform + SinTransform*SinTransform)/(double)Window.Length()) - DbRef);

    return(DONE);
}

dspError_t Spectrogram::CalcPower(float * PowerInDb, float Frequency, int32 BandwidthSelect, CWindowSettings::Type windowType, SIG_PARMS Signal, bool bPreEmphasis)
{
    // dB reference values determined empirically from 215.332 Hz sine wave (20th harmonic of 2048 FFT at 22050 Hz sampling rate)
    // at 70.7% full scale using waveform generator.
    static const double DbRef[2][3][3] =  // pre-emphasis off
    {
        //  8-bit    16-bit
        0., 56.7711, 74.6711,   // narrowband
        0., 54.    , 85.    ,   // mediumband (guess)
        0., 48.6510, 96.8988,   // wideband
        // pre-emphasis on
        //  8-bit    16-bit
        //0., 32.8843, 55.7011,   // narrowband
        0., 32.8843, 81.1259,   // narrowband
        0., 26.8450, 75.0776,   // mediumband
        0., 24.5982, 72.8105
    };  // wideband
    float Bandwidth[] = {NARROW_BW, MEDIUM_BW, WIDE_BW};
    dspError_t dspError_t = CalcPower(PowerInDb, Frequency, Bandwidth[BandwidthSelect], windowType, Signal, bPreEmphasis, (float) DbRef[bPreEmphasis][BandwidthSelect][abs(Signal.SmpDataFmt)]);
    return dspError_t;
}



