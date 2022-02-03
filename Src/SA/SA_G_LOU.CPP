/////////////////////////////////////////////////////////////////////////////
// sa_g_lou.cpp:
// Implementation of the CPlotLoudness class.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_plot.h"
#include "sa_g_lou.h"
#include "Process\Process.h"
#include "Process\sa_p_lou.h"
#include "sa_minic.h"
#include "sa_graph.h"
#include "Process\sa_p_InstantaneousPower.h"

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
// CPlotLoudness
// Plot window for displaying loudness data. This class only handles the
// special drawing of the data. All other work is done in the base class.

IMPLEMENT_DYNCREATE(CPlotLoudness, CPlotWnd)

/////////////////////////////////////////////////////////////////////////////
// CPlotLoudness message map

BEGIN_MESSAGE_MAP(CPlotLoudness, CPlotWnd)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotLoudness construction/destruction/creation

/***************************************************************************/
// CPlotLoudness::CPlotLoudness Constructor
/***************************************************************************/
CPlotLoudness::CPlotLoudness() {
}



void  CPlotLoudness::CopyTo(CPlotWnd * pT) {
    CPlotWnd::CopyTo(pT);
}



CPlotWnd * CPlotLoudness::NewCopy(void) {
    CPlotWnd * pRet = new CPlotLoudness();

    CopyTo(pRet);

    return pRet;
}



/***************************************************************************/
// CPlotLoudness::~CPlotLoudness Destructor
/***************************************************************************/
CPlotLoudness::~CPlotLoudness() {
}

/////////////////////////////////////////////////////////////////////////////
// CPlotLoudness message handlers

/***************************************************************************/
// CPlotLoudness::OnDraw Painting
// The data to draw is coming from a temporary file, created by the loudness
// processing class, which is called to do data processing if necessary
// before drawing. Call the function PlotPaintFinish at the end of the
// drawing to let the plot base class do common jobs like drawing the
// cursors.
/***************************************************************************/
void CPlotLoudness::OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView) {
    // get pointer to main frame, graph, and document
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaDoc * pModel = pView->GetDocument();
    // create loudness data
    CProcessLoudness * pLoudness = (CProcessLoudness *)pModel->GetLoudness(); // get pointer to loudness object
    short int nResult = LOWORD(pLoudness->Process(this, pModel)); // process data
    nResult = CheckResult(nResult, pLoudness); // chaeck the process result
    if (nResult == PROCESS_ERROR) {
        return;
    }
    if (nResult != PROCESS_CANCELED) {
        BOOL bPercent = pModel->GetCIntensityParm().nScaleMode == 1;
        if (bPercent) {
            double dLoudToPercent = 32767/2/100.0;
            double dMaxPercent = 103.;

            pGraph->SetLegendScale(SCALE | NUMBERS , 0, int(dMaxPercent), _T("Linear Magnitude (%)")); // set legend scale
            // do common plot paint jobs
            PlotPrePaint(pDC, rWnd, rClip);
            SetProcessMultiplier(dLoudToPercent);
        } else {
            // dB
            pGraph->SetLegendScale(SCALE | NUMBERS , -31, 3, _T("Relative Intensity (dB)")); // set legend scale
            // do common plot paint jobs
            PlotPrePaint(pDC, rWnd, rClip);
            SetProcessMultiplier(PRECISION_MULTIPLIER);
        }

        PlotStandardPaint(pDC, rWnd, rClip, pLoudness, pModel, bPercent ? 0 : PAINT_DB); // do standard data paint
    }
    if (researchSettings.m_bShowInstantaneousPower) {
        // create raw data
        CProcessInstantaneousPower power;
        CProcessInstantaneousPower * pPower = &power;
        nResult = LOWORD(pPower->Process(this, pModel)); // process data
        nResult = CheckResult(nResult, pPower); // check the process result
        if (nResult == PROCESS_ERROR) {
            return;
        }
        if (nResult != PROCESS_CANCELED) {
            BOOL bPercent = pModel->GetCIntensityParm().nScaleMode == 1;
            if (bPercent) {
                double dLoudToPercent = 32767/4/100.0;

                SetProcessMultiplier(dLoudToPercent);
            } else {
                // dB
                SetProcessMultiplier(32768);
            }
            CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
            pMainWnd->SwapInOverlayColors(4);
            PlotStandardPaint(pDC, rWnd, rClip, pPower, pModel, bPercent ? 0 : PAINT_DB); // do standard data paint
            pMainWnd->SwapOutOverlayColors();
        }
    }
    // do common plot paint jobs
    PlotPaintFinish(pDC, rWnd, rClip);
}



