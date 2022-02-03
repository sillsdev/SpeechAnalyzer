/////////////////////////////////////////////////////////////////////////////
// sa_p_mel.cpp:
// Implementation of the CProcessMelogram class.
// Author: Corey Wenger
// copyright 2000 JAARS Inc. SIL
//
// Adapted from sa_p_gra.cpp
//
// CHANGE HISTORY:
//  Date     By             Description
// 9/29/00  CLW    Changed status bar text during processing.
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Process.h"
#include "sa_p_melogram.h"
#include <limits.h>
#include "param.h"
#include "math.h"
#include "AbstractPitchProcess.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CProcessMelogram
// class to calculate melogram for wave data. parameters used in this
// object give better results for music.

/***************************************************************************/
// CProcessMelogram::CProcessMelogram Constructor
/***************************************************************************/
CProcessMelogram::CProcessMelogram() {
    m_nMinValidSemitone100 = 0;
    m_nMaxValidSemitone100 = 0;
}

/***************************************************************************/
// CProcessMelogram::Process Processing melogram data
// The processed melogram data is stored in a temporary file. To create
// it helper functions of the base class are used. While processing a process
// bar, placed on the status bar, has to be updated. The level tells which
// processing level this process has been called, start process start on
// which processing percentage this process starts (for the progress bar).
// The status bar process bar will be updated depending on the level and the
// progress start. The return value returns the highest level througout the
// calling queue, or -1 in case of an error in the lower word of the long
// value and the end process progress percentage in the higher word.
/***************************************************************************/
long CProcessMelogram::Process(void * pCaller, Model * pModel, int nProgress, int nLevel) {
    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }

    {
        int nMinValidSemitone100;
        int nMaxValidSemitone100;
        if (pModel->GetMusicParm()->nCalcRangeMode) {
            nMinValidSemitone100 = pModel->GetMusicParm()->nCalcLowerBound*100;
            nMaxValidSemitone100 = pModel->GetMusicParm()->nCalcUpperBound*100;
        } else {
            EGender nGender = pModel->GetGender(); // use guessing
            switch (nGender) {
            case male: // adult male
                nMinValidSemitone100 = 2750; // ~40Hz
                nMaxValidSemitone100 = 7150; // ~500Hz
                break;
            case female: // adult female
                nMinValidSemitone100 = 3950; // ~80Hz
                nMaxValidSemitone100 = 8450; // ~1070Hz
                break;
            case child: // child
                nMinValidSemitone100 = 3950; // ~80Hz
                nMaxValidSemitone100 = 8450; // ~1070Hz
                break;
            default:
                nMinValidSemitone100 = 2750; // ~40Hz
                nMaxValidSemitone100 = 8450; // ~1070Hz
            }
        }
        // check if data needs to be recalculated
        if (m_nMinValidSemitone100 != nMinValidSemitone100 || m_nMaxValidSemitone100 != nMaxValidSemitone100) {
            // invalidate data
            if (IsDataReady()) {
                SetDataInvalid();
            }
            m_nMinValidSemitone100 = nMinValidSemitone100;
            m_nMaxValidSemitone100 = nMaxValidSemitone100;
        }
    }
    if (IsDataReady()) {
        if (GetDataSize() < 2) {
            return Exit(PROCESS_ERROR);    // error, not enough data
        }
        if (m_nMinValue == SHRT_MAX) {
            return MAKELONG(PROCESS_NO_DATA, 100);    // error, no valid data
        }
        return MAKELONG(--nLevel, nProgress); // data is already ready
    }

    //TRACE(_T("Process: CProcessMelogram\n"));

    BOOL bBackground = false;

    if (nLevel < 0) { // memory allocation failed or previous processing error
        if ((nLevel == PROCESS_CANCELED)) {
            CancelProcess();    // set your own cancel flag
        }
        return MAKELONG(nLevel, nProgress);
    }

    // start grappl process
    if (!bBackground) {
        target.BeginWaitCursor();    // wait cursor
    }
    if (!StartProcess(pCaller, IDS_STATTXT_PROCESSMEL)) { // memory allocation failed or previous processing error
        EndProcess(); // end data processing
        if (!bBackground) {
            target.EndWaitCursor();
        }
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // if file has not been created
    if (wcslen(GetProcessFileName())==0) {
        // create the temporary melogram file
        if (!CreateTempFile(_T("MEL"))) { // creating error
            EndProcess(); // end data processing
            if (!bBackground) {
                target.EndWaitCursor();
            }
            SetDataInvalid();
            return MAKELONG(PROCESS_ERROR, nProgress);
        }
        m_dwDataPos = 0;
        m_nMinValue = 0x7FFF;
        m_nMaxValue = 0;
        // process raw data into grappl pitch data
        // initialise user parameters
        m_CalcParm.sampfreq = (int32)pModel->GetSamplesPerSec();
        m_CalcParm.eightbit = (int16)(!pModel->Is16Bit());
        m_CalcParm.mode = Grappl_fullpitch;
        // convert from semitones to Hz
        int16 nMaxFreqInHz = (int16)(220. * pow(2., ((double)m_nMaxValidSemitone100 / 100. - 57.) / 12.) + 0.5);
        int16 nMinFreqInHz = (int16)(220. * pow(2., ((double)m_nMinValidSemitone100 / 100. - 57.) / 12.) + 0.5);
        m_CalcParm.smoothfreq = (int32)(nMaxFreqInHz < 1200 ? 2100 : 0); // Allow higher frequency range for music processing
        m_CalcParm.minpitch = nMinFreqInHz;
        m_CalcParm.maxpitch = nMaxFreqInHz;
        m_CalcParm.calcint = 100;
        m_CalcParm.minvoiced16 = 32;
        m_CalcParm.maxchange_pc10 = 100;
        m_CalcParm.minsigpoints  = 6;
        m_CalcParm.minmeanweight = 60;
        m_CalcParm.maxinterp_pc10 = 300;
        m_CalcParm.reslag = 0;
        // check if enough workspace for grappl
        UINT nWorkSpace = grapplWorkspace();
        if (GetBufferSize() < (DWORD)nWorkSpace) {
            // buffer too small
            TCHAR szText[6];
            swprintf_s(szText, _T("%u"), nWorkSpace);
            app.ErrorMessage(IDS_ERROR_GRAPPLSPACE, szText);
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
            if (!bBackground) {
                target.EndWaitCursor();
            }
            SetDataInvalid();
            return MAKELONG(PROCESS_ERROR, nProgress);
        }
    }

    bool alldone = false;
    bool nomore = false;
    // get block size
    DWORD dwDataSize = pModel->GetDataSize();                     // raw data size
    DWORD dwBlockSize = 0x10000 - pModel->GetBlockAlign(true);    // 64k - 1
    if (GetBufferSize() < dwBlockSize) {
        dwBlockSize = GetBufferSize();
    }

    HPSTR pBlockStart;
    // start processing
    while (m_dwDataPos < dwDataSize) {
        // get raw data block
        pBlockStart = pModel->GetWaveData(m_dwDataPos, TRUE); // get pointer to data block
        if (!pBlockStart) {
            return Exit(PROCESS_ERROR);    // error, reading failed
        }
        m_dwDataPos += dwBlockSize;
        if (m_dwDataPos >= dwDataSize) {
            dwBlockSize -= (m_dwDataPos - dwDataSize);
            nomore = TRUE;
        }
        // set grappl input buffer
        uint16 length = (WORD)(dwBlockSize / pModel->GetBlockAlign(true));
        //TRACE("grappl length %d\n",length);
        if (!grapplSetInbuff((pGrappl)m_lpBuffer, (pGrappl)pBlockStart, length, nomore)) {
            return Exit(PROCESS_ERROR);
        }

        // process
        pGrappl_res pResults;
        int16 nresults;
        short nSemitone100;

        while (grapplGetResults((pGrappl)m_lpBuffer, &pResults, &nresults, &alldone)) {
            // get max and min values and save the results
            for (int16 nLoop = 0; nLoop < nresults; nLoop++) {
                nSemitone100 = -1;
                if (pResults->fsmooth16 > 0) {
                    // calculate semitone and adjust result to precision multiplier format
                    nSemitone100 = (short)(log((double)pResults->fsmooth16 / (16.0 * 440.0)) / log(2.0) * 1200.0 + 0.5 + 6900.);
                    // limit results to selected range.
                    if (nSemitone100 > m_nMaxValidSemitone100 || nSemitone100 < m_nMinValidSemitone100) {
                        nSemitone100 = -1;    // set this point as unset
                    }
                }
                if (nSemitone100 > 0) {
                    // find min and max values
                    if (nSemitone100 > m_nMaxValue) {
                        m_nMaxValue = nSemitone100;    // save maximum value
                    }
                    if (nSemitone100 < m_nMinValue) {
                        m_nMinValue = nSemitone100;    // save minimum value
                    }
                }
                // write one result of the processed melogram data
                try {
                    Write((HPSTR)&nSemitone100, sizeof(int16));
                } catch (CFileException * e) {
                    // error writing file
                    app.ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
					// error, writing failed
					e->Delete();
					return Exit(PROCESS_ERROR);
                }
                pResults++;
            }
        }

        // set progress bar
        SetProgress(nProgress + (int)(100 * m_dwDataPos / dwDataSize / (DWORD)nLevel));
        if (IsCanceled()) {
            return Exit(PROCESS_CANCELED);    // process canceled
        }
        if (bBackground || alldone) {
            break;
        }
    }

    // calculate the actual progress
    nProgress = nProgress + (int)(100 / nLevel);
    // close the temporary file and read the status
    // close the file
    CloseTempFile();
    if (alldone) {
        if (GetDataSize() < 2) {
            return Exit(PROCESS_ERROR);    // error, not enough data
        }
        SetStatusFlag(PROCESS_NO_PITCH, m_nMinValue == SHRT_MAX);
    }
    EndProcess((nProgress >= 95));          // end data processing
    if (!bBackground) {
        target.EndWaitCursor();
    }
    if (nomore || alldone) {
        SetDataReady();
    }
    return MAKELONG(nLevel, nProgress);
}

/***************************************************************************/
// CProcessMelogram::IsVoiced  Checks if waveform section at offset is voiced.
/***************************************************************************/
BOOL CProcessMelogram::IsVoiced(Model * pModel, DWORD dwWaveOffset) {
    UINT nSmpSize = pModel->GetSampleSize();
    DWORD dwSmpOffset = dwWaveOffset / nSmpSize;
    DWORD dwPitchBlock = m_dwBufferOffset;
    BOOL bDone;
    Process(this, (Model *)pModel);
    BOOL bVoiced = (GetProcessedData( dwSmpOffset / Grappl_calc_intvl, &bDone) > 0);
    if (!bDone) {
        return FALSE;
    }
    if (dwPitchBlock != UNDEFINED_OFFSET && m_dwBufferOffset != dwPitchBlock) {
        GetProcessedData( dwPitchBlock, TRUE);    // restore pitch buffer
    }
    return(bVoiced);
}
