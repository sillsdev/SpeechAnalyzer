/////////////////////////////////////////////////////////////////////////////
// sa_g_cpi.h:
// Interface of the CPlotCustomPitch class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_G_CPI_H
#define _SA_G_CPI_H


//###########################################################################
// CPlotCustomPitch plot window

class CPlotCustomPitch : public CPlotWnd {
    DECLARE_DYNCREATE(CPlotCustomPitch)

// Construction/destruction/creation
public:
    CPlotCustomPitch();
    virtual ~CPlotCustomPitch();

    // Attributes
private:

    // Operations
public:
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);
    virtual CPlotWnd * NewCopy(void);

    // Generated message map functions
protected:
    DECLARE_MESSAGE_MAP()
};

#endif //_SA_G_CPI_H
