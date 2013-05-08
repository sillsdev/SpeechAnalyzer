/////////////////////////////////////////////////////////////////////////////
// sa_g_mag.h:
// Interface of the CPlotMagnitude class
// Author: Todd Jones
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _sa_g_mag_H
#define _sa_g_mag_H

//###########################################################################
// CPlotMagnitude plot window

class CPlotMagnitude : public CPlotWnd
{
    DECLARE_DYNCREATE(CPlotMagnitude)

// Construction/destruction/creation
public:
    CPlotMagnitude();
    virtual ~CPlotMagnitude();
    virtual CPlotWnd * NewCopy(void);
    virtual void CopyTo(CPlotWnd * pTarg);

    // Attributes
private:

    // Operations
public:
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

protected:
    DECLARE_MESSAGE_MAP()
};

#endif //_sa_g_mag_H
