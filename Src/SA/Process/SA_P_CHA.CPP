/////////////////////////////////////////////////////////////////////////////
// sa_p_cha.cpp:
// Implementation of the CProcessChange class.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Process.h"
#include "sa_p_cha.h"
#include "sa_p_pit.h"
#include "sa_p_lou.h"
#include "isa_doc.h"
#include "FileUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CProcessChange
// class to calculate change values for wave data. Change is calculated by
// using acf best fit residual (quality) and smoothed loudness results,
// produced by processing smoothed loudness and raw pitch.

/////////////////////////////////////////////////////////////////////////////
// CProcessChange construction/destruction/creation

/***************************************************************************/
// CProcessChange::CProcessChange Constructor
/***************************************************************************/
CProcessChange::CProcessChange() {
    m_pQualityFile = new CFile;
    m_qualityFileStatus.m_szFullName[0] = 0; // no file name
    m_hQualityData = NULL;
    m_nQualityMaxValue = 0;
    m_dwQualityBufferOffset = UNDEFINED_OFFSET; // buffer undefined, force buffer reload
    m_bQualityBlockBegin = FALSE;
}

/***************************************************************************/
// CProcessChange::~CProcessChange Destructor
/***************************************************************************/
CProcessChange::~CProcessChange() {
    // delete the temporary quality file
    FileUtils::Remove(m_qualityFileStatus.m_szFullName);
    if (m_pQualityFile) {
        delete m_pQualityFile;
    }
    // free the quality buffer memory
    if (m_hQualityData) {
        ::GlobalUnlock(m_hQualityData);
        ::GlobalFree(m_hQualityData);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CProcessChange helper functions

/***************************************************************************/
// CProcessChange::Exit Exit on Error
// Standard exit function if an error occured.
/***************************************************************************/
long CProcessChange::Exit(int nError) {
    if (m_pQualityFile) {
        m_pQualityFile->Abort();
    }
    FileUtils::Remove(m_qualityFileStatus.m_szFullName);
    m_qualityFileStatus.m_szFullName[0] = 0;
    return CProcess::Exit(nError);
}

/***************************************************************************/
// CProcessChange::CalculateACF8 Calculate auto correlation coefficient 8 bit
// The caller delivers a pointer to a BOOLEAN result variable. If this will
// be FALSE after the operation, an error occured.
/***************************************************************************/
UINT CProcessChange::CalculateACF8(CProcess * pProc, unsigned char * pSRDdata,
                                   BOOL * pbRes, UINT nACFinterval, UINT nLagData, UINT nMaxLag,
                                   DWORD * pdwBlockPos, DWORD dwDataPos, int nLoopCount) {
    CProcessSmoothLoudness * pProcess = (CProcessSmoothLoudness *)pProc; // cast pointer
    DWORD dwPos = dwDataPos - (nLagData >> 1);
    UINT nResult = 0;
    if (dwPos < *pdwBlockPos) {
        // reload new block
        DWORD dwNewBlock = 0;
        if (dwDataPos > nMaxLag) {
            dwNewBlock = dwDataPos - nMaxLag;    // lowest needed data
        }
        if ((dwNewBlock + GetProcessBufferSize()) >= (dwPos + nLagData)) {
            *pdwBlockPos = dwNewBlock;
        } else {
            *pdwBlockPos = dwPos;
        }
        pSRDdata = (unsigned char *)pProcess->GetSmoothRawData(*pdwBlockPos, TRUE);
        if (!pSRDdata) {
            *pbRes = FALSE; // reading failed
            return 0;
        }
    }
    DWORD dwBlockEnd = *pdwBlockPos + GetProcessBufferSize() - nLagData;
    pSRDdata += (dwPos - *pdwBlockPos); // set the source pointer
    do {
        if (dwPos >= dwBlockEnd) { // end of data block
            // reload new block
            DWORD dwNewBlock = 0;
            if (dwDataPos > nMaxLag) {
                dwNewBlock = dwDataPos - nMaxLag;    // lowest needed data
            }
            if ((dwNewBlock + GetProcessBufferSize()) >= (dwPos + nLagData)) {
                *pdwBlockPos = dwNewBlock;
            } else {
                *pdwBlockPos = dwPos;
            }
            pSRDdata = (unsigned char *)pProcess->GetSmoothRawData(*pdwBlockPos, TRUE);
            if (!pSRDdata) {
                *pbRes = FALSE; // reading failed
                return 0;
            }
            dwBlockEnd = *pdwBlockPos + GetProcessBufferSize() - nLagData;
            pSRDdata += (dwPos - *pdwBlockPos); // set the source pointer
        }
        int nData = *pSRDdata - *(pSRDdata + nLagData);
        nResult += (UINT)(nData >= 0 ? nData : -nData);
        dwPos += nACFinterval;
        pSRDdata += nACFinterval;
    } while (--nLoopCount);
    return nResult;
}

/***************************************************************************/
// CProcessChange::CalculateACF16 Calculate auto correlation coefficient 16 bit
// The caller delivers a pointer to a BOOLEAN result variable. If this will
// be FALSE after the operation, an error occured.
/***************************************************************************/
DWORD CProcessChange::CalculateACF16(CProcess * pProc, unsigned char * pSRDdata,
                                     BOOL * pbRes, UINT nACFinterval, UINT nLagData, UINT nMaxLag,
                                     DWORD * pdwBlockPos, DWORD dwDataPos, int nLoopCount) {
    CProcessSmoothLoudness * pProcess = (CProcessSmoothLoudness *)pProc; // cast pointer
    DWORD dwPos = (dwDataPos - nLagData) & ~1L; // must be even
    DWORD dwResult = 0;
    if (dwPos < *pdwBlockPos) {
        // reload new block
        DWORD dwNewBlock = 0;
        if (dwDataPos > nMaxLag * 2) {
            dwNewBlock = dwDataPos - nMaxLag * 2;    // lowest needed data
        }
        if ((dwNewBlock + GetProcessBufferSize()) >= (dwPos + nLagData * 2)) {
            *pdwBlockPos = dwNewBlock;
        } else {
            *pdwBlockPos = dwPos;
        }
        pSRDdata = (unsigned char *)pProcess->GetSmoothRawData(*pdwBlockPos, TRUE);
        if (!pSRDdata) {
            *pbRes = FALSE; // reading failed
            return 0;
        }
    }
    pSRDdata += (dwPos - *pdwBlockPos); // set the source pointer
    short int * lpSRDdata = (short int *)pSRDdata; // cast pointer
    DWORD dwBlockEnd = *pdwBlockPos + GetProcessBufferSize() - nLagData * 2;
    do {
        if (dwPos >= dwBlockEnd) { // end of data block
            // reload new block
            DWORD dwNewBlock = 0;
            if (dwDataPos > nMaxLag * 2) {
                dwNewBlock = dwDataPos - nMaxLag * 2;    // lowest needed data
            }
            if ((dwNewBlock + GetProcessBufferSize()) >= (dwPos + nLagData * 2)) {
                *pdwBlockPos = dwNewBlock;
            } else {
                *pdwBlockPos = dwPos;
            }
            pSRDdata = (unsigned char *)pProcess->GetSmoothRawData(*pdwBlockPos, TRUE);
            if (!pSRDdata) {
                *pbRes = FALSE; // reading failed
                return 0;
            }
            pSRDdata += (dwPos - *pdwBlockPos); // set the source pointer
            lpSRDdata = (short int *)pSRDdata; // cast pointer
            dwBlockEnd = *pdwBlockPos + GetProcessBufferSize() - nLagData * 2;
        }
        int nData = *lpSRDdata - *(lpSRDdata + nLagData);
        dwResult += (DWORD)(nData >= 0 ? nData : -nData);
        dwPos += nACFinterval * 2;
        lpSRDdata += nACFinterval;
    } while (--nLoopCount);
    return dwResult;
}

/***************************************************************************/
// CProcessChange::SetDataInvalid Delete processed data
/***************************************************************************/
void CProcessChange::SetDataInvalid() {
    CProcess::SetDataInvalid();
    FileUtils::Remove(m_qualityFileStatus.m_szFullName);
    m_qualityFileStatus.m_szFullName[0] = 0;
}

/***************************************************************************/
// CProcessChange::Process Processing change data
// The processed change data is stored in a temporary file. To create it
// helper functions of the base class are used. While processing a process
// bar, placed on the status bar, has to be updated. The level tells which
// processing level this process has been called, start process start on
// which processing percentage this process starts (for the progress bar).
// The status bar process bar will be updated depending on the level and the
// progress start. The return value returns the highest level throughout the
// calling queue, or -1 in case of an error in the lower word of the long
// value and the end process progress percentage in the higher word.
//
// Based on WinCecil 2.1a pitch algorithm instead of WinCecil 2.2 (as employed
// in sa_p_pit.cpp) because the latter algorithm does not produce a change
// (quality) value based on the accumulation of magnitude differences between
// corresponding samples in adjacent frames, as required for the change graph.
// In the future, this code should be replaced with a correlation metric based
// on fragments.
/***************************************************************************/
long CProcessChange::Process(void * pCaller, ISaDoc * pDoc, int nProgress,
                             int nLevel) {
    //TRACE(_T("Process: CProcessChange\n"));
    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    if (IsDataReady()) {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    }
    // create raw pitch data, its quality and smoothed loudness output is needed to calculate change
    CProcessSmoothLoudness * pLoudness = (CProcessSmoothLoudness *)pDoc->GetSmoothLoudness(); // get pointer to loudness object
    long lResult = pLoudness->Process(pCaller, pDoc, nProgress, ++nLevel); // process data
    nLevel = (short int)LOWORD(lResult);
    nProgress = HIWORD(lResult);

    if (nLevel < 0) { // previous processing error
        if ((nLevel == PROCESS_CANCELED)) {
            CancelProcess();    // set your own cancel flag
        }
        return MAKELONG(nLevel, nProgress);
    }

    // allocate global buffer for the processed quality data
    // start pitch process
    // wait cursor
    BeginWaitCursor();
    // start data processing
    if (!StartProcess(pCaller, IDS_STATTXT_PROCESSPIT, FALSE)) {
        // end data processing
        EndProcess();
        EndWaitCursor();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // allocate global buffer for the processed quality data
    // not yet allocated
    if (!m_hQualityData) {
        // allocate memory
        m_hQualityData = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, GetProcessBufferSize());
        if (!m_hQualityData) {
            // memory allocation error
            ErrorMessage(IDS_ERROR_MEMALLOC);
            // end data processing
            EndProcess();
            EndWaitCursor();
            return MAKELONG(PROCESS_ERROR, nProgress);
        }
        m_lpQualityData = (HPSTR)::GlobalLock(m_hQualityData); // lock memory
        if (!m_lpQualityData) {
            // memory lock error
            ErrorMessage(IDS_ERROR_MEMLOCK);
            ::GlobalFree(m_hQualityData);
            m_hQualityData = NULL;
            EndProcess(); // end data processing
            EndWaitCursor();
            return MAKELONG(PROCESS_ERROR, nProgress);
        }
    }
    CUttParm myUttParm;
    CUttParm * pUttParm = &myUttParm;
    pDoc->GetUttParm(pUttParm); // get sa parameters utterance member data
    UINT nBlockAlign = pDoc->GetBlockAlign();
    // calculate current processing position (copied but not understood from WINCECIL)
    DWORD dwLoopPos, dwLoopStart;
    if (pUttParm->nMinFreq == 0) {
        pUttParm->nMinFreq = 50;    // to prevent 0 divisions
    }
    if (pUttParm->nMaxFreq == 0) {
        pUttParm->nMaxFreq = 500;    // to prevent 0 divisions
    }
    dwLoopStart = ((pDoc->GetSamplesPerSec() / (2 * MAX_CALCULATION_FREQUENCY)
                    + pDoc->GetSamplesPerSec() / pUttParm->nMinFreq) / 2
                   + CALCULATION_DATALENGTH(pDoc->GetSamplesPerSec()) / 2 + 5) / CALCULATION_INTERVAL(pDoc->GetSamplesPerSec()) + 1;
    dwLoopPos = 0;
    DWORD dwDataPos = dwLoopStart * CALCULATION_INTERVAL(pDoc->GetSamplesPerSec()) - CALCULATION_DATALENGTH(pDoc->GetSamplesPerSec()) / 2;
    if (nBlockAlign > 1) {
        dwDataPos *= 2;    // 16 bit data
    }

    // create the temporary file for quality data
    if (!CreateAuxTempFile(_T("QUA"), m_pQualityFile, m_qualityFileStatus)) {
        return Exit(PROCESS_ERROR);
    }

    // calculate lags at which to look for smallest autocorrelation coefficient
    int nMinLag = (int)(pDoc->GetSamplesPerSec() / (DWORD)pUttParm->nMaxFreq);
    int nMaxLag = (int)(pDoc->GetSamplesPerSec() / (DWORD)pUttParm->nMinFreq);
    int naACFlags[80]; // to be filled with lags at 5% intervals between nMinLag and nMaxLag
    // fill up the lag array
    naACFlags[0] = nMinLag - 1; // extra value to constrain fine tuning at limit
    naACFlags[1] = nMinLag;
    long lVal = (long)nMinLag * 256;
    int nVal, nNumLags = 1; // number of lags in naACFlags to scan over
    while (TRUE) {
        lVal = (lVal * 105L) / 100L;
        nVal = (int)(lVal / 256);
        if (nVal == naACFlags[nNumLags]) {
            continue;
        }
        if (nVal > nMaxLag) {
            nVal = nMaxLag;
        }
        naACFlags[++nNumLags] = nVal;
        if (nVal == nMaxLag) {
            break;
        }
    }
    naACFlags[++nNumLags] = nMaxLag + 1; // extra value
    nNumLags--;
    // process smoothed raw data into raw pitch data
    DWORD dwDataSize = pLoudness->GetDataSize(); // size of processed data
    DWORD dwLoopEnd = dwDataSize - dwLoopStart; // end of loop
    short int * pQualityData = (short int *)m_lpQualityData; // pointer to quality data
    DWORD dwPitchCount = 0; // reset processed data counter
    // calculate lowest needed first data index in smooth raw data block and load data block
    BOOL bRes = TRUE; // calculation result
    DWORD dwBlockPos = 0;
    if (nBlockAlign > 1) { // 16 bit data (value must be even)
        if (dwDataPos > (DWORD)nMaxLag) {
            dwBlockPos = (dwDataPos - nMaxLag) & ~1L;
        }
    } else { // 8 bit data
        if (dwDataPos > (DWORD)(nMaxLag >> 1)) {
            dwBlockPos = dwDataPos - (nMaxLag >> 1);    // 8 bit data
        }
    }
    HPSTR pSRDdata = pLoudness->GetSmoothRawData(dwBlockPos, TRUE);
    if (!pSRDdata) {
        return Exit(PROCESS_ERROR);    // error, reading failed
    }
    m_nMinValue = 0x7FFF;
    m_nQualityMaxValue = 0;
    while (dwLoopPos < dwDataSize) {
        if ((dwLoopPos >= dwLoopStart) && (dwLoopPos < dwLoopEnd)) {
            // now do rough search for best autocorrelation coefficient fit
            DWORD dwResult, dwMinCoeff = ULONG_MAX;
            UINT nLagData, nACFinterval = 30;
            int nFoundLag = 0;
            for (int nLoop = 0; nLoop < nNumLags; nLoop++) {
                // try each lag
                nLagData = (unsigned int)naACFlags[nLoop + 1]; // first entry is dummy
                int nInnerLoop = CALCULATION_DATALENGTH(pDoc->GetSamplesPerSec()) / nACFinterval + 1;
                if (nBlockAlign > 1) // 16 bit data
                    dwResult = CalculateACF16(pLoudness, (unsigned char *)pSRDdata, &bRes, nACFinterval, nLagData,
                                              nMaxLag, &dwBlockPos, dwDataPos, nInnerLoop);
                else // 8 bit data
                    dwResult = CalculateACF8(pLoudness, (unsigned char *)pSRDdata, &bRes, nACFinterval, nLagData,
                                             nMaxLag, &dwBlockPos, dwDataPos, nInnerLoop);
                if (!bRes) {
                    break;
                }
                if (dwResult < dwMinCoeff) {
                    // best fit so far
                    dwMinCoeff = dwResult;
                    nFoundLag = nLoop + 1;
                }
            }
            if (!bRes) {
                return Exit(PROCESS_ERROR);    // error, reading failed
            }
            // now home in on range between lags on either side of found lag
            nACFinterval = 12;
            dwMinCoeff = ULONG_MAX;
            int nLagLow = naACFlags[nFoundLag - 1] + 1;
            int nLagHigh = naACFlags[nFoundLag + 1] - 1;
            for (nLagData = (unsigned int)nLagLow; nLagData <= (unsigned int)nLagHigh; nLagData++) {
                // scan around best fit
                int nInnerLoop = CALCULATION_DATALENGTH(pDoc->GetSamplesPerSec()) / nACFinterval;
                if (nBlockAlign > 1) // 16 bit data
                    dwResult = CalculateACF16(pLoudness, (unsigned char *)pSRDdata, &bRes, nACFinterval, nLagData,
                                              nLagHigh, &dwBlockPos, dwDataPos, nInnerLoop);
                else // 8 bit data
                    dwResult = CalculateACF8(pLoudness, (unsigned char *)pSRDdata, &bRes, nACFinterval, nLagData,
                                             nLagHigh, &dwBlockPos, dwDataPos, nInnerLoop);
                if (!bRes) {
                    break;
                }
                if (dwResult < dwMinCoeff) {
                    dwMinCoeff = dwResult;
                    nFoundLag = nLagData;
                }
            }
            if (!bRes) {
                return Exit(PROCESS_ERROR);    // error, reading failed
            }
            // here we have the quality result ready! Store it!
            if (nBlockAlign > 1) { // 16 bit data
                // quality has to be stored divided by PRECISION_MULTIPLIER to fit into the 16bit result
                UINT nQuality = (UINT)(dwMinCoeff / PRECISION_MULTIPLIER);
                *pQualityData++ = (short int)nQuality; // save result
                if (nQuality > (UINT)m_nQualityMaxValue) {
                    m_nQualityMaxValue = nQuality;    // save maximum quality value
                }
            } else { // 8 bit data
                *pQualityData++ = (short int)dwMinCoeff; // save result
                if (dwMinCoeff > (UINT)m_nQualityMaxValue) {
                    m_nQualityMaxValue = (UINT)dwMinCoeff;    // save maximum quality value
                }
            }
            // check this isn't twice fundamental
            if (nFoundLag >= (nMinLag * 2)) {
                // check frequency / 2
                nLagData = nFoundLag / 2;
                int nInnerLoop = CALCULATION_DATALENGTH(pDoc->GetSamplesPerSec()) / nACFinterval;
                if (nBlockAlign > 1) // 16 bit data
                    dwResult = CalculateACF16(pLoudness, (unsigned char *)pSRDdata, &bRes, nACFinterval, nLagData,
                                              nLagData, &dwBlockPos, dwDataPos, nInnerLoop);
                else // 8 bit data
                    dwResult = CalculateACF8(pLoudness, (unsigned char *)pSRDdata, &bRes, nACFinterval, nLagData,
                                             nLagData, &dwBlockPos, dwDataPos, nInnerLoop);
                // if it fits reasonably well...
                if ((2 * dwResult) < (3 * dwMinCoeff)) {
                    nFoundLag = nLagData;
                } else {
                    // ...else check for 3x fundamental
                    if ((nFoundLag >= (nMinLag * 3)) && bRes) {
                        nLagData = nFoundLag / 3;
                        int nInnerLoop = CALCULATION_DATALENGTH(pDoc->GetSamplesPerSec()) / nACFinterval;
                        if (nBlockAlign > 1) // 16 bit data
                            dwResult = CalculateACF16(pLoudness, (unsigned char *)pSRDdata, &bRes, nACFinterval, nLagData,
                                                      nLagData, &dwBlockPos, dwDataPos, nInnerLoop);
                        else // 8 bit data
                            dwResult = CalculateACF8(pLoudness, (unsigned char *)pSRDdata, &bRes, nACFinterval, nLagData,
                                                     nLagData, &dwBlockPos, dwDataPos, nInnerLoop);
                        if ((2 * dwResult) < (3 * dwMinCoeff)) {
                            nFoundLag = nLagData;
                        }
                    }
                }
            }
            if (!bRes) {
                return Exit(PROCESS_ERROR);    // error, reading failed
            }
            // calc frequency from found lag and store width multiplier
            // discard points which haven't converged and lie on upper frequency bound
            int nPitchResult = -1;
            if ((nFoundLag != nMinLag) && (nFoundLag != nMaxLag)) {
                nPitchResult = (short)((PRECISION_MULTIPLIER * (unsigned long)(pDoc->GetSamplesPerSec())) / (unsigned long)nFoundLag);
            }
            // pitch result is ready, store it
            if (nPitchResult >= 0) {
                if (nPitchResult > m_nMaxValue) {
                    m_nMaxValue = nPitchResult;    // save maximum value
                }
                if (nPitchResult < m_nMinValue) {
                    m_nMinValue = nPitchResult;    // save minimum value
                }
            }
            // set new calculation window start
            if (nBlockAlign > 1) {
                dwDataPos += CALCULATION_INTERVAL(pDoc->GetSamplesPerSec()) * 2;    // 16 bit data
            } else {
                dwDataPos += CALCULATION_INTERVAL(pDoc->GetSamplesPerSec());    // 8 bit data
            }
        } else { // proplogue and epilogue
            *pQualityData++ = 0;    // reset quality
        }
        dwLoopPos++;
        // set progress bar
        SetProgress(nProgress + (int)(100 * dwLoopPos / dwDataSize / (DWORD)nLevel));
        if (IsCanceled()) {
            return Exit(PROCESS_CANCELED);    // process canceled
        }
        if ((++dwPitchCount == GetProcessBufferSize() / 2) || (dwLoopPos >= dwDataSize)) { // pitch buffer is full or processing finished
            // write the processed quality data block
            try {
                m_pQualityFile->Write((HPSTR)m_lpQualityData, dwPitchCount * 2);
            } catch (CFileException * e) {
                // error writing file
                ErrorMessage(IDS_ERROR_WRITETEMPFILE, m_qualityFileStatus.m_szFullName);
				// error, writing failed
				e->Delete();
				return Exit(PROCESS_ERROR);
            }
			// reset pointer to begin of quality data buffer
			pQualityData = (short int *)m_lpQualityData;
			// reset processed data counter
			dwPitchCount = 0;
        }
    }
    // calculate the actual progress
    nProgress = nProgress + (int)(100 / nLevel);
    // close the quality data temp file and read the status
    if (m_pQualityFile) {
        m_pQualityFile->Abort();
    }

    CString szFullName = m_qualityFileStatus.m_szFullName;
	// read the status
	CFile::GetStatus(szFullName, m_qualityFileStatus);
    if (m_qualityFileStatus.m_size < 2 * sizeof(int)) {
		// error, not enough data
		return Exit(PROCESS_ERROR);
	}

	// memory allocation failed or previous processing error
	if (nLevel < 0) {
		// end data processing
		EndProcess();
        EndWaitCursor();
        if ((nLevel == PROCESS_CANCELED)) {
			// set your own cancel flag
			CancelProcess();
        }
        return MAKELONG(nLevel, nProgress);
    }

    // start change process
    if (!StartProcess(pCaller, IDS_STATTXT_PROCESSCHA)) { // start data processing
        EndProcess(); // end data processing
        EndWaitCursor();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    if (!pLoudness->SmoothData(2)) { // smooth smoothed loudness two times
        EndProcess(); // end data processing
        EndWaitCursor();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    // calculate current processing position (copied but not understood from WINCECIL)
    dwLoopStart = ((pDoc->GetSamplesPerSec() / (2 * MAX_CALCULATION_FREQUENCY)
                    + pDoc->GetSamplesPerSec() / pUttParm->nMinFreq) / 2
                   + CALCULATION_DATALENGTH(pDoc->GetSamplesPerSec()) / 2 + 5) / CALCULATION_INTERVAL(pDoc->GetSamplesPerSec()) + 1;
    dwDataSize = pLoudness->GetDataSize();
    dwLoopEnd = dwDataSize - dwLoopStart; // end of loop
    // create the temporary change file
    if (!CreateTempFile(_T("CHA"))) { // creating error
        EndProcess(); // end data processing
        EndWaitCursor();
        SetDataInvalid();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // process smoothed loudness and quality data into change data
    short int * pChange = (short int *)m_lpBuffer; // pointer to smoothed pitch data
    DWORD dwChangeCount = 0; // reset processed data counter
    dwLoopPos = 0;
    DWORD dwChangeBlock = 0;
    // prepare source data
    short int * pQuality = (short int *)GetQualityData(dwChangeBlock, TRUE); // read quality data block
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
        *pChange = (short)(((long)*pQuality++ * 200 * lMultiplier) / ((long)*pLoudData++ + 70) / lMultiplier);
        if ((dwLoopPos < dwLoopStart) || (dwLoopPos >= dwLoopEnd)) {
            *pChange = 0;
        }
        if (*pChange > m_nMaxValue) {
            m_nMaxValue = *pChange;    // save maximum value
        }
        pChange++;
        dwLoopPos++;
        // set progress bar
        SetProgress(nProgress + (int)(100 * dwLoopPos / dwDataSize / (DWORD)nLevel));
        if (IsCanceled()) {
            return Exit(PROCESS_CANCELED);    // process canceled
        }
        if ((++dwChangeCount == GetProcessBufferSize() / 2) || (dwLoopPos >= dwDataSize)) { 
			// change buffer is full or processing finished
            // write the processed change data block
            try {
                Write(m_lpBuffer, dwChangeCount * 2);
            } catch (CFileException * e) {
                // error writing file
                ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
				// error, writing failed
				e->Delete();
				return Exit(PROCESS_ERROR);
            }
			// reset pointer to begin of pitch data buffer
			pChange = (short int *)m_lpBuffer;
            dwChangeBlock += dwChangeCount;
			// reset processed data counter
			dwChangeCount = 0;
            if (dwChangeCount < dwDataSize) {
                // read next source data blocks
				// read quality data block
				pQuality = (short int *)GetQualityData(dwChangeBlock, TRUE);
				// read smoothed loudness data block
				pLoudData = (short int *)pLoudness->GetProcessedData(dwChangeBlock, TRUE);
                if ((!pQuality) || (!pLoudData)) {
					// error, reading failed
					return Exit(PROCESS_ERROR);
                }
            }
        }
    }
	// calculate the actual progress
	nProgress = nProgress + (int)(100 / nLevel);
    // close the temporary change file and read the status
	// close the file
	CloseTempFile();
	// smooth data two times
	bRes = SmoothData(2);
    if (bRes) {
		// end data processing
		EndProcess((nProgress >= 95));
    } else {
        EndProcess();
    }
    EndWaitCursor();
    SetDataReady(bRes);
    if (bRes) {
        return MAKELONG(nLevel, nProgress);
    } else {
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
}

/***************************************************************************/
// CProcessChange::GetQualityData Read processed quality data from temp. file
// Reads a block of processed quality data from the temporary file into the
// processed quality data buffer and returns the pointer to the data. The
// user gives the data offset (data number) in the file to tell what data he
// needs. If bBlockBegin is set, this given offset data will always be at the
// top of the data block (where the returned pointer points to). If
// bBlockBegin is not set, the data block is not reread, if the given offset
// data is already somewhere in the buffer, and only the actual pointer to
// the data block will be returned. Processed data is always 16bit data!
/***************************************************************************/
void * CProcessChange::GetQualityData(DWORD dwOffset, BOOL bBlockBegin) {
    if (((!bBlockBegin && !m_bQualityBlockBegin) && ((dwOffset >= m_dwQualityBufferOffset) && (dwOffset < m_dwQualityBufferOffset + GetProcessBufferSize() / 2)))
            || ((bBlockBegin) && (m_dwQualityBufferOffset == dwOffset))) {
        // this data is actually in buffer
        return m_lpQualityData; // return pointer to data
    } else { // new data block has to be read
        m_bQualityBlockBegin = bBlockBegin;
        if (bBlockBegin) {
            m_dwQualityBufferOffset = dwOffset;    // given offset ist first sample in data block
        } else {
            m_dwQualityBufferOffset = dwOffset - (dwOffset % (GetProcessBufferSize() / 2));    // new block offset
        }
        // open the temporary file
        if (!m_pQualityFile->Open(m_qualityFileStatus.m_szFullName, CFile::modeRead | CFile::shareExclusive)) {
            // error opening file
            ErrorMessage(IDS_ERROR_OPENTEMPFILE, m_qualityFileStatus.m_szFullName);
            m_pQualityFile->Abort();
            return NULL;
        }
        // find the right position in the data
        if (m_dwQualityBufferOffset != 0L) {
            try {
                m_pQualityFile->Seek(m_dwQualityBufferOffset * 2, CFile::begin);
            } catch (CFileException * e) {
                // error seeking file
                ErrorMessage(IDS_ERROR_READTEMPFILE, m_qualityFileStatus.m_szFullName);
                m_pQualityFile->Abort();
				e->Delete();
                return NULL;
            }
        }
        // read the processed quality data block
        try {
            m_pQualityFile->Read((HPSTR)m_lpQualityData, GetProcessBufferSize());
        } catch (CFileException * e) {
            // error reading file
            ErrorMessage(IDS_ERROR_READTEMPFILE, m_qualityFileStatus.m_szFullName);
            m_pQualityFile->Abort();
			e->Delete();
            return NULL;
        }
        // close the quality data temp file

        if (m_pQualityFile) {
            m_pQualityFile->Abort();
        }

        // return the new data pointer
        return m_lpQualityData;
    }
}

/***************************************************************************/
// CProcessChange::GetQualityData Read processed quality data from temp. file
// Reads one data of processed quality data and returns it. The user gives
// the data offset (data number) in the file to tell what data he needs. If
// bBlockBegin is set, this given offset data will always be at the top of
// the data block (where the returned pointer points to). If bBlockBegin is
// not set, the data block is not reread, if the given offset data is already
// somewhere in the buffer, and only the actual pointer to the data block
// will be returned. Processed data is always 16bit data!
/***************************************************************************/
int CProcessChange::GetQualityData(DWORD dwOffset, BOOL * bRes) {
    if ((dwOffset >= m_dwQualityBufferOffset) && (dwOffset < m_dwQualityBufferOffset + GetProcessBufferSize() / 2)) {
        // this data is actually in buffer
        short int * lpData = (short int *)m_lpQualityData; // cast buffer pointer
        return *(lpData + (dwOffset - m_dwQualityBufferOffset)); // return data
    } else { // new data block has to be read
        m_dwQualityBufferOffset = dwOffset - (dwOffset % (GetProcessBufferSize() / 2)); // new block offset
        // open the temporary file
        if (!m_pQualityFile->Open(m_qualityFileStatus.m_szFullName, CFile::modeRead | CFile::shareExclusive)) {
            // error opening file
            ErrorMessage(IDS_ERROR_OPENTEMPFILE, m_qualityFileStatus.m_szFullName);
            m_pQualityFile->Abort();
            *bRes = FALSE; // set operation result
            return 0;
        }
        // find the right position in the data
        if (m_dwQualityBufferOffset != 0L) {
            try {
                m_pQualityFile->Seek(m_dwQualityBufferOffset * 2, CFile::begin);
            } catch (CFileException * e) {
                // error seeking file
                ErrorMessage(IDS_ERROR_READTEMPFILE, m_qualityFileStatus.m_szFullName);
                m_pQualityFile->Abort();
				// set operation result
				*bRes = FALSE;
                e->Delete();
				return 0;
            }
        }
        // read the processed quality data block
        try {
            m_pQualityFile->Read((HPSTR)m_lpQualityData, GetProcessBufferSize());
        } catch (CFileException * e) {
            // error reading file
            ErrorMessage(IDS_ERROR_READTEMPFILE, m_qualityFileStatus.m_szFullName);
            m_pQualityFile->Abort();
			// set operation result
			*bRes = FALSE;
            e->Delete();
			return 0;
        }
        // close the quality data temp file
        if (m_pQualityFile) {
            m_pQualityFile->Abort();
        }

        // return the data
        short int * lpData = (short int *)m_lpQualityData; // cast buffer pointer
        return *(lpData + (dwOffset - m_dwQualityBufferOffset)); // return data
    }
}

