/////////////////////////////////////////////////////////////////////////////
// sa_mplot.cpp:
// Implementation of the CPlotHelperWnd
//                       CMultiPlotWnd       classes.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_plot.h"
#include "sa_mplot.h"

#include "sa_doc.h"
#include "sa.h"
#include "sa_view.h"
#include "mainfrm.h"
#include "math.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


//###########################################################################
// CMultiPlotWnd


IMPLEMENT_DYNCREATE(CMultiPlotWnd, CPlotWnd)

BEGIN_MESSAGE_MAP(CMultiPlotWnd, CPlotWnd)
END_MESSAGE_MAP()

/***************************************************************************/
// CMultiPlotWnd::CMultiPlotWnd Constructor
/***************************************************************************/
CMultiPlotWnd::CMultiPlotWnd() {
    ShowCursors(false,true);
}

/***************************************************************************/
// CMultiPlotWnd::~CMultiPlotWnd Destructor
/***************************************************************************/
CMultiPlotWnd::~CMultiPlotWnd() {
    // the plot windows support autocleanup, so we don't need to destroy
    // them here.
    while (!m_List.IsEmpty()) {
        delete m_List.RemoveHead();
    }
}

void CMultiPlotWnd::AddPlot(CPlotWnd * plot, UINT id, CSaView * pView) {
    if ((m_List.GetCount() == 1) &&
            (GetBasePlotID() == IDD_BLANK)) {
        CPlotWndInfo * pInfo = m_List.RemoveHead();
        pInfo->m_pPlot->DestroyWindow();
        delete pInfo;
    }
    m_List.AddTail(new CPlotWndInfo(plot, id, pView->GetDocument()));
}

void CMultiPlotWnd::OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView *) {
    // check for invalid views and remove their plots before drawing
    VerifyPlots();

    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    CSaApp * pApp = (CSaApp *)AfxGetApp();

    POSITION pos = m_List.GetHeadPosition();

    int j = 0;
    while (pos) {
        CPlotWndInfo * pInfo = m_List.GetNext(pos);
        if (pApp->IsDocumentOpened(pInfo->m_pDoc)) {
            POSITION pos = pInfo->m_pDoc->GetFirstViewPosition();
            CSaView * pView = (CSaView *)pInfo->m_pDoc->GetNextView(pos);
            if (j) {
                pMainWnd->SwapInOverlayColors(j);
            }
            pInfo->m_pPlot->OnDraw(pDC, rWnd, rClip, pView);
            if (j) {
                pMainWnd->SwapOutOverlayColors();
            }
        }
        j++;
    }
}

void CMultiPlotWnd::VerifyPlots() {
    // check for invalid views and remove their plots
    CSaApp * pApp = (CSaApp *)AfxGetApp();
    POSITION pos = m_List.GetHeadPosition();
    while (pos) {
        POSITION oldPos = pos;
        CPlotWndInfo * pInfo = m_List.GetNext(pos);

        // we need to find the document and view
        // if we don't find both, then destroy the plot object
        if (pApp->IsDocumentOpened(pInfo->m_pDoc)) {
            POSITION pos2 = pInfo->m_pDoc->GetFirstViewPosition();
            CSaView * pView = (CSaView *) pInfo->m_pDoc->GetNextView(pos2);
            if (pView) {
                continue;
            }
        }

        m_List.RemoveAt(oldPos);
        pInfo->m_pPlot->DestroyWindow();
        TRACE(_T("Destroying plot %lp\n"),pInfo->m_pPlot);
        delete pInfo;
    }
}

BOOL CMultiPlotWnd::IsIDincluded(UINT id) {
    POSITION pos = m_List.GetHeadPosition();
    while (pos) {
        CPlotWndInfo * pInfo = m_List.GetNext(pos);
        if (pInfo->m_uPlotID==id) {
            return TRUE;
        }
    }

    return FALSE;
}


void CMultiPlotWnd::MergeInOverlayPlot(CMultiPlotWnd *) {
    ASSERT(0);
    // method not implemented yet.
}


void CMultiPlotWnd::RemovePlot(const CPlotWnd * pPlot) {
    POSITION pos = m_List.GetHeadPosition();
    while (pos) {
        POSITION oldPos = pos;
        CPlotWndInfo * pInfo = m_List.GetNext(pos);
        if (pInfo->m_pPlot == pPlot) {
            m_List.RemoveAt(oldPos);
            pInfo->m_pPlot->DestroyWindow();
            delete pInfo;
        }
    }
}

UINT CMultiPlotWnd::GetBasePlotID() {
    POSITION pos = m_List.GetHeadPosition();
    if (pos) {
        CPlotWndInfo * pInfo = m_List.GetNext(pos);
        return pInfo->m_uPlotID;
    }
    ASSERT(FALSE);
    return (UINT)-1;
};

void CMultiPlotWnd::RemoveRtPlots() {
    if (m_List.GetCount() <= 1) {
        return;
    }
    POSITION pos = m_List.GetHeadPosition();
    if (pos) {
        // first graph always stays put.
        m_List.GetNext(pos);
        while (pos) {
            POSITION oldPos = pos;
            CPlotWndInfo * pInfo = m_List.GetNext(pos);
            if (pInfo->m_pPlot->IsRtOverlay()) {
                m_List.RemoveAt(oldPos);
                pInfo->m_pPlot->DestroyWindow();
                delete pInfo;
            }
        }
    }
}

void CMultiPlotWnd::SetHelperMode(int nMode, int nID, CRect * prParent) {
    m_HelperWnd.SetMode(nMode, nID, prParent);
}
