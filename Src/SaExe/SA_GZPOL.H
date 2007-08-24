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

class CZPlotPolar : public CPlotWnd
{
  // Construction/destruction/creation
public:
  CZPlotPolar(int nIndexSDP);
  virtual ~CZPlotPolar();
  virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

  // Attributes
private:
  HANDLE  m_hDataX;          // needed to get m_lpDataX
  HPSTR   m_lpDataX;         // pointer to Z-graph data
  HANDLE  m_hDataY;          // needed to get m_lpDataY
  HPSTR   m_lpDataY;         // pointer to Z-graph data
  HANDLE  m_hDataSym;        // needed to get m_lpDataSym
  HPSTR   m_lpDataSym;       // pointer to Z-graph data
  const m_nIndexSDP;

  // Operations
private:
  CProcessSDP* GetSDP(CSaDoc *pDoc);

  // Generated message map functions
protected:
  //{{AFX_MSG(CZPlotPolar)
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

#endif //_SA_GZPOL_H
