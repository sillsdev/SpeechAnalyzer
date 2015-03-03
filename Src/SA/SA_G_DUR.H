/////////////////////////////////////////////////////////////////////////////
// sa_g_dur.h:
// Interface of the CPlotDurations class
// Author: Alec Epting
// copyright 1998 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_G_DUR_H
#define _SA_G_DUR_H

//###########################################################################
// CPlotDurations plot window

class CProcessDurations;

class CPlotDurations : public CPlotWnd {
    DECLARE_DYNCREATE(CPlotDurations)

// Construction/destruction/creation
public:
    CPlotDurations();
    virtual ~CPlotDurations();
    virtual CPlotWnd * NewCopy(void);
    virtual void CopyTo(CPlotWnd * pTarg);

    //Attributes
private:


    // Operations
public:
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

protected:
    virtual void DurationsPaint(DWORD dwMinData, DWORD dwMaxData, CDC * pDC,
                                CRect rWnd, CRect rClip,
                                CProcess * pDurations, int nFlags = 0); // standard plot painting


    // Generated message map functions
protected:
    DECLARE_MESSAGE_MAP()
};

#endif //_SA_G_DUR_H
