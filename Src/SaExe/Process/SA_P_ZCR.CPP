/////////////////////////////////////////////////////////////////////////////
// sa_p_zcr.cpp:
// Implementation of the CProcessZCross class.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
//
// Revision History
//   1.5Test8
//        SDM added changes by CLW in 1.07a
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Process.h"
#include "sa_p_zcr.h"
#include "sa_p_lou.h"

#include "isa_doc.h"
#include "resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CProcessZCross
// class to calculate zero crossings for wave data.

/////////////////////////////////////////////////////////////////////////////
// CProcessZCross construction/destruction/creation

/***************************************************************************/
// CProcessZCross::CProcessZCross Constructor
/***************************************************************************/
CProcessZCross::CProcessZCross() {
}

/***************************************************************************/
// CProcessZCross::~CProcessZCross Destructor
/***************************************************************************/
CProcessZCross::~CProcessZCross() {
}

/////////////////////////////////////////////////////////////////////////////
// CProcessZCross helper functions

/***************************************************************************/
// CProcessZCross::Process Processing zero crossing data
// The processed zero crossing data is stored in a temporary file. To
// create it helper functions of the base class are used. While processing a
// process bar, placed on the status bar, has to be updated. The level tells
// which processing level this process has been called, start process start
// on which processing percentage this process starts (for the progress bar).
// The status bar process bar will be updated depending on the level and the
// progress start. The return value returns the highest level througout the
// calling queue, or -1 in case of an error in the lower word of the long
// value and the end process progress percentage in the higher word.
/***************************************************************************/
long CProcessZCross::Process(void * pCaller, ISaDoc * pDoc, int nProgress,
                             int nLevel) {
    TRACE(_T("Process: CProcessZCross\n"));
    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    if (IsDataReady()) {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    }
    // create loudness data, it is needed to calculate zero crossing
    CProcessLoudness * pLoudness = (CProcessLoudness *)pDoc->GetLoudness(); // get pointer to loudness object
    long lResult = pLoudness->Process(pCaller, pDoc, nProgress, ++nLevel); // process data
    nLevel = (short int)LOWORD(lResult);
    nProgress = HIWORD(lResult);
    int nMaxLoud = pLoudness->GetMaxValue(); // get maximum value of loudness data

    if (nLevel < 0) { // previous processing error
        EndProcess(); // end data processing
        EndWaitCursor();
        if ((nLevel == PROCESS_CANCELED)) {
            CancelProcess();    // set your own cancel flag
        }
        return MAKELONG(nLevel, nProgress);
    }

    // start zero crossing process
    BeginWaitCursor(); // wait cursor
    if (!StartProcess(pCaller, IDS_STATTXT_PROCESSZCR)) { // memory allocation failed
        EndProcess(); // end data processing
        EndWaitCursor();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // create the temporary file
    if (!CreateTempFile(_T("ZCR"))) { // creating error
        EndProcess(); // end data processing
        EndWaitCursor();
        SetDataInvalid();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // process raw data into zero crossing data
    DWORD dwDataSize = pDoc->GetDataSize(); // size of raw data
    FmtParm * pFmtParm = pDoc->GetFmtParm(); // get sa parameters format member data
    UINT nSmpSize = pFmtParm->wBlockAlign / pFmtParm->wChannels;
    HPSTR pRawData = NULL, pBlockStart; // pointers to raw data
    short int * pZCrossData = (short int *)m_lpBuffer; // pointer to zero crossing data
    // set critical loudness range (10% of max loudness)
    int nMaxCriticalLoud = nMaxLoud / (10);
    int nMinCriticalLoud = -nMaxLoud / (10);
    // prepare processing loop
    UINT nCalcDataLength = CALCULATION_DATALENGTH(pFmtParm->dwSamplesPerSec) * pFmtParm->dwSamplesPerSec/22050;  //!!based on min pitch?
    UINT nCalcInterval = nCalcDataLength / 3;   // more than 2 x bandwidth
    DWORD dwDataEnd = dwDataSize - nCalcDataLength * nSmpSize; // end of loop
    DWORD dwDataPos = 0; // raw data position pointer
    UINT nBlockNumber = UINT(-1); // actual loaded data block number
    DWORD dwZCrossCount = 0; // reset processed data counter
    /*********************************************************************/
    // Added by CLW 9/21-25/98
    // Add zero at beginning to center frames
    /*********************************************************************/
    *pZCrossData++ = 0;
    dwZCrossCount++;
    /*********************************************************************/
    // start processing loop
    while (dwDataPos < dwDataEnd) {
        DWORD dwWindowEnd = dwDataPos + nCalcDataLength * nSmpSize; // window loop end
        int nZeroCount = 0; // counter for zero crossings
        int nOldData, nData; // data holder
        // prepare old data
        pBlockStart = pDoc->GetWaveData(dwDataPos); // get pointer to data block
        if (!pBlockStart) {
            return Exit(PROCESS_ERROR);    // error, reading failed
        }
        if ((dwDataPos / GetProcessBufferSize()) != nBlockNumber) {
            nBlockNumber = (UINT)(dwDataPos / GetProcessBufferSize()); // new block loaded
            pRawData = pBlockStart + dwDataPos % GetProcessBufferSize(); // CLW 9/28/98 (Mod)
        }
        if (nSmpSize == 1) { // 8 bit data
            // read a byte
            nOldData = (unsigned char)*pRawData - 128;
        } else { // 16 bit data
            // read a word
            short * pTemp = (short *)pRawData++;
            nOldData = (unsigned char)((*pTemp / 256) + 128) - 128;
            dwDataPos++;
        }
        pRawData++; // increment position in raw data
        dwDataPos++;
        // start window loop
        while (dwDataPos < dwWindowEnd) {
            /*********************************************************************/
            // Modified by CLW 9/28/98
            // New blocks loaded from beginning of first frame that extends past
            // the end of the buffer. This is done because the data pointer is backed
            // up to the start of the next frame. Previously, this meant backing
            // up into the previous buffer space. The result was frames with
            // zeroes at the beginning.
            /*********************************************************************/
            /*
            // does current frame extend past end of buffer?
            if (dwWindowEnd > (DWORD)pBlockStart + GetProcessBufferSize())
            { pBlockStart = pDoc->GetWaveData(dwDataPos, TRUE); // get pointer to data block
              if (!pRawData) return Exit(PROCESS_ERROR); // error, reading failed
              nBlockNumber++; // new block loaded
              pRawData = pBlockStart;
            }
            ********************************************************************
            */
            pBlockStart = pDoc->GetWaveData(dwDataPos); // get pointer to data block
            if (!pBlockStart) {
                return Exit(PROCESS_ERROR);    // error, reading failed
            }
            if ((dwDataPos / GetProcessBufferSize()) != nBlockNumber) {
                nBlockNumber = (UINT)(dwDataPos / GetProcessBufferSize()); // new block loaded
                pRawData = pBlockStart + dwDataPos % GetProcessBufferSize(); // CLW 9/28/98 (Mod)
            }
            // read data
            if (nSmpSize == 1) { // 8 bit data
                // read a byte
                nData = ((unsigned char)*pRawData - 128)*256; // Loudness is scaled assuming 16 bit samples
            } else { // 16 bit data
                // read a word
                short * pTemp = (short *)pRawData++;
                nData = *pTemp;
                dwDataPos++;
            }
            pRawData++; // increment position in raw data
            dwDataPos++;
            // ignore background noise
            if ((nData < nMaxCriticalLoud) && (nData > nMinCriticalLoud)) {
                continue;
            }
            // if product < 0, the x-axis was crossed
            if ((nData * nOldData) < 0) {
                nZeroCount++;
            }
            nOldData = nData;
        }
        // zero crossings result is ready, store it
        *pZCrossData++ = short(nZeroCount); // save result
        if (nZeroCount > m_nMaxValue) {
            m_nMaxValue = nZeroCount;    // save maximum value
        }
        // set new calculation window start
        pRawData -= (nCalcDataLength - ZCROSS_SAMPLEWINDOWWIDTH(pFmtParm->dwSamplesPerSec)) * nSmpSize;
        dwDataPos -= (nCalcDataLength - ZCROSS_SAMPLEWINDOWWIDTH(pFmtParm->dwSamplesPerSec)) * nSmpSize;
        /*********************************************************************/
        // Added by CLW 9/21-25/98
        // fill zeroes at end of data
        /*********************************************************************/
        while ((dwDataPos > dwDataEnd) && (dwDataPos < dwDataSize - nCalcInterval)) {
            *pZCrossData++ = 0;
            dwZCrossCount++;
            dwDataPos += nCalcInterval * nSmpSize;
        }
        /*********************************************************************/
        // set progress bar
        SetProgress(nProgress + (int)(100 * dwDataPos / dwDataSize / (DWORD)nLevel));
        if (IsCanceled()) {
            return Exit(PROCESS_CANCELED);    // process canceled
        }
        if ((++dwZCrossCount == GetProcessBufferSize() / 2) || (dwDataPos >= dwDataEnd)) { // zero crossing buffer is full or processing finished
            // write the processed data block
            try {
                Write(m_lpBuffer, dwZCrossCount * 2);
            } catch (CFileException e) {
                // error writing file
                ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
                return Exit(PROCESS_ERROR); // error, writing failed
            }
            pZCrossData = (short int *)m_lpBuffer; // reset pointer to begin of zero crossing data buffer
            dwZCrossCount = 0; // reset processed data counter
        }
    }
    // calculate the actual progress
    nProgress = nProgress + (int)(100 / nLevel);
    // close the temporary file and read the status
    CloseTempFile(); // close the file
    EndProcess((nProgress >= 95)); // end data processing
    EndWaitCursor();
    SetDataReady();
    return MAKELONG(nLevel, nProgress);
}
