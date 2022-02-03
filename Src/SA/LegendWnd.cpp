/////////////////////////////////////////////////////////////////////////////
// sa_annot.cpp:
// Implementation of the CLegendWnd
//                       CXScaleWnd
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
#include "LegendWnd.h"
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

//###########################################################################
// CLegendWnd
// Legend window contained in the graphs. It displays the names of the
// annotation windows and a scale for the graphs data. It has to keep track
// with the magnify factor (for the scale).

/////////////////////////////////////////////////////////////////////////////
// CLegendWnd message map

BEGIN_MESSAGE_MAP(CLegendWnd, CWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_RBUTTONDOWN()
    ON_WM_LBUTTONDOWN()
    ON_WM_CREATE()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLegendWnd construction/destruction/creation

extern CSaApp NEAR theApp;
/***************************************************************************/
// CLegendWnd::CLegendWnd Constructor
/***************************************************************************/
CLegendWnd::CLegendWnd() {

	m_nScaleMode = NO_SCALE | SCALE_INFO;
	// set divisions
    m_nGridDivisions = -1; 
	// scale min value
    m_dScaleMinValue = 0;
	// scale max value
    m_dScaleMaxValue = 100;
    m_dFirstGridPos = 0;
    m_fGridDistance = 0;
    m_bRecalculate = TRUE;
    m_nHeightUsed = -1;
    m_nDivisionsUsed = -1;
    m_d3dOffset = 0.;
    m_fMagnifyUsed = 0;
    m_rRecalc.SetRect(0, 0, 0, 0);
    m_bRecalcUp = TRUE;
}

/***************************************************************************/
// CLegendWnd::~CLegendWnd Destructor
/***************************************************************************/
CLegendWnd::~CLegendWnd() {
}

/***************************************************************************/
// CLegendWnd::PreCreateWindow Creation
// Called from the framework before the creation of the window. Registers
// the new window class.
/***************************************************************************/
BOOL CLegendWnd::PreCreateWindow(CREATESTRUCT & cs) {
    // register the window class
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
                                       AfxGetApp()->LoadStandardCursor(IDC_ARROW), 0, 0);
    return CWnd::PreCreateWindow(cs);
}

/***************************************************************************/
// CLegendWnd::GetWindowWidth
// Calculating the width of the current font times the longest text to
// display.
/***************************************************************************/
int CLegendWnd::GetWindowWidth() {
    
	// get width of the legend window font
    TEXTMETRIC tm;
	// get device context
    CDC * pDC = GetDC();
	// select legend font
    CFont * pOldFont = pDC->SelectObject(&m_font);
	// get text metrics
    pDC->GetTextMetrics(&tm);
	// set back old font
    pDC->SelectObject(pOldFont);
    ReleaseDC(pDC);
    CString szText;
	// is longest text in legend
	// return font width
    szText.LoadString(IDS_WINDOW_REFERENCE_AS);
    return tm.tmAveCharWidth * (szText.GetLength() + 2) + 3;
}

/***************************************************************************/
// CLegendWnd::SetScale Set the legend scale
// There are different modes for the legend scale possible. nMode sets this
// mode. Then the values tell the vertical range and the string will be drawn
// as dimension title. The string pointers can be NULL (no dimension drawn).
// The function returns TRUE if the legend window has to be redrawn.
/***************************************************************************/
BOOL CLegendWnd::SetScale(int nMode, double dMinValue, double dMaxValue, TCHAR * pszDimension, int nGridDivisions,  double d3dOffset) {
    
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
    if (m_dScaleMinValue != dMinValue) {
		// set scale min value
        m_dScaleMinValue = dMinValue;
        bRedraw = TRUE;
    }
    if (m_dScaleMaxValue != dMaxValue) {
		// set scale max value
        m_dScaleMaxValue = dMaxValue;
        bRedraw = TRUE;
    }
    if (pszDimension) {
        if (m_szScaleDimension != pszDimension) {
			// set scale dimension text
            m_szScaleDimension = pszDimension;
            bRedraw = TRUE;
        }
    } else if (!m_szScaleDimension.IsEmpty()) {
        m_szScaleDimension.Empty();
        bRedraw = TRUE;
    }
    if (bRedraw) {
        m_bRecalculate = TRUE;
    }
    return bRedraw;
}

#undef min // the min() macro hides the min() member function std::numeric_limits<>::min()

/***************************************************************************/
// CLegendWnd::CalculateScale Calculate new scale
// Calculates the new scale parameters if necessary. If the plot window
// height or the magnify factor have changed or the flag m_bRecalculate
// is TRUE, a new calculation is needed. If the pointer to the device
// context is NULL, the function will get the pointer by itself to get the
// text metrics from the actual window font.
/***************************************************************************/
void CLegendWnd::CalculateScale(CDC * pDC, CRect * prWnd) {

	// get pointer to graph
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    // calculate new scale with magnify
	// get magnify
    double fMagnify = pGraph->GetMagnify(); 
    if ((!m_bRecalculate) && 
		(prWnd->Height() == m_nHeightUsed) && 
		((m_fMagnifyUsed == fMagnify) && (m_nDivisionsUsed == m_nGridDivisions))) {
		// scale up to date
        return;
    }
    if (prWnd->Height() <= 0) {
		// no scale to draw
        return;
    }
    // scale has changed, recalculate
    m_bRecalculate = FALSE;
    m_nHeightUsed = prWnd->Height();
    m_nDivisionsUsed = m_nGridDivisions;
    m_fMagnifyUsed = fMagnify;
    // calculate new scale with magnify
    double dMinValue = m_dScaleMinValue;
    double dMaxValue = dMinValue + (m_dScaleMaxValue - dMinValue) / fMagnify;
    if (m_dScaleMinValue < 0) {
        dMinValue = m_dScaleMinValue / fMagnify;
        dMaxValue = m_dScaleMaxValue / fMagnify;
    }
    if (m_nScaleMode & LOG10) {
        // logarithmic scale
        if (dMinValue < 0.) {
			// Log plots must draw ranges greater than zero
            dMinValue = std::numeric_limits<double>::min();
        }
        if (dMaxValue < dMinValue) {
			// Log plots must draw non-inverted
            dMaxValue = dMinValue + std::numeric_limits<double>::min();
        }
        m_fBase = log10(dMinValue);
        double fTop = log10(dMaxValue);
        if (fTop <= m_fBase) {
			// The displayed range is small make big enough to avoid divide by zero
            fTop += 1.5*fTop*std::numeric_limits<double>::epsilon();
        }

		// distance
        m_fGridDistance = (double)(prWnd->Height() - 2) / (fTop - m_fBase);
        if (m_fGridDistance < 1.0) {
            // Why is this code necessary... Height() could be zero, but isn't 0 a legitimate grid distance
            ASSERT(FALSE);
            TRACE(_T("Exceptional legend grid distance\n"));
            m_fGridDistance = 1.0;
        }
		// location which should map to m_dScaleMinValue
        m_dFirstGridPos = prWnd->bottom - 1;
    } else {
        // linear scale
        m_fNumbPerPix = (dMaxValue - dMinValue) / ((double)prWnd->Height()/* - 3*/);
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
            DWORD dwMinScale = (DWORD)(m_fNumbPerPix * (double)(2 * tm.tmHeight));
            if (!dwMinScale) {
                dwMinScale = 1;
            }
            // find the 10 based log of this distance
            DWORD dw10Base = (DWORD)log10((double)dwMinScale);
            // now find the next appropriate scale division
            // Fix axis min problem in log plots CLW 9/21/00
            m_fBase = pow(10, (double)dw10Base);
            if ((DWORD)(m_fBase * 2) < dwMinScale) {
                if (((DWORD)m_fBase * 5) % 2) {
                    m_fBase *= 6;
                } else {
                    m_fBase *= 5;
                }
            } else {
                m_fBase *= 2;
            }
            if ((DWORD)m_fBase < dwMinScale) {
                m_fBase = pow(10, (double)(++dw10Base));
            }
        } else {
            m_fBase = (m_dScaleMaxValue - m_dScaleMinValue)/m_nGridDivisions;
        }
        // calculate grid distance
		// gridline distance in pixels
        m_fGridDistance = m_fBase / m_fNumbPerPix;
        // calculate first (top) gridline position in plot window client coordinates
        if (!(m_nScaleMode & ARBITRARY)) {
            m_dFirstGridPos = prWnd->bottom - (floor(dMaxValue/(m_fBase))*m_fBase - dMinValue) / m_fNumbPerPix;
        } else {
            m_dFirstGridPos = prWnd->top;
        }
    }
}

/***************************************************************************/
// CLegendWnd::OnCreate Window creation
// Creation of the legend font, used in the window.
/***************************************************************************/
int CLegendWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }
    // create legend font
    LPCTSTR pszName = _T("MS Sans Serif");
    m_font.CreateFont(15, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                      ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                      DEFAULT_QUALITY, (VARIABLE_PITCH | FF_ROMAN), pszName);


    // get pointer to graph
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    if (pGraph->IsPlotID(IDD_STAFF)) {
        CString szButtonText(_T('\0'),2);

        szButtonText.SetAt(0,DISPLAY_NOTE_ICON);
        hPlayButton = ::CreateWindow(_T("BUTTON"),szButtonText,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                                     1,6,17,18,m_hWnd,(HMENU)IDM_PLAY,theApp.m_hInstance,NULL);
        szButtonText = "&&";
        hPlayBothButton = ::CreateWindow(_T("BUTTON"),szButtonText,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                                         18,6,17,18,m_hWnd,(HMENU)IDM_PLAY_BOTH,theApp.m_hInstance,NULL);
        szButtonText.SetAt(0,DISPLAY_PLAY_ICON);
        hPlayWaveButton = ::CreateWindow(_T("BUTTON"),szButtonText,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                                         35,6,17,18,m_hWnd,(HMENU)IDM_PLAY_WAVE,theApp.m_hInstance,NULL);
        szButtonText.SetAt(0,DISPLAY_STOP_ICON);
        hStopButton = ::CreateWindow(_T("BUTTON"),szButtonText,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                                     1,26,17,18,m_hWnd,(HMENU)IDM_STOP,theApp.m_hInstance,NULL);

        hVoiceButton = ::CreateWindow(_T("BUTTON"),_T("Voice"),WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                                      18,26,34,18,m_hWnd,(HMENU)IDM_VOICE,theApp.m_hInstance,NULL);
        hTempoButton = ::CreateWindow(_T("BUTTON"),_T("Tempo"),WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                                      3,50,47,20,m_hWnd,(HMENU)IDM_TEMPO,theApp.m_hInstance,NULL);

        hImportButton = CreateWindow(_T("BUTTON"),_T("Import"),WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                                     3,73,47,20,m_hWnd,(HMENU)IDM_IMPORT,theApp.m_hInstance,NULL);
        hExportButton = CreateWindow(_T("BUTTON"),_T("Export"),WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                                     3,96,47,20,m_hWnd,(HMENU)IDM_EXPORT,theApp.m_hInstance,NULL);
        hConvertButton = CreateWindow(_T("BUTTON"),_T("Convert"),WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                                      3,119,47,20,m_hWnd,(HMENU)IDM_CONVERT,theApp.m_hInstance,NULL);

        //Set the font for the play, stop, loop & pause buttons
        hButtonFont = CreateFont(-24,0,0,0,0,0,0,0,
                                 ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY,FF_DONTCARE|DEFAULT_PITCH,_T("Musique"));
        ::SendMessage(hPlayButton,WM_SETFONT,(WPARAM)hButtonFont,0);
        ::SendMessage(hPlayWaveButton,WM_SETFONT,(WPARAM)hButtonFont,0);
        //::SendMessage(hPauseButton,WM_SETFONT,(WPARAM)hButtonFont,0);
        ::SendMessage(hStopButton,WM_SETFONT,(WPARAM)hButtonFont,0);
        //::SendMessage(hLoopButton,WM_SETFONT,(WPARAM)hButtonFont,0);

        //Set the font for the voice and convert buttons - DDO 08/11/00
        hButtonFont = CreateFont(-11,0,0,0,FW_NORMAL,0,0,0,
                                 ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY,FF_DONTCARE|DEFAULT_PITCH,NULL);
        ::SendMessage(hVoiceButton,   WM_SETFONT, (WPARAM)hButtonFont, 0);
        ::SendMessage(hTempoButton,   WM_SETFONT, (WPARAM)hButtonFont, 0);
        ::SendMessage(hImportButton, WM_SETFONT, (WPARAM)hButtonFont, 0);
        ::SendMessage(hExportButton, WM_SETFONT, (WPARAM)hButtonFont, 0);
        ::SendMessage(hConvertButton, WM_SETFONT, (WPARAM)hButtonFont, 0);
    } else if (pGraph->IsPlotID(IDD_TWC) || pGraph->IsPlotID(IDD_WAVELET)) {
        CRect rWnd;
        GetClientRect(rWnd);
        int YDown = rWnd.Height() - 23; // Place Down button 23 units above bottom (so it's bottom is 3 above bottom of window)
        int YUp   = rWnd.Height() - 42; // Place Up   button 43 units above bottom (right above Down button)

        m_hDownButton = ::CreateWindow(_T("BUTTON"),_T("6"),WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                                       6, YDown, 19, 19, m_hWnd, (HMENU)IDM_DOWN, theApp.m_hInstance, NULL);
        m_hUpButton = ::CreateWindow(_T("BUTTON"),_T("5"),WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                                     6, YUp,   19, 19, m_hWnd, (HMENU)IDM_UP, theApp.m_hInstance, NULL);
        hButtonFont = CreateFont(-18,0,0,0,0,0,0,0,
                                 SYMBOL_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY,FF_DONTCARE|DEFAULT_PITCH,_T("Marlett"));

        ::SendMessage(m_hDownButton,WM_SETFONT,(WPARAM)hButtonFont,0);
        ::SendMessage(m_hUpButton,WM_SETFONT,(WPARAM)hButtonFont,0);
    } else {
        hPlayButton = hPlayBothButton = hPlayBothButton = hPauseButton = hStopButton = hLoopButton =
                                            hVoiceButton = hTempoButton = hConvertButton = hImportButton = hExportButton = NULL;
        hButtonFont = NULL;
    }
    return 0;
}

/***************************************************************************/
// CLegendWnd::OnCommand
// Handle button presses.
/***************************************************************************/
BOOL CLegendWnd::OnCommand(WPARAM wParam, LPARAM /* lParam */) {
	
	// get pointer to graph
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    if (pGraph->IsPlotID(IDD_STAFF)) {
        CPlotStaff * staff = (CPlotStaff *) pGraph->GetPlot();
        switch (LOWORD(wParam)) {
        case IDM_PLAY:
            staff->SetFocusedGraph(pGraph);
            staff->PlaySelection();
            break;
        case IDM_PLAY_BOTH:
            staff->SetFocusedGraph(pGraph);
            staff->PlaySelection(TRUE, TRUE);
            break;
        case IDM_PLAY_WAVE:
            staff->SetFocusedGraph(pGraph);
            staff->PlaySelection(FALSE, TRUE);
            break;
        case IDM_PAUSE:
            staff->SetFocusedGraph(pGraph);
            staff->PausePlay();
            break;
        case IDM_STOP:
            staff->SetFocusedGraph(pGraph);
            staff->StopPlay();
            break;
        case IDM_LOOP:
            staff->SetFocusedGraph(pGraph);
            staff->LoopPlay();
            break;
        case IDM_CONVERT:
            staff->SetFocusedGraph(pGraph);
            staff->Convert();
			// for some reason I have to do this again!
            staff->SetFocusedGraph(pGraph);
            break;
        case IDM_EXPORT:
            staff->SetFocusedGraph(pGraph);
            staff->ExportFile();
			// for some reason I have to do this again!
            staff->SetFocusedGraph(pGraph);
            break;
        case IDM_IMPORT:
            staff->SetFocusedGraph(pGraph);
            staff->ImportFile();
			// for some reason I have to do this again!
            staff->SetFocusedGraph(pGraph);
            break;
        case IDM_VOICE:
            staff->SetFocusedGraph(pGraph);
            staff->ChooseVoice();
            break;
        case IDM_TEMPO:
            staff->SetFocusedGraph(pGraph);
            staff->ChooseTempo();
            break;
        }
        return TRUE;
    } else if (pGraph->IsPlotID(IDD_TWC)) {
        static_cast<CSaView *>(pGraph->GetParent())->SetFocusedGraph(pGraph);
        CPlotTonalWeightChart * pTWC = (CPlotTonalWeightChart *) pGraph->GetPlot();
        WORD UpDown = LOWORD(wParam);
        BOOL bKeyShifted = GetKeyState(VK_SHIFT) >> 8;
        pTWC->AdjustSemitoneOffset(UpDown,bKeyShifted);
        return TRUE;
    }

    // 8/03/01 ARH Added to use arrows on wavelet graph
    else if (pGraph->IsPlotID(IDD_WAVELET)) {
        CPlotWavelet * pWavelet = (CPlotWavelet *) pGraph->GetPlot();
        switch (LOWORD(wParam)) {
        case IDM_UP:
            pWavelet->IncreaseDrawingLevel();
            pWavelet->RedrawPlot(true);
            break;

        case IDM_DOWN:
            pWavelet->DecreaseDrawingLevel();
            pWavelet->RedrawPlot(true);
            break;

        }
        return TRUE;
    } else {
        return FALSE;
    }
}

/***************************************************************************/
// CLegendWnd::OnSetFocus
// If this is a Staff Control, keyboard focus is given to the staff edit window
/***************************************************************************/
void CLegendWnd::OnSetFocus(CWnd *) {
    // get pointer to graph
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    if (pGraph->IsPlotID(IDD_STAFF)) {
        ::SetFocus(pGraph->GetPlot()->m_hWnd);
    }
}

/***************************************************************************/
// CLegendWnd::OnPaint
// Paints using OnDraw, which is shared with OnPrint
/***************************************************************************/
void CLegendWnd::OnPaint() {
    CRect dummyRect1(0,0,0,0);
    CRect dummyRect2(0,0,0,0);
    CRect dummyRect3(0,0,0,0);
	// device context for painting
    CPaintDC dc(this);
    OnDraw(&dc,dummyRect1,dummyRect2,dummyRect3,NULL);
}

/***************************************************************************/
// CLegendWnd::OnDraw Drawing
// This function has to calculate the vertical scale in any case, because
// the plot will ask for the gridline positions to draw his gridlines.
/***************************************************************************/
void CLegendWnd::OnDraw(CDC * pDC,
                        const CRect & printRect,
                        const CRect & printPlotWnd,
                        const CRect & printXscaleRect,
                        const CRect * printAnnotation) {
    // get pointer to graph
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();

    // Create staff control buttons - TCJ 4/19/2000
    if (pGraph->IsPlotID(IDD_STAFF)) {
        CPlotStaff * staff = (CPlotStaff *) pGraph->GetPlot();
        staff->HideButtons();
        ::ShowWindow(hPlayButton,SW_SHOW);
        ::ShowWindow(hPlayBothButton,SW_SHOW);
        ::ShowWindow(hPlayWaveButton,SW_SHOW);
        // ::ShowWindow(hPauseButton,SW_SHOW);
        ::ShowWindow(hStopButton,SW_SHOW);
        // ::ShowWindow(hLoopButton,SW_SHOW);
        ::ShowWindow(hVoiceButton,SW_SHOW);
        ::ShowWindow(hTempoButton,SW_SHOW);
        ::ShowWindow(hImportButton,SW_SHOW);
        ::ShowWindow(hExportButton,SW_SHOW);
        ::ShowWindow(hConvertButton,SW_SHOW);
        return;
    }

    // select legend font
    CFont * pOldFont = pDC->SelectObject(&m_font);
    // get text metrics
    TEXTMETRIC tm;
    pDC->GetTextMetrics(&tm);
    // calculate new scale with magnify
	// get magnify
    double fMagnify = pGraph->GetMagnify(); 
    double dScaleMinValue = m_dScaleMinValue;
    double dScaleMaxValue = dScaleMinValue + (m_dScaleMaxValue - dScaleMinValue) / fMagnify;
    if (m_dScaleMinValue < 0) {
        dScaleMinValue = m_dScaleMinValue / fMagnify;
        dScaleMaxValue = m_dScaleMaxValue / fMagnify;
    }
    // get coordinates of plot window
    CRect rPlotWnd;

    if (pDC->IsPrinting()) {
        rPlotWnd = printPlotWnd;
    } else {
        pGraph->GetPlot()->GetClientRect(rPlotWnd);
    }

    // calculate scale parameters
	// calculate the scale
    CalculateScale(pDC, &rPlotWnd); 
    // get window coordinates
    CRect rWnd;

    if (pDC->IsPrinting()) {
        rWnd = printRect;
    } else {
        GetClientRect(rWnd);
    }
    if (rWnd.Height() == 0) {
		// set back old font
        pDC->SelectObject(pOldFont);  
		// nothing to draw
        return; 
    }

    int YDown = rWnd.Height() - 23;
    int YUp   = rWnd.Height() - 43;
    ::MoveWindow(m_hDownButton, 6, YDown,20, 20, TRUE);
    ::MoveWindow(m_hUpButton,   6, YUp,  20, 20, TRUE);

    // get pointer view and document
    CSaView * pView = (CSaView *)pGraph->GetParent();
    CSaDoc * pModel = (CSaDoc *)pView->GetDocument();
    // set font colors
	// get pointer to colors from main frame
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd(); 
    Colors * pColors = pMainWnd->GetColors();
	// set font color
    pDC->SetTextColor(pColors->cScaleFont); 
	// letters may overlap, so they must be transparent
    pDC->SetBkMode(TRANSPARENT); 
	// save bottom
    int nBottom = rWnd.bottom;
	// save top
    int nTop = rWnd.top;
    rWnd.left += 2;
    rWnd.bottom = -2;

    int resourceIndex[] = {
        IDS_WINDOW_PHONETIC,
        IDS_WINDOW_TONE,
        IDS_WINDOW_PHONEMIC,
        IDS_WINDOW_ORTHO,
        IDS_WINDOW_GLOSS,
        IDS_WINDOW_GLOSS_NAT,
        IDS_WINDOW_REFERENCE,
        IDS_WINDOW_MUSIC_PL1,
        IDS_WINDOW_MUSIC_PL2,
        IDS_WINDOW_MUSIC_PL3,
        IDS_WINDOW_MUSIC_PL4
    };

    CString szText;
    for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
        int current = CGraphWnd::m_anAnnWndOrder[nLoop];
        if (pGraph->HaveAnnotation(current)) {
            // gloss window is visible
			int rid = resourceIndex[current];
            szText.LoadString(rid);
            if (pDC->IsPrinting()) {
                rWnd.bottom += printAnnotation[current].Height();
            } else {
                rWnd.bottom += pGraph->GetAnnotationWnd(current)->GetWindowHeight();
            }
            pDC->DrawText(szText, szText.GetLength(), rWnd, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_NOCLIP);
            rWnd.top = rWnd.bottom + 2;
        }
    }
    rWnd.bottom = rWnd.top;
    rWnd.top = nTop;
    // draw 3D window border
    CPen penDkgray(PS_SOLID, 1, pColors->cSysBtnShadow);
    CPen penLtgray(PS_SOLID, 1, pColors->cSysBtnHilite);
    CPen * pOldPen = pDC->SelectObject(&penLtgray);

    pDC->MoveTo(rWnd.right - 1, rWnd.top);
    pDC->LineTo(rWnd.right - 1, rWnd.bottom);
    pDC->SelectObject(&penDkgray);
    if (rWnd.Height() > 0) {
        pDC->MoveTo(rWnd.right - 2, rWnd.top);
        pDC->LineTo(rWnd.right - 2, rWnd.bottom - 1);
    }

    rWnd.top = rWnd.bottom;
    rWnd.bottom = nBottom; // restore bottom
    if (pGraph->HaveXScale()) {
        if (pDC->IsPrinting()) {
            rWnd.bottom -= printXscaleRect.Height();
        } else {
            rWnd.bottom -= pGraph->GetXScaleWnd()->GetWindowHeight();
        }
    }
    if (rWnd.Height() > 0) {
        pDC->MoveTo(rWnd.right - 1, rWnd.top - 1);
        pDC->LineTo(rWnd.right - 1, rWnd.bottom);
    }
    pDC->SelectObject(pOldPen);
	// don't draw over the border
    rWnd.right -= 1; 
    if ((m_nScaleMode != NO_SCALE) || (rPlotWnd.Height() <= 0)) {
        if (rWnd.Height() < tm.tmHeight) {
			// set back old font
            pDC->SelectObject(pOldFont);
			// not enough space to draw scale
            return;
        }
        // set default scale limits
		// upper text position limit
        int nTextTopLimit = rWnd.top + tm.tmHeight / 2;
		// lower text position limit
        int nTextBottomLimit = nBottom - tm.tmHeight / 2;
		// upper line position limit
        int nLineTopLimit = rWnd.top;
		// lower line position limit
        int nLineBottomLimit = rWnd.bottom;
        // draw the vertical dimension text
        if (!m_szScaleDimension.IsEmpty()) {
            // create the vertical font
            LOGFONT logFont;
			// fill up logFont
            m_font.GetObject(sizeof(LOGFONT), (void *)&logFont);
			// turn it 90 degrees
            logFont.lfEscapement = 900;
            logFont.lfWeight = FW_BOLD;

            logFont.lfHeight = -13;

            swprintf_s(logFont.lfFaceName,32,_T("%s"), _T("Arial"));
            CFont vFont;
			// create the vertical font
            vFont.CreateFontIndirect(&logFont);
			// select the vertical font
            pDC->SelectObject(&vFont);
            TEXTMETRIC tmv;
            pDC->GetTextMetrics(&tmv);
            // enough space?
            int nHeight = (m_szScaleDimension.GetLength() + 2) * tmv.tmAveCharWidth;
            if (nHeight < rWnd.Height()) {
				// draw the text
                pDC->TextOut(rWnd.left, rWnd.bottom - (rWnd.Height() - nHeight) / 2, m_szScaleDimension);
            }
			// set back normal text font
            pDC->SelectObject(&m_font);
        }
        // create line pen
        CPen pen(PS_SOLID, 1, pColors->cScaleLines);
        CPen * pOldPen = pDC->SelectObject(&pen);
        // draw the scale
		// vertical line horizontal position
        int nHorPos = rWnd.right - 4;
        // draw the vertical line
        pDC->MoveTo(nHorPos, rWnd.bottom - 1);
        if (fMagnify <= 1.0 || m_d3dOffset != 0.) {
            pDC->LineTo(nHorPos, (int) floor(rWnd.top + rWnd.Height()*m_d3dOffset) - 1);
        } else {
            pDC->LineTo(nHorPos, rWnd.top - 1);
        }
        // draw the arrows
        if ((dScaleMaxValue > 0) && (fMagnify <= 1.0) && (m_d3dOffset == 0.)) {
            // draw up arrow
            pDC->LineTo(nHorPos - 2, rWnd.top + 5);
            pDC->LineTo(nHorPos + 2, rWnd.top + 5);
            pDC->LineTo(nHorPos, rWnd.top + 2);
            pDC->MoveTo(nHorPos + 2, rWnd.top + 6);
            pDC->LineTo(nHorPos - 3, rWnd.top + 6);
            // modify the scale line limits
			// upper line position limit
            nLineTopLimit = rWnd.top + 7;
        }
        if ((dScaleMinValue < 0) && ((fMagnify <= 1.0) || (dScaleMaxValue == 0))) {
            // draw down arrow
            pDC->MoveTo(nHorPos, rWnd.bottom);
            pDC->LineTo(nHorPos - 2, rWnd.bottom - 5);
            pDC->LineTo(nHorPos + 2, rWnd.bottom - 5);
            pDC->LineTo(nHorPos, rWnd.bottom);
            pDC->MoveTo(nHorPos + 2, rWnd.bottom - 4);
            pDC->LineTo(nHorPos - 2, rWnd.bottom - 4);
            // modify the scale line limits
			// lower line position limit
            nLineBottomLimit = rWnd.bottom - 6;
        }
        if (m_nScaleMode & SCALE) {
            if (m_nScaleMode & LOG10) {
                // logarithmic scale
                int nLogPos = (int) floor(m_fBase);
                int nLogDisp = (int) ceil(dScaleMinValue / pow(10, floor(m_fBase)));
                double fPixelBase = rWnd.top + m_dFirstGridPos + (m_fBase - nLogPos) * m_fGridDistance;
                int nPixelPos = round2Int(fPixelBase);
                int nLastNumber = nPixelPos + 2 * tm.tmHeight;
                int nBase = (int)pow(10, (double)nLogPos);
                while (nPixelPos > nLineTopLimit) {
                    if (nLogDisp > 9) {
                        nLogDisp = 1;
                        nLogPos++;
                        fPixelBase -= m_fGridDistance;
                        nPixelPos = round2Int(fPixelBase);
                        if (nPixelPos > nLineTopLimit) {
                            // draw large line
                            pDC->MoveTo(nHorPos - 3, nPixelPos);
                            pDC->LineTo(nHorPos + 1, nPixelPos);
                        } else {
                            break;
                        }
                        nBase = (int)pow(10, (double)nLogPos);
                    } else {
                        // draw small line
                        nPixelPos = round2Int(fPixelBase - log10((double)nLogDisp) * m_fGridDistance);
                        if (nPixelPos > nLineTopLimit) {
                            pDC->MoveTo(nHorPos - 2, nPixelPos);
                            pDC->LineTo(nHorPos, nPixelPos);
                        } else {
                            break;
                        }
                    }
                    // draw the number
                    if ((m_nScaleMode & NUMBERS) && (nPixelPos > nTextTopLimit) && (nPixelPos < nTextBottomLimit)
                            && (nLastNumber > (nPixelPos + tm.tmHeight + 2)) && (nLogDisp < 6) && (nLogDisp != 4)) {
                        CRect rText(rWnd.left, nPixelPos - tm.tmHeight, rWnd.right - 9, nPixelPos + tm.tmHeight);
                        TCHAR szText[16];
                        swprintf_s(szText, _T("%d"), nBase * nLogDisp);
                        pDC->DrawText(szText, -1, rText, DT_SINGLELINE | DT_VCENTER | DT_RIGHT | DT_NOCLIP);
                        nLastNumber = nPixelPos;
                    }
                    nLogDisp++;
                }
            } else {
				// linear scale
                // draw the positive lines and numbers
                if (m_dScaleMaxValue == 0) {
                    dScaleMinValue = m_dScaleMinValue;
                }

                double dScaleMinLoopValue = dScaleMinValue;
                double dScaleMaxLoopValue = dScaleMaxValue;

                BOOL bLargeLine = TRUE;

                if (!(m_nScaleMode & ARBITRARY)) {
                    int nHalfTicks = (int) floor(-dScaleMinLoopValue/(m_fBase/2));

                    // The first tick needs to be a multiple of (m_fBase/2) from zero
                    dScaleMinLoopValue = - nHalfTicks * (m_fBase/2);

                    // we will draw a number here if it is an even number of half ticks from zero
                    bLargeLine = ((nHalfTicks & 1) == 0);
                }

                // there are positive values or no magnify (we need at least the zero line)
                for (double dLoop = dScaleMinLoopValue; m_fBase > 0 ? dLoop <= dScaleMaxLoopValue : dLoop >= dScaleMaxLoopValue; dLoop += (m_fBase / 2)) {
                    // calculate line position
                    int nPos = round2Int(rWnd.bottom - (double)(1 - m_d3dOffset)*(dLoop - dScaleMinValue) / m_fNumbPerPix);
                    if (nPos <= nLineTopLimit) {
                        break;
                    }
                    // draw the line
                    if (nPos <= nLineBottomLimit) {
                        if (bLargeLine) {
                            // draw a large line
                            pDC->MoveTo(nHorPos - 3, nPos);
                            pDC->LineTo(nHorPos + 1, nPos);
                            // draw the number
                            if ((m_nScaleMode & NUMBERS) && (nPos > nTextTopLimit) && (nPos < nTextBottomLimit)) {
                                CRect rText(rWnd.left, nPos - tm.tmHeight, rWnd.right - 9, nPos + tm.tmHeight);
                                TCHAR szText[16];
                                int nWidth = (int) -log10(m_fBase);
                                if (nWidth < 0) {
                                    nWidth = 0;
                                }
                                swprintf_s(szText, _T("%.*f"), nWidth, dLoop);
                                pDC->DrawText(szText, -1, rText, DT_SINGLELINE | DT_VCENTER | DT_RIGHT | DT_NOCLIP);
                            }
                        } else {
                            // draw a small line
                            pDC->MoveTo(nHorPos - 2, nPos);
                            pDC->LineTo(nHorPos, nPos);
                        }
                    }
                    bLargeLine = !bLargeLine;
                }
            }
        }
        pDC->SelectObject(pOldPen);
    } else if (m_nScaleMode & SCALE_INFO && !pGraph->IsPlotID(IDD_STAFF)) {
        // no scale, but text
        TCHAR szText[16];
        int nHeight = 7 * tm.tmHeight / 2;
        if (pGraph->IsAreaGraph() && !pGraph->IsPlotID(IDD_RECORDING)) {
			// add extra space for recalculation button
            nHeight += tm.tmHeight + 4;
        }
        if (rWnd.Height() >= nHeight) {
            rWnd.top += (rWnd.Height() - nHeight) / 2;

            // create and write effective sample size text
            swprintf_s(szText, _T("%u Bit"), pModel->GetQuantization());
            pDC->DrawText(szText, -1, rWnd, DT_SINGLELINE | DT_TOP | DT_LEFT | DT_NOCLIP);
            rWnd.top += 5 * tm.tmHeight / 4;

            // create and write sample rate text
            swprintf_s(szText, _T("%ld Hz"), pModel->GetSamplesPerSec());
            pDC->DrawText(szText, -1, rWnd, DT_SINGLELINE | DT_TOP | DT_LEFT | DT_NOCLIP);
            rWnd.top += 5 * tm.tmHeight / 4;

            // create and write length text
			// get sampled data size in seconds
            double fDataSec = pModel->GetTimeFromBytes(pModel->GetDataSize());
            swprintf_s(szText, _T("%-5.1f sec"), fDataSec);
            pDC->DrawText(szText, -1, rWnd, DT_SINGLELINE | DT_TOP | DT_LEFT | DT_NOCLIP);
        }
    }
    // draw the dimension text for the xscale
    if ((!pGraph->IsAreaGraph() || pGraph->IsPlotID(IDD_RECORDING)) && !pGraph->IsPlotID(IDD_TWC) &&
            pGraph->HaveXScale()  && ((nBottom - tm.tmHeight - 2) > rWnd.top)) {
        // check if text available
        CString * pszText = pGraph->GetXScaleWnd()->GetDimensionText();
        if (!pszText->IsEmpty()) {
            // set up the text rectangle
            CRect rText(rWnd.left, nBottom - tm.tmHeight - 2, rWnd.right - 3, nBottom - 1);
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
            pDC->DrawText(*pszText, -1, rText, DT_SINGLELINE | DT_VCENTER | DT_RIGHT | DT_NOCLIP);
			// set back normal text font
            pDC->SelectObject(&m_font);
        }
    }
    // create the recalculation button
    if ((pGraph->IsAreaGraph() && !pGraph->IsPlotID(IDD_RECORDING) && !pGraph->IsPlotID(IDD_SNAPSHOT) &&
            (pGraph->HaveXScale() || !(m_nScaleMode & SCALE))) && ((nBottom - tm.tmHeight - 4) > rWnd.top)) {
        // set up the recalculation button rectangle
        m_rRecalc.SetRect(rWnd.left, nBottom - tm.tmHeight - 4, rWnd.right - 3, nBottom - 2);
        // create the pens
        CPen penLtgray(PS_SOLID, 1, pColors->cSysBtnHilite);
        CPen penDkgray(PS_SOLID, 1, pColors->cSysBtnShadow);
        pOldPen = pDC->SelectObject(&penLtgray);
        if (m_bRecalcUp) {
            // draw up state
            pDC->MoveTo(m_rRecalc.left, m_rRecalc.bottom - 1);
            pDC->LineTo(m_rRecalc.left, m_rRecalc.top);
            pDC->LineTo(m_rRecalc.right - 1, m_rRecalc.top);
            pDC->MoveTo(m_rRecalc.left + 1, m_rRecalc.bottom - 2);
            pDC->LineTo(m_rRecalc.left + 1, m_rRecalc.top + 1);
            pDC->LineTo(m_rRecalc.right - 1, m_rRecalc.top + 1);
            pDC->SelectObject(&penDkgray);
            pDC->MoveTo(m_rRecalc.right - 1, m_rRecalc.top);
            pDC->LineTo(m_rRecalc.right - 1, m_rRecalc.bottom - 1);
            pDC->LineTo(m_rRecalc.left, m_rRecalc.bottom - 1);
            pDC->MoveTo(m_rRecalc.right - 2, m_rRecalc.top + 1);
            pDC->LineTo(m_rRecalc.right - 2, m_rRecalc.bottom - 2);
            pDC->LineTo(m_rRecalc.left + 1, m_rRecalc.bottom - 2);
        } else {
            // draw down state
            pDC->MoveTo(m_rRecalc.right - 1, m_rRecalc.top);
            pDC->LineTo(m_rRecalc.right - 1, m_rRecalc.bottom - 1);
            pDC->LineTo(m_rRecalc.left, m_rRecalc.bottom - 1);
            pDC->MoveTo(m_rRecalc.right - 2, m_rRecalc.top + 1);
            pDC->LineTo(m_rRecalc.right - 2, m_rRecalc.bottom - 2);
            pDC->LineTo(m_rRecalc.left + 1, m_rRecalc.bottom - 2);
            pDC->SelectObject(&penDkgray);
            pDC->MoveTo(m_rRecalc.left, m_rRecalc.bottom - 1);
            pDC->LineTo(m_rRecalc.left, m_rRecalc.top);
            pDC->LineTo(m_rRecalc.right -1 , m_rRecalc.top);
            pDC->MoveTo(m_rRecalc.left + 1, m_rRecalc.bottom - 2);
            pDC->LineTo(m_rRecalc.left + 1, m_rRecalc.top + 1);
            pDC->LineTo(m_rRecalc.right - 1, m_rRecalc.top + 1);
        }
        pDC->SelectObject(pOldPen);
        szText.LoadString(IDS_RECALC_BUTTON);
        pDC->DrawText(szText, -1, m_rRecalc, DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_NOCLIP);
    } else {
        m_rRecalc.SetRect(0, 0, 0, 0);
    }
	// set back old font
    pDC->SelectObject(pOldFont);

}

/***************************************************************************/
// CLegendWnd::OnEraseBkgnd Erasing background
/***************************************************************************/
BOOL CLegendWnd::OnEraseBkgnd(CDC * pDC) {
    // get background color from main frame
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    // create the background brush
    CBrush backBrush(pMainWnd->GetColors()->cScaleBkg);
    CBrush * pOldBrush = pDC->SelectObject(&backBrush);
    CRect rClip;
	// erase the area needed
    pDC->GetClipBox(&rClip);
    pDC->PatBlt(rClip.left, rClip.top, rClip.Width(), rClip.Height(), PATCOPY);
    pDC->SelectObject(pOldBrush);
    return TRUE;
}


/***************************************************************************/
// CLegendWnd::OnRButtonDown Mouse right button down
// This event initiates a popup menu. The graph also has to get focus, so
// the parent graph is informed to do this.
/***************************************************************************/
void CLegendWnd::OnRButtonDown(UINT nFlags, CPoint point) {

    // inform parent graph
    GetParent()->SendMessage(WM_RBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
    // handle the floating popup menu
	CMainFrame * pMainWnd = (CMainFrame*)AfxGetMainWnd();
    CMenu menu;
    if (menu.LoadMenu(pMainWnd->GetPopup())) {
        CMenu & popup = *menu.GetSubMenu(4);
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
        popup.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,  pMainWnd);
    }
    CWnd::OnRButtonDown(nFlags, point);
}

/***************************************************************************/
// CLegendWnd::OnLButtonDown Mouse left button down
// This event should set the focus to the parent, so the window informs the
// parent graph to do this. Then it also sets the recalculation button to
// the down state and redraws it, if the click hits the buttons area.
/***************************************************************************/
void CLegendWnd::OnLButtonDown(UINT nFlags, CPoint point) {
    // inform parent graph
    if (m_rRecalc.PtInRect(point)) {
        // recalculation button hit
        m_bRecalcUp = FALSE;
        InvalidateRect(m_rRecalc);
		// get all further mouse input
        SetCapture();
    }
    CGraphWnd * pWnd = (CGraphWnd *)GetParent();
	// send message to parent
    pWnd->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
    CWnd::OnLButtonDown(nFlags, point);
}

/***************************************************************************/
// CLegendWnd::OnLButtonUp Mouse left button down
// If the recalculation button is in the down state, it releases the button
// and starts a recalculation process.
/***************************************************************************/
void CLegendWnd::OnLButtonUp(UINT nFlags, CPoint point) {
    if (!m_bRecalcUp) {
        m_bRecalcUp = TRUE;
        InvalidateRect(m_rRecalc);
        ReleaseCapture();
        if (m_rRecalc.PtInRect(point)) {
			// send restart process
            GetParent()->GetParent()->SendMessage(WM_COMMAND, ID_RESTART_PROCESS, 0L);
        }
    }
    CWnd::OnLButtonUp(nFlags, point);
}

/***************************************************************************/
// CLegendWnd::OnMouseMove Mouse move
// If the recalculation button is in the down state, and the mouse moves
// outside of the button, it releases it.
/***************************************************************************/
void CLegendWnd::OnMouseMove(UINT nFlags, CPoint point) {
    if (!m_bRecalcUp) {
        if (!m_rRecalc.PtInRect(point)) {
            // move not over recalculation button
            m_bRecalcUp = TRUE;
            InvalidateRect(m_rRecalc);
            ReleaseCapture();
        }
    }
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CPlotWnd * pPlot = pGraph->GetPlot();
    CSaView * pView = (CSaView *)pGraph->GetParent();
	// clear mouse position
    pPlot->SetMousePointerPosition(CPoint(UNDEFINED_OFFSET, UNDEFINED_OFFSET));
	// reset state
    pPlot->SetMouseButtonState(0);
    const BOOL bForceUpdate = TRUE;
    pGraph->UpdateStatusBar(pView->GetStartCursorPosition(), pView->GetStopCursorPosition(), bForceUpdate);
    CWnd::OnMouseMove(nFlags, point);
}

int CLegendWnd::GetScaleMode() {
	// return scale mode
    return m_nScaleMode;
}
double CLegendWnd::GetFirstGridPosition() {
	// return position of first gridline
    return m_dFirstGridPos;
}
double CLegendWnd::GetGridDistance() {
	// return distance between gridlines
    return m_fGridDistance;
}
CString * CLegendWnd::GetDimensionText() {
	// return pointer to dimension text
    return &m_szScaleDimension;
}
double CLegendWnd::GetScaleBase() {
	// return base of scale
    return m_fBase;
}
double CLegendWnd::GetScaleMaxValue() {
	// return max scale value
    return m_dScaleMaxValue;
}
double CLegendWnd::GetScaleMinValue() {
	// return min scale value
    return m_dScaleMinValue;
}
