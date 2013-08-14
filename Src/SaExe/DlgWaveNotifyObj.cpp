#include "stdafx.h"
#include "DlgWaveNotifyObj.h"
#include "sa_plot.h"
#include "sa_graph.h"
#include "fileInformation.h"
#include "sa_view.h"
#include "sa.h"
#include "mainfrm.h"
#include "FileUtils.h"
#include "WaveOutDevice.h"
#include "WaveInDevice.h"
#include "DlgFnKeys.h"
#include "DlgRecorderOptions.h"
#include "Process\sa_p_fra.h"
#include "Process\Process.h"
#include "settings\obstream.h"

//###########################################################################
// CDlgWaveNotifyObj Notify object for playing wave data
// This object is used to make the bridge between the CWave class, playing
// or recording wave data in the back ground, and a dialog, showing the user
// progress and status of the playing/recording and allowing him to control
// the process. This object only handles messages between the two classes.

CDlgWaveNotifyObj::CDlgWaveNotifyObj()
{
    m_pClient = NULL;
}

/***************************************************************************/
// CDlgWaveNotifyObj::~CDlgWaveNotifyObj Destructor
/***************************************************************************/
CDlgWaveNotifyObj::~CDlgWaveNotifyObj()
{
}

void CDlgWaveNotifyObj::Attach( IWaveNotifiable * pClient)
{
    m_pClient = pClient;
}

/***************************************************************************/
// CDlgWaveNotifyObj::BlockFinished Block finished playing
// The actually playing data block has been finished. Notify the player or
// the recorder dialog.
/***************************************************************************/
void CDlgWaveNotifyObj::BlockFinished(UINT nLevel, DWORD dwPosition, UINT nSpeed)
{
    ASSERT(m_pClient);
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    // check if player is playing
    if (pMainWnd->IsPlayerPlaying())
    {
        // call player
        m_pClient->BlockFinished(nLevel, dwPosition, nSpeed);
        return;
    }
    if (pMainWnd->IsPlayerTestRun())
    {
        return;    // no call
    }
    // call recorder
    m_pClient->BlockFinished(nLevel, dwPosition, nSpeed);
}

/***************************************************************************/
// CDlgWaveNotifyObj::BlockStored Recorded block stored
// The actually recorded data block has been stored. Notify the recorder
// dialog.
/***************************************************************************/
void CDlgWaveNotifyObj::BlockStored(UINT nLevel, DWORD dwPosition, BOOL * bSaveOverride)
{
    ASSERT(m_pClient);
    m_pClient->BlockStored(nLevel, dwPosition, bSaveOverride);
}

/***************************************************************************/
// CDlgWaveNotifyObj::StoreFailed Block storing failed
// The actually recorded data block has been failed storing. Notify the
// recorder dialog.
/***************************************************************************/
void CDlgWaveNotifyObj::StoreFailed()
{
    ASSERT(m_pClient);
    m_pClient->StoreFailed();
}

/***************************************************************************/
// CDlgWaveNotifyObj::EndPlayback Playback finished
// The playback has been finished. Notify the player or the recorder dialog.
/***************************************************************************/
void CDlgWaveNotifyObj::EndPlayback()
{
    ASSERT(m_pClient);
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    // check if player is playing
    if (pMainWnd->IsPlayerPlaying())
    {
        // call player
        m_pClient->EndPlayback();
        return;
    }
    if (pMainWnd->IsPlayerTestRun())
    {
        // call function key dialog (test run)
        m_pClient->EndPlayback();
        return;
    }
    // call recorder
    m_pClient->EndPlayback();
}

/***************************************************************************/
// CDlgWaveNotifyObj::GetWaveData Delivers wave data to playback
// All the requests for wave data from the CWave objects are going through
// this function. If the player is playing, the data will be prepared by
// the document. If the request is coming from the recorder, the recorder will
// deliver the data.
/***************************************************************************/
HPSTR CDlgWaveNotifyObj::GetWaveData(CView * pView, DWORD dwPlayPosition, DWORD dwDataSize)
{

    //TRACE("GetWaveData %d %d\n",dwPlayPosition, dwDataSize);
    ASSERT(m_pClient!=NULL);
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    // check if player is playing
    if (pMainWnd->IsPlayerPlaying() || pMainWnd->IsPlayerTestRun())
    {
        // request for data comes from player
        CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
        DWORD dwWaveBufferSize = pDoc->GetWaveDataBufferSize();
		// if the request is outside of the documents buffer or
        if (((dwPlayPosition + dwDataSize) > (pDoc->GetWaveBufferIndex() + dwWaveBufferSize)) ||
			// the request is greater than the buffer?
            ((dwPlayPosition + dwDataSize) > (dwPlayPosition - (dwPlayPosition % dwWaveBufferSize) + dwWaveBufferSize)))
        {
            return pDoc->GetWaveData(dwPlayPosition, TRUE);     // get pointer to data block
        }
        else
        {
            HPSTR pData = pDoc->GetWaveData(dwPlayPosition);    // get pointer to data block
            if (pData == NULL)
            {
                // error while reading data
                return NULL;
            }
            pData += dwPlayPosition - pDoc->GetWaveBufferIndex();
            return pData;
        }
    }
    else
    {
        // request for data comes from recorder
        return m_pClient->GetWaveData(dwPlayPosition, dwDataSize);
    }
}

