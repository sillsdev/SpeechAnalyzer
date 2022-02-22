/////////////////////////////////////////////////////////////////////////////
// sa_g_cha.cpp:
// Implementation of the CPlotChange class.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_plot.h"
#include "sa_g_cha.h"
#include "sa_minic.h"
#include "sa_graph.h"

#include "sa_doc.h"
#include "sa_view.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CPlotChange
// Plot window for displaying change data. This class only handles the
// special drawing of the data. All other work is done in the base class.

IMPLEMENT_DYNCREATE(CPlotChange, CPlotWnd)

/////////////////////////////////////////////////////////////////////////////
// CPlotChange message map

BEGIN_MESSAGE_MAP(CPlotChange, CPlotWnd)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotChange construction/destruction/creation

/***************************************************************************/
// CPlotChange::CPlotChange Constructor
/***************************************************************************/
CPlotChange::CPlotChange() {
}

/***************************************************************************/
// CPlotChange::~CPlotChange Destructor
/***************************************************************************/
CPlotChange::~CPlotChange() {
}


void  CPlotChange::CopyTo(CPlotWnd * pT) {
    CPlotWnd::CopyTo(pT);
}



CPlotWnd * CPlotChange::NewCopy(void) {
    CPlotWnd * pRet = new CPlotChange();

    CopyTo(pRet);

    return pRet;
}


/////////////////////////////////////////////////////////////////////////////
// CPlotChange message handlers

/***************************************************************************/
// CPlotChange::OnDraw Drawing
// The data to draw is coming from a temporary file, created by the change
// processing class, which is called to do data processing if necessary
// before drawing. Call the function PlotPaintFinish at the end of the
// drawing to let the plot base class do common jobs like drawing the
// cursors.
/***************************************************************************/
void CPlotChange::OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView) {
    // get pointer to graph, view and document
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaDoc  *  pModel   = pView->GetDocument();
    // create change data
    CProcessChange * pChange = (CProcessChange *)pModel->GetChange(); // get pointer to change object
    short int nResult = LOWORD(pChange->Process(this, pModel)); // process data
    nResult = CheckResult(nResult, pChange); // check the process result
    if (nResult == PROCESS_ERROR) {
        return;
    }
    if (nResult != PROCESS_CANCELED) {
        pGraph->SetLegendScale(SCALE | NUMBERS, 0, pChange->GetMaxValue() / PRECISION_MULTIPLIER, _T("Change")); // set legend scale
        // do common plot paint jobs
        PlotPrePaint(pDC, rWnd, rClip);
        SetProcessMultiplier(PRECISION_MULTIPLIER);
        PlotStandardPaint(pDC, rWnd, rClip, pChange, pModel); // do standard data paint
    }
    // do common plot paint jobs
    PlotPaintFinish(pDC, rWnd, rClip);
}
