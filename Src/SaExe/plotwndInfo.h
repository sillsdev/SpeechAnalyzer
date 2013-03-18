#ifndef PLOTWNDINFO_H
#define PLOTWNDINFO_H

class CPlotWndInfo {
public:
    CPlotWndInfo(CPlotWnd * pPlot, UINT Id, const CSaDoc * pDoc) {
        m_pPlot = pPlot;
        m_uPlotID = Id;
        m_pDoc = pDoc;
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
