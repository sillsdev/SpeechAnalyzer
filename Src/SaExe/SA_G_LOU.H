/////////////////////////////////////////////////////////////////////////////
// sa_g_lou.h:
// Interface of the CPlotLoudness class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_G_LOU_H
#define _SA_G_LOU_H

//###########################################################################
// CPlotLoudness plot window

class CPlotLoudness : public CPlotWnd
{
    DECLARE_DYNCREATE(CPlotLoudness)

// Construction/destruction/creation
public:
    CPlotLoudness();
    virtual ~CPlotLoudness();
    virtual CPlotWnd * NewCopy(void);
    virtual void CopyTo(CPlotWnd * pTarg);

    // Attributes
private:

    // Operations
public:
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

protected:
    //{{AFX_MSG(CPlotLoudness)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif //_SA_G_LOU_H
