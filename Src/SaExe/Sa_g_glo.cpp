/////////////////////////////////////////////////////////////////////////////
// sa_g_glo.cpp:
// Implementation of the CPlotGlottis class.
// Author: Alec Epting
// copyright 1997 JAARS Inc. SIL
//
// Revision History
//   1.06.6U5
//        SDM Added note to Alec re changes needed to be compatible with new graph alignment
//        SDM Changed float to double
//        SDM ifdef out the OnDraw function
//   4/26/2001
//        1.06.6U5 changes were in error in this code thread. 
//        Removed #ifdef
//        Removed erroneous comments
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_plot.h"
#include "sa_g_glo.h"
#include "Process\sa_proc.h"
#include "Process\sa_p_glo.h"
#include "sa_graph.h"

#include "sa_doc.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CPlotGlottis
// Plot window for displaying the glottal waveform. This class only handles the
// special drawing of the data. All other work is done in the base class.

IMPLEMENT_DYNCREATE(CPlotGlottis, CPlotWnd)

/////////////////////////////////////////////////////////////////////////////
// CPlotGlottis message map

BEGIN_MESSAGE_MAP(CPlotGlottis, CPlotWnd)
    //{{AFX_MSG_MAP(CPlotGlottis)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotGlottis construction/destruction/creation

/***************************************************************************/
// CPlotGlottis::CPlotGlottis Constructor
/***************************************************************************/
CPlotGlottis::CPlotGlottis()
{
}


void  CPlotGlottis::CopyTo(CPlotWnd * pT)
{
  CPlotGlottis * pTarg = (CPlotGlottis *)pT;

  CPlotWnd::CopyTo(pT);

  pTarg->m_bDotDraw = m_bDotDraw;
}



CPlotWnd * CPlotGlottis::NewCopy(void)
{
  CPlotWnd * pRet = new CPlotGlottis();

  CopyTo(pRet);

  return pRet;
}


/***************************************************************************/
// CPlotGlottis::~CPlotGlottis Destructor
/***************************************************************************/
CPlotGlottis::~CPlotGlottis()
{
}

/////////////////////////////////////////////////////////////////////////////
// CPlotGlottis helper functions

/////////////////////////////////////////////////////////////////////////////
// CPlotGlottis message handlers

/***************************************************************************/
// CPlotGlottis::OnDraw drawing
// The data to draw is coming from the documents glottal wave temp file. Call the
// function PlotPaintFinish at the end of the drawing to let the plot base
// class do common jobs like drawing the cursors.
/***************************************************************************/
void CPlotGlottis::OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView)
{
  // SDM 7/19/2001 Copied code from Raw to update to allow full zooming unable to test
  //               underlying process GPFs.
  
  // get pointer to graph, view and document
  CGraphWnd* pGraph = (CGraphWnd*)GetParent();
  CSaDoc   * pDoc   = pView->GetDocument();
  // create change data
  CProcessGlottis* pGlottalWave = (CProcessGlottis*)pDoc->GetGlottalWave(); // get pointer to glottal waveform
  short int nResult = LOWORD(pGlottalWave->Process(this, pDoc)); // process data
  nResult = CheckResult(nResult, pGlottalWave); // check the process result
  if (nResult == PROCESS_ERROR) return;
  if (nResult != PROCESS_CANCELED)
  {
    pGraph->SetLegendScale(SCALE | NUMBERS, -50, 50, _T("Waveform")); // set legend scale
    SetProcessMultiplier(32768./100.);
    // do common plot paint jobs
    PlotPrePaint(pDC, rWnd, rClip);
    PlotStandardPaint(pDC, rWnd, rClip, pGlottalWave, pDoc, (m_bDotDraw ? PAINT_CROSSES : 0)  | PAINT_MIN); // do standard data paint
  }
  // do common plot paint jobs
  PlotPaintFinish(pDC, rWnd, rClip);
}
