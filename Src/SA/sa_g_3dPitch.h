#ifndef SA_G_3DPITCH_H
#define SA_G_3DPITCH_H

/////////////////////////////////////////////////////////////////////////////
// CPlot3dPitch command target

class CProcess3dPitch;

class CPlot3dPitch : public CPlotWnd {
    DECLARE_DYNCREATE(CPlot3dPitch)
public:
    CPlot3dPitch();

    virtual BOOL  EraseBkgnd(CDC* /*pDC*/) {
        return TRUE;   // we dont need to erase this plot
    }
    virtual void OnDraw(CDC* pDC, CRect rWnd, CRect rClip, CSaView * pView);

    DECLARE_MESSAGE_MAP()

private:
    BOOL CreateSpectroPalette(CDC* pDC, CDocument* pModel); // creates the palette
    void PopulateBmiColors(RGBQUAD* Quadcolors);
    BOOL OnDraw2(CDC* pDC, CRect rWnd, CRect rClip, CSaView& view, CSaDoc& doc);
    BOOL OnDrawCorrelations(CDC* pDC, CRect rWnd, CRect rClip, CSaView& view, CSaDoc& doc);

    enum PaletteMode {                      // mode of palette
        SYSTEMCOLOR,
        HALFCOLOR,
        FULLCOLOR,
    };
    static BOOL bPaletteInit;               // TRUE, if palette initialized
    static int nPaletteMode;                // mode of created palette
    static CPalette SpectroPalette;         // color palette

    unique_ptr<CProcess3dPitch> m_p3dPitch;

};

#endif
