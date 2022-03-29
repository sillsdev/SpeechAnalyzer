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
#include "ProgressStatusBar.h"
#include "ThreadManager.h"

class CProcessSpectrogram;
class CProcessSpectroFormants;
class CSaDoc;
class CMainFrame;
struct SaContext;

class CPlotSpectrogram : public CPlotWnd {

    DECLARE_DYNCREATE(CPlotSpectrogram)

public:
    virtual CPlotWnd * NewCopy(void);
    virtual void CopyTo(CPlotWnd * pTarg);
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);
    void SetSpectroAB(char MySpectroAB);
    char GetSpectroAB() const ;
    bool IsRealTime() const;
    virtual BOOL IsAreaGraph() const;

protected:
    virtual BOOL EraseBkgnd(CDC * pDC);

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg BOOL OnSetCursor(CWnd * pWnd, UINT nHitTest, UINT message);
    DECLARE_MESSAGE_MAP()

private:
    BOOL CreateSpectroPalette(CDC * pDC); // creates the palette
    void PopulateBmiColors(RGBQUAD * Quadcolors, SaContext& context);
    bool OnDraw2(std::unique_ptr<CDC> & memDC, CRect rWnd, CRect rClip, SaContext & context);
    void OnDrawSpectrogram(std::unique_ptr<CDC> & memDC, CRect rWnd, CRect rClip, BOOL bSmooth, BOOL * bAliased, SaContext& context);
    void OnDrawFormantTracksFragment(std::unique_ptr<CDC>& memDC, CRect rWnd, CRect rClip, SaContext& context);
    void OnDrawFormantTracksTime(std::unique_ptr<CDC>& memDC, CRect rWnd, CRect rClip, SaContext& context);
    CProcessSpectrogram* GetSpectrogram(CSaDoc& model);

    enum PaletteMode {                      // mode of palette
        SYSTEMCOLOR,
        HALFCOLOR,
        FULLCOLOR,
    };
    static BOOL bPaletteInit;               // TRUE, if palette initialized
    static int nPaletteMode;                // mode of created palette
    static CPalette SpectroPalette;         // color palette
    char SpectroAB;                         // indicates whether the plotted spectrogram is SpectrogramA or SpectrogramB
    thread_handle threadId = -1;            // thread id for formant track processor
};

#endif
