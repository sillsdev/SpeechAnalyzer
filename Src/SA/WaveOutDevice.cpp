#include "stdafx.h"
#include "WaveOutDevice.h"
#include "sa_wave.h"
#include "sa_doc.h"
#include "sa.h"
#include "sa_view.h"
#include "mainfrm.h"
#include "FmtParm.h"
#include "PlayMixer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


//###########################################################################
// CWaveOutDevice output device
// Output device for playing. It uses a notification window to receive no-
// tification messages from the output device.

BEGIN_MESSAGE_MAP(CWaveOutDevice, CWnd)
    ON_MESSAGE(MM_WOM_DONE, OnWomDone)
END_MESSAGE_MAP()

CWaveOutDevice::CWaveOutDevice() {

    m_hOutDev = NULL;
    m_hWnd = NULL;
    m_pMixer = new CPlayMixer();
}

CWaveOutDevice::~CWaveOutDevice() {

    ASSERT(m_hOutDev == NULL);
    if (m_pMixer) {
        delete m_pMixer;
        m_pMixer = NULL;
    }
}

/***************************************************************************/
// CWaveOutDevice::Create Creation
// Creates a child window with the given parameters as notification window.
/***************************************************************************/
BOOL CWaveOutDevice::Create() {

    // create notification window with size 0
    if (!CreateEx(0, AfxRegisterWndClass(0), _T("Wave Wnd"), WS_POPUP, 0, 0, 0, 0, NULL, NULL)) {
        TRACE(_T("Failed to create wave notification window"));
        return FALSE;
    }
    return TRUE;
}

/***************************************************************************/
// CWaveOutDevice::Open Open the output device
// The device receives a handle to the notification window.
/***************************************************************************/
BOOL CWaveOutDevice::Open(WAVEFORMATEX & format) {

    // make sure we have a callback window
    if (!m_hWnd) {
        // create the callback window
        Create();
        ASSERT(m_hWnd);
    }

    // see if output device already open for this format
    if (m_hOutDev!=NULL) {
        // see if it can handle this format
        if (CanDoFormat(format)) {
            return TRUE;    // OK
        }

        // already open, but for a different format
        // error opening device
        CSaApp * pApp = (CSaApp *)AfxGetApp();
        pApp->ErrorMessage(IDS_ERROR_MMIO_OPENFORMAT);
        return FALSE;
    }

    MMRESULT mmr = waveOutOpen(&m_hOutDev, WAVE_MAPPER, &format, (UINT)GetSafeHwnd(), 0, CALLBACK_WINDOW);
    if (mmr != MMSYSERR_NOERROR) {
        // display error message
        MMERR(mmr);
        return FALSE;
    }
    return TRUE;
}

/***************************************************************************/
// CWaveOutDevice::CanDoFormat Check if the device could open for this format
/***************************************************************************/
BOOL CWaveOutDevice::CanDoFormat(WAVEFORMATEX & format) {

    // device already opened?
    if (m_hOutDev==NULL) {
        TRACE(_T("Not open"));
        return FALSE;
    }

    HWAVEOUT hDev = NULL;
    MMRESULT mmr = waveOutOpen(&hDev, WAVE_MAPPER, &format, NULL, 0, WAVE_FORMAT_QUERY);
    if (mmr != MMSYSERR_NOERROR) {
        MMERR(mmr); // display error message
        return FALSE;
    }
    return TRUE;
}

/***************************************************************************/
// CWaveOutDevice::Close Close the output device
/***************************************************************************/
BOOL CWaveOutDevice::Close() {

    if (m_hOutDev!=NULL) {
        // close the device
        waveOutReset(m_hOutDev);
        MSG msg;
        while (::PeekMessage(&msg, NULL, MM_WOM_DONE, MM_WOM_DONE, PM_REMOVE)) {
            // There are two headers in the queue which are released by waveOutReset
            // They must be freed before we destroy the object
            // Since the OnWomDone starts new block we need to only call WaveOutDone
            WAVEHDR * pHdr = (WAVEHDR *)msg.lParam;     // get pointer to wave header
            CWave * pWave = (CWave *)(pHdr->dwUser);    // get pointer to CWave object
            WaveOutDone(pWave, pHdr);                   // wave block done
        }
        MMRESULT mmr = waveOutClose(m_hOutDev);
        if (mmr != MMSYSERR_NOERROR) {
            MMERR(mmr);
        }
        m_hOutDev = NULL;
    }
    // destroy the window
    if (m_hWnd!=NULL) {
        DestroyWindow();
    }
    ASSERT(m_hWnd == NULL);
    return TRUE;
}

/***************************************************************************/
// CWaveOutDevice::Play Start playing
/***************************************************************************/
BOOL CWaveOutDevice::Play(int nBuffer, UINT nVolume, CWave * pWave, BOOL bPause) {
    if (pWave->GetPlaybackSize(nBuffer)==0) {
        return TRUE;
    }

    // device has to be opened
    WAVEFORMATEX format = pWave->GetFormat().GetWaveFormatEX();
    if (!Open(format)) {
        return FALSE;
    }

    // allocate a header
    WAVEHDR * pHdr = new WAVEHDR;
    ASSERT(pHdr);
    // fill out the wave header
    memset(pHdr, 0, sizeof(WAVEHDR));
    pHdr->lpData = (char *) pWave->GetPlaybackPointer(nBuffer); // pointer to the data buffer
    pHdr->dwBufferLength = pWave->GetPlaybackSize(nBuffer); // size of the data buffer
    pHdr->dwUser = (DWORD)(void *)pWave;   // so we can find the object
    // prepare the header
    MMRESULT mmr = waveOutPrepareHeader(m_hOutDev, pHdr, sizeof(WAVEHDR));
    if (mmr!=MMSYSERR_NOERROR) {
        MMERR(mmr);     // display error message
        return FALSE;
    }
    // set the volume
    BOOL bResult = FALSE;
    SetVolume(nVolume,bResult);
    if (bPause) {
        mmr = waveOutPause(m_hOutDev);
        if (mmr!=MMSYSERR_NOERROR) {
            MMERR(mmr); // display error message
            return FALSE;
        }
    }
    // start it playing
    mmr = waveOutWrite(m_hOutDev, pHdr, sizeof(WAVEHDR));
    if (mmr!=MMSYSERR_NOERROR) {
        MMERR(mmr); // display error message
        return FALSE;
    }

    if (!bPause) {
        mmr = waveOutRestart(m_hOutDev);
        if (mmr!=MMSYSERR_NOERROR) {
            MMERR(mmr); // display error message
            return FALSE;
        }
    }

    return TRUE;
}

/***************************************************************************/
// CWaveOutDevice::GetVolume Get the playback volume
/***************************************************************************/
UINT CWaveOutDevice::GetVolume(BOOL & bResult) {

    bResult = FALSE;
    DWORD dwVolume = 0xbfff; // Value in case of error (75%)

    if (m_hOutDev!=NULL) {
        // already open
        MMRESULT mmr = m_pMixer->GetVolume(m_hOutDev, &dwVolume);
        bResult = (mmr==MMSYSERR_NOERROR);
        return (UINT)(((float)(LOWORD(dwVolume)) * (float)100 / (float)0x0FFFF) + 0.5);
    }

    // the device is not already open
    // device has to be opened first
    CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
    CView * pView = pMDIFrameWnd->GetCurrSaView();
    CSaDoc * pModel = (CSaDoc *)pView->GetDocument();
    CFmtParm fmtParm;
    pModel->GetFmtParm(fmtParm,true);
    WAVEFORMATEX formatEx = fmtParm.GetWaveFormatEX();
    MMRESULT mmr = waveOutOpen(&m_hOutDev, WAVE_MAPPER, &formatEx, NULL, 0, CALLBACK_NULL);
    if (mmr!=MMSYSERR_NOERROR) {
        bResult = FALSE;
        return (UINT)(((float)(LOWORD(dwVolume)) * (float)100 / (float)0x0FFFF) + 0.5);
    }

    mmr = m_pMixer->GetVolume(m_hOutDev, &dwVolume);
    if (mmr!=MMSYSERR_NOERROR) {
        bResult = FALSE;
        return (UINT)(((float)(LOWORD(dwVolume)) * (float)100 / (float)0x0FFFF) + 0.5);
    }

    // close it again
    mmr = waveOutReset(m_hOutDev);
    if (mmr!=MMSYSERR_NOERROR) {
        bResult = FALSE;
        return (UINT)(((float)(LOWORD(dwVolume)) * (float)100 / (float)0x0FFFF) + 0.5);
    }

    mmr = waveOutClose(m_hOutDev);
    if (mmr!=MMSYSERR_NOERROR) {
        bResult = FALSE;
        return (UINT)(((float)(LOWORD(dwVolume)) * (float)100 / (float)0x0FFFF) + 0.5);
    }

    m_hOutDev = NULL;
    bResult = TRUE;
    return (UINT)(((float)(LOWORD(dwVolume)) * (float)100 / (float)0x0FFFF) + 0.5);
}

/***************************************************************************/
// CWaveOutDevice::SetVolume Set the playback volume
/***************************************************************************/
void CWaveOutDevice::SetVolume(UINT nVolume, BOOL & bResult) {

    bResult = FALSE;
    DWORD dwVolume= DWORD(((float)0x0FFFF * (float)nVolume / (float)100) + 0.5);

    if (m_hOutDev!=NULL) {
        MMRESULT mmr = m_pMixer->SetVolume(m_hOutDev, dwVolume);
        bResult = (mmr!=MMSYSERR_NOERROR);
        return;
    }

    // device has to be opened first
    CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
    CView * pView = pMDIFrameWnd->GetCurrSaView();
    CSaDoc * pModel = (CSaDoc *)pView->GetDocument();

    CFmtParm fmtParm;
    pModel->GetFmtParm(fmtParm,true); // get pointer to wave format parameters
    WAVEFORMATEX formatEx = fmtParm.GetWaveFormatEX();
    MMRESULT mmr = waveOutOpen(&m_hOutDev, WAVE_MAPPER, &formatEx, NULL, 0, CALLBACK_NULL);
    if (mmr!=MMSYSERR_NOERROR) {
        bResult = FALSE;
        return;
    }
    mmr = m_pMixer->SetVolume(m_hOutDev, dwVolume);
    if (mmr!=MMSYSERR_NOERROR) {
        bResult = FALSE;
        return;
    }
    // close it again
    mmr = waveOutReset(m_hOutDev);
    if (mmr!=MMSYSERR_NOERROR) {
        bResult = FALSE;
        return;
    }
    mmr = waveOutClose(m_hOutDev);
    if (mmr!=MMSYSERR_NOERROR) {
        bResult = FALSE;
        return;
    }
    m_hOutDev = NULL;
    bResult = TRUE;
}


/***************************************************************************/
// CWaveOutDevice::ShowMixer
/***************************************************************************/
BOOL CWaveOutDevice::ShowMixer(BOOL bShow) {
    BOOL bWasOpen = m_hOutDev!=NULL;
    BOOL bResult = FALSE;
    MMRESULT mmr = 0;
    if (!bWasOpen) {
        // device has to be opened first
        CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
        CView * pView = pMDIFrameWnd->GetCurrSaView();

        CSaDoc * pModel = (CSaDoc *)pView->GetDocument();
        CFmtParm fmtParm;
        pModel->GetFmtParm(fmtParm,true); // get pointer to wave format parameters
        WAVEFORMATEX formatEx = fmtParm.GetWaveFormatEX();
        mmr = waveOutOpen(&m_hOutDev, WAVE_MAPPER, &formatEx, NULL, 0, CALLBACK_NULL);
    }

    if (bShow && !mmr) {
        bResult = m_pMixer->ShowMixerControls(m_hOutDev);
    }
    if (!bShow && !mmr) {
        bResult = m_pMixer->CanShowMixerControls(m_hOutDev);
    }

    if (!bWasOpen) {
        // close it again
        if (m_hOutDev) {
            waveOutReset(m_hOutDev);
            mmr = waveOutClose(m_hOutDev);
            m_hOutDev = NULL;
        }
    }
    return bResult;
}


/***************************************************************************/
// CWaveOutDevice::ConnectMixer Connect window to receive mixer callback msg's
/***************************************************************************/
BOOL CWaveOutDevice::ConnectMixer(CWnd * pCallback) {
    //TRACE("ConnectMixer\n");
    if (m_hOutDev!=NULL) {
        // already opened
        MMRESULT mmr = m_pMixer->Connect(m_hOutDev, pCallback->GetSafeHwnd());
        if (mmr!=MMSYSERR_NOERROR) {
            MMERR(mmr);
            return FALSE;
        }
        return TRUE;
    }

    // object is not already open
    // device has to be opened first
    CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
    CView * pView = pMDIFrameWnd->GetCurrSaView();
    CSaDoc * pModel = (CSaDoc *)pView->GetDocument();
    CFmtParm fmtParm;
    pModel->GetFmtParm(fmtParm,true);
    WAVEFORMATEX formatEx = fmtParm.GetWaveFormatEX();
    MMRESULT mmr = waveOutOpen(&m_hOutDev, WAVE_MAPPER, &formatEx, NULL, 0, CALLBACK_NULL);
    if (mmr!=MMSYSERR_NOERROR) {
        MMERR(mmr);
        return FALSE;
    }

    // already opened
    mmr = m_pMixer->Connect(m_hOutDev, pCallback->GetSafeHwnd());
    if (mmr!=MMSYSERR_NOERROR) {
        MMERR(mmr);
        return FALSE;
    }

    // reset it
    mmr = waveOutReset(m_hOutDev);
    if (mmr!=MMSYSERR_NOERROR) {
        MMERR(mmr);
        return FALSE;
    }

    // close it
    mmr = waveOutClose(m_hOutDev);
    if (mmr!=MMSYSERR_NOERROR) {
        MMERR(mmr);
        return FALSE;
    }

    m_hOutDev = NULL;
    return TRUE;
}

/***************************************************************************/
// CWaveOutDevice::Reset Reset the device
/***************************************************************************/
void CWaveOutDevice::Reset() {
    if (m_hOutDev==NULL) {
        return;
    }

    MMRESULT mmr = waveOutReset(m_hOutDev);
    if (mmr) {
        MMERR(mmr);
    }
}

/***************************************************************************/
// CWaveOutDevice::WaveOutDone Playing finished
/***************************************************************************/
void CWaveOutDevice::WaveOutDone(CWave * /*pWave*/, WAVEHDR * pHdr) {

    //TRACE("WaveOutDone\n");
    // unprepare the header
    MMRESULT mmr = waveOutUnprepareHeader(m_hOutDev, pHdr, sizeof(WAVEHDR));
    if (mmr!=MMSYSERR_NOERROR) {
        MMERR(mmr);     // display error message
    }
    // free the header
    if (pHdr!=NULL) {
        delete pHdr;
    }
}

/**
* CWaveOutDevice::OnWomDone
* The MM_WOM_DONE message is sent to a window when the given output buffer is
* being returned to the application.
* Buffers are returned to the application when they have been played, or as
* the result of a call to the waveOutReset function.
*/
LRESULT CWaveOutDevice::OnWomDone(WPARAM /*wParam*/, LPARAM lParam) {

    //TRACE("OnWomDone\n");
    WAVEHDR * pHdr = (WAVEHDR *)lParam;         // get pointer to wave header
    CWave * pWave = (CWave *)(pHdr->dwUser);    // get pointer to CWave object
    CWaveOutDevice * pOutDev = pWave->GetOutDevice(); // get pointer to output device
    ASSERT(pOutDev);
    pOutDev->WaveOutDone(pWave, pHdr);          // wave block done
    pWave->NextBlock();                         // prepare next data block
    return 0;
}
