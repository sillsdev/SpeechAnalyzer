/////////////////////////////////////////////////////////////////////////////
// sa_g_spe.h:
// Interface of the CPlotSpecialB
//          CPlotSpecialC
//          CPlotSpecialD
//          CPlotSpecialE
//          CPlotSpecialF
//          CPlotSpecialG
//          CPlotSpecialH
//          CPlotBlank    classes.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_G_SPE_H
#define _SA_G_SPE_H

//###########################################################################
// CPlotBlank plot window

class CPlotBlank : public CPlotWnd
{   DECLARE_DYNCREATE(CPlotBlank)

// Construction/destruction/creation
public:
  CPlotBlank();
  virtual ~CPlotBlank();

  // Attributes
private:

  // Operations
public:
  virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

  // Generated message map functions
protected:
  //{{AFX_MSG(CPlotBlank)
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

#endif //_SA_G_SPE_H
