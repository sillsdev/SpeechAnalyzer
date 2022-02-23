/////////////////////////////////////////////////////////////////////////////
// sa_p_spu.cpp:
// Implementation of the CProcessSpectrum class.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include <math.h>
#include "sa_process.h"
#include "sa_p_spu.h"
#include "sa_p_fra.h"
#include "sa_p_grappl.h"
#include "SaParam.h"
#include "Lpc.h"
#include "param.h"
#include "ScopedCursor.h"
#include "ResearchSettings.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CProcessSpectrum
// class to calculate spectrum for wave data. The class creates an object
// of the class Spectrum that does the calculation.

CProcessSpectrum::CProcessSpectrum(Context & context) : CProcess(context) {
    m_nSpectralBands = 0;
    m_stBandPower.Max.Raw = m_stBandPower.Min.Raw = (float)UNDEFINED_DATA;
    m_stBandPower.Max.Smooth = m_stBandPower.Min.Smooth = (float)UNDEFINED_DATA;
    m_stBandPower.Max.Lpc = m_stBandPower.Min.Lpc = (float)UNDEFINED_DATA;
    m_dwFrameStart = UNDEFINED_OFFSET;   // to force processing first time
}

/***************************************************************************/
// CProcessSpectrum::Exit Exit on Error
// Standard exit function if an error occured.
/***************************************************************************/
long CProcessSpectrum::Exit(int nError, void * mem) {
    // free the raw data buffer
    if (mem) {
        delete [] mem;
    }
    SetDataInvalid();
    return MAKELONG(nError, 100);
}

/***************************************************************************/
// CProcessSpectrum::GetProcessedData Read processed data from spectrum process
// Gives back just the pointer to the data buffer for the processed spectrum
// data.
/***************************************************************************/
void * CProcessSpectrum::GetProcessedData(DWORD, BOOL) {
    // return pointer to data
    return m_lpBuffer;
}

/***************************************************************************/
// CProcessSpectrum::GetProcessedData Read processed data from spectrum process
// Returns one data of processed data from the spectrum data buffer. The user
// gives the data offset (data number) to tell what data he needs. He also
// gives a pointer to a BOOLEAN result variable, but this is just for
// compatibility with the base class. Processed data is always 16bit data!
// pCaller is for further use.
/***************************************************************************/
int CProcessSpectrum::GetProcessedData(DWORD dwOffset, BOOL *) {
    short int * lpData = (short int *)m_lpBuffer; // cast buffer pointer
    return *(lpData + dwOffset); // return data
}

/***************************************************************************/
// CProcessSpectrum::GetSpectralData Read spectral data from spectrum process
// Returns one element of processed data from the spectrum data buffer. The user
// gives the index (data number) to tell what data he needs.  Spectral values
// are returned in a structure containing raw, cepstrally-smoothed, and LPC data.
/***************************************************************************/
SSpectValue & CProcessSpectrum::GetSpectralData(unsigned short wIndex) {

    SSpectValue * lpData = (SSpectValue *)m_lpBuffer; // cast buffer pointer
    return (lpData[wIndex]); // return data
}

/***************************************************************************/
// CProcessSpectrum::GetFormants Returns pointer to formant frequencies
// and their log magnitudes, along with Lpc error.
/***************************************************************************/
SFormantFrame * CProcessSpectrum::GetFormants() {
    return (SFormantFrame *)(((SSpectValue *)m_lpBuffer) + m_nSpectralBands);
}

/***************************************************************************/
// CProcessSpectrum::GetFormant  Read formant data from spectrum process
// Returns one element of processed data from the formant data buffer. The user
// gives the index (data number) to tell what data he needs.  Formant values
// are returned in a structure containing cepstrally-smoothed and LPC derived
// formant frequency values.
/***************************************************************************/
SFormant & CProcessSpectrum::GetFormant(unsigned short wIndex) {
    SFormantFrame * lpData = GetFormants();
    return(lpData->Formant[wIndex]);
}

/***************************************************************************/
// CProcessSpectrum::GetSpectralRegionPower  Computes average power in
// specified region of spectrum, between wFreqLo and wFreqHi.
/***************************************************************************/
float CProcessSpectrum::GetSpectralRegionPower(Model * pModel, unsigned short wFreqLo, unsigned short wFreqHi) {

    if (m_lpBuffer==NULL) {
        return 0;
    }

    // get indexes to start and end frequencies
    DWORD dwSamplesPerSec = pModel->GetSamplesPerSec();
    uint32 nIndexLo = uint32((wFreqLo * MAX_FFT_LENGTH + (dwSamplesPerSec/2)) / dwSamplesPerSec);
    uint32 nIndexHi = uint32((wFreqHi * MAX_FFT_LENGTH + (dwSamplesPerSec/2)) / dwSamplesPerSec);
    double dfSumOfSpectra = 0;
    SSpectValue * pSpectralPower = (SSpectValue *)m_lpBuffer;

    // loop over specified region
    for (uint32 i = nIndexLo; i <= nIndexHi; i++) {
        // get spectral value & add to sum
        dfSumOfSpectra += pSpectralPower[i].Lpc;
    }

    // divide by total spectra
    WORD numberOfSpectralValues = WORD(nIndexHi - nIndexLo +1);

    float fAveragePower = float((dfSumOfSpectra + (numberOfSpectralValues /2)) / numberOfSpectralValues);
    fAveragePower = float((!fAveragePower)?MIN_LOG_PWR:10.*log10((double)fAveragePower) - GetSpectralPowerRange().fdBRef);
    return fAveragePower;
}

/***************************************************************************/
// CProcessSpectrum::Process Processing spectrum data
// The processed spectrum data is stored in the process data buffer. There is
// no temporary file created and no process bar displayed (just the wait
// cursor). The level tells which processing level this process has been
// called, start process start on which processing percentage this process
// starts (for the progress bar of other processed that call this one). The
// return value returns the highest level througout the calling queue, or -1
// in case of an error in the lower word of the long value and the end
// process progress percentage in the higher word. This function needs a
// pointer to the view instead the pointer to the document like other process
// calls. It calculates spectrum data.
/***************************************************************************/

long CProcessSpectrum::Process(void * pCaller, Model * pModel, DWORD dwFrameStart, DWORD dwFrameSize, SSpectProcSelect SpectraSelected, int nProgress, int nLevel) {
    if (IsCanceled()) {
        return Exit(PROCESS_CANCELED, NULL);    // process canceled
    }

    // Check to see if the spectrum needs to be recalculated.
    if (dwFrameStart != m_dwFrameStart ||
            dwFrameSize != m_dwFrameSize ||
            m_stParmSpec.nSmoothLevel != m_stParmProc.nSmoothLevel ||
            m_stParmSpec.nPeakSharpFac != m_stParmProc.nPeakSharpFac ||
            app.GetResearchSettings().window != m_stParmProc.GetWindow() ||
            SpectraSelected.bCepstralSpectrum != m_stSpectraProc.bCepstralSpectrum  ||
            SpectraSelected.bLpcSpectrum != m_stSpectraProc.bLpcSpectrum) {
        // must reprocess
        SetDataInvalid();
        m_dwFrameStart = dwFrameStart;
        m_dwFrameSize = dwFrameSize;
        m_stParmProc.nSmoothLevel = m_stParmSpec.nSmoothLevel;
        m_stParmProc.nPeakSharpFac = m_stParmSpec.nPeakSharpFac;
        m_stParmProc.SetWindow(app.GetResearchSettings().window);   // DSP window applied only for

        // cursors aligned to samples
        m_stSpectraProc.bCepstralSpectrum = SpectraSelected.bCepstralSpectrum;
        m_stSpectraProc.bLpcSpectrum = SpectraSelected.bLpcSpectrum;
    }

    if (IsDataReady()) {
        return MAKELONG(PROCESS_UNNECESSARY, 100);    // data is already ready
    }

    // Start the process, allocating a buffer for the processed data.
    CScopedCursor cursor(view);
    m_nSpectralBands = MAX_FFT_LENGTH / 2;   // should be high enough to ensure FFT resolution is greater than screen resolution
    //!!frame size must be less than FFT length (2 x nSpectralBands)
    m_nFormants = MAX_NUM_FORMANTS + 1;  // includes F[0], the fundamental frequency
    SetDataSize((DWORD)m_nSpectralBands * sizeof(SSpectValue) + sizeof(SFormantFrame));
    if (!StartProcess(pCaller, PROCESSSPU, (DWORD)GetDataSize(sizeof(char)))) {
        EndProcess(); // end data processing
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    // Get signal and frame parameters.
    DWORD wSmpSize = pModel->GetSampleSize();

    // Get fragments.  Processing should have been completed in the spectrum plot class at the very latest.
    CProcessFragments * pFragments = (CProcessFragments *)pModel->GetFragments();

    int nWindowSize = dwFrameSize / wSmpSize;
    int nWindowStart = dwFrameStart / wSmpSize;

    switch (m_stParmProc.GetWindow().lengthMode) {
    case kBandwidth:
        nWindowSize = CDspWin::CalcLength(m_stParmProc.GetWindow().bandwidth, pModel->GetSamplesPerSec(), m_stParmProc.GetWindow().type);
        break;
    case kTime:
        nWindowSize = (int)(0.001* m_stParmProc.GetWindow().time*pModel->GetSamplesPerSec() + 0.5);
    case kBetweenCursors:
    default:
        if (m_stParmProc.GetWindow().equivalentLength) {
            nWindowSize = CDspWin::CalcEquivalentLength(nWindowSize, m_stParmProc.GetWindow().type);
        }
        break;
    }

    nWindowStart -= nWindowSize/2;
    if (m_stParmProc.GetWindow().center) {
        nWindowStart += dwFrameSize / wSmpSize;
    }

    if (nWindowStart < 0) { // shift start of window
        nWindowStart = 0;
    }
    if ((nWindowStart+nWindowSize + 1) > int(pModel->GetDataSize()/wSmpSize)) {
        nWindowSize = (pModel->GetDataSize()/wSmpSize) - 1 - nWindowStart;
    }

    // Initialize frame structure.
    SSigParms stFrameParm;
    stFrameParm.Length = nWindowSize;
    stFrameParm.SmpRate = pModel->GetSamplesPerSec();
    if (wSmpSize == 1) {
        stFrameParm.SmpDataFmt = PCM_UBYTE;
    } else {
        stFrameParm.SmpDataFmt = PCM_2SSHORT;
    }
    stFrameParm.Source = (ESIG_SOURCE)pModel->GetGender();

    // Initialize spectrum settings.
    SSpectrumSettings stSpectSetting;
    stSpectSetting.wLength = m_nSpectralBands;
    stSpectSetting.nScaleSelect = LINEAR;  // save as linear values, convert in plot code to dB after consolidating each band
    stSpectSetting.fSmoothFreq = (float)(m_stParmProc.nSmoothLevel+1.F)*100.F;
    stSpectSetting.fFFTRadius= 1.F - (float)m_stParmProc.nPeakSharpFac/200.F;

    stSpectSetting.nWindowType = (char)m_stParmProc.GetWindow().type;

    // Allocate a temporary global buffer for the waveform frame data.  Large frames may consume a vast
    // amount of memory, and therefore, the frame size should be limited.  This is not much of a restriction,
    // however, since the spectrum of a large frame of data for a nonstationary signal such as speech is not very
    // useful anyway.
    stFrameParm.Start = (void *) new char[nWindowSize*wSmpSize];
    if (!stFrameParm.Start) {
        // memory lock error
        app.ErrorMessage(IDS_ERROR_MEMLOCK);
        EndProcess();
        return Exit(PROCESS_ERROR, NULL); // error, memory allocation
    }

    // Get frame data and fill up frame data buffer.
    DWORD dwOldWaveBufferIndex = pModel->GetWaveBufferIndex();  // save current buffer offset into waveform
    DWORD dwDataPos = nWindowStart*wSmpSize;
    DWORD dwFrameEnd = (nWindowStart + nWindowSize)*wSmpSize;
    BPTR pFrameData = (BPTR)stFrameParm.Start; // pointer to special raw data buffer
    while (dwDataPos < dwFrameEnd) { // processing loop
        BPTR pBlockData = pModel->GetWaveData(dwDataPos, TRUE); // get pointer to data block
        if (!pBlockData) {
            pModel->GetWaveData(dwOldWaveBufferIndex, TRUE);
            EndProcess();
            return Exit(PROCESS_ERROR, stFrameParm.Start); // error, reading failed
        }
        // copy this block
        DWORD dwBlockEnd = dwDataPos + GetProcessBufferSize();
        if (dwBlockEnd > dwFrameEnd) {
            dwBlockEnd = dwFrameEnd;
        }
        while (dwDataPos < dwBlockEnd) {
            *pFrameData++ = *pBlockData++;
            dwDataPos++;
        }
    }

    // Set the FFT length.  The FFT length must be greater than number of samples in the data frame in order to sample
    // the signal's Fourier transform adequately, i.e. without aliasing in the time domain.
    unsigned short nFFTLength = MAX_FFT_LENGTH;

    // Calculate standard spectrum if in static calculation mode.
    dspError_t Err;
    CSpectrum * poSpectrum = NULL;
    SSpectrumParms stSpectParm;
    memset(&stSpectParm,0,sizeof(stSpectParm));
    if (SpectraSelected.bCepstralSpectrum) {
        Err = CSpectrum::CreateObject(&poSpectrum, stSpectSetting, stFrameParm, nFFTLength);
        if (Err) {
            if (poSpectrum) {
                delete poSpectrum;
            }
        } else {
            stSpectParm = poSpectrum->GetSpectParms();
            m_stBandPower.fdBRef = stSpectParm.fdBRef;
        }
    }

    // Calculate LPC spectrum only if cursors aligned to a single voiced fragment.
    ULONG dwFrameStartIndex = nWindowStart;
    ULONG dwFragmentIndex = pFragments->GetFragmentIndex(dwFrameStartIndex);
    SFragParms stFragment = pFragments->GetFragmentParms(dwFragmentIndex);
    UNUSED_ALWAYS(stFragment);

    CProcessGrappl * pAutoPitch = pModel->GetGrappl();
    BOOL bVoiced = pAutoPitch->IsVoiced(pModel, nWindowStart*wSmpSize);

    CLinPredCoding * poLpcObject = NULL;
    SLPCModel * pstLpcModel = NULL;
    double LpcError = (double)UNDEFINED_DATA;

    if ((SpectraSelected.bLpcSpectrum) /*!! temporarily removed to study LPC results on various start positions
                                       && dwFrameStartIndex == stFragment.dwOffset && stFrameParm.Length == stFragment.wLength */) {
        // Lpc analysis done only on a single fragment

        //!!Should I take first few samples in frame to permit shifting by order?
        //!!Where is point of glottal closure?
        //!!Ensure frame length less than FFT length to prevent aliasing

        // Specify LPC settings.
        SLPCSettings stLpcSetting;

        stLpcSetting.Process.Flags = PRED_COEFF | GAIN | POWER_SPECTRUM | FORMANTS | PRED_SIGNAL | MEAN_SQ_ERR | ENERGY| WINDOW_SIGNAL;

        // stLpcSetting.nMethod = LPC_AUTOCOR;        //use autocorrelation LPC analysis
        // stLpcSetting.nMethod = LPC_CEPSTRAL;       //use cepstral LPC analysis
        // stLpcSetting.nMethod = LPC_COVAR_LATTICE;  //use covariance LPC analysis
        stLpcSetting.nMethod = uint8(app.GetResearchSettings().spectrumLpcMethod);

        //if (bRemoveDcBias)
        //    stLpcSetting.Process.Flags |= NO_DC_BIAS;

        if (bVoiced) {
            stLpcSetting.Process.Flags |= PRE_EMPHASIS;  // turn pre-emphasis on to remove effects of glottis and lip radiation
            DWORD dwBandwidth = (pModel->GetSignalBandWidth()==0)?pModel->GetSamplesPerSec()/2:pModel->GetSignalBandWidth();
            // allow 2 poles per kHz of signal bandwidth and reserve 4 for zero approximation
            stLpcSetting.nOrder = (unsigned char)(dwBandwidth * 2/1000 * app.GetResearchSettings().spectrumLpcOrderFsMult + app.GetResearchSettings().spectrumLpcOrderExtra);
        } else {
            DWORD dwBandwidth = (pModel->GetSignalBandWidth()==0)?pModel->GetSamplesPerSec()/2:pModel->GetSignalBandWidth();
            // allow 2 poles per kHz of signal bandwidth and reserve 4 for zero approximation
            stLpcSetting.nOrder = (unsigned char)(dwBandwidth * 2/1000 * app.GetResearchSettings().spectrumLpcOrderFsMult + app.GetResearchSettings().spectrumLpcOrderExtra);
        }
        stLpcSetting.nFrameLen = (unsigned short)stFrameParm.Length;
        stLpcSetting.fFFTRadius = stSpectSetting.fFFTRadius;


        // The order of the LPC model must be less than the frame length
        // Should be less than half the frame length
        if (stLpcSetting.nFrameLen < (USHORT)(stLpcSetting.nOrder*2)) {
            stLpcSetting.nOrder = UCHAR(stLpcSetting.nFrameLen >> 1);
        }

        // Construct an LPC object.
        Err = CLinPredCoding::CreateObject(&poLpcObject, app, stLpcSetting, stFrameParm, nFFTLength);

        // Perform LPC analysis.
        if (!Err) {
            Err = poLpcObject->GetLpcModel(&pstLpcModel, stFrameParm.Start);
        }
        if (bVoiced)
            for (int i = 0; i < app.GetResearchSettings().spectrumLpcOrderAuxMax; i+=2) {
                // try a few times until error drops below threshold
                if (Err) {
                    break;
                }
                LpcError = 100. * pstLpcModel->dMeanSqPredError / pstLpcModel->dMeanEnergy;
                if (LpcError < LPC_ERROR_THRESHOLD) {
                    break;
                }
                delete poLpcObject;
                stLpcSetting.nOrder += 2;  // increase the order by a pole pair
                Err = CLinPredCoding::CreateObject(&poLpcObject, app, stLpcSetting, stFrameParm, nFFTLength);
                if (Err) {
                    break;
                }
                Err = poLpcObject->GetLpcModel(&pstLpcModel, stFrameParm.Start);
            }
        if (!Err) {
            Err = poLpcObject->GetPowerSpectrum(stSpectSetting.wLength, stSpectSetting.nScaleSelect);
        }
        if (!Err && !poSpectrum) {
            m_stBandPower.fdBRef = poLpcObject->GetDecibelPowerRef();
        }
    }


    // If neither object could be constructed, quit.
    if (!poSpectrum && !poLpcObject) {
        pModel->GetWaveData(dwOldWaveBufferIndex, TRUE);       // restore waveform buffer
        EndProcess();
        return Exit(PROCESS_ERROR, stFrameParm.Start); // exit
    }

    // Consolidate spectral values in the process buffer.
    SSpectValue * pSpectralPower = (SSpectValue *)m_lpBuffer;
    SFormantFrame * pFormantFrame = (SFormantFrame *)(pSpectralPower + m_nSpectralBands);

    m_stBandPower.Max.Raw = m_stBandPower.Max.Smooth = m_stBandPower.Max.Lpc = FLT_MIN_NEG;
    m_stBandPower.Min.Raw = m_stBandPower.Min.Smooth = m_stBandPower.Min.Lpc = FLT_MAX;
    for (unsigned short i = 0; i < m_nSpectralBands; i++) {
        if (poSpectrum) {
            pSpectralPower[i].Raw = stSpectParm.pfRawSpectrum[i];
            m_stBandPower.Max.Raw = max(m_stBandPower.Max.Raw, pSpectralPower[i].Raw);
            m_stBandPower.Min.Raw = min(m_stBandPower.Min.Raw, pSpectralPower[i].Raw);
            pSpectralPower[i].Smooth = stSpectParm.pfSmoothSpectrum[i];
            m_stBandPower.Max.Smooth = max(m_stBandPower.Max.Smooth, pSpectralPower[i].Smooth);
            m_stBandPower.Min.Smooth = min(m_stBandPower.Min.Smooth, pSpectralPower[i].Smooth);
        } else {
            pSpectralPower[i].Raw = (float)UNDEFINED_DATA;
            pSpectralPower[i].Smooth = (float)UNDEFINED_DATA;
        }
        if (pstLpcModel) {
            pSpectralPower[i].Lpc = pstLpcModel->pPwrSpectrum[i];
            m_stBandPower.Max.Lpc = max(m_stBandPower.Max.Lpc, pSpectralPower[i].Lpc);
            m_stBandPower.Min.Lpc = min(m_stBandPower.Min.Lpc, pSpectralPower[i].Lpc);
        } else {
            pSpectralPower[i].Lpc = (float)UNDEFINED_DATA;
        }
    }

    if (m_stBandPower.Max.Raw == FLT_MIN_NEG) {
        m_stBandPower.Max.Raw = (float)UNDEFINED_DATA;
    }
    if (m_stBandPower.Min.Raw == FLT_MAX) {
        m_stBandPower.Min.Raw = (float)UNDEFINED_DATA;
    }
    if (m_stBandPower.Max.Smooth == FLT_MIN_NEG) {
        m_stBandPower.Max.Smooth = (float)UNDEFINED_DATA;
    }
    if (m_stBandPower.Min.Smooth == FLT_MAX) {
        m_stBandPower.Min.Smooth = (float)UNDEFINED_DATA;
    }
    if (m_stBandPower.Max.Lpc == FLT_MIN_NEG) {
        m_stBandPower.Max.Lpc = (float)UNDEFINED_DATA;
    }
    if (m_stBandPower.Min.Lpc == FLT_MAX) {
        m_stBandPower.Min.Lpc = (float)UNDEFINED_DATA;
    }

    const double MaxFormantBandwidth = 1000.F;   // determined empirically
    const int MinFormantFrequency = int(70.F);         // based on highpass cut-in frequency
    unsigned short j = 0;
    for (unsigned short i = 0; i <= MAX_NUM_FORMANTS; i++) {
        if (poSpectrum) {
            pFormantFrame->Formant[i].Cepstral.FrequencyInHertz = poSpectrum->GetSpectParms().pstFormant[i].FrequencyInHertz;
            pFormantFrame->Formant[i].Cepstral.BandwidthInHertz = (float)UNDEFINED_DATA;
            pFormantFrame->Formant[i].Cepstral.PowerInDecibels = poSpectrum->GetSpectParms().pstFormant[i].PowerInDecibels;
        } else {
            pFormantFrame->Formant[i].Cepstral.FrequencyInHertz = (float)UNDEFINED_DATA;
            pFormantFrame->Formant[i].Cepstral.BandwidthInHertz = (float)UNDEFINED_DATA;
            pFormantFrame->Formant[i].Cepstral.PowerInDecibels = FLT_MAX_NEG;
        }

        if (pstLpcModel) {
            pFormantFrame->Formant[j].Lpc.FrequencyInHertz = pstLpcModel->Formant[i].FrequencyInHertz;
            pFormantFrame->Formant[j].Lpc.BandwidthInHertz = pstLpcModel->Formant[i].BandwidthInHertz;
            pFormantFrame->Formant[j].Lpc.PowerInDecibels = pstLpcModel->Formant[i].PowerInDecibels;
            if (j > 0 && /*bVoiced &&*/ (pstLpcModel->Formant[i].BandwidthInHertz > MaxFormantBandwidth ||
                                         pFormantFrame->Formant[j].Lpc.FrequencyInHertz < MinFormantFrequency)) {
                continue;
            }
        } else {
            pFormantFrame->Formant[j].Lpc.FrequencyInHertz = (float)UNDEFINED_DATA;
            pFormantFrame->Formant[j].Lpc.BandwidthInHertz = (float)UNDEFINED_DATA;
            pFormantFrame->Formant[j].Lpc.PowerInDecibels = FLT_MAX_NEG;
        }
        j++;
    }

    for (; j <= MAX_NUM_FORMANTS; j++) {
        pFormantFrame->Formant[j].Lpc.FrequencyInHertz = (float)UNDEFINED_DATA;
        pFormantFrame->Formant[j].Lpc.BandwidthInHertz = (float)UNDEFINED_DATA;
        pFormantFrame->Formant[j].Lpc.PowerInDecibels = FLT_MAX_NEG;
    }

    if (pstLpcModel) {
        pFormantFrame->LpcErrorInPercent = LpcError;
    } else {
        pFormantFrame->LpcErrorInPercent = (double)UNDEFINED_DATA;
    }

    if (poSpectrum) {
		// delete the spectrum object
        delete poSpectrum;
    }
    if (poLpcObject) {
		// delete the Lpc object
        delete poLpcObject;
    }

    // free the temporary frame buffer
    delete [] stFrameParm.Start;
	// restore wave buffer
    pModel->GetWaveData(dwOldWaveBufferIndex, TRUE);
	// calculate the actual progress
    nProgress = nProgress + (int)(100 / nLevel);
	// update status bar with the progress and allow cancel (ESC key)
    SetProgress(nProgress);
    EndProcess(nProgress >= 100);
    SetDataReady();

    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);
    }

    return MAKELONG(nLevel, nProgress);
}

void CProcessSpectrum::SetSpectrumParms(CSpectrumParm * pParmSpec) {
    m_stParmSpec = *pParmSpec;
}

CSpectrumParm * CProcessSpectrum::GetSpectrumParms(void) {
    return &m_stParmSpec;
}

// return processed data size in spectrum data structures
DWORD CProcessSpectrum::GetDataSize() {
    return GetDataSize(sizeof(SSpectValue));
}

// return processed data size in LPC data structures
DWORD CProcessSpectrum::GetDataSize(size_t nElements) {
    return (DWORD)CProcess::GetDataSize(nElements);
}

unsigned short CProcessSpectrum::GetSpectralCount() {
    return m_nSpectralBands;
}

// return max and min of spectral power bands
SSpectPowerRange & CProcessSpectrum::GetSpectralPowerRange() {

    return m_stBandPower;
}

// returns number of formants calculated, including fundamental (F0)
unsigned short CProcessSpectrum::GetFormantCount() {
    return m_nFormants;
}
