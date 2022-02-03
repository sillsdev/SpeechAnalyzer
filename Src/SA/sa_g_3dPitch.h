#ifndef SA_G_3DPITCH_H
#define SA_G_3DPITCH_H

/////////////////////////////////////////////////////////////////////////////
// CPlot3dPitch command target

class CProcess3dPitch;

class CPlot3dPitch : public CPlotWnd {
    DECLARE_DYNCREATE(CPlot3dPitch)
public:
    CPlot3dPitch();

private:
    enum PaletteMode {                        // mode of palette
        SYSTEMCOLOR,
        HALFCOLOR,
        FULLCOLOR,
    };
    static BOOL bPaletteInit;              // TRUE, if palette initialized
    static int nPaletteMode;        // mode of created palette
    static CPalette SpectroPalette;     // color palette

    CProcess3dPitch * m_p3dPitch;

private:
    BOOL CreateSpectroPalette(CDC * pDC, CDocument * pModel); // creates the palette
    void populateBmiColors(RGBQUAD * Quadcolors,CSaView * pView);
    BOOL OnDraw2(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);
    BOOL OnDrawCorrelations(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

public:
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

public:
    virtual ~CPlot3dPitch();
    virtual BOOL  EraseBkgnd(CDC * /*pDC*/) {
        return TRUE;   // we dont need to erase this plot
    }

protected:
    DECLARE_MESSAGE_MAP()
};

#endif
