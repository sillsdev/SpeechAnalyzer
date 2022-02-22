/////////////////////////////////////////////////////////////////////////////
// sa_p_rat.cpp:
// Implementation of the CProcessRatio class.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "sa_process.h"
#include "sa_p_rat.h"
#include "sa_p_cha.h"
#include "sa_p_lou.h"
#include "param.h"
#include "ScopedCursor.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CProcessRatio
// class to calculate ratio detector values for wave data.

/***************************************************************************/
// CProcessRatio::Process Processing change data
// The processed ratio data is stored in a temporary file. To create it
// helper functions of the base class are used. While processing a process
// bar, placed on the status bar, has to be updated. The level tells which
// processing level this process has been called, start process start on
// which processing percentage this process starts (for the progress bar).
// The status bar process bar will be updated depending on the level and the
// progress start. The return value returns the highest level througout the
// calling queue, or -1 in case of an error in the lower word of the long
// value and the end process progress percentage in the higher word.
/***************************************************************************/
long CProcessRatio::Process(void * pCaller, Model * pModel, int nProgress,
                            int nLevel) {
    //TRACE(_T("Process: CProcessRatio\n"));
    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    if (IsDataReady()) {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    }
    // create raw change data, its quality and smoothed loudness output is needed to calculate change
    CProcessChange * pChange = (CProcessChange *)pModel->GetChange(); // get pointer to raw change object
    CProcessSmoothLoudness * pLoudness = (CProcessSmoothLoudness *)pModel->GetSmoothLoudness(); // get pointer to smoothed loudness object
    long lResult = pChange->Process(pCaller, pModel, nProgress, ++nLevel); // process data
    nLevel = (short int)LOWORD(lResult);
    nProgress = HIWORD(lResult);

    if (nLevel < 0) { // previous processing error
        if ((nLevel == PROCESS_CANCELED)) {
            CancelProcess();    // set your own cancel flag
        }
        return MAKELONG(nLevel, nProgress);
    }

    // start change process
    CScopedCursor cursor(view);
    if (!StartProcess(pCaller, PROCESSRAT)) { // memory allocation failed
        EndProcess(); // end data processing
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    if (!pLoudness->SmoothData(2)) { // smooth smoothed loudness two times
        EndProcess(); // end data processing
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    const CUttParm * pUttParm = pModel->GetUttParm(); // get sa parameters utterance member data
    WORD nBlockAlign = pModel->GetBlockAlign();
    // calculate current processing position (copied but not understood from WINCECIL)
    DWORD dwLoopStart = ((pModel->GetSamplesPerSec() / (2 * MAX_CALCULATION_FREQUENCY)
                          + pModel->GetSamplesPerSec() / pUttParm->nMinFreq) / 2
                         + CALCULATION_DATALENGTH(pModel->GetSamplesPerSec()) / 2 + 5) / CALCULATION_INTERVAL(pModel->GetSamplesPerSec()) + 1;
    DWORD dwDataSize = pLoudness->GetDataSize();
    DWORD dwLoopEnd = dwDataSize - dwLoopStart; // end of loop
    // create the temporary change file
    if (!CreateTempFile(_T("RAT"))) { // creating error
        EndProcess(); // end data processing
        SetDataInvalid();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // process smoothed loudness and quality data into change data
    short int * pChangeData = (short int *)m_lpBuffer; // pointer to smoothed change data
    DWORD dwChangeCount = 0; // reset processed data counter
    DWORD dwLoopPos = 0;
    DWORD dwChangeBlock = 0;
    // prepare source data
    short int * pQuality = (short int *)pChange->GetQualityData(dwChangeBlock, TRUE); // read quality data block
    short int * pLoudData = (short int *)pLoudness->GetProcessedData(dwChangeBlock, TRUE); // read smoothed loudness data block
    if ((!pQuality) || (!pLoudData)) {
        return Exit(PROCESS_ERROR);    // error, reading failed
    }
    long lMultiplier = 1;
    if (nBlockAlign > 1) {
        lMultiplier = PRECISION_MULTIPLIER;    // 16 bit data, quality comes divided by PRECISION_MULTIPLIER
    }
    // start processing
    while (dwLoopPos < dwDataSize) {
        *pChangeData = (short)(((long)*pQuality++ * 200 * lMultiplier) / ((long)*pLoudData++ + 70) / lMultiplier);
        if ((dwLoopPos < dwLoopStart) || (dwLoopPos >= dwLoopEnd)) {
            *pChangeData = 0;
        }
        if (*pChangeData > m_nMaxValue) {
            m_nMaxValue = *pChangeData;    // save maximum value
        }
        pChange++;
        dwLoopPos++;
        // set progress bar
        SetProgress(nProgress + (int)(100 * dwLoopPos / dwDataSize / (DWORD)nLevel));
        if (IsCanceled()) {
            return Exit(PROCESS_CANCELED);    // process canceled
        }
        if ((++dwChangeCount == GetProcessBufferSize() / 2) || (dwLoopPos >= dwDataSize)) { // change buffer is full or processing finished
            // write the processed change data block
            try {
                Write(m_lpBuffer, dwChangeCount * 2);
            } catch (CFileException * e) {
                // error writing file
                app.ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
				// error, writing failed
				e->Delete();
				return Exit(PROCESS_ERROR);
            }
            pChangeData = (short int *)m_lpBuffer; // reset pointer to begin of change data buffer
            dwChangeBlock += dwChangeCount;
            dwChangeCount = 0; // reset processed data counter
            if (dwChangeCount < dwDataSize) {
                // read next source data blocks
                pQuality = (short int *)pChange->GetQualityData(dwChangeBlock, TRUE); // read quality data block
                pLoudData = (short int *)pLoudness->GetProcessedData(dwChangeBlock, TRUE); // read smoothed loudness data block
                if ((!pQuality) || (!pLoudData)) {
                    return Exit(PROCESS_ERROR);    // error, reading failed
                }
            }
        }
    }
    nProgress = nProgress + (int)(100 / nLevel); // calculate the actual progress
    // close the temporary change file and read the status
    CloseTempFile(); // close the file
    BOOL bRes = SmoothData(2); // smooth data two times
    if (bRes) {
        EndProcess((nProgress >= 95));    // end data processing
    } else {
        EndProcess();
    }
    SetDataReady(bRes);
    if (bRes) {
        return MAKELONG(nLevel, nProgress);
    } else {
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
}
