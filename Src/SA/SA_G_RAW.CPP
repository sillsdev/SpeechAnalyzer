/////////////////////////////////////////////////////////////////////////////
// sa_g_raw.cpp:
// Implementation of the CPlotRawData class.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
//
// Revision History
//    1.06.6U4
//         SDM Changed OnDraw to align to pixel to eliminate scrolling errors
//         SDM Changed float to double to eliminate scrolling errors
//    1.06.6U5
//         SDM Changed OnDraw to SetLegend before drawing grid, (draw grid correctly)
//  08/29/2000
//         DDO Changed a few functions to local functions so objects derived
//             from this one could override them. At the time I did this it
//             was for the sake of the recording plot.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_plot.h"
#include "sa_g_raw.h"
#include "sa_minic.h"
#include "sa_graph.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "mainfrm.h"
#include "sa_mplot.h"
#include "sa.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CPlotRawData
// Plot window for displaying raw data. This class only handles the
// special drawing of the data. All other work is done in the base class.

IMPLEMENT_DYNCREATE(CPlotRawData, CPlotWnd)

BEGIN_MESSAGE_MAP(CPlotRawData, CPlotWnd)
END_MESSAGE_MAP()

CPlotRawData::CPlotRawData() {
}

void  CPlotRawData::CopyTo(CPlotWnd * pT) {
    CPlotWnd::CopyTo(pT);
}

CPlotWnd * CPlotRawData::NewCopy(void) {
    CPlotWnd * pRet = new CPlotRawData();

    CopyTo(pRet);

    return pRet;
}


/***************************************************************************/
// CPlotRawData::~CPlotRawData Destructor
/***************************************************************************/
CPlotRawData::~CPlotRawData() {
}

/////////////////////////////////////////////////////////////////////////////
// CPlotRawData helper functions

//*******************************************************************
// 08/29/2000 - DDO Added so plots derived from this one can
//                  override this function.
//*******************************************************************
BOOL CPlotRawData::SetLegendScale() {
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    return pGraph->SetLegendScale(SCALE | NUMBERS, -100, 100, _T("Waveform (percentage)")); // set legend scale
}

/////////////////////////////////////////////////////////////////////////////
// CPlotRawData message handlers

/***************************************************************************/
// CPlotRawData::OnDraw drawing
// The data to draw is coming from the documents wave file. Call the
// function PlotPaintFinish at the end of the drawing to let the plot base
// class do common jobs like drawing the cursors.
/***************************************************************************/
void CPlotRawData::OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView) {

    // get pointer to graph, view and document
    SaContext context = pView->GetSaContext();
    
    // create raw data
    CProcessRaw * pRaw = context.model.GetRaw();            // get pointer to change object
    short int nResult = LOWORD(pRaw->Process(this));        // process data
    nResult = CheckResult(nResult, pRaw);                   // check the process result
    if (nResult == PROCESS_ERROR) {
        return;
    }

    if (nResult != PROCESS_CANCELED) {
        if (SetLegendScale()) {
            return;                                         // set legend scale invalidated entire graph no point in drawing now
        }
        SetProcessMultiplier(32768.0/100.0);
        // do common plot paint jobs
        PlotPrePaint(pDC, rWnd, rClip);
        PlotStandardPaint(pDC, rWnd, rClip, pRaw, &context.model, (m_bDotDraw ? PAINT_CROSSES : 0)  | PAINT_MIN); // do standard data paint
    }

    if (context.app.GetResearchSettings().showHilbertTransform) {
        // create raw data
        CProcessHilbert * pHilbert = context.model.GetHilbert();    // get pointer to change object
        nResult = LOWORD(pHilbert->Process(this));    // process data
        nResult = CheckResult(nResult, pHilbert);           // check the process result
        if (nResult == PROCESS_ERROR) {
            return;
        }
        if (nResult != PROCESS_CANCELED) {
            SetProcessMultiplier(0.5*32768./100.);          // Because of potential for overflow the hilbert transformed data is scaled 50%
            CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
            pMainWnd->SwapInOverlayColors(4);
            PlotStandardPaint(pDC, rWnd, rClip, pHilbert, &context.model, (m_bDotDraw ? PAINT_CROSSES : 0)  | PAINT_MIN); // do standard data paint
            pMainWnd->SwapOutOverlayColors();
        }
    }
    // do common plot paint jobs
    PlotPaintFinish(pDC, rWnd, rClip);
}