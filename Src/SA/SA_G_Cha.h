/////////////////////////////////////////////////////////////////////////////
// sa_g_cha.h:
// Interface of the CPlotChange class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_G_CHA_H
#define _SA_G_CHA_H


//###########################################################################
// CPlotChange plot window

class CPlotChange : public CPlotWnd {
    DECLARE_DYNCREATE(CPlotChange)

// Construction/destruction/creation
public:
    CPlotChange();
    virtual ~CPlotChange();
    virtual CPlotWnd * NewCopy(void);
    virtual void CopyTo(CPlotWnd * pTarg);


    // Attributes
private:

    // Operations
public:
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

    // Generated message map functions
protected:
    DECLARE_MESSAGE_MAP()
};

#endif //_SA_G_CHA_H
