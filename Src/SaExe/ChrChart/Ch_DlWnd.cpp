/////////////////////////////////////////////////////////////////////////////
// ch_dlwnd.cpp:
// Implementation of the CChartText (dialog child window)
//             CChartLine (dialog child window)
//             CChartChar (dialog child window) classes.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ch_dlwnd.h"
#include "ch_dlg.h"
#include "sa_view.h"
#include "sa_doc.h"
#include "mainfrm.h"
#include "windows.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern CSaApp NEAR theApp;

//###########################################################################
// CChartText
// Special window with 3D border to display wave file transcription.

IMPLEMENT_DYNCREATE(CChartText, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CChartText message map

BEGIN_MESSAGE_MAP(CChartText, CWnd)
	//{{AFX_MSG_MAP(CChartText)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChartText construction/destruction/creation

/***************************************************************************/
// CChartText::CChartText Constructor
/***************************************************************************/
CChartText::CChartText()
{
	m_nID = 0;
	m_nMode = NO_LINES;
	m_pFont = NULL;
	m_szText.Empty();
}

/***************************************************************************/
// CChartText::~CChartText Destructor
/***************************************************************************/
CChartText::~CChartText()
{
}

/////////////////////////////////////////////////////////////////////////////
// CChartText helper functions

/***************************************************************************/
// CChartText::Init Initialisation
// Creates the window and places it over the dialog control.
/***************************************************************************/
void CChartText::Init(UINT nID, int nMode, CFont* pFont, CWnd* pParent)
{
	m_nID = nID;
	if (nID && pFont && pParent)
	{
		m_nMode = nMode;
		m_pFont = pFont;
		CWnd* pWnd = pParent->GetDlgItem(nID);
		pWnd->GetWindowText(m_szText); // get the text
		CRect rWnd(0, 0, 0, 0);
		Create(NULL, NULL, WS_CHILD|WS_VISIBLE, rWnd, pParent, 1);
		pWnd->GetWindowRect(rWnd);
		pWnd->DestroyWindow(); // destroy the old window
		pParent->ScreenToClient(rWnd);
		MoveWindow(rWnd, TRUE);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CChartText message handlers

/***************************************************************************/
// CChartText::OnPaint Painting
/***************************************************************************/
void CChartText::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// get window coordinates
	CRect rWnd;
	GetClientRect(rWnd);
	// draw the lines
	dc.MoveTo(rWnd.TopLeft());
	if (m_nMode & HORZTOP_LINE)
	{
		dc.LineTo(rWnd.right, rWnd.top);
		dc.MoveTo(rWnd.left, rWnd.top + 1);
		if (m_nMode & HORZTOP_BOLD)
		{
			dc.LineTo(rWnd.right, rWnd.top + 1);
			dc.MoveTo(rWnd.TopLeft());
		}
	}
	if (m_nMode & VERTLEFT_LINE)
	{
		dc.LineTo(rWnd.left, rWnd.bottom);
		dc.MoveTo(rWnd.left + 1, rWnd.top);
		if (m_nMode & VERTLEFT_BOLD) dc.LineTo(rWnd.left + 1, rWnd.bottom);
	}
	dc.MoveTo(rWnd.left, rWnd.bottom - 1);
	if (m_nMode & HORZBOTTOM_LINE)
	{
		dc.LineTo(rWnd.right, rWnd.bottom - 1);
		dc.MoveTo(rWnd.left, rWnd.bottom - 2);
		if (m_nMode & HORZBOTTOM_BOLD) dc.LineTo(rWnd.right, rWnd.bottom - 2);
	}
	if (m_nMode & VERTRIGHT_LINE)
	{
		dc.MoveTo(rWnd.right - 1, rWnd.top);
		dc.LineTo(rWnd.right - 1, rWnd.bottom);
		dc.MoveTo(rWnd.right - 2, rWnd.top);
		if (m_nMode & VERTRIGHT_BOLD) dc.LineTo(rWnd.right - 2, rWnd.bottom);
	}
	// draw the text
	if (m_szText.GetLength())
	{
		// set the font
		CFont* pOldFont = dc.SelectObject(m_pFont); // select actual font
		dc.SetBkMode(TRANSPARENT);
		// draw the text
		UINT nFormat = DT_VCENTER;
		if (m_nMode & TEXT_TOP) nFormat = DT_TOP;
		if (m_nMode & TEXT_BOTTOM) nFormat = DT_BOTTOM;
		if (m_nMode & TEXT_LEFT) nFormat |= DT_LEFT;
		else
		{
			if (m_nMode & TEXT_RIGHT) nFormat |= DT_RIGHT;
			else nFormat |= DT_CENTER;
		}
		dc.DrawText(m_szText, m_szText.GetLength(), rWnd, DT_SINGLELINE | DT_NOCLIP | nFormat);
		dc.SelectObject(pOldFont); // allows for proper restoration
	}
}

/***************************************************************************/
// CChartText::OnEraseBkgnd Erasing background
// Draw background, unless its mode requests transparent.
/***************************************************************************/
BOOL CChartText::OnEraseBkgnd(CDC* pDC)
{
	if (m_nMode & BCK_TRANSPARENT) return TRUE;
	if (m_nMode & BCK_DARK)
	{
		// create the dark background
		CBrush backBrush(GetSysColor(COLOR_BTNSHADOW));
		CBrush* pOldBrush = pDC->SelectObject(&backBrush);
		CRect rClip;
		pDC->GetClipBox(&rClip); // erase the area needed
		pDC->PatBlt(rClip.left, rClip.top, rClip.Width(), rClip.Height(), PATCOPY);
		pDC->SelectObject(pOldBrush);
	}
	else
	{
		// fill with dialog background
		//CWnd* pParentWnd = GetParent();
		CRect rWnd;
		GetClientRect(rWnd);
		// KG 11/00: 32-bit conversion - Make CSliderVertical system gray
		CBrush brGray(GetSysColor(COLOR_3DFACE));
		pDC->FillRect(rWnd,&brGray);
	}
	return TRUE;
}

//###########################################################################
// CChartLine
// Special window with 3D border to display wave file transcription.

IMPLEMENT_DYNCREATE(CChartLine, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CChartLine message map

BEGIN_MESSAGE_MAP(CChartLine, CWnd)
	//{{AFX_MSG_MAP(CChartLine)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChartLine construction/destruction/creation

/***************************************************************************/
// CChartLine::CChartLine Constructor
/***************************************************************************/
CChartLine::CChartLine()
{
	m_nMode = NORMAL;
}

/***************************************************************************/
// CChartLine::~CChartLine Destructor
/***************************************************************************/
CChartLine::~CChartLine()
{
}

/////////////////////////////////////////////////////////////////////////////
// CChartLine helper functions

/***************************************************************************/
// CChartLine::Init Initialisation
// Creates the window and places it over the dialog control. The window size
// will be increased to be able to draw the dots in the corners.
/***************************************************************************/
void CChartLine::Init(UINT nID, int nMode, CWnd* pParent)
{
	if (nID && pParent)
	{
		m_nMode = nMode;
		CWnd* pWnd = pParent->GetDlgItem(nID);
		CRect rWnd(0, 0, 0, 0);
		Create(NULL, NULL, WS_CHILD|WS_VISIBLE, rWnd, pParent, 1);
		pWnd->GetWindowRect(rWnd);
		pWnd->DestroyWindow(); // destroy the old window
		// inflate rectangle for dots
		rWnd.InflateRect(DOT_SIZE / 2, DOT_SIZE / 2);
		pParent->ScreenToClient(rWnd);
		MoveWindow(rWnd, TRUE);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CChartLine message handlers

/***************************************************************************/
// CChartLine::OnPaint Painting
/***************************************************************************/
void CChartLine::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// get window coordinates
	CRect rWnd;
	GetClientRect(rWnd);
	// prepare the line coordinates
	rWnd.InflateRect(-DOT_SIZE / 2, -DOT_SIZE / 2);
	CPoint ptFrom(rWnd.TopLeft());
	CPoint ptTo(rWnd.BottomRight());
	if (m_nMode & LINE_TOP) ptTo.y = rWnd.top;
	if (m_nMode & LINE_LEFT) ptTo.x = rWnd.left;
	if (m_nMode & DIAG_RL)
	{
		ptFrom.y = rWnd.bottom;
		ptTo.y = rWnd.top;
	}
	// draw the line
	dc.MoveTo(ptFrom);
	dc.LineTo(ptTo);
	// draw the dots
	CBrush* pOldBrush = (CBrush*)dc.SelectStockObject(BLACK_BRUSH);
	if ((m_nMode & END_DOTS) || (m_nMode & START_DOT))
		dc.Ellipse(ptFrom.x - DOT_SIZE / 2 + 1, ptFrom.y - DOT_SIZE / 2, ptFrom.x + DOT_SIZE / 2 + 1, ptFrom.y + DOT_SIZE / 2);
	if (m_nMode & END_DOTS)
		dc.Ellipse(ptTo.x - DOT_SIZE / 2 + 1, ptTo.y - DOT_SIZE / 2, ptTo.x + DOT_SIZE / 2 + 1, ptTo.y + DOT_SIZE / 2);
	dc.SelectObject(pOldBrush);
}

/***************************************************************************/
// CChartLine::OnEraseBkgnd Erasing background
// Don't draw any background, it's always transparent.
/***************************************************************************/
BOOL CChartLine::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

//###########################################################################
// CChartChar
// Special window for drawing static text with dialog background color.

IMPLEMENT_DYNCREATE(CChartChar, CWnd)
	static CChartChar *m_pLastBubble = NULL;// create the bubble window (zoom)

/////////////////////////////////////////////////////////////////////////////
// CChartChar message map

BEGIN_MESSAGE_MAP(CChartChar, CWnd)
	//{{AFX_MSG_MAP(CChartChar)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChartChar construction/destruction/creation

/***************************************************************************/
// CChartChar::CChartChar Constructor
/***************************************************************************/
CChartChar::CChartChar()
{
	m_szChar.Empty();
	m_nID = 0;
	m_nType = VOWEL;
	m_nMode = NORMAL;
	m_pFont = NULL;
	m_pParent = NULL;
	m_pCaller = NULL;
	m_pCreator = NULL;
	m_bReadyToSelect = FALSE;
	m_pBubble = NULL;
	m_nPlayState = kStateIdle;
}

/***************************************************************************/
// CChartChar::~CChartChar Destructor
/***************************************************************************/
CChartChar::~CChartChar()
{
	if (m_pBubble)
	{
		m_pBubble->DestroyWindow();
		delete m_pBubble;
		if(m_pBubble == m_pLastBubble) m_pLastBubble = NULL;
	}
	if (m_nMode & ICH_BUBBLE)
	{
		if (m_pFont) delete m_pFont;
	}
	if (IsWindow(m_hWnd))
	{
		KillTimer(1);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CChartChar helper functions

/***************************************************************************/
// CChartChar::Setup Character data setup
// Sets up all the necessary data for the character.
/***************************************************************************/
void CChartChar::Setup(CString* pszChar, CString* pszSoundFile, UINT nID, int nType, int nMode,
	CFont* pFont, CWnd* pParent, CDlgCharChart* pCaller, CChartChar* pCreator)
{
	m_nID = nID;
	m_szChar = *pszChar;
	if(pszSoundFile)
		m_szSoundFile = *pszSoundFile;
	else m_szSoundFile = "";
	m_nType = nType;
	m_nMode = nMode;
	m_pFont = pFont;
	m_pParent = pParent;
	m_pCaller = pCaller;
	m_pCreator = pCreator;
}

/***************************************************************************/
// CChartChar::Init Initialisation
// Creates the window and places it over the dialog control. If the window
// is a bubble window, it has to create a new bigger font and to create
// the window after the font size. The position of the window will be such,
// that the bottom right corner lies in the center of the window, which ID
// has been passed as parameter. Be sure to call setup before!
/***************************************************************************/
void CChartChar::Init()
{
	if ((m_nID && m_pFont && m_pParent && m_pCaller)
		&& (((m_nMode & ICH_BUBBLE) && m_pCreator) || ((m_nMode & ICH_BUBBLE) == 0)))
	{
		CWnd* pWnd;
		if (m_nMode & ICH_BUBBLE) pWnd = m_pCreator;
		else pWnd = m_pParent->GetDlgItem(m_nID);
		CRect rWnd(0, 0, 0, 0);
		Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rWnd, m_pParent, 1);
		pWnd->GetWindowRect(rWnd);
		if ((m_nMode & ICH_BUBBLE) == 0) pWnd->DestroyWindow(); // destroy the old window
		m_pParent->ScreenToClient(rWnd);
		if (m_nMode & ICH_BUBBLE)
		{
			// create a new bigger font
			LOGFONT logFont;
			m_pFont->GetObject(sizeof(LOGFONT), (void*)&logFont); // fill up logFont
			// modify the logFont
			logFont.lfHeight *= 2;
			logFont.lfWidth *= 2;
			m_pFont = new CFont;
			m_pFont->CreateFontIndirect(&logFont); // create the modified font
			// get textmetrics
			CDC* pDC = GetDC(); // device context
			CFont* oldFont = pDC->SelectObject(m_pFont); // select actual font
			TEXTMETRIC tm;
			pDC->GetTextMetrics(&tm);
			// resize and position the window
			rWnd.bottom = rWnd.top + rWnd.Height() / 2;
			rWnd.right = rWnd.left + rWnd.Width() / 2;
			rWnd.top = rWnd.bottom - tm.tmHeight;
			rWnd.left = rWnd.right - pDC->GetTextExtent(m_szChar).cx - tm.tmMaxCharWidth/2;
			pDC->SelectObject(oldFont);  // set back old font
			ReleaseDC(pDC);
			// fit it into parent (dialog)
			CRect rDlg;
			m_pParent->GetWindowRect(rDlg);
			m_pParent->ScreenToClient(rDlg);
			if (rWnd.right > rDlg.right) rWnd.OffsetRect(rDlg.right - rWnd.right, 0);
			if (rWnd.left < rDlg.left) rWnd.OffsetRect(rDlg.left - rWnd.left, 0);
			if (rWnd.bottom > rDlg.bottom) rWnd.OffsetRect(0, rDlg.bottom - rWnd.bottom);
			if (rWnd.top < rDlg.top) rWnd.OffsetRect(0, rDlg.top - rWnd.top);
		}
		MoveWindow(rWnd, TRUE);
		// bring window to top, so it receives mouse messages
		if ((m_nMode & ICH_BUBBLE) == 0) 
			BringWindowToTop();
		else
		{
			m_nPlayState = kStateBegin;
			SetTimer(1, SOUND_DELAY, NULL); // start the timer
		}
	}
}

/***************************************************************************/
// CChartChar::ChangeMode Change the display mode on the fly
/***************************************************************************/
void CChartChar::ChangeMode(int nMode)
{
	if (m_nMode != nMode)
	{
		m_nMode = nMode;
		Invalidate(); // redraw with the new mode
	}
}

/////////////////////////////////////////////////////////////////////////////
// CChartChar message handlers

/***************************************************************************/
// CChartChar::OnPaint Painting
/***************************************************************************/
void CChartChar::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// get window coordinates
	CRect rWnd;
	GetClientRect(rWnd);
	// set the font
	CFont* pOldFont = dc.SelectObject(m_pFont); // select actual font
	CFont cRegularFont;
	dc.SetBkMode(TRANSPARENT);
	// draw the text
	if ((m_nMode & ICH_SELECTED) && ((m_nMode & ICH_BUBBLE) == 0))
		dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT)); // set highlighted text
	// if ((m_nMode & ICH_GRAYED) && ((m_nMode & ICH_BUBBLE) == 0))
	// dc.SetTextColor(GetSysColor(COLOR_GRAYTEXT)); // set highlighted text
	rWnd.bottom--; // put the character one pixel higher
	dc.DrawText(m_szChar, m_szChar.GetLength(), rWnd, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOCLIP);
	rWnd.bottom++;
	dc.SelectObject(pOldFont); // allows for proper restoration
	if (m_nMode & ICH_BUBBLE)
	{
		// draw the borders
		dc.MoveTo(rWnd.TopLeft());
		dc.LineTo(rWnd.right - 1, rWnd.top);
		dc.LineTo(rWnd.right - 1, rWnd.bottom - 1);
		dc.LineTo(rWnd.left, rWnd.bottom - 1);
		dc.LineTo(rWnd.left, rWnd.top);
	}
}

/***************************************************************************/
// CChartChar::OnEraseBkgnd Erasing background
// Don't draw any background unless it's selected or a bubble window.
/***************************************************************************/
BOOL CChartChar::OnEraseBkgnd(CDC* pDC)
{
	if ((m_nMode & ICH_SELECTED) || (m_nMode & ICH_BUBBLE))
	{
		// create the background brush
		CBrush backBrush;
		if (m_nMode & ICH_SELECTED) backBrush.CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
		else backBrush.CreateSolidBrush(RGB(255,255,223));
		CBrush* pOldBrush = pDC->SelectObject(&backBrush);
		CRect rClip;
		pDC->GetClipBox(&rClip); // erase the area needed
		pDC->PatBlt(rClip.left, rClip.top, rClip.Width(), rClip.Height(), PATCOPY);
		pDC->SelectObject(pOldBrush);
	}
	return TRUE;
}

/***************************************************************************/
// CChartChar::OnLButtonDown Mouse left button down
// The user probably wants to select this character. Prepare for button up
// (only if character enabled).
/***************************************************************************/
void CChartChar::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (((m_nMode & ICH_DISABLED) == 0) && ((m_nMode & ICH_GRAYED) == 0))
	{
		m_bReadyToSelect = TRUE;
		SetCapture(); // get all further mouse input
	}
	CWnd::OnLButtonDown(nFlags, point);
}

/***************************************************************************/
// CChartChar::OnLButtonDblClk Mouse left button doubleclick
// The user probably wants to select this character. Prepare for button up
// (only if character enabled).
/***************************************************************************/
void CChartChar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (((m_nMode & ICH_DISABLED) == 0) && ((m_nMode & ICH_GRAYED) == 0))
	{
		m_bReadyToSelect = TRUE;
		SetCapture(); // get all further mouse input
	}
	CWnd::OnLButtonDblClk(nFlags, point);
}

/***************************************************************************/
// CChartChar::OnLButtonUp Mouse left button down
// If this character previously also received a button down, then it's time
// to select this character. The function sends two notification messages
// to the caller. One, just for the appropriate ID and a broad one, with
// the ID selected as parameter and a pointer to the selected CString.
/***************************************************************************/
void CChartChar::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bReadyToSelect)
	{
		m_bReadyToSelect = FALSE;
		ReleaseCapture();
		KillTimer(1);
		if (m_pBubble)
		{
			m_pBubble->DestroyWindow();
			delete m_pBubble;
			if(m_pBubble == m_pLastBubble) m_pLastBubble = NULL;
			m_pBubble = NULL;
		}
		// check if cursor still in character window
		CRect rWnd;
		GetClientRect(rWnd);
		if (rWnd.PtInRect(point))
		{
			m_pCaller->SendMessage(WM_COMMAND, m_nID, 0); // notify caller
			m_pCaller->SendMessage(WM_USER_CHARSELECT, m_nID, (long)(&m_szChar));
		}
	}
	CWnd::OnLButtonUp(nFlags, point);
}

/***************************************************************************/
// CChartChar::OnMouseMove Mouse left button down
// If the cursor moves over the character window and zoom is allowed, then
// capture the mouse input and start the zoom timer. As soon as the mouse
// is out of the character window, stop zoom.
/***************************************************************************/
void CChartChar::OnMouseMove(UINT nFlags, CPoint point)
{
	if (((m_nMode & ICH_NO_ZOOM) == 0) && ((m_nMode & ICH_GRAYED) == 0))
	{
		// check if cursor is in character window
		CRect rWnd;
		GetClientRect(rWnd);
		if (rWnd.PtInRect(point))
		{
			if (m_pBubble == NULL)
			{
				// no bubble (zoom) yet
				SetCapture(); // get all further mouse input
				SetTimer(1, ZOOM_DELAY, NULL); // start the zoom timer
			}
		}
		else
		{
			m_bReadyToSelect = FALSE;
			KillTimer(1);
			m_nPlayState = kStateIdle;
			ReleaseCapture();
			if (m_pBubble)
			{
				m_pBubble->DestroyWindow();
				delete m_pBubble;
				if(m_pBubble == m_pLastBubble) m_pLastBubble = NULL;
				m_pBubble = NULL;
			}
		}
	}
	CWnd::OnMouseMove(nFlags, point);
}

static void playSoundFile(const CString &szSoundFile);

/***************************************************************************/
// CChartChar::OnTimer Timer event
/***************************************************************************/
void CChartChar::OnTimer(UINT nIDEvent)
{
	KillTimer(1);
	if(m_nPlayState == kStateIdle)
	{
		m_pBubble = new CChartChar;

		if(m_pLastBubble)
		{
			m_pLastBubble->m_pCreator->m_pBubble = NULL; // Disconnect From Creator
			m_pLastBubble->DestroyWindow();
			delete m_pLastBubble;
		}
		m_pLastBubble = m_pBubble;

		((CChartChar*)m_pBubble)->Setup(&m_szChar, &m_szSoundFile, m_nID, m_nType, ICH_NO_ZOOM | ICH_DISABLED | ICH_BUBBLE, m_pFont, m_pParent, m_pCaller, this);
		((CChartChar*)m_pBubble)->Init();
	}
	else 
	{
		DWORD dwLength = 0;
		CSaView *pSaView = reinterpret_cast<CMainFrame*>(AfxGetMainWnd())->GetCurrSaView();

		m_nPlayState++;

		// We need to try to play something
		if (!m_pCaller->m_bPlay[CDlgCharChart::kPlaySound])
			m_nPlayState = kStateDone;

		if (m_nPlayState == kStateWord)
		{
			if(!m_pCaller->m_bPlay[CDlgCharChart::kPlayWord])
				m_nPlayState++;
			else
			{
				PlaySound(NULL,NULL,SND_ASYNC);
				dwLength = pSaView->OnPlaybackWord();
			}
		}

		if (m_nPlayState == kStateSegment)
		{
			if(!m_pCaller->m_bPlay[CDlgCharChart::kPlaySegment])
				m_nPlayState++;
			else
			{
				PlaySound(NULL,NULL,SND_ASYNC);
				dwLength = pSaView->OnPlaybackSegment();
			}
		}

		if (m_nPlayState == kStateSound)
			playSoundFile(m_szSoundFile);

		if (dwLength)
		{
			// We are playing a sound, but not the last sound
			int nDelayMS = int(pSaView->GetDocument()->GetTimeFromBytes(dwLength)*1000) + SOUND_DELAY;
			SetTimer(1, nDelayMS, NULL); // start the zoom timer
		}

		if (m_pCreator)
			m_pCreator->SetCapture();
	}
	CWnd::OnTimer(nIDEvent);
}

/**
* returns true if IPAHelp seems to be installed
*/
bool checkIPAHelp() {

	// retrieve IPA Help location from registry
	TCHAR szPathBuf[_MAX_PATH + 1];
	HKEY hKey;
	DWORD dwBufLen = MAX_PATH + 1;
	if (RegOpenKeyEx( HKEY_CURRENT_USER, _T("Software\\SIL\\IPA Help"), 0, KEY_QUERY_VALUE, &hKey ))
		return false;
	long nError = RegQueryValueEx( hKey, _T("Location"), NULL, NULL, (LPBYTE) szPathBuf, &dwBufLen);
	RegCloseKey(hKey);
	if (nError || !wcslen(szPathBuf))
		return false;

	// check for existence
	return (_taccess(szPathBuf,0)!=-1);
}

static void playSoundFile(const CString &szSoundFile)
{
	if (szSoundFile.IsEmpty())
		return;

	// retrieve IPA Help location from registry
	TCHAR szPathBuf[_MAX_PATH + 1];
	HKEY hKey;
	DWORD dwBufLen = MAX_PATH + 1;
	if (RegOpenKeyEx( HKEY_CURRENT_USER, _T("Software\\SIL\\IPA Help"), 0, KEY_QUERY_VALUE, &hKey ))
		return;
	long nError = RegQueryValueEx( hKey, _T("Location"), NULL, NULL, (LPBYTE) szPathBuf, &dwBufLen);
	RegCloseKey(hKey);

	if (nError || !wcslen(szPathBuf))
		return;

	CSaString szPath = szPathBuf;
	if (!(szPath.Right(1) == _T("\\")))
		szPath += _T("\\");
	szPath += _T("Sounds - IPA\\");
	szPath += szSoundFile;
	szPath += ".wav";

	// play the WAV file
	PlaySound(szPath,NULL,SND_FILENAME|SND_ASYNC);
}
