//////////////////////////////////////////////////////////////////////////////
// playerRecorder.cpp:
// Implementation of the CDlgWaveNotifyObj (notify object)
//                       CDlgPlayer (dialog)
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
#include "DlgPlayer.h"
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
#include "objectostream.h"
#include "Sa_Doc.h"
#include "SA_View.h"

//###########################################################################
// CDlgPlayer dialog
// Displays the player that allows the user to control the wave data playing
// process, done by the CWave class.

bool CDlgPlayer::bLaunched = false; // player not launched at startup

BEGIN_MESSAGE_MAP(CDlgPlayer, CDialog)
	ON_BN_CLICKED(IDC_PLAY_CONTROLS, OnPlayControls)
	ON_BN_CLICKED(IDC_REPEAT, OnRepeat)
	ON_BN_CLICKED(IDC_SETUP, OnSetup)
	ON_BN_CLICKED(ID_CLOSE, OnClose)
	ON_CBN_SELCHANGE(IDC_PLAYMODE, OnSelchangePlaymode)
	ON_COMMAND(IDC_DELAYSCROLL, OnDelayScroll)
	ON_COMMAND(IDC_DELAYSLIDER, OnDelaySlide)
	ON_COMMAND(IDC_PAUSE, OnPause)
	ON_COMMAND(IDC_PLAY, OnPlay)
	ON_COMMAND(IDC_SPEEDSCROLL, OnSpeedScroll)
	ON_COMMAND(IDC_SPEEDSLIDER, OnSpeedSlide)
	ON_COMMAND(IDC_STOP, OnStop)
	ON_COMMAND(IDC_VOLUMESCROLL, OnVolumeScroll)
	ON_COMMAND(IDC_VOLUMESLIDER, OnVolumeSlide)
	ON_COMMAND(IDHELP, OnHelpPlayer)
	ON_EN_KILLFOCUS(IDC_DELAYEDIT, OnKillfocusDelayEdit)
	ON_EN_KILLFOCUS(IDC_SPEEDEDIT, OnKillfocusSpeedEdit)
	ON_EN_KILLFOCUS(IDC_VOLUMEEDIT, OnKillfocusVolumeEdit)
	ON_MESSAGE(MM_MIXM_CONTROL_CHANGE, OnMixerControlChange)
	ON_MESSAGE(WM_USER_SETUP_FNKEYS, OnSetupFnKeys)
	ON_WM_CLOSE()
	ON_WM_TIMER()
END_MESSAGE_MAP()

CDlgPlayer::CDlgPlayer(CWnd * pParent) : CDialog(CDlgPlayer::IDD, pParent) {
	m_nComboPlayMode = 4;	// start cursor to right window (ID_PLAYBACK_LEFTWIN_TO_ENDCUR)
	m_nSpeed = 100;
	m_nDelay = 1000;
	m_bRepeat = FALSE;
	m_bTestRunning = FALSE;
	m_pDoc = NULL;
	m_pView = NULL;
	m_NotifyObj.Attach(this); // attach notify object
	m_pWave = new CWave();      // create CWave object
	BOOL bResult;
	m_nVolume = m_pWave->GetVolume(bResult);
	m_bFullSize = FALSE;
	m_dwPlayPosition = 0;
	m_bFnKeySetting = false;
}

/***************************************************************************/
// Added because sometimes the player object was deleted without setting
// this flag to FALSE. That caused some GPF's. Also added the DestroyWindow
// because the MFC code was giving ASSERTS without it. Possible memory
// leaks. 09/30/2000 - DDO
/***************************************************************************/
CDlgPlayer::~CDlgPlayer() {
	bLaunched = false;
	if (m_pWave != NULL) {
		delete m_pWave;
		m_pWave = NULL;
	}
	DestroyWindow();
}

/***************************************************************************/
// CDlgPlayer::Create Creation
/***************************************************************************/
BOOL CDlgPlayer::Create() {
	bLaunched = true; // player launched
	return CDialog::Create(CDlgPlayer::IDD);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgPlayer helper functions

/***************************************************************************/
// CDlgPlayer::DoDataExchange Data exchange
/***************************************************************************/
void CDlgPlayer::DoDataExchange(CDataExchange * pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_PLAYMODE, m_nComboPlayMode);
	DDX_Text(pDX, IDC_VOLUMEEDIT, m_nVolume);
	DDV_MinMaxInt(pDX, m_nVolume, 0, 100);
	DDX_Text(pDX, IDC_SPEEDEDIT, m_nSpeed);
	DDV_MinMaxInt(pDX, m_nSpeed, 10, 400);
	DDX_Text(pDX, IDC_DELAYEDIT, m_nDelay);
	DDV_MinMaxUInt(pDX, m_nDelay, 0, 9999);
	DDX_Check(pDX, IDC_REPEAT, m_bRepeat);
}

/***************************************************************************/
// CDlgPlayer::SetTotalTime Set total time display
/***************************************************************************/
void CDlgPlayer::SetTotalTime() {
	CSaDoc * pModel = (CSaDoc *)m_pDoc;       // cast pointer to document
	DWORD dwEnd = pModel->GetDataSize();      // data end
	if (dwEnd > 0) {
		// display depends on the submode
		switch (m_nSubMode) {
		case ID_PLAYBACK_STARTCUR_TO_FILEEND:
		case ID_PLAYBACK_ENCCUR_TO_FILEEND:
		case ID_PLAYBACK_FILE:
			break;
		case ID_PLAYBACK_WINDOW:
		case ID_PLAYBACK_ENDCUR_TO_RIGHTWIN:
		case ID_PLAYBACK_STARTCUR_TO_RIGHTWIN:
			dwEnd = DWORD(m_pView->GetDataPosition(0)) + m_pView->GetDataFrame();
			break;
		case ID_PLAYBACK_CURSORS:
		case ID_PLAYBACK_LEFTWIN_TO_ENDCUR:
			dwEnd = m_pView->GetStopCursorPosition();
			break;
		case ID_PLAYBACK_LEFTWIN_TO_STARTCUR:
			dwEnd = m_pView->GetStartCursorPosition();
			break;
		default:
			break;
		}
		double fDataSec = m_pView->GetDocument()->GetTimeFromBytes(dwEnd); // calculate time
		m_LEDTotalTime.SetTime((int)fDataSec / 60, (int)(fDataSec * 10) % 600);
	}
}

/***************************************************************************/
// CDlgPlayer::SetPositionTime Set position time display
/***************************************************************************/
void CDlgPlayer::SetPositionTime() {
	CSaDoc * pModel = (CSaDoc *)m_pDoc; // cast pointer to document
	if (pModel->GetDataSize() > 0) {
		DWORD dwPosition = m_dwPlayPosition; // data position
		if (m_dwPlayPosition == 0) {
			// display depends on the submode
			switch (m_nSubMode) {
			case ID_PLAYBACK_FILE:
				break;
			case ID_PLAYBACK_WINDOW:
			case ID_PLAYBACK_LEFTWIN_TO_STARTCUR:
			case ID_PLAYBACK_LEFTWIN_TO_ENDCUR:
				dwPosition = DWORD(m_pView->GetDataPosition(0));
				TRACE("DataPosition=%lu\n", dwPosition);
				break;
			case ID_PLAYBACK_CURSORS:
			case ID_PLAYBACK_STARTCUR_TO_RIGHTWIN:
			case ID_PLAYBACK_STARTCUR_TO_FILEEND:
				dwPosition = m_pView->GetStartCursorPosition();
				break;
			case ID_PLAYBACK_ENCCUR_TO_FILEEND:
			case ID_PLAYBACK_ENDCUR_TO_RIGHTWIN:
				dwPosition = m_pView->GetStopCursorPosition();
				break;
			default:
				break;
			}
		}
		double fDataSec = pModel->GetTimeFromBytes(dwPosition); // calculate time
		m_LEDPosTime.SetTime((int)fDataSec / 60, (int)(fDataSec * 10) % 600);
		SetDlgItemText(IDC_TIMERTEXT, _T("Start Time"));
	}
}

/***************************************************************************/
// CDlgPlayer::SetPlayerFullSize Set the player size to full size
/***************************************************************************/
void CDlgPlayer::SetPlayerFullSize() {
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

const char * CDlgPlayer::GetMode(EMode mode) {
	switch (mode) {
	case IDLE: return "IDLE";
	case STOPPED: return "STOPPED";
	case PLAYING: return "PLAYING";
	case RECORDING: return "RECORDING";
	case PAUSED: return "PAUSED";
	}
	return "ERROR!";
}

/***************************************************************************/
// CDlgPlayer::SetPlayerMode Set the player mode
/***************************************************************************/
bool CDlgPlayer::SetPlayerMode(EMode mode, UINT nSubMode, BOOL bFullSize, BOOL bFnKey, SSpecific * pSpecific) {

	TRACE(">>SetPlayerMode %s->%s\n", GetMode(m_nMode), GetMode(mode));
	nSubMode = nSubMode & 0x0ffff;

	// SDM 1.06.6U6
	if ((m_bFnKeySetting) &&
		((!bFnKey) || (nSubMode != SubModeUndefined) || (!m_bRepeat))) {
		// restore original player setting
		m_bFnKeySetting = false;
		m_nVolume = m_nOldVolume;
		m_nSpeed = m_nOldSpeed;
		m_nDelay = m_nOldDelay;
		m_bRepeat = m_bOldRepeat;
		UpdateData(FALSE);
		OnKillfocusVolumeEdit();
		OnKillfocusSpeedEdit();
		OnKillfocusDelayEdit();
		OnRepeat();
		if (m_nFnKey == 24) { // SDM 1.5Test10.4
			AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_PROCESS_BATCH_COMMANDS, 0L);    // SDM 1.5Test8.5 resume batch command
		}
	}

	// SDM 1.06.6U6 bFnKey not valid for nSubMode = SubModeUndefined (this is a repeat command or stop command)
	if ((bFnKey) && (nSubMode != SubModeUndefined)) {
		// save the current player setting
		m_nOldVolume = m_nVolume;
		m_nOldSpeed = m_nSpeed;
		m_nOldDelay = m_nDelay;
		m_bOldRepeat = (m_bRepeat == TRUE);
		m_bFnKeySetting = true;
		m_nFnKey = nSubMode; // SDM 1.5Test10.4
		// get the function key setting (Fn key number is in nSubMode)
		CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
		CFnKeys * pFnKeys = pMainFrame->GetFnKeys(NULL);
		m_nVolume = pFnKeys->nVolume[nSubMode];
		m_nSpeed = pFnKeys->nSpeed[nSubMode];
		m_nDelay = pFnKeys->nDelay[nSubMode];
		m_bRepeat = pFnKeys->bRepeat[nSubMode];
		nSubMode = pFnKeys->nMode[nSubMode];

		/*
		* This is the order of the strings in the combo control
		* End Cursor->File End		     (0)  ID_PLAYBACK_ENCCUR_TO_FILEEND
		* Between Cursors                (1)  ID_PLAYBACK_CURSORS
		* Left Window->Begin Cursor      (2)  ID_PLAYBACK_LEFTWIN_TO_STARTCUR
		* Begin Cursor->Right Window     (3)  ID_PLAYBACK_STARTCUR_TO_RIGHTWIN
		* Left Window->End Cursor        (4)  ID_PLAYBACK_LEFTWIN_TO_ENDCUR
		* End Cursor->Right Window       (5)  ID_PLAYBACK_ENDCUR_TO_RIGHTWIN
		* Current Window                 (6)  ID_PLAYBACK_WINDOW
		* Entire File                    (7)  ID_PLAYBACK_FILE
		* Begin Cursor->File End		 (8)  ID_PLAYBACK_STARTCUR_TO_FILEEND
		*/
		TRACE("nSubMode=%d\n", nSubMode);
		switch (nSubMode) {
		case ID_PLAYBACK_ENCCUR_TO_FILEEND:
			m_nComboPlayMode = 0;
			break;
		case ID_PLAYBACK_CURSORS:
			m_nComboPlayMode = 1;
			break;
		case ID_PLAYBACK_LEFTWIN_TO_STARTCUR:
			m_nComboPlayMode = 2;
			break;
		case ID_PLAYBACK_STARTCUR_TO_RIGHTWIN:
			m_nComboPlayMode = 3;
			break;
		case ID_PLAYBACK_LEFTWIN_TO_ENDCUR:
			m_nComboPlayMode = 4;
			break;
		case ID_PLAYBACK_ENDCUR_TO_RIGHTWIN:
			m_nComboPlayMode = 5;
			break;
		case ID_PLAYBACK_WINDOW:
			m_nComboPlayMode = 6;
			break;
		default:
		case ID_PLAYBACK_FILE:
			m_nComboPlayMode = 7;
			break;
		case ID_PLAYBACK_STARTCUR_TO_FILEEND:
			m_nComboPlayMode = 8;
			break;
		}
		UpdateData(FALSE);
		OnKillfocusVolumeEdit();
		OnKillfocusSpeedEdit();
		OnKillfocusDelayEdit();
		OnRepeat();
	}
	if (nSubMode != SubModeUndefined) {
		m_nSubMode = nSubMode;    // save submode
	}

	switch (nSubMode) {
	case ID_PLAYBACK_FILE:
	case ID_PLAYBACK_STARTCUR_TO_FILEEND:
	case ID_PLAYBACK_ENCCUR_TO_FILEEND:
	case ID_PLAYBACK_CURSORS:
	case ID_PLAYBACK_LEFTWIN_TO_STARTCUR:
	case ID_PLAYBACK_STARTCUR_TO_RIGHTWIN:
	case ID_PLAYBACK_LEFTWIN_TO_ENDCUR:
	case ID_PLAYBACK_ENDCUR_TO_RIGHTWIN:
		m_pView->EnableScrolling(true);
		break;
	default:
	case ID_PLAYBACK_WINDOW:
		m_pView->EnableScrolling(false);
		break;
	}

	/*
	* This is the order of the strings in the combo control
	* End Cursor->File End		     (0)  ID_PLAYBACK_ENCCUR_TO_FILEEND
	* Between Cursors                (1)  ID_PLAYBACK_CURSORS
	* Left Window->Begin Cursor      (2)  ID_PLAYBACK_LEFTWIN_TO_STARTCUR
	* Begin Cursor->Right Window     (3)  ID_PLAYBACK_STARTCUR_TO_RIGHTWIN
	* Left Window->End Cursor        (4)  ID_PLAYBACK_LEFTWIN_TO_ENDCUR
	* End Cursor->Right Window       (5)  ID_PLAYBACK_ENDCUR_TO_RIGHTWIN
	* Current Window                 (6)  ID_PLAYBACK_WINDOW
	* Entire File                    (7)  ID_PLAYBACK_FILE
	* Begin Cursor->File End		 (8)  ID_PLAYBACK_STARTCUR_TO_FILEEND
	*/
	// set mode combobox, but only if not called for full size
	if (!bFullSize) {
		TRACE("m_nSubMode=%d\n", m_nSubMode);
		switch (m_nSubMode) {
		case ID_PLAYBACK_ENCCUR_TO_FILEEND:
			m_nComboPlayMode = 0;
			break;
		case ID_PLAYBACK_CURSORS:
			m_nComboPlayMode = 1;
			break;
		case ID_PLAYBACK_LEFTWIN_TO_STARTCUR:
			m_nComboPlayMode = 2;
			break;
		case ID_PLAYBACK_STARTCUR_TO_RIGHTWIN:
			m_nComboPlayMode = 3;
			break;
		case ID_PLAYBACK_LEFTWIN_TO_ENDCUR:
			m_nComboPlayMode = 4;
			break;
		case ID_PLAYBACK_ENDCUR_TO_RIGHTWIN:
			m_nComboPlayMode = 5;
			break;
		case ID_PLAYBACK_WINDOW:
			m_nComboPlayMode = 6;
			break;
		default:
		case ID_PLAYBACK_FILE:
			m_nComboPlayMode = 7;
			break;
		case ID_PLAYBACK_STARTCUR_TO_FILEEND:
			m_nComboPlayMode = 8;
			break;
		}
		UpdateData(FALSE);
	}
	if ((!m_bFullSize) && (bFullSize)) {
		SetPlayerFullSize();    // change the size
	}

	// No errors
	if (m_nMode == mode) {
		return TRUE;    // no change
	}

	// stop, whatever the player is doing
	if (m_pWave != NULL) {
		m_pWave->Stop();
	}

	SetTotalTime();
	SetPositionTime();
	DWORD dwSize, dwStart = 0;
	CSaApp * pApp = (CSaApp *)AfxGetApp(); // get pointer to application
	CSaDoc * pModel = (CSaDoc *)m_pDoc; // cast pointer to document
	switch (mode) {
	case PLAYING:
		if (m_nMode != PAUSED) {
			m_dwPlayPosition = 0;
		}
		m_nMode = PLAYING;
		m_play.Flash(FALSE);        // stop flashing Play button
		m_play.Push();              // push Play button
		m_pause.Release();          // release Pause button
		m_pause.EnableWindow(TRUE); // enable Pause button
		GetDlgItem(IDC_PLAYMODE)->EnableWindow(FALSE); // disable mode window
		// get pointer to document
		if (m_pWave != NULL) {
			BOOL bError = FALSE;
			TRACE("play m_nSubMode\n", m_nSubMode);
			switch (m_nSubMode) {
			case ID_PLAYBACK_STARTCUR_TO_FILEEND:
				dwStart = m_pView->GetStartCursorPosition();
				dwSize = pModel->GetDataSize() - dwStart;
				break;
			case ID_PLAYBACK_ENCCUR_TO_FILEEND:
				dwStart = m_pView->GetStopCursorPosition();
				dwSize = pModel->GetDataSize() - dwStart;
				break;
			case ID_PLAYBACK_FILE:
				dwSize = pModel->GetDataSize();
				break;
			case ID_PLAYBACK_WINDOW:
				dwStart = DWORD(m_pView->GetDataPosition(0));
				dwSize = m_pView->GetDataFrame();
				break;
			case ID_PLAYBACK_CURSORS:
				dwStart = m_pView->GetStartCursorPosition();
				dwSize = m_pView->GetStopCursorPosition() - dwStart;
				break;
			case ID_PLAYBACK_LEFTWIN_TO_STARTCUR:
				dwStart = DWORD(m_pView->GetDataPosition(0));
				dwSize = m_pView->GetStartCursorPosition();
				if (dwSize > dwStart) {
					dwSize -= dwStart;
				} else {
					bError = TRUE;
				}
				break;
			case ID_PLAYBACK_STARTCUR_TO_RIGHTWIN:
				dwStart = m_pView->GetStartCursorPosition();
				dwSize = DWORD(m_pView->GetDataPosition(0) + m_pView->GetDataFrame());
				if (dwSize > dwStart) {
					dwSize -= dwStart;
				} else {
					bError = TRUE;
				}
				break;
			case ID_PLAYBACK_LEFTWIN_TO_ENDCUR:
				dwStart = DWORD(m_pView->GetDataPosition(0));
				dwSize = m_pView->GetStopCursorPosition();
				if (dwSize > dwStart) {
					dwSize -= dwStart;
				} else {
					bError = TRUE;
				}
				break;
			case ID_PLAYBACK_ENDCUR_TO_RIGHTWIN:
				dwStart = m_pView->GetStopCursorPosition();
				dwSize = DWORD(m_pView->GetDataPosition(0)) + m_pView->GetDataFrame();
				if (dwSize > dwStart) {
					dwSize -= dwStart;
				} else {
					bError = TRUE;
				}
				break;
			case ID_PLAYBACK_SPECIFIC:
				dwSize = pModel->GetDataSize();
				if (pSpecific) {
					DWORD dwSpecificSize = pModel->GetBytesFromTime(pSpecific->end);
					DWORD dwSpecificStart = pModel->GetBytesFromTime(pSpecific->begin);

					if (dwSpecificStart < dwSize) {
						dwStart = dwSpecificStart;
					}

					if (dwSpecificSize < dwSize) {
						dwSize = dwSpecificSize;
						if (dwSize > dwStart) {
							dwSize -= dwStart;
						} else {
							dwSize = 0;
						}
					}
				}
				break;
			default:
				dwStart = dwSize = 0;
				break;
			}
			if (m_dwPlayPosition == 0) {
				m_dwPlayPosition = dwStart;
			}
			if (bError) {
				// play mode not playable
				pApp->ErrorMessage(IDS_ERROR_PLAYMODE);
			} else {
				SetDlgItemText(IDC_TIMERTEXT, _T("Current Time"));
				// SDM 1.5Test10.5
				m_pView->SetPlaybackPosition(dwStart);

				//TRACE("Post UPDATE_PLAYER 1\n");
				AfxGetMainWnd()->PostMessageW(WM_USER_UPDATE_PLAYER, 1, 0);

				bError = !m_pWave->Play(m_dwPlayPosition, dwStart + dwSize - m_dwPlayPosition, m_nVolume, m_nSpeed, m_pView, &m_NotifyObj);
			}

			if (bError) {
				// play not successfull
				m_nMode = STOPPED;
				// release Play button
				m_play.Release();
				// disable Pause button
				m_pause.EnableWindow(FALSE);
				// enable mode window
				GetDlgItem(IDC_PLAYMODE)->EnableWindow(TRUE);
				m_VUBar.Reset();
				// An error has occurred
				return FALSE;
			}
		}
		break;

	case PAUSED:
		if (m_pWave) {
			m_pWave->Stop();
		}
		m_nOldMode = m_nMode;
		m_nMode = PAUSED;
		m_pView->StopPlaybackTimer();
		m_pView->SetPlaybackFlash(true);
		m_pView->SelectSegment();
		// start flashing paused button
		m_play.Flash(TRUE);
		m_VUBar.SetVU(0);
		break;

	case STOPPED:
	{
		bool postUpdate = (m_nMode == PLAYING);
		bool wasIdle = (m_nMode == IDLE);
		m_nMode = STOPPED;
		if (m_pWave) {
			m_pWave->Stop();
		}
		m_dwPlayPosition = 0;
		SetPositionTime();
		m_pView->StopPlaybackTimer();
		if (!wasIdle) {
			// SDM 1.06.6U6 hide playback indicators
			m_pView->SetPlaybackPosition(0xffffffff);
		}

		if (postUpdate) {
			//TRACE("Post UPDATE_PLAYER 2\n");
			AfxGetMainWnd()->PostMessageW(WM_USER_UPDATE_PLAYER, 2, 0);
		}

		m_play.Release();                   // release Play button
		m_stop.Release();                   // release Stop button
		m_pause.Release();                  // release Pause button
		m_pause.EnableWindow(FALSE);        // disable Pause button
		GetDlgItem(IDC_PLAYMODE)->EnableWindow(TRUE); // disable mode window
		m_VUBar.Reset();
	}
	break;

	default:
		m_nMode = IDLE;
		if (m_pWave) {
			m_pWave->Stop();
		}
		m_play.Release();                   // release Play button
		m_stop.Release();                   // release Stop button
		m_pause.Release();                  // release Pause button
		m_pause.EnableWindow(FALSE);        // disable Pause button
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
void CDlgPlayer::BlockFinished(UINT nLevel, DWORD dwPosition, UINT nSpeed) {

	m_pView->SetPlaybackPosition(dwPosition, nSpeed);

	//TRACE("Post UPDATE_PLAYER 3\n");
	AfxGetMainWnd()->PostMessageW(WM_USER_UPDATE_PLAYER, 3, 0);

	// update the VU bar
	m_VUBar.SetVU((int)nLevel);
	// update the time
	m_dwPlayPosition = dwPosition;

	CSaDoc * pModel = (CSaDoc *)m_pDoc;
	DWORD dwBlockEnd = m_dwPlayPosition - pModel->GetSampleSize();
	double fDataSec = pModel->GetTimeFromBytes(dwBlockEnd);
	m_LEDPosTime.SetTime((int)fDataSec / 60, (int)(fDataSec * 10) % 600);


}

/***************************************************************************/
// CDlgPlayer::EndPlayback Playback finished
// The playback has been finished. Set player mode to stop.
/***************************************************************************/
void CDlgPlayer::EndPlayback() {
	if (m_nMode != IDLE) {
		SetPlayerMode(STOPPED, SubModeUndefined, m_bFullSize, m_bFnKeySetting);   // SDM 1.06.6U6
		if (m_bRepeat) {
			SetTimer(ID_TIMER_DELAY, m_nDelay, NULL);    // start repeating
		}
	}
}

/***************************************************************************/
// CDlgPlayer::ChangeView Change the parent view
// The parent view has changed. The dialogs caption text must be adapted and
// the playing time changes.
/***************************************************************************/
void CDlgPlayer::ChangeView(CSaView * pView) {

	if (m_pView == pView) {
		return;
	}
	// set file name in players caption
	m_pView = pView;                                // set the new pointer to the active view
	m_pDoc = pView->GetDocument();                  // get the pointer to the attached document
	CSaDoc * pModel = (CSaDoc *)m_pDoc;
	CString szTitle = pModel->GetFilenameFromTitle().c_str(); // load the document title
	CString szCaption;
	GetWindowText(szCaption);       // get the current caption string
	int nFind = szCaption.Find(' ');
	if (nFind != -1) {
		szCaption = szCaption.Left(nFind);    // extract part left of first space
	}
	szCaption += " - " + szTitle;   // add new document title
	SetWindowText(szCaption);       // write the new caption string
	if (pModel->GetDataSize() == 0) { // no data to play
		SetPlayerMode(IDLE, m_nSubMode, m_bFullSize);
	} else {
		// update the time displays
		SetPlayerMode(STOPPED, m_nSubMode, m_bFullSize);
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
BOOL CDlgPlayer::OnInitDialog() {

	CDialog::OnInitDialog();
	m_pWave->GetOutDevice()->ConnectMixer(this);

	// preset the player modes
	m_nMode = IDLE;
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
	CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
	CSaDoc * pSaDoc = pMainWnd->GetCurrDoc();
	CProcessFragments * pFragmenter = pSaDoc->GetFragments();
	if (pFragmenter) {
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
	CMainFrame * pMainFrameWnd = (CMainFrame *)AfxGetMainWnd();
	CSaView * pView = (CSaView *)pMainFrameWnd->GetCurrSaView();
	ChangeView(pView); // setup for this view
	// get dialog sizes for small and full size views
	GetWindowRect(rWnd); // get screen coordinates of dialog
	m_rSize.right = rWnd.Width();
	m_rSize.bottom = rWnd.Height();
	m_rSize.left = 0;
	m_rSize.top = 0;
	CWnd * pWnd = GetDlgItem(IDC_MIXER);
	if (pWnd) {
		pWnd->EnableWindow(m_pWave->GetOutDevice()->ShowMixer(FALSE));
	}

	// hide dialog
	MoveWindow(rWnd.left, rWnd.top, 0, 0);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************/
// CDlgPlayer::OnPlay Button play hit
/***************************************************************************/
void CDlgPlayer::OnPlay() {
	UpdateData(TRUE);
	if (m_nMode != PLAYING) {
		SetPlayerMode(PLAYING, m_nSubMode, m_bFullSize);
	}
}

/***************************************************************************/
// CDlgPlayer::OnStop Button stop hit
/***************************************************************************/
void CDlgPlayer::OnStop() {
	// RLJ, 1 Oct 1999
	// Added the following 'if' statement so that when the Stop button is hit,
	// the next Repeat is canceled. Since it leaves the 'Repeat' checkbox
	// unchanged, however, future repeats can still be scheduled simply by
	// depressing the Play button.
	if (m_bRepeat) {
		m_bRepeat = FALSE;
	}

	if (m_nMode != STOPPED) {
		SetPlayerMode(STOPPED, m_nSubMode, m_bFullSize);
	} else {
		m_stop.Release();
	}
}

/***************************************************************************/
// CDlgPlayer::OnPause Button pause hit
/***************************************************************************/
void CDlgPlayer::OnPause() {
	if (m_nMode != PAUSED) {
		SetPlayerMode(PAUSED, m_nSubMode, m_bFullSize);
	} else {
		SetPlayerMode(m_nOldMode, m_nSubMode, m_bFullSize);
	}
}

/***************************************************************************/
// CDlgPlayer::OnMixerControlChange Mixer has changed volume settings
/***************************************************************************/
LRESULT CDlgPlayer::OnMixerControlChange(WPARAM, LPARAM) {
	BOOL bResult = FALSE;

	m_nVolume = m_pWave->GetVolume(bResult);

	if (bResult) {
		SetDlgItemInt(IDC_VOLUMEEDIT, m_nVolume, TRUE);
		m_SliderVolume.SetPosition(m_nVolume);
	}

	return 0;
}

/***************************************************************************/
// CDlgPlayer::OnVolumeSlide Volume slider position changed
/***************************************************************************/
void CDlgPlayer::OnVolumeSlide() {
	m_nVolume = m_SliderVolume.GetPosition();
	SetDlgItemInt(IDC_VOLUMEEDIT, m_SliderVolume.GetPosition(), TRUE);
	m_pWave->SetVolume(m_nVolume);
}

/***************************************************************************/
// CDlgPlayer::OnVolumeScroll Volume spin control hit
/***************************************************************************/
void CDlgPlayer::OnVolumeScroll() {
	m_nVolume = GetDlgItemInt(IDC_VOLUMEEDIT, NULL, TRUE);
	if (m_SpinVolume.UpperButtonClicked()) {
		m_nVolume++;
	} else {
		m_nVolume--;
	}
	if ((int)m_nVolume < 0) {
		m_nVolume = 0;
	}
	if (m_nVolume > 100) {
		m_nVolume = 100;
	}
	SetDlgItemInt(IDC_VOLUMEEDIT, m_nVolume, TRUE);
	m_SliderVolume.SetPosition(m_nVolume);
	m_pWave->SetVolume(m_nVolume);
}

/***************************************************************************/
// CDlgPlayer::OnKillfocusVolumeEdit Volume edited
/***************************************************************************/
void CDlgPlayer::OnKillfocusVolumeEdit() {
	m_nVolume = GetDlgItemInt(IDC_VOLUMEEDIT, NULL, TRUE);
	if ((int)m_nVolume < 0) {
		m_nVolume = 0;
	}
	if (m_nVolume > 100) {
		m_nVolume = 100;
	}
	SetDlgItemInt(IDC_VOLUMEEDIT, m_nVolume, TRUE);
	m_SliderVolume.SetPosition(m_nVolume);
	m_pWave->SetVolume(m_nVolume);
}

/***************************************************************************/
// CDlgPlayer::EnableSpeedSlider Enable/Disable speed slider
/***************************************************************************/
void CDlgPlayer::EnableSpeedSlider(BOOL bState) {
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
void CDlgPlayer::OnSpeedSlide() {
	UINT nSpeed = m_SliderSpeed.GetPosition();
	if (nSpeed > 50) {
		m_nSpeed = 100 + (nSpeed - 50) * 233 / 50;
	} else {
		m_nSpeed = 10 + nSpeed * 90 / 50;
	}
	SetDlgItemInt(IDC_SPEEDEDIT, m_nSpeed, TRUE);
	m_pWave->SetSpeed(m_nSpeed);
}

/***************************************************************************/
// CDlgPlayer::OnSpeedScroll Speed spin control hit
/***************************************************************************/
void CDlgPlayer::OnSpeedScroll() {
	m_nSpeed = GetDlgItemInt(IDC_SPEEDEDIT, NULL, TRUE);
	if (m_SpinSpeed.UpperButtonClicked()) {
		m_nSpeed++;
	} else {
		m_nSpeed--;
	}
	if ((int)m_nSpeed < 10) {
		m_nSpeed = 10;
	}
	if (m_nSpeed > 333) {
		m_nSpeed = 333;
	}
	SetDlgItemInt(IDC_SPEEDEDIT, m_nSpeed, TRUE);
	if (m_nSpeed > 100) {
		m_SliderSpeed.SetPosition(50 + 50 * (m_nSpeed - 100) / 233);
	} else {
		m_SliderSpeed.SetPosition(50 * (m_nSpeed - 10) / 90);
	}
	m_pWave->SetSpeed(m_nSpeed);
}

/***************************************************************************/
// CDlgPlayer::OnKillfocusSpeedEdit Speed edited
/***************************************************************************/
void CDlgPlayer::OnKillfocusSpeedEdit() {
	m_nSpeed = GetDlgItemInt(IDC_SPEEDEDIT, NULL, TRUE);
	if ((int)m_nSpeed < 10) {
		m_nSpeed = 10;
	}
	if (m_nSpeed > 333) {
		m_nSpeed = 333;
	}
	SetDlgItemInt(IDC_SPEEDEDIT, m_nSpeed, TRUE);
	if (m_nSpeed > 100) {
		m_SliderSpeed.SetPosition(50 + 50 * (m_nSpeed - 100) / 233);
	} else {
		m_SliderSpeed.SetPosition(50 * (m_nSpeed - 10) / 90);
	}
	m_pWave->SetSpeed(m_nSpeed);
}

/***************************************************************************/
// CDlgPlayer::OnDelaySlide Delay slider position changed
/***************************************************************************/
void CDlgPlayer::OnDelaySlide() {
	m_nDelay = m_SliderDelay.GetPosition() * 100;
	if (m_nDelay > 9999) {
		m_nDelay = 9999;
	}
	SetDlgItemInt(IDC_DELAYEDIT, m_nDelay, TRUE);
}

/***************************************************************************/
// CDlgPlayer::OnDelayScroll Delay spin control hit
/***************************************************************************/
void CDlgPlayer::OnDelayScroll() {
	m_nDelay = GetDlgItemInt(IDC_DELAYEDIT, NULL, TRUE);
	if (m_SpinDelay.UpperButtonClicked()) {
		m_nDelay += 100;
	} else {
		if (m_nDelay == 9999) {
			m_nDelay -= 99;
		} else {
			if (m_nDelay > 100) {
				m_nDelay -= 100;
			} else {
				m_nDelay = 0;
			}
		}
	}
	if (m_nDelay > 9999) {
		m_nDelay = 9999;
	}
	SetDlgItemInt(IDC_DELAYEDIT, m_nDelay, TRUE);
	if (m_nDelay == 9999) {
		m_SliderDelay.SetPosition(100);
	} else {
		m_SliderDelay.SetPosition(m_nDelay / 100);
	}
}

/***************************************************************************/
// CDlgPlayer::OnKillfocusDelayEdit Delay edited
/***************************************************************************/
void CDlgPlayer::OnKillfocusDelayEdit() {
	m_nDelay = GetDlgItemInt(IDC_DELAYEDIT, NULL, TRUE);
	if ((int)m_nDelay < 0) {
		m_nDelay = 0;
	}
	if (m_nDelay > 9999) {
		m_nDelay = 9999;
	}
	SetDlgItemInt(IDC_DELAYEDIT, m_nDelay, TRUE);
	if (m_nDelay == 9999) {
		m_SliderDelay.SetPosition(100);
	} else {
		m_SliderDelay.SetPosition(m_nDelay / 100);
	}
}

/***************************************************************************/
// CDlgPlayer::OnRepeat Replay repeat clicked
/***************************************************************************/
void CDlgPlayer::OnRepeat() {
	UpdateData(TRUE);
	if (m_bRepeat) {
		// enable delay editing
		m_SliderDelay.EnableWindow(TRUE); // enable delay slider
		m_SpinDelay.EnableWindow(TRUE); // enable delay spin control
		GetDlgItem(IDC_DELAYEDIT)->EnableWindow(TRUE); // enable delay edit control
		GetDlgItem(IDC_DELAYTEXT)->EnableWindow(TRUE); // enable delay dimension text
	} else {
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
void CDlgPlayer::OnSelchangePlaymode() {

	UpdateData(TRUE);
	UINT nSubMode = ID_PLAYBACK_FILE;
	m_pView->EnableScrolling(false);
	/*
	* This is the order of the strings in the combo control
	* End Cursor->File End		     (0)  ID_PLAYBACK_ENCCUR_TO_FILEEND
	* Between Cursors                (1)  ID_PLAYBACK_CURSORS
	* Left Window->Begin Cursor      (2)  ID_PLAYBACK_LEFTWIN_TO_STARTCUR
	* Begin Cursor->Right Window     (3)  ID_PLAYBACK_STARTCUR_TO_RIGHTWIN
	* Left Window->End Cursor        (4)  ID_PLAYBACK_LEFTWIN_TO_ENDCUR
	* End Cursor->Right Window       (5)  ID_PLAYBACK_ENDCUR_TO_RIGHTWIN
	* Current Window                 (6)  ID_PLAYBACK_WINDOW
	* Entire File                    (7)  ID_PLAYBACK_FILE
	* Begin Cursor->File End		 (8)  ID_PLAYBACK_STARTCUR_TO_FILEEND
	*/
	TRACE("m_nComboPlayMode=%d\n", m_nComboPlayMode);
	switch (m_nComboPlayMode) {
	case 0:
		nSubMode = ID_PLAYBACK_ENCCUR_TO_FILEEND;
		m_pView->EnableScrolling(true);
		break;
	case 1:
		nSubMode = ID_PLAYBACK_CURSORS;
		m_pView->EnableScrolling(true);
		break;
	case 2:
		nSubMode = ID_PLAYBACK_LEFTWIN_TO_STARTCUR;
		m_pView->EnableScrolling(true);
		break;
	case 3:
		nSubMode = ID_PLAYBACK_STARTCUR_TO_RIGHTWIN;
		m_pView->EnableScrolling(true);
		break;
	case 4:
		nSubMode = ID_PLAYBACK_LEFTWIN_TO_ENDCUR;
		m_pView->EnableScrolling(true);
		break;
	case 5:
		nSubMode = ID_PLAYBACK_ENDCUR_TO_RIGHTWIN;
		m_pView->EnableScrolling(true);
		break;
	case 6:
		nSubMode = ID_PLAYBACK_WINDOW;
		break;
	default:
	case 7:
		nSubMode = ID_PLAYBACK_FILE;
		m_pView->EnableScrolling(true);
		break;
	case 8:
		nSubMode = ID_PLAYBACK_STARTCUR_TO_FILEEND;
		m_pView->EnableScrolling(true);
		break;
	}
	SetPlayerMode(STOPPED, nSubMode, m_bFullSize);
	SetTotalTime();
	SetPositionTime();
}

/***************************************************************************/
// CDlgPlayer::OnOK Button ok hit
// If the focus is on the editable controls, just change the focus to the
// next control, otherwise close the dialog with ok.
/***************************************************************************/
void CDlgPlayer::OnOK() {
	CWnd * pWnd = GetFocus(); // who has the focus?
	if ((pWnd->GetDlgCtrlID() == IDC_VOLUMEEDIT) || (pWnd->GetDlgCtrlID() == IDC_SPEEDEDIT)) {
		NextDlgCtrl();    // focus is on edit control, so move to the next control on enter
	} else {
		OnClose();
	}
}

/***************************************************************************/
// CDlgPlayer::OnCancel Button cancel hit
/***************************************************************************/
void CDlgPlayer::OnCancel() {
	if (IsPlaying()) {
		OnStop();
	} else {
		OnClose();
	}
}

/***************************************************************************/
// CDlgPlayer::OnClose Close the dialog
/***************************************************************************/
void CDlgPlayer::OnClose() {
	OnStop(); // stop the player
	CDialog::OnClose();
	bLaunched = false;
	if (m_pWave != NULL) {
		delete m_pWave;
		m_pWave = NULL;
	}
	DestroyWindow();
}

/***************************************************************************/
// CDlgPlayer::OnTimer Timer event, repeat playback
/***************************************************************************/
void CDlgPlayer::OnTimer(UINT nIDEvent) {
	KillTimer(ID_TIMER_DELAY);
	if ((m_nMode == STOPPED) && (m_bRepeat)) {
		SetPlayerMode(PLAYING, SubModeUndefined, m_bFullSize, m_bFnKeySetting);     // SDM 1.06.6U6
	}
	CWnd::OnTimer(nIDEvent);
}

/***************************************************************************/
// CDlgPlayer::OnSetupFnKeys Setup function keys
/***************************************************************************/
LRESULT CDlgPlayer::OnSetupFnKeys(WPARAM /*wParam*/, LPARAM) {
	OnSetup();
	return 0;
}

/***************************************************************************/
// CDlgPlayer::OnSetup Button setup hit
/***************************************************************************/
void CDlgPlayer::OnSetup() {
	if (m_pWave) {
		m_pWave->Stop();    // stop recording
	}
	SetPlayerMode(STOPPED, m_nSubMode, m_bFullSize);
	m_VUBar.SetVU(0); // reset the VU bar
	// create the dialog
	m_bTestRunning = TRUE;

	CDlgFnKeys dlg;
	dlg.m_pView = m_pView;
	dlg.m_pDoc = m_pDoc;
	dlg.DoModal();
	m_bTestRunning = FALSE;
}

/***************************************************************************/
// CDlgPlayer::OnPlayControls Show Volume Controls mixer
/***************************************************************************/
void CDlgPlayer::OnPlayControls() {
	m_pWave->GetOutDevice()->ConnectMixer(this);
	m_pWave->GetOutDevice()->ShowMixer();
}

/***************************************************************************/
// CDlgPlayer::OnHelpPlayer Call Player help
/***************************************************************************/
void CDlgPlayer::OnHelpPlayer() {
	// create the pathname
	CString szPath = AfxGetApp()->m_pszHelpFilePath;
	szPath += "::/User_Interface/Menus/Playback/Player.htm";
	::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

void CDlgPlayer::BlockStored(UINT /*nLevel*/, DWORD /*dwPosition*/, BOOL * /* bSaveOverride*/) {
	ASSERT(FALSE);
}

void CDlgPlayer::StoreFailed() {
	ASSERT(FALSE);
}

BPTR CDlgPlayer::GetWaveData(DWORD /*dwPlayPosition*/, DWORD /*dwDataSize*/) {
	return NULL;
}

bool CDlgPlayer::IsPlaying() {
	return ((m_nMode == PLAYING) || (m_bFnKeySetting));   // return TRUE if player is playing
}

bool CDlgPlayer::IsPaused() {
	return (m_nMode == PAUSED);                         // return TRUE if player is playing
}

bool CDlgPlayer::IsFullSize() {
	return m_bFullSize;         // return TRUE if player has full size
}

bool CDlgPlayer::IsTestRun() {
	return m_bTestRunning;      // return TRUE if function key dialog open
}

bool CDlgPlayer::IsLaunched() {
	return bLaunched;
}

UINT CDlgPlayer::GetSubmode() {
	return m_nSubMode;
}

