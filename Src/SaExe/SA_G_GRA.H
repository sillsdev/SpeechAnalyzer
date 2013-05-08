/////////////////////////////////////////////////////////////////////////////
// sa_g_gra.h:
// Interface of the CPlotGrappl class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_G_GRA_H
#define _SA_G_GRA_H

//###########################################################################
// CPlotGrappl plot window

class CPlotGrappl : public CPlotWnd
{
    DECLARE_DYNCREATE(CPlotGrappl)

// Construction/destruction/creation
public:
    CPlotGrappl();
    virtual ~CPlotGrappl();
    virtual CPlotWnd * NewCopy(void);

    // Attributes
private:

    // Operations
public:
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

protected:

    // Generated message map functions
protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    DECLARE_MESSAGE_MAP()
};

#endif //_SA_G_GRA_H
