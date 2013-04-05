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

#include "stdafx.h"
#include "Process.h"
#include "sa_p_raw.h"
#include "Process\sa_w_adj.h"

#include "isa_doc.h"
#include "resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CProcessRaw::CProcessRaw()
{
}

CProcessRaw::~CProcessRaw()
{
}

/***************************************************************************/
// CProcessRaw::Process Processing raw data (convert raw data to 16-bit)
/***************************************************************************/
long CProcessRaw::Process(void * pCaller, ISaDoc * pDoc, int nProgress, int nLevel)
{

    if (IsCanceled())
    {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    if (IsDataReady())
    {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    }
    TRACE(_T("Process: CProcessRaw\n"));
    BeginWaitCursor(); // wait cursor
    if (!StartProcess(pCaller, IDS_STATTXT_PROCESSRAW))   // memory allocation failed
    {
        EndProcess(); // end data processing
        EndWaitCursor();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // create the temporary file
    if (!CreateTempFile(_T("RAW")))   // creating error
    {
        EndProcess(); // end data processing
        EndWaitCursor();
        SetDataInvalid();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    // process adjust data
    pDoc->GetAdjust()->Process(pCaller, pDoc, nProgress, nLevel);

    // process raw data
    DWORD dwDataSize = pDoc->GetDataSize();                                 // size of raw data
    DWORD nSmpSize = pDoc->GetSampleSize();

    short int * pProcessedData = (short int *)m_lpBuffer;                   // pointer to process data
    // calculate current processing position
    DWORD dwDataPos = 0;
    DWORD dwProcessCount = 0;

    DWORD dwDocWaveBufferSize = GetBufferSize();
    HPSTR pDocData = dwDataSize ? pDoc->GetWaveData(dwDataPos,FALSE) : 0;   // get pointer to data block
    DWORD dwDocWavBufferPosition = pDoc->GetWaveBufferIndex();

    int nScale = pDoc->GetBitsPerSample() == 8 ? 256 : 1;

    // processing loop
    while (dwDataPos < dwDataSize)
    {
        // do we need more data?
        if (dwDataPos >= dwDocWavBufferPosition + dwDocWaveBufferSize)
        {
            // get pointer to data block
            pDocData = pDoc->GetWaveData(dwDataPos,TRUE);
            dwDocWavBufferPosition = pDoc->GetWaveBufferIndex();
        }

        int nRes;
        if (nSmpSize == 1)
        {
            BYTE * pData = reinterpret_cast<BYTE *>(pDocData + dwDataPos - dwDocWavBufferPosition);
            BYTE bData = *pData;                                            // data range is 0...255 (128 is center)
            nRes = bData - 128;
        }
        else
        {
            short int * pData = reinterpret_cast<short int *>(pDocData + dwDataPos - dwDocWavBufferPosition);
            nRes = *pData;                                                  // 16 bit data
        }

        *pProcessedData++ = (short)(nRes * nScale);

        dwDataPos += nSmpSize;

        // is the process buffer is full or processing finished?
        if ((++dwProcessCount == GetProcessBufferSize() / 2) ||
            (dwDataPos >= dwDataSize))
        {
            // set progress bar
            SetProgress(nProgress + (int)(100 * dwDataPos / dwDataSize / (DWORD)nLevel));
            if (IsCanceled())
            {
                return Exit(PROCESS_CANCELED);    // process canceled
            }

            // write the processed data block
            try
            {
                Write(m_lpBuffer, dwProcessCount * sizeof(short));
            }
            catch (CFileException)
            {
                // error writing file
                ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
                return Exit(PROCESS_ERROR);             // error, writing failed
            }

            dwProcessCount = 0;                         // reset counter
            pProcessedData = (short int *)m_lpBuffer;   // reset pointer to begin of process data buffer
        }
    }
    // calculate the actual progress
    nProgress = nProgress + (int)(100 / nLevel);
    // close the temporary file and read the status
    CloseTempFile(); // close the file
    EndProcess(nProgress >= 95); // end data processing
    EndWaitCursor();
    SetDataReady(TRUE);
    return MAKELONG(nLevel, nProgress);
}


