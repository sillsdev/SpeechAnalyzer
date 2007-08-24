/////////////////////////////////////////////////////////////////////////////
// sa_g_spi.h:
// Interface of the CPlotSmoothedPitch class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_G_SPI_H
#define _SA_G_SPI_H

//###########################################################################
// CPlotSmoothedPitch plot window

class CPlotSmoothedPitch : public CPlotWnd
{   DECLARE_DYNCREATE(CPlotSmoothedPitch)

// Construction/destruction/creation
public:
  CPlotSmoothedPitch();
  virtual ~CPlotSmoothedPitch();
  virtual CPlotWnd * NewCopy(void);

  // Attributes
private:

  // Operations
public:
  virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

  // Generated message map functions
protected:
  //{{AFX_MSG(CPlotSmoothedPitch)
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

#endif //_SA_G_SPI_H
