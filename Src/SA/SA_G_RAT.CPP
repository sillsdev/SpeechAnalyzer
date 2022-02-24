/////////////////////////////////////////////////////////////////////////////
// sa_g_rat.cpp:
// Implementation of the CPlotRatio class.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_plot.h"
#include "sa_g_rat.h"
#include "sa_minic.h"
#include "sa_graph.h"
#include "sa_doc.h"
#include "sa_view.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CPlotRatio
// Plot window for displaying ratio detector data. This class only handles
// the special drawing of the data. All other work is done in the base class.

IMPLEMENT_DYNCREATE(CPlotRatio, CPlotWnd)

/////////////////////////////////////////////////////////////////////////////
// CPlotRatio message map

BEGIN_MESSAGE_MAP(CPlotRatio, CPlotWnd)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotRatio construction/destruction/creation

/***************************************************************************/
// CPlotRatio::CPlotRatio Constructor
/***************************************************************************/
CPlotRatio::CPlotRatio() {
}

/***************************************************************************/
// CPlotRatio::~CPlotRatio Destructor
/***************************************************************************/
CPlotRatio::~CPlotRatio() {
}

/////////////////////////////////////////////////////////////////////////////
// CPlotRatio message handlers

/***************************************************************************/
// CPlotRatio::OnDraw Drawing
// The data to draw is coming from a temporary file, created by the ratio
// processing class, which is called to do data processing if necessary
// before drawing. Call the function PlotPaintFinish at the end of the
// drawing to let the plot base class do common jobs like drawing the
// cursors.
/***************************************************************************/
void CPlotRatio::OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView) {
    // get pointer to graph, view and document
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaDoc  *  pModel   = pView->GetDocument();
    // create ratio data
    CProcessRatio * pRatio = (CProcessRatio *)pModel->GetRatio(); // get pointer to ratio object
    short int nResult = LOWORD(pRatio->Process(this)); // process data
    nResult = CheckResult(nResult, pRatio); // check the process result
    if (nResult == PROCESS_ERROR) {
        return;
    }
    if (nResult != PROCESS_CANCELED) {
        pGraph->SetLegendScale(SCALE | NUMBERS, 0, pRatio->GetMaxValue() / PRECISION_MULTIPLIER, _T("Ratio")); // set legend scale
        // do common plot paint jobs
        PlotPrePaint(pDC, rWnd, rClip);
        SetProcessMultiplier(PRECISION_MULTIPLIER);
        PlotStandardPaint(pDC, rWnd, rClip, pRatio, pModel); // do standard data paint
    }
    // do common plot paint jobs
    PlotPaintFinish(pDC, rWnd, rClip);
}
