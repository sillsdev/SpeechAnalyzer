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
#include "Process\sa_proc.h"
#include "Process\sa_p_lou.h"
#include "Process\sa_p_gra.h"
#include "Process\sa_p_fra.h"
#include "sa_minic.h"
#include "sa_graph.h"
#include "sa_segm.h"
#include "dsp\mathx.h"

#include "sa_doc.h"
#include "sa.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "math.h"
#include "string.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//###########################################################################
// CPlotHelperWnd
// Helper window to display additional user information. It displays various
// information on different positions in the parents client area depending
// on the set display mode.

IMPLEMENT_DYNCREATE(CPlotHelperWnd, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CPlotHelperWnd message map

BEGIN_MESSAGE_MAP(CPlotHelperWnd, CWnd)
//{{AFX_MSG_MAP(CPlotHelperWnd)
ON_WM_PAINT()
ON_WM_ERASEBKGND()
ON_WM_LBUTTONDOWN()
ON_WM_RBUTTONDOWN()
ON_WM_CREATE()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

extern CSaApp NEAR theApp;
/////////////////////////////////////////////////////////////////////////////
// CPlotHelperWnd construction/destruction/creation

/***************************************************************************/
// CPlotHelperWnd::CPlotHelperWnd Constructor
/***************************************************************************/
CPlotHelperWnd::CPlotHelperWnd()
{
  m_nMode = 0; // default mode = hidden window
  m_szText = "";
  m_rParent.SetRect(0, 0, 0, 0);
  m_nHeight = 0;
  m_nCharWidth = 0;
}

/***************************************************************************/
// CPlotHelperWnd::CPlotHelperWnd Destructor
/***************************************************************************/
CPlotHelperWnd::~CPlotHelperWnd()
{
}

/***************************************************************************/
// CPlotHelperWnd::OnCreate Window creation
/***************************************************************************/
int CPlotHelperWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CWnd::OnCreate(lpCreateStruct) == -1) return -1;
  // create the helper window font (same font as status bar)
  const TCHAR* pszName = _T("MS Sans Serif");
  m_font.CreateFont(-10, 0, 0, 0, FW_NORMAL, 0, 0, 0,
    ANSI_CHARSET, OUT_DEFAULT_PRECIS, (CLIP_LH_ANGLES | CLIP_STROKE_PRECIS),
    DEFAULT_QUALITY, FF_SWISS, pszName);
  // get text metrics
  TEXTMETRIC tm;
  CDC* pDC = GetDC();
  // select the helper window font
  CFont* oldFont = pDC->SelectObject(&m_font);
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
CRect CPlotHelperWnd::SetPosition(int nWidth, int nHeight, CRect* prParent)
{
  // set default (left top)
  CRect rHelper(prParent->left, prParent->top,
    prParent->left + nWidth, prParent->top + nHeight);
  if (m_nMode & POS_HCENTER)
  {
    // center horizontally
    rHelper.left += (prParent->Width() - nWidth) / 2;
    rHelper.right = rHelper.left + nWidth;
  }
  if (m_nMode & POS_HRIGHT)
  {
    // align right
    rHelper.left = prParent->right - nWidth;
    rHelper.right = prParent->right;
  }
  if (m_nMode & POS_VCENTER)
  {
    // center vertically
    rHelper.top += (prParent->Height() - nHeight) / 2;
    rHelper.bottom = rHelper.top + nHeight;
  }
  if (m_nMode & POS_VBOTTOM)
  {
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
int CPlotHelperWnd::SetMode(int nMode, int nID, CRect* prParent)
{
  int nOldMode = m_nMode; // save actual mode
  static int nOldID = 0; // save last mode
  if (prParent == NULL) prParent = &m_rParent; // don't change the coordinates
  CRect rWnd(0, 0, 0, 0);
  if ((nMode != m_nMode) || (nID != nOldID) || (*prParent != m_rParent))
  {
    m_nMode = nMode;
    nOldID = nID;
    m_rParent = *prParent;
    switch (m_nMode & MODE_MASK)
    {
    case MODE_TEXT:
      {
        // load new text to display
        if (nID != -1) m_szText.LoadString(nID);
        
        CDC dc;
        dc.CreateCompatibleDC(NULL);
        
        // select the helper window font
        CFont* oldFont = dc.SelectObject(&m_font);
        CSize size = dc.GetTextExtent(m_szText);
        dc.SelectObject(oldFont); // set back old font
        
        // set new coordinates for window
        rWnd = SetPosition(size.cx + m_nCharWidth, m_nHeight + 4, prParent);
      }
      break;
    default: // default mode is hidden window
      break;
    }
    if(GetSafeHwnd())
      MoveWindow(rWnd);
  }
  return nOldMode; // return old mode
}

/////////////////////////////////////////////////////////////////////////////
// CPlotHelperWnd message handlers

/***************************************************************************/
// CPlotHelperWnd::OnPaint Painting
/***************************************************************************/
void CPlotHelperWnd::OnPaint()
{
  CPaintDC dc(this); // device context for painting
  if (m_nMode != MODE_HIDDEN)
  {
    // select the helper window font
    CFont* oldFont = dc.SelectObject(&m_font);
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
BOOL CPlotHelperWnd::OnEraseBkgnd(CDC* pDC)
{
  if ((m_nMode & MODE_MASK) != MODE_HIDDEN)
  {
    CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd(); // get pointer to colors from main frame
    Colors* pColors = pMainWnd->GetColors();
    CRect rWnd;

    GetClientRect(rWnd);

    CBrush brush(pColors->cSysBtnFace);
    pDC->FillRect(&rWnd, &brush);
    // paint 3D-frame
    CPen penDkgray(PS_SOLID, 1, pColors->cSysBtnShadow);
    CPen penLtgray(PS_SOLID, 1, pColors->cSysBtnHilite);
    CPen* pOldPen = pDC->SelectObject(&penDkgray);
    if (m_nMode & FRAME_POPIN)
    {
      // draw popping in frame
      pDC->MoveTo(0, rWnd.bottom);
      pDC->LineTo(0, 0);
      pDC->LineTo(rWnd.right - 1, 0);
      pDC->SelectObject(&penLtgray);
      pDC->LineTo(rWnd.right - 1, rWnd.bottom - 1);
      pDC->LineTo(0, rWnd.bottom - 1);
    }
    if (m_nMode & FRAME_POPOUT)
    {
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
void CPlotHelperWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
  CWnd* pWnd = GetParent();
  ClientToScreen(&point);
  pWnd->ScreenToClient(&point);
  pWnd->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
}

/***************************************************************************/
// CPlotHelperWnd::OnRButtonDown Mouse right button down
/***************************************************************************/
void CPlotHelperWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
  CWnd* pWnd = GetParent();
  ClientToScreen(&point);
  pWnd->ScreenToClient(&point);
  pWnd->SendMessage(WM_RBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
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
//{{AFX_MSG_MAP(CPlotWnd)
ON_WM_PAINT()
ON_WM_CREATE()
ON_WM_SIZE()
ON_WM_ERASEBKGND()
ON_WM_MOUSEMOVE()
ON_WM_RBUTTONDOWN()
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()
ON_WM_KEYDOWN()
ON_WM_KEYUP()
ON_WM_SYSCOMMAND()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotWnd construction/destruction/creation

/***************************************************************************/
// CPlotWnd::CPlotWnd Constructor
/***************************************************************************/
CPlotWnd::CPlotWnd()
{
  m_bRtPlot = false;
  m_bInitialPlot = TRUE;
  m_pStartCursor = NULL;
  m_pStopCursor = NULL;
  m_pPrivateCursor = NULL;
  m_pPlaybackCursor = NULL;
  m_pLastProcess = NULL;
  m_pAreaProcess = NULL;
  m_bBoundaries = FALSE;
  m_bLineDraw = TRUE;
  m_bDotDraw = FALSE;
  m_bCursors = FALSE;
  m_bPrivateCursor = FALSE;
  m_bGrid = TRUE;
  m_fMagnify = 1.0;
  m_dProcessMultiplier = 1.0;
  m_dwHighLightPosition = 0;
  m_dwHighLightLength = 0;
  m_pParent = NULL;
  m_MousePointerPosn = CPoint(UNDEFINED_OFFSET, UNDEFINED_OFFSET);
  m_MouseButtonState = 0;
  m_bAnimationPlot = FALSE;
  m_dwAnimationFrame = UNDEFINED_OFFSET;
  SetHorizontalCursors(FALSE);
  SetBold(FALSE);


  /*!!for editing plots
  CDC* pDC = GetDC();
  m_SegmentParms.dcMem.CreateCompatibleDC(pDC);
  ReleaseDC(pDC);
  */
  m_pBitmapSave = NULL;
}


void CPlotWnd::CopyTo(CPlotWnd * pTarg)
{
  // copies don't have any cursors.
  pTarg->m_pStartCursor   = NULL;
  pTarg->m_pStopCursor    = NULL;
  pTarg->m_pPrivateCursor = NULL;
  pTarg->m_bPrivateCursor = FALSE;

  // copies don't have a process???
  pTarg->m_pAreaProcess        = NULL;
  pTarg->m_pLastProcess        = NULL;
  pTarg->m_bBoundaries         = m_bBoundaries;
  pTarg->m_bLineDraw           = m_bLineDraw;
  pTarg->m_bDotDraw            = m_bDotDraw;
  pTarg->m_bCursors            = m_bCursors;
  pTarg->m_bGrid               = m_bGrid;
  pTarg->m_fMagnify            = m_fMagnify;
  pTarg->m_dProcessMultiplier  = m_dProcessMultiplier;
  pTarg->m_szPlotName          = m_szPlotName;
  pTarg->m_dwHighLightPosition = 0;
  pTarg->m_dwHighLightLength   = 0;
}


/***************************************************************************/
// CPlotWnd::~CPlotWnd Destructor
/***************************************************************************/
CPlotWnd::~CPlotWnd()
{
  if (m_pStartCursor)    delete m_pStartCursor;
  if (m_pStopCursor)     delete m_pStopCursor;
  if (m_pPrivateCursor)  delete m_pPrivateCursor;
  if (m_pPlaybackCursor) delete m_pPlaybackCursor;

  // turn off any highlighting in raw waveform due to dynamic update mode
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
  CSaView* pView = (pMainWnd)?(CSaView*)pMainWnd->GetCurrSaView():NULL;
  if (!pView) return;
  BOOL bDynamicUpdate = (pView->GetGraphUpdateMode() == DYNAMIC_UPDATE);
  BOOL bAnimationPlot = this->IsAnimationPlot();
  if (bDynamicUpdate && bAnimationPlot)
  {
    int nWaveGraphIndex = pView->GetGraphIndexForIDD(IDD_RAWDATA);
    CGraphWnd* pWaveGraph = pView->GetGraph(nWaveGraphIndex);
    if (pWaveGraph)
	{ // there is a raw waveform graph
      CPlotWnd* pWavePlot = pWaveGraph->GetPlot();
      if(pWavePlot)
      {
        pWavePlot->SetHighLightArea(0, 0);
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
BOOL CPlotWnd::PreCreateWindow(CREATESTRUCT& cs)
{
  // register the window class
  cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
    AfxGetApp()->LoadStandardCursor(IDC_ARROW), 0, 0);
  //::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
  return CWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CPlotWnd helper functions

/***************************************************************************/
// CPlotWnd::ShowBoundaries Show or hide the boundaries
/***************************************************************************/
void CPlotWnd::ShowBoundaries(BOOL bShow, BOOL bRedraw)
{
  m_bBoundaries = bShow;
  if (bRedraw) RedrawPlot(); // repaint whole plot window
}

/***************************************************************************/
// CPlotWnd::SetMagnify Set the magnify factor
/***************************************************************************/
void CPlotWnd::SetMagnify(double fMagnify, BOOL bRedraw)
{
  if (m_fMagnify != fMagnify)
  {
    m_fMagnify = fMagnify;
    if (bRedraw) RedrawPlot(); // repaint whole plot window
  }
}

/***************************************************************************/
// CPlotWnd::SetLineDraw Set line drawing style
/***************************************************************************/
void CPlotWnd::SetLineDraw(BOOL bLine)
{
  if (m_bLineDraw != bLine)
  {
    // style changes
    m_bLineDraw = bLine;
    RedrawPlot(); // repaint whole plot window
  }
}

/***************************************************************************/
// CPlotWnd::SetDotsDraw Set dots drawing style
/***************************************************************************/
void CPlotWnd::SetDotsDraw(BOOL bDots)
{
  if (m_bDotDraw != bDots) // style changes
  {
    m_bDotDraw = bDots;
    RedrawPlot(); // repaint whole plot window
  }
}

/***************************************************************************/
// CPlotWnd::ShowCursors Set cursors visible/hidden
/***************************************************************************/
void CPlotWnd::ShowCursors(BOOL bPrivate, BOOL bShow)
{
  if (bPrivate)
  {
    m_bPrivateCursor = bShow;
    if (m_bPrivateCursor) m_bCursors = FALSE;
  }
  else
  {
    m_bCursors = bShow;
    if (m_bCursors) m_bPrivateCursor = FALSE;
  }
}

/***************************************************************************/
// CPlotWnd::ShowGrid Show or hide gridlines
/***************************************************************************/
BOOL CPlotWnd::ShowGrid(BOOL bShow, BOOL bRedraw)
{
  BOOL bResult = m_bGrid;
  m_bGrid = bShow;
  if (bRedraw) RedrawPlot(); // repaint whole plot window

  return bResult;
}

//**************************************************************************
// 08/30/2000 - DDO Added so this could be overridden.
//**************************************************************************
double CPlotWnd::GetDataPosition(int iWidth)
{
  CSaView* pView = (CSaView*)GetParent()->GetParent();
  return pView->GetDataPosition(iWidth);
}

//**************************************************************************
// 08/30/2000 - DDO Added so this could be overridden.
//**************************************************************************
DWORD CPlotWnd::AdjustDataFrame(int iWidth)
{
  CSaView* pView = (CSaView*)GetParent()->GetParent();
  return pView->AdjustDataFrame(iWidth);
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
// plot), its window size ill be 0.
/***************************************************************************/
void CPlotWnd::ChangeCursorPosition(CSaView* pView, DWORD dwNewPosition, CCursorWnd* pWnd, BOOL /*bMove*/)
{
  UNUSED_ALWAYS(pView);

  CGraphWnd* pGraph = (CGraphWnd*)GetParent(); // get pointer to parent graph
  // get window coordinates
  CRect rWnd, rNewWnd, rNewLine, rOldLine;
  GetClientRect(rWnd);
  if(rWnd.Width() == 0) return; // cursor not visible SDM 1.5Test8.5
  rNewWnd = rWnd;
  // get actual data position and frame
  double fDataPos;
  DWORD dwDataFrame;
  // check if area graph type
  if (pGraph->IsAreaGraph())
  {
    // get necessary data from area plot
    fDataPos = GetAreaPosition();
    dwDataFrame = GetAreaLength(&rWnd);
  }
  else
  {
    // get necessary data from view
    fDataPos = GetDataPosition(rNewWnd.Width()); // data index of first sample to display
    dwDataFrame = AdjustDataFrame(rNewWnd.Width()); // number of data points to display
  }
  if ((m_bCursors &&   // added by AKE to hide cursors in graph edit mode
    (dwNewPosition >= (DWORD)fDataPos) && (dwNewPosition < ((DWORD)fDataPos + dwDataFrame))))
  {
    // cursor is visible
    ASSERT(rNewWnd.Width());
    double fBytesPerPix = (double)dwDataFrame / (double)rNewWnd.Width(); // calculate data samples per pixel
    //    if ((DWORD)rNewWnd.Width() > dwDataFrame) fBytesPerPix = (double)pView->GetDocument()->GetFmtParm()->wBlockAlign;
    // calculate actual cursor position in pixel
    // SDM 1.06.6U4 calculate position based on pixel aligned graph
    int nPixelPos = (int)round(((double)dwNewPosition - fDataPos) / fBytesPerPix);
    // set the coordinates for the cursor window
    rNewLine.SetRect(nPixelPos, 0, nPixelPos + 1, rNewWnd.bottom);
    //SDM 1.06.5
    // Move mode offers no real advantage
    // we always draw only a line
    //if (bMove) rNewWnd = rNewLine;
    //else
    rNewWnd.SetRect(nPixelPos - CURSOR_WINDOW_HALFWIDTH, 0, nPixelPos + CURSOR_WINDOW_HALFWIDTH, rNewWnd.bottom);
  }
  else // cursor is not visible
  {
    rNewLine.SetRect(0, 0, 0, 0); // shrink it to 0 size
    rNewWnd = rNewLine;
  }
  // get the actual (old) position of cursor window
  pWnd->GetWindowRect(rOldLine);
  ScreenToClient(rOldLine);
  // get the line position in the middle
  if (rOldLine.Width() > 1) // cursor window has large width
  {
    rOldLine.left += CURSOR_WINDOW_HALFWIDTH;
    rOldLine.right -= (CURSOR_WINDOW_HALFWIDTH - 1);
  }
  // check if new cursor line position
  if (rOldLine != rNewLine)
  {
    // invalidate and update old position
    if(pWnd->IsDrawn())  // SDM 1.5Test10.5
      InvalidateRect(rOldLine, TRUE); // redraw old cursor position
    pWnd->MoveWindow(rNewWnd, FALSE); // move the cursor window to the new position
    pWnd->SetDrawn(FALSE);  // SDM 1.5Test10.5
    //  SDM 1.06.5
    //  By delaying update all changes are complete so the order of changes of cursors
    //  becomes irrelevant to cursor redraw.
    //  Underlying windows will be updated first because of the WS_EX_TRANSPARENT style of cursor windows
    //  UpdateWindow(); // update this region before redrawing the cursor window
    rNewLine.SetRect(rNewLine.left - rNewWnd.left, rNewLine.top, rNewLine.left - rNewWnd.left + 1, rNewLine.bottom);
    pWnd->InvalidateRect(rNewLine, TRUE); // redraw new cursor line
    //  pWnd->UpdateWindow(); // update the cursor
  }
  // move the cursor window to the new position
  //    else pWnd->MoveWindow(rNewWnd, FALSE);
}

/***************************************************************************/
// CPlotWnd::SetStartCursor Position the start cursor
/***************************************************************************/
void CPlotWnd::SetStartCursor(CSaView* pView)
{
  DWORD dwStartCursor = pView->GetStartCursorPosition();
  if (m_pStartCursor)
    ChangeCursorPosition(pView, dwStartCursor, m_pStartCursor);
  BOOL bDynamicUpdate = (pView->GetGraphUpdateMode() == DYNAMIC_UPDATE);
  if (bDynamicUpdate && m_bAnimationPlot)
  {
    // Finish pitch processing if necessary.
    CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();
    CProcessGrappl* pAutoPitch = (CProcessGrappl*)pDoc->GetGrappl(); 
    if (pAutoPitch->IsDataReady())
	{
      // Finish fragmenting if necessary.
      CProcessFragments* pFragments = (CProcessFragments*)pDoc->GetFragments();
      if (pFragments->IsDataReady())
	  {
        FmtParm* pFmtParm = pDoc->GetFmtParm();
        int nSmpSize = pFmtParm->wBlockAlign / pFmtParm->wChannels;
        m_dwAnimationFrame = pFragments->GetFragmentIndex(dwStartCursor/nSmpSize);  // set the animation frame
	  }
    }
  }
}

/***************************************************************************/
// CPlotWnd::SetStopCursor Position the stop cursor
/***************************************************************************/
void CPlotWnd::SetStopCursor(CSaView* pView)
{
  if (m_pStopCursor)
  {
    ChangeCursorPosition(pView, pView->GetStopCursorPosition(), m_pStopCursor);
  }
}

/***************************************************************************/
// CPlotWnd::SetPlaybackPosition
/***************************************************************************/
void CPlotWnd::SetPlaybackCursor(CSaView* pSaView, DWORD dwPos)
{
  if (m_pPlaybackCursor)
  {
    ChangeCursorPosition(pSaView, dwPos, m_pPlaybackCursor);
  }
}

/***************************************************************************/
// CPlotWnd::MoveStartCursor Move the start cursor
/***************************************************************************/
void CPlotWnd::MoveStartCursor(CSaView* pView, DWORD dwNewPosition)
{
  if (!m_bCursors) return; // no cursors visible
  ChangeCursorPosition(pView, dwNewPosition, m_pStartCursor, TRUE);
}

/***************************************************************************/
// CPlotWnd::MoveStopCursor Move the stop cursor
/***************************************************************************/
void CPlotWnd::MoveStopCursor(CSaView* pView, DWORD dwNewPosition)
{
  if (!m_bCursors) return; // no cursors visible
  ChangeCursorPosition(pView, dwNewPosition, m_pStopCursor, TRUE);
}

/***************************************************************************/
// CPlotWnd::SetInitialPrivateCursor Initialize the private cursor window
/***************************************************************************/
void CPlotWnd::SetInitialPrivateCursor()
{
  CRect rWnd;
  // get the coordinates of the private cursor
  m_pPrivateCursor->GetClientRect(rWnd);
  if (rWnd.Height() == 0)
  {
    // private cursor has not been initialized yet
    GetClientRect(rWnd);
    // set the cursor window size
    // CLW 10/23/00 if TWC graph, set cursor to middle of window
    if (m_szPlotName == "TWC")
    {
      CRect rPlotWindow;
      GetWindowRect(rPlotWindow);
      rWnd.top = rPlotWindow.Height() / 2 - 1;
      rWnd.bottom = rWnd.top + 1;
    }
    else
    {
      rWnd.left = -CURSOR_WINDOW_HALFWIDTH / 2;
      rWnd.right = rWnd.left + 2 * CURSOR_WINDOW_HALFWIDTH;
    }
    // move the private cursor window
    m_pPrivateCursor->MoveWindow(rWnd);
  }
}

/***************************************************************************/
// CPlotWnd::GetStartCursorPosition Returns the position of the start cursor
// The position is returned as pixel coordinate relative to the client area
// of the plot window. The pixel number returned is the center pixel of the
// cursor window. If the cursor is not visible, the function returns -1.
/***************************************************************************/
int CPlotWnd::GetStartCursorPosition()
{
  CRect rWnd;
  // get the coordinates of the private cursor
  m_pStartCursor->GetWindowRect(rWnd);
  ScreenToClient(rWnd);
  if (rWnd.Width() <= 0) return -1;
  if (rWnd.Width() == 1) return rWnd.left;
  else return rWnd.left + CURSOR_WINDOW_HALFWIDTH;
}

/***************************************************************************/
// CPlotWnd::GetStopCursorPosition Returns the position of the stop cursor
// The position is returned as pixel coordinate relative to the client area
// of the plot window. The pixel number returned is the center pixel of the
// cursor window. If the cursor is not visible, the function returns -1.
/***************************************************************************/
int CPlotWnd::GetStopCursorPosition()
{
  CRect rWnd;
  // get the coordinates of the private cursor
  m_pStopCursor->GetWindowRect(rWnd);
  ScreenToClient(rWnd);
  if (rWnd.Width() <= 0) return -1;
  if (rWnd.Width() == 1) return rWnd.left;
  else return rWnd.left + CURSOR_WINDOW_HALFWIDTH;
}

/***************************************************************************/
// CPlotWnd::GetPrivateCursorPosition Returns the pos. of the private cursor
// The position is returned as pixel coordinate relative to the client area
// of the plot window. The pixel number returned is the center pixel of the
// cursor window. If the cursor is not visible, the function returns -1.
/***************************************************************************/
int CPlotWnd::GetPrivateCursorPosition()
{
  CRect rWnd;
  // get the coordinates of the private cursor
  m_pPrivateCursor->GetWindowRect(rWnd);
  ScreenToClient(rWnd);

  if (rWnd.Width() <= 0) return -1;
  if (rWnd.Width() == 1) return rWnd.left;
  if (rWnd.Height() <= 0) return -1;
  if (rWnd.Height() == 1) return rWnd.top;
  if (rWnd.Width() < rWnd.Height())
  {
    return rWnd.left + CURSOR_WINDOW_HALFWIDTH;
  }
  else
  {
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
void CPlotWnd::ScrollPlot(CSaView* pView, int nScrollAmount, DWORD dwOldPos, DWORD dwFrame)
{
  // hide the helper windows (do not scroll them)
  int nOldMode = m_HelperWnd.SetMode(MODE_HIDDEN);
  // now scroll
  ScrollWindow(nScrollAmount, 0);
  UpdateWindow();
  // replace the helper windows
  m_HelperWnd.SetMode(nOldMode);
  // move the cursor windows if necessary
  DWORD dwStartCursorPos = pView->GetStartCursorPosition(); // position of start cursor
  DWORD dwStopCursorPos = pView->GetStopCursorPosition(); // position of stop cursor
  // move cursor windows
  CRect rCursor;
  if ((dwStartCursorPos < dwOldPos) || (dwStartCursorPos >= (dwOldPos + dwFrame)))
    SetStartCursor(pView); // start cursor was not visible, move it if it's now visible
  if ((dwStopCursorPos < dwOldPos) || (dwStopCursorPos >= (dwOldPos + dwFrame)))
    SetStopCursor(pView); // stop cursor was not visible, move it if it's now visible
}

/***************************************************************************/
// CPlotWnd::RedrawPlot Redraw plot including cursors
// If the flag bEntire is TRUE (default), the entire plot will be repainted,
// else only the part between (and with) the cursor windows are repainted,
// but only if boundaries are displayed.
/***************************************************************************/
void CPlotWnd::RedrawPlot(BOOL bEntire)
{
  // kg - in case we don't have a window yet..
  if (IsWindow(m_hWnd) == FALSE) return;
  CRect rWnd;
  GetClientRect(rWnd);
  if (bEntire)
  {
    // invalidate entire plot window
    InvalidateRect(NULL);
  }
  else
  {
    if (HaveBoundaries() && HaveCursors()) // boundaries displayed?
    {
      // invalidate region between (and with) cursor windows
      CRect rStart, rStop;
      m_pStartCursor->GetWindowRect(rStart);
      m_pStopCursor->GetWindowRect(rStop);
      rWnd.UnionRect(rStart, rStop); // union of both rectangles
      ScreenToClient(rWnd);
      InvalidateRect(rWnd); // invalidate this area
    }
  }
}

/***************************************************************************/
// CPlotWnd::CheckResult Check the processing result
// The result of data processing can be an error or a canceled state. In the
// second case, the helper window will be activated with the message about
// the canceled state. In case of processing error, the graph will be closed.
//**************************************************************************/
short int CPlotWnd::CheckResult(short int nResult, CDataProcess* pProcess)
{
  m_pLastProcess = pProcess; // save pointer to process object for further use
  if(!this->GetSafeHwnd())
    return nResult;

  CRect rClient;
  GetClientRect(rClient);

  CDC* pDC = GetDC();
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
  Colors* pColor = pMainWnd->GetColors();  // get application colors
  CBrush Eraser(pColor->cPlotBkg);
  switch(nResult)
  {
  case PROCESS_CANCELED:
    // process has been canceled, display helper window
    pDC->FillRect(&rClient, &Eraser);  // clear the plot area
    m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_CANCELED, &rClient);
    break;
  case PROCESS_NO_DATA:
    // no data to process
    pDC->FillRect(&rClient, &Eraser);  // clear the plot area
    //m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_NOVOICING, &rClient);
    break;
  case PROCESS_DATA_OVERLOAD:
    // too much data to process
    // pDC->FillRect(&rClient, &Eraser);  // clear the plot area
    m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_CURCLOSER, &rClient);
    break;
  case PROCESS_UNVOICED:

    {
      // process data is unvoiced
      // pDC->FillRect(&rClient, &Eraser);  // clear the plot area
      CGraphWnd* pGraph = (CGraphWnd*)GetParent();
      CSaView* pView = (CSaView*)pGraph->GetParent();
      BOOL bDynamicUpdate = (pView->GetGraphUpdateMode() == DYNAMIC_UPDATE);
      if (!bDynamicUpdate) m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_SELECTVOICED, &rClient);
      else m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_UNVOICED, &rClient);
      break;
    }
  case PROCESS_ERROR:
    {
      // processing error
      CSaApp* pApp = (CSaApp*)AfxGetApp();
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
BOOL CPlotWnd::PlotPrePaintDots(CDC* pDC, int nTop, CRect rClip,
                                CLegendWnd* pLegend, CXScaleWnd* pXScale,
                                COLORREF cColor, int nStyle)
{
  double dXFirstGridPos = pXScale->GetFirstGridPosition();
  double dYFirstGridPos = pLegend->GetFirstGridPosition();
  double fXDistance = pXScale->GetGridDistance();
  double fYDistance = pLegend->GetGridDistance();

  if(fXDistance == 0. || fXDistance == std::numeric_limits<double>::infinity())
    return TRUE;

  if(fYDistance == 0. || fYDistance == std::numeric_limits<double>::infinity())
    return TRUE;

  int nXCount = (int) ceil((rClip.left - 2 - dXFirstGridPos)/fXDistance);
  double dXPos = dXFirstGridPos + nXCount * fXDistance;
  if (pLegend->GetScaleMode() & LOG10)
  {
    // logarithmic scale
    double dScaleMinValue = pLegend->GetScaleMinValue();
    double fBase = pLegend->GetScaleBase();
    while (dXPos <= rClip.right + 2)
    {
      int nXPos = round(dXPos);

      int nLogDisp = (int)ceil(dScaleMinValue / pow(10, floor(fBase)));
      double fBasePos = (double)nTop + dYFirstGridPos + (fBase - floor(fBase)) * fYDistance;
      int nYPos = round(fBasePos - log10((double)nLogDisp) * fYDistance);
      while (nYPos > rClip.top)
      {
        // set pixel
        pDC->SetPixel(nXPos, nYPos, cColor);
        if (nStyle == 3)
        {
          pDC->SetPixel(nXPos, (nYPos + 1), cColor);
          pDC->SetPixel((nXPos + 1), nYPos, cColor);
          pDC->SetPixel(nXPos, (nYPos - 1), cColor);
          pDC->SetPixel((nXPos - 1), nYPos, cColor);
        }
        if (nLogDisp > 9)
        {
          nLogDisp = 0;
          fBasePos -= fYDistance;
        }
        nLogDisp++;
        nYPos = round(fBasePos - log10((double)nLogDisp) * fYDistance);
      }
      dXPos = dXFirstGridPos + ++nXCount * fXDistance;
    }
  }
  else
  {
    // linear scale
    while (dXPos <= rClip.right + 2)
    {
      int nXPos = round(dXPos);
      double dYPos = dYFirstGridPos;
      while (dYPos < rClip.bottom)
      {
        int nYPos = round(dYPos);
        // set pixel
        pDC->SetPixel(nXPos, nYPos, cColor);
        if (nStyle == 3)
        {
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
void CPlotWnd::PlotPrePaint(CDC* pDC, CRect rWnd, CRect rClip, CLegendWnd* pLegend, BOOL bShowCursors, BOOL bPrivateCursor)
{
  // get pointer to graph, grid structure, legend- and x-scale window
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
  const Grid cGrid = GetGrid();
  const Grid *pGrid = &cGrid;
  if(!pLegend)
    pLegend = pGraph->GetLegendWnd();
  CXScaleWnd* pXScale = pGraph->GetXScaleWnd();
  // calculate the scales
  pLegend->CalculateScale(NULL, &rWnd);
  pXScale->CalculateScale(NULL, rWnd.Width());

  if (!HaveGrid()) return; // no grid to draw

  ShowCursors(bPrivateCursor, bShowCursors);

  // prepare to get color from main frame
  Colors* pColors = pMainWnd->GetColors();
  // set background color for dashed or dotted lines
  pDC->SetBkColor(pColors->cPlotBkg);
  static const unsigned char bit[8] =
  {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
  if (pGrid->bXGrid)
  {
    // draw x -grid
    if (pGrid->nXStyle)
    {
      // line style
      int nPenStyle = PS_SOLID;
      unsigned char nPenPattern=0xff;
      switch (pGrid->nXStyle)
      {
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

      if ((pGrid->nXStyle != 3))
      {
        // create and select pen
        CPen pen(nPenStyle, 1, pColors->cPlotGrid);
        CPen* pOldPen = pDC->SelectObject(&pen);
        double fStartPos = pXScale->GetFirstGridPosition();
        double fDistance = pXScale->GetGridDistance();
        double fGridPos = (double)pLegend->GetFirstGridPosition();
        double fGridDistance = pLegend->GetGridDistance()/8.;
        if(fDistance > 0.)
        {
          int nCount = (int) ceil((rClip.left - 2 - fStartPos)/fDistance);
          double dPos = fStartPos + nCount * fDistance;
          
          while (dPos <= rClip.right + 2)
          {
            // draw a line
            int Index = 0;
            int nPos = round(dPos);
            for (double y=fGridPos-fGridDistance*8.5; y < rWnd.bottom;)
            {
              if (bit[Index]&nPenPattern)
              {
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
    if ((!pGrid->nXStyle) || (pGrid->nXStyle == 3))
    {
      // dot style (1 pixel) or "- - - - - - - - - - -" style (2x2 pixels)
      PlotPrePaintDots(pDC, rWnd.top, rClip, pLegend, pXScale,
        pColors->cPlotGrid, pGrid->nXStyle);
    }
  } 
  
  if (pGrid->bYGrid)
  {
    // line style
    int nPenStyle = PS_SOLID;
    unsigned char nPenPattern=0xff;
    // draw y -grid
    if (pGrid->nYStyle)
    {
      // line style
      nPenStyle = PS_SOLID;
      nPenPattern = 0xff;;
      switch (pGrid->nYStyle)
      {
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

    if (((pGrid->nYStyle) && (pGrid->nYStyle != 3)))
    {
      // create and select pen
      CPen pen(nPenStyle, 1, pColors->cPlotGrid);
      CPen* pOldPen = pDC->SelectObject(&pen);
      double fGridPos = pXScale->GetFirstGridPosition();
      double fGridDistance = pXScale->GetGridDistance()/8.;
      if (pLegend->GetScaleMode() & LOG10)
      {
        // logarithmic scale
        double fYStartPos = (double)pLegend->GetFirstGridPosition();
        double fDistance = pLegend->GetGridDistance();
        double dScaleMinValue = pLegend->GetScaleMinValue();
        double fBase = pLegend->GetScaleBase();
        int nLogDisp = (int)ceil(dScaleMinValue / pow(10, floor(fBase)));
        double fBasePos = (double)rWnd.top + fYStartPos + (fBase - floor(fBase)) * fDistance;
        int nPos = (int)round(fBasePos - log10((double)nLogDisp) * fDistance);
        while (nPos > rClip.top)
        {
          // draw a grid line
          int Index = 0;
          for (double x = fGridPos-fGridDistance*8.5;x<rWnd.right;)
          {
            if (bit[Index]&nPenPattern)
            {
              pDC->MoveTo((int)x, nPos);
              pDC->LineTo( (int)(x + fGridDistance), nPos);
            }
            Index = (Index+1)%8;
            x += fGridDistance;
          }
          if (nLogDisp > 9)
          {
            nLogDisp = 0;
            fBasePos -= fDistance;
          }
          nLogDisp++;
          nPos = (int)round(fBasePos - log10((double)nLogDisp) * fDistance);
        }
      }
      else
      {
        // linear scale
        double dStartPos = pLegend->GetFirstGridPosition();
        int nCount = 0;
        double fDistance = pLegend->GetGridDistance();
        double dPos = dStartPos;
        while (dPos < rClip.bottom)
        {
          // draw a grid line
          int Index = 0;
          int nPos = round(dPos);
          for (double x = fGridPos-fGridDistance*8.5;x<rWnd.right;)
          {
            if (bit[Index]&nPenPattern)
            {
              pDC->MoveTo((int)round(x), nPos);
              pDC->LineTo( (int)round(x + fGridDistance), nPos);
            }
            Index = (Index+1)%8;
            x += fGridDistance;
          }
          dPos = dStartPos + ++nCount * fDistance;
        }
      }
      pDC->SelectObject(pOldPen);
    }
    else if ((!pGrid->nYStyle) || (pGrid->nYStyle == 3))
    {
      // dot style (1 pixel) or "- - - - - - - - - - -" style (2x2 pixels)
      PlotPrePaintDots(pDC, rWnd.top, rClip, pLegend, pXScale,
        pColors->cPlotGrid, pGrid->nYStyle);
    }
  }
}



void DrawData(CRect &rClip, CDataSource &cData, const CXScale &cXScale, const CYScale &cYScale, CDrawSegment &cDrawSegment)
{
  int nFirst = (int) floor(cXScale.GetSample(rClip.left)) - 1; 
  int nLast = (int) ceil(cXScale.GetSample(rClip.right)) + 1; 
  int x = 0;

  BOOL bValid = TRUE;

  CDataSource::CValues cValues;

  int nSample = nFirst > 0 ? nFirst : 0;

  int nPasses = 0;
  while ( nSample <= nLast )
  {
    x = cXScale.GetX(nSample);
    int nNext = (int) floor(cXScale.GetSample(x+1)); 

    cData.GetValues(nSample, nNext, cValues, bValid);
    
    cDrawSegment.DrawTo(x,cValues, cYScale, bValid);    
    nPasses++;
  }
  // Flush drawing functions
  bValid = FALSE;
  cDrawSegment.DrawTo(x,0, cYScale, bValid);
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
void CPlotWnd::PlotStandardPaint(CDC* pDC, CRect rWnd, CRect rClip,
                                 CDataProcess* pProcess, CSaDoc *pProcessDoc, int nFlags)
{
  if(rClip.IsRectEmpty())
    return;

  // get pointer to graph, view, document mainframe and legend window
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
  CLegendWnd* pLegend = pGraph->GetLegendWnd();

  // get pointer to color structure from main frame
  Colors* pColor = pMainWnd->GetColors();

  CXScale *pXScale = NULL;
  CXScale *pXScaleRaw = NULL;
  // Create pXScale
  {
    CSaView* pView = (CSaView*)pGraph->GetParent();
    CSaDoc* pHostDoc = (CSaDoc*)pView->GetDocument();
    CSaDoc *pDoc = pProcessDoc;

    FmtParm* pFmtParm = pDoc->GetFmtParm();
    WORD wSmpSize =  WORD(pFmtParm->wBlockAlign / pFmtParm->wChannels);

    // calculate size factor between raw data and process data
    double fSizeFactor = (double)wSmpSize * ceil((double)(pDoc->GetDataSize()/wSmpSize)/ (double)(pProcess->GetDataSize()));
    
    // get necessary data from document and from view
    double fDataPos = GetDataPosition(rWnd.Width()); // data index of first sample to display
    DWORD dwDataFrame = AdjustDataFrame(rWnd.Width()); // number of data points to display
    if (dwDataFrame == 0)
      return; // nothing to draw
    
    // calculate raw data samples per pixel
    double fBytesPerPix = double(dwDataFrame)*pFmtParm->dwAvgBytesPerSec/pHostDoc->GetFmtParm()->dwAvgBytesPerSec / (double)rWnd.Width();

    pXScale = new CXScaleLinear(fBytesPerPix/fSizeFactor, fDataPos/fSizeFactor);
    pXScaleRaw = new CXScaleLinear(fBytesPerPix, fDataPos);
  }

  CYScale *pYScale = NULL;
  // Create YScale
  double fMultiplier = GetProcessMultiplier();
  BOOL bLog10 = (BOOL)(pLegend->GetScaleMode() & LOG10); // logarithmic/linear scale
  
  double dCenterPos;
  double fBase = pLegend->GetScaleBase();
  if (bLog10)
  {
    // logarithmic scale
    dCenterPos = pLegend->GetFirstGridPosition();
    fBase = pLegend->GetScaleBase() + log10(fMultiplier);
    m_fVScale = pLegend->GetGridDistance();

    pYScale = new CYScaleLog(-m_fVScale, dCenterPos + fBase*m_fVScale);
  }
  else // linear scale
  {
    double dScaleMin = pLegend->GetScaleMinValue();
    if(dScaleMin < 0.)
      dScaleMin /= GetMagnify(); // this is dumb... legend should report the actual

    if(nFlags & PAINT_DB)
    {
      // calculate vertical scaling factor
      fBase = pLegend->GetScaleBase();
      m_fVScale = pLegend->GetGridDistance() / fBase;
      dCenterPos = rWnd.bottom +  dScaleMin*m_fVScale; // x-axis vertical position

      double dblDbReference = -20*log10(double(0x7fff)) + 6; // loudness represents rms value so peak is +3dB and we want another 3db headroom

      pYScale = new CYScaleDB(-m_fVScale, dCenterPos, dblDbReference, 20.);
    }
    else if(nFlags & PAINT_SEMITONES)
    {
      // calculate vertical scaling factor
      fBase = pLegend->GetScaleBase();
      m_fVScale = pLegend->GetGridDistance() / fBase;
      dCenterPos = rWnd.bottom +  dScaleMin*m_fVScale; // x-axis vertical position
      
      pYScale = new CYScaleSemitones(-m_fVScale, dCenterPos, 10);
    }
    else
    {
      // calculate vertical scaling factor
      fBase = pLegend->GetScaleBase() * fMultiplier;
      m_fVScale = pLegend->GetGridDistance() / fBase;
      dCenterPos = rWnd.bottom +  dScaleMin*m_fVScale*fMultiplier; // x-axis vertical position

      pYScale = new CYScaleLinear(-m_fVScale, dCenterPos);

      // draw bottom line
      CPen penAxes(PS_SOLID, 1, pColor->cPlotAxes);
      CPen *pOldPen = pDC->SelectObject(&penAxes);
      pDC->MoveTo(rWnd.left, round(dCenterPos)); // draw the line
      pDC->LineTo(rWnd.right, round(dCenterPos));
      pDC->SelectObject(pOldPen);
    }
  }

  CDataSource *pSource = NULL;
  switch (nFlags & (SKIP_UNSET | SKIP_MISSING))
  {
  case 0:
    pSource = new CDataSourceSimple(*pProcess);
    break;
  default:
    pSource = new CDataSourceValidate(*pProcess, nFlags & SKIP_UNSET, nFlags & SKIP_MISSING);
  }


  CDrawSegment* pSegment = NULL;
  // Create pSegment
  if (nFlags & PAINT_CROSSES)
  {
    // paint small crosses for each data point
    pSegment = new CDrawSegmentSample(*pDC);
  }
  else if (nFlags & PAINT_DOTS)
  {
    // paint dots (only) for each data point
    pSegment = new CDrawSegmentDotOnly(*pDC);
  }
  else if (m_bLineDraw)
  {
    // Smooth line drawing
    pSegment = new CDrawSegmentLine(*pDC);
  }
  else
  {
    // Paint Solid
    pSegment = new CDrawSegmentSolid(*pDC, nFlags & (PAINT_DB|PAINT_SEMITONES) ? rWnd.bottom : round(dCenterPos));
  }

  int nLineThickness = GetPenThickness(); // Sometimes Based on VScale

  CPen penData(PS_SOLID, nLineThickness, pColor->cPlotData[0]);
  CPen penHiData(PS_SOLID, nLineThickness, pColor->cPlotHiData);
  CPen* pOldPen = pDC->SelectObject(&penData);

  CRect rClipPortion(rClip);

  try
  {    
    int nXStart = m_dwHighLightLength ? pXScaleRaw->GetX(m_dwHighLightPosition) : 0;
    int nXEnd = m_dwHighLightLength ? pXScaleRaw->GetX(m_dwHighLightPosition + m_dwHighLightLength) : 0;
    
    CRgn cClipRgn;
    cClipRgn.CreateRectRgnIndirect(rClip);

    GetClipRgn(pDC->m_hDC, cClipRgn);

    
    if(nXStart > rClip.left)
    {
      rClipPortion.left=rClip.left;
      rClipPortion.right=nXStart;
      
      CRgn cClipPortionRgn;
      cClipPortionRgn.CreateRectRgnIndirect(rClipPortion);
      //ExtSelectClipRgn(pDC->m_hDC, cClipPortionRgn, RGN_AND);

      pDC->SelectObject(&penData);
      pSegment->SetColor(pColor->cPlotData[0]);
      
      rClipPortion.InflateRect(nLineThickness+1,0);
      DrawData(rClipPortion, *pSource, *pXScale, *pYScale, *pSegment);

      ExtSelectClipRgn(pDC->m_hDC, cClipRgn, RGN_COPY);
    }
    if(nXStart < rClip.right && nXEnd >= rClip.left)
    {
      pDC->SelectObject(&penData);
      rClipPortion.left = nXStart;
      rClipPortion.right = nXEnd;
      
      CRgn cClipPortionRgn;
      cClipPortionRgn.CreateRectRgnIndirect(rClipPortion);
      //ExtSelectClipRgn(pDC->m_hDC, cClipPortionRgn, RGN_AND);

      pDC->SelectObject(&penHiData);
      pSegment->SetColor(pColor->cPlotHiData);
      
      rClipPortion.InflateRect(nLineThickness+1,0);
      DrawData(rClipPortion, *pSource, *pXScale, *pYScale, *pSegment);

      ExtSelectClipRgn(pDC->m_hDC, cClipRgn, RGN_COPY);
    }
    if(nXEnd < rClip.right)
    {
      rClipPortion.left = nXEnd;
      rClipPortion.right = rClip.right;
      
      CRgn cClipPortionRgn;
      cClipPortionRgn.CreateRectRgnIndirect(rClipPortion);
      //ExtSelectClipRgn(pDC->m_hDC, cClipPortionRgn, RGN_AND);

      pDC->SelectObject(&penData);
      pSegment->SetColor(pColor->cPlotData[0]);
      
			CRect tmp;
			pDC->GetClipBox(tmp);
			tmp.DeflateRect(2,2);

			CBrush brush(pColor->cSysBtnFace);
			pDC->FrameRect(tmp, &brush);

      rClipPortion.InflateRect(nLineThickness+1,0);
      DrawData(rClipPortion, *pSource, *pXScale, *pYScale, *pSegment);

      ExtSelectClipRgn(pDC->m_hDC, cClipRgn, RGN_COPY);
    }

  }
  catch (...)
  {
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
void CPlotWnd::PlotPaintFinish(CDC* pDC, CRect rWnd, CRect rClip)
{
  // get pointer to graph, view and document
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  CSaView* pView = (CSaView*)pGraph->GetParent();
  CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();

  if (m_bBoundaries)
  {
    // paint boundaries
    double fDataStart;
    DWORD dwDataFrame;
    // check if area graph type
    if (pGraph->IsAreaGraph())
    {
      // get necessary data from area plot
      fDataStart = GetAreaPosition();
      dwDataFrame = GetAreaLength(&rWnd);
    }
    else
    {
      // get necessary data from document and from view
      fDataStart = GetDataPosition(rWnd.Width()); // data index of first sample to display
      dwDataFrame = AdjustDataFrame(rWnd.Width()); // number of data points to display
    }
    if (dwDataFrame && rWnd.Width())
    {
      // calculate the number of data samples per pixel
      double fBytesPerPix = (double)dwDataFrame / (double)rWnd.Width();
      // get pointer to phonetic string
      CString* pPhonetic = pDoc->GetSegment(PHONETIC)->GetString();
      if (!pPhonetic->IsEmpty()) // string is not empty
      {
        // get pointer to phonetic offset and duration arrays
        CSegment* pOffsets = pDoc->GetSegment(PHONETIC);
        // position prepare loop. Find first boundary to display in clipping rect
        double fStart = fDataStart + ((double)rClip.left) * fBytesPerPix;
        int nLoop = 0;
        if (fStart > 0)
        {
          for (nLoop = 1; nLoop < pPhonetic->GetLength(); nLoop++)
          {
            if (pOffsets->GetStop(nLoop)> fStart) break; // this is it
          }
        }
        // first char must be at lower position
        if (nLoop > 0) nLoop = pDoc->GetSegment(PHONETIC)->GetPrevious(nLoop);
        // check for overlap and make correction (draw previous character boundaries too)
        if ((nLoop > 0) && (pOffsets->GetOffset(nLoop) < (pOffsets->GetStop(nLoop - 1)))) nLoop--;
        // display loop, create pen and brush
        CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd(); // get color from main frame
        CPen penBoundary(PS_SOLID, 1, pMainWnd->GetColors()->cPlotBoundaries);
        CPen* pOldPen = pDC->SelectObject(&penBoundary);
        CBrush brushBoundary(pMainWnd->GetColors()->cPlotBoundaries);
        CBrush* pOldBrush = (CBrush*)pDC->SelectObject(&brushBoundary);
        // define points for the boundary arrows
        POINT points[3];
        points[0].y = points[1].y = 0;
        points[2].y = 4;
        int nDisplayPos; // position in pixels to display boundary
        do
        {
          // calculate start boundary
          nDisplayPos = round((pOffsets->GetOffset(nLoop) - fDataStart)/fBytesPerPix);
          // calculate stop boundary
          int nDurationPos = round((pOffsets->GetStop(nLoop) - fDataStart)/fBytesPerPix);
          // next string start
          nLoop = pDoc->GetSegment(PHONETIC)->GetNext(nLoop);
          // draw the lines
          pDC->MoveTo(nDisplayPos, 0); // paint start line
          pDC->LineTo(nDisplayPos, rWnd.bottom);
          points[0].x = nDisplayPos + 4; // draw start arrow
          points[1].x = points[2].x = nDisplayPos;
          pDC->Polygon(points, 3);
          pDC->MoveTo(nDurationPos, 0); // paint stop line
          pDC->LineTo(nDurationPos, rWnd.bottom);
          points[0].x = nDurationPos - 4; // draw stop arrow
          points[1].x = points[2].x = nDurationPos;
          pDC->Polygon(points, 3);
        } while ((nDisplayPos < rClip.right) && (nLoop >= 0));
        pDC->SelectObject(pOldPen); // select back old pen
        pDC->SelectObject(pOldBrush);
      }
    }
  }

  // paint cursors
  if (rClip.Width() >= rWnd.Width()) // whole graph drawn
  {
    if (m_bCursors)
    {
      SetStopCursor(pView);
      SetStartCursor(pView);
    }
  }

  // if this graph has focus, update the status bar.   AKE 10/21/2000
  if (m_bInitialPlot && pView->GetFocusedGraphWnd())
  {
    pGraph->UpdateStatusBar(pView->GetStartCursorPosition(), pView->GetStopCursorPosition(), TRUE); // update the status bar
    m_bInitialPlot = FALSE;
  }
}

/***************************************************************************/
// CPlotWnd::IsCanceled Return cancel state
//**************************************************************************/
BOOL CPlotWnd::IsCanceled()
{
  if (m_pLastProcess) return m_pLastProcess->IsCanceled();
  return FALSE;
}

/***************************************************************************/
// CPlotWnd::RestartProcess Restart a canceled process or recalc an area graph
//**************************************************************************/
void CPlotWnd::RestartProcess()
{
  m_HelperWnd.SetMode(MODE_HIDDEN); // hide helper window
  if (IsAreaGraph())
  {
    m_pAreaProcess->UpdateArea();
    CGraphWnd* pGraph = (CGraphWnd*)GetParent();
    pGraph->RedrawGraph(TRUE, TRUE); // repaint whole graph
  }

  if(m_pLastProcess && IsCanceled())
  {
    m_pLastProcess->RestartProcess();
    RedrawPlot(); // repaint whole plot
  }
}

/***************************************************************************/
// CPlotWnd::GetAreaPosition Return area position of area graph
// If the current area length is 0, the function returns the data position
// of the view. In case of an error 0 is returned.
//**************************************************************************/
DWORD CPlotWnd::GetAreaPosition()
{
  if (m_pAreaProcess)
  {
    CAreaDataProcess* pProcess = (CAreaDataProcess*)m_pAreaProcess; // cast pointer
    if (pProcess->GetAreaLength() == 0)
    {
      // area length is zero, so return the views position
      CRect rWnd(0,0,0,0);
      if(GetSafeHwnd())
        GetClientRect(rWnd);
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
DWORD CPlotWnd::GetAreaLength(CRect * pRwnd)
{
  if (m_pAreaProcess)
  {
    CAreaDataProcess* pProcess = (CAreaDataProcess*)m_pAreaProcess; // cast pointer
    DWORD dwFrame = pProcess->GetAreaLength();
    if (dwFrame == 0)
    {
      // area length is zero, so return the views length
      CRect rWnd;
      if (pRwnd)
      {
        rWnd = *pRwnd;
      }
      else
      {
        GetClientRect(rWnd);
      }
      return AdjustDataFrame(rWnd.Width());
    }
    return dwFrame;
  }
  return 0;
}

/***************************************************************************/
// CPlotWnd::SetHighLightArea Sets a highlighted area
// dwStart and dwStop contain the area that has to be highlighted (in raw
// data bytes). They always will be snapped to the next zero crossing unless
// they are at the beginning or end of the raw data. If the flag bRedraw is
// TRUE (default), the area that has to change its colors will be invali-
// dated. It takes areas only for plots with visible and non private cursors.
// Start and Stop are not range checked. Selected segments will be
// deselected.
//**************************************************************************/
void CPlotWnd::SetHighLightArea(DWORD dwStart, DWORD dwStop, BOOL bRedraw, BOOL bSecondSelection)
{
  if (m_bCursors)
  {
    CSaView* pView = (CSaView*)GetParent()->GetParent();
    CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();
    FmtParm* pFmtParm = pDoc->GetFmtParm();
    UINT nSampleSize = pFmtParm->wBlockAlign / pFmtParm->wChannels;
    if (nSampleSize == 2)
    {
      // positions have to be even for 16 bit
      dwStart &= ~1;
      dwStop &= ~1;
    }
    if (dwStart > 0) dwStart = pDoc->SnapCursor(START_CURSOR, dwStart, 0, pDoc->GetDataSize() - nSampleSize);
    if ((dwStop > 0) && (dwStop < (pDoc->GetDataSize() - nSampleSize)))
      dwStop = pDoc->SnapCursor(STOP_CURSOR, dwStop, 0, pDoc->GetDataSize() - nSampleSize);

    if(m_dwHighLightPosition == dwStart && m_dwHighLightLength == dwStop - dwStart)
      return;

    if (bRedraw)
    {
      // calculate the actual and the new highlighted rectangles
      CRect rWnd;
      GetClientRect(rWnd);
      double fDataPos = GetDataPosition(rWnd.Width()); // data index of first sample to display
      DWORD dwDataFrame = AdjustDataFrame(rWnd.Width()); // number of data points to display
      ASSERT(rWnd.Width());
      double fBytesPerPix = (double)dwDataFrame / (double)rWnd.Width();

      int nHighLightPixLeft = 0;
      int nHighLightPixRight = 0;
      if ((dwDataFrame != 0) && (m_dwHighLightLength != 0))
      {
        // there is actually something to highlight
        if ((m_dwHighLightPosition + m_dwHighLightLength) > fDataPos)
        {
          if (m_dwHighLightPosition < fDataPos)
          {
            m_dwHighLightLength -= ((DWORD)fDataPos - m_dwHighLightPosition);
            m_dwHighLightPosition = (DWORD)fDataPos;
          }
          // SDM 1.06.6U4 align selection to graph aligned to pixels
          nHighLightPixLeft = round(((double)m_dwHighLightPosition - fDataPos) / fBytesPerPix);
          nHighLightPixRight = round(((double)(m_dwHighLightPosition + m_dwHighLightLength) - fDataPos) / fBytesPerPix);
        }
      }
      // this is the actual highlighted rectangle in the plot
      CRect rOldHi(nHighLightPixLeft - 1, rWnd.top, nHighLightPixRight + 1, rWnd.bottom);
      nHighLightPixLeft = round(((double)dwStart - fDataPos) / fBytesPerPix);
      nHighLightPixRight = round(((double)dwStop - fDataPos) / fBytesPerPix);
      // this is the new highlighted rectangle in the plot
      CRect rNewHi(nHighLightPixLeft - 1, rWnd.top, nHighLightPixRight + 1, rWnd.bottom);

      // now find the difference
      rWnd.UnionRect(rOldHi, rNewHi);

      m_dwHighLightPosition = dwStart;
      m_dwHighLightLength = dwStop - dwStart;

      rWnd.InflateRect(2,0);
      InvalidateRect(rWnd,TRUE);
    }
    else
    {
      m_dwHighLightPosition = dwStart;
      m_dwHighLightLength = dwStop - dwStart;
    }
    if (m_dwHighLightLength && !bSecondSelection)
    {
      // deselect segment, if one selected
      CSegment* pSegment = pView->FindSelectedAnnotation();
      if (pSegment)
      {
        pView->ChangeAnnotationSelection(pSegment, pSegment->GetSelection(), 0, 0);
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// CPlotWnd message handlers

/***************************************************************************/
// CPlotWnd::OnCreate Window creation
/***************************************************************************/
int CPlotWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CWnd::OnCreate(lpCreateStruct) == -1) return -1;

  m_pStartCursor = new CStartCursorWnd();
  m_pStopCursor = new CStopCursorWnd();
  m_pPrivateCursor = new CPrivateCursorWnd();
  m_pPlaybackCursor = new CPrivateCursorWnd();
  CRect rWnd(0, 0, 0, 0);
  // create the start cursor
  if (!m_pStartCursor->Create(NULL, _T("Cursor"), WS_CHILD | WS_VISIBLE, rWnd, this, 0))
  {
    delete m_pStartCursor;
    m_pStartCursor = NULL;
  }
  // create the stop cursor
  if (!m_pStopCursor->Create(NULL, _T("Cursor"), WS_CHILD | WS_VISIBLE, rWnd, this, 0))
  {
    delete m_pStopCursor;
    m_pStopCursor = NULL;
  }
  // create the private cursor
  if (!m_pPrivateCursor->Create(NULL, _T("Cursor"), WS_CHILD | WS_VISIBLE, rWnd, this, 0))
  {
    delete m_pPrivateCursor;
    m_pPrivateCursor = NULL;
  }
  // create the playback cursor
  if (!m_pPlaybackCursor->Create(NULL, _T("Cursor"), WS_CHILD | WS_VISIBLE, rWnd, this, 0))
  {
    delete m_pPlaybackCursor;
    m_pPlaybackCursor = NULL;
  }

  // create the helper window
  m_HelperWnd.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, rWnd, this, 1);

  return 0;
}

/***************************************************************************/
// CPlotWnd::OnSize Sizing the window
// Before the resizing of a plot window, the cursor windows are set to size
// 0, so that after the redraw of the plot they will be redrawn too.
/***************************************************************************/
void CPlotWnd::OnSize(UINT nType, int cx, int cy)
{
  m_pStartCursor->MoveWindow(0, 0, 0, 0);
  m_pStartCursor->ResetPosition();
  m_pStopCursor->MoveWindow(0, 0, 0, 0);
  m_pStopCursor->ResetPosition();
  m_pPrivateCursor->MoveWindow(0, 0, 0, 0);
  m_pPrivateCursor->ResetPosition();
  CWnd::OnSize(nType, cx, cy);
}

/***************************************************************************/
// CPlotWnd::EraseBkgnd Erasing background        10/24/2000 - DDO
/***************************************************************************/
BOOL CPlotWnd::OnEraseBkgnd(CDC* pDC)
{
  return EraseBkgnd(pDC);
}
/***************************************************************************/
// CPlotWnd::OnEraseBkgnd Erasing background
// Allows highlighting of selected areas.
/***************************************************************************/
BOOL CPlotWnd::EraseBkgnd(CDC* pDC)
{
  CRect rWnd;
  GetClientRect(rWnd);
  if (rWnd.Width()==0)  return TRUE; // nothing to erase
  double fDataPos = GetDataPosition(rWnd.Width()); // data index of first sample to display
  DWORD dwDataFrame = AdjustDataFrame(rWnd.Width()); // number of data points to display
  ASSERT(rWnd.Width());
  double fBytesPerPix = (double)dwDataFrame / (double)rWnd.Width();

  int nHighLightPixLeft = 0;
  int nHighLightPixRight = 0;
  if ((dwDataFrame != 0) && (m_dwHighLightLength != 0))
  {
    // there is something to highlight
    if ((m_dwHighLightPosition + m_dwHighLightLength) > fDataPos)
    {
      DWORD dwHighLightPosition = m_dwHighLightPosition;
      DWORD dwHighLightLength = m_dwHighLightLength;
      if (m_dwHighLightPosition < fDataPos)
      {
        dwHighLightPosition = (DWORD)fDataPos;
        dwHighLightLength -= ((DWORD)fDataPos - m_dwHighLightPosition);
      }
      // SDM 1.06.6U4 align selection to graph aligned to pixels
      nHighLightPixLeft = round(((double)dwHighLightPosition - fDataPos) / fBytesPerPix);
      nHighLightPixRight = round(((double)(dwHighLightPosition + dwHighLightLength) - fDataPos) / fBytesPerPix);
    }
  }
  // get background color from main frame
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
  // create the background brush
  CBrush backBrush(pMainWnd->GetColors()->cPlotBkg);
  CBrush* pOldBrush = pDC->SelectObject(&backBrush);
  CRect rClip;
  pDC->GetClipBox(rClip); // get the area needed to redraw
  pDC->LPtoDP(rClip);
  if (rClip.left < nHighLightPixLeft)
  {
    // draw left of highlighted area
    int nWidth = nHighLightPixLeft - rClip.left;
    if (nWidth > rClip.Width()) nWidth = rClip.Width();
    pDC->PatBlt(rClip.left, rClip.top, nWidth, rClip.Height(), PATCOPY);
    rClip.left += nWidth;
  }
  if ((rClip.left < rClip.right) && (rClip.left < nHighLightPixRight)
    && (nHighLightPixRight - nHighLightPixLeft))
  {
    // draw highlighted area
    int nWidth = nHighLightPixRight - rClip.left;
    if (nWidth > rClip.Width()) nWidth = rClip.Width();
    COLORREF cHighlight = pMainWnd->GetColors()->cPlotHiBkg;

//    if(m_dwHighLightPosition != pView->GetStartCursorPosition() || 
//       m_dwHighLightPosition + m_dwHighLightLength != pView->GetStartCursorPosition())
//    {
//      // Light Highlight for fragment
//      cHighlight = RGB( (GetRValue(cHighlight) + GetRValue(pMainWnd->GetColors()->cPlotBkg))/2,
//        (GetGValue(cHighlight) + GetGValue(pMainWnd->GetColors()->cPlotBkg))/2,
//        (GetBValue(cHighlight) + GetBValue(pMainWnd->GetColors()->cPlotBkg))/2);
//
//    }


    CBrush highBrush(cHighlight);
    pDC->SelectObject(&highBrush);
    pDC->PatBlt(rClip.left, rClip.top, nWidth, rClip.Height(), PATCOPY);
    pDC->SelectObject(&backBrush);
    rClip.left += nWidth;
  }
  if (rClip.left < rClip.right)
  {
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
void CPlotWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
  // inform parent graph
  CGraphWnd* pWnd = (CGraphWnd*)GetParent();
  pWnd->SendMessage(WM_RBUTTONDOWN, nFlags, MAKELONG(point.x, point.y)); // send message to parent
  // handle the floating popup menu
  CMenu mPopup;
  if (mPopup.LoadMenu(((CMainFrame*)AfxGetMainWnd())->GetPopup())) // SDM 1.5Test8.5
  {
    // Show restricted submenu according to EXPERIMENTAL_ACCESS
    CMenu& pFloatingPopup = EXPERIMENTAL_ACCESS ? *mPopup.GetSubMenu(3) : *mPopup.GetSubMenu(0);
    ASSERT(pFloatingPopup.m_hMenu != NULL);
    // attach the layout menu
    CMenu* mLayout = new CLayoutMenu;
    TCHAR szString[256]; // don't change the string
    if(pFloatingPopup.GetMenuString(ID_GRAPHS_LAYOUT, szString, sizeof(szString)/sizeof(TCHAR), MF_BYCOMMAND)) // SDM 1.5Test8.5
    {
      if (mLayout) VERIFY(pFloatingPopup.ModifyMenu(ID_GRAPHS_LAYOUT, MF_BYCOMMAND | MF_POPUP, (UINT)mLayout->m_hMenu, szString));
    }
    // pop the menu up
    ClientToScreen(&point);
    pFloatingPopup.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,  AfxGetMainWnd());
    if (mLayout) delete mLayout;
  }
  CWnd::OnRButtonDown(nFlags, point);
}

/***************************************************************************/
// CPlotWnd::OnLButtonDown Mouse left button down
// Sets the start or stop cursor (or both) cursor to the mouse pointer
// position (if possible) in the plot. The graph also has to gain focus, so
// the parent graph has to be informed.
/***************************************************************************/
void CPlotWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
  //  m_MousePointerPosn = point;
  //  m_MouseButtonState = nFlags;
  // inform parent graph
  CGraphWnd* pWnd = (CGraphWnd*)GetParent();
  pWnd->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y)); // send message to parent
  // get client coordinates
  CRect rWnd;
  GetClientRect(rWnd);
  // get pointer to view
  CSaView* pView = (CSaView*)pWnd->GetParent();
  if (pView->GetDocument()->GetDataSize() > 0)
  {
    if (m_bCursors)
    {
      // if shift key not pressed -> start cursor is to move
      if (!(nFlags & MK_SHIFT))
      {
        // position and drag the start cursor
        ClientToScreen(&point);
        m_pStartCursor->ScreenToClient(&point);
        m_pStartCursor->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
      }
      else // stop cursor is to move
      {
        // position and drag the stop cursor
        ClientToScreen(&point);
        m_pStopCursor->ScreenToClient(&point);
        m_pStopCursor->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
      }
    }
    if (m_bPrivateCursor)
    {
      if (m_bHorizontalCursors)
      {
        // THIS CODE IS FOR HORIZONTAL CURSORS - TCJ 5/9/00
        //  see also CPrivateCursorWnd for 2 additional horizontal cursor functions

        // get the actual (old) position of cursor window
        CRect rLine;
        m_pPrivateCursor->GetWindowRect(rLine);
        ScreenToClient(rLine);
        // get the line position in the middle
        rLine.top += CURSOR_WINDOW_HALFWIDTH;
        rLine.bottom -= (CURSOR_WINDOW_HALFWIDTH - 1);
        // move the private cursor window
        InvalidateRect(rLine, TRUE); // redraw old cursor position
        rWnd.top = point.y - CURSOR_WINDOW_HALFWIDTH;
        rWnd.bottom = point.y + CURSOR_WINDOW_HALFWIDTH;
        m_pPrivateCursor->MoveWindow(rWnd, FALSE); // move the cursor window to the new position
        UpdateWindow(); // update this region before redrawing the cursor window
        rLine.SetRect(rWnd.left, CURSOR_WINDOW_HALFWIDTH, rWnd.right, CURSOR_WINDOW_HALFWIDTH + 1);
        //          rLine.SetRect(rWnd.left, point.y + CURSOR_WINDOW_HALFWIDTH, rWnd.right, point.y + CURSOR_WINDOW_HALFWIDTH + 1);

        m_pPrivateCursor->InvalidateRect(rLine, TRUE); // redraw new cursor line
        m_pPrivateCursor->UpdateWindow(); // update the cursor
        // and drag the cursor
        m_pPrivateCursor->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(0, CURSOR_WINDOW_HALFWIDTH));
      }
      else
      {
        // THIS CODE IS FOR VERTICAL CURSORS

        // get the actual (old) position of cursor window
        CRect rLine;
        m_pPrivateCursor->GetWindowRect(rLine);
        ScreenToClient(rLine);
        // get the line position in the middle
        rLine.left += CURSOR_WINDOW_HALFWIDTH;
        rLine.right -= (CURSOR_WINDOW_HALFWIDTH - 1);
        // move the private cursor window
        InvalidateRect(rLine, TRUE); // redraw old cursor position
        rWnd.left = point.x - CURSOR_WINDOW_HALFWIDTH;
        rWnd.right = point.x + CURSOR_WINDOW_HALFWIDTH;
        rLine.SetRect(CURSOR_WINDOW_HALFWIDTH, rWnd.top, CURSOR_WINDOW_HALFWIDTH + 1, rWnd.bottom);
        m_pPrivateCursor->InvalidateRect(rLine, TRUE); // redraw new cursor line
        m_pPrivateCursor->MoveWindow(rWnd, FALSE); // move the cursor window to the new position
        UpdateWindow(); // update this region before redrawing the cursor window
        // and drag the cursor
        m_pPrivateCursor->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(CURSOR_WINDOW_HALFWIDTH, 0));
      }
    }
  }
  CWnd::OnLButtonDown(nFlags, point);
}

/***************************************************************************/
// CPlotWnd::OnMouseMove
/***************************************************************************/
void CPlotWnd::OnMouseMove(UINT nFlags, CPoint point)
{
  m_MousePointerPosn = point;
  m_MouseButtonState = nFlags;

  CGraphWnd* pGraph = (CGraphWnd *)GetParent();
  CSaView* pView = (CSaView *)pGraph->GetParent();

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
void CPlotWnd::OnPaint()
{
  CPaintDC dc(this); // device context for painting
  CRect rWnd, rClip;
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  CSaView* pView = (CSaView*)pGraph->GetParent();

  GetClientRect(rWnd);
  dc.GetClipBox(&rClip);
  dc.LPtoDP(&rClip);

  OnDraw(&dc,rWnd,rClip, pView); // virtual
}

CPlotWnd * CPlotWnd::NewCopy(void)
{
  return NULL;
}



CString CPlotWnd::GetPlotName() const
{
  return m_szPlotName;
}



void CPlotWnd::SetPlotName(const CString & plotName)
{
  m_szPlotName = plotName;
}

/***************************************************************************/
// CPlotWnd::StandardAnimateFrame  Animate plot for a single frame
/***************************************************************************/
void CPlotWnd::StandardAnimateFrame(DWORD dwFrameIndex)
{
  m_dwAnimationFrame = dwFrameIndex;
  // Get pointer to raw waveform graph
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  CSaView* pView = (CSaView*)pGraph->GetParent();
  int nWaveGraphIndex = pView->GetGraphIndexForIDD(IDD_RAWDATA);
  CGraphWnd* pWaveGraph = pView->GetGraph(nWaveGraphIndex);
  // Highlight raw data frame for which process data is calculated
  if (pWaveGraph)
  { // raw waveform graph present
	CSaDoc* pDoc = pView->GetDocument();
	FmtParm* pFmtParm = pDoc->GetFmtParm();
    WORD wSmpSize = WORD(pFmtParm->wBlockAlign / pFmtParm->wChannels);
	CProcessFragments* pFragments = pDoc->GetFragments();
    FRAG_PARMS FragParms = pFragments->GetFragmentParms(m_dwAnimationFrame);
    DWORD dwFrameStart = FragParms.dwOffset * wSmpSize;
    DWORD dwFrameSize = (DWORD)FragParms.wLength * (DWORD)wSmpSize;
    CPlotWnd* pWavePlot = pWaveGraph->GetPlot();
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
void CPlotWnd::StandardEndAnimation()
{
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  CSaView* pView = (CSaView*)pGraph->GetParent();
  DWORD dwStartCursor = pView->GetStartCursorPosition();
  CSaDoc* pDoc = pView->GetDocument();
  FmtParm* pFmtParm = pDoc->GetFmtParm();
  WORD wSmpSize = WORD(pFmtParm->wBlockAlign / pFmtParm->wChannels);
  CProcessFragments* pFragments = pDoc->GetFragments();
  m_dwAnimationFrame = pFragments->GetFragmentIndex(dwStartCursor/wSmpSize);  // reset to start cursor fragment
  int nWaveGraphIndex = pView->GetGraphIndexForIDD(IDD_RAWDATA);
  CGraphWnd* pWaveGraph = pView->GetGraph(nWaveGraphIndex);
  if (pWaveGraph) 
  {
    CPlotWnd* pWavePlot = pWaveGraph->GetPlot();
	pWavePlot->SetHighLightArea(0, 0);
  }
  RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}


/***************************************************************************/
// CPlotWnd::GraphHasFocus  Handles plot when its graph gains or loses focus
/***************************************************************************/
void CPlotWnd::GraphHasFocus(BOOL bFocus)
{
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
  CSaView* pView = (CSaView*)pMainWnd->GetCurrSaView();
  if(!pView) return;
  if (m_bAnimationPlot)
  {
    int nWaveGraphIndex = pView->GetGraphIndexForIDD(IDD_RAWDATA);
    CGraphWnd* pWaveGraph = pView->GetGraph(nWaveGraphIndex);
    if (pWaveGraph)
	{
      BOOL bDynamicUpdate = (pView->GetGraphUpdateMode() == DYNAMIC_UPDATE);
	  if (bDynamicUpdate)
	  {
        CPlotWnd* pWavePlot = pWaveGraph->GetPlot();
        if (bFocus)
		{ // Highlight raw data frame for which formants are calculated
          CSaDoc* pDoc   = pView->GetDocument();
          FmtParm* pFmtParm = pDoc->GetFmtParm(); // get sa parameters format member data
          WORD wSmpSize = (WORD)(pFmtParm->wBlockAlign / pFmtParm->wChannels);  // calculate sample size in bytes
          CProcessFragments* pFragments = (CProcessFragments*)pDoc->GetFragments();  // data should be ready -- dynamic mode enabled
          DWORD dwFrame = m_dwAnimationFrame;
          if (dwFrame == UNDEFINED_OFFSET) dwFrame = pFragments->GetFragmentIndex(pView->GetStartCursorPosition()/wSmpSize);
          FRAG_PARMS FragParms = pFragments->GetFragmentParms(dwFrame);
          DWORD dwFrameStart = FragParms.dwOffset * wSmpSize;
          DWORD dwFrameSize = (DWORD)FragParms.wLength * (DWORD)wSmpSize;
          pWavePlot->SetHighLightArea(dwFrameStart, dwFrameStart + dwFrameSize, TRUE, TRUE);
          pWavePlot->UpdateWindow();
		}
        else if (!pView->IsAnimating())
		{ // turn off highlighted area in raw data
          pWavePlot->SetHighLightArea(0, 0);
          pWavePlot->UpdateWindow();
		}
	  }
	}
  }
}


/***************************************************************************/
// CPlotWnd::OnSysCommand System menu command
/***************************************************************************/
void CPlotWnd::OnSysCommand(UINT nID, LPARAM lParam)
{
  switch(nID)
  {
  case SC_KEYMENU:
    GetParent()->SendMessage(WM_SYSCOMMAND,nID,lParam);
    return;
  case SC_CLOSE:
    GetParent()->SendMessage(WM_SYSCOMMAND,nID,lParam);
    break;
  case SC_MAXIMIZE:
  case SC_MINIMIZE:
  case SC_RESTORE:
  default: break;
  }
  CWnd::OnSysCommand(nID, lParam);
}


void CPlotWnd::RemoveRtPlots()
{
  // do nothing.
}

bool CPlotWnd::bIsRtOverlay()
{
  return m_bRtPlot;
}

void CPlotWnd::SetRtOverlay()
{
  m_bRtPlot = true;
}

/***************************************************************************/
// CPlotWnd::CalcWaveOffsetAtPixel  Calculate wave offset from pixel position
/***************************************************************************/
DWORD CPlotWnd::CalcWaveOffsetAtPixel(CPoint pixel)
{
  // get actual data position, frame and data size and alignment
  double fDataPos;
  DWORD dwDataFrame;
  // check if area graph type
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  CSaView* pView = (CSaView*)pGraph->GetParent();
  // get client area width
  CRect rWnd;
  GetClientRect(rWnd);
  int nWidth = rWnd.Width();
  if (pGraph->IsAreaGraph())
  {
    // get necessary data from area plot
    fDataPos = GetAreaPosition();
    dwDataFrame = GetAreaLength();
  }
  else
  {
    // get necessary data from document and from view
    fDataPos = pView->GetDataPosition(nWidth); // data index of first sample to display
    dwDataFrame = pView->AdjustDataFrame(nWidth); // number of data points to display
  }
  CSaDoc* pDoc = pView->GetDocument();
  int nSmpSize = pDoc->GetFmtParm()->wBlockAlign / pDoc->GetFmtParm()->wChannels;
  // calculate data samples per pixel
  double fSamplesPerPix = nWidth ? (double)dwDataFrame / (double)(nWidth*nSmpSize) : 0.;

  // calculate the start cursor position
  DWORD dwWaveOffset = (DWORD) (nSmpSize*round(fDataPos/nSmpSize + pixel.x * fSamplesPerPix));
  return dwWaveOffset;
}

Grid CPlotWnd::GetGrid() const 
{
  return *static_cast<CMainFrame*>(AfxGetMainWnd())->GetGrid();
}



CDataSourceSimple::CDataSourceSimple(CDataProcess &cProcess) : m_cProcess(cProcess) 
{ 
  m_nSamples = m_cProcess.GetDataSize();
}

void CDataSourceSimple::GetValues(int &nFirstSample, int nLastSample, CValues &values, BOOL &bValid)
{
  bValid = TRUE;

  if (nFirstSample < 0) 
    nFirstSample = 0;
  if((nLastSample - nFirstSample)*sizeof(short int) >= m_cProcess.GetProcessBufferSize())
    nLastSample = nFirstSample + m_cProcess.GetProcessBufferSize()/(4*sizeof(short int));

  int nMax = INT_MIN;
  int nMin = INT_MAX;
  
  int nFirstIndexOffset = (nFirstSample < m_nSamples ? nFirstSample : m_nSamples - 1); 
  int nLastIndex = (nLastSample < m_nSamples ? nLastSample : m_nSamples - 1) - nFirstIndexOffset;

  short int* pData = reinterpret_cast<short*>(m_cProcess.GetProcessedDataBlock(nFirstIndexOffset*sizeof(short int), (nLastIndex+1)*sizeof(short int)));

  for (int nSampleIndex = 0; nSampleIndex <= nLastIndex; nSampleIndex++)
  {
    int nLast = pData[nSampleIndex];
    if (nLast > nMax) 
      nMax = nLast;
    if (nLast < nMin) 
      nMin = nLast;
  }

  nFirstSample = nLastSample + 1;
  values.nMax = nMax;
  values.nMin = nMin;
  values.nLast = pData[nLastIndex];
  values.nFirst = pData[0];
}

CDataSourceValidate::CDataSourceValidate(CDataProcess &cProcess, BOOL bUnset, BOOL bMissing) : 
  m_cProcess(cProcess), m_bSkipUnset(bUnset), m_bSkipMissing(bMissing)
{ 
  m_nSamples = m_cProcess.GetDataSize();
}

void CDataSourceValidate::GetValues(int &nFirstSample, int nLastSample, CValues &values, BOOL &bValid)
{
  bValid = TRUE;

  if (nFirstSample < 0) 
    nFirstSample = 0;
  if((nLastSample - nFirstSample)*sizeof(short int) >= m_cProcess.GetProcessBufferSize())
    nLastSample = nFirstSample + m_cProcess.GetProcessBufferSize()/(4*sizeof(short int));

  int nMax = INT_MIN;
  int nMin = INT_MAX;
  
  int nFirstIndexOffset = (nFirstSample < m_nSamples ? nFirstSample : m_nSamples - 1); 
  int nLastIndex = (nLastSample < m_nSamples ? nLastSample : m_nSamples - 1) - nFirstIndexOffset;

  short int* pData = reinterpret_cast<short*>(m_cProcess.GetProcessedDataBlock(nFirstIndexOffset*sizeof(short int), (nLastIndex+1)*sizeof(short int)));

  for (int nSampleIndex = 0; nSampleIndex <= nLastIndex; nSampleIndex++)
  {
    int nTemp = pData[nSampleIndex];
    if ((m_bSkipUnset && nTemp == -1) || (m_bSkipMissing && nTemp == -2))
    {
      if(nSampleIndex)
      {
        // Everything up to this point is valid
        nLastSample = nFirstSample + nSampleIndex - 1;
      }
      else
      {
        bValid = FALSE;

        while((nSampleIndex <= nLastIndex) && ((m_bSkipUnset && nTemp == -1) || (m_bSkipMissing && nTemp == -2)))
          nTemp = pData[++nSampleIndex];

        // Everything up to this point is invalid
        nLastSample = nFirstSample + nSampleIndex - 1;
      }
      nLastIndex = 0; // make sure last is set correctly
      break;
    }

    if (nTemp > nMax) 
      nMax = nTemp;
    if (nTemp < nMin) 
      nMin = nTemp;
  }

  nFirstSample = nLastSample + 1;
  values.nMax = nMax;
  values.nMin = nMin;
  values.nLast = pData[nLastIndex];
  values.nFirst = pData[0];
}

const double CYScaleSemitones::dSemitoneScale = 12.0 / log10(2.0);
const double CYScaleSemitones::dSemitoneReference =  + (69. - log10(440.0)* 12.0 / log10(2.0));

void CDrawSegment::DrawTo(int x, CDataSource::CValues &values, const CYScale &cYScale, BOOL bValid)
{
  if(bValid && values.nMax != values.nMin)
  {
    DrawTo(x, values.nFirst, cYScale, bValid);
    DrawTo(x, values.nMax, cYScale, bValid);
    DrawTo(x, values.nMin, cYScale, bValid);
    DrawTo(x, values.nLast, cYScale, bValid);
  }
  else
  {
    DrawTo(x, values.nMax, cYScale, bValid);
  }
}

void CDrawSegment::DrawTo(CDataSource::CValues &values, const CXScale &cXScale, int y, BOOL bValid)
{
  if(bValid && values.nMax != values.nMin)
  {
    DrawTo(values.nFirst, cXScale, y, bValid);
    DrawTo(values.nMax, cXScale, y, bValid);
    DrawTo(values.nMin, cXScale, y, bValid);
    DrawTo(values.nLast, cXScale, y, bValid);
  }
  else
  {
    DrawTo(values.nMax, cXScale, y, bValid);
  }
}

void CDrawSegmentSolid::DrawTo(int x, int value, const CYScale &cYScale, BOOL bValid)
{
  if(x==m_nCurrentX)
  {
    if(bValid)
    {
      if(value > m_nMaxValue)
        m_nMaxValue = value;
      if(value < m_nMinValue)
        m_nMinValue = value;
    }
  }
  else
  {
    // new x value draw old
    if(m_nOldX != m_nCurrentX && m_nMinValue <= m_nMaxValue)
    {
      CRect cRect;
      cRect.left = (m_nOldX + m_nCurrentX + 1)/2;
      cRect.right = (m_nCurrentX + x + 1)/2;

      cRect.top = cYScale.GetY(m_nMaxValue);
      cRect.bottom = cYScale.GetY(m_nMinValue);

      cRect.NormalizeRect();

      if(cRect.top > m_nReferenceY)
        cRect.top = m_nReferenceY;

      if(cRect.bottom < m_nReferenceY)
        cRect.bottom = m_nReferenceY;

      m_cDC.FillSolidRect(cRect, m_clr);
    }
    if(bValid)
    {
      m_nMinValue = m_nMaxValue = value;
    }
    else
    {
      m_nMinValue = INT_MAX;
      m_nMaxValue = INT_MIN;
    }
    m_nOldX = m_nCurrentX;
    m_nCurrentX = x;
  }
}

void CDrawSegmentLine::DrawTo(int x, int value, const CYScale &cYScale, BOOL bValid)
{
  if(bValid)
  {
    CPoint point(x,cYScale.GetY(value));
    m_pPoints[m_nSize++] = point;

    if(m_nSize >= 100)
    {
      Flush();
      m_pPoints[m_nSize++] = point;
    }
  }
  else
  {
    Flush();
  }
}

void CDrawSegmentLine::DrawTo(int x, const CXScale &cXScale, int y, BOOL bValid)
{
  if(bValid)
  {
    CPoint point(cXScale.GetX(x),y);
    m_pPoints[m_nSize++] = point;

    if(m_nSize >= 100)
    {
      Flush();
      m_pPoints[m_nSize++] = point;
    }
  }
  else
  {
    Flush();
  }
}

void CDrawSegmentLine::Flush()
{
  if(m_nSize >= 2)
    Polyline(m_cDC.m_hDC, m_pPoints, m_nSize);
  
  m_nSize = 0;
}

void CDrawSegmentSample::DrawTo(int x, CDataSource::CValues &values, const CYScale &cYScale, BOOL bValid)
{
  if(bValid && values.nMax != values.nMin)
  {
    DrawTo(x, values.nMax, cYScale, bValid);
    DrawTo(x, values.nMin, cYScale, bValid);
  }
  else
  {
    DrawTo(x, values.nMax, cYScale, bValid);
  }
}

void CDrawSegmentSample::DrawTo(int x, int value, const CYScale &cYScale, BOOL bValid)
{
  if(bValid)
  {
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

void CDrawSegmentDotOnly::DrawTo(int x, CDataSource::CValues &values, const CYScale &cYScale, BOOL bValid)
{
  if(bValid && values.nMax != values.nMin)
  {
    DrawTo(x, values.nMax, cYScale, bValid);
    DrawTo(x, values.nMin, cYScale, bValid);
  }
  else
  {
    DrawTo(x, values.nMax, cYScale, bValid);
  }
}

void CDrawSegmentDotOnly::DrawTo(int x, int value, const CYScale &cYScale, BOOL bValid)
{
  if(bValid)
  {
    int y = cYScale.GetY(value);
    CYScaleLinear cIdentity(1,0);
    CDrawSegmentLine::DrawTo(x,y, cIdentity , bValid);
    CDrawSegmentLine::DrawTo(x+1,y, cIdentity , bValid);
    CDrawSegmentLine::DrawTo(x,y, cIdentity , FALSE);
  }
}
