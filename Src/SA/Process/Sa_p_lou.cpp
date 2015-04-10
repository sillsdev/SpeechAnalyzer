/////////////////////////////////////////////////////////////////////////////
// sa_p_lou.cpp:
// Implementation of the CProcessLoudness
//                       CProcessSmoothLoudness classes.
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

#include "stdafx.h"
#include "Process.h"
#include "sa_p_lou.h"
#include "sa_p_gra.h"
#include "isa_doc.h"
#include "FileUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

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

/////////////////////////////////////////////////////////////////////////////
// CProcessLoudness construction/destruction/creation

/***************************************************************************/
// CProcessLoudness::CProcessLoudness Constructor
/***************************************************************************/
CProcessLoudness::CProcessLoudness() {
}

/***************************************************************************/
// CProcessLoudness::~CProcessLoudness Destructor
/***************************************************************************/
CProcessLoudness::~CProcessLoudness() {
}

/////////////////////////////////////////////////////////////////////////////
// CProcessLoudness helper functions

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
long CProcessLoudness::Process(void * pCaller, ISaDoc * pDoc, int nProgress, int nLevel) {

    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    if (IsDataReady()) {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    }
    CProcessGrappl * pAutoPitch = (CProcessGrappl *)pDoc->GetGrappl();
    short int nResult = LOWORD(pAutoPitch->Process(this, pDoc)); // process data
    //TRACE(_T("Process: CProcessLoudness\n"));

    BeginWaitCursor(); // wait cursor
    if (!StartProcess(pCaller, IDS_STATTXT_PROCESSLOU)) { // memory allocation failed
        EndProcess(); // end data processing
        EndWaitCursor();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // create the temporary file
    if (!CreateTempFile(_T("LOU"))) { // creating error
        EndProcess(); // end data processing
        EndWaitCursor();
        SetDataInvalid();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // process raw data into loudness data
    DWORD dwDataSize = pDoc->GetDataSize(); // size of raw data
    DWORD nSmpSize = pDoc->GetSampleSize();
    // Finish pitch processing if necessary.
    UINT nCalcDataLength = CALCULATION_DATALENGTH(pDoc->GetSamplesPerSec());  //!!this should be based on sampling frequency
    if (nResult >= 0 && pAutoPitch->GetMaxValue()) {
        nCalcDataLength = (UINT)(2*pDoc->GetSamplesPerSec() / (pAutoPitch->GetMaxValue() / PRECISION_MULTIPLIER));
    }
    UINT nCalcInterval = nCalcDataLength / 3;   // more than 2 x bandwidth
    nCalcDataLength = nCalcInterval * 3;

    HPSTR pRawData = NULL, pBlockStart; // pointers to raw data
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
            pBlockStart = pDoc->GetWaveData(dwDataPos); // get pointer to data block
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
                    try {
                        Write(m_lpBuffer, dwLoudCount * 2);
                        *m_lpBuffer = 0;
                        if (dwDataPos >= dwDataSize) {
                            Write(m_lpBuffer, 2);
                        }
                    } catch (CFileException e) {
                        // error writing file
                        ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
                        return Exit(PROCESS_ERROR); // error, writing failed
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
    EndWaitCursor();
    SetDataReady(bResult);
    if (bResult) {
        return MAKELONG(nLevel, nProgress);
    } else {
        SetDataInvalid();
        return MAKELONG(PROCESS_ERROR, 100);
    }
}


//###########################################################################
// CProcessSmoothLoudness
// class to calculate loudness values for smoothed wave data. It's the same
// process as for normal loudness data (see class CProcessLoudness), but it
// uses the presmoothed raw data to process loudness and doesn't smooth the
// loudness data at the end. This data will be the base data for further
// processes. This class needs a third buffer (besides the raw data and the
// processed data buffer) for the smoothed raw data. It will be twice the
// size of the raw data buffer and will be filled up with smoothed raw data,
// every time a new raw data block has been read. The first part of this new
// buffers will contain the smoothed data of the previous calculation and
// second part the actual smoothed data. The previous data part allows going
// back and forth between two blocks of smoothed data (which will be
// recommended from the smooth loudness process) without recalculating the
// same block twice. The smoothed raw data will also be saved into a
// temporary file for further use (e. g. pitch calculation).

/////////////////////////////////////////////////////////////////////////////
// CProcessSmoothLoudness construction/destruction/creation

/***************************************************************************/
// CProcessSmoothLoudness::CProcessSmoothLoudness Constructor
/***************************************************************************/
CProcessSmoothLoudness::CProcessSmoothLoudness() {
    m_pSRDfile = new CFile();
    m_SRDfileStatus.m_szFullName[0] = 0; // no file name
    m_hSRDdata = NULL;
    m_dwSRDBufferOffset = UNDEFINED_OFFSET; // buffer undefined, force buffer reload
    m_bSRDBlockBegin = FALSE;
}

/***************************************************************************/
// CProcessSmoothLoudness::~CProcessSmoothLoudness Destructor
/***************************************************************************/
CProcessSmoothLoudness::~CProcessSmoothLoudness() {
    // delete the temporary file
    FileUtils::Remove(m_SRDfileStatus.m_szFullName);
    if (m_pSRDfile) {
        delete m_pSRDfile;
        m_pSRDfile = NULL;
    }
    // free the buffer memory
    if (m_hSRDdata) {
        ::GlobalUnlock(m_hSRDdata);
        ::GlobalFree(m_hSRDdata);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CProcessSmoothLoudness helper functions

/***************************************************************************/
// CProcessSmoothLoudness::Exit Exit on Error
// Standard exit function if an error occured.
/***************************************************************************/
long CProcessSmoothLoudness::Exit(int nError, HANDLE hSmoothBlock) {
    if (m_pSRDfile) {
        m_pSRDfile->Abort();
    }

    FileUtils::Remove(m_SRDfileStatus.m_szFullName);
    m_SRDfileStatus.m_szFullName[0] = 0;
    // free the smoothed data buffer
    ::GlobalUnlock(hSmoothBlock);
    ::GlobalFree(hSmoothBlock);
    return CProcess::Exit(nError);
}

/***************************************************************************/
// CProcessSmoothLoudness::SmoothRawData Smooth raw data
// This function smoothes a block of raw data from the raw data buffer and
// puts it into the target buffer. It also saves the smoothed raw data in
// a temporary file for further use (e. g. for pitch calculation). The
// process is a sort of lowpass moving average filter. It's not an optimal
// lowpass filter. It has considerable sidelobe leakage, but it is linear
// phase and can be easily adapted to any sampling frequency which is an
// integral fraction or multiple of the frequency for which it was designed.
// A set of optimal filters should be provided which may be selected based
// on the sampling frequency. The raw data will normally be processed into
// the second half of the target buffer, while the first half holds the data
// from the last process. This technique allows going back and forth between
// two blocks of smoothed data (which will be recommended from the smooth
// loudness process) without recalculating the same block twice. The return
// parameter gives the pointer to the target data back, in which the requested
// data will be found. The block numbers tell this function, if the user wants
// to process a new block (so the data from the second buffer half has to be
// copied into the first half, new data has to be processed into the second
// half and the pointer to the middle of the buffer (new data) is returned),
// or if he just wants to see old data (just return the buffer pointer). If
// nOldBlock contains -1, this is the first call and the buffer has to be
// cleaned up first. dwStart contains the calculation size of the data part,
// that is needed from the old data to calculate the new data block. If an
// error occurs, NULL is returned.
/***************************************************************************/
HPSTR CProcessSmoothLoudness::SmoothRawData(ISaDoc * pDoc, HPSTR pTarget, UINT nSmpSize,
        int nOldBlock, int nBlock, int * pnLastDone,
        DWORD dwSmplPerSec, DWORD dwStart, UINT * nAverage, long * lAverage) {
    if (nOldBlock > nBlock) {
        return pTarget;    // old data needed, just give back the buffer pointer
    }
    if (*pnLastDone >= nBlock) {
        return pTarget + GetProcessBufferSize();    // this block is ready in buffer
    }
    *pnLastDone = nBlock;
    int nSmoothWidth = (int)(dwSmplPerSec / (2 * MAX_CALCULATION_FREQUENCY));
    int nSmoothSamp = nSmoothWidth; // smooth width in samples CLW 12/31/98
    DWORD dwBufferSamp = GetProcessBufferSize() / nSmpSize; // buffer size in samples CLW 12/31/98
    if (nSmpSize > 1) { // 16 bit data
        nSmoothWidth *= 2;
        dwStart *= 2;
    }
    if (nOldBlock == -1) { // first call
        // clean up buffer
        char * lpTarget = pTarget;
        DWORD dwLoopEnd = GetProcessBufferSize() + (long)nSmoothWidth + 2;
        for (DWORD dwLoop = 0; dwLoop < dwLoopEnd; dwLoop++) {
            *lpTarget++ = 0;
        }
    } else { // not first call
        // copy upper half of target buffer into lower half
        short int * lpTargetUpper = (short int *)(pTarget + GetProcessBufferSize());
        short int * lpTargetLower = (short int *)pTarget;
        for (DWORD dwLoop = 0; dwLoop < (GetProcessBufferSize() / 2); dwLoop++) {
            *lpTargetLower++ = *lpTargetUpper++;
        }
    }
    // start process
    DWORD dwDataPos = (long)nBlock * GetProcessBufferSize(); // position in raw data
    DWORD dwReloadPos = 0; // new data block has to be reloaded because of samples
    // needed from adjacent blocks
    DWORD dwEnd; // number of loops to process
    // The following section has been cleaned up using nSmoothSamp and
    // nBufferSamp to eliminate 'if (nSmpSize > 1)' tests.
    // References to dwStart have also been removed. CLW 1/6/98
    DWORD dwDataSize = pDoc->GetDataSize();
    if (nBlock == 0) { // First block, so no overlap into previous block
        if ((((long)nBlock + 1) * GetProcessBufferSize()) < dwDataSize)
            // not last block, so overlap into next block
        {
            dwEnd = dwBufferSamp - (long)nSmoothSamp / 2;
        } else { // last block
            dwEnd = dwDataSize / nSmpSize - nSmoothSamp + 1;
        }
        dwReloadPos = dwEnd - (long)nSmoothSamp + (long)nSmoothSamp / 2;
    } else { // not first block, so overlap into previous block
        dwDataPos -= nSmoothWidth / 2; // not first processing
        if ((((long)nBlock + 1) * GetProcessBufferSize()) < dwDataSize)
            // not last block, so overlap into next block
        {
            dwEnd = dwBufferSamp;    // we can process every sample in this block
        } else // last block
            dwEnd = (dwDataSize - (long)nBlock * GetProcessBufferSize()) / nSmpSize
                    - nSmoothSamp + nSmoothSamp / 2 + 1;
        dwReloadPos = dwEnd - (long)nSmoothSamp;
    }
    // check end of data
    if ((dwDataPos + nSmoothWidth) >= dwDataSize) {
        return pTarget + GetProcessBufferSize();    // end of data
    }
    // load new raw data block
    HPSTR pBlockStart = pDoc->GetWaveData(dwDataPos, TRUE); // get data
    if (!pBlockStart) {
        return NULL;    // reading failed
    }
    if (nOldBlock == -1) { // first call
        // form the moving average seed
        int nSeed = 0;
        *lAverage = 0;
        if (nSmpSize > 1) { // 16 bit data
            short int * lpSource = (short int *)pBlockStart; // cast pointer
            int nLoopEnd = nSmoothWidth / 2;
            for (int i = 0; i < nLoopEnd; i++) {
                *lAverage += *lpSource++;    // get data
            }
        } else { // 8 bit data
            unsigned char * lpSource = (unsigned char *)pBlockStart; // cast pointer
            for (int i = 0; i < nSmoothWidth; i++) {
                nSeed += *lpSource++;    // get data
            }
            *nAverage = nSeed; // save the result
        }
    }
    // process data
    if (nSmpSize > 1) { // 16 bit data
        short int * lpSource = (short int *)pBlockStart; // cast pointer
        short int * lpTarget;
        if (nBlock > 0) {
            lpTarget = (short int *)(pTarget + GetProcessBufferSize());
        }
        // CLW 12/3/98
        // else lpTarget = (short int *)(pTarget + GetProcessBufferSize() + nSmoothWidth + 2);
        else {
            lpTarget = (short int *)(pTarget + GetProcessBufferSize() + nSmoothWidth / 2);
        }
        int nDisplacement = nSmoothWidth / 2;
        do {
            if (dwEnd == dwReloadPos) { // new data block has to be reloaded
                DWORD dwNewDataPos = (DWORD)nBlock * GetProcessBufferSize() + nSmoothWidth / 2; // CLW 12/31/98
                pBlockStart = pDoc->GetWaveData(dwNewDataPos, TRUE); // get data
                if (!pBlockStart) {
                    return NULL;    // reading failed
                }
                lpSource = (short int *)pBlockStart;
            }
            // write current smoothed value
            *lpTarget++ = (short)(*lAverage / nDisplacement);
            // drop first point and add next point to moving average
            *lAverage += (long)*(lpSource + nDisplacement) - (long)*lpSource;
            lpSource++; // CLW 12/3/98 Was included on previous line
        } while (--dwEnd);
        // fill zeroes at end of last block CLW 1/5/98
        if ((((long)nBlock + 1) * GetProcessBufferSize()) >= dwDataSize) { // is this the last block?
            dwEnd = (long)nSmoothSamp - (long)nSmoothSamp / 2 - 1;
            do {
                *lpTarget++ = 0L;
            } while (--dwEnd);
        }
    } else { // 8 bit data
        unsigned char * lpSource = (unsigned char *)pBlockStart; // cast pointer
        unsigned char * lpTarget;
        if (nBlock > 0) {
            lpTarget = (unsigned char *)(pTarget + GetProcessBufferSize());
        } else {
            lpTarget = (unsigned char *)(pTarget + GetProcessBufferSize() + nSmoothWidth + 1);
        }
        do {
            if (dwEnd == dwReloadPos) { // new data block has to be reloaded
                pBlockStart = pDoc->GetWaveData((DWORD)nBlock * GetProcessBufferSize(), TRUE); // get data
                if (!pBlockStart) {
                    return NULL;    // reading failed
                }
                lpSource = (unsigned char *)pBlockStart;
            }
            // write current smoothed value
            *lpTarget++ = (unsigned char)(*nAverage / nSmoothWidth);
            // drop first point and add next point to moving average
            *nAverage += *(lpSource + nSmoothWidth) - *lpSource;
            lpSource++; // CLW 12/3/98 Was included on previous line
        } while (--dwEnd);
    }
    // write the processed data block into the smoothed raw data temporary file
    DWORD dwAmount = GetProcessBufferSize();
    if (((DWORD)(nBlock + 1) * GetProcessBufferSize()) > pDoc->GetDataSize()) {
        dwAmount = pDoc->GetDataSize() - ((DWORD)nBlock * GetProcessBufferSize());
    }
    try {
        m_pSRDfile->Write((HPSTR)(pTarget + GetProcessBufferSize()), dwAmount);
    } catch (CFileException e) {
        // error writing file
        ErrorMessage(IDS_ERROR_WRITETEMPFILE, m_SRDfileStatus.m_szFullName);
        return NULL; // writing failed
    }
    return pTarget + GetProcessBufferSize();
}

/***************************************************************************/
// CProcessSmoothLoudness::SetDataInvalid Delete processed data
/***************************************************************************/
void CProcessSmoothLoudness::SetDataInvalid() {
    CProcess::SetDataInvalid();
    FileUtils::Remove(m_SRDfileStatus.m_szFullName);
    m_SRDfileStatus.m_szFullName[0] = 0;
}

/***************************************************************************/
// CProcessSmoothLoudness::Process Processing smooth loudness data
// The processed smooth loudness data is stored in a temporary file. To
// create it helper functions of the base class are used. While processing
// a process bar, placed on the status bar, has to be updated. The level
// tells which processing level this process has been called, start process
// start on which processing percentage this process starts (for the progress
// bar). The status bar process bar will be updated depending on the level
// and the progress start. The return value returns the highest level
// throughout the calling queue, or -1 in case of an error in the lower word
// of the long value and the end process progress percentage in the higher
// word.
/***************************************************************************/
long CProcessSmoothLoudness::Process(void * pCaller, ISaDoc * pDoc,
                                     int nProgress, int nLevel) {
    //TRACE(_T("Process: CProcessSmoothLoudness\n"));
    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    if (IsDataReady()) {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    }
    BeginWaitCursor(); // wait cursor
    if (!StartProcess(pCaller, IDS_STATTXT_PROCESSSLO)) { // memory allocation failed
        EndProcess(); // end data processing
        EndWaitCursor();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // alocate the temporary global buffer for the smoothed raw data
    HANDLE hSmoothBlock = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, 2 * GetProcessBufferSize());
    if (!hSmoothBlock) {
        // memory allocation error
        ErrorMessage(IDS_ERROR_MEMALLOC);
        EndProcess(); // end data processing
        EndWaitCursor();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    HPSTR lpSmoothBlock = (HPSTR)::GlobalLock(hSmoothBlock); // lock memory
    if (!lpSmoothBlock) {
        // memory lock error
        ErrorMessage(IDS_ERROR_MEMLOCK);
        EndProcess(); // end data processing
        EndWaitCursor();
        ::GlobalFree(hSmoothBlock);
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // create the temporary file for smoothed loudness
    if (!CreateTempFile(_T("SLO"))) { // creating error
        EndProcess(); // end data processing
        EndWaitCursor();
        SetDataInvalid();
        // free the smoothed data buffer
        ::GlobalUnlock(hSmoothBlock);
        ::GlobalFree(hSmoothBlock);
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    // create the temporary file for smoothed raw data
    if (!CreateAuxTempFile(_T("SRD"), m_pSRDfile, m_SRDfileStatus)) {
        return Exit(PROCESS_ERROR, NULL);
    }

    // process raw data into smoothed loudness data
    DWORD dwDataSize = pDoc->GetDataSize(); // size of raw data
    CUttParm myUttParm;
    CUttParm * pUttParm = &myUttParm;
    pDoc->GetUttParm(pUttParm); // get sa parameters utterance member data
    DWORD nSmpSize = pDoc->GetSampleSize();

    // It would be nice to adjust these values, BUT
    // The Change process assumes they are constant
    UINT nCalcDataLength = CALCULATION_DATALENGTH(pDoc->GetSamplesPerSec());  //!!this should be based on sampling frequency
    UINT nCalcInterval = CALCULATION_INTERVAL(pDoc->GetSamplesPerSec());   // more than 2 x bandwidth

    HPSTR pSmoothData = NULL, pBlockStart; // pointers to raw data
    short int * pLoudData = (short int *)m_lpBuffer; // pointer to loudness data
    // calculate current processing position (copied but not understood from WINCECIL)
    DWORD dwDataPos, dwLoudStart;
    if (pUttParm->nMinFreq == 0) {
        pUttParm->nMinFreq = 50;    // to prevent 0 divisions
    }
    dwLoudStart = ((pDoc->GetSamplesPerSec() / (2 * MAX_CALCULATION_FREQUENCY)
                    + pDoc->GetSamplesPerSec() / pUttParm->nMinFreq) / 2
                   + nCalcDataLength / 2 + 5) / nCalcInterval + 1;
    dwDataPos = dwLoudStart * nCalcInterval - nCalcDataLength / 2;
    if (nSmpSize > 1) {
        dwDataPos *= 2;    // 16 bit data
    }
    // prepare loudness loop
    DWORD dwLoudCount = 0;
    for (; dwLoudCount < dwLoudStart; dwLoudCount++) {
        *pLoudData++ = 0; // initialize buffer
    }
    UINT nFirstOffset = (UINT)dwDataPos; // raw data pointer offset for start
    UINT nResult = 0;
    UINT nCalcLoop = 0; // calculation loop counter
    int nBlockNumber = -1; // actual loaded data block number
    int nCalculatedBlock = -1; // actual calculated block in memory
    UINT nAverage; // smoothing average values
    long lAverage;
    while (dwDataPos < dwDataSize) { // processing loop
        pBlockStart = pDoc->GetWaveData(dwDataPos); // get pointer to data block
        if (!pBlockStart) {
            return Exit(PROCESS_ERROR, hSmoothBlock);    // error, reading failed
        }
        if ((dwDataPos / GetProcessBufferSize()) != (UINT)nBlockNumber) {
            int nOldBlockNumber = nBlockNumber;
            nBlockNumber = (int)(dwDataPos / GetProcessBufferSize()); // new block loaded
            // smooth the raw data
            pSmoothData = SmoothRawData(pDoc, lpSmoothBlock, nSmpSize, nOldBlockNumber,
                                        nBlockNumber, &nCalculatedBlock, pDoc->GetSamplesPerSec(),
                                        dwLoudStart, &nAverage, &lAverage);
            if (!pSmoothData) {
                return Exit(PROCESS_ERROR, hSmoothBlock);    // error, reading failed
            }
            pSmoothData += nFirstOffset;
            nFirstOffset = 0;
        }
        if (nSmpSize == 1) { // 8 bit data
            // process a byte
            int nRes = (unsigned char)*pSmoothData - 128;
            nResult += (nRes >= 0 ? nRes : -nRes);
            pSmoothData += 4; // increment position in raw data
            dwDataPos += 4;
        } else { // 16 bit data
            // process a word
            short * pTemp = (short *)pSmoothData;
            int nRes = (unsigned char)((*pTemp / 256) + 128) - 128;
            nResult += (nRes >= 0 ? nRes : -nRes);
            pSmoothData += 8; // increment position in raw data
            dwDataPos += 8;
        }
        // if loudness result ready, store it
        if ((++nCalcLoop > nCalcDataLength / 4) || (dwDataPos >= dwDataSize)) {
            nCalcLoop = 0; // reset loop counter
            nResult = (UINT)((DWORD)nResult * PRECISION_MULTIPLIER / (nCalcDataLength / 4)); // correct result
            *pLoudData++ = (short)nResult; // save result
            if ((int)nResult > m_nMaxValue) {
                m_nMaxValue = (int)nResult;    // save maximum value
            }
            nResult = 0;
            if (nSmpSize == 1) { // 8 bit data
                pSmoothData -= (nCalcDataLength + 4) - nCalcInterval; // reset position in raw data
                if (dwDataPos < dwDataSize) {
                    dwDataPos -= (nCalcDataLength + 4) - nCalcInterval;
                }
            } else { // 16 bit data
                pSmoothData -= ((nCalcDataLength + 4) - nCalcInterval) * 2; // reset position in raw data
                if (dwDataPos < dwDataSize) {
                    dwDataPos -= ((nCalcDataLength + 4) - nCalcInterval) * 2;
                }
            }
            // set progress bar
            SetProgress(nProgress + (int)(100 * dwDataPos / dwDataSize / (DWORD)nLevel));
            if (IsCanceled()) {
                return Exit(PROCESS_CANCELED, hSmoothBlock);    // process canceled
            }
            if ((++dwLoudCount == GetProcessBufferSize() / 2) || (dwDataPos >= dwDataSize)) { // loudness buffer is full or processing finished
                // write the processed data block
                try {
                    Write(m_lpBuffer, dwLoudCount * 2);
                } catch (CFileException e) {
                    // error writing file
                    ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
                    return Exit(PROCESS_ERROR, hSmoothBlock); // error, writing failed
                }
                dwLoudCount = 0; // reset counter
                pLoudData = (short int *)m_lpBuffer; // reset pointer to begin of loudness data buffer
            }
        }
    }
    // calculate the actual progress
    nProgress = nProgress + (int)(100 / nLevel);
    // close the temporary file and read the status
    CloseTempFile(); // close the file
    // close the smoothed raw data temp file and read the status
    if (m_pSRDfile) {
        m_pSRDfile->Abort();
    }

    CString szFullName = m_SRDfileStatus.m_szFullName;
    CFile::GetStatus(szFullName, m_SRDfileStatus); // read the status
    if (GetDataSize() < 2) {
        return Exit(PROCESS_ERROR, hSmoothBlock);    // error, not enough data
    }
    EndProcess((nProgress >= 95)); // end data processing
    EndWaitCursor();
    // free the temporary smoothed data buffer
    ::GlobalUnlock(hSmoothBlock);
    ::GlobalFree(hSmoothBlock);
    // allocate permanent global buffer for the processed smoothed raw data
    if (!m_hSRDdata) { // not yet allocated
        m_hSRDdata = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, GetProcessBufferSize()); // allocate memory
        if (!m_hSRDdata) {
            // memory allocation error
            ErrorMessage(IDS_ERROR_MEMALLOC);
            FileUtils::Remove(m_SRDfileStatus.m_szFullName);
            m_SRDfileStatus.m_szFullName[0] = 0;
            SetDataInvalid();
            return MAKELONG(PROCESS_ERROR, 100);
        }
        m_lpSRDdata = (HPSTR)::GlobalLock(m_hSRDdata); // lock memory
        if (!m_lpSRDdata) {
            // memory lock error
            ErrorMessage(IDS_ERROR_MEMLOCK);
            ::GlobalFree(m_hSRDdata);
            m_hSRDdata = NULL;
            FileUtils::Remove(m_SRDfileStatus.m_szFullName);
            m_SRDfileStatus.m_szFullName[0] = 0;
            SetDataInvalid();
            return MAKELONG(PROCESS_ERROR, 100);
        }
    }
    SetDataReady();
    return MAKELONG(nLevel, nProgress);
}

/***************************************************************************/
// CProcessSmoothLoudness::GetSmoothRawData Read smooth raw data from temp. file
// Reads a block of processed smooth raw data from the temporary file into
// the permanent smooth raw data data buffer and returns the pointer to the
// data. The user gives the data offset (data number) in the file to tell
// what data he needs. If bBlockBegin is set, this given offset data will
// always be at the top of the data block (where the returned pointer points
// to). If bBlockBegin is not set, the data block is not reread, if the given
// offset data is already somewhere in the buffer, and only the actual
// pointer to the data block will be returned. This smooth raw data is either
// 8 or 16bit data, like its source the raw data.
/***************************************************************************/
HPSTR CProcessSmoothLoudness::GetSmoothRawData(DWORD dwOffset, BOOL bBlockBegin) {
    if (((!bBlockBegin && !m_bSRDBlockBegin) && ((dwOffset >= m_dwSRDBufferOffset) && (dwOffset < m_dwSRDBufferOffset + GetProcessBufferSize())))
            || ((bBlockBegin) && (m_dwSRDBufferOffset == dwOffset))) {
        // this data is actually in buffer
        return m_lpSRDdata; // return pointer to data
    } else { // new data block has to be read
        m_bSRDBlockBegin = bBlockBegin;
        if (bBlockBegin) {
            m_dwSRDBufferOffset = dwOffset;    // given offset ist first sample in data block
        } else {
            m_dwSRDBufferOffset = dwOffset - (dwOffset % GetProcessBufferSize());    // new block offset
        }
        // open the temporary file
        if (!m_pSRDfile->Open(m_SRDfileStatus.m_szFullName, CFile::modeRead | CFile::shareExclusive)) {
            // error opening file
            ErrorMessage(IDS_ERROR_OPENTEMPFILE, m_SRDfileStatus.m_szFullName);
            m_pSRDfile->Abort();
            return NULL;
        }
        // find the right position in the data
        if (m_dwSRDBufferOffset != 0L) {
            try {
                m_pSRDfile->Seek(m_dwSRDBufferOffset, CFile::begin);
            } catch (CFileException e) {
                // error seeking file
                ErrorMessage(IDS_ERROR_READTEMPFILE, m_SRDfileStatus.m_szFullName);
                m_pSRDfile->Abort();
                return NULL;
            }
        }
        // read the processed data block
        try {
            m_pSRDfile->Read((HPSTR)m_lpSRDdata, GetProcessBufferSize());
        } catch (CFileException e) {
            // error reading file
            ErrorMessage(IDS_ERROR_READTEMPFILE, m_SRDfileStatus.m_szFullName);
            m_pSRDfile->Abort();
            return NULL;
        }

        // close the temporary file
        if (m_pSRDfile) {
            m_pSRDfile->Abort();
        }

        // return the new data pointer
        return m_lpSRDdata;
    }
}

DWORD CProcessSmoothLoudness::GetSmoothRawDataSize() {
    // return processed data size in words (16 bit)
    return (DWORD)m_SRDfileStatus.m_size;
}
