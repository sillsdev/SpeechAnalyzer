/////////////////////////////////////////////////////////////////////////////
// sa_minic.cpp:
// Implementation of the CMiniCaptionWnd class.
// Author: Rod Early, changes: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_minic.h"
#include "sa_graph.h"

#include "rs_minic.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"

#include "sa.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CMiniCaptionWnd
// This class creates special child windows within a MDI Child Window. This
// windows can have no, normal or minimized caption bars (default is minimi-
// zed. The style can be set any time by function call. The OS does not
// handle the focus for windows within an MDI child, so this class has to.
// It notifies the parent view, by function call. It works for windows 3.1
// and higher and for windows 95. It's used as base class for the graphs.

IMPLEMENT_DYNCREATE(CMiniCaptionWnd, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CMiniCaptionWnd message map

BEGIN_MESSAGE_MAP(CMiniCaptionWnd, CWnd)
    ON_WM_NCHITTEST()
    ON_WM_NCCALCSIZE()
    ON_WM_NCPAINT()
    ON_WM_ERASEBKGND()
    ON_WM_NCLBUTTONDOWN()
    ON_WM_NCLBUTTONDBLCLK()
    ON_WM_NCLBUTTONUP()
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
    ON_WM_GETMINMAXINFO()
    ON_WM_INITMENU()
    ON_WM_SYSCOMMAND()
    ON_WM_KEYDOWN()
    ON_WM_KEYUP()
END_MESSAGE_MAP()

#define MINI_CAP_HEIGHT 10
#define MINI_CAP_WIDTH 10

/////////////////////////////////////////////////////////////////////////////
// CMiniCaptionWnd construction/destruction/creation

/***************************************************************************/
// CMiniCaptionWnd::CMiniCaptionWnd Constructor
/***************************************************************************/
CMiniCaptionWnd::CMiniCaptionWnd() {
    // initialize variables
	// default is minicaption with caption text style
    m_nCaption = MiniWithCaption;
	// set it once and leave it alone.
	// m_iNCHeight is will be the height of the caption bar.
    m_iNCHeight = MINI_CAP_HEIGHT+2;
	// window has no focus
    m_bFocus = FALSE;
}

/***************************************************************************/
// CMiniCaptionWnd::~CMiniCaptionWnd Destructor
/***************************************************************************/
CMiniCaptionWnd::~CMiniCaptionWnd() {
}

/***************************************************************************/
// CMiniCaptionWnd::Create Creation
// Creates a child window with the given parameters.
/***************************************************************************/
BOOL CMiniCaptionWnd::Create(LPCTSTR lpszClassName,
                             LPCTSTR lpszWindowName,
                             DWORD dwStyle,
                             const RECT & rect,
                             CWnd * pParentWnd,
                             UINT nID, CCreateContext * pContext) {
    // can't use for desktop or pop-up windows (use CreateEx instead)
    ASSERT(pParentWnd != NULL);
    ASSERT((dwStyle & WS_POPUP) == 0);

    return CreateEx(0, lpszClassName, lpszWindowName,
                    dwStyle,
                    rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
                    pParentWnd->GetSafeHwnd(), (HMENU)nID, (LPSTR)pContext);
}

/***************************************************************************/
// CMiniCaptionWnd::PreCreateWindow Creation
// Called from the framework before the creation of the window. Registers
// the new window class and changes the style as desired.
/***************************************************************************/
BOOL CMiniCaptionWnd::PreCreateWindow(CREATESTRUCT & cs) {
    // register the window class
	int rezID = IDR_MAINFRAME_SA;
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
                                       AfxGetApp()->LoadStandardCursor(IDC_ARROW), 0,
                                       LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(rezID)));
    // precreate the window
    BOOL bRet = CWnd::PreCreateWindow(cs);
    // change the style
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();

	// default style for None, Mini and MiniWithCaption
	cs.style = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_BORDER | WS_SYSMENU;

	// use thickframe for everything except NoneThin
	if (pMainWnd->GetCaptionStyle() != NoneThin) {
		cs.style |= WS_THICKFRAME;
	}

	// for normal, enable all the normal menu stuff
	if (pMainWnd->GetCaptionStyle() == Normal) {
		cs.style |= WS_THICKFRAME | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
	}
    return bRet;
}

/////////////////////////////////////////////////////////////////////////////
// CMiniCaptionWnd helper functions

/***************************************************************************/
// CMiniCaptionWnd::RedrawCaption Redraws a window caption
// For some reason (focus change) the caption has to be redrawn, so this
// helper function can be called. It invalidates the caption rectangle of the
// minicaption window in the parent view (because this area is defined as non
// client, it cannot be invalidated directly). This will cause a draw message.
/***************************************************************************/
void CMiniCaptionWnd::RedrawCaption() {
    
	// invalidate caption rectangle
    CRect rcWnd;
    GetWindowRect(&rcWnd);
    
	ScreenToClient(rcWnd);
    RedrawWindow(rcWnd, NULL, RDW_FRAME|RDW_INVALIDATE);
}

/***************************************************************************/
// CMiniCaptionWnd::SetGraphFocus Set/Resets the focus
// If the minicaption window has to receive or loose focus, this function
// will do it. On receiving focus it will set the focus, bring the window to
// the top, save the new focus state and force a redraw of the caption.
/***************************************************************************/
void CMiniCaptionWnd::SetGraphFocus(BOOL bFocus) {
    if (bFocus) {
        SetFocus();
        BringWindowToTop();
    }
    m_bFocus = bFocus;
    RedrawCaption();
}

/***************************************************************************/
// CMiniCaptionWnd::CalcNCAreas Calculates non client areas
// This helper function can be called to calculate all non client areas in
// the caption of the minicaption window. The function needs pointers to
// rectangle structures for the result. If bClientCoords is TRUE, is calcu-
// lates with client, otherwise with screen coordinates.
// This is only used when the minicaption is enabled (caption or none)
/***************************************************************************/
void CMiniCaptionWnd::CalcNCAreas(BOOL isPrinting, const CRect & printRect, LPRECT prcNC, LPRECT prcBar, LPRECT prcSysMenu, LPRECT prcCaption,
                                  LPRECT prcRightmost, LPRECT prcRightTwo, LPRECT prcRightOne)

{
    // get window placement information
    WINDOWPLACEMENT wpl;
    wpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(&wpl);

	// we have to calculate the border in the event that the 
	// windows theme changes.
	CRect wRect;
	CRect cRect;
	GetWindowRect(&wRect);
	GetClientRect(&cRect);
	int border = (wRect.Width() - cRect.Width()) / 2;

	// get window coordinates
    CRect rcWnd;
    if (isPrinting) {
        rcWnd = printRect;
    } else {
		rcWnd = wRect;
    }

    // calculate the inside of the non-client area, which is the client area.
    // client coordinates
	// we will calculate everything in client coordinates, where the origin of the
	// upper-left corner of the window is 0,0.
    prcNC->top = border+m_iNCHeight;
    prcNC->left = border;
	prcNC->bottom = prcNC->top + cRect.Height();
	prcNC->right = prcNC->left + cRect.Width();

	// assume origin is upper left corner of window
	// calculate the menubar rectangle
	prcBar->top = border;
	prcBar->left = border;
	prcBar->bottom = prcBar->top + m_iNCHeight;
	prcBar->right = prcBar->left + cRect.Width();

	// gap between buttons and border and buttons
    int nButtonGap = 1;
	int nButtonWidth = m_iNCHeight;
	int nButtonHeight = m_iNCHeight;

    // set system menu button rectangle
	// little gap on the left for Win95
    prcSysMenu->left = prcBar->left;
    prcSysMenu->right = prcSysMenu->left + nButtonWidth;
    prcSysMenu->top = prcBar->top;
    prcSysMenu->bottom = prcSysMenu->top + nButtonHeight;

    *prcRightmost = *prcSysMenu;
    // set rightmost button rectangle
	prcRightmost->top += 1;
    prcRightmost->right = prcBar->right;
    prcRightmost->left = prcRightmost->right - nButtonWidth;

    // the maximize button goes to the immediate left of close (rightmost)
    *prcRightOne = *prcRightmost;
	// leave a gap for Win95
	prcRightOne->right = prcRightmost->left - nButtonGap;
    prcRightOne->left = prcRightOne->right - nButtonWidth;

    // the minimize button goes to the immediate left of maximize
    *prcRightTwo = *prcRightOne;
	// leave no gap
	prcRightTwo->right = prcRightOne->left - nButtonGap;
    prcRightTwo->left = prcRightTwo->right - nButtonWidth;

    // the caption is everything else
    *prcCaption = *prcNC;
    prcCaption->left = prcSysMenu->right + nButtonGap;
    prcCaption->right = prcRightTwo->left - nButtonGap;
}

/////////////////////////////////////////////////////////////////////////////
// CMiniCaptionWnd message handlers

/***************************************************************************/
// CMiniCaptionWnd::OnNcHitTest Mouse hit test
// Called from the framework, if the mouse pointer hits the windows non
// client area. Determines, on which button or other part the mouse hits and
// gives the result back as return parameter.
/***************************************************************************/
LRESULT CMiniCaptionWnd::OnNcHitTest(CPoint point) {

	// using the default caption, or no caption
    if ((m_nCaption == None) || (m_nCaption == NoneThin) || (m_nCaption == Normal)) {
		// no or normal caption window style
        return CWnd::OnNcHitTest(point);
    }
    // get window placement information
    WINDOWPLACEMENT wpl;
    wpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(&wpl);

	if (wpl.showCmd == SW_SHOWMINIMIZED) {
		// we are done...
		return CWnd::OnNcHitTest(point);
	}

	CRect wRect;
	GetWindowRect(&wRect);

    CRect rcBar, rcNC, rcSysMenu, rcRightmost, rcRightOne, rcRightTwo, rcCaption;
    CRect dummyRect;
    // calculate NC areas in screen, not client coordinates
    CalcNCAreas(FALSE, dummyRect, &rcNC, &rcBar, &rcSysMenu, &rcCaption, &rcRightmost, &rcRightTwo, &rcRightOne);

	// adjust to screen coordinates
	OffsetRect(&rcBar, wRect.left, wRect.top);
	OffsetRect(&rcSysMenu, wRect.left, wRect.top);
	OffsetRect(&rcRightmost, wRect.left, wRect.top);
	OffsetRect(&rcRightTwo, wRect.left, wRect.top);
	OffsetRect(&rcRightOne, wRect.left, wRect.top);

	// is it within the menubar?
    // for the hit test avoid the resize areas
	rcBar.InflateRect(-1, -1);
    // check the hit
	if (!rcBar.PtInRect(point)) {
		return CWnd::OnNcHitTest(point);
	}

    // we're in the NC area. Are we in the menu box or other special area?
    if (rcSysMenu.PtInRect(point)) {
		// system menu area
        return HTSYSMENU;
    }
    if (rcRightmost.PtInRect(point)) {
		// rightmost button
        return HTCLOSE;
    }
	// minimize for Win95, caption for Win31
    if (rcRightTwo.PtInRect(point)) {
        return HTMINBUTTON;
    }
    if (rcRightOne.PtInRect(point)) {
		// second right button
        return HTMAXBUTTON;
    }
	// the rest is caption
    return HTCAPTION;
}

/**
* CMiniCaptionWnd::OnNcCalcSize Non client area size calculation
* Called from the framework, for sizing of the NC area.
*/
void CMiniCaptionWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS * lpncsp) {
    
	// pickup the default calculations from the base window class
	CWnd::OnNcCalcSize( bCalcValidRects, lpncsp);

    // modes - NoneThin, None, Mini, MiniWithCaption, Normal,
	// for everything else, we are done.
    if ((m_nCaption == None) || (m_nCaption == NoneThin) || (m_nCaption == Normal)) {
		// standard
        return;
    }
    // adjust the client area to compensate for the nonclient area
	lpncsp->rgrc[0].top += m_iNCHeight;

    // if there's no room for the client area, set top and bottom equal to each other.
    // This should give the non-client area the room it needs
    if (lpncsp->rgrc[0].top > lpncsp->rgrc[0].bottom) {
        lpncsp->rgrc[0].bottom = lpncsp->rgrc[0].top;
    }
}

void CMiniCaptionWnd::OnNcPaint() {

	//this is used for the second parameter
	// of OnNcDraw which is only really needed for printing.
	CRect dummyRect(0,0,0,0);

    // paint the standard frame
    CWnd::OnNcPaint();

    CWindowDC dc(this);
    OnNcDraw(&dc, dummyRect);
}

void CMiniCaptionWnd::Darken(COLORREF & c) {

    BYTE r = GetRValue(c);
    BYTE g = GetGValue(c);
    BYTE b = GetBValue(c);
    BYTE grey = BYTE(((r*30) + (g*59) + (b*11))/100);
    if (grey > 162) {
        // to brite - darken
        r = BYTE((r > 128) ? (r-128) : 0);
        g = BYTE((g > 128) ? (g-128) : 0);
        b = BYTE((b > 128) ? (b-128) : 0);
        c = RGB(r,g,b);
    }
}

/**
* Draw the minicaption caption text
*/
void CMiniCaptionWnd::DrawCaption(CDC * pDC, LONG top, LONG left, LONG height, LONG width) {

	// draw the caption text
	// left text border
	CRect rcWnd(left,top,left+width,top+height);

	// create and select system font
	LOGFONT lFont;
	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lFont), &lFont, 0);
	if (lFont.lfHeight<0) {
		lFont.lfHeight += 2;
	} else {
		lFont.lfHeight = rcWnd.Height();
	}
	CFont font;
	font.CreateFontIndirect(&lFont);
	CFont * pOldFont = pDC->SelectObject(&font);

	// get the text to draw
	CString szText;
	GetWindowText(szText.GetBuffer(32), 32);
	szText.ReleaseBuffer(-1);

	// fit text into window
	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);
	// number of possible chars
	int nNumberChars = rcWnd.Width() / tm.tmAveCharWidth;
	if (szText.GetLength() > nNumberChars) {
		// limit text
		szText = szText.Left(nNumberChars - 2) + "...";
	}

	// draw the text (left aligned for Win95, centered in Win31
	pDC->DrawText(szText, -1, rcWnd, DT_VCENTER | DT_SINGLELINE | DT_LEFT);
	// allows for proper restoration
	pDC->SelectObject(pOldFont);
}

/***************************************************************************/
// CMiniCaptionWnd::OnNcPaint Non client area painting
// Called from the framework, for painting of the NC area.
/***************************************************************************/
void CMiniCaptionWnd::OnNcDraw(CDC * pDC, const CRect & printRect) {

	if ((m_nCaption == Normal) || (m_nCaption == None) || (m_nCaption == NoneThin)) {
		// do nothing
		return;
	}

	CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
	// get pointer to parent view
    CSaView * pView = (CSaView *)GetParent();
    Colors * pColors = pMainWnd->GetColors();

    // get window placement information
    CRect rcBar, rcCaption, rcSysMenu, rcRightmost, prcRightOne, prcRightTwo, rcNC;
    WINDOWPLACEMENT wpl;
    wpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(&wpl);
	if (wpl.showCmd == SW_SHOWMINIMIZED) {
		return;
	}

	CalcNCAreas(pDC->IsPrinting(),printRect, &rcNC, &rcBar, &rcSysMenu, &rcCaption, &rcRightmost, &prcRightTwo, &prcRightOne);

    // prepare drawing the buttons
    CDC * pDisplayMemDC = new CDC();
    pDisplayMemDC->CreateCompatibleDC(pDC);


	// draw the 3D frame for Win95 over the original frame
	// add the 3D frame inside
	CPen penLtLine(PS_SOLID, 1, pColors->cSysBtnShadow);
	CPen * poldPen = pDC->SelectObject(&penLtLine);
	// draw dark colored part
	// top shadow line needed
	pDC->MoveTo(rcNC.right, rcNC.top);
	pDC->LineTo(rcNC.left, rcNC.top);
	pDC->LineTo(rcNC.left, rcNC.bottom);
		
	CPen penDkLine(PS_SOLID, 1, pColors->cSysBtnHilite);
	pDC->SelectObject(&penDkLine);
	// draw light colored part
	pDC->MoveTo(rcNC.left + 1, rcNC.bottom);
	pDC->LineTo(rcNC.right, rcNC.bottom);
	pDC->LineTo(rcNC.right, rcNC.top-1);
	// restore old pen
	pDC->SelectObject(poldPen);

	// caption and bitmaps needed
    // set the colors as appropriate for whether this window has focus
    pDC->SetBkMode(TRANSPARENT);
    CBrush bshCaption;
    COLORREF textColor = pColors->cSysInActiveCapText;

    if ((m_bFocus) && (pView->ViewIsActive())) {
        // this graph and the parent view have focus
        bshCaption.CreateSolidBrush(pColors->cSysActiveCap);
		textColor = pColors->cSysCapText;
    } else {
        bshCaption.CreateSolidBrush(pColors->cSysInActiveCap);
    }
    if (pDC->IsPrinting()) {
        // we need a dark text color because the background is going to be white.
        Darken(textColor);
    }
    pDC->SetTextColor(textColor);

	if (pDC->IsPrinting()) {
		pDC->FrameRect(&rcBar, &bshCaption);
	} else {
		pDC->FillRect(&rcBar, &bshCaption);
	}

	// draw the miniaturized caption.
	if (m_nCaption == MiniWithCaption) {
		DrawCaption(pDC, rcBar.top, rcSysMenu.right + 2, m_iNCHeight - 2, prcRightTwo.left - rcSysMenu.right - 2 - 1);
	}

    // fill the button backgrounds
    CBrush bButton;
    bButton.CreateSolidBrush(pColors->cSysBtnFace);

    if (!pDC->IsPrinting()) {

        // draw the leftmost button
        // this is an icon in Win95
        CBitmap * pbmp = new CBitmap;
        pbmp->LoadBitmap(IDB_MINICAP_ICON);
		CBitmap * pbmpOld = (CBitmap *)pDisplayMemDC->SelectObject(pbmp);
        pDC->StretchBlt(rcSysMenu.left, rcSysMenu.top, rcSysMenu.Width(), rcSysMenu.Height(), pDisplayMemDC, 0, 0, 32, 32, SRCCOPY);
		// deletes pbmp
        delete pDisplayMemDC->SelectObject(pbmpOld);

        // go on on right. Close always in Win95, restore or max in Win31
        pbmp = new CBitmap();
        pbmp->LoadBitmap(IDB_MINICAP_CLOSE);
		CSize dim = pbmp->GetBitmapDimension();
        pbmpOld = (CBitmap *)pDisplayMemDC->SelectObject(pbmp);
        pDC->BitBlt(rcRightmost.left, rcRightmost.top, rcRightmost.Width(), rcRightmost.Height(), pDisplayMemDC, 0, 0, SRCCOPY);
		// deletes pbmp
        delete pDisplayMemDC->SelectObject(pbmpOld);

        // draw the maximize (minimize/restore always in Win31) button
        pbmp = new CBitmap();
        pbmp->LoadBitmap((wpl.showCmd == SW_SHOWMAXIMIZED) ? IDB_MINICAP_RESTORE : IDB_MINICAP_MAXIMIZE);
        pbmpOld = (CBitmap *)pDisplayMemDC->SelectObject(pbmp);
        pDC->BitBlt(prcRightOne.left, prcRightOne.top, prcRightOne.Width(), prcRightOne.Height(), pDisplayMemDC, 0,0, SRCCOPY);
		// deletes pbmp
        delete pDisplayMemDC->SelectObject(pbmpOld);

        // draw the minimize button (only Win95)
        pbmp = new CBitmap();
        pbmp->LoadBitmap((wpl.showCmd == SW_SHOWMINIMIZED) ? IDB_MINICAP_RESTORE : IDB_MINICAP_MINIMIZE);
        pbmpOld = (CBitmap *)pDisplayMemDC->SelectObject(pbmp);
        pDC->BitBlt(prcRightTwo.left, prcRightTwo.top, prcRightTwo.Width(), prcRightTwo.Height(), pDisplayMemDC, 0,0, SRCCOPY);
		// deletes pbmp
        delete pDisplayMemDC->SelectObject(pbmpOld);
    }
    delete pDisplayMemDC;
}

/***************************************************************************/
// CMiniCaptionWnd::OnEraseBkgnd Erasing background
/***************************************************************************/
BOOL CMiniCaptionWnd::OnEraseBkgnd(CDC * pDC) {
    // get background color from main frame
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    CRect rErase;
    GetUpdateRect(rErase);
    pDC->FillSolidRect(rErase, pMainWnd->GetColors()->cPlotBkg);
    return TRUE;
}

/***************************************************************************/
// CMiniCaptionWnd::OnNcLButtonDown Non client area mouse left button down
// Called from the framework, if the mouse pointer is over the NC area while
// the user presses the left mouse button down. This event has to set the
// focus to this window. For this reason the parent view is called.
/***************************************************************************/
void CMiniCaptionWnd::OnNcLButtonDown(UINT nHitTest, CPoint point) {
	// if no caption window style there is nothing else to do
	if (m_nCaption == None) {
		CWnd::OnNcLButtonDown(nHitTest, point);
		return;
	}
	TRACE("OnNcLButtonDown\n");

	WINDOWPLACEMENT wpl;
	wpl.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&wpl);
	if (wpl.showCmd == SW_SHOWMINIMIZED) {
		TRACE("early exit\n");
		CWnd::OnNcLButtonDown(nHitTest, point);
		return;
	}

	// set focused graph in parent view
	// get pointer to parent view
    CSaView * pView = (CSaView *)GetParent();
	// cast pointer
    CGraphWnd * pGraph = (CGraphWnd *)this;
    pView->SetFocusedGraph(pGraph);

	// minicaption or normal window style
    switch (nHitTest) {
    case HTMINBUTTON:
    case HTMAXBUTTON:
    case HTCLOSE:
		// dont't call base class OnNcLButtonDown for these three buttons!
        return;
    default:
        break;
    }

    // if maximized, no drag (caption), call CWnd only if not maximized
    if ((wpl.showCmd == SW_SHOWMAXIMIZED) && (nHitTest == HTCAPTION)) {
        return;
    }
    CWnd::OnNcLButtonDown(nHitTest, point);
}

/***************************************************************************/
// CMiniCaptionWnd::OnNcLButtonDblClk Non cl. area mouse left button dblclick
// Called from the framework, if the user doubleclicks on the NC area. There
// is some action necessary like maximizing or restoring the window.
/***************************************************************************/
void CMiniCaptionWnd::OnNcLButtonDblClk(UINT nHitTest, CPoint point) {
    
	// nothing more to do for non caption window style
    if (m_nCaption == None) {
        CWnd::OnNcLButtonDblClk(nHitTest, point);
        return;
    }
	TRACE("OnNcLButtonDblClk\n");

	// minicaption and normal window style
    WINDOWPLACEMENT wpl;
    wpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(&wpl);
	if (wpl.showCmd == SW_SHOWMINIMIZED) {
		CWnd::OnNcLButtonDblClk(nHitTest, point);
		return;
	}

    if (nHitTest == HTSYSMENU) {
		// system menu dblclk
		// in minimized state dblclk restores
        if (wpl.showCmd == SW_SHOWMINIMIZED) {
            wpl.showCmd = SW_RESTORE;
            SetWindowPlacement(&wpl);
        } else {
			// close the window
            OnSysCommand(SC_CLOSE, 0L);
        }
    } else {
        // caption-dblclicking operations.
        if (nHitTest == HTCAPTION) {
            switch (wpl.showCmd) {
            case SW_SHOWMAXIMIZED:
                wpl.showCmd = SW_RESTORE;
                break;
            case SW_SHOWMINIMIZED:
                wpl.showCmd = SW_RESTORE;
                break;
            case SW_SHOWNORMAL:
                wpl.showCmd = SW_SHOWMAXIMIZED;
                break;
            }
            SetWindowPlacement(&wpl);
            return;
        }
    }
    CWnd::OnNcLButtonDblClk(nHitTest, point);
}

/***************************************************************************/
// CMiniCaptionWnd::OnNcLButtonUp Non client area mouse left button up
// Called from the framework, if the user leaves the left mouse button up.
// Dependent from the area he hit, this will cause some action like maxi-
// mizing, restoring, minimizing the window.
/***************************************************************************/
void CMiniCaptionWnd::OnNcLButtonUp(UINT nHitTest, CPoint point) {

	// nothing more to do for non caption window style
    if (m_nCaption == None) {
        CWnd::OnNcLButtonUp(nHitTest, point);
        return;
    }
	TRACE("OnNcLButtonUp\n");

    // minicaption or normal window style
    WINDOWPLACEMENT wpl;
    wpl.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&wpl);
	if (wpl.showCmd == SW_SHOWMINIMIZED) {
		CWnd::OnNcLButtonUp(nHitTest, point);
		return;
	}

	switch (nHitTest) {
    case HTMINBUTTON:
        wpl.showCmd = (wpl.showCmd == SW_SHOWMINIMIZED) ? SW_RESTORE : SW_SHOWMINIMIZED;
        SetWindowPlacement(&wpl);
        break;
    case HTMAXBUTTON:
        wpl.showCmd = (wpl.showCmd == SW_SHOWMAXIMIZED) ? SW_RESTORE : SW_SHOWMAXIMIZED;
        SetWindowPlacement(&wpl);
        break;
    case HTCLOSE:
        OnSysCommand(SC_CLOSE, 0L);
        break;
    default:
		CWnd::OnNcLButtonUp(nHitTest, point);
    }
}

/***************************************************************************/
// CMiniCaptionWnd::OnLButtonDown Client area mouse left button down
// Called from the framework, if the mouse pointer is over the client area
// while the user presses the left mouse button down. This event has to set
// the focus to this window. For this reason the parent view is called.
/***************************************************************************/
void CMiniCaptionWnd::OnLButtonDown(UINT nFlags, CPoint point) {
    // set focused graph in parent view
	// get pointer to parent view
    CSaView * pView = (CSaView *)GetParent();
    pView->SetFocusedGraph((CGraphWnd *)this);
    CWnd::OnLButtonDown(nFlags, point);
}

/***************************************************************************/
// CMiniCaptionWnd::OnRButtonDown Client area mouse right button down
// Called from the framework, if the mouse pointer is over the client area
// while the user presses the right mouse button down. This event has to set
// the focus to this window. For this reason the parent view is called.
/***************************************************************************/
void CMiniCaptionWnd::OnRButtonDown(UINT nFlags, CPoint point) {
    // set focused graph in parent view
	// get pointer to parent view
    CSaView * pView = (CSaView *)GetParent();
	// set focused graph
    pView->SetFocusedGraph((CGraphWnd *)this);
    CWnd::OnRButtonDown(nFlags, point);
}

/***************************************************************************/
// CMiniCaptionWnd::OnGetMinMaxInfo System needs min or max information
// Called from the framework, if the system needs the min or max position
// and size information from the window. This function manipulates the
// maximum size, to fit the window into the parents client area.
/***************************************************************************/
void CMiniCaptionWnd::OnGetMinMaxInfo(MINMAXINFO * lpMMI) {
    // get the parents client coordinates
    CRect rWnd;
    GetParent()->GetClientRect(rWnd);
    // modify the minmaxinfo
    lpMMI->ptMaxSize.x = rWnd.Width() + 2 * GetSystemMetrics(SM_CXFRAME);
    lpMMI->ptMaxSize.y = rWnd.Height() + 2 * GetSystemMetrics(SM_CYFRAME);
    lpMMI->ptMaxPosition.x = rWnd.left - GetSystemMetrics(SM_CXFRAME);
    lpMMI->ptMaxPosition.y = rWnd.top - GetSystemMetrics(SM_CYFRAME);
    // call the base class function
    CWnd::OnGetMinMaxInfo(lpMMI);
}

/***************************************************************************/
// CMiniCaptionWnd::OnInitMenu System menu initialisation
/***************************************************************************/
void CMiniCaptionWnd::OnInitMenu(CMenu *) {
    // get copy of system menu
    CMenu * pSysMenu = GetSystemMenu(FALSE);
    // get window placement information
    WINDOWPLACEMENT wpl;
    wpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(&wpl);
    // modify menu for maximized state
    if (wpl.showCmd == SW_SHOWMAXIMIZED) {
        pSysMenu->EnableMenuItem(SC_RESTORE, MF_BYCOMMAND | MF_ENABLED);
        pSysMenu->EnableMenuItem(SC_MOVE, MF_BYCOMMAND | MF_GRAYED);
        pSysMenu->EnableMenuItem(SC_SIZE, MF_BYCOMMAND | MF_GRAYED);
        pSysMenu->EnableMenuItem(SC_MINIMIZE, MF_BYCOMMAND | MF_ENABLED);
        pSysMenu->EnableMenuItem(SC_MAXIMIZE, MF_BYCOMMAND | MF_GRAYED);
    } else {
        // modify menu for minimized state
        if (wpl.showCmd == SW_SHOWMINIMIZED) {
            pSysMenu->EnableMenuItem(SC_RESTORE, MF_BYCOMMAND | MF_ENABLED);
            pSysMenu->EnableMenuItem(SC_MOVE, MF_BYCOMMAND | MF_ENABLED);
            pSysMenu->EnableMenuItem(SC_SIZE, MF_BYCOMMAND | MF_GRAYED);
            pSysMenu->EnableMenuItem(SC_MINIMIZE, MF_BYCOMMAND | MF_GRAYED);
            pSysMenu->EnableMenuItem(SC_MAXIMIZE, MF_BYCOMMAND | MF_ENABLED);
        } else {
            // modify menu for normal state
            pSysMenu->EnableMenuItem(SC_RESTORE, MF_BYCOMMAND | MF_GRAYED);
            pSysMenu->EnableMenuItem(SC_MOVE, MF_BYCOMMAND | MF_ENABLED);
            pSysMenu->EnableMenuItem(SC_SIZE, MF_BYCOMMAND | MF_ENABLED);
            pSysMenu->EnableMenuItem(SC_MINIMIZE, MF_BYCOMMAND | MF_ENABLED);
            pSysMenu->EnableMenuItem(SC_MAXIMIZE, MF_BYCOMMAND | MF_ENABLED);
        }
    }
}

/***************************************************************************/
// CMiniCaptionWnd::OnSysCommand System menu command
/***************************************************************************/
void CMiniCaptionWnd::OnSysCommand(UINT nID, LPARAM lParam) {
    switch (nID) {
    case SC_KEYMENU:
        GetParent()->SendMessage(WM_SYSCOMMAND,nID,lParam);
        return;
    case SC_CLOSE:
        // this causes the parent to be notified and destroy me
		// inform parent
        GetParent()->PostMessage(WM_USER_GRAPH_DESTROYED, 0, (LONG)this);
        break;
    case SC_MAXIMIZE:
    case SC_MINIMIZE:
    case SC_RESTORE:
    default:
        break;
    }
    CWnd::OnSysCommand(nID, lParam);
}

/***************************************************************************/
// CMiniCaptionWnd::OnKeyDown Key down command
// Because this window has input focus it receives the WM_KEYDOWN messages
// the parent view should receive. So it sends the messages further to the
// parent view.
/***************************************************************************/
void CMiniCaptionWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
    GetParent()->SendMessage(WM_KEYDOWN, nChar, MAKELONG(nRepCnt, nFlags));
    CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CMiniCaptionWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
    GetParent()->SendMessage(WM_KEYUP, nChar, MAKELONG(nRepCnt, nFlags));
    CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CMiniCaptionWnd::DrawCaptionText(CDC * pDC, const CRect & rcWnd) {

	// get pointer to parent view
    CSaView * pView = (CSaView *)GetParent();
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();

    WINDOWPLACEMENT wpl;
    wpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(&wpl);
    Colors * pColors = pMainWnd->GetColors();
    COLORREF textColor = pColors->cSysInActiveCapText;

    if ((m_bFocus) && (pView->ViewIsActive())) {
        // this graph and the parent view have focus
        textColor = pColors->cSysCapText;
    }
    if (pDC->IsPrinting()) {
        // we need a dark text color because the background
        // is going to be white.
        Darken(textColor);
    }
    pDC->SetTextColor(textColor);

    LOGFONT lFont;
    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lFont), &lFont, 0);

    lFont.lfWeight = FW_EXTRABOLD;

    CFont font;
    font.CreateFontIndirect(&lFont);
    CFont * pOldFont = pDC->SelectObject(&font);

    // get the text to draw
    CString szText;
    GetWindowText(szText.GetBuffer(32), 32);
    szText.ReleaseBuffer(-1);

    // fit text into window
    TEXTMETRIC tm;
    pDC->GetTextMetrics(&tm);
	// number of possible chars
    int nNumberChars = rcWnd.Width() / tm.tmAveCharWidth;
    if (szText.GetLength() > nNumberChars) {
		// limit text
        szText = szText.Left(nNumberChars - 2) + "...";
    }

    // draw the text (left aligned for Win95, centered in Win31
    CRect copyRcWnd(rcWnd);
    pDC->DrawText(szText, -1, &copyRcWnd, DT_VCENTER | DT_SINGLELINE | DT_LEFT);
	// allows for proper restoration
    pDC->SelectObject(pOldFont);
}
