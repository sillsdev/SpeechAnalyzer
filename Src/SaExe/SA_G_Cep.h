/////////////////////////////////////////////////////////////////////////////
// sa_g_cep.h:
// Interface of the CPlotCepstralPitch class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_G_CEP_H
#define _SA_G_CEP_H

//###########################################################################
// CPlotCepstralPitch plot window

class CPlotCepstralPitch : public CPlotWnd
{   DECLARE_DYNCREATE(CPlotCepstralPitch)

// Construction/destruction/creation
public:
  CPlotCepstralPitch();
  virtual ~CPlotCepstralPitch();

  // Attributes
private:

  // Operations
public:
  virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

  // Generated message map functions
protected:
  //{{AFX_MSG(CPlotCepstralPitch)
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

#endif //_SA_G_CEP_H
