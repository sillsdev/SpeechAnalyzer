/////////////////////////////////////////////////////////////////////////////
// sa_p_raw.cpp:
// Implementation of the CProcessRaw class.
// Author: Steve MacLean
// copyright 2001 JAARS Inc. SIL
//
// Revision History
//   7/17/2001
//        SDM created to abstract/hide file quantization issues (simplify)
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "sa_process.h"
#include "sa_p_raw.h"
#include "sa_w_adj.h"
#include "ScopedCursor.h"

/***************************************************************************/
// CProcessRaw::Process Processing raw data (convert raw data to 16-bit)
/***************************************************************************/
long CProcessRaw::Process(void * pCaller, int nProgress, int nLevel) {

    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    if (IsDataReady()) {
		// data is already ready
        return MAKELONG(--nLevel, nProgress);    
    }
    //TRACE(_T("Process: CProcessRaw\n"));
    CScopedCursor cursor(target);
    if (!StartProcess(pCaller, PROCESSRAW)) { 
		// memory allocation failed
		// end data processing
        EndProcess(); 
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // create the temporary file
    if (!CreateTempFile(_T("RAW"))) {
		// creating error
		// end data processing
        EndProcess(); 
        SetDataInvalid();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    // process adjust data
    model.GetAdjust()->Process(pCaller, nProgress, nLevel);

    // process raw data
    DWORD dwDataSize = model.GetDataSize();										// size of raw data
    DWORD nSmpSize = model.GetSampleSize();

    short int * pProcessedData = (short int *)m_lpBuffer;						// pointer to process data
    // calculate current processing position
    DWORD dwDataPos = 0;
    DWORD dwProcessCount = 0;

    DWORD dwDocWaveBufferSize = GetBufferSize();
	// get pointer to data block
    BPTR pDocData = (dwDataSize!=0) ? model.GetWaveData(dwDataPos,FALSE) : 0;   
    DWORD dwDocWavBufferPosition = model.GetWaveBufferIndex();

    int nScale = model.GetBitsPerSample() == 8 ? 256 : 1;

    // processing loop
    while (dwDataPos < dwDataSize) {
        // do we need more data?
        if (dwDataPos >= dwDocWavBufferPosition + dwDocWaveBufferSize) {
            // get pointer to data block
            pDocData = model.GetWaveData(dwDataPos,TRUE);
            dwDocWavBufferPosition = model.GetWaveBufferIndex();
        }

        int nRes;
        if (nSmpSize == 1) {
            BYTE * pData = reinterpret_cast<BYTE *>(pDocData + dwDataPos - dwDocWavBufferPosition);
            BYTE bData = *pData;                                            // data range is 0...255 (128 is center)
            nRes = bData - 128;
        } else {
            short int * pData = reinterpret_cast<short int *>(pDocData + dwDataPos - dwDocWavBufferPosition);
            nRes = *pData;                                                  // 16 bit data
        }

        *pProcessedData++ = (short)(nRes * nScale);

        dwDataPos += nSmpSize;

        // is the process buffer is full or processing finished?
        if ((++dwProcessCount == GetProcessBufferSize() / 2) ||
            (dwDataPos >= dwDataSize)) {
            // set progress bar
            SetProgress(nProgress + (int)(100 * dwDataPos / dwDataSize / (DWORD)nLevel));
            if (IsCanceled()) {
                return Exit(PROCESS_CANCELED);    // process canceled
            }

            // write the processed data block
            if (!Write(m_lpBuffer, dwProcessCount * sizeof(short))) {
                // error writing file
                app.ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
				return Exit(PROCESS_ERROR);
            }

            dwProcessCount = 0;                         // reset counter
            pProcessedData = (short int *)m_lpBuffer;   // reset pointer to begin of process data buffer
        }
    }
    // calculate the actual progress
    nProgress = nProgress + (int)(100 / nLevel);
    // close the temporary file and read the status
    CloseTempFile();                // close the file
    EndProcess(nProgress >= 95);    // end data processing
    SetDataReady(TRUE);
    return MAKELONG(nLevel, nProgress);
}

