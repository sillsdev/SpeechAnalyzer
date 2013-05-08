/////////////////////////////////////////////////////////////////////////////
// sa_g_spu.h:
// Interface of the CPlotSpectrum class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_G_SPU_H
#define _SA_G_SPU_H

//###########################################################################
// CPlotSpectrum plot window

class CPlotSpectrum : public CPlotWnd
{
    DECLARE_DYNCREATE(CPlotSpectrum)

// Construction/destruction/creation
public:
    CPlotSpectrum();
    virtual ~CPlotSpectrum();

    // Attributes
public:
    enum FreqScale {HALF_SCALE = 0, FULL_SCALE = 1};
private:

    // Operations
public:
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);
    virtual void AnimateFrame(DWORD dwFrameIndex);
    virtual void EndAnimation();

    // Generated message map functions
protected:
    DECLARE_MESSAGE_MAP()
};

#endif //_SA_G_SPU_H
