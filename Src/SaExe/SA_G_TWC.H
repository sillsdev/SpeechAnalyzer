/////////////////////////////////////////////////////////////////////////////
// sa_g_twc.h:
// Interface of the CPlotTonalWeightChart class
// Author: Todd Jones
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//  07/20/2000
//         RLJ Added support for Up (/\) and Down (\/) buttons in TWC legend
/////////////////////////////////////////////////////////////////////////////
#ifndef _sa_g_twc_H
#define _sa_g_twc_H

//###########################################################################
// CPlotTonalWeightChart plot window

class CProcessTonalWeightChart;

class CPlotTonalWeightChart : public CPlotWnd
{
    DECLARE_DYNCREATE(CPlotTonalWeightChart)

// Construction/destruction/creation
public:
    CPlotTonalWeightChart();
    virtual ~CPlotTonalWeightChart();

    // Attributes
private:
    double   m_dSemitoneOffset;          // for adjusting Melogram and TWC y-scale (Semitone #)
    BOOL     m_bSemitoneOffsetAdjusted;  // to indicate whether or not m_fSemitoneOffset has been adjusted yet.

    // Operations
public:
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);
    void AdjustSemitoneOffset(WORD UpDown, BOOL bKeyShifted);
    double SetSemitoneOffset(double dSemitoneOffset);
    double GetSemitoneOffset() const
    {
        return m_dSemitoneOffset;
    }
    virtual CGrid GetGrid() const;

private:
    void PaintHelper(CDC * pDC, CRect rWnd, CRect rClip,
                     CProcessTonalWeightChart * pProcess, CSaDoc * pProcessDoc, int nFlags);


    // Generated message map functions
protected:
    //{{AFX_MSG(CPlotTonalWeightChart)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif //_sa_g_twc_H
