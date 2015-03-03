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

class CPlotPitch : public CPlotWnd {
    DECLARE_DYNCREATE(CPlotPitch)

public:
    CPlotPitch();
    virtual ~CPlotPitch();

    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);
    virtual CPlotWnd * NewCopy(void);

protected:
    DECLARE_MESSAGE_MAP()
};

#endif
