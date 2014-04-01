/////////////////////////////////////////////////////////////////////////////
// sa_sbar.cpp:
// Implementation of the CSaDataStatusBar
//                       CSaProgressStatusBar classes.
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
#include "sa_sbar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CDataStatusBar
// Customized status bar class. The base class is the general status bar
// class.

BEGIN_MESSAGE_MAP(CDataStatusBar, CStatusBar)
    ON_WM_PAINT()
END_MESSAGE_MAP()

CDataStatusBar::CDataStatusBar()
{
    m_pFont = NULL;
    m_bPositionSymbol = m_bScaleSymbol = m_bFrequencySymbol = FALSE;
    m_bLengthSymbol = m_bCepstralSpectrumSymbol = FALSE;
    m_bAmplitudeSymbol = m_bRawSpectrumSymbol = m_bNoteSymbol = FALSE;
    m_bPitchSymbol = m_bLpcSpectrumSymbol = FALSE;
}

/***************************************************************************/
// CDataStatusBar::~CDataStatusBar Destructor
/***************************************************************************/
CDataStatusBar::~CDataStatusBar()
{
    if (m_pFont!=NULL)
    {
        delete m_pFont;
    }
}

/***************************************************************************/
// CDataStatusBar::Init Initialisation
// Setting up the font and the pane sizes of the status bar.
/***************************************************************************/
void CDataStatusBar::Init()
{
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
    SetPaneInfo(0, ID_STATUSPANE_SAMPLES, SBPS_NORMAL , STATUSPANE_5_WIDTH);
    SetPaneInfo(1, ID_STATUSPANE_FORMAT, SBPS_NORMAL , STATUSPANE_6_WIDTH);
    SetPaneInfo(2, ID_STATUSPANE_CHANNELS, SBPS_NORMAL , STATUSPANE_7_WIDTH);
    SetPaneInfo(3, ID_STATUSPANE_SIZE, SBPS_NORMAL , STATUSPANE_7_WIDTH);
    SetPaneInfo(4, ID_STATUSPANE_TLENGTH, SBPS_NORMAL , STATUSPANE_7_WIDTH);
    SetPaneInfo(5, ID_STATUSPANE_TYPE, SBPS_NORMAL , STATUSPANE_7_WIDTH);
    SetPaneInfo(6, ID_STATUSPANE_BITRATE, SBPS_NORMAL , STATUSPANE_7_WIDTH);
    SetPaneInfo(7, ID_STATUSPANE_EMPTY, SBPS_STRETCH, 0);
    SetPaneInfo(8, ID_STATUSPANE_1, SBPS_NORMAL , STATUSPANE_1_WIDTH);
    SetPaneInfo(9, ID_STATUSPANE_2, SBPS_NORMAL , STATUSPANE_2_WIDTH);
    SetPaneInfo(10, ID_STATUSPANE_3, SBPS_NORMAL , STATUSPANE_3_WIDTH);
    SetPaneInfo(11, ID_STATUSPANE_4, SBPS_NORMAL , STATUSPANE_4_WIDTH);
}

/***************************************************************************/
// CDataStatusBar::DoPaint Painting
// ## Under construction!
/***************************************************************************/
void CDataStatusBar::OnPaint()
{
    CStatusBar::OnPaint();
    CDC * pDC = GetDC();
    DoPaint(pDC);
    ReleaseDC(pDC);
}

void CDataStatusBar::DoPaint(CDC * pDC)
{
    CStatusBar::DoPaint(pDC); // Draw statusbar
    CRect rItem;

    // create brush and pen
    CBrush brushBlack(GetSysColor(COLOR_ACTIVECAPTION));
    CBrush * pOldBrush = (CBrush *)pDC->SelectObject(&brushBlack);
    CPen pen(PS_SOLID, 1, GetSysColor(COLOR_ACTIVECAPTION));
    CPen * pOldPen = pDC->SelectObject(&pen);

    if (m_bScaleSymbol)
    {
        // get item rectangle for scale pane
        GetItemRect(ID_STATUSPANE_POSITION, rItem);
        rItem.right = rItem.left + rItem.Height() + 4; // set right border
        rItem.InflateRect(-1, -1); // avoid 3D borders
        // draw position symbol
        // paint scale
        rItem.left += 1;
        rItem.right -= 1;
        rItem.top += 1;
        rItem.bottom -= 3;
        pDC->MoveTo(rItem.right - 2, rItem.top);
        pDC->LineTo(rItem.right - 4, rItem.top + 6);
        pDC->LineTo(rItem.right, rItem.top + 6);
        pDC->LineTo(rItem.right - 2, rItem.top);
        pDC->MoveTo(rItem.left + 2, rItem.bottom - 6);
        pDC->LineTo(rItem.left, rItem.bottom);
        pDC->LineTo(rItem.left + 4, rItem.bottom);
        pDC->LineTo(rItem.left + 2, rItem.bottom - 6);
        pDC->LineTo(rItem.right - 2, rItem.top);
        int nHCenterPos = rItem.left + rItem.Width() / 2;
        pDC->MoveTo(nHCenterPos, rItem.top + 1);
        pDC->LineTo(nHCenterPos, rItem.bottom - 1);
    }

    if (m_bPositionSymbol)
    {
        // get item rectangle for position pane
        GetItemRect(ID_STATUSPANE_POSITION, rItem);
        rItem.right = rItem.left + rItem.Height() + 4; // set right border
        rItem.InflateRect(-1, -1); // avoid 3D borders
        // draw position symbol
        // paint cursor lines
        pDC->MoveTo(rItem.right - 3, rItem.top + 2);
        pDC->LineTo(rItem.right - 3, rItem.bottom - 2);
        pDC->MoveTo(rItem.right - 5, rItem.top + 2);
        pDC->LineTo(rItem.right - 5, rItem.bottom - 2);
        // paint arrow
        int nVCenterPos = rItem.top + rItem.Height() / 2;
        pDC->MoveTo(rItem.left + 1, nVCenterPos);
        pDC->LineTo(rItem.right - 6, nVCenterPos);
        pDC->LineTo(rItem.right - 8, nVCenterPos - 2);
        pDC->LineTo(rItem.right - 8, nVCenterPos + 2);
        pDC->LineTo(rItem.right - 6, nVCenterPos);
    }

    if (m_bLengthSymbol)
    {
        // get item rectangle for length pane
        GetItemRect(ID_STATUSPANE_LENGTH, rItem);
        rItem.right = rItem.left + rItem.Height() + 4; // set right border
        rItem.InflateRect(-1, -1); // avoid 3D borders
        // draw position symbol
        // paint cursor lines
        pDC->MoveTo(rItem.right - 3, rItem.top + 2);
        pDC->LineTo(rItem.right - 3, rItem.bottom - 2);
        pDC->MoveTo(rItem.left + 2, rItem.top + 2);
        pDC->LineTo(rItem.left + 2, rItem.bottom - 2);
        // paint arrows
        int nVCenterPos = rItem.top + rItem.Height() / 2;
        pDC->MoveTo(rItem.left + 2, nVCenterPos);
        pDC->LineTo(rItem.right - 4, nVCenterPos);
        pDC->LineTo(rItem.right - 6, nVCenterPos - 2);
        pDC->LineTo(rItem.right - 6, nVCenterPos + 2);
        pDC->LineTo(rItem.right - 4, nVCenterPos);
        pDC->MoveTo(rItem.left + 3, nVCenterPos);
        pDC->LineTo(rItem.left + 5, nVCenterPos - 2);
        pDC->LineTo(rItem.left + 5, nVCenterPos + 2);
        pDC->LineTo(rItem.left + 3, nVCenterPos);
    }

    if (m_bAmplitudeSymbol)
    {
        // get item rectangle for amplitude pane
        GetItemRect(ID_STATUSPANE_AMPLITUDE, rItem);
        rItem.right = rItem.left + rItem.Height() + 4; // set right border
        rItem.InflateRect(-2, -2); // avoid 3D borders
        // paint top line
        pDC->MoveTo(rItem.left, rItem.top + 3);
        pDC->LineTo(rItem.left + 3, rItem.top);
        pDC->LineTo(rItem.right - 7, rItem.top);
        pDC->LineTo(rItem.right - 3, rItem.top + 4);
        // paint arrows
        int nHCenterPos = rItem.left + rItem.Width() / 2;
        pDC->MoveTo(nHCenterPos - 2, rItem.bottom - 2);
        pDC->LineTo(nHCenterPos - 2, rItem.top + 1);
        pDC->LineTo(nHCenterPos - 4, rItem.top + 3);
        pDC->LineTo(nHCenterPos, rItem.top + 3);
        pDC->LineTo(nHCenterPos - 2, rItem.top + 1);
    }

    // pitch symbol re-instated because of larger status bar panes CLW 12/9/00
    if (m_bNoteSymbol)
    {
        // get item rectangle for pitch pane
        GetItemRect(ID_STATUSPANE_NOTE, rItem);
        rItem.right = rItem.left + rItem.Height() + 4; // set right border
        rItem.InflateRect(-2, -2); // avoid 3D borders
        // paint note circle
        int nHCenterPos = rItem.left + rItem.Width() / 2;
        pDC->Ellipse(nHCenterPos - 6, rItem.bottom - 5, nHCenterPos - 1, rItem.bottom);
        // paint lines
        pDC->MoveTo(nHCenterPos - 2, rItem.bottom - 3);
        pDC->LineTo(nHCenterPos - 2, rItem.top);
        pDC->LineTo(nHCenterPos, rItem.top + 2);
        pDC->LineTo(nHCenterPos + 2, rItem.top + 6);
    }

    // pitch symbol
    if (m_bPitchSymbol)
    {
        // get item rectangle for pitch pane
        GetItemRect(ID_STATUSPANE_PITCH, rItem);
        rItem.right = rItem.left + rItem.Height() + 4; // set right border
        rItem.InflateRect(-2, -2); // avoid 3D borders
        // Paint frame
        // pDC->Rectangle(rItem);
        // Paint center line
        pDC->MoveTo(rItem.left, (rItem.top + rItem.bottom)/2);
        pDC->LineTo(rItem.right, (rItem.top + rItem.bottom)/2);
        // Paint wave
        rItem.InflateRect(-2, -1); // shrink wav a bit
        rItem.right = rItem.Width()/2 + rItem.left;
        pDC->Arc(&rItem,CPoint(rItem.right,(rItem.top + rItem.bottom)/2),CPoint(rItem.left,(rItem.top + rItem.bottom)/2));
        rItem.OffsetRect(rItem.Width(),0);
        pDC->Arc(&rItem,CPoint(rItem.left,(rItem.top + rItem.bottom)/2),CPoint(rItem.right,(rItem.top + rItem.bottom)/2));
    }

    // restore brush and pen
    pDC->SelectObject(pOldBrush);
    pDC->SelectObject(pOldPen);
}

/***************************************************************************/
// CDataStatusBar::SetPaneSymbol Show or hide a pane symbol
/***************************************************************************/
void CDataStatusBar::SetPaneSymbol(int nPaneID, BOOL bShow)
{
    // Pane #5 Symbols
    if ((nPaneID == ID_STATUSPANE_SAMPLES) ||
        (nPaneID == ID_STATUSPANE_FORMAT) ||
        (nPaneID == ID_STATUSPANE_CHANNELS) ||
        (nPaneID == ID_STATUSPANE_SIZE) ||
        (nPaneID == ID_STATUSPANE_TLENGTH) ||
        (nPaneID == ID_STATUSPANE_TYPE) ||
        (nPaneID == ID_STATUSPANE_BITRATE) ||
        (nPaneID == ID_STATUSPANE_EMPTY))
    {
        return;
    }

	// Pane #1 symbols
    if (nPaneID == ID_STATUSPANE_1)
    {
        m_bScaleSymbol = m_bPositionSymbol = m_bFrequencySymbol = FALSE;
        return;
    }
    if (nPaneID == ID_STATUSPANE_SCALE)
    {
        m_bScaleSymbol = bShow;
        m_bPositionSymbol = m_bFrequencySymbol = FALSE;
        return;
    }
    if (nPaneID == ID_STATUSPANE_FREQUENCY)
    {
        m_bFrequencySymbol = bShow;
        m_bPositionSymbol = m_bScaleSymbol = FALSE;
        return;
    }
    if (nPaneID == ID_STATUSPANE_POSITION)
    {
        m_bPositionSymbol = bShow;
        m_bScaleSymbol = m_bFrequencySymbol = FALSE;
        return;
    }

    // Pane #2 Symbols
    if (nPaneID == ID_STATUSPANE_2)
    {
        m_bCepstralSpectrumSymbol = FALSE;
        m_bLengthSymbol = FALSE;
        return;
    }
    if (nPaneID == ID_STATUSPANE_CEPSTRALSPECTRUM)
    {
        m_bCepstralSpectrumSymbol = bShow;
        m_bLengthSymbol = FALSE;
        return;
    }
    if (nPaneID == ID_STATUSPANE_LENGTH)
    {
        m_bLengthSymbol = bShow;
        m_bCepstralSpectrumSymbol = FALSE;
        return;
    }
    if (nPaneID == ID_STATUSPANE_REL_PITCH)
    {
        m_bLengthSymbol = FALSE;
        m_bCepstralSpectrumSymbol = FALSE;
        return;
    }

    // Pane #3 Symbols
    if (nPaneID == ID_STATUSPANE_3)
    {
        m_bRawSpectrumSymbol = FALSE;
        m_bAmplitudeSymbol = FALSE;
        m_bNoteSymbol = FALSE;
        return;
    }
    if (nPaneID == ID_STATUSPANE_RAWSPECTRUM)
    {
        m_bRawSpectrumSymbol = bShow;
        m_bAmplitudeSymbol = FALSE;
        m_bNoteSymbol = FALSE;
        return;
    }
    if (nPaneID == ID_STATUSPANE_AMPLITUDE)
    {
        m_bAmplitudeSymbol = bShow;
        m_bRawSpectrumSymbol = FALSE;
        m_bNoteSymbol = FALSE;
        return;
    }
    if (nPaneID == ID_STATUSPANE_NOTE)
    {
        m_bNoteSymbol = bShow;
        m_bAmplitudeSymbol = FALSE;
        m_bRawSpectrumSymbol = FALSE;
        return;
    }

    // Pane #4 Symbols
    if (nPaneID == ID_STATUSPANE_4)
    {
        m_bLpcSpectrumSymbol = FALSE;
        m_bPitchSymbol = FALSE;
        return;
    }
    if (nPaneID == ID_STATUSPANE_LPCSPECTRUM)
    {
        m_bLpcSpectrumSymbol = bShow;
        m_bPitchSymbol = FALSE;
        return;
    }
    if (nPaneID == ID_STATUSPANE_PITCH)
    {
        m_bPitchSymbol = bShow;
        m_bLpcSpectrumSymbol = FALSE;
        return;
    }

}


/***************************************************************************/
// CDataStatusBar::SetPaneText Set text in status panes
/***************************************************************************/
BOOL CDataStatusBar::SetPaneText(int nPaneID, LPCTSTR lpszText, BOOL bUpdate)
{
    int nPaneIndex = 0;

    switch (nPaneID)
    {
    case ID_STATUSPANE_SAMPLES:
    case ID_STATUSPANE_FORMAT:
    case ID_STATUSPANE_CHANNELS:
    case ID_STATUSPANE_SIZE:
    case ID_STATUSPANE_TLENGTH:
    case ID_STATUSPANE_TYPE:
    case ID_STATUSPANE_BITRATE:
    case ID_STATUSPANE_EMPTY:
        nPaneIndex = CommandToIndex(nPaneID);
        break;
    case ID_STATUSPANE_1:
    case ID_STATUSPANE_POSITION:
    case ID_STATUSPANE_SCALE:
    case ID_STATUSPANE_FREQUENCY:
        nPaneIndex = CommandToIndex(ID_STATUSPANE_1);
        break;
    case ID_STATUSPANE_2:
    case ID_STATUSPANE_LENGTH:
    case ID_STATUSPANE_REL_PITCH:
    case ID_STATUSPANE_CEPSTRALSPECTRUM:
        nPaneIndex = CommandToIndex(ID_STATUSPANE_2);
        break;
    case ID_STATUSPANE_3:
    case ID_STATUSPANE_AMPLITUDE:
    case ID_STATUSPANE_RAWSPECTRUM:
    case ID_STATUSPANE_NOTE:
        nPaneIndex = CommandToIndex(ID_STATUSPANE_3);
        break;
    case ID_STATUSPANE_4:
    case ID_STATUSPANE_PITCH:
    case ID_STATUSPANE_LPCSPECTRUM:
        nPaneIndex = CommandToIndex(ID_STATUSPANE_4);
        break;
    default:
        return FALSE;
    }

    CStatusBar::SetPaneText(nPaneIndex, lpszText, bUpdate);

    return TRUE;
}

/***************************************************************************/
// CDataStatusBar::GetItemRect  Get rectangle for status pane
/***************************************************************************/
void CDataStatusBar::GetItemRect(int nPaneID, LPRECT lpRect)
{

    switch (nPaneID)
    {
    case ID_STATUSPANE_SAMPLES:
    case ID_STATUSPANE_FORMAT:
    case ID_STATUSPANE_CHANNELS:
    case ID_STATUSPANE_SIZE:
    case ID_STATUSPANE_TLENGTH:
    case ID_STATUSPANE_TYPE:
    case ID_STATUSPANE_BITRATE:
    case ID_STATUSPANE_EMPTY:
        CStatusBar::GetItemRect(CommandToIndex(nPaneID), lpRect);
        break;
    case ID_STATUSPANE_1:
    case ID_STATUSPANE_POSITION:
    case ID_STATUSPANE_SCALE:
    case ID_STATUSPANE_FREQUENCY:
        CStatusBar::GetItemRect(CommandToIndex(ID_STATUSPANE_1), lpRect);
        break;
    case ID_STATUSPANE_2:
    case ID_STATUSPANE_LENGTH:
    case ID_STATUSPANE_REL_PITCH:
    case ID_STATUSPANE_CEPSTRALSPECTRUM:
        CStatusBar::GetItemRect(CommandToIndex(ID_STATUSPANE_2), lpRect);
        break;
    case ID_STATUSPANE_3:
    case ID_STATUSPANE_AMPLITUDE:
    case ID_STATUSPANE_RAWSPECTRUM:
    case ID_STATUSPANE_NOTE:
        CStatusBar::GetItemRect(CommandToIndex(ID_STATUSPANE_3), lpRect);
        break;
    case ID_STATUSPANE_4:
    case ID_STATUSPANE_PITCH:
    case ID_STATUSPANE_LPCSPECTRUM:
        CStatusBar::GetItemRect(CommandToIndex(ID_STATUSPANE_4), lpRect);
        break;
    }

}


/////////////////////////////////////////////////////////////////////////////
// CDataStatusBar message handlers

//###########################################################################
// CProgressStatusBar
// Customized status bar class for displaying progress information. The base
// class is the general status bar class.

/////////////////////////////////////////////////////////////////////////////
// CProgressStatusBar message map

BEGIN_MESSAGE_MAP(CProgressStatusBar, CStatusBar)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgressStatusBar construction/destruction/creation

/***************************************************************************/
// CProgressStatusBar::CProgressStatusBar Constructor
/***************************************************************************/
CProgressStatusBar::CProgressStatusBar()
{
    m_pFont = NULL;
    m_pProcessOwner = NULL;
    m_pProcessCaller = NULL;
    m_bIsPrinting = FALSE;
    m_nInMessageLoop = 0;
}

/***************************************************************************/
// CProgressStatusBar::~CProgressStatusBar Destructor
/***************************************************************************/
CProgressStatusBar::~CProgressStatusBar()
{
    if (m_pFont)
    {
        delete m_pFont;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CProgressStatusBar helper functions

/***************************************************************************/
// CProgressStatusBar::MessageLoop Do windows message loop
// This function enables the escape key down message to come trough long
// processing. Each call will allow to do windows key message processing
// of MFC for this message.
/***************************************************************************/
void CProgressStatusBar::MessageLoop()
{
    static volatile DWORD dwTickLast = 0; // This will allow first call to succeed
    DWORD dwThis = GetTickCount(); // MS since system start

    if (dwThis - dwTickLast > 90)
    {
        dwTickLast = dwThis; // Update when timer has expired
        MSG msg;

        m_nInMessageLoop++;

		// This is risky so only do it if the escape key is pressed
        if (GetAsyncKeyState(VK_ESCAPE) < 0)   
        {
            while (::PeekMessage(&msg, NULL, WM_KEYDOWN, WM_KEYDOWN, PM_NOREMOVE))
            {
                AfxGetApp()->PumpMessage();
            }
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
void CProgressStatusBar::Init()
{
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
void CProgressStatusBar::SetProcessOwner(void * pProcess, void * pCaller, int nProcessID)
{

    m_pProcessOwner = pProcess;
    m_pProcessCaller = pCaller;

    if (GetSafeHwnd())
    {
        CString szText;

        if (nProcessID >= 0)
        {
            // display the given text
            szText.LoadString(nProcessID);
            SetPaneText(ID_PROGRESSPANE_LEFTTEXT, szText);
        }
        else
        {
            // display standard text
            szText.LoadString(IDS_STATTXT_PROCESSING);
            SetPaneText(ID_PROGRESSPANE_LEFTTEXT, szText);
        }
    }
}

/***************************************************************************/
// CProgressStatusBar::InitProgress Initialisation of the progress bar
/***************************************************************************/
void CProgressStatusBar::InitProgress()
{
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
void CProgressStatusBar::SetProgress(int nVal)
{
    if (m_ProgressBar.GetProgress() != nVal)
    {
        m_ProgressBar.SetProgress(nVal); // set the progress bar

        static DWORD dwTickLast = 0;
        DWORD dwThis = GetTickCount();

        if (dwThis - dwTickLast > 100)
        {
            dwTickLast = dwThis;
            CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
            CSaDoc * pDoc = pMainWnd->GetCurrDoc();
            BOOL bState = (pDoc) ? pDoc->EnableBackgroundProcessing(FALSE) : 0;
            MessageLoop(); // do windows message loop
            pDoc ? pDoc->EnableBackgroundProcessing(bState) : 0;
        }
    }
}

/***************************************************************************/
// CProgressStatusBar::SetPaneText Set text in status panes
/***************************************************************************/
BOOL CProgressStatusBar::SetPaneText(int nPaneID, LPCTSTR lpszText, BOOL bUpdate)
{

    switch (nPaneID)
    {
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
void CProgressStatusBar::GetItemRect(int nPaneID, LPRECT lpRect)
{
    switch (nPaneID)
    {
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
// CProgressStatusBar message handlers



/////////////////////////////////////////////////////////////////////////////
// CProgressStatusBar::DelayShow() - this is disabled because the only code
// the uses it is the MFC code for print preview which gets confused because
// we have two status bars and so it ends up displaying both in preview and
// then making the progress bar stick in the frame when preview is done.
// To prevent this, we disable the method.
void CProgressStatusBar::DelayShow()
{
    // do nothing.
}

void * CProgressStatusBar::GetProcessOwner()
{
    return m_pProcessOwner;   // return the process owner
}

void * CProgressStatusBar::GetProcessCaller()
{
    return m_pProcessCaller;   // return the process caller
}

int  CProgressStatusBar::GetProgress()
{
    return m_ProgressBar.GetProgress();   // get progress
}

void CProgressStatusBar::SetIsPrintingFlag(BOOL isPrinting)
{
    m_bIsPrinting = isPrinting;
}

BOOL CProgressStatusBar::InProcessMessageLoop() const
{
    return m_nInMessageLoop != 0;
}
