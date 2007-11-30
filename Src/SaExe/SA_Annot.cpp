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
//         SDM change CAnnotationWnd::OnLMouseDown to use CASegmentSelection to select segment
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
//         SDM moved creation of CAnnotationEdit to OnCreateEdit
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
#include "Process\sa_proc.h"
#include "sa_segm.h"
#include "dsp\dspTypes.h"
// SDM 1.06.6
#include "sa_edit.h"
#include "sa_doc.h"
#include "sa.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "math.h"
#include "sa_g_stf.h"
#include "sa_g_twc.h"
#include "Partiture.hpp"

#include "sa_g_wavelet.h"				// ARH 8/3/01  Added to use the arrow buttons on the wavelet graph


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
    //{{AFX_MSG_MAP(CLegendWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_RBUTTONDOWN()
    ON_WM_LBUTTONDOWN()
    ON_WM_CREATE()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLegendWnd construction/destruction/creation

extern CSaApp NEAR theApp;
/***************************************************************************/
// CLegendWnd::CLegendWnd Constructor
/***************************************************************************/
CLegendWnd::CLegendWnd()
{
  m_nScaleMode = NO_SCALE | SCALE_INFO;
  m_nGridDivisions = -1; // set divisions
  m_dScaleMinValue = 0;  // scale min value
  m_dScaleMaxValue = 100;  // scale max value
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
CLegendWnd::~CLegendWnd()
{
}

/***************************************************************************/
// CLegendWnd::PreCreateWindow Creation
// Called from the framework before the creation of the window. Registers
// the new window class.
/***************************************************************************/
BOOL CLegendWnd::PreCreateWindow(CREATESTRUCT& cs)
{
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
int CLegendWnd::GetWindowWidth()
{
  // get width of the legend window font
  TEXTMETRIC tm;
  CDC* pDC = GetDC(); // get device context
  CFont* pOldFont = pDC->SelectObject(&m_font); // select legend font
  pDC->GetTextMetrics(&tm); // get text metrics
  pDC->SelectObject(pOldFont);  // set back old font
  ReleaseDC(pDC);
  CString szText;
  szText.LoadString(IDS_WINDOW_PHONEMIC); // is longest text in legend
  return tm.tmAveCharWidth * (szText.GetLength() + 2) + 3; // return font width
}

/***************************************************************************/
// CLegendWnd::SetScale Set the legend scale
// There are different modes for the legend scale possible. nMode sets this
// mode. Then the values tell the vertical range and the string will be drawn
// as dimension title. The string pointers can be NULL (no dimension drawn).
// The function returns TRUE if the legend window has to be redrawn.
/***************************************************************************/
BOOL CLegendWnd::SetScale(int nMode, double dMinValue, double dMaxValue, TCHAR* pszDimension, int nGridDivisions,  double d3dOffset)
{
  BOOL bRedraw = FALSE;
  if (m_nScaleMode != nMode)
  {
    m_nScaleMode = nMode; // set scale mode
    bRedraw = TRUE;
  }
  if (m_nGridDivisions != nGridDivisions)
  {
    m_nGridDivisions = nGridDivisions; // set divisions
    bRedraw = TRUE;
  }
  if (m_d3dOffset != d3dOffset)
  {
    m_d3dOffset = d3dOffset;
    bRedraw = TRUE;
  }
  if (m_dScaleMinValue != dMinValue)
  {
    m_dScaleMinValue = dMinValue; // set scale min value
    bRedraw = TRUE;
  }
  if (m_dScaleMaxValue != dMaxValue)
  {
    m_dScaleMaxValue = dMaxValue; // set scale max value
    bRedraw = TRUE;
  }
  if (pszDimension)
  {
    if (m_szScaleDimension != pszDimension)
    {
      m_szScaleDimension = pszDimension; // set scale dimension text
      bRedraw = TRUE;
    }
  }
  else if (!m_szScaleDimension.IsEmpty())
  {
    m_szScaleDimension.Empty();
    bRedraw = TRUE;
  }
  if (bRedraw)
    m_bRecalculate = TRUE;
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
void CLegendWnd::CalculateScale(CDC* pDC, CRect* prWnd)
{
  CGraphWnd* pGraph = (CGraphWnd*)GetParent(); // get pointer to graph
  // calculate new scale with magnify
  double fMagnify = pGraph->GetMagnify(); // get magnify
  if (!m_bRecalculate && (prWnd->Height() == m_nHeightUsed)
    && (m_fMagnifyUsed == fMagnify) && (m_nDivisionsUsed == m_nGridDivisions))
    return; // scale up to date
  if (prWnd->Height() <= 0)
    return; // no scale to draw
  // scale has changed, recalculate
  m_bRecalculate = FALSE;
  m_nHeightUsed = prWnd->Height();
  m_nDivisionsUsed = m_nGridDivisions;
  m_fMagnifyUsed = fMagnify;
  // calculate new scale with magnify
  double dMinValue = m_dScaleMinValue;
  double dMaxValue = dMinValue + (m_dScaleMaxValue - dMinValue) / fMagnify;
  if (m_dScaleMinValue < 0)
  {
    dMinValue = m_dScaleMinValue / fMagnify;
    dMaxValue = m_dScaleMaxValue / fMagnify;
  }
  if (m_nScaleMode & LOG10)
  {
    // logarithmic scale
    if(dMinValue < 0.)
      // Log plots must draw ranges greater than zero
      dMinValue = std::numeric_limits<double>::min();
    if (dMaxValue < dMinValue)
      // Log plots must draw non-inverted
      dMaxValue = dMinValue + std::numeric_limits<double>::min();
    m_fBase = log10(dMinValue);
    double fTop = log10(dMaxValue);
    if (fTop <= m_fBase)
      // The displayed range is small make big enough to avoid divide by zero
      fTop += 1.5*fTop*std::numeric_limits<double>::epsilon();

    m_fGridDistance = (double)(prWnd->Height() - 2) / (fTop - m_fBase); // distance
    if (m_fGridDistance < 1.0)
    {
      // Why is this code necessary... Height() could be zero, but isn't 0 a legitimate grid distance
      ASSERT(FALSE);
      TRACE(_T("Exceptional legend grid distance\n"));
      m_fGridDistance = 1.0;
    }
    m_dFirstGridPos = prWnd->bottom - 1;  // location which should map to m_dScaleMinValue
  }
  else
  {
    // linear scale
    m_fNumbPerPix = (dMaxValue - dMinValue) / ((double)prWnd->Height()/* - 3*/);
    if(m_nGridDivisions == -1)
    {
      TEXTMETRIC tm;
      // check if pDC known
      if (pDC)
        pDC->GetTextMetrics(&tm); // get text metrics
      else
      {
        // get pDC to get textmetrics
        CDC* pDC = GetDC(); // get device context
        CFont* pOldFont = pDC->SelectObject(&m_font); // select legend font
        pDC->GetTextMetrics(&tm); // get text metrics
        pDC->SelectObject(pOldFont);  // set back old font
        ReleaseDC(pDC);
      }
      // find minimum scale distance in measures
      DWORD dwMinScale = (DWORD)(m_fNumbPerPix * (double)(2 * tm.tmHeight));
      if (!dwMinScale)
        dwMinScale = 1;
      // find the 10 based log of this distance
      DWORD dw10Base = (DWORD)log10((double)dwMinScale);
      // now find the next appropriate scale division
      // Fix axis min problem in log plots CLW 9/21/00
      m_fBase = pow(10, (double)dw10Base);
      if ((DWORD)(m_fBase * 2) < dwMinScale)
      {
        if (((DWORD)m_fBase * 5) % 2)
          m_fBase *= 6;
        else
          m_fBase *= 5;
      }
      else
        m_fBase *= 2;
      if ((DWORD)m_fBase < dwMinScale)
        m_fBase = pow(10, (double)(++dw10Base));
    }
    else
    {
      m_fBase = (m_dScaleMaxValue - m_dScaleMinValue)/m_nGridDivisions;
    }
    // calculate grid distance
    m_fGridDistance = m_fBase / m_fNumbPerPix; // gridline distance in pixels
    // calculate first (top) gridline position in plot window client coordinates
    if(!(m_nScaleMode & ARBITRARY))
      m_dFirstGridPos = prWnd->bottom - (floor(dMaxValue/(m_fBase))*m_fBase - dMinValue) / m_fNumbPerPix;
    else
      m_dFirstGridPos = prWnd->top;
  }
}

/////////////////////////////////////////////////////////////////////////////
// CLegendWnd message handlers

/***************************************************************************/
// CLegendWnd::OnCreate Window creation
// Creation of the legend font, used in the window.
/***************************************************************************/
int CLegendWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CWnd::OnCreate(lpCreateStruct) == -1)
    return -1;
  // create legend font
  const TCHAR* pszName = _T("MS Sans Serif");
  m_font.CreateFont(15, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                    ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                    DEFAULT_QUALITY, (VARIABLE_PITCH | FF_ROMAN), pszName);


  // get pointer to graph
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  if (pGraph->IsPlotID(IDD_STAFF))
  {
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
    //      ::SendMessage(hVoiceButton,   WM_SETFONT, (WPARAM)m_font.m_hObject, 0);
    //      ::SendMessage(hTempoButton,   WM_SETFONT, (WPARAM)m_font.m_hObject, 0);
    //      ::SendMessage(hConvertButton, WM_SETFONT, (WPARAM)m_font.m_hObject, 0);
  }
  else if (pGraph->IsPlotID(IDD_TWC) || pGraph->IsPlotID(IDD_WAVELET))
  {
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
  }
  else
  {
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
extern "C" void cdecl Splash(HWND hWnd,HINSTANCE hInst,char *BitmapName);
BOOL CLegendWnd::OnCommand(WPARAM wParam, LPARAM /* lParam */)
{
  CGraphWnd* pGraph = (CGraphWnd*)GetParent(); // get pointer to graph

  if (pGraph->IsPlotID(IDD_STAFF))
  {
    CPlotStaff *staff = (CPlotStaff*) pGraph->GetPlot();
    switch(LOWORD(wParam))
    {
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
      case IDM_CONVERT:      //DDO - 08/14/2000
        staff->SetFocusedGraph(pGraph);
        staff->Convert();
        staff->SetFocusedGraph(pGraph);// for some reason I have to do this again!
        break;
      case IDM_EXPORT:
        staff->SetFocusedGraph(pGraph);
        staff->ExportFile();
        staff->SetFocusedGraph(pGraph);// for some reason I have to do this again!
        break;
      case IDM_IMPORT:
        staff->SetFocusedGraph(pGraph);
        staff->ImportFile();
        staff->SetFocusedGraph(pGraph);// for some reason I have to do this again!
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
  }
  else if (pGraph->IsPlotID(IDD_TWC))
  {
    static_cast<CSaView*>(pGraph->GetParent())->SetFocusedGraph(pGraph);
    CPlotTonalWeightChart *pTWC = (CPlotTonalWeightChart*) pGraph->GetPlot();
    WORD UpDown = LOWORD(wParam);
    BOOL bKeyShifted = GetKeyState(VK_SHIFT) >> 8;
    pTWC->AdjustSemitoneOffset(UpDown,bKeyShifted);
    return TRUE;
  }
	
	// 8/03/01 ARH Added to use arrows on wavelet graph
	else if (pGraph->IsPlotID(IDD_WAVELET))
	{
		CPlotWavelet *pWavelet = (CPlotWavelet *) pGraph->GetPlot();
    switch(LOWORD(wParam))
    {
			case IDM_UP:
				pWavelet->IncreaseDrawingLevel();
				pWavelet->RedrawPlot(true);
				//MessageBox(_T("up"), "up", MB_OK);
				break;

			case IDM_DOWN:
				pWavelet->DecreaseDrawingLevel();
				pWavelet->RedrawPlot(true);
				//MessageBox(_T("down"), "down", MB_OK);
				break;

		}
		return TRUE;

	}

  else
    return FALSE;
}

/***************************************************************************/
// CLegendWnd::OnSetFocus
// If this is a Staff Control, keyboard focus is given to the staff edit window
/***************************************************************************/
void CLegendWnd::OnSetFocus(CWnd *)
{
  // get pointer to graph
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  if (pGraph->IsPlotID(IDD_STAFF))
  {
    ::SetFocus(pGraph->GetPlot()->m_hWnd);
  }
}

/***************************************************************************/
// CLegendWnd::OnPaint
// Paints using OnDraw, which is shared with OnPrint
/***************************************************************************/
void CLegendWnd::OnPaint()
{
  CRect dummyRect1(0,0,0,0);
  CRect dummyRect2(0,0,0,0);
  CRect dummyRect3(0,0,0,0);

  CPaintDC dc(this); // device context for painting
  
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
                        const CRect * printAnnotation)
{
  // get pointer to graph
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();

  // Create staff control buttons - TCJ 4/19/2000
  if (pGraph->IsPlotID(IDD_STAFF))
  {
    CPlotStaff *staff = (CPlotStaff*) pGraph->GetPlot();
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
  CFont* pOldFont = pDC->SelectObject(&m_font);
  // get text metrics
  TEXTMETRIC tm;
  pDC->GetTextMetrics(&tm);
  // calculate new scale with magnify
  double fMagnify = pGraph->GetMagnify(); // get magnify
  double dScaleMinValue = m_dScaleMinValue;
  double dScaleMaxValue = dScaleMinValue + (m_dScaleMaxValue - dScaleMinValue) / fMagnify;
  if (m_dScaleMinValue < 0)
  {
    dScaleMinValue = m_dScaleMinValue / fMagnify;
    dScaleMaxValue = m_dScaleMaxValue / fMagnify;
  }
  // get coordinates of plot window
  CRect rPlotWnd;

  if (pDC->IsPrinting())
  {
    rPlotWnd = printPlotWnd;
  }
  else
  {
    pGraph->GetPlot()->GetClientRect(rPlotWnd);
  }

  // calculate scale parameters
  CalculateScale(pDC, &rPlotWnd); // calculate the scale
  // get window coordinates
  CRect rWnd;

  if (pDC->IsPrinting())
  {
    rWnd = printRect;
  }
  else
  {
    GetClientRect(rWnd);
  }
  if (rWnd.Height() == 0)
  {
    pDC->SelectObject(pOldFont);  // set back old font
    return; // nothing to draw
  }

  int YDown = rWnd.Height() - 23;
  int YUp   = rWnd.Height() - 43;
  ::MoveWindow(m_hDownButton, 6, YDown,20, 20, TRUE);
  ::MoveWindow(m_hUpButton,   6, YUp,  20, 20, TRUE);

  // get pointer view and document
  CSaView* pView = (CSaView*)pGraph->GetParent();
  CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();
  // set font colors
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd(); // get pointer to colors from main frame
  Colors* pColors = pMainWnd->GetColors();
  pDC->SetTextColor(pColors->cScaleFont); // set font color
  pDC->SetBkMode(TRANSPARENT); // letters may overlap, so they must be transparent
  int nBottom = rWnd.bottom; // save bottom
  int nTop = rWnd.top; // save top
  rWnd.left += 2;
  rWnd.bottom = -2;

  int StringIndex[] =
  {
      IDS_WINDOW_PHONETIC,
      IDS_WINDOW_TONE,
      IDS_WINDOW_PHONEMIC,
      IDS_WINDOW_ORTHO,
      IDS_WINDOW_GLOSS,
      IDS_WINDOW_REFERENCE,
      IDS_WINDOW_MUSIC_PL1,
      IDS_WINDOW_MUSIC_PL2,
      IDS_WINDOW_MUSIC_PL3,
      IDS_WINDOW_MUSIC_PL4
  };


  CString szText;
  for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER;nLoop++)
  {
    int current = CGraphWnd::m_anAnnWndOrder[nLoop];
    if (pGraph->HaveAnnotation(current))
    {
      // gloss window is visible
      szText.LoadString(StringIndex[current]);
      if (pDC->IsPrinting())
      {
        rWnd.bottom += printAnnotation[current].Height();
      }
      else
      {
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
  CPen* pOldPen = pDC->SelectObject(&penLtgray);
  
	pDC->MoveTo(rWnd.right - 1, rWnd.top);
	pDC->LineTo(rWnd.right - 1, rWnd.bottom);
	pDC->SelectObject(&penDkgray);
	if (rWnd.Height() > 0)
	{ pDC->MoveTo(rWnd.right - 2, rWnd.top);
	  pDC->LineTo(rWnd.right - 2, rWnd.bottom - 1);
  }

  rWnd.top = rWnd.bottom;
  rWnd.bottom = nBottom; // restore bottom
  if (pGraph->HaveXScale())
  {
    if (pDC->IsPrinting())
    {
      rWnd.bottom -= printXscaleRect.Height();
    }
    else
    {
      rWnd.bottom -= pGraph->GetXScaleWnd()->GetWindowHeight();
    }
  }
  if (rWnd.Height() > 0)
  {
    pDC->MoveTo(rWnd.right - 1, rWnd.top - 1);
    pDC->LineTo(rWnd.right - 1, rWnd.bottom);
  }
  pDC->SelectObject(pOldPen);
  rWnd.right -= 1; // don't draw over the border
  if ((m_nScaleMode != NO_SCALE) || (rPlotWnd.Height() <= 0))
  {
    if (rWnd.Height() < tm.tmHeight)
    {
      pDC->SelectObject(pOldFont); // set back old font
      return; // not enough space to draw scale
    }
    // set default scale limits
    int nTextTopLimit = rWnd.top + tm.tmHeight / 2; // upper text position limit
    int nTextBottomLimit = nBottom - tm.tmHeight / 2; // lower text position limit
    int nLineTopLimit = rWnd.top; // upper line position limit
    int nLineBottomLimit = rWnd.bottom; // lower line position limit
    // draw the vertical dimension text
    if (!m_szScaleDimension.IsEmpty())
    {
      // create the vertical font
      LOGFONT logFont;
      m_font.GetObject(sizeof(LOGFONT), (void*)&logFont); // fill up logFont
      logFont.lfEscapement = 900; // turn it 90 degrees
      logFont.lfWeight = FW_BOLD;

		 logFont.lfHeight = -13;

      _stprintf(logFont.lfFaceName, _T("%s"), _T("Arial"));
      CFont vFont;
      vFont.CreateFontIndirect(&logFont); // create the vertical font
      pDC->SelectObject(&vFont); // select the vertical font
      TEXTMETRIC tmv;
      pDC->GetTextMetrics(&tmv);
      // enough space?
      int nHeight = (m_szScaleDimension.GetLength() + 2) * tmv.tmAveCharWidth;
      if (nHeight < rWnd.Height()) // draw the text
        pDC->TextOut(rWnd.left, rWnd.bottom - (rWnd.Height() - nHeight) / 2, m_szScaleDimension);
      pDC->SelectObject(&m_font); // set back normal text font
    }
    // create line pen
    CPen pen(PS_SOLID, 1, pColors->cScaleLines);
    CPen* pOldPen = pDC->SelectObject(&pen);
    // draw the scale
    int nHorPos = rWnd.right - 4; // vertical line horizontal position
    // draw the vertical line
    pDC->MoveTo(nHorPos, rWnd.bottom - 1);
    if (fMagnify <= 1.0 || m_d3dOffset != 0.) pDC->LineTo(nHorPos, (int) floor(rWnd.top + rWnd.Height()*m_d3dOffset) - 1);
    else pDC->LineTo(nHorPos, rWnd.top - 1);
    // draw the arrows
    if ((dScaleMaxValue > 0) && (fMagnify <= 1.0) && (m_d3dOffset == 0.))
    {
      // draw up arrow
      pDC->LineTo(nHorPos - 2, rWnd.top + 5);
      pDC->LineTo(nHorPos + 2, rWnd.top + 5);
      pDC->LineTo(nHorPos, rWnd.top + 2);
      pDC->MoveTo(nHorPos + 2, rWnd.top + 6);
      pDC->LineTo(nHorPos - 3, rWnd.top + 6);
      // modify the scale line limits
      nLineTopLimit = rWnd.top + 7; // upper line position limit
    }
    if ((dScaleMinValue < 0) && ((fMagnify <= 1.0) || (dScaleMaxValue == 0)))
    {
      // draw down arrow
      pDC->MoveTo(nHorPos, rWnd.bottom);
      pDC->LineTo(nHorPos - 2, rWnd.bottom - 5);
      pDC->LineTo(nHorPos + 2, rWnd.bottom - 5);
      pDC->LineTo(nHorPos, rWnd.bottom);
      pDC->MoveTo(nHorPos + 2, rWnd.bottom - 4);
      pDC->LineTo(nHorPos - 2, rWnd.bottom - 4);
      // modify the scale line limits
      nLineBottomLimit = rWnd.bottom - 6; // lower line position limit
    }
    if (m_nScaleMode & SCALE)
    {
      if (m_nScaleMode & LOG10)
      {
        // logarithmic scale
        int nLogPos = (int) floor(m_fBase);
        int nLogDisp = (int) ceil(dScaleMinValue / pow(10, floor(m_fBase)));
        double fPixelBase = rWnd.top + m_dFirstGridPos + (m_fBase - nLogPos) * m_fGridDistance;
        int nPixelPos = round(fPixelBase);
        int nLastNumber = nPixelPos + 2 * tm.tmHeight;
        int nBase = (int)pow(10, (double)nLogPos);
        while(nPixelPos > nLineTopLimit)
        {
          if (nLogDisp > 9)
          {
            nLogDisp = 1;
            nLogPos++;
            fPixelBase -= m_fGridDistance;
            nPixelPos = round(fPixelBase);
            if (nPixelPos > nLineTopLimit)
            {
              // draw large line
              pDC->MoveTo(nHorPos - 3, nPixelPos);
              pDC->LineTo(nHorPos + 1, nPixelPos);
            }
            else
              break;
            nBase = (int)pow(10, (double)nLogPos);
          }
          else
          {
            // draw small line
            nPixelPos = round(fPixelBase - log10((double)nLogDisp) * m_fGridDistance);
            if (nPixelPos > nLineTopLimit)
            {
              pDC->MoveTo(nHorPos - 2, nPixelPos);
              pDC->LineTo(nHorPos, nPixelPos);
            }
            else break;
          }
          // draw the number
          if ((m_nScaleMode & NUMBERS) && (nPixelPos > nTextTopLimit) && (nPixelPos < nTextBottomLimit)
            && (nLastNumber > (nPixelPos + tm.tmHeight + 2)) && (nLogDisp < 6) && (nLogDisp != 4))
          {
            CRect rText(rWnd.left, nPixelPos - tm.tmHeight, rWnd.right - 9, nPixelPos + tm.tmHeight);
            TCHAR szText[16];
            _stprintf(szText, _T("%d"), nBase * nLogDisp);
            pDC->DrawText(szText, -1, rText, DT_SINGLELINE | DT_VCENTER | DT_RIGHT | DT_NOCLIP);
            nLastNumber = nPixelPos;
          }
          nLogDisp++;
        }
      }
      else // linear scale
      {
        // draw the positive lines and numbers
        if (m_dScaleMaxValue == 0) dScaleMinValue = m_dScaleMinValue;

        double dScaleMinLoopValue = dScaleMinValue;
        double dScaleMaxLoopValue = dScaleMaxValue;

        BOOL bLargeLine = TRUE;

        if(!(m_nScaleMode & ARBITRARY))
        {
          int nHalfTicks = (int) floor(-dScaleMinLoopValue/(m_fBase/2));
          
          // The first tick needs to be a multiple of (m_fBase/2) from zero
          dScaleMinLoopValue = - nHalfTicks * (m_fBase/2);

          // we will draw a number here if it is an even number of half ticks from zero
          bLargeLine = ((nHalfTicks & 1) == 0);
        }

        // there are positive values or no magnify (we need at least the zero line)
        for (double dLoop = dScaleMinLoopValue; m_fBase > 0 ? dLoop <= dScaleMaxLoopValue : dLoop >= dScaleMaxLoopValue; dLoop += (m_fBase / 2))
        {
          // calculate line position
          int nPos = round(rWnd.bottom - (double)(1 - m_d3dOffset)*(dLoop - dScaleMinValue) / m_fNumbPerPix);
          if (nPos <= nLineTopLimit)
            break;
          // draw the line
          if (nPos <= nLineBottomLimit)
          {
            if (bLargeLine)
            {
              // draw a large line
              pDC->MoveTo(nHorPos - 3, nPos);
              pDC->LineTo(nHorPos + 1, nPos);
              // draw the number
              if ((m_nScaleMode & NUMBERS) && (nPos > nTextTopLimit) && (nPos < nTextBottomLimit))
              {
                CRect rText(rWnd.left, nPos - tm.tmHeight, rWnd.right - 9, nPos + tm.tmHeight);
                TCHAR szText[16];
                int nWidth = (int) -log10(m_fBase);
                if(nWidth < 0)
                  nWidth = 0;
                _stprintf(szText, _T("%.*f"), nWidth, dLoop);
                pDC->DrawText(szText, -1, rText, DT_SINGLELINE | DT_VCENTER | DT_RIGHT | DT_NOCLIP);
              }
            }
            else
            {
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
  }
  else if (m_nScaleMode & SCALE_INFO && !pGraph->IsPlotID(IDD_STAFF)) // TCJ 4/20/00
  {
    // no scale, but text
    TCHAR szText[16];
    int nHeight = 7 * tm.tmHeight / 2;
    if (pGraph->IsAreaGraph() && !pGraph->IsPlotID(IDD_RECORDING))
      nHeight += tm.tmHeight + 4; // add extra space for recalculation button
    if (rWnd.Height() >= nHeight)
    {
      rWnd.top += (rWnd.Height() - nHeight) / 2;
      // get sa parameters document member data
      SaParm* pSaParm = pDoc->GetSaParm();
      // create and write effective sample size text
      _stprintf(szText, _T("%u Bit"), pSaParm->byQuantization);
      pDC->DrawText(szText, -1, rWnd, DT_SINGLELINE | DT_TOP | DT_LEFT | DT_NOCLIP);
      rWnd.top += 5 * tm.tmHeight / 4;
      // get sa parameters format member data
      FmtParm* pFmtParm = pDoc->GetFmtParm();
      // create and write sample rate text
      _stprintf(szText, _T("%ld Hz"), pFmtParm->dwSamplesPerSec);
      pDC->DrawText(szText, -1, rWnd, DT_SINGLELINE | DT_TOP | DT_LEFT | DT_NOCLIP);
      rWnd.top += 5 * tm.tmHeight / 4;
      // create and write length text
      double fDataSec = pDoc->GetTimeFromBytes(pDoc->GetUnprocessedDataSize()); // get sampled data size in seconds
      _stprintf(szText, _T("%-5.1f sec"), fDataSec);
      pDC->DrawText(szText, -1, rWnd, DT_SINGLELINE | DT_TOP | DT_LEFT | DT_NOCLIP);
    }
  }
  // draw the dimension text for the xscale
  if ((!pGraph->IsAreaGraph() || pGraph->IsPlotID(IDD_RECORDING)) && !pGraph->IsPlotID(IDD_TWC) &&
    pGraph->HaveXScale()  && ((nBottom - tm.tmHeight - 2) > rWnd.top))
  {
    // check if text available
    CString* pszText = pGraph->GetXScaleWnd()->GetDimensionText();
    if (!pszText->IsEmpty())
    {
      // set up the text rectangle
      CRect rText(rWnd.left, nBottom - tm.tmHeight - 2, rWnd.right - 3, nBottom - 1);
      // create the bold font
      LOGFONT logFont;
      m_font.GetObject(sizeof(LOGFONT), (void*)&logFont); // fill up logFont
      logFont.lfWeight = FW_BOLD;
      CFont bFont;
      bFont.CreateFontIndirect(&logFont); // create the bold font
      pDC->SelectObject(&bFont); // select the bold font
      pDC->DrawText(*pszText, -1, rText, DT_SINGLELINE | DT_VCENTER | DT_RIGHT | DT_NOCLIP);
      pDC->SelectObject(&m_font); // set back normal text font
    }
  }
  // create the recalculation button
  if ((pGraph->IsAreaGraph() && !pGraph->IsPlotID(IDD_RECORDING) && !pGraph->IsPlotID(IDD_SNAPSHOT) &&
    (pGraph->HaveXScale() || !(m_nScaleMode & SCALE))) && ((nBottom - tm.tmHeight - 4) > rWnd.top))
  {
    // set up the recalculation button rectangle
    m_rRecalc.SetRect(rWnd.left, nBottom - tm.tmHeight - 4, rWnd.right - 3, nBottom - 2);
    // create the pens
    CPen penLtgray(PS_SOLID, 1, pColors->cSysBtnHilite);
    CPen penDkgray(PS_SOLID, 1, pColors->cSysBtnShadow);
    pOldPen = pDC->SelectObject(&penLtgray);
    if (m_bRecalcUp)
    {
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
    }
    else
    {
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
  }
  else
    m_rRecalc.SetRect(0, 0, 0, 0);
  pDC->SelectObject(pOldFont);  // set back old font
  
}

/***************************************************************************/
// CLegendWnd::OnEraseBkgnd Erasing background
/***************************************************************************/
BOOL CLegendWnd::OnEraseBkgnd(CDC* pDC)
{
  // get background color from main frame
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
  // create the background brush
  CBrush backBrush(pMainWnd->GetColors()->cScaleBkg);
  CBrush* pOldBrush = pDC->SelectObject(&backBrush);
  CRect rClip;
  pDC->GetClipBox(&rClip); // erase the area needed
  pDC->PatBlt(rClip.left, rClip.top, rClip.Width(), rClip.Height(), PATCOPY);
  pDC->SelectObject(pOldBrush);
  return TRUE;
}


/***************************************************************************/
// CLegendWnd::OnRButtonDown Mouse right button down
// This event initiates a popup menu. The graph also has to get focus, so
// the parent graph is informed to do this.
/***************************************************************************/
void CLegendWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
  // inform parent graph
  GetParent()->SendMessage(WM_RBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
  // handle the floating popup menu
  CMenu mPopup;
  if (mPopup.LoadMenu(((CMainFrame*)AfxGetMainWnd())->GetPopup()))//SDM 1.5Test8.5
  {
    CMenu& pFloatingPopup = *mPopup.GetSubMenu(0);
    ASSERT(pFloatingPopup.m_hMenu != NULL);
    // attach the layout menu
    CMenu* mLayout = new CLayoutMenu;
    TCHAR szString[256]; // don't change the string
    if (pFloatingPopup.GetMenuString(ID_GRAPHS_LAYOUT, szString, sizeof(szString)/sizeof(TCHAR), MF_BYCOMMAND))//SDM 1.5Test8.5
    {
      if (mLayout)
        VERIFY(pFloatingPopup.ModifyMenu(ID_GRAPHS_LAYOUT, MF_BYCOMMAND | MF_POPUP, (UINT)mLayout->m_hMenu, szString));
    }
    // pop the menu up
    ClientToScreen(&point);
    pFloatingPopup.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,  AfxGetMainWnd());
    if (mLayout)
      delete mLayout;
  }
  CWnd::OnRButtonDown(nFlags, point);
}

/***************************************************************************/
// CLegendWnd::OnLButtonDown Mouse left button down
// This event should set the focus to the parent, so the window informs the
// parent graph to do this. Then it also sets the recalculation button to
// the down state and redraws it, if the click hits the buttons area.
/***************************************************************************/
void CLegendWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
  // inform parent graph
  if (m_rRecalc.PtInRect(point))
  {
    // recalculation button hit
    m_bRecalcUp = FALSE;
    InvalidateRect(m_rRecalc);
    SetCapture(); // get all further mouse input
  }
  CGraphWnd* pWnd = (CGraphWnd*)GetParent();
  pWnd->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y)); // send message to parent
  CWnd::OnLButtonDown(nFlags, point);
}

/***************************************************************************/
// CLegendWnd::OnLButtonUp Mouse left button down
// If the recalculation button is in the down state, it releases the button
// and starts a recalculation process.
/***************************************************************************/
void CLegendWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
  if (!m_bRecalcUp)
  {
    m_bRecalcUp = TRUE;
    InvalidateRect(m_rRecalc);
    ReleaseCapture();
    if (m_rRecalc.PtInRect(point))
      GetParent()->GetParent()->SendMessage(WM_COMMAND, ID_RESTART_PROCESS, 0L); // send restart process
  }
  CWnd::OnLButtonUp(nFlags, point);
}

/***************************************************************************/
// CLegendWnd::OnMouseMove Mouse move
// If the recalculation button is in the down state, and the mouse moves
// outside of the button, it releases it.
/***************************************************************************/
void CLegendWnd::OnMouseMove(UINT nFlags, CPoint point)
{
  if (!m_bRecalcUp)
  {
    if (!m_rRecalc.PtInRect(point))
    {
      // move not over recalculation button
      m_bRecalcUp = TRUE;
      InvalidateRect(m_rRecalc);
      ReleaseCapture();
    }
  }
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  CPlotWnd* pPlot = pGraph->GetPlot();
  CSaView* pView = (CSaView*)pGraph->GetParent();
  pPlot->SetMousePointerPosition(CPoint(UNDEFINED_OFFSET, UNDEFINED_OFFSET)); // clear mouse position 
  pPlot->SetMouseButtonState(0);  // reset state
  const int bForceUpdate = TRUE;
  pGraph->UpdateStatusBar(pView->GetStartCursorPosition(), pView->GetStopCursorPosition(), bForceUpdate);
  CWnd::OnMouseMove(nFlags, point);
}

//###########################################################################
// CXScaleWnd
// X-Scale window contained in the graphs. It displays the x-axis dimension
// and scale for the graphs data. It has to keep track with scrolling and
// zooming.

/////////////////////////////////////////////////////////////////////////////
// CXScaleWnd message map

BEGIN_MESSAGE_MAP(CXScaleWnd, CWnd)
    //{{AFX_MSG_MAP(CXScaleWnd)
    ON_WM_ERASEBKGND()
    ON_WM_RBUTTONDOWN()
    ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
    ON_WM_PAINT()
    ON_WM_CREATE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXScaleWnd construction/destruction/creation

/***************************************************************************/
// CXScaleWnd::CXScaleWnd Constructor
/***************************************************************************/
CXScaleWnd::CXScaleWnd()
{
  m_nScaleMode = SCALE | NUMBERS | TIME_FROM_VIEW;
  m_nGridDivisions = -1; // set divisions
  m_fScaleMinValue = 0;  // scale min value
  m_fScaleMaxValue = 100;  // scale max value
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
CXScaleWnd::~CXScaleWnd()
{
}

/***************************************************************************/
// CXScaleWnd::PreCreateWindow Creation
// Called from the framework before the creation of the window. Registers
// the new window class.
/***************************************************************************/
BOOL CXScaleWnd::PreCreateWindow(CREATESTRUCT& cs)
{
  // register the window class
  cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
    AfxGetApp()->LoadStandardCursor(IDC_ARROW), 0, 0);
  return CWnd::PreCreateWindow(cs);
}

/***************************************************************************/
// CXScaleWnd::GetWindowHeight
// Calculating the height of the current font.
/***************************************************************************/
int CXScaleWnd::GetWindowHeight()
{
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();

  //*************************************************************
  // 09/27/2000 - DDO
  // If this x-scale's graph is the TWC graph then make sure
  // the x-scale window height is large enough so the TWC plot
  // window's height is exactly the same height as the melogram
  // window's plot. All other graphs will have a fixed x-scale
  // window height.
  //*************************************************************
  if (pGraph->IsPlotID(IDD_TWC)) return GetTWCXScaleWindowHeight();

  // get height of the x-scale window font
  TEXTMETRIC tm;
  CDC* pDC = GetDC(); // get device context
  CFont* pOldFont = pDC->SelectObject(&m_font); // select x-scale font
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
int CXScaleWnd::GetTWCXScaleWindowHeight()
{
  //***********************************************************
  // Get a pointer to the melogram's plot window.
  //***********************************************************
  CGraphWnd * pGraph    = (CGraphWnd*)GetParent();
  CSaView   * pView     = (CSaView*)pGraph->GetParent();
  CGraphWnd * pMelGraph = pView->GraphIDtoPtr(IDD_MELOGRAM);
  if (!pMelGraph) return 0;
  CPlotWnd* pMelPlot = pMelGraph->GetPlot();
  if (!pMelPlot) return 0;

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
BOOL CXScaleWnd::SetScale(int nMode, double dMinValue, double dMaxValue, TCHAR* pszDimension, int nGridDivisions, double d3dOffset)
{
  BOOL bRedraw = FALSE;
  if (m_nScaleMode != nMode)
  {
    m_nScaleMode = nMode; // set scale mode
    bRedraw = TRUE;
  }
  if (m_nGridDivisions != nGridDivisions)
  {
    m_nGridDivisions = nGridDivisions; // set divisions
    bRedraw = TRUE;
  }
  if (m_d3dOffset != d3dOffset)
  {
    m_d3dOffset = d3dOffset;
    bRedraw = TRUE;
  }
  if (m_fScaleMinValue != dMinValue)
  {
    m_fScaleMinValue = dMinValue; // set scale min value
    bRedraw = TRUE;
  }
  if (m_fScaleMaxValue != dMaxValue)
  {
    m_fScaleMaxValue = dMaxValue; // set scale max value
    bRedraw = TRUE;
  }
  if (pszDimension)
  {
    if (m_szScaleDimension != pszDimension)
    {
      m_szScaleDimension = pszDimension; // set scale dimension text
      bRedraw = TRUE;
    }
  }
  else
  {
    if (!m_szScaleDimension.IsEmpty())
    {
      m_szScaleDimension.Empty();
      bRedraw = TRUE;
    }
  }
  if (bRedraw)
    m_bRecalculate = TRUE;
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
void CXScaleWnd::CalculateScale(CDC* pDC, int nWidth)
{
  // get pointer to graph, view and document
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  CSaView* pView = (CSaView*)pGraph->GetParent();
  CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();
  // check if regular time scale expected
  if (m_nScaleMode & TIME_FROM_VIEW)
  {
    if (pGraph->IsAreaGraph())
    {
      // get scale range from area plot
      m_fScaleMinValue = pDoc->GetTimeFromBytes(pGraph->GetPlot()->GetAreaPosition());
      m_fScaleMaxValue = m_fScaleMinValue + pDoc->GetTimeFromBytes(pGraph->GetPlot()->GetAreaLength());
    }
    else
    {
      // get scale range from view
      // SDM 1.06.6U4 Get value of first pixel
      DWORD dwFrame = pView->AdjustDataFrame(nWidth);
      double fPos = pView->GetDataPosition(nWidth);
      m_fScaleMinValue = fPos*pDoc->GetTimeFromBytes(1);
      m_fScaleMaxValue = m_fScaleMinValue + pDoc->GetTimeFromBytes(dwFrame);
    }
    // set scale dimension
    m_szScaleDimension = "t(sec)";
  }
  if (!m_bRecalculate && (nWidth == m_nWidthUsed) && (m_nDivisionsUsed == m_nGridDivisions)
    && (m_fMinValueUsed == m_fScaleMinValue) && (m_fMaxValueUsed == m_fScaleMaxValue))
    return; // scale up to date
  if (nWidth <= 0)
    return; // no scale to draw
  // scale has changed, recalculate
  m_bRecalculate = FALSE;
  m_nWidthUsed = nWidth;
  m_nDivisionsUsed = m_nGridDivisions;
  m_fMinValueUsed = m_fScaleMinValue;
  m_fMaxValueUsed = m_fScaleMaxValue;
  m_fNumbPerPix = 1000 * (m_fScaleMaxValue - m_fScaleMinValue) / (double)nWidth;
  if(m_nGridDivisions == -1)
  {
    TEXTMETRIC tm;
    // check if pDC known
    if (pDC)
      pDC->GetTextMetrics(&tm); // get text metrics
    else
    {
      // get pDC to get textmetrics
      CDC* pDC = GetDC(); // get device context
      CFont* pOldFont = pDC->SelectObject(&m_font); // select legend font
      pDC->GetTextMetrics(&tm); // get text metrics
      pDC->SelectObject(pOldFont);  // set back old font
      ReleaseDC(pDC);
    }
    // find minimum scale distance in measures
    DWORD dwMinScale = (DWORD)(m_fNumbPerPix * (double)(tm.tmAveCharWidth * 10));
    if (!dwMinScale)
      dwMinScale = 1;
    // find the 10 based log of this distance
    DWORD dw10Base = (DWORD)log10((double)dwMinScale);
    // now find the next appropriate scale division
    DWORD dwBase = (DWORD)pow(10, (double)dw10Base);
    if ((dwBase * 2) < dwMinScale)
    {
      if ((dwBase * 5) % 2)
        dwBase *= 6;
      else
        dwBase *= 5;
    }
    else
      dwBase *= 2;
    if (dwBase < dwMinScale)
      dwBase = (DWORD)pow(10, (double)(++dw10Base));
    m_fNumbPerPix /= 1000;
    m_fBase = (double)dwBase / 1000;
    // calculate grid distance
    m_fGridDistance = m_fBase / m_fNumbPerPix; // gridline distance in pixels

    // calculate first (leftmost) gridline position in plot window client coordinates
    DWORD dwDivisions = (DWORD)(m_fScaleMinValue / m_fBase);
    m_fPosition = (double)dwDivisions * m_fBase;
    m_fFirstGridPos = ((m_fPosition - m_fScaleMinValue) / m_fNumbPerPix);
    
    m_fBase /= 2;
    dwDivisions = (DWORD)(m_fScaleMinValue / m_fBase);
    m_fPosition = (double)dwDivisions * m_fBase;
    m_bLargeLine = TRUE;
    if (dwDivisions % 2)
      m_bLargeLine = FALSE;
  }
  else
  {
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
int CXScaleWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CWnd::OnCreate(lpCreateStruct) == -1)
    return -1;
  // create legend font
  const TCHAR* pszName = _T("MS Sans Serif");
  m_font.CreateFont(15, 0, 0, 0, FW_NORMAL, 0, 0, 0,
    ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
    DEFAULT_QUALITY, (VARIABLE_PITCH | FF_ROMAN), pszName);
  return 0;
}




/***************************************************************************/
// CXScaleWnd::OnPaint
// Paints using OnDraw, which is shared with OnPrint
/***************************************************************************/
void CXScaleWnd::OnPaint()
{
  CRect dummyRect1(0,0,0,0);
  CRect dummyRect2(0,0,0,0);

  CPaintDC dc(this); // device context for painting

	OnDraw(&dc,dummyRect1,dummyRect2);
}


/***************************************************************************/
// CXScaleWnd::OnDraw Drawing
/***************************************************************************/
void CXScaleWnd::OnDraw(CDC * pDC,
                        const CRect & printRect,
                        const CRect & printPlotWnd)
{				
  // get pointer to graph and document
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();

  // x scale doesn't make sense for TWC graph.
  if (pGraph->IsPlotID(IDD_TWC)) return;
  CSaView* pView = (CSaView*)pGraph->GetParent();
  CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();

  // select x-scale font
  CFont* pOldFont = pDC->SelectObject(&m_font);
  // get text metrics
  TEXTMETRIC tm;
  pDC->GetTextMetrics(&tm);

  // get coordinates of plot window
  CRect rPlotWnd;

  if (pDC->IsPrinting())
  {
    rPlotWnd = printPlotWnd;
  }
  else
  {
    pGraph->GetPlot()->GetClientRect(rPlotWnd);
  }

  if ((rPlotWnd.Height() <= 0) || (pDoc->GetUnprocessedDataSize() == 0))
  {
    pDC->SelectObject(pOldFont); // set back old font
    return; // nothing to draw
  }
  CalculateScale(pDC, rPlotWnd.Width()); // calculate scale
  // get window coordinates
  CRect rWnd;

  if (pDC->IsPrinting())
  {
    rWnd = printRect;
  }
  else
  {
    GetClientRect(rWnd);
  }


  if (rWnd.Width() == 0)
  {
    pDC->SelectObject(pOldFont);  // set back old font
    return; // nothing to draw
  }
  // draw 3D window border
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd(); // get pointer to colors from main frame
  Colors* pColors = pMainWnd->GetColors();
  CPen penLtgray(PS_SOLID, 1, pColors->cSysBtnHilite);
  CPen* pOldPen = pDC->SelectObject(&penLtgray);
  pDC->MoveTo(rWnd.left, rWnd.top);
  pDC->LineTo(rWnd.right, rWnd.top);
  pDC->SelectObject(pOldPen);
  // set font colors
  pDC->SetTextColor(pColors->cScaleFont); // set font color
  pDC->SetBkMode(TRANSPARENT); // letters may overlap, so they must be transparent
  rWnd.top += 2; // don't draw over the border
  rWnd.bottom -= 2;
  if ((m_nScaleMode != NO_SCALE) && (pDoc->GetUnprocessedDataSize() > 0))
  {
    if (rWnd.Width() < (5 * tm.tmAveCharWidth))
    {
      pDC->SelectObject(pOldFont);  // set back old font
      return; // not enough space to draw scale
    }
    // draw the dimension
    int nDimLeftLimit = 0;
    int nDimRightLimit = 0;
    if (!m_szScaleDimension.IsEmpty() && !pGraph->IsPlotID(IDD_TWC) &&
      (!pGraph->HaveLegend() || (pGraph->IsAreaGraph() && !pGraph->IsPlotID(IDD_RECORDING))))
    {
      // create the bold font
      LOGFONT logFont;
      m_font.GetObject(sizeof(LOGFONT), (void*)&logFont); // fill up logFont
      logFont.lfWeight = FW_BOLD;
      CFont bFont;
      bFont.CreateFontIndirect(&logFont); // create the bold font
      pDC->SelectObject(&bFont); // select the bold font
      pDC->DrawText(m_szScaleDimension, m_szScaleDimension.GetLength(), rWnd, DT_SINGLELINE | DT_BOTTOM | DT_CENTER | DT_NOCLIP);
      nDimRightLimit = (m_szScaleDimension.GetLength() + 2 + 8) * tm.tmAveCharWidth;
      nDimLeftLimit = rWnd.Width() / 2 - nDimRightLimit / 2;
      nDimRightLimit += nDimLeftLimit;
      pDC->SelectObject(&m_font); // set back normal text font
    }
    // create line pen
    CPen pen(PS_SOLID, 1, pColors->cScaleLines);
    CPen* pOldPen = pDC->SelectObject(&pen);
    // draw the scale
    int nVertPos = rWnd.top; // horizontal line vertical position
    // draw the horizontal line
    pDC->MoveTo(rWnd.left, nVertPos);
    pDC->LineTo(rWnd.left + round(rWnd.Width()*(1 - m_d3dOffset)), nVertPos);
    if (m_nScaleMode & SCALE)
    {
      // draw the vertical lines and numbers
      int nTextWidth2 = tm.tmAveCharWidth * 4;
      int nTextLeftLimit = nTextWidth2; // left text position limit
      int nTextRightLimit = rWnd.right - nTextWidth2; // right text position limit
      double fPosition = m_fPosition;
      BOOL bLargeLine = m_bLargeLine;
      while (m_fBase > 0 ? fPosition <= m_fScaleMaxValue : fPosition >= m_fScaleMaxValue)
      {
        // calculate line position
        int nPos = (int)((1 - m_d3dOffset)*(fPosition - m_fScaleMinValue) / m_fNumbPerPix);
        // draw the line
        if (bLargeLine)
        {
          // draw a large line
          pDC->MoveTo(nPos, nVertPos);
          pDC->LineTo(nPos, nVertPos + 4);
          // draw the number
          if ((m_nScaleMode & NUMBERS) && (nPos > nTextLeftLimit) && (nPos < nTextRightLimit)
            && ((nPos < nDimLeftLimit) || (nPos >= nDimRightLimit)))
          {
            CRect rText(nPos - nTextWidth2, nVertPos + 3, nPos + nTextWidth2, rWnd.bottom);
            TCHAR szText[16];
            if (m_nScaleMode & TIME_FROM_VIEW)
              _stprintf(szText, _T("%8.3f"), fPosition);
            else
              _stprintf(szText, _T("%5.0f"), fPosition);
            pDC->DrawText(szText, -1, rText, DT_SINGLELINE | DT_TOP | DT_CENTER | DT_NOCLIP);
          }
        }
        else
        {
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
  pDC->SelectObject(pOldFont);  // set back old font
}



/***************************************************************************/
// CXScaleWnd::OnEraseBkgnd Erasing background
/***************************************************************************/
BOOL CXScaleWnd::OnEraseBkgnd(CDC* pDC)
{
  // get background color from main frame
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
  // create the background brush
  CBrush backBrush(pMainWnd->GetColors()->cScaleBkg);
  CBrush* pOldBrush = pDC->SelectObject(&backBrush);
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
void CXScaleWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
  // inform parent graph
  GetParent()->SendMessage(WM_RBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
  // handle the floating popup menu
  CMenu mPopup;
  if (mPopup.LoadMenu(((CMainFrame*)AfxGetMainWnd())->GetPopup()))//SDM 1.5Test8.5
  {
    CMenu& pFloatingPopup = *mPopup.GetSubMenu(0);
    ASSERT(pFloatingPopup.m_hMenu != NULL);
    // attach the layout menu
    CMenu* mLayout = new CLayoutMenu;
    TCHAR szString[256]; // don't change the string
    if (pFloatingPopup.GetMenuString(ID_GRAPHS_LAYOUT, szString, sizeof(szString)/sizeof(TCHAR), MF_BYCOMMAND))//SDM 1.5Test8.5
    {
      if (mLayout)
        VERIFY(pFloatingPopup.ModifyMenu(ID_GRAPHS_LAYOUT, MF_BYCOMMAND | MF_POPUP, (UINT)mLayout->m_hMenu, szString));
    }
    // pop the menu up
    ClientToScreen(&point);
    pFloatingPopup.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,  AfxGetMainWnd());
    if (mLayout)
      delete mLayout;
  }
  CWnd::OnRButtonDown(nFlags, point);
}

/***************************************************************************/
// CXScaleWnd::OnLButtonDown Mouse left button down
// This event should set the focus to the parent, so the window informs the
// parent graph to do this.
/***************************************************************************/
void CXScaleWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
  // inform parent graph
  CGraphWnd* pWnd = (CGraphWnd*)GetParent();
  pWnd->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y)); // send message to parent
  CWnd::OnLButtonDown(nFlags, point);
}

/***************************************************************************/
// CXScaleWnd::OnMouseMove Mouse move
// Detects mouse movement outside the plot area
/***************************************************************************/
void CXScaleWnd::OnMouseMove(UINT nFlags, CPoint point)
{
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  CPlotWnd* pPlot = pGraph->GetPlot();
  CSaView* pView = (CSaView*)pGraph->GetParent();
  pPlot->SetMousePointerPosition(CPoint(UNDEFINED_OFFSET, UNDEFINED_OFFSET)); // clear mouse position 
  pPlot->SetMouseButtonState(0);  // reset state
  const int bForceUpdate = TRUE;
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
    //{{AFX_MSG_MAP(CAnnotationWnd)
    ON_WM_ERASEBKGND()
    ON_WM_RBUTTONDOWN()
    ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
    ON_WM_PAINT()
    ON_WM_CREATE()
    ON_WM_LBUTTONDBLCLK()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnnotationWnd construction/destruction/creation

/***************************************************************************/
// CAnnotationWnd::CAnnotationWnd Constructor
/***************************************************************************/
CAnnotationWnd::CAnnotationWnd(int nIndex)
{
  m_nIndex = nIndex; // index of annotation window
  m_bHintUpdateBoundaries = FALSE;
}

/***************************************************************************/
// CAnnotationWnd::PreCreateWindow Creation
// Called from the framework before the creation of the window. Registers
// the new window class.
/***************************************************************************/
BOOL CAnnotationWnd::PreCreateWindow(CREATESTRUCT& cs)
{
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
int CAnnotationWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if(CWnd::OnCreate(lpCreateStruct) == -1)
    return -1;
  return 0;
}

/***************************************************************************/
// CAnnotationWnd::GetFont
/***************************************************************************/
CFont* CAnnotationWnd::GetFont()
{
  // get pointer to graph, view and document
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  CSaView* pView = (CSaView*)pGraph->GetParent();
  CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();

  return pDoc->GetFont(m_nIndex);
}


/***************************************************************************/
// CAnnotationWnd::GetWindowHeight
// Calculating the height of the current font of this window. If the window
// is the tone window, the font height does not include the descenders,
// because tone never has descenders.
/***************************************************************************/
int CAnnotationWnd::GetWindowHeight()
{
  // get height of the annotation window font
  TEXTMETRIC tm;
  CDC* pDC = GetDC(); // get device context
  // SDM 1.06.4
  CFont* pOldFont = pDC->SelectObject(GetFont()); // select annotation font
  pDC->GetTextMetrics(&tm); // get text metrics
  pDC->SelectObject(pOldFont);  // set back old font
  ReleaseDC(pDC);
  // SDM 1.06.6 Pitch Contours has descenders
  //    if (m_nIndex == TONE) tm.tmHeight = tm.tmAscent + 1; // exclude descender space for tone
  // SDM 1.06.6 Give necessary space to last window
  //    if (m_nIndex == (ANNOT_WND_NUMBER - 1)) return tm.tmHeight + 2; // last (lowest) window
  return tm.tmHeight + 3; // return font height + border space
}



/***************************************************************************/
// CAnnotationWnd::OnPaint
// Paints using OnDraw, which is shared with OnPrint
/***************************************************************************/
void CAnnotationWnd::OnPaint()
{
  CRect dummyRect(0,0,0,0); // needed for second OnDraw parameter
  // which is only really used for printing

  CPaintDC dc(this); // device context for painting

	OnDraw(&dc,dummyRect); // virtual
}


/***************************************************************************/
// CAnnotationWnd::AnnotationStandardPaint Standard painting of annotation
// This function may be used for classic non text annotation windows.
/***************************************************************************/
void CAnnotationWnd::OnDraw(CDC * pDC, const CRect & printRect)
{          
  // get window coordinates
  CRect rWnd;
  CRect rClip; // get invalid region

  if (pDC->IsPrinting())
  {
    rClip = printRect;
    rWnd  = printRect;
  }
  else
  {
    GetClientRect(rWnd);
    pDC->GetClipBox(&rClip);
    pDC->LPtoDP(&rClip);
  }

  if (rWnd.Width() == 0) return; // nothing to draw

  // get pointer to graph, view and document
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  CSaView* pView = (CSaView*)pGraph->GetParent();
  CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();

  // select annotation font
  //SDM 1.06.4
  CFont* pOldFont = pDC->SelectObject(GetFont());
  // get text metrics
  TEXTMETRIC tm;
  pDC->GetTextMetrics(&tm);

  // set font colors
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd(); // get pointer to colors from main frame
  Colors* pColors = pMainWnd->GetColors();

  pDC->SetTextColor(pColors->cAnnotationFont[m_nIndex]); // set font color
  pDC->SetBkMode(TRANSPARENT); // letters may overlap, so they must be transparent
  // draw 3D window border
  CPen penDkgray(PS_SOLID, 1, pColors->cSysBtnShadow);
  CPen penLtgray(PS_SOLID, 1, pColors->cSysBtnHilite);
  CPen* pOldPen = pDC->SelectObject(&penDkgray);
  // SDM 1.06.6 make annotation windows symettrical
  //    // check if this is the bottom window of annotation windows
  //    BOOL bBottom = TRUE;
  //    if (m_nIndex < (ANNOT_WND_NUMBER - 1))
  //    {  forint nLoop = m_nIndex + 1; nLoop < ANNOT_WND_NUMBER; nLoop++)
  //      if (pGraph->HaveAnnotation(nLoop)) bBottom = FALSE;
  //    }
  //    if (!bBottom)
  //    { pDC->MoveTo(rClip.left, rWnd.bottom - 2);
  //      pDC->LineTo(rClip.right, rWnd.bottom - 2);
  //      pDC->SelectObject(&penLtgray);
  //    }
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
  if (pGraph->IsPlotID(IDD_TWC))
  {
    pDC->SelectObject(pOldFont);  // set back old font
    return;
  }

  // SDM 1.06.6
  // Exclude Boundaries
  rWnd.SetRect(rWnd.left, rWnd.top + 1, rWnd.right, rWnd.bottom - 1);
  // check if area graph type
  double fDataStart;
  DWORD dwDataFrame;
  if (pGraph->IsAreaGraph())
  {
    // get necessary data from area plot
    fDataStart = pGraph->GetPlot()->GetAreaPosition();
    dwDataFrame = pGraph->GetPlot()->GetAreaLength();
  }
  else
  {
    // check if graph has private cursor
    if (pGraph->HavePrivateCursor())
    {
      // get necessary data from between public cursors
      FmtParm * pFmtParm = pDoc->GetFmtParm();
      WORD wSmpSize = WORD(pFmtParm->wBlockAlign / pFmtParm->wChannels);
      fDataStart = (double)pView->GetStartCursorPosition(); // data index of first sample to display
      dwDataFrame = pView->GetStopCursorPosition() - (DWORD)fDataStart + wSmpSize; // number of data points to display
    }
    else
    {
      // get necessary data from document and from view
      fDataStart = pView->GetDataPosition(rWnd.Width()); // data index of first sample to display
      dwDataFrame = pView->AdjustDataFrame(rWnd.Width()); // number of data points to display
    }
  }
  if (dwDataFrame == 0)
    return; // nothing to draw
  // calculate the number of data samples per pixel
  ASSERT(rWnd.Width());
  double fBytesPerPix = (double)dwDataFrame / (double)rWnd.Width();
  // get pointer to annotation string
  CString* pAnnot = pDoc->GetSegment(m_nIndex)->GetString();
  if (!pAnnot->IsEmpty()) // string is not empty
  {
    // get pointer to annotation offset and duration arrays
    CSegment* pSegment = pDoc->GetSegment(m_nIndex);
    // position prepare loop. Find first char to display in clipping rect
    double fStart = fDataStart + (double)(rClip.left - tm.tmAveCharWidth) * fBytesPerPix;
    int nLoop = 0;
    if ((fStart > 0) && (pAnnot->GetLength() > 1))
    {
      for (nLoop = 1; nLoop < pAnnot->GetLength(); nLoop++)
      {
        // SDM 1.06.6U2
        if ((double)(pSegment->GetStop(nLoop)) > fStart) // first char must be at lower position
        {
          nLoop--; // this is it
          break;
        }
      }
    }
    if (nLoop < pAnnot->GetLength()) // there is something to display
    {
      // display loop
      int nDisplayPos; // start position in pixels to display character(s)
      // Create Font For "*"
      CFont cFontAsterisk;
      cFontAsterisk.CreateFont( tm.tmHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE|DEFAULT_PITCH,_T("MS Sans Serif") );
      do
      {
        //SDM 1.06.6U4
        nDisplayPos = round((pSegment->GetOffset(nLoop) - fDataStart) / fBytesPerPix);
        // check if the character is selected
        BOOL bSelect = FALSE;
        if (pSegment->GetSelection() == nLoop)
          bSelect = TRUE;
        // put all characters width same offset in one string
        CString szAnnot = pAnnot->GetAt(nLoop);
        DWORD dwOffset = pSegment->GetOffset(nLoop++);
        while((nLoop < pAnnot->GetLength()) && (dwOffset == pSegment->GetOffset(nLoop)))
        {
          szAnnot += pAnnot->GetAt(nLoop++);
        }
        // calculate duration
        int nDisplayStop = round((pSegment->GetStop(nLoop - 1) - fDataStart)/ fBytesPerPix);

        // SDM 1.5Test8.1
        if (m_bHintUpdateBoundaries) // Show New Boundaries
        {
          if (bSelect)
          {
            nDisplayPos = round((m_dwHintStart - fDataStart)/ fBytesPerPix);
            nDisplayStop = round((m_dwHintStop - fDataStart)/ fBytesPerPix);
          }
          else if(((m_nIndex == PHONETIC) || (m_nIndex >= MUSIC_PL1 && m_nIndex <= MUSIC_PL4)) && !m_bOverlap)
          {
            if (pSegment->GetSelection() == (nLoop)) // Segment before selected segment
            {
              nDisplayStop = round((m_dwHintStart - fDataStart)/ fBytesPerPix);
            }
            else if ((pSegment->GetPrevious(nLoop)!=-1)
              &&(pSegment->GetSelection() == pSegment->GetPrevious(pSegment->GetPrevious(nLoop)))) // Segment after selected segment
            {
              nDisplayPos = round((m_dwHintStop - fDataStart)/ fBytesPerPix);
            }
          }
        }
        if ((nDisplayStop - nDisplayPos) < 2)
          nDisplayStop++; // must be at least 2 to display a point
        if ((nDisplayStop - nDisplayPos) < 2)
          nDisplayPos--; // must be at least 2 to display a point
        // set rectangle to display string centered within
        rWnd.SetRect(nDisplayPos, rWnd.top, nDisplayStop, rWnd.bottom);
        // highlight background if selected character
        COLORREF normalTextColor = pDC->GetTextColor();
        if (bSelect)
        {
          normalTextColor = pDC->SetTextColor(pColors->cSysColorHiText); // set highlighted text
          CBrush brushHigh(pColors->cSysColorHilite);
          CPen penHigh(PS_SOLID, 1, pColors->cSysColorHilite);
          CBrush* pOldBrush = (CBrush*)pDC->SelectObject(&brushHigh);
          CPen* pOldPen = pDC->SelectObject(&penHigh);
          pDC->Rectangle(rWnd.left, rWnd.top + 1, rWnd.right, rWnd.bottom - 1);
          pDC->SelectObject(pOldBrush);
          pDC->SelectObject(pOldPen);
        }
        // check, if there is enough space to display the character(s)

        BOOL bNotEnough = (nDisplayStop - nDisplayPos) <= tm.tmAveCharWidth;

        if(!bNotEnough)
        {
          bNotEnough = ((nDisplayStop - nDisplayPos) <= (szAnnot.GetLength() * tm.tmAveCharWidth));
        }

        if (bNotEnough)
        {
          // not enough space, draw dot or star with MS Sans Serif
          pDC->SelectObject(cFontAsterisk);
          if (m_nIndex == ORTHO)
            pDC->DrawText(_T("*"), 1, rWnd, DT_VCENTER | DT_SINGLELINE | DT_CENTER | DT_NOCLIP);
          else
            pDC->DrawText(_T("."), 1, rWnd, DT_VCENTER | DT_SINGLELINE | DT_CENTER | DT_NOCLIP);
          //SDM 1.06.4
          pDC->SelectObject(GetFont()); // reselect specific annotation font
        }
        else // enough space to display character(s), draw the string
          pDC->DrawText(szAnnot, szAnnot.GetLength(), rWnd, DT_VCENTER | DT_SINGLELINE | DT_CENTER | DT_NOCLIP);
        if (bSelect)
          pDC->SetTextColor(normalTextColor); // set back old text color
      } while ((nDisplayPos < rClip.right) && (nLoop < pAnnot->GetLength()));
    }
  }
  //SDM 1.06.5
  //keep up to date of changes force redraw of deselected virtual selections
  pView->ASelection().Update(pView); // get current selection information
  // Show virtual selection (No text just highlight)
  if (pView->ASelection().GetSelection().bVirtual && (pView->ASelection().GetSelection().nAnnotationIndex == m_nIndex))
  {
    int nStart = int(((double)pView->ASelection().GetSelection().dwStart - fDataStart)/ fBytesPerPix);
    int nStop = int(((double)pView->ASelection().GetSelection().dwStop - fDataStart)/ fBytesPerPix + 1);
    rWnd.SetRect(nStart, rWnd.top, nStop, rWnd.bottom);
    CBrush brushBk(pColors->cSysColorHilite);
    CPen penHigh(PS_INSIDEFRAME, 1, pColors->cSysColorHilite);
    CBrush* pOldBrush = (CBrush*)pDC->SelectObject(&brushBk);
    CPen* pOldPen = pDC->SelectObject(&penHigh);
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
void CAnnotationWnd::SetHintUpdateBoundaries(BOOL bHint, DWORD dwStart, DWORD dwStop, BOOL bOverlap)
{
  // get pointer to graph, view and document
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  CSaView* pView = (CSaView*)pGraph->GetParent();
  CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();
  CSegment* pSegment = pDoc->GetSegment(m_nIndex);

  if (bHint==TRUE)
  {
    pSegment->LimitPosition(pDoc,dwStart, dwStop, CSegment::LIMIT_MOVING_BOTH | (bOverlap ? 0 : CSegment::LIMIT_NO_OVERLAP));
    if (pSegment->CheckPosition(pDoc, dwStart, dwStop, CSegment::MODE_EDIT, bOverlap)==-1)
      bHint=FALSE;
  }

  if ((m_bHintUpdateBoundaries != bHint)||(m_dwHintStart!=dwStart)||(m_dwHintStop != dwStop)||(bOverlap != m_bOverlap)) // If change
  {
    if ((bHint!=FALSE)||(m_bHintUpdateBoundaries!=FALSE))
      InvalidateRect(NULL); // If hint drawn or will be drawn
    m_bHintUpdateBoundaries = bHint;  // request for draw hint of updated boundaries
    m_dwHintStart = dwStart;
    m_dwHintStop = dwStop;
    m_bOverlap = bOverlap;
  }
}

/////////////////////////////////////////////////////////////////////////////
// CAnnotationWnd message handlers



/***************************************************************************/
// CAnnotationWnd::OnEraseBkgnd Erasing background
/***************************************************************************/
BOOL CAnnotationWnd::OnEraseBkgnd(CDC* pDC)
{
  // get background color from main frame
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
  // create the background brush
  CBrush backBrush(pMainWnd->GetColors()->cAnnotationBkg[m_nIndex]);
  CBrush* pOldBrush = pDC->SelectObject(&backBrush);
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
void CAnnotationWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
  // inform parent graph
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  pGraph->SendMessage(WM_RBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
  if (pGraph->IsPlotID(IDD_TWC)) 
  {
    CWnd::OnRButtonDown(nFlags, point);
    return;
  }
  // get pointer to view
  CSaView* pView = (CSaView*)pGraph->GetParent();
  // get pointer to segment object
  CSegment* pSegment = pView->GetDocument()->GetSegment(m_nIndex);
  // if there is no selection in this window, deselect all the others too
  if (pSegment->GetSelection() == -1)
    pView->ChangeAnnotationSelection(pSegment, -1, 0, 0);
  // handle the floating popup menu
  CMenu mPopup;
  if (mPopup.LoadMenu(((CMainFrame*)AfxGetMainWnd())->GetPopup()))//SDM 1.5Test8.5
  {
    CMenu& pmFloatingPopup = *mPopup.GetSubMenu(1);
    ASSERT(&pmFloatingPopup != NULL && pmFloatingPopup.m_hMenu != NULL);
    if(&pmFloatingPopup != NULL && pmFloatingPopup.m_hMenu != NULL)
    {
      ClientToScreen(&point);
      pmFloatingPopup.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, AfxGetMainWnd());
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
void CAnnotationWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
  // inform parent plot
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  pGraph->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y)); // send message to parent
  if (pGraph->IsPlotID(IDD_TWC)) 
  {
    CWnd::OnLButtonDown(nFlags, point);
    return;
  }
  // get pointer to view and to document
  CSaView* pView = (CSaView*)pGraph->GetParent();
  CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();
  // find out, which character has been clicked
  CRect rWnd;
  GetClientRect(rWnd);
  // check if area graph type
  double fDataStart;
  DWORD dwDataFrame;
  if (pGraph->IsAreaGraph())
  {
    // get necessary data from area plot
    fDataStart = pGraph->GetPlot()->GetAreaPosition();
    dwDataFrame = pGraph->GetPlot()->GetAreaLength();
  }
  else
  {
    // check if graph has private cursor
    if (pGraph->HavePrivateCursor())
    {
      // get necessary data from between public cursors
      FmtParm * pFmtParm = pDoc->GetFmtParm();
      WORD wSmpSize = WORD(pFmtParm->wBlockAlign / pFmtParm->wChannels);
      fDataStart = (LONG)pView->GetStartCursorPosition(); // data index of first sample to display
      dwDataFrame = pView->GetStopCursorPosition() - (DWORD)fDataStart + wSmpSize; // number of data points to display
    }
    else
    {
      // get necessary data from document and from view
      fDataStart = pView->GetDataPosition(rWnd.Width()); // data index of first sample to display
      dwDataFrame = pView->AdjustDataFrame(rWnd.Width()); // number of data points to display
    }
  }
  if (dwDataFrame != 0) // data is available
  {
    ASSERT(rWnd.Width());
    double fBytesPerPix = (double)dwDataFrame / (double)rWnd.Width();
    // calculate clicked data position
    LONG dwPosition = (LONG)((double)point.x * fBytesPerPix + (double)fDataStart);

    //SDM 1.06.5
    //SDM 1.06.6
    //SDM 1.5Test8.5
    if (((CMainFrame*)AfxGetMainWnd())->IsEditAllowed()&&(!pView->ASelection().SelectFromPosition(pView, m_nIndex, dwPosition))&&(pView->ASelection().GetSelection().nAnnotationIndex!=-1))//SDM 1.5Test8.5
    {
      // Selection not changed
      if (GetTickCount() < (m_nSelectTickCount + SLOW_CLICK_TIME_LIMIT * 1000))
      {
        OnCreateEdit();
      }
    };
    // SDM 1.06.6U2
    if (pView->ASelection().GetSelection().nAnnotationIndex != -1)
      m_nSelectTickCount = GetTickCount();
    else
      // disable slow click (timed out)
      m_nSelectTickCount = GetTickCount() - DWORD(SLOW_CLICK_TIME_LIMIT * 1000);
  }
  CWnd::OnLButtonDown(nFlags, point);
}

/***************************************************************************/
// CAnnotationWnd::OnMouseMove Mouse move
// Detects mouse movement outside the plot area
/***************************************************************************/
void CAnnotationWnd::OnMouseMove(UINT nFlags, CPoint point)
{
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  CPlotWnd* pPlot = pGraph->GetPlot();
  CSaView* pView = (CSaView*)pGraph->GetParent();
  pPlot->SetMousePointerPosition(CPoint(UNDEFINED_OFFSET, UNDEFINED_OFFSET)); // clear mouse position 
  pPlot->SetMouseButtonState(0);  // reset state
  const int bForceUpdate = TRUE;
  pGraph->UpdateStatusBar(pView->GetStartCursorPosition(), pView->GetStopCursorPosition(), bForceUpdate);
  CWnd::OnMouseMove(nFlags, point);
} 

//###########################################################################
// CAnnotationWnd::OnLButtonDblClk
// disable slow click after dblClk
//###########################################################################
void CAnnotationWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  if (pGraph->IsPlotID(IDD_TWC)) 
  {
    CWnd::OnLButtonDblClk(nFlags, point);
    return;
  }
  CSaView* pView = (CSaView*)pGraph->GetParent();
  // if nothing selected select it
  if(pView->GetDocument()->GetSegment(m_nIndex)->GetSelection() == -1)
    SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y)); // send message to parent
  AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_EDIT_EDITOR, 0L);

  // disable slow click (timed out)
  m_nSelectTickCount = GetTickCount() - DWORD(SLOW_CLICK_TIME_LIMIT * 1000);

  CWnd::OnLButtonDblClk(nFlags, point);
}

// SDM 1.5Test10.7
//###########################################################################
// CAnnotationWnd::CreateEdit
//###########################################################################
void CAnnotationWnd::OnCreateEdit(const CString *szInitialString)
{
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  if (pGraph->IsPlotID(IDD_TWC)) 
  {
    return;
  }
  CSaView* pView = (CSaView*)pGraph->GetParent();
  // Slow click show annotation edit dialog over selection
  CRect rWnd;
  GetClientRect(rWnd);
  // check if area graph type
  double fDataStart = 0;
  DWORD dwDataFrame = 0;
  if (pGraph->IsAreaGraph())
  {
    // get necessary data from area plot
    fDataStart = pGraph->GetPlot()->GetAreaPosition();
    dwDataFrame = pGraph->GetPlot()->GetAreaLength();
  }
  else
  {
    // check if graph has private cursor
    if (pGraph->HavePrivateCursor())
    {
      // get necessary data from between public cursors
      CSaDoc *pDoc = pView->GetDocument();
      FmtParm * pFmtParm = pDoc->GetFmtParm();
      WORD wSmpSize = WORD(pFmtParm->wBlockAlign / pFmtParm->wChannels);
      fDataStart = (LONG)pView->GetStartCursorPosition(); // data index of first sample to display
      dwDataFrame = pView->GetStopCursorPosition() - (DWORD)fDataStart + wSmpSize; // number of data points to display
    }
    else
    {
      // get necessary data from document and from view
      fDataStart = pView->GetDataPosition(rWnd.Width()); // data index of first sample to display
      dwDataFrame = pView->AdjustDataFrame(rWnd.Width()); // number of data points to display
    }
  }
  if (dwDataFrame != 0) // data is available
  {
    ASSERT(rWnd.Width());
    double fBytesPerPix = (double)dwDataFrame / (double)rWnd.Width();
    // Calculate Window Position
    GetClientRect(rWnd);
    CRect rEdit(rWnd);
    rEdit.left = int((pView->ASelection().GetSelection().dwStart - fDataStart)/fBytesPerPix + 0.5);
    rEdit.right = int(((double)pView->ASelection().GetSelection().dwStop - (double)fDataStart)/fBytesPerPix + 1.5);
    rEdit.IntersectRect(rWnd,rEdit);
    if (rEdit.Width() && dwDataFrame)
    {
      if (rEdit.Width() < 2*rEdit.Height())
      {
        // Edit box too small
        int nMore = 2*rEdit.Height()  - rEdit.Width();
        rEdit.left -= nMore/2;
        rEdit.right += nMore/2;
        rEdit.IntersectRect(rWnd,rEdit);
      }
      ClientToScreen(rEdit);
      // ShowWindow
      CAnnotationEdit* pAnnotationEdit = new CAnnotationEdit();
      pAnnotationEdit->Create(CAnnotationEdit::IDD);
      pAnnotationEdit->SetWindowPos(&wndTop,rEdit.left,rEdit.top + 1,rEdit.Width(),rEdit.Height() - 2,SWP_SHOWWINDOW);
      if(szInitialString)
        pAnnotationEdit->SetText(*szInitialString);
      pAnnotationEdit = NULL; // window needs to destroy itself
    }
  }
}

int CALLBACK EnumFontFamExProc(
  const ENUMLOGFONTEX *lpelfe,    // logical-font data
  const NEWTEXTMETRICEX *lpntme,  // physical-font data
  DWORD FontType,           // type of font
  LPARAM lParam             // application-defined data
)
{
  UNUSED_ALWAYS(FontType);
  UNUSED_ALWAYS(lpntme);
  long &nCharset = *(long*) lParam;

  if(nCharset == DEFAULT_CHARSET) nCharset = lpelfe->elfLogFont.lfCharSet;
  else
  {
    // If ANSI is one of the many chose it.
    if(lpelfe->elfLogFont.lfCharSet == ANSI_CHARSET)
      nCharset = lpelfe->elfLogFont.lfCharSet;

    // Otherwise we just keep the first enumerated charset (presumably the default)
  }

  return 0;
}

void CAnnotationWnd::CreateAnnotationFont(CFont *pFont, int nPoints, LPCTSTR szFace)
{
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
void CGlossWnd::OnDraw(CDC * pDC, const CRect & printRect)
{
  CRect rWnd;
  CRect rClip; // get invalid region

  if (pDC->IsPrinting())
  {
    rWnd  = printRect;
    rClip = printRect;
  }
  else
  {
    GetClientRect(rWnd);
    pDC->GetClipBox(&rClip);
    pDC->LPtoDP(&rClip);
  }

  // select gloss font
  CFont* pOldFont = pDC->SelectObject(GetFont());
  // get text metrics
  TEXTMETRIC tm;
  pDC->GetTextMetrics(&tm);
  // get window coordinates
  if (rWnd.Width() == 0)
    return; // nothing to draw
  // set font colors
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd(); // get pointer to colors from main frame
  Colors* pColors = pMainWnd->GetColors();

  pDC->SetTextColor(pColors->cAnnotationFont[m_nIndex]); // set font color
  pDC->SetBkMode(TRANSPARENT); // letters may overlap, so they must be transparent
  // draw 3D window border
  CPen penDkgray(PS_SOLID, 1, pColors->cSysBtnShadow);
  CPen penLtgray(PS_SOLID, 1, pColors->cSysBtnHilite);
  CPen* pOldPen = pDC->SelectObject(&penDkgray);
  // SDM 1.06.6 make annotation windows symettrical
  pDC->MoveTo(rClip.left, rWnd.bottom - 1);
  pDC->LineTo(rClip.right, rWnd.bottom - 1);
  pDC->SelectObject(&penLtgray);
  pDC->MoveTo(rClip.left, rWnd.top);
  pDC->LineTo(rClip.right, rWnd.top);
  pDC->SelectObject(pOldPen);

  // get pointer to graph, view and document
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  CSaView* pView = (CSaView*)pGraph->GetParent();
  CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();

  //*******************************************************
  // 09/27/2000 - DDO If the graph is the TWC graph
  // then we don't want to draw the annotation text.
  //*******************************************************
  if (pGraph->IsPlotID(IDD_TWC))
  {
    pDC->SelectObject(pOldFont);  // set back old font
    return;
  }

  // SDM 1.06.6
  // Exclude Boundaries
  rWnd.SetRect(rWnd.left, rWnd.top + 1, rWnd.right, rWnd.bottom - 1);

  // check if area graph type
  double fDataStart;
  DWORD dwDataFrame;
  if (pGraph->IsAreaGraph())
  {
    // get necessary data from area plot
    fDataStart = pGraph->GetPlot()->GetAreaPosition();
    dwDataFrame = pGraph->GetPlot()->GetAreaLength();
  }
  else
  {
    // check if graph has private cursor
    if (pGraph->HavePrivateCursor())
    {
      // get necessary data from between public cursors
      FmtParm * pFmtParm = pDoc->GetFmtParm();
      WORD wSmpSize = WORD(pFmtParm->wBlockAlign / pFmtParm->wChannels);
      fDataStart = pView->GetStartCursorPosition(); // data index of first sample to display
      dwDataFrame = pView->GetStopCursorPosition() - (DWORD) fDataStart + wSmpSize; // number of data points to display
    }
    else
    {
      // get necessary data from document and from view
      fDataStart = pView->GetDataPosition(rWnd.Width()); // data index of first sample to display
      dwDataFrame = pView->AdjustDataFrame(rWnd.Width()); // number of data points to display
    }
  }
  if (dwDataFrame == 0)
    return; // nothing to draw
  // calculate the number of data samples per pixel
  double fBytesPerPix = (double)dwDataFrame / (double)rWnd.Width();
  // get pointer to gloss strings
  const CStringArray* pGloss = pDoc->GetSegment(m_nIndex)->GetTexts(); //SDM 1.5Test8.1
  if (pGloss->GetUpperBound() != -1) // array is not empty
  {
    // get pointer to gloss offset and duration arrays
    CSegment* pSegment = pDoc->GetSegment(m_nIndex);
    CSegment* pPhonetic = pDoc->GetSegment(PHONETIC);
    // position prepare loop. Find first string to display in clipping rect
    int nLoop = 0;
    if ((fDataStart > 0) && (pGloss->GetSize() > 1))
    {
      double fStart = fDataStart + (double)(rClip.left - tm.tmAveCharWidth) * fBytesPerPix;
      for (nLoop = 1; nLoop < pGloss->GetSize(); nLoop++)
      {
        if ((double)(pSegment->GetStop(nLoop)) > fStart) // first string must be at lower position
        {
          nLoop--; // this is it
          break;
        }
      }
    }
    if (nLoop < pGloss->GetSize()) // there is something to display
    {
      // display loop
      int nDisplayPos, nDisplayStop;
      CString string;
      do
      {
        string.Empty();
        // get the string to display
        string = pGloss->GetAt(nLoop);
        // insert a space after the delimiter
        if (string.GetLength() > 1)
        {
          CString szSpace = " ";
          string = string.GetAt(0) + szSpace + string.Right(string.GetLength() - 1);
        }
        nDisplayPos = round((pSegment->GetOffset(nLoop) - fDataStart) / fBytesPerPix);
        // check if the character is selected
        BOOL bSelect = FALSE;
        if (pSegment->GetSelection() == nLoop)
          bSelect = TRUE;
        // calculate duration
        nDisplayStop = round((pSegment->GetStop(nLoop)- fDataStart)/ fBytesPerPix);
        //SDM 1.06.2
        if (m_bHintUpdateBoundaries) // Show New Boundaries
        {
          if (bSelect)
          {
            nDisplayPos = round((m_dwHintStart - fDataStart)/ fBytesPerPix);
            nDisplayStop = round((m_dwHintStop - fDataStart)/ fBytesPerPix);
          }
          else if (pSegment->GetSelection() == (nLoop+1)) // Segment prior to selected segment
          {
            // SDM 1.5Test11.1
            int nIndex = pPhonetic->GetPrevious(pPhonetic->FindOffset(m_dwHintStart));
            DWORD dwStop = pPhonetic->GetStop(nIndex);
            nDisplayStop = round((dwStop - fDataStart)/ fBytesPerPix);
          }
          else if (pSegment->GetSelection() == (nLoop-1)) // Segment after selected segment
          {
            // SDM 1.5Test11.1
            int nIndex = pPhonetic->GetNext(pPhonetic->FindStop(m_dwHintStop));
            DWORD dwStart = pPhonetic->GetOffset(nIndex);
            nDisplayPos = round((dwStart - fDataStart)/ fBytesPerPix);
          }
        }
        if ((nDisplayStop - nDisplayPos) < 2)
          nDisplayStop++; // must be at least 2 to display a point
        if ((nDisplayStop - nDisplayPos) < 2)
          nDisplayPos--; // must be at least 2 to display a point
        // set rectangle to display string centered within
        rWnd.SetRect(nDisplayPos, rWnd.top, nDisplayStop, rWnd.bottom);
        // highlight background if selected character
        COLORREF normalTextColor = pDC->GetTextColor();
        if (bSelect)
        {
          normalTextColor = pDC->SetTextColor(pColors->cSysColorHiText); // set highlighted text
          CBrush brushHigh(pColors->cSysColorHilite);
          CPen penHigh(PS_SOLID, 1, pColors->cSysColorHilite);
          CBrush* pOldBrush = (CBrush*)pDC->SelectObject(&brushHigh);
          CPen* pOldPen = pDC->SelectObject(&penHigh);
          pDC->Rectangle(rWnd.left, rWnd.top + 1, rWnd.right, rWnd.bottom - 1);
          pDC->SelectObject(pOldBrush);
          pDC->SelectObject(pOldPen);
        }
        if ((nDisplayStop-nDisplayPos) <= (string.GetLength() * tm.tmAveCharWidth)) // not enough space
        {
          if ((nDisplayStop-nDisplayPos) <= 4 * tm.tmAveCharWidth) // even not enough space for at least two characters with dots
          {
            // draw only first character
            TCHAR c = string.GetAt(0);
            pDC->DrawText(&c, 1, rWnd, DT_VCENTER | DT_SINGLELINE | DT_LEFT | DT_NOCLIP); // print first character
          }
          else
          {
            // draw as many characters as possible and 3 dots
            string = string.Left((nDisplayStop-nDisplayPos) / tm.tmAveCharWidth - 2) + "...";
            pDC->DrawText(string, string.GetLength(), rWnd, DT_VCENTER | DT_SINGLELINE | DT_LEFT | DT_NOCLIP);
          }
        }
        else // enough space to display string
        {
          pDC->DrawText(string, 1, rWnd, DT_VCENTER | DT_SINGLELINE | DT_LEFT | DT_NOCLIP);
          pDC->DrawText(LPCTSTR(string) + 1, string.GetLength() - 1, rWnd, DT_VCENTER | DT_SINGLELINE | DT_CENTER | DT_NOCLIP);
        }
        if (bSelect)
          pDC->SetTextColor(normalTextColor); // set back old text color
      } while ((nDisplayPos < rClip.right) && (++nLoop < pGloss->GetSize()));
    }
  }
  pDC->SelectObject(pOldFont);  // set back old font
  
  //SDM 1.06.5
  //keep up to date of changes force redraw of deselected virtual selections
  pView->ASelection().Update(pView); // get current selection information
}

//SDM 1.5Test8.1
//###########################################################################
// CReferenceWnd
// Reference window contained in the graphs. It displays the Reference information of
// the actually opened documents wave data. To do this it needs information
// from the view and the document to keep track with zooming and scrolling.

/***************************************************************************/
// CReferenceWnd::OnDraw Drawing
/***************************************************************************/
void CReferenceWnd::OnDraw(CDC * pDC, const CRect & printRect)
{
  CRect rWnd;
  CRect rClip; // get invalid region

  if (pDC->IsPrinting())
  {
    rWnd  = printRect;
    rClip = printRect;
  }
  else
  {
    GetClientRect(rWnd);
    pDC->GetClipBox(&rClip);
    pDC->LPtoDP(&rClip);
  }

  // select gloss font
  CFont* pOldFont = pDC->SelectObject(GetFont());
  // get text metrics
  TEXTMETRIC tm;
  pDC->GetTextMetrics(&tm);
  // get window coordinates
  if (rWnd.Width() == 0)
    return; // nothing to draw
  // set font colors
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd(); // get pointer to colors from main frame
  Colors* pColors = pMainWnd->GetColors();

  pDC->SetTextColor(pColors->cAnnotationFont[m_nIndex]); // set font color
  pDC->SetBkMode(TRANSPARENT); // letters may overlap, so they must be transparent
  // draw 3D window border
  CPen penDkgray(PS_SOLID, 1, pColors->cSysBtnShadow);
  CPen penLtgray(PS_SOLID, 1, pColors->cSysBtnHilite);
  CPen* pOldPen = pDC->SelectObject(&penDkgray);
  // SDM 1.06.6 make annotation windows symettrical
  pDC->MoveTo(rClip.left, rWnd.bottom - 1);
  pDC->LineTo(rClip.right, rWnd.bottom - 1);
  pDC->SelectObject(&penLtgray);
  pDC->MoveTo(rClip.left, rWnd.top);
  pDC->LineTo(rClip.right, rWnd.top);
  pDC->SelectObject(pOldPen);

  // get pointer to graph, view and document
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  CSaView* pView = (CSaView*)pGraph->GetParent();
  CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();

  //*******************************************************
  // 09/27/2000 - DDO If the graph is the TWC graph
  // then we don't want to draw the annotation text.
  //*******************************************************
  if (pGraph->IsPlotID(IDD_TWC))
  {
    pDC->SelectObject(pOldFont);  // set back old font
    return;
  }

  // SDM 1.06.6
  // Exclude Boundaries
  rWnd.SetRect(rWnd.left, rWnd.top + 1, rWnd.right, rWnd.bottom - 1);

  // check if area graph type
  double fDataStart;
  DWORD dwDataFrame;
  if (pGraph->IsAreaGraph())
  {
    // get necessary data from area plot
    fDataStart = pGraph->GetPlot()->GetAreaPosition();
    dwDataFrame = pGraph->GetPlot()->GetAreaLength();
  }
  else
  {
    // check if graph has private cursor
    if (pGraph->HavePrivateCursor())
    {
      // get necessary data from between public cursors
      FmtParm * pFmtParm = pDoc->GetFmtParm();
      WORD wSmpSize = WORD(pFmtParm->wBlockAlign / pFmtParm->wChannels);
      fDataStart = pView->GetStartCursorPosition(); // data index of first sample to display
      dwDataFrame = pView->GetStopCursorPosition() - (DWORD) fDataStart + wSmpSize; // number of data points to display
    }
    else
    {
      // get necessary data from document and from view
      fDataStart = pView->GetDataPosition(rWnd.Width()); // data index of first sample to display
      dwDataFrame = pView->AdjustDataFrame(rWnd.Width()); // number of data points to display
    }
  }
  if (dwDataFrame == 0)
    return; // nothing to draw
  // calculate the number of data samples per pixel
  double fBytesPerPix = (double)dwDataFrame / (double)rWnd.Width();
  // get pointer to gloss strings
  const CStringArray* pGloss = pDoc->GetSegment(m_nIndex)->GetTexts();
  if (pGloss->GetUpperBound() != -1) // array is not empty
  {
    // get pointer to gloss offset and duration arrays
    CSegment* pSegment = pDoc->GetSegment(m_nIndex);
    // position prepare loop. Find first string to display in clipping rect
    int nLoop = 0;
    if ((fDataStart > 0) && (pGloss->GetSize() > 1))
    {
      double fStart = fDataStart + (double)(rClip.left - tm.tmAveCharWidth) * fBytesPerPix;
      for (nLoop = 1; nLoop < pGloss->GetSize(); nLoop++)
      {
        if ((double)(pSegment->GetStop(nLoop)) > fStart) // first string must be at lower position
        {
          nLoop--; // this is it
          break;
        }
      }
    }
    if (nLoop < pGloss->GetSize()) // there is something to display
    {
      // display loop
      int nDisplayPos, nDisplayStop;
      CString string;
      do
      {
        string.Empty();
        // get the string to display
        string = pGloss->GetAt(nLoop);
        nDisplayPos = round((pSegment->GetOffset(nLoop) - fDataStart) / fBytesPerPix);
        // check if the character is selected
        BOOL bSelect = FALSE;
        if (pSegment->GetSelection() == nLoop) bSelect = TRUE;
        // calculate duration
        nDisplayStop = round((pSegment->GetStop(nLoop) - fDataStart)/ fBytesPerPix);
        //SDM 1.06.2
        if (m_bHintUpdateBoundaries) // Show New Boundaries
        {
          if (bSelect)
          {
            nDisplayPos = round((m_dwHintStart - fDataStart)/ fBytesPerPix);
            nDisplayStop = round((m_dwHintStop - fDataStart)/ fBytesPerPix);
          }
          else if (pSegment->GetSelection() == (nLoop+1)) // Segment prior to selected segment
          {
            nDisplayStop = round((m_dwHintStart - fDataStart)/ fBytesPerPix);
          }
          else if (pSegment->GetSelection() == (nLoop-1)) // Segment after selected segment
          {
            nDisplayPos = round((m_dwHintStop - fDataStart)/ fBytesPerPix);
          }
        }
        if ((nDisplayStop - nDisplayPos) < 2)
          nDisplayStop++; // must be at least 2 to display a point
        if ((nDisplayStop - nDisplayPos) < 2)
          nDisplayPos--; // must be at least 2 to display a point
        // set rectangle to display string centered within
        rWnd.SetRect(nDisplayPos, rWnd.top, nDisplayStop, rWnd.bottom);
        // highlight background if selected character
        COLORREF normalTextColor = pDC->GetTextColor();
        if (bSelect)
        {
          normalTextColor = pDC->SetTextColor(pColors->cSysColorHiText); // set highlighted text
          CBrush brushHigh(pColors->cSysColorHilite);
          CPen penHigh(PS_SOLID, 1, pColors->cSysColorHilite);
          CBrush* pOldBrush = (CBrush*)pDC->SelectObject(&brushHigh);
          CPen* pOldPen = pDC->SelectObject(&penHigh);
          pDC->Rectangle(rWnd.left, rWnd.top + 1, rWnd.right, rWnd.bottom - 1);
          pDC->SelectObject(pOldBrush);
          pDC->SelectObject(pOldPen);
        }

        BOOL bNotEnough = (nDisplayStop - nDisplayPos) <= tm.tmAveCharWidth;
        if(!bNotEnough)
        {
          bNotEnough = ((nDisplayStop - nDisplayPos) <= (string.GetLength() * tm.tmAveCharWidth));
        }

        if (bNotEnough)
        {
          if ((nDisplayStop-nDisplayPos) <= 4 * tm.tmAveCharWidth) // even not enough space for at least two characters with dots
          {
            pDC->DrawText(_T("*"), 1, rWnd, DT_VCENTER | DT_SINGLELINE | DT_LEFT | DT_NOCLIP); // print first character
          }
          else
          {
            // draw as many characters as possible and 3 dots
            string = string.Left((nDisplayStop-nDisplayPos) / tm.tmAveCharWidth - 2) + "...";
            pDC->DrawText((LPCTSTR)string, string.GetLength(), rWnd, DT_VCENTER | DT_SINGLELINE | DT_LEFT | DT_NOCLIP);
          }
        }
        else // enough space to display string
        {
          pDC->DrawText((LPCTSTR)string, string.GetLength(), rWnd, DT_VCENTER | DT_SINGLELINE | DT_CENTER | DT_NOCLIP);
        }
        if (bSelect) pDC->SetTextColor(normalTextColor); // set back old text color
      } while ((nDisplayPos < rClip.right) && (++nLoop < pGloss->GetSize()));
    }
  }
  pDC->SelectObject(pOldFont);  // set back old font

  //SDM 1.06.5
  //keep up to date of changes force redraw of deselected virtual selections
  pView->ASelection().Update(pView); // get current selection information
  // Show virtual selection (No text just highlight)
  if (pView->ASelection().GetSelection().bVirtual && (pView->ASelection().GetSelection().nAnnotationIndex == m_nIndex))
  {
    int nStart = int(((double)pView->ASelection().GetSelection().dwStart - fDataStart)/ fBytesPerPix);
    int nStop = int(((double)pView->ASelection().GetSelection().dwStop - fDataStart)/ fBytesPerPix + 1);
    rWnd.SetRect(nStart, rWnd.top, nStop, rWnd.bottom);
    CBrush brushBk(pColors->cSysColorHilite);
    CPen penHigh(PS_INSIDEFRAME, 1, pColors->cSysColorHilite);
    CBrush* pOldBrush = (CBrush*)pDC->SelectObject(&brushBk);
    CPen* pOldPen = pDC->SelectObject(&penHigh);
    pDC->Rectangle(rWnd.left, rWnd.top + 1, rWnd.right, rWnd.bottom - 1);
    pDC->SelectObject(pOldBrush);
    pDC->SelectObject(pOldPen);
  }
  pDC->SelectObject(pOldFont);  // set back old font
}


