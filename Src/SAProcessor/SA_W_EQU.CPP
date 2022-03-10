/////////////////////////////////////////////////////////////////////////////
// sa_w_equ.cpp:
// Implementation of the CProcessWbEquation class.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "sa_process.h"
#include "sa_w_equ.h"
#include "WbProcess.h"
#include "ScopedCursor.h"

//###########################################################################
// CProcessWbEquation
// Workbench process class to calculate new raw data after an equation given
// by the user.

CProcessWbEquation::CProcessWbEquation(Context& context) : CWbProcess( context, WbEquation) {
    m_szEquation.clear();
}

/***************************************************************************/
// CProcessWbEquation::CheckFunction Check validity of equation
/***************************************************************************/
BOOL CProcessWbEquation::CheckFunction(LPCSTR pszFunction) {
    if (pszFunction && strlen(pszFunction)!=0) {
        int nType = 0;
        ParseUtf8(pszFunction, &nType, 1.0);
        if (nType) {
            return TRUE;
        }
    }
    return FALSE;
}

/***************************************************************************/
// CProcessWbEquation::Process Processing new raw data after an equation
// The processed change data is stored in a temporary file. To create it
// helper functions of the base class are used. While processing a process
// bar, placed on the status bar, has to be updated. The level tells which
// processing level this process has been called, start process start on
// which processing percentage this process starts (for the progress bar).
// The status bar process bar will be updated depending on the level and the
// progress start. The return value returns the highest level throughout the
// calling queue, or -1 in case of an error in the lower word of the long
// value and the end process progress percentage in the higher word.
/***************************************************************************/
long CProcessWbEquation::Process(void * pCaller, int nProgress, int nLevel) {
    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    // check if nested workbench processes
    int nOldLevel = nLevel; // save original level
    CWbProcess * pLowerProcess = NULL;
    if (nLevel < MAX_FILTER_NUMBER) {
        pLowerProcess = frame.GetWbProcess( model.GetWbProcess() - 1, nLevel);
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
        if ((nLevel == PROCESS_CANCELED)) {
            CancelProcess();    // set your own cancel flag
        }
        SetDataInvalid();
    }

    if (nLevel < 0) { // previous processing error
        if ((nLevel == PROCESS_CANCELED)) {
            CancelProcess();    // set your own cancel flag
        }
        return MAKELONG(nLevel, nProgress);
    }

    // start process
    CScopedCursor cursor(target);
    if (!StartProcess(pCaller, PROCESSWBEQU)) { // memory allocation failed
        EndProcess(); // end data processing
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // create the temporary file
    if (!CreateTempFile(_T("EQU"))) { // creating error
        EndProcess(); // end data processing
        SetDataInvalid();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // get source data size
    DWORD dwDataSize;
    if (pLowerProcess) {
        dwDataSize = pLowerProcess->GetProcessedWaveDataSize();
    } else {
        dwDataSize = model.GetDataSize();// size of raw data
    }
    DWORD dwBufferSize = GetBufferSize();
    DWORD wSmpSize = model.GetSampleSize();
    BPTR pSourceData, pBlockStart;                 // pointers to source data
    BPTR pTargetData;                              // pointers to target data
    DWORD dwDataPos = 0;                            // data position pointer
    DWORD dwRangeStart = 0;
    DWORD dwRangeEnd = dwDataSize - wSmpSize;

    // start processing loop
    while (dwDataPos < dwDataSize) {
        // load source data
        if (pLowerProcess) {
            pBlockStart = pLowerProcess->GetProcessedWaveData(dwDataPos);
        } else {
            pBlockStart = model.GetAdjustedUnprocessedWaveData(dwDataPos);    // get pointer to raw data block
        }
        if (!pBlockStart) {
            return Exit(PROCESS_ERROR);    // error, reading failed
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
            // process this data point with the function parser from Z-Graph
            if (m_szEquation.length()!=0 && (dwDataPos >= dwRangeStart) && (dwDataPos <= dwRangeEnd)) {
                int nType = 0;
                double dData = (double)nData;
                if (!nData) {
                    dData = 1.7E-12;    // to protect parser from crashing
                }
                dData = (int)ParseUtf8(m_szEquation.c_str(), &nType, dData);
                if (!nType) {
                    return Exit(PROCESS_ERROR);    // error, parsing failed
                }
                nData = (int)dData;
                // do boundary check
                if (wSmpSize == 1) { // 8 bit data
                    if (dData > 127) {
                        nData = 127;
                    }
                    if (dData < -128) {
                        nData = -128;
                    }
                } else {              // 16 bit data
                    if (dData > (double)32767) {
                        nData = 0x7FFF;
                    }
                    if (dData < (double)-32767) {
                        nData = 0xFFFF;
                    }
                }
            }
            // save data
            if (wSmpSize == 1) { // 8 bit data
                bData = BYTE(nData + 128);
                *pTargetData++ = bData;
            } else {              // 16 bit data
                *pTargetData++ = (BYTE)nData;
                *pTargetData++ = (BYTE)(nData >> 8);
            }
            // set progress bar
            SetProgress(nProgress + (int)(100 * dwDataPos / dwDataSize / (DWORD)nLevel));
            if (IsCanceled()) {
                return Exit(PROCESS_CANCELED); // process canceled
            }
        }
        // result block is ready, store it
        if (dwDataPos >= dwBlockEnd) {
            // write the processed data block
            if (!Write(m_lpBuffer, dwBufferSize)) {
                // error writing file
                app.ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
				return Exit(PROCESS_ERROR);
            }
        }
    }
    nProgress = nProgress + (int)(100 / nLevel); // calculate the actual progress
    // close the temporary file and read the status
    CloseTempFile(); // close the file
    EndProcess((nProgress >= 95)); // end data processing
    SetDataReady();
    return MAKELONG(nLevel, nProgress);
}

static LPCSTR psz_equation = "equation";
static LPCSTR psz_text = "text";

/***************************************************************************/
// CProcessWbEquation::WriteProperties Write equation properties
/***************************************************************************/
void CProcessWbEquation::WriteProperties(CObjectOStream & obs) {
    obs.WriteBeginMarker(psz_equation);
    obs.WriteString(psz_text, m_szEquation.c_str());
    obs.WriteEndMarker(psz_equation);
}

/***************************************************************************/
// CProcessWbEquation::ReadProperties Read equation properties
/***************************************************************************/
bool CProcessWbEquation::ReadStreamString(CObjectIStream& stream, LPCSTR pszMarker, string& szResult) {
    size_t size = stream.GetBufferSize() + 1;
    array_ptr<char> buffer(size);
    bool result = stream.bReadString(pszMarker, buffer.get(), buffer.size());
    if (result) {
        szResult = buffer.get();
    }
    return result;
}

BOOL CProcessWbEquation::ReadProperties(CObjectIStream & obs) {
    string szSafe = m_szEquation;
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_equation)) {
        return FALSE;
    }
    while (!obs.bAtEnd()) {
        if (ReadStreamString(obs,psz_text,m_szEquation));
        else if (obs.bEnd(psz_equation)) {
            break;
        }
    }
    // check if properties changed
    if (szSafe != m_szEquation) {
        SetDataInvalid();
    }
    return TRUE;
}

double CProcessWbEquation::ParseUtf8(LPCSTR in, INT* att, double X, double Y) {
    return parse(in, att, X, Y);
}

