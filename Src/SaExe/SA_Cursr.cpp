/////////////////////////////////////////////////////////////////////////////
// sa_cursr.cpp:
// Implementation of the CStartCursorWnd
//                       CStopCursorWnd
//                       CPrivateCursorWnd classes.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
// 1.06.1.2
//       SDM Added code to detect CONTROL+SHIFT on cursor move and cause Update Boundaries
//       SDM Restricted select to control key only
//       SDM Added message handlers OnKey... to Start&Stop cursors
// 1.06.6U4
//       SDM Changed CPrivateCursorWnd to use CPrivateCursorWnd::ChangePosition()
// 1.06.6U5
//       SDM Modified Calculate position to adjust for new plot alignment
// 1.5Test8.1
//       SDM Added support for no overlap drag UpdateBoundaries
// 1.5Test10.2
//       SDM Added annotation deselection on MouseUp if cursors do not include selected
// 1.5Test10.4
//       SDM Made cursor windows dependent on CCursorWnd (new class)
//       SDM added bDrawn flag
// 1.5Test11.0
//       SDM replaced GetOffset() + GetDuration() with CSegment::GetStop()
// 1.5Test11.4
//       SDM added support for editing PHONEMIC/TONE/ORTHO to span multiple segments
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_cursr.h"
#include "sa_plot.h"
#include "sa_graph.h"
#include "Process\sa_proc.h"
#include "sa_segm.h"
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

/////////////////////////////////////////////////////////////////////////////
// CStartCursorWnd message map

BEGIN_MESSAGE_MAP(CStartCursorWnd, CCursorWnd)
	//{{AFX_MSG_MAP(CStartCursorWnd)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_KEYUP()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStartCursorWnd construction/destruction/creation

/***************************************************************************/
// CStartCursorWnd::CStartCursorWnd Constructor
/***************************************************************************/
CStartCursorWnd::CStartCursorWnd()
{
	m_bCursorDrag = FALSE;
	m_dwDragPos = (DWORD)UNDEFINED_OFFSET;
	m_rWnd.SetRect(0, 0, 0, 0);
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
							 const RECT& rect, CWnd* pParentWnd, UINT /*nID*/, CCreateContext* /*pContext*/)
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
BOOL CStartCursorWnd::PreCreateWindow(CREATESTRUCT& cs)
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
DWORD CStartCursorWnd::CalculateCursorPosition(CView* pSaView, 
											   int nPosition,
											   int nWidth, 
											   DWORD* pStopCursor)
{
	CSaView* pView = (CSaView*)pSaView; // cast pointer
	CSaDoc* pDoc = (CSaDoc*)pView->GetDocument(); // get pointer to document
	CPlotWnd* pPlot = (CPlotWnd*)GetParent(); // get pointer to parent plot
	CGraphWnd* pGraph = (CGraphWnd*)pPlot->GetParent(); // get pointer to graph
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
	int nSmpSize = pDoc->GetFmtParm()->wBlockAlign / pDoc->GetFmtParm()->wChannels;
	// calculate data samples per pixel
	ASSERT(nWidth);
	double fSamplesPerPix = (double)dwDataFrame / (double)(nWidth*nSmpSize);
	// calculate the start cursor position
	DWORD dwCursor = (DWORD) round(fDataPos/nSmpSize + ((double)nPosition) * fSamplesPerPix);
	dwCursor = dwCursor*nSmpSize;
	// check the range
	if (dwCursor >= (dwDataSize - (DWORD)nSmpSize)) dwCursor = dwDataSize - (DWORD)(2 * nSmpSize);
	// calculate minimum position for stop cursor
	*pStopCursor = dwCursor + (DWORD)(CURSOR_MIN_DISTANCE * fSamplesPerPix*nSmpSize);
	if (*pStopCursor >= dwDataSize) *pStopCursor = dwDataSize - (DWORD)nSmpSize;
	return dwCursor;
}

/////////////////////////////////////////////////////////////////////////////
// CStartCursorWnd message handlers


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
#ifdef DoNotPaint
		if ( pDC->IsPrinting()
			|| ((rParent != m_rWnd)
			|| ( (rClip.Width() > 1) && rSect.IntersectRect(rClip, rWnd) )
			)
			)
#endif
		{
			// last time we did not paint here - store new locatation and draw cursor
			if (!pDC->IsPrinting())
			{
				m_rWnd = rParent;
			}
			// get color from main frame
			CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
			// check if plot background is dark to set appropriate raster operation mode
			COLORREF cColor = pMainWnd->GetColors()->cPlotBkg;
			BOOL bDarkBkg = FALSE;
			if ((GetRValue(cColor) + GetGValue(cColor) + GetBValue(cColor)) < 381) bDarkBkg = TRUE;
			cColor = pMainWnd->GetColors()->cPlotStartCursor;
			CPen pen(PS_SOLID, 1, cColor);
			CPen* pOldPen = pDC->SelectObject(&pen);
			int oldRop = 0;
			if (FALSE && !pDC->IsPrinting())
			{
				if (bDarkBkg) oldRop = pDC->SetROP2(R2_MASKPENNOT); // set drawing mode for dark bkg
				else oldRop = pDC->SetROP2(R2_MERGEPENNOT); // set drawing mode for light bkg
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
// If the cursor is beeing dragged, the window has to be moved. If it is not
// dragged, and the cursor moves exactly over the center of the cursor window
// the cursor changes to a size symbol.
/***************************************************************************/
void CStartCursorWnd::OnMouseMove(UINT nFlags, CPoint point)
{

	// get pointer to parent plot, graph, and view
	CPlotWnd* pWnd = (CPlotWnd*)GetParent();  // get parent plot
	CGraphWnd* pGraph = (CGraphWnd*)pWnd->GetParent();
	CSaView* pView = (CSaView*)pGraph->GetParent();
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
			if (m_nEditBoundaries == BOUNDARIES_EDIT_NO_OVERLAP) 
			{
				pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), dwCursor, dwStopCursor,CSegment::LIMIT_MOVING_START | CSegment::LIMIT_NO_OVERLAP); // Limit positions of cursors
			} 
			else if (m_nEditBoundaries == BOUNDARIES_EDIT_OVERLAP) 
			{
				pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), dwCursor, dwStopCursor,CSegment::LIMIT_MOVING_START); // Limit positions of cursors
			} 
			else 
			{
				// Added and modified from CStartCursorWnd::OnLButtonUp and modified by AKE 7/22/01 to deselect segment
				// while cursor is moving
				CSegment* pSegment = pView->FindSelectedAnnotation();
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
				CAnnotationWnd* pWnd = pGraph->GetAnnotationWnd(nLoop);
				pWnd->SetHintUpdateBoundaries(m_nEditBoundaries!=0, dwCursor, dwStopCursor,m_nEditBoundaries == BOUNDARIES_EDIT_OVERLAP);
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

		// stop cursor is to move also
		pView->MoveStopCursor(dwStopCursor);
		// move start cursor
		pView->MoveStartCursor(dwCursor);
		// set the highlight area for raw data
		if ((pView->GetFocusedGraphID() == IDD_RAWDATA) && 
			((nFlags&(MK_CONTROL|MK_SHIFT)) == MK_CONTROL) && 
			(!pView->GetEditBoundaries(nFlags)) && 
			(!pView->GetDocument()->IsMultiChannel()) && 
			((pView->GetGraphUpdateMode() == STATIC_UPDATE) || 
			 (!pView->GetDynamicGraphCount()))) 
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
		CSaDoc *pDoc = pView->GetDocument();
		CProcessFragments* pFragments = pDoc->GetFragments();
		if (pFragments && pFragments->IsDataReady()) 
		{
			FmtParm* pFmtParm = pView->GetDocument()->GetFmtParm();
			WORD wSmpSize = WORD(pFmtParm->wBlockAlign / pFmtParm->wChannels);
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
	CPlotWnd* pWnd = (CPlotWnd*)GetParent();
	CGraphWnd* pGraph = (CGraphWnd*)pWnd->GetParent();
	CSaView* pView = (CSaView*)pGraph->GetParent();

	// inform graph
	pGraph->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
	// set boundaries mode
	CRect rCursorRect;
	GetClientRect(rCursorRect);
	if (rCursorRect.PtInRect(point) || !pView->GetEditBoundaries(nFlags)) 
	{
		m_nEditBoundaries = pView->GetEditBoundaries(nFlags);
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
		if (m_nEditBoundaries == BOUNDARIES_EDIT_NO_OVERLAP) 
		{
			pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), m_dwStartDragPos, dwStopCursor,CSegment::LIMIT_MOVING_START | CSegment::LIMIT_NO_OVERLAP); // Limit positions of cursors
		} 
		else if(m_nEditBoundaries == BOUNDARIES_EDIT_OVERLAP) 
		{
			pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), m_dwStartDragPos, dwStopCursor,CSegment::LIMIT_MOVING_START); // Limit positions of cursors
		} 
		else 
		{
			// Moved from CStartCursorWnd::OnLButtonUp and modified by AKE 7/22/01 to deselect segment if
			// not editing segment boundaries in order to allow highlighting for cut and paste operation
			CSegment* pSegment = pView->FindSelectedAnnotation();
			int nIndex = pSegment->GetSelection();
			if ((dwStopCursor > pSegment->GetStop(nIndex)) ||
				(m_dwStartDragPos < pSegment->GetOffset(nIndex))) 
			{
				// Deselect segment
				pView->DeselectAnnotations();
			}
		}
		// detect update request and update annotationWnd to hint
		if (pGraph->HaveAnnotation(nLoop)) 
		{
			// Selected annotation is visible
			CAnnotationWnd* pWnd = pGraph->GetAnnotationWnd(nLoop);
			pWnd->SetHintUpdateBoundaries(m_nEditBoundaries!=0,m_dwStartDragPos, dwStopCursor,m_nEditBoundaries == BOUNDARIES_EDIT_OVERLAP);//SDM 1.5Test8.1
		}
	}

	// stop cursor is to move also
	pView->MoveStopCursor(dwStopCursor);
	// move start cursor
	pView->MoveStartCursor(m_dwStartDragPos);
	// update the status bar
	pGraph->UpdateStatusBar(m_dwStartDragPos, dwStopCursor);
	// clear the highlight area on mouse down
	CGraphWnd *pRaw = pView->GraphIDtoPtr(IDD_RAWDATA);
	if (pRaw) pRaw->GetPlot()->SetHighLightArea(0, 0);
	// limit cursor movement to graph window
	pWnd->ClientToScreen(&rWnd);

	// AKE 07/15/00, for tracking movement of start cursor as it is dragged
	CSaDoc *pDoc = pView->GetDocument();
	CProcessFragments* pFragments = pDoc->GetFragments();
	if (pFragments && pFragments->IsDataReady()) 
	{
		FmtParm* pFmtParm = pView->GetDocument()->GetFmtParm();
		WORD wSmpSize = WORD(pFmtParm->wBlockAlign / pFmtParm->wChannels);
		DWORD dwFragmentIndex = pFragments->GetFragmentIndex(m_dwStartDragPos/wSmpSize);
		m_dwDragPos = m_dwStartDragPos;
		pView->SendMessage(WM_USER_CURSOR_IN_FRAGMENT, START_CURSOR, dwFragmentIndex);
	}
	// Set Focus to receive key events so we can track CTRL-SHIFT changes during drag
	SetFocus();
	ClipCursor(rWnd);
	CWnd::OnLButtonDown(nFlags, point);
	pView->BroadcastMessage(WM_USER_CURSOR_CLICKED, START_CURSOR, WM_LBUTTONDOWN);
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
	CPlotWnd* pWnd = (CPlotWnd*)GetParent();
	ClientToScreen(&point);
	pWnd->ScreenToClient(&point);
	// track mouse in plot
	pWnd->SetMousePointerPosition(point);
	pWnd->SetMouseButtonState(nFlags);
	// get window coordinates of parent
	CRect rWnd;
	pWnd->GetClientRect(rWnd);

	// get pointer to parent view
	CGraphWnd* pGraph = (CGraphWnd*)pWnd->GetParent();
	CSaView* pView = (CSaView*)pGraph->GetParent();
	pGraph->SetGraphFocus(TRUE); // Reset Focus

	// set the new positions
	DWORD dwStopCursor;
	DWORD dwStartCursor = CalculateCursorPosition(pView, point.x, rWnd.Width(), &dwStopCursor);
	if (pView->GetStopCursorPosition() >= dwStopCursor) 
	{
		dwStopCursor = pView->GetStopCursorPosition();
	}
	int nLoop = pView->FindSelectedAnnotationIndex();
	if (nLoop!=-1) 
	{
		if (m_nEditBoundaries == BOUNDARIES_EDIT_NO_OVERLAP) 
		{
			pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), dwStartCursor, dwStopCursor,CSegment::LIMIT_MOVING_START | CSegment::LIMIT_NO_OVERLAP); // Limit positions of cursors
		} 
		else if(m_nEditBoundaries == BOUNDARIES_EDIT_OVERLAP) 
		{
			pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), dwStartCursor, dwStopCursor,CSegment::LIMIT_MOVING_START); // Limit positions of cursors
		}

		// clear hint request
		if (pGraph->HaveAnnotation(nLoop)) 
		{
			// Selected annotation is visible
			CAnnotationWnd* pWnd = pGraph->GetAnnotationWnd(nLoop);
			pWnd->SetHintUpdateBoundaries(FALSE,m_nEditBoundaries == BOUNDARIES_EDIT_OVERLAP);//SDM 1.5Test8.1
		}
	}

	// reset start cursor if beyond stop
	if (dwStartCursor >= dwStopCursor) 
	{
		FmtParm* pFmtParm = pView->GetDocument()->GetFmtParm();
		dwStartCursor = dwStopCursor - pFmtParm->wBlockAlign;
	}

	// stop cursor is to move also
	pView->SetStopCursorPosition(dwStopCursor, SNAP_RIGHT);
	// move start cursor
	pView->SetCursorPosition(START_CURSOR, dwStartCursor, SNAP_BOTH);

	// detect update request
	//  If CTRL+SHIFT update segment boundaries
	if (( m_nEditBoundaries && (nLoop!=-1)) &&
		(pView->GetAnnotation(nLoop)->CheckCursors(pView->GetDocument(), m_nEditBoundaries==BOUNDARIES_EDIT_OVERLAP) != -1)) 
	{

		CSaDoc* pDoc = pView->GetDocument();

		int nIndex = pDoc->GetSegment(nLoop)->GetSelection();

		// first adjust cursors to old segment boundaries (undo to here)
		pView->SetStartCursorPosition(pDoc->GetSegment(nLoop)->GetOffset(nIndex), SNAP_LEFT);
		pView->SetStopCursorPosition(pDoc->GetSegment(nLoop)->GetStop(nIndex), SNAP_RIGHT);

		pDoc->CheckPoint(); // Save state

		// Reload cursor locations to new segment boundaries
		pView->SetStartCursorPosition(dwStartCursor, SNAP_LEFT);
		pView->SetStopCursorPosition(dwStopCursor, SNAP_RIGHT);

		// Do update
		pDoc->UpdateSegmentBoundaries(m_nEditBoundaries == BOUNDARIES_EDIT_OVERLAP);
	}

	CWnd::OnLButtonUp(nFlags, point);
	pView->BroadcastMessage(WM_USER_CURSOR_CLICKED, START_CURSOR, WM_LBUTTONUP);
}

/***************************************************************************/
// CStartCursorWnd::OnRButtonDown Mouse right button down
// This event should initiate a popup menu, so the window sends the message
// further to the parent.
/***************************************************************************/
void CStartCursorWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
	CPlotWnd* pWnd = (CPlotWnd*)GetParent();
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
	CWnd* pWnd = GetParent();
	CGraphWnd* pGraph = (CGraphWnd*)pWnd->GetParent();

	if(m_bCursorDrag)
	{
		CSaView* pView = (CSaView*)pGraph->GetParent();
		int nLoop = pView->FindSelectedAnnotationIndex();
		if(nLoop != -1)
		{
			//int m_nEditBoundaries = pView->GetEditBoundaries(nFlags);
			if(pGraph->HaveAnnotation(nLoop))// Selected annotation is visible
			{
				CAnnotationWnd* pWnd = pGraph->GetAnnotationWnd(nLoop);
				pWnd->SetHintUpdateBoundaries(m_nEditBoundaries!=0,m_nEditBoundaries == BOUNDARIES_EDIT_OVERLAP);
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
	CWnd* pWnd = GetParent();
	CGraphWnd* pGraph = (CGraphWnd*)pWnd->GetParent();

	if(m_bCursorDrag)
	{
		CSaView* pView = (CSaView*)pGraph->GetParent();
		int nLoop = pView->FindSelectedAnnotationIndex();
		if(nLoop != -1)
		{
			//int editBoundaries = pView->GetEditBoundaries(nFlags);
			if(pGraph->HaveAnnotation(nLoop))// Selected annotation is visible
			{
				CAnnotationWnd* pWnd = pGraph->GetAnnotationWnd(nLoop);
				pWnd->SetHintUpdateBoundaries(m_nEditBoundaries!=0,m_nEditBoundaries == BOUNDARIES_EDIT_OVERLAP);
			}
		}
	}

	pGraph->SendMessage(WM_KEYDOWN, nChar, MAKELONG(nRepCnt, nFlags));
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

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
	//{{AFX_MSG_MAP(CStopCursorWnd)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStopCursorWnd construction/destruction/creation

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
							const RECT& rect, CWnd* pParentWnd, UINT /*nID*/, CCreateContext* /*pContext*/)
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
BOOL CStopCursorWnd::PreCreateWindow(CREATESTRUCT& cs)
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
DWORD CStopCursorWnd::CalculateCursorPosition( CView* pSaView, 
											   int nPosition,
											   int nWidth, 
											   DWORD* pStartCursor)
{
	CSaView* pView = (CSaView*)pSaView; // cast pointer
	CSaDoc* pDoc = (CSaDoc*)pView->GetDocument(); // get pointer to document
	CPlotWnd* pPlot = (CPlotWnd*)GetParent(); // get pointer to parent plot
	CGraphWnd* pGraph = (CGraphWnd*)pPlot->GetParent(); // get pointer to graph
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
	int nSmpSize = pDoc->GetFmtParm()->wBlockAlign / pDoc->GetFmtParm()->wChannels;
	// calculate data samples per pixel
	ASSERT(nWidth);
	double fSamplesPerPix = (double)dwDataFrame / (double)(nWidth*nSmpSize);

	TRACE("sampperpix=%f\n",fSamplesPerPix);

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
	if (dwCursor > dwDifference) *pStartCursor = dwCursor - dwDifference;
	return dwCursor;
}

/////////////////////////////////////////////////////////////////////////////
// CStopCursorWnd message handlers



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
#ifdef DoNotPaint
		if ( pDC->IsPrinting()
			|| ((rParent != m_rWnd)
			|| (  (rClip.Width() > 1) && rSect.IntersectRect(rClip, rWnd)   )
			)
			)
#endif
		{
			// last time we did not paint here - store new locatation and draw cursor
			if (!pDC->IsPrinting())
			{
				m_rWnd = rParent;
			}
			// get color from main frame
			CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
			// check if plot background is dark to set appropriate raster operation mode
			COLORREF cColor = pMainWnd->GetColors()->cPlotBkg;
			BOOL bDarkBkg = FALSE;
			if ((GetRValue(cColor) + GetGValue(cColor) + GetBValue(cColor)) < 381) bDarkBkg = TRUE;
			cColor = pMainWnd->GetColors()->cPlotStopCursor;
			CPen pen(PS_SOLID, 1, cColor);
			CPen* pOldPen = pDC->SelectObject(&pen);
			int oldRop = 0;
			if (FALSE && !pDC->IsPrinting())
			{
				if (bDarkBkg) oldRop = pDC->SetROP2(R2_MASKPENNOT); // set drawing mode for dark bkg
				else oldRop = pDC->SetROP2(R2_MERGEPENNOT); // set drawing mode for light bkg
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
void CStopCursorWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	// get pointer to parent plot, parent graph and to view
	CPlotWnd* pWnd = (CPlotWnd*)GetParent();
	CGraphWnd* pGraph = (CGraphWnd*)pWnd->GetParent();
	CSaView* pView = (CSaView*)pGraph->GetParent();
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
			if (m_nEditBoundaries  == BOUNDARIES_EDIT_NO_OVERLAP) 
			{
				pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), dwStartCursor, dwCursor,CSegment::LIMIT_MOVING_STOP | CSegment::LIMIT_NO_OVERLAP); // Limit positions of cursors
			} 
			else if (m_nEditBoundaries  == BOUNDARIES_EDIT_OVERLAP) 
			{
				pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), dwStartCursor, dwCursor,CSegment::LIMIT_MOVING_STOP); // Limit positions of cursors
			} 
			else 
			{
				// Added and modified from CStopCursorWnd::OnLButtonUp and modified by AKE 7/22/01 to deselect segment
				// while cursor is moving
				CSegment* pSegment = pView->FindSelectedAnnotation();
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
				CAnnotationWnd* pWnd = pGraph->GetAnnotationWnd(nLoop);
				pWnd->SetHintUpdateBoundaries(m_nEditBoundaries!=0, dwStartCursor, dwCursor,m_nEditBoundaries  == BOUNDARIES_EDIT_OVERLAP);//SDM 1.5Test8.1
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
		CSaDoc *pDoc = pView->GetDocument();
		CProcessFragments* pFragments = pDoc->GetFragments();
		if (pFragments && pFragments->IsDataReady()) 
		{
			FmtParm* pFmtParm = pView->GetDocument()->GetFmtParm();
			WORD wSmpSize = WORD(pFmtParm->wBlockAlign / pFmtParm->wChannels);
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
void CStopCursorWnd::OnLButtonDown( UINT nFlags, CPoint point)
{
	// get pointer to parent plot, parent graph and to view
	CPlotWnd* pWnd = (CPlotWnd*)GetParent();
	CGraphWnd* pGraph = (CGraphWnd*)pWnd->GetParent();
	CSaView* pView = (CSaView*)pGraph->GetParent();
	// inform graph
	pGraph->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
	// set boundaries mode
	CRect rCursorRect;
	GetClientRect(rCursorRect);
	if (rCursorRect.PtInRect(point) || !pView->GetEditBoundaries(nFlags)) 
	{
		m_nEditBoundaries = pView->GetEditBoundaries(nFlags);
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
		if (m_nEditBoundaries  == BOUNDARIES_EDIT_NO_OVERLAP) 
		{
			pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), dwStartCursor, dwCursor,CSegment::LIMIT_MOVING_STOP | CSegment::LIMIT_NO_OVERLAP); // Limit positions of cursors
		} 
		else if (m_nEditBoundaries  == BOUNDARIES_EDIT_OVERLAP) 
		{
			pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), dwStartCursor, dwCursor,CSegment::LIMIT_MOVING_STOP); // Limit positions of cursors
		} 
		else 
		{
			// Moved from CStopCursorWnd::OnLButtonUp and modified by AKE 7/22/01 to deselect segment,
			// providing consistent operation with start cursor
			CSegment* pSegment = pView->FindSelectedAnnotation();
			int nIndex = pSegment->GetSelection();
			if ((dwCursor > pSegment->GetStop(nIndex)) ||
				(dwStartCursor < pSegment->GetOffset(nIndex))) 
			{
				// Deselect segment
				pView->DeselectAnnotations();
			}
		}
		// detect update request and update annotationWnd to hint
		if (pGraph->HaveAnnotation(nLoop))// Selected annotation is visible
		{
			CAnnotationWnd* pWnd = pGraph->GetAnnotationWnd(nLoop);
			pWnd->SetHintUpdateBoundaries(m_nEditBoundaries!=0, dwStartCursor, dwCursor,m_nEditBoundaries  == BOUNDARIES_EDIT_OVERLAP);//SDM 1.5Test8.1
		}
	}

	TRACE("start=%lu stop=%lu\n",dwStartCursor,dwCursor);

	// move start cursor
	pView->MoveStartCursor(dwStartCursor);
	// stop cursor is to move also
	pView->MoveStopCursor(dwCursor);
	// clear the highlight area on mouse down
	CGraphWnd *pRaw = pView->GraphIDtoPtr(IDD_RAWDATA);
	if (pRaw) pRaw->GetPlot()->SetHighLightArea(0, 0);
	// update the status bar
	pGraph->UpdateStatusBar(dwStartCursor, dwCursor);
	// AKE 07/15/00, for tracking movement of start cursor as it is dragged
	CSaDoc *pDoc = pView->GetDocument();
	CProcessFragments* pFragments = pDoc->GetFragments();
	if (pFragments && pFragments->IsDataReady()) 
	{
		FmtParm* pFmtParm = pView->GetDocument()->GetFmtParm();
		WORD wSmpSize = WORD(pFmtParm->wBlockAlign / pFmtParm->wChannels);
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
	CWnd* pWnd = GetParent();
	ClientToScreen(&point);
	pWnd->ScreenToClient(&point);
	// get window coordinates of parent
	CRect rWnd;
	pWnd->GetClientRect(rWnd);
	// get pointer to parent view
	CGraphWnd* pGraph = (CGraphWnd*)pWnd->GetParent();
	CSaView* pView = (CSaView*)pGraph->GetParent();
	pGraph->SetGraphFocus(TRUE); // Reset Focus

	// set the new positions
	DWORD dwStartCursor;
	DWORD dwCursor = CalculateCursorPosition(pView, point.x, rWnd.Width(), &dwStartCursor);
	if (pView->GetStartCursorPosition() <= dwStartCursor)
	{
		dwStartCursor = pView->GetStartCursorPosition();
	}
	int nLoop = pView->FindSelectedAnnotationIndex();
	if(nLoop!=-1)
	{
		if (m_nEditBoundaries  == BOUNDARIES_EDIT_NO_OVERLAP)
		{
			pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), dwStartCursor, dwCursor,CSegment::LIMIT_MOVING_STOP | CSegment::LIMIT_NO_OVERLAP); // Limit positions of cursors
		}
		else if (m_nEditBoundaries  == BOUNDARIES_EDIT_OVERLAP)
		{
			pView->GetAnnotation(nLoop)->LimitPosition(pView->GetDocument(), dwStartCursor, dwCursor,CSegment::LIMIT_MOVING_STOP); // Limit positions of cursors
		}
		// detect update request and update annotationWnd to hint
		if(pGraph->HaveAnnotation(nLoop))// Selected annotation is visible
		{
			CAnnotationWnd* pWnd = pGraph->GetAnnotationWnd(nLoop);
			pWnd->SetHintUpdateBoundaries(FALSE,m_nEditBoundaries  == BOUNDARIES_EDIT_OVERLAP);//SDM 1.5Test8.1
		}
	}

	// move start cursor
	pView->SetStartCursorPosition(dwStartCursor, SNAP_LEFT);

	// move stop cursor also
	pView->SetCursorPosition(STOP_CURSOR, dwCursor, SNAP_BOTH);

	// detect update request
	//  If CTRL+SHIFT update segment boundaries
	if(m_nEditBoundaries&&(nLoop!=-1)
		&&(pView->GetAnnotation(nLoop)->CheckCursors(pView->GetDocument(),m_nEditBoundaries  == BOUNDARIES_EDIT_OVERLAP) != -1)) // Only allow update of PHONETIC and GLOSS
	{
		CSaDoc* pDoc = pView->GetDocument();
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
		pDoc->UpdateSegmentBoundaries(m_nEditBoundaries  == BOUNDARIES_EDIT_OVERLAP);//SDM 1.5Test8.1
	}
	/* Moved to CStopCursorWnd::OnLButtonDown by AKE 7/22/01 to deselect annotation immediately
	if(pView->FindSelectedAnnotation())
	{
	CSegment* pSegment = pView->FindSelectedAnnotation();
	int nIndex = pSegment->GetSelection();

	if((pView->GetStopCursorPosition() > pSegment->GetStop(nIndex)) ||
	(pView->GetStartCursorPosition() < pSegment->GetOffset(nIndex)))
	{
	// Deselect segment
	pView->ChangeAnnotationSelection(pSegment, nIndex, 0, 0);
	pView->RefreshGraphs(FALSE); // refresh the graphs between cursors
	}
	}
	*/
	CWnd::OnLButtonUp(nFlags, point);
}

/***************************************************************************/
// CStopCursorWnd::OnRButtonDown Mouse right button down
// This event should initiate a popup menu, so the window sends the message
// further to the parent.
/***************************************************************************/
void CStopCursorWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
	CPlotWnd* pWnd = (CPlotWnd*)GetParent();
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
	CWnd* pWnd = GetParent();
	CGraphWnd* pGraph = (CGraphWnd*)pWnd->GetParent();

	if(m_bCursorDrag)
	{
		CSaView* pView = (CSaView*)pGraph->GetParent();
		int nLoop = pView->FindSelectedAnnotationIndex();
		if(nLoop != -1)
		{
			//int m_nEditBoundaries = pView->GetEditBoundaries(nFlags);
			if(pGraph->HaveAnnotation(nLoop))// Selected annotation is visible
			{
				CAnnotationWnd* pWnd = pGraph->GetAnnotationWnd(nLoop);
				pWnd->SetHintUpdateBoundaries(m_nEditBoundaries!=0,m_nEditBoundaries == BOUNDARIES_EDIT_OVERLAP);//SDM 1.5Test8.1
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
	CWnd* pWnd = GetParent();
	CGraphWnd* pGraph = (CGraphWnd*)pWnd->GetParent();

	if(m_bCursorDrag)
	{
		CSaView* pView = (CSaView*)pGraph->GetParent();
		int nLoop = pView->FindSelectedAnnotationIndex();
		if(nLoop != -1)
		{
			//int m_nEditBoundaries = pView->GetEditBoundaries(nFlags);
			if(pGraph->HaveAnnotation(nLoop))// Selected annotation is visible
			{
				CAnnotationWnd* pWnd = pGraph->GetAnnotationWnd(nLoop);
				pWnd->SetHintUpdateBoundaries(m_nEditBoundaries!=0,m_nEditBoundaries == BOUNDARIES_EDIT_OVERLAP);//SDM 1.5Test8.1
			}
		}
	}

	pGraph->SendMessage(WM_KEYUP, nChar, MAKELONG(nRepCnt, nFlags));
	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

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
	//{{AFX_MSG_MAP(CPrivateCursorWnd)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrivateCursorWnd construction/destruction/creation

/***************************************************************************/
// CPrivateCursorWnd::CPrivateCursorWnd Constructor
/***************************************************************************/
CPrivateCursorWnd::CPrivateCursorWnd()
{
	m_bCursorDrag = FALSE;
	m_rWnd.SetRect(0, 0, 0, 0);
}

/***************************************************************************/
// CPrivateCursorWnd::~CPrivateCursorWnd Destructor
/***************************************************************************/
CPrivateCursorWnd::~CPrivateCursorWnd()
{
}

/***************************************************************************/
// CPrivateCursorWnd::Create Creation
// Creates a child window with the given parameters.
/***************************************************************************/
BOOL CPrivateCursorWnd::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
							   const RECT& rect, CWnd* pParentWnd, UINT /*nID*/, CCreateContext* /*pContext*/)
{
	return CWnd::CreateEx(WS_EX_TRANSPARENT, lpszClassName, lpszWindowName, dwStyle, rect.left,
		rect.top, rect.right - rect.left, rect.bottom - rect.top,
		pParentWnd->GetSafeHwnd(), 0);
}

/***************************************************************************/
// CPrivateCursorWnd::PreCreateWindow Creation
// Called from the framework before the creation of the window. Registers
// the new window class.
/***************************************************************************/
BOOL CPrivateCursorWnd::PreCreateWindow(CREATESTRUCT& cs)
{
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
void CPrivateCursorWnd::OnPaint()
{
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
void CPrivateCursorWnd::OnDraw(CDC * pDC, const CRect & printRect)
{
	CPlotWnd* pWnd = (CPlotWnd*)GetParent();
	CGraphWnd* pGraph = (CGraphWnd*)pWnd->GetParent();
	if (pGraph->IsPlotID(IDD_TWC))
	{
		OnDrawHorizontalCursor(pDC,printRect);
		return;
	}

	//TRACE(_T("Drawn\n"))
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
#ifdef DoNotPaint
		if ( pDC->IsPrinting()
			|| ((rParent != m_rWnd)
			|| ( (rClip.Width() > 1) && rSect.IntersectRect(rClip, rWnd)   )
			)
			)
#endif
		{
			// last time we did not paint here - store new locatation and draw cursor
			if (!pDC->IsPrinting())
			{
				m_rWnd = rParent;
			}
			// get color from main frame
			CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
			// check if plot background is dark to set appropriate raster operation mode
			COLORREF cColor = pMainWnd->GetColors()->cPlotBkg;
			BOOL bDarkBkg = FALSE;
			if ((GetRValue(cColor) + GetGValue(cColor) + GetBValue(cColor)) < 381) bDarkBkg = TRUE;
			cColor = pMainWnd->GetColors()->cPlotPrivateCursor;
			CPen pen(PS_SOLID, 1, cColor);
			CPen* pOldPen = pDC->SelectObject(&pen);
			int oldRop = 0;
			if (FALSE && !pDC->IsPrinting())
			{
				if (bDarkBkg) oldRop = pDC->SetROP2(R2_MASKPENNOT); // set drawing mode for dark bkg
				else oldRop = pDC->SetROP2(R2_MERGEPENNOT); // set drawing mode for light bkg
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
// CPrivateCursorWnd::OnDrawHorizontalCursor - TCJ 5/9/00
//
// *** Modify this function plus ::OnDrawHorizontalCursor(), ::OnLButtonDown() & code
//      in CPlotWnd::OnLButtonDown() to add HORIZONTAL cursor support for new windows.
/***************************************************************************/
void CPrivateCursorWnd::OnDrawHorizontalCursor(CDC * pDC, const CRect & printRect)
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

		if (rWnd.Height() == 0)
		{
			m_rWnd.SetRect(0, 0, 0, 0);
		}
	}

	if (rClip.Width() > 0)
	{
		if (rWnd.Height() > 1)
		{
			// cursor window is larger than one pixel
			rWnd.top = CURSOR_WINDOW_HALFWIDTH;
			rWnd.bottom = rWnd.top + 1;
		}

		if (!pDC->IsPrinting())
		{
			// get the coordinates in the parent window
			rParent = rWnd;
			ClientToScreen(rParent);
			GetParent()->ScreenToClient(rParent);
		}

		//SDM 1.06.6

#ifdef DoNotPaint
		if ( pDC->IsPrinting()
			|| ((rParent != m_rWnd)
			|| ( (rClip.Height() > 1) && rSect.IntersectRect(rClip, rWnd))
			)
			)
#endif
		{
			// last time we did not paint here - store new locatation and draw cursor
			if (!pDC->IsPrinting())
			{
				m_rWnd = rParent;
			}
			// get color from main frame
			CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
			// check if plot background is dark to set appropriate raster operation mode
			COLORREF cColor = pMainWnd->GetColors()->cPlotBkg;
			BOOL bDarkBkg = FALSE;
			if ((GetRValue(cColor) + GetGValue(cColor) + GetBValue(cColor)) < 381) bDarkBkg = TRUE;
			cColor = pMainWnd->GetColors()->cPlotPrivateCursor;
			CPen pen(PS_SOLID, 1, cColor);
			CPen* pOldPen = pDC->SelectObject(&pen);
			int oldRop = 0;
			if (FALSE && !pDC->IsPrinting())
			{
				if (bDarkBkg) oldRop = pDC->SetROP2(R2_MASKPENNOT); // set drawing mode for dark bkg
				else oldRop = pDC->SetROP2(R2_MERGEPENNOT); // set drawing mode for light bkg
			}

			pDC->MoveTo(rClip.left, rWnd.top);
			pDC->LineTo(rClip.right, rWnd.top);

			if (FALSE && !pDC->IsPrinting())
			{
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
void CPrivateCursorWnd::OnMouseMove(UINT nFlags, CPoint mousePoint)
{
	if (m_bCursorDrag)
	{
		// get pointer to parent plot, parent graph and to view
		CPlotWnd* pWnd = (CPlotWnd*)GetParent();
		// move cursor
		CPoint point = mousePoint;
		ClientToScreen(&point);
		pWnd->ScreenToClient(&point);
		// SDM 1.06.6U4
		ChangeCursorPosition(point);
	}
	else
	{
		// TCJ 5/25/00 - Added support for horizontal cursor
		// get pointer to parent plot
		CPlotWnd* pWnd = (CPlotWnd*)GetParent();
		CGraphWnd* pGraph = (CGraphWnd*)pWnd->GetParent();
		if (pGraph->IsPlotID(IDD_TWC))
		{
			if (mousePoint.y == CURSOR_WINDOW_HALFWIDTH) // cursor is over the line
			{
				// create Vertical two-headed arrow pointer, for horizontal cursor bar
				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENS));
			}
		}
		else
		{
			if (mousePoint.x == CURSOR_WINDOW_HALFWIDTH) // cursor is over the line
			{
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
void CPrivateCursorWnd::OnLButtonDown(UINT nFlags, CPoint mousePoint)
{
	// get pointer to parent plot, parent graph and to view
	CPlotWnd* pWnd = (CPlotWnd*)GetParent();
	CGraphWnd* pGraph = (CGraphWnd*)pWnd->GetParent();
	// inform graph
	pGraph->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(mousePoint.x, mousePoint.y));
	// set drag mode
	m_bCursorDrag = TRUE;
	SetCapture(); // receive all mouse input

	if (pGraph->IsPlotID(IDD_TWC))
	{
		// create Vertical two-headed arrow pointer, for horizontal cursor bar - TCJ 5/2000
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENS));
	}
	else
	{
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
void CPrivateCursorWnd::OnLButtonUp(UINT nFlags, CPoint mousePoint)
{
	m_bCursorDrag = FALSE;
	ReleaseCapture(); // mouse input also to other windows
	ClipCursor(NULL); // free mouse to move everywhere
	// get pointer to parent plot, parent graph and to view
	CPlotWnd* pWnd = (CPlotWnd*)GetParent();
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
void CPrivateCursorWnd::ChangeCursorPosition(CPoint point)
{
	// get pointer to parent plot
	CPlotWnd* pWnd = (CPlotWnd*)GetParent();
	CGraphWnd* pGraph = (CGraphWnd*)pWnd->GetParent();
	if (pGraph->IsPlotID(IDD_TWC))
	{
		ChangeHorizontalCursorPosition(point);
		return;
	}

	CRect rOldLine, rNewLine, rNewWnd;
	// get the actual (old) position of cursor window
	GetWindowRect(rOldLine);
	pWnd->ScreenToClient(rOldLine);
	// get the line position in the middle
	if (rOldLine.Width() > 1) // cursor window has large width
	{
		rOldLine.left += CURSOR_WINDOW_HALFWIDTH;
		rOldLine.right -= (CURSOR_WINDOW_HALFWIDTH - 1);
	}
	rNewLine.SetRect(point.x, rOldLine.top, point.x + 1, rOldLine.bottom);
	rNewWnd.SetRect(point.x - CURSOR_WINDOW_HALFWIDTH, rOldLine.top, point.x + CURSOR_WINDOW_HALFWIDTH, rOldLine.bottom);
	// check if new cursor line position
	if (rOldLine != rNewLine)
	{
		// invalidate and update old position
		pWnd->InvalidateRect(rOldLine, TRUE); // redraw old cursor position
		MoveWindow(rNewWnd, FALSE); // move the cursor window to the new position
		rNewLine.SetRect(rNewLine.left - rNewWnd.left, rNewLine.top, rNewLine.left - rNewWnd.left + 1, rNewLine.bottom);
		InvalidateRect(rNewLine, TRUE); // redraw new cursor line
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
void CPrivateCursorWnd::ChangeHorizontalCursorPosition(CPoint point)
{
	// get pointer to parent plot, parent graph and to view
	CPlotWnd* pWnd = (CPlotWnd*)GetParent();
	CGraphWnd* pGraph = (CGraphWnd*)pWnd->GetParent();
	CRect rOldLine, rNewLine, rNewWnd;

	// get the actual (old) position of cursor window
	GetWindowRect(rOldLine);
	pWnd->ScreenToClient(rOldLine);

	// get the line position in the middle
	if (rOldLine.Height() > 1) // cursor window has large width
	{
		rOldLine.top += CURSOR_WINDOW_HALFWIDTH;
		rOldLine.bottom -= (CURSOR_WINDOW_HALFWIDTH - 1);
	}

	rNewLine.SetRect(rOldLine.left, point.y, rOldLine.right, point.y + 1);
	rNewWnd.SetRect(rOldLine.left, point.y - CURSOR_WINDOW_HALFWIDTH, rOldLine.right, point.y + CURSOR_WINDOW_HALFWIDTH);

	// check if new cursor line position
	if (rOldLine != rNewLine)
	{
		// invalidate and update old position
		pWnd->InvalidateRect(rOldLine, TRUE); // redraw old cursor position
		MoveWindow(rNewWnd, FALSE); // move the cursor window to the new position

		//  rNewLine.SetRect(rNewLine.left, rNewLine.top - rNewWnd.top, rNewLine.right, rNewLine.top - rNewWnd.top + 1);
		rNewLine.SetRect(rNewLine.left, rNewLine.top - rNewWnd.top + 8, rNewLine.right, rNewLine.top - rNewWnd.top + 1 + 8);

		InvalidateRect(rNewLine, TRUE); // redraw new cursor line
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
void CPrivateCursorWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
	CPlotWnd* pWnd = (CPlotWnd*)GetParent();
	ClientToScreen(&point);
	pWnd->ScreenToClient(&point);
	pWnd->SendMessage(WM_RBUTTONDOWN, nFlags, MAKELONG(point.x, point.y)); // send message to parent
	CWnd::OnRButtonDown(nFlags, point);
}


