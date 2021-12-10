/////////////////////////////////////////////////////////////////////////////
// sa_plot.cpp:
// Implementation of the CPlotHelperWnd
//                       CPlotWnd        classes.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
// 1.06.1.2
//       SDM disable cursor move on mouse down if CONTROL + SHIFT
// 1.06.5
//       SDM fixed bug in cursor redraw in ChangeCursorPosition
// 1.06.6U4
//       SDM fixed scrolling bugs, aligned graphs to pixel boundaries
// 1.06.6U6
//       SDM Added function SetPlaybackPosition
//       SDM Added member m_pPlaybackCursor
// 1.5Test8.5
//       SDM Added support for dynamic popup menu
// 1.5Test10.5
//       SDM modified ChangeCursorPosition to use CCursorWnd::bDrawn flag
// 1.5Test13.5
//  10/12/00 CLW made TWC grid lines dashed
//
//  10/24/00 DDO Changed the OnEraseBkgnd function so it calls a new
//               function called EraseBkgnd(). I needed to make OnEraseBkgnd
//               virtual so I could override it in the derived recording
//               (i.e. position view) plot class.
//  04/21/01 AKE fixed standard plot to use same type of calculation for process
//               data indexing as cursor offset into wave file.
//  4/2002
//       SDM Revised PlotStandardPaint to be more object oriented and more
//           maintainable.  Included the drawing object declarations here
/***************************************************************************/
#include "stdafx.h"
#include "sa_plot.h"
#include "Process\Process.h"
#include "Process\sa_p_lou.h"
#include "Process\sa_p_gra.h"
#include "Process\sa_p_fra.h"
#include "sa_minic.h"
#include "sa_graph.h"
#include "Segment.h"
#include "sa_doc.h"
#include "sa.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "math.h"
#include "string.h"
#include "PrivateCursorWnd.h"
#include "LegendWnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// 10% margin for scroll during playback
#define SCROLL_MARGIN 10

//###########################################################################
// CPlotHelperWnd
// Helper window to display additional user information. It displays various
// information on different positions in the parents client area depending
// on the set display mode.

IMPLEMENT_DYNCREATE(CPlotHelperWnd, CWnd)

BEGIN_MESSAGE_MAP(CPlotHelperWnd, CWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
    ON_WM_CREATE()
END_MESSAGE_MAP()

extern CSaApp NEAR theApp;
/////////////////////////////////////////////////////////////////////////////
// CPlotHelperWnd construction/destruction/creation

/***************************************************************************/
// CPlotHelperWnd::CPlotHelperWnd Constructor
/***************************************************************************/
CPlotHelperWnd::CPlotHelperWnd() {
    m_nMode = 0; // default mode = hidden window
    m_szText = "";
    m_rParent.SetRect(0, 0, 0, 0);
    m_nHeight = 0;
    m_nCharWidth = 0;
}

/***************************************************************************/
// CPlotHelperWnd::CPlotHelperWnd Destructor
/***************************************************************************/
CPlotHelperWnd::~CPlotHelperWnd() {
}

/***************************************************************************/
// CPlotHelperWnd::OnCreate Window creation
/***************************************************************************/
int CPlotHelperWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }
    // create the helper window font (same font as status bar)
    LPCTSTR pszName = _T("MS Sans Serif");
    m_font.CreateFont(-10, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                      ANSI_CHARSET, OUT_DEFAULT_PRECIS, (CLIP_LH_ANGLES | CLIP_STROKE_PRECIS),
                      DEFAULT_QUALITY, FF_SWISS, pszName);
    // get text metrics
    TEXTMETRIC tm;
    CDC * pDC = GetDC();
    // select the helper window font
    CFont * oldFont = pDC->SelectObject(&m_font);
    pDC->GetTextMetrics(&tm);
    // set the average char width and the height
    m_nCharWidth = tm.tmAveCharWidth;
    m_nHeight = tm.tmHeight;
    pDC->SelectObject(oldFont); // set back old font
    ReleaseDC(pDC);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CPlotHelperWnd helper functions

/***************************************************************************/
// CPlotHelperWnd::SetPosition Set the helper windows position
// The function sets the position of the helper window in the client area
// depending on the set mode, and returns a CRect with the result. The
// width and height parameters must contain the correct (total) width and
// height of the helper window.
/***************************************************************************/
CRect CPlotHelperWnd::SetPosition(int nWidth, int nHeight, CRect * prParent) {
    // set default (left top)
    CRect rHelper(prParent->left, prParent->top,
                  prParent->left + nWidth, prParent->top + nHeight);
    if (m_nMode & POS_HCENTER) {
        // center horizontally
        rHelper.left += (prParent->Width() - nWidth) / 2;
        rHelper.right = rHelper.left + nWidth;
    }
    if (m_nMode & POS_HRIGHT) {
        // align right
        rHelper.left = prParent->right - nWidth;
        rHelper.right = prParent->right;
    }
    if (m_nMode & POS_VCENTER) {
        // center vertically
        rHelper.top += (prParent->Height() - nHeight) / 2;
        rHelper.bottom = rHelper.top + nHeight;
    }
    if (m_nMode & POS_VBOTTOM) {
        // align bottom
        rHelper.top = prParent->bottom - nHeight;
        rHelper.bottom = prParent->bottom;
    }
    return rHelper; // return the result
}

/***************************************************************************/
// CPlotHelperWnd::SetMode Set display mode
// The function sets a new display mode and returns the old one. It does not
// change the helper windows coordinates, if the pointer to the parents
// client area contains NULL (default).
/***************************************************************************/
int CPlotHelperWnd::SetMode(int nMode, int nID, CRect * prParent) {
    int nOldMode = m_nMode; // save actual mode
    if (prParent == NULL) {
        prParent = &m_rParent;    // don't change the coordinates
    }
    CRect rWnd(0, 0, 0, 0);
    if ((nMode != m_nMode) || (nID != m_nId) || (*prParent != m_rParent)) {
        m_nMode = nMode;
        m_nId = nID;
        m_rParent = *prParent;
        switch (m_nMode & MODE_MASK) {
        case MODE_TEXT: {
            // load new text to display
            if (nID != -1) {
                m_szText.LoadString(nID);
            }

            CDC dc;
            dc.CreateCompatibleDC(NULL);

            // select the helper window font
            CFont * oldFont = dc.SelectObject(&m_font);
            CSize size = dc.GetTextExtent(m_szText);
            dc.SelectObject(oldFont); // set back old font

            // set new coordinates for window
            rWnd = SetPosition(size.cx + m_nCharWidth, m_nHeight + 4, prParent);
        }
        break;
        default: // default mode is hidden window
            break;
        }
        if (GetSafeHwnd()) {
            MoveWindow(rWnd);
        }
    }
    return nOldMode; // return old mode
}

/***************************************************************************/
// CPlotHelperWnd::OnPaint Painting
/***************************************************************************/
void CPlotHelperWnd::OnPaint() {
    CPaintDC dc(this); // device context for painting
    if (m_nMode != MODE_HIDDEN) {
        // select the helper window font
        CFont * oldFont = dc.SelectObject(&m_font);
        // get client coordinates
        CRect rWnd;
        GetClientRect(rWnd);
        dc.SetBkMode(TRANSPARENT); // letters may overlap, so they must be transparent
        dc.DrawText(m_szText, m_szText.GetLength(), rWnd, DT_VCENTER | DT_CENTER | DT_SINGLELINE | DT_NOCLIP);
        dc.SelectObject(oldFont); // set back old font
    }
}

/***************************************************************************/
// CPlotHelperWnd::OnEraseBkgnd Erasing background
/***************************************************************************/
BOOL CPlotHelperWnd::OnEraseBkgnd(CDC * pDC) {
    if ((m_nMode & MODE_MASK) != MODE_HIDDEN) {
        CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd(); // get pointer to colors from main frame
        Colors * pColors = pMainWnd->GetColors();
        CRect rWnd;

        GetClientRect(rWnd);

        CBrush brush(pColors->cSysBtnFace);
        pDC->FillRect(&rWnd, &brush);
        // paint 3D-frame
        CPen penDkgray(PS_SOLID, 1, pColors->cSysBtnShadow);
        CPen penLtgray(PS_SOLID, 1, pColors->cSysBtnHilite);
        CPen * pOldPen = pDC->SelectObject(&penDkgray);
        if (m_nMode & FRAME_POPIN) {
            // draw popping in frame
            pDC->MoveTo(0, rWnd.bottom);
            pDC->LineTo(0, 0);
            pDC->LineTo(rWnd.right - 1, 0);
            pDC->SelectObject(&penLtgray);
            pDC->LineTo(rWnd.right - 1, rWnd.bottom - 1);
            pDC->LineTo(0, rWnd.bottom - 1);
        }
        if (m_nMode & FRAME_POPOUT) {
            // draw popping out frame
            pDC->MoveTo(rWnd.right - 1, 0);
            pDC->LineTo(rWnd.right - 1, rWnd.bottom - 1);
            pDC->LineTo(0, rWnd.bottom - 1);
            pDC->SelectObject(&penLtgray);
            pDC->LineTo(0, 0);
            pDC->LineTo(rWnd.right - 1, 0);
        }
        pDC->SelectObject(pOldPen);
    }
    return TRUE;
}

/***************************************************************************/
// CPlotHelperWnd::OnLButtonDown Mouse left button down
/***************************************************************************/
void CPlotHelperWnd::OnLButtonDown(UINT nFlags, CPoint point) {
    CWnd * pWnd = GetParent();
    ClientToScreen(&point);
    pWnd->ScreenToClient(&point);
    pWnd->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
}

/***************************************************************************/
// CPlotHelperWnd::OnRButtonDown Mouse right button down
/***************************************************************************/
void CPlotHelperWnd::OnRButtonDown(UINT nFlags, CPoint point) {
    CWnd * pWnd = GetParent();
    ClientToScreen(&point);
    pWnd->ScreenToClient(&point);
    pWnd->SendMessage(WM_RBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
}

bool CPlotHelperWnd::IsCanceled() {
    return (m_nId == IDS_HELPERWND_CANCELED);
}

//###########################################################################
// CPlotWnd
// Base class for all plots in the graph windows. Does all jobs, common to
// plots. It embeds a little child window (helper window) to display special
// information for the user. The class definition and implementation for this
// window are in the same files as CPlotWnd is.


IMPLEMENT_DYNCREATE(CPlotWnd, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CPlotWnd message map

BEGIN_MESSAGE_MAP(CPlotWnd, CWnd)
    ON_WM_PAINT()
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_WM_MOUSEMOVE()
    ON_WM_RBUTTONDOWN()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()

/***************************************************************************/
// CPlotWnd::CPlotWnd Constructor
/***************************************************************************/
CPlotWnd::CPlotWnd() :
m_MousePointerPos(UNDEFINED_OFFSET, UNDEFINED_OFFSET),
m_PopupMenuPos(UNDEFINED_OFFSET, UNDEFINED_OFFSET) {
    m_bRtPlot = false;
    m_bInitialPlot = TRUE;
    m_pStartCursor = NULL;
    m_pStopCursor = NULL;
    m_pLastProcess = NULL;
    m_pAreaProcess = NULL;
    m_bLineDraw = TRUE;
    m_bDotDraw = FALSE;
    m_bCursors = false;
    m_bPrivateCursor = false;
    m_bGrid = TRUE;
    m_fMagnify = 1.0;
    m_dProcessMultiplier = 1.0;
    m_dwHighLightPosition = 0;
    m_dwHighLightLength = 0;
    m_pParent = NULL;
    m_MouseButtonState = 0;
    m_bAnimationPlot = FALSE;
    m_dwAnimationFrame = UNDEFINED_OFFSET;
    SetHorizontalCursors(FALSE);
    SetBold(FALSE);

    m_pBitmapSave = NULL;
}


void CPlotWnd::CopyTo(CPlotWnd * pTarg) {
    // copies don't have any cursors.
    pTarg->m_pStartCursor = NULL;
    pTarg->m_pStopCursor = NULL;

    // copies don't have a process???
    pTarg->m_pAreaProcess = NULL;
    pTarg->m_pLastProcess = NULL;
    pTarg->m_bLineDraw = m_bLineDraw;
    pTarg->m_bDotDraw = m_bDotDraw;
    pTarg->m_bCursors = m_bCursors;
    pTarg->m_bGrid = m_bGrid;
    pTarg->m_fMagnify = m_fMagnify;
    pTarg->m_dProcessMultiplier = m_dProcessMultiplier;
    pTarg->m_szPlotName = m_szPlotName;
    pTarg->m_dwHighLightPosition = 0;
    pTarg->m_dwHighLightLength = 0;
}

/***************************************************************************/
// CPlotWnd::~CPlotWnd Destructor
/***************************************************************************/
CPlotWnd::~CPlotWnd() {

    if (m_pStartCursor) {
        delete m_pStartCursor;
        m_pStartCursor=NULL;
    }
    if (m_pStopCursor) {
        delete m_pStopCursor;
        m_pStopCursor=NULL;
    }

    // turn off any highlighting in raw waveform due to dynamic update mode
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    CSaView * pView = (pMainWnd)?(CSaView *)pMainWnd->GetCurrSaView():NULL;
    if (!pView) {
        return;
    }
    bool bDynamicUpdate = (pView->GetGraphUpdateMode() == DYNAMIC_UPDATE);
    BOOL bAnimationPlot = this->IsAnimationPlot();

    if ((bDynamicUpdate) && (bAnimationPlot)) {
        int nWaveGraphIndex = pView->GetGraphIndexForIDD(IDD_RAWDATA);
        CGraphWnd * pWaveGraph = pView->GetGraph(nWaveGraphIndex);
        if (pWaveGraph) {
            // there is a raw waveform graph
            CPlotWnd * pWavePlot = pWaveGraph->GetPlot();
            if (pWavePlot!=NULL) {
                pWavePlot->ClearHighLightArea();
                pWavePlot->UpdateWindow();
            }
        }
    }
}

/***************************************************************************/
// CPlotWnd::PreCreateWindow Creation
// Called from the framework before the creation of the window. Registers
// the new window class.
/***************************************************************************/
BOOL CPlotWnd::PreCreateWindow(CREATESTRUCT & cs) {
    // register the window class
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, AfxGetApp()->LoadStandardCursor(IDC_ARROW), 0, 0);
    //::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
    return CWnd::PreCreateWindow(cs);
}

/***************************************************************************/
// CPlotWnd::SetMagnify Set the magnify factor
/***************************************************************************/
void CPlotWnd::SetMagnify(double fMagnify, BOOL bRedraw) {
    if (m_fMagnify != fMagnify) {
        m_fMagnify = fMagnify;
        if (bRedraw) {
			// repaint whole plot window
            RedrawPlot();    
        }
    }
}

/***************************************************************************/
// CPlotWnd::SetLineDraw Set line drawing style
/***************************************************************************/
void CPlotWnd::SetLineDraw(BOOL bLine) {
    if (m_bLineDraw != bLine) {
        // style changes
        m_bLineDraw = bLine;
        RedrawPlot(); // repaint whole plot window
    }
}

/***************************************************************************/
// CPlotWnd::SetDotsDraw Set dots drawing style
/***************************************************************************/
void CPlotWnd::SetDotsDraw(BOOL bDots) {
    if (m_bDotDraw != bDots) { // style changes
        m_bDotDraw = bDots;
        RedrawPlot(); // repaint whole plot window
    }
}

/***************************************************************************/
// CPlotWnd::ShowCursors Set cursors visible/hidden
/***************************************************************************/
void CPlotWnd::ShowCursors(bool bPrivate, bool bShow) {
    if (bPrivate) {
        m_bPrivateCursor = bShow;
        if (m_bPrivateCursor) {
            m_bCursors = false;
        }
    } else {
        m_bCursors = bShow;
        if (m_bCursors) {
            m_bPrivateCursor = false;
        }
    }
}

/***************************************************************************/
// CPlotWnd::ShowGrid Show or hide gridlines
/***************************************************************************/
BOOL CPlotWnd::ShowGrid(BOOL bShow, BOOL bRedraw) {
    BOOL bResult = m_bGrid;
    m_bGrid = bShow;
    if (bRedraw) {
        RedrawPlot();    // repaint whole plot window
    }

    return bResult;
}

//**************************************************************************
// 08/30/2000 - DDO Added so this could be overridden.
//**************************************************************************
double CPlotWnd::GetDataPosition(int iWidth) {
    CSaView * pView = (CSaView *)GetParent()->GetParent();
    return pView->GetDataPosition(iWidth);
}

//**************************************************************************
// 08/30/2000 - DDO Added so this could be overridden.
//**************************************************************************
DWORD CPlotWnd::AdjustDataFrame(int iWidth) {
    CSaView * pView = (CSaView *)GetParent()->GetParent();
    return pView->CalcDataFrame(iWidth);
}

/***************************************************************************/
// CPlotWnd::ChangeCursorPosition Change the current cursor position
// This function repositions and resizes the cursor window, pointed to by
// the parameter pWnd. The window is positioned in the parent plot window
// according to the data range, which this window displays and the parameter
// dwNewPosition, which is the new position (in raw data bytes). The flag
// bMove indicates, if the cursor is to move (TRUE) or to place (FALSE). If
// the cursor is to move, its window width has to stay (or to become) 1
// pixel, if it is to place its width has to stay (or become) twice
// CURSOR_WINDOW_HALFWIDTH. If the position of the cursor line (in the middle
// of the cursor window) changes, the old position has to be invalidated and
// redrawn. If the cursor is not visible on the new position (out of the
// plot), its window size will be 0.
/***************************************************************************/
void CPlotWnd::ChangeCursorPosition( CSaView * pView, DWORD dwNewPosition, CCursorWnd * pWnd, bool scroll) {
    
	// get pointer to parent graph
    CGraphWnd * pGraph = (CGraphWnd *)GetParent(); 

    // get window coordinates
    CRect rWnd;
    GetClientRect(rWnd);
    if (rWnd.Width() == 0) {
		// cursor not visible SDM 1.5Test8.5
        return;    
    }

    CRect rNewWnd = rWnd;
    // get actual data position and frame
    double fDataPos = 0;
    DWORD dwDataFrame = 0;
    // check if area graph type
    if (pGraph->IsAreaGraph()) {
        // get necessary data from area plot
        fDataPos = GetAreaPosition();
        dwDataFrame = GetAreaLength(&rWnd);
    } else {
        // get necessary data from view
		// data index of first sample to display
        fDataPos = GetDataPosition(rNewWnd.Width());
		// number of data points to display
        dwDataFrame = AdjustDataFrame(rNewWnd.Width());     
    }

    CRect rNewLine;
	// added by AKE to hide cursors in graph edit mode
    if (((m_bCursors) &&   
         (dwNewPosition >= (DWORD)fDataPos) && 
		 (dwNewPosition < ((DWORD)fDataPos + dwDataFrame)))) {
        // cursor is visible
        ASSERT(rNewWnd.Width());
		// calculate data samples per pixel
        double fBytesPerPix = (double)dwDataFrame / (double)rNewWnd.Width();
        // calculate actual cursor position in pixel
        // SDM 1.06.6U4 calculate position based on pixel aligned graph
        int nPixelPos = (int)round2Int(((double)dwNewPosition - fDataPos) / fBytesPerPix);
        // set the coordinates for the cursor window
        rNewLine.SetRect(nPixelPos, 0, nPixelPos + 1, rNewWnd.bottom);
        //SDM 1.06.5
        // Move mode offers no real advantage
        // we always draw only a line
        rNewWnd.SetRect(nPixelPos - CURSOR_WINDOW_HALFWIDTH, 0, nPixelPos + CURSOR_WINDOW_HALFWIDTH, rNewWnd.bottom);
    } else {
        if (scroll) {
			// data index of first sample to display
            DWORD start = (DWORD)GetDataPosition(rNewWnd.Width());      
			// number of data points to display
			DWORD size = pView->GetDataFrame();
			// margin remaining before we scroll
            DWORD margin = (size*10)/100;
            if (dwNewPosition>margin) {
                DWORD newStart = dwNewPosition-margin;
                TRACE("%d %d %d %d\n",start,size,dwNewPosition,newStart);
                pView->Scroll(newStart);
                pGraph->UpdateWindow();
            } else {
                // cursor is not visible
				// shrink it to 0 size
                rNewLine.SetRect(0, 0, 0, 0); 
                rNewWnd = rNewLine;
            }
        } else {
            // cursor is not visible
			// shrink it to 0 size
            rNewLine.SetRect(0, 0, 0, 0); 
            rNewWnd = rNewLine;
        }
    }

    // get the actual (old) position of cursor window
    CRect rOldLine;
    pWnd->GetWindowRect(rOldLine);
    ScreenToClient(rOldLine);

    // get the line position in the middle
    if (rOldLine.Width() > 1) { 
		// cursor window has large width
        rOldLine.left += CURSOR_WINDOW_HALFWIDTH;
        rOldLine.right -= (CURSOR_WINDOW_HALFWIDTH - 1);
    }
    // check if new cursor line position
    if (rOldLine != rNewLine) {
        // invalidate and update old position
		// SDM 1.5Test10.5
        if (pWnd->IsDrawn()) { 
			// redraw old cursor position
            InvalidateRect(rOldLine, TRUE);    
        }
		// move the cursor window to the new position
        pWnd->MoveWindow(rNewWnd, FALSE); 
		// SDM 1.5Test10.5
        pWnd->SetDrawn(FALSE);  
        //  SDM 1.06.5
        //  By delaying update all changes are complete so the order of changes of cursors
        //  becomes irrelevant to cursor redraw.
        //  Underlying windows will be updated first because of the WS_EX_TRANSPARENT style of cursor windows
        rNewLine.SetRect(rNewLine.left - rNewWnd.left, rNewLine.top, rNewLine.left - rNewWnd.left + 1, rNewLine.bottom);
		// redraw new cursor line
        pWnd->InvalidateRect(rNewLine, TRUE); 
    }
}

/***************************************************************************/
// CPlotWnd::SetStartCursor Position the start cursor
/***************************************************************************/
void CPlotWnd::SetStartCursor(CSaView * pView) {

    DWORD dwStartCursor = pView->GetStartCursorPosition();
    if (m_pStartCursor) {
		ChangeCursorPosition( pView, dwStartCursor, m_pStartCursor);
    }
    bool bDynamicUpdate = (pView->GetGraphUpdateMode() == DYNAMIC_UPDATE);
    if ((bDynamicUpdate) && (m_bAnimationPlot)) {
        // Finish pitch processing if necessary.
        CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
        CProcessGrappl * pAutoPitch = (CProcessGrappl *)pDoc->GetGrappl();
        if (pAutoPitch->IsDataReady()) {
            // Finish fragmenting if necessary.
            CProcessFragments * pFragments = (CProcessFragments *)pDoc->GetFragments();
            if (pFragments->IsDataReady()) {
                DWORD nSmpSize = pDoc->GetSampleSize();
                m_dwAnimationFrame = pFragments->GetFragmentIndex(dwStartCursor/nSmpSize);  // set the animation frame
            }
        }
    }
}

void CPlotWnd::HideCursors() {
    if (m_pStartCursor!=NULL) {
        m_pStartCursor->ShowWindow(SW_HIDE);
    }
    if (m_pStopCursor!=NULL) {
        m_pStopCursor->ShowWindow(SW_HIDE);
    }
}

void CPlotWnd::ShowCursors() {
    if (m_pStartCursor!=NULL) {
        m_pStartCursor->ShowWindow(SW_SHOW);
    }
    if (m_pStopCursor!=NULL) {
        m_pStopCursor->ShowWindow(SW_SHOW);
    }
}

/***************************************************************************/
// CPlotWnd::SetStopCursor Position the stop cursor
/***************************************************************************/
void CPlotWnd::SetStopCursor(CSaView * pView) {
    if (m_pStopCursor) {
        ChangeCursorPosition( pView, pView->GetStopCursorPosition(), m_pStopCursor);
    }
}

/***************************************************************************/
// CPlotWnd::SetPlaybackCursor
/***************************************************************************/
void CPlotWnd::SetPlaybackCursor(CSaView * pView, bool scroll) {
    if (m_PlaybackCursor.IsCreated()) {
        ChangeCursorPosition( pView, pView->GetPlaybackCursorPosition(), &m_PlaybackCursor, scroll);
    }
}

void CPlotWnd::SetPlaybackFlash(bool val) {
    if (m_PlaybackCursor.IsCreated()) {
        m_PlaybackCursor.Flash(val);
    }
}

/***************************************************************************/
// CPlotWnd::MoveStartCursor Move the start cursor
/***************************************************************************/
void CPlotWnd::MoveStartCursor(CSaView * pView, DWORD dwNewPosition) {
	// no cursors visible?
    if (!m_bCursors) return;
    ChangeCursorPosition( pView, dwNewPosition, m_pStartCursor);
}

/***************************************************************************/
// CPlotWnd::MoveStopCursor Move the stop cursor
/***************************************************************************/
void CPlotWnd::MoveStopCursor( CSaView * pView, DWORD dwNewPosition) {
    // no cursors visible
    if (!m_bCursors) return;
    ChangeCursorPosition( pView, dwNewPosition, m_pStopCursor);
}

/***************************************************************************/
// CPlotWnd::SetInitialPrivateCursor Initialize the private cursor window
/***************************************************************************/
void CPlotWnd::SetInitialPrivateCursor() {

    CRect rWnd;
    // get the coordinates of the private cursor
    m_PrivateCursor.GetClientRect(rWnd);
    if (rWnd.Height() == 0) {
        // private cursor has not been initialized yet
        GetClientRect(rWnd);
        // set the cursor window size
        // CLW 10/23/00 if TWC graph, set cursor to middle of window
        if (m_szPlotName == "TWC") {
            CRect rPlotWindow;
            GetWindowRect(rPlotWindow);
            rWnd.top = rPlotWindow.Height() / 2 - 1;
            rWnd.bottom = rWnd.top + 1;
        } else {
            rWnd.left = -CURSOR_WINDOW_HALFWIDTH / 2;
            rWnd.right = rWnd.left + 2 * CURSOR_WINDOW_HALFWIDTH;
        }
        // move the private cursor window
        m_PrivateCursor.MoveWindow(rWnd);
    }
}

/***************************************************************************/
// CPlotWnd::GetStartCursorPosition Returns the position of the start cursor
// The position is returned as pixel coordinate relative to the client area
// of the plot window. The pixel number returned is the center pixel of the
// cursor window. If the cursor is not visible, the function returns -1.
/***************************************************************************/
int CPlotWnd::GetStartCursorPosition() {
    CRect rWnd;
    // get the coordinates of the private cursor
    m_pStartCursor->GetWindowRect(rWnd);
    ScreenToClient(rWnd);
    if (rWnd.Width() <= 0) {
        return -1;
    }
    if (rWnd.Width() == 1) {
        return rWnd.left;
    } else {
        return rWnd.left + CURSOR_WINDOW_HALFWIDTH;
    }
}

/***************************************************************************/
// CPlotWnd::GetStopCursorPosition Returns the position of the stop cursor
// The position is returned as pixel coordinate relative to the client area
// of the plot window. The pixel number returned is the center pixel of the
// cursor window. If the cursor is not visible, the function returns -1.
/***************************************************************************/
int CPlotWnd::GetStopCursorPosition() {
    CRect rWnd;
    // get the coordinates of the private cursor
    m_pStopCursor->GetWindowRect(rWnd);
    ScreenToClient(rWnd);
    if (rWnd.Width() <= 0) {
        return -1;
    }
    if (rWnd.Width() == 1) {
        return rWnd.left;
    } else {
        return rWnd.left + CURSOR_WINDOW_HALFWIDTH;
    }
}

/***************************************************************************/
// CPlotWnd::GetPrivateCursorPosition Returns the position of the private cursor
// The position is returned as pixel coordinate relative to the client area
// of the plot window. The pixel number returned is the center pixel of the
// cursor window. If the cursor is not visible, the function returns -1.
/***************************************************************************/
int CPlotWnd::GetPrivateCursorPosition() {
    CRect rWnd;
    // get the coordinates of the private cursor
    m_PrivateCursor.GetWindowRect(rWnd);
    ScreenToClient(rWnd);

    if (rWnd.Width() <= 0) {
        return -1;
    }
    if (rWnd.Width() == 1) {
        return rWnd.left;
    }
    if (rWnd.Height() <= 0) {
        return -1;
    }
    if (rWnd.Height() == 1) {
        return rWnd.top;
    }
    if (rWnd.Width() < rWnd.Height()) {
        return rWnd.left + CURSOR_WINDOW_HALFWIDTH;
    } else {
        return rWnd.top  + CURSOR_WINDOW_HALFWIDTH;
    }
}

/***************************************************************************/
// CPlotWnd::ScrollPlot Scroll the plot
// Scrolling the plot with the cursors. The cursor windows have to be moved.
// nScrollAmount contains the amount to scroll in pixels. dwOldPos and
// dwOldFrame contains the plot position and width (in bytes of raw data) of
// the plot before scrolling.
/***************************************************************************/
void CPlotWnd::ScrollPlot(CSaView * pView, int nScrollAmount, DWORD dwOldPos, DWORD dwFrame) {
    
	// hide the helper windows (do not scroll them)
    int nOldMode = m_HelperWnd.SetMode(MODE_HIDDEN);
    // now scroll
    ScrollWindow(nScrollAmount, 0);
    UpdateWindow();
    // replace the helper windows
    m_HelperWnd.SetMode(nOldMode);
    // move the cursor windows if necessary
	// position of start cursor
    DWORD dwStartCursorPos = pView->GetStartCursorPosition(); 
	// position of stop cursor
    DWORD dwStopCursorPos = pView->GetStopCursorPosition(); 
    DWORD dwPlaybackCursorPos = pView->GetPlaybackCursorPosition();

    // move cursor windows
    CRect rCursor;
    if ((dwStartCursorPos < dwOldPos) || (dwStartCursorPos >= (dwOldPos + dwFrame))) {
		// start cursor was not visible, move it if it's now visible
        SetStartCursor(pView);    
    }
    if ((dwStopCursorPos < dwOldPos) || (dwStopCursorPos >= (dwOldPos + dwFrame))) {
		// stop cursor was not visible, move it if it's now visible
        SetStopCursor(pView);    
    }
    if ((dwPlaybackCursorPos < dwOldPos) || (dwPlaybackCursorPos >= (dwOldPos + dwFrame))) {
		// stop cursor was not visible, move it if it's now visible
        SetPlaybackCursor(pView, false);    
    }
}

/***************************************************************************/
// CPlotWnd::RedrawPlot Redraw plot including cursors
// If the flag bEntire is TRUE (default), the entire plot will be repainted,
// else only the part between (and with) the cursor windows are repainted,
// but only if boundaries are displayed.
/***************************************************************************/
void CPlotWnd::RedrawPlot( BOOL bEntire) {
    // kg - in case we don't have a window yet..
    if (IsWindow(m_hWnd) == FALSE) {
        return;
    }
    CRect rWnd;
    GetClientRect(rWnd);
    if (bEntire) {
        // invalidate entire plot window
        InvalidateRect(NULL);
    } else {
		if ((m_pParent->HasBoundaries()) && (m_bCursors)) {
            // boundaries displayed?
            // invalidate region between (and with) cursor windows
            CRect rStart;
			CRect rStop;
            m_pStartCursor->GetWindowRect(rStart);
            m_pStopCursor->GetWindowRect(rStop);
			// union of both rectangles
            rWnd.UnionRect(rStart, rStop); 
            ScreenToClient(rWnd);
			// invalidate this area
            InvalidateRect(rWnd); 
        }
    }
}

/***************************************************************************/
// CPlotWnd::CheckResult Check the processing result
// The result of data processing can be an error or a canceled state. In the
// second case, the helper window will be activated with the message about
// the canceled state. In case of processing error, the graph will be closed.
//**************************************************************************/
short int CPlotWnd::CheckResult(short int nResult, CProcess * pProcess, bool clearPlot) {
    // save pointer to process object for further use
    m_pLastProcess = pProcess; 
    if (!this->GetSafeHwnd()) {
        return nResult;
    }
    CRect rClient;
    GetClientRect(rClient);
    CDC * pDC = GetDC();
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    Colors * pColor = pMainWnd->GetColors(); // get application colors
    CBrush backgroundBrush(pColor->cPlotBkg);
    switch (nResult) {
    case PROCESS_CANCELED:
        TRACE("canceled\n");
        // process has been canceled, display helper window
        if (clearPlot) {
            // clear the plot area
            pDC->FillRect(&rClient, &backgroundBrush);
        }
        m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_CANCELED, &rClient);
        break;
    case PROCESS_NO_DATA:
        TRACE("nodata\n");
        if (clearPlot) {
            // no data to process
            // clear the plot area
            pDC->FillRect(&rClient, &backgroundBrush);
        }
        m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_NOVOICING, &rClient);
        break;
    case PROCESS_DATA_OVERLOAD:
        // too much data to process
        m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_CURCLOSER, &rClient);
        break;
    case PROCESS_UNVOICED: {
        // process data is unvoiced
        CGraphWnd * pGraph = (CGraphWnd *)GetParent();
        CSaView * pView = (CSaView *)pGraph->GetParent();
        bool bDynamicUpdate = (pView->GetGraphUpdateMode() == DYNAMIC_UPDATE);
        if (!bDynamicUpdate) {
            m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_SELECTVOICED, &rClient);
        } else {
            m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_UNVOICED, &rClient);
        }
        break;
    }
    case PROCESS_ERROR: {
        // processing error
        CSaApp * pApp = (CSaApp *)AfxGetApp();
        pApp->ErrorMessage(IDS_ERROR_PROCESS);
        GetParent()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
        break;
    }
    }
    ReleaseDC(pDC);
    return nResult;
}

/***************************************************************************/
// CPlotWnd::PlotPrePaintDots
//**************************************************************************/
BOOL CPlotWnd::PlotPrePaintDots(CDC * pDC, int nTop, CRect rClip,
                                CLegendWnd * pLegend, CXScaleWnd * pXScale,
                                COLORREF cColor, int nStyle) {
    double dXFirstGridPos = pXScale->GetFirstGridPosition();
    double dYFirstGridPos = pLegend->GetFirstGridPosition();
    double fXDistance = pXScale->GetGridDistance();
    double fYDistance = pLegend->GetGridDistance();

    if (fXDistance == 0. || fXDistance == std::numeric_limits<double>::infinity()) {
        return TRUE;
    }

    if (fYDistance == 0. || fYDistance == std::numeric_limits<double>::infinity()) {
        return TRUE;
    }

    int nXCount = (int) ceil((rClip.left - 2 - dXFirstGridPos)/fXDistance);
    double dXPos = dXFirstGridPos + nXCount * fXDistance;
    if (pLegend->GetScaleMode() & LOG10) {
        // logarithmic scale
        double dScaleMinValue = pLegend->GetScaleMinValue();
        double fBase = pLegend->GetScaleBase();
        while (dXPos <= rClip.right + 2) {
            int nXPos = round2Int(dXPos);

            int nLogDisp = (int)ceil(dScaleMinValue / pow(10, floor(fBase)));
            double fBasePos = (double)nTop + dYFirstGridPos + (fBase - floor(fBase)) * fYDistance;
            int nYPos = round2Int(fBasePos - log10((double)nLogDisp) * fYDistance);
            while (nYPos > rClip.top) {
                // set pixel
                pDC->SetPixel(nXPos, nYPos, cColor);
                if (nStyle == 3) {
                    pDC->SetPixel(nXPos, (nYPos + 1), cColor);
                    pDC->SetPixel((nXPos + 1), nYPos, cColor);
                    pDC->SetPixel(nXPos, (nYPos - 1), cColor);
                    pDC->SetPixel((nXPos - 1), nYPos, cColor);
                }
                if (nLogDisp > 9) {
                    nLogDisp = 0;
                    fBasePos -= fYDistance;
                }
                nLogDisp++;
                nYPos = round2Int(fBasePos - log10((double)nLogDisp) * fYDistance);
            }
            dXPos = dXFirstGridPos + ++nXCount * fXDistance;
        }
    } else {
        // linear scale
        while (dXPos <= rClip.right + 2) {
            int nXPos = round2Int(dXPos);
            double dYPos = dYFirstGridPos;
            while (dYPos < rClip.bottom) {
                int nYPos = round2Int(dYPos);
                // set pixel
                pDC->SetPixel(nXPos, nYPos, cColor);
                if (nStyle == 3) {
                    pDC->SetPixel(nXPos, (nYPos + 1), cColor);
                    pDC->SetPixel((nXPos + 1), nYPos, cColor);
                    pDC->SetPixel(nXPos, (nYPos - 1), cColor);
                    pDC->SetPixel((nXPos - 1), nYPos, cColor);
                }
                dYPos += fYDistance;
            }
            dXPos += fXDistance;
        }
    }
    return TRUE;
}

/***************************************************************************/
// CPlotWnd::PlotPrePaint Do paint common parts before data has been drawn
// This function should be called at the beginning of every derived plots
// OnPaint function before having drawn its own painting job. It paints the
// gridlines.
//**************************************************************************/
void CPlotWnd::PlotPrePaint(CDC * pDC, CRect rWnd, CRect rClip, CLegendWnd * pLegend, bool bShowCursors, bool bPrivateCursor) {
    // get pointer to graph, grid structure, legend- and x-scale window
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    const CGrid cGrid = GetGrid();
    const CGrid * pGrid = &cGrid;
    if (!pLegend) {
        pLegend = pGraph->GetLegendWnd();
    }
    CXScaleWnd * pXScale = pGraph->GetXScaleWnd();
    // calculate the scales
    pLegend->CalculateScale(NULL, &rWnd);
    pXScale->CalculateScale(NULL, rWnd.Width());

    if (!HaveGrid()) {
        return;    // no grid to draw
    }

    ShowCursors(bPrivateCursor, bShowCursors);

    // prepare to get color from main frame
    Colors * pColors = pMainWnd->GetColors();
    // set background color for dashed or dotted lines
    pDC->SetBkColor(pColors->cPlotBkg);
    static const unsigned char bit[8] =
    {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
    if (pGrid->bXGrid) {
        // draw x -grid
        if (pGrid->nXStyle) {
            // line style
            int nPenStyle = PS_SOLID;
            unsigned char nPenPattern=0xff;
            switch (pGrid->nXStyle) {
            case 2:
                // nPenStyle = PS_DASH;
                nPenPattern = 0xBB;/* 10111011 */
                break;
            case 3:
                //nPenStyle = PS_DOT;
                nPenPattern = 0x55;/* 01010101 */
                break;
            case 4:
                //nPenStyle = PS_DASHDOT;
                nPenPattern = 0xD7;/* 11010111 */
                break;
            case 5:
                //nPenStyle = PS_DASHDOTDOT;
                nPenPattern = 0xAB;/* 10101011 */
                break;
            default:
                ;
            } // switch (pGrid->nXStyle)

            if ((pGrid->nXStyle != 3)) {
                // create and select pen
                CPen pen(nPenStyle, 1, pColors->cPlotGrid);
                CPen * pOldPen = pDC->SelectObject(&pen);
                double fStartPos = pXScale->GetFirstGridPosition();
                double fDistance = pXScale->GetGridDistance();
                double fGridPos = (double)pLegend->GetFirstGridPosition();
                double fGridDistance = pLegend->GetGridDistance()/8.;
                if (fDistance > 0.) {
                    int nCount = (int) ceil((rClip.left - 2 - fStartPos)/fDistance);
                    double dPos = fStartPos + nCount * fDistance;

                    while (dPos <= rClip.right + 2) {
                        // draw a line
                        int Index = 0;
                        int nPos = round2Int(dPos);
                        for (double y=fGridPos-fGridDistance*8.5; y < rWnd.bottom;) {
                            if (bit[Index]&nPenPattern) {
                                pDC->MoveTo(nPos, (int)y);
                                pDC->LineTo(nPos, (int)(y + fGridDistance));
                            }
                            Index = (Index+1)%8;
                            y += fGridDistance;
                        }
                        dPos = fStartPos + ++nCount * fDistance;
                    }
                }
                pDC->SelectObject(pOldPen);
            }
        }
        if ((!pGrid->nXStyle) || (pGrid->nXStyle == 3)) {
            // dot style (1 pixel) or "- - - - - - - - - - -" style (2x2 pixels)
            PlotPrePaintDots(pDC, rWnd.top, rClip, pLegend, pXScale,
                             pColors->cPlotGrid, pGrid->nXStyle);
        }
    }

    if (pGrid->bYGrid) {
        // line style
        int nPenStyle = PS_SOLID;
        unsigned char nPenPattern=0xff;
        // draw y -grid
        if (pGrid->nYStyle) {
            // line style
            nPenStyle = PS_SOLID;
            nPenPattern = 0xff;;
            switch (pGrid->nYStyle) {
            case 2:
                // nPenStyle = PS_DASH;
                nPenPattern = 0xBB;/* 10111011 */
                break;
            case 3:
                //nPenStyle = PS_DOT;
                nPenPattern = 0x55;/* 01010101 */
                break;
            case 4:
                //nPenStyle = PS_DASHDOT;
                nPenPattern = 0xD7;/* 11010111 */
                break;
            case 5:
                //nPenStyle = PS_DASHDOTDOT;
                nPenPattern = 0xAB;/* 10101011 */
                break;
            default:
                ;
            }
        }

        if (((pGrid->nYStyle) && (pGrid->nYStyle != 3))) {
            // create and select pen
            CPen pen(nPenStyle, 1, pColors->cPlotGrid);
            CPen * pOldPen = pDC->SelectObject(&pen);
            double fGridPos = pXScale->GetFirstGridPosition();
            double fGridDistance = pXScale->GetGridDistance()/8.;
            if (pLegend->GetScaleMode() & LOG10) {
                // logarithmic scale
                double fYStartPos = (double)pLegend->GetFirstGridPosition();
                double fDistance = pLegend->GetGridDistance();
                double dScaleMinValue = pLegend->GetScaleMinValue();
                double fBase = pLegend->GetScaleBase();
                int nLogDisp = (int)ceil(dScaleMinValue / pow(10, floor(fBase)));
                double fBasePos = (double)rWnd.top + fYStartPos + (fBase - floor(fBase)) * fDistance;
                int nPos = (int)round2Int(fBasePos - log10((double)nLogDisp) * fDistance);
                while (nPos > rClip.top) {
                    // draw a grid line
                    int Index = 0;
                    for (double x = fGridPos-fGridDistance*8.5; x<rWnd.right;) {
                        if (bit[Index]&nPenPattern) {
                            pDC->MoveTo((int)x, nPos);
                            pDC->LineTo((int)(x + fGridDistance), nPos);
                        }
                        Index = (Index+1)%8;
                        x += fGridDistance;
                    }
                    if (nLogDisp > 9) {
                        nLogDisp = 0;
                        fBasePos -= fDistance;
                    }
                    nLogDisp++;
                    nPos = (int)round2Int(fBasePos - log10((double)nLogDisp) * fDistance);
                }
            } else {
                // linear scale
                double dStartPos = pLegend->GetFirstGridPosition();
                int nCount = 0;
                double fDistance = pLegend->GetGridDistance();
                double dPos = dStartPos;
                while (dPos < rClip.bottom) {
                    // draw a grid line
                    int Index = 0;
                    int nPos = round2Int(dPos);
                    for (double x = fGridPos-fGridDistance*8.5; x<rWnd.right;) {
                        if (bit[Index]&nPenPattern) {
                            pDC->MoveTo((int)round2Int(x), nPos);
                            pDC->LineTo((int)round2Int(x + fGridDistance), nPos);
                        }
                        Index = (Index+1)%8;
                        x += fGridDistance;
                    }
                    dPos = dStartPos + ++nCount * fDistance;
                }
            }
            pDC->SelectObject(pOldPen);
        } else if ((!pGrid->nYStyle) || (pGrid->nYStyle == 3)) {
            // dot style (1 pixel) or "- - - - - - - - - - -" style (2x2 pixels)
            PlotPrePaintDots(pDC, rWnd.top, rClip, pLegend, pXScale,
                             pColors->cPlotGrid, pGrid->nYStyle);
        }
    }
}

void DrawData(CRect & rClip, CDataSource & dataSource, const CXScale & cXScale, const CYScale & cYScale, CDrawSegment & drawSegment) {

    int nFirst = (int) floor(cXScale.GetSample(rClip.left)) - 1;
    int nLast = (int) ceil(cXScale.GetSample(rClip.right)) + 1;
    int x = 0;

    BOOL bValid = TRUE;

    CDataSource::SValues values;

    int nSample = nFirst > 0 ? nFirst : 0;

    int nPasses = 0;
    while (nSample <= nLast) {
        x = cXScale.GetX(nSample);
        int nNext = (int) floor(cXScale.GetSample(x+1));
        dataSource.GetValues(nSample, nNext, values, bValid);
        drawSegment.DrawTo(x, values, cYScale, bValid);
        nPasses++;
    }
    // Flush drawing functions
    bValid = FALSE;
    drawSegment.DrawTo(x,0, cYScale, bValid);
}


/***************************************************************************/
// CPlotWnd::PlotStandardPaint Standard plot paint job
// This function does a standard painting with 16 bit data from the delivered
// process class. The data to paint is coming from a temporary file, created
// by the process class. With the parameter nFlags the painting algorithm
// can be influenced. The flags are defined in the class definition of this
// class. The two data parameters are responsible for the data range, that
// will be used to display the data. They must not be influenced by the
// magnify factor and not divided by the precision multiplier, this will be
// done here.
//**************************************************************************/
void CPlotWnd::PlotStandardPaint(CDC * pDC, CRect rWnd, CRect rClip, CProcess * pProcess, CSaDoc * pProcessDoc, int nFlags) {
    if (rClip.IsRectEmpty()) {
        return;
    }

    // get pointer to graph, view, document mainframe and legend window
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    CLegendWnd * pLegend = pGraph->GetLegendWnd();

    // get pointer to color structure from main frame
    Colors * pColor = pMainWnd->GetColors();

    CXScale * pXScale = NULL;
    CXScale * pXScaleRaw = NULL;
    // Create pXScale
    {
        CSaView * pView = (CSaView *)pGraph->GetParent();
        CSaDoc * pHostDoc = (CSaDoc *)pView->GetDocument();
        CSaDoc * pDoc = pProcessDoc;

        // calculate size factor between raw data and process data

        double fSizeFactor = (double)pDoc->GetSampleSize() * ceil((double)(pDoc->GetDataSize()/pDoc->GetSampleSize())/(double)(pProcess->GetDataSize()));
        //TRACE(L"plot %s -----------\n",(LPCTSTR)m_szPlotName);
        //TRACE("doc sample size %d\n",pDoc->GetSampleSize());
        //TRACE("doc data size %d\n",pDoc->GetDataSize());
        //TRACE("proc data size %d\n",pProcess->GetDataSize());
        //TRACE("size factor %f\n",fSizeFactor);

        // get necessary data from document and from view
        double fDataPos = GetDataPosition(rWnd.Width()); 
		// number of data points to display
        DWORD dwDataFrame = AdjustDataFrame(rWnd.Width()); 
        if (dwDataFrame == 0) {
			// nothing to draw
            return;    
        }

        // calculate raw data samples per pixel
        double fBytesPerPix = double(dwDataFrame)*pDoc->GetAvgBytesPerSec()/pHostDoc->GetAvgBytesPerSec()/(double)rWnd.Width();
        //TRACE("bytes per pix %f\n",fBytesPerPix);
        //TRACE("----\n");

        pXScale = new CXScaleLinear(fBytesPerPix/fSizeFactor, fDataPos/fSizeFactor);
        pXScaleRaw = new CXScaleLinear(fBytesPerPix, fDataPos);
    }

    CYScale * pYScale = NULL;
    // Create YScale
    double fMultiplier = GetProcessMultiplier();
	// logarithmic/linear scale
    BOOL bLog10 = (BOOL)(pLegend->GetScaleMode() & LOG10); 

    double dCenterPos;
    double fBase = pLegend->GetScaleBase();
    if (bLog10) {
        // logarithmic scale
        dCenterPos = pLegend->GetFirstGridPosition();
        fBase = pLegend->GetScaleBase() + log10(fMultiplier);
        m_fVScale = pLegend->GetGridDistance();

        pYScale = new CYScaleLog(-m_fVScale, dCenterPos + fBase*m_fVScale);
    } else { // linear scale
        double dScaleMin = pLegend->GetScaleMinValue();
        if (dScaleMin < 0.) {
			// this is dumb... legend should report the actual
            dScaleMin /= GetMagnify();    
        }

        if (nFlags & PAINT_DB) {
            // calculate vertical scaling factor
            fBase = pLegend->GetScaleBase();
            m_fVScale = pLegend->GetGridDistance() / fBase;
			// x-axis vertical position
            dCenterPos = rWnd.bottom +  dScaleMin*m_fVScale; 
			// loudness represents rms value so peak is +3dB and we want another 3db headroom
            double dblDbReference = -20*log10(double(0x7fff)) + 6; 

            pYScale = new CYScaleDB(-m_fVScale, dCenterPos, dblDbReference, 20.);
        } else if (nFlags & PAINT_SEMITONES) {
            // calculate vertical scaling factor
            fBase = pLegend->GetScaleBase();
            m_fVScale = pLegend->GetGridDistance() / fBase;
			// x-axis vertical position
            dCenterPos = rWnd.bottom +  dScaleMin*m_fVScale; 

            pYScale = new CYScaleSemitones(-m_fVScale, dCenterPos, 10);
        } else {
            // calculate vertical scaling factor
            fBase = pLegend->GetScaleBase() * fMultiplier;
            m_fVScale = pLegend->GetGridDistance() / fBase;
			// x-axis vertical position
            dCenterPos = rWnd.bottom +  dScaleMin*m_fVScale*fMultiplier; 

            pYScale = new CYScaleLinear(-m_fVScale, dCenterPos);

            // draw bottom line
            CPen penAxes(PS_SOLID, 1, pColor->cPlotAxes);
            CPen * pOldPen = pDC->SelectObject(&penAxes);
			// draw the line
            pDC->MoveTo(rWnd.left, round2Int(dCenterPos)); 
            pDC->LineTo(rWnd.right, round2Int(dCenterPos));
            pDC->SelectObject(pOldPen);
        }
    }

    CDataSource * pSource = NULL;
    switch (nFlags & (SKIP_UNSET | SKIP_MISSING)) {
    case 0:
        pSource = new CDataSourceSimple(*pProcess);
        break;
    default:
        pSource = new CDataSourceValidate(*pProcess, nFlags & SKIP_UNSET, nFlags & SKIP_MISSING);
    }


    CDrawSegment * pSegment = NULL;
    // Create pSegment
    if (nFlags & PAINT_CROSSES) {
        // paint small crosses for each data point
        pSegment = new CDrawSegmentSample(*pDC);
    } else if (nFlags & PAINT_DOTS) {
        // paint dots (only) for each data point
        pSegment = new CDrawSegmentDotOnly(*pDC);
    } else if (m_bLineDraw) {
        // Smooth line drawing
        pSegment = new CDrawSegmentLine(*pDC);
    } else {
        // Paint Solid
        pSegment = new CDrawSegmentSolid(*pDC, nFlags & (PAINT_DB|PAINT_SEMITONES) ? rWnd.bottom : round2Int(dCenterPos));
    }
	// Sometimes Based on VScale
    int nLineThickness = GetPenThickness(); 

    CPen penData(PS_SOLID, nLineThickness, pColor->cPlotData[0]);
    CPen penHiData(PS_SOLID, nLineThickness, pColor->cPlotHiData);
    CPen * pOldPen = pDC->SelectObject(&penData);

    CRect rClipPortion(rClip);

    try {
        int nXStart = m_dwHighLightLength ? pXScaleRaw->GetX(m_dwHighLightPosition) : 0;
        int nXEnd = m_dwHighLightLength ? pXScaleRaw->GetX(m_dwHighLightPosition + m_dwHighLightLength) : 0;

        CRgn cClipRgn;
        cClipRgn.CreateRectRgnIndirect(rClip);

        GetClipRgn(pDC->m_hDC, cClipRgn);


        if (nXStart > rClip.left) {
            rClipPortion.left=rClip.left;
            rClipPortion.right=nXStart;

            CRgn clipPortionRgn;
            clipPortionRgn.CreateRectRgnIndirect(rClipPortion);

            pDC->SelectObject(&penData);
            pSegment->SetColor(pColor->cPlotData[0]);

            rClipPortion.InflateRect(nLineThickness+1,0);
            DrawData(rClipPortion, *pSource, *pXScale, *pYScale, *pSegment);

            ExtSelectClipRgn(pDC->m_hDC, cClipRgn, RGN_COPY);
        }
        if (nXStart < rClip.right && nXEnd >= rClip.left) {
            pDC->SelectObject(&penData);
            rClipPortion.left = nXStart;
            rClipPortion.right = nXEnd;

            CRgn clipPortionRgn;
            clipPortionRgn.CreateRectRgnIndirect(rClipPortion);

            pDC->SelectObject(&penHiData);
            pSegment->SetColor(pColor->cPlotHiData);

            rClipPortion.InflateRect(nLineThickness+1,0);
            DrawData(rClipPortion, *pSource, *pXScale, *pYScale, *pSegment);

            ExtSelectClipRgn(pDC->m_hDC, cClipRgn, RGN_COPY);
        }
        if (nXEnd < rClip.right) {
            rClipPortion.left = nXEnd;
            rClipPortion.right = rClip.right;

            CRgn clipPortionRgn;
            clipPortionRgn.CreateRectRgnIndirect(rClipPortion);

            pDC->SelectObject(&penData);
            pSegment->SetColor(pColor->cPlotData[0]);

            CRect tmp;
            //pDC->GetClipBox(tmp);
            tmp = rWnd;
            tmp.DeflateRect(2,2);

            CBrush brush(pColor->cSysBtnFace);
            pDC->FrameRect(tmp, &brush);

            rClipPortion.InflateRect(nLineThickness+1,0);
            DrawData(rClipPortion, *pSource, *pXScale, *pYScale, *pSegment);

            ExtSelectClipRgn(pDC->m_hDC, cClipRgn, RGN_COPY);
        }

    } catch (...) {
    }

    pDC->SelectObject(pOldPen);

    // Applying delete to NULL has no effect
    delete pSource;
    delete pXScale;
    delete pXScaleRaw;
    delete pYScale;
    delete pSegment;
}

/***************************************************************************/
// CPlotWnd::PlotPaintFinish Finish paint job painting common plot parts
// This function should be called at the end of every derived plots OnPaint
// function after having finished its own painting job. It paints the
// cursors and the segment boundaries.
//**************************************************************************/
void CPlotWnd::PlotPaintFinish( CDC * pDC, CRect rWnd, CRect rClip) {
    
	// get pointer to graph, view and document
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaView * pView = (CSaView *)pGraph->GetParent();
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();

	if (m_pParent->HasBoundaries()) {
        // paint boundaries
        double fDataStart;
        DWORD dwDataFrame;
        // check if area graph type
        if (pGraph->IsAreaGraph()) {
            // get necessary data from area plot
            fDataStart = GetAreaPosition();
            dwDataFrame = GetAreaLength(&rWnd);
        } else {
            // get necessary data from document and from view
			// data index of first sample to display
            fDataStart = GetDataPosition(rWnd.Width());
			// number of data points to display
            dwDataFrame = AdjustDataFrame(rWnd.Width());    
        }
        if ((dwDataFrame>0) && (rWnd.Width()>0)) {
            // calculate the number of data samples per pixel
            double fBytesPerPix = (double)dwDataFrame / (double)rWnd.Width();
            // get pointer to phonetic string
            CString pPhonetic = pDoc->GetSegment(PHONETIC)->GetContent();
            if (!pPhonetic.IsEmpty()) { 
				// string is not empty
                // get pointer to phonetic offset and duration arrays
                CSegment * pOffsets = pDoc->GetSegment(PHONETIC);
                // position prepare loop. Find first boundary to display in clipping rect
                double fStart = fDataStart + ((double)rClip.left) * fBytesPerPix;
                int nLoop = 0;
                if (fStart > 0) {
                    for (nLoop = 1; nLoop < pPhonetic.GetLength(); nLoop++) {
                        if (pOffsets->GetStop(nLoop)> fStart) {
							// this is it
                            break;    
                        }
                    }
                }
                // first char must be at lower position
                if (nLoop > 0) {
                    nLoop = pDoc->GetSegment(PHONETIC)->GetPrevious(nLoop);
                }
                // check for overlap and make correction (draw previous character boundaries too)
                if ((nLoop > 0) && (pOffsets->GetOffset(nLoop) < (pOffsets->GetStop(nLoop - 1)))) {
                    nLoop--;
                }
                // display loop, create pen and brush
				// get color from main frame
                CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd(); 
                CPen penBoundary(PS_SOLID, 1, pMainWnd->GetColors()->cPlotBoundaries);
                CPen * pOldPen = pDC->SelectObject(&penBoundary);
                CBrush brushBoundary(pMainWnd->GetColors()->cPlotBoundaries);
                CBrush * pOldBrush = (CBrush *)pDC->SelectObject(&brushBoundary);
                // define points for the boundary arrows
                POINT points[3];
                points[0].y = points[1].y = 0;
                points[2].y = 4;
				// position in pixels to display boundary
                int nDisplayPos = 0; 
                do {
                    // calculate start boundary
                    nDisplayPos = round2Int((pOffsets->GetOffset(nLoop) - fDataStart)/fBytesPerPix);
                    // calculate stop boundary
                    int nDurationPos = round2Int((pOffsets->GetStop(nLoop) - fDataStart)/fBytesPerPix);
                    // next string start
                    nLoop = pDoc->GetSegment(PHONETIC)->GetNext(nLoop);
                    // draw the lines
                    pDC->MoveTo(nDisplayPos, 0); 
					// paint start line
                    pDC->LineTo(nDisplayPos, rWnd.bottom);
                    points[0].x = nDisplayPos + 4; 
					// draw start arrow
                    points[1].x = points[2].x = nDisplayPos;
                    pDC->Polygon(points, 3);
                    pDC->MoveTo(nDurationPos, 0); 
					// paint stop line
                    pDC->LineTo(nDurationPos, rWnd.bottom);
                    points[0].x = nDurationPos - 4; 
					// draw stop arrow
                    points[1].x = points[2].x = nDurationPos;
                    pDC->Polygon(points, 3);
                } while ((nDisplayPos < rClip.right) && (nLoop >= 0));
                pDC->SelectObject(pOldPen); 
				// select back old pen
                pDC->SelectObject(pOldBrush);
            }
        }
    }

    // paint cursors
	// whole graph drawn
    if (rClip.Width() >= rWnd.Width()) { 
        if (m_bCursors) {
            SetStopCursor(pView);
            SetStartCursor(pView);
            SetPlaybackCursor(pView,false);
        }
    }

    // if this graph has focus, update the status bar.   AKE 10/21/2000
    if (m_bInitialPlot && pView->GetFocusedGraphWnd()) {
		// update the status bar
        pGraph->UpdateStatusBar(pView->GetStartCursorPosition(), pView->GetStopCursorPosition(), TRUE); 
        m_bInitialPlot = FALSE;
    }
}

/***************************************************************************/
// CPlotWnd::IsCanceled Return cancel state
//**************************************************************************/
BOOL CPlotWnd::IsCanceled() {
    return (m_pLastProcess) ? m_pLastProcess->IsCanceled() : FALSE;
}

/***************************************************************************/
// CPlotWnd::RestartProcess Restart a canceled process or recalc an area graph
//**************************************************************************/
void CPlotWnd::RestartProcess() {
    m_HelperWnd.SetMode(MODE_HIDDEN);   // hide helper window
    if (IsAreaGraph()) {
        m_pAreaProcess->UpdateArea();
        CGraphWnd * pGraph = (CGraphWnd *)GetParent();
        pGraph->RedrawGraph(TRUE, TRUE); // repaint whole graph
    }

    if (m_pLastProcess && IsCanceled()) {
        m_pLastProcess->RestartProcess();
        RedrawPlot(); // repaint whole plot
    }
}

/***************************************************************************/
// CPlotWnd::GetAreaPosition Return area position of area graph
// If the current area length is 0, the function returns the data position
// of the view. In case of an error 0 is returned.
//**************************************************************************/
DWORD CPlotWnd::GetAreaPosition() {
    if (m_pAreaProcess) {
        CProcessAreaData * pProcess = (CProcessAreaData *)m_pAreaProcess; // cast pointer
        if (pProcess->GetAreaLength() == 0) {
            // area length is zero, so return the views position
            CRect rWnd(0,0,0,0);
            if (GetSafeHwnd()) {
                GetClientRect(rWnd);
            }
            return (DWORD)GetDataPosition(rWnd.Width());
        }
        return pProcess->GetAreaPosition();
    }
    return 0;
}

/***************************************************************************/
// CPlotWnd::GetAreaLength Return area length of area graph
// If the current area length is 0, the function returns the frame length of
// the view. In case of an error 0 is returned.
//**************************************************************************/
DWORD CPlotWnd::GetAreaLength(CRect * pRwnd) {
    if (m_pAreaProcess) {
        CProcessAreaData * pProcess = (CProcessAreaData *)m_pAreaProcess; // cast pointer
        DWORD dwFrame = pProcess->GetAreaLength();
        if (dwFrame == 0) {
            // area length is zero, so return the views length
            CRect rWnd;
            if (pRwnd) {
                rWnd = *pRwnd;
            } else {
                GetClientRect(rWnd);
            }
            return AdjustDataFrame(rWnd.Width());
        }
        return dwFrame;
    }
    return 0;
}

void CPlotWnd::ClearHighLightArea() {
	SetHighLightArea( 0, 0, TRUE, FALSE);
}

/***************************************************************************
* CPlotWnd::SetHighLightArea 
* Sets a highlighted area
* dwStart and dwStop -  contain the area that has to be highlighted (in raw data bytes). 
* They always will be snapped to the next zero crossing unless they are at the 
* beginning or end of the raw data. 
* bRedraw - If the flag bRedraw is TRUE (default), the area that has to change its 
* colors will be invalidated. It takes areas only for plots with visible and non 
* private cursors.
* Start and Stop are not range checked. 
* Selected segments will be deselected.
***************************************************************************/
void CPlotWnd::SetHighLightArea(DWORD dwStart, DWORD dwStop, BOOL bRedraw, BOOL bSecondSelection) {
    
	if (!m_bCursors) return;

    CSaView * pView = (CSaView *)GetParent()->GetParent();
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
    DWORD nSampleSize = pDoc->GetSampleSize();
    if (nSampleSize == 2) {
        // positions have to be even for 16 bit
        dwStart &= ~1;
        dwStop &= ~1;
    }
    if (dwStart > 0) {
        dwStart = pDoc->SnapCursor(START_CURSOR, dwStart, 0, pDoc->GetDataSize() - nSampleSize);
    }
    if ((dwStop > 0) && (dwStop < (pDoc->GetDataSize() - nSampleSize))) {
        dwStop = pDoc->SnapCursor(STOP_CURSOR, dwStop, 0, pDoc->GetDataSize() - nSampleSize);
    }

    if ((m_dwHighLightPosition == dwStart) &&
        (m_dwHighLightLength == (dwStop - dwStart))) {
        return;
    }

    if (bRedraw) {
        // calculate the actual and the new highlighted rectangles
        CRect rWnd;
        GetClientRect(rWnd);
		// data index of first sample to display
        double fDataPos = GetDataPosition(rWnd.Width());
		// number of data points to display
        DWORD dwDataFrame = AdjustDataFrame(rWnd.Width());  
        ASSERT(rWnd.Width());
        double fBytesPerPix = (double)dwDataFrame / (double)rWnd.Width();

        int nHighLightPixLeft = 0;
        int nHighLightPixRight = 0;
        if ((dwDataFrame != 0) && (m_dwHighLightLength != 0)) {
            // there is actually something to highlight
            if ((m_dwHighLightPosition + m_dwHighLightLength) > fDataPos) {
                if (m_dwHighLightPosition < fDataPos) {
                    m_dwHighLightLength -= ((DWORD)fDataPos - m_dwHighLightPosition);
                    m_dwHighLightPosition = (DWORD)fDataPos;
                }
                // SDM 1.06.6U4 align selection to graph aligned to pixels
                nHighLightPixLeft = round2Int(((double)m_dwHighLightPosition - fDataPos) / fBytesPerPix);
                nHighLightPixRight = round2Int(((double)(m_dwHighLightPosition + m_dwHighLightLength) - fDataPos) / fBytesPerPix);
            }
        }
        // this is the actual highlighted rectangle in the plot
        CRect rOldHi(nHighLightPixLeft - 1, rWnd.top, nHighLightPixRight + 1, rWnd.bottom);
        nHighLightPixLeft = round2Int(((double)dwStart - fDataPos) / fBytesPerPix);
        nHighLightPixRight = round2Int(((double)dwStop - fDataPos) / fBytesPerPix);
        // this is the new highlighted rectangle in the plot
        CRect rNewHi(nHighLightPixLeft - 1, rWnd.top, nHighLightPixRight + 1, rWnd.bottom);

        // now find the difference
        rWnd.UnionRect(rOldHi, rNewHi);

        m_dwHighLightPosition = dwStart;
        m_dwHighLightLength = dwStop - dwStart;

        rWnd.InflateRect(2,0);
        InvalidateRect(rWnd,TRUE);
    } else {
        m_dwHighLightPosition = dwStart;
        m_dwHighLightLength = dwStop - dwStart;
    }

    if ((m_dwHighLightLength>0) && (!bSecondSelection)) {
        // deselect segment, if one selected
        CSegment * pSegment = pView->FindSelectedAnnotation();
        if (pSegment!=NULL) {
            pView->ChangeAnnotationSelection(pSegment, pSegment->GetSelection(), 0, 0);
        }
    }
}

/***************************************************************************/
// CPlotWnd::OnCreate Window creation
/***************************************************************************/
int CPlotWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }

    m_pStartCursor = new CStartCursorWnd();
    m_pStopCursor = new CStopCursorWnd();
    CRect rWnd(0, 0, 0, 0);
    // create the start cursor
    if (!m_pStartCursor->Create(NULL, _T("Cursor"), WS_CHILD | WS_VISIBLE, rWnd, this, 0)) {
        delete m_pStartCursor;
        m_pStartCursor = NULL;
    }
    // create the stop cursor
    if (!m_pStopCursor->Create(NULL, _T("Cursor"), WS_CHILD | WS_VISIBLE, rWnd, this, 0)) {
        delete m_pStopCursor;
        m_pStopCursor = NULL;
    }
    // create the private cursor
    m_PrivateCursor.Create(NULL, _T("Cursor"), WS_CHILD | WS_VISIBLE, rWnd, this, 0);
    // create the playback cursor
    m_PlaybackCursor.Create(NULL, _T("Cursor"), WS_CHILD | WS_VISIBLE, rWnd, this, 0);

    // create the helper window
    m_HelperWnd.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, rWnd, this, 1);

    return 0;
}

/***************************************************************************/
// CPlotWnd::OnSize Sizing the window
// Before the resizing of a plot window, the cursor windows are set to size
// 0, so that after the redraw of the plot they will be redrawn too.
/***************************************************************************/
void CPlotWnd::OnSize(UINT nType, int cx, int cy) {
    m_pStartCursor->MoveWindow(0, 0, 0, 0);
    m_pStartCursor->ResetPosition();
    m_pStopCursor->MoveWindow(0, 0, 0, 0);
    m_pStopCursor->ResetPosition();
    m_PrivateCursor.MoveWindow(0, 0, 0, 0);
    m_PrivateCursor.ResetPosition();
    CWnd::OnSize(nType, cx, cy);
}

/***************************************************************************/
// CPlotWnd::EraseBkgnd Erasing background        10/24/2000 - DDO
/***************************************************************************/
BOOL CPlotWnd::OnEraseBkgnd(CDC * pDC) {
    return EraseBkgnd(pDC);
}
/***************************************************************************/
// CPlotWnd::OnEraseBkgnd Erasing background
// Allows highlighting of selected areas.
/***************************************************************************/
BOOL CPlotWnd::EraseBkgnd(CDC * pDC) {

    CRect rWnd;
    GetClientRect(rWnd);
    if (rWnd.Width()==0) {
		// nothing to erase
        return TRUE;    
    }

	// data index of first sample to display
    double fDataPos = GetDataPosition(rWnd.Width());
	// number of data points to display
    DWORD dwDataFrame = AdjustDataFrame(rWnd.Width()); 
    ASSERT(rWnd.Width());
    double fBytesPerPix = (double)dwDataFrame / (double)rWnd.Width();

    int nHighLightPixLeft = 0;
    int nHighLightPixRight = 0;
    if ((dwDataFrame != 0) && (m_dwHighLightLength != 0)) {
        // there is something to highlight
        if ((m_dwHighLightPosition + m_dwHighLightLength) > fDataPos) {
            DWORD dwHighLightPosition = m_dwHighLightPosition;
            DWORD dwHighLightLength = m_dwHighLightLength;
            if (m_dwHighLightPosition < fDataPos) {
                dwHighLightPosition = (DWORD)fDataPos;
                dwHighLightLength -= ((DWORD)fDataPos - m_dwHighLightPosition);
            }
            // SDM 1.06.6U4 align selection to graph aligned to pixels
            nHighLightPixLeft = round2Int(((double)dwHighLightPosition - fDataPos) / fBytesPerPix);
            nHighLightPixRight = round2Int(((double)(dwHighLightPosition + dwHighLightLength) - fDataPos) / fBytesPerPix);
        }
    }
    // get background color from main frame
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    // create the background brush
    CBrush backBrush(pMainWnd->GetColors()->cPlotBkg);
    CBrush * pOldBrush = pDC->SelectObject(&backBrush);
    CRect rClip;
	// get the area needed to redraw
    pDC->GetClipBox(rClip); 
    pDC->LPtoDP(rClip);
    if (rClip.left < nHighLightPixLeft) {
        // draw left of highlighted area
        int nWidth = nHighLightPixLeft - rClip.left;
        if (nWidth > rClip.Width()) {
            nWidth = rClip.Width();
        }
        pDC->PatBlt(rClip.left, rClip.top, nWidth, rClip.Height(), PATCOPY);
        rClip.left += nWidth;
    }
    if ((rClip.left < rClip.right) && 
		(rClip.left < nHighLightPixRight) && 
		(nHighLightPixRight - nHighLightPixLeft)) {
        // draw highlighted area
        int nWidth = nHighLightPixRight - rClip.left;
        if (nWidth > rClip.Width()) {
            nWidth = rClip.Width();
        }
        COLORREF cHighlight = pMainWnd->GetColors()->cPlotHiBkg;

        CBrush highBrush(cHighlight);
        pDC->SelectObject(&highBrush);
        pDC->PatBlt(rClip.left, rClip.top, nWidth, rClip.Height(), PATCOPY);
        pDC->SelectObject(&backBrush);
        rClip.left += nWidth;
    }
    if (rClip.left < rClip.right) {
        // draw right of highlighted area
        pDC->PatBlt(rClip.left, rClip.top, rClip.Width(), rClip.Height(), PATCOPY);
    }
    pDC->SelectObject(pOldBrush);
    return TRUE;
}

/***************************************************************************/
// CPlotWnd::OnRButtonDown Mouse right button down
// This event initiates a popup menu. The graph also has to get focus, so
// the parent graph is called to do this.
/***************************************************************************/
void CPlotWnd::OnRButtonDown(UINT nFlags, CPoint point) {

	TRACE("OnRButtonDown %d,%d\n",point.x,point.y);
    // inform parent graph
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
	// send message to parent
    pGraph->SendMessage(WM_RBUTTONDOWN, nFlags, MAKELONG(point.x, point.y)); 

	CMainFrame * pMainWnd = (CMainFrame*)AfxGetMainWnd();

	// handle the floating popup menu
    CMenu menu;
    if (menu.LoadMenu(pMainWnd->GetPopup())) { 
        // Show restricted submenu according to EXPERIMENTAL_ACCESS
        CMenu & popup = EXPERIMENTAL_ACCESS ? *menu.GetSubMenu(3) : *menu.GetSubMenu(0);
        ASSERT(popup.m_hMenu != NULL);
        // attach the layout menu
        CLayoutMenu layout;
		// don't change the string
        TCHAR szString[256]; 
        if (popup.GetMenuString(ID_GRAPHS_LAYOUT, szString, sizeof(szString)/sizeof(TCHAR), MF_BYCOMMAND)) {
            BOOL bResult = popup.ModifyMenu(ID_GRAPHS_LAYOUT, MF_BYCOMMAND | MF_POPUP, (UINT)layout.m_hMenu, szString);
			ASSERT(bResult);
        }
		pGraph->SetPopupMenuLocation(point);
        // pop the menu up
        ClientToScreen(&point);
        popup.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,  pMainWnd);
    }
    CWnd::OnRButtonDown(nFlags, point);
}

/***************************************************************************/
// CPlotWnd::OnLButtonDown Mouse left button down
// Sets the start or stop cursor (or both) cursor to the mouse pointer
// position (if possible) in the plot. The graph also has to gain focus, so
// the parent graph has to be informed.
/***************************************************************************/
void CPlotWnd::OnLButtonDown(UINT nFlags, CPoint point) {
    
	// inform parent graph
    CGraphWnd * pWnd = (CGraphWnd *)GetParent();
	// send message to parent
    pWnd->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y)); 
    // get client coordinates
    CRect rWnd;
    GetClientRect(rWnd);
    // get pointer to view
    CSaView * pView = (CSaView *)pWnd->GetParent();
    if (pView->GetDocument()->GetDataSize() > 0) {
        if (m_bCursors) {
            // if shift key not pressed -> start cursor is to move
            if (!(nFlags & MK_SHIFT)) {
                // position and drag the start cursor
                ClientToScreen(&point);
                m_pStartCursor->ScreenToClient(&point);
                m_pStartCursor->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
            } else { // stop cursor is to move
                // position and drag the stop cursor
                ClientToScreen(&point);
                m_pStopCursor->ScreenToClient(&point);
                m_pStopCursor->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
            }
        }

        if (m_bPrivateCursor) {
            if (m_bHorizontalCursors) {
                // THIS CODE IS FOR HORIZONTAL CURSORS - TCJ 5/9/00
                //  see also CPrivateCursorWnd for 2 additional horizontal cursor functions
                // get the actual (old) position of cursor window
                CRect rLine;
                m_PrivateCursor.GetWindowRect(rLine);
                ScreenToClient(rLine);
                // get the line position in the middle
                rLine.top += CURSOR_WINDOW_HALFWIDTH;
                rLine.bottom -= (CURSOR_WINDOW_HALFWIDTH - 1);
                // move the private cursor window
				// redraw old cursor position
                InvalidateRect(rLine, TRUE); 
                rWnd.top = point.y - CURSOR_WINDOW_HALFWIDTH;
                rWnd.bottom = point.y + CURSOR_WINDOW_HALFWIDTH;
				// move the cursor window to the new position
                m_PrivateCursor.MoveWindow(rWnd, FALSE); 
				// update this region before redrawing the cursor window
                UpdateWindow(); 
                rLine.SetRect(rWnd.left, CURSOR_WINDOW_HALFWIDTH, rWnd.right, CURSOR_WINDOW_HALFWIDTH + 1);

				// redraw new cursor line
                m_PrivateCursor.InvalidateRect(rLine, TRUE); 
				// update the cursor
                m_PrivateCursor.UpdateWindow(); 
                // and drag the cursor
                m_PrivateCursor.SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(0, CURSOR_WINDOW_HALFWIDTH));
            } else {
                // THIS CODE IS FOR VERTICAL CURSORS
                // get the actual (old) position of cursor window
                CRect rLine;
                m_PrivateCursor.GetWindowRect(rLine);
                ScreenToClient(rLine);
                // get the line position in the middle
                rLine.left += CURSOR_WINDOW_HALFWIDTH;
                rLine.right -= (CURSOR_WINDOW_HALFWIDTH - 1);
                // move the private cursor window
				// redraw old cursor position
                InvalidateRect(rLine, TRUE); 
                rWnd.left = point.x - CURSOR_WINDOW_HALFWIDTH;
                rWnd.right = point.x + CURSOR_WINDOW_HALFWIDTH;
                rLine.SetRect(CURSOR_WINDOW_HALFWIDTH, rWnd.top, CURSOR_WINDOW_HALFWIDTH + 1, rWnd.bottom);
				// redraw new cursor line
                m_PrivateCursor.InvalidateRect(rLine, TRUE); 
				// move the cursor window to the new position
                m_PrivateCursor.MoveWindow(rWnd, FALSE); 
				// update this region before redrawing the cursor window
                UpdateWindow(); 
                // and drag the cursor
                m_PrivateCursor.SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(CURSOR_WINDOW_HALFWIDTH, 0));
            }
        }
    }
    CWnd::OnLButtonDown(nFlags, point);
}

/***************************************************************************/
// CPlotWnd::OnMouseMove
/***************************************************************************/
void CPlotWnd::OnMouseMove(UINT nFlags, CPoint point) {

    m_MousePointerPos = point;
    m_MouseButtonState = nFlags;
    //TRACE("x/y %d %d\n",point.x,point.y);
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaView * pView = (CSaView *)pGraph->GetParent();

    // update the status bar
    const BOOL bForceUpdate = TRUE;
    pGraph->UpdateStatusBar(pView->GetStartCursorPosition(), pView->GetStopCursorPosition(), bForceUpdate);

    // continue with message handler
    CWnd::OnMouseMove(nFlags, point);
}

/***************************************************************************/
// CPlotWnd::OnPaint Painting
// General paint procedure used by all CPlotWnd subclasses
// OnDraw is virtual and implemented in each subclass.
// OnDraw is reused by both OnPaint and OnPrint.
/***************************************************************************/
void CPlotWnd::OnPaint() {
    CPaintDC dc(this);              // device context for painting
    CRect rWnd, rClip;
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaView * pView = (CSaView *)pGraph->GetParent();

    GetClientRect(rWnd);
    dc.GetClipBox(&rClip);
    dc.LPtoDP(&rClip);
    OnDraw(&dc,rWnd,rClip, pView);  // virtual

}

CPlotWnd * CPlotWnd::NewCopy(void) {
    return NULL;
}

CString CPlotWnd::GetPlotName() const {
    return m_szPlotName;
}

void CPlotWnd::SetPlotName(const CString & plotName) {
    m_szPlotName = plotName;
}

/***************************************************************************/
// CPlotWnd::StandardAnimateFrame  Animate plot for a single frame
/***************************************************************************/
void CPlotWnd::StandardAnimateFrame(DWORD dwFrameIndex) {
    m_dwAnimationFrame = dwFrameIndex;
    // Get pointer to raw waveform graph
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaView * pView = (CSaView *)pGraph->GetParent();
    int nWaveGraphIndex = pView->GetGraphIndexForIDD(IDD_RAWDATA);
    CGraphWnd * pWaveGraph = pView->GetGraph(nWaveGraphIndex);
    // Highlight raw data frame for which process data is calculated
    if (pWaveGraph) {
        // raw waveform graph present
        CSaDoc * pDoc = pView->GetDocument();
        DWORD wSmpSize = pDoc->GetSampleSize();
        CProcessFragments * pFragments = pDoc->GetFragments();
        SFragParms FragParms = pFragments->GetFragmentParms(m_dwAnimationFrame);
        DWORD dwFrameStart = FragParms.dwOffset * wSmpSize;
        DWORD dwFrameSize = (DWORD)FragParms.wLength * (DWORD)wSmpSize;
        CPlotWnd * pWavePlot = pWaveGraph->GetPlot();
        pWavePlot->SetHighLightArea(dwFrameStart, dwFrameStart + dwFrameSize, TRUE, TRUE);
        pWavePlot->UpdateWindow();
    }
    // Redraw plot
    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}


/***************************************************************************/
// CPlotWnd::StandardEndAnimation  Terminate animation and turn off frame
// highlighting in the raw waveform plot.
/***************************************************************************/
void CPlotWnd::StandardEndAnimation() {
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaView * pView = (CSaView *)pGraph->GetParent();
    DWORD dwStartCursor = pView->GetStartCursorPosition();
    CSaDoc * pDoc = pView->GetDocument();
    DWORD wSmpSize = pDoc->GetSampleSize();
    CProcessFragments * pFragments = pDoc->GetFragments();
    m_dwAnimationFrame = pFragments->GetFragmentIndex(dwStartCursor/wSmpSize);  // reset to start cursor fragment
    int nWaveGraphIndex = pView->GetGraphIndexForIDD(IDD_RAWDATA);
    CGraphWnd * pWaveGraph = pView->GetGraph(nWaveGraphIndex);
    if (pWaveGraph) {
        CPlotWnd * pWavePlot = pWaveGraph->GetPlot();
        pWavePlot->ClearHighLightArea();
    }
    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}


/***************************************************************************/
// CPlotWnd::GraphHasFocus  Handles plot when its graph gains or loses focus
/***************************************************************************/
void CPlotWnd::GraphHasFocus(BOOL bFocus) {
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    CSaView * pView = (CSaView *)pMainWnd->GetCurrSaView();
    if (!pView) {
        return;
    }
    if (m_bAnimationPlot) {
        int nWaveGraphIndex = pView->GetGraphIndexForIDD(IDD_RAWDATA);
        CGraphWnd * pWaveGraph = pView->GetGraph(nWaveGraphIndex);
        if (pWaveGraph) {
            bool bDynamicUpdate = (pView->GetGraphUpdateMode() == DYNAMIC_UPDATE);
            if (bDynamicUpdate) {
                CPlotWnd * pWavePlot = pWaveGraph->GetPlot();
                if (bFocus) {
                    // Highlight raw data frame for which formants are calculated
                    CSaDoc * pDoc   = pView->GetDocument();
                    WORD wSmpSize = (WORD)(pDoc->GetSampleSize());  // calculate sample size in bytes
                    CProcessFragments * pFragments = (CProcessFragments *)pDoc->GetFragments(); // data should be ready -- dynamic mode enabled
                    DWORD dwFrame = m_dwAnimationFrame;
                    if (dwFrame == UNDEFINED_OFFSET) {
                        dwFrame = pFragments->GetFragmentIndex(pView->GetStartCursorPosition()/wSmpSize);
                    }
                    SFragParms FragParms = pFragments->GetFragmentParms(dwFrame);
                    DWORD dwFrameStart = FragParms.dwOffset * wSmpSize;
                    DWORD dwFrameSize = (DWORD)FragParms.wLength * (DWORD)wSmpSize;
                    pWavePlot->SetHighLightArea(dwFrameStart, dwFrameStart + dwFrameSize, TRUE, TRUE);
                    pWavePlot->UpdateWindow();
                } else if (!pView->IsAnimating()) {
                    // turn off highlighted area in raw data
                    pWavePlot->ClearHighLightArea();
                    pWavePlot->UpdateWindow();
                }
            }
        }
    }
}


/***************************************************************************/
// CPlotWnd::OnSysCommand System menu command
/***************************************************************************/
void CPlotWnd::OnSysCommand(UINT nID, LPARAM lParam) {
    switch (nID) {
    case SC_KEYMENU:
        GetParent()->SendMessage(WM_SYSCOMMAND,nID,lParam);
        return;
    case SC_CLOSE:
        GetParent()->SendMessage(WM_SYSCOMMAND,nID,lParam);
        break;
    case SC_MAXIMIZE:
    case SC_MINIMIZE:
    case SC_RESTORE:
    default:
        break;
    }
    CWnd::OnSysCommand(nID, lParam);
}


void CPlotWnd::RemoveRtPlots() {
    // do nothing.
}

bool CPlotWnd::IsRtOverlay() {
    return m_bRtPlot;
}

void CPlotWnd::SetRtOverlay() {
    m_bRtPlot = true;
}

/***************************************************************************/
// CPlotWnd::CalcWaveOffsetAtPixel  Calculate wave offset from pixel position
/***************************************************************************/
DWORD CPlotWnd::CalcWaveOffsetAtPixel(CPoint pixel) {
    // get actual data position, frame and data size and alignment
    double fDataPos;
    DWORD dwDataFrame;
    // check if area graph type
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaView * pView = (CSaView *)pGraph->GetParent();
    // get client area width
    CRect rWnd;
    GetClientRect(rWnd);
    int nWidth = rWnd.Width();
    if (pGraph->IsAreaGraph()) {
        // get necessary data from area plot
        fDataPos = GetAreaPosition();
        dwDataFrame = GetAreaLength();
    } else {
        // get necessary data from document and from view
		// data index of first sample to display
        fDataPos = pView->GetDataPosition(nWidth); 
		// number of data points to display
        dwDataFrame = pView->CalcDataFrame(nWidth); 
    }
    CSaDoc * pDoc = pView->GetDocument();
    DWORD nSmpSize = pDoc->GetSampleSize();
    // calculate data samples per pixel
    double fSamplesPerPix = nWidth ? (double)dwDataFrame / (double)(nWidth*nSmpSize) : 0.;

    // calculate the start cursor position
    DWORD dwWaveOffset = (DWORD)(nSmpSize*round2Int(fDataPos/nSmpSize + pixel.x * fSamplesPerPix));
    return dwWaveOffset;
}

CGrid CPlotWnd::GetGrid() const {
    return *static_cast<CMainFrame *>(AfxGetMainWnd())->GetGrid();
}

CDataSourceSimple::CDataSourceSimple(CProcess & cProcess) : m_cProcess(cProcess) {
    m_nSamples = m_cProcess.GetDataSize();
}

void CDataSourceSimple::GetValues(int & nFirstSample, int nLastSample, SValues & values, BOOL & bValid) {

    bValid = TRUE;

    if (nFirstSample < 0) {
        nFirstSample = 0;
    }
    if ((nLastSample - nFirstSample)*sizeof(short int) >= m_cProcess.GetProcessBufferSize()) {
        nLastSample = nFirstSample + m_cProcess.GetProcessBufferSize()/(4*sizeof(short int));
    }

    int nMax = INT_MIN;
    int nMin = INT_MAX;

    int nFirstIndexOffset = (nFirstSample < m_nSamples ? nFirstSample : m_nSamples - 1);
    int nLastIndex = (nLastSample < m_nSamples ? nLastSample : m_nSamples - 1) - nFirstIndexOffset;

    short int * pData = reinterpret_cast<short *>(m_cProcess.GetProcessedDataBlock(nFirstIndexOffset*sizeof(short int), (nLastIndex+1)*sizeof(short int)));
	if (pData!=NULL) {
		for (int nSampleIndex = 0; nSampleIndex <= nLastIndex; nSampleIndex++) {
			int nLast = pData[nSampleIndex];
			if (nLast > nMax) {
				nMax = nLast;
			}
			if (nLast < nMin) {
				nMin = nLast;
			}
		}
	}

    nFirstSample = nLastSample + 1;
    values.nMax = nMax;
    values.nMin = nMin;
	if (pData!=NULL) {
		values.nLast = pData[nLastIndex];
		values.nFirst = pData[0];
	} else {
		values.nLast = 0;
		values.nFirst = 0;
	}
}

CDataSourceValidate::CDataSourceValidate(CProcess & cProcess, BOOL bUnset, BOOL bMissing) :
    m_cProcess(cProcess), m_bSkipUnset(bUnset), m_bSkipMissing(bMissing) {
    m_nSamples = m_cProcess.GetDataSize();
}

void CDataSourceValidate::GetValues(int & nFirstSample, int nLastSample, SValues & values, BOOL & bValid) {
    bValid = TRUE;

    if (nFirstSample < 0) {
        nFirstSample = 0;
    }
    if ((nLastSample - nFirstSample)*sizeof(short int) >= m_cProcess.GetProcessBufferSize()) {
        nLastSample = nFirstSample + m_cProcess.GetProcessBufferSize()/(4*sizeof(short int));
    }

    int nMax = INT_MIN;
    int nMin = INT_MAX;

    int nFirstIndexOffset = (nFirstSample < m_nSamples ? nFirstSample : m_nSamples - 1);
    int nLastIndex = (nLastSample < m_nSamples ? nLastSample : m_nSamples - 1) - nFirstIndexOffset;

    short int * pData = reinterpret_cast<short *>(m_cProcess.GetProcessedDataBlock(nFirstIndexOffset*sizeof(short int), (nLastIndex+1)*sizeof(short int)));

    for (int nSampleIndex = 0; nSampleIndex <= nLastIndex; nSampleIndex++) {
        int nTemp = pData[nSampleIndex];
        if ((m_bSkipUnset && nTemp == -1) || (m_bSkipMissing && nTemp == -2)) {
            if (nSampleIndex) {
                // Everything up to this point is valid
                nLastSample = nFirstSample + nSampleIndex - 1;
            } else {
                bValid = FALSE;

                while ((nSampleIndex <= nLastIndex) && ((m_bSkipUnset && nTemp == -1) || (m_bSkipMissing && nTemp == -2))) {
                    nTemp = pData[++nSampleIndex];
                }

                // Everything up to this point is invalid
                nLastSample = nFirstSample + nSampleIndex - 1;
            }
            nLastIndex = 0; // make sure last is set correctly
            break;
        }

        if (nTemp > nMax) {
            nMax = nTemp;
        }
        if (nTemp < nMin) {
            nMin = nTemp;
        }
    }

    nFirstSample = nLastSample + 1;
    values.nMax = nMax;
    values.nMin = nMin;
    values.nLast = pData[nLastIndex];
    values.nFirst = pData[0];
}

const double CYScaleSemitones::dSemitoneScale = 12.0 / log10(2.0);
const double CYScaleSemitones::dSemitoneReference =  + (69. - log10(440.0)* 12.0 / log10(2.0));

void CDrawSegment::DrawTo(int x, CDataSource::SValues & values, const CYScale & cYScale, BOOL bValid) {
    if (bValid && values.nMax != values.nMin) {
        DrawTo(x, values.nFirst, cYScale, bValid);
        DrawTo(x, values.nMax, cYScale, bValid);
        DrawTo(x, values.nMin, cYScale, bValid);
        DrawTo(x, values.nLast, cYScale, bValid);
    } else {
        DrawTo(x, values.nMax, cYScale, bValid);
    }
}

void CDrawSegment::DrawTo(CDataSource::SValues & values, const CXScale & cXScale, int y, BOOL bValid) {
    if (bValid && values.nMax != values.nMin) {
        DrawTo(values.nFirst, cXScale, y, bValid);
        DrawTo(values.nMax, cXScale, y, bValid);
        DrawTo(values.nMin, cXScale, y, bValid);
        DrawTo(values.nLast, cXScale, y, bValid);
    } else {
        DrawTo(values.nMax, cXScale, y, bValid);
    }
}

void CDrawSegmentSolid::DrawTo(int x, int value, const CYScale & cYScale, BOOL bValid) {
    if (x==m_nCurrentX) {
        if (bValid) {
            if (value > m_nMaxValue) {
                m_nMaxValue = value;
            }
            if (value < m_nMinValue) {
                m_nMinValue = value;
            }
        }
    } else {
        // new x value draw old
        if (m_nOldX != m_nCurrentX && m_nMinValue <= m_nMaxValue) {
            CRect cRect;
            cRect.left = (m_nOldX + m_nCurrentX + 1)/2;
            cRect.right = (m_nCurrentX + x + 1)/2;

            cRect.top = cYScale.GetY(m_nMaxValue);
            cRect.bottom = cYScale.GetY(m_nMinValue);

            cRect.NormalizeRect();

            if (cRect.top > m_nReferenceY) {
                cRect.top = m_nReferenceY;
            }

            if (cRect.bottom < m_nReferenceY) {
                cRect.bottom = m_nReferenceY;
            }

            m_cDC.FillSolidRect(cRect, m_clr);
        }
        if (bValid) {
            m_nMinValue = m_nMaxValue = value;
        } else {
            m_nMinValue = INT_MAX;
            m_nMaxValue = INT_MIN;
        }
        m_nOldX = m_nCurrentX;
        m_nCurrentX = x;
    }
}

void CDrawSegmentLine::DrawTo(int x, int value, const CYScale & cYScale, BOOL bValid) {
    if (bValid) {
        CPoint point(x,cYScale.GetY(value));
        m_pPoints[m_nSize++] = point;

        if (m_nSize >= 100) {
            Flush();
            m_pPoints[m_nSize++] = point;
        }
    } else {
        Flush();
    }
}

void CDrawSegmentLine::DrawTo(int x, const CXScale & cXScale, int y, BOOL bValid) {
    if (bValid) {
        CPoint point(cXScale.GetX(x),y);
        m_pPoints[m_nSize++] = point;

        if (m_nSize >= 100) {
            Flush();
            m_pPoints[m_nSize++] = point;
        }
    } else {
        Flush();
    }
}

void CDrawSegmentLine::Flush() {
    if (m_nSize >= 2) {
        Polyline(m_cDC.m_hDC, m_pPoints, m_nSize);
    }

    m_nSize = 0;
}

void CDrawSegmentSample::DrawTo(int x, CDataSource::SValues & values, const CYScale & cYScale, BOOL bValid) {
    if (bValid && values.nMax != values.nMin) {
        DrawTo(x, values.nMax, cYScale, bValid);
        DrawTo(x, values.nMin, cYScale, bValid);
    } else {
        DrawTo(x, values.nMax, cYScale, bValid);
    }
}

void CDrawSegmentSample::DrawTo(int x, int value, const CYScale & cYScale, BOOL bValid) {
    if (bValid) {
        int y = cYScale.GetY(value);
        CYScaleLinear cIdentity(1,0);
        CDrawSegmentLine::DrawTo(x-1,y, cIdentity , bValid);
        CDrawSegmentLine::DrawTo(x+2,y, cIdentity , bValid);
        CDrawSegmentLine::DrawTo(x,y, cIdentity , FALSE);
        CDrawSegmentLine::DrawTo(x,y+1, cIdentity , bValid);
        CDrawSegmentLine::DrawTo(x,y-2, cIdentity , bValid);
        CDrawSegmentLine::DrawTo(x,y, cIdentity , FALSE);
    }
}

void CDrawSegmentDotOnly::DrawTo(int x, CDataSource::SValues & values, const CYScale & cYScale, BOOL bValid) {
    if (bValid && values.nMax != values.nMin) {
        DrawTo(x, values.nMax, cYScale, bValid);
        DrawTo(x, values.nMin, cYScale, bValid);
    } else {
        DrawTo(x, values.nMax, cYScale, bValid);
    }
}

void CDrawSegmentDotOnly::DrawTo(int x, int value, const CYScale & cYScale, BOOL bValid) {
    if (bValid) {
        int y = cYScale.GetY(value);
        CYScaleLinear cIdentity(1,0);
        CDrawSegmentLine::DrawTo(x,y, cIdentity , bValid);
        CDrawSegmentLine::DrawTo(x+1,y, cIdentity , bValid);
        CDrawSegmentLine::DrawTo(x,y, cIdentity , FALSE);
    }
}

void CPlotWnd::SetParent(CGraphWnd * setParent) {
    m_pParent = setParent;
}

CGraphWnd * CPlotWnd::GetParent() {
    return m_pParent;
}

CPoint CPlotWnd::GetMousePointerPosition() {
    return m_MousePointerPos;
}

UINT CPlotWnd::GetMouseButtonState() {
    return m_MouseButtonState;
}

void CPlotWnd::SetMousePointerPosition(CPoint point) {
    m_MousePointerPos = point;
}

void CPlotWnd::SetMouseButtonState(UINT state) {
    m_MouseButtonState = state;
}

void CPlotWnd::PostNcDestroy() {
    delete this;
}

// return magnify factor
double CPlotWnd::GetMagnify() {
    return m_fMagnify;
}

// return drawing style
BOOL CPlotWnd::HaveDrawingStyleLine() {
    return m_bLineDraw;
}

// return drawing style
BOOL CPlotWnd::HaveDrawingStyleDots() {
    return m_bDotDraw;
}

bool CPlotWnd::HasCursors() {
    return m_bCursors;
}

bool CPlotWnd::HasPrivateCursor() {
    // private cursor visible?
    return m_bPrivateCursor;
}

BOOL CPlotWnd::HaveGrid() {
    // gridlines visible?
    return m_bGrid;
}

CStartCursorWnd * CPlotWnd::GetStartCursorWnd() {
    // return pointer to start cursor window
    return m_pStartCursor;
}

CStopCursorWnd * CPlotWnd::GetStopCursorWnd() {
    // return pointer to start cursor window
    return m_pStopCursor;
}

void CPlotWnd::OnDraw(CDC * /*pDC*/, CRect /*rWnd*/, CRect /*rClip*/, CSaView * /*pView*/) {
}

DWORD CPlotWnd::GetHighLightPosition() {
    // return highlight area position
    return m_dwHighLightPosition;
}

DWORD CPlotWnd::GetHighLightLength() {
    // return highlight area length
    return m_dwHighLightLength;
}

void CPlotWnd::GenderInfoChanged(int /*nGender*/) {
}

BOOL CPlotWnd::IsAnimationPlot() {
    // TRUE = plot can be animated (defaults to FALSE in constructor)
    return m_bAnimationPlot;
}

void CPlotWnd::SetAnimationFrame(DWORD dwFrameIndex) {
    // set animation frame to fragment index
    m_dwAnimationFrame = dwFrameIndex;
}

void CPlotWnd::AnimateFrame(DWORD /*dwFrameIndex*/) {
}

// animate a single frame (fragment)
void CPlotWnd::EndAnimation() {
}

// terminate animation and return to resting state
BOOL CPlotWnd::IsAreaGraph() const {
    return m_pAreaProcess != NULL;
}

void CPlotWnd::SetHorizontalCursors(BOOL bValue) {
    m_bHorizontalCursors = bValue;
}

int CPlotWnd::GetPenThickness() const {
    return m_bBold ? 2 : 1;
}

void CPlotWnd::SetBold(BOOL bValue) {
    m_bBold = bValue;
}

BOOL CPlotWnd::GetBold() const {
    return m_bBold;
}

double CPlotWnd::GetProcessMultiplier() const {
    return m_dProcessMultiplier;
}

double CPlotWnd::SetProcessMultiplier(double dScale) {
    double old = m_dProcessMultiplier;
    m_dProcessMultiplier = dScale;
    return old;
}

CDataSource::~CDataSource() {
}

CDataSourceSimple::~CDataSourceSimple() {
}

CDataSourceValidate::~CDataSourceValidate() {
}

void CPlotWnd::ChangePrivateCursorPosition(CPoint point) {
    m_PrivateCursor.ChangeCursorPosition(point);
}

void CPlotWnd::GetPrivateCursorWindowRect(CRect * rect) {
    m_PrivateCursor.GetWindowRect(rect);
}

void CPlotWnd::OnPrivateCursorDraw(CDC * pDC, CRect rect) {
    m_PrivateCursor.OnDraw(pDC, rect);
}


