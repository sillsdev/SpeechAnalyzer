/////////////////////////////////////////////////////////////////////////////
// sa_annot.cpp:
// Implementation of the CXScaleWnd
//                       CAnnotationWnd
//                       CPhoneticWnd
//                       CToneWnd
//                       CPhonemicWnd
//                       COrthographicWnd
//                       CGlossWnd classes.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revisions
//   1.06.2
//         SDM modified constructors from CAnnotationWnd sub classes to call CAnnotationWnd constructor
//         SDM added SetHintUpdateBoundaries and members m_bHintUpdateBoundaries...
//   1.06.4
//         SDM removed member m_font from virtual class CAnnotationWnd
//   1.06.5
//         SDM change CAnnotationWnd::OnLMouseDown to use CSegmentSelection to select segment
//         SDM change CAnnotationWnd::OnDraw to draw 'virtual' selections
//         SDM added support for CDlgEditor
//   1.06.6
//         SDM added popup annotation editor
//   1.06.6U2
//         SDM fixed bug in drawing Annotation window which failed to draw a selected segment
//             if it was zoomed and more than 50% off the screen
//         SDM fixed a bug in the debug version which attempted to start a in-place editor
//             when there was no selection and caused an Assertion fault.
//   1.06.6U4
//         SDM fixed scrolling to properly draw
//         SDM changed floats to double to fix precision errors in scrolling
//         SDM aligned graphs to pixel boundaries, the pixel which contains GetDataPosition()
//   1.06.6U5
//         SDM added code to handle empty documents to scrolling fixes
//   1.5Test8.1
//         SDM added support for Reference annotation
//         SDM added support for m_bOverlap (HINT)
//   1.5Test8.5
//         SDM added support for dynamic popup menu changes
//         SDM disable editing in pDisplayPlot mode
//   1.5Test10.7
//         SDM moved creation of CDlgAnnotationEdit to OnCreateEdit
//   1.5Test11.1
//         SDM change CGlossWnd::OnDraw to properly draw hint during gloss change
//   07/20/2000
//         RLJ Added support for Up (/\) and Down (\/) buttons in TWC legend
//   08/11/00
//         DDO Changed the font used for the up and down arrow buttons on the
//             TWC legend. It used to be Wingdings and now it's Marlett. This
//             may be a problem in Windows 3.1, we'll see. However, in Win9x
//             it's a great solution since Windows uses that font internally
//             for things like the min, max and close buttons at the top of
//             most windows. Therefore, the font is almost guaranteed to be
//             installed.
//   08/11/00
//         DDO Moved the convert button from the TWC controls window to the
//             staff window so it's underneath the voice buttons. I also
//             made the font for the two buttons non bold so I could get
//             the word "convert" on the button. I also increased the size
//             of the play, pause, etc. buttons by 2 pixels each and the
//             voice and convert buttons by 4 (this included bumping their
//             left coordinates left by 2 pixels), also to help not crowd
//             the convert button's caption.
//   08/14/00
//         DDO Added a call to the convert function from here. It used to
//             be called in sa_g_mbt.cpp before the convert button was moved
//             from the TWC controls to the staff legend.
//   09/22/00
//         DDO Added some checks in the OnDraw for a recording graph because
//             the recording graph is an area graph but the recalc. button
//             shouldn't be displayed in the legend of the recording graph.
//   09/25/00
//         DDO Changed the way the x-scale window is drawn for the TWC
//             graph. If the magnitude window is visible, then the TWC's
//             x-scale is about the height of the magnitude window. That
//             keeps the TWC and melograms aligned.
//   09/27/00
//         DDO Added a check in the OnDraw for annotations for whether or
//             not the parent graph is a TWC graph. If it is then don't
//             draw the text.
//         DDO Added a function called GetTWCXScaleWindowHeight() which
//             contains code added on 9/25 to calculate the height of the
//             TWC's x-scale window. On 9/25 I neglected to consider the
//             affects of annotation windows being visible.
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "sa_annot.h"
#include "sa_plot.h"
#include "sa_graph.h"
#include "Segment.h"
#include "DlgAnnotationEdit.h"
#include "sa_doc.h"
#include "sa.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "math.h"
#include "sa_g_stf.h"
#include "sa_g_twc.h"
#include "Partiture.hpp"
#include "sa_g_wavelet.h"               // ARH 8/3/01  Added to use the arrow buttons on the wavelet graph
#include "GlossSegment.h"
#include "GlossNatSegment.h"
#include "ReferenceSegment.h"
#include "Process\Process.h"
#include "sa_ipa.h"
#include "PhoneticSegment.h"
#include "LegendWnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
static HINSTANCE hInstance = NULL;

#undef min // the min() macro hides the min() member function std::numeric_limits<>::min()

//###########################################################################
// CXScaleWnd
// X-Scale window contained in the graphs. It displays the x-axis dimension
// and scale for the graphs data. It has to keep track with scrolling and
// zooming.

/////////////////////////////////////////////////////////////////////////////
// CXScaleWnd message map

BEGIN_MESSAGE_MAP(CXScaleWnd, CWnd)
    ON_WM_ERASEBKGND()
    ON_WM_RBUTTONDOWN()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_PAINT()
    ON_WM_CREATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXScaleWnd construction/destruction/creation

/***************************************************************************/
// CXScaleWnd::CXScaleWnd Constructor
/***************************************************************************/
CXScaleWnd::CXScaleWnd() {
    m_nScaleMode = SCALE | NUMBERS | TIME_FROM_VIEW;
	// set divisions
    m_nGridDivisions = -1;
	// scale min value
    m_fScaleMinValue = 0;
	// scale max value
    m_fScaleMaxValue = 100;
    m_fFirstGridPos = 0;
    m_fGridDistance = 0;
    m_bRecalculate = TRUE;
    m_nWidthUsed = -1;
    m_nDivisionsUsed = -1;
    m_d3dOffset = 0.;
    m_fMinValueUsed = 0;
    m_fMaxValueUsed = 0;
}

/***************************************************************************/
// CXScaleWnd::~CXScaleWnd Destructor
/***************************************************************************/
CXScaleWnd::~CXScaleWnd() {
}

/***************************************************************************/
// CXScaleWnd::PreCreateWindow Creation
// Called from the framework before the creation of the window. Registers
// the new window class.
/***************************************************************************/
BOOL CXScaleWnd::PreCreateWindow(CREATESTRUCT & cs) {
    // register the window class
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
                                       AfxGetApp()->LoadStandardCursor(IDC_ARROW), 0, 0);
    return CWnd::PreCreateWindow(cs);
}

/***************************************************************************/
// CXScaleWnd::GetWindowHeight
// Calculating the height of the current font.
/***************************************************************************/
int CXScaleWnd::GetWindowHeight() {
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();

    //*************************************************************
    // 09/27/2000 - DDO
    // If this x-scale's graph is the TWC graph then make sure
    // the x-scale window height is large enough so the TWC plot
    // window's height is exactly the same height as the melogram
    // window's plot. All other graphs will have a fixed x-scale
    // window height.
    //*************************************************************
    if (pGraph->IsPlotID(IDD_TWC)) {
        return GetTWCXScaleWindowHeight();
    }

    // get height of the x-scale window font
    TEXTMETRIC tm;
    CDC * pDC = GetDC(); // get device context
    CFont * pOldFont = pDC->SelectObject(&m_font); // select x-scale font
    pDC->GetTextMetrics(&tm); // get text metrics
    pDC->SelectObject(pOldFont);  // set back old font
    ReleaseDC(pDC);
    return 3 * tm.tmHeight / 2 + 2; // return window height
}

/**
*  Helper function for the GetWindowHeight() function.  The TWC window is
*  a special case.  When the Melogram, TWC and Magnitude graphs are all
*  present the TWC graph is exteded to overlap the Magnitude height.
*  The extra space needs to be consumed by increasing the size of the
*  XScale window.
**/
int CXScaleWnd::GetTWCXScaleWindowHeight() {
    //***********************************************************
    // Get a pointer to the melogram's plot window.
    //***********************************************************
    CGraphWnd * pGraph    = (CGraphWnd *)GetParent();
    CSaView  *  pView     = (CSaView *)pGraph->GetParent();
    CGraphWnd * pMelGraph = pView->GraphIDtoPtr(IDD_MELOGRAM);
    if (!pMelGraph) {
        return 0;
    }
    CPlotWnd * pMelPlot = pMelGraph->GetPlot();
    if (!pMelPlot) {
        return 0;
    }

    //***********************************************************
    // Get the rectangle of the client areas of the melogram
    // and the TWC graph and calculate the difference between
    // the two heights.
    //***********************************************************
    CRect rMelWnd;
    CRect rTWCWnd;
    pMelGraph->GetClientRect(&rMelWnd);
    pGraph->GetClientRect(&rTWCWnd);
    int nHeight = rTWCWnd.Height() - rMelWnd.Height() + pMelGraph->GetXScaleWnd()->GetWindowHeight();

    return (nHeight < 0) ? 0 : nHeight;
}

/***************************************************************************/
// CXScaleWnd::SetScale Set the x-scale
// There are different modes for the x-scale scale possible. nMode sets this
// mode. If the scale range is taken from the view (normal case), then there
// is no setting necessary. Otherwise the values tell the horizontal range
// and the string will be drawn as dimension title (time for view taken
// scale). The string pointers can be NULL (no dimension drawn). The function
// returns TRUE if the legend window has to be redrawn.
/***************************************************************************/
BOOL CXScaleWnd::SetScale(int nMode, double dMinValue, double dMaxValue, TCHAR * pszDimension, int nGridDivisions, double d3dOffset) {
    BOOL bRedraw = FALSE;
    if (m_nScaleMode != nMode) {
		// set scale mode
        m_nScaleMode = nMode;
        bRedraw = TRUE;
    }
    if (m_nGridDivisions != nGridDivisions) {
		// set divisions
        m_nGridDivisions = nGridDivisions;
        bRedraw = TRUE;
    }
    if (m_d3dOffset != d3dOffset) {
        m_d3dOffset = d3dOffset;
        bRedraw = TRUE;
    }
    if (m_fScaleMinValue != dMinValue) {
		// set scale min value
        m_fScaleMinValue = dMinValue;
        bRedraw = TRUE;
    }
    if (m_fScaleMaxValue != dMaxValue) {
		// set scale max value
        m_fScaleMaxValue = dMaxValue;
        bRedraw = TRUE;
    }
    if (pszDimension) {
        if (m_szScaleDimension != pszDimension) {
			// set scale dimension text
            m_szScaleDimension = pszDimension;
            bRedraw = TRUE;
        }
    } else {
        if (!m_szScaleDimension.IsEmpty()) {
            m_szScaleDimension.Empty();
            bRedraw = TRUE;
        }
    }
    if (bRedraw) {
        m_bRecalculate = TRUE;
    }
    return bRedraw;
}

/***************************************************************************/
// CXScaleWnd::CalculateScale Calculate new scale
// Calculates the new scale parameters if necessary. If the plot window
// width, the values have changed or the flag m_bRecalculate is TRUE, a new
// calculation is needed. If the pointer to the device contaext is NULL, the
// function will get the pointer by itself to get the text metrics from the
// actual window font.
/***************************************************************************/
void CXScaleWnd::CalculateScale(CDC * pDC, int nWidth) {
    // get pointer to graph, view and document
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaView * pView = (CSaView *)pGraph->GetParent();
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
    // check if regular time scale expected
    if (m_nScaleMode & TIME_FROM_VIEW) {
        if (pGraph->IsAreaGraph()) {
            // get scale range from area plot
            m_fScaleMinValue = pDoc->GetTimeFromBytes(pGraph->GetPlot()->GetAreaPosition());
            m_fScaleMaxValue = m_fScaleMinValue + pDoc->GetTimeFromBytes(pGraph->GetPlot()->GetAreaLength());
        } else {
            // get scale range from view
            // SDM 1.06.6U4 Get value of first pixel
            DWORD dwFrame = pView->CalcDataFrame(nWidth);
            double fPos = pView->GetDataPosition(nWidth);
            m_fScaleMinValue = fPos*pDoc->GetTimeFromBytes(1);
            m_fScaleMaxValue = m_fScaleMinValue + pDoc->GetTimeFromBytes(dwFrame);
        }
        // set scale dimension
        m_szScaleDimension = "t(sec)";
    }
    if (!m_bRecalculate && (nWidth == m_nWidthUsed) && (m_nDivisionsUsed == m_nGridDivisions)
            && (m_fMinValueUsed == m_fScaleMinValue) && (m_fMaxValueUsed == m_fScaleMaxValue)) {
        return;    // scale up to date
    }
    if (nWidth <= 0) {
        return;    // no scale to draw
    }
    // scale has changed, recalculate
    m_bRecalculate = FALSE;
    m_nWidthUsed = nWidth;
    m_nDivisionsUsed = m_nGridDivisions;
    m_fMinValueUsed = m_fScaleMinValue;
    m_fMaxValueUsed = m_fScaleMaxValue;
    m_fNumbPerPix = 1000 * (m_fScaleMaxValue - m_fScaleMinValue) / (double)nWidth;
    if (m_nGridDivisions == -1) {
        TEXTMETRIC tm;
        // check if pDC known
        if (pDC) {
			// get text metrics
            pDC->GetTextMetrics(&tm);
        } else {
            // get pDC to get textmetrics
			// get device context
            CDC * pDC = GetDC();
			// select legend font
            CFont * pOldFont = pDC->SelectObject(&m_font);
			// get text metrics
            pDC->GetTextMetrics(&tm);
			// set back old font
            pDC->SelectObject(pOldFont);
            ReleaseDC(pDC);
        }
        // find minimum scale distance in measures
        DWORD dwMinScale = (DWORD)(m_fNumbPerPix * (double)(tm.tmAveCharWidth * 10));
        if (!dwMinScale) {
            dwMinScale = 1;
        }
        // find the 10 based log of this distance
        DWORD dw10Base = (DWORD)log10((double)dwMinScale);
        // now find the next appropriate scale division
        DWORD dwBase = (DWORD)pow(10, (double)dw10Base);
        if ((dwBase * 2) < dwMinScale) {
            if ((dwBase * 5) % 2) {
                dwBase *= 6;
            } else {
                dwBase *= 5;
            }
        } else {
            dwBase *= 2;
        }
        if (dwBase < dwMinScale) {
            dwBase = (DWORD)pow(10, (double)(++dw10Base));
        }
        m_fNumbPerPix /= 1000;
        m_fBase = (double)dwBase / 1000;
        // calculate grid distance
		// gridline distance in pixels
        m_fGridDistance = m_fBase / m_fNumbPerPix;

        // calculate first (leftmost) gridline position in plot window client coordinates
        DWORD dwDivisions = (DWORD)(m_fScaleMinValue / m_fBase);
        m_fPosition = (double)dwDivisions * m_fBase;
        m_fFirstGridPos = ((m_fPosition - m_fScaleMinValue) / m_fNumbPerPix);

        m_fBase /= 2;
        dwDivisions = (DWORD)(m_fScaleMinValue / m_fBase);
        m_fPosition = (double)dwDivisions * m_fBase;
        m_bLargeLine = TRUE;
        if (dwDivisions % 2) {
            m_bLargeLine = FALSE;
        }
    } else {
        m_fNumbPerPix /= 1000;
        m_fBase = (m_fScaleMaxValue - m_fScaleMinValue)/m_nGridDivisions;
        // calculate first (leftmost) gridline position in plot window client coordinates
        m_fPosition = m_fScaleMinValue;
        m_fFirstGridPos = 0.;

        m_fBase /= 2;
        m_bLargeLine = TRUE;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CXScaleWnd message handlers

/***************************************************************************/
// CXScaleWnd::OnCreate Window creation
// Creation of the legend font, used in the window.
/***************************************************************************/
int CXScaleWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }
    // create legend font
    LPCTSTR pszName = _T("MS Sans Serif");
    m_font.CreateFont(15, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                      ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                      DEFAULT_QUALITY, (VARIABLE_PITCH | FF_ROMAN), pszName);
    return 0;
}

/***************************************************************************/
// CXScaleWnd::OnPaint
// Paints using OnDraw, which is shared with OnPrint
/***************************************************************************/
void CXScaleWnd::OnPaint() {
    CRect dummyRect1(0,0,0,0);
    CRect dummyRect2(0,0,0,0);
	// device context for painting
    CPaintDC dc(this); 
    OnDraw(&dc,dummyRect1,dummyRect2);
}

/***************************************************************************/
// CXScaleWnd::OnDraw Drawing
/***************************************************************************/
void CXScaleWnd::OnDraw(CDC * pDC,
                        const CRect & printRect,
                        const CRect & printPlotWnd) {
    // get pointer to graph and document
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();

    // x scale doesn't make sense for TWC graph.
    if (pGraph->IsPlotID(IDD_TWC)) {
        return;
    }
    CSaView * pView = (CSaView *)pGraph->GetParent();
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();

    // select x-scale font
    CFont * pOldFont = pDC->SelectObject(&m_font);
    // get text metrics
    TEXTMETRIC tm;
    pDC->GetTextMetrics(&tm);

    // get coordinates of plot window
    CRect rPlotWnd;

    if (pDC->IsPrinting()) {
        rPlotWnd = printPlotWnd;
    } else {
        pGraph->GetPlot()->GetClientRect(rPlotWnd);
    }

    if ((rPlotWnd.Height() <= 0) || (pDoc->GetDataSize() == 0)) {
        pDC->SelectObject(pOldFont); // set back old font
        return; // nothing to draw
    }
    CalculateScale(pDC, rPlotWnd.Width()); // calculate scale
    // get window coordinates
    CRect rWnd;

    if (pDC->IsPrinting()) {
        rWnd = printRect;
    } else {
        GetClientRect(rWnd);
    }


    if (rWnd.Width() == 0) {
        pDC->SelectObject(pOldFont);  // set back old font
        return; // nothing to draw
    }
    // draw 3D window border
	// get pointer to colors from main frame
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    Colors * pColors = pMainWnd->GetColors();
    CPen penLtgray(PS_SOLID, 1, pColors->cSysBtnHilite);
    CPen * pOldPen = pDC->SelectObject(&penLtgray);
    pDC->MoveTo(rWnd.left, rWnd.top);
    pDC->LineTo(rWnd.right, rWnd.top);
    pDC->SelectObject(pOldPen);
    // set font colors
    pDC->SetTextColor(pColors->cScaleFont);
	// letters may overlap, so they must be transparent
    pDC->SetBkMode(TRANSPARENT);
	// don't draw over the border
    rWnd.top += 2;
    rWnd.bottom -= 2;
    if ((m_nScaleMode != NO_SCALE) && (pDoc->GetDataSize() > 0)) {
        if (rWnd.Width() < (5 * tm.tmAveCharWidth)) {
			// set back old font
            pDC->SelectObject(pOldFont);
			// not enough space to draw scale
            return;
        }
        // draw the dimension
        int nDimLeftLimit = 0;
        int nDimRightLimit = 0;
        if ((!m_szScaleDimension.IsEmpty()) && (!pGraph->IsPlotID(IDD_TWC)) &&
            (!pGraph->HasLegend() || (pGraph->IsAreaGraph() && !pGraph->IsPlotID(IDD_RECORDING)))) {
            // create the bold font
            LOGFONT logFont;
			// fill up logFont
            m_font.GetObject(sizeof(LOGFONT), (void *)&logFont);
            logFont.lfWeight = FW_BOLD;
            CFont bFont;
			// create the bold font
            bFont.CreateFontIndirect(&logFont);
			// select the bold font
            pDC->SelectObject(&bFont);
            pDC->DrawText(m_szScaleDimension, m_szScaleDimension.GetLength(), rWnd, DT_SINGLELINE | DT_BOTTOM | DT_CENTER | DT_NOCLIP);
            nDimRightLimit = (m_szScaleDimension.GetLength() + 2 + 8) * tm.tmAveCharWidth;
            nDimLeftLimit = rWnd.Width() / 2 - nDimRightLimit / 2;
            nDimRightLimit += nDimLeftLimit;
			// set back normal text font
            pDC->SelectObject(&m_font);
        }
        // create line pen
        CPen pen(PS_SOLID, 1, pColors->cScaleLines);
        CPen * pOldPen = pDC->SelectObject(&pen);
        // draw the scale
		// horizontal line vertical position
        int nVertPos = rWnd.top;
        // draw the horizontal line
        pDC->MoveTo(rWnd.left, nVertPos);
        pDC->LineTo(rWnd.left + round2Int(rWnd.Width()*(1 - m_d3dOffset)), nVertPos);
        if (m_nScaleMode & SCALE) {
            // draw the vertical lines and numbers
            int nTextWidth2 = tm.tmAveCharWidth * 4;
			// left text position limit
            int nTextLeftLimit = nTextWidth2;
			// right text position limit
            int nTextRightLimit = rWnd.right - nTextWidth2;
            double fPosition = m_fPosition;
            BOOL bLargeLine = m_bLargeLine;
            while (m_fBase > 0 ? fPosition <= m_fScaleMaxValue : fPosition >= m_fScaleMaxValue) {
                // calculate line position
                int nPos = (int)((1 - m_d3dOffset)*(fPosition - m_fScaleMinValue) / m_fNumbPerPix);
                // draw the line
                if (bLargeLine) {
                    // draw a large line
                    pDC->MoveTo(nPos, nVertPos);
                    pDC->LineTo(nPos, nVertPos + 4);
                    // draw the number
                    if ((m_nScaleMode & NUMBERS) && (nPos > nTextLeftLimit) && (nPos < nTextRightLimit)
                            && ((nPos < nDimLeftLimit) || (nPos >= nDimRightLimit))) {
                        CRect rText(nPos - nTextWidth2, nVertPos + 3, nPos + nTextWidth2, rWnd.bottom);
                        TCHAR szText[16];
                        if (m_nScaleMode & TIME_FROM_VIEW) {
                            swprintf_s(szText, _T("%8.3f"), fPosition);
                        } else {
                            swprintf_s(szText, _countof(szText), _T("%5.0f"), fPosition);
                        }
                        pDC->DrawText(szText, -1, rText, DT_SINGLELINE | DT_TOP | DT_CENTER | DT_NOCLIP);
                    }
                } else {
                    // draw a small line
                    pDC->MoveTo(nPos, nVertPos);
                    pDC->LineTo(nPos, nVertPos + 3);
                }
                fPosition += m_fBase;
                bLargeLine = !bLargeLine;
            }
        }
        pDC->SelectObject(pOldPen);
    }
	// set back old font
    pDC->SelectObject(pOldFont);
}

/***************************************************************************/
// CXScaleWnd::OnEraseBkgnd Erasing background
/***************************************************************************/
BOOL CXScaleWnd::OnEraseBkgnd(CDC * pDC) {
    // get background color from main frame
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    // create the background brush
    CBrush backBrush(pMainWnd->GetColors()->cScaleBkg);
    CBrush * pOldBrush = pDC->SelectObject(&backBrush);
    CRect rClip;
    pDC->GetClipBox(&rClip); // erase the area needed
    pDC->PatBlt(rClip.left, rClip.top, rClip.Width(), rClip.Height(), PATCOPY);
    pDC->SelectObject(pOldBrush);
    return TRUE;
}

/***************************************************************************/
// CXScaleWnd::OnRButtonDown Mouse right button down
// This event initiates a popup menu. The graph also has to get focus, so
// the parent graph is informed to do this.
/***************************************************************************/
void CXScaleWnd::OnRButtonDown(UINT nFlags, CPoint point) {

    // inform parent graph
    GetParent()->SendMessage(WM_RBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
    // handle the floating popup menu
	CMainFrame * pMainWnd = (CMainFrame*)AfxGetMainWnd();
    CMenu menu;
    if (menu.LoadMenu(pMainWnd->GetPopup())) {
        CMenu & popup = *menu.GetSubMenu(0);
        ASSERT(popup.m_hMenu != NULL);
        // attach the layout menu
        CLayoutMenu layout;
		// don't change the string
        TCHAR szString[256]; 
		//SDM 1.5Test8.5
        if (popup.GetMenuString( ID_GRAPHS_LAYOUT, szString, sizeof(szString)/sizeof(TCHAR), MF_BYCOMMAND)) { 
            BOOL bResult = popup.ModifyMenu(ID_GRAPHS_LAYOUT, MF_BYCOMMAND | MF_POPUP, (UINT)layout.m_hMenu, szString);
			ASSERT(bResult);
        }
        // pop the menu up
        ClientToScreen(&point);
        popup.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, pMainWnd);
    }
    CWnd::OnRButtonDown(nFlags, point);
}

/***************************************************************************/
// CXScaleWnd::OnLButtonDown Mouse left button down
// This event should set the focus to the parent, so the window informs the
// parent graph to do this.
/***************************************************************************/
void CXScaleWnd::OnLButtonDown(UINT nFlags, CPoint point) {
    // inform parent graph
    CGraphWnd * pWnd = (CGraphWnd *)GetParent();
    pWnd->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y)); // send message to parent
    CWnd::OnLButtonDown(nFlags, point);
}

/***************************************************************************/
// CXScaleWnd::OnMouseMove Mouse move
// Detects mouse movement outside the plot area
/***************************************************************************/
void CXScaleWnd::OnMouseMove(UINT nFlags, CPoint point) {
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CPlotWnd * pPlot = pGraph->GetPlot();
    CSaView * pView = (CSaView *)pGraph->GetParent();
    pPlot->SetMousePointerPosition(CPoint(UNDEFINED_OFFSET, UNDEFINED_OFFSET)); // clear mouse position
    pPlot->SetMouseButtonState(0);  // reset state
    const BOOL bForceUpdate = TRUE;
    pGraph->UpdateStatusBar(pView->GetStartCursorPosition(), pView->GetStopCursorPosition(), bForceUpdate);
    CWnd::OnMouseMove(nFlags, point);
}

//###########################################################################
// CAnnotationWnd
// Base class for all the annotation windows in the graph. The windows are
// related closely to the segment (CSegment) classes owned by the document.

/////////////////////////////////////////////////////////////////////////////
// CAnnotationWnd message map

BEGIN_MESSAGE_MAP(CAnnotationWnd, CWnd)
    ON_WM_ERASEBKGND()
    ON_WM_RBUTTONDOWN()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_PAINT()
    ON_WM_CREATE()
    ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnnotationWnd construction/destruction/creation

CAnnotationWnd::CAnnotationWnd(int nIndex) {
	// index of annotation window
    m_nIndex = nIndex; 
    m_bHintUpdateBoundaries = false;
    m_bTranscriptionBoundary = false;
	m_bHoverOver = false;
}

/***************************************************************************/
// CAnnotationWnd::PreCreateWindow Creation
// Called from the framework before the creation of the window. Registers
// the new window class.
/***************************************************************************/
BOOL CAnnotationWnd::PreCreateWindow(CREATESTRUCT & cs) {
    // register the window class
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
                                       AfxGetApp()->LoadStandardCursor(IDC_ARROW), 0, 0);
    return CWnd::PreCreateWindow(cs);
}

/***************************************************************************/
// CAnnotationWnd::OnCreate Window Creation
// Creation of the font and setting the font colors, used in the
// window.
/***************************************************************************/
int CAnnotationWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }
    return 0;
}

/***************************************************************************/
// CAnnotationWnd::GetFont
/***************************************************************************/
CFont * CAnnotationWnd::GetFont() {
    // get pointer to graph, view and document
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaView * pView = (CSaView *)pGraph->GetParent();
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
    return pDoc->GetFont(m_nIndex);
}

/***************************************************************************/
// CAnnotationWnd::GetWindowHeight
// Calculating the height of the current font of this window. If the window
// is the tone window, the font height does not include the descenders,
// because tone never has descenders.
/***************************************************************************/
int CAnnotationWnd::GetWindowHeight() {
    // get height of the annotation window font
    TEXTMETRIC tm;
	// get device context
    CDC * pDC = GetDC(); 
    // SDM 1.06.4
	// select annotation font
    CFont * pOldFont = pDC->SelectObject(GetFont()); 
	// get text metrics
    pDC->GetTextMetrics(&tm); 
	// set back old font
    pDC->SelectObject(pOldFont);  
    ReleaseDC(pDC);
	// return font height + border space
    return tm.tmHeight + 3; 
}

/***************************************************************************/
// CAnnotationWnd::OnPaint
// Paints using OnDraw, which is shared with OnPrint
/***************************************************************************/
void CAnnotationWnd::OnPaint() {
	// needed for second OnDraw parameter
    CRect dummyRect(0,0,0,0);
    // which is only really used for printing
	// device context for painting
    CPaintDC dc(this); 
	// virtual
    OnDraw(&dc,dummyRect);
}

/***************************************************************************/
// CAnnotationWnd::AnnotationStandardPaint Standard painting of annotation
// This function may be used for classic non text annotation windows.
/***************************************************************************/
void CAnnotationWnd::OnDraw(CDC * pDC, const CRect & printRect) {

    // get window coordinates
    CRect rWnd;
	// get invalid region
    CRect rClip; 

    if (pDC->IsPrinting()) {
        rClip = printRect;
        rWnd  = printRect;
    } else {
        GetClientRect(rWnd);
        pDC->GetClipBox(&rClip);
        pDC->LPtoDP(&rClip);
    }

    if (rWnd.Width() == 0) {
		// nothing to draw
        return;    
    }

    // get pointer to graph, view and document
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaView * pView = (CSaView *)pGraph->GetParent();
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();

    // select annotation font
    //SDM 1.06.4
    CFont * pOldFont = pDC->SelectObject(GetFont());
    // get text metrics
    TEXTMETRIC tm;
    pDC->GetTextMetrics(&tm);

    // set font colors
	// get pointer to colors from main frame
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd(); 
    Colors * pColors = pMainWnd->GetColors();

	// set font color
    pDC->SetTextColor(pColors->cAnnotationFont[m_nIndex]); 
	// letters may overlap, so they must be transparent
    pDC->SetBkMode(TRANSPARENT); 
    // draw 3D window border
    CPen penDkgray(PS_SOLID, 1, pColors->cSysBtnShadow);
    CPen penLtgray(PS_SOLID, 1, pColors->cSysBtnHilite);
    CPen * pOldPen = pDC->SelectObject(&penDkgray);

    // draw bottom border
    pDC->MoveTo(rClip.left, rWnd.bottom - 1);
    pDC->LineTo(rClip.right, rWnd.bottom - 1);
    pDC->SelectObject(&penLtgray);
    pDC->MoveTo(rClip.left, rWnd.top);
    pDC->LineTo(rClip.right, rWnd.top);
    pDC->SelectObject(pOldPen);

    //*******************************************************
    // 09/27/2000 - DDO If the graph is the TWC graph
    // then we don't want to draw the annotation text.
    //*******************************************************
    if (pGraph->IsPlotID(IDD_TWC)) {
		// set back old font
        pDC->SelectObject(pOldFont);  
        return;
    }

    // SDM 1.06.6
    // Exclude Boundaries
    rWnd.SetRect(rWnd.left, rWnd.top + 1, rWnd.right, rWnd.bottom - 1);

    // check if area graph type
    double fDataStart;
    DWORD dwDataFrame;
    if (pGraph->IsAreaGraph()) {
        // get necessary data from area plot
        fDataStart = pGraph->GetPlot()->GetAreaPosition();
        dwDataFrame = pGraph->GetPlot()->GetAreaLength();
    } else {
        // check if graph has private cursor
        if (pGraph->HasPrivateCursor()) {
            // get necessary data from between public cursors
            WORD wSmpSize = WORD(pDoc->GetSampleSize());
			// data index of first sample to display
            fDataStart = (double)pView->GetStartCursorPosition(); 
			// number of data points to display
            dwDataFrame = pView->GetStopCursorPosition() - (DWORD)fDataStart + wSmpSize; 
        } else {
            // get necessary data from document and from view
			// data index of first sample to display
            fDataStart = pView->GetDataPosition(rWnd.Width());
			// number of data points to display
            dwDataFrame = pView->CalcDataFrame(rWnd.Width()); 
        }
    }
    if (dwDataFrame == 0) {
		// nothing to draw
        return;
    }

    // calculate the number of data samples per pixel
    ASSERT(rWnd.Width());
    double fBytesPerPix = (double)dwDataFrame / (double)rWnd.Width();

    // get pointer to annotation offset and duration arrays
    CSegment * pSegment = pDoc->GetSegment(m_nIndex);

    // get pointer to annotation string
    if (pSegment->GetOffsetSize()>0) {

        // string is not empty
        // position prepare loop. Find first char to display in clipping rect
        double fStart = fDataStart + (double)(rClip.left - tm.tmAveCharWidth) * fBytesPerPix;

        int nLoop = 0;
        if ((fStart > 0) && (pSegment->GetOffsetSize() > 1)) {
            for (nLoop = 1; nLoop < pSegment->GetOffsetSize(); nLoop++) {
                // SDM 1.06.6U2
                if ((double)(pSegment->GetStop(nLoop)) > fStart) {
                    // first char must be at lower position
                    nLoop--; // this is it
                    break;
                }
            }
        }

        if (nLoop < pSegment->GetOffsetSize()) {
            // there is something to display
            // display loop
            int nDisplayPos; // start position in pixels to display character(s)
            // Create Font For "*"
            CFont cFontAsterisk;
            cFontAsterisk.CreateFont(tm.tmHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                                     CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE|DEFAULT_PITCH,_T("MS Sans Serif"));
            do {
                //SDM 1.06.6U4
                DWORD offset = pSegment->GetOffset(nLoop);
                nDisplayPos = round2Int((offset - fDataStart) / fBytesPerPix);
                // check if the character is selected
                BOOL bSelect = FALSE;
                if (pSegment->GetSelection() == nLoop) {
                    bSelect = TRUE;
                }

                // put all characters width same offset in one string
                CString szAnnot = pSegment->GetText(nLoop);

                // calculate duration
                int nDisplayStop = round2Int((pSegment->GetStop(nLoop) - fDataStart)/ fBytesPerPix);

                nLoop++;

                // SDM 1.5Test8.1
                if (m_bHintUpdateBoundaries) { // Show New Boundaries
                    if (bSelect) {
                        nDisplayPos = round2Int((m_dwHintStart - fDataStart)/ fBytesPerPix);
                        nDisplayStop = round2Int((m_dwHintStop - fDataStart)/ fBytesPerPix);
                    } else if (((m_nIndex == PHONETIC) || ((m_nIndex >= MUSIC_PL1) && (m_nIndex <= MUSIC_PL4))) && (!m_bOverlap)) {
                        if (pSegment->GetSelection() == (nLoop)) { // Segment before selected segment
                            nDisplayStop = round2Int((m_dwHintStart - fDataStart)/ fBytesPerPix);
                        } else if ((pSegment->GetPrevious(nLoop)!=-1) &&
                                   (pSegment->GetSelection() == pSegment->GetPrevious(pSegment->GetPrevious(nLoop)))) { // Segment after selected segment
                            nDisplayPos = round2Int((m_dwHintStop - fDataStart)/ fBytesPerPix);
                        }
                    }
                }
                if ((nDisplayStop - nDisplayPos) < 2) {
                    nDisplayStop++;    // must be at least 2 to display a point
                }
                if ((nDisplayStop - nDisplayPos) < 2) {
                    nDisplayPos--;    // must be at least 2 to display a point
                }

                // set rectangle to display string centered within
                rWnd.SetRect(nDisplayPos, rWnd.top, nDisplayStop, rWnd.bottom);
                // highlight background if selected character
                COLORREF normalTextColor = pDC->GetTextColor();
                if (bSelect) {
                    normalTextColor = pDC->SetTextColor(pColors->cSysColorHiText); // set highlighted text
                    CBrush brushHigh(pColors->cSysColorHilite);
                    CPen penHigh(PS_SOLID, 1, pColors->cSysColorHilite);
                    CBrush * pOldBrush = (CBrush *)pDC->SelectObject(&brushHigh);
                    CPen * pOldPen = pDC->SelectObject(&penHigh);
                    pDC->Rectangle(rWnd.left, rWnd.top - 1, rWnd.right, rWnd.bottom);
                    pDC->SelectObject(pOldBrush);
                    pDC->SelectObject(pOldPen);
                }

                DrawTranscriptionBorders(pDC,rWnd,pColors);

                // check, if there is enough space to display the character(s)
                BOOL bNotEnough = (nDisplayStop - nDisplayPos) <= tm.tmAveCharWidth;
                if (!bNotEnough) {
                    bNotEnough = ((nDisplayStop - nDisplayPos) <= (szAnnot.GetLength() * tm.tmAveCharWidth));
                }

                if (bNotEnough) {
                    // not enough space, draw dot or star with MS Sans Serif
                    pDC->SelectObject(cFontAsterisk);
                    if (m_nIndex == ORTHO) {
                        pDC->DrawText(_T("*"), 1, rWnd, DT_VCENTER | DT_SINGLELINE | DT_CENTER | DT_NOCLIP);
                    } else {
                        pDC->DrawText(_T("."), 1, rWnd, DT_VCENTER | DT_SINGLELINE | DT_CENTER | DT_NOCLIP);
                    }
                    //SDM 1.06.4
                    // reselect specific annotation font
                    pDC->SelectObject(GetFont());
                } else {
                    // enough space to display character(s), draw the string
                    pDC->DrawText(szAnnot, szAnnot.GetLength(), rWnd, DT_VCENTER | DT_SINGLELINE | DT_CENTER | DT_NOCLIP);
                }
                if (bSelect) {
                    pDC->SetTextColor(normalTextColor);    // set back old text color
                }
            } while ((nDisplayPos < rClip.right) && (nLoop < pSegment->GetOffsetSize()));
        }
    }

    //SDM 1.06.5
    //keep up to date of changes force redraw of deselected virtual selections
    // get current selection information
    pView->UpdateSelection();

    // Show virtual selection (No text just highlight)
    if ((pView->IsSelectionVirtual()) && (pView->GetSelectionIndex() == m_nIndex)) {
        int nStart = int(((double)pView->GetSelectionStart() - fDataStart)/ fBytesPerPix);
        int nStop = int(((double)pView->GetSelectionStop() - fDataStart)/ fBytesPerPix + 1);
        rWnd.SetRect(nStart, rWnd.top, nStop, rWnd.bottom);
        CBrush brushBk(pColors->cSysColorHilite);
        CPen penHigh(PS_INSIDEFRAME, 1, pColors->cSysColorHilite);
        CBrush * pOldBrush = (CBrush *)pDC->SelectObject(&brushBk);
        CPen * pOldPen = pDC->SelectObject(&penHigh);
        pDC->Rectangle(rWnd.left, rWnd.top + 1, rWnd.right, rWnd.bottom - 1);
        pDC->SelectObject(pOldBrush);
        pDC->SelectObject(pOldPen);
    }
    pDC->SelectObject(pOldFont);  // set back old font
}

// SDM 1.06.2
// SDM 1.5Test8.1 modified
/***************************************************************************/
// CAnnotationWnd::SetHintUpdateBoundaries Set Hint Data and Invalidate
/***************************************************************************/
void CAnnotationWnd::SetHintUpdateBoundaries(bool bHint, DWORD dwStart, DWORD dwStop, bool bOverlap) {

    // get pointer to graph, view and document
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaView * pView = (CSaView *)pGraph->GetParent();
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
    CSegment * pSegment = pDoc->GetSegment(m_nIndex);

    if (bHint==TRUE) {
        pSegment->LimitPosition(pDoc,dwStart, dwStop, (bOverlap)?CSegment::LIMIT_MOVING_BOTH:CSegment::LIMIT_MOVING_BOTH_NO_OVERLAP);
        if (pSegment->CheckPosition(pDoc, dwStart, dwStop, CSegment::MODE_EDIT, bOverlap)==-1) {
            bHint=FALSE;
        }
    }

    if ((m_bHintUpdateBoundaries != bHint) ||
            (m_dwHintStart!=dwStart) ||
            (m_dwHintStop != dwStop) ||
            (bOverlap != m_bOverlap)) {
		// If change
        if ((bHint) || (m_bHintUpdateBoundaries)) {
			// If hint drawn or will be drawn
            InvalidateRect(NULL);
        }
		// request for draw hint of updated boundaries
        m_bHintUpdateBoundaries = bHint;
        m_dwHintStart = dwStart;
        m_dwHintStop = dwStop;
        m_bOverlap = bOverlap;
    }
}

/***************************************************************************/
// CAnnotationWnd::OnEraseBkgnd Erasing background
/***************************************************************************/
BOOL CAnnotationWnd::OnEraseBkgnd(CDC * pDC) {
    
	// get background color from main frame
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    // create the background brush
    CBrush backBrush(pMainWnd->GetColors()->cAnnotationBkg[m_nIndex]);
    CBrush * pOldBrush = pDC->SelectObject(&backBrush);
    CRect rClip;
    pDC->GetClipBox(&rClip); // erase the area needed
    pDC->PatBlt(rClip.left, rClip.top, rClip.Width(), rClip.Height(), PATCOPY);

    pDC->SelectObject(pOldBrush);
    return TRUE;
}

/***************************************************************************/
// CAnnotationWnd::OnRButtonDown Mouse right button down
// This event initiates a popup menu. The graph also has to get focus, so
// the parent graph is informed to do this.
/***************************************************************************/
void CAnnotationWnd::OnRButtonDown(UINT nFlags, CPoint point) {

    // inform parent graph
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    pGraph->SendMessage(WM_RBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
    if (pGraph->IsPlotID(IDD_TWC)) {
        CWnd::OnRButtonDown(nFlags, point);
        return;
    }
    // get pointer to view
    CSaView * pView = (CSaView *)pGraph->GetParent();
    // get pointer to segment object
    CSegment * pSegment = pView->GetDocument()->GetSegment(m_nIndex);
    // if there is no selection in this window, deselect all the others too
    if (pSegment->GetSelection() == -1) {
        pView->ChangeAnnotationSelection(pSegment, -1, 0, 0);
    }
    // handle the floating popup menu
	CMainFrame * pMainWnd = (CMainFrame*)AfxGetMainWnd();
    CMenu menu;
    if (menu.LoadMenu(pMainWnd->GetPopup())) {
        CMenu & popup = *menu.GetSubMenu(1);
        ASSERT((&popup != NULL) && (popup.m_hMenu != NULL));
        if ((&popup != NULL) && (popup.m_hMenu != NULL)) {
			pGraph->SetPopupMenuLocation(point);
            ClientToScreen(&point);
            popup.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, pMainWnd);
        }
    }
    CWnd::OnRButtonDown(nFlags, point);
}

/***************************************************************************/
// CAnnotationWnd::OnLButtonDown Mouse left button down
// This event should set the focus to the parent, so the window informs the
// parent graph to do this. It selects the clicked segment and informs the
// view.
/***************************************************************************/
void CAnnotationWnd::OnLButtonDown(UINT nFlags, CPoint point) {

    // inform parent plot
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
	// send message to parent
    pGraph->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
    if (pGraph->IsPlotID(IDD_TWC)) {
        CWnd::OnLButtonDown(nFlags, point);
        return;
    }

    // get pointer to view and to document
    CSaView * pView = (CSaView *)pGraph->GetParent();
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
    // find out, which character has been clicked
    CRect rWnd;
    GetClientRect(rWnd);
    // check if area graph type
    double fDataStart;
    DWORD dwDataFrame;
    if (pGraph->IsAreaGraph()) {
        // get necessary data from area plot
        fDataStart = pGraph->GetPlot()->GetAreaPosition();
        dwDataFrame = pGraph->GetPlot()->GetAreaLength();
    } else {
        // check if graph has private cursor
        if (pGraph->HasPrivateCursor()) {
            // get necessary data from between public cursors
            WORD wSmpSize = WORD(pDoc->GetSampleSize());
			// data index of first sample to display
            fDataStart = (LONG)pView->GetStartCursorPosition();
			// number of data points to display
            dwDataFrame = pView->GetStopCursorPosition() - (DWORD)fDataStart + wSmpSize;
        } else {
            // get necessary data from document and from view
			// data index of first sample to display
            fDataStart = pView->GetDataPosition(rWnd.Width());
			// number of data points to display
            dwDataFrame = pView->CalcDataFrame(rWnd.Width());
        }
    }
	
	// data is available
    if (dwDataFrame != 0) { 
        ASSERT(rWnd.Width());
        double fBytesPerPix = (double)dwDataFrame / (double)rWnd.Width();
        // calculate clicked data position
        LONG dwPosition = (LONG)((double)point.x * fBytesPerPix + (double)fDataStart);
        //SDM 1.06.5
        //SDM 1.06.6
        //SDM 1.5Test8.5
        int index = pView->GetSelectionIndex();
        BOOL selected = !pView->SelectFromPosition(m_nIndex, dwPosition, false);
        TRACE("selection_index=%d selectpos=%d\n",index,selected);

        // SDM 1.06.6U2
        if (pView->GetSelectionIndex() != -1) {
            m_nSelectTickCount = GetTickCount();
        } else {
            // disable slow click (timed out)
            m_nSelectTickCount = GetTickCount() - DWORD(SLOW_CLICK_TIME_LIMIT * 1000);
        }
    }
    CWnd::OnLButtonDown(nFlags, point);
}

/***************************************************************************/
// CAnnotationWnd::OnMouseMove Mouse move
// Detects mouse movement outside the plot area
/***************************************************************************/
void CAnnotationWnd::OnMouseMove(UINT nFlags, CPoint point) {

	//CSaApp * pApp = (CSaApp*)AfxGetApp();
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CPlotWnd * pPlot = pGraph->GetPlot();
    CSaView * pView = (CSaView *)pGraph->GetParent();
	//CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();

	// clear mouse position
    pPlot->SetMousePointerPosition(CPoint(UNDEFINED_OFFSET, UNDEFINED_OFFSET)); 
	// reset state
    pPlot->SetMouseButtonState(0);

    const BOOL bForceUpdate = TRUE;
    pGraph->UpdateStatusBar(pView->GetStartCursorPosition(), pView->GetStopCursorPosition(), bForceUpdate);
    CWnd::OnMouseMove(nFlags, point);
}

//###########################################################################
// CAnnotationWnd::OnLButtonDblClk
// disable slow click after dblClk
//###########################################################################
void CAnnotationWnd::OnLButtonDblClk(UINT nFlags, CPoint point) {

    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    if (pGraph->IsPlotID(IDD_TWC)) {
        CWnd::OnLButtonDblClk(nFlags, point);
        return;
    }
	CSaView * pView = (CSaView *)pGraph->GetParent();
	// if nothing selected select it
	if (pView->GetDocument()->GetSegment(m_nIndex)->GetSelection() == -1) {
		// send message to parent
		SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));    
	}
	AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_EDIT_EDITOR, 0L);
	// disable slow click (timed out)
	m_nSelectTickCount = GetTickCount() - DWORD(SLOW_CLICK_TIME_LIMIT * 1000);
    CWnd::OnLButtonDblClk(nFlags, point);
}

// SDM 1.5Test10.7
//###########################################################################
// CAnnotationWnd::CreateEdit
//###########################################################################
void CAnnotationWnd::OnCreateEdit(const CString * szInitialString) {

    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    if (pGraph->IsPlotID(IDD_TWC)) {
        return;
    }
    CSaView * pView = (CSaView *)pGraph->GetParent();

    // Slow click show annotation edit dialog over selection
    CRect rWnd;
    GetClientRect(rWnd);
    // check if area graph type
    double fDataStart = 0;
    DWORD dwDataFrame = 0;
    if (pGraph->IsAreaGraph()) {
        // get necessary data from area plot
        fDataStart = pGraph->GetPlot()->GetAreaPosition();
        dwDataFrame = pGraph->GetPlot()->GetAreaLength();
    } else {
        // check if graph has private cursor
        if (pGraph->HasPrivateCursor()) {
            // get necessary data from between public cursors
            CSaDoc * pDoc = pView->GetDocument();
            WORD wSmpSize = WORD(pDoc->GetSampleSize());
			// data index of first sample to display
            fDataStart = (LONG)pView->GetStartCursorPosition(); 
			// number of data points to display
            dwDataFrame = pView->GetStopCursorPosition() - (DWORD)fDataStart + wSmpSize; 
        } else {
            // get necessary data from document and from view
			// data index of first sample to display
            fDataStart = pView->GetDataPosition(rWnd.Width());
			// number of data points to display
            dwDataFrame = pView->CalcDataFrame(rWnd.Width());
        }
    }

    // data is available
    if (dwDataFrame != 0) {
        ASSERT(rWnd.Width());
        double fBytesPerPix = (double)dwDataFrame / (double)rWnd.Width();
        // Calculate Window Position
        GetClientRect(rWnd);
        CRect rEdit(rWnd);
        rEdit.left = int((pView->GetSelectionStart() - fDataStart)/fBytesPerPix + 0.5);
        rEdit.right = int(((double)pView->GetSelectionStop() - (double)fDataStart)/fBytesPerPix + 1.5);
        rEdit.IntersectRect(rWnd,rEdit);
        if (rEdit.Width() && dwDataFrame) {
            if (rEdit.Width() < 2*rEdit.Height()) {
                // Edit box too small
                int nMore = 2*rEdit.Height()  - rEdit.Width();
                rEdit.left -= nMore/2;
                rEdit.right += nMore/2;
                rEdit.IntersectRect(rWnd,rEdit);
            }
            ClientToScreen(rEdit);
            // ShowWindow
            CDlgAnnotationEdit * pAnnotationEdit = new CDlgAnnotationEdit();
            pAnnotationEdit->Create(CDlgAnnotationEdit::IDD);
            pAnnotationEdit->SetWindowPos(&wndTop,rEdit.left,rEdit.top + 1,rEdit.Width(),rEdit.Height() - 2,SWP_SHOWWINDOW);
            if (szInitialString) {
                pAnnotationEdit->SetText(*szInitialString);
            }
            pAnnotationEdit = NULL; // window needs to destroy itself
        }
    }
}

int CALLBACK EnumFontFamExProc(const ENUMLOGFONTEX * lpelfe,    // logical-font data
                               const NEWTEXTMETRICEX * lpntme, // physical-font data
                               DWORD FontType,                 // type of font
                               LPARAM lParam                   // application-defined data
                              ) {
    UNUSED_ALWAYS(FontType);
    UNUSED_ALWAYS(lpntme);
    long & nCharset = *(long *) lParam;

    if (nCharset == DEFAULT_CHARSET) {
        nCharset = lpelfe->elfLogFont.lfCharSet;
    } else {
        // If ANSI is one of the many chose it.
        if (lpelfe->elfLogFont.lfCharSet == ANSI_CHARSET) {
            nCharset = lpelfe->elfLogFont.lfCharSet;
        }
        // Otherwise we just keep the first enumerated charset (presumably the default)
    }
    return 0;
}

void CAnnotationWnd::CreateAnnotationFont(CFont * pFont, int nPoints, LPCTSTR szFace) {
    HDC dc = ::GetDC(NULL);
    int nFontSizeLU = MulDiv(nPoints, GetDeviceCaps(dc, LOGPIXELSY), 72);
    LOGFONT lfEnum;
    lfEnum.lfCharSet = DEFAULT_CHARSET;
    lstrcpyn((LPTSTR)lfEnum.lfFaceName, szFace, LF_FACESIZE);
    lfEnum.lfPitchAndFamily = 0;
    long nCharset = DEFAULT_CHARSET;
    EnumFontFamiliesEx(dc, &lfEnum, (FONTENUMPROC)EnumFontFamExProc, (long) &nCharset, 0);
    ::ReleaseDC(NULL, dc);
    pFont->CreateFont(-nFontSizeLU, 0,0,0, FW_NORMAL, 0,0,0, UCHAR(nCharset == DEFAULT_CHARSET ? ANSI_CHARSET : nCharset),
                      OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                      DEFAULT_PITCH|FF_DONTCARE, (LPCTSTR)szFace);
}

//###########################################################################
// CGlossWnd
// Gloss window contained in the graphs. It displays the gloss information of
// the actually opened documents wave data. To do this it needs information
// from the view and the document to keep track with zooming and scrolling.

/***************************************************************************/
// CGlossWnd::OnDraw Drawing
/***************************************************************************/
void CGlossWnd::OnDraw(CDC * pDC, const CRect & printRect) {

    CRect rWnd;
	// get invalid region
    CRect rClip;

    if (pDC->IsPrinting()) {
        rWnd  = printRect;
        rClip = printRect;
    } else {
        GetClientRect(rWnd);
        pDC->GetClipBox(&rClip);
        pDC->LPtoDP(&rClip);
    }

    // select gloss font
    CFont * pOldFont = pDC->SelectObject(GetFont());
    // get text metrics
    TEXTMETRIC tm;
    pDC->GetTextMetrics(&tm);
    // get window coordinates
    if (rWnd.Width() == 0) {
        return;    // nothing to draw
    }

    // set font colors
	// get pointer to colors from main frame
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    Colors * pColors = pMainWnd->GetColors();

	// set font color
    pDC->SetTextColor(pColors->cAnnotationFont[m_nIndex]);
	// letters may overlap, so they must be transparent
    pDC->SetBkMode(TRANSPARENT);
    // draw 3D window border
    CPen penDkgray(PS_SOLID, 1, pColors->cSysBtnShadow);
    CPen penLtgray(PS_SOLID, 1, pColors->cSysBtnHilite);

    CPen * pOldPen = pDC->SelectObject(&penDkgray);
    // SDM 1.06.6 make annotation windows symettrical
    pDC->MoveTo(rClip.left, rWnd.bottom - 1);
    pDC->LineTo(rClip.right, rWnd.bottom - 1);
    pDC->SelectObject(&penLtgray);
    pDC->MoveTo(rClip.left, rWnd.top);
    pDC->LineTo(rClip.right, rWnd.top);
    pDC->SelectObject(pOldPen);

    // get pointer to graph, view and document
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaView * pView = (CSaView *)pGraph->GetParent();
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();

    //*******************************************************
    // 09/27/2000 - DDO If the graph is the TWC graph
    // then we don't want to draw the annotation text.
    //*******************************************************
    if (pGraph->IsPlotID(IDD_TWC)) {
        pDC->SelectObject(pOldFont);  // set back old font
        return;
    }

    // SDM 1.06.6
    // Exclude Boundaries
    rWnd.SetRect(rWnd.left, rWnd.top + 1, rWnd.right, rWnd.bottom - 1);

    // check if area graph type
    double fDataStart;
    DWORD dwDataFrame;
    if (pGraph->IsAreaGraph()) {
        // get necessary data from area plot
        fDataStart = pGraph->GetPlot()->GetAreaPosition();
        dwDataFrame = pGraph->GetPlot()->GetAreaLength();
    } else {
        // check if graph has private cursor
        if (pGraph->HasPrivateCursor()) {
            // get necessary data from between public cursors
            WORD wSmpSize = WORD(pDoc->GetSampleSize());
			// data index of first sample to display
            fDataStart = pView->GetStartCursorPosition();
			// number of data points to display
            dwDataFrame = pView->GetStopCursorPosition() - (DWORD) fDataStart + wSmpSize;
        } else {
            // get necessary data from document and from view
			// data index of first sample to display
            fDataStart = pView->GetDataPosition(rWnd.Width());
			// number of data points to display
            dwDataFrame = pView->CalcDataFrame(rWnd.Width());
        }
    }
    if (dwDataFrame == 0) {
		// nothing to draw
        return;
    }
    // calculate the number of data samples per pixel
    double fBytesPerPix = (double)dwDataFrame / (double)rWnd.Width();

    // get pointer to gloss strings
    CGlossSegment * pGloss = (CGlossSegment *)pDoc->GetSegment(m_nIndex);
    if (pGloss->GetOffsetSize()==0) {
		// nothing to process.
		// restore and quit
		// set back old font
		pDC->SelectObject(pOldFont);
		pView->UpdateSelection();
		return;
	}

    // array is not empty
    // get pointer to gloss offset and duration arrays
    CSegment * pPhonetic = pDoc->GetSegment(PHONETIC);
    // position prepare loop. Find first string to display in clipping rect
    int nLoop = 0;
    if ((fDataStart > 0) && (pGloss->GetOffsetSize() > 1)) {
        double fStart = fDataStart + (double)(rClip.left - tm.tmAveCharWidth) * fBytesPerPix;
        for (nLoop = 1; nLoop < pGloss->GetOffsetSize(); nLoop++) {
            if ((double)(pGloss->GetStop(nLoop)) > fStart) {
				// first string must be at lower position
				// this is it
                nLoop--;
                break;
            }
        }
    }

    if (nLoop >= pGloss->GetOffsetSize()) {
		// no data to display
		// set back old font
		pDC->SelectObject(pOldFont);
		pView->UpdateSelection();
		return;
	}

    // there is something to display
    // display loop
    int nDisplayPos = 0;
    int nDisplayStop = 0;
    CString string;
	CString szSpace = " ";
    do {
        string.Empty();

        // get the string to display
        string = pGloss->GetText(nLoop);

		if (string.GetLength()>1) {
			// insert a space after the delimiter
			string = string.GetAt(0) + szSpace + string.Right(string.GetLength() - 1);
		}

		nDisplayPos = round2Int((pGloss->GetOffset(nLoop) - fDataStart) / fBytesPerPix);
        // check if the character is selected
        BOOL bSelect = FALSE;
        if (pGloss->GetSelection() == nLoop) {
            bSelect = TRUE;
        }
        // calculate duration
        nDisplayStop = round2Int((pGloss->GetStop(nLoop)- fDataStart)/ fBytesPerPix);
        //SDM 1.06.2
        if (m_bHintUpdateBoundaries) {
			// Show New Boundaries
            if (bSelect) {
                nDisplayPos = round2Int((m_dwHintStart - fDataStart)/ fBytesPerPix);
                nDisplayStop = round2Int((m_dwHintStop - fDataStart)/ fBytesPerPix);
            } else if (pGloss->GetSelection() == (nLoop+1)) {
				// Segment prior to selected segment
                // SDM 1.5Test11.1
                int nIndex = pPhonetic->GetPrevious(pPhonetic->FindOffset(m_dwHintStart));
                DWORD dwStop = pPhonetic->GetStop(nIndex);
                nDisplayStop = round2Int((dwStop - fDataStart)/ fBytesPerPix);
            } else if (pGloss->GetSelection() == (nLoop-1)) {
				// Segment after selected segment
                // SDM 1.5Test11.1
                int nIndex = pPhonetic->GetNext(pPhonetic->FindStop(m_dwHintStop));
                DWORD dwStart = pPhonetic->GetOffset(nIndex);
                nDisplayPos = round2Int((dwStart - fDataStart)/ fBytesPerPix);
            }
        }
        if ((nDisplayStop - nDisplayPos) < 2) {
			// must be at least 2 to display a point
            nDisplayStop++;
        }
        if ((nDisplayStop - nDisplayPos) < 2) {
			// must be at least 2 to display a point
            nDisplayPos--;
        }
        // set rectangle to display string centered within
        rWnd.SetRect(nDisplayPos, rWnd.top, nDisplayStop, rWnd.bottom);
        // highlight background if selected character
        COLORREF normalTextColor = pDC->GetTextColor();
        if (bSelect) {
			// set highlighted text
            normalTextColor = pDC->SetTextColor(pColors->cSysColorHiText);
            CBrush brushHigh(pColors->cSysColorHilite);
            CPen penHigh(PS_SOLID, 1, pColors->cSysColorHilite);
            CBrush * pOldBrush = (CBrush *)pDC->SelectObject(&brushHigh);
            CPen * pOldPen = pDC->SelectObject(&penHigh);
            pDC->Rectangle(rWnd.left, rWnd.top - 1, rWnd.right, rWnd.bottom);
            pDC->SelectObject(pOldBrush);
            pDC->SelectObject(pOldPen);
        }

        DrawTranscriptionBorders(pDC,rWnd,pColors);

		// not enough space
        if ((nDisplayStop-nDisplayPos) <= (string.GetLength() * tm.tmAveCharWidth)) {
			// even not enough space for at least two characters with dots
            if ((nDisplayStop-nDisplayPos) <= 4 * tm.tmAveCharWidth) {
                // draw only first character
                TCHAR c = string.GetAt(0);
				// print first character
                pDC->DrawText(&c, 1, rWnd, DT_VCENTER | DT_SINGLELINE | DT_LEFT | DT_NOCLIP);
            } else {
                // draw as many characters as possible and 3 dots
                string = string.Left((nDisplayStop-nDisplayPos) / tm.tmAveCharWidth - 2) + "...";
                pDC->DrawText((LPCTSTR)string, string.GetLength(), rWnd, DT_VCENTER | DT_SINGLELINE | DT_LEFT | DT_NOCLIP);
            }
        } else { // enough space to display string
            TCHAR c = string.GetAt(0);
            pDC->DrawText((LPCTSTR)&c, 1, rWnd, DT_VCENTER | DT_SINGLELINE | DT_LEFT | DT_NOCLIP);
            CString temp = string.Right(string.GetLength()-1);
            pDC->DrawText((LPCTSTR)temp, string.GetLength() - 1, rWnd, DT_VCENTER | DT_SINGLELINE | DT_CENTER | DT_NOCLIP);
        }
        if (bSelect) {
			// set back old text color
            pDC->SetTextColor(normalTextColor);
        }
    } while ((nDisplayPos < rClip.right) && (++nLoop < pGloss->GetOffsetSize()));

	// set back old font
    pDC->SelectObject(pOldFont);

    //SDM 1.06.5
    //keep up to date of changes force redraw of deselected virtual selections
    // get current selection information
    pView->UpdateSelection();
}

void CAnnotationWnd::SetHintUpdateBoundaries(bool bHint, bool bOverlap) {
    SetHintUpdateBoundaries(bHint,m_dwHintStart,m_dwHintStop, bOverlap);
}

void CAnnotationWnd::ShowTranscriptionBoundaries(BOOL val) {
    m_bTranscriptionBoundary = (val)?true:false;
}

void CAnnotationWnd::DrawTranscriptionBorders(CDC * pDC, CRect rWnd, Colors * pColors) {

    // draw the borders
    if (m_bTranscriptionBoundary) {

        // define points for the boundary arrows
        POINT points[3];
        points[0].y = 0;
        points[1].y = 0;
        points[2].y = 4;

        CPen penBoundary(PS_SOLID, 1, pColors->cPlotBoundaries);
        CBrush brushBoundary(pColors->cPlotBoundaries);
        CPen * pOldPen = pDC->SelectObject(&penBoundary);
        CBrush * pOldBrush = (CBrush *)pDC->SelectObject(&brushBoundary);

        // draw the left line
        pDC->MoveTo(rWnd.left, rWnd.top-2);
        pDC->LineTo(rWnd.left, rWnd.bottom);
        // draw top left triangle
        points[0].x = rWnd.left + 4;
        points[1].x = rWnd.left;
        points[2].x = rWnd.left;
        pDC->Polygon(points, 3);

        // draw the right line
        pDC->MoveTo(rWnd.right, rWnd.top-1);
        pDC->LineTo(rWnd.right, rWnd.bottom);
        // draw top left triangle
        points[0].x = rWnd.right - 4;
        points[1].x = rWnd.right;
        points[2].x = rWnd.right;
        pDC->Polygon(points, 3);

        pDC->SelectObject(pOldPen);
        pDC->SelectObject(pOldBrush);
    }
}

/***************************************************************************/
// CMusicPhraseWnd::OnDraw
// For AS, for PL1 only, display the unabbreviated contents of gloss
/***************************************************************************/
void CMusicPhraseWnd::OnDraw(CDC * pDC, const CRect & printRect) {
	CAnnotationWnd::OnDraw( pDC, printRect);
}

CFont * CXScaleWnd::GetFont() {
	// return selected font
    return &m_font;
}
int CXScaleWnd::GetScaleMode() {
	// return scale mode
    return m_nScaleMode;
}
double CXScaleWnd::GetFirstGridPosition() {
	// return position of first gridline
    return m_fFirstGridPos;
}
double CXScaleWnd::GetGridDistance() {
	// return distance between gridlines
    return m_fGridDistance;
}
CString * CXScaleWnd::GetDimensionText() {
	// return pointer to dimension text
    return &m_szScaleDimension;
}

CPhoneticWnd::CPhoneticWnd(int nIndex) : CAnnotationWnd(nIndex) {
}
CToneWnd::CToneWnd(int nIndex) : CAnnotationWnd(nIndex) {
}
CPhonemicWnd::CPhonemicWnd(int nIndex) : CAnnotationWnd(nIndex) {
}
COrthographicWnd::COrthographicWnd(int nIndex) : CAnnotationWnd(nIndex) {
}
CGlossWnd::CGlossWnd(int nIndex) : CAnnotationWnd(nIndex) {
}
CMusicPhraseWnd::CMusicPhraseWnd(int nIndex) : CAnnotationWnd(nIndex) {
}
