/////////////////////////////////////////////////////////////////////////////
// sa_p_cha.cpp:
// Implementation of the CProcessChange class.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "sa_process.h"
#include "sa_p_cha.h"
#include "sa_p_lou.h"
#include "sa_p_smlou.h"
#include "param.h"
#include "ScopedCursor.h"
#include "funcs.h"

//###########################################################################
// CProcessChange
// class to calculate change values for wave data. Change is calculated by
// using acf best fit residual (quality) and smoothed loudness results,
// produced by processing smoothed loudness and raw pitch.

CProcessChange::CProcessChange(Context& context) : CProcess(context) {
    qualityFilename.clear(); // no file name
    m_hQualityData = NULL;
    m_nQualityMaxValue = 0;
    m_dwQualityBufferOffset = UNDEFINED_OFFSET; // buffer undefined, force buffer reload
    m_bQualityBlockBegin = FALSE;
}

CProcessChange::~CProcessChange() {
    // delete the temporary quality file
    FileUtils::Remove(qualityFilename.c_str());
    // free the quality buffer memory
    if (m_hQualityData) {
        ::GlobalUnlock(m_hQualityData);
        ::GlobalFree(m_hQualityData);
    }
}

/***************************************************************************/
// CProcessChange::Exit Exit on Error
// Standard exit function if an error occured.
/***************************************************************************/
long CProcessChange::Exit(int nError) {
    FileUtils::Remove(qualityFilename.c_str());
    qualityFilename.clear();
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
    FileUtils::Remove(qualityFilename.c_str());
    qualityFilename.clear();
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
long CProcessChange::Process(void * pCaller, int nProgress, int nLevel) {

    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    if (IsDataReady()) {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    }
    // create raw pitch data, its quality and smoothed loudness output is needed to calculate change
    CProcessSmoothLoudness * pLoudness = (CProcessSmoothLoudness *)model.GetSmoothLoudness(); // get pointer to loudness object
    long lResult = pLoudness->Process(pCaller, nProgress, ++nLevel); // process data
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
    CScopedCursor cursor(target);
    // start data processing
    if (!StartProcess(pCaller, PROCESSPIT, FALSE)) {
        // end data processing
        EndProcess();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // allocate global buffer for the processed quality data
    // not yet allocated
    if (!m_hQualityData) {
        // allocate memory
        m_hQualityData = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, GetProcessBufferSize());
        if (!m_hQualityData) {
            // memory allocation error
            app.ErrorMessage(IDS_ERROR_MEMALLOC);
            // end data processing
            EndProcess();
            return MAKELONG(PROCESS_ERROR, nProgress);
        }
        m_lpQualityData = (BPTR)::GlobalLock(m_hQualityData); // lock memory
        if (!m_lpQualityData) {
            // memory lock error
            app.ErrorMessage(IDS_ERROR_MEMLOCK);
            ::GlobalFree(m_hQualityData);
            m_hQualityData = NULL;
            EndProcess(); // end data processing
            return MAKELONG(PROCESS_ERROR, nProgress);
        }
    }

    CUttParm uttParm = {};
    model.GetUttParm(&uttParm); // get sa parameters utterance member data
    WORD nBlockAlign = model.GetBlockAlign();
    // calculate current processing position (copied but not understood from WINCECIL)
    DWORD dwLoopPos, dwLoopStart;
    if (uttParm.nMinFreq == 0) {
        uttParm.nMinFreq = 50;    // to prevent 0 divisions
    }
    if (uttParm.nMaxFreq == 0) {
        uttParm.nMaxFreq = 500;    // to prevent 0 divisions
    }
    dwLoopStart = ((model.GetSamplesPerSec() / (2 * MAX_CALCULATION_FREQUENCY)
                    + model.GetSamplesPerSec() / uttParm.nMinFreq) / 2
                   + CALCULATION_DATALENGTH(model.GetSamplesPerSec()) / 2 + 5) / CALCULATION_INTERVAL(model.GetSamplesPerSec()) + 1;
    dwLoopPos = 0;
    DWORD dwDataPos = dwLoopStart * CALCULATION_INTERVAL(model.GetSamplesPerSec()) - CALCULATION_DATALENGTH(model.GetSamplesPerSec()) / 2;
    if (nBlockAlign > 1) {
        dwDataPos *= 2;    // 16 bit data
    }

    TCHAR szTempPath[_MAX_PATH];
    FileUtils::GetTempFileName(_T("QUA"), szTempPath, _countof(szTempPath));
    // create and open the file
    ofstream file;
    file.open(szTempPath, std::fstream::trunc | std::fstream::out | std::fstream::binary);
    if (!file.is_open() || file.bad() || file.fail()) {
        // error opening file
        app.ErrorMessage(IDS_ERROR_OPENTEMPFILE, (LPTSTR)szTempPath);
        return Exit(PROCESS_ERROR);
    }

    // calculate lags at which to look for smallest autocorrelation coefficient
    int nMinLag = (int)(model.GetSamplesPerSec() / (DWORD)uttParm.nMaxFreq);
    int nMaxLag = (int)(model.GetSamplesPerSec() / (DWORD)uttParm.nMinFreq);
    
    // to be filled with lags at 5% intervals between nMinLag and nMaxLag
    int naACFlags[80] = {}; 
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
    BPTR pSRDdata = pLoudness->GetSmoothRawData(dwBlockPos, TRUE);
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
                int nInnerLoop = CALCULATION_DATALENGTH(model.GetSamplesPerSec()) / nACFinterval + 1;
                if (nBlockAlign > 1) // 16 bit data
                    dwResult = CalculateACF16(pLoudness, (unsigned char *)pSRDdata, &bRes, nACFinterval, nLagData, nMaxLag, &dwBlockPos, dwDataPos, nInnerLoop);
                else // 8 bit data
                    dwResult = CalculateACF8(pLoudness, (unsigned char *)pSRDdata, &bRes, nACFinterval, nLagData, nMaxLag, &dwBlockPos, dwDataPos, nInnerLoop);
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
                int nInnerLoop = CALCULATION_DATALENGTH(model.GetSamplesPerSec()) / nACFinterval;
                if (nBlockAlign > 1) // 16 bit data
                    dwResult = CalculateACF16(pLoudness, (unsigned char *)pSRDdata, &bRes, nACFinterval, nLagData, nLagHigh, &dwBlockPos, dwDataPos, nInnerLoop);
                else // 8 bit data
                    dwResult = CalculateACF8(pLoudness, (unsigned char *)pSRDdata, &bRes, nACFinterval, nLagData, nLagHigh, &dwBlockPos, dwDataPos, nInnerLoop);
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
                int nInnerLoop = CALCULATION_DATALENGTH(model.GetSamplesPerSec()) / nACFinterval;
                if (nBlockAlign > 1) // 16 bit data
                    dwResult = CalculateACF16(pLoudness, (unsigned char *)pSRDdata, &bRes, nACFinterval, nLagData, nLagData, &dwBlockPos, dwDataPos, nInnerLoop);
                else // 8 bit data
                    dwResult = CalculateACF8(pLoudness, (unsigned char *)pSRDdata, &bRes, nACFinterval, nLagData, nLagData, &dwBlockPos, dwDataPos, nInnerLoop);
                // if it fits reasonably well...
                if ((2 * dwResult) < (3 * dwMinCoeff)) {
                    nFoundLag = nLagData;
                } else {
                    // ...else check for 3x fundamental
                    if ((nFoundLag >= (nMinLag * 3)) && bRes) {
                        nLagData = nFoundLag / 3;
                        int nInnerLoop = CALCULATION_DATALENGTH(model.GetSamplesPerSec()) / nACFinterval;
                        if (nBlockAlign > 1) // 16 bit data
                            dwResult = CalculateACF16(pLoudness, (unsigned char *)pSRDdata, &bRes, nACFinterval, nLagData, nLagData, &dwBlockPos, dwDataPos, nInnerLoop);
                        else // 8 bit data
                            dwResult = CalculateACF8(pLoudness, (unsigned char *)pSRDdata, &bRes, nACFinterval, nLagData, nLagData, &dwBlockPos, dwDataPos, nInnerLoop);
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
                nPitchResult = (short)((PRECISION_MULTIPLIER * (unsigned long)(model.GetSamplesPerSec())) / (unsigned long)nFoundLag);
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
                dwDataPos += CALCULATION_INTERVAL(model.GetSamplesPerSec()) * 2;    // 16 bit data
            } else {
                dwDataPos += CALCULATION_INTERVAL(model.GetSamplesPerSec());    // 8 bit data
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
            file.write((BPTR)m_lpQualityData, dwPitchCount * 2);
            if (file.bad() || file.fail()) {
                // error writing file
                app.ErrorMessage(IDS_ERROR_WRITETEMPFILE, qualityFilename.c_str());
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
    if (file.is_open()) {
        file.close();
    }

    wstring szFullName = qualityFilename;
	// read the status
    long size = ::get_file_size(qualityFilename.c_str());
    if (size < 2 * sizeof(int)) {
		// error, not enough data
		return Exit(PROCESS_ERROR);
	}

	// memory allocation failed or previous processing error
	if (nLevel < 0) {
		// end data processing
		EndProcess();
        if ((nLevel == PROCESS_CANCELED)) {
			// set your own cancel flag
			CancelProcess();
        }
        return MAKELONG(nLevel, nProgress);
    }

    // start change process data processing
    if (!StartProcess(pCaller, PROCESSCHA)) { 
        EndProcess(); // end data processing
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    if (!pLoudness->SmoothData(2)) { // smooth smoothed loudness two times
        EndProcess(); // end data processing
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    // calculate current processing position (copied but not understood from WINCECIL)
    dwLoopStart = ((model.GetSamplesPerSec() / (2 * MAX_CALCULATION_FREQUENCY)
                    + model.GetSamplesPerSec() / uttParm.nMinFreq) / 2
                   + CALCULATION_DATALENGTH(model.GetSamplesPerSec()) / 2 + 5) / CALCULATION_INTERVAL(model.GetSamplesPerSec()) + 1;
    dwDataSize = pLoudness->GetDataSize();
    dwLoopEnd = dwDataSize - dwLoopStart; // end of loop

    // create the temporary change file
    if (!CreateTempFile(_T("CHA"))) { // creating error
        EndProcess(); // end data processing
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
            file.write(m_lpBuffer, dwChangeCount * 2);
            if (file.bad() || file.fail()) {
                // error writing file
                app.ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
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
    SetDataReady(bRes);
    return (bRes) ? MAKELONG(nLevel, nProgress) : MAKELONG(PROCESS_ERROR, nProgress);
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
void * CProcessChange::GetQualityData( DWORD dwOffset, BOOL bBlockBegin) {

    if (((!bBlockBegin && !m_bQualityBlockBegin) && ((dwOffset >= m_dwQualityBufferOffset) && (dwOffset < m_dwQualityBufferOffset + GetProcessBufferSize() / 2)))
            || ((bBlockBegin) && (m_dwQualityBufferOffset == dwOffset))) {
        // this data is actually in buffer
        return m_lpQualityData; // return pointer to data
    }

    // new data block has to be read
    m_bQualityBlockBegin = bBlockBegin;
    if (bBlockBegin) {
        m_dwQualityBufferOffset = dwOffset;    // given offset ist first sample in data block
    } else {
        m_dwQualityBufferOffset = dwOffset - (dwOffset % (GetProcessBufferSize() / 2));    // new block offset
    }

    // open the temporary file
    ifstream file;
    file.open(qualityFilename.c_str(), std::fstream::in | std::fstream::binary);
    if (!file.is_open() || file.bad() || file.fail()) {
        // error opening file
        app.ErrorMessage(IDS_ERROR_OPENTEMPFILE, qualityFilename.c_str());
        return NULL;
    }
    // find the right position in the data
    if (m_dwQualityBufferOffset != 0L) {
        file.seekg(m_dwQualityBufferOffset * 2, ios_base::beg);
        if (file.bad() || file.fail()) {
            // error seeking file
            app.ErrorMessage(IDS_ERROR_READTEMPFILE, qualityFilename.c_str());
            return NULL;
        }
    }
    // read the processed quality data block
    file.read((BPTR)m_lpQualityData, GetProcessBufferSize());
    if (file.bad()) {
        // error reading file
        app.ErrorMessage(IDS_ERROR_READTEMPFILE, qualityFilename.c_str());
        return NULL;
    }

    // return the new data pointer
    return m_lpQualityData;
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
    }

    // new data block has to be read
    m_dwQualityBufferOffset = dwOffset - (dwOffset % (GetProcessBufferSize() / 2)); // new block offset
    
    // open the temporary file
    ifstream file;
    file.open(qualityFilename.c_str(), std::ifstream::in | std::ifstream::binary);
    if (!file.is_open() || file.bad() || file.fail()) {
        // error opening file
        app.ErrorMessage(IDS_ERROR_OPENTEMPFILE, qualityFilename.c_str());
        *bRes = FALSE; // set operation result
        return 0;
    }
    // find the right position in the data
    if (m_dwQualityBufferOffset != 0L) {
        file.seekg(m_dwQualityBufferOffset * 2, std::ifstream::beg);
        if (file.bad() || file.fail()) {
            // error seeking file
            app.ErrorMessage(IDS_ERROR_READTEMPFILE, qualityFilename.c_str());
			*bRes = FALSE;
			return 0;
        }
    }
    // read the processed quality data block
    file.read((BPTR)m_lpQualityData, GetProcessBufferSize());
    if (file.bad()) {
        // error reading file
        app.ErrorMessage(IDS_ERROR_READTEMPFILE, qualityFilename.c_str());
		// set operation result
		*bRes = FALSE;
		return 0;
    }

    // return the data
    short int * lpData = (short int *)m_lpQualityData; // cast buffer pointer
    return *(lpData + (dwOffset - m_dwQualityBufferOffset)); // return data
}

DWORD CProcessChange::GetQualityDataSize() {
    // return processed data size in words (16 bit)
    return (DWORD)(get_file_size(qualityFilename.c_str())/2);
}

int CProcessChange::GetQualityMaxValue() {
    return m_nQualityMaxValue;   // return maximum value
}
