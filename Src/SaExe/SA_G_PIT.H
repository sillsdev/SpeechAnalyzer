/////////////////////////////////////////////////////////////////////////////
// sa_g_pit.h:
// Interface of the CPlotPitch class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_G_PIT_H
#define _SA_G_PIT_H

//###########################################################################
// CPlotPitch plot window

class CPlotPitch : public CPlotWnd
{
    DECLARE_DYNCREATE(CPlotPitch)

// Construction/destruction/creation
public:
    CPlotPitch();
    virtual ~CPlotPitch();

    // Attributes
private:

    // Operations
public:
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);
    virtual CPlotWnd * NewCopy(void);

    // Generated message map functions
protected:
    //{{AFX_MSG(CPlotPitch)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif //_SA_G_PIT_H
