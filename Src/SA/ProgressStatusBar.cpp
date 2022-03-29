/////////////////////////////////////////////////////////////////////////////
// sa_sbar.cpp:
// Implementation of the CProgressStatusBar class.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
//
// Revision History:
//
// 08/30/2000 - DDO I fixed a problem in which the scale and position pane
//                  symbols were forced to take opposite states. That was
//                  fine as long as one of them is to be shown. But it fails
//                  to work correctly when both are turned off.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "mainfrm.h"
#include "sa_doc.h"
#include "ProgressStatusBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define SB_FONT_SIZE  -10 // status bar font size

//###########################################################################
// CProgressStatusBar
// Customized status bar class for displaying progress information. The base
// class is the general status bar class.

/////////////////////////////////////////////////////////////////////////////
// CProgressStatusBar message map

BEGIN_MESSAGE_MAP(CProgressStatusBar, CStatusBar)
END_MESSAGE_MAP()

/***************************************************************************/
// CProgressStatusBar::CProgressStatusBar Constructor
/***************************************************************************/
CProgressStatusBar::CProgressStatusBar() {
	m_pFont = nullptr;
    isPrinting = false;
    m_nInMessageLoop = 0;
}

/***************************************************************************/
// CProgressStatusBar::~CProgressStatusBar Destructor
/***************************************************************************/
CProgressStatusBar::~CProgressStatusBar() {
    if (m_pFont) {
        delete m_pFont;
        m_pFont = nullptr;
    }
}

/***************************************************************************/
// CProgressStatusBar::MessageLoop Do windows message loop
// This function enables the escape key down message to come through long
// processing. Each call will allow to do windows key message processing
// of MFC for this message.
/***************************************************************************/
void CProgressStatusBar::MessageLoop() {

    // This will allow first call to succeed
    static volatile DWORD dwTickLast = 0;
    // MS since system start
    DWORD dwThis = GetTickCount();
    if (dwThis - dwTickLast > 90) {
        // Update when timer has expired
        dwTickLast = dwThis; 
        MSG msg;
        m_nInMessageLoop++;
        // This is risky so only do it if the escape key is pressed
        if (GetAsyncKeyState(VK_ESCAPE) < 0) {
            while (::PeekMessage(&msg, NULL, WM_KEYDOWN, WM_KEYDOWN, PM_NOREMOVE)) {
                AfxGetApp()->PumpMessage();
            }
            if (!owners.empty()) {
              owners.front()->CancelProcess();
            }
            return;
        }
        // let MFC do its idle processing
        LONG lIdle = 0L;
        while (AfxGetApp()->OnIdle(lIdle++));
        m_nInMessageLoop--;
    }
}

/***************************************************************************/
// CProgressStatusBar::Init Initialisation
// Setting up the font and the pane sizes of the status bar.
/***************************************************************************/
void CProgressStatusBar::Init() {

    // statusbar font initialisation
    LOGFONT logFont;
    m_pFont = GetFont(); // get normal used font
    m_pFont->GetObject(sizeof(LOGFONT), (void *)&logFont); // fill up logFont
    CClientDC dc(this);  // used to calculate the font size
    logFont.lfHeight = -::MulDiv(SB_FONT_SIZE, dc.GetDeviceCaps(LOGPIXELSY), 72);
    logFont.lfCharSet = ANSI_CHARSET;
    logFont.lfQuality = DEFAULT_QUALITY;
    logFont.lfClipPrecision = CLIP_LH_ANGLES | CLIP_STROKE_PRECIS;
    logFont.lfPitchAndFamily = FF_SWISS;
    m_pFont = new CFont;  // create new font
    m_pFont->CreateFontIndirect(&logFont);
    SetFont(m_pFont, TRUE); // set new font in bar
    SetPaneInfo(0, ID_PROGRESSPANE_1, SBPS_STRETCH, 0);
    SetPaneInfo(1, ID_PROGRESSPANE_2, SBPS_NORMAL , PROGRESSPANE_2_WIDTH);
    SetPaneInfo(2, ID_PROGRESSPANE_3, SBPS_NORMAL , PROGRESSPANE_3_WIDTH);
    // build the progress bar window
    CRect rWnd(0, 0, 0, 0);
    m_ProgressBar.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rWnd, this, 1);

    // set the static texts
    CString szText;
    szText.LoadString(IDS_STATTXT_PROCESSING);
    SetPaneText(ID_PROGRESSPANE_LEFTTEXT, szText);
    szText.LoadString(IDS_STATTXT_ESCTOCANCEL);
    SetPaneText(ID_PROGRESSPANE_RIGHTTEXT, szText);
}

/***************************************************************************/
// CProgressStatusBar::SetProcessOwner Set the owner of the process
/***************************************************************************/
void CProgressStatusBar::SetProgressOwner(CProcess * pProcess, void * pCaller, ProcessorType processorType) {
    owners.push_front(pProcess);
    callers[pProcess] = pCaller;
    if (GetSafeHwnd()) {
        CString szText;
        int nID = pProcess->GetProcessorText(processorType);
        if (nID >= 0) {
            // display the given text
            szText.LoadString(nID);
            SetPaneText(ID_PROGRESSPANE_LEFTTEXT, szText);
        } else {
            // display standard text
            szText.LoadString(IDS_STATTXT_PROCESSING);
            SetPaneText(ID_PROGRESSPANE_LEFTTEXT, szText);
        }
    }
}

/***************************************************************************/
// CProgressStatusBar::ClearProcessOwner clears the owner and caller
/***************************************************************************/
void CProgressStatusBar::ClearProgressOwner(CProcess * pProcess) {
    owners.remove(pProcess);
    callers.erase(pProcess);
}

/***************************************************************************/
// CProgressStatusBar::InitProgress Initialisation of the progress bar
/***************************************************************************/
void CProgressStatusBar::InitProgress() {
    // get the pane coordinates for the progress bar
    CRect rWnd;
    GetItemRect(ID_PROGRESSPANE_BAR, rWnd);
    // move the bar
    m_ProgressBar.MoveWindow(rWnd);
    // initialize the progress
    m_ProgressBar.SetProgress(0);
}

/***************************************************************************/
// CProgressStatusBar::SetProgress Set progress bar
// This function checks, if the progress value has changed. If it has, it
// calls the windows message loop processing to allow MFC message processing
// and it sets the progress bar to the new value.
/***************************************************************************/
void CProgressStatusBar::SetProgress( int nVal) {
    
    // only update for the current owner
   if (m_ProgressBar.GetProgress() != nVal) {
       m_ProgressBar.SetProgress(nVal);
    }

    // every 100 ms check the message loop
    static DWORD dwTickLast = 0;
    DWORD dwThis = GetTickCount();
    if (dwThis - dwTickLast > 100) {
        dwTickLast = dwThis;
        CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
        CSaDoc * pModel = pMainWnd->GetCurrDoc();
        BOOL bState = (pModel) ? pModel->EnableBackgroundProcessing(FALSE) : 0;
        MessageLoop(); // do windows message loop
        pModel ? pModel->EnableBackgroundProcessing(bState) : 0;
    }
}

/***************************************************************************/
// CProgressStatusBar::SetPaneText Set text in status panes
/***************************************************************************/
BOOL CProgressStatusBar::SetPaneText(int nPaneID, LPCTSTR lpszText, BOOL bUpdate) {

    switch (nPaneID) {
    case ID_PROGRESSPANE_1:
    case ID_PROGRESSPANE_LEFTTEXT:
        CStatusBar::SetPaneText(CommandToIndex(ID_PROGRESSPANE_1), lpszText, bUpdate);
        break;
    case ID_PROGRESSPANE_2:
    case ID_PROGRESSPANE_BAR:
        CStatusBar::SetPaneText(CommandToIndex(ID_PROGRESSPANE_2), lpszText, bUpdate);
        break;
    case ID_PROGRESSPANE_3:
    case ID_PROGRESSPANE_RIGHTTEXT:
        CStatusBar::SetPaneText(CommandToIndex(ID_PROGRESSPANE_3), lpszText, bUpdate);
        break;
    default:
        return FALSE;
    }

    return TRUE;
}


/***************************************************************************/
// CProgressStatusBar::GetItemRect  Get rectangle for status pane
/***************************************************************************/
void CProgressStatusBar::GetItemRect(int nPaneID, LPRECT lpRect) {
    switch (nPaneID) {
    case ID_PROGRESSPANE_1:
    case ID_PROGRESSPANE_LEFTTEXT:
        CStatusBar::GetItemRect(CommandToIndex(ID_PROGRESSPANE_1), lpRect);
        break;
    case ID_PROGRESSPANE_2:
    case ID_PROGRESSPANE_BAR:
        CStatusBar::GetItemRect(CommandToIndex(ID_PROGRESSPANE_2), lpRect);
        break;
    case ID_PROGRESSPANE_3:
    case ID_PROGRESSPANE_RIGHTTEXT:
        CStatusBar::GetItemRect(CommandToIndex(ID_PROGRESSPANE_3), lpRect);
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CProgressStatusBar::DelayShow() - this is disabled because the only code
// the uses it is the MFC code for print preview which gets confused because
// we have two status bars and so it ends up displaying both in preview and
// then making the progress bar stick in the frame when preview is done.
// To prevent this, we disable the method.
void CProgressStatusBar::DelayShow() {
    // do nothing.
}

CProcess * CProgressStatusBar::GetProgressOwner() {
    // return the process owner
    return (owners.empty())?nullptr:owners.front();   
}

void * CProgressStatusBar::GetProgressCaller() {
    // return the process caller
    return (owners.empty()) ? nullptr : callers[owners.front()];
}

int  CProgressStatusBar::GetProgress() {
    return m_ProgressBar.GetProgress();   // get progress
}

void CProgressStatusBar::SetIsPrintingFlag(bool isPrinting) {
    isPrinting = isPrinting;
}

BOOL CProgressStatusBar::InProcessMessageLoop() const {
    return m_nInMessageLoop != 0;
}
