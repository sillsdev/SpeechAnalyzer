//***************************************************************************
//* sa_g_wavelet.h (v1.0)                                                   *
//* ----------------                                                        *
//* Project          : Speech Analyzer                                      *
//* Author           : Andy Heitke                                          *
//* Date Started     : 6/11/01                                              *
//* Customer Name    : JAARS / SIL                                          *
//* Description      : This is the main header file for the Wavelet         *
//*                    graph in SA                                          *
//* Revision History : 7/30/01 ARH - Imported file into Speech Analyzer     *
//*                                  v2.0 project                           *
//***************************************************************************
#ifndef SA_G_WAVELET_H
#define SA_G_WAVELET_H

#define sparsePaletteSize 30l                   // SDM 1.5Test10.3

#define IDM_BUTTON1     300                     // used for the little arrow buttons on the side of the graph
#define IDM_BUTTON2     301                     // these could be used for implementing a time/frequency zoom
// but currently control the "red line" showing the different
// energy bands

typedef unsigned char UBYTE;

struct RGB
{
    long r;
    long g;
    long b;
};

//**************************************************************************
// CPlotWavelet Class
//**************************************************************************
class CPlotWavelet : public CPlotWnd
{
    DECLARE_DYNCREATE(CPlotWavelet)

private:
    enum PaletteMode { SYSTEMCOLOR, HALFCOLOR, FULLCOLOR};

    static BOOL bPaletteInit;                                 // TRUE, if palette initialized
    static int nPaletteMode;                                  // mode of created palette
    static CPalette SpectroPalette;                       // color palette

    long drawing_level;                                             // This variable gets updated by the arrow buttons on the legend
    // and tells the red line which level to draw
    long max_drawing_level;


private:
    BOOL CreateSpectroPalette(CDC * pDC, CDocument * pDoc); // creates the palette
    void populateBmiColors(RGBQUAD * Quadcolors,CSaView * pView);

public:
    CPlotWavelet();

    void IncreaseDrawingLevel()
    {
        if (drawing_level < max_drawing_level)
        {
            drawing_level++;
        }
    }
    void DecreaseDrawingLevel()
    {
        if (drawing_level > 1)
        {
            drawing_level--;
        }
    }


    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);
    virtual ~CPlotWavelet();

protected:
    DECLARE_MESSAGE_MAP()
};

#endif
