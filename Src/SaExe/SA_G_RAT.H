/////////////////////////////////////////////////////////////////////////////
// sa_g_rat.h:
// Interface of the CPlotRatio class
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_G_RAT_H
#define _SA_G_RAT_H

//###########################################################################
// CPlotRatio plot window

class CPlotRatio : public CPlotWnd
{
  DECLARE_DYNCREATE(CPlotRatio)

// Construction/destruction/creation
public:
  CPlotRatio();
  virtual ~CPlotRatio();

  // Attributes
private:

  // Operations
public:
  virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

  // Generated message map functions
protected:
  //{{AFX_MSG(CPlotRatio)
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

#endif //_SA_G_RAT_H
