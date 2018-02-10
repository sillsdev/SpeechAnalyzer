/////////////////////////////////////////////////////////////////////////////
// sa_g_gra.cpp:
// Implementation of the CPlotGrappl class.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "sa_plot.h"
#include "sa_g_gra.h"
#include "Process\Process.h"
#include "Process\sa_p_gra.h"
#include "sa_minic.h"
#include "sa_graph.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "sa_mplot.h"
#include "LegendWnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//###########################################################################
// CPlotGrappl
// Plot window for displaying grappl pitch data. This class only handles the
// special drawing of the data. All other work is done in the base class.

IMPLEMENT_DYNCREATE(CPlotGrappl, CPlotWnd)

/////////////////////////////////////////////////////////////////////////////
// CPlotGrappl message map

BEGIN_MESSAGE_MAP(CPlotGrappl, CPlotWnd)
    ON_WM_CREATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotGrappl construction/destruction/creation

/***************************************************************************/
// CPlotGrappl::CPlotGrappl Constructor
/***************************************************************************/
CPlotGrappl::CPlotGrappl() {
}


CPlotWnd * CPlotGrappl::NewCopy() {
    CPlotWnd * pRet = new CPlotGrappl;

    CopyTo(pRet);

    return pRet;
}


/***************************************************************************/
// CPlotGrappl::~CPlotGrappl Destructor
/***************************************************************************/
CPlotGrappl::~CPlotGrappl() {
}

/////////////////////////////////////////////////////////////////////////////
// CPlotGrappl message handlers

/***************************************************************************/
// CPlotGrappl::OnDraw Drawing
// The data to draw is coming from a temporary file, created by the grappl
// processing class, which is called to do data processing if necessary
// before drawing. Call the function PlotPaintFinish at the end of the
// drawing to let the plot base class do common jobs like drawing the
// cursors.
/***************************************************************************/
void CPlotGrappl::OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView) {
    
	// get pointer to main frame, graph, and document
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaDoc  *  pDoc   = pView->GetDocument();

    // create grappl data
    CProcessGrappl * pGrappl = (CProcessGrappl *)pDoc->GetGrappl(); // get pointer to grappl object
    short int nResult = LOWORD(pGrappl->Process(this, pDoc));       // process data
    nResult = CheckResult(nResult, pGrappl);                        // check the process result
    if (nResult == PROCESS_ERROR) {
        return;
    }
    if (pGrappl->IsStatusFlag(PROCESS_NO_PITCH)) {
        //temporarily disable till we think of something better.
        TRACE(_T("No data on %lp\n"),this);
        if (!pGraph->GetPlot()->IsKindOf(RUNTIME_CLASS(CMultiPlotWnd))) {
            m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_NOVOICING, &rWnd);
        }
    } else if (pGrappl->IsDataReady()) {
        // get pointer to pitch parameters
        const CPitchParm * pPitchParm = pDoc->GetPitchParm();
        // set data range
        int nMinData = 0;
        int nMaxData = 0;
        if (pPitchParm->nRangeMode) {
            // manual range mode
            nMinData = pPitchParm->nLowerBound;
            nMaxData = pPitchParm->nUpperBound;
        } else {
            // auto range mode
            CPitchParm::GetAutoRange(pDoc, nMaxData, nMinData);
        }
        SetProcessMultiplier(PRECISION_MULTIPLIER);
        SetBold(FALSE);
        if (pPitchParm->nScaleMode == 1) {
            // linear display
            pGraph->SetLegendScale(SCALE | NUMBERS, nMinData, nMaxData, _T("f(Hz)")); // set legend scale
            // do common plot paint jobs
            PlotPrePaint(pDC, rWnd, rClip);
            PlotStandardPaint(pDC, rWnd, rClip, pGrappl, pDoc, SKIP_UNSET); // do standard data paint */
        } else  if (pPitchParm->nScaleMode == 2) {
            // semitone display
            static const double dSemitoneScale = 12.0 / log(2.0);
            static const double dSemitoneReference =  + (69. - log(440.0)* 12.0 / log(2.0)) / dSemitoneScale;
            double dMin = nMinData > 0 ? (dSemitoneReference + log((double)nMinData)) * dSemitoneScale : 0;
            double dMax = nMaxData > 0 ? (dSemitoneReference + log((double)nMaxData)) * dSemitoneScale : 0;
            pGraph->SetLegendScale(SCALE | NUMBERS, dMin, dMax, _T("Semitones")); // set legend scale
            // do common plot paint jobs
            PlotPrePaint(pDC, rWnd, rClip);
            PlotStandardPaint(pDC, rWnd, rClip, pGrappl, pDoc, SKIP_UNSET | PAINT_SEMITONES); // do standard data paint
        } else {
            // logarithmic display
            pGraph->SetLegendScale(SCALE | NUMBERS | LOG10, nMinData, nMaxData, _T("f(Hz)")); // set legend scale
            // do common plot paint jobs
            PlotPrePaint(pDC, rWnd, rClip);
            PlotStandardPaint(pDC, rWnd, rClip, pGrappl, pDoc, SKIP_UNSET | PAINT_LOG10); // do standard data paint
        }
    }
    // do common plot paint jobs
    PlotPaintFinish(pDC, rWnd, rClip);

}

int CPlotGrappl::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CPlotWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }

    return 0;
}
