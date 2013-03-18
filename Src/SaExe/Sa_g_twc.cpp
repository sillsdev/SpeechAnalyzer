/////////////////////////////////////////////////////////////////////////////
// sa_g_twc.cpp:
// Implementation of the CPlotTonalWeightChart class.
// Author: Todd Jones
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//  07/20/2000
//         RLJ Added support for Up (/\) and Down (\/) buttons in TWC legend
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_plot.h"
#include "sa_g_twc.h"
#include "sa_g_mel.h"
#include "Process\Process.h"
#include "Process\sa_p_twc.h"
#include "Process\sa_p_mel.h"
#include "sa_graph.h"

#include "sa_doc.h"
#include "sa_view.h"
#include "mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CPlotTonalWeightChart
// Plot window for displaying spectrum data. This class only handles the
// special drawing of the data. All other work is done in the base class.

IMPLEMENT_DYNCREATE(CPlotTonalWeightChart, CPlotWnd)

/////////////////////////////////////////////////////////////////////////////
// CPlotTonalWeightChart message map

BEGIN_MESSAGE_MAP(CPlotTonalWeightChart, CPlotWnd)
    //{{AFX_MSG_MAP(CPlotTonalWeightChart)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotTonalWeightChart construction/destruction/creation

/***************************************************************************/
// CPlotTonalWeightChart::CPlotTonalWeightChart Constructor
/***************************************************************************/
CPlotTonalWeightChart::CPlotTonalWeightChart() {
    m_bSemitoneOffsetAdjusted = FALSE; // m_SemitoneOffset has not yet been adjusted
    m_dSemitoneOffset = 0.0f;
    SetHorizontalCursors();
}

/***************************************************************************/
// CPlotTonalWeightChart::~CPlotTonalWeightChart Destructor
/***************************************************************************/
CPlotTonalWeightChart::~CPlotTonalWeightChart() {
}

/////////////////////////////////////////////////////////////////////////////
// CPlotTonalWeightChart message handlers

Grid CPlotTonalWeightChart::GetGrid() const {
    Grid modifiedGrid(*reinterpret_cast<CMainFrame *>(AfxGetMainWnd())->GetGrid());

    // nPenStyle = PS_DASHDOT;
    modifiedGrid.nYStyle = 4; // Dashdot

    return modifiedGrid;
}

/***************************************************************************/
// CPlotTonalWeightChart::OnDraw Drawing
// The data to paint is coming from a temporary file, created by the TWC
// processing class, which is called to do data processing if necessary
// before painting. Call the function PlotPaintFinish at the end of the
// painting to let the plot base class do common jobs like painting the
// cursors.
/***************************************************************************/
void CPlotTonalWeightChart::OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView) {
    TRACE(_T("Draw TWC\n"));
    // if nothing to draw just return
    if (!rClip.bottom && !rClip.right) {
        return;
    }
    rClip.top = 0; // drawing the entire vertical plot gets the correct maximum x value
    // get pointer to graph, view and document
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaDoc  *  pDoc   = pView->GetDocument();

    CProcessTonalWeightChart * pTonalWeightChart = (CProcessTonalWeightChart *)pDoc->GetTonalWeightChart(); // get pointer to TWC object

    double dMaxSemitone;
    double dMinSemitone;

    if (!CPlotMelogram::GetScaleValues(pDoc, &dMaxSemitone,&dMinSemitone)) {
        short nMelGraphIndex = (short)pView->GetGraphIndexForIDD(IDD_MELOGRAM);
        CGraphWnd * pMelGraph = pView->GetGraph(nMelGraphIndex);
        pMelGraph->UpdateWindow();
        pMelGraph->GetPlot()->UpdateWindow();
        CLegendWnd * pMelLegend = pMelGraph->GetLegendWnd();
        dMinSemitone = pMelLegend->GetScaleMinValue();
        dMaxSemitone = pMelLegend->GetScaleMaxValue();
    }

    FmtParm * pFmtParm = pDoc->GetFmtParm(); // get sa parameters format member data
    UINT nBlockAlign = pFmtParm->wBlockAlign;
    WORD wSmpSize = WORD(nBlockAlign / pFmtParm->wChannels);

    CProcessMelogram * pMelogram = (CProcessMelogram *)pDoc->GetMelogram(); // get pointer to melogram object
    DWORD dwMelDataSize = pMelogram->GetDataSize() * 2; // size of melogram data
    DWORD dwRawDataSize = pDoc->GetDataSize(); // size of raw data
    double fInvScaleFactor = (double)dwMelDataSize / (double)dwRawDataSize;
    DWORD dwFrameStart = (DWORD)((double)pView->GetStartCursorPosition() * fInvScaleFactor) & ~1; // must be multiple of two
    DWORD dwFrameSize  = ((DWORD)((double)pView->GetStopCursorPosition() * fInvScaleFactor) & ~1) - dwFrameStart + wSmpSize;
    if (pView->GetStaticTWC()) {
        dwFrameStart = 0;
        dwFrameSize  = dwMelDataSize;
    }

    short nResult = LOWORD(pTonalWeightChart->Process(this, pDoc, dwFrameStart, dwFrameSize, (short) dMinSemitone, (short) dMaxSemitone)); // process data
    nResult = CheckResult(nResult, pTonalWeightChart); // check the process result

    if (nResult == PROCESS_ERROR) {
        return;
    }

    else if (nResult == PROCESS_NO_DATA) {
        m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_NOPITCH, &rWnd);
    }

    else if (nResult != PROCESS_CANCELED) {
        m_HelperWnd.SetMode(MODE_HIDDEN);
        if (nResult && !pDC->IsPrinting()) {
            // new data processed, all has to be displayed
            Invalidate();
            UpdateWindow();
            return;
        }

        DWORD dwDataSize = pTonalWeightChart->GetDataSize();
        double dMinSemitone = pTonalWeightChart->GetMinSemitone();
        double dMaxSemitone = pTonalWeightChart->GetMaxSemitone();

        if (!m_bSemitoneOffsetAdjusted) {
            short int * pTWCData = (short int *)pTonalWeightChart->GetProcessedData(0, TRUE);
            DWORD dwMaxBin = 0;
            short nMaxBinValue = 0;
            for (DWORD i=0; i<dwDataSize; i++) {
                if (pTWCData[i] > nMaxBinValue) {
                    nMaxBinValue = pTWCData[i];
                    dwMaxBin = i;
                }
            }
            // calibrate semitones
            double dSemitoneOffset = dMinSemitone + double(dwMaxBin /*+ 0.5*/) / int(dwDataSize / (dMaxSemitone - dMinSemitone) + 0.5);

            if (dSemitoneOffset > dMaxSemitone || dSemitoneOffset < dMinSemitone) {
                dSemitoneOffset = floor((dMaxSemitone + dMinSemitone)/2); // Just in case ???
            }

            m_bSemitoneOffsetAdjusted = TRUE;
            SetSemitoneOffset(-dSemitoneOffset);
        }

        double dMin = dMinSemitone + GetSemitoneOffset();
        double dMax = dMaxSemitone + GetSemitoneOffset();

        // set legend scale to the same as melogram so gridlines line up with TWC shifted legend.
        pGraph->SetLegendScale(SCALE | NUMBERS, dMin, dMax, _T("     Semitones")); // set legend scale (" " characters in "  Semitones" are to distance it from Up button

        PlotPrePaint(pDC, rWnd, rClip, NULL, TRUE, TRUE);

        PaintHelper(pDC, rWnd, rClip, pTonalWeightChart, pDoc, SKIP_UNSET); // do standard data paint

    }

    // do common plot paint jobs
    PlotPaintFinish(pDC, rWnd, rClip);
}

/***************************************************************************/
// CPlotTonalWeightChart::double CPlotTonalWeightChart::AdjustSemitoneOffset(WORD UpDown, BOOL bKeyShifted)
// Increment Offset
/***************************************************************************/
void CPlotTonalWeightChart::AdjustSemitoneOffset(WORD UpDown, BOOL bKeyShifted) {
    switch (UpDown) {
    case IDM_UP:
        if (bKeyShifted) { // If SHIFT key was pressed along with UP button
            SetSemitoneOffset(GetSemitoneOffset() - 0.1);
        } else {
            //  Decrement to next lower integer
            SetSemitoneOffset(ceil(GetSemitoneOffset()) - 1.0);
        }
        break; // switch
    case IDM_DOWN:
        if (bKeyShifted) { // If SHIFT key was pressed along with DOWN button
            SetSemitoneOffset(GetSemitoneOffset() + 0.1);
        } else {
            //  Increment to next higher integer
            SetSemitoneOffset(floor(GetSemitoneOffset()) + 1.0);
        }
        break;
    }
}

/***************************************************************************/
// double CPlotTonalWeightChart::SetSemitoneOffset(double dSemitoneOffset)
// Set Offset and make other adjustments as necessary
/***************************************************************************/
double CPlotTonalWeightChart::SetSemitoneOffset(double dSemitoneOffset) {
    double dResult = m_dSemitoneOffset;
    if (dSemitoneOffset != m_dSemitoneOffset) {
        m_dSemitoneOffset = dSemitoneOffset;

        CGraphWnd * pGraph = (CGraphWnd *)GetParent();
        pGraph->RedrawGraph(TRUE, TRUE); // Redraw TWC
        pGraph->UpdateStatusBar(0, 0, TRUE);

        CSaView * pView = static_cast<CSaView *>(pGraph->GetParent());
        short nMelGraphIndex = (short)pView->GetGraphIndexForIDD(IDD_MELOGRAM);
        CGraphWnd * pMelGraph = pView->GetGraph(nMelGraphIndex);
        pMelGraph->RedrawGraph(TRUE, TRUE); // Redraw Melogram
        pMelGraph->UpdateStatusBar(0, 0, TRUE);
    }

    return dResult;
}

void DrawHistogram(CRect & rClip, CDataSource & cData, const CXScale & cXScale, const CYScale & cYScale, CDrawSegment & cDrawSegment) {
    int nFirst = (int) floor(cYScale.GetValue(rClip.bottom)) - 1;
    int nLast = (int) ceil(cYScale.GetValue(rClip.top)) + 1;


    BOOL bValid = TRUE;

    CDataSource::CValues cValues;

    int nSample = nFirst > 0 ? nFirst : 0;

    while (nSample <= nLast) {
        int y = cYScale.GetY(nSample);
        int nSampleGet = nSample;
        int nNext = (int) floor(cYScale.GetValue(y-1));

        // nSample is updated by GetValues
        cData.GetValues(nSample, nNext, cValues, bValid);

        if (cValues.nMax==cValues.nMax) {
            int yBegin = y;
            int yEnd = cYScale.GetY(nSampleGet+1);
            cDrawSegment.DrawTo(cValues.nMax,cXScale, yBegin, bValid);
            cDrawSegment.DrawTo(cValues.nMax,cXScale, yEnd, bValid);
            // Uncomment to draw full histogram bars
            cDrawSegment.DrawTo(0           ,cXScale, yEnd, bValid);
        } else {
            cDrawSegment.DrawTo(cValues,cXScale, y, bValid);
        }

    }
    // Flush drawing functions
    bValid = FALSE;
    cDrawSegment.DrawTo(0, cXScale, 0, bValid);
}

void CPlotTonalWeightChart::PaintHelper(CDC * pDC, CRect rWnd, CRect rClip,
                                        CProcessTonalWeightChart * pProcess, CSaDoc * /*pProcessDoc*/, int /*nFlags*/) {
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
    // Create pXScale
    {
        pXScale = new CXScaleLinear((pProcess->GetMaxValue())/(0.9*rWnd.Width() + 1), 0);
    }

    CYScale * pYScale = NULL;
    // Create YScale
    {
        double dScaleMin = pLegend->GetScaleMinValue() - GetSemitoneOffset() - pProcess->GetMinSemitone();

        // calculate vertical scaling factor
        double fBase = pLegend->GetScaleBase() * pProcess->GetBinsPerSemitone();
        double m_fVScale = pLegend->GetGridDistance() / fBase;
        double dCenterPos = rWnd.bottom + (dScaleMin)*m_fVScale / pProcess->GetBinsPerSemitone();  // x-axis vertical position

        pYScale = new CYScaleLinear(-m_fVScale, dCenterPos);
    }

    CDataSource * pSource = new CDataSourceSimple(*pProcess);

    CDrawSegment * pSegment = NULL;
    {
        // Smooth line drawing
        pSegment = new CDrawSegmentLine(*pDC);
    }

    int nLineThickness = GetPenThickness(); // Sometimes Based on VScale

    CPen penData(PS_SOLID, nLineThickness, pColor->cPlotData[0]);
    CPen * pOldPen = pDC->SelectObject(&penData);

    try {
        pDC->SelectObject(&penData);
        pSegment->SetColor(pColor->cPlotData[0]);

        DrawHistogram(rClip, *pSource, *pXScale, *pYScale, *pSegment);
    } catch (...) {
    }

    pDC->SelectObject(pOldPen);

    // Applying delete to NULL has no effect
    delete pSource;
    delete pXScale;
    delete pYScale;
    delete pSegment;
}