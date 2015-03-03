/////////////////////////////////////////////////////////////////////////////
// sa_g_poa.h:
// Interface of the CPlotPOA class
// Author: Alec Epting
// copyright 1999 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_G_POA_H
#define _SA_G_POA_H

//###########################################################################
// CPlotPOA plot window

class CPlotPOA : public CPlotWnd {
    DECLARE_DYNCREATE(CPlotPOA)

// Construction/destruction/creation
public:
    CPlotPOA();
    virtual ~CPlotPOA();

    // Attributes
private:
    DWORD   m_dwFrameStart;
    BOOL    m_bShowModelData;

    // Operations
public:
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);
    void GraphHasFocus(BOOL bFocus);
    void AnimateFrame(DWORD dwFrameIndex);          // animate a single frame (fragment)
    void EndAnimation();                            // terminate animation


    // Generated message map functions
protected:
    DECLARE_MESSAGE_MAP()
};

#endif //_SA_G_POA_H
