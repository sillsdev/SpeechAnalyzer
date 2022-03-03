/////////////////////////////////////////////////////////////////////////////
// sa_p_InstantaneousPower.cpp:
// Implementation of the CProcessInstantaneousPower class.
// Author: Steve MacLean
// copyright 2002 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "sa_process.h"
#include "sa_p_InstantaneousPower.h"
#include "sa_p_raw.h"
#include "sa_p_glo.h"
#include "Butterworth.h"
#include "ScopedCursor.h"

static const double pi = 3.14159265358979323846264338327950288419716939937511;

// Cascade a zero/pole of the form (s-a)/(s+a) or (s+a)/(s-a) which has a A(w) response = 1
// but a non constant phase response
void CascadeAllPass(CZTransform & zTransform, double dCenterFrequency, double dSamplingRate) {
    double dRelativeFrequency = pi*dCenterFrequency/dSamplingRate;
    double d = (1 - (dRelativeFrequency))/(1 + (dRelativeFrequency));
    double numerator[2];
    double denominator[2];

    numerator[0] = d;
    numerator[1] = -1;
    denominator[0] = 1.;
    denominator[1] = -d;

    zTransform *= CZTransform(1, numerator, denominator);
}
/***************************************************************************/
// CProcessInstantaneousPower::Process
/***************************************************************************/
long CProcessInstantaneousPower::Process(void * pCaller, int nProgress, int nLevel) {
    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    if (IsDataReady()) {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    }

    CScopedCursor cursor(target);
    if (!StartProcess(pCaller)) { // memory allocation failed
        EndProcess(); // end data processing
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    CProcessRaw * pRaw = model.GetRaw();
    short nResult = LOWORD(pRaw->Process( this, nLevel));
    // Exit if error has occurred or the spectrum process has been canceled.
    if (nResult < 0) {
        SetDataReady(FALSE);
        if (nResult == PROCESS_CANCELED) {
            CancelProcess();
        }
        return Exit(nResult);
    }

    CProcessIIRFilter * phaseFilter = (CProcessIIRFilter *)model.GetHilbert();

    long lResult = phaseFilter->Process(pCaller, nProgress, ++nLevel);
    nLevel = (short int)LOWORD(lResult);
    if ((nLevel == PROCESS_CANCELED)) {
        nProgress = HIWORD(lResult);
        CancelProcess(); // set your own cancel flag
        return MAKELONG(nLevel, nProgress);
    }

    // create the temporary file
    if (!CreateTempFile(_T("TEA"))) { // creating error
        EndProcess(); // end data processing
        SetDataInvalid();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    double dScale = 0.707/2;

    DWORD dwWaveSize = phaseFilter->GetDataSize();
    DWORD dwBufferSize = GetProcessBufferSize();
    short * pProcData = (short *)m_lpBuffer;
    DWORD dwProcDataCount = 0;

    *pProcData++ = 0;
    dwProcDataCount++;

    m_nMaxValue = 0;

    double dLastData = 0.;
    double dLastPhasedData = 0.;

    for (DWORD dwWaveOffset = 1; dwWaveOffset < dwWaveSize - 1; dwWaveOffset++) {
        short * pData = (short *)pRaw->GetProcessedDataBlock((dwWaveOffset-1)*sizeof(short),3*sizeof(short));
        short * pDataPhased = (short *)phaseFilter->GetProcessedDataBlock((dwWaveOffset-1)*sizeof(short),3*sizeof(short));
        if (!pData || !pDataPhased) { // reading failed
            EndProcess(); // end data processing
            SetDataInvalid();
            return MAKELONG(-1, nProgress);
        }

        double dTeager = 3.*pData[1]*pData[1]-2.*pData[1]*pData[0]-2.*pData[1]*pData[2]+0.5*pData[0]*pData[0]+0.5*pData[2]*pData[2];
        double dTeagerPhased = 4*(3.*pDataPhased[1]*pDataPhased[1]-2.*pDataPhased[1]*pDataPhased[0]-2.*pDataPhased[1]*pDataPhased[2]+0.5*pDataPhased[0]*pDataPhased[0]+0.5*pDataPhased[2]*pDataPhased[2]);

        double dDataPE = pData[1] - dLastData;
        double dPhasedDataPE = 2*(pDataPhased[1] - dLastPhasedData);

        double dPhaseShifted = dPhasedDataPE * dPhasedDataPE + dDataPE * dDataPE;

        dLastData = 0;//pData[1];
        dLastPhasedData = 0;//pDataPhased[1];

        //    short nValue = short(0.5*dScale*(((dTeager)+(dTeagerPhased)) + 0.5));
        //    short nValue = short(dScale*((dTeager+dTeagerPhased - dPhaseShifted) + 0.5));

        short nValue = short(dScale*sqrt(dPhaseShifted) + 0.5);

        *pProcData++ = nValue;
        dwProcDataCount++;

        if (dwProcDataCount >= dwBufferSize / sizeof(short)) {
            // write the processed data block
            try {
                Write(m_lpBuffer, (UINT)dwProcDataCount * sizeof(short));
            } catch (CFileException * e) {
				// display message
                app.AfxMessageBox(IDS_ERROR_WRITETEMPFILE, MB_OK | MB_ICONEXCLAMATION, 0);
				// end data processing
				EndProcess();
                SetDataInvalid();
                e->Delete();
				return MAKELONG(-1, nProgress);
            }
            dwProcDataCount = 0; // reset counter
            pProcData = (short *)m_lpBuffer; // reset pointer to begin of processed data buffer
            // set progress bar
            SetProgress(nProgress + (int)(100 * dwWaveOffset / dwWaveSize / (DWORD)nLevel));
            if (IsCanceled()) {
                EndProcess(); // end data processing
                SetDataInvalid();
                return MAKELONG(PROCESS_CANCELED, nProgress);
            }
        }
        m_nMaxValue = max(m_nMaxValue, nValue);
    }

    *pProcData++ = 0;
    dwProcDataCount++;

    if (dwProcDataCount) {
        try {
            Write(m_lpBuffer, (UINT)dwProcDataCount * sizeof(short));
        } catch (CFileException * e) {
			// display message
			app.AfxMessageBox(IDS_ERROR_WRITETEMPFILE, MB_OK | MB_ICONEXCLAMATION, 0);
			// end data processing
			EndProcess();
            SetDataInvalid();
            e->Delete();
			return MAKELONG(-1, nProgress);
        }
    }

    // calculate the actual progress
    nProgress = nProgress + (int)(100 / nLevel);
    // close the temporary file and read the status
    CloseTempFile(); // close the file
    EndProcess((nProgress >= 95)); // end data processing
    SetDataReady();
    return MAKELONG(nLevel, nProgress);
}

