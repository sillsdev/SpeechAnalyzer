/////////////////////////////////////////////////////////////////////////////
// sa_mplot.h:  The multi-plot class
// Author: Doug Eberman
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_MPLOT_H
#define _SA_MPLOT_H

#include "plotwndinfo.h"

//###########################################################################
// CMultiPlotWnd

class CMultiPlotWnd : public CPlotWnd {
    DECLARE_DYNCREATE(CMultiPlotWnd)

// Construction/destruction/creation
public:
    CMultiPlotWnd();
    virtual ~CMultiPlotWnd();
    BOOL IsIDincluded(UINT id);
    void AddPlot(CPlotWnd * plot, UINT id, CSaView * pView);
    int GetNumPlots() {
        return m_List.GetCount();
    };
    void RemovePlot(const CPlotWnd * pPlot);
    UINT GetBasePlotID();
    const PlotWndListDef * GetList() {
        return &m_List;
    };
    PlotWndListDef m_List;

    // Operations
public:
    void SetHelperMode(int nMode, int nID, CRect * prParent);
    void RemoveRtPlots();
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);
    void MergeInOverlayPlot(CMultiPlotWnd * pMPlot);
    void VerifyPlots(); // Verify plots (especially is document still open)

    // Generated message map functions
protected:
    DECLARE_MESSAGE_MAP()
};

#endif //_SA_MPLOT_H


