/////////////////////////////////////////////////////////////////////////////
// sa_p_twc.cpp:
// Implementation of the CProcessTonalWeightChart class.
// Author: Corey Wenger
// copyright 2000 JAARS Inc. SIL
//
// TO DO:            1.
//                   2.
//                   3.
//
// Revision History
//
//  10/24/00
//       CLW removed SaveCursorPosition code.
//             This scheme does not work if waveform data is removed.
//             A better and simpler way is to start at 0 and process
//             to the end of the melogram data.
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "Process.h"
#include "sa_p_twc.h"
#include "sa_p_melogram.h"
#include "math.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CProcessTonalWeightChart
// class to calculate zero crossings for wave data.

/***************************************************************************/
// CProcessTonalWeightChart::CProcessTonalWeightChart Constructor
/***************************************************************************/
CProcessTonalWeightChart::CProcessTonalWeightChart(Context * pContext) : CProcess(pContext) {
    m_dwFrameSize = 0;
}

/***************************************************************************/
// CProcessTonalWeightChart::Process Processes tonal weighting chart data
// The processed tonal weighting chart data is stored in a temporary file. To
// create it helper functions of the base class are used. While processing a
// process bar, placed on the status bar, has to be updated. The level tells
// which processing level this process has been called, start process start
// on which processing percentage this process starts (for the progress bar).
// The status bar process bar will be updated depending on the level and the
// progress start. The return value returns the highest level througout the
// calling queue, or -1 in case of an error in the lower word of the long
// value and the end process progress percentage in the higher word.
/***************************************************************************/
long CProcessTonalWeightChart::Process(void * pCaller, Model * pModel, DWORD dwFrameStart, DWORD dwFrameSize, short nMinSemitone, short nMaxSemitone, int nProgress, int nLevel) {
    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }

    // create melogram data (need this to determine cursor position in melogram data)
    CProcessMelogram * pMelogram = (CProcessMelogram *)pModel->GetMelogram(); // get pointer to melogram object
    long lResult = pMelogram->Process(pCaller, pModel, nProgress, ++nLevel); // process data
    nLevel = (short int)LOWORD(lResult);
    if (nLevel == PROCESS_CANCELED) {
        return Exit(PROCESS_CANCELED);
    }
    nProgress = HIWORD(lResult);
    DWORD dwMelDataSize = pMelogram->GetDataSize() * 2; // size of melogram data
    if (!dwMelDataSize) {
        return Exit(PROCESS_NO_DATA);    // error, no valid data
    }

    // check if data needs to be recalculated
    if (dwFrameStart != m_dwFrameStart || dwFrameSize != m_dwFrameSize
            || m_dMinSemitone != nMinSemitone || m_dMaxSemitone != nMaxSemitone) { //check if cursors moved, requiring new spectrum
        // invalidate data
        if (IsDataReady()) {
            SetDataInvalid();
        }
        // save current cursor positions
        m_dwFrameStart = dwFrameStart;
        m_dwFrameSize = dwFrameSize;
    }
    if (IsDataReady()) {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    }

    //TRACE(_T("Process: CProcessTonalWeightChart\n"));

    if (nLevel < 0) { // previous processing error
        if ((nLevel == PROCESS_CANCELED)) {
            CancelProcess();    // set your own cancel flag
        }
        return MAKELONG(nLevel, nProgress);
    }

    // start tonal weighting chart process
    pTarget->BeginWaitCursor(); // wait cursor
    // to do: need new status bar text for TWC
    if (!StartProcess(pCaller, PROCESSTWC)) { // memory allocation failed
        EndProcess(); // end data processing
        pTarget->EndWaitCursor();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    // create the temporary file
    if (!CreateTempFile(_T("TWC"))) { // creating error
        EndProcess(); // end data processing
        pTarget->EndWaitCursor();
        SetDataInvalid();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    // prepare processing loop
    DWORD dwDataPos = dwFrameStart; // melogram data position pointer
    // prepare histogram parameters
    m_nBinsPerSemitone = 10;
    // CLW 1/18/01 Use semitones passed in the parameter list
    m_dMinSemitone = nMinSemitone;
    m_dMaxSemitone = nMaxSemitone;

    CHistogram * ppoHist = MakeTwcHistogram(pMelogram, m_nBinsPerSemitone, m_dMinSemitone, m_dMaxSemitone, dwFrameStart/2, dwFrameSize/2);
    if (!ppoHist) {
        {
            if (ppoHist)
            { delete ppoHist; 
                ppoHist = NULL; 
            }
        }        return Exit(PROCESS_ERROR); // error, reading failed
    }

    // set progress bar
    SetProgress(nProgress + (int)(100 * (dwDataPos - dwFrameStart) / dwFrameSize / (DWORD)nLevel));
    if (IsCanceled()) {
        if (ppoHist) {
 delete ppoHist; 
                ppoHist = NULL;
            }
            return Exit(PROCESS_CANCELED); // process canceled
    }

    double fCoeff = .001;
    dspError_t Err = ppoHist->GetHistogram((short int *)m_lpBuffer, fCoeff, PDF);
    if (Err) {
        if (ppoHist) {
 delete ppoHist; 
                ppoHist = NULL;
            }
            return Exit(PROCESS_ERROR); // error, reading failed
    }
    // Make max value 0 if the total number of counts is 0. CLW 9/26/00
    if (ppoHist->GetTotalCounts(this)) {
        m_nMaxValue = (int)((double)ppoHist->GetMaxValue(this) / fCoeff / (double)ppoHist->GetTotalCounts(this));
    } else {
        m_nMaxValue = 0;
    }

    // write the processed data block
    try {
        Write(m_lpBuffer, ppoHist->GetHistogramParms().nBins * 2);
    } catch (CFileException * e) {
        // error writing file
        pApp->ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
        if (ppoHist) {
 delete ppoHist; 
                ppoHist = NULL;
            }
            // error, writing failed
		e->Delete();
		return Exit(PROCESS_ERROR);
    }
    if (ppoHist)         {
 delete ppoHist; 
            ppoHist = NULL;
        }
        // calculate the actual progress
    nProgress = nProgress + (int)(100 / nLevel);
    // close the temporary file and read the status
    CloseTempFile(); // close the file
    EndProcess((nProgress >= 95)); // end data processing
    pTarget->EndWaitCursor();
    SetDataReady();
    return MAKELONG(nLevel, nProgress);
}

// Reusable function to make a CHistogram of a section of a wave file
// Assumes the CProcessMelogram data is ready (callers responsibility
// Creates a new CHistogram object, callers responsibility to delete
CHistogram * CProcessTonalWeightChart::MakeTwcHistogram(CProcessMelogram * pMelogram, int nBinsPerSemitone, double dMinSemitone, double dMaxSemitone, DWORD dwFrameStart, DWORD dwFrameSize) {
    // prepare histogram parameters
    CHistogram * ppoHist = NULL;
    SHistogramParms stHistParms;
    memset(&stHistParms,0,sizeof(SHistogramParms));

    stHistParms.nBins = short((dMaxSemitone - dMinSemitone) * (double)nBinsPerSemitone);
    stHistParms.pBinDivs = (short *)new short[stHistParms.nBins + 1];
    if (!stHistParms.pBinDivs) {
        return NULL;
    }
    double fSemitone;
    USHORT nBin;
    for (nBin = 0; nBin <= stHistParms.nBins; nBin++) {
        fSemitone = dMinSemitone + (double)nBin / (double)nBinsPerSemitone;
        stHistParms.pBinDivs[nBin] = (short)(fSemitone * MEL_MULT + 0.5);
    }
    // prepare process parameters
    SProcParms stProcParms;
    memset(&stProcParms,0,sizeof(SProcParms));
    stProcParms.dwBufferSize = GetBufferSize() / sizeof(SHORT);
    stProcParms.dwBlockOffset = dwFrameStart;
    stProcParms.dwBlockLength = dwFrameSize;
    DWORD dwDataPos = dwFrameStart;
    short int * pMelogramData = (short int *)pMelogram->GetProcessedData(dwDataPos, TRUE); // read melogram data block
    stProcParms.pBufferStart = pMelogramData;
    stProcParms.stSigParms.Length = pMelogram->GetDataSize();
    stProcParms.stSigParms.SmpDataFmt = PCM_2SSHORT;
    stProcParms.stSigParms.SmpRate = 0; // not applicable
    dspError_t Err = CHistogram::CreateObject(&ppoHist, stHistParms, stProcParms); // create histogram object
    delete [] stHistParms.pBinDivs;
    if (Err) {
        return NULL;
    }
    // process histogram
    while (dwDataPos < dwFrameStart + dwFrameSize - 1) {
        // load buffer with next block of melogram data
        DWORD dwBatchSize = stProcParms.dwBufferSize;
        if (dwDataPos + dwBatchSize > dwFrameStart + dwFrameSize) {
            dwBatchSize = dwFrameSize + dwFrameStart - dwDataPos;
        }
        short int * pMelogramData = (short int *)pMelogram->GetProcessedData(dwDataPos, TRUE); // read melogram data block
        if (!pMelogramData) {
            delete ppoHist;
            return NULL;
        }
        Err = ppoHist->GenerateHistogram();// process data in buffer
        dwDataPos += dwBatchSize;
        switch (Err) {
        case DONE:
            break;

        case HIST_BUFFER_CALLBACK:
            break;

        default:
            delete ppoHist;
            return NULL;
        }
    }

    return ppoHist;
}