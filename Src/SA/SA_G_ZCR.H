////////////////////////////////////////////////////////////////////////////
// sa_g_zcr.h:
// Interface of the CPlotZCross class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_G_ZCR_H
#define _SA_G_ZCR_H

//###########################################################################
// CPlotZCross plot window

class CPlotZCross : public CPlotWnd {
    DECLARE_DYNCREATE(CPlotZCross)

// Construction/destruction/creation
public:
    CPlotZCross();
    virtual ~CPlotZCross();
    virtual CPlotWnd * NewCopy(void);

    // Attributes
private:

    // Operations
public:
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

    // Generated message map functions
protected:
    DECLARE_MESSAGE_MAP()
};

#endif //_SA_G_ZCR_H
