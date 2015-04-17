#include "stdafx.h"
#include "PrivateCursorWnd.h"
#include "MainFrm.h"
#include "Sa_plot.h"
#include "Sa_graph.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CPrivateCursorWnd
// Private cursor window, contains a cursor line and is normally drawn over
// the graph. The windows width is normally more than one pixel and if the
// user moves the mouse pointer over it, the pointer changes his look. But
// if he drags the cursor, the windows size is reduced to one pixel to
// enable quick and easy drag.

/////////////////////////////////////////////////////////////////////////////
// CPrivateCursorWnd message map

BEGIN_MESSAGE_MAP(CPrivateCursorWnd, CCursorWnd)
    ON_WM_PAINT()
    ON_WM_MOUSEMOVE()
    ON_WM_RBUTTONDOWN()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_TIMER()
END_MESSAGE_MAP()

/***************************************************************************/
// CPrivateCursorWnd::CPrivateCursorWnd Constructor
/***************************************************************************/
CPrivateCursorWnd::CPrivateCursorWnd() {
    m_bCursorDrag = false;
    created = false;
    m_rWnd.SetRect(0, 0, 0, 0);
    hidden = false;
}

/***************************************************************************/
// CPrivateCursorWnd::~CPrivateCursorWnd Destructor
/***************************************************************************/
CPrivateCursorWnd::~CPrivateCursorWnd() {
}

/***************************************************************************/
// CPrivateCursorWnd::Create Creation
// Creates a child window with the given parameters.
/***************************************************************************/
BOOL CPrivateCursorWnd::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
                               const RECT & rect, CWnd * pParentWnd, UINT /*nID*/, CCreateContext * /*pContext*/) {
    BOOL bResult = CWnd::CreateEx(WS_EX_TRANSPARENT, lpszClassName, lpszWindowName, dwStyle, rect.left,
                                  rect.top, rect.right - rect.left, rect.bottom - rect.top,
                                  pParentWnd->GetSafeHwnd(), 0);
    created = (bResult!=FALSE);
    return bResult;
}

/***************************************************************************/
// CPrivateCursorWnd::PreCreateWindow Creation
// Called from the framework before the creation of the window. Registers
// the new window class.
/***************************************************************************/
BOOL CPrivateCursorWnd::PreCreateWindow(CREATESTRUCT & cs) {
    cs.lpszClass = AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS,
                                       AfxGetApp()->LoadCursor(IDC_MFINGERNW),
                                       (HBRUSH)GetStockObject(NULL_BRUSH)/*(HBRUSH)(COLOR_WINDOW+1)*/, 0);
    return CWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CPrivateCursorWnd message handlers


/***************************************************************************/
// CPrivateCursorWnd::OnPaint Painting
/***************************************************************************/
void CPrivateCursorWnd::OnPaint() {
    CPaintDC dc(this); // device context for painting
    CRect dummyRect(0,0,0,0); // needed for second OnDraw parameter which
    // is only used for printing

    OnDraw(&dc,dummyRect); // shared between printing and display
}


/***************************************************************************/
// CPrivateCursorWnd::OnDraw Painting
// The cursor window saves the position, on which the cursor line has been
// drawn and does not draw it again on the same positon (because this would
// cause a wrong cursor color) unless the clipping rectangle is different
// from the line rectangle inside the cursor window.
/***************************************************************************/
void CPrivateCursorWnd::OnDraw(CDC * pDC, const CRect & printRect) {
    CPlotWnd * pWnd = (CPlotWnd *)GetParent();
    CGraphWnd * pGraph = (CGraphWnd *)pWnd->GetParent();
    if (pGraph->IsPlotID(IDD_TWC)) {
        OnDrawHorizontalCursor(pDC,printRect);
        return;
    }

    //TRACE(_T("Drawn\n"))
    bDrawn = TRUE;
    // get window coordinates and invalid region
    CRect rWnd,rClip, rParent, rSect;

    if (pDC->IsPrinting()) {
        rWnd    = printRect;
        rClip = printRect;
    } else {
        GetClientRect(rWnd);
        pDC->GetClipBox(&rClip);
        pDC->LPtoDP(&rClip);
        if (rWnd.Width() == 0) {
            m_rWnd.SetRect(0, 0, 0, 0);
        }
    }
    if (rClip.Height() > 0) {
        if (rWnd.Width() > 1) {
            // cursor window is larger than one pixel
            rWnd.left = CURSOR_WINDOW_HALFWIDTH;
            rWnd.right = rWnd.left + 1;
        }
        if (!pDC->IsPrinting()) {
            // get the coordinates in the parent window
            rParent = rWnd;
            ClientToScreen(rParent);
            GetParent()->ScreenToClient(rParent);
        }
        //SDM 1.06.6
        // Original version disabled painting if this was the last paint location
        // except if painting or the rect was bigger than the rClipWidth
        //
        // This caused the cursor to be failed to be drawn if the cursors were not moved in a
        // limited sequence (not well documented)  this made the code fragile
        //
        // Because the window has the style WS_EX_TRANSPARENT all underlying painting is
        // done before painting this window.  Therefore if we only invalidate this window
        // when a move actually occurs the painting will always work correctly.  All moves of
        // the cursor windows should happen through the CPlotWnd::ChangePosition function which will
        // guarantee this behavior.
        //
        // There is one remaining additional restriction.  The cursor window must not be drawn on top of
        // each other.  This will erase them.  (They can be temporarily on top of each other, as long as
        // painting does not occur. This alows setting the cursors in any order within a procedure.)
        //
        {
            // last time we did not paint here - store new locatation and draw cursor
            if (!pDC->IsPrinting()) {
                m_rWnd = rParent;
            }
            // get color from main frame
            CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
            // check if plot background is dark to set appropriate raster operation mode
            COLORREF cColor = pMainWnd->GetColors()->cPlotBkg;
            BOOL bDarkBkg = FALSE;
            if ((GetRValue(cColor) + GetGValue(cColor) + GetBValue(cColor)) < 381) {
                bDarkBkg = TRUE;
            }
            cColor = pMainWnd->GetColors()->cPlotPrivateCursor;
            CPen pen(PS_SOLID, 1, cColor);
            CPen * pOldPen = pDC->SelectObject(&pen);
            int oldRop = 0;
            if (FALSE && !pDC->IsPrinting()) {
                if (bDarkBkg) {
                    oldRop = pDC->SetROP2(R2_MASKPENNOT);    // set drawing mode for dark bkg
                } else {
                    oldRop = pDC->SetROP2(R2_MERGEPENNOT);    // set drawing mode for light bkg
                }
            }
            pDC->MoveTo(rWnd.left, rClip.top);
            pDC->LineTo(rWnd.left, rClip.bottom);
            if (FALSE && !pDC->IsPrinting()) {
                pDC->SetROP2(oldRop); // set back old drawing mode
            }
            pDC->SelectObject(pOldPen);
        }
    }
}

/***************************************************************************/
// CPrivateCursorWnd::OnDrawHorizontalCursor - TCJ 5/9/00
//
// *** Modify this function plus ::OnDrawHorizontalCursor(), ::OnLButtonDown() & code
//      in CPlotWnd::OnLButtonDown() to add HORIZONTAL cursor support for new windows.
/***************************************************************************/
void CPrivateCursorWnd::OnDrawHorizontalCursor(CDC * pDC, const CRect & printRect) {

    bDrawn = TRUE;
    // get window coordinates and invalid region
    CRect rWnd,rClip, rParent, rSect;

    if (pDC->IsPrinting()) {
        rWnd    = printRect;
        rClip = printRect;
    } else {
        GetClientRect(rWnd);
        pDC->GetClipBox(&rClip);
        pDC->LPtoDP(&rClip);

        if (rWnd.Height() == 0) {
            m_rWnd.SetRect(0, 0, 0, 0);
        }
    }

    if (rClip.Width() > 0) {
        if (rWnd.Height() > 1) {
            // cursor window is larger than one pixel
            rWnd.top = CURSOR_WINDOW_HALFWIDTH;
            rWnd.bottom = rWnd.top + 1;
        }

        if (!pDC->IsPrinting()) {
            // get the coordinates in the parent window
            rParent = rWnd;
            ClientToScreen(rParent);
            GetParent()->ScreenToClient(rParent);
        }

        //SDM 1.06.6

        {
            // last time we did not paint here - store new locatation and draw cursor
            if (!pDC->IsPrinting()) {
                m_rWnd = rParent;
            }
            // get color from main frame
            CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
            // check if plot background is dark to set appropriate raster operation mode
            COLORREF cColor = pMainWnd->GetColors()->cPlotBkg;
            BOOL bDarkBkg = FALSE;
            if ((GetRValue(cColor) + GetGValue(cColor) + GetBValue(cColor)) < 381) {
                bDarkBkg = TRUE;
            }
            cColor = pMainWnd->GetColors()->cPlotPrivateCursor;
            CPen pen(PS_SOLID, 1, cColor);
            CPen * pOldPen = pDC->SelectObject(&pen);
            int oldRop = 0;
            if (FALSE && !pDC->IsPrinting()) {
                if (bDarkBkg) {
                    oldRop = pDC->SetROP2(R2_MASKPENNOT);    // set drawing mode for dark bkg
                } else {
                    oldRop = pDC->SetROP2(R2_MERGEPENNOT);    // set drawing mode for light bkg
                }
            }

            pDC->MoveTo(rClip.left, rWnd.top);
            pDC->LineTo(rClip.right, rWnd.top);

            if (FALSE && !pDC->IsPrinting()) {
                pDC->SetROP2(oldRop); // set back old drawing mode
            }
            pDC->SelectObject(pOldPen);
        }
    }
}

/***************************************************************************/
// CPrivateCursorWnd::OnMouseMove Mouse moved
// If the cursor is beeing dragged, the window has to be moved. If it is not
// dragged, and the cursor moves exactly over the center of the cursor window
// the cursor changes to a size symbol.
/***************************************************************************/
void CPrivateCursorWnd::OnMouseMove(UINT nFlags, CPoint mousePoint) {
    if (m_bCursorDrag) {
        // get pointer to parent plot, parent graph and to view
        CPlotWnd * pWnd = (CPlotWnd *)GetParent();
        // move cursor
        CPoint point = mousePoint;
        ClientToScreen(&point);
        pWnd->ScreenToClient(&point);
        // SDM 1.06.6U4
        ChangeCursorPosition(point);
    } else {
        // Added support for horizontal cursor
        // get pointer to parent plot
        CPlotWnd * pWnd = (CPlotWnd *)GetParent();
        CGraphWnd * pGraph = (CGraphWnd *)pWnd->GetParent();
        if (pGraph->IsPlotID(IDD_TWC)) {
            if (mousePoint.y == CURSOR_WINDOW_HALFWIDTH) { // cursor is over the line
                // create Vertical two-headed arrow pointer, for horizontal cursor bar
                SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENS));
            }
        } else {
            if (mousePoint.x == CURSOR_WINDOW_HALFWIDTH) { // cursor is over the line
                // create Horizontal two-headed arrow pointer, for vertical cursor bar
                SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
            }
        }
    }

    CWnd::OnMouseMove(nFlags, mousePoint);
}

/***************************************************************************/
// CPrivateCursorWnd::OnLButtonDown Mouse left button down
// This event should set the focus to the parent, so the window calls the
// view to do this. And then the mouse input is captured, the cursor changed
// to a size symbol and mouse movement limited to drag the cursor window with
// the mouse pointer.
//
// *** Modify this function plus ::OnDrawHorizontalCursor(), ::OnLButtonDown() & code
//      in CPlotWnd::OnLButtonDown() to add HORIZONTAL cursor support for new windows.
/***************************************************************************/
void CPrivateCursorWnd::OnLButtonDown(UINT nFlags, CPoint mousePoint) {
    // get pointer to parent plot, parent graph and to view
    CPlotWnd * pWnd = (CPlotWnd *)GetParent();
    CGraphWnd * pGraph = (CGraphWnd *)pWnd->GetParent();
    // inform graph
    pGraph->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(mousePoint.x, mousePoint.y));
    // set drag mode
    m_bCursorDrag = true;
    SetCapture(); // receive all mouse input

    if (pGraph->IsPlotID(IDD_TWC)) {
        // create Vertical two-headed arrow pointer, for horizontal cursor bar - TCJ 5/2000
        SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENS));
    } else {
        // create Horizontal two-headed arrow pointer, for vertical cursor bar
        SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
    }

    // move cursor
    CPoint point = mousePoint;
    ClientToScreen(&point);
    pWnd->ScreenToClient(&point);

    // SDM 1.06.6U4
    ChangeCursorPosition(point);

    // get window coordinates of parent
    CRect rWnd;
    pWnd->GetClientRect(rWnd);

    // limit cursor movement to plot window
    pWnd->ClientToScreen(&rWnd);
    ClipCursor(rWnd);
    CWnd::OnLButtonDown(nFlags, mousePoint);
}

/***************************************************************************/
// CPrivateCursorWnd::OnLButtonUp Mouse left button Up
// The mouse input is freed for other windows again and the mouse move limit
// is cancelled. Then the private cursor window will be set back to the big
// width.
/***************************************************************************/
void CPrivateCursorWnd::OnLButtonUp(UINT nFlags, CPoint mousePoint) {
    m_bCursorDrag = false;
    ReleaseCapture(); // mouse input also to other windows
    ClipCursor(NULL); // free mouse to move everywhere
    // get pointer to parent plot, parent graph and to view
    CPlotWnd * pWnd = (CPlotWnd *)GetParent();
    // move cursor
    CPoint point = mousePoint;
    ClientToScreen(&point);
    pWnd->ScreenToClient(&point);
    // SDM 1.06.6U4
    ChangeCursorPosition(point);
    CWnd::OnLButtonUp(nFlags, mousePoint);
}
// SDM 1.06.6U4
/***************************************************************************/
// CPrivateCursorWnd::ChangeCursorPosition
// common function to move cursor to current point in parent coordinates
/***************************************************************************/
void CPrivateCursorWnd::ChangeCursorPosition(CPoint point) {
    // get pointer to parent plot
    CPlotWnd * pWnd = (CPlotWnd *)GetParent();
    CGraphWnd * pGraph = (CGraphWnd *)pWnd->GetParent();
    if (pGraph->IsPlotID(IDD_TWC)) {
        ChangeHorizontalCursorPosition(point);
        return;
    }

    CRect rOldLine, rNewLine, rNewWnd;
    // get the actual (old) position of cursor window
    GetWindowRect(rOldLine);
    pWnd->ScreenToClient(rOldLine);
    // get the line position in the middle
    if (rOldLine.Width() > 1) { // cursor window has large width
        rOldLine.left += CURSOR_WINDOW_HALFWIDTH;
        rOldLine.right -= (CURSOR_WINDOW_HALFWIDTH - 1);
    }
    rNewLine.SetRect(point.x, rOldLine.top, point.x + 1, rOldLine.bottom);
    rNewWnd.SetRect(point.x - CURSOR_WINDOW_HALFWIDTH, rOldLine.top, point.x + CURSOR_WINDOW_HALFWIDTH, rOldLine.bottom);
    // check if new cursor line position
    if (rOldLine != rNewLine) {
        // invalidate and update old position
		// redraw old cursor position
        pWnd->InvalidateRect(rOldLine, TRUE);
		// move the cursor window to the new position
        MoveWindow(rNewWnd, FALSE);
        rNewLine.SetRect(rNewLine.left - rNewWnd.left, rNewLine.top, rNewLine.left - rNewWnd.left + 1, rNewLine.bottom);
		// redraw new cursor line
        InvalidateRect(rNewLine, TRUE);
    }
    // update the status bar
    pGraph->UpdateStatusBar(0, 0, TRUE);
}

/***************************************************************************/
// CPrivateCursorWnd::ChangeHorizontalCursorPosition - TCJ 5/9/00
//
// *** Modify this function plus ::OnDrawHorizontalCursor(), ::OnLButtonDown() & code
//      in CPlotWnd::OnLButtonDown() to add HORIZONTAL cursor support for new windows.
/***************************************************************************/
void CPrivateCursorWnd::ChangeHorizontalCursorPosition(CPoint point) {
    // get pointer to parent plot, parent graph and to view
    CPlotWnd * pWnd = (CPlotWnd *)GetParent();
    CGraphWnd * pGraph = (CGraphWnd *)pWnd->GetParent();
    CRect rOldLine, rNewLine, rNewWnd;

    // get the actual (old) position of cursor window
    GetWindowRect(rOldLine);
    pWnd->ScreenToClient(rOldLine);

    // get the line position in the middle
    if (rOldLine.Height() > 1) { // cursor window has large width
        rOldLine.top += CURSOR_WINDOW_HALFWIDTH;
        rOldLine.bottom -= (CURSOR_WINDOW_HALFWIDTH - 1);
    }

    rNewLine.SetRect(rOldLine.left, point.y, rOldLine.right, point.y + 1);
    rNewWnd.SetRect(rOldLine.left, point.y - CURSOR_WINDOW_HALFWIDTH, rOldLine.right, point.y + CURSOR_WINDOW_HALFWIDTH);

    // check if new cursor line position
    if (rOldLine != rNewLine) {
        // invalidate and update old position
		// redraw old cursor position
        pWnd->InvalidateRect(rOldLine, TRUE);
		// move the cursor window to the new position
        MoveWindow(rNewWnd, FALSE);

        // rNewLine.SetRect(rNewLine.left, rNewLine.top - rNewWnd.top, rNewLine.right, rNewLine.top - rNewWnd.top + 1);
        rNewLine.SetRect(rNewLine.left, rNewLine.top - rNewWnd.top + 8, rNewLine.right, rNewLine.top - rNewWnd.top + 1 + 8);

		// redraw new cursor line
        InvalidateRect(rNewLine, TRUE);
    }
    // update the status bar
    pGraph->UpdateStatusBar(0, 0, TRUE);
    // make the plot window repaint itself
    pWnd->Invalidate(TRUE);
}

/***************************************************************************/
// CPrivateCursorWnd::OnRButtonDown Mouse right button down
// This event should initiate a popup menu, so the window sends the message
// further to the parent.
/***************************************************************************/
void CPrivateCursorWnd::OnRButtonDown(UINT nFlags, CPoint point) {
    CPlotWnd * pWnd = (CPlotWnd *)GetParent();
    ClientToScreen(&point);
    pWnd->ScreenToClient(&point);
    pWnd->SendMessage(WM_RBUTTONDOWN, nFlags, MAKELONG(point.x, point.y)); // send message to parent
    CWnd::OnRButtonDown(nFlags, point);
}

void CPrivateCursorWnd::ResetPosition() {
    // reset old cursor position
    m_rWnd.SetRect(0, 0, 0, 0);
}

bool CPrivateCursorWnd::IsDragging() {
    return m_bCursorDrag;
}

bool CPrivateCursorWnd::IsCreated() {
    return created;
}

void CPrivateCursorWnd::Flash(bool on) {
    if (!created) {
        return;
    }
    if (on) {
        SetTimer(ID_TIMER_FLASH, 500, NULL);
        ShowWindow(SW_SHOW);
        hidden = false;
    } else {
        KillTimer(ID_TIMER_FLASH);
        ShowWindow(SW_SHOW);
        hidden = false;
    }
}

void CPrivateCursorWnd::OnTimer(UINT /*nIDEvent*/) {
    if (!created) {
        return;
    }
    if (hidden) {
        ShowWindow(SW_SHOW);
        hidden = false;
    } else {
        ShowWindow(SW_HIDE);
        hidden = true;
    }
}


