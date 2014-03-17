#include "stdafx.h"
#include "StopCursorWnd.h"
#include "SA_Cursr.h"
#include "Sa_Doc.h"
#include "SA_View.h"
#include "Segment.h"
#include "Sa_plot.h"
#include "Sa_graph.h"
#include "MainFrm.h"
#include "Process\sa_p_fra.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CStopCursorWnd
// Stop cursor window, contains a cursor line and is normally drawn over the
// graph. The windows width is normally more than one pixel and if the user
// moves the mouse pointer over it, the pointer changes his look. But if he
// drags the stop cursor, the windows size is reduced to one pixel to enable
// quick and easy drag.

/////////////////////////////////////////////////////////////////////////////
// CStopCursorWnd message map

BEGIN_MESSAGE_MAP(CStopCursorWnd, CCursorWnd)
    ON_WM_PAINT()
    ON_WM_MOUSEMOVE()
    ON_WM_RBUTTONDOWN()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_KEYDOWN()
    ON_WM_KEYUP()
END_MESSAGE_MAP()

/***************************************************************************/
// CStopCursorWnd::CStopCursorWnd Constructor
/***************************************************************************/
CStopCursorWnd::CStopCursorWnd()
{
    m_bCursorDrag = FALSE;
    m_rWnd.SetRect(0, 0, 0, 0);
}

/***************************************************************************/
// CStopCursorWnd::~CStopCursorWnd Destructor
/***************************************************************************/
CStopCursorWnd::~CStopCursorWnd()
{
}

/***************************************************************************/
// CStopCursorWnd::Create Creation
// Creates a child window with the given parameters.
/***************************************************************************/
BOOL CStopCursorWnd::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
                            const RECT & rect, CWnd * pParentWnd, UINT /*nID*/, CCreateContext * /*pContext*/)
{
    return CWnd::CreateEx(WS_EX_TRANSPARENT, lpszClassName, lpszWindowName, dwStyle, rect.left,
                          rect.top, rect.right - rect.left, rect.bottom - rect.top,
                          pParentWnd->GetSafeHwnd(), 0);
}

/***************************************************************************/
// CStopCursorWnd::PreCreateWindow Creation
// Called from the framework before the creation of the window. Registers
// the new window class.
/***************************************************************************/
BOOL CStopCursorWnd::PreCreateWindow(CREATESTRUCT & cs)
{
    cs.lpszClass = AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS,
                                       AfxGetApp()->LoadCursor(IDC_MFINGERNW),
                                       (HBRUSH)GetStockObject(NULL_BRUSH)/*(HBRUSH)(COLOR_WINDOW+1)*/, 0);
    return CWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CStopCursorWnd helper functions

/***************************************************************************/
// CStopCursorWnd::CalculateCursorPosition
// Calculates the stop cursor position in data samples for the given posit-
// ion in client coordinates and returns it as parameter. The caller has to
// deliver a pointer to the view, the actual position in client coordinates
// he wants to calculate and the width of the parent graph window. The
// calculates besides the new stop cursor position also the maximum start
// cursor position. This result will be put in a DWORD, to which the pointer
// given as 4th parameter points to. The results has been range checked.
/***************************************************************************/
DWORD CStopCursorWnd::CalculateCursorPosition(  CView * pSaView, int nPosition, int nWidth, DWORD * pStartCursor)
{
    CSaView * pView = (CSaView *)pSaView; // cast pointer
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument(); // get pointer to document
    CPlotWnd * pPlot = (CPlotWnd *)GetParent(); // get pointer to parent plot
    CGraphWnd * pGraph = (CGraphWnd *)pPlot->GetParent(); // get pointer to graph
    // get actual data position, frame and data size and alignment
    double fDataPos;
    DWORD dwDataFrame;
    // check if area graph type
    if (pGraph->IsAreaGraph())
    {
        // get necessary data from area plot
        fDataPos = pPlot->GetAreaPosition();
        dwDataFrame = pPlot->GetAreaLength();
    }
    else
    {
        // get necessary data from document and from view
        fDataPos = pView->GetDataPosition(nWidth); // data index of first sample to display
        dwDataFrame = pView->AdjustDataFrame(nWidth); // number of data points to display
    }
    DWORD nSmpSize = pDoc->GetSampleSize();
    // calculate data samples per pixel
    ASSERT(nWidth);
    double fSamplesPerPix = (double)dwDataFrame / (double)(nWidth*nSmpSize);

    // calculate the start cursor position
    DWORD dwCursor = (DWORD) round(fDataPos/nSmpSize + ((double)nPosition) * fSamplesPerPix);
    dwCursor = dwCursor*nSmpSize;
    // check for minimum position
    if (dwCursor < (DWORD)nSmpSize)
    {
        dwCursor = (DWORD)nSmpSize;
    }
    // check for maximum position
    DWORD dwDataSize = pDoc->GetDataSize();
    if (dwCursor >= dwDataSize)
    {
        dwCursor = dwDataSize - (DWORD)nSmpSize;
    }
    // calculate maximum position for start cursor
    DWORD dwDifference = (DWORD)(CURSOR_MIN_DISTANCE * fSamplesPerPix * nSmpSize);
    *pStartCursor = 0;
    if (dwCursor > dwDifference)
    {
        *pStartCursor = dwCursor - dwDifference;
    }
    return dwCursor;
}

/***************************************************************************/
// CStopCursorWnd::OnPaint Painting
/***************************************************************************/
void CStopCursorWnd::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    CRect dummyRect(0,0,0,0); // needed for second OnDraw parameter which
    // is only used for printing

    OnDraw(&dc,dummyRect); // shared between printing and display
}

/***************************************************************************/
// CStopCursorWnd::OnDraw Painting
// The cursor window saves the position, on which the cursor line has been
// drawn and does not draw it again on the same positon (because this would
// cause a wrong cursor color) unless the clipping rectangle is different
// from the line rectangle inside the cursor window.
/***************************************************************************/
void CStopCursorWnd::OnDraw(CDC * pDC, const CRect & printRect)
{
    bDrawn = TRUE;
    // get window coordinates and invalid region
    CRect rWnd,rClip, rParent, rSect;

    if (pDC->IsPrinting())
    {
        rWnd    = printRect;
        rClip = printRect;
    }
    else
    {
        GetClientRect(rWnd);
        pDC->GetClipBox(&rClip);
        pDC->LPtoDP(&rClip);
        if (rWnd.Width() == 0)
        {
            m_rWnd.SetRect(0, 0, 0, 0);
        }
    }
    if (rClip.Height() > 0)
    {
        if (rWnd.Width() > 1)
        {
            // cursor window is larger than one pixel
            rWnd.left = CURSOR_WINDOW_HALFWIDTH;
            rWnd.right = rWnd.left + 1;
        }
        if (!pDC->IsPrinting())
        {
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
            if (!pDC->IsPrinting())
            {
                m_rWnd = rParent;
            }
            // get color from main frame
            CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
            // check if plot background is dark to set appropriate raster operation mode
            COLORREF cColor = pMainWnd->GetColors()->cPlotBkg;
            BOOL bDarkBkg = FALSE;
            if ((GetRValue(cColor) + GetGValue(cColor) + GetBValue(cColor)) < 381)
            {
                bDarkBkg = TRUE;
            }
            cColor = pMainWnd->GetColors()->cPlotStopCursor;
            CPen pen(PS_SOLID, 1, cColor);
            CPen * pOldPen = pDC->SelectObject(&pen);
            int oldRop = 0;
            if (FALSE && !pDC->IsPrinting())
            {
                if (bDarkBkg)
                {
                    oldRop = pDC->SetROP2(R2_MASKPENNOT);    // set drawing mode for dark bkg
                }
                else
                {
                    oldRop = pDC->SetROP2(R2_MERGEPENNOT);    // set drawing mode for light bkg
                }
            }
            pDC->MoveTo(rWnd.left, rClip.top);
            pDC->LineTo(rWnd.left, rClip.bottom);
            if (FALSE && !pDC->IsPrinting())
            {
                pDC->SetROP2(oldRop); // set back old drawing mode
            }
            pDC->SelectObject(pOldPen);
        }
    }
}

/***************************************************************************/
// CStopCursorWnd::OnMouseMove Mouse moved
// If the cursor is beeing dragged, the window has to be moved. If it is not
// dragged, and the cursor moves exactly over the center of the cursor window
// the cursor changes to a size symbol.
/***************************************************************************/
void CStopCursorWnd::OnMouseMove( UINT nFlags, CPoint point)
{
    // get pointer to parent plot, parent graph and to view
    CPlotWnd * pWnd = (CPlotWnd *)GetParent();
    CGraphWnd * pGraph = (CGraphWnd *)pWnd->GetParent();
    CSaView * pView = (CSaView *)pGraph->GetParent();
    // calculate parent client coordinates
    ClientToScreen(&point);
    pWnd->ScreenToClient(&point);
    // track mouse in plot
    pWnd->SetMousePointerPosition(point);
    pWnd->SetMouseButtonState(nFlags);
    // get cursor positions
    DWORD dwCursor = pView->GetStopCursorPosition();
    DWORD dwStartCursor = pView->GetStartCursorPosition();
    if (m_bCursorDrag)
    {
        // get window coordinates of parent
        CRect rWnd;
        pWnd->GetClientRect(rWnd);
        // calculate new stop cursor position
        dwCursor = CalculateCursorPosition(pView, point.x, rWnd.Width(), &dwStartCursor);
        if (pView->GetStartCursorPosition() <= dwStartCursor)
        {
            dwStartCursor = pView->GetStartCursorPosition();
        }
        int nLoop = pView->FindSelectedAnnotationIndex();
        if (nLoop !=-1)
        {
            if (m_nEditBoundaries  == BOUNDARIES_EDIT_BOUNDARIES)
            {
                pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), dwStartCursor, dwCursor,CSegment::LIMIT_MOVING_STOP_NO_OVERLAP); // Limit positions of cursors
            }
            else if (m_nEditBoundaries  == BOUNDARIES_EDIT_SEGMENT_SIZE)
            {
                pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), dwStartCursor, dwCursor,CSegment::LIMIT_MOVING_STOP); // Limit positions of cursors
            }
            else
            {
                // Added and modified from CStopCursorWnd::OnLButtonUp and modified by AKE 7/22/01 to deselect segment
                // while cursor is moving
                CSegment * pSegment = pView->FindSelectedAnnotation();
                int nIndex = pSegment->GetSelection();
                if ((dwCursor > pSegment->GetStop(nIndex)) ||
                    (dwStartCursor < pSegment->GetOffset(nIndex)))
                {
                    // Deselect segment
                    pView->DeselectAnnotations();
                }
            }
            // detect update request and update annotationWnd to hint
            if (pGraph->HaveAnnotation(nLoop))
            {
                // Selected annotation is visible
                CAnnotationWnd * pWnd = pGraph->GetAnnotationWnd(nLoop);
                pWnd->SetHintUpdateBoundaries( m_nEditBoundaries!=0, dwStartCursor, dwCursor,m_nEditBoundaries  == BOUNDARIES_EDIT_SEGMENT_SIZE);//SDM 1.5Test8.1
            }
        }

        // scroll left if the stop cursor bumps up against the left edge of the plot window
        if (point.x <= CURSOR_WINDOW_HALFWIDTH)
        {
            pView->Scroll((UINT)SB_LINELEFT);
            pGraph->UpdateWindow();
        }

        // scroll right if the stop cursor bumps up against the right edge of the plot window
        if (point.x >= rWnd.Width() - CURSOR_WINDOW_HALFWIDTH)
        {
            pView->Scroll((UINT)SB_LINERIGHT);
            pGraph->UpdateWindow();
        }

        // move start cursor
        pView->MoveStartCursor(dwStartCursor);
        // stop cursor is to move also
        pView->MoveStopCursor(dwCursor);

        // update the status bar
        pGraph->UpdateStatusBar(dwStartCursor, dwCursor);

        // AKE 07/15/00, for tracking movement of stop cursor as it is dragged
        CSaDoc * pDoc = pView->GetDocument();
        CProcessFragments * pFragments = pDoc->GetFragments();
        if (pFragments && pFragments->IsDataReady())
        {
            DWORD wSmpSize = pDoc->GetSampleSize();
            DWORD OldFragmentIndex = pFragments->GetFragmentIndex(m_dwDragPos/wSmpSize);
            DWORD dwFragmentIndex = pFragments->GetFragmentIndex(dwCursor/wSmpSize);
            if (dwFragmentIndex != OldFragmentIndex)
            {
                // cursor has crossed fragment boundary
                m_dwDragPos = dwCursor;
                pView->SendMessage(WM_USER_CURSOR_IN_FRAGMENT, STOP_CURSOR, dwFragmentIndex);
            }
        }
    }
    else
    {
        if (point.x == CURSOR_WINDOW_HALFWIDTH)
        {
            // cursor is over the line
            SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE)); // set size cursor
        }
    }
    // update the status bar
    const BOOL bForceUpdate = TRUE;
    pGraph->UpdateStatusBar(dwStartCursor, dwCursor, bForceUpdate);

    CWnd::OnMouseMove(nFlags, point);
}

/***************************************************************************/
// CStopCursorWnd::OnLButtonDown Mouse left button down
// This event should set the focus to the parent, so the window calls the
// view to do this. And then the mouse input is captured, the cursor changed
// to a size symbol and mouse movement limited to drag the cursor window with
// the mouse pointer.
/***************************************************************************/
void CStopCursorWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
    // get pointer to parent plot, parent graph and to view
    CPlotWnd * pWnd = (CPlotWnd *)GetParent();
    CGraphWnd * pGraph = (CGraphWnd *)pWnd->GetParent();
    CSaView * pView = (CSaView *)pGraph->GetParent();
    // inform graph
    pGraph->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
    // set boundaries mode
    CRect rCursorRect;
    GetClientRect(rCursorRect);
    if (rCursorRect.PtInRect(point) || (pView->GetEditBoundaries()!=BOUNDARIES_EDIT_NULL))
    {
        m_nEditBoundaries = pView->GetEditBoundaries();
    }
    else
    {
        m_nEditBoundaries = BOUNDARIES_EDIT_NULL;
    }
    // set drag mode
    m_bCursorDrag = TRUE;
    SetCapture(); // receive all mouse input
    SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE)); // set size cursor
    // calculate parent client coordinates
    ClientToScreen(&point);
    pWnd->ScreenToClient(&point);
    // get window coordinates of parent
    CRect rWnd;
    pWnd->GetClientRect(rWnd);
    // calculate new stop cursor position
    DWORD dwStartCursor;
    DWORD dwCursor = CalculateCursorPosition(pView, point.x, rWnd.Width(), &dwStartCursor);
    if (pView->GetStartCursorPosition() <= dwStartCursor)
    {
        dwStartCursor = pView->GetStartCursorPosition();
    }
    int nLoop = pView->FindSelectedAnnotationIndex();
    if (nLoop!=-1)
    {
        if (m_nEditBoundaries  == BOUNDARIES_EDIT_BOUNDARIES)
        {
            pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), dwStartCursor, dwCursor,CSegment::LIMIT_MOVING_STOP_NO_OVERLAP); // Limit positions of cursors
        }
        else if (m_nEditBoundaries  == BOUNDARIES_EDIT_SEGMENT_SIZE)
        {
            pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), dwStartCursor, dwCursor,CSegment::LIMIT_MOVING_STOP); // Limit positions of cursors
        }
        else
        {
            // Moved from CStopCursorWnd::OnLButtonUp and modified by AKE 7/22/01 to deselect segment,
            // providing consistent operation with start cursor
            CSegment * pSegment = pView->FindSelectedAnnotation();
            int nIndex = pSegment->GetSelection();
            if ((dwCursor > pSegment->GetStop(nIndex)) ||
                    (dwStartCursor < pSegment->GetOffset(nIndex)))
            {
                // Deselect segment
                pView->DeselectAnnotations();
            }
        }
        // detect update request and update annotationWnd to hint
        if (pGraph->HaveAnnotation(nLoop))   // Selected annotation is visible
        {
            CAnnotationWnd * pWnd = pGraph->GetAnnotationWnd(nLoop);
            pWnd->SetHintUpdateBoundaries( m_nEditBoundaries!=0, dwStartCursor, dwCursor,m_nEditBoundaries  == BOUNDARIES_EDIT_SEGMENT_SIZE);//SDM 1.5Test8.1
        }
    }

    //TRACE("start=%lu stop=%lu\n",dwStartCursor,dwCursor);

    // move start cursor
    pView->MoveStartCursor(dwStartCursor);
    // stop cursor is to move also
    pView->MoveStopCursor(dwCursor);
    // clear the highlight area on mouse down
    CGraphWnd * pRaw = pView->GraphIDtoPtr(IDD_RAWDATA);
    if (pRaw)
    {
        pRaw->GetPlot()->SetHighLightArea(0, 0);
    }
    // update the status bar
    pGraph->UpdateStatusBar(dwStartCursor, dwCursor);
    // AKE 07/15/00, for tracking movement of start cursor as it is dragged
    CSaDoc * pDoc = pView->GetDocument();
    CProcessFragments * pFragments = pDoc->GetFragments();
    if (pFragments && pFragments->IsDataReady())
    {
        DWORD wSmpSize = pDoc->GetSampleSize();
        DWORD dwFragmentIndex = pFragments->GetFragmentIndex(dwCursor/wSmpSize);
        m_dwDragPos = dwCursor;
        pView->SendMessage(WM_USER_CURSOR_IN_FRAGMENT, STOP_CURSOR, dwFragmentIndex);
    }
    // Set Focus to receive key events so we can track CTRL-SHIFT changes during drag
    SetFocus();
    pWnd->ClientToScreen(&rWnd);
    ClipCursor(rWnd);
    CWnd::OnLButtonDown(nFlags, point);
}

/***************************************************************************/
// CStopCursorWnd::OnLButtonUp Mouse left button Up
// The mouse input is freed for other windows again and the mouse move limit
// is cancelled. Then the new cursor position in the wave data has to be
// calculated and set.
/***************************************************************************/
void CStopCursorWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
    m_bCursorDrag = FALSE;
    ReleaseCapture(); // mouse input also to other windows
    ClipCursor(NULL); // free mouse to move everywhere
    // calculate parent client coordinates
    CWnd * pWnd = GetParent();
    ClientToScreen(&point);
    pWnd->ScreenToClient(&point);
    // get window coordinates of parent
    CRect rWnd;
    pWnd->GetClientRect(rWnd);
    // get pointer to parent view
    CGraphWnd * pGraph = (CGraphWnd *)pWnd->GetParent();
    CSaView * pView = (CSaView *)pGraph->GetParent();
    pGraph->SetGraphFocus(TRUE); // Reset Focus

    // set the new positions
    DWORD dwStartCursor;
    DWORD dwCursor = CalculateCursorPosition(pView, point.x, rWnd.Width(), &dwStartCursor);
    if (pView->GetStartCursorPosition() <= dwStartCursor)
    {
        dwStartCursor = pView->GetStartCursorPosition();
    }
    int nLoop = pView->FindSelectedAnnotationIndex();
    if (nLoop!=-1)
    {
        if (m_nEditBoundaries == BOUNDARIES_EDIT_BOUNDARIES)
        {
            pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), dwStartCursor, dwCursor,CSegment::LIMIT_MOVING_STOP_NO_OVERLAP); // Limit positions of cursors
        }
        else if (m_nEditBoundaries == BOUNDARIES_EDIT_SEGMENT_SIZE)
        {
            pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), dwStartCursor, dwCursor,CSegment::LIMIT_MOVING_STOP); // Limit positions of cursors
        }
        // detect update request and update annotationWnd to hint
        if (pGraph->HaveAnnotation(nLoop))   // Selected annotation is visible
        {
            CAnnotationWnd * pWnd = pGraph->GetAnnotationWnd(nLoop);
            pWnd->SetHintUpdateBoundaries( false, m_nEditBoundaries  == BOUNDARIES_EDIT_SEGMENT_SIZE);//SDM 1.5Test8.1
        }
    }

    // move start cursor
    pView->SetStartCursorPosition(dwStartCursor, SNAP_LEFT);

    // move stop cursor also
    pView->SetCursorPosition(STOP_CURSOR, dwCursor, SNAP_BOTH);

    // detect update request
    // If CTRL+SHIFT update segment boundaries
    if ((m_nEditBoundaries&&(nLoop!=-1) &&
            (pView->GetAnnotation(nLoop)->CheckCursors(pView->GetDocument(),m_nEditBoundaries  == BOUNDARIES_EDIT_SEGMENT_SIZE) != -1)))
    {
        // Only allow update of PHONETIC and GLOSS
        CSaDoc * pDoc = pView->GetDocument();
        // save state for undo ability
        int nIndex = pDoc->GetSegment(nLoop)->GetSelection();

        // first adjust cursors to old segment boundaries (undo to here)
        pView->SetStartCursorPosition(pDoc->GetSegment(nLoop)->GetOffset(nIndex), SNAP_LEFT);
        pView->SetStopCursorPosition(pDoc->GetSegment(nLoop)->GetStop(nIndex), SNAP_RIGHT);

        pDoc->CheckPoint(); // Save state

        // Reload cursor locations to new segment boundaries
        pView->SetStartCursorPosition(dwStartCursor, SNAP_LEFT);
        pView->SetStopCursorPosition(dwCursor, SNAP_RIGHT);

        // Do update
        pDoc->UpdateSegmentBoundaries(m_nEditBoundaries  == BOUNDARIES_EDIT_SEGMENT_SIZE);//SDM 1.5Test8.1
    }
    CWnd::OnLButtonUp(nFlags, point);
}

/***************************************************************************/
// CStopCursorWnd::OnRButtonDown Mouse right button down
// This event should initiate a popup menu, so the window sends the message
// further to the parent.
/***************************************************************************/
void CStopCursorWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
    CPlotWnd * pWnd = (CPlotWnd *)GetParent();
    ClientToScreen(&point);
    pWnd->ScreenToClient(&point);
    pWnd->SendMessage(WM_RBUTTONDOWN, nFlags, MAKELONG(point.x, point.y)); // send message to parent
    CWnd::OnRButtonDown(nFlags, point);
}

/***************************************************************************/
// CStopCursorWnd::OnKeyDown Track key changes during drag
/***************************************************************************/
void CStopCursorWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    CWnd * pWnd = GetParent();
    CGraphWnd * pGraph = (CGraphWnd *)pWnd->GetParent();

    if (m_bCursorDrag)
    {
        CSaView * pView = (CSaView *)pGraph->GetParent();
        int nLoop = pView->FindSelectedAnnotationIndex();
        if (nLoop != -1)
        {
            if (pGraph->HaveAnnotation(nLoop))   // Selected annotation is visible
            {
                CAnnotationWnd * pWnd = pGraph->GetAnnotationWnd(nLoop);
                pWnd->SetHintUpdateBoundaries( m_nEditBoundaries!=0, m_nEditBoundaries == BOUNDARIES_EDIT_SEGMENT_SIZE);//SDM 1.5Test8.1
            }
        }
    }

    pGraph->SendMessage(WM_KEYDOWN, nChar, MAKELONG(nRepCnt, nFlags));
    CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

/***************************************************************************/
// CStopCursorWnd::OnKeyUp Track key changes during drag
/***************************************************************************/
void CStopCursorWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    CWnd * pWnd = GetParent();
    CGraphWnd * pGraph = (CGraphWnd *)pWnd->GetParent();

    if (m_bCursorDrag)
    {
        CSaView * pView = (CSaView *)pGraph->GetParent();
        int nLoop = pView->FindSelectedAnnotationIndex();
        if (nLoop != -1)
        {
            if (pGraph->HaveAnnotation(nLoop))   // Selected annotation is visible
            {
                CAnnotationWnd * pWnd = pGraph->GetAnnotationWnd(nLoop);
                pWnd->SetHintUpdateBoundaries( m_nEditBoundaries!=0, m_nEditBoundaries == BOUNDARIES_EDIT_SEGMENT_SIZE);//SDM 1.5Test8.1
            }
        }
    }

    pGraph->SendMessage(WM_KEYUP, nChar, MAKELONG(nRepCnt, nFlags));
    CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CStopCursorWnd::ResetPosition()
{
    m_rWnd.SetRect(0, 0, 0, 0);
}

BOOL CStopCursorWnd::IsDragging()
{
    return m_bCursorDrag;
}
