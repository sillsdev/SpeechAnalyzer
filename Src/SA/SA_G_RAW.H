/////////////////////////////////////////////////////////////////////////////
// sa_g_raw.h:
// Interface of the CPlotRawData class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_G_RAW_H
#define _SA_G_RAW_H

class CPlotRawData : public CPlotWnd {

    DECLARE_DYNCREATE(CPlotRawData)

public:
    CPlotRawData();
    virtual ~CPlotRawData();
    virtual CPlotWnd * NewCopy(void);
    virtual void CopyTo(CPlotWnd * pTarg);
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

protected:
    virtual BOOL SetLegendScale();

    DECLARE_MESSAGE_MAP()
};

#endif
