#include "stdafx.h"
#include "DlgFnKeys.h"
#include "sa_plot.h"
#include "sa_graph.h"
#include "fileInformation.h"
#include "sa_view.h"
#include "sa.h"
#include "mainfrm.h"
#include "FileUtils.h"
#include "WaveOutDevice.h"
#include "WaveInDevice.h"
#include "Process\sa_p_fra.h"
#include "Process\Process.h"
#include "settings\obstream.h"

//###########################################################################
// CDlgFnKeys dialog
// Displays the controls to setup the function keys.

BEGIN_MESSAGE_MAP(CDlgFnKeys, CDialog)
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
END_MESSAGE_MAP()

CDlgFnKeys::CDlgFnKeys(CWnd * pParent) : CDialog(CDlgFnKeys::IDD, pParent)
{

    m_nDelay = 1000;
    m_nSpeed = 50;
    m_bRepeat = FALSE;
    m_nPlayMode = 4;
    m_bNoTest = FALSE;
    m_bTestRunning = FALSE;
    m_pDoc = NULL;
    m_pView = NULL;
    m_NotifyObj.Attach(this); // attach notify object
    m_pWave = new CWave;      // create CWave object
    BOOL bResult = FALSE;
    m_nVolume = m_pWave->GetVolume(bResult);
    m_nSelection = -1;
}

/***************************************************************************/
// CDlgFnKeys::DoDataExchange Data exchange
/***************************************************************************/
void CDlgFnKeys::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_DELAYEDIT, m_nDelay);
    DDV_MinMaxUInt(pDX, m_nDelay, 0, 9999);
    DDX_Text(pDX, IDC_SPEEDEDIT, m_nSpeed);
    DDV_MinMaxUInt(pDX, m_nSpeed, 10, 400);
    DDX_Text(pDX, IDC_VOLUMEEDIT, m_nVolume);
    DDV_MinMaxUInt(pDX, m_nVolume, 0, 100);
    DDX_Check(pDX, IDC_REPEAT, m_bRepeat);
    DDX_CBIndex(pDX, IDC_PLAYMODE, m_nPlayMode);
}

/***************************************************************************/
// CDlgFnKeys::EndPlayback Playback finished
/***************************************************************************/
void CDlgFnKeys::EndPlayback()
{

    if (m_pWave)
    {
        m_pWave->Stop();
    }
    if (m_bRepeat)
    {
        SetTimer(ID_TIMER_DELAY, m_nDelay, NULL);    // start repeating
    }
    else
    {
        OnTest();
    }
}

/***************************************************************************/
// CDlgFnKeys::OnInitDialog Dialog initialization
// The dialog is centered over the main frame window.
/***************************************************************************/
BOOL CDlgFnKeys::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_pWave->GetOutDevice()->ConnectMixer(this);
    // save the caption of the test button
    CWnd * pWnd = GetDlgItem(IDC_FNTEST);
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
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    pMainWnd->GetFnKeys(&m_fnKeys);
    // fill up the list box
    CListBox * pLB = (CListBox *)GetDlgItem(IDC_FNLIST);
    TCHAR szText[4];
    for (int nLoop = 1; nLoop <= 12; nLoop++)
    {
        swprintf_s(szText, _countof(szText), _T("F%u"), nLoop);
        pLB->AddString(szText);
    }
    pLB->SetCurSel(0);
    // disable delay editing
    m_SliderDelay.EnableWindow(FALSE); // disable delay slider
    m_SpinDelay.EnableWindow(FALSE); // disable delay spin control
    GetDlgItem(IDC_DELAYEDIT)->EnableWindow(FALSE); // disable delay edit control
    GetDlgItem(IDC_DELAYTEXT)->EnableWindow(FALSE); // disable delay dimension text
    OnSelchangeFnlist();
    if (m_bNoTest)
    {
        GetDlgItem(IDC_FNTEST)->EnableWindow(FALSE);    // disable test run
    }
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
    m_nVolume = m_pWave->GetVolume(bResult);
    if (bResult)
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
    if (m_SpinVolume.UpperButtonClicked())
    {
        m_nVolume++;
    }
    else
    {
        m_nVolume--;
    }
    if ((int)m_nVolume < 0)
    {
        m_nVolume = 0;
    }
    if (m_nVolume > 100)
    {
        m_nVolume = 100;
    }
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
    if ((int)m_nVolume < 0)
    {
        m_nVolume = 0;
    }
    if (m_nVolume > 100)
    {
        m_nVolume = 100;
    }
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
    if (nSpeed > 50)
    {
        m_nSpeed = 100 + (nSpeed - 50) * 233 / 50;
    }
    else
    {
        m_nSpeed = 10 + nSpeed * 90 / 50;
    }
    SetDlgItemInt(IDC_SPEEDEDIT, m_nSpeed, TRUE);
    m_pWave->SetSpeed(m_nSpeed);
}

/***************************************************************************/
// CDlgFnKeys::OnSpeedScroll Speed spin control hit
/***************************************************************************/
void CDlgFnKeys::OnSpeedScroll()
{
    m_nSpeed = GetDlgItemInt(IDC_SPEEDEDIT, NULL, TRUE);
    if (m_SpinSpeed.UpperButtonClicked())
    {
        m_nSpeed++;
    }
    else
    {
        m_nSpeed--;
    }
    if (m_nSpeed > 333)
    {
        m_nSpeed = 333;
    }
    if (m_nSpeed < 10)
    {
        m_nSpeed = 10;
    }
    SetDlgItemInt(IDC_SPEEDEDIT, m_nSpeed, TRUE);
    if (m_nSpeed > 100)
    {
        m_SliderSpeed.SetPosition(50 + 50 * (m_nSpeed - 100) / 233);
    }
    else
    {
        m_SliderSpeed.SetPosition(50 * (m_nSpeed - 10) / 90);
    }
    m_pWave->SetSpeed(m_nSpeed);
}

/***************************************************************************/
// CDlgFnKeys::OnKillfocusSpeedEdit Speed edited
/***************************************************************************/
void CDlgFnKeys::OnKillfocusSpeedEdit()
{
    m_nSpeed = GetDlgItemInt(IDC_SPEEDEDIT, NULL, TRUE);
    if ((int)m_nSpeed < 10)
    {
        m_nSpeed = 10;
    }
    if (m_nSpeed > 333)
    {
        m_nSpeed = 333;
    }
    SetDlgItemInt(IDC_SPEEDEDIT, m_nSpeed, TRUE);
    if (m_nSpeed > 100)
    {
        m_SliderSpeed.SetPosition(50 + 50 * (m_nSpeed - 100) / 233);
    }
    else
    {
        m_SliderSpeed.SetPosition(50 * (m_nSpeed - 10) / 90);
    }
    m_pWave->SetSpeed(m_nSpeed);
}

/***************************************************************************/
// CDlgFnKeys::OnDelaySlide Delay slider position changed
/***************************************************************************/
void CDlgFnKeys::OnDelaySlide()
{
    m_nDelay = m_SliderDelay.GetPosition() * 100;
    if (m_nDelay > 9999)
    {
        m_nDelay = 9999;
    }
    SetDlgItemInt(IDC_DELAYEDIT, m_nDelay, TRUE);
}

/***************************************************************************/
// CDlgFnKeys::OnDelayScroll Delay spin control hit
/***************************************************************************/
void CDlgFnKeys::OnDelayScroll()
{
    m_nDelay = GetDlgItemInt(IDC_DELAYEDIT, NULL, TRUE);
    if (m_SpinDelay.UpperButtonClicked())
    {
        m_nDelay += 100;
    }
    else
    {
        if (m_nDelay == 9999)
        {
            m_nDelay -= 99;
        }
        else
        {
            if (m_nDelay > 100)
            {
                m_nDelay -= 100;
            }
            else
            {
                m_nDelay = 0;
            }
        }
    }
    if (m_nDelay > 9999)
    {
        m_nDelay = 9999;
    }
    SetDlgItemInt(IDC_DELAYEDIT, m_nDelay, TRUE);
    if (m_nDelay == 9999)
    {
        m_SliderDelay.SetPosition(100);
    }
    else
    {
        m_SliderDelay.SetPosition(m_nDelay / 100);
    }
}

/***************************************************************************/
// CDlgFnKeys::OnKillfocusDelayEdit Delay edited
/***************************************************************************/
void CDlgFnKeys::OnKillfocusDelayEdit()
{
    m_nDelay = GetDlgItemInt(IDC_DELAYEDIT, NULL, TRUE);
    if ((int)m_nDelay < 0)
    {
        m_nDelay = 0;
    }
    if (m_nDelay > 9999)
    {
        m_nDelay = 9999;
    }
    SetDlgItemInt(IDC_DELAYEDIT, m_nDelay, TRUE);
    if (m_nDelay == 9999)
    {
        m_SliderDelay.SetPosition(100);
    }
    else
    {
        m_SliderDelay.SetPosition(m_nDelay / 100);
    }
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
            m_fnKeys.nMode[m_nSelection] = ID_PLAYBACK_CURSORS;
            break;
        case 1:
            m_fnKeys.nMode[m_nSelection] = ID_PLAYBACK_LTOSTART;
            break;
        case 2:
            m_fnKeys.nMode[m_nSelection] = ID_PLAYBACK_STARTTOR;
            break;
        case 3:
            m_fnKeys.nMode[m_nSelection] = ID_PLAYBACK_LTOSTOP;
            break;
        case 4:
            m_fnKeys.nMode[m_nSelection] = ID_PLAYBACK_STOPTOR;
            break;
        case 5:
            m_fnKeys.nMode[m_nSelection] = ID_PLAYBACK_WINDOW;
            break;
        case 6:
            m_fnKeys.nMode[m_nSelection] = ID_PLAYBACK_FILE;
            break;
        }
    }
    CListBox * pLB = (CListBox *)GetDlgItem(IDC_FNLIST);
    m_nSelection = pLB->GetCurSel();
    m_nDelay = m_fnKeys.nDelay[m_nSelection];
    m_nSpeed = m_fnKeys.nSpeed[m_nSelection];
    m_nVolume = m_fnKeys.nVolume[m_nSelection];
    m_bRepeat = m_fnKeys.bRepeat[m_nSelection];
    switch (m_fnKeys.nMode[m_nSelection])
    {
    case ID_PLAYBACK_CURSORS:
        m_nPlayMode = 0;
        break;
    case ID_PLAYBACK_LTOSTART:
        m_nPlayMode = 1;
        break;
    case ID_PLAYBACK_STARTTOR:
        m_nPlayMode = 2;
        break;
    case ID_PLAYBACK_LTOSTOP:
        m_nPlayMode = 3;
        break;
    case ID_PLAYBACK_STOPTOR:
        m_nPlayMode = 4;
        break;
    case ID_PLAYBACK_WINDOW:
        m_nPlayMode = 5;
        break;
    case ID_PLAYBACK_FILE:
        m_nPlayMode = 6;
        break;
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
    if (m_pWave)
    {
        m_pWave->Stop();
    }
    UpdateData(TRUE);
    OnSelchangeFnlist();
    // save the function key setup
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
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
    if (m_pWave)
    {
        m_pWave->Stop();
    }
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
    KillTimer(ID_TIMER_DELAY);
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
    CWnd * pWnd = GetDlgItem(IDC_FNTEST);
    CSaApp * pApp = (CSaApp *)AfxGetApp(); // get pointer to application
    if (m_bTestRunning)
    {
        if (m_pWave)
        {
            m_pWave->Stop();
        }
        KillTimer(ID_TIMER_DELAY);
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
        CSaView * pView = (CSaView *)m_pView; // cast pointer to view
        CSaDoc * pDoc = (CSaDoc *)m_pDoc; // cast pointer to document
        WORD wSmpSize = (WORD)pDoc->GetSampleSize();
        BOOL bError = FALSE;
        switch (m_nPlayMode)
        {
        case 0:
            dwStart = pView->GetStartCursorPosition();
            dwSize = pView->GetStopCursorPosition() - dwStart + wSmpSize;
            break;
        case 1:
            dwStart = DWORD(pView->GetDataPosition(0));
            dwSize = pView->GetStartCursorPosition();
            if (dwSize > dwStart)
            {
                dwSize -= dwStart;
            }
            else
            {
                bError = TRUE;
            }
            break;
        case 2:
            dwStart = pView->GetStartCursorPosition();
            dwSize = DWORD(pView->GetDataPosition(0) + pView->GetDataFrame());
            if (dwSize > dwStart)
            {
                dwSize -= dwStart;
            }
            else
            {
                bError = TRUE;
            }
            break;
        case 3:
            dwStart = DWORD(pView->GetDataPosition(0));
            dwSize = pView->GetStopCursorPosition();
            if (dwSize > dwStart)
            {
                dwSize -= dwStart;
            }
            else
            {
                bError = TRUE;
            }
            break;
        case 4:
            dwStart = pView->GetStopCursorPosition();
            dwSize = DWORD(pView->GetDataPosition(0) + pView->GetDataFrame());
            if (dwSize > dwStart)
            {
                dwSize -= dwStart;
            }
            else
            {
                bError = TRUE;
            }
            break;
        case 5:
            dwStart = DWORD(pView->GetDataPosition(0));
            dwSize = pView->GetDataFrame();
            break;
        case 6:
            dwSize = pDoc->GetDataSize();
            break;
        default:
            dwStart = dwSize = 0;
            break;
        }
        if (bError)
        {
            pApp->ErrorMessage(IDS_ERROR_PLAYMODE);    // play mode not playable
        }
        else
        {
            bError = !m_pWave->Play(dwStart, dwSize, m_nVolume, m_nSpeed, m_pView, &m_NotifyObj);
        }
        if (bError)
        {
            m_bTestRunning = FALSE;
            pWnd->SetWindowText(m_szTest); // set back the original Test button caption
        }
    }
    // display pending error messages
    pApp->DisplayMessages();
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

void CDlgFnKeys::BlockFinished(UINT /*nLevel*/, DWORD /*dwPosition*/, UINT)
{
    ASSERT(FALSE);
}

void CDlgFnKeys::BlockStored(UINT /*nLevel*/, DWORD /*dwPosition*/, BOOL * /*bSaveOverride*/)
{
    ASSERT(FALSE);
}

void CDlgFnKeys::StoreFailed()
{
    ASSERT(FALSE);
}

HPSTR CDlgFnKeys::GetWaveData(DWORD /*dwPlayPosition*/, DWORD /*dwDataSize*/)
{
    ASSERT(FALSE);
    return NULL;
}

void CDlgFnKeys::NoTest()
{
    m_bNoTest = TRUE;   // don't allow test run
}
