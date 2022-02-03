#ifndef PLOTWNDINFO_H
#define PLOTWNDINFO_H

class CPlotWndInfo {
public:
    CPlotWndInfo(CPlotWnd * pPlot, UINT Id, const CSaDoc * pModel) {
        m_pPlot = pPlot;
        m_uPlotID = Id;
        m_pDoc = pModel;
    };
    ~CPlotWndInfo() {
        delete m_pPlot;
    };
    CPlotWnd * m_pPlot;
    UINT m_uPlotID;
    const CSaDoc * m_pDoc;
};

typedef CList< CPlotWndInfo *, CPlotWndInfo *> PlotWndListDef;

#endif
