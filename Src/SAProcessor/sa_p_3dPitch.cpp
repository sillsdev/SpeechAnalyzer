// sa_p_3dPitch.cpp : implementation file
//

#include "pch.h"
#include "sa_p_3dPitch.h"
#include "Butterworth.h"
#include "ScopedCursor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CProcess3dPitch::CProcess3dPitch(Context& context) : CProcess(context) {
    m_dFilterUpperFrequency = 1000.;
    m_dFilterLowerFrequency = 70.;
    m_nFilterOrder = 5;
}

static int ReadDataBlock(CProcessButterworth & source, DWORD dwStart, DWORD dwStop, DWORD dwPos, int wSmpSize);

/***************************************************************************/
// CProcess3dPitch::Process
/***************************************************************************/
long CProcess3dPitch::Process(void * pCaller, int nProgress, int nLevel) {
    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    // check if nested workbench processes
    int nOldLevel = nLevel; // save original level

    if ((nLevel == nOldLevel) && (IsDataReady())) {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    } else {
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
    // memory allocation failed or previous processing error
    if (!StartProcess(pCaller, PROCESSWBLP)) {
        EndProcess(); // end data processing
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // create the temporary file
    if (!CreateTempFile(_T("PCC"))) { // creating error
        EndProcess(); // end data processing
        SetDataInvalid();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // get source data size
    DWORD dwDataSize = model.GetDataSize();    // size of raw data
    DWORD wSmpSize = model.GetSampleSize();

    CProcessButterworth butterworth(context, Plain);
    butterworth.SetSourceProcess(NULL);
    butterworth.SetFilterFilter(TRUE);
    butterworth.LowPass(m_nFilterOrder, m_dFilterUpperFrequency);

    // first do forward pass
    long lResult = butterworth.Process(pCaller, nProgress, ++nLevel);
    nLevel = (short int)LOWORD(lResult);
    if ((nLevel == PROCESS_CANCELED)) {
        nProgress = HIWORD(lResult);
        CancelProcess(); // set your own cancel flag
        return MAKELONG(nLevel, nProgress);
    }

    // start processing loop
    for (DWORD dwIntegerPitchPeriod = 22; dwIntegerPitchPeriod < 480; dwIntegerPitchPeriod+= 5 /*dwIntegerPitchPeriod/16*/) {
        double sumXnXnt = 0.;
        // initialize to ~ -30dB noise to mask low amplitude signals (& kill divide by 0)
        double sumXnXn = 1024*1024*10;
        double sumXntXnt = 1024*1024*10;

        DWORD dwCorrelationLength = dwIntegerPitchPeriod > 40 ? dwIntegerPitchPeriod : 70;
        DWORD dwDataSamples = dwDataSize/wSmpSize;

        for (DWORD dwDataPos = 0; dwDataPos < dwDataSamples; dwDataPos+= 4) {
            // set progress bar
            if (IsCanceled()) {
                return Exit(PROCESS_CANCELED); // process canceled
            }

            DWORD dwBlockEnd = dwDataPos + dwIntegerPitchPeriod + dwCorrelationLength - 1;
            DWORD dwBlockStart = dwDataPos;

            if (dwBlockEnd < dwDataSamples) {
                double Xn = ReadDataBlock(butterworth, dwBlockStart, dwBlockEnd, dwDataPos + dwCorrelationLength - 1, wSmpSize);
                double Xnt = ReadDataBlock(butterworth, dwBlockStart, dwBlockEnd, dwDataPos + dwIntegerPitchPeriod + dwCorrelationLength - 1, wSmpSize);

                sumXnXn += Xn*Xn;
                sumXnXnt += Xn*Xnt;
                sumXntXnt += Xnt*Xnt;
            }

            short sResult = 0;

            if (dwDataPos > dwCorrelationLength) {
                double crossCorrelation = sumXnXnt/sqrt(sumXnXn*sumXntXnt);

                sResult = crossCorrelation > 0 ? short(crossCorrelation*1000. + 0.5) : short(0);

                double Xn = ReadDataBlock(butterworth, dwBlockStart, dwBlockEnd, dwDataPos, wSmpSize);
                double Xnt = ReadDataBlock(butterworth, dwBlockStart, dwBlockEnd, dwDataPos + dwIntegerPitchPeriod, wSmpSize);

                sumXnXn -= Xn*Xn;
                sumXnXnt -= Xn*Xnt;
                sumXntXnt -= Xnt*Xnt;
            }

            Write(&sResult, sizeof(short));
        }
    }

    nProgress = nProgress + (int)(100 / nLevel); // calculate the actual progress
    // close the temporary file and read the status
    CloseTempFile();                    // close the file
    EndProcess((nProgress >= 95));      // end data processing
    SetDataReady();
    return MAKELONG(nLevel, nProgress);
}

static int ReadDataBlock(CProcessButterworth & source, DWORD dwStart, DWORD dwStop, DWORD dwPos, int wSmpSize) {
    BYTE * pSourceData = (BYTE *) source.GetProcessedDataBlock(dwStart*wSmpSize, (dwStop - dwStart + 1)*wSmpSize);
    pSourceData += (dwPos - dwStart)*wSmpSize;
    int nData;
    if (wSmpSize == 1) { // 8 bit data
        if (!pSourceData) {
            assert(FALSE);
            trace("Failed reading source data\n");
            return 0;
        }
        BYTE bData;
        bData = *((BYTE *)pSourceData); // data range is 0...255 (128 is center)
        nData = bData - 128;
        nData *= 256;
    } else {              // 16 bit data
        if (!pSourceData) {
            assert(FALSE);
            trace("Failed reading source data\n");
            return 0;
        }
        nData = *((short int *)pSourceData);
    }
    return nData;
}
