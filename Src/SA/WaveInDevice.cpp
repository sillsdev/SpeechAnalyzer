#include "stdafx.h"
#include "WaveInDevice.h"
#include "sa_wave.h"
#include "sa_doc.h"
#include "sa.h"
#include "sa_view.h"
#include "mainfrm.h"
#include "mixer.h"
#include "RecMixer.h"
#include "WaveOutDevice.h"
#include "FmtParm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// Input device for recording. It uses a notification window to receive
// notification messages from the input device.

BEGIN_MESSAGE_MAP(CWaveInDevice, CWnd)
    ON_MESSAGE(MM_WIM_DATA, OnWimData)
END_MESSAGE_MAP()

CWaveInDevice::CWaveInDevice() {

    m_hInDev = NULL;
    m_hWnd = NULL;
    m_pHighPassFilter = NULL;
    m_pMixer = new CRecMixer;
    m_nBuffers = 0;
}

CWaveInDevice::~CWaveInDevice() {
    if (m_pMixer) {
        delete m_pMixer;
    }
    if (m_pHighPassFilter) {
        delete m_pHighPassFilter;
    }
}

BOOL CWaveInDevice::Create() {
    // create notification window with size 0
    if (!CreateEx(0, AfxRegisterWndClass(0), _T("Wave Wnd"), WS_POPUP, 0, 0, 0, 0, NULL, NULL)) {
        TRACE(_T("Failed to create wave notification window"));
        return FALSE;
    }
    return TRUE;
}

/***************************************************************************/
// CWaveInDevice::Open Open the input device
// The device receives a handle to the notification window.
/***************************************************************************/
BOOL CWaveInDevice::Open(WAVEFORMATEX & format) {
    MMRESULT mmr;
    // make sure we have a callback window
    if (!m_hWnd) {
        Create(); // create the callback window
        ASSERT(m_hWnd);
    }
    // see if input device already open for this format
    if (m_hInDev!=NULL) {
        // see if it can handle this format
        if (CanDoFormat(format)) {
            return TRUE;    // OK
        } else { // already open, but for a different format
            // error opening device
            CSaApp * pApp = (CSaApp *)AfxGetApp();
            pApp->ErrorMessage(IDS_ERROR_MMIO_OPENFORMAT);
            return FALSE;
        }
    }
    mmr = waveInOpen(&m_hInDev, WAVE_MAPPER, &format, (UINT)GetSafeHwnd(), 0, CALLBACK_WINDOW);
    if (mmr != 0) {
        MMERR(mmr); // display error message
        return FALSE;
    }
    return TRUE;
}

/***************************************************************************/
// CWaveInDevice::CanDoFormat Check if the device could open for this format
/***************************************************************************/
BOOL CWaveInDevice::CanDoFormat(WAVEFORMATEX & format) {

    MMRESULT mmr;
    // device already opened?
    if (m_hInDev==NULL) {
        TRACE(_T("Not open"));
        return FALSE;
    }
    mmr = waveInOpen(NULL, WAVE_MAPPER, &format, NULL, 0, WAVE_FORMAT_QUERY);
    if (mmr != 0) {
        MMERR(mmr); // display error message
        return FALSE;
    }
    return TRUE;
}

/***************************************************************************/
// CWaveInDevice::Close Close the input device
/***************************************************************************/
BOOL CWaveInDevice::Close() {
    if (m_hInDev) {
        // close the device
        waveInReset(m_hInDev);
        MSG msg;
        while (::PeekMessage(&msg, NULL, MM_WIM_DATA, MM_WIM_DATA, PM_REMOVE)) {
            // There are two headers in the queue which are released by waveInReset
            // They must be freed before we destroy the object
            // Since the OnWimData starts new block we need to only call WaveInDone
            WAVEHDR * pHdr = (WAVEHDR *)msg.lParam; // get pointer to wave header
            CWave * pWave = (CWave *)(pHdr->dwUser); // get pointer to CWave object
            WaveInDone(pWave, pHdr); // wave block done
            waveInReset(m_hInDev);
        }
        MMRESULT mmr = waveInClose(m_hInDev);
        if (mmr != 0) {
            MMERR(mmr);    // display error message
        }
        m_hInDev = NULL;
    }
    // destroy the window
    if (m_hWnd) {
        DestroyWindow();
    }
    ASSERT(m_hWnd == NULL);
    return TRUE;
}

/***************************************************************************/
// CWaveInDevice::AttachHighPassFilter  Construct a highpass filter
/***************************************************************************/
BOOL CWaveInDevice::AttachHighPassFilter(uint32 wSmpRate) {
    ASSERT(m_pHighPassFilter == NULL);
    return (CHighPassFilter70::CreateObject(&m_pHighPassFilter, wSmpRate) == DONE);
}

/***************************************************************************/
// CWaveInDevice::Record Start recording
/***************************************************************************/
BOOL CWaveInDevice::Record(int nBuffer, CWave * pWave, BOOL bStart) {

    // device has to be opened
    WAVEFORMATEX format = pWave->GetFormat().GetWaveFormatEX();
    if (!Open(format)) {
        return FALSE;
    }
    // allocate a header
    WAVEHDR * pHdr = (WAVEHDR *) new WAVEHDR;
    ASSERT(pHdr);
    // fill out the wave header
    memset(pHdr, 0, sizeof(WAVEHDR));
    pHdr->lpData = (char *) pWave->GetBufferPointer(nBuffer); // pointer to the data buffer
    // initialize buffer size
    pWave->SetBufferSize(nBuffer, pWave->CalculateBufferSize(), pWave->CalculateBufferSize());
    pHdr->dwBufferLength = pWave->GetBufferSize(nBuffer); // size of the data buffer
    pHdr->dwUser = (DWORD)(void *)pWave;   // so we can find the object

    // prepare the header
    MMRESULT mmr = waveInPrepareHeader(m_hInDev, pHdr, sizeof(WAVEHDR));
    if (mmr) {
        MMERR(mmr); // display error message
        return FALSE;
    }
    m_nBuffers++;

    // send it to the driver
    mmr = waveInAddBuffer(m_hInDev, pHdr, sizeof(WAVEHDR));
    if (mmr) {
        MMERR(mmr); // display error message
        return FALSE;
    }

    if (bStart) {
        // start the recording
        mmr = waveInStart(m_hInDev);
        if (mmr) {
            MMERR(mmr);
            return FALSE;
        }
    }

    return TRUE;
}

/***************************************************************************/
// CWaveInDevice::GetVolume Get the playback volume
/***************************************************************************/
MMRESULT CWaveInDevice::GetVolume(UINT & uVolume) {

    DWORD dwVolume = 0xbfff;            // set default
    MMRESULT result = MMSYSERR_NOERROR;

    if (m_hInDev!=NULL) {
        result = m_pMixer->GetVolume(m_hInDev, &dwVolume);
        if (result!=MMSYSERR_NOERROR) {
            return result;
        }
    } else {
        // device has to be opened first
        CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
        CView * pView = pMDIFrameWnd->GetCurrSaView();
        CSaDoc * pModel = (CSaDoc *)pView->GetDocument();
        CFmtParm fmtParm;
        pModel->GetFmtParm(fmtParm,true);     // get pointer to wave format parameters
        WAVEFORMATEX waveFormatEx = fmtParm.GetWaveFormatEX();
        result = waveInOpen(&m_hInDev , WAVE_MAPPER, &waveFormatEx, NULL, 0, CALLBACK_NULL);
        if (result!=MMSYSERR_NOERROR) {
            return result;
        }
        result = m_pMixer->GetVolume(m_hInDev, &dwVolume);
        if (result!=MMSYSERR_NOERROR) {
            return result;
        }
        result = waveInReset(m_hInDev);
        if (result!=MMSYSERR_NOERROR) {
            return result;
        }
        result = waveInClose(m_hInDev);
        if (result!=MMSYSERR_NOERROR) {
            return result;
        }
        m_hInDev = NULL;
    }

    uVolume = (UINT)(((float)(LOWORD(dwVolume)) * (float)100 / (float)0x0FFFF) + 0.5);

    return MMSYSERR_NOERROR;

}

/***************************************************************************/
// CWaveInDevice::SetVolume Set the playback volume
/***************************************************************************/
void CWaveInDevice::SetVolume(UINT nVolume, BOOL * pResult) {

    DWORD dwVolume= DWORD(((float)0x0FFFF * (float)nVolume / (float)100) + 0.5);
    BOOL bWasOpen = m_hInDev!=NULL;
    MMRESULT mmr = 0;
    if (!bWasOpen) {
        // device has to be opened first
        CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
        CView * pView = pMDIFrameWnd->GetCurrSaView();
        CSaDoc * pModel = (CSaDoc *)pView->GetDocument();
        CFmtParm fmtParm;
        pModel->GetFmtParm(fmtParm,true); // get pointer to wave format parameters
        WAVEFORMATEX waveFormatEx = fmtParm.GetWaveFormatEX();
        mmr = waveInOpen(&m_hInDev , WAVE_MAPPER, &waveFormatEx, NULL, 0, CALLBACK_NULL);
    }

    if (!mmr) {
        mmr = m_pMixer->SetVolume(m_hInDev, dwVolume);
    }

    if (!bWasOpen) {
        // close it again
        if (m_hInDev) {
            waveInReset(m_hInDev);
            mmr = waveInClose(m_hInDev);
            m_hInDev = NULL;
        }
    }
    if (pResult) {
        *pResult = (mmr == MMSYSERR_NOERROR);
    }
}


/***************************************************************************/
// CWaveInDevice::ShowMixer
/***************************************************************************/
BOOL CWaveInDevice::ShowMixer(BOOL bShow) {

    BOOL bResult = FALSE;
    BOOL bWasOpen = m_hInDev!=NULL;
    MMRESULT mmr = 0;
    if (!bWasOpen) {
        // device has to be opened first
        CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
        ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
        CView * pView = pMDIFrameWnd->GetCurrSaView();
        CSaDoc * pModel = (CSaDoc *)pView->GetDocument();
        CFmtParm fmtParm;
        pModel->GetFmtParm(fmtParm,true); // get pointer to wave format parameters
        WAVEFORMATEX waveFormatEx = fmtParm.GetWaveFormatEX();
        mmr = waveInOpen(&m_hInDev , WAVE_MAPPER, &waveFormatEx, NULL, 0, CALLBACK_NULL);
    }

    if (bShow && !mmr) {
        bResult = m_pMixer->ShowMixerControls(m_hInDev);
    }
    if (!bShow && !mmr) {
        bResult = m_pMixer->CanShowMixerControls(m_hInDev);
    }

    if (!bWasOpen) {
        // close it again
        if (m_hInDev) {
            waveInReset(m_hInDev);
            mmr = waveInClose(m_hInDev);
            m_hInDev = NULL;
        }
    }
    return bResult;
}


/***************************************************************************/
// CWaveInDevice::ConnectMixer Connect window to receive mixer callback msg's
/***************************************************************************/
BOOL CWaveInDevice::ConnectMixer(CWnd * pCallback) {

    BOOL bWasOpen = m_hInDev!=NULL;
    MMRESULT mmr = MMSYSERR_NOERROR;
    if (!bWasOpen) {
        // device has to be opened first
        CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
        CView * pView = pMDIFrameWnd->GetCurrSaView();
        CSaDoc * pModel = (CSaDoc *)pView->GetDocument();
        // get pointer to wave format parameters
        CFmtParm fmtParm;
        pModel->GetFmtParm(fmtParm,true);
        WAVEFORMATEX waveFormatEx = fmtParm.GetWaveFormatEX();
        mmr = waveInOpen(&m_hInDev, WAVE_MAPPER, &waveFormatEx, NULL, 0, CALLBACK_NULL);
        if (mmr!=MMSYSERR_NOERROR) {
            MMERR(mmr);
            return FALSE;
        }
    }

    mmr = m_pMixer->Connect(m_hInDev, pCallback->GetSafeHwnd());
    if (mmr!=MMSYSERR_NOERROR) {
        // display error message
        MMERR(mmr);
        return FALSE;
    }

    if (!bWasOpen) {
        // close it again
        if (m_hInDev!=NULL) {
            mmr = waveInReset(m_hInDev);
            if (mmr!=MMSYSERR_NOERROR) {
                // display error message
                MMERR(mmr);
                return FALSE;
            }
            mmr = waveInClose(m_hInDev);
            m_hInDev = NULL;
            if (mmr!=MMSYSERR_NOERROR) {
                // display error message
                MMERR(mmr);
                return FALSE;
            }
        }
    }

    return TRUE;
}

/***************************************************************************/
// CWaveInDevice::Reset Reset the device
/***************************************************************************/
void CWaveInDevice::Reset() {
    if (!m_hInDev) {
        return;
    }
    MMRESULT mmr = waveInReset(m_hInDev);
    if (mmr) {
        MMERR(mmr);
    }
}

/***************************************************************************/
// CWaveInDevice::WaveInDone Block finished recording
/***************************************************************************/
void CWaveInDevice::WaveInDone(CWave * /*pWave*/, WAVEHDR * pHdr) {
    // unprepare the header
    MMRESULT mmr = waveInUnprepareHeader(m_hInDev, pHdr, sizeof(WAVEHDR));
    m_nBuffers--;
    if (mmr) {
        MMERR(mmr);    // display error message
    }
    // free the header
    delete(pHdr);
}

/***************************************************************************/
// CWaveInDevice::OnWimData
/***************************************************************************/
LRESULT CWaveInDevice::OnWimData(WPARAM /*wParam*/, LPARAM lParam) {

    WAVEHDR * pHdr = (WAVEHDR *)lParam;         // get pointer to wave header
    CWave * pWave = (CWave *)(pHdr->dwUser);    // get pointer to CWave object
    CWaveInDevice * pInDev = pWave->GetInDevice(); // get pointer to input device
    ASSERT(pInDev);
    pWave->SetBufferSize(pWave->GetActiveBlock(), pHdr->dwBytesRecorded, pHdr->dwBytesRecorded);

    pInDev->WaveInDone(pWave, pHdr);            // wave block done
    pWave->StoreBlock();                        // store data block
    return 0;
}

CHighPassFilter70 * CWaveInDevice::GetHighPassFilter() {
    return m_pHighPassFilter;
}

void CWaveInDevice::DetachHighPassFilter() {
    delete m_pHighPassFilter;
    m_pHighPassFilter = NULL;
}

int CWaveInDevice::GetPreparedBuffers() const {
    return m_nBuffers;
}
