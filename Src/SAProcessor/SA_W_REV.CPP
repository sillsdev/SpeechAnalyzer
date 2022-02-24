/////////////////////////////////////////////////////////////////////////////
// sa_w_rev.cpp:
// Implementation of the CProcessWbEcho
//             CProcessWbReverb classes.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "sa_process.h"
#include "sa_w_rev.h"
#include "math.h"
#include "WbProcess.h"
#include "ScopedCursor.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CProcessWbEcho
// Workbench process class to calculate new raw data with a echo function.

CProcessWbEcho::CProcessWbEcho(Context context) : CWbProcess(context, WbEcho) {
    m_nDelay = 100;
    m_nGain = -20;
}

/***************************************************************************/
// CProcessWbEcho::Process Processing new raw data with a echo function
// The processed change data is stored in a temporary file. To create it
// helper functions of the base class are used. While processing a process
// bar, placed on the status bar, has to be updated. The level tells which
// processing level this process has been called, start process start on
// which processing percentage this process starts (for the progress bar).
// The status bar process bar will be updated depending on the level and the
// progress start. The return value returns the highest level througout the
// calling queue, or -1 in case of an error in the lower word of the long
// value and the end process progress percentage in the higher word.
/***************************************************************************/
long CProcessWbEcho::Process(void * pCaller, int nProgress, int nLevel) {
    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    // check if nested workbench processes
    int nOldLevel = nLevel; // save original level
    CWbProcess * pLowerProcess = NULL;
    if (nLevel < MAX_FILTER_NUMBER) {
        pLowerProcess = main.GetWbProcess(model.GetWbProcess() - 1, nLevel);
        if (pLowerProcess) {
            // there is at least one more workbench processes to process first
            long lResult = pLowerProcess->Process(pCaller, nProgress, ++nLevel);
            nLevel = (short int)LOWORD(lResult);
            nProgress = HIWORD(lResult);
        }
    }
    if ((nLevel == nOldLevel) && (IsDataReady())) {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    } else {
        SetDataInvalid();
    }

    if (nLevel < 0) { // previous processing error
        if ((nLevel == PROCESS_CANCELED)) {
            CancelProcess();    // set your own cancel flag
        }
        return MAKELONG(nLevel, nProgress);
    }

    // start process
    // wait cursor
    CScopedCursor cursor(view);
    if (!StartProcess(pCaller, PROCESSWBECHO)) {
        // memory allocation failed
        // end data processing
        EndProcess();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // create the temporary file
    if (!CreateTempFile(_T("ECH"))) { // creating error
        EndProcess(); // end data processing
        SetDataInvalid();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // get source data size
    DWORD dwDataSize;
    if (pLowerProcess) {
        dwDataSize = pLowerProcess->GetProcessedWaveDataSize();
    } else {
        dwDataSize = model.GetDataSize();    // size of raw data
    }
    DWORD dwBufferSize = GetBufferSize();
    DWORD wSmpSize = model.GetSampleSize();
    BPTR pSourceData, pBlockStart; // pointers to source data
    BPTR pTargetData; // pointers to target data
    DWORD dwDataPos = 0; // data position pointer
    DWORD dwRangeStart = 0;
    DWORD dwRangeEnd = dwDataSize - wSmpSize;

    float fGain = (float)1.0 / (float)pow(10.0, (double)m_nGain / -20.0);
    DWORD dwFifoSize = model.GetSamplesPerSec() * m_nDelay / 1000;
    // alocate the temporary global buffer for the fifo
    HANDLE hFifo = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, dwFifoSize * 2); // fifo is 16 bit
    if (!hFifo) {
        // memory allocation error
        app.ErrorMessage(IDS_ERROR_MEMALLOC);
        return Exit(PROCESS_ERROR); // error, memory allocation
    }
    BPTR lpFifo = (BPTR)::GlobalLock(hFifo); // lock memory
    if (!lpFifo) {
        // memory lock error
        app.ErrorMessage(IDS_ERROR_MEMLOCK);
        ::GlobalFree(hFifo);
        return Exit(PROCESS_ERROR); // error, memory allocation
    }
    // initialize fifo with silence
    short int * pFifo = (short int *)lpFifo;
    for (DWORD dwFifoIndex = 0; dwFifoIndex < dwFifoSize; dwFifoIndex++) {
        pFifo[dwFifoIndex] = 0;
    }
    DWORD dwFifoIndex = 0;
    // start processing loop
    while (dwDataPos < dwDataSize) {
        // load source data
        if (pLowerProcess) {
            pBlockStart = pLowerProcess->GetProcessedWaveData(dwDataPos);
        } else {
            pBlockStart = model.GetAdjustedUnprocessedWaveData(dwDataPos);    // get pointer to raw data block
        }
        if (!pBlockStart) {
            ::GlobalUnlock(hFifo);
            ::GlobalFree(hFifo);
            return Exit(PROCESS_ERROR); // error, reading failed
        }
        pSourceData = pBlockStart;
        pTargetData = m_lpBuffer;
        DWORD dwBlockEnd = dwDataPos + dwBufferSize;
        if (dwBlockEnd > dwDataSize) {
            dwBlockEnd = dwDataSize;
            dwBufferSize = dwDataSize - dwDataPos;
        }
        int nData;
        BYTE bData;
        while (dwDataPos < dwBlockEnd) {
            // read data
            if (wSmpSize == 1) { // 8 bit data
                bData = *pSourceData++; // data range is 0...255 (128 is center)
                nData = bData - 128;
            } else {              // 16 bit data
                nData = *((short int *)pSourceData++);
                pSourceData++;
                dwDataPos++;
            }
            dwDataPos++;
            // process data
            if (wSmpSize == 1) {
                int nTempData = nData + (int)((float)(pFifo[dwFifoIndex]) * fGain);
                pFifo[dwFifoIndex] = (short)nData;
                if ((dwDataPos >= dwRangeStart) && (dwDataPos <= dwRangeEnd)) {
                    nData = nTempData;
                    if (nData > 127) {
                        nData = 127;
                    }
                    if (nData < -128) {
                        nData = -128;
                    }
                }
            } else {
                long lData = (long)nData + (long)((float)((long)pFifo[dwFifoIndex]) * fGain);
                pFifo[dwFifoIndex] = (short)nData;
                if ((dwDataPos >= dwRangeStart) && (dwDataPos <= dwRangeEnd)) {
                    if (lData > 32767) {
                        lData = 32767;
                    }
                    if (lData < -32767) {
                        lData = -32767;
                    }
                    nData = (int)lData;
                }
            }
            dwFifoIndex = ++dwFifoIndex % dwFifoSize;
            // save data
            if (wSmpSize == 1) { // 8 bit data
                bData = (BYTE)(nData + 128);
                *pTargetData++ = bData;
            } else {              // 16 bit data
                *pTargetData++ = (BYTE)nData;
                *pTargetData++ = (BYTE)(nData >> 8);
            }
            // set progress bar
            SetProgress(nProgress + (int)(100 * dwDataPos / dwDataSize / (DWORD)nLevel));
            if (IsCanceled()) {
                ::GlobalUnlock(hFifo);
                ::GlobalFree(hFifo);
                return Exit(PROCESS_CANCELED); // process canceled
            }
        }
        // result block is ready, store it
        if (dwDataPos >= dwBlockEnd) {
            // write the processed data block
            try {
                Write(m_lpBuffer, dwBufferSize);
            } catch (CFileException * e) {
                // error writing file
                ::GlobalUnlock(hFifo);
                ::GlobalFree(hFifo);
                app.ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
				// error, writing failed
				e->Delete();
				return Exit(PROCESS_ERROR);
            }
        }
    }
    // free the fifo buffer
    ::GlobalUnlock(hFifo);
    ::GlobalFree(hFifo);
    nProgress = nProgress + (int)(100 / nLevel); // calculate the actual progress
    // close the temporary file and read the status
    CloseTempFile(); // close the file
    EndProcess((nProgress >= 95)); // end data processing
    SetDataReady();
    return MAKELONG(nLevel, nProgress);
}

static LPCSTR psz_echo = "echo";
static LPCSTR psz_reverb = "reverb";
static LPCSTR psz_delay = "delay";
static LPCSTR psz_gain = "gain";

/***************************************************************************/
// CProcessWbEcho::WriteProperties Write echo properties
/***************************************************************************/
void CProcessWbEcho::WriteProperties(CObjectOStream & obs) {
    obs.WriteBeginMarker(psz_echo);
    obs.WriteUInt(psz_delay, m_nDelay);
    obs.WriteInteger(psz_gain, m_nGain);
    obs.WriteEndMarker(psz_echo);
}

/***************************************************************************/
// CProcessWbEcho::ReadProperties Read echo properties
/***************************************************************************/
BOOL CProcessWbEcho::ReadProperties(CObjectIStream & obs) {
    UINT nDelay = m_nDelay;
    int nGain = m_nGain;
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_echo)) {
        return FALSE;
    }
    while (!obs.bAtEnd()) {
        if (obs.bReadUInt(psz_delay, m_nDelay));
        else if (obs.bReadInteger(psz_gain, m_nGain));
        else if (obs.bEnd(psz_echo)) {
            break;
        }
    }
    // check if properties changed
    if ((nDelay != m_nDelay) || (nGain != m_nGain)) {
        SetDataInvalid();
    }
    return TRUE;
}

//###########################################################################
// CProcessWbReverb
// Workbench process class to calculate new raw data with a reverb function.

CProcessWbReverb::CProcessWbReverb(Context context) : CWbProcess(context, WbReverb) {
    m_nDelay = 100;
    m_nGain = -20;
}

/***************************************************************************/
// CProcessWbReverb::Process Processing new raw data with a reverb function
// The processed change data is stored in a temporary file. To create it
// helper functions of the base class are used. While processing a process
// bar, placed on the status bar, has to be updated. The level tells which
// processing level this process has been called, start process start on
// which processing percentage this process starts (for the progress bar).
// The status bar process bar will be updated depending on the level and the
// progress start. The return value returns the highest level througout the
// calling queue, or -1 in case of an error in the lower word of the long
// value and the end process progress percentage in the higher word.
/***************************************************************************/
long CProcessWbReverb::Process(void * pCaller, int nProgress, int nLevel) {
    //  TRACE(_T("Process: CProcessWbReverb\n"));
    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    // check if nested workbench processes
    int nOldLevel = nLevel; // save original level
    CWbProcess * pLowerProcess = NULL;
    if (nLevel < MAX_FILTER_NUMBER) {
        pLowerProcess = main.GetWbProcess(model.GetWbProcess() - 1, nLevel);
        if (pLowerProcess) {
            // there is at least one more workbench processes to process first
            long lResult = pLowerProcess->Process(pCaller, nProgress, ++nLevel);
            nLevel = (short int)LOWORD(lResult);
            nProgress = HIWORD(lResult);
        }
    }
    if ((nLevel == nOldLevel) && (IsDataReady())) {
		// data is already ready
        return MAKELONG(--nLevel, nProgress);    
    } else {
        if ((nLevel == PROCESS_CANCELED)) {
			// set your own cancel flag
            CancelProcess();    
        }
        SetDataInvalid();
    }

    if (nLevel < 0) { 
		// previous processing error
        if ((nLevel == PROCESS_CANCELED)) {
			// set your own cancel flag
            CancelProcess();    
        }
        return MAKELONG(nLevel, nProgress);
    }

    // start process
	// wait cursor
    CScopedCursor cursor(view);
	// memory allocation failed
    if (!StartProcess(pCaller, PROCESSWBREV)) { 
		// end data processing
        EndProcess(); 
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // create the temporary file
	// creating error
    if (!CreateTempFile(_T("REV"))) { 
		// end data processing
        EndProcess(); 
        SetDataInvalid();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // get source data size
    DWORD dwDataSize;
    if (pLowerProcess) {
        dwDataSize = pLowerProcess->GetProcessedWaveDataSize();
    } else {
		// size of raw data
        dwDataSize = model.GetDataSize();    
    }
    DWORD dwBufferSize = GetBufferSize();
    DWORD wSmpSize = model.GetSampleSize();
	// pointers to source data
    BPTR pSourceData, pBlockStart; 
	// pointers to target data
    BPTR pTargetData; 
	// data position pointer
    DWORD dwDataPos = 0; 
    DWORD dwRangeStart = 0;
    DWORD dwRangeEnd = dwDataSize - wSmpSize;

    float fGain = (float)1.0 / (float)pow(10.0, (double)m_nGain / -20.0);
    DWORD dwFifoSize = model.GetSamplesPerSec() * m_nDelay / 1000;
    // allocate the temporary global buffer for the fifo
	// fifo is 16 bit
    HANDLE hFifo = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, dwFifoSize * 2); 
    if (!hFifo) {
        // memory allocation error
        app.ErrorMessage(IDS_ERROR_MEMALLOC);
		// error, memory allocation
        return Exit(PROCESS_ERROR); 
    }
	// lock memory
    BPTR lpFifo = (BPTR)::GlobalLock(hFifo); 
    if (!lpFifo) {
        // memory lock error
        app.ErrorMessage(IDS_ERROR_MEMLOCK);
        ::GlobalFree(hFifo);
		// error, memory allocation
        return Exit(PROCESS_ERROR); 
    }
    // initialize fifo with silence
    short int * pFifo = (short int *)lpFifo;
    DWORD dwFifoIndex;
    for (dwFifoIndex = 0; dwFifoIndex < dwFifoSize; dwFifoIndex++) {
        pFifo[dwFifoIndex] = 0;
    }
    dwFifoIndex = 0;
    // start processing loop
    while (dwDataPos < dwDataSize) {
        // load source data
        if (pLowerProcess) {
            pBlockStart = pLowerProcess->GetProcessedWaveData(dwDataPos);
        } else {
            // get pointer to raw data block
            pBlockStart = model.GetAdjustedUnprocessedWaveData(dwDataPos);
        }
        if (!pBlockStart) {
            ::GlobalUnlock(hFifo);
            ::GlobalFree(hFifo);
			// error, reading failed
            return Exit(PROCESS_ERROR); 
        }
        pSourceData = pBlockStart;
        pTargetData = m_lpBuffer;
        DWORD dwBlockEnd = dwDataPos + dwBufferSize;
        if (dwBlockEnd > dwDataSize) {
            dwBlockEnd = dwDataSize;
            dwBufferSize = dwDataSize - dwDataPos;
        }
        int nData;
        BYTE bData;
        while (dwDataPos < dwBlockEnd) {
            // read data
			// 8 bit data
            if (wSmpSize == 1) { 
				// data range is 0...255 (128 is center)
                bData = *pSourceData++; 
                nData = bData - 128;
            } else {
				// 16 bit data
                nData = *((short int *)pSourceData++);
                pSourceData++;
                dwDataPos++;
            }
            dwDataPos++;
            // process data
            if (wSmpSize == 1) {
                int nTempData = nData + (int)((float)(pFifo[dwFifoIndex]) * fGain);
                if ((dwDataPos >= dwRangeStart) && (dwDataPos <= dwRangeEnd)) {
                    nData = nTempData;
                    if (nData > 127) {
                        nData = 127;
                    }
                    if (nData < -128) {
                        nData = -128;
                    }
                }
                pFifo[dwFifoIndex] = (short)nData;
            } else {
                long lData = (long)nData + (long)((float)((long)pFifo[dwFifoIndex]) * fGain);
                if ((dwDataPos >= dwRangeStart) && (dwDataPos <= dwRangeEnd)) {
                    if (lData > 32767) {
                        lData = 32767;
                    }
                    if (lData < -32767) {
                        lData = -32767;
                    }
                    nData = (int)lData;
                }
                pFifo[dwFifoIndex] = (short)nData;
            }
            dwFifoIndex = ++dwFifoIndex % dwFifoSize;
            // save data
            if (wSmpSize == 1) { // 8 bit data
                bData = (BYTE)(nData + 128);
                *pTargetData++ = bData;
            } else {              // 16 bit data
                *pTargetData++ = (BYTE)nData;
                *pTargetData++ = (BYTE)(nData >> 8);
            }
            // set progress bar
            SetProgress(nProgress + (int)(100 * dwDataPos / dwDataSize / (DWORD)nLevel));
            if (IsCanceled()) {
                ::GlobalUnlock(hFifo);
                ::GlobalFree(hFifo);
				// process canceled
                return Exit(PROCESS_CANCELED); 
            }
        }
        // result block is ready, store it
        if (dwDataPos >= dwBlockEnd) {
            // write the processed data block
            try {
                Write(m_lpBuffer, dwBufferSize);
            } catch (CFileException * e) {
                // error writing file
                ::GlobalUnlock(hFifo);
                ::GlobalFree(hFifo);
                app.ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
				// error, writing failed
				e->Delete();
                return Exit(PROCESS_ERROR); 
            }
        }
    }
    // free the fifo buffer
    ::GlobalUnlock(hFifo);
    ::GlobalFree(hFifo);
	// calculate the actual progress
    nProgress = nProgress + (int)(100 / nLevel); 
    // close the temporary file and read the status
	// close the file
    CloseTempFile();
	// end data processing
    EndProcess((nProgress >= 95)); 
    SetDataReady();
    return MAKELONG(nLevel, nProgress);
}

/***************************************************************************/
// CProcessWbReverb::WriteProperties Write reverb properties
/***************************************************************************/
void CProcessWbReverb::WriteProperties(CObjectOStream & obs) {
    obs.WriteBeginMarker(psz_reverb);
    obs.WriteUInt(psz_delay, m_nDelay);
    obs.WriteInteger(psz_gain, m_nGain);
    obs.WriteEndMarker(psz_reverb);
}

/***************************************************************************/
// CProcessWbReverb::ReadProperties Read reverb properties
/***************************************************************************/
BOOL CProcessWbReverb::ReadProperties(CObjectIStream & obs) {
    UINT nDelay = m_nDelay;
    int nGain = m_nGain;
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_reverb)) {
        return FALSE;
    }
    while (!obs.bAtEnd()) {
        if (obs.bReadUInt(psz_delay, m_nDelay));
        else if (obs.bReadInteger(psz_gain, m_nGain));
        else if (obs.bEnd(psz_reverb)) {
            break;
        }
    }
    // check if properties changed
    if ((nDelay != m_nDelay) || (nGain != m_nGain)) {
        SetDataInvalid();
    }
    return TRUE;
}



