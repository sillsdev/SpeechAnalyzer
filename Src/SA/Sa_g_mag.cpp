/////////////////////////////////////////////////////////////////////////////
// sa_g_mag.cpp:
// Implementation of the CPlotMagnitude class.
// Author: Todd Jones
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_plot.h"
#include "sa_g_mag.h"
#include "Process\Process.h"
#include "Process\sa_p_lou.h"
#include "sa_minic.h"
#include "sa_graph.h"


#include "math.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CPlotMagnitude
// Plot window for displaying magnitude data. This class only handles the
// special drawing of the data. All other work is done in the base class.

IMPLEMENT_DYNCREATE(CPlotMagnitude, CPlotWnd)

/////////////////////////////////////////////////////////////////////////////
// CPlotMagnitude message map

BEGIN_MESSAGE_MAP(CPlotMagnitude, CPlotWnd)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotMagnitude construction/destruction/creation

/***************************************************************************/
// CPlotMagnitude::CPlotMagnitude Constructor
/***************************************************************************/
CPlotMagnitude::CPlotMagnitude() {
}



void  CPlotMagnitude::CopyTo(CPlotWnd * pT) {
    CPlotWnd::CopyTo(pT);
}



CPlotWnd * CPlotMagnitude::NewCopy(void) {
    CPlotWnd * pRet = new CPlotMagnitude();

    CopyTo(pRet);

    return pRet;
}



/***************************************************************************/
// CPlotMagnitude::~CPlotMagnitude Destructor
/***************************************************************************/
CPlotMagnitude::~CPlotMagnitude() {
}

/////////////////////////////////////////////////////////////////////////////
// CPlotMagnitude message handlers

/***************************************************************************/
// CPlotMagnitude::OnDraw Painting
// The data to draw is coming from a temporary file, created by the magnitude
// processing class, which is called to do data processing if necessary
// before drawing. Call the function PlotPaintFinish at the end of the
// drawing to let the plot base class do common jobs like drawing the
// cursors.
/***************************************************************************/
void CPlotMagnitude::OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView) {
    // get pointer to main frame, graph, and document
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaDoc * pDoc = pView->GetDocument();
    // create loudness data
    CProcessLoudness * pLoudness = (CProcessLoudness *)pDoc->GetLoudness(); // get pointer to loudness object
    short int nResult = LOWORD(pLoudness->Process(this, pDoc)); // process data
    nResult = CheckResult(nResult, pLoudness); // check the process result
    if (nResult == PROCESS_ERROR) {
        return;
    }
    if (nResult != PROCESS_CANCELED) {
        pGraph->SetLegendScale(SCALE | NUMBERS, 0, pLoudness->GetMaxValue(), _T("Magnitude")); // set legend scale
        // do common plot paint jobs
        PlotPrePaint(pDC, rWnd, rClip);
        SetBold(FALSE);
        PlotStandardPaint(pDC, rWnd, rClip, pLoudness, pDoc); // do standard data paint
    }
    // do common plot paint jobs
    PlotPaintFinish(pDC, rWnd, rClip);
}



