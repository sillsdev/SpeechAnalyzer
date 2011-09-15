//////////////////////////////////////////////////////////////////////////////
// playerRecorder.cpp:
// Implementation of the CDlgWaveNotifyObj (notify object)
//                       CDlgPlayer (dialog)
//                       CDlgRecorder (dialog)
//                       CDlgRecorderOptions (dialog)
//                       CDlgFnKeys (dialog)
//
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//
// 01/12/2000
//    SDM   Extract from sa_dlg2.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_plot.h"
#include "Process\sa_proc.h"
#include "sa_graph.h"
#include "Process\sa_p_fra.h"
#include "playerRecorder.h"
#include "fileInformation.h"
#include "sa_view.h"
#include "sa.h"
#include "mainfrm.h"
#include "settings\obstream.h"

//###########################################################################
// CDlgWaveNotifyObj Notify object for playing wave data
// This object is used to make the bridge between the CWave class, playing
// or recording wave data in the back ground, and a dialog, showing the user
// progress and status of the playing/recording and allowing him to control
// the process. This object only handles messages between the two classes.

/////////////////////////////////////////////////////////////////////////////
// CDlgWaveNotifyObj message map

/////////////////////////////////////////////////////////////////////////////
// CDlgWaveNotifyObj construction/destruction/creation

/***************************************************************************/
// CDlgWaveNotifyObj::CDlgWaveNotifyObj Constructor
/***************************************************************************/
CDlgWaveNotifyObj::CDlgWaveNotifyObj()
{
  m_pDlg = NULL;
}

/***************************************************************************/
// CDlgWaveNotifyObj::~CDlgWaveNotifyObj Destructor
/***************************************************************************/
CDlgWaveNotifyObj::~CDlgWaveNotifyObj()
{
}

/////////////////////////////////////////////////////////////////////////////
// CDlgWaveNotifyObj helper functions

/***************************************************************************/
// CDlgWaveNotifyObj::BlockFinished Block finished playing
// The actually playing data block has been finished. Notify the player or
// the recorder dialog.
/***************************************************************************/
void CDlgWaveNotifyObj::BlockFinished(UINT nLevel, DWORD dwPosition, UINT nSpeed)
{
  ASSERT(m_pDlg);
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
  // check if player is playing
  if (pMainWnd->IsPlayerPlaying())
  {
    // call player
    m_pDlg->BlockFinished(nLevel, dwPosition, nSpeed);
    return;
  }
  if (pMainWnd->IsPlayerTestRun()) return; // no call
  // call recorder
  m_pDlg->BlockFinished(nLevel, dwPosition, nSpeed);
}

/***************************************************************************/
// CDlgWaveNotifyObj::BlockStored Recorded block stored
// The actually recorded data block has been stored. Notify the recorder
// dialog.
/***************************************************************************/
void CDlgWaveNotifyObj::BlockStored(UINT nLevel, DWORD dwPosition, BOOL *bSaveOverride)
{
  ASSERT(m_pDlg);
  m_pDlg->BlockStored(nLevel, dwPosition, bSaveOverride);
}

/***************************************************************************/
// CDlgWaveNotifyObj::StoreFailed Block storing failed
// The actually recorded data block has been failed storing. Notify the
// recorder dialog.
/***************************************************************************/
void CDlgWaveNotifyObj::StoreFailed()
{
  ASSERT(m_pDlg);
  m_pDlg->StoreFailed();
}

/***************************************************************************/
// CDlgWaveNotifyObj::EndPlayback Playback finished
// The playback has been finished. Notify the player or the recorder dialog.
/***************************************************************************/
void CDlgWaveNotifyObj::EndPlayback()
{
  ASSERT(m_pDlg);
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
  // check if player is playing
  if (pMainWnd->IsPlayerPlaying())
  {
    // call player
    CDlgPlayer* pDlg = (CDlgPlayer*)m_pDlg; // cast to player dialog
    pDlg->EndPlayback();
    return;
  }
  if (pMainWnd->IsPlayerTestRun())
  {
    // call function key dialog (test run)
    CDlgFnKeys* pFnDlg = (CDlgFnKeys*)m_pDlg; // cast to function key dialog
    pFnDlg->EndPlayback();
    return;
  }
  // call recorder
  m_pDlg->EndPlayback();
}

/***************************************************************************/
// CDlgWaveNotifyObj::GetWaveData Delivers wave data to playback
// All the requests for wave data from the CWave objects are going through
// this function. If the player is playing, the data will be prepared by
// the document. If the request is coming from the recorder, the recorder will
// deliver the data.
/***************************************************************************/
HPSTR CDlgWaveNotifyObj::GetWaveData(CView* pView, DWORD dwPlayPosition, DWORD dwDataSize)
{
  ASSERT(m_pDlg);
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
  // check if player is playing
  if (pMainWnd->IsPlayerPlaying() || pMainWnd->IsPlayerTestRun())
  {
    // request for data comes from player
    CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();
    DWORD dwWaveBufferSize = GetBufferSize();
    if (((dwPlayPosition + dwDataSize) > (pDoc->GetWaveBufferIndex() + dwWaveBufferSize))
      || ((dwPlayPosition + dwDataSize) > (dwPlayPosition - (dwPlayPosition % dwWaveBufferSize) + dwWaveBufferSize)))
    {
      return pDoc->GetWaveData(dwPlayPosition, TRUE); // get pointer to data block
    }
    else
    {
      HPSTR pData = pDoc->GetWaveData(dwPlayPosition); // get pointer to data block
      if (pData == NULL) return NULL; // error while reading data
      pData += dwPlayPosition - pDoc->GetWaveBufferIndex();
      return pData;
    }
  }
  else
  {
    // request for data comes from recorder
    return m_pDlg->GetWaveData(dwPlayPosition, dwDataSize);
  }
}

//###########################################################################
// CDlgPlayer dialog
// Displays the player that allows the user to control the wave data playing
// process, done by the CWave class.

/////////////////////////////////////////////////////////////////////////////
// CDlgPlayer static members definition

BOOL CDlgPlayer::bPlayer = FALSE; // player not launched at startup

/////////////////////////////////////////////////////////////////////////////
// CDlgPlayer message map

BEGIN_MESSAGE_MAP(CDlgPlayer, CDlgAudio)
//{{AFX_MSG_MAP(CDlgPlayer)
  ON_COMMAND(IDC_PLAY, OnPlay)
  ON_COMMAND(IDC_STOP, OnStop)
  ON_COMMAND(IDC_PAUSE, OnPause)
  ON_COMMAND(IDC_VOLUMESLIDER, OnVolumeSlide)
  ON_COMMAND(IDC_VOLUMESCROLL, OnVolumeScroll)
  ON_EN_KILLFOCUS(IDC_VOLUMEEDIT, OnKillfocusVolumeEdit)
  ON_COMMAND(IDC_SPEEDSLIDER, OnSpeedSlide)
  ON_COMMAND(IDC_SPEEDSCROLL, OnSpeedScroll)
  ON_EN_KILLFOCUS(IDC_SPEEDEDIT, OnKillfocusSpeedEdit)
  ON_COMMAND(IDC_DELAYSLIDER, OnDelaySlide)
  ON_COMMAND(IDC_DELAYSCROLL, OnDelayScroll)
  ON_EN_KILLFOCUS(IDC_DELAYEDIT, OnKillfocusDelayEdit)
  ON_CBN_SELCHANGE(IDC_PLAYMODE, OnSelchangePlaymode)
  ON_WM_CLOSE()
  ON_BN_CLICKED(IDC_REPEAT, OnRepeat)
  ON_WM_TIMER()
  ON_BN_CLICKED(IDC_SETUP, OnSetup)
  ON_BN_CLICKED(IDC_PLAY_CONTROLS, OnPlayControls)
  ON_MESSAGE(MM_MIXM_CONTROL_CHANGE, OnMixerControlChange)
	ON_COMMAND(IDHELP, OnHelpPlayer)
	//}}AFX_MSG_MAP
  ON_BN_CLICKED(ID_CLOSE, OnClose)
  ON_MESSAGE(WM_USER_SETUP_FNKEYS, OnSetupFnKeys)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgPlayer construction/destruction/creation

/***************************************************************************/
// CDlgPlayer::CDlgPlayer Constructor
/***************************************************************************/
CDlgPlayer::CDlgPlayer(CWnd* pParent) : CDlgAudio(CDlgPlayer::IDD, pParent)
{
  //{{AFX_DATA_INIT(CDlgPlayer)
  m_nComboPlayMode = 2;
  m_nSpeed = 100;
  m_nDelay = 1000;
  m_bRepeat = FALSE;
  m_bTestRunning = FALSE;
  //}}AFX_DATA_INIT
  m_pDoc = NULL;
  m_pView = NULL;
  m_NotifyObj.Attach(this); // attach notify object
  m_pWave = new CWave;      // create CWave object
  m_nVolume = m_pWave->GetVolume();
  m_bFullSize = FALSE;
  m_dwPlayPosition = 0;
  m_bFnKeySetting = FALSE;
}

/***************************************************************************/
// Added because sometimes the player object was deleted without setting
// this flag to FALSE. That caused some GPF's. Also added the DestroyWindow
// because the MFC code was giving ASSERTS without it. Possible memory
// leaks. 09/30/2000 - DDO
/***************************************************************************/
CDlgPlayer::~CDlgPlayer()
{
  bPlayer = FALSE;
	if (m_pWave)
		delete m_pWave;
  DestroyWindow();
}

/***************************************************************************/
// CDlgPlayer::Create Creation
/***************************************************************************/
BOOL CDlgPlayer::Create()
{
  bPlayer = TRUE; // player launched
  return CDialog::Create(CDlgPlayer::IDD);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgPlayer helper functions

/***************************************************************************/
// CDlgPlayer::DoDataExchange Data exchange
/***************************************************************************/
void CDlgPlayer::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CDlgPlayer)
  DDX_CBIndex(pDX, IDC_PLAYMODE, m_nComboPlayMode);
  DDX_Text(pDX, IDC_VOLUMEEDIT, m_nVolume);
  DDV_MinMaxInt(pDX, m_nVolume, 0, 100);
  DDX_Text(pDX, IDC_SPEEDEDIT, m_nSpeed);
  DDV_MinMaxInt(pDX, m_nSpeed, 10, 400);
  DDX_Text(pDX, IDC_DELAYEDIT, m_nDelay);
  DDV_MinMaxUInt(pDX, m_nDelay, 0, 9999);
  DDX_Check(pDX, IDC_REPEAT, m_bRepeat);
  //}}AFX_DATA_MAP
}

/***************************************************************************/
// CDlgPlayer::SetTotalTime Set total time display
/***************************************************************************/
void CDlgPlayer::SetTotalTime()
{
  CSaView* pView = (CSaView*)m_pView; // cast pointer to view
  CSaDoc* pDoc = (CSaDoc*)m_pDoc; // cast pointer to document
  DWORD dwEnd = pDoc->GetDataSize(); // data end
  if (dwEnd > 0)
  {
    // display depends on the submode
    switch(m_nSubMode)
    {
      case ID_PLAYBACK_FILE:
        break;
      case ID_PLAYBACK_WINDOW:
      case ID_PLAYBACK_STOPTOR:
      case ID_PLAYBACK_STARTTOR:
        dwEnd = DWORD(pView->GetDataPosition(0)) + pView->GetDataFrame();
        break;
      case ID_PLAYBACK_CURSORS:
      case ID_PLAYBACK_LTOSTOP:
        dwEnd = pView->GetStopCursorPosition();
        break;
      case ID_PLAYBACK_LTOSTART:
        dwEnd = pView->GetStartCursorPosition();
        break;
      default: break;
    }
    double fDataSec = pView->GetDocument()->GetTimeFromBytes(dwEnd); // calculate time
    m_LEDTotalTime.SetTime((int)fDataSec / 60, (int)(fDataSec * 10) % 600);
  }
}

/***************************************************************************/
// CDlgPlayer::SetPositionTime Set position time display
/***************************************************************************/
void CDlgPlayer::SetPositionTime()
{
  CSaView* pView = (CSaView*)m_pView; // cast pointer to view
  CSaDoc* pDoc = (CSaDoc*)m_pDoc; // cast pointer to document
  if (pDoc->GetDataSize() > 0)
  {
    DWORD dwPosition = m_dwPlayPosition; // data position
    if (m_dwPlayPosition == 0)
    {
      // display depends on the submode
      switch(m_nSubMode)
      {
        case ID_PLAYBACK_FILE:
          break;
        case ID_PLAYBACK_WINDOW:
        case ID_PLAYBACK_LTOSTART:
        case ID_PLAYBACK_LTOSTOP:
          dwPosition = DWORD(pView->GetDataPosition(0));
          break;
        case ID_PLAYBACK_CURSORS:
        case ID_PLAYBACK_STARTTOR:
          dwPosition = pView->GetStartCursorPosition();
          break;
        case ID_PLAYBACK_STOPTOR:
          dwPosition = pView->GetStopCursorPosition();
          break;
        default: break;
      }
    }
    double fDataSec = pDoc->GetTimeFromBytes(dwPosition); // calculate time
    m_LEDPosTime.SetTime((int)fDataSec / 60, (int)(fDataSec * 10) % 600);
    SetDlgItemText(IDC_TIMERTEXT, _T("Start Time"));
  }
}

/***************************************************************************/
// CDlgPlayer::SetPlayerFullSize Set the player size to full size
/***************************************************************************/
void CDlgPlayer::SetPlayerFullSize()
{
  m_bFullSize = TRUE; // player will become full size, is now small
  CenterWindow(); // move it in place (the zero window)
  CRect rWnd;
  GetWindowRect(rWnd); // get screen coordinates of dialog
  rWnd.left = rWnd.left - m_rSize.right / 2;
  rWnd.top = rWnd.top - m_rSize.bottom / 2;
  rWnd.right = rWnd.left + m_rSize.right;
  rWnd.bottom = rWnd.top + m_rSize.bottom;
  MoveWindow(rWnd, TRUE); // change the size
  CenterWindow(); // move it in place again
}


const UINT CDlgPlayer::SubModeUndefined = 0x0ffff;

/***************************************************************************/
// CDlgPlayer::SetPlayerMode Set the player mode
/***************************************************************************/
BOOL CDlgPlayer::SetPlayerMode(UINT nMode, UINT nSubMode, BOOL bFullSize, BOOL bFnKey, SSpecific *pSpecific)
{
  TRACE(_T("SetPlayerMode %d %d %d %d\n"),(int)nMode,(int)nSubMode,(int)bFullSize,(int)bFnKey);

  nSubMode = nSubMode & 0x0ffff;
  // SDM 1.06.6U6
  if (m_bFnKeySetting && (!bFnKey ||(nSubMode != SubModeUndefined)|| !m_bRepeat))
  {
    // restore original player setting
    m_bFnKeySetting = FALSE;
    m_nVolume = m_nOldVolume;
    m_nSpeed = m_nOldSpeed;
    m_nDelay = m_nOldDelay;
    m_bRepeat = m_bOldRepeat;
    UpdateData(FALSE);
    OnKillfocusVolumeEdit();
    OnKillfocusSpeedEdit();
    OnKillfocusDelayEdit();
    OnRepeat();
    if (m_nFnKey == 24) // SDM 1.5Test10.4
      AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_PROCESS_BATCH_COMMANDS, 0L);// SDM 1.5Test8.5 resume batch command
  }
  // SDM 1.06.6U6 bFnKey not valid for nSubMode = SubModeUndefined (this is a repeat command or stop command)
  if (bFnKey && (nSubMode != SubModeUndefined))
  {
    // save the current player setting
    m_nOldVolume = m_nVolume;
    m_nOldSpeed = m_nSpeed;
    m_nOldDelay = m_nDelay;
    m_bOldRepeat = m_bRepeat;
    m_bFnKeySetting = TRUE;
    m_nFnKey = nSubMode; // SDM 1.5Test10.4
    // get the function key setting (Fn key number is in nSubMode)
    CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
    FnKeys* pFnKeys = pMainFrame->GetFnKeys(NULL);
    m_nVolume = pFnKeys->nVolume[nSubMode];
    m_nSpeed = pFnKeys->nSpeed[nSubMode];
    m_nDelay = pFnKeys->nDelay[nSubMode];
    m_bRepeat = pFnKeys->bRepeat[nSubMode];
    nSubMode = pFnKeys->nMode[nSubMode];
    switch(nSubMode)
    {
      case ID_PLAYBACK_CURSORS: m_nComboPlayMode = 0; break;
      case ID_PLAYBACK_LTOSTART: m_nComboPlayMode = 1; break;
      case ID_PLAYBACK_STARTTOR: m_nComboPlayMode = 2; break;
      case ID_PLAYBACK_LTOSTOP: m_nComboPlayMode = 3; break;
      case ID_PLAYBACK_STOPTOR: m_nComboPlayMode = 4; break;
      case ID_PLAYBACK_WINDOW: m_nComboPlayMode = 5; break;
      default: m_nComboPlayMode = 6; break;
    }
    UpdateData(FALSE);
    OnKillfocusVolumeEdit();
    OnKillfocusSpeedEdit();
    OnKillfocusDelayEdit();
    OnRepeat();
  }
  if (nSubMode != SubModeUndefined) m_nSubMode = nSubMode; // save submode
  // set mode combobox, but only if not called for full size
  if (!bFullSize)
  {
    switch(m_nSubMode)
    {
      case ID_PLAYBACK_CURSORS: m_nComboPlayMode = 0; break;
      case ID_PLAYBACK_LTOSTART: m_nComboPlayMode = 1; break;
      case ID_PLAYBACK_STARTTOR: m_nComboPlayMode = 2; break;
      case ID_PLAYBACK_LTOSTOP: m_nComboPlayMode = 3; break;
      case ID_PLAYBACK_STOPTOR: m_nComboPlayMode = 4; break;
      case ID_PLAYBACK_WINDOW: m_nComboPlayMode = 5; break;
      default: m_nComboPlayMode = 6; break;
    }
    UpdateData(FALSE);
  }
  if ((!m_bFullSize) && (bFullSize)) SetPlayerFullSize(); // change the size
  // No errors
  if (m_nMode == nMode) return TRUE; // no change
  // stop, whatever the player is doing
  if (m_pWave) m_pWave->Stop();
  SetTotalTime();
  SetPositionTime();
  DWORD dwSize, dwStart = 0;
  CSaApp* pApp = (CSaApp*)AfxGetApp(); // get pointer to application
  CSaView* pView = (CSaView*)m_pView;  // cast pointer to view
  CSaDoc* pDoc = (CSaDoc*)m_pDoc;  // cast pointer to document
  switch (nMode)
  {
    case IDC_PLAY:
      if (m_nMode != IDC_PAUSE) m_dwPlayPosition = 0;
      m_nMode = IDC_PLAY;
      m_play.Flash(FALSE); // stop flashing Play button
      m_play.Push(); // push Play button
      m_pause.Release(); // release Pause button
      m_pause.EnableWindow(TRUE); // enable Pause button
      GetDlgItem(IDC_PLAYMODE)->EnableWindow(FALSE); // disable mode window
      // get pointer to document
      if (m_pWave)
      {
        BOOL bError = FALSE;
        switch(m_nSubMode)
        {
          case ID_PLAYBACK_FILE:
            dwSize = pDoc->GetDataSize();
            break;
          case ID_PLAYBACK_WINDOW:
            dwStart = DWORD(pView->GetDataPosition(0));
            dwSize = pView->GetDataFrame();
            break;
          case ID_PLAYBACK_CURSORS:
            dwStart = pView->GetStartCursorPosition();
            dwSize = pView->GetStopCursorPosition() - dwStart;
            break;
          case ID_PLAYBACK_LTOSTART:
            dwStart = DWORD(pView->GetDataPosition(0));
            dwSize = pView->GetStartCursorPosition();
            if (dwSize > dwStart) dwSize -= dwStart;
            else bError = TRUE;
            break;
          case ID_PLAYBACK_STARTTOR:
            dwStart = pView->GetStartCursorPosition();
            dwSize = DWORD(pView->GetDataPosition(0) + pView->GetDataFrame());
            if (dwSize > dwStart) dwSize -= dwStart;
            else bError = TRUE;
            break;
          case ID_PLAYBACK_LTOSTOP:
            dwStart = DWORD(pView->GetDataPosition(0));
            dwSize = pView->GetStopCursorPosition();
            if (dwSize > dwStart) dwSize -= dwStart;
            else bError = TRUE;
            break;
          case ID_PLAYBACK_STOPTOR:
            dwStart = pView->GetStopCursorPosition();
            dwSize = DWORD(pView->GetDataPosition(0)) + pView->GetDataFrame();
            if (dwSize > dwStart) dwSize -= dwStart;
            else bError = TRUE;
            break;
          case ID_PLAYBACK_SPECIFIC:
            dwSize = pDoc->GetDataSize();
            if(pSpecific)
            {
              DWORD dwSpecificSize = pDoc->GetBytesFromTime(pSpecific->end);
              DWORD dwSpecificStart = pDoc->GetBytesFromTime(pSpecific->begin);

              if(dwSpecificStart < dwSize)
                dwStart = dwSpecificStart;

              if(dwSpecificSize < dwSize)
              {
                dwSize = dwSpecificSize;
                if (dwSize > dwStart) 
                  dwSize -= dwStart;
                else
                  dwSize = 0;
              }
            }
            break;
          default: dwStart = dwSize = 0; break;
        }
        if (m_dwPlayPosition == 0) m_dwPlayPosition = dwStart;
        if (bError) pApp->ErrorMessage(IDS_ERROR_PLAYMODE);  // play mode not playable
        else
        {
          SetDlgItemText(IDC_TIMERTEXT, _T("Current Time"));
          m_pView->SetPlaybackPosition(dwStart); // SDM 1.5Test10.5
          bError = !m_pWave->Play(m_dwPlayPosition, dwStart + dwSize - m_dwPlayPosition, m_nVolume, m_nSpeed, m_pView, &m_NotifyObj);
        }
        
        if (bError)
        {
          m_nMode = IDC_STOP;  // play not successfull
          m_play.Release(); // release Play button
          m_pause.EnableWindow(FALSE); // disable Pause button
          GetDlgItem(IDC_PLAYMODE)->EnableWindow(TRUE); // enable mode window
          m_VUBar.Reset();
          // An error has occurred
          return FALSE;
        }
      }
      break;
    case IDC_PAUSE:
      if (m_pWave) m_pWave->Stop();
      m_nOldMode = m_nMode;
      m_nMode = IDC_PAUSE;
      // start flashing paused button
      m_play.Flash(TRUE);
      m_VUBar.SetVU(0);
      break;
    case IDC_STOP:
      m_nMode = IDC_STOP;
      if (m_pWave) m_pWave->Stop();
      m_dwPlayPosition = 0;
      SetPositionTime();
      m_pView->SetPlaybackPosition(); // SDM 1.06.6U6 hide playback indicators
      m_play.Release(); // release Play button
      m_stop.Release(); // release Stop button
      m_pause.Release(); // release Pause button
      m_pause.EnableWindow(FALSE); // disable Pause button
      GetDlgItem(IDC_PLAYMODE)->EnableWindow(TRUE); // disable mode window
      m_VUBar.Reset();
      break;
    default:
      m_nMode = 0;
      if (m_pWave) m_pWave->Stop();
      m_play.Release(); // release Play button
      m_stop.Release(); // release Stop button
      m_pause.Release(); // release Pause button
      m_pause.EnableWindow(FALSE); // disable Pause button
      GetDlgItem(IDC_PLAYMODE)->EnableWindow(FALSE); // disable mode window
      m_LEDTotalTime.SetTime(100, 1000);
      m_LEDPosTime.SetTime(100, 1000);
      m_VUBar.Reset();
      break;
  }
  // No errors
  return TRUE;
}

/***************************************************************************/
// CDlgPlayer::BlockFinished Playing Block finished
// The actually playing data block has been finished playing. Update the
// dialog display controls.
/***************************************************************************/
void CDlgPlayer::BlockFinished(UINT nLevel, DWORD dwPosition, UINT nSpeed)
{
  m_pView->SetPlaybackPosition(dwPosition, nSpeed);
  // update the VU bar
  m_VUBar.SetVU((int)nLevel);
  // update the time
  m_dwPlayPosition = dwPosition;
  CSaDoc* pDoc = (CSaDoc*)m_pDoc;
  FmtParm* pFmtParm = pDoc->GetFmtParm();
  DWORD dwBlockEnd = m_dwPlayPosition - pFmtParm->wBlockAlign/pFmtParm->wChannels;
  double fDataSec = pDoc->GetTimeFromBytes(dwBlockEnd);
  m_LEDPosTime.SetTime((int)fDataSec / 60, (int)(fDataSec * 10) % 600);
}

/***************************************************************************/
// CDlgPlayer::EndPlayback Playback finished
// The playback has been finished. Set player mode to stop.
/***************************************************************************/
void CDlgPlayer::EndPlayback()
{
  if (m_nMode != 0)
  {
    SetPlayerMode(IDC_STOP, SubModeUndefined, m_bFullSize, m_bFnKeySetting);  // SDM 1.06.6U6
    if (m_bRepeat) SetTimer(1, m_nDelay, NULL); // start repeating
  }
}

/***************************************************************************/
// CDlgPlayer::ChangeView Change the parent view
// The parent view has changed. The dialogs caption text must be adapted and
// the playing time changes.
/***************************************************************************/
void CDlgPlayer::ChangeView(CSaView* pView)
{
  if(m_pView == pView)
    return;
  // set file name in players caption
  m_pView = pView; // set the new pointer to the active view
  m_pDoc = pView->GetDocument(); // get the pointer to the attached document
  CSaDoc* pDoc = (CSaDoc*)m_pDoc;
  CString szTitle = pDoc->GetTitle(); // load the document title
  int nFind = szTitle.Find(':');
  if (nFind != -1) szTitle = szTitle.Left(nFind); // extract part left of :
  CString szCaption;
  GetWindowText(szCaption); // get the current caption string
  nFind = szCaption.Find(' ');
  if (nFind != -1) szCaption = szCaption.Left(nFind); // extract part left of first space
  szCaption += " - " + szTitle; // add new document title
  SetWindowText(szCaption); // write the new caption string
  if (pDoc->GetDataSize() == 0) // no data to play
  {
    SetPlayerMode(0, m_nSubMode, m_bFullSize);
  }
  else
  {
    // update the time displays
    SetPlayerMode(IDC_STOP, m_nSubMode, m_bFullSize);
    SetTotalTime();
    SetPositionTime();
  }
}

/////////////////////////////////////////////////////////////////////////////
// CDlgPlayer message handlers

/***************************************************************************/
// CDlgPlayer::OnInitDialog Dialog initialization
// All the necessary special control windows have to be created and placed
// over their placeholders in the dialog. The dialog is centered over the
// mainframe window.
/***************************************************************************/
BOOL CDlgPlayer::OnInitDialog()
{
  CDialog::OnInitDialog();
  m_pWave->GetOutDevice()->ConnectMixer(this);
  // preset the player modes
  m_nMode = 0;
  m_nSubMode = ID_PLAYBACK_FILE; // default
  CRect rWnd;
  // build and place the play toggle button
  m_play.Init(IDC_PLAY, "Play", this);
  // build and place the stop toggle button
  m_stop.Init(IDC_STOP, "Stop", this);
  // build and place the pause toggle button
  m_pause.Init(IDC_PAUSE, "Pause", this);
  // build and place the total time LED window
  m_LEDTotalTime.Init(IDC_TOTALTIME, this);
  m_LEDTotalTime.SetTime(0, 0);
  // build and place the position time LED window
  m_LEDPosTime.Init(IDC_POSITIONTIME, this);
  m_LEDPosTime.SetTime(100, 600);
  // build and place the VUbar window
  m_VUBar.Init(IDC_VUBAR, this);
  m_VUBar.SetVU(0);
  // build and place the speed slider window
  m_SliderSpeed.Init(IDC_SPEEDSLIDER, this);
  m_SliderSpeed.SetPosition(50);
  // build and place the speed spin control
  m_SpinSpeed.Init(IDC_SPEEDSCROLL, this);
  // enable speed slider if fragmenter complete
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
  CSaDoc *pSaDoc = pMainWnd->GetCurrDoc();
  CProcessFragments* pFragmenter = pSaDoc->GetFragments();
  if (pFragmenter)
  {
    BOOL bState = pFragmenter->IsDataReady();
    GetDlgItem(IDC_SPEEDFRAME)->EnableWindow(bState);
    GetDlgItem(IDC_SPEEDEDIT)->EnableWindow(bState);
    GetDlgItem(IDC_SPEEDTEXT)->EnableWindow(bState);
    m_SliderSpeed.EnableWindow(bState);
    m_SpinSpeed.EnableWindow(bState);
  }
  // build and place the volume slider window
  m_SliderVolume.Init(IDC_VOLUMESLIDER, this);
  m_SliderVolume.SetPosition(m_nVolume);
  // build and place the volume spin control
  m_SpinVolume.Init(IDC_VOLUMESCROLL, this);
  // build and place the delay slider window
  m_SliderDelay.Init(IDC_DELAYSLIDER, this);
  m_SliderDelay.SetPosition(10);
  // build and place the delay spin control
  m_SpinDelay.Init(IDC_DELAYSCROLL, this);
  // disable delay editing
  m_SliderDelay.EnableWindow(FALSE); // disable delay slider
  m_SpinDelay.EnableWindow(FALSE); // disable delay spin control
  GetDlgItem(IDC_DELAYEDIT)->EnableWindow(FALSE); // disable delay edit control
  GetDlgItem(IDC_DELAYTEXT)->EnableWindow(FALSE); // disable delay dimension text
  // get pointer to view
  CMainFrame* pMainFrameWnd = (CMainFrame*)AfxGetMainWnd();
  CSaView* pView = (CSaView*)pMainFrameWnd->GetCurrSaView();
  ChangeView(pView); // setup for this view
  // get dialog sizes for small and full size views
  GetWindowRect(rWnd); // get screen coordinates of dialog
  m_rSize.right = rWnd.Width();
  m_rSize.bottom = rWnd.Height();
  m_rSize.left = 0;
  m_rSize.top = 0;
  CWnd* pWnd = GetDlgItem(IDC_MIXER);
  if(pWnd)
    pWnd->EnableWindow(m_pWave->GetOutDevice()->ShowMixer(FALSE));

  // hide dialog
  MoveWindow(rWnd.left, rWnd.top, 0, 0);
  return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************/
// CDlgPlayer::OnPlay Button play hit
/***************************************************************************/
void CDlgPlayer::OnPlay()
{
  UpdateData(TRUE);
  if (m_nMode != IDC_PLAY) SetPlayerMode(IDC_PLAY, m_nSubMode, m_bFullSize);
}

/***************************************************************************/
// CDlgPlayer::OnStop Button stop hit
/***************************************************************************/
void CDlgPlayer::OnStop()
{
  // RLJ, 1 Oct 1999
  // Added the following 'if' statement so that when the Stop button is hit,
  // the next Repeat is canceled. Since it leaves the 'Repeat' checkbox
  // unchanged, however, future repeats can still be scheduled simply by
  // depressing the Play button.
  if (m_bRepeat)
    m_bRepeat = FALSE;

  if (m_nMode != IDC_STOP) SetPlayerMode(IDC_STOP, m_nSubMode, m_bFullSize);
  else m_stop.Release();
}

/***************************************************************************/
// CDlgPlayer::OnPause Button pause hit
/***************************************************************************/
void CDlgPlayer::OnPause()
{
  if (m_nMode != IDC_PAUSE) SetPlayerMode(IDC_PAUSE, m_nSubMode, m_bFullSize);
  else SetPlayerMode(m_nOldMode, m_nSubMode, m_bFullSize);
}

/***************************************************************************/
// CDlgPlayer::OnMixerControlChange Mixer has changed volume settings
/***************************************************************************/
LRESULT CDlgPlayer::OnMixerControlChange(WPARAM, LPARAM)
{
  BOOL bResult = FALSE;
  
  m_nVolume = m_pWave->GetVolume(&bResult);

  if(bResult)
  {
    SetDlgItemInt(IDC_VOLUMEEDIT, m_nVolume, TRUE);
    m_SliderVolume.SetPosition(m_nVolume);
  }

  return 0;
}

/***************************************************************************/
// CDlgPlayer::OnVolumeSlide Volume slider position changed
/***************************************************************************/
void CDlgPlayer::OnVolumeSlide()
{
  m_nVolume = m_SliderVolume.GetPosition();
  SetDlgItemInt(IDC_VOLUMEEDIT, m_SliderVolume.GetPosition(), TRUE);
  m_pWave->SetVolume(m_nVolume);
}

/***************************************************************************/
// CDlgPlayer::OnVolumeScroll Volume spin control hit
/***************************************************************************/
void CDlgPlayer::OnVolumeScroll()
{
  m_nVolume = GetDlgItemInt(IDC_VOLUMEEDIT, NULL, TRUE);
  if (m_SpinVolume.UpperButtonClicked()) m_nVolume++;
  else m_nVolume--;
  if ((int)m_nVolume < 0) m_nVolume = 0;
  if (m_nVolume > 100) m_nVolume = 100;
  SetDlgItemInt(IDC_VOLUMEEDIT, m_nVolume, TRUE);
  m_SliderVolume.SetPosition(m_nVolume);
  m_pWave->SetVolume(m_nVolume);
}

/***************************************************************************/
// CDlgPlayer::OnKillfocusVolumeEdit Volume edited
/***************************************************************************/
void CDlgPlayer::OnKillfocusVolumeEdit()
{
  m_nVolume = GetDlgItemInt(IDC_VOLUMEEDIT, NULL, TRUE);
  if ((int)m_nVolume < 0) m_nVolume = 0;
  if (m_nVolume > 100) m_nVolume = 100;
  SetDlgItemInt(IDC_VOLUMEEDIT, m_nVolume, TRUE);
  m_SliderVolume.SetPosition(m_nVolume);
  m_pWave->SetVolume(m_nVolume);
}

/***************************************************************************/
// CDlgPlayer::EnableSpeedSlider Enable/Disable speed slider
/***************************************************************************/
void CDlgPlayer::EnableSpeedSlider(BOOL bState)
{
  GetDlgItem(IDC_SPEEDFRAME)->EnableWindow(bState);
  GetDlgItem(IDC_SPEEDEDIT)->EnableWindow(bState);
  GetDlgItem(IDC_SPEEDTEXT)->EnableWindow(bState);
  m_SliderSpeed.EnableWindow(bState);
  m_SliderSpeed.UpdateWindow();
  m_SpinSpeed.EnableWindow(bState);
  m_SpinSpeed.UpdateWindow();
}

/***************************************************************************/
// CDlgPlayer::OnSpeedSlide Speed slider position changed
/***************************************************************************/
void CDlgPlayer::OnSpeedSlide()
{
  UINT nSpeed = m_SliderSpeed.GetPosition();
  if (nSpeed > 50) m_nSpeed = 100 + (nSpeed - 50) * 233 / 50;
  else m_nSpeed = 10 + nSpeed * 90 / 50;
  SetDlgItemInt(IDC_SPEEDEDIT, m_nSpeed, TRUE);
  m_pWave->SetSpeed(m_nSpeed);
}

/***************************************************************************/
// CDlgPlayer::OnSpeedScroll Speed spin control hit
/***************************************************************************/
void CDlgPlayer::OnSpeedScroll()
{
  m_nSpeed = GetDlgItemInt(IDC_SPEEDEDIT, NULL, TRUE);
  if (m_SpinSpeed.UpperButtonClicked()) m_nSpeed++;
  else m_nSpeed--;
  if ((int)m_nSpeed < 10) m_nSpeed = 10;
  if (m_nSpeed > 333) m_nSpeed = 333;
  SetDlgItemInt(IDC_SPEEDEDIT, m_nSpeed, TRUE);
  if (m_nSpeed > 100) m_SliderSpeed.SetPosition(50 + 50 * (m_nSpeed - 100) / 233);
  else m_SliderSpeed.SetPosition(50 * (m_nSpeed - 10) / 90);
  m_pWave->SetSpeed(m_nSpeed);
}

/***************************************************************************/
// CDlgPlayer::OnKillfocusSpeedEdit Speed edited
/***************************************************************************/
void CDlgPlayer::OnKillfocusSpeedEdit()
{
  m_nSpeed = GetDlgItemInt(IDC_SPEEDEDIT, NULL, TRUE);
  if ((int)m_nSpeed < 10) m_nSpeed = 10;
  if (m_nSpeed > 333) m_nSpeed = 333;
  SetDlgItemInt(IDC_SPEEDEDIT, m_nSpeed, TRUE);
  if (m_nSpeed > 100) m_SliderSpeed.SetPosition(50 + 50 * (m_nSpeed - 100) / 233);
  else m_SliderSpeed.SetPosition(50 * (m_nSpeed - 10) / 90);
  m_pWave->SetSpeed(m_nSpeed);
}

/***************************************************************************/
// CDlgPlayer::OnDelaySlide Delay slider position changed
/***************************************************************************/
void CDlgPlayer::OnDelaySlide()
{
  m_nDelay = m_SliderDelay.GetPosition() * 100;
  if (m_nDelay > 9999) m_nDelay = 9999;
  SetDlgItemInt(IDC_DELAYEDIT, m_nDelay, TRUE);
}

/***************************************************************************/
// CDlgPlayer::OnDelayScroll Delay spin control hit
/***************************************************************************/
void CDlgPlayer::OnDelayScroll()
{
  m_nDelay = GetDlgItemInt(IDC_DELAYEDIT, NULL, TRUE);
  if (m_SpinDelay.UpperButtonClicked()) m_nDelay += 100;
  else
  {
    if (m_nDelay == 9999) m_nDelay -= 99;
    else
    {
      if (m_nDelay > 100) m_nDelay -= 100;
      else m_nDelay = 0;
    }
  }
  if (m_nDelay > 9999) m_nDelay = 9999;
  SetDlgItemInt(IDC_DELAYEDIT, m_nDelay, TRUE);
  if (m_nDelay == 9999) m_SliderDelay.SetPosition(100);
  else m_SliderDelay.SetPosition(m_nDelay / 100);
}

/***************************************************************************/
// CDlgPlayer::OnKillfocusDelayEdit Delay edited
/***************************************************************************/
void CDlgPlayer::OnKillfocusDelayEdit()
{
  m_nDelay = GetDlgItemInt(IDC_DELAYEDIT, NULL, TRUE);
  if ((int)m_nDelay < 0) m_nDelay = 0;
  if (m_nDelay > 9999) m_nDelay = 9999;
  SetDlgItemInt(IDC_DELAYEDIT, m_nDelay, TRUE);
  if (m_nDelay == 9999) m_SliderDelay.SetPosition(100);
  else m_SliderDelay.SetPosition(m_nDelay / 100);
}

/***************************************************************************/
// CDlgPlayer::OnRepeat Replay repeat clicked
/***************************************************************************/
void CDlgPlayer::OnRepeat()
{
  UpdateData(TRUE);
  if (m_bRepeat)
  {
    // enable delay editing
    m_SliderDelay.EnableWindow(TRUE); // enable delay slider
    m_SpinDelay.EnableWindow(TRUE); // enable delay spin control
    GetDlgItem(IDC_DELAYEDIT)->EnableWindow(TRUE); // enable delay edit control
    GetDlgItem(IDC_DELAYTEXT)->EnableWindow(TRUE); // enable delay dimension text
  }
  else
  {
    // disable delay editing
    m_SliderDelay.EnableWindow(FALSE); // disable delay slider
    m_SpinDelay.EnableWindow(FALSE); // disable delay spin control
    GetDlgItem(IDC_DELAYEDIT)->EnableWindow(FALSE); // disable delay edit control
    GetDlgItem(IDC_DELAYTEXT)->EnableWindow(FALSE); // disable delay dimension text
  }
}

/***************************************************************************/
// CDlgPlayer::OnSelchangePlaymode Player mode selection changed
/***************************************************************************/
void CDlgPlayer::OnSelchangePlaymode()
{
  UpdateData(TRUE);
  UINT nSubMode = ID_PLAYBACK_FILE;
  switch(m_nComboPlayMode)
  {
    case 0: nSubMode = ID_PLAYBACK_CURSORS; break;
    case 1: nSubMode = ID_PLAYBACK_LTOSTART; break;
    case 2: nSubMode = ID_PLAYBACK_STARTTOR; break;
    case 3: nSubMode = ID_PLAYBACK_LTOSTOP; break;
    case 4: nSubMode = ID_PLAYBACK_STOPTOR; break;
    case 5: nSubMode = ID_PLAYBACK_WINDOW; break;
    case 6: nSubMode = ID_PLAYBACK_FILE; break;
    default: break;
  }
  SetPlayerMode(IDC_STOP, nSubMode, m_bFullSize);
  SetTotalTime();
  SetPositionTime();
}

/***************************************************************************/
// CDlgPlayer::OnOK Button ok hit
// If the focus is on the editable controls, just change the focus to the
// next control, otherwise close the dialog with ok.
/***************************************************************************/
void CDlgPlayer::OnOK()
{
  CWnd* pWnd = GetFocus(); // who has the focus?
  if ((pWnd->GetDlgCtrlID() == IDC_VOLUMEEDIT) || (pWnd->GetDlgCtrlID() == IDC_SPEEDEDIT))
    NextDlgCtrl(); // focus is on edit control, so move to the next control on enter
  else
  {
    OnClose();
  }
}

/***************************************************************************/
// CDlgPlayer::OnCancel Button cancel hit
/***************************************************************************/
void CDlgPlayer::OnCancel()
{
  if(IsPlaying())
    OnStop();
  else
    OnClose();
}

/***************************************************************************/
// CDlgPlayer::OnClose Close the dialog
/***************************************************************************/
void CDlgPlayer::OnClose()
{
  OnStop(); // stop the player
  CDialog::OnClose();
  bPlayer = FALSE;
  if(m_pWave) delete m_pWave;
  m_pWave = NULL;
  DestroyWindow();
}

/***************************************************************************/
// CDlgPlayer::OnTimer Timer event, repeat playback
/***************************************************************************/
void CDlgPlayer::OnTimer(UINT nIDEvent)
{
  KillTimer(1);
  if ((m_nMode == IDC_STOP) && m_bRepeat)
    SetPlayerMode(IDC_PLAY, SubModeUndefined, m_bFullSize, m_bFnKeySetting); // SDM 1.06.6U6
  CWnd::OnTimer(nIDEvent);
}

/***************************************************************************/
// CDlgPlayer::OnSetupFnKeys Setup function keys
/***************************************************************************/
LRESULT CDlgPlayer::OnSetupFnKeys(WPARAM /*wParam*/, LPARAM)
{
  OnSetup();
  return 0;
}

/***************************************************************************/
// CDlgPlayer::OnSetup Button setup hit
/***************************************************************************/
void CDlgPlayer::OnSetup()
{
  if (m_pWave) m_pWave->Stop(); // stop recording
  SetPlayerMode(IDC_STOP, m_nSubMode, m_bFullSize);
  m_VUBar.SetVU(0); // reset the VU bar
  // create the dialog
  m_bTestRunning = TRUE;
  CDlgFnKeys* pDlgFnKeys = new CDlgFnKeys;
  pDlgFnKeys->m_pView = m_pView;
  pDlgFnKeys->m_pDoc = m_pDoc;
  pDlgFnKeys->DoModal();
  delete pDlgFnKeys;
  m_bTestRunning = FALSE;
}

/***************************************************************************/
// CDlgPlayer::OnPlayControls Show Volume Controls mixer
/***************************************************************************/
void CDlgPlayer::OnPlayControls() 
{
  m_pWave->GetOutDevice()->ConnectMixer(this);

  m_pWave->GetOutDevice()->ShowMixer();	
}

/***************************************************************************/
// CDlgPlayer::OnHelpPlayer Call Player help
/***************************************************************************/
void CDlgPlayer::OnHelpPlayer()
{
	// create the pathname
	CString szPath = AfxGetApp()->m_pszHelpFilePath;
	szPath += "::/User_Interface/Menus/Playback/Player.htm";
	::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

//###########################################################################
// CDlgRecorder dialog
// Displays the recorder that allows the user to control the wave data
// recording process, done by the CWave class.

/////////////////////////////////////////////////////////////////////////////
// CDlgRecorder message map

BEGIN_MESSAGE_MAP(CDlgRecorder, CDlgAudio)
  //{{AFX_MSG_MAP(CDlgRecorder)
  ON_COMMAND(IDC_RECORD, OnRecord)
  ON_COMMAND(IDC_PLAY, OnPlay)
  ON_COMMAND(IDC_STOP, OnStop)
  ON_COMMAND(IDC_PAUSE, OnPause)
  ON_WM_CLOSE()
  ON_BN_CLICKED(ID_CLOSE, OnButtonClose)
  ON_BN_CLICKED(IDC_SETTINGS, OnSettings)
  ON_BN_CLICKED(IDC_APPLY, OnApply)
  ON_COMMAND(IDC_VOLUMESLIDER, OnVolumeSlide)
  ON_COMMAND(IDC_VOLUMESCROLL, OnVolumeScroll)
  ON_EN_KILLFOCUS(IDC_VOLUMEEDIT, OnKillfocusVolumeEdit)
  ON_COMMAND(IDC_RECVOLUMESLIDER, OnRecVolumeSlide)
  ON_COMMAND(IDC_RECVOLUMESCROLL, OnRecVolumeScroll)
  ON_EN_KILLFOCUS(IDC_RECVOLUMEEDIT, OnKillfocusRecVolumeEdit)
  ON_MESSAGE(MM_MIXM_CONTROL_CHANGE, OnMixerControlChange)
	ON_BN_CLICKED(IDC_MIXER, OnMixer)
	ON_COMMAND(IDHELP, OnHelpRecorder)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgRecorder construction/destruction/creation

/***************************************************************************/
// CDlgRecorder::CDlgRecorder Constructor
/***************************************************************************/
CDlgRecorder::CDlgRecorder(CWnd* pParent) : CDlgAudio(CDlgRecorder::IDD, pParent)
{
  //{{AFX_DATA_INIT(CDlgRecorder)
  //}}AFX_DATA_INIT
  m_hmmioFile = NULL;
  m_szFileName[0] = 0; // no file name
  m_pDoc = NULL;
  m_pView = NULL;
  m_NotifyObj.Attach(this); // attach notify object
  m_pWave = new CWave;      // create CWave object
  m_nVolume = m_pWave->GetVolume();
  m_nRecVolume = m_pWave->GetInDevice()->GetVolume();
  m_dwRecordSize = 0;
  m_dwPlayPosition = 0;
  m_bFileReady = TRUE;
  m_bFileApplied = FALSE;
  // allocate wave data buffer
  CSaApp* pApp = (CSaApp*)AfxGetApp(); // get pointer to application
  m_hData = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, MMIO_BUFFER_SIZE); // allocate memory
  if (!m_hData)
  {
    // memory allocation error
    pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
    return;
  }
  m_lpData = (HPSTR)::GlobalLock(m_hData); // lock memory
  if (!m_lpData)
  {
    // memory lock error
    pApp->ErrorMessage(IDS_ERROR_MEMLOCK);
    ::GlobalFree(m_hData);
    return;
  }
}

/////////////////////////////////////////////////////////////////////////////
// CDlgRecorder helper functions

/***************************************************************************/
// CDlgRecorder::DoDataExchange Data exchange
/***************************************************************************/
void CDlgRecorder::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CDlgRecorder)
  DDX_Text(pDX, IDC_VOLUMEEDIT, m_nVolume);
  DDX_Text(pDX, IDC_RECVOLUMEEDIT, m_nRecVolume);
  //}}AFX_DATA_MAP
}

/***************************************************************************/
// CDlgRecorder::SetTotalTime Set total time display
/***************************************************************************/
void CDlgRecorder::SetTotalTime()
{
  CSaDoc* pDoc = (CSaDoc*)m_pDoc; // cast pointer to document
  double fDataSec = pDoc->GetTimeFromBytes(m_dwRecordSize); // calculate time
  m_LEDTotalTime.SetTime((int)fDataSec / 60, (int)(fDataSec * 10) % 600);
}

/***************************************************************************/
// CDlgRecorder::SetPositionTime Set position time display
/***************************************************************************/
void CDlgRecorder::SetPositionTime()
{
  CSaDoc* pDoc = (CSaDoc*)m_pDoc; // cast pointer to document
  if ((m_nMode == IDC_RECORD) || ((m_nMode == IDC_PAUSE) && (m_nOldMode == IDC_RECORD)))
  {
    double fDataSec = pDoc->GetTimeFromBytes(m_dwRecordSize); // calculate time
    m_LEDPosTime.SetTime((int)fDataSec / 60, (int)(fDataSec * 10) % 600);
  }
  else
  {
    double fDataSec = pDoc->GetTimeFromBytes(m_dwPlayPosition); // calculate time
    m_LEDPosTime.SetTime((int)fDataSec / 60, (int)(fDataSec * 10) % 600);
  }
}

/***************************************************************************/
// CDlgRecorder::SetSettingsText Set settings text
/***************************************************************************/
void CDlgRecorder::SetSettingsText()
{
  CSaDoc* pDoc = (CSaDoc*)m_pDoc; // cast pointer to document
  // get data format parameters
  FmtParm* pFmtParm = pDoc->GetFmtParm();
  // get sa parameters
  SaParm saParm;
  pDoc->GetSaParm(&saParm);

  TCHAR szBuffer[60];
  _stprintf(szBuffer, _T("%u Hz"), pFmtParm->dwSamplesPerSec);
  if (pFmtParm->wChannels > 1)
    _stprintf(szBuffer, _T("%s, %u-bit\nStereo"), szBuffer, pFmtParm->wBitsPerSample);
  else
    _stprintf(szBuffer, _T("%s, %u-bit\nMono"), szBuffer, pFmtParm->wBitsPerSample);
  if (saParm.wFlags & SA_FLAG_HIGHPASS)
    _stprintf(szBuffer, _T("%s, Highpass"), szBuffer);
  if (GetStaticSourceInfo().bEnable)
    _stprintf(szBuffer, _T("%s\nAdd file information"), szBuffer);
  SetDlgItemText(IDC_SETTINGSTEXT, szBuffer);
}

/***************************************************************************/
// CDlgRecorder::BlockStored Recording Block finished
// The actually recording data block has been finished. Update the dialog
// display controls.
/***************************************************************************/
void CDlgRecorder::BlockStored(UINT nLevel, DWORD dwPosition, BOOL *)
{
  // update the VU bar
  m_VUBar.SetVU((int)nLevel);
  // save the position
  if (m_nMode == IDC_RECORD)
  {
    m_dwRecordSize = dwPosition;
    // update the time
    CSaDoc* pDoc = (CSaDoc*)m_pDoc;
    double fDataSec = pDoc->GetTimeFromBytes(m_dwRecordSize); // get sampled data size in seconds
    m_LEDPosTime.SetTime((int)fDataSec / 60, (int)(fDataSec * 10) % 600);
  }
}

/***************************************************************************/
// CDlgRecorder::StoreFailed Recorded block has been failed storing
/***************************************************************************/
void CDlgRecorder::StoreFailed()
{
  // stop the recorder
  SetRecorderMode(IDC_STOP);
  // inform the user
  CSaApp* pApp = (CSaApp*)AfxGetApp();
  pApp->ErrorMessage(IDS_ERROR_STOREWAVEBLOCK, m_szFileName);
}

/***************************************************************************/
// CDlgRecorder::BlockFinished Playing Block finished
// The actually playing data block has been finished playing. Update the
// dialog display controls.
/***************************************************************************/
void CDlgRecorder::BlockFinished(UINT nLevel, DWORD dwPosition, UINT)
{
  m_dwPlayPosition = dwPosition;
  // update the VU bar
  m_VUBar.SetVU((int)nLevel);
  // update the time
  CSaDoc* pDoc = (CSaDoc*)m_pDoc;
  double fDataSec = pDoc->GetTimeFromBytes(dwPosition); // get sampled data size in seconds
  m_LEDPosTime.SetTime((int)fDataSec / 60, (int)(fDataSec * 10) % 600);
}

/***************************************************************************/
// CDlgRecorder::EndPlayback Playback finished
// The playback has been finished. Set recorder mode to stop.
/***************************************************************************/
void CDlgRecorder::EndPlayback()
{
  if (m_nMode != 0)
  {
    SetRecorderMode(IDC_STOP);
  }
}

/***************************************************************************/
// CDlgRecorder::GetWaveData Request for next wave data block to play
// This function delivers the next wave data block to play to the CWave
// object from the recorder internal (just recorded) wave file. The file is
// always open, the file pointer is already in the data subchunk and the
// function just delivers the block requested.
/***************************************************************************/
HPSTR CDlgRecorder::GetWaveData(DWORD dwPlayPosition, DWORD dwDataSize)
{
  // find the right position in the data
  if (mmioSeek(m_hmmioFile, dwPlayPosition + 40, SEEK_SET) == -1) return NULL;
  // read the waveform data block
  if (mmioRead(m_hmmioFile, (HPSTR)m_lpData, dwDataSize) == -1) return NULL;
  return m_lpData;
}

/***************************************************************************/
// CWave::HighPassFilter Filter recorded wave file
// If the highpass flag is set, the sample data in the wave file is
// filtered, eliminating frequency components below 70 Hz.  Assumes file
// is already open and positioned at the end of the data chunk.
/***************************************************************************/
void CDlgRecorder::HighPassFilter()
{
  CSaApp* pApp = (CSaApp*)AfxGetApp(); // get pointer to application

  // get out of 'data' chunk
  if (mmioAscend(m_hmmioFile, &m_mmckinfoSubchunk, 0))
  {
    // error writing data chunk
    pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, m_szFileName);
    m_bFileReady = FALSE;
    return;
  }

  // get data sizes
  FmtParm* pFmtParm = ((CSaDoc *)m_pDoc)->GetFmtParm();
  UINT wSmpSize = pFmtParm->wBlockAlign / pFmtParm->wChannels;   // sample size in bytes
  DWORD dwRecordingSize = m_mmckinfoSubchunk.cksize;             // recording size in bytes

  // get out of 'RIFF' chunk, to write RIFF size
  if (mmioAscend(m_hmmioFile, &m_mmckinfoParent, 0))
  {
    // error writing RIFF chunk
    pApp->ErrorMessage(IDS_ERROR_WRITERIFFCHUNK, m_szFileName);
    m_bFileReady = FALSE;
    return;
  }

  // set file pointer to begin of file
  mmioSeek(m_hmmioFile, 0, SEEK_SET);

  // descend into 'RIFF' chunk again
  if (mmioDescend(m_hmmioFile, &m_mmckinfoParent, NULL, MMIO_FINDRIFF))
  {
    // error descending into RIFF chunk
    pApp->ErrorMessage(IDS_ERROR_WAVECHUNK, m_szFileName);
    m_bFileReady = FALSE;
    return;
  }

  // descend into 'data' chunk
  if (mmioDescend(m_hmmioFile, &m_mmckinfoSubchunk, &m_mmckinfoParent, MMIO_FINDCHUNK))
  {
    // error descending into data chunk
    pApp->ErrorMessage(IDS_ERROR_READDATACHUNK, m_szFileName);
    m_bFileReady = FALSE;
    return;
  }

  // filter recording backwards-in-time
  CWaveInDevice *pRecorder = m_pWave->GetInDevice();
  CHighPassFilter70 *pHighPassFilter = pRecorder->GetHighPassFilter();      //!!check if NULL?

  DWORD dwDataOffset = dwRecordingSize;
  mmioSeek(m_hmmioFile, (long)dwDataOffset, SEEK_CUR);   // go to end of data chunk
  long lDataSize = MMIO_BUFFER_SIZE;   //!!buffer size assumed to be even

  do
  {
    if (dwDataOffset < MMIO_BUFFER_SIZE) lDataSize = dwDataOffset;  // last block less than buffer size
    dwDataOffset -= (DWORD)lDataSize;
    mmioSeek(m_hmmioFile, -lDataSize, SEEK_CUR);                    // move to start of block
    if (mmioRead(m_hmmioFile, m_lpData, lDataSize) == -1)           // load it
    {
      // error
      pApp->ErrorMessage(IDS_ERROR_READDATACHUNK);
      m_bFileReady = FALSE;
      return;
    }

    if (wSmpSize == 1)
    {
      // 8-bit unsigned
      pHighPassFilter->BackwardPass((unsigned char *)m_lpData, (unsigned long)lDataSize);    // filter backwards
      //!!ck. return code?
      //UINT nMaxLevel = max(abs(pHighPassFilter->GetMax()-128), abs(pHighPassFilter->GetMin()-128));  // get max level for block
      //m_pWave->SetMaxLevel(100*(long)nMaxLevel/128);                                            // set max level
    }
    else
    {
      // 16-bit signed
      pHighPassFilter->BackwardPass((short *)m_lpData, (unsigned long)lDataSize/wSmpSize);   // filter backwards
      //!!ck. return code?
      //UINT nMaxLevel = max(abs(pHighPassFilter->GetMax()), abs(pHighPassFilter->GetMin()));     // get max level for block
      //m_pWave->SetMaxLevel(100*(long)nMaxLevel/32768);                                          // set max level
    }

    mmioSeek(m_hmmioFile, -lDataSize, SEEK_CUR);                    // return to start of block
    if (mmioWrite(m_hmmioFile, m_lpData, lDataSize) == -1)          // write filtered data
    {
      // error
      pApp->ErrorMessage(IDS_ERROR_RECHPFILTER);
      m_bFileReady = FALSE;
    }
    mmioSeek(m_hmmioFile, -lDataSize, SEEK_CUR);                    // return to start of block
  }while(dwDataOffset > 0);

  pRecorder->DetachHighPassFilter();                                 // done, remove filter

  // ascend out of data chunk
  if (mmioAscend(m_hmmioFile, &m_mmckinfoSubchunk, 0))
  {
    // error writing data chunk
    pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, m_szFileName);
    m_bFileReady = FALSE;
    return;
  }
}

/***************************************************************************/
// CDlgRecorder::SetRecorderMode Set the recorder mode
/***************************************************************************/
void CDlgRecorder::SetRecorderMode(UINT nMode)
{
  if ((m_nMode == nMode) && (m_nMode != IDC_STOP)) return; // no change
  // stop, whatever the recorder is doing
  if (m_pWave) m_pWave->Stop();
  SetTotalTime();
  SetPositionTime();
  CSaDoc* pDoc = (CSaDoc*)m_pDoc;  // cast pointer to document
  switch (nMode)
  {
    case IDC_RECORD:
      GetDlgItem(IDC_APPLY)->EnableWindow(FALSE); // disable apply button
      // reset the file pointer
      if (m_nMode == IDC_STOP)
      {
        DeleteTempFile(); // delete old temporary mmio file
        m_bFileReady = CreateTempFile(); // create new temporary mmio file
        m_dwRecordSize = 0;
        if (!m_bFileReady) break;
      }
      // stop monitoring
      if ((m_nMode == IDC_STOP) || (m_nMode == IDC_PAUSE))
      {
        if (m_pWave) m_pWave->Stop();
      }
      // enable/disable the buttons for recording
      m_nMode = IDC_RECORD;
      m_record.Flash(FALSE); // stop flashing Record button
      m_play.Release(); // release Play button
      m_pause.Release(); // release Pause button
      m_play.EnableWindow(FALSE); // disable Play button
      m_pause.EnableWindow(TRUE); // enable Pause button
      GetDlgItem(IDC_SETTINGS)->EnableWindow(FALSE); // disable settings button
      // start or continue recording
      if (m_pWave)
      {
        if (!m_pWave->Record(m_hmmioFile, m_pView, m_dwRecordSize, &m_NotifyObj)) // record
        {
          m_nMode = IDC_STOP;  // record not successfull
          m_record.Release(); // release Record button
          m_pause.EnableWindow(FALSE); // disable Pause button
          GetDlgItem(IDC_SETTINGS)->EnableWindow(TRUE); // enable settings button
        }
      }
      break;
    case IDC_PLAY:
      m_nMode = IDC_PLAY;
      m_play.Flash(FALSE); // stop flashing Play button
      m_play.Push(); // push Play button
      m_record.Release(); // release Record button
      m_pause.Release(); // release Pause button
      m_record.EnableWindow(FALSE); // disable Record button
      m_pause.EnableWindow(TRUE); // enable Pause button
      GetDlgItem(IDC_SETTINGS)->EnableWindow(FALSE); // disable settings button
      GetDlgItem(IDC_APPLY)->EnableWindow(FALSE); // disable apply button
      // play back the recorded file
      if (!m_pWave->Play(m_dwPlayPosition, m_dwRecordSize - m_dwPlayPosition, m_nVolume, 100, m_pView, &m_NotifyObj))
      {
        m_nMode = IDC_STOP;  // play not successfull
        m_play.Release(); // release Play button
        m_pause.EnableWindow(FALSE); // disable Pause button
        GetDlgItem(IDC_SETTINGS)->EnableWindow(TRUE); // enable settings button
      }
      break;
    case IDC_PAUSE:
      if (m_pWave) m_pWave->Stop();
      m_nOldMode = m_nMode;
      m_nMode = IDC_PAUSE;
      // start flashing paused buttons
      if (m_nOldMode == IDC_RECORD) m_record.Flash(TRUE);
      else m_play.Flash(TRUE);
      // start monitoring again
      if (!m_pWave->Monitor(m_pView, &m_NotifyObj)) // monitor
      {
        m_nMode = IDC_STOP;  // monitor not successfull
        m_record.Release(); // release Record button
        m_play.Release(); // release Play button
        m_stop.Release(); // release Stop button
        m_pause.Release(); // release Pause button
      }
      m_VUBar.SetVU(0);
      break;
    case IDC_STOP:
      if ((m_nMode == IDC_RECORD) || (m_nMode == IDC_PLAY) || (m_nMode == IDC_PAUSE))
      {
        if (m_pWave) m_pWave->Stop();
        if ((m_nMode == IDC_RECORD) || ((m_nMode == IDC_PAUSE) && (m_nOldMode == IDC_RECORD)))
          if (pDoc->GetSaParm()->wFlags & SA_FLAG_HIGHPASS) HighPassFilter();
      }
      m_nMode = IDC_STOP;
      m_dwPlayPosition = 0;
      SetPositionTime();
      m_record.Release(); // release Record button
      m_play.Release(); // release Play button
      m_stop.Release(); // release Stop button
      m_pause.Release(); // release Pause button
      m_record.EnableWindow(TRUE); // enable Record button
      m_play.EnableWindow(TRUE); // enable Play button
      m_pause.EnableWindow(FALSE); // disable Pause button
      GetDlgItem(IDC_SETTINGS)->EnableWindow(TRUE); // enable settings button
      if (m_dwRecordSize > 0)
      {
        m_play.EnableWindow(TRUE); // enable Play button
        GetDlgItem(IDC_APPLY)->EnableWindow(TRUE); // enable apply button
      }
      else
      {
        m_play.EnableWindow(FALSE); // disable Play button
        GetDlgItem(IDC_APPLY)->EnableWindow(FALSE); // disable apply button
      }
      if (!m_pWave->Monitor(m_pView, &m_NotifyObj)) // monitor
      {
        m_nMode = IDC_STOP;  // monitor not successfull
      }
      m_VUBar.SetVU(0);
      break;
    default:
      m_nMode = 0;
      m_record.Release(); // release Record button
      m_play.Release(); // release Play button
      m_stop.Release(); // release Stop button
      m_pause.Release(); // release Pause button
      m_record.EnableWindow(TRUE); // enable Record button
      m_play.EnableWindow(TRUE); // enable Play button
      m_pause.EnableWindow(FALSE); // disable Pause button
      GetDlgItem(IDC_SETTINGS)->EnableWindow(TRUE); // enable settings button
      GetDlgItem(IDC_APPLY)->EnableWindow(FALSE); // disable apply button
      if (m_pWave) m_pWave->Stop();
      m_LEDTotalTime.SetTime(100, 1000);
      m_LEDPosTime.SetTime(100, 1000);
      m_VUBar.SetVU(0);
      break;
    }
    // display pending error messages
    CSaApp* pApp = (CSaApp*)AfxGetApp(); // get pointer to application
    pApp->DisplayErrorMessage();
}

/***************************************************************************/
// CDlgRecorder::CreateTempFile Create the temporary wave file
// Creates the temporary wave file, opens it and creates the RIFF chunk and
// the Data subchunk. The file stays open with the file pointer ready
// to write.
/***************************************************************************/
BOOL CDlgRecorder::CreateTempFile()
{
  // create the temporary file
  TCHAR lpszTempPath[_MAX_PATH];
  GetTempPath(_MAX_PATH, lpszTempPath);
  GetTempFileName(lpszTempPath, _T("WAV"), 0, m_szFileName);
  // create and open the file
  CSaApp* pApp = (CSaApp*)AfxGetApp();
  m_hmmioFile = mmioOpen(m_szFileName, NULL, MMIO_CREATE | MMIO_READWRITE | MMIO_EXCLUSIVE);
  if (!(m_hmmioFile))
  {
    // error opening file
    pApp->ErrorMessage(IDS_ERROR_FILEOPEN, m_szFileName);
    return FALSE;
  }
  // create a 'RIFF' chunk with a 'WAVE' form type
  m_mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); // prepare search code
  // set chunk size
  m_mmckinfoParent.cksize = 0;
  if (mmioCreateChunk(m_hmmioFile, &m_mmckinfoParent, MMIO_CREATERIFF)) // create the 'RIFF' chunk
  {
    // error creating RIFF chunk
    pApp->ErrorMessage(IDS_ERROR_WRITERIFFCHUNK, m_szFileName);
    return FALSE;
  }
  // create the 'fmt ' subchunk
  m_mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
  // set chunk size
  m_mmckinfoSubchunk.cksize = 16;
  if (mmioCreateChunk(m_hmmioFile, &m_mmckinfoSubchunk, 0)) // create the 'data' chunk
  {
    // error creating format chunk
    pApp->ErrorMessage(IDS_ERROR_WRITEFORMATCHUNK, m_szFileName);
    return FALSE;
  }
  // write data into 'fmt ' chunk
  if (mmioWrite(m_hmmioFile, (HPSTR)m_szFileName, 16) == -1) // fill up fmt chunk
  {
    // error writing format chunk
    pApp->ErrorMessage(IDS_ERROR_WRITEFORMATCHUNK, m_szFileName);
    return FALSE;
  }
  // get out of 'fmt ' chunk
  if (mmioAscend(m_hmmioFile, &m_mmckinfoSubchunk, 0))
  {
    // error writing format chunk
    pApp->ErrorMessage(IDS_ERROR_WRITEFORMATCHUNK, m_szFileName);
    return FALSE;
  }
  // create the 'data' subchunk
  m_mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
  // set chunk size
  m_mmckinfoSubchunk.cksize = 0;
  if (mmioCreateChunk(m_hmmioFile, &m_mmckinfoSubchunk, 0)) // create the 'data' chunk
  {
    // error creating data chunk
    pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, m_szFileName);
    return FALSE;
  }
  return TRUE;
}

/***************************************************************************/
// CDlgRecorder::DeleteTempFile Close and delete the temporary wave file
/***************************************************************************/
void CDlgRecorder::DeleteTempFile()
{
  if (m_szFileName[0] != 0)
  {
    // close and delete the temporary wave file
    if (m_hmmioFile) mmioClose(m_hmmioFile, 0);
    try
    {
      CFile::Remove(m_szFileName);
      m_szFileName[0] = 0;
    }
    catch (CFileException e)
    {
      // error deleting file
      CSaApp* pApp = (CSaApp*)AfxGetApp();
      pApp->ErrorMessage(IDS_ERROR_DELTEMPFILE, m_szFileName);
    }
  }
}

/***************************************************************************/
// CDlgRecorder::CleanUp Clean up memory and delete the temporary file
// If the user allows closing, the function returns TRUE, else FALSE.
/***************************************************************************/
void CDlgRecorder::CleanUp()
{
  if (m_pWave)
  {
    m_pWave->Stop();
    delete m_pWave; // delete the CWave object
		m_pWave = NULL;
  }
  DeleteTempFile();
  if (m_hData)
  {
    ::GlobalUnlock(m_hData);
    ::GlobalFree(m_hData);
  }
}

/***************************************************************************/
// CDlgRecorder::CloseRecorder Close the recorder
// If the user allows closing, the function returns TRUE, else FALSE.
/***************************************************************************/
BOOL CDlgRecorder::CloseRecorder()
{
  if (m_nMode != IDC_STOP) SetRecorderMode(IDC_STOP); // stop recording
  m_VUBar.SetVU(0); // reset the VU bar
  if ((m_bFileReady && !m_bFileApplied) && (m_dwRecordSize > 0))
  {
    // temporary data not applied, ask user
    int nResponse = AfxMessageBox(IDS_QUESTION_APPLYRECORD, MB_YESNOCANCEL | MB_ICONQUESTION, 0);
    if (nResponse == IDYES)
    {
      // apply the data
      OnApply();
      return FALSE;
    }
    if (nResponse == IDCANCEL)
    {
      // continue with recorder
      SetRecorderMode(IDC_STOP);
      return FALSE;
    }
  }
  CleanUp();
  if (m_bFileApplied) EndDialog(IDOK);
  else EndDialog(IDCANCEL);
  return TRUE;
}

/***************************************************************************/
// CDlgRecorder::Apply Apply the wave file to a document
// This function closes the wavefile properly (writes the format parameters
// into the format chunk). Then it sets up the sa parameters and then tells
// the document the current filename (with path) of the temporary file and
// the document will prompt the user to save it. After that the recorder will
// be closed.
/***************************************************************************/
BOOL CDlgRecorder::Apply(CDocument* pDocument)
{
  CSaDoc* pDoc = (CSaDoc*)pDocument; // cast document pointer
  CSaApp* pApp = (CSaApp*)AfxGetApp(); // get pointer to application
  // set file pointer to end of file (also end of 'data' chunk)
  mmioSeek(m_hmmioFile, 0, SEEK_END);
  // get out of 'data' chunk
  if (mmioAscend(m_hmmioFile, &m_mmckinfoSubchunk, 0))
  {
    // error writing data chunk
    pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, m_szFileName);
    return FALSE;
  }
  m_dwRecordSize = m_mmckinfoSubchunk.cksize; // get recorded data size
  // get out of 'RIFF' chunk, to write RIFF size
  if (mmioAscend(m_hmmioFile, &m_mmckinfoParent, 0))
  {
    // error writing RIFF chunk
    pApp->ErrorMessage(IDS_ERROR_WRITERIFFCHUNK, m_szFileName);
    return FALSE;
  }
  // now rewrite the correct format parameters
  mmioSeek(m_hmmioFile, 0, SEEK_SET); // set file pointer to begin of file
  // descend into 'RIFF' chunk again
  if (mmioDescend(m_hmmioFile, &m_mmckinfoParent, NULL, MMIO_FINDRIFF))
  {
    // error descending into RIFF chunk
    pApp->ErrorMessage(IDS_ERROR_WRITERIFFCHUNK, m_szFileName);
    return FALSE;
  }
  // descend into 'fmt ' chunk
  MMCKINFO mmckinfoSubchunk;
  mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
  if (mmioDescend(m_hmmioFile, &mmckinfoSubchunk, &m_mmckinfoParent, MMIO_FINDCHUNK))
  {
    // error descending into format chunk
    pApp->ErrorMessage(IDS_ERROR_WRITEFORMATCHUNK, m_szFileName);
    return FALSE;
  }
  // write the format parameters into 'fmt ' chunk
  FmtParm* pFmtParm = pDoc->GetFmtParm(); // get pointer to format parameters
  long lError = mmioWrite(m_hmmioFile, (HPSTR)&pFmtParm->wTag, sizeof(WORD));
  if (lError != -1) lError = mmioWrite(m_hmmioFile, (HPSTR)&pFmtParm->wChannels, sizeof(WORD));
  if (lError != -1) lError = mmioWrite(m_hmmioFile, (HPSTR)&pFmtParm->dwSamplesPerSec, sizeof(DWORD));
  if (lError != -1) lError = mmioWrite(m_hmmioFile, (HPSTR)&pFmtParm->dwAvgBytesPerSec, sizeof(DWORD));
  if (lError != -1) lError = mmioWrite(m_hmmioFile, (HPSTR)&pFmtParm->wBlockAlign, sizeof(WORD));
  if (lError != -1) lError = mmioWrite(m_hmmioFile, (HPSTR)&pFmtParm->wBitsPerSample, sizeof(WORD));
  if (lError == -1)
  {
    // error writing format chunk
    pApp->ErrorMessage(IDS_ERROR_WRITEFORMATCHUNK, m_szFileName);
    return FALSE;
  }
  mmioClose(m_hmmioFile, 0); // close file
  // get sa parameters
  SaParm saParm;
  pDoc->GetSaParm(&saParm);
  // set the sa parameters
  saParm.RecordTimeStamp = CTime::GetCurrentTime();
  saParm.dwRecordBandWidth = pFmtParm->dwSamplesPerSec / 2;
  if (saParm.wFlags & SA_FLAG_HIGHPASS) saParm.dwRecordBandWidth -= 70;
  saParm.byRecordSmpSize = (BYTE)pFmtParm->wBitsPerSample;
  saParm.dwNumberOfSamples = m_dwRecordSize / pFmtParm->wBlockAlign;
  saParm.dwSignalBandWidth = saParm.dwRecordBandWidth;
  saParm.byQuantization = (BYTE)pFmtParm->wBitsPerSample;
  pDoc->SetSaParm(&saParm);
  // tell the document to apply the file
	pDoc->ApplyWaveFile(m_szFileName, m_dwRecordSize);
  // if player is visible, disable the speed slider until required processing is completed
  CMainFrame* pMain = (CMainFrame*)AfxGetMainWnd();
  CDlgPlayer* pPlayer = pMain->GetPlayer();
  if (pPlayer && pPlayer->IsWindowVisible()) pPlayer->EnableSpeedSlider(FALSE);

  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgRecorder message handlers

/***************************************************************************/
// CDlgRecorder::OnInitDialog Dialog initialization
// All the necessary special control windows have to be created and placed
// over their placeholders in the dialog. The dialog is centered over the
// mainframe window.
/***************************************************************************/
BOOL CDlgRecorder::OnInitDialog()
{
  CDialog::OnInitDialog();
  m_pWave->GetOutDevice()->ConnectMixer(this);
  m_pWave->GetInDevice()->ConnectMixer(this);
  m_nMode = 0;
  // get pointer to view and document
  CMainFrame* pMDIFrameWnd = (CMainFrame*)AfxGetMainWnd();
  m_pView = (CSaView*)pMDIFrameWnd->GetCurrSaView();
  m_pDoc = (CSaDoc*)m_pView->GetDocument();
  
  // change the font for the text controls
  CWnd* pWnd = GetDlgItem(IDC_SETTINGSTITLE);
  CFont* pFont = pWnd->GetFont(); // get the standard font
  LOGFONT logFont;
  pFont->GetObject(sizeof(LOGFONT), (void*)&logFont); // fill up logFont
  // modify the logFont
  logFont.lfWeight = FW_NORMAL; // not bold
  m_Font.CreateFontIndirect(&logFont); // create the modified font
  pWnd->SetFont(&m_Font); // set the modified font
  GetDlgItem(IDC_SETTINGSTEXT)->SetFont(&m_Font);
  // set file name in recorders caption
  CString szTitle = m_pDoc->GetTitle(); // load file name
  int nFind = szTitle.Find(':');
  if (nFind != -1) szTitle = szTitle.Left(nFind); // extract part left of :
  CString szCaption;
  GetWindowText(szCaption);
  szCaption += " - " + szTitle;
  SetWindowText(szCaption);
  CRect rWnd;
  // build and place the record toggle button
  m_record.Init(IDC_RECORD, "Rec", this);
  // build and place the play toggle button
  m_play.Init(IDC_PLAY, "Play", this);
  m_play.EnableWindow(FALSE); // disable Play button
  // build and place the stop toggle button
  m_stop.Init(IDC_STOP, "Stop", this);
  // build and place the pause toggle button
  m_pause.Init(IDC_PAUSE, "Pause", this);
  // build and place the total time LED window
  m_LEDTotalTime.Init(IDC_TOTALTIME, this);
  m_LEDTotalTime.SetTime(100, 1000);
  // build and place the position time LED window
  m_LEDPosTime.Init(IDC_POSITIONTIME, this);
  m_LEDPosTime.SetTime(0, 0);
  // build and place the VUbar window
  m_VUBar.Init(IDC_VUBAR, this);
  m_VUBar.SetVU(0);
  // build and place the volume slider windows
  m_SliderVolume.Init(IDC_VOLUMESLIDER, this);
  m_SliderRecVolume.Init(IDC_RECVOLUMESLIDER, this);
  // build and place the volume spin controls
  m_SpinVolume.Init(IDC_VOLUMESCROLL, this);
  m_SpinRecVolume.Init(IDC_RECVOLUMESCROLL, this);

  pWnd = GetDlgItem(IDC_MIXER);
  if(pWnd)
    pWnd->EnableWindow(m_pWave->GetInDevice()->ShowMixer(FALSE));

  OnMixerControlChange(0,0);

  CenterWindow(); // center dialog on frame window
  // set the settings text
  SetSettingsText();
  OnStop(); // put recorder into stop mode

  return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************/
// CDlgRecorder::OnRecord Button record hit
/***************************************************************************/
void CDlgRecorder::OnRecord()
{
  if ((m_nMode != IDC_RECORD) && (m_nMode != IDC_PAUSE)) SetRecorderMode(IDC_RECORD);
}

/***************************************************************************/
// CDlgRecorder::OnPlay Button play hit
/***************************************************************************/
void CDlgRecorder::OnPlay()
{
  if ((m_nMode != IDC_PLAY) && (m_nMode != IDC_PAUSE)) SetRecorderMode(IDC_PLAY);
}

/***************************************************************************/
// CDlgRecorder::OnStop Button stop hit
/***************************************************************************/
void CDlgRecorder::OnStop()
{
  if (m_nMode != IDC_STOP) SetRecorderMode(IDC_STOP);
  else m_stop.Release();
}

/***************************************************************************/
// CDlgRecorder::OnPause Button pause hit
/***************************************************************************/
void CDlgRecorder::OnPause()
{
  if (m_nMode != IDC_PAUSE) SetRecorderMode(IDC_PAUSE);
  else SetRecorderMode(m_nOldMode);
}

/***************************************************************************/
// CDlgRecorder::OnMixerControlChange Mixer has changed volume settings
/***************************************************************************/
LRESULT CDlgRecorder::OnMixerControlChange(WPARAM, LPARAM)
{
  BOOL bResult = FALSE;
  
  m_nVolume = m_pWave->GetVolume(&bResult);

  if(bResult)
  {
    SetDlgItemInt(IDC_VOLUMEEDIT, m_nVolume, TRUE);
    m_SliderVolume.SetPosition(m_nVolume);
  }

  m_nRecVolume = GetRecVolume();
  SetDlgItemInt(IDC_RECVOLUMEEDIT, m_nRecVolume, TRUE);
  m_SliderRecVolume.SetPosition(m_nRecVolume);

  return 0;
}

/***************************************************************************/
// CDlgRecorder::OnVolumeSlide Volume slider position changed
/***************************************************************************/
void CDlgRecorder::OnVolumeSlide()
{
  m_nVolume = m_SliderVolume.GetPosition();
  SetDlgItemInt(IDC_VOLUMEEDIT, m_SliderVolume.GetPosition(), TRUE);
  m_pWave->SetVolume(m_nVolume);
}

/***************************************************************************/
// CDlgRecorder::OnVolumeScroll Volume spin control hit
/***************************************************************************/
void CDlgRecorder::OnVolumeScroll()
{
  m_nVolume = GetDlgItemInt(IDC_VOLUMEEDIT, NULL, TRUE);
  if (m_SpinVolume.UpperButtonClicked()) m_nVolume++;
  else m_nVolume--;
  if ((int)m_nVolume < 0) m_nVolume = 0;
  if (m_nVolume > 100) m_nVolume = 100;
  SetDlgItemInt(IDC_VOLUMEEDIT, m_nVolume, TRUE);
  m_SliderVolume.SetPosition(m_nVolume);
  m_pWave->SetVolume(m_nVolume);
}


/***************************************************************************/
// CDlgRecorder::OnKillfocusVolumeEdit Volume edited
/***************************************************************************/
void CDlgRecorder::OnKillfocusVolumeEdit()
{
  m_nVolume = GetDlgItemInt(IDC_VOLUMEEDIT, NULL, TRUE);
  if ((int)m_nVolume < 0) m_nVolume = 0;
  if (m_nVolume > 100) m_nVolume = 100;
  SetDlgItemInt(IDC_VOLUMEEDIT, m_nVolume, TRUE);
  m_SliderVolume.SetPosition(m_nVolume);
  m_pWave->SetVolume(m_nVolume);
}

/***************************************************************************/
// CDlgRecorder::OnRecVolumeSlide Volume slider position changed
/***************************************************************************/
void CDlgRecorder::OnRecVolumeSlide()
{
  m_nRecVolume = m_SliderRecVolume.GetPosition();
  SetRecVolume(m_nRecVolume);
}

/***************************************************************************/
// CDlgRecorder::OnRecVolumeScroll Volume spin control hit
/***************************************************************************/
void CDlgRecorder::OnRecVolumeScroll()
{
  m_nRecVolume = GetDlgItemInt(IDC_RECVOLUMEEDIT, NULL, TRUE);
  if (m_SpinRecVolume.UpperButtonClicked()) m_nRecVolume++;
  else m_nRecVolume--;
  if ((int)m_nRecVolume < 0) m_nRecVolume = 0;
  if (m_nRecVolume > 100) m_nRecVolume = 100;
  SetRecVolume(m_nRecVolume);
}


/***************************************************************************/
// CDlgRecorder::OnKillfocusVolumeEdit Volume edited
/***************************************************************************/
void CDlgRecorder::OnKillfocusRecVolumeEdit()
{
  m_nRecVolume = GetDlgItemInt(IDC_RECVOLUMEEDIT, NULL, TRUE);
  if ((int)m_nRecVolume < 0) m_nRecVolume = 0;
  if (m_nRecVolume > 100) m_nRecVolume = 100;
  SetRecVolume(m_nRecVolume);
}

void CDlgRecorder::OnMixer() 
{
  m_pWave->GetInDevice()->ShowMixer();
}

void CDlgRecorder::SetRecVolume(int nVolume)
{
  BOOL bResult;
  m_pWave->GetInDevice()->SetVolume(nVolume, &bResult);
  if(!bResult)
  {
    EnableRecVolume(FALSE);
    nVolume = 0;
  }
  SetDlgItemInt(IDC_RECVOLUMEEDIT, nVolume, TRUE);
  m_SliderRecVolume.SetPosition(nVolume);  
}

void CDlgRecorder::EnableRecVolume(BOOL bEnable)
{
  CWnd* pWnd = GetDlgItem(IDC_RECVOLUMEEDIT);
  if(pWnd)
    pWnd->EnableWindow(bEnable);

  m_SliderRecVolume.EnableWindow(bEnable);
}

UINT CDlgRecorder::GetRecVolume()
{
  BOOL bResult;
  UINT nVolume = m_pWave->GetInDevice()->GetVolume(&bResult);
  EnableRecVolume(bResult);

  return nVolume;
}
/***************************************************************************/
// CDlgRecorder::OnClose Close the dialog
/***************************************************************************/
void CDlgRecorder::OnClose()
{
  CloseRecorder();
}

/***************************************************************************/
// CDlgRecorder::OnButtonClose Button close hit
/***************************************************************************/
void CDlgRecorder::OnButtonClose()
{
  SendMessage(WM_CLOSE);
}

/***************************************************************************/
// CDlgRecorder::OnCancel ESC key hit (there is no cancel key.)
// When the ESC key is hit, this does the same as if the CLOSE button was hit.
// (If OnButtonClose is not called here, a General Protection Fault results.)
/***************************************************************************/
void CDlgRecorder::OnCancel()
{
  CDialog::OnCancel();
  OnButtonClose();
}

/***************************************************************************/
// CDlgRecorder::OnSettings Button settings hit
/***************************************************************************/
void CDlgRecorder::OnSettings()
{
  SetRecorderMode(IDC_STOP);
  CSaDoc* pDoc = (CSaDoc*)m_pDoc; // cast document pointer
  // get format parameters
  FmtParm fmtParm;
  pDoc->GetFmtParm(&fmtParm);
  // get sa parameters
  SaParm saParm;
  pDoc->GetSaParm(&saParm);
  // create the dialog
  CDlgRecorderOptions dlgRecorderOptions(this);

  // init the dialog
  dlgRecorderOptions.SetSamplingRate(fmtParm.dwSamplesPerSec);
  dlgRecorderOptions.SetBitDepth(fmtParm.wBitsPerSample);
  dlgRecorderOptions.SetChannels(fmtParm.wChannels);
  dlgRecorderOptions.SetHighpass((saParm.wFlags & SA_FLAG_HIGHPASS) ? TRUE : FALSE);

  if (dlgRecorderOptions.DoModal() == IDOK)
  {
    if ((m_bFileReady && m_dwRecordSize) &&
        (
          (dlgRecorderOptions.GetSamplingRate() != (fmtParm.dwSamplesPerSec)) ||
          (dlgRecorderOptions.GetBitDepth() != fmtParm.wBitsPerSample) ||
          (dlgRecorderOptions.GetChannels() != fmtParm.wChannels) ||
          (dlgRecorderOptions.GetHighpass() != ((saParm.wFlags & SA_FLAG_HIGHPASS) ? TRUE : FALSE))
        )
      )
      
    {
      // ask user to delete recorded file before changing the settings
      int nResponse = AfxMessageBox(IDS_QUESTION_DELRECORD, MB_YESNO | MB_ICONQUESTION, 0);
      if (nResponse == IDYES)
      {
        m_play.EnableWindow(FALSE); // disable Play button
        GetDlgItem(IDC_APPLY)->EnableWindow(FALSE); // disable apply button
        DeleteTempFile(); // delete the temporary mmio file
        m_bFileReady = CreateTempFile(); // create new temporary mmio file
        m_dwRecordSize = 0;
      }
      else
      {
        // continue with recorder
        return;
      }
    }
    fmtParm.dwSamplesPerSec = dlgRecorderOptions.GetSamplingRate();
    fmtParm.wBitsPerSample = dlgRecorderOptions.GetBitDepth();
    fmtParm.wChannels = dlgRecorderOptions.GetChannels();
    fmtParm.wBlockAlign = WORD(fmtParm.wBitsPerSample / 8);
    fmtParm.dwAvgBytesPerSec = fmtParm.dwSamplesPerSec * fmtParm.wBlockAlign;
    if (dlgRecorderOptions.GetHighpass()) saParm.wFlags |= SA_FLAG_HIGHPASS;
    else
    {
      CWaveInDevice *pRecorder = m_pWave->GetInDevice();
      if (pRecorder->GetHighPassFilter()) pRecorder->DetachHighPassFilter();
      saParm.wFlags &= ~SA_FLAG_HIGHPASS;
    }
    // set format parameters
    pDoc->SetFmtParm(&fmtParm);
    // set sa parameters
    pDoc->SetSaParm(&saParm);

    if(!GetStaticSourceInfo().bEnable)
    {
      CSaDoc* pDoc = (CSaDoc*)m_pDoc; // cast document pointer
      SourceParm* pSourceParm = pDoc->GetSourceParm();

      pSourceParm->szCountry.Empty();
      pSourceParm->szDialect.Empty();
      pSourceParm->szEthnoID = "???";
      pSourceParm->szFamily.Empty();
      pSourceParm->szLanguage.Empty();
      pSourceParm->szRegion.Empty();
      pSourceParm->szTranscriber.Empty();
      pSourceParm->szSpeaker.Empty();
      pSourceParm->szReference.Empty();
      pSourceParm->szFreeTranslation.Empty();
      pSourceParm->szDescription.Empty();
      pSourceParm->nGender = -1;
    }
    SetSettingsText();
  }
  SetRecorderMode(IDC_STOP);
}

/***************************************************************************/
// CDlgRecorder::OnApply Button apply hit
// This function calls the function to apply the wave file to a document.
// After that, the recorder will be closed.
/***************************************************************************/
void CDlgRecorder::OnApply()
{
  if (m_pWave) m_pWave->Stop(); // stop recording
  m_VUBar.SetVU(0); // reset the VU bar
  if (m_bFileReady && !m_bFileApplied) // file is ready but not yet applied
  {
    if (!Apply(m_pDoc))
    {
      m_bFileReady = FALSE;
      CloseRecorder();
      return;
    }
    m_bFileApplied = TRUE;
  }
  ClearFileName(); // file has been overtaken from document
  CloseRecorder(); // close the recorder
  sourceInfo &m_source = GetStaticSourceInfo();
  if(m_source.bEnable)
  {
    CSaDoc* pDoc = (CSaDoc*)m_pDoc; // cast document pointer
    SourceParm* pSourceParm = pDoc->GetSourceParm();

    pSourceParm->szCountry = m_source.source.szCountry;
    pSourceParm->szDialect = m_source.source.szDialect;
    pSourceParm->szEthnoID = m_source.source.szEthnoID.Left(3);
    pSourceParm->szFamily = m_source.source.szFamily;
    pSourceParm->szLanguage = m_source.source.szLanguage;
    pSourceParm->nGender = m_source.source.nGender;
    pSourceParm->szRegion = m_source.source.szRegion;
    pSourceParm->szSpeaker = m_source.source.szSpeaker;
    pSourceParm->szReference = m_source.source.szReference;
    pSourceParm->szTranscriber = m_source.source.szTranscriber;
    pDoc->GetSaParm()->szDescription = m_source.source.szDescription;
    pSourceParm->szFreeTranslation = m_source.source.szFreeTranslation;
  }
  return;
}

/***************************************************************************/
// CDlgRecorder::OnHelpRecorder Call Export Table help
/***************************************************************************/
void CDlgRecorder::OnHelpRecorder()
{
	// create the pathname
	CString szPath = AfxGetApp()->m_pszHelpFilePath;
	szPath += "::/User_Interface/Menus/File/Record_New.htm";
	::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

/***************************************************************************/
// CDlgRecorder::GetDocument()
/***************************************************************************/
CSaDoc* CDlgRecorder::GetDocument()
{
  return m_pDoc;
}

/***************************************************************************/
// CDlgRecorder::GetStaticSourceInfo()
/***************************************************************************/
CDlgRecorder::sourceInfo & CDlgRecorder::GetStaticSourceInfo()
{
  static sourceInfo m_source;

  return m_source;
};

static const char * psz_sourceInfo = "sourceInfo";
static const char * psz_enable = "enable";
static const char * psz_country = "country";
static const char * psz_dialect = "dialect";
static const char * psz_ethnoID = "ethnoID";
static const char * psz_family = "family";
static const char * psz_language = "language";
static const char * psz_gender = "gender";
static const char * psz_region = "region";
static const char * psz_speaker = "speaker";
static const char * psz_reference = "reference";
static const char * psz_transcriber = "transcriber";
static const char * psz_freeTranslation = "freeTranslation";
static const char * psz_fileDescription = "fileDescription";

void CDlgRecorder::sourceInfo::WriteProperties(Object_ostream& obs)
{
  obs.WriteBeginMarker(psz_sourceInfo);

  sourceInfo &m_source = GetStaticSourceInfo();

  // write out properties
  obs.WriteInteger(psz_enable, m_source.bEnable);
  obs.WriteString(psz_country, m_source.source.szCountry);
  obs.WriteString(psz_dialect, m_source.source.szDialect);
  obs.WriteString(psz_ethnoID, m_source.source.szEthnoID);
  obs.WriteString(psz_family, m_source.source.szFamily);
  obs.WriteString(psz_language, m_source.source.szLanguage);
	CSaString szGender;
	szGender.Format(_T("%d"), m_source.source.nGender);
  obs.WriteString(psz_gender, szGender);
  obs.WriteString(psz_region, m_source.source.szRegion);
  obs.WriteString(psz_speaker, m_source.source.szSpeaker);
  obs.WriteString(psz_reference, m_source.source.szReference);
  obs.WriteString(psz_transcriber, m_source.source.szTranscriber);
  obs.WriteString(psz_fileDescription, m_source.source.szDescription);
  obs.WriteString(psz_freeTranslation, m_source.source.szFreeTranslation);

  obs.WriteEndMarker(psz_sourceInfo);
}

BOOL CDlgRecorder::sourceInfo::bReadProperties(Object_istream& obs)
{
  if ( !obs.bAtBackslash() || !obs.bReadBeginMarker(psz_sourceInfo) )
  {
    return FALSE;
  }

  sourceInfo &m_source = GetStaticSourceInfo();

  while ( !obs.bAtEnd() )
  {
		CSaString szGender;
		szGender.Format(_T("%d"), m_source.source.nGender);
    if ( obs.bReadInteger(psz_enable, m_source.bEnable) );
    else if (obs.bReadString(psz_country, &m_source.source.szCountry));
    else if (obs.bReadString(psz_dialect, &m_source.source.szDialect));
    else if (obs.bReadString(psz_ethnoID, &m_source.source.szEthnoID));
    else if (obs.bReadString(psz_family, &m_source.source.szFamily));
    else if (obs.bReadString(psz_language, &m_source.source.szLanguage));
    else if (obs.bReadString(psz_gender, &szGender))
			m_source.source.nGender = _ttoi(szGender.GetBuffer(2));
    else if (obs.bReadString(psz_region, &m_source.source.szRegion));
    else if (obs.bReadString(psz_speaker, &m_source.source.szSpeaker));
    else if (obs.bReadString(psz_reference, &m_source.source.szReference));
    else if (obs.bReadString(psz_transcriber, &m_source.source.szTranscriber));
    else if (obs.bReadString(psz_fileDescription, &m_source.source.szDescription));
    else if (obs.bReadString(psz_freeTranslation, &m_source.source.szFreeTranslation));
    else if ( obs.bEnd(psz_sourceInfo) )
      break;
  }
  return TRUE;
}

//###########################################################################
// CDlgRecorderOptions dialog
// Displays the controls to change recorder options.

/////////////////////////////////////////////////////////////////////////////
// CDlgRecorderOptions message map

BEGIN_MESSAGE_MAP(CDlgRecorderOptions, CDialog)
//{{AFX_MSG_MAP(CDlgRecorderOptions)
	ON_BN_CLICKED(IDC_DEFAULT, OnDefault)
	ON_BN_CLICKED(IDC_SOURCE, OnSource)
	ON_BN_CLICKED(IDC_SOURCE_AUTO, OnSourceAuto)
	ON_COMMAND(IDHELP, OnHelpRecorderOptions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgRecorderOptions construction/destruction/creation

/***************************************************************************/
// CDlgRecorderOptions::CDlgRecorderOptions Constructor
/***************************************************************************/
CDlgRecorderOptions::CDlgRecorderOptions(CDlgRecorder* pParent) : CDialog(CDlgRecorderOptions::IDD, pParent), m_pRecorder(pParent)
{
  m_bSourceAuto = CDlgRecorder::GetStaticSourceInfo().bEnable;
  m_nMode = 0;
  //{{AFX_DATA_INIT(CDlgRecorderOptions)
  m_nRate = -1;
  m_nBits = -1;
  m_bHighpass = TRUE;
  //}}AFX_DATA_INIT
}

/////////////////////////////////////////////////////////////////////////////
// CDlgRecorderOptions helper functions

/***************************************************************************/
// CDlgRecorderOptions::DoDataExchange Data exchange
/***************************************************************************/
void CDlgRecorderOptions::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CDlgRecorderOptions)
  DDX_Radio(pDX, IDC_11KHZ, m_nRate);
  DDX_Radio(pDX, IDC_8BIT, m_nBits);
  DDX_Check(pDX, IDC_HIGHPASS, m_bHighpass);
  DDX_Radio(pDX, IDC_MONO, m_nMode);
	DDX_Check(pDX, IDC_SOURCE_AUTO, m_bSourceAuto);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CDlgRecorderOptions message handlers

/***************************************************************************/
// CDlgRecorderOptions::OnInitDialog Dialog initialization
// The dialog is centered over the main frame window.
/***************************************************************************/
BOOL CDlgRecorderOptions::OnInitDialog()
{
  CDialog::OnInitDialog();
  CenterWindow(); // center dialog on recorder window
  return TRUE;
}

/***************************************************************************/
// CDlgRecorderOptions::OnDefault Default button hit
/***************************************************************************/
void CDlgRecorderOptions::OnDefault()
{
  // set defaults
  m_nRate = 1;
  m_nBits = 1;
  m_bHighpass = TRUE;
  m_nMode = 0;
  UpdateData(FALSE);
}

/***************************************************************************/
// CDlgRecorderOptions::SetSamplingRate(int nRate)
/***************************************************************************/
void CDlgRecorderOptions::SetSamplingRate(int nRate)
{
  switch(nRate)
  {
  case 11025:
    m_nRate = 0;
    break;
  case 22050:
    m_nRate = 1;
    break;
  case 44100:
    m_nRate = 2;
    break;
  default:
    ASSERT(FALSE);
    m_nRate = -1;    
  }
}

/***************************************************************************/
// CDlgRecorderOptions::GetSamplingRate()
/***************************************************************************/
unsigned int CDlgRecorderOptions::GetSamplingRate()
{
  int nRate = 22050;

  switch(m_nRate)
  {
  case 0:
    nRate = 11025;
    break;
  case 1:
    nRate = 22050;
    break;
  case 2:
    nRate = 44100;
    break;
  }
  return nRate;
}

/***************************************************************************/
// CDlgRecorderOptions::SetBitDepth(int nBits)
/***************************************************************************/
void CDlgRecorderOptions::SetBitDepth(int nBits)
{
  switch(nBits)
  {
  case 8:
    m_nBits = 0;
    break;
  case 16:
    m_nBits = 1;
    break;
  default:
    ASSERT(FALSE);
    m_nBits = -1;    
  }
}

/***************************************************************************/
// CDlgRecorderOptions::GetBitDepth()
/***************************************************************************/
short int CDlgRecorderOptions::GetBitDepth()
{
  short int nBits = 16;
  switch(m_nBits)
  {
  case 0:
    nBits = 8;
    break;
  case 1:
    nBits = 16;
  }
  return nBits;
}

/***************************************************************************/
// CDlgRecorderOptions::SetHighpass(BOOL bHighpass)
/***************************************************************************/
void CDlgRecorderOptions::SetHighpass(BOOL bHighpass)
{
  m_bHighpass = bHighpass;
}

/***************************************************************************/
// CDlgRecorderOptions::GetHighpass()
/***************************************************************************/
BOOL CDlgRecorderOptions::GetHighpass()
{
  return m_bHighpass;
}

/***************************************************************************/
// CDlgRecorderOptions::SetChannels(int nChannels)
/***************************************************************************/
void CDlgRecorderOptions::SetChannels(int nChannels)
{
  UNUSED(nChannels);
  ASSERT(nChannels == 1);
}
/***************************************************************************/
// CDlgRecorderOptions::GetChannels()
/***************************************************************************/
short CDlgRecorderOptions::GetChannels()
{
  return 1;
}

/***************************************************************************/
// CDlgRecorderOptions::GetRecorder()
/***************************************************************************/
CDlgRecorder& CDlgRecorderOptions::GetRecorder()
{
  return *m_pRecorder;
}

void CDlgRecorderOptions::OnSourceAuto() 
{
  UpdateData(TRUE);
  CDlgRecorder::GetStaticSourceInfo().bEnable = m_bSourceAuto;	
}

/***************************************************************************/
// CDlgRecorderOptions::OnSource
/***************************************************************************/
void CDlgRecorderOptions::OnSource() 
{
  UpdateData(TRUE);
  CSaDoc* pDoc = GetRecorder().GetDocument(); // cast document pointer
  SourceParm* pSourceParm = pDoc->GetSourceParm();
  CDlgRecorder::sourceInfo &m_source = CDlgRecorder::GetStaticSourceInfo();
  if(m_source.bEnable)
  {
    pSourceParm->szCountry = m_source.source.szCountry;
    pSourceParm->szDialect = m_source.source.szDialect;
    pSourceParm->szEthnoID = m_source.source.szEthnoID.Left(3);
    pSourceParm->szFamily = m_source.source.szFamily;
    pSourceParm->szLanguage = m_source.source.szLanguage;
    pSourceParm->nGender  = m_source.source.nGender;
    pSourceParm->szRegion = m_source.source.szRegion;
    pSourceParm->szSpeaker = m_source.source.szSpeaker;
    pSourceParm->szReference = m_source.source.szReference;
    pSourceParm->szTranscriber = m_source.source.szTranscriber;
    pSourceParm->szDescription = m_source.source.szDescription;
    pSourceParm->szFreeTranslation = m_source.source.szFreeTranslation;
  }
  CString szCaption;
  szCaption.LoadString(IDS_DLGTITLE_FILEINFO);                 // load caption string
  CDlgFileInformation* dlgFileInformation;                     // file information dialog
  dlgFileInformation = new CDlgFileInformation(szCaption, NULL, 0, TRUE);  // create the property sheet
  // set file description string
  dlgFileInformation->m_dlgUserPage.m_szFileDesc =
    pSourceParm->szDescription;
  dlgFileInformation->m_dlgUserPage.m_szFreeTranslation =
    pSourceParm->szFreeTranslation;
  if (dlgFileInformation->DoModal() == IDOK)
  {
    // get new file description string
    pDoc->GetSaParm()->szDescription = dlgFileInformation->m_dlgUserPage.m_szFileDesc;
    pSourceParm->szDescription = dlgFileInformation->m_dlgUserPage.m_szFileDesc;
    pSourceParm->szFreeTranslation = dlgFileInformation->m_dlgUserPage.m_szFreeTranslation;
    pSourceParm->szCountry = dlgFileInformation->m_dlgSourcePage.m_szCountry;
    pSourceParm->szDialect = dlgFileInformation->m_dlgSourcePage.m_szDialect;
    if (dlgFileInformation->m_dlgSourcePage.m_szEthnoID.GetLength() < 3)
      dlgFileInformation->m_dlgSourcePage.m_szEthnoID += "   ";
    pSourceParm->szEthnoID = dlgFileInformation->m_dlgSourcePage.m_szEthnoID.Left(3);
    pSourceParm->szFamily = dlgFileInformation->m_dlgSourcePage.m_szFamily;
    pSourceParm->szLanguage = dlgFileInformation->m_dlgSourcePage.m_szLanguage;
    pSourceParm->nGender = dlgFileInformation->m_dlgSourcePage.m_nGender;
    pSourceParm->szRegion = dlgFileInformation->m_dlgSourcePage.m_szRegion;
    pSourceParm->szSpeaker = dlgFileInformation->m_dlgSourcePage.m_szSpeaker;
    pSourceParm->szReference = dlgFileInformation->m_dlgSourcePage.m_szReference;
    pSourceParm->szTranscriber = dlgFileInformation->m_dlgSourcePage.m_szTranscriber;

    m_source.source.szCountry = dlgFileInformation->m_dlgSourcePage.m_szCountry;
    m_source.source.szDialect = dlgFileInformation->m_dlgSourcePage.m_szDialect;
    m_source.source.szEthnoID = dlgFileInformation->m_dlgSourcePage.m_szEthnoID.Left(3);
    m_source.source.szFamily = dlgFileInformation->m_dlgSourcePage.m_szFamily;
    m_source.source.szLanguage = dlgFileInformation->m_dlgSourcePage.m_szLanguage;
    m_source.source.nGender = dlgFileInformation->m_dlgSourcePage.m_nGender;
    m_source.source.szRegion = dlgFileInformation->m_dlgSourcePage.m_szRegion;
    m_source.source.szSpeaker = dlgFileInformation->m_dlgSourcePage.m_szSpeaker;
    m_source.source.szReference = dlgFileInformation->m_dlgSourcePage.m_szReference;
    m_source.source.szTranscriber = dlgFileInformation->m_dlgSourcePage.m_szTranscriber;
		m_source.source.szFreeTranslation = dlgFileInformation->m_dlgUserPage.m_szFreeTranslation;
		m_source.source.szDescription = dlgFileInformation->m_dlgUserPage.m_szFileDesc;
  }
  delete dlgFileInformation;                                    // delete the property sheet
}

/***************************************************************************/
// CDlgRecorderOptions::OnHelpRecorderOptions Call Recorder Settings help
/***************************************************************************/
void CDlgRecorderOptions::OnHelpRecorderOptions()
{
	// create the pathname
	CString szPath = AfxGetApp()->m_pszHelpFilePath;
	szPath += "::/User_Interface/Menus/File/Recorder_Settings.htm";
	::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

//###########################################################################
// CDlgFnKeys dialog
// Displays the controls to setup the function keys.

/////////////////////////////////////////////////////////////////////////////
// CDlgFnKeys message map

BEGIN_MESSAGE_MAP(CDlgFnKeys, CDlgAudio)
//{{AFX_MSG_MAP(CDlgFnKeys)
  ON_COMMAND(IDC_VOLUMESLIDER, OnVolumeSlide)
  ON_COMMAND(IDC_VOLUMESCROLL, OnVolumeScroll)
  ON_EN_KILLFOCUS(IDC_VOLUMEEDIT, OnKillfocusVolumeEdit)
  ON_COMMAND(IDC_SPEEDSLIDER, OnSpeedSlide)
  ON_COMMAND(IDC_SPEEDSCROLL, OnSpeedScroll)
  ON_EN_KILLFOCUS(IDC_SPEEDEDIT, OnKillfocusSpeedEdit)
  ON_COMMAND(IDC_DELAYSLIDER, OnDelaySlide)
  ON_COMMAND(IDC_DELAYSCROLL, OnDelayScroll)
  ON_EN_KILLFOCUS(IDC_DELAYEDIT, OnKillfocusDelayEdit)
  ON_BN_CLICKED(IDC_REPEAT, OnRepeat)
  ON_LBN_SELCHANGE(IDC_FNLIST, OnSelchangeFnlist)
  ON_WM_CLOSE()
  ON_WM_TIMER()
  ON_COMMAND(IDC_FNTEST, OnTest)
  ON_MESSAGE(MM_MIXM_CONTROL_CHANGE, OnMixerControlChange)
	ON_COMMAND(IDHELP, OnHelpFnKeys)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgFnKeys construction/destruction/creation

/***************************************************************************/
// CDlgFnKeys::CDlgFnKeys Constructor
/***************************************************************************/
CDlgFnKeys::CDlgFnKeys(CWnd* pParent) : CDlgAudio(CDlgFnKeys::IDD, pParent)
{
  //{{AFX_DATA_INIT(CDlgFnKeys)
  m_nDelay = 1000;
  m_nSpeed = 50;
  m_bRepeat = FALSE;
  m_nPlayMode = 4;
  //}}AFX_DATA_INIT
  m_bNoTest = FALSE;
  m_bTestRunning = FALSE;
  m_pDoc = NULL;
  m_pView = NULL;
  m_NotifyObj.Attach(this); // attach notify object
  m_pWave = new CWave;      // create CWave object
  m_nVolume = m_pWave->GetVolume();
  m_nSelection = -1;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgFnKeys helper functions

/***************************************************************************/
// CDlgFnKeys::DoDataExchange Data exchange
/***************************************************************************/
void CDlgFnKeys::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CDlgFnKeys)
  DDX_Text(pDX, IDC_DELAYEDIT, m_nDelay);
  DDV_MinMaxUInt(pDX, m_nDelay, 0, 9999);
  DDX_Text(pDX, IDC_SPEEDEDIT, m_nSpeed);
  DDV_MinMaxUInt(pDX, m_nSpeed, 10, 400);
  DDX_Text(pDX, IDC_VOLUMEEDIT, m_nVolume);
  DDV_MinMaxUInt(pDX, m_nVolume, 0, 100);
  DDX_Check(pDX, IDC_REPEAT, m_bRepeat);
  DDX_CBIndex(pDX, IDC_PLAYMODE, m_nPlayMode);
  //}}AFX_DATA_MAP
}

/***************************************************************************/
// CDlgFnKeys::EndPlayback Playback finished
/***************************************************************************/
void CDlgFnKeys::EndPlayback()
{
  if (m_pWave) m_pWave->Stop();
  if (m_bRepeat) SetTimer(1, m_nDelay, NULL); // start repeating
  else OnTest();
}

/////////////////////////////////////////////////////////////////////////////
// CDlgFnKeys message handlers

/***************************************************************************/
// CDlgFnKeys::OnInitDialog Dialog initialization
// The dialog is centered over the main frame window.
/***************************************************************************/
BOOL CDlgFnKeys::OnInitDialog()
{
  CDialog::OnInitDialog();
  m_pWave->GetOutDevice()->ConnectMixer(this);
  // save the caption of the test button
  CWnd* pWnd = GetDlgItem(IDC_FNTEST);
  pWnd->GetWindowText(m_szTest); // get the Test button caption
  // build and place the speed slider window
  m_SliderSpeed.Init(IDC_SPEEDSLIDER, this);
  m_SliderSpeed.SetPosition(50);
  // build and place the speed spin control
  m_SpinSpeed.Init(IDC_SPEEDSCROLL, this);
  // build and place the volume slider window
  m_SliderVolume.Init(IDC_VOLUMESLIDER, this);
  m_SliderVolume.SetPosition(m_nVolume);
  // build and place the volume spin control
  m_SpinVolume.Init(IDC_VOLUMESCROLL, this);
  // build and place the delay slider window
  m_SliderDelay.Init(IDC_DELAYSLIDER, this);
  m_SliderDelay.SetPosition(10);
  // build and place the delay spin control
  m_SpinDelay.Init(IDC_DELAYSCROLL, this);
  // load the function key setup
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
  pMainWnd->GetFnKeys(&m_fnKeys);
  // fill up the list box
  CListBox* pLB = (CListBox*)GetDlgItem(IDC_FNLIST);
  TCHAR szText[4];
  for (int nLoop = 1; nLoop <= 12; nLoop++)
  {
    _stprintf(szText, _T("F%u"), nLoop);
    pLB->AddString(szText);
  }
  pLB->SetCurSel(0);
  // disable delay editing
  m_SliderDelay.EnableWindow(FALSE); // disable delay slider
  m_SpinDelay.EnableWindow(FALSE); // disable delay spin control
  GetDlgItem(IDC_DELAYEDIT)->EnableWindow(FALSE); // disable delay edit control
  GetDlgItem(IDC_DELAYTEXT)->EnableWindow(FALSE); // disable delay dimension text
  OnSelchangeFnlist();
  if (m_bNoTest) GetDlgItem(IDC_FNTEST)->EnableWindow(FALSE); // disable test run
  CenterWindow(); // center dialog on recorder window
  // disable closing the dialog other than with cancel or ok
  return TRUE;
}

/***************************************************************************/
// CDlgFnKeys::OnMixerControlChange Mixer has changed volume settings
/***************************************************************************/
LRESULT CDlgFnKeys::OnMixerControlChange(WPARAM, LPARAM)
{
  BOOL bResult = FALSE;
  
  m_nVolume = m_pWave->GetVolume(&bResult);

  if(bResult)
  {
    SetDlgItemInt(IDC_VOLUMEEDIT, m_nVolume, TRUE);
    m_SliderVolume.SetPosition(m_nVolume);
  }

  return 0;
}

/***************************************************************************/
// CDlgFnKeys::OnVolumeSlide Volume slider position changed
/***************************************************************************/
void CDlgFnKeys::OnVolumeSlide()
{
  m_nVolume = m_SliderVolume.GetPosition();
  SetDlgItemInt(IDC_VOLUMEEDIT, m_SliderVolume.GetPosition(), TRUE);
  m_pWave->SetVolume(m_nVolume);
}

/***************************************************************************/
// CDlgFnKeys::OnVolumeScroll Volume spin control hit
/***************************************************************************/
void CDlgFnKeys::OnVolumeScroll()
{
  m_nVolume = GetDlgItemInt(IDC_VOLUMEEDIT, NULL, TRUE);
  if (m_SpinVolume.UpperButtonClicked()) m_nVolume++;
  else m_nVolume--;
  if ((int)m_nVolume < 0) m_nVolume = 0;
  if (m_nVolume > 100) m_nVolume = 100;
  SetDlgItemInt(IDC_VOLUMEEDIT, m_nVolume, TRUE);
  m_SliderVolume.SetPosition(m_nVolume);
  m_pWave->SetVolume(m_nVolume);
}

/***************************************************************************/
// CDlgFnKeys::OnKillfocusVolumeEdit Volume edited
/***************************************************************************/
void CDlgFnKeys::OnKillfocusVolumeEdit()
{
  m_nVolume = GetDlgItemInt(IDC_VOLUMEEDIT, NULL, TRUE);
  if ((int)m_nVolume < 0) m_nVolume = 0;
  if (m_nVolume > 100) m_nVolume = 100;
  SetDlgItemInt(IDC_VOLUMEEDIT, m_nVolume, TRUE);
  m_SliderVolume.SetPosition(m_nVolume);
  m_pWave->SetVolume(m_nVolume);
}

/***************************************************************************/
// CDlgFnKeys::OnSpeedSlide Speed slider position changed
/***************************************************************************/
void CDlgFnKeys::OnSpeedSlide()
{
  UINT nSpeed = m_SliderSpeed.GetPosition();
  if (nSpeed > 50) m_nSpeed = 100 + (nSpeed - 50) * 233 / 50;
  else m_nSpeed = 10 + nSpeed * 90 / 50;
  SetDlgItemInt(IDC_SPEEDEDIT, m_nSpeed, TRUE);
  m_pWave->SetSpeed(m_nSpeed);
}

/***************************************************************************/
// CDlgFnKeys::OnSpeedScroll Speed spin control hit
/***************************************************************************/
void CDlgFnKeys::OnSpeedScroll()
{
  m_nSpeed = GetDlgItemInt(IDC_SPEEDEDIT, NULL, TRUE);
  if (m_SpinSpeed.UpperButtonClicked()) m_nSpeed++;
  else m_nSpeed--;
  if (m_nSpeed > 333) m_nSpeed = 333;
  if (m_nSpeed < 10) m_nSpeed = 10;
  SetDlgItemInt(IDC_SPEEDEDIT, m_nSpeed, TRUE);
  if (m_nSpeed > 100) m_SliderSpeed.SetPosition(50 + 50 * (m_nSpeed - 100) / 233);
  else m_SliderSpeed.SetPosition(50 * (m_nSpeed - 10) / 90);
  m_pWave->SetSpeed(m_nSpeed);
}

/***************************************************************************/
// CDlgFnKeys::OnKillfocusSpeedEdit Speed edited
/***************************************************************************/
void CDlgFnKeys::OnKillfocusSpeedEdit()
{
  m_nSpeed = GetDlgItemInt(IDC_SPEEDEDIT, NULL, TRUE);
  if ((int)m_nSpeed < 10) m_nSpeed = 10;
  if (m_nSpeed > 333) m_nSpeed = 333;
  SetDlgItemInt(IDC_SPEEDEDIT, m_nSpeed, TRUE);
  if (m_nSpeed > 100) m_SliderSpeed.SetPosition(50 + 50 * (m_nSpeed - 100) / 233);
  else m_SliderSpeed.SetPosition(50 * (m_nSpeed - 10) / 90);
  m_pWave->SetSpeed(m_nSpeed);
}

/***************************************************************************/
// CDlgFnKeys::OnDelaySlide Delay slider position changed
/***************************************************************************/
void CDlgFnKeys::OnDelaySlide()
{
  m_nDelay = m_SliderDelay.GetPosition() * 100;
  if (m_nDelay > 9999) m_nDelay = 9999;
  SetDlgItemInt(IDC_DELAYEDIT, m_nDelay, TRUE);
}

/***************************************************************************/
// CDlgFnKeys::OnDelayScroll Delay spin control hit
/***************************************************************************/
void CDlgFnKeys::OnDelayScroll()
{
  m_nDelay = GetDlgItemInt(IDC_DELAYEDIT, NULL, TRUE);
  if (m_SpinDelay.UpperButtonClicked()) m_nDelay += 100;
  else
  {
    if (m_nDelay == 9999) m_nDelay -= 99;
    else
    {
      if (m_nDelay > 100) m_nDelay -= 100;
      else m_nDelay = 0;
    }
  }
  if (m_nDelay > 9999) m_nDelay = 9999;
  SetDlgItemInt(IDC_DELAYEDIT, m_nDelay, TRUE);
  if (m_nDelay == 9999) m_SliderDelay.SetPosition(100);
  else m_SliderDelay.SetPosition(m_nDelay / 100);
}

/***************************************************************************/
// CDlgFnKeys::OnKillfocusDelayEdit Delay edited
/***************************************************************************/
void CDlgFnKeys::OnKillfocusDelayEdit()
{
  m_nDelay = GetDlgItemInt(IDC_DELAYEDIT, NULL, TRUE);
  if ((int)m_nDelay < 0) m_nDelay = 0;
  if (m_nDelay > 9999) m_nDelay = 9999;
  SetDlgItemInt(IDC_DELAYEDIT, m_nDelay, TRUE);
  if (m_nDelay == 9999) m_SliderDelay.SetPosition(100);
  else m_SliderDelay.SetPosition(m_nDelay / 100);
}

/***************************************************************************/
// CDlgFnKeys::OnRepeat Replay repeat clicked
/***************************************************************************/
void CDlgFnKeys::OnRepeat()
{
  UpdateData(TRUE);
  if (m_bRepeat)
  {
    // enable delay editing
    m_SliderDelay.EnableWindow(TRUE); // enable delay slider
    m_SpinDelay.EnableWindow(TRUE); // enable delay spin control
    GetDlgItem(IDC_DELAYEDIT)->EnableWindow(TRUE); // enable delay edit control
    GetDlgItem(IDC_DELAYTEXT)->EnableWindow(TRUE); // enable delay dimension text
  }
  else
  {
    // disable delay editing
    m_SliderDelay.EnableWindow(FALSE); // disable delay slider
    m_SpinDelay.EnableWindow(FALSE); // disable delay spin control
    GetDlgItem(IDC_DELAYEDIT)->EnableWindow(FALSE); // disable delay edit control
    GetDlgItem(IDC_DELAYTEXT)->EnableWindow(FALSE); // disable delay dimension text
  }
}

/***************************************************************************/
// CDlgFnKeys::OnSelchangeFnlist Keylist selection changed
/***************************************************************************/
void CDlgFnKeys::OnSelchangeFnlist()
{
  if (m_nSelection != -1)
  {
    UpdateData(TRUE);
    // save changes
    m_fnKeys.nDelay[m_nSelection] = m_nDelay;
    m_fnKeys.nSpeed[m_nSelection] = m_nSpeed;
    m_fnKeys.nVolume[m_nSelection] = m_nVolume;
    m_fnKeys.bRepeat[m_nSelection] = m_bRepeat;
    switch (m_nPlayMode)
    {
      case 0:
        m_fnKeys.nMode[m_nSelection] = ID_PLAYBACK_CURSORS; break;
      case 1:
        m_fnKeys.nMode[m_nSelection] = ID_PLAYBACK_LTOSTART; break;
      case 2:
        m_fnKeys.nMode[m_nSelection] = ID_PLAYBACK_STARTTOR; break;
      case 3:
        m_fnKeys.nMode[m_nSelection] = ID_PLAYBACK_LTOSTOP; break;
      case 4:
        m_fnKeys.nMode[m_nSelection] = ID_PLAYBACK_STOPTOR; break;
      case 5:
        m_fnKeys.nMode[m_nSelection] = ID_PLAYBACK_WINDOW; break;
      case 6:
        m_fnKeys.nMode[m_nSelection] = ID_PLAYBACK_FILE; break;
    }
  }
  CListBox* pLB = (CListBox*)GetDlgItem(IDC_FNLIST);
  m_nSelection = pLB->GetCurSel();
  m_nDelay = m_fnKeys.nDelay[m_nSelection];
  m_nSpeed = m_fnKeys.nSpeed[m_nSelection];
  m_nVolume = m_fnKeys.nVolume[m_nSelection];
  m_bRepeat = m_fnKeys.bRepeat[m_nSelection];
  switch (m_fnKeys.nMode[m_nSelection])
  {
    case ID_PLAYBACK_CURSORS:
      m_nPlayMode = 0; break;
    case ID_PLAYBACK_LTOSTART:
      m_nPlayMode = 1; break;
    case ID_PLAYBACK_STARTTOR:
      m_nPlayMode = 2; break;
    case ID_PLAYBACK_LTOSTOP:
      m_nPlayMode = 3; break;
    case ID_PLAYBACK_STOPTOR:
      m_nPlayMode = 4; break;
    case ID_PLAYBACK_WINDOW:
      m_nPlayMode = 5; break;
    case ID_PLAYBACK_FILE:
      m_nPlayMode = 6; break;
  }
  UpdateData(FALSE);
  OnKillfocusDelayEdit();
  OnKillfocusSpeedEdit();
  OnKillfocusVolumeEdit();
  OnRepeat();
}

/***************************************************************************/
// CDlgFnKeys::OnOK Button OK hit
/***************************************************************************/
void CDlgFnKeys::OnOK()
{
  if (m_pWave) m_pWave->Stop();
  UpdateData(TRUE);
  OnSelchangeFnlist();
  // save the function key setup
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
  pMainWnd->SetFnKeys(&m_fnKeys);
  if (m_pWave)
	{
		delete m_pWave; // delete the CWave object
		m_pWave = NULL;
	}
  CDialog::OnOK();
}

/***************************************************************************/
// CDlgFnKeys::OnCancel Button cancel hit
/***************************************************************************/
void CDlgFnKeys::OnCancel()
{
  OnClose();
}

/***************************************************************************/
// CDlgFnKeys::OnClose Close the dialog
/***************************************************************************/
void CDlgFnKeys::OnClose()
{
  if (m_pWave) m_pWave->Stop();
  if (m_pWave)
	{
		delete m_pWave; // delete the CWave object
		m_pWave = NULL;
	}
  CDialog::OnCancel();
}

/***************************************************************************/
// CDlgFnKeys::OnTimer Timer event, repeat playback
/***************************************************************************/
void CDlgFnKeys::OnTimer(UINT nIDEvent)
{
  KillTimer(1);
  if (m_bTestRunning)
  {
    m_bTestRunning = FALSE;
    OnTest();
  }
  CWnd::OnTimer(nIDEvent);
}

/***************************************************************************/
// CDlgFnKeys::OnTest Test run
/***************************************************************************/
void CDlgFnKeys::OnTest()
{
  CWnd* pWnd = GetDlgItem(IDC_FNTEST);
  CSaApp* pApp = (CSaApp*)AfxGetApp(); // get pointer to application
  if (m_bTestRunning)
  {
    if (m_pWave) m_pWave->Stop();
    KillTimer(1);
    m_bTestRunning = FALSE;
    pWnd->SetWindowText(m_szTest); // set back the original Test button caption
  }
  else
  {
    m_bTestRunning = TRUE;
    UpdateData(TRUE);
    CString szText;
    szText.LoadString(IDS_STOP_TEST);
    pWnd->SetWindowText(szText); // set the stop Test button caption
    DWORD dwSize, dwStart = 0;
    CSaView* pView = (CSaView*)m_pView;  // cast pointer to view
    CSaDoc* pDoc = (CSaDoc*)m_pDoc;  // cast pointer to document
    FmtParm* pFmtParm = pDoc->GetFmtParm(); // get sa parameters format member data
    WORD wSmpSize = WORD(pFmtParm->wBlockAlign / pFmtParm->wChannels);
    BOOL bError = FALSE;
    switch(m_nPlayMode)
    {
      case 0:
        dwStart = pView->GetStartCursorPosition();
        dwSize = pView->GetStopCursorPosition() - dwStart + wSmpSize;
        break;
      case 1:
        dwStart = DWORD(pView->GetDataPosition(0));
        dwSize = pView->GetStartCursorPosition();
        if (dwSize > dwStart) dwSize -= dwStart;
        else bError = TRUE;
        break;
      case 2:
        dwStart = pView->GetStartCursorPosition();
        dwSize = DWORD(pView->GetDataPosition(0) + pView->GetDataFrame());
        if (dwSize > dwStart) dwSize -= dwStart;
        else bError = TRUE;
        break;
      case 3:
        dwStart = DWORD(pView->GetDataPosition(0));
        dwSize = pView->GetStopCursorPosition();
        if (dwSize > dwStart) dwSize -= dwStart;
        else bError = TRUE;
        break;
      case 4:
        dwStart = pView->GetStopCursorPosition();
        dwSize = DWORD(pView->GetDataPosition(0) + pView->GetDataFrame());
        if (dwSize > dwStart) dwSize -= dwStart;
        else bError = TRUE;
        break;
      case 5:
        dwStart = DWORD(pView->GetDataPosition(0));
        dwSize = pView->GetDataFrame();
        break;
      case 6:
        dwSize = pDoc->GetDataSize();
        break;
      default: dwStart = dwSize = 0; break;
    }
    if (bError) pApp->ErrorMessage(IDS_ERROR_PLAYMODE); // play mode not playable
    else bError = !m_pWave->Play(dwStart, dwSize, m_nVolume, m_nSpeed, m_pView, &m_NotifyObj);
    if (bError)
    {
      m_bTestRunning = FALSE;
      pWnd->SetWindowText(m_szTest); // set back the original Test button caption
    }
  }
  // display pending error messages
  pApp->DisplayErrorMessage();
}

/***************************************************************************/
// CDlgFnKeys::OnHelpFnKeys Call Function Keys help
/***************************************************************************/
void CDlgFnKeys::OnHelpFnKeys()
{
	// create the pathname
	CString szPath = AfxGetApp()->m_pszHelpFilePath;
	szPath += "::/User_Interface/Menus/Playback/Playback_Function_Keys_Setup.htm";
	::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

