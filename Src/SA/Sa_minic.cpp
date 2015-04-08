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

/////////////////////////////////////////////////////////////////////////////
// CMiniCaptionWnd construction/destruction/creation

/***************************************************************************/
// CMiniCaptionWnd::CMiniCaptionWnd Constructor
/***************************************************************************/
CMiniCaptionWnd::CMiniCaptionWnd() {
    // initialize variables
    m_nCaption = MiniWithCaption; // default is minicaption with caption text style

    m_iNCHeight = GetSystemMetrics(SM_CYCAPTION) / 2;
    m_ixSizFrame = GetSystemMetrics(SM_CXFRAME);
    m_iySizFrame = GetSystemMetrics(SM_CYFRAME);
    m_bFocus = FALSE; // window has no focus
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
	CSaApp * pApp = (CSaApp*)AfxGetApp();
	int rezID = (pApp->IsAudioSync())?IDR_MAINFRAME_AS:IDR_MAINFRAME_SA;
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
                                       AfxGetApp()->LoadStandardCursor(IDC_ARROW), 0,
                                       LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(rezID)));
    // precreate the window
    BOOL bRet = CWnd::PreCreateWindow(cs);
    // change the style
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    if (pMainWnd->GetCaptionStyle() == NoneThin) { // thin window border
        cs.style = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_BORDER | WS_SYSMENU;
    } else { // normal (thick frame) window border
        if (pMainWnd->GetCaptionStyle() == Normal) { // normal window style
            cs.style = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_THICKFRAME | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_BORDER | WS_SYSMENU;
        } else {
            cs.style = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_THICKFRAME | WS_BORDER | WS_SYSMENU;
        }
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
    rcWnd.bottom = rcWnd.top + m_iNCHeight;
    rcWnd.top += m_iySizFrame;
    rcWnd.left += m_ixSizFrame;
    rcWnd.right -= m_ixSizFrame;
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
/***************************************************************************/
void CMiniCaptionWnd::CalcNCAreas(BOOL isPrinting, const CRect & printRect, BOOL printDoubleHeight,
                                  LPRECT prcNC, LPRECT prcSysMenu, LPRECT prcCaption,
                                  LPRECT prcRightmost, LPRECT prcRightTwo, LPRECT prcRightOne,
                                  LPRECT prcFrame, BOOL bClientCoords)

{
    // get window placement information
    WINDOWPLACEMENT wpl;
    wpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(&wpl);

    // get window coordinates
    CRect rcWnd;
    int ncHeight;

    if (isPrinting) {
        rcWnd = printRect;
        ncHeight = (printDoubleHeight) ? m_iNCHeight * 2 : m_iNCHeight;
    } else {
        ncHeight = m_iNCHeight;
        GetWindowRect(&rcWnd);
    }

    // set correction factor for Win95
    int nCorr = 0; // correction because of 3D border for Win95
    if (wpl.showCmd != SW_SHOWMAXIMIZED) {
        nCorr = 1;
    }
    // set caption frame rectangle
    if (bClientCoords) {
        // client coordinates
        prcNC->top = 0;
        prcNC->left = m_ixSizFrame - nCorr;
        prcNC->right = rcWnd.Width() - m_ixSizFrame + nCorr;
    } else {
        // screen coordinates
        prcNC->top = rcWnd.top;
        prcNC->left = rcWnd.left + m_ixSizFrame - nCorr;
        prcNC->right = rcWnd.right - m_ixSizFrame + nCorr;
    }
    prcNC->bottom = prcNC->top + ncHeight + m_iySizFrame - nCorr;
    prcNC->top += m_iySizFrame - nCorr; // avoids the frame
    // calculate vertical button position and sizes
    int nButtonTop, nButtonWidth, nButtonHeight, nButtonGap = 0;
    nButtonGap = (ncHeight + 1) / 10; // gap between buttons and border and buttons
    nButtonTop = prcNC->top;
    if (m_nCaption != Normal) {
        nButtonWidth = MINICAPTION_HEIGHT + 1;
    } else {
        nButtonWidth = GetSystemMetrics(SM_CXSIZE);
    }
    nButtonHeight = MINICAPTION_HEIGHT;
    // set non client frame rectangle
    prcFrame->left = prcNC->left;
    prcFrame->top = prcNC->bottom;
    prcFrame->right = prcNC->right - nCorr;
    prcFrame->bottom = rcWnd.Height() - m_iySizFrame;

    // set system menu button rectangle (is icon in Win95)
    prcSysMenu->left = prcNC->left + nButtonGap; // little gap on the left for Win95
    prcSysMenu->right = prcSysMenu->left + nButtonWidth;
    prcSysMenu->top = nButtonTop;
    prcSysMenu->bottom = prcSysMenu->top + nButtonHeight;

    *prcRightmost = *prcSysMenu;
    // set rightmost button rectangle
    prcRightmost->right = prcNC->right - nButtonGap; // little gap on the right for Win95
    prcRightmost->left = prcRightmost->right - nButtonWidth;
    // in Win95 the maximize button goes to the immediate left of close (rightmost)
    *prcRightOne = *prcRightmost;
    prcRightOne->right = prcRightmost->left - nButtonGap; // leave a gap for Win95
    prcRightOne->left = prcRightOne->right - nButtonWidth;
    // in Win95 the minimize button goes to the immediate left of maximize
    *prcRightTwo = *prcRightOne;
    prcRightTwo->right = prcRightOne->left; // leave no gap
    prcRightTwo->left = prcRightTwo->right - nButtonWidth;

    // the caption is everything else
    *prcCaption = *prcNC;
    prcCaption->left = prcSysMenu->right;
    prcCaption->right = prcRightTwo->left;
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
    if ((m_nCaption == None) || (m_nCaption == NoneThin) || (m_nCaption == Normal)) {
        return CWnd::OnNcHitTest(point);    // no or normal caption window style
    }

    // get window placement information
    WINDOWPLACEMENT wpl;
    wpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(&wpl);
    // minicaption window style
    CRect rcWnd, rcNC, rcSysMenu, rcRightmost, rcRightOne, rcRightTwo, rcCaption, rcFrame;
    GetWindowRect(rcWnd);
    CRect dummyRect;
    // calculate NC areas in screen, not client coordinates
    CalcNCAreas(FALSE,dummyRect, FALSE, &rcNC, &rcSysMenu, &rcCaption, &rcRightmost, &rcRightTwo, &rcRightOne, &rcFrame, FALSE);

    // for the hit test avoid the resize areas
    rcNC.InflateRect(-1, -1);
    // check the hit
    if (rcNC.PtInRect(point)) {
        // we're in the NC area. Are we in the menu box or other special area?
        if (rcSysMenu.PtInRect(point)) {
            return HTSYSMENU;    // system menu area
        }
        if (rcRightmost.PtInRect(point)) {
            return HTCLOSE;    // rightmost button
        }
        if (rcRightTwo.PtInRect(point)) { // minimize for Win95, caption for Win31
            return HTMINBUTTON;
        }
        if (rcRightOne.PtInRect(point)) {
            return HTMAXBUTTON;    // second right button
        }
        // in minimized state and win95 caption has system menu function
        if (wpl.showCmd == SW_SHOWMINIMIZED) {
            return HTCAPTION;
        }
        return HTCAPTION; // the rest is caption
    } else {
        // in minimized state room under caption (Win95) has system menu function
        if (wpl.showCmd == SW_SHOWMINIMIZED) {
            return HTCAPTION;
        }
        return CWnd::OnNcHitTest(point);
    }
}

/***************************************************************************/
// CMiniCaptionWnd::OnNcCalcSize Non client area size calculation
// Called from the framework, for sizeing of the NC area.
/***************************************************************************/
void CMiniCaptionWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS * lpncsp) {
    CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);

    // modes - NoneThin, None, Mini, MiniWithCaption, Normal,
    if ((m_nCaption == None) ||
            (m_nCaption == NoneThin) ||
            (m_nCaption == Normal)) {
        return; // standard
    }

    // all window styles, get window placement information
    WINDOWPLACEMENT wpl;
    wpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(&wpl);

    // adjust the client area to compensate for the nonclient area
    if (wpl.showCmd != SW_SHOWMAXIMIZED) {
        // window is not maximized
        lpncsp->rgrc[0].top += m_iNCHeight - 1;
    } else {
        // window is maximized
        lpncsp->rgrc[0].top += m_iNCHeight; // because of 3D border for Win95
        lpncsp->rgrc[0].bottom -= 1;
        lpncsp->rgrc[0].left += 1;
        lpncsp->rgrc[0].right -= 1;
    }

    // if there's no room for the client area, set top and bottom equal to each other.
    // This should give the non-client area the room it needs
    if (lpncsp->rgrc[0].top > lpncsp->rgrc[0].bottom) {
        lpncsp->rgrc[0].bottom = lpncsp->rgrc[0].top;
    }
}

void CMiniCaptionWnd::OnNcPaint() {
    CRect dummyRect(0,0,0,0); //this is used for the second parameter
    // of OnNcDraw which is only really needed
    // for printing.

    // paint the standard frame
    CWnd::OnNcPaint();
    if (m_nCaption == NoneThin) {
        return;    // nothing else to draw
    }
    CWindowDC dc(this);
    OnNcDraw(&dc, dummyRect, FALSE);
}

void Darken(COLORREF & c) {
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

/***************************************************************************/
// CMiniCaptionWnd::OnNcPaint Non client area painting
// Called from the framework, for painting of the NC area.
/***************************************************************************/
void CMiniCaptionWnd::OnNcDraw(CDC * pDC, const CRect & printRect, BOOL bPrintDoubleHeight) {
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    CSaView * pView = (CSaView *)GetParent(); // get pointer to parent view
    Colors * pColors = pMainWnd->GetColors();

    // get window placement information
    CRect rcWnd, rcCaption, rcSysMenu, rcRightmost, prcRightOne, prcRightTwo, rcNC, rcFrame;
    WINDOWPLACEMENT wpl;
    wpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(&wpl);
    CalcNCAreas(pDC->IsPrinting(),printRect, bPrintDoubleHeight, &rcNC, &rcSysMenu, &rcCaption, &rcRightmost, &prcRightTwo, &prcRightOne, &rcFrame);

    // modes - NoneThin, None, Mini, MiniWithCaption, Normal,

    if (m_nCaption != Normal) {
        // prepare drawing the buttons
        CBitmap * pbmp, *pbmpOld;
        CDC * pDisplayMemDC = new CDC;
        pDisplayMemDC->CreateCompatibleDC(pDC);

        // draw the caption
        if ((m_nCaption == Mini) || (m_nCaption == MiniWithCaption)) { // caption and bitmaps needed
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

            if (wpl.showCmd == SW_SHOWMINIMIZED) { // Win95 minimized
                // draw the caption background for minimized Win95 or normal size caption
                rcWnd = rcNC;
                // this rectangle is in absolute windows coordindates
                CRect rTemp;
                GetWindowRect(rTemp);
                int width = rTemp.Width();
                TRACE("width=%d left=%d right=%d ",width,rTemp.left,rTemp.right);
                rcWnd.left = 4;
                rcWnd.right = width-4;
                rcWnd.top = 4;
                rcWnd.bottom = rcWnd.top + rTemp.Height() - (2*4);
                TRACE("width=%d left=%d right=%d\n",width,rcWnd.left,rcWnd.right);
                if (pDC->IsPrinting()) {
                    pDC->FrameRect(&rcWnd, &bshCaption);
                } else {
                    pDC->FillRect(&rcWnd, &bshCaption);
                }

                // draw the caption text
                rcWnd.left = rcSysMenu.right+6;     // left text border
                rcWnd.right = prcRightTwo.left - 1; // right text border
                rcWnd.bottom -= 1;

                // create and select system font
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
                int nNumberChars = rcWnd.Width() / tm.tmAveCharWidth; // number of possible chars
                if (szText.GetLength() > nNumberChars) {
                    szText = szText.Left(nNumberChars - 2) + "...";    // limit text
                }
                // draw the text (left aligned for Win95, centered in Win31
                pDC->DrawText(szText, -1, rcWnd, DT_VCENTER | DT_SINGLELINE | DT_LEFT);
                pDC->SelectObject(pOldFont); // allows for proper restoration

            } else {
                // maximimized
                if (pDC->IsPrinting()) {
                    pDC->FrameRect(&rcNC, &bshCaption);
                } else {
                    pDC->FillRect(&rcNC, &bshCaption); // normal case
                }

                if (m_nCaption == MiniWithCaption) { // draw the miniaturized caption.
                    rcWnd = rcNC;

                    // draw the caption text
                    rcWnd.left = rcSysMenu.right + 2; // left text border
                    rcWnd.right = prcRightTwo.left - 1; // right text border
                    rcWnd.bottom -= 2;

                    // create and select system font
                    LOGFONT lFont;
                    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lFont), &lFont, 0);
                    if (lFont.lfHeight<0) {
                        lFont.lfHeight+=2;
                    } else {
                        lFont.lfHeight = rcWnd.bottom - rcWnd.top - 1;
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
                    int nNumberChars = rcWnd.Width() / tm.tmAveCharWidth; // number of possible chars
                    if (szText.GetLength() > nNumberChars) {
                        szText = szText.Left(nNumberChars - 2) + "...";    // limit text
                    }

                    // draw the text (left aligned for Win95, centered in Win31
                    pDC->DrawText(szText, -1, rcWnd, DT_VCENTER | DT_SINGLELINE | DT_LEFT);
                    pDC->SelectObject(pOldFont); // allows for proper restoration
                }
            }

            // fill the button backgrounds
            CBrush bButton;
            bButton.CreateSolidBrush(pColors->cSysBtnFace);

            if (!pDC->IsPrinting()) {
                pDC->FillRect(&rcRightmost, &bButton);
                pDC->FillRect(&prcRightOne, &bButton);
                pDC->FillRect(&prcRightTwo, &bButton);

                // draw the leftmost button
                // this is an icon in Win95
                pbmp = new CBitmap;
                pbmp->LoadBitmap(IDB_MINICAP_ICON);
                pbmpOld = (CBitmap *)pDisplayMemDC->SelectObject(pbmp);
                pDC->StretchBlt(rcSysMenu.left, rcSysMenu.top, rcSysMenu.Width(), rcSysMenu.Height(), pDisplayMemDC, 0, 0, 32, 32, SRCCOPY);
                delete pDisplayMemDC->SelectObject(pbmpOld); // deletes pbmp

                // go on on right. Close always in Win95, restore or max in Win31
                pbmp = new CBitmap;
                pbmp->LoadBitmap(IDB_MINICAP_CLOSE);
                pbmpOld = (CBitmap *)pDisplayMemDC->SelectObject(pbmp);
                pDC->BitBlt(rcRightmost.left, rcRightmost.top, rcRightmost.Width(), rcRightmost.Height(), pDisplayMemDC, 0,0, SRCCOPY);
                delete pDisplayMemDC->SelectObject(pbmpOld); // deletes pbmp

                // draw the maximize (minimize/restore always in Win31) button
                pbmp = new CBitmap;
                pbmp->LoadBitmap((wpl.showCmd == SW_SHOWMAXIMIZED) ? IDB_MINICAP_RESTORE : IDB_MINICAP_MAXIMIZE);
                pbmpOld = (CBitmap *)pDisplayMemDC->SelectObject(pbmp);
                pDC->BitBlt(prcRightOne.left, prcRightOne.top, prcRightOne.Width(), prcRightOne.Height(), pDisplayMemDC, 0,0, SRCCOPY);
                delete pDisplayMemDC->SelectObject(pbmpOld); // deletes pbmp

                // draw the minimize button (only Win95)
                pbmp = new CBitmap;
                pbmp->LoadBitmap((wpl.showCmd == SW_SHOWMINIMIZED) ? IDB_MINICAP_RESTORE : IDB_MINICAP_MINIMIZE);
                pbmpOld = (CBitmap *)pDisplayMemDC->SelectObject(pbmp);
                pDC->BitBlt(prcRightTwo.left, prcRightTwo.top, prcRightTwo.Width(), prcRightTwo.Height(), pDisplayMemDC, 0,0, SRCCOPY);
                delete pDisplayMemDC->SelectObject(pbmpOld); // deletes pbmp
            } // end if IsPrinting
        } else {
            TRACE(_T("Caption requested\n"));
        }
        delete pDisplayMemDC;
    } else {
        // calculate frame for normal caption style
        GetClientRect(rcFrame);
        rcFrame.OffsetRect(GetSystemMetrics(SM_CXFRAME) - 1, GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME) - 1);
        rcFrame.right++;
    }

    // draw the 3D frame for Win95 over the original frame
    if (wpl.showCmd != SW_SHOWMINIMIZED) {
        // add the 3D frame inside
        CPen penLtLine(PS_SOLID, 1, pColors->cSysBtnShadow);
        CPen * poldPen = pDC->SelectObject(&penLtLine);
        // draw dark colored part
        if ((m_nCaption != Mini) || (m_nCaption != MiniWithCaption)) {
            // top shadow line needed
            pDC->MoveTo(rcFrame.right, rcFrame.top - 1);
            pDC->LineTo(rcFrame.left, rcFrame.top - 1);
        } else {
            pDC->MoveTo(rcFrame.left, rcFrame.top);
        }
        pDC->LineTo(rcFrame.left, rcFrame.bottom);
        CPen penDkLine(PS_SOLID, 1, pColors->cSysBtnHilite);
        pDC->SelectObject(&penDkLine);

        // draw light colored part
        pDC->LineTo(rcFrame.right, rcFrame.bottom);
        pDC->LineTo(rcFrame.right, rcFrame.top - 1);

        // restore old pen
        pDC->SelectObject(poldPen);
    }

    if (m_nCaption == Normal) {
        // draw the caption except for the bitmaps
        pDC->SetBkMode(TRANSPARENT);

        // set the colors as appropriate for whether this window has focus
        CBrush  bshCaption;
        CPen    penLine;

        if ((m_bFocus) && (pView->ViewIsActive())) {
            // this graph and the parent view have focus
            bshCaption.CreateSolidBrush(pColors->cSysActiveCap);
            penLine.CreatePen(PS_SOLID, 1, pColors->cSysActiveCap);
        } else {
            bshCaption.CreateSolidBrush(pColors->cSysInActiveCap);
            penLine.CreatePen(PS_SOLID, 1, pColors->cSysInActiveCap);
        }
        CPen * poldPen = pDC->SelectObject(&penLine);

        // draw the caption background
        rcWnd = rcNC;
        {
            rcWnd.left  = rcSysMenu.right - 1;
            rcWnd.right = prcRightTwo.left + 5; // right text border
            if (wpl.showCmd == SW_SHOWMINIMIZED) { // Win95 minimized
                rcWnd.bottom--;
                rcWnd.left --;
                rcWnd.right++; // right text border
            } else {
                rcWnd.top++;
            }

            if (GetSystemMetrics(SM_CYSIZE) >= 19) {
                rcWnd.left--;
            }

            if (GetSystemMetrics(SM_CYSIZE) > 19) {
                rcWnd.right += 2;
                rcWnd.left--;
            }

            // fill below the buttons
            CRect rTempRight(rcWnd.right, rcRightmost.top + GetSystemMetrics(SM_CYSIZE) - 1, rcFrame.right, rcWnd.bottom);
            if (wpl.showCmd == SW_SHOWMINIMIZED) {
                rTempRight.top--;
                rTempRight.right -= (2 * GetSystemMetrics(SM_CXFRAME) - 1);
            }
            pDC->FillRect(&rTempRight, &bshCaption);

            CRect rTempLeft(rcFrame.left, rTempRight.top + 1, rcWnd.left, rTempRight.bottom);
            if (wpl.showCmd != SW_SHOWMINIMIZED) {
                rTempLeft.left++;
            }
            if (GetSystemMetrics(SM_CYSIZE) >= 19) {
                rTempLeft.top--;
            }
            pDC->FillRect(&rTempLeft, &bshCaption);

            // lines on top
            pDC->MoveTo(rTempLeft.left, rcWnd.top);
            pDC->LineTo(rTempRight.right, rcWnd.top);
            pDC->MoveTo(rTempRight.left, rcWnd.top + 1);
            pDC->LineTo(rTempRight.right, rcWnd.top + 1);

            // left of system menu
            CRect rTemp(rTempLeft.left, rcWnd.top, rcSysMenu.left, rcWnd.bottom);
            if (wpl.showCmd != SW_SHOWMINIMIZED) {
                rTemp.right++;
            }
            pDC->FillRect(&rTemp, &bshCaption);

            // right of close button
            rTemp.left = rTempRight.right - rTemp.Width();
            rTemp.right = rTempRight.right;
            pDC->FillRect(&rTemp, &bshCaption);

            // left of close button
            rTemp.OffsetRect(-GetSystemMetrics(SM_CXSIZE), 0);
            pDC->FillRect(&rTemp, &bshCaption);
        }

        // this graph and the parent view have focus
        if (pDC->IsPrinting()) {
            pDC->FrameRect(&rcWnd, &bshCaption);
        } else {
            rcWnd.left = GetSystemMetrics(SM_CXSIZE)-9;
            pDC->FillRect(&rcWnd, &bshCaption);
        }

        // draw the caption text
        rcWnd.bottom -= 1;
        rcWnd.left += 3;

        // restore old pen
        pDC->SelectObject(poldPen);
        DrawCaptionText(pDC, rcWnd);
    }
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
    // set focused graph in parent view
    CSaView * pView = (CSaView *)GetParent(); // get pointer to parent view
    CGraphWnd * pGraph = (CGraphWnd *)this; // cast pointer
    pView->SetFocusedGraph(pGraph);
    // if no caption window style there is nothing else to do
    if (m_nCaption == None) {
        CWnd::OnNcLButtonDown(nHitTest, point);
        return;
    }
    // minicaption or normal window style
    switch (nHitTest) {
    case HTMINBUTTON:
    case HTMAXBUTTON:
    case HTCLOSE:
        return;
        break; // dont't call base class OnNcLButtonDown for this three buttons!
    default:
        break;
    }
    // get window placement information
    WINDOWPLACEMENT wpl;
    wpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(&wpl);
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
    // minicaption and normal window style
    WINDOWPLACEMENT wpl;
    wpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(&wpl);
    if (nHitTest == HTSYSMENU) { // system menu dblclk
        if (wpl.showCmd == SW_SHOWMINIMIZED) { // in minimized state dblclk restores
            wpl.showCmd = SW_RESTORE;
            SetWindowPlacement(&wpl);
        } else {
            OnSysCommand(SC_CLOSE, 0L);    // close the window
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
    CWnd::OnNcLButtonUp(nHitTest, point);

    // minicaption or normal window style
    WINDOWPLACEMENT wpl;
    wpl.length = sizeof(WINDOWPLACEMENT);
    switch (nHitTest) {
    case HTMINBUTTON:
        TRACE(_T("NC min button\n"));
        GetWindowPlacement(&wpl);
        wpl.showCmd = (wpl.showCmd == SW_SHOWMINIMIZED) ? SW_RESTORE : SW_SHOWMINIMIZED;
        SetWindowPlacement(&wpl);
        break;
    case HTMAXBUTTON:
        TRACE(_T("NC max button\n"));
        GetWindowPlacement(&wpl);
        wpl.showCmd = (wpl.showCmd == SW_SHOWMAXIMIZED) ? SW_RESTORE : SW_SHOWMAXIMIZED;
        SetWindowPlacement(&wpl);
        break;
    case HTCLOSE:
        TRACE(_T("NC close button\n"));
        OnSysCommand(SC_CLOSE, 0L);
        break;
    default:
        CWnd::OnNcLButtonDown(nHitTest, point);
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
    CSaView * pView = (CSaView *)GetParent(); // get pointer to parent view
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
    CSaView * pView = (CSaView *)GetParent(); // get pointer to parent view
    pView->SetFocusedGraph((CGraphWnd *)this); // set focused graph
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
        GetParent()->PostMessage(WM_USER_GRAPH_DESTROYED, 0, (LONG)this); // inform parent
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
    CSaView * pView = (CSaView *)GetParent(); // get pointer to parent view
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
    int nNumberChars = rcWnd.Width() / tm.tmAveCharWidth; // number of possible chars
    if (szText.GetLength() > nNumberChars) {
        szText = szText.Left(nNumberChars - 2) + "..."; // limit text
    }

    // draw the text (left aligned for Win95, centered in Win31
    CRect copyRcWnd(rcWnd);
    pDC->DrawText(szText, -1, &copyRcWnd, DT_VCENTER | DT_SINGLELINE | DT_LEFT);

    pDC->SelectObject(pOldFont); // allows for proper restoration
}
