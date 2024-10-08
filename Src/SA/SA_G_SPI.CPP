/////////////////////////////////////////////////////////////////////////////
// sa_g_spi.cpp:
// Implementation of the CPlotSmoothedPitch class.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "sa_plot.h"
#include "sa_g_spi.h"
#include "Process\Process.h"
#include "Process\sa_p_spi.h"
#include "sa_minic.h"
#include "sa_graph.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "LegendWnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CPlotSmoothedPitch
// Plot window for displaying smoothed pitch data. This class only handles
// the special drawing of the data. All other work is done in the base class.

IMPLEMENT_DYNCREATE(CPlotSmoothedPitch, CPlotWnd)

/////////////////////////////////////////////////////////////////////////////
// CPlotSmoothedPitch message map

BEGIN_MESSAGE_MAP(CPlotSmoothedPitch, CPlotWnd)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotSmoothedPitch construction/destruction/creation

/***************************************************************************/
// CPlotSmoothedPitch::CPlotSmoothedPitch Constructor
/***************************************************************************/
CPlotSmoothedPitch::CPlotSmoothedPitch() {
}


CPlotWnd * CPlotSmoothedPitch::NewCopy() {
    CPlotWnd * pRet = new CPlotSmoothedPitch();

    CopyTo(pRet);

    return pRet;
}

/***************************************************************************/
// CPlotSmoothedPitch::~CPlotSmoothedPitch Destructor
/***************************************************************************/
CPlotSmoothedPitch::~CPlotSmoothedPitch() {
}

/////////////////////////////////////////////////////////////////////////////
// CPlotSmoothedPitch message handlers

/***************************************************************************/
// CPlotSmoothedPitch::OnDraw Drawing
// The data to draw is coming from a temporary file, created by the pitch
// processing class, which is called to do data processing if necessary
// before drawing. Call the function PlotdrawFinish at the end of the
// drawing to let the plot base class do common jobs like drawing the
// cursors.
/***************************************************************************/
void CPlotSmoothedPitch::OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView) {
    // get pointer to graph, view and document
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaDoc  *  pDoc   = pView->GetDocument();
    // create smoothed pitch data
    CProcessSmoothedPitch * pPitch = (CProcessSmoothedPitch *)pDoc->GetSmoothedPitch(); // get pointer to smoothed pitch object
    short int nResult = LOWORD(pPitch->Process(this, pDoc)); // process data
    nResult = CheckResult(nResult, pPitch); // check the process result
    if (nResult == PROCESS_ERROR) {
        return;
    }
    if (pPitch->IsStatusFlag(PROCESS_NO_PITCH)) {
        m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_ADJUSTPARMS, &rWnd);
    } else if (pPitch->IsDataReady()) {
        m_HelperWnd.SetMode(MODE_HIDDEN);
        // get pointer to pitch parameters
        const CPitchParm * pPitchParm = pDoc->GetPitchParm();
        // set data range
        int nMinData, nMaxData;
        if (pPitchParm->nRangeMode) {
            // manual range mode
            nMinData = pPitchParm->nLowerBound;
            nMaxData = pPitchParm->nUpperBound;
        } else {
            // auto range mode
            CPitchParm::GetAutoRange(pDoc, nMaxData, nMinData);
        }
        SetProcessMultiplier(PRECISION_MULTIPLIER);
        if (pPitchParm->nScaleMode == 1) {
            // linear display
            pGraph->SetLegendScale(SCALE | NUMBERS, nMinData, nMaxData, _T("f(Hz)")); // set legend scale
            // do common plot paint jobs
            PlotPrePaint(pDC, rWnd, rClip);
            PlotStandardPaint(pDC, rWnd, rClip, pPitch, pDoc, SKIP_UNSET); // do standard data paint
        } else  if (pPitchParm->nScaleMode == 2) {
            // semitone display
            static const double dSemitoneScale = 12.0 / log(2.0);
            static const double dSemitoneReference =  + (69. - log(440.0)* 12.0 / log(2.0)) / dSemitoneScale;
            double dMin = nMinData > 0 ? (dSemitoneReference + log((double)nMinData)) * dSemitoneScale : 0;
            double dMax = nMaxData > 0 ? (dSemitoneReference + log((double)nMaxData)) * dSemitoneScale : 0;
            pGraph->SetLegendScale(SCALE | NUMBERS, dMin, dMax, _T("Semitones")); // set legend scale
            // do common plot paint jobs
            PlotPrePaint(pDC, rWnd, rClip);
            PlotStandardPaint(pDC, rWnd, rClip, pPitch, pDoc, SKIP_UNSET | PAINT_SEMITONES); // do standard data paint
        } else {
            // logarithmic display
            pGraph->SetLegendScale(SCALE | NUMBERS | LOG10, nMinData, nMaxData, _T("f(Hz)")); // set legend scale
            // do common plot paint jobs
            PlotPrePaint(pDC, rWnd, rClip);
            PlotStandardPaint(pDC, rWnd, rClip, pPitch, pDoc, SKIP_UNSET | PAINT_LOG10); // do standard data paint
        }
    }
    // do common plot paint jobs
    PlotPaintFinish(pDC, rWnd, rClip);
}
