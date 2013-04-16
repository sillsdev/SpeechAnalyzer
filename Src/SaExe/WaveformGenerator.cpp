#include "stdafx.h"
#include "WaveformGenerator.h"
#include "Process\Process.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CProcessWaveformGenerator::CProcessWaveformGenerator()
{
}

CProcessWaveformGenerator::~CProcessWaveformGenerator()
{
}

/***************************************************************************/
// CDlgWaveformGenerator::process::Process Generate wav file
/***************************************************************************/
long CProcessWaveformGenerator::Process(CWaveformGeneratorSettings & parms, void * pCaller, int nProgress, int nLevel)
{

    //TRACE(_T("Process: CProcessWbGenerator\n"));
    if (IsCanceled())
    {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    // check if nested workbench processes
    if (IsDataReady())
    {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    }
    else
    {
        SetDataInvalid();
    }

    if (nLevel < 0)   // previous processing error
    {
        if ((nLevel == PROCESS_CANCELED))
        {
            CancelProcess();    // set your own cancel flag
        }
        return MAKELONG(nLevel, nProgress);
    }

    // start process
    BeginWaitCursor(); // wait cursor
    if (!StartProcess(pCaller, IDS_STATTXT_PROCESSWBGENERATOR))   // memory allocation failed or previous processing error
    {
        EndProcess(); // end data processing
        EndWaitCursor();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // create the temporary file
    if (!CreateTempFile(_T("GEN")))   // creating error
    {
        EndProcess(); // end data processing
        EndWaitCursor();
        SetDataInvalid();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    DWORD dwBufferSize = GetBufferSize();
    // get source data size
    DWORD dwDataSize = DWORD(parms.m_fFileLength*parms.pcm.wf.nSamplesPerSec*parms.pcm.wf.nBlockAlign);
    DWORD dwDataPos = 0; // data position pointer
    // allocate the temporary global buffer for the fifo
    // start processing loop
    while (dwDataPos < dwDataSize)
    {
        DWORD dwBlockEnd = dwDataPos + dwBufferSize;
        if (dwBlockEnd > dwDataSize)
        {
            dwBlockEnd = dwDataSize;
            dwBufferSize = dwDataSize - dwDataPos;
        }

        // Synthesize
        parms.SynthesizeSamples(m_lpBuffer, dwDataPos, dwBufferSize);

        dwDataPos += dwBufferSize;

        // result block is ready, store it
        if (dwDataPos >= dwBlockEnd)
        {
            // write the processed data block
            try
            {
                Write(m_lpBuffer, dwBufferSize);
            }
            catch (CFileException e)
            {
                // error writing file
                ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
                return Exit(PROCESS_ERROR); // error, writing failed
            }
        }
        // set progress bar
        SetProgress(nProgress + (int)(100 * dwDataPos / dwDataSize / (DWORD)nLevel));
        if (IsCanceled())
        {
            return Exit(PROCESS_CANCELED); // process canceled
        }
    }
    nProgress = nProgress + (int)(100 / nLevel); // calculate the actual progress
    // close the temporary file and read the status
    CloseTempFile(); // close the file
    EndProcess((nProgress >= 95)); // end data processing
    EndWaitCursor();
    SetDataReady();
    return MAKELONG(nLevel, nProgress);
}
