/////////////////////////////////////////////////////////////////////////////
// sa_gZpol.h:
// Interface of the CZPlotPolar
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_GZPOL_H
#define _SA_GZPOL_H

//###########################################################################
// CZPlotPolar plot window

class CZPlotPolar : public CPlotWnd {
    // Construction/destruction/creation
public:
    CZPlotPolar(int nIndexSDP);
    virtual ~CZPlotPolar();
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

    // Attributes
private:
    HANDLE  m_hDataX;          // needed to get m_lpDataX
    BPTR   m_lpDataX;         // pointer to Z-graph data
    HANDLE  m_hDataY;          // needed to get m_lpDataY
    BPTR   m_lpDataY;         // pointer to Z-graph data
    HANDLE  m_hDataSym;        // needed to get m_lpDataSym
    BPTR   m_lpDataSym;       // pointer to Z-graph data
    const int m_nIndexSDP;

    // Operations
private:
    CProcessSDP * GetSDP(CSaDoc * pModel);

    // Generated message map functions
protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    DECLARE_MESSAGE_MAP()
};

#endif //_SA_GZPOL_H
