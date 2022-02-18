/////////////////////////////////////////////////////////////////////////////
// sa_p_spu.cpp:
// Implementation of the CProcessPOA class.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "process.h"
#include "sa_p_poa.h"

#include "lpc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CProcessPOA
// class to calculate POA for wave data. The class creates an object
// of the class POA that does the calculation for a face diagram which
// shows the Place of Articulation in the vocal tract.

/***************************************************************************/
// CProcessPOA::Exit Exit on Error
// Standard exit function if an error occured.
/***************************************************************************/
long CProcessPOA::Exit(int nError, HANDLE) {
    // free the raw data buffer
    SetDataInvalid();
    pTarget->EndWaitCursor();
    return MAKELONG(nError, 100);
}

/***************************************************************************/
// CProcessPOA::GetProcessedData Read processed data from POA process
// Gives back just the pointer to the data buffer for the processed POA
// data.
/***************************************************************************/
void * CProcessPOA::GetProcessedData( DWORD, BOOL) {
    // return pointer to data
    return m_lpBuffer;
}

/***************************************************************************/
// CProcessPOA::GetProcessedData Read processed data from POA process
// Returns one data of processed data from the POA data buffer. The user
// gives the data offset (data number) to tell what data he needs. He also
// gives a pointer to a BOOLEAN result variable, but this is just for
// compatibility with the base class. Processed data is always 16bit data!
// pCaller is for further use.
/***************************************************************************/
int CProcessPOA::GetProcessedData(DWORD dwOffset, BOOL *) {
    short int * lpData = (short int *)m_lpBuffer; // cast buffer pointer
    return *(lpData + dwOffset); // return data
}

/***************************************************************************/
// CProcessPOA::Process Processing POA data
// The processed POA data is stored in the process data buffer. There is
// no temporary file created and no process bar displayed (just the wait
// curser). The level tells which processing level this process has been
// called, start process start on which processing percentage this process
// starts (for the progress bar of other processed that call this one). The
// return value returns the highest level throughout the calling queue, or -1
// in case of an error in the lower word of the long value and the end
// process progress percentage in the higher word. This function needs a
// pointer to the view instead the pointer to the document like other process
// calls. It calculates POA data.
/***************************************************************************/
long CProcessPOA::Process(void * pCaller, Model * pModel, DWORD dwStart, DWORD dwStop, int nProgress, int nLevel) {
    //TRACE(_T("Process: CProcessPOA\n"));
    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    if (IsDataReady()) {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    }

    if (nLevel < 0) { // previous processing error
        if ((nLevel == PROCESS_CANCELED)) {
            CancelProcess();    // set your own cancel flag
        }
        return MAKELONG(nLevel, nProgress);
    }

    pTarget->BeginWaitCursor();
    // memory allocation failed
    if (!StartProcess(pCaller, PROCESSPOA, FALSE)) {
        // end data processing
        EndProcess();
        pTarget->EndWaitCursor();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    // Get waveform and buffer parameters.
    DWORD dwWaveBufferSize = GetBufferSize();
    DWORD wSmpSize = pModel->GetSampleSize();         //compute sample size in bytes
    DWORD dwFrameSize = dwStop - dwStart + wSmpSize;

    // Set signal parameters.
    DWORD dwOldWaveBufferIndex = pModel->GetWaveBufferIndex();    // save current buffer offset into waveform
    SSigParms Signal;
    Signal.Start = (void *)pModel->GetWaveData(dwStart, TRUE);    //load sample
    //buffer starting
    //at begin cursor
    if (wSmpSize == 1) {
        Signal.SmpDataFmt = PCM_UBYTE;          //samples are unsigned 8 bit
    } else {
        Signal.SmpDataFmt = PCM_2SSHORT;        //samples are 2's complement 16 bit
    }

    DWORD   dwWaveSize = pModel->GetDataSize();
    if (dwStart + dwWaveBufferSize <= dwWaveSize) {
        Signal.Length = dwWaveBufferSize/wSmpSize;          //signal fills buffer
    } else {
        Signal.Length = (dwWaveSize - dwStart)/wSmpSize;    //partly fills
    }

    // Specify LPC settings.
    SLPCSettings LpcSetting;

    Signal.SmpRate = pModel->GetSamplesPerSec();              //set sample rate
    LpcSetting.Process.Flags = PRE_EMPHASIS | NORM_CROSS_SECT | MEAN_SQ_ERR | ENERGY;
    LpcSetting.nMethod = LPC_COVAR_LATTICE;                         //use covariance LPC analysis  //!!autocorrelation for vc modeling?
    LpcSetting.nOrder = MODEL_SECTION_COUNT;                        //!!19 sections
    LpcSetting.nFrameLen = (unsigned short)(dwFrameSize/wSmpSize);

    // Construct an LPC object for vocal tract modeling.
    CLinPredCoding * pLpcObject;
    dspError_t Err;

    Err = CLinPredCoding::CreateObject(&pLpcObject, pApp, LpcSetting, Signal);
    if (Err) {
        pModel->GetWaveData(dwOldWaveBufferIndex, TRUE);
        return Exit(PROCESS_ERROR, NULL);
    }

    // Perform LPC analysis.
    SLPCModel * pLpcModel;
    Err = pLpcObject->GetLpcModel(&pLpcModel, Signal.Start);

    if (Err) {
        pModel->GetWaveData(dwOldWaveBufferIndex, TRUE);
        return Exit(PROCESS_ERROR, NULL);
    }

    // Allocate global buffer for the processed data
    SetDataSize(sizeof(SVocalTractModel) + (pLpcModel->nNormCrossSectAreas-1) * sizeof(*pLpcModel->pNormCrossSectArea));
    if (!m_lpBuffer) { // not yet allocated
        m_lpBuffer = new char[GetDataSize(sizeof(char))];
        if (!m_lpBuffer) {
            pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
            SetDataSize(0);
            delete pLpcObject; // delete the Lpc object
            pModel->GetWaveData(dwOldWaveBufferIndex, TRUE);
            pTarget->EndWaitCursor();
            return MAKELONG(PROCESS_ERROR, nProgress);
        }
    }

    // Copy vocal tract model to data buffer;
    SVocalTractModel * pVocalTractModel = (SVocalTractModel *)m_lpBuffer;
    if (pLpcModel->dMeanEnergy) {
        pVocalTractModel->dErrorRatio = pLpcModel->dMeanSqPredError / pLpcModel->dMeanEnergy;
    } else {
        pVocalTractModel->dErrorRatio = (double)UNDEFINED_DATA;
    }
    pVocalTractModel->nNormCrossSectAreas = pLpcModel->nNormCrossSectAreas;
    for (USHORT i = 0; i < pVocalTractModel->nNormCrossSectAreas; i++) {
        pVocalTractModel->dNormCrossSectArea[i] = pLpcModel->pNormCrossSectArea[i];
    }

	// delete the Lpc object
    delete pLpcObject; 
	// restore wave buffer
    pModel->GetWaveData(dwOldWaveBufferIndex, TRUE);
	// calculate the actual progress
    nProgress = nProgress + (int)(100 / nLevel);
	// show progress and allow for cancel (ESC key)
    SetProgress(nProgress);
    EndProcess();
    SetDataReady();
    pTarget->EndWaitCursor();

    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);
    }

    return MAKELONG(nLevel, nProgress);
}

DWORD CProcessPOA::GetDataSize() {
    // return processed data size in LPC data structures
    return GetDataSize(sizeof(SLPCModel));
}

DWORD CProcessPOA::GetDataSize(size_t nElements) {
    // return processed data size in LPC data structures
    return CProcess::GetDataSize(nElements);
}
