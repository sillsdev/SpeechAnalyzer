/////////////////////////////////////////////////////////////////////////////
// sa_p_lou.cpp:
// Implementation of the CProcessLoudness classes.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//   1.5Test8
//        SDM added changes by CLW in 1.07a
//   6/21/2001
//        SDM changed CProcessLoudness to return RootMeanSquare values scaled for
//            16-bit scale.
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "sa_process.h"
#include "sa_p_lou.h"
#include "sa_p_grappl.h"
#include "param.h"
#include "ScopedCursor.h"

//###########################################################################
// CProcessLoudness
//
// Techically, this class does not calculate loudness, which is a perceptual
// phenomenon.  It calculates rms magnitude (which when plotted on a dB scale
// becomes relative intensity).  It accumulates the average squared energy within
// a calculation window, the width of which is based on the sampling frequency
// and the lowest pitch period.  The calculations are done at a lower rate than
// the sampling frequency, since the averaging window acts as a lowpass filter.
// reducing the frequency content of the intensity signal and therefore
// permitting a lower sampling rate.
//
// Some improvements to consider:
// 1. Instead of using a rectangular window, use a Kaiser window having
//    stopband attenuation greater than the signal-to-noise quantization
//    ratio. This will reduce sidelobe leakage.
// 2. Adapt the bandwidth of the window to the local pitch frequency in
//    voiced speech and to some nominal value for unvoiced speech.  This
//    will track the envelope more precisely.
// 3. Eliminate the smoothing operation.  It is unnecessary.

/***************************************************************************/
// CProcessLoudness::Process Processing loudness data
// The processed loudness data is stored in a temporary file. To create it
// helper functions of the base class are used. While processing a process
// bar, placed on the status bar, has to be updated. The level tells which
// processing level this process has been called, start process start on
// which processing percentage this process starts (for the progress bar).
// The status bar process bar will be updated depending on the level and the
// progress start. The return value returns the highest level througout the
// calling queue, or -1 in case of an error in the lower word of the long
// value and the end process progress percentage in the higher word.
/***************************************************************************/
long CProcessLoudness::Process(void * pCaller, int nProgress, int nLevel) {

    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    if (IsDataReady()) {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    }
    CProcessGrappl * pAutoPitch = (CProcessGrappl *)model.GetGrappl();
    short int nResult = LOWORD(pAutoPitch->Process(this)); // process data

    CScopedCursor cursor(target);
    if (!StartProcess(pCaller, PROCESSLOU)) { // memory allocation failed
        EndProcess(); // end data processing
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // create the temporary file
    if (!CreateTempFile(_T("LOU"))) { // creating error
        EndProcess(); // end data processing
        SetDataInvalid();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // process raw data into loudness data
    DWORD dwDataSize = model.GetDataSize(); // size of raw data
    DWORD nSmpSize = model.GetSampleSize();
    // Finish pitch processing if necessary.
    UINT nCalcDataLength = CALCULATION_DATALENGTH(model.GetSamplesPerSec());  //!!this should be based on sampling frequency
    if (nResult >= 0 && pAutoPitch->GetMaxValue()) {
        nCalcDataLength = (UINT)(2*model.GetSamplesPerSec() / (pAutoPitch->GetMaxValue() / PRECISION_MULTIPLIER));
    }
    UINT nCalcInterval = nCalcDataLength / 3;   // more than 2 x bandwidth
    nCalcDataLength = nCalcInterval * 3;

    BPTR pRawData = NULL, pBlockStart; // pointers to raw data
    short int * pLoudData = (short int *)m_lpBuffer; // pointer to loudness data
    // calculate current processing position
    DWORD dwDataPos, dwLoudStart;
    dwLoudStart = static_cast<DWORD>(ceil(nCalcDataLength / 2. / nCalcInterval));
    dwDataPos = dwLoudStart * nCalcInterval - nCalcDataLength / 2;
    if (nSmpSize > 1) {
        dwDataPos *= 2;    // 16 bit data
    }
    // prepare loudness loop
    DWORD dwLoudCount = 0;
    for (; dwLoudCount < dwLoudStart; dwLoudCount++) {
        *pLoudData++ = 0; // initialize buffer
    }
    /**************************************************************************/
    // CLW 10/28/98 nFirstOffset keeps track of the offset into the block. This
    // is important because the loudness calculation is not on consecutive
    // samples.
    /**************************************************************************/
    double dResult = 0;
    double dOldResult[3] = {-1,-1,-1};
    UINT nCalcLoop = 0; // calculation loop counter
    DWORD dwNextBlock = 0;
    while (dwDataPos < dwDataSize) { // processing loop
        if (dwNextBlock <= dwDataPos) {
            pBlockStart = model.GetWaveData(dwDataPos); // get pointer to data block
            if (!pBlockStart) {
                return Exit(PROCESS_ERROR);    // error, reading failed
            }
            UINT nFirstOffset = (UINT)(dwDataPos % GetProcessBufferSize()); // Find offset into block CLW 10/28/98
            pRawData = pBlockStart + nFirstOffset;
            dwNextBlock = dwDataPos + GetProcessBufferSize() - nFirstOffset;
        }
        if (nSmpSize == 1) { // 8 bit data
            // process a byte
            double dRes = ((unsigned char)*pRawData - 128)*256.;
            dResult += dRes*dRes;
            pRawData += 1; // increment position in raw data
            dwDataPos += 1;
        } else { // 16 bit data
            // process a word
            short * pTemp = (short *)pRawData;
            double dRes = *pTemp;
            dResult += dRes*dRes;
            pRawData += 2; // increment position in raw data
            dwDataPos += 2;
        }
        // if loudness result ready, store it
        if ((++nCalcLoop >= nCalcInterval) || (dwDataPos >= dwDataSize)) {
            dOldResult[2] = dOldResult[1];
            dOldResult[1] = dOldResult[0];
            dOldResult[0] = dResult;
            dResult = (dOldResult[0] + dOldResult[1] + dOldResult[2])/(nCalcLoop + 2 * nCalcInterval);

            nCalcLoop = 0; // reset loop counter

            // set progress bar
            SetProgress(nProgress + (int)(100 * dwDataPos / dwDataSize / (DWORD)nLevel));
            if (IsCanceled()) {
                return Exit(PROCESS_CANCELED);    // process canceled
            }

            if (dOldResult[2] >= 0) {
                *pLoudData++ = (short)(sqrt(dResult) + 0.5); // rms magnitude
                if ((++dwLoudCount == GetProcessBufferSize() / 2) || (dwDataPos + nCalcDataLength/2 >= dwDataSize)) { // loudness buffer is full or processing finished
                    // write the processed data block
                    if (!Write(m_lpBuffer, dwLoudCount * 2)) {
                        // error writing file
                        app.ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
                        return Exit(PROCESS_ERROR);
                    }
                    *m_lpBuffer = 0;
                    if (dwDataPos >= dwDataSize) {
                        if (!Write(m_lpBuffer, 2)) {
                            // error writing file
                            app.ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
                            return Exit(PROCESS_ERROR);
                        }
                    }
                    dwLoudCount = 0; // reset counter
                    pLoudData = (short int *)m_lpBuffer; // reset pointer to begin of loudness data buffer
                }
            }
            dResult = 0;
        }
    }
    // calculate the actual progress
    nProgress = nProgress + (int)(100 / nLevel);
    // close the temporary file and read the status
    CloseTempFile(); // close the file
    if (GetDataSize() < 2) {
        return Exit(PROCESS_ERROR);    // error, not enough data
    }
    BOOL bResult = SmoothData(2); // smooth data two times
    EndProcess(((bResult) && (nProgress >= 95))); // end data processing
    SetDataReady(bResult);
    if (bResult) {
        return MAKELONG(nLevel, nProgress);
    } else {
        SetDataInvalid();
        return MAKELONG(PROCESS_ERROR, 100);
    }
}

void CProcessLoudness::SetDataInvalid() {
    CProcess::SetDataInvalid();
}
