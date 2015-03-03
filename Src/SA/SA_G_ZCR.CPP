/////////////////////////////////////////////////////////////////////////////
// sa_g_zcr.cpp:
// Implementation of the CPlotZCross class.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_plot.h"
#include "sa_g_zcr.h"
#include "Process\Process.h"
#include "Process\sa_p_zcr.h"
#include "sa_minic.h"
#include "sa_graph.h"

#include "sa_doc.h"
#include "sa_view.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CPlotZCross
// Plot window for displaying zero crossing data. This class only handles
// the special drawing of the data. All other work is done in the base class.

IMPLEMENT_DYNCREATE(CPlotZCross, CPlotWnd)

/////////////////////////////////////////////////////////////////////////////
// CPlotZCross message map

BEGIN_MESSAGE_MAP(CPlotZCross, CPlotWnd)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotZCross construction/destruction/creation

/***************************************************************************/
// CPlotZCross::CPlotZCross Constructor
/***************************************************************************/
CPlotZCross::CPlotZCross() {
}



CPlotWnd * CPlotZCross::NewCopy() {
    CPlotWnd * pRet = new CPlotZCross;

    CopyTo(pRet);

    return pRet;
}

/***************************************************************************/
// CPlotZCross::~CPlotZCross Destructor
/***************************************************************************/
CPlotZCross::~CPlotZCross() {
}

/////////////////////////////////////////////////////////////////////////////
// CPlotZCross message handlers

/***************************************************************************/
// CPlotZCross::OnDraw Drawing
// The data to draw is coming from a temporary file, created by the zero
// crossings processing class, which is called to do data processing if
// necessary before drawing. Call the function PlotPaintFinish at the end of
// the drawing to let the plot base class do common jobs like drawing the
// cursors.
/***************************************************************************/
void CPlotZCross::OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView) {
    // get pointer to graph, view and document
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaDoc  *  pDoc   = pView->GetDocument();
    // create zero crossings data
    CProcessZCross * pZCross = (CProcessZCross *)pDoc->GetZCross(); // get pointer to zero crossings object
    short int nResult = LOWORD(pZCross->Process(this, pDoc)); // process data
    nResult = CheckResult(nResult, pZCross); // check the process result
    if (nResult == PROCESS_ERROR) {
        return;
    }
    if (nResult != PROCESS_CANCELED) {
        pGraph->SetLegendScale(SCALE | NUMBERS, -0.5, pZCross->GetMaxValue(), _T("Zero Crossing")); // set legend scale
        // do common plot paint jobs
        PlotPrePaint(pDC, rWnd, rClip);
        PlotStandardPaint(pDC, rWnd, rClip, pZCross, pDoc); // do standard data paint
    }
    // do common plot paint jobs
    PlotPaintFinish(pDC, rWnd, rClip);
}
