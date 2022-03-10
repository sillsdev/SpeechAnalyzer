/////////////////////////////////////////////////////////////////////////////
// sa_p_pit.cpp:
// Implementation of the CProcessPitch class.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <limits.h>
#include "sa_process.h"
#include "sa_p_pitch.h"
#include "AbstractPitchProcess.h"
#include "param.h"
#include "scopedcursor.h"

//###########################################################################
// CProcessPitch
// class to calculate pitch for wave data. This class stores raw pitch and
// quality data into temporary files.

/***************************************************************************/
// CProcessPitch::Process Processing pitch data
// The processed pitch data is stored in a temporary file. 
// Also quality data, a product of pitch calculating is stored in a different temporary file for further use. 
// To create it helper functions of the base class are used.
// While processing a process bar, placed on the status bar, has to be updated. 
// The level tells which processing level this process has been called, start process start on which processing percentage this process starts (for the progress bar). 
// The status bar process bar will be updated depending on the level and the progress start. 
// The return value returns the highest level througout the calling queue, or -1 in case of an error in the lower word of the long value and the end process progress percentage in the higher word.  
// Uses WinCecil 2.2 pitch algorithm.
/***************************************************************************/
long CProcessPitch::Process(void * pCaller,  int nProgress, int nLevel) {

    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    if (IsDataReady()) {
        return MAKELONG(--nLevel, nProgress);           // data is already ready
    }
    DWORD dwDataSize = model.GetDataSize();             // raw data size
    if (!dwDataSize) {
        return Exit(PROCESS_NO_DATA);                   // error, no valid data
    }
    const CUttParm * pUttParm = model.GetUttParm();     // get sa parameters utterance member data

    if (nLevel < 0) {                                   // previous processing error
        if ((nLevel == PROCESS_CANCELED)) {
            CancelProcess();                            // set your own cancel flag
        }
        return MAKELONG(nLevel, nProgress);
    }

    // start pitch process
    CScopedCursor cursor(target);
    if (!StartProcess(pCaller, PROCESSPIT)) { // previous processing error
        EndProcess();                                   // end data processing
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    // if file has not been created
    if (wcslen(GetProcessFileName())==0) {
        // create the temporary grappl pitch file
        if (!CreateTempFile(_T("PIT"))) {
            // creating error
            EndProcess();                               // end data processing
            SetDataInvalid();
            return MAKELONG(PROCESS_ERROR, nProgress);
        }
        // initialize parameters
        m_dwDataPos = 0;
        m_nMinValue = SHRT_MAX;
        m_CalcParm.sampfreq = (int32)model.GetSamplesPerSec();
        DWORD wSmpSize = model.GetSampleSize();
        m_CalcParm.eightbit = (int16)(wSmpSize == 1);
        m_CalcParm.mode = Grappl_fullpitch;;
        m_CalcParm.smoothfreq = 1000L;
        m_CalcParm.calcint= 100;                        //!!this should be based on sampling frequency
        m_CalcParm.minmeanweight = 60;
        m_CalcParm.maxinterp_pc10 = 300;
        m_CalcParm.minpitch = int16(pUttParm->nMinFreq);
        m_CalcParm.maxpitch = int16(pUttParm->nMaxFreq);
        m_CalcParm.minvoiced16 = int16((pUttParm->TruncatedCritLoud(model.GetBitsPerSample())*16+8)/PRECISION_MULTIPLIER);
        m_CalcParm.maxchange_pc10 = int16(pUttParm->nMaxChange * 10);
        m_CalcParm.minsigpoints = int16(pUttParm->nMinGroup);
        m_CalcParm.reslag = 0;

        // check if enough workspace for grappl
        UINT nWorkSpace = grapplWorkspace();
        if (GetBufferSize() < (DWORD)nWorkSpace) {
            // buffer too small
            TCHAR szText[6];
            swprintf_s(szText, _T("%u"), nWorkSpace);
            app.GrapplErrorMessage( szText);
            return Exit(PROCESS_ERROR); // error, buffer too small
        }
        // init grappl
        if (!grapplInit(m_lpBuffer, &m_CalcParm)) {
            return Exit(PROCESS_ERROR);
        }
    } else {
        // open file to append data
        if (!OpenFileToAppend()) {
            EndProcess(); // end data processing
            SetDataInvalid();
            return MAKELONG(PROCESS_ERROR, nProgress);
        }
    }
    bool alldone = false;
    bool nomore = false;

    // get block size
    DWORD dwBlockSize = 0x10000 - model.GetBlockAlign(true);    // 64k - 1
    if (GetBufferSize() < dwBlockSize) {
        dwBlockSize = GetBufferSize();
    }
    BPTR pBlockStart;
    // start processing
    while (m_dwDataPos < dwDataSize) {
        // get raw data block
        pBlockStart = model.GetWaveData(m_dwDataPos, TRUE);     // get pointer to data block
        if (!pBlockStart) {
            return Exit(PROCESS_ERROR);                         // error, reading failed
        }
        m_dwDataPos += dwBlockSize;
        if (m_dwDataPos >= dwDataSize) {
            dwBlockSize -= (m_dwDataPos - dwDataSize);
            nomore = TRUE;
        }
        // set grappl input buffer
        uint16 length = (WORD)(dwBlockSize / model.GetBlockAlign(true));
        //TRACE("grappl length %d\n",length);
        if (!grapplSetInbuff((pGrappl)m_lpBuffer, (pGrappl)pBlockStart, length, nomore)) {
            return Exit(PROCESS_ERROR);
        }
        // process
        pGrappl_res pResults;
        int16 nresults;

        while (grapplGetResults((pGrappl)m_lpBuffer, &pResults, &nresults, &alldone)) {
            // get max and min values and save the results
            for (int16 nLoop = 0; nLoop < nresults; nLoop++) {
                // find min and max values
                if (pResults->fcalc16 >= 0) {
                    // adjust result to precision multiplier format
                    pResults->fcalc16 = (int16)(((long)pResults->fcalc16 * (long)PRECISION_MULTIPLIER + 8L)/ 16L);
                    if (pResults->fcalc16 > m_nMaxValue) {
                        m_nMaxValue = pResults->fcalc16;    // save maximum value
                    }
                    if (pResults->fcalc16 < m_nMinValue) {
                        m_nMinValue = pResults->fcalc16;    // save minimum value
                    }
                } else {
                    pResults->fcalc16 = -1;    // set this point as unset
                }
                // write one result of the processed grappl pitch data
                if (!Write((BPTR)&pResults->fcalc16, sizeof(int16))) {
                    // error writing file
                    app.ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
					return Exit(PROCESS_ERROR);
                }

                pResults++;
            }
        }
        // set progress bar
        SetProgress(nProgress + (int)(100 * m_dwDataPos / dwDataSize / (DWORD)nLevel));
        if (IsCanceled()) {
            return Exit(PROCESS_CANCELED);      // process canceled
        }
        if (alldone) {
            break;
        }
    }

    // calculate the actual progress
    nProgress = nProgress + (int)(100 / nLevel);
    // close the temporary file and read the status
    // close the file
    CloseTempFile();
    if (GetDataSize() < 2) {
        return Exit(PROCESS_ERROR);             // error, not enough data
    }

    if (alldone) {
        if (GetDataSize() < 2) {
            return Exit(PROCESS_ERROR);    // error, not enough data        //!!should be TRUE = close file?
        }
        SetStatusFlag(PROCESS_NO_PITCH, m_nMinValue == SHRT_MAX);
    }
    EndProcess((nProgress >= 95)); // end data processing
    if (nomore || alldone) {
        SetDataReady();
    }
    return MAKELONG(nLevel, nProgress);
}
