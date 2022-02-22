/////////////////////////////////////////////////////////////////////////////
// sa_wave.cpp:
// Implementation of the CWave class.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
//
// Revision History
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "sa_wave.h"
#include "sa_doc.h"
#include "sa.h"
#include "sa_view.h"
#include "mainfrm.h"
#include "mixer.h"
#include "WaveOutDevice.h"
#include "WaveInDevice.h"
#include "FmtParm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//###########################################################################
// CWave
// This class plays and records wave files in the background. For playback it
// uses two buffers and while one buffer is playing in background it processes
// the data for the other buffer.

/////////////////////////////////////////////////////////////////////////////
// CWave construction/destruction/creation

/***************************************************************************/
// CWave::CWave Constructor
/***************************************************************************/
CWave::CWave() {
    m_bRecording = false;
    m_bBackgroundEnabled = FALSE;
    m_pNotifyObj = NULL;
    m_pView = NULL;
    m_pWaveWarp = NULL;
    m_nActiveBlock = 0;
    m_nNextBlock = 0;
    m_nMaxLevel = 0;
    m_hmmioFile = NULL;
    for (int i=0; i < m_kBuffers; i++) {
        m_lpWaveData[i] = NULL;
        m_dwBufferSizes[i] = 0;
        m_dwProcessedSize[i] = 0;
    }
    // create device objects
    m_pOutDev = new CWaveOutDevice();
    m_pInDev = new CWaveInDevice();
}

/***************************************************************************/
// CWave::~CWave Destructor
/***************************************************************************/
CWave::~CWave() {

    for (int i = 0; i < m_kBuffers; i++) {
        if (m_lpWaveData[i]!=NULL) {
            delete [] m_lpWaveData[i];
            m_lpWaveData[i] = NULL;
        }
    }
    if (m_pWaveWarp!=NULL) {
        delete m_pWaveWarp;
        m_pWaveWarp = NULL;
    }
    // delete device objects
    if (m_pOutDev) {
        delete m_pOutDev;
        m_pOutDev = NULL;
    }
    if (m_pInDev) {
        delete m_pInDev;
        m_pInDev = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CWave helper functions

/***************************************************************************/
// CWave::AllocateBuffers Allocate wave buffers
// Allocates the wave buffers for playback and recording. In case of an
// error the function returns FALSE, otherwise TRUE.
/***************************************************************************/
BOOL CWave::AllocateBuffers() {

    CSaApp * pApp = (CSaApp *)AfxGetApp(); // get pointer to application
    for (int i = 0; i < m_kBuffers; i++) {
        if (m_lpWaveData[i]==NULL) {
            m_lpWaveData[i] = new char[MMIO_BUFFER_SIZE * (DWORD)10];
        }
        if (m_lpWaveData[i]==NULL) {
            // memory lock error
            pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
            return FALSE;
        }
    }
    return TRUE;
}

/***************************************************************************/
// CWave::CalculateBufferSize
// Adjust size to current parameters
// Adjusts the wave buffer size to a size according to the current sampling
// rate and the sample size.
/***************************************************************************/
DWORD CWave::CalculateBufferSize() {

    // assuming 44100 samples per sec?
    DWORD dwFactor = 44100 / m_FmtParm.dwSamplesPerSec;
    // if it's 8 bit, we don't need as much
    if (m_FmtParm.wBlockAlign == 1) {
        dwFactor *= 2;
    }
    // the data coming in will be forced to single channel
    // so we don't need to worry about that

    // limit total scaling...
    if (dwFactor > 4) {
        dwFactor = 4;
    }
    if (dwFactor < 1) {
        dwFactor = 1;
    }
    return MMIO_BUFFER_SIZE / dwFactor;
}

/***************************************************************************/
// CWave::ProcessData Processing wave data
// Here the wave data ready to play will be processed before playing, for
// instance to play it reverse or faster or slower.
/***************************************************************************/
BOOL CWave::ProcessData(int nBuffer) {
    // Set playback buffer pointer and buffer size.
    BPTR pTarget = GetBufferPointer(nBuffer); // pointer to target buffer
    DWORD dwBufferSize = CalculateBufferSize();
    UINT wSmpSize = m_FmtParm.GetSampleSize();
    int nMaxValue = 0, nMinValue = 0;

    // If wavewarp object not constructed, simply copy the sample data to the play buffer
    if (m_pWaveWarp==NULL) {
        DWORD dwDataSize = min(m_dwEnd - m_dwPlayPosition, dwBufferSize);
        if (dwDataSize > 0) {
            BPTR pData = m_pNotifyObj->GetWaveData(m_pView, m_dwPlayPosition, dwDataSize);
            if (pData==NULL) {
                return FALSE;
            }
            CopyBuffer(pData, pTarget, dwDataSize, (wSmpSize == 1), &nMaxValue, &nMinValue);
            m_dwPlayPosition += dwDataSize;
            if (m_dwPlayPosition >= m_dwEnd) {
                m_bProcessDone = TRUE;
            }
        }
        SetBufferSize(nBuffer, dwDataSize, dwDataSize);      // set member variables for playback
    } else {
        //  Otherwise, pass sample data to warping object
        if (m_pWaveWarp->SetPlayBuffer((void *)pTarget, dwBufferSize/wSmpSize) != DONE) {
            return FALSE;
        }

        // Load playback buffer with waveform fragments from start to stop cursors.
        DWORD dwDataSize=0;
        DWORD dwPlayLength = 0;
        DWORD dwProcess=0;
        dspError_t Status;
        do {
            if (m_CallbackData.dwOffset * wSmpSize >= m_dwEnd) {
                Status = DONE;
                break;
            }
            dwProcess = m_CallbackData.dwOffset * wSmpSize;                                 // convert to byte offset
            DWORD dwProcessEnd = max(dwProcess + m_CallbackData.wLength*wSmpSize, m_dwEnd); // get endpoint of last fragment
            dwDataSize = min(dwProcessEnd - dwProcess, dwBufferSize);                       // resize block to buffer size or to end of last fragment
            BPTR pData = m_pNotifyObj->GetWaveData(m_pView, dwProcess, dwDataSize);        // retrieve sample data
            if (m_pWaveWarp->SetWaveBuffer((void *)pData) != DONE) {
                return FALSE;                                                               // pass waveform buffer pointer
            }
            Status = m_pWaveWarp->FillPlayBuffer(m_CallbackData.dwOffset, dwDataSize/wSmpSize, &m_CallbackData, &dwPlayLength);
        } while (Status == OUTSIDE_WAVE_BUFFER);

        if (Status < DONE) {
            CSaApp * pApp = (CSaApp *)AfxGetApp();                  // get pointer to application
            pApp->ErrorMessage(IDS_ERROR_WAVEWARP, _T("playback")); // send error message
            return FALSE;
        } else if (Status == DONE) {
            m_CallbackData.dwOffset = dwProcess  / wSmpSize;
            m_CallbackData.wLength = (WORD)(dwDataSize / wSmpSize);
            m_bProcessDone = TRUE;
        }

        // Set playback member variable to update timer
        // use play length to avoid snapping to fragment boundaries
        m_dwPlayPosition += (dwPlayLength*m_nSpeed+50)/100*wSmpSize;

        // Use requested fragment to limit position (avoid runout errors)
        if (m_dwPlayPosition < m_CallbackData.dwOffset * wSmpSize) {
            m_dwPlayPosition = m_CallbackData.dwOffset * wSmpSize;
        } else if (m_dwPlayPosition > (m_CallbackData.dwOffset + m_CallbackData.wLength) * wSmpSize) {
            m_dwPlayPosition = (m_CallbackData.dwOffset + m_CallbackData.wLength) * wSmpSize;
        }

        // Don't go beyond end
        if (m_dwPlayPosition >= m_dwEnd) {
            m_dwPlayPosition = m_dwEnd;
        }

        // Set playback size
        DWORD dwPlaySize = dwPlayLength * wSmpSize;
        SetBufferSize(nBuffer, dwDataSize, dwPlaySize);

        // Set find min and max values.
        FindMinMax((BPTR)pTarget, dwPlaySize, (wSmpSize == 1),  &nMaxValue, &nMinValue);
    }

    // Set peak level for VU meter.
    m_nMaxLevel = max((UINT)abs(nMaxValue), (UINT)abs(nMinValue));
    if (wSmpSize == 1) {
        m_nMaxLevel = 100 * m_nMaxLevel / 128;
    } else {
        m_nMaxLevel = (UINT)(100 * (long)m_nMaxLevel / 32768);
    }
    return TRUE;
}

/***************************************************************************/
// CWave::CopyBuffer Copy source into target buffer and find min/max values
/***************************************************************************/
void CWave::CopyBuffer(BPTR pSource, BPTR pTarget, DWORD dwLength, BOOL bIs8Bit, int * piMax, int * piMin) {

    int nData;
    do {
        // read data
        if (bIs8Bit) {              // 8 bit per sample
            *pTarget++ = *pSource;  // copy data
            BYTE bData = *pSource++;// data range is 0...255 (128 is center)
            nData = bData - 128;
        } else {                    // 16 bit data
            *pTarget++ = *pSource;  // copy data
            nData = *((short int *)pSource++);
            *pTarget++ = *pSource;  // copy data
            pSource++;
            dwLength--;
        }
        // process data
        if (nData > *piMax) {
            *piMax = nData;         // store higher value
        } else if (nData < *piMin) {
            *piMin = nData;         // store lower value
        }
    } while (--dwLength > 0);
}

/***************************************************************************/
// CWave::FindMinMax Find min/max values
/***************************************************************************/
void CWave::FindMinMax(BPTR pSource, DWORD dwLength, BOOL bIs8Bit, int * piMax, int * piMin) {

    int nData=0;
    BYTE bData=0;
    if (dwLength == 0) {
        return;
    }
    do {
        if (bIs8Bit) { // 8 bit per sample
            bData = *pSource++; // data range is 0...255 (128 is center)
            nData = bData - 128;
        } else {              // 16 bit data
            nData = *((short int *)pSource++);
            pSource++;
            dwLength--;
        }
        // process data
        if (nData > *piMax) {
            *piMax = nData;    // store higher value
        } else {
            if (nData < *piMin) {
                *piMin = nData;    // store lower value
            }
        }
    } while (--dwLength > 0);
}

/***************************************************************************/
// CWave::GetBufferPointer Get the pointer to the actual buffer
/***************************************************************************/
char * CWave::GetBufferPointer(int nBuffer) {

    if ((nBuffer < 0) || (nBuffer >= m_kBuffers)) {
        return NULL;
    }
    return m_lpWaveData[nBuffer];
}

/***************************************************************************/
// CWave::GetPlaybackPointer Get pointer to the first sample to play
/***************************************************************************/
char * CWave::GetPlaybackPointer(int nBuffer) {

    char * pPlayback = GetBufferPointer(nBuffer);
    if (m_pWaveWarp==NULL) {
        return pPlayback;
    }

    DWORD dwProcess = m_CallbackData.dwOffset * m_FmtParm.GetSampleSize();
    if (dwProcess < m_dwStart) {
        return (pPlayback + m_dwStart - dwProcess);    // playback to start in middle of fragment
    }
    return pPlayback;
}

/***************************************************************************/
// CWave::GetBufferSize Get the size of the actual buffer
/***************************************************************************/
DWORD CWave::GetBufferSize(int nBuffer) {
    if ((nBuffer < 0) || (nBuffer >= m_kBuffers)) {
        return 0;
    }
    return m_dwBufferSizes[nBuffer];
}

/***************************************************************************/
// CWave::GetPlaybackSize Get the size to play back
/***************************************************************************/
DWORD CWave::GetPlaybackSize(int nBuffer) {

    DWORD dwBufferSize = GetBufferSize(nBuffer);
    if (m_pWaveWarp!=NULL) {
        DWORD wSmpSize = m_FmtParm.GetSampleSize();
        DWORD dwFragment = m_CallbackData.dwOffset * wSmpSize;
        WORD wFragmentSize = (WORD)(m_CallbackData.wLength * wSmpSize);

        if (dwFragment < m_dwStart) {
            dwBufferSize -= (m_dwStart - dwFragment);    // reduce if playback starts in middle of fragment
        } else if (dwFragment + wFragmentSize > m_dwEnd) {
            dwBufferSize -= (dwFragment + wFragmentSize - m_dwEnd);    // trim last fragment to end
        }
    }
    return dwBufferSize;
}

/***************************************************************************/
// CWave::GetProcessedSize Get the size of the proc. data for the act. buffer
/***************************************************************************/
DWORD CWave::GetProcessedSize(int nBuffer) {

    if ((nBuffer < 0) || (nBuffer >= m_kBuffers)) {
        return 0;
    }
    return m_dwProcessedSize[nBuffer];
}

/***************************************************************************/
// CWave::SetBufferSize Set the size of the actual buffer
// The parameter dwProcessedSize sets the size of the raw data block that
// has been used to get the size of the data in the buffer given by the
// parameter dwBufferSize. They are both the same for normal speed recording
// and playback.
/***************************************************************************/
void CWave::SetBufferSize(int nBuffer, DWORD dwProcessedSize, DWORD dwBufferSize) {

    if ((nBuffer < 0) || (nBuffer >= m_kBuffers)) {
        return;
    }
    m_dwProcessedSize[nBuffer] = dwProcessedSize;
    m_dwBufferSizes[nBuffer] = dwBufferSize;
}

/***************************************************************************/
// CWave::Play Play wave data
// The caller delivers the number of the first data sample to play and the
// size of the data block to play. Then pointers to the actual view and to
// the notify object are needed (to get necessary data and to inform the
// player dialog via the notify object). Two buffers are allocated to hold
// the data to play. Returns FALSE in case of error, else TRUE.
/***************************************************************************/
BOOL CWave::Play(DWORD dwStart, DWORD dwSize, UINT nVolume, UINT nSpeed, CView * pView, CWaveNotifyObj * pNotify) {

    m_pNotifyObj = pNotify;         // set pointer to notify object
    m_pView = pView;                // set pointer to view
    CSaDoc * pModel = (CSaDoc *)pView->GetDocument();
    CProcessFragments * pFragmenter = pModel->GetFragments();
    pModel->GetFmtParm(m_FmtParm,true);   // set pointer to wave format parameters

    m_dwStart = dwStart;            // set start index
    m_dwPlayPosition = dwStart;     // set index of first sample to play
    m_dwEnd = dwStart + dwSize;     // set index of last sample to play
    m_nActiveBlock = 0;             // use block 0 first
    m_nNextBlock = 0;               // use block 0 first

    // allocate data buffers
    if (!AllocateBuffers()) {
        return FALSE;
    }

    // set up playback volume and speed
    SetVolume(nVolume);
    SetSpeed(nSpeed);
    m_bPlayDone = FALSE;
    m_bProcessDone = FALSE;

    m_bBackgroundEnabled = pModel->IsBackgroundProcessing();
    if (m_bBackgroundEnabled) {
        pModel->EnableBackgroundProcessing(FALSE);    // disable background processing during playback
    }

    if ((nSpeed != 100) && !pFragmenter->IsDataReady()) {
        // finish fragmenting
        short int nResult = LOWORD(pFragmenter->Process(this, pModel)); // process data
        if ((nResult == PROCESS_ERROR) || (nResult == PROCESS_NO_DATA) || (nResult == PROCESS_CANCELED)) {
            pFragmenter->SetDataInvalid();
            m_bBackgroundEnabled = FALSE;
            pFragmenter->RestartProcess();
            TRACE(_T("fragmenter failed\n"));
        }
    }

    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    // check if player is playing
    if (((pMainWnd->IsPlayerPlaying()) || (pMainWnd->IsPlayerTestRun())) &&
            (pFragmenter->IsDataReady())) {
        // create WaveWarp object
        dspError_t Err = CWaveWarp::CreateObject(&m_pWaveWarp, pModel, dwStart/m_FmtParm.wBlockAlign, (USHORT)nSpeed, &m_CallbackData);
        if (Err) {
            CSaApp * pApp = (CSaApp *)AfxGetApp(); // get pointer to application
            pApp->ErrorMessage(IDS_ERROR_WAVEWARP, _T("initialization")); // send error message
            return FALSE;
        }
    } else {
        TRACE(_T("CWaveWarp object not created: PlayerPlaying %u, PlayerTest %u, FragmentReady %u\n"), pMainWnd->IsPlayerPlaying(), pMainWnd->IsPlayerTestRun(),  pFragmenter->IsDataReady());
    }

    for (int i = 0; i < m_kPlayBuffers; i++) {

        m_dwPosition[m_nNextBlock] = m_dwPlayPosition;
        // fill up buffer with data
        if (!ProcessData(m_nNextBlock)) {
            return FALSE;
        }
        m_nProcessedSpeed[m_nNextBlock] = m_nSpeed;
        m_nProcessedMax[m_nNextBlock] = m_nMaxLevel;
        // play buffer 0
        if (!m_pOutDev->Play(m_nNextBlock, nVolume, this, !m_bProcessDone && (i + 1) < m_kPlayBuffers)) {
            return FALSE;
        }

        m_nNextBlock = ++m_nNextBlock % m_kPlayBuffers;

        if (m_bProcessDone) {
            break;
        }
    }

    if ((m_pNotifyObj!=NULL) && (!m_bPlayDone)) {
        m_pNotifyObj->BlockFinished(m_nProcessedMax[m_nActiveBlock], m_dwPosition[m_nActiveBlock], m_nProcessedSpeed[m_nActiveBlock]);    // update player dialog
    }
    return TRUE;
}

/***************************************************************************/
// CWave::Monitor Monitor wave data
// The caller delivers pointers to the actual view and to the notify object.
/***************************************************************************/
BOOL CWave::Monitor(CView * pView, CWaveNotifyObj * pNotify) {
    return Record(NULL, pView, 0, pNotify, false); // start monitoring
}

/***************************************************************************/
// CWave::Record Record wave data
// If the flag bRecord is FALSE, it will only start monitoring the device,
// and not save the data. The caller delivers a handle to the temporary
// mmio file and pointers to the actual view and to the notify object. The
// notify object informs the player dialog. Two buffers are allocated to
// receive the recorded data (they need to be stretched size for the
// possible slowed replay after record). The parameter dwOffset can contain
// an offset, from where the actual record pointer will add up.
/***************************************************************************/
BOOL CWave::Record(HMMIO hmmioFile,
                   CView * pView,
                   DWORD dwOffset,
                   CWaveNotifyObj * pNotify,
                   bool bRecord /*=true*/) {

    m_hmmioFile = hmmioFile;            // save handle to file object
    m_bRecording = bRecord;
    m_pNotifyObj = pNotify;             // set pointer to notify object
    m_pView = pView;                    // set pointer to view
    CSaDoc * pModel = (CSaDoc *)pView->GetDocument();

    pModel->GetFmtParm(m_FmtParm,true);   // set pointer to wave format parameters
    m_nMaxLevel = 0;
    m_dwRecordPointer = dwOffset;
    m_dwEnd = 0;

    if (pModel->IsUsingHighPassFilter()) {
        if (!m_pInDev->GetHighPassFilter()) {
            if (!m_pInDev->AttachHighPassFilter(pModel->GetSamplesPerSec())) {
                // if can't construct highpass filter, reset the flag
                pModel->DisableHighPassFilter();
                CSaApp * pApp = (CSaApp *)AfxGetApp();      // get pointer to application
                pApp->ErrorMessage(IDS_ERROR_RECHPFILTER);  // send error message
                return FALSE;
            }
        }
    }
    if (!m_pInDev->GetPreparedBuffers()) {
        m_nActiveBlock = 0; // use block 0 first
        // We are not already recording/monitoring
        if (!AllocateBuffers()) {
            return FALSE;    // allocate data buffers
        }

        for (int i = 1; i < m_kRecordBuffers; i++) {
            if (!m_pInDev->Record(i - 1, this)) {
                return FALSE;    // record into buffer i
            }
        }

        if (!m_pInDev->Record(m_kRecordBuffers - 1, this, TRUE)) {
            return FALSE;    // record into buffer 1
        }
    }

    return TRUE;
}

/***************************************************************************/
// CWave::Stop Stop playing or recording wave data
/***************************************************************************/
void CWave::Stop() {
    if (m_pOutDev) {
        m_pOutDev->Reset();
    }
    if (m_pInDev) {
        m_pInDev->Reset();
    }
    m_pOutDev->Close(); // close sound device
    m_pInDev->Close(); // close sound device
    m_dwPlayPosition = m_dwEnd;
    m_pNotifyObj = NULL; // don't notify
    if (m_pWaveWarp!=NULL) {
        delete m_pWaveWarp;
        m_pWaveWarp = NULL;
    }
    CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
    CSaDoc * pModel = (CSaDoc *)pMain->GetCurrDoc();
    if (pModel && m_bBackgroundEnabled) {
        pModel->EnableBackgroundProcessing(TRUE);
    }
}

/***************************************************************************/
// CWave::NextBlock Next data block needed
// One data block has been played, the next one is needed. At this time,
// the next block already has been started playing and there is now time to
// fill up the finished one with new data and send it to the playing device
// before it finishes playing the actual playing block.
/***************************************************************************/
void CWave::NextBlock() {
    //TRACE("NextBlock\n");
    if (!m_bPlayDone) {
        m_nActiveBlock = ++m_nActiveBlock % m_kPlayBuffers;   // switch buffers
        m_bPlayDone = (m_nActiveBlock == m_nNextBlock);

        if ((m_pNotifyObj!=NULL) && (!m_bPlayDone)) {
            m_pNotifyObj->BlockFinished(m_nProcessedMax[m_nActiveBlock], m_dwPosition[m_nActiveBlock], m_nProcessedSpeed[m_nActiveBlock]);     // update player dialog
        }

        // fill buffer with data and play
        if (!m_bProcessDone) {
            m_dwPosition[m_nNextBlock] = m_dwPlayPosition;
            if (ProcessData(m_nNextBlock)) {
                m_nProcessedSpeed[m_nNextBlock] = m_nSpeed;
                m_nProcessedMax[m_nNextBlock] = m_nMaxLevel;
                BOOL bResult = FALSE;
                m_pOutDev->Play(m_nNextBlock, GetVolume(bResult), this);   // play it
            } else {
                // error in processing
                m_pOutDev->Close();                 // close sound device
                if (m_pNotifyObj!=NULL) {
                    m_pNotifyObj->EndPlayback();    // inform notify object
                }
                if (m_pWaveWarp!=NULL) {
                    delete m_pWaveWarp;
                    m_pWaveWarp = NULL;
                }
            }
            m_nNextBlock = ++m_nNextBlock % m_kPlayBuffers;   // switch buffers
        }
    }

    if (m_bPlayDone) {
        // shutdown playback device if processing complete
        m_pOutDev->Close();                 // close sound device
        if (m_pNotifyObj!=NULL) {
            m_pNotifyObj->EndPlayback();    // inform notify object
        }
        if (m_pWaveWarp!=NULL) {
            delete m_pWaveWarp;
            m_pWaveWarp = NULL;
        }
        CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
        CSaDoc * pModel = (CSaDoc *)pMain->GetCurrDoc();
        if (m_bBackgroundEnabled) {
            pModel->EnableBackgroundProcessing(TRUE);
        }
    }
}

/***************************************************************************/
// CWave::StoreBlock Store recorded data block
// Once data block has been recorded, the data must be stored in the
// temporary file. By this time, the next block has already has started
// recording, and there is now time to store the recorded data. The
// buffer must then be returned to the input device, before it finishes
// recording into the actual block.
/***************************************************************************/
void CWave::StoreBlock() {
    // find the maximum level in the recorded data
    BPTR pSource = GetBufferPointer(m_nActiveBlock); // pointer to source buffer
    int nMaxValue = 0;
    int nData, nMinValue = 0;
    BYTE bData;
    DWORD wSmpSize = m_FmtParm.GetSampleSize();
    DWORD dwDataSize = GetBufferSize(m_nActiveBlock); // get buffer size
    if (dwDataSize > 0) {
        CHighPassFilter70 * pHighPassFilter = m_pInDev->GetHighPassFilter();
        if (pHighPassFilter) {
            DWORD dwDataLength = dwDataSize / wSmpSize;
            if (wSmpSize == 1) { // 8 bits per sample, 0 to 255 range
                pHighPassFilter->ForwardPass((unsigned char *)pSource, dwDataLength);
                nMinValue = pHighPassFilter->GetBlockMin() - 128;
                nMaxValue = pHighPassFilter->GetBlockMax() - 128;
            } else { // 16 bits per sample, -32768 to 32767 range
                pHighPassFilter->ForwardPass((short *)pSource, dwDataLength);
                nMinValue = pHighPassFilter->GetBlockMin();
                nMaxValue = pHighPassFilter->GetBlockMax();
            }
        } else {
            do {
                // read data
                if (wSmpSize == 1) { // 8 bit per sample
                    bData = *pSource++; // data range is 0...255 (128 is center)
                    nData = bData - 128;
                } else {              // 16 bit data
                    nData = *((short int *)pSource++);
                    pSource++;
                    dwDataSize--;
                }
                // process data
                if (nData > nMaxValue) {
                    nMaxValue = nData;    // store higher value
                } else {
                    if (nData < nMinValue) {
                        nMinValue = nData;    // store lower value
                    }
                }
            } while (--dwDataSize > 0);
        }
    }
    // set peak level
    if (nMinValue * -1 > nMaxValue) {
        m_nMaxLevel = (UINT)(nMinValue * -1);
    } else {
        m_nMaxLevel = (UINT)nMaxValue;
    }
    // norm the level to %
    if (wSmpSize == 1) { // 8 bit per sample
        m_nMaxLevel = 100 * m_nMaxLevel / 128;
    } else {
        m_nMaxLevel = (UINT)((LONG)100 * (LONG)m_nMaxLevel / 32768);
    }

    // inform notify object
    BOOL bRecord = m_bRecording;
    if (m_pNotifyObj!=NULL) {
        BOOL * pOverride = bRecord ? &bRecord : NULL;
        m_pNotifyObj->BlockStored(m_nMaxLevel, m_dwRecordPointer+GetBufferSize(m_nActiveBlock), pOverride);
    }

    if (bRecord) {
        // store recorded data block in temporary file
        if (mmioWrite(m_hmmioFile, GetBufferPointer(m_nActiveBlock), (long)GetBufferSize(m_nActiveBlock)) == -1) {
            // error
            if (m_pNotifyObj!=NULL) {
                m_pNotifyObj->StoreFailed();
            }
        }
        // actualize play pointer
        m_dwRecordPointer += GetBufferSize(m_nActiveBlock);
    }

    // inform notify object
    if (m_pNotifyObj!=NULL) {
        m_pInDev->Record(m_nActiveBlock, this); // record again into this buffer
        m_nActiveBlock = ++m_nActiveBlock % m_kRecordBuffers; // next buffer
    }
}

/***************************************************************************/
// MMERR Error processing
// Public function to process device error results.
/***************************************************************************/
void MMERR(MMRESULT mmr) {
    CSaApp * pApp = (CSaApp *)AfxGetApp(); // get pointer to application
    switch (mmr) {
    case WAVERR_BADFORMAT:
        pApp->ErrorMessage(IDS_ERROR_MMIO_BADFORMAT);
        break;
    case MMSYSERR_NOMEM:
        pApp->ErrorMessage(IDS_ERROR_MMIO_NOMEMORY);
        break;
    case MMSYSERR_ALLOCATED:
        pApp->ErrorMessage(IDS_ERROR_MMIO_ALRALLOC);
        break;
    case MMSYSERR_BADDEVICEID:
        if (::GetWindowsVersion()<6) {
            pApp->ErrorMessage(IDS_ERROR_MMIO_BADID);
        } else {
            pApp->ErrorMessage(IDS_ERROR_MMIO_BADID_VISTA);
        }
        break;
    case MMSYSERR_INVALHANDLE:
        pApp->ErrorMessage(IDS_ERROR_MMIO_INVHANDLE);
        break;
    case WAVERR_UNPREPARED:
        pApp->ErrorMessage(IDS_ERROR_MMIO_NOHEADER);
        break;
    case MMSYSERR_INVALPARAM:
        pApp->ErrorMessage(IDS_ERROR_MMIO_INVALIDPARAM);
        break;
    default:
        pApp->ErrorMessage(IDS_ERROR_MMIO_UNKNOWN);
        TRACE(_T("Unknown MMSYSERR 0x%X\n"), mmr);
        break;
    }
}

CFmtParm CWave::GetFormat() {
    return m_FmtParm;
}

int CWave::GetActiveBlock() {
    // returns the last block ID
    return m_nActiveBlock;
}

int CWave::GetNextBlock() {
    // returns the last block ID
    return m_nNextBlock;
}

UINT CWave::GetVolume(BOOL & bResult) {
    // get playback volume
    return m_pOutDev->GetVolume(bResult);
}

void CWave::SetVolume(UINT nVolume) {
    // set playback volume
    BOOL bResult = FALSE;
    m_pOutDev->SetVolume(nVolume,bResult);
}

void CWave::SetSpeed(UINT nSpeed) {
    // set playback speed
    m_nSpeed = nSpeed;
}

void CWave::SetMaxLevel(UINT nMaxLevel) {
    // set peak magnitude
    m_nMaxLevel = nMaxLevel;
}

CWaveOutDevice * CWave::GetOutDevice() {
    return m_pOutDev;
}

CWaveInDevice * CWave::GetInDevice() {
    return m_pInDev;
}
