/////////////////////////////////////////////////////////////////////////////
// sa_g_glo.h:
// Interface of the CPlotGlottis class
// Author: Alec Epting
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_G_GLO_H
#define _SA_G_GLO_H

//###########################################################################
// CPlotGlottis plot window

class CPlotGlottis : public CPlotWnd {
    DECLARE_DYNCREATE(CPlotGlottis)

// Construction/destruction/creation
public:
    CPlotGlottis();
    virtual ~CPlotGlottis();
    virtual CPlotWnd * NewCopy(void);
    virtual void CopyTo(CPlotWnd * pTarg);


    // Attributes

    // Operations
public:
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

    // Generated message map functions
protected:
    DECLARE_MESSAGE_MAP()
};

#endif //_SA_G_GLO_H
