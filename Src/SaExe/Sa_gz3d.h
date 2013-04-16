/////////////////////////////////////////////////////////////////////////////
// sa_gZ3d.h:
// Interface of the CPlot3D
//                  CPlotF1F2
//                  CPlotF2F1
//                  CPlotF2F1F1 classes.
//                  CPlotInvSDP classes.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//   06/15/2000
//       RLJ Changed MAX_NUMBER_VOWELS from 10 to 12, to accommodate
//             addition of /e/ and /o/
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_GZ3D_H
#define _SA_GZ3D_H

#include "sa_plot.h"
#include "DSP/FORMANTS.H"

struct SGraph;
class CScatterGraph;

#define zNUM_POINTS  (65536 / sizeof(double))    // max number of points that can be contained in a memory segment

class CFormantChart : public CPlotWnd
{
    DECLARE_DYNCREATE(CFormantChart)

public:
    CFormantChart();
    virtual ~CFormantChart();

    virtual void GenderInfoChanged(int nGender);
    virtual void SetStartCursor(CSaView * pView);
    virtual void SetStopCursor(CSaView * pView);
    void GraphHasFocus(BOOL bFocus);

protected:
    short int ProcessFTFormants();
    short int ProcessFormants();
    void StandardPlot(CSaView * pView, CDC * pDC, CRect rClient, CRect rClip,
                      SGraph & GraphSettings, BOOL bFlipChart = FALSE, BOOL bShowAxes = FALSE, BOOL bValidData = TRUE);
    POINT GetXYCoordinates(void * pG, double x , double y, BOOL bFlip = FALSE);
    virtual POINT MapVowelsToChart(CDC * /*pDC*/, SFormantFreq & /*VowelFormant*/, int /*VowelSymbolHeight*/, CScatterGraph * /*pFormantChart*/,
                                   SGraph & /*FormantChartParms*/, BOOL /*bFlip*/ = FALSE)
    {
        POINT p = {0,0};
        return p;
    }
    void AnimateFrame(DWORD dwFrameIndex);
    void EndAnimation();

private:
    BOOL m_bVoiced;
    BOOL m_bOutOfRange;

protected:
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CPlot3D plot window

#define MAX_NUMBER_VOWELS 12 // number of vowels shown in graphs

class CPlot3D : public CFormantChart
{
    DECLARE_DYNCREATE(CPlot3D)

// Construction/destruction/creation
public:
    CPlot3D();
    virtual ~CPlot3D();
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

    // Attributes
private:

    // Operations
public:
    static RANGE GetChartRange(int nFormant, int nGender);
protected:
    virtual POINT MapVowelsToChart(CDC * pDC, SFormantFreq & Formant, int SymbolHeight, CScatterGraph * pFormantChart,
                                   SGraph & ChartParms, BOOL /* bFlip */);

    // Generated message map functions
protected:
    //{{AFX_MSG(CPlot3D)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CPlotF1F2 plot window

class CPlotF1F2 : public CFormantChart
{
    DECLARE_DYNCREATE(CPlotF1F2)

// Construction/destruction/creation
public:
    CPlotF1F2();
    virtual ~CPlotF1F2();
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

    // Attributes
private:

    // Operations
public:
    static RANGE GetChartRange(int nFormant, int nGender);
protected:
    virtual POINT MapVowelsToChart(CDC * pDC, SFormantFreq & Formant, int /* SymbolHeight */, CScatterGraph * pFormantChart,
                                   SGraph & /* ChartParms */, BOOL bFlip);

    // Generated message map functions
protected:
    //{{AFX_MSG(CPlotF1F2)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CPlotF2F1 plot window

class CPlotF2F1 : public CFormantChart
{
    DECLARE_DYNCREATE(CPlotF2F1)

// Construction/destruction/creation
public:
    CPlotF2F1();
    virtual ~CPlotF2F1();
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

    // Attributes
private:

    // Operations
public:
    static RANGE GetChartRange(int nFormant, int nGender);
protected:
    virtual POINT MapVowelsToChart(CDC * pDC, SFormantFreq & Formant, int /* SymbolHeight */, CScatterGraph * pFormantChart,
                                   SGraph & /* ChartParms */, BOOL bFlip = FALSE);

    // Generated message map functions
protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CPlotF2F1F1 plot window

class CPlotF2F1F1 : public CFormantChart
{
    DECLARE_DYNCREATE(CPlotF2F1F1)

// Construction/destruction/creation
public:
    CPlotF2F1F1();
    virtual ~CPlotF2F1F1();
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

protected:
    virtual POINT MapVowelsToChart(CDC * pDC, SFormantFreq & Formant, int /* SymbolHeight */, CScatterGraph * pFormantChart,
                                   SGraph & /* ChartParms */, BOOL bFlip);

    // Generated message map functions
protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CPlotInvSDP plot window

class CProcessSDP;
class CPlotInvSDP : public CFormantChart
{
    DECLARE_DYNCREATE(CPlotInvSDP)

// Construction/destruction/creation
public:
    CPlotInvSDP();
    virtual ~CPlotInvSDP();
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

private:
    HANDLE  m_hDataX;          // needed to get m_lpDataX
    HPSTR   m_lpDataX;         // pointer to Z-graph data
    HANDLE  m_hDataY;          // needed to get m_lpDataY
    HPSTR   m_lpDataY;         // pointer to Z-graph data
    HANDLE  m_hDataSym;        // needed to get m_lpDataSym
    HPSTR   m_lpDataSym;       // pointer to Z-graph data

    CProcessSDP * GetSDP(CSaDoc * pDoc);

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    DECLARE_MESSAGE_MAP()
};

#endif //_SA_GZ3D_H
