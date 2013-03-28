/////////////////////////////////////////////////////////////////////////////
// sa_g_spg.h:
// Interface of the CPlotSpectrogram classes.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
//
// Revision History
//   1.5Test11.1A
//        RLJ Added SpectroType enum and SpectroAB attribute.
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_G_SPG_H
#define _SA_G_SPG_H

#include "Sa_plot.h"

class CProcessSpectrogram;
class CProcessSpectroFormants;

class CPlotSpectrogram : public CPlotWnd
{

    DECLARE_DYNCREATE(CPlotSpectrogram)

public:
    CPlotSpectrogram();
    virtual ~CPlotSpectrogram();

    virtual CPlotWnd * NewCopy(void);
    virtual void CopyTo(CPlotWnd * pTarg);
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);
    void SetSpectroAB(char MySpectroAB);
    char GetSpectroAB() const ;
    bool IsRealTime() const;
    virtual BOOL IsAreaGraph() const;

protected:
    virtual BOOL  EraseBkgnd(CDC * pDC);

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg BOOL OnSetCursor(CWnd * pWnd, UINT nHitTest, UINT message);
    DECLARE_MESSAGE_MAP()

private:
    BOOL CreateSpectroPalette(CDC * pDC, CDocument * pDoc); // creates the palette
    void populateBmiColors(RGBQUAD * Quadcolors,CSaView * pView);
    BOOL OnDraw2(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);
    BOOL OnDrawSpectrogram(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView, BOOL bSmooth, BOOL * bAliased = NULL);
    BOOL OnDrawFormantTracks(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);
    BOOL OnDrawFormantTracksFragment(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);
    BOOL OnDrawFormantTracksTime(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);
    CProcessSpectrogram * GetSpectrogram(CSaDoc * pDoc);
    CProcessSpectroFormants * GetFormantProcess(CSaDoc * pDoc);

    enum PaletteMode                          // mode of palette
    {
        SYSTEMCOLOR,
        HALFCOLOR,
        FULLCOLOR,
    };
    static BOOL bPaletteInit;              // TRUE, if palette initialized
    static int nPaletteMode;        // mode of created palette
    static CPalette SpectroPalette;     // color palette
    char SpectroAB;                       // indicates whether the plotted spectrogram is SpectrogramA or SpectrogramB

};

#endif
