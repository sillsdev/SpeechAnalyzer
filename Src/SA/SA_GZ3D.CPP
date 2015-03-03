/////////////////////////////////////////////////////////////////////////////
// sa_gZ3D.cpp:
// Implementation of the CPlot3D
//             CPlotF1F2
//             CPlotF2F1
//             CPlotF2F1F1 classes.
//             CPlotInvSDP classes.
// Author: Urs Ruchti
//         Modified by A. Epting
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "sa_plot.h"
#include "sa_gZ3D.h"
#include "Process\Process.h"
#include "Process\sa_p_spu.h"
#include "Process\sa_p_fmt.h"
#include "Process\sa_p_fra.h"
#include "Process\sa_p_gra.h"
#include "sa_minic.h"
#include "sa_graph.h"
#include "DlgVowelFormants.h"

#include <malloc.h>
#include "sa_doc.h"
#include "sa.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "Process\FormantTracker.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// FormantChart
// Plot window for displaying formants data. This class only handles
// the special drawing of the data. All other work is done in the base class.
// For the drawing, this class holds three buffers (each 1kByte) for the 3D
// data to deliver to the Z-graph functions.

IMPLEMENT_DYNCREATE(CFormantChart, CPlotWnd)

/////////////////////////////////////////////////////////////////////////////
// FormantChart message map

BEGIN_MESSAGE_MAP(CFormantChart, CPlotWnd)
    ON_WM_CREATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// FormantChart construction/destruction/creation

/***************************************************************************/
// CFormantChart::FormantChart Constructor
/***************************************************************************/
CFormantChart::CFormantChart() {
    m_bAnimationPlot = TRUE;
    m_bVoiced = TRUE;
    m_bOutOfRange = FALSE;
}

/***************************************************************************/
// CFormantChart::~FormantChart Destructor
/***************************************************************************/
CFormantChart::~CFormantChart() {
}

/***************************************************************************/
// CFormantChart::SetStartCursor Position the start cursor
/***************************************************************************/
void CFormantChart::SetStartCursor(CSaView * pView) {
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
    CProcessFormants * pFormants = pDoc->GetFormants();
    pFormants->ResetTracking();  // start over
    CFormantParm * pFormantSource = pFormants->GetFormantParms();
    CPlotWnd::SetStartCursor(pView);

    // Finish pitch processing if necessary.
    CProcessGrappl * pAutoPitch = (CProcessGrappl *)pDoc->GetGrappl();
    short int nResult = LOWORD(pAutoPitch->Process(this, pDoc)); // process data
    if (nResult == PROCESS_ERROR || nResult == PROCESS_NO_DATA || nResult == PROCESS_CANCELED) {
        if (nResult == PROCESS_CANCELED) {
            pFormants->CancelProcess();
        }
        return;
    }
    DWORD dwWaveStart = (pView->GetStartCursorPosition() / Grappl_calc_intvl) * Grappl_calc_intvl;
    DWORD dwWaveStop = dwWaveStart;
    if (pFormantSource->bFromCepstralSpectrum) {
        dwWaveStop = (pView->GetStopCursorPosition() / Grappl_calc_intvl) * Grappl_calc_intvl;
    }
    m_bVoiced = TRUE;
    for (DWORD dwWave = dwWaveStart; dwWave <= dwWaveStop; dwWave += Grappl_calc_intvl)
        if (!pAutoPitch->IsVoiced(pDoc, dwWave)) {
            m_bVoiced = FALSE;
            break;
        }
}

/***************************************************************************/
// CFormantChart::SetStopCursor Position the stop cursor
/***************************************************************************/
void CFormantChart::SetStopCursor(CSaView * pView) {
    CPlotWnd::SetStopCursor(pView);
    // Finish pitch processing if necessary.
    CSaDoc * pDoc   = pView->GetDocument();
    CProcessFormants * pFormants = (CProcessFormants *)pDoc->GetFormants(); // get pointer to formants object
    CFormantParm * pFormantSource = pFormants->GetFormantParms();
    CProcessGrappl * pAutoPitch = (CProcessGrappl *)pDoc->GetGrappl();
    short int nResult = LOWORD(pAutoPitch->Process(this, pDoc)); // process data
    if (nResult == PROCESS_ERROR || nResult == PROCESS_NO_DATA || nResult == PROCESS_CANCELED) {
        if (nResult == PROCESS_CANCELED) {
            pFormants->CancelProcess();
        }
    }
    DWORD dwWaveStart = (pView->GetStartCursorPosition() / Grappl_calc_intvl) * Grappl_calc_intvl;
    DWORD dwWaveStop = dwWaveStart;
    if (pFormantSource->bFromCepstralSpectrum) {
        dwWaveStop = (pView->GetStopCursorPosition() / Grappl_calc_intvl) * Grappl_calc_intvl;
    }
    m_bVoiced = TRUE;
    for (DWORD dwWave = dwWaveStart; dwWave <= dwWaveStop; dwWave += Grappl_calc_intvl)
        if (!pAutoPitch->IsVoiced(pDoc, dwWave)) {
            m_bVoiced = FALSE;
            break;
        }
}


/***************************************************************************/
// CFormantChart::OnGenderInfoChanged  Gender info has changed
/***************************************************************************/
void CFormantChart::GenderInfoChanged(int /* nGender */) {
    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}


/***************************************************************************/
// CFormantChart::ProcessFTFormants Process FormantTracker Formants
/***************************************************************************/
short int CFormantChart::ProcessFTFormants() {
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaView * pView = (CSaView *)pGraph->GetParent();
    CSaDoc * pDoc   = pView->GetDocument();
    // Process FormantTracker formants
    CProcessFormantTracker * pFTFormants = pDoc->GetFormantTracker(); // get pointer to FormantTracker object
    short int nResult = LOWORD(pFTFormants->Process(pView, pDoc));
    nResult = CheckResult(nResult, pFTFormants); // check the process result

    return nResult;
}

/***************************************************************************/
// CFormantChart::ProcessFormants  Process formants for frame
/***************************************************************************/
short int CFormantChart::ProcessFormants() {

    // Get pointers to document and main frame.
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaView * pView = (CSaView *)pGraph->GetParent();
    CSaDoc * pDoc   = pView->GetDocument();
    DWORD wSmpSize = pDoc->GetSampleSize();  // calculate sample size in bytes
    bool bDynamicUpdate = (pView->GetGraphUpdateMode() == DYNAMIC_UPDATE);

    // Get formant object and formant plot parameters.
    CProcessFormants * pFormants = (CProcessFormants *)pDoc->GetFormants(); // get pointer to formants object
    CFormantParm * pFormantParms = pFormants->GetFormantParms();   //!!move to formant processing class

    // Finish pitch processing if necessary.
    CProcessGrappl * pAutoPitch = (CProcessGrappl *)pDoc->GetGrappl();
    short int nResult = LOWORD(pAutoPitch->Process(this, pDoc)); // process data
    if (nResult == PROCESS_ERROR || nResult == PROCESS_NO_DATA || nResult == PROCESS_CANCELED) {
        if (nResult == PROCESS_CANCELED) {
            pFormants->CancelProcess();
        }
        return nResult;
    }

    // Finish fragmenting if necessary.
    CProcessFragments * pFragments = (CProcessFragments *)pDoc->GetFragments();
    nResult = LOWORD(pFragments->Process(this, pDoc)); // process data
    if (nResult == PROCESS_ERROR || nResult == PROCESS_NO_DATA || nResult == PROCESS_CANCELED) {
        if (nResult == PROCESS_CANCELED) {
            pFormants->CancelProcess();
        }
        return nResult;
    }

    // Set frame parameters.
    DWORD dwFrameStart, dwFrameSize;
    SSpectProcSelect SpectraSelected;

    if (bDynamicUpdate) {
        // formants extracted from LPC spectrum for current fragment as start cursor is dragged or plot is animating
        if (m_dwAnimationFrame == UNDEFINED_OFFSET) {
            // initialize animation frame for new graph
            DWORD dwStartCursor = pView->GetStartCursorPosition();
            m_dwAnimationFrame = pFragments->GetFragmentIndex(dwStartCursor/wSmpSize);
        }
        SFragParms FragParms = pFragments->GetFragmentParms(m_dwAnimationFrame);
        dwFrameStart = FragParms.dwOffset * wSmpSize;
        dwFrameSize = (DWORD)FragParms.wLength * (DWORD)wSmpSize;
        SpectraSelected.bCepstralSpectrum = FALSE;    // turn off to reduce processing time
        SpectraSelected.bLpcSpectrum = TRUE;
    } else {
        // formants extracted from cepstrally-smoothed and LPC spectra for sample data between start and stop cursors
        dwFrameStart = pView->GetStartCursorPosition();
        dwFrameSize = pView->GetStopCursorPosition() - dwFrameStart + wSmpSize;
        SpectraSelected.bCepstralSpectrum = TRUE;
        SpectraSelected.bLpcSpectrum = TRUE;
    }

    // Process FormantTracker formants
    nResult = ProcessFTFormants();
    if (nResult == PROCESS_ERROR) {
        return nResult;
    }

    // If frame unvoiced, exit.
    if (((!bDynamicUpdate) && ((!m_bVoiced) || (!pAutoPitch->IsVoiced(pDoc, dwFrameStart)))) ||
            ((bDynamicUpdate) && ((!pAutoPitch->IsVoiced(pDoc, dwFrameStart)) || (!pAutoPitch->IsVoiced(pDoc, dwFrameStart+dwFrameSize-wSmpSize))))) {
        return PROCESS_UNVOICED;
    }

    // Extract formants.
    BOOL bTrackFormants = pFormantParms->bTrackFormants && pView->IsAnimationRequested();
    if (pView->GetGraphUpdateMode() == STATIC_UPDATE ||
            ((bDynamicUpdate) && (!pView->IsAnimationRequested())) ||
            (((bDynamicUpdate) || (!pFormantParms->bTrackFormants)) && (pView->IsAnimationRequested()))) {
        nResult = LOWORD(pFormants->Process(this, pDoc, bTrackFormants, dwFrameStart, dwFrameSize, SpectraSelected));    // process data
    }
    return nResult;
}

/***************************************************************************/
// CFormantChart::StandardPlot
/***************************************************************************/
void CFormantChart::StandardPlot(CSaView * pView, CDC * pDC, CRect rClient, CRect rClip,
                                 SGraph & zG, BOOL bFlipChart, BOOL bShowAxes, BOOL bValidData) {
#define VOWEL_FONT_HEIGHT 25

    if (IsIconic()) {
        return;    // nothing to draw
    }

    if (bValidData) {
        m_HelperWnd.SetMode(MODE_HIDDEN);    // hide helper window
    }

    // get a handle to windows for Z-graph
    HDC hdc = pDC->GetSafeHdc();

    // set up an array of colors to be used in the Z-graph
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    Colors * pColor = pMainWnd->GetColors(); // get application colors
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
    CProcessFormants * pFormants = (CProcessFormants *)pDoc->GetFormants(); // get pointer to formants object
    CFormantParm * pFormantParms = pFormants->GetFormantParms();   //!!move to formant processing class
    ProcessFTFormants(); // process FormantTracker formants
    bool bDynamicUpdate = (pView->GetGraphUpdateMode() == DYNAMIC_UPDATE);
    BOOL bTrackFormants = pFormantParms->bTrackFormants;
    BOOL bCluster = (pFormants->GetFormantFrameCount() > 1);
    zRGB PlotColorArray[3];
    if (pView->IsAnimationRequested() && bTrackFormants) {
        PlotColorArray[0] = pColor->cPlotData[4];
        PlotColorArray[1] = pColor->cPlotData[2];
    } else {
        CFormantParm * pFormantSource = pFormants->GetFormantParms();
        PlotColorArray[0] = ((pFormantSource->bFromCepstralSpectrum) && (zG.n == 1) && (!bDynamicUpdate)) ? pColor->cPlotData[0] : pColor->cPlotData[2];
        PlotColorArray[1] = pColor->cPlotData[0];
    }

    int far * DataPointSymbol = NULL;
    if (zG.n) {
        DataPointSymbol = (int far *)new int[zG.n + 1];
        int nPoints = bCluster ? zG.n - 2 : zG.n - 1;
        int nSymbol;
        for (nSymbol = 0; nSymbol < nPoints; nSymbol++) {
            DataPointSymbol[nSymbol] = 'L';    // LPC point
        }
        DataPointSymbol[nSymbol] = 'C';   // cepstral point
        if (bCluster) {
            PlotColorArray[2] = pColor->cPlotStopCursor;  // for mean value of tracking cluster
            DataPointSymbol[++nSymbol] = 'M';
        }
    }

    // fill In the [SGraph] structure elements
    zG.hdc          = hdc            ; // windows HDC
    zG.RWindow      = &rClient       ; // RECT with plot dimensions
    zG.symbols      = (int far *)DataPointSymbol; // symbols array (no symbols used)
    zG.axis_color   = pColor->cPlotGrid; // graph axis color
    zG.bkg_color    = pColor->cPlotBkg;   // graph bkg. color
    zG.plot_color   = PlotColorArray ;    // array of plot colors
    zG.fill_style   = 0              ;
    zG.title        = _T("\0")           ;
    zG.legend_position = zNO_LEGEND  ;
    zG.symbols_flag = OFF            ;
    zG.grid_flag    = HaveGrid()     ;
    zG.auto_scale   = OFF            ;
    zG.draw_forward = TRUE           ;
    zG._3D_flag     = (zG.zdata != NULL);
    zG.bkg_gradient_mode = zNO_GRADIENT;
    zG.digit_format = zFIXED         ; // round axis numbers to nearest whole integer
    zG.digit_precision = 0           ;
    zG.grid_pen_style = zDOTTED      ; // use dotted lines for grid
    zG.shading_style = zNO_SHADING   ;
    zG.quality      = zG3D_QUALITY_HI;
    zG.function     = NULL           ;

    if ((!bValidData) && (zG.zdata == (double *)-1)) {
        zG.zdata = NULL;    // Patch, to allow 3D graph with no data
    }

    // draw the graph -- this could be done by calling the ScatterGraph function,
    // but we need to draw vowels over the graph and therefore need to create
    // a ScatterGraph object and use its member functions
    CScatterGraph * pFormantChart = new CScatterGraph(&zG); // construct new graph window
    pFormantChart->zInitGraph();                            // initialize it

    // scale data and reverse it as needed
    pFormantChart->zScaleData();
    if (bFlipChart) {
        pFormantChart->zFlip();
    }

    // draw the graph and plot the data
    if (bShowAxes) {
        pFormantChart->zDrawGraphAxes();
    } else {
        pFormantChart->zzDrawGraphAxes(FALSE, FALSE);
    }
    pFormantChart->zDrawGrid();
    if (zG.n) {
        pFormantChart->zDrawDataPoints();
    }

    // set the vowel font
    pDC->SetTextColor(pColor->cPlotStopCursor); // set font color
    pDC->SetBkMode(TRANSPARENT); // letters must be transparent
    CFont * pOldFont = pDC->SelectObject((CFont *)pDoc->GetFont(PHONETIC));
    TEXTMETRIC tm;
    pDC->GetTextMetrics(&tm);
    CFont GridVowelFont;
    int nHeight = max(VOWEL_FONT_HEIGHT, (int)(.15 * min(rClient.Height(),rClient.Width()) + 0.5));
    GridVowelFont.CreateFont(nHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, tm.tmCharSet, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH|FF_DONTCARE,
                             pMainWnd->GetFontFace(PHONETIC));  // create font for vowels on the grid
    pDC->SelectObject(&GridVowelFont); // select phonetic font

    // select a pen
    CPen GridVowelPen(PS_SOLID, 1, pColor->cPlotGrid);
    CPen * pOldPen = pDC->SelectObject(&GridVowelPen);

    POINT p;

    // draw the vowels
    EGender nGender = pDoc->GetGender();

    // Draw Vowel Set Title
    const CSaString & szVowelSetName = GetDefaultVowelSet().GetName();
    if ((!szVowelSetName.IsEmpty()) && (szVowelSetName != "(None)")) {
        CFont vowelTitleFont;
        const _TCHAR VowelTitleTypeface[] = _T("Arial");
        vowelTitleFont.CreateFont(int(0.6*VOWEL_FONT_HEIGHT), 0, 0, 0, FW_NORMAL, 0, 0, 0,
                                  ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  DEFAULT_QUALITY, (VARIABLE_PITCH | FF_ROMAN), VowelTitleTypeface);
        CFont * pOldFont = pDC->SelectObject(&vowelTitleFont);
        CSize TitleSize = pDC->GetTextExtent((CString &)szVowelSetName);
        int nLeft = pFormantChart->X_LEFT + TitleSize.cy/3;
        int nRight = nLeft + TitleSize.cx;
        int nBottom = pFormantChart->Y_BOTTOM;
        int nTop = nBottom - TitleSize.cy;
        CRect rect(nLeft, nTop, nRight, nBottom);
        COLORREF oldColor = pDC->SetTextColor(pColor->cPlotData[0]); // set font color
        pDC->DrawText(szVowelSetName, rect, DT_SINGLELINE | DT_TOP | DT_LEFT | DT_NOCLIP | DT_NOPREFIX);

        if (pFormants->GetVowelCount(nGender) == 0) {
            int nTop = pFormantChart->Y_TOP + (pFormantChart->Y_BOTTOM - pFormantChart->Y_TOP)/3;
            CRect rVowel(pFormantChart->X_LEFT, nTop, pFormantChart->X_RIGHT,pFormantChart->Y_BOTTOM);

            CSaString szWarning;

            szWarning.LoadString(IDS_EMPTY_FORMANT_VOWEL_SET);
            pDC->DrawText(szWarning, rVowel, DT_SINGLELINE | DT_CENTER | DT_TOP | DT_NOCLIP | DT_NOPREFIX);
        }
        pDC->SetTextColor(oldColor); // reset font color
        pDC->SelectObject(pOldFont);
    }

    for (int nVowel = 0; nVowel < pFormants->GetVowelCount(nGender); nVowel++) {
        CSaString vowel = pFormants->GetVowel(nVowel, nGender);
        SFormantFreq Formant = pFormants->GetVowelFreq(nVowel, nGender, pFormantParms->bMelScale);
        CSize extent = pDC->GetTextExtent(vowel);
        p = MapVowelsToChart(pDC, Formant, extent.cy, pFormantChart, zG, bFlipChart);
        // display the vowel symbol
        CRect rect(p.x - extent.cx/2, p.y - extent.cy/2, p.x + extent.cx/2, p.y + extent.cy/2);
        pDC->DrawText(vowel, rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_NOCLIP);
    }

    // draw the nearest vowel
    if ((bDynamicUpdate) && (bValidData) && (szVowelSetName != "(None)") && (pFormants->GetVowelCount(nGender))) {

#define NEAREST_SPACING 15

        // draw the 'nearest vowel' character
        CSaString vowelId = pFormants->FindNearestVowel(pDoc);
        vowelId = vowelId.Trim();
        CSize extent = pDC->GetTextExtent(vowelId);

        TEXTMETRIC tm;
        pDC->GetTextMetrics(&tm);

        int nTop = pFormantChart->Y_TOP;
        int nBottom = nTop + extent.cy;
        int nRight = pFormantChart->X_RIGHT - NEAREST_SPACING;
        int nLeft = nRight - extent.cx;
        CRect rect(nLeft, nTop, nRight, nBottom);
        pDC->DrawText(vowelId, rect, DT_SINGLELINE | DT_BOTTOM | DT_CENTER | DT_NOCLIP);

        pDC->SetTextColor(pColor->cPlotData[0]); // set font color

        const TCHAR vowelIdTitleTypeface[] = _T("Arial");
        CFont titleFont;
        titleFont.CreateFont(int(0.6*VOWEL_FONT_HEIGHT), 0, 0, 0, FW_NORMAL, 0, 0, 0,
                             ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, (VARIABLE_PITCH | FF_ROMAN), vowelIdTitleTypeface);
        CFont * oldFont = pDC->SelectObject(&titleFont);
        const CSaString title = _T("Nearest Vowel: ");
        CSize extent2 = pDC->GetTextExtent(title);

        TEXTMETRIC tm2;
        pDC->GetTextMetrics(&tm2);

        // adjust top and bottom so that our baselines match
        int diff = tm.tmDescent - tm2.tmDescent;
        nTop -= diff;
        nBottom -= diff;
        nRight = pFormantChart->X_RIGHT - NEAREST_SPACING - extent.cx;
        nLeft = nRight - extent2.cx;
        CRect rect2(nLeft, nTop, nRight, nBottom);
        pDC->DrawText(title, rect2, DT_SINGLELINE | DT_BOTTOM | DT_CENTER | DT_NOCLIP);
        pDC->SelectObject(oldFont);

    }
    pDC->SelectObject(pOldPen);  // restore old pen
    pDC->SelectObject(pOldFont); // restore old font

    // label the graph
    pFormantChart->zDrawLegend();

    TRACE("sps=%d\n",pDoc->GetSamplesPerSec());

    // check if data is outside graph boundaries
    if ((zG.xdata!=NULL) && (zG.ydata!=NULL) && (bValidData)) {
        TRACE("zG=%f %f %f %f %f %f %d\n",zG.xstart,zG.xend,zG.ystart,zG.yend,zG.xdata[0],zG.ydata[0],zG.n);
        int xScaleDirection = (zG.xend > zG.xstart ? 1 : -1);
        int yScaleDirection = (zG.yend > zG.ystart ? 1 : -1);
        int nInRangeCount = 0;
        for (int i = 0; i < zG.n; i++) {
            BOOL isInRange = (xScaleDirection * (zG.xdata[i] - zG.xend) < 0) &&
                             (xScaleDirection * (zG.xdata[i] - zG.xstart) > 0) &&
                             (yScaleDirection * (zG.ydata[i] - zG.yend) < 0) &&
                             (yScaleDirection * (zG.ydata[i] - zG.ystart) > 0);
            if (isInRange) {
                nInRangeCount++;
            }
        }
        if (nInRangeCount == 0) {
            m_bOutOfRange = TRUE;
            m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_OUT_OF_RANGE, &rClient);
        } else {
            m_bOutOfRange = FALSE;
            m_HelperWnd.SetMode(MODE_HIDDEN);
        }
    }

    // delete the graph object
    pFormantChart->zTerminateGraph();
    delete pFormantChart;
    if (DataPointSymbol) {
        delete [] DataPointSymbol;
    }

    if (zG._3D_flag) {
        // set legend scale
        GetParent()->SetLegendScale(SCALE | NUMBERS | ARBITRARY, zG.zstart, zG.zend, zG.V_Title, 5, 0.25);
        GetParent()->SetXScale(SCALE | NUMBERS, int(zG.ystart), int(zG.yend), zG.H_Title, zG.y_axis_divisions, 0.20);
    } else {
        // set legend scale
        GetParent()->SetLegendScale(SCALE | NUMBERS | ARBITRARY, zG.ystart, zG.yend, zG.V_Title, zG.y_axis_divisions);
        GetParent()->SetXScale(SCALE | NUMBERS, int(zG.xstart), int(zG.xend), zG.H_Title, zG.x_axis_divisions);
    }

    // finish the plot
    PlotPaintFinish(pDC, rClient, rClip);

    //redraw helper window
    if (m_bOutOfRange) {
        m_HelperWnd.Invalidate();
        m_HelperWnd.UpdateWindow();
    }
}

/***************************************************************************/
// CFormantChart::GetXYCoordinates Converts XY-coordinates in plot pixel point
/***************************************************************************/
POINT CFormantChart::GetXYCoordinates(void * pG, double x , double y, BOOL bFlip) {
    CScatterGraph * pGraph2D = (CScatterGraph *)pG; // cast pointer
    INT grid_width  = abs(pGraph2D->X_RIGHT - pGraph2D->X_LEFT);
    INT grid_height = abs(pGraph2D->Y_TOP - pGraph2D->Y_BOTTOM);
    double a = (double) grid_width / (pGraph2D->xmax - pGraph2D->xmin);
    double b = (double) pGraph2D->X_LEFT - a * pGraph2D->xmin;
    double c = zY_ADJUST * (double) grid_height / (pGraph2D->ymin - pGraph2D->ymax);
    double d = (double) pGraph2D->Y_TOP - c * pGraph2D->ymax;
    POINT p;
    // scale x and y coordinates
    if (bFlip) {
        p.x = (INT)(a * ((pGraph2D->xmax + pGraph2D->xmin) - x) + b);
        p.y = (INT)(c * ((pGraph2D->ymax + pGraph2D->ymin) - y) + d);
    } else {
        p.x = (INT)(a * x + b);
        p.y = (INT)(c * y + d);
    }

    return p;
}

/***************************************************************************/
// CFormantChart::AnimateFrame  Animate a single frame
/***************************************************************************/
void CFormantChart::AnimateFrame(DWORD dwFrameIndex) {
    CPlotWnd::StandardAnimateFrame(dwFrameIndex);
}

/***************************************************************************/
// CFormantChart::EndAnimation  Terminate animation
/***************************************************************************/
void CFormantChart::EndAnimation(void) {
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaView * pView = (CSaView *)pGraph->GetParent();
    int nWaveGraphIndex = pView->GetGraphIndexForIDD(IDD_RAWDATA);
    CGraphWnd * pWaveGraph = pView->GetGraph(nWaveGraphIndex);
    if (pWaveGraph) {
        CPlotWnd * pWavePlot = pWaveGraph->GetPlot();
        pWavePlot->SetHighLightArea(0, 0);
    }
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
    CProcessFormants * pFormants = (CProcessFormants *)pDoc->GetFormants(); // get pointer to formants object
    CFormantParm * pFormantParms = pFormants->GetFormantParms();
    if (pFormantParms->bSmoothFormants) {
        pFormants->SmoothMedian(pDoc);
    }
    m_dwAnimationFrame = UNDEFINED_OFFSET;
    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

/***************************************************************************/
// CFormantChart::GraphHasFocus  Handle plot when graph gains/loses focus
/***************************************************************************/
void CFormantChart::GraphHasFocus(BOOL bFocus) {
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    CSaView * pView = (pMainWnd)?(CSaView *)pMainWnd->GetCurrSaView():NULL;
    if (!pView) {
        return;
    }
    if (m_bAnimationPlot) {
        int nWaveGraphIndex = pView->GetGraphIndexForIDD(IDD_RAWDATA);
        CGraphWnd * pWaveGraph = pView->GetGraph(nWaveGraphIndex);
        if (pWaveGraph) {
            BOOL bDynamicUpdate = (pView->GetGraphUpdateMode() == DYNAMIC_UPDATE);
            if (bDynamicUpdate) {
                CPlotWnd * pWavePlot = pWaveGraph->GetPlot();
                CSaDoc * pDoc = pView->GetDocument();
                CProcessFormants * pFormants = pDoc->GetFormants();
                BOOL bCluster = (pFormants->GetFormantFrameCount() > 1);
                if (bFocus && !bCluster) {
                    // Highlight raw data frame for which formants are calculated
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
                    pWavePlot->SetHighLightArea(0, 0);
                    pWavePlot->UpdateWindow();
                }
            }
        }
    }
}

//###########################################################################
// CPlot3D
// Plot window for displaying formants data. This class only handles
// the special drawing of the data. All other work is done in the base class.
// For the drawing, this class holds three buffers (each 1kByte) for the 3D
// data to deliver to the Z-graph functions.

IMPLEMENT_DYNCREATE(CPlot3D, CFormantChart)

/////////////////////////////////////////////////////////////////////////////
// FormantChart message map

BEGIN_MESSAGE_MAP(CPlot3D, CPlotWnd)
    ON_WM_CREATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlot3D construction/destruction/creation

/***************************************************************************/
// CPlot3D::CPlot3D Constructor
/***************************************************************************/
CPlot3D::CPlot3D() {
}

/***************************************************************************/
// CPlot3D::~CPlot3D Destructor
/***************************************************************************/
CPlot3D::~CPlot3D() {
}

/***************************************************************************/
// CPlot3D::GetChartRange Retrieves ranges for each axis
// Returns range of plot along an axis for a given formant and gender
/***************************************************************************/
SRange CPlot3D::GetChartRange(int nFormant, int nGender) {
    //               F1          F2            F3
    static SRange Scale[3][4] = {
        {{0.,0.},{200.,1000.},{650.,2500.},{1500.,3200.}},  // man
        {{0.,0.},{200.,1000.},{700.,3000.},{1850.,3500.}},  // woman
        {{0.,0.},{350.,1100.},{900.,3400.},{1950.,3900.}}
    }; // child
    if (nFormant < 0 || nFormant >= 4 || nGender < 0 || nGender >= 3) {
        nFormant = nGender = 0;
    }
    return Scale[nGender][nFormant];
}

/***************************************************************************/
// CPlot3D::MapVowelsToChart  Map vowels to chart coordinates
/***************************************************************************/
POINT CPlot3D::MapVowelsToChart(CDC * pDC, SFormantFreq & Formant, int SymbolHeight, CScatterGraph * pFormantChart,
                                SGraph & ChartParms, BOOL /* bFlip */) {
    POINT p = pFormantChart->zMapCoordinates(Formant.F[2], Formant.F[1], Formant.F[3]); // converts 3D point to 2D
    POINT pBase = pFormantChart->zMapCoordinates(Formant.F[2], Formant.F[1], ChartParms.zstart);
    pDC->MoveTo(p.x, p.y + SymbolHeight / 2);
    pDC->LineTo(pBase.x, pBase.y);
    return p;
}

/////////////////////////////////////////////////////////////////////////////
// CPlot3D message handlers

/***************************************************************************/
// CPlot3D::OnCreate Window creation
// The plot has to get the area process index, because it is a area process
// plot.
/***************************************************************************/
int CPlot3D::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CPlotWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }
    return 0;
}

/***************************************************************************/
// CPlot3D::OnDraw Drawing
// The data to draw is coming from a temporary file, created by the
// formants processing class, which is called to do data processing if
// necessary before drawing. The plot itself is drawn by the ZGraph
// functions.
/***************************************************************************/
void CPlot3D::OnDraw(CDC * pDC, CRect rClient, CRect rClip, CSaView * pView) {
    // Get pointer to application, document, and formant processing object.
    CSaApp * pApp = (CSaApp *)AfxGetApp();
    CSaDoc * pDoc = pView->GetDocument();
    CProcessFormants * pFormants = (CProcessFormants *)pDoc->GetFormants();
    CProcessFormantTracker * pFTFormants = pDoc->GetFormantTracker(); // get pointer to FormantTracker object
    CFormantParm * pFormantParms = pFormants->GetFormantParms();
    CProcessFragments * pFragments = (CProcessFragments *)pDoc->GetFragments();
    WORD wSmpSize = (WORD)(pDoc->GetSampleSize());  // calculate sample size in bytes
    bool bDynamicUpdate = (pView->GetGraphUpdateMode() == DYNAMIC_UPDATE);
    BOOL bDataValid = TRUE;

    // Get the formants.
    short nResult = ProcessFormants();
    nResult = CheckResult(nResult, pFormants); // check the process result
    switch (nResult) {
    case PROCESS_CANCELED:
        bDataValid = FALSE;
        break;
    case PROCESS_DATA_OVERLOAD: {
        int nWaveGraphIndex = pView->GetGraphIndexForIDD(IDD_RAWDATA);
        CGraphWnd * pWaveGraph = pView->GetGraph(nWaveGraphIndex);
        if (pWaveGraph) {
            CPlotWnd * pWavePlot = pWaveGraph->GetPlot();
            if (bDynamicUpdate && pWavePlot) {
                // turn off highlighted area in raw data
                pWavePlot->SetHighLightArea(0, 0);
                pWavePlot->UpdateWindow();
            }
        }
    }
    bDataValid = FALSE;
    break;
    case PROCESS_ERROR:
    case PROCESS_NO_DATA:
    case PROCESS_UNVOICED:
        bDataValid = FALSE;
        break;
    case PROCESS_DONE:
        //  Invalidate();
        //  return;
        break;
    }

    SGraph FormantChartParm;
    memset(&FormantChartParm, zDEFAULT, sizeof(FormantChartParm));

    // Allocate buffers for x, y, and z coordinates.  For a 3D graph, the y-axis is horizontal, the
    // z-axis is vertical, and the x-axis is diagonal.  Data increases along the x-axis toward the lower
    // left corner if the graph parameter draw_forward is TRUE.
    DWORD dwFormantFrameCount = pFormants->GetFormantFrameCount();
    DWORD startCursorPos = pView->GetStartCursorPosition();
    DWORD stopCursorPos = pView->GetStopCursorPosition();
    DWORD startFrag = pFragments->GetFragmentIndex(startCursorPos / wSmpSize);
    DWORD stopFrag = pFragments->GetFragmentIndex(stopCursorPos / wSmpSize);
    double sizeFactorFT2Raw = (double)pFTFormants->GetDataSize() / (double)pDoc->GetDataSize() / (sizeof(SFormantFreq) / 2);
    double sizeFactorFT2Frag = sizeFactorFT2Raw * (stopCursorPos - startCursorPos) / (stopFrag - startFrag);
    DWORD ftIndex = (DWORD)((double)startCursorPos * sizeFactorFT2Raw);
    SFormantFreq * formants = pFTFormants->GetFormant(ftIndex);

    double meanF1 = 0;
    double meanF2 = 0;
    double meanF3 = 0;

    if (formants==NULL) {
        bDataValid = FALSE;
    }

    if (bDataValid) {
        DWORD dwMaxNumberPoints = dwFormantFrameCount;
        BOOL bCluster = (dwFormantFrameCount > 1);
        if (bCluster) {
            dwMaxNumberPoints++;
        }
        if (dwMaxNumberPoints > zNUM_POINTS) {
            dwMaxNumberPoints = zNUM_POINTS;    // limit number to buffer sizes
        }
        FormantChartParm.xdata = (double *)malloc(dwMaxNumberPoints * sizeof(double));
        if (!FormantChartParm.xdata) {
            // memory allocation error
            pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
            GetParent()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
            return;
        }
        FormantChartParm.ydata = (double *)malloc(dwMaxNumberPoints * sizeof(double));
        if (!FormantChartParm.ydata) {
            // memory allocation error
            free(FormantChartParm.xdata);
            pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
            GetParent()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
            return;
        }
        FormantChartParm.zdata = (double *)malloc(dwMaxNumberPoints * sizeof(double));
        if (!FormantChartParm.zdata) {
            // memory allocation error
            free(FormantChartParm.xdata);
            free(FormantChartParm.ydata);
            pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
            GetParent()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
            return;
        }

        // Collect formant frequency points for all frames.
        DWORD ix = 0, iy = 0, iz = 0;
        for (DWORD dwFormantFrame = 0; dwFormantFrame < dwFormantFrameCount; dwFormantFrame++) {
            DWORD dwFTFormantFrame = (DWORD)((double)dwFormantFrame * sizeFactorFT2Frag);
            if (dwFormantFrame != 0) {
                formants = pFTFormants->GetFormant(ftIndex + dwFTFormantFrame);
            }
            if ((formants->F[1] != (float)UNDEFINED_DATA) &&
                    (formants->F[2] != (float)UNDEFINED_DATA) &&
                    (formants->F[3] != (float)UNDEFINED_DATA)) {
                meanF1 = (meanF1 * ix + formants->F[1]) / (ix + 1);
                meanF2 = (meanF2 * iy + formants->F[2]) / (iy + 1);
                meanF3 = (meanF3 * iz + formants->F[3]) / (iz + 1);
                if (pFormantParms->bMelScale) {
                    FormantChartParm.xdata[ix++] = Scale::ToMels((double)formants->F[2]);
                    FormantChartParm.ydata[iy++] = Scale::ToMels((double)formants->F[1]);
                    FormantChartParm.zdata[iz++] = Scale::ToMels((double)formants->F[3]);
                } else {
                    FormantChartParm.xdata[ix++] = (double)formants->F[2];
                    FormantChartParm.ydata[iy++] = (double)formants->F[1];
                    FormantChartParm.zdata[iz++] = (double)formants->F[3];
                }
            }
        }

        // Get mean if there is a tracking cluster.
        if (bCluster) {
            if (pFormantParms->bMelScale) {
                FormantChartParm.xdata[ix++] = Scale::ToMels(meanF2);
                FormantChartParm.ydata[iy++] = Scale::ToMels(meanF1);
                FormantChartParm.zdata[iz++] = Scale::ToMels(meanF3);
            } else {
                FormantChartParm.xdata[ix++] = meanF2;
                FormantChartParm.ydata[iy++] = meanF1;
                FormantChartParm.zdata[iz++] = meanF3;
            }
        }

        FormantChartParm.n = (int)ix;  // set total number of data points
    } else {
        FormantChartParm.zdata = (double *)-1; // Force graph to still be 3D
    }

    // Customize the axes.
    FormantChartParm.x_axis_style = zLINEAR;
    FormantChartParm.y_axis_style = zLINEAR;
    FormantChartParm.x_axis_divisions = 5;                 // x-axis divisions
    FormantChartParm.y_axis_divisions = 6;                 // y-axis divisions
    FormantChartParm.H_Title = pFormantParms->bMelScale ? _T("F1(mels)"): _T("F1 (Hz)");  // horizontal axis title
    FormantChartParm.V_Title = pFormantParms->bMelScale ? _T("F3 (mels)"): _T("F3 (Hz)");  // vertical axis title
    FormantChartParm.item_title_format = zNUMBERS;         // show numbers along axes
    FormantChartParm.line_connect = ON;          ;

    EGender nGender = pDoc->GetGender();
    SRange FormantScale = GetChartRange(2, nGender);
    FormantChartParm.xstart = pFormantParms->bMelScale ? Scale::ToMels(FormantScale.Low): FormantScale.Low;
    FormantChartParm.xend = pFormantParms->bMelScale ? Scale::ToMels(FormantScale.High): FormantScale.High;
    FormantScale = GetChartRange(1, nGender);
    FormantChartParm.ystart = pFormantParms->bMelScale ? Scale::ToMels(FormantScale.Low): FormantScale.Low;
    FormantChartParm.yend = pFormantParms->bMelScale ? Scale::ToMels(FormantScale.High): FormantScale.High;
    FormantScale = GetChartRange(3, nGender);
    FormantChartParm.zstart = pFormantParms->bMelScale ? Scale::ToMels(FormantScale.Low): FormantScale.Low;
    FormantChartParm.zend = pFormantParms->bMelScale ? Scale::ToMels(FormantScale.High): FormantScale.High;

    // Draw the chart.
    BOOL bFlipChart = FALSE;
    BOOL bShowAxes = TRUE;
    StandardPlot(pView, pDC, rClient, rClip, FormantChartParm, bFlipChart, bShowAxes, bDataValid);

    if (bDataValid) {
        // Release data point buffers.
        free(FormantChartParm.xdata);
        free(FormantChartParm.ydata);
        free(FormantChartParm.zdata);
    }
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  F1 F2 PLOT  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

//###########################################################################
// CPlotF1F2
// Plot window for displaying formants data. This class only handles
// the special drawing of the data. All other work is done in the base class.
// For the drawing, this class holds three buffers (each 1kByte) for the x/y
// data to deliver to the Z-graph functions.

IMPLEMENT_DYNCREATE(CPlotF1F2, CFormantChart)

/////////////////////////////////////////////////////////////////////////////
// CPlotF1F2 message map

BEGIN_MESSAGE_MAP(CPlotF1F2, CPlotWnd)
    ON_WM_CREATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotF1F2 construction/destruction/creation

/***************************************************************************/
// CPlotF1F2::CPlotF1F2 Constructor
/***************************************************************************/
CPlotF1F2::CPlotF1F2() {
}

/***************************************************************************/
// CPlotF1F2::~CPlotF1F2 Destructor
/***************************************************************************/
CPlotF1F2::~CPlotF1F2() {
}

/***************************************************************************/
// CPlotF1F2::GetChartRange Retrieves ranges for each axis
// Returns range of plot along an axis for a given formant and gender
/***************************************************************************/
SRange CPlotF1F2::GetChartRange(int nFormant, int nGender) {
    //                     F1          F2
    static SRange Scale[3][3] = {
        {{0.0,0.0}, {200.0,1000.0}, {500.0,2500.0}},  // man
        {{0.0,0.0}, {200.0,1000.0}, {700.0,3000.0}},  // woman
        {{0.0,0.0}, {350.0,1100.0}, {900.0,3400.0}}
    }; // child
    if ((nFormant < 0) ||
            (nFormant >= 3) ||
            (nGender < 0) ||
            (nGender >= 3)) {
        nFormant = nGender = 0;
    }

    return Scale[nGender][nFormant];
}

/***************************************************************************/
// CPlotF1F2::MapVowelsToChart  Map vowels to chart coordinates
/***************************************************************************/
POINT CPlotF1F2::MapVowelsToChart(CDC * /* pDC */, SFormantFreq & Formant, int /* SymbolHeight */, CScatterGraph * pFormantChart,
                                  SGraph & /* ChartParms */, BOOL bFlip) {
    return GetXYCoordinates(pFormantChart, Formant.F[1], Formant.F[2], bFlip); // converts 2D point to plot coordinates
}
/////////////////////////////////////////////////////////////////////////////
// CPlotF1F2 message handlers

/***************************************************************************/
// CPlotF1F2::OnCreate Window creation
// The plot has to get the area process index, because it is a area process
// plot.
/***************************************************************************/
int CPlotF1F2::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CPlotWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }
    return 0;
}

/***************************************************************************/
// CPlotF1F2::OnDraw Painting
// The data to draw is coming from a temporary file, created by the
// formants processing class, which is called to do data processing if
// necessary before painting. The plot itself is painted by the ZGraph
// functions.
/***************************************************************************/
void CPlotF1F2::OnDraw(CDC * pDC, CRect rClient, CRect rClip, CSaView * pView) {
    TRACE("CPlotF1F2::OnDraw\n");

    // Get pointer to application, document, and formant processing object.
    CSaApp * pApp = (CSaApp *)AfxGetApp();
    CSaDoc * pDoc = pView->GetDocument();
    CProcessFormants * pFormants = (CProcessFormants *)pDoc->GetFormants();
    CProcessFormantTracker * pFTFormants = pDoc->GetFormantTracker(); // get pointer to FormantTracker object
    CFormantParm * pFormantParms = pFormants->GetFormantParms();
    CProcessFragments * pFragments = (CProcessFragments *)pDoc->GetFragments();
    WORD wSmpSize = (WORD)(pDoc->GetSampleSize());  // calculate sample size in bytes
    bool bDynamicUpdate = (pView->GetGraphUpdateMode() == DYNAMIC_UPDATE);
    BOOL bDataValid = TRUE;

    // Get the formants.
    short int nResult = ProcessFormants();
    nResult = CheckResult(nResult, pFormants); // check the process result
    switch (nResult) {
    case PROCESS_CANCELED:
        bDataValid = FALSE;
        break;
    case PROCESS_DATA_OVERLOAD: {
        int nWaveGraphIndex = pView->GetGraphIndexForIDD(IDD_RAWDATA);
        CGraphWnd * pWaveGraph = pView->GetGraph(nWaveGraphIndex);
        if (pWaveGraph) {
            CPlotWnd * pWavePlot = pWaveGraph->GetPlot();
            if ((bDynamicUpdate) && (pWavePlot!=NULL)) {
                // turn off highlighted area in raw data
                pWavePlot->SetHighLightArea(0, 0);
                pWavePlot->UpdateWindow();
            }
        }
    }
    bDataValid = FALSE;
    break;
    case PROCESS_ERROR:
    case PROCESS_NO_DATA:
    case PROCESS_UNVOICED:
        bDataValid = FALSE;
        break;
    case PROCESS_DONE:
        //  Invalidate();
        //  return;
        break;
    }

    SGraph FormantChartParm;
    memset(&FormantChartParm, zDEFAULT, sizeof(FormantChartParm));

    // Allocate buffers for x and y coordinates;
    DWORD dwFormantFrameCount = pFormants->GetFormantFrameCount();
    DWORD startCursorPos = pView->GetStartCursorPosition();
    DWORD stopCursorPos = pView->GetStopCursorPosition();
    DWORD startFrag = pFragments->GetFragmentIndex(startCursorPos / wSmpSize);
    DWORD stopFrag = pFragments->GetFragmentIndex(stopCursorPos / wSmpSize);
    double sizeFactorFT2Raw = (double)pFTFormants->GetDataSize() / (double)pDoc->GetDataSize() / (sizeof(SFormantFreq) / 2);
    double sizeFactorFT2Frag = sizeFactorFT2Raw * (stopCursorPos - startCursorPos) / (stopFrag - startFrag);
    DWORD ftIndex = (DWORD)((double)startCursorPos * sizeFactorFT2Raw);
    SFormantFreq * formants = pFTFormants->GetFormant(ftIndex);

    double meanF1 = 0;
    double meanF2 = 0;

    if (formants==NULL) {
        bDataValid = FALSE;
    }

    if (bDataValid) {
        DWORD dwMaxNumberPoints = dwFormantFrameCount;
        BOOL bCluster = (dwFormantFrameCount > 1);
        if (bCluster) {
            dwMaxNumberPoints++;
        }
        if (dwMaxNumberPoints > zNUM_POINTS) {
            dwMaxNumberPoints = zNUM_POINTS;    // limit number to buffer sizes
        }
        FormantChartParm.xdata = (double *)malloc(dwMaxNumberPoints * sizeof(double));
        if (!FormantChartParm.xdata) {
            // memory allocation error
            pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
            GetParent()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
            return;
        }
        FormantChartParm.ydata = (double *)malloc(dwMaxNumberPoints * sizeof(double));
        if (!FormantChartParm.ydata) {
            // memory allocation error
            free(FormantChartParm.xdata);
            pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
            GetParent()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
            return;
        }
        FormantChartParm.zdata = NULL;

        // Collect formant frequency pairs for all frames.
        DWORD ix = 0, iy = 0;
        for (DWORD dwFormantFrame = 0; dwFormantFrame < dwFormantFrameCount; dwFormantFrame++) {
            DWORD dwFTFormantFrame = (DWORD)((double)dwFormantFrame * sizeFactorFT2Frag);
            if (dwFormantFrame != 0) {
                formants = pFTFormants->GetFormant(ftIndex + dwFTFormantFrame);
            }

            TRACE("ftIndex=%d\n",ftIndex+dwFTFormantFrame);

            if ((formants->F[1] != (float)UNDEFINED_DATA) &&
                    (formants->F[2] != (float)UNDEFINED_DATA)) {
                meanF1 = (meanF1 * ix + formants->F[1]) / (ix + 1);
                meanF2 = (meanF2 * iy + formants->F[2]) / (iy + 1);
                if (pFormantParms->bMelScale) {
                    FormantChartParm.xdata[ix++] = Scale::ToMels((double)formants->F[1]);
                    FormantChartParm.ydata[iy++] = Scale::ToMels((double)formants->F[2]);
                    TRACE("FTFormant %f %f (1)\n",formants->F[1],formants->F[2]);
                } else {
                    FormantChartParm.xdata[ix++] = (double)formants->F[1];
                    FormantChartParm.ydata[iy++] = (double)formants->F[2];
                    TRACE("FTFormant %f %f (2)\n",formants->F[1],formants->F[2]);
                }
            }
        }

        // Get mean if there is a tracking cluster.
        if (bCluster) {
            if (pFormantParms->bMelScale) {
                FormantChartParm.xdata[ix++] = Scale::ToMels(meanF1);
                FormantChartParm.ydata[iy++] = Scale::ToMels(meanF2);
            } else {
                FormantChartParm.xdata[ix++] = meanF1;
                FormantChartParm.ydata[iy++] = meanF2;
            }
        }
        FormantChartParm.n = (int)ix;  // set total number of data points
    }

    // Customize the axes.
    FormantChartParm.x_axis_style = zLINEAR;
    FormantChartParm.y_axis_style = zLINEAR;
    FormantChartParm.x_axis_divisions = 6;                 // x-axis divisions
    FormantChartParm.y_axis_divisions = 5;                 // y-axis divisions
    FormantChartParm.item_title_format = zNUMBERS;         // no numbers along axes
    FormantChartParm.line_connect = FALSE;

    EGender nGender = pDoc->GetGender();
    SRange FormantScale = GetChartRange(1, nGender);
    FormantChartParm.xstart = pFormantParms->bMelScale ? Scale::ToMels(FormantScale.Low): FormantScale.Low;
    FormantChartParm.xend = pFormantParms->bMelScale ? Scale::ToMels(FormantScale.High): FormantScale.High;
    FormantScale = GetChartRange(2, nGender);
    FormantChartParm.ystart = pFormantParms->bMelScale ? Scale::ToMels(FormantScale.Low): FormantScale.Low;
    FormantChartParm.yend = pFormantParms->bMelScale ? Scale::ToMels(FormantScale.High): FormantScale.High;
    FormantChartParm.zstart = 0, FormantChartParm.zend = 0;

    // use CSaStrings so they autodelete
    CSaString szHTitle;
    CSaString szVTitle;

    szHTitle = pFormantParms->bMelScale ? "F1(mels)" : "F1 (Hz)";
    szVTitle = pFormantParms->bMelScale ? "F2 (mels)" : "F2 (Hz)";

    FormantChartParm.H_Title = const_cast<LPTSTR>((LPCTSTR)szHTitle);
    FormantChartParm.V_Title = const_cast<LPTSTR>((LPCTSTR)szVTitle);

    // Draw the chart.
    BOOL bFlipChart = FALSE;
    BOOL bShowAxes = TRUE;
    StandardPlot(pView, pDC, rClient, rClip, FormantChartParm, bFlipChart, bShowAxes, bDataValid);

    if (bDataValid) {
        // Release data point and title buffers.
        free(FormantChartParm.xdata);
        free(FormantChartParm.ydata);
    }
}


// ++++++++++++++++++++++++++  BEGIN UPGRADE OF F2 F1 PLOT  ++++++++++++++++++++++++++ tdg


//###########################################################################
// CPlotF2F1
// Plot window for displaying formants data. This class only handles
// the special drawing of the data. All other work is done in the base class.
// For the drawing, this class holds three buffers (each 1kByte) for the x/y
// data to deliver to the Z-graph functions.

IMPLEMENT_DYNCREATE(CPlotF2F1, CPlotWnd)

/////////////////////////////////////////////////////////////////////////////
// CPlotF2F1 message map

BEGIN_MESSAGE_MAP(CPlotF2F1, CPlotWnd)
    ON_WM_CREATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotF2F1 construction/destruction/creation

/***************************************************************************/
// CPlotF2F1::CPlotF2F1 Constructor
/***************************************************************************/
CPlotF2F1::CPlotF2F1() {
}

/***************************************************************************/
// CPlotF2F1::~CPlotF2F1 Destructor
/***************************************************************************/
CPlotF2F1::~CPlotF2F1() {
}

/***************************************************************************/
// CPlotF2F1::GetChartRange Retrieves ranges for each axis
// Returns range of plot along an axis for a given formant and gender
/***************************************************************************/
SRange CPlotF2F1::GetChartRange(int nFormant, int nGender) {
    //                F1              F2
    static SRange Scale[3][3] = {
        {{0.0,0.0}, {200.0,1000.0}, {500.0,2500.0}},  // man
        {{0.0,0.0}, {200.0,1000.0}, {700.0,3000.0}},  // woman
        {{0.0,0.0}, {350.0,1100.0}, {900.0,3400.0}}
    }; // child

    if (nFormant < 0 || nFormant >= 3 || nGender < 0 || nGender >= 3) {
        nFormant = nGender = 0;
    }
    return Scale[nGender][nFormant];
}

/***************************************************************************/
// CPlotF2F1::MapVowelsToChart  Map vowels to chart coordinates
/***************************************************************************/
POINT CPlotF2F1::MapVowelsToChart(CDC * /* pDC */, SFormantFreq & Formant, int /* SymbolHeight */, CScatterGraph * pFormantChart,
                                  SGraph & /* ChartParms */, BOOL bFlip) {
    return GetXYCoordinates(pFormantChart, Formant.F[2], Formant.F[1], bFlip); // converts 2D point to plot coordinates
}

/////////////////////////////////////////////////////////////////////////////
// CPlotF2F1 message handlers

/***************************************************************************/
// CPlotF2F1::OnCreate Window creation
// The plot has to get the area process index, because it is a area process
// plot.
/***************************************************************************/
int CPlotF2F1::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CPlotWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }
    return 0;
}

/***************************************************************************/
// CPlotF2F1::OnDraw Drawing
// The data to draw is coming from a temporary file, created by the
// formants processing class, which is called to do data processing if
// necessary before painting. The plot itself is painted by the ZGraph
// functions.
/***************************************************************************/
void CPlotF2F1::OnDraw(CDC * pDC, CRect rClient, CRect rClip, CSaView * pView) {
    // Get pointer to application, document, and formant processing object.
    CSaApp * pApp = (CSaApp *)AfxGetApp();
    CSaDoc * pDoc = pView->GetDocument();
    CProcessFormants * pFormants = (CProcessFormants *)pDoc->GetFormants();
    // get pointer to FormantTracker object
    CProcessFormantTracker * pFTFormants = pDoc->GetFormantTracker();
    CFormantParm * pFormantParms = pFormants->GetFormantParms();
    CProcessFragments * pFragments = (CProcessFragments *)pDoc->GetFragments();
    // calculate sample size in bytes
    WORD wSmpSize = (WORD)(pDoc->GetSampleSize());

    bool bDynamicUpdate = (pView->GetGraphUpdateMode() == DYNAMIC_UPDATE);
    BOOL bDataValid = TRUE;

    // Get the formants.
    short int nResult = ProcessFormants();
    nResult = CheckResult(nResult, pFormants);  // check the process result
    switch (nResult) {
    case PROCESS_CANCELED:
        bDataValid = FALSE;
        break;
    case PROCESS_DATA_OVERLOAD: {
        int nWaveGraphIndex = pView->GetGraphIndexForIDD(IDD_RAWDATA);
        CGraphWnd * pWaveGraph = pView->GetGraph(nWaveGraphIndex);
        if (pWaveGraph) {
            CPlotWnd * pWavePlot = pWaveGraph->GetPlot();
            if ((bDynamicUpdate) && (pWavePlot!=NULL)) {
                // turn off highlighted area in raw data
                pWavePlot->SetHighLightArea(0, 0);
                pWavePlot->UpdateWindow();
            }
        }
    }
    bDataValid = FALSE;
    break;
    case PROCESS_ERROR:
    case PROCESS_NO_DATA:
    case PROCESS_UNVOICED:
        bDataValid = FALSE;
        break;
    case PROCESS_DONE:
        //  Invalidate();
        //  return;
        break;
    }

    SGraph formantChartParm;
    memset(&formantChartParm, zDEFAULT, sizeof(SGraph));

    // Allocate buffers for x and y coordinates;
    DWORD dwFormantFrameCount = pFormants->GetFormantFrameCount();
    DWORD startCursorPos = pView->GetStartCursorPosition();
    DWORD stopCursorPos = pView->GetStopCursorPosition();
    DWORD startFrag = pFragments->GetFragmentIndex(startCursorPos / wSmpSize);
    DWORD stopFrag = pFragments->GetFragmentIndex(stopCursorPos / wSmpSize);
    double sizeFactorFT2Raw = (double)pFTFormants->GetDataSize() / (double)pDoc->GetDataSize() / (sizeof(SFormantFreq) / 2);
    double sizeFactorFT2Frag = sizeFactorFT2Raw * (stopCursorPos - startCursorPos) / (stopFrag - startFrag);
    DWORD ftIndex = (DWORD)((double)startCursorPos * sizeFactorFT2Raw);
    SFormantFreq * formant = pFTFormants->GetFormant(ftIndex);

    double meanF1 = 0;
    double meanF2 = 0;

    if (formant==NULL) {
        bDataValid = FALSE;
    }

    if (bDataValid) {
        DWORD dwMaxNumberPoints = dwFormantFrameCount;
        TRACE("dwMaxNumberPoints=%d\n",dwMaxNumberPoints);

        BOOL bCluster = (dwFormantFrameCount > 1);
        if (bCluster) {
            dwMaxNumberPoints++;
        }
        if (dwMaxNumberPoints > zNUM_POINTS) {
            dwMaxNumberPoints = zNUM_POINTS;    // limit number to buffer sizes
        }
        formantChartParm.xdata = (double *)malloc(dwMaxNumberPoints * sizeof(double));
        if (formantChartParm.xdata==NULL) {
            // memory allocation error
            pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
            GetParent()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
            return;
        }
        formantChartParm.ydata = (double *)malloc(dwMaxNumberPoints * sizeof(double));
        if (formantChartParm.ydata==NULL) {
            // memory allocation error
            free(formantChartParm.xdata);
            pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
            GetParent()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
            return;
        }
        formantChartParm.zdata = NULL;

        // Collect formant frequency pairs for all frames.
        DWORD ix = 0, iy = 0;
        for (DWORD dwFormantFrame = 0; dwFormantFrame < dwFormantFrameCount; dwFormantFrame++) {
            DWORD dwFTFormantFrame = (DWORD)((double)dwFormantFrame * sizeFactorFT2Frag);
            if (dwFormantFrame != 0) {
                formant = pFTFormants->GetFormant(ftIndex + dwFTFormantFrame);
            }
            if ((formant->F[2] != (float)UNDEFINED_DATA) &&
                    (formant->F[1] != (float)UNDEFINED_DATA)) {
                meanF1 = (meanF1 * ix + formant->F[1]) / (ix + 1);
                meanF2 = (meanF2 * iy + formant->F[2]) / (iy + 1);
                if (pFormantParms->bMelScale) {
                    formantChartParm.xdata[ix++] = Scale::ToMels((double)formant->F[2]);
                    formantChartParm.ydata[iy++] = Scale::ToMels((double)formant->F[1]);
                } else {
                    formantChartParm.xdata[ix++] = (double)formant->F[2];
                    formantChartParm.ydata[iy++] = (double)formant->F[1];
                }
            }
        }

        // Get mean if there is a tracking cluster.
        if (bCluster) {
            if (pFormantParms->bMelScale) {
                formantChartParm.xdata[ix++] = Scale::ToMels(meanF2);
                formantChartParm.ydata[iy++] = Scale::ToMels(meanF1);
            } else {
                formantChartParm.xdata[ix++] = meanF2;
                formantChartParm.ydata[iy++] = meanF1;
            }
        }

        formantChartParm.n = (int)ix;  // set total number of data points
    }

    // Customize the axes.
    formantChartParm.line_connect = FALSE;
    formantChartParm.x_axis_style = zLINEAR;
    formantChartParm.y_axis_style = zLINEAR;
    formantChartParm.x_axis_divisions = 5;                 // x-axis divisions
    formantChartParm.y_axis_divisions = 6;                 // y-axis divisions
    formantChartParm.item_title_format = zNUMBERS;

    EGender nGender = pDoc->GetGender();
    SRange FormantScale = GetChartRange(2, nGender);
    formantChartParm.xend = pFormantParms->bMelScale ? Scale::ToMels(FormantScale.Low): FormantScale.Low;
    formantChartParm.xstart = pFormantParms->bMelScale ? Scale::ToMels(FormantScale.High): FormantScale.High;
    FormantScale = GetChartRange(1, nGender);
    formantChartParm.yend = pFormantParms->bMelScale ? Scale::ToMels(FormantScale.Low): FormantScale.Low;
    formantChartParm.ystart = pFormantParms->bMelScale ? Scale::ToMels(FormantScale.High): FormantScale.High;
    formantChartParm.zstart = 0, formantChartParm.zend = 0;

    CSaString szHTitle;
    CSaString szVTitle;

    szHTitle = pFormantParms->bMelScale ? "F2(mels)" : "F2 (Hz)";
    szVTitle = pFormantParms->bMelScale ? "F1 (mels)" : "F1 (Hz)";

    formantChartParm.H_Title = const_cast<LPTSTR>((LPCTSTR)szHTitle);
    formantChartParm.V_Title = const_cast<LPTSTR>((LPCTSTR)szVTitle);

    // Draw the chart.
    BOOL bFlipChart = FALSE;
    BOOL bShowAxes = TRUE;
    StandardPlot(pView, pDC, rClient, rClip, formantChartParm, bFlipChart, bShowAxes, bDataValid);

    if (bDataValid) {
        // Release data point and title buffers.
        free(formantChartParm.xdata);
        free(formantChartParm.ydata);
    }
}


// +++++++++++++++++++++++++++++++++++ END F2 F1 PLOT  +++++++++++++++++++++++++++++++ tdg


//###########################################################################
// CPlotF2F1F1
// Plot window for displaying formants data. This class only handles
// the special drawing of the data. All other work is done in the base class.
// For the drawing, this class holds three buffers (each 1kByte) for the x/y
// data to deliver to the Z-graph functions.

IMPLEMENT_DYNCREATE(CPlotF2F1F1, CFormantChart)

/////////////////////////////////////////////////////////////////////////////
// CPlotF2F1F1 message map

BEGIN_MESSAGE_MAP(CPlotF2F1F1, CPlotWnd)
    ON_WM_CREATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotF2F1F1 construction/destruction/creation

/***************************************************************************/
// CPlotF2F1F1::CPlotF2F1F1 Constructor
/***************************************************************************/
CPlotF2F1F1::CPlotF2F1F1() {
}

/***************************************************************************/
// CPlotF2F1F1::~CPlotF2F1F1 Destructor
/***************************************************************************/
CPlotF2F1F1::~CPlotF2F1F1() {
}

/***************************************************************************/
// CPlotF2F1F1::MapVowelsToChart  Map vowels to chart coordinates
/***************************************************************************/
POINT CPlotF2F1F1::MapVowelsToChart(CDC * /* pDC */, SFormantFreq & Formant, int /* SymbolHeight */, CScatterGraph * pFormantChart,
                                    SGraph & /* ChartParms */, BOOL bFlip) {
    return GetXYCoordinates(pFormantChart, Formant.F[2] - Formant.F[1], Formant.F[1], bFlip); // converts 2D point to plot coordinates
}

/////////////////////////////////////////////////////////////////////////////
// CPlotF2F1F1 message handlers

/***************************************************************************/
// CPlotF2F1F1::OnCreate Window creation
// The plot has to get the area process index, because it is a area process
// plot.
/***************************************************************************/
int CPlotF2F1F1::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CPlotWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }
    return 0;
}

/***************************************************************************/
// CPlotF2F1F1::OnDraw Drawing
// The data to draw is coming from a temporary file, created by the
// formants processing class, which is called to do data processing if
// necessary before painting. The plot itself is painted by the ZGraph
// functions.
/***************************************************************************/
void CPlotF2F1F1::OnDraw(CDC * pDC, CRect rClient, CRect rClip, CSaView * pView) {
    // Get pointer to application, document, and formant processing object.
    CSaApp * pApp = (CSaApp *)AfxGetApp();
    CSaDoc * pDoc = pView->GetDocument();
    CProcessFormants * pFormants = (CProcessFormants *)pDoc->GetFormants();
    // get pointer to spectrogram object
    CProcessFormantTracker * pFTFormants = pDoc->GetFormantTracker();
    CFormantParm * pFormantParms = pFormants->GetFormantParms();
    CProcessFragments * pFragments = (CProcessFragments *)pDoc->GetFragments();
    // calculate sample size in bytes
    WORD wSmpSize = (WORD)(pDoc->GetSampleSize());
    bool bDynamicUpdate = (pView->GetGraphUpdateMode() == DYNAMIC_UPDATE);
    BOOL bDataValid = TRUE;

    // Get the formants.
    short int nResult = ProcessFormants();
    nResult = CheckResult(nResult, pFormants); // check the process result
    switch (nResult) {
    case PROCESS_CANCELED:
        bDataValid = FALSE;
        break;
    case PROCESS_DATA_OVERLOAD: {
        int nWaveGraphIndex = pView->GetGraphIndexForIDD(IDD_RAWDATA);
        CGraphWnd * pWaveGraph = pView->GetGraph(nWaveGraphIndex);
        if (pWaveGraph) {
            CPlotWnd * pWavePlot = pWaveGraph->GetPlot();
            if ((bDynamicUpdate) && (pWavePlot!=NULL)) {
                // turn off highlighted area in raw data
                pWavePlot->SetHighLightArea(0, 0);
                pWavePlot->UpdateWindow();
            }
        }
    }
    bDataValid = FALSE;
    break;
    case PROCESS_ERROR:
    case PROCESS_NO_DATA:
    case PROCESS_UNVOICED:
        bDataValid = FALSE;
        break;
    case PROCESS_DONE:
        //  Invalidate();
        //  return;
        break;
    }

    SGraph FormantChartParm;
    memset(&FormantChartParm, zDEFAULT, sizeof(FormantChartParm));

    // Allocate buffers for x and y coordinates;
    DWORD dwFormantFrameCount = pFormants->GetFormantFrameCount();
    DWORD startCursorPos = pView->GetStartCursorPosition();
    DWORD stopCursorPos = pView->GetStopCursorPosition();
    DWORD startFrag = pFragments->GetFragmentIndex(startCursorPos / wSmpSize);
    DWORD stopFrag = pFragments->GetFragmentIndex(stopCursorPos / wSmpSize);
    double sizeFactorFT2Raw = (double)pFTFormants->GetDataSize() / (double)pDoc->GetDataSize() / (sizeof(SFormantFreq) / 2);
    double sizeFactorFT2Frag = sizeFactorFT2Raw * (stopCursorPos - startCursorPos) / (stopFrag - startFrag);
    DWORD ftIndex = (DWORD)((double)startCursorPos * sizeFactorFT2Raw);
    SFormantFreq * formants = pFTFormants->GetFormant(ftIndex);

    double meanF1 = 0;
    double meanF2 = 0;

    if (formants==NULL) {
        bDataValid = FALSE;
    }

    if (bDataValid) {
        DWORD dwMaxNumberPoints = dwFormantFrameCount;
        BOOL bCluster = (dwFormantFrameCount > 1);
        if (bCluster) {
            dwMaxNumberPoints++;
        }
        if (dwMaxNumberPoints > zNUM_POINTS) {
            dwMaxNumberPoints = zNUM_POINTS;    // limit number to buffer sizes
        }
        FormantChartParm.xdata = (double *)malloc(dwMaxNumberPoints * sizeof(double));
        if (!FormantChartParm.xdata) {
            // memory allocation error
            pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
            GetParent()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
            return;
        }
        FormantChartParm.ydata = (double *)malloc(dwMaxNumberPoints * sizeof(double));
        if (!FormantChartParm.ydata) {
            // memory allocation error
            free(FormantChartParm.xdata);
            pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
            GetParent()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
            return;
        }
        FormantChartParm.zdata = NULL;

        // Collect formant frequency pairs for all frames.
        DWORD ix = 0, iy = 0;
        for (DWORD dwFormantFrame = 0; dwFormantFrame < dwFormantFrameCount; dwFormantFrame++) {
            DWORD dwFTFormantFrame = (DWORD)((double)dwFormantFrame * sizeFactorFT2Frag);
            if (dwFormantFrame != 0) {
                formants = pFTFormants->GetFormant(ftIndex + dwFTFormantFrame);
            }
            if ((formants->F[1] != (float)UNDEFINED_DATA) &&
                    (formants->F[2] != (float)UNDEFINED_DATA)) {
                meanF1 = (meanF1 * ix + formants->F[1]) / (ix + 1);
                meanF2 = (meanF2 * iy + formants->F[2]) / (iy + 1);
                if (pFormantParms->bMelScale) {
                    FormantChartParm.xdata[ix++] = Scale::ToMels((double)formants->F[2]) - Scale::ToMels((double)formants->F[1]);
                    FormantChartParm.ydata[iy++] = Scale::ToMels((double)formants->F[1]);
                } else {
                    FormantChartParm.xdata[ix++] = (double)formants->F[2] - (double)formants->F[1];
                    FormantChartParm.ydata[iy++] = (double)formants->F[1];
                }
            }
        }

        // Get mean if there is a tracking cluster.
        if (bCluster) {
            if (pFormantParms->bMelScale) {
                FormantChartParm.xdata[ix++] = Scale::ToMels(meanF2) - Scale::ToMels(meanF1);
                FormantChartParm.ydata[iy++] = Scale::ToMels(meanF1);
            } else {
                FormantChartParm.xdata[ix++] = meanF2 - meanF1;
                FormantChartParm.ydata[iy++] = meanF1;
            }
        }

        FormantChartParm.n = (int)ix;  // set total number of data points
    }

    // Customize the chart.
    EGender nGender = pDoc->GetGender();
    switch (nGender) { // set the frequency ranges
    case male:  // male
        FormantChartParm.xend   = 0  , FormantChartParm.xstart = 2500;
        FormantChartParm.yend   = 200, FormantChartParm.ystart = 1000;
        break;
    case female:  // female
        FormantChartParm.xend   = 0,   FormantChartParm.xstart = 3000;
        FormantChartParm.yend   = 200, FormantChartParm.ystart = 1000;
        break;
    case child:  // child
        FormantChartParm.xend   = 0,   FormantChartParm.xstart = 3000;
        FormantChartParm.yend   = 350, FormantChartParm.ystart = 1100;
    }
    if (pFormantParms->bMelScale) {
        FormantChartParm.xstart = Scale::ToMels(FormantChartParm.xstart);
        FormantChartParm.xend = Scale::ToMels(FormantChartParm.xend);
        FormantChartParm.ystart = Scale::ToMels(FormantChartParm.ystart);
        FormantChartParm.yend = Scale::ToMels(FormantChartParm.yend);
    }
    FormantChartParm.zstart = 0, FormantChartParm.zend = 0;
    FormantChartParm.line_connect = FALSE;           ;
    FormantChartParm.x_axis_style = zLINEAR;
    FormantChartParm.y_axis_style = zLINEAR;
    FormantChartParm.x_axis_divisions = 6;                 // x-axis divisions
    FormantChartParm.y_axis_divisions = 5;                 // y-axis divisions
    //  FormantChartParm.item_title_format = zNO_ITEM_TITLES;  // no numbers along axes
    FormantChartParm.item_title_format = zNUMBERS;

    CSaString szHTitle;
    CSaString szVTitle;

    szHTitle = "[F2-F1]";
    szVTitle = pFormantParms->bMelScale ? "F1 (mels)" : "F1 (Hz)";

    FormantChartParm.H_Title = const_cast<LPTSTR>((LPCTSTR)szHTitle);
    FormantChartParm.V_Title = const_cast<LPTSTR>((LPCTSTR)szVTitle);

    // Draw the chart.
    BOOL bFlipChart = FALSE;
    BOOL bShowAxes = TRUE;
    StandardPlot(pView, pDC, rClient, rClip, FormantChartParm, bFlipChart, bShowAxes, bDataValid);

    if (bDataValid) {
        // Release data point and title buffers.
        free(FormantChartParm.xdata);
        free(FormantChartParm.ydata);
    }
}


//###########################################################################
// CPlotInvSDP
// Plot window for displaying formants data. This class only handles
// the special drawing of the data. All other work is done in the base class.
// For the drawing, this class holds two buffers (each 1kByte) for the polar
// data to deliver to the Z-graph functions.

IMPLEMENT_DYNCREATE(CPlotInvSDP, CPlotWnd)

/////////////////////////////////////////////////////////////////////////////
// CPlotInvSDP message map

BEGIN_MESSAGE_MAP(CPlotInvSDP, CPlotWnd)
    ON_WM_CREATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotInvSDP construction/destruction/creation

/***************************************************************************/
// CPlotInvSDP::CPlotInvSDP Constructor
/***************************************************************************/
CPlotInvSDP::CPlotInvSDP() {
    m_hDataX = NULL;
    m_hDataY = NULL;
    m_hDataSym = NULL;
}

/***************************************************************************/
// CPlotInvSDP::~CPlotInvSDP Destructor
/***************************************************************************/
CPlotInvSDP::~CPlotInvSDP() {
    // free the buffer memory
    if (m_hDataX) {
        ::GlobalUnlock(m_hDataX);
        ::GlobalFree(m_hDataX);
    }
    if (m_hDataY) {
        ::GlobalUnlock(m_hDataY);
        ::GlobalFree(m_hDataY);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CPlotInvSDP message handlers

/***************************************************************************/
// CPlotInvSDP::OnCreate Window creation
// The plot has to get the area process index, because it is a area process
// plot.
/***************************************************************************/
int CPlotInvSDP::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CPlotWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }
    // get pointer to document
    CSaView * pView = (CSaView *)GetParent()->GetParent();
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
    // process workbench if necessary
    pDoc->WorkbenchProcess();
    // get pointer to process class
    CProcessSDP * pSDP = GetSDP(pDoc); // get pointer to SDP object
    // save process
    m_pAreaProcess = pSDP;
    return 0;
}

CProcessSDP * CPlotInvSDP::GetSDP(CSaDoc * pDoc) {
    return pDoc->GetSDP(2); // InvSDP has index == 2
}

/***************************************************************************/
// CPlotInvSDP::OnDraw Drawing
// The data to paint is raw data, only lightly processed directly before
// painting. The process called, will do anything than just save the area
// data for the plot and set the right flags in the process status. The plot
// itself is painted by the ZGraph functions. Because the regular polar graph
// does not support to draw just a dot, and we need that, the zgraph drawing
// function has been changed to be able to draw just a dot.
/***************************************************************************/
void CPlotInvSDP::OnDraw(CDC * pDC, CRect rClient, CRect rClip, CSaView * pView) {
#define zTWOPI         6.28318530718
#define zNUM_SAMPLES   512
#define zMAX_NUMPANES  8
#define zMAX_NUMPOINTS zNUM_SAMPLES * 2 * zMAX_NUMPANES

    CSaDoc * pDoc = pView->GetDocument();

    if (IsIconic()) {
        return;    // nothing to draw
    }
    // get pointer to view and document
    // process workbench if necessary
    pDoc->WorkbenchProcess();
    // get the SDP parameters
    SDPParm * pSDPParm = pDoc->GetSDPParm();
    long lSteps = (long)pSDPParm->nSteps;
    BOOL bAuto = (pSDPParm->nStepMode == 0);
    BOOL bAverage = pSDPParm->bAverage;
    // calculate the normfactor
    double dfNormRadFactor = zTWOPI / 360.0 * (double)pSDPParm->nUpperBound;
    // allocate the three data buffers
    CSaApp * pApp = (CSaApp *)AfxGetApp();
    if (!m_hDataX) { // not yet allocated
        m_hDataX = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (DWORD)zNUM_SAMPLES * sizeof(double)); // allocate memory
        if (!m_hDataX) {
            // memory allocation error
            pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
            GetParent()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
            return;
        }
        m_lpDataX = (HPSTR)::GlobalLock(m_hDataX); // lock memory
        if (!m_lpDataX) {
            // memory lock error
            pApp->ErrorMessage(IDS_ERROR_MEMLOCK);
            ::GlobalFree(m_hDataX);
            m_hDataX = NULL;
            GetParent()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
            return;
        }
    }
    if (!m_hDataY) { // not yet allocated
        m_hDataY = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (DWORD)zNUM_SAMPLES * sizeof(double)); // allocate memory
        if (!m_hDataY) {
            // memory allocation error
            pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
            GetParent()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
            return;
        }
        m_lpDataY = (HPSTR)::GlobalLock(m_hDataY); // lock memory
        if (!m_lpDataY) {
            // memory lock error
            pApp->ErrorMessage(IDS_ERROR_MEMLOCK);
            ::GlobalFree(m_hDataY);
            m_hDataY = NULL;
            GetParent()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
            return;
        }
    }
    if (!m_hDataSym) { // not yet allocated
        m_hDataSym = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (DWORD)zNUM_SAMPLES * sizeof(short int)); // allocate memory
        if (!m_hDataSym) {
            // memory allocation error
            pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
            GetParent()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
            return;
        }
        m_lpDataSym = (HPSTR)::GlobalLock(m_hDataSym); // lock memory
        if (!m_lpDataSym) {
            // memory lock error
            pApp->ErrorMessage(IDS_ERROR_MEMLOCK);
            ::GlobalFree(m_hDataSym);
            m_hDataSym = NULL;
            GetParent()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
            return;
        }
    }
    // get pointer to process class
    CProcessSDP * pSDP = GetSDP(pDoc); // get pointer to SDP object
    // check if process is idle
    if (pSDP->IsIdle()) {
        m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_POSCUR, &rClient);
    } else {
        m_HelperWnd.SetMode(MODE_HIDDEN); // hide helper window
        int nMaxValue = 1;
        int nMinValue = 0;
        BOOL bRadiusValid = TRUE;
        DWORD dwStart = pSDP->GetAreaPosition();
        DWORD dwNumber = pSDP->GetAreaLength();
        if ((dwNumber - 1) > zNUM_SAMPLES) {
            dwNumber = zNUM_SAMPLES + 1;    // limit number to buffer sizes
        }
        // get a handle to windows for Z-graph
        HDC hdc = pDC->GetSafeHdc();
        // set up an array of colors to be used in the Z-graph
        zRGB PlotColorArray[zMAX_DATA_SETS + 1];
        // get pointer to color structure from main frame
        CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
        Colors * pColor = pMainWnd->GetColors();
        for (int nLoop = 0; nLoop <= zMAX_DATA_SETS; nLoop++) {
            PlotColorArray[nLoop] = pColor->cPlotData[0];
        }
        // cast the pointers to the data buffers
        double * pDataX = (double *)m_lpDataX; // pointer to X-axis data
        double * pDataY = (double *)m_lpDataY; // pointer to X-axis data
        short int * pDataSym = (short int *)m_lpDataSym; // pointer to symbol data

        pSDP->SetArea(pView);
        if (TRUE) {
            // new data has to be processed
            dwStart = pSDP->GetAreaPosition();
            UINT nBlockAlign = pDoc->GetBlockAlign();
            if (bAuto) {
                // auto step mode, determine steps
                lSteps = (long)(pSDP->GetAreaLength() / (zNUM_SAMPLES * (DWORD)nBlockAlign));
                if (!lSteps) {
                    lSteps = 1;
                }
            }
            dwNumber = (pSDP->GetAreaLength() - 1) / (DWORD)lSteps + 1;
            if ((dwNumber - 1) > zNUM_SAMPLES) {
                dwNumber = zNUM_SAMPLES + 1;    // limit number to buffer sizes
            }
            if (dwNumber <= 2) {
                return;
            }
            DWORD dwSmplSteps = (DWORD)lSteps * (DWORD)nBlockAlign;
            DWORD dwRawBufferSize = pDoc->GetWaveDataBufferSize();
            // fill up data buffers
            HPSTR pData = pDoc->GetWaveData(dwStart, TRUE); // get pointer to wave data block
            if (pData == NULL) {
                return;    // error occured while reading data block
            }
            DWORD dwDataPos = dwStart;
            dwStart += dwRawBufferSize;
            int nLastData = 0, nData;
            nMaxValue = 0;
            nMinValue = 0x7FFF;
            bRadiusValid = FALSE;
            // first find min and max values
            for (DWORD dwLoop = 0; dwLoop < dwNumber; dwLoop++) {
                // read data
                if (bAverage) {
                    // build average inside step
                    long lAverageData = 0;
                    for (DWORD dwLoop = 0; dwLoop < (DWORD)lSteps; dwLoop++) {
                        if (dwDataPos >= dwStart) {
                            // reload data
                            pData = pDoc->GetWaveData(dwDataPos, TRUE); // get new data block
                            if (pData == NULL) {
                                return;    // error occured while reading data block
                            }
                            dwStart = dwDataPos + dwRawBufferSize;
                        }
                        if (nBlockAlign == 1) { // 8 bit data
                            BYTE bData = *pData++; // data range is 0...255 (128 is center)
                            nData = bData - 128;
                        } else {              // 16 bit data
                            nData = *((short int *)pData++);
                            pData++;
                            dwDataPos++;
                        }
                        dwDataPos++;
                        lAverageData += (long)nData; // build data sum
                    }
                    nData = (int)(lAverageData / lSteps);
                } else {
                    // take just one sample
                    if (dwDataPos >= dwStart) {
                        // reload data
                        pData = pDoc->GetWaveData(dwDataPos, TRUE); // get new data block
                        if (pData == NULL) {
                            return;    // error occured while reading data block
                        }
                        dwStart = dwDataPos + dwRawBufferSize;
                    }
                    if (nBlockAlign == 1) { // 8 bit data
                        BYTE bData = *pData; // data range is 0...255 (128 is center)
                        nData = bData - 128;
                    } else {
                        nData = *((short int *)pData);    // 16 bit data
                    }
                    pData += dwSmplSteps;
                    dwDataPos += dwSmplSteps;
                }
                if (!dwLoop) {
                    nLastData = nData;    // save first data
                }
                // find min and max values
                if (nData > nMaxValue) {
                    nMaxValue = nData;    // store higher value
                }
                if (nData < nMinValue) {
                    nMinValue = nData;    // store lower value
                }
            }
            // reset data pointers
            dwStart = pSDP->GetAreaPosition();
            pData = pDoc->GetWaveData(dwStart, TRUE); // get first wave data block
            if (pData == NULL) {
                return;    // error occured while reading data block
            }
            dwDataPos = dwStart;
            dwStart += dwRawBufferSize;
            // now process the data and fill up the buffers
            double dfHminL = 0.0001;
            if (nMaxValue != nMinValue) {
                dfHminL = (double)nMaxValue - (double)nMinValue;
            }
            pData += dwSmplSteps;
            dwDataPos += dwSmplSteps;
            for (DWORD dwLoop = 1; dwLoop < dwNumber; dwLoop++) {
                // read data
                if (bAverage) {
                    // build average inside step
                    long lAverageData = 0;
                    for (DWORD dwLoop = 0; dwLoop < (DWORD)lSteps; dwLoop++) {
                        if (dwDataPos >= dwStart) {
                            // reload data
                            pData = pDoc->GetWaveData(dwDataPos, TRUE); // get new data block
                            if (pData == NULL) {
                                return;    // error occured while reading data block
                            }
                            dwStart = dwDataPos + dwRawBufferSize;
                        }
                        if (nBlockAlign == 1) { // 8 bit data
                            BYTE bData = *pData++; // data range is 0...255 (128 is center)
                            nData = bData - 128;
                        } else {              // 16 bit data
                            nData = *((short int *)pData++);
                            pData++;
                            dwDataPos++;
                        }
                        dwDataPos++;
                        lAverageData += (long)nData; // build data sum
                    }
                    nData = (int)(lAverageData / lSteps);
                } else {
                    // take just one sample
                    if (dwDataPos >= dwStart) {
                        // reload data
                        pData = pDoc->GetWaveData(dwDataPos, TRUE); // get new data block
                        if (pData == NULL) {
                            return;    // error occured while reading data block
                        }
                        dwStart = dwDataPos + dwRawBufferSize;
                    }
                    if (nBlockAlign == 1) { // 8 bit data
                        BYTE bData = *pData; // data range is 0...255 (128 is center)
                        nData = bData - 128;
                    } else {
                        nData = *((short int *)pData);    // 16 bit data
                    }
                    pData += dwSmplSteps;
                    dwDataPos += dwSmplSteps;
                }
                // calculate radius
                double dfRadius = ((double)nLastData - (double)nMinValue) / dfHminL * (double)pSDPParm->nUpperBound; // radius
                if (dfRadius) {
                    bRadiusValid = TRUE;
                }
                // calculate angle
                double dfTheta = ((double)nData - (double)nMinValue) / dfHminL * dfNormRadFactor; // radius
                *pDataY++ = dfRadius * sin(dfTheta);
                *pDataX++ = dfRadius * cos(dfTheta);
                *pDataSym++ = zDOT;
                nLastData = nData;
            }
        }
        // set up Z-graph structure
        SGraph zG;
        // fill In the [SGraph] structure elements
        zG.hdc = hdc;                   // windows HDC
        zG.RWindow = &rClient;          // RECT with window dimensions
        zG.xdata = (double *)m_lpDataX; // x-data array, radius
        zG.ydata = (double *)m_lpDataY; // y-data array, angle (radian)
        zG.zdata = NULL;                // z-data array
        zG.symbols = (int *)m_lpDataSym; // symbols array
        if (((nMinValue == 0) && (nMaxValue == 0)) || !bRadiusValid) {
            zG.n = 0;    // nothing to display
        } else {
            zG.n = (int)(dwNumber - 1);    // total number of data points
        }
        zG.axis_color = pColor->cPlotGrid; // graph axis color
        zG.bkg_color = pColor->cPlotBkg; // graph bkg. color
        zG.plot_color = PlotColorArray; // array of plot colors
        zG.fill_style = 0;
        zG.title = _T("\0");
        zG.H_Title = _T("");            // horizontal axis title
        zG.V_Title = _T("Inverse SDP"); // vertical axis title
        zG.legend_position = zNO_LEGEND;
        zG.item_title_format = zNUMBERS; // show numbers along axes
        zG.line_connect = OFF;
        zG.symbols_flag = OFF;
        zG.grid_flag = HaveGrid();      // grid on/off for graph
        zG.auto_scale = OFF;            // use manuel scaling
        zG.draw_forward = TRUE;
        zG._3D_flag = FALSE;
        zG.bkg_gradient_mode = zNO_GRADIENT;
        zG.x_axis_style = zLINEAR;
        zG.y_axis_style = zLINEAR;
        zG.x_axis_divisions = zBESTFIT_DIVISIONS; // x-axis divisions
        zG.y_axis_divisions = zBESTFIT_DIVISIONS; // y-axis divisions
        zG.digit_format = zFIXED;       // round axis numbers to nearest whole integer
        zG.digit_precision = 1;
        zG.grid_pen_style = zDOTTED;    // use dotted lines for grid
        zG.shading_style = zNO_SHADING;
        zG.xstart = 0, zG.xend = pSDPParm->nUpperBound;
        zG.ystart = 0, zG.yend = pSDPParm->nUpperBound; // set the x, y ranges
        zG.zstart = 0,   zG.zend = 0;
        zG.quality = zG3D_QUALITY_HI;
        zG.function = NULL;
        // draw the graph
        ScatterGraph(&zG);
    }
    // finish the plot
    PlotPaintFinish(pDC, rClient, rClip);
}
