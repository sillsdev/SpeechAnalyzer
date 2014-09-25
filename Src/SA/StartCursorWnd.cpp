#include "stdafx.h"
#include "StartCursorWnd.h"
#include "sa_plot.h"
#include "sa_graph.h"
#include "Process\Process.h"
#include "Segment.h"
#include "Process\sa_p_fra.h"

#include "math.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CStartCursorWnd
// Start cursor window, contains a cursor line and is normally drawn over the
// graph. The windows width is normally more than one pixel and if the user
// moves the mouse pointer over it, the pointer changes his look. But if he
// drags the start cursor, the windows size is reduced to one pixel to enable
// quick and easy drag.

BEGIN_MESSAGE_MAP(CStartCursorWnd, CCursorWnd)
    ON_WM_PAINT()
    ON_WM_MOUSEMOVE()
    ON_WM_RBUTTONDOWN()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_KEYUP()
    ON_WM_KEYDOWN()
END_MESSAGE_MAP()

/***************************************************************************/
// CStartCursorWnd::CStartCursorWnd Constructor
/***************************************************************************/
CStartCursorWnd::CStartCursorWnd()
{
    m_bCursorDrag = FALSE;
    m_dwDragPos = (DWORD)UNDEFINED_OFFSET;
    m_rWnd.SetRect(0, 0, 0, 0);
	m_bSelectDrag = false;
}

/***************************************************************************/
// CStartCursorWnd::~CStartCursorWnd Destructor
/***************************************************************************/
CStartCursorWnd::~CStartCursorWnd()
{
}

/***************************************************************************/
// CStartCursorWnd::Create Creation
// Creates a child window with the given parameters.
/***************************************************************************/
BOOL CStartCursorWnd::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
                             const RECT & rect, CWnd * pParentWnd, UINT /*nID*/, CCreateContext * /*pContext*/)
{
    return CWnd::CreateEx(WS_EX_TRANSPARENT, lpszClassName, lpszWindowName, dwStyle, rect.left,
                          rect.top, rect.right - rect.left, rect.bottom - rect.top,
                          pParentWnd->GetSafeHwnd(), 0);
}

/***************************************************************************/
// CStartCursorWnd::PreCreateWindow Creation
// Called from the framework before the creation of the window. Registers
// the new window class.
/***************************************************************************/
BOOL CStartCursorWnd::PreCreateWindow(CREATESTRUCT & cs)
{
    cs.lpszClass = AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS,
                                       AfxGetApp()->LoadCursor(IDC_MFINGERNW), (HBRUSH)GetStockObject(NULL_BRUSH), 0);
    return CWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CStartCursorWnd helper functions

/***************************************************************************/
// CStartCursorWnd::CalculateCursorPosition
// Calculates the start cursor position in data samples for the given posit-
// ion in client coordinates and returns it as parameter. The caller has to
// deliver a pointer to the view, the actual position in client coordinates
// he wants to calculate and the width of the parent graph window. The
// calculates besides the new start cursor position also the minimum stop
// cursor position. This result will be put in a DWORD, to which the pointer
// given as 4th parameter points to. The results has been range checked.
/***************************************************************************/
DWORD CStartCursorWnd::CalculateCursorPosition( CView * pSaView,
												int nPosition,
												int nWidth,
												DWORD * pStopCursor)
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
    DWORD dwDataSize = pDoc->GetDataSize();
    DWORD nSmpSize = pDoc->GetSampleSize();
    // calculate data samples per pixel
    ASSERT(nWidth);
    double fSamplesPerPix = (double)dwDataFrame / (double)(nWidth*nSmpSize);
    // calculate the start cursor position
    DWORD dwCursor = (DWORD) round(fDataPos/nSmpSize + ((double)nPosition) * fSamplesPerPix);
    dwCursor = dwCursor*nSmpSize;
    // check the range
    if (dwCursor >= (dwDataSize - (DWORD)nSmpSize))
    {
        dwCursor = dwDataSize - (DWORD)(2 * nSmpSize);
    }
    // calculate minimum position for stop cursor
    *pStopCursor = dwCursor + (DWORD)(CURSOR_MIN_DISTANCE*fSamplesPerPix*nSmpSize);
    if (*pStopCursor >= dwDataSize)
    {
        *pStopCursor = dwDataSize - (DWORD)nSmpSize;
    }
    return dwCursor;
}

/***************************************************************************/
// CStartCursorWnd::OnPaint Painting
/***************************************************************************/
void CStartCursorWnd::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    CRect dummyRect(0,0,0,0); // needed for second OnDraw parameter which
    // is only used for printing

    OnDraw(&dc,dummyRect); // shared between printing and display
}


/***************************************************************************/
// CStartCursorWnd::OnDraw Painting
// The cursor window saves the position, on which the cursor line has been
// drawn and does not draw it again on the same positon (because this would
// cause a wrong cursor color) unless the clipping rectangle is different
// from the line rectangle inside the cursor window.
/***************************************************************************/
void CStartCursorWnd::OnDraw(CDC * pDC, const CRect & printRect)
{
    bDrawn = TRUE;
    // get window coordinates and invalid region
    CRect rWnd,rClip, rParent;

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

    CRect rSect;
    if (rClip.Height() > 0)
    {
        if (rWnd.Width() > 1)
        {
            // cursor window is larger than one pixel
            rWnd.left = CURSOR_WINDOW_HALFWIDTH;
            rWnd.right = rWnd.left + 1;
        }
        // get the coordinates in the parent window
        if (!pDC->IsPrinting())
        {
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
            cColor = pMainWnd->GetColors()->cPlotStartCursor;
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
// CStartCursorWnd::OnMouseMove Mouse moved
// If the cursor is being dragged, the window has to be moved. If it is not
// dragged, and the cursor moves exactly over the center of the cursor window
// the cursor changes to a size symbol.
/***************************************************************************/
void CStartCursorWnd::OnMouseMove(UINT nFlags, CPoint point)
{

    // get pointer to parent plot, graph, and view
    CPlotWnd * pWnd = (CPlotWnd *)GetParent(); // get parent plot
    CGraphWnd * pGraph = (CGraphWnd *)pWnd->GetParent();
    CSaView * pView = (CSaView *)pGraph->GetParent();
    // calculate parent client coordinates
    ClientToScreen(&point);
    pWnd->ScreenToClient(&point);
    // track mouse in plot
    pWnd->SetMousePointerPosition(point);
    pWnd->SetMouseButtonState(nFlags);
    // get cursor positions
    DWORD dwStopCursor = pView->GetStopCursorPosition();
    DWORD dwCursor = pView->GetStartCursorPosition();
    if (m_bCursorDrag)
    {
        // get window coordinates of parent
        CRect rWnd;
        pWnd->GetClientRect(rWnd);
        // calculate new start cursor position
        dwCursor = CalculateCursorPosition(pView, point.x, rWnd.Width(), &dwStopCursor);

        if (pView->GetStopCursorPosition() >= dwStopCursor)
        {
            dwStopCursor = pView->GetStopCursorPosition();
        }
        int nLoop = pView->FindSelectedAnnotationIndex();
        if (nLoop!=-1)
        {
            if (m_nEditBoundaries == BOUNDARIES_EDIT_BOUNDARIES)
            {
                pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), dwCursor, dwStopCursor, CSegment::LIMIT_MOVING_START_NO_OVERLAP); // Limit positions of cursors
            }
            else if (m_nEditBoundaries == BOUNDARIES_EDIT_SEGMENT_SIZE)
            {
                pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), dwCursor, dwStopCursor,CSegment::LIMIT_MOVING_START); // Limit positions of cursors
            }
            else
            {
                // Added and modified from CStartCursorWnd::OnLButtonUp and modified by AKE 7/22/01 to deselect segment
                // while cursor is moving
                CSegment * pSegment = pView->FindSelectedAnnotation();
                int nIndex = pSegment->GetSelection();
                if ((dwStopCursor > pSegment->GetStop(nIndex)) ||
                    (dwCursor < pSegment->GetOffset(nIndex)))
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
                pWnd->SetHintUpdateBoundaries( m_nEditBoundaries!=0, dwCursor, dwStopCursor,m_nEditBoundaries == BOUNDARIES_EDIT_SEGMENT_SIZE);
            }
        }

        // scroll left if the start cursor bumps up against the left edge of the plot window
        if (point.x <= CURSOR_WINDOW_HALFWIDTH)
        {
            pView->Scroll((UINT)SB_LINELEFT);
            pGraph->UpdateWindow();
        }

        // scroll right if the start cursor bumps up against the right edge of the plot window
        if (point.x >= rWnd.Width() - CURSOR_WINDOW_HALFWIDTH)
        {
            pView->Scroll((UINT)SB_LINERIGHT);
            pGraph->UpdateWindow();
        }

		// set the highlight area for raw data
		if ((pView->GetFocusedGraphID() == IDD_RAWDATA) &&
			((nFlags&(MK_CONTROL|MK_SHIFT)) == MK_CONTROL) &&
			(pView->GetEditBoundaries()==BOUNDARIES_EDIT_NULL) &&
			((pView->GetGraphUpdateMode() == STATIC_UPDATE) || (!pView->GetDynamicGraphCount())))
		{
			m_bSelectDrag = true;
			pView->HideCursors();
		}

        // stop cursor is to move also
        pView->MoveStopCursor(dwStopCursor);
        // move start cursor
        pView->MoveStartCursor(dwCursor);
        
		// set the highlight area for raw data
        if ((pView->GetFocusedGraphID() == IDD_RAWDATA) &&
            ((nFlags&(MK_CONTROL|MK_SHIFT)) == MK_CONTROL) &&
            (pView->GetEditBoundaries()==BOUNDARIES_EDIT_NULL) &&
            ((pView->GetGraphUpdateMode() == STATIC_UPDATE) || (!pView->GetDynamicGraphCount())))
        {
            if (dwCursor > m_dwStartDragPos)
            {
                pWnd->SetHighLightArea(m_dwStartDragPos, dwCursor);
            }
            else
            {
                pWnd->SetHighLightArea(dwCursor, m_dwStartDragPos);
            }
        }

        // AKE 07/15/00, for tracking movement of start cursor as it is dragged
        CSaDoc * pDoc = pView->GetDocument();
        CProcessFragments * pFragments = pDoc->GetFragments();
        if ((pFragments!=NULL) && (pFragments->IsDataReady()))
        {
            WORD wSmpSize = WORD(pDoc->GetSampleSize());
            DWORD OldFragmentIndex = pFragments->GetFragmentIndex(m_dwDragPos/wSmpSize);
            DWORD dwFragmentIndex = pFragments->GetFragmentIndex(dwCursor/wSmpSize);
            if (dwFragmentIndex != OldFragmentIndex)
            {
                // cursor has crossed fragment boundary
                m_dwDragPos = dwCursor;
                pView->SendMessage(WM_USER_CURSOR_IN_FRAGMENT, START_CURSOR, dwFragmentIndex);
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
    pGraph->UpdateStatusBar(dwCursor, dwStopCursor, bForceUpdate);

    CWnd::OnMouseMove(nFlags, point);
}

/***************************************************************************/
// CStartCursorWnd::OnLButtonDown Mouse left button down
// This event should set the focus to the parent, so the window informs the
// graph to do this. And then the mouse input is captured, the cursor changed
// to a size symbol and mouse movement limited to drag the cursor window with
// the mouse pointer.
/***************************************************************************/
void CStartCursorWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
    // get pointer to parent plot, parent graph and to view
    CPlotWnd * pWnd = (CPlotWnd *)GetParent();
    CGraphWnd * pGraph = (CGraphWnd *)pWnd->GetParent();
    CSaView * pView = (CSaView *)pGraph->GetParent();

    // inform graph
    pGraph->SendMessage( WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));

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
    // track mouse in plot
    pWnd->SetMousePointerPosition(point);
    pWnd->SetMouseButtonState(nFlags);
    // get window coordinates of parent
    CRect rWnd;
    pWnd->GetClientRect(rWnd);
    // calculate new start cursor position
    DWORD dwStopCursor;
    m_dwStartDragPos = CalculateCursorPosition(pView, point.x, rWnd.Width(), &dwStopCursor);
    if (pView->GetStopCursorPosition() >= dwStopCursor)
    {
        dwStopCursor = pView->GetStopCursorPosition();
    }

    int nLoop = pView->FindSelectedAnnotationIndex();
    if (nLoop!=-1)
    {
        if (m_nEditBoundaries == BOUNDARIES_EDIT_BOUNDARIES)
        {
            pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), m_dwStartDragPos, dwStopCursor,CSegment::LIMIT_MOVING_START_NO_OVERLAP); // Limit positions of cursors
        }
        else if (m_nEditBoundaries == BOUNDARIES_EDIT_SEGMENT_SIZE)
        {
            pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), m_dwStartDragPos, dwStopCursor,CSegment::LIMIT_MOVING_START); // Limit positions of cursors
        }
        else
        {
            // Moved from CStartCursorWnd::OnLButtonUp and modified by AKE 7/22/01 to deselect segment if
            // not editing segment boundaries in order to allow highlighting for cut and paste operation
            CSegment * pSegment = pView->FindSelectedAnnotation();
			if (pSegment!=NULL) {
				int nIndex = pSegment->GetSelection();
				if ((dwStopCursor > pSegment->GetStop(nIndex)) ||
					(m_dwStartDragPos < pSegment->GetOffset(nIndex)))
				{
					// Deselect segment
					pView->DeselectAnnotations();
				}
			}
        }
        // detect update request and update annotationWnd to hint
        if (pGraph->HaveAnnotation(nLoop))
        {
            // Selected annotation is visible
            CAnnotationWnd * pWnd = pGraph->GetAnnotationWnd(nLoop);
            pWnd->SetHintUpdateBoundaries( m_nEditBoundaries!=0, m_dwStartDragPos, dwStopCursor,m_nEditBoundaries == BOUNDARIES_EDIT_SEGMENT_SIZE);//SDM 1.5Test8.1
        }
    } 
	else
	{
		if (pView->IsSelectionVirtual()) 
		{
			pView->DeselectAnnotations();
		}
	}

	// also check and deselect the advanced selection in the view

    // stop cursor is to move also
    pView->MoveStopCursor(dwStopCursor);
    // move start cursor
    pView->MoveStartCursor(m_dwStartDragPos);
    // update the status bar
    pGraph->UpdateStatusBar(m_dwStartDragPos, dwStopCursor);
    // clear the highlight area on mouse down
    CGraphWnd * pRaw = pView->GraphIDtoPtr(IDD_RAWDATA);
    if (pRaw)
    {
        pRaw->GetPlot()->SetHighLightArea(0, 0);
    }
    // limit cursor movement to graph window
    pWnd->ClientToScreen(&rWnd);

    // AKE 07/15/00, for tracking movement of start cursor as it is dragged
    CSaDoc * pDoc = pView->GetDocument();
    CProcessFragments * pFragments = pDoc->GetFragments();
    if (pFragments && pFragments->IsDataReady())
    {
        WORD wSmpSize = WORD(pDoc->GetSampleSize());
        DWORD dwFragmentIndex = pFragments->GetFragmentIndex(m_dwStartDragPos/wSmpSize);
        m_dwDragPos = m_dwStartDragPos;
        pView->SendMessage(WM_USER_CURSOR_IN_FRAGMENT, START_CURSOR, dwFragmentIndex);
    }

    // Set Focus to receive key events so we can track CTRL-SHIFT changes during drag
    SetFocus();
    ClipCursor(rWnd);
    CWnd::OnLButtonDown(nFlags, point);

}

/***************************************************************************/
// CStartCursorWnd::OnLButtonUp Mouse left button Up
// The mouse input is freed for other windows again and the mouse move limit
// is cancelled. Then the new cursor position in the wave data has to be
// calculated and set.
/***************************************************************************/
void CStartCursorWnd::OnLButtonUp(UINT nFlags, CPoint point)
{

    m_bCursorDrag = FALSE;
    ReleaseCapture(); // mouse input also to other windows
    ClipCursor(NULL); // free mouse to move everywhere
    // calculate parent client coordinates
    CPlotWnd * pWnd = (CPlotWnd *)GetParent();
    ClientToScreen(&point);
    pWnd->ScreenToClient(&point);
    // track mouse in plot
    pWnd->SetMousePointerPosition(point);
    pWnd->SetMouseButtonState(nFlags);
    // get window coordinates of parent
    CRect rWnd;
    pWnd->GetClientRect(rWnd);

    // get pointer to parent view
    CGraphWnd * pGraph = (CGraphWnd *)pWnd->GetParent();
    CSaView * pView = (CSaView *)pGraph->GetParent();
    pGraph->SetGraphFocus(TRUE); // Reset Focus

    // set the new positions
    DWORD dwStopCursor = 0;
    DWORD dwStartCursor = CalculateCursorPosition(pView, point.x, rWnd.Width(), &dwStopCursor);

    if (pView->GetStopCursorPosition() >= dwStopCursor)
    {
        dwStopCursor = pView->GetStopCursorPosition();
    }

    int nLoop = pView->FindSelectedAnnotationIndex();
    if (nLoop!=-1)
    {
        if (m_nEditBoundaries == BOUNDARIES_EDIT_BOUNDARIES)
        {
            pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), dwStartCursor, dwStopCursor,CSegment::LIMIT_MOVING_START_NO_OVERLAP); // Limit positions of cursors
        }
        else if (m_nEditBoundaries == BOUNDARIES_EDIT_SEGMENT_SIZE)
        {
            pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), dwStartCursor, dwStopCursor,CSegment::LIMIT_MOVING_START); // Limit positions of cursors
        }

        // clear hint request
        if (pGraph->HaveAnnotation(nLoop))
        {
            // Selected annotation is visible
            CAnnotationWnd * pWnd = pGraph->GetAnnotationWnd(nLoop);
            pWnd->SetHintUpdateBoundaries( false, m_nEditBoundaries == BOUNDARIES_EDIT_SEGMENT_SIZE);//SDM 1.5Test8.1
        }
    }

    // reset start cursor if beyond stop
    if (dwStartCursor >= dwStopCursor)
    {
        dwStartCursor = dwStopCursor - pView->GetDocument()->GetBlockAlign();
    }

	// set the highlight area for raw data
    if (m_bSelectDrag)
    {
		// calculate the location where the mouse was lifted
		CRect rWnd;
		pWnd->GetClientRect(rWnd);
		DWORD dwTempStopCursor = 0;
		DWORD dwCursor = CalculateCursorPosition( pView, point.x, rWnd.Width(), &dwTempStopCursor);
        if (dwCursor > m_dwStartDragPos)
        {
			dwStartCursor = m_dwStartDragPos;
			dwStopCursor = dwCursor;
        }
        else
        {
			dwStartCursor = dwCursor;
			dwStopCursor = m_dwStartDragPos;
        }
		pView->ShowCursors();
		m_bSelectDrag = false;
    }

    // stop cursor is to move also
    pView->SetStopCursorPosition(dwStopCursor, SNAP_RIGHT);
    // move start cursor
    pView->SetCursorPosition( START_CURSOR, dwStartCursor, SNAP_BOTH);

    // detect update request
    //  If CTRL+SHIFT update segment boundaries
    if (((m_nEditBoundaries) && (nLoop!=-1)) &&
        (pView->GetAnnotation(nLoop)->CheckCursors(pView->GetDocument(), m_nEditBoundaries==BOUNDARIES_EDIT_SEGMENT_SIZE) != -1))
    {

        CSaDoc * pDoc = pView->GetDocument();

        int nIndex = pDoc->GetSegment(nLoop)->GetSelection();

        // first adjust cursors to old segment boundaries (undo to here)
        pView->SetStartCursorPosition(pDoc->GetSegment(nLoop)->GetOffset(nIndex), SNAP_LEFT);
        pView->SetStopCursorPosition(pDoc->GetSegment(nLoop)->GetStop(nIndex), SNAP_RIGHT);

        pDoc->CheckPoint(); // Save state

        // Reload cursor locations to new segment boundaries
        pView->SetStartCursorPosition(dwStartCursor, SNAP_LEFT);
        pView->SetStopCursorPosition(dwStopCursor, SNAP_RIGHT);

        // Do update
        pDoc->UpdateSegmentBoundaries(m_nEditBoundaries == BOUNDARIES_EDIT_SEGMENT_SIZE);
    }

    CWnd::OnLButtonUp(nFlags, point);
}

/***************************************************************************/
// CStartCursorWnd::OnRButtonDown Mouse right button down
// This event should initiate a popup menu, so the window sends the message
// further to the parent.
/***************************************************************************/
void CStartCursorWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
    CPlotWnd * pWnd = (CPlotWnd *)GetParent();
    ClientToScreen(&point);
    pWnd->ScreenToClient(&point);
    pWnd->SendMessage(WM_RBUTTONDOWN, nFlags, MAKELONG(point.x, point.y)); // send message to parent
    CWnd::OnRButtonDown(nFlags, point);
}

/***************************************************************************/
// CStartCursorWnd::OnKeyUp Mouse Track key changes during drag
/***************************************************************************/
void CStartCursorWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
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
                pWnd->SetHintUpdateBoundaries( m_nEditBoundaries!=0, m_nEditBoundaries == BOUNDARIES_EDIT_SEGMENT_SIZE);
            }
        }
    }

    pGraph->SendMessage(WM_KEYUP, nChar, MAKELONG(nRepCnt, nFlags));
    CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

/***************************************************************************/
// CStartCursorWnd::OnKeyDown Track key changes during drag
/***************************************************************************/
void CStartCursorWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
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
                pWnd->SetHintUpdateBoundaries( m_nEditBoundaries!=0, m_nEditBoundaries == BOUNDARIES_EDIT_SEGMENT_SIZE);
            }
        }
    }

    pGraph->SendMessage(WM_KEYDOWN, nChar, MAKELONG(nRepCnt, nFlags));
    CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CStartCursorWnd::ResetPosition()
{
    m_rWnd.SetRect(0, 0, 0, 0);
}

BOOL CStartCursorWnd::IsDragging()
{
    return m_bCursorDrag;
}
