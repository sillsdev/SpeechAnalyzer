// FormantTracker.cpp: implementation of the CFormantTracker class.
//
// Author: Steve MacLean
// copyright 2003 SIL
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_doc.h"
#include "Process.h"
#include "FormantTracker.h"
#include "Process/TrackState.h"
#include "FileUtils.h"
#include "StringUtils.h"
#include "Process/AnalyticLpcAnalysis.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const double pi = 3.14159265358979323846264338327950288419716939937511;

CProcessFormantTracker::CProcessFormantTracker(CProcess & Real, CProcess & Imag, CProcess & Pitch) {
    // real is the raw audio data
    m_pReal = &Real;
    // imaginary is the hilbert data
    m_pImag = &Imag;
    // pitch is the grappl process data
    m_pPitch = &Pitch;
}

CProcessFormantTracker::~CProcessFormantTracker() {

}

long CProcessFormantTracker::Process(void * pCaller, ISaDoc * pDoc, int nProgress, int nLevel) {
    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }

    int nOldLevel = nLevel;         // save original level
    int nOldProgress = nProgress;   // save original progress

    if ((m_pReal == NULL) || (m_pImag == NULL) || (m_pPitch == NULL)) {
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    if (!m_pReal->IsDataReady()) {
        SetDataInvalid();
        long lResult = m_pReal->Process(pCaller, pDoc, nProgress, ++nLevel);
        nLevel = (short int)LOWORD(lResult);
        nProgress = HIWORD(lResult);
    }

    if ((nLevel >= 0) && !m_pImag->IsDataReady()) {
        SetDataInvalid();
        long lResult = m_pImag->Process(pCaller, pDoc, nProgress, ++nLevel);
        nLevel = (short int)LOWORD(lResult);
        nProgress = HIWORD(lResult);
    }

    if ((nLevel >= 0) && (!m_pPitch->IsDataReady())) {
        SetDataInvalid();
        for (int i = 0; (i < 3) && (!m_pPitch->IsDataReady()); i++) { // make sure pitch is finished!
            long lResult = m_pPitch->Process(pCaller, pDoc, nProgress, ++nLevel);
            nLevel = (short int)LOWORD(lResult);
            nProgress = HIWORD(lResult);
        }
    }

    if ((nLevel == nOldLevel) && (IsDataReady())) {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    }

    nLevel = nOldLevel; // reset level and progress so progress bar is more realistic
    nProgress = nOldProgress;

    // get source data size
    DWORD dwDataSize = m_pReal->GetDataSize();
    TRACE("dwDataSize=%lu\n",dwDataSize);

    if (nLevel < 0) { // memory allocation failed or previous processing error
        if ((nLevel == PROCESS_CANCELED)) {
            CancelProcess();    // set your own cancel flag
        }
        return MAKELONG(nLevel, nProgress);
    }

    // start process
    BeginWaitCursor(); // wait cursor
    if (!StartProcess(pCaller, IDS_STATTXT_PROCESSFMT)) { // start data processing
        EndProcess(); // end data processing
        EndWaitCursor();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    // create the temporary file
    if (!CreateTempFile(_T("FT"))) { // creating error
        EndProcess(); // end data processing
        EndWaitCursor();
        SetDataReady(FALSE);
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    DWORD dwDataPos = 0; // data position pointer

    STrackState state;

    // Set the initial tracking estimates
    double samplingRate = pDoc->GetSamplesPerSec();
    double bandwidthInHertz = 250;
    double radius = exp(-bandwidthInHertz/samplingRate*pi);

    int formants = int(samplingRate/2000);
    state.trackIn.push_back(std::polar(radius, double(0)));
    state.trackOut.push_back(std::polar(radius, double(0)));
    for (int i=0; i < formants; i++) {
        double frequency = 2*pi*(i*1000+500)/samplingRate;
        CDBL formantEstimate(std::polar(radius, frequency));
        state.trackIn.push_back(formantEstimate);
        state.trackOut.push_back(formantEstimate);
    }

    // create the window
    CDspWin window = CDspWin::FromBandwidth(formantTrackerOptions.m_dWindowBandwidth, pDoc->GetSamplesPerSec(), formantTrackerOptions.m_nWindowType);
    state.window.assign(window.WindowDouble(),window.WindowDouble()+window.Length());

    // determine processing interval
    DWORD dwInterval = DWORD(samplingRate / formantTrackerOptions.m_dUpdateRate);

    //Determine initial data preload
    DWORD dwInitial = ((dwDataSize % dwInterval) + window.Length() + dwInterval)/2;

    // preload the data deque with zeroes
    state.data.resize(state.window.size(), 0);

    AdvanceData(state, dwDataPos, dwInitial);

    while (dwDataPos < dwDataSize) {
        // set progress bar
        SetProgress(nProgress + (int)(100 * dwDataPos / dwDataSize / (DWORD)nLevel));
        if (IsCanceled()) {
            return Exit(PROCESS_CANCELED);    // process canceled
        }

        BOOL bRes = TRUE;
        int pitch = m_pPitch->GetProcessedData((DWORD)(dwDataPos / Grappl_calc_intvl), &bRes) / PRECISION_MULTIPLIER;

        if (!BuildTrack(state, samplingRate, pitch)) {
            return Exit(PROCESS_CANCELED);
        }
        WriteTrack(state, samplingRate, pitch);

        state.trackIn = state.trackOut;

        AdvanceData(state, dwDataPos + dwInitial, dwInterval);

        dwDataPos += dwInterval;
    }

    nProgress = nProgress + (int)(100 / nLevel); // calculate the actual progress

    // close the temporary file and read the status
    CloseTempFile(); // close the file
    EndProcess((nProgress >= 95)); // end data processing
    EndWaitCursor();
    SetDataReady();
    return MAKELONG(nLevel, nProgress);
}

/**
* Bring in new data to be process.
* If nSamples exceeds the size of the data array, then dwDataPos
* is recalculated so that the data array is filled.
*
* The data array is treated as a deque.  Data is always added to the
* end.
*
* This routine does not due any processing of the data.
*
* Real is the raw audio data
* Imaginary is the hilbert transform data
*
* @param[in,out] state the current state of the process
* @param[in] the current position
* @param[in] the number of samples to process.  can be negative
*
*/
void CProcessFormantTracker::AdvanceData(STrackState & state, DWORD dwDataPos, int nSamples) {
    //TRACE("advance %d %d\n",dwDataPos, nSamples);

    DWORD dwSize = m_pReal->GetDataSize();
    double fSizeFactor = dwSize / m_pImag->GetDataSize();
    DWORD dwImagPos = (DWORD)((double)dwDataPos / fSizeFactor);             // imaginary data position
    unsigned int nImagSmpSize = sizeof(short) / (unsigned int)fSizeFactor;  // imaginary data sample size

    if (nSamples > 0) {
        if (::abs(nSamples) > state.data.size()) {
            AdvanceData(state, dwDataPos + nSamples - state.data.size(), state.data.size());
        } else {
            state.data.erase(state.data.begin(), state.data.begin() + nSamples);
            for (int i=0; i< nSamples; i++) {
                double dReal = 0;
                double dImag = 0;
                if (dwDataPos+i < dwSize) {
                    dReal = *reinterpret_cast<short *>(m_pReal->GetProcessedObject(dwDataPos + i, sizeof(short)));
                    if (nImagSmpSize == 1) {
                        dImag = *reinterpret_cast<char *>(m_pImag->GetProcessedObject(dwImagPos + i, sizeof(char)));
                    } else {
                        dImag = *reinterpret_cast<short *>(m_pImag->GetProcessedObject(dwImagPos + i, sizeof(short)));
                    }
                }
                CDBL sample(dReal, dImag);
                state.data.push_back(sample);
            }
        }
    } else {
        if (::abs(nSamples) > state.data.size()) {
            AdvanceData(state, dwDataPos - nSamples + state.data.size(), state.data.size());
        } else {
            state.data.erase(state.data.end() - nSamples, state.data.end());

            for (int i=0; i< nSamples; i++) {
                double dReal = 0;
                double dImag = 0;
                if ((dwDataPos >= DWORD(i)) && (dwDataPos < dwSize)) {
                    dReal = *reinterpret_cast<short *>(m_pReal->GetProcessedObject(dwDataPos-i, sizeof(short), TRUE));
                    dImag = *reinterpret_cast<short *>(m_pImag->GetProcessedObject(dwDataPos-i, sizeof(short), TRUE));
                }
                CDBL sample(dReal, dImag);
                state.data.push_front(sample);
            }
        }
    }
}

/**
* Builds the formant trackes from the incoming raw, pitch and hilbert process data.
*
* @param[in,out] state the current process state
* @param[in] the wave form sample rate
* @param[in] the pitch value
* @returns false if the user cancels the process, otherwise true
*/
bool CProcessFormantTracker::BuildTrack(STrackState & state, double samplingRate, int pitch) {
    ASSERT(state.window.size() == state.data.size());

    size_t tracks = state.trackIn.size();
    state.trackOut.resize(tracks);

    size_t tracksCalculate = tracks < MAX_NUM_FORMANTS ? tracks : MAX_NUM_FORMANTS;

    size_t zeroFilterOrder = (state.trackIn.size() - 1)*(formantTrackerOptions.m_bAzfAddConjugateZeroes ? 2 : 1) + 1;
    state.filtered.resize(state.window.size() + zeroFilterOrder);

    if (state.windowed.size() != (state.window.size() + 2*zeroFilterOrder)) {
        state.windowed.clear();
        state.windowed.assign(state.window.size() + 2*zeroFilterOrder, 0);
    }

    for (int i = state.window.size() - 1; i >= 0 ; i--) {
        double w = state.window[i];
        CDBL d = state.data[i];
        CDBL r = w*d;
        state.windowed[i + zeroFilterOrder] = r;
    }

    double radiusAZF = exp(-formantTrackerOptions.m_dAzfMinBandwidth/samplingRate*pi);
    double radiusDTF = exp(-formantTrackerOptions.m_dDtfMinBandwidth/samplingRate*pi);

    double pitchAngle = 2.0 * pi * pitch / samplingRate;
    CDBL pitchTrack(cos(pitchAngle), sin(pitchAngle));

    for (size_t formant = 1; formant < tracksCalculate; formant++) {

        if (IsCanceled()) {
            return false;
        }

        // Build DTF
        CDBL denominator[] = { 1, ((state.trackIn[formant].imag() > 0) ? - (state.trackIn[formant]/std::abs(state.trackIn[formant]) * radiusDTF) : 0)};
        CZTransformGeneric<CDBL> DTF(1, NULL, denominator);

        // Build All Zero Filter
        CZTransformGeneric<CDBL> azf;

        // Put a zero at pitch so formant 1 doesn't track pitch
        if ((formantTrackerOptions.m_bAzfAddConjugateZeroes) && (pitchTrack.imag())) {
            CDBL numerator[] = { 1, -2 * pitchTrack.real() * radiusAZF, radiusAZF * radiusAZF };
            azf *= CZTransformGeneric<CDBL>(2, numerator, NULL);
        } else {
            CDBL numerator[] = { 1, -pitchTrack*radiusAZF };
            azf *= CZTransformGeneric<CDBL>(1, numerator, NULL);
        }

        for (size_t zero = 1; zero < tracks; zero++) {
            if (zero == formant) {
                continue;
            }

            CDBL track = ((zero < formant) && (formantTrackerOptions.m_bAzfMostRecent)) ? state.trackOut[zero] : state.trackIn[zero];
            if (formantTrackerOptions.m_bAzfDiscardLpcBandwidth) {
                track /= std::abs(track);
            }
            if ((formantTrackerOptions.m_bAzfAddConjugateZeroes) && (track.imag()!=0.0)) {
                CDBL numerator[] = { 1, -2*track.real() * radiusAZF, std::norm(track) * radiusAZF*radiusAZF };
                azf *= CZTransformGeneric<CDBL>(2, numerator, NULL);
            } else {
                CDBL numerator[] = { 1, -track };
                azf *= CZTransformGeneric<CDBL>(1, numerator, NULL);
            }
        }

        if (formantTrackerOptions.m_bAzfAddConjugateZeroes) {
            // The filter is real... CDBL*double is faster than CDBL*CDBL so this is an optimization
            // Flatten AZF
            state.zeroFilterDBL.resize(zeroFilterOrder+1);
            double temp = azf.Tick(1).real();
            //TRACE("temp=%f\n",temp);
            state.zeroFilterDBL[0] = temp;
            for (unsigned int z=1; z<=zeroFilterOrder; z++) {
                state.zeroFilterDBL[z] = azf.Tick(0).real();
            }

            // Apply AZF & DTF
            for (unsigned int i = zeroFilterOrder; i < state.windowed.size() ; i++) {
                CDBL z = 0;
                for (unsigned int j = 0; j <= zeroFilterOrder; j++) {
                    z += state.windowed[i-j]*state.zeroFilterDBL[j];
                }
                state.filtered[i - zeroFilterOrder] = DTF.Tick(z);
            }
        } else {
            // The filter is complex...
            // Flatten AZF
            state.zeroFilterCDBL.resize(zeroFilterOrder+1);
            state.zeroFilterCDBL[0] = azf.Tick(1);
            for (unsigned int z=1; z<zeroFilterOrder; z++) {
                state.zeroFilterCDBL[z] = azf.Tick(0);
            }

            // Apply AZF & DTF
            for (unsigned int i = zeroFilterOrder; i < state.windowed.size() ; i++) {
                CDBL z = 0;
                for (unsigned int j = 0; j <= zeroFilterOrder; j++) {
                    z+= state.windowed[i-j]*state.zeroFilterCDBL[j];
                }
                state.filtered[i - zeroFilterOrder] = DTF.Tick(z);
            }
        }

        // Calculate LPC
        CAnalyticLpcAnalysis lpc(state.filtered, 1);

        // Store Frequency & BW
        CDBL predictor = lpc.GetPredictor()[1];
        state.trackOut[formant] = predictor;
    }
    return true;
}

/**
* writes the pitch and formant data to the temporary file
* @param[in,out] state the current state of the process
* @param[in] samplingRate
* @param[in] pitch the pitch value to be written in the first array entry.
*/
void CProcessFormantTracker::WriteTrack(STrackState & state, double samplingRate, int pitch) {
    SFormantFreq formant;
    BOOL bIsDataValid = state.trackOut.size() && (pitch > 0);

    formant.F[0] = float(bIsDataValid ? atan2(state.trackOut[0].imag(), state.trackOut[0].real())*samplingRate/2/pi : UNDEFINED_DATA);
    for (unsigned int i=1; i< MAX_NUM_FORMANTS; i++) {
        bIsDataValid = (i < state.trackOut.size()) && (pitch > 0);
        formant.F[i] = float((bIsDataValid) ? (atan2(state.trackOut[i].imag(), state.trackOut[i].real())*samplingRate/2/pi) : UNDEFINED_DATA);
    }

    // write unvoiced formant frame
    Write((HPSTR)&formant, (UINT)sizeof(SFormantFreq));
}

/**
* CProcessFormantTracker::GetFormant Read formant data
* Reads a slice of processed data from the temporary file into the processed
* data buffer and returns the pointer to the data. The returned pointer
* points to a slice of formant data. pCaller is a pointer to the
* calling plot and enables this function to get the process index of the
* plot. nIndex is the horizontal index in the formant data.  The function
* returns NULL on error.
*/
SFormantFreq * CProcessFormantTracker::GetFormant(DWORD dwIndex) {
    // read the data
    size_t sSize = sizeof(SFormantFreq);
    return (SFormantFreq *)GetProcessedObject(dwIndex, sSize);
}
