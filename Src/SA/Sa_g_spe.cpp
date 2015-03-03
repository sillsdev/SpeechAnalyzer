/////////////////////////////////////////////////////////////////////////////
// sa_g_spe.cpp:
// Implementation of the CPlotBlank    class.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_plot.h"
#include "sa_g_spe.h"
#include "sa_doc.h"
#include "sa_view.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CPlotBlank
// Plot window for displaying a blank plot. This class only handles the
// special drawing of the data. All other work is done in the base class.

IMPLEMENT_DYNCREATE(CPlotBlank, CPlotWnd)

/////////////////////////////////////////////////////////////////////////////
// CPlotBlank message map

BEGIN_MESSAGE_MAP(CPlotBlank, CPlotWnd)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotBlank construction/destruction/creation

/***************************************************************************/
// CPlotBlank::CPlotBlank Constructor
/***************************************************************************/
CPlotBlank::CPlotBlank() {
}

/***************************************************************************/
// CPlotBlank::~CPlotBlank Destructor
/***************************************************************************/
CPlotBlank::~CPlotBlank() {
}

/////////////////////////////////////////////////////////////////////////////
// CPlotBlank message handlers

/***************************************************************************/
// CPlotBlank::OnDraw Painting
// It's a blank plot, so nothing except the helper window is drawn.
/***************************************************************************/
void CPlotBlank::OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * /*pView*/) {
    // display helper window
    m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_BLANKGRAPH, &rWnd);
    // do common plot paint jobs
    PlotPaintFinish(pDC, rWnd, rClip);
}
