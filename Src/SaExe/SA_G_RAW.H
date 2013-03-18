/////////////////////////////////////////////////////////////////////////////
// sa_g_raw.h:
// Interface of the CPlotRawData class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_G_RAW_H
#define _SA_G_RAW_H

//###########################################################################
// CPlotRawData plot window

class CPlotRawData : public CPlotWnd {
    DECLARE_DYNCREATE(CPlotRawData)

// Construction/destruction/creation
public:
    CPlotRawData();
    virtual ~CPlotRawData();
    virtual CPlotWnd * NewCopy(void);
    virtual void CopyTo(CPlotWnd * pTarg);


    // Attributes
private:

    // Operations
public:
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

protected:
    virtual BOOL SetLegendScale();                             // 08/29/2000 - DDO

    // Generated message map functions
protected:
    //{{AFX_MSG(CPlotRawData)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif //_SA_G_RAW_H
