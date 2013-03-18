// sa_g_3dPitch.cpp : implementation file
//

#include "stdafx.h"
#include "sa.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "mainfrm.h"
#include "sa_plot.h"
#include "sa_graph.h"
#include "SA_G_3dPitch.h"
#include "Process\sa_p_3dPitch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef unsigned char UBYTE;

/////////////////////////////////////////////////////////////////////////////
// CPlot3dPitch

CPlot3dPitch::CPlot3dPitch() {
    m_p3dPitch = new CProcess3dPitch;
    m_pLastProcess = m_p3dPitch;
}

CPlot3dPitch::~CPlot3dPitch() {
    delete m_p3dPitch;
}


IMPLEMENT_DYNCREATE(CPlot3dPitch, CPlotWnd)

/////////////////////////////////////////////////////////////////////////////
// CPlot3dPitch static member definition

BOOL CPlot3dPitch::bPaletteInit = FALSE;  // palette not initialized yet
int CPlot3dPitch::nPaletteMode = SYSTEMCOLOR; // use system colors only
CPalette CPlot3dPitch::SpectroPalette; // color palette

/////////////////////////////////////////////////////////////////////////////
// CPlot3dPitch message map

BEGIN_MESSAGE_MAP(CPlot3dPitch, CPlotWnd)
    //{{AFX_MSG_MAP(CPlot3dPitch)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

// SDM 1.5Test10.3
#define sparsePaletteSize 30l

// SDM 1.5Test10.3
struct RGB {
    long r;
    long g;
    long b;
};

// SDM 1.5Test10.3
static struct RGB sparsePalette[sparsePaletteSize+1] = {
    {    0,    0,    0},// Black
    {   70,    0,   90},// Dark Purple
    {   87,    0,  114},// Purple
    {   74,    0,  138},
    {   60,    0,  160},
    {    0,    0,  170},// Dark Blue
    {    0,    0,  200},
    {    0,    0,  220},
    {    0,    0,  255},// Blue
    {    0,   70,  255},
    {    0,  150,  255},
    {    0,  200,  255},
    {    0,  220,  255},
    {    0,  255,  255},// Cyan
    {    0,  255,  180},
    {    0,  255,  150},
    {    0,  255,  130},
    {    0,  255,    0},// Green
    {  135,  255,    0},
    {  173,  255,    0},
    {  210,  255,    0},
    {  255,  255,    0},// Yellow
    {  255,  234,    0},
    {  255,  210,    0},
    {  255,  181,    0},
    {  255,  140,    0},//Orange
    {  255,   90,    0},
    {  255,    0,    0},// Red
    {  255,    0,  100},
    {  255,    0,  155},
    {  255,    0,  255} // Magenta
};

static unsigned char sparseBlack[] = {
    255, // Black
    250,
    245, // Dark Purple
    240, // Purple
    238,
    236,
    233, // Dark Blue
    230,
    227,
    224, // Blue
    218,
    212,
    206,
    199,
    192, // Cyan
    184,
    176,
    168,
    160, // Green
    152,
    144,
    136,
    128, // Yellow
    122,
    116,
    109,
    102, //Orange
    89,
    76 , // Red
    63,
    50 ,
    0    // Magenta
};

/////////////////////////////////////////////////////////////////////////////
// CPlot3dPitch helper functions

/***************************************************************************/
// CPlot3dPitch::CreateSpectroPalette Creates the palette for spectrogram
// This function creates the colors needed by the spectrogram. If the system
// supports only 16 colors, it won't do anything and the spectrogram will
// work with this 16 colors. If it supports 256 colors, this function creates
// a 117 grayscale and above a 117 color palette (total 236 colors available
// in system palette). If there are more than 256 colors supported, this
// function creates the full 233 grayscale and above the 233 color palette.
// The function returns FALSE in case on error, else TRUE.
/***************************************************************************/
BOOL CPlot3dPitch::CreateSpectroPalette(CDC * pDC, CDocument * /*pSaDoc*/) {
#define SET_RGB(r,g,b) ((PC_NOCOLLAPSE << 24) | RGB(r,g,b)) // PC_NOCOLLAPSE make new entry if possible

    // get device capabilities
    int nRasterCaps = pDC->GetDeviceCaps(RASTERCAPS);
    // SDM 1.5Test10.7 rebuild palette on DeviceCaps() change
    if ((nRasterCaps & RC_BITBLT) == 0) {
        nPaletteMode = SYSTEMCOLOR; // use system colors only
        return FALSE; // device is not able to handle raster operations
    }

    if (nRasterCaps & RC_PALETTE) {
        nRasterCaps = TRUE;
    } else {
        nRasterCaps = FALSE;
    }
    int nNumColors;
    if (nRasterCaps) {
        nNumColors = pDC->GetDeviceCaps(SIZEPALETTE);
    } else {
        nNumColors = pDC->GetDeviceCaps(NUMCOLORS);
    }

    if (nNumColors == -1) {
        int nBits = pDC->GetDeviceCaps(BITSPIXEL);
        nNumColors = 1 << nBits;
    }

    int nDesiredPaletteMode = HALFCOLOR;
    if (nNumColors < 256) {
        nDesiredPaletteMode = SYSTEMCOLOR; // use system colors only
    }
    if (nNumColors > 256) {
        nDesiredPaletteMode = FULLCOLOR;
    }

    if (nDesiredPaletteMode != nPaletteMode) { // not yet compatible with this context
        if (nNumColors < 256) {
            nPaletteMode = SYSTEMCOLOR; // use system colors only
            return TRUE; // less than 256 colors supported, just use system colors
        }
        int nPaletteSize = 117;
        nPaletteMode = HALFCOLOR;
        if (nNumColors > 256) {
            // more than 256 colors supported
            nPaletteSize = 233;
            nPaletteMode = FULLCOLOR;
        }

        // get pointer to plot colors
        // create palette
        LPLOGPALETTE lpLogPalette = (LPLOGPALETTE)new char[sizeof(LOGPALETTE)
                                    + (2 * nPaletteSize - 1) * sizeof(PALETTEENTRY)];
        if (!lpLogPalette) {
            return FALSE;
        }
        lpLogPalette->palVersion = 0x300;
        lpLogPalette->palNumEntries = WORD(2 * nPaletteSize);
        if (!bPaletteInit) {
            if (!SpectroPalette.CreatePalette(lpLogPalette)) {
                return FALSE;
            }
        } else {
            if (!SpectroPalette.ResizePalette(2*nPaletteSize)) {
                return FALSE;
            }
        }
        WORD wColorIndex;
        long lRed, lGreen, lBlue, lGrayLevel;
        // create grayscale palette
        for (wColorIndex = 0; (int)wColorIndex < nPaletteSize; wColorIndex++) {
            lGrayLevel = 255 - (long)(((double)wColorIndex * 255.) / (double)nPaletteSize + 0.5);
            *(unsigned long *)&lpLogPalette->palPalEntry[wColorIndex] = SET_RGB(lGrayLevel, lGrayLevel, lGrayLevel);
        }
        // create color palette // SDM 1.5Test10.3
        for (wColorIndex = 0; (int)wColorIndex < nPaletteSize; wColorIndex++) {
            long lowSparseIndex = wColorIndex * sparsePaletteSize / nPaletteSize;
            long remainderSparseIndex = (wColorIndex * sparsePaletteSize) % nPaletteSize;

            // interpolate colors
            lRed = (sparsePalette[lowSparseIndex+1].r * remainderSparseIndex
                    + sparsePalette[lowSparseIndex].r * (nPaletteSize - remainderSparseIndex))/ nPaletteSize;

            lGreen = (sparsePalette[lowSparseIndex+1].g * remainderSparseIndex
                      + sparsePalette[lowSparseIndex].g * (nPaletteSize - remainderSparseIndex))/ nPaletteSize;

            lBlue = (sparsePalette[lowSparseIndex+1].b * remainderSparseIndex
                     + sparsePalette[lowSparseIndex].b * (nPaletteSize - remainderSparseIndex))/ nPaletteSize;

            *(unsigned long *)&lpLogPalette->palPalEntry[wColorIndex+nPaletteSize] = SET_RGB(lRed, lGreen, lBlue);
        }
        SpectroPalette.SetPaletteEntries(0, 2 * nPaletteSize, lpLogPalette->palPalEntry);
        delete [] lpLogPalette;
        bPaletteInit = TRUE;
    }
    // select the new palette
    CPalette * pOldSysPalette;
    pOldSysPalette = pDC->SelectPalette(&SpectroPalette, FALSE);
    if (pOldSysPalette) { // SDM 1.5Test11.32
        pOldSysPalette->UnrealizeObject();
    }
    pDC->RealizePalette();
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CPlot3dPitch message handlers

void CPlot3dPitch::populateBmiColors(RGBQUAD * QuadColors, CSaView * /*pView*/) {
    CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
    // prepare to draw
    COLORREF Color[256];
    // get pointer to plot colors
    Colors * pColors = pMain->GetColors();
    // create palette
    Color[235] = pColors->cPlotBkg;  // background

    int bGrayScale = FALSE;
    int nMinThreshold = 0;
    int nMaxThreshold = 234;
    if (bGrayScale) {
        // create grayscale palette
        Color[0] = RGB(255, 255, 255);   // white
        Color[234] = RGB(0, 0, 0);     // black
        unsigned int nPaletteSize = 234;
        // use closest system gray
        for (WORD wColorIndex = 1; wColorIndex < 234; wColorIndex++) {
            if (wColorIndex <= (WORD)nMinThreshold) {
                Color[wColorIndex] = RGB(255, 255, 255);
            } else if (wColorIndex >= (WORD)nMaxThreshold) {
                Color[wColorIndex] = RGB(0, 0, 0);
            } else {
                long intermediate = (wColorIndex - nMinThreshold) * (sizeof(sparseBlack)-1) * 234
                                    / (nMaxThreshold - nMinThreshold);
                long lowSparseIndex = intermediate  / nPaletteSize ;
                long remainderSparseIndex = intermediate % nPaletteSize;

                // interpolate colors
                long lGray = sparseBlack[lowSparseIndex+1] * remainderSparseIndex / nPaletteSize
                             + sparseBlack[lowSparseIndex] * (nPaletteSize - remainderSparseIndex) / nPaletteSize;

                Color[wColorIndex] = RGB(lGray, lGray, lGray);
            }
        }
    } else {
        // create color map
        // SDM 1.5Test10.3
        Color[0] = RGB(0, 0, 0);       // black
        Color[234] = RGB(255,  0, 255);     // magenta
        long lRed, lGreen, lBlue;
        unsigned int nPaletteSize = 234;

        // use closest system color
        for (WORD wColorIndex = 0; wColorIndex < nPaletteSize; wColorIndex++) {
            if (wColorIndex <= (WORD)nMinThreshold) {
                Color[wColorIndex] = RGB(0, 0, 0);
            } else if (wColorIndex >= (WORD)nMaxThreshold) {
                Color[wColorIndex] = RGB(255, 0, 255);
            } else {
                long intermediate = (wColorIndex - nMinThreshold) * sparsePaletteSize * 234
                                    / (nMaxThreshold - nMinThreshold);
                long lowSparseIndex = intermediate  / nPaletteSize ;
                long remainderSparseIndex = intermediate % nPaletteSize;


                // interpolate colors
                lRed = sparsePalette[lowSparseIndex+1].r * remainderSparseIndex / nPaletteSize
                       + sparsePalette[lowSparseIndex].r * (nPaletteSize - remainderSparseIndex) / nPaletteSize;

                lGreen = sparsePalette[lowSparseIndex+1].g * remainderSparseIndex / nPaletteSize
                         + sparsePalette[lowSparseIndex].g * (nPaletteSize - remainderSparseIndex) / nPaletteSize;

                lBlue = sparsePalette[lowSparseIndex+1].b * remainderSparseIndex / nPaletteSize
                        + sparsePalette[lowSparseIndex].b * (nPaletteSize - remainderSparseIndex) / nPaletteSize;

                Color[wColorIndex] = RGB(lRed, lGreen, lBlue);
            }
        }
    }

    for (int i = 0; i < 256; i++) {
        QuadColors[i].rgbBlue = GetBValue(Color[i]);
        QuadColors[i].rgbGreen = GetGValue(Color[i]);
        QuadColors[i].rgbRed = GetRValue(Color[i]);
        QuadColors[i].rgbReserved = 0;
    }
}

/***************************************************************************/
// CPlot3dPitch::OnDraw Drawing
// The data to draw is coming from a temporary file, created by the
// spectrogram processing class, which is called to do data processing if
// necessary before drawing. Call the function PlotPaintFinish at the end of
// the drawing to let the plot base class do common jobs like drawing the
// cursors.
/***************************************************************************/
BOOL CPlot3dPitch::OnDrawCorrelations(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView) {
    // get pointer to graph, view, document, application and mainframe
    CSaDoc  *  pDoc   = pView->GetDocument();

    BITMAP * pBitmap = new BITMAP;
    HBITMAP hBitmap = (HBITMAP) GetCurrentObject(pDC->GetSafeHdc(),OBJ_BITMAP);

    GetObject(hBitmap,sizeof(BITMAP),pBitmap);

    // get spectrogram dimensions
    int nHeight = 0;
    for (DWORD dwIntegerPitchPeriod = 22; dwIntegerPitchPeriod < 480; dwIntegerPitchPeriod+= 5 /*dwIntegerPitchPeriod/16*/) {
        nHeight++;
    }

    int nWidth = m_p3dPitch->GetDataSize()/nHeight;
    double fDataStart = pView->GetDataPosition(rWnd.Width()); // data index of first sample to display
    double fDataLength = pView->GetDataFrame(); // number of data points to display

    fDataStart *= double(nWidth)/pDoc->GetDataSize();
    fDataLength *= double(nWidth)/pDoc->GetDataSize();
    double fWidthFactor = double(rWnd.Width())/fDataLength;

    double fHeightFactor = (double)rWnd.Height() / (double)nHeight;

    // draw the spectrogram: right now there is no intelligent stretch algorithm
    // built in. If the graph is larger than he was when the data has been
    // processed, then the missing pixels just will be copied from existing data
    // points. If he is smaller, data points will be skipped.
    double fYPix = 0.;
    int nNextYPix = 0;
    for (int y = 0; y < nHeight; y++) {
        // spectrogram print needed
        int nYPix = nNextYPix;
        fYPix += fHeightFactor;
        nNextYPix = (int)(fYPix);
        while (nYPix < nNextYPix) {
            double fXPix = (double)-fDataStart*fWidthFactor;;
            int nNextXPix = (int)fXPix;
            UBYTE * pRow = ((UBYTE *) pBitmap->bmBits)  + pBitmap->bmWidthBytes*nYPix;
            for (int x = 0; x < nWidth; x++) {
                int nXPix = nNextXPix;
                fXPix += fWidthFactor;
                nNextXPix = (int)(fXPix);
                if (nXPix < rClip.left) {
                    continue;
                }

                BOOL bResult;
                short correlationK = (short) m_p3dPitch->GetProcessedData(x + y*nWidth, &bResult);
                UBYTE ubColor = BYTE(correlationK > 0 ? correlationK*234/1024 : 0);
                while (nXPix < nNextXPix) {
                    if (nXPix > rClip.right) {
                        break;
                    }

                    pRow[nXPix] = ubColor;

                    nXPix++;
                }
            }
            nYPix++;
        }
    }

    if (pBitmap) {
        delete pBitmap;
    }

    // do common plot paint jobs
    PlotPaintFinish(pDC, rWnd, rClip);

    return TRUE;
}

BOOL CPlot3dPitch::OnDraw2(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView) {
    if (IsIconic()) {
        return FALSE;    // nothing to draw
    }
    // get pointer to graph, view, document, application and mainframe
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaDoc  *  pDoc   = pView->GetDocument();

    if (rWnd.Height() <= 0) {
        return FALSE;    // nothing to draw
    }

    // check if process is idle
    {
        // create spectrogram data
        short int nResult = LOWORD(m_p3dPitch->Process(this, pDoc)); // process data
        nResult = CheckResult(nResult, m_p3dPitch); // check the process result
        if (nResult == PROCESS_ERROR) {
            return FALSE;
        }
        if (nResult != PROCESS_CANCELED) {
            if (nResult) {
                // new data processed, all has to be displayed
                Invalidate();
                return FALSE;
            }
            // set legend scale
            pGraph->SetLegendScale(SCALE | NUMBERS, 22, 480, _T("integer pitch samples")); // set legend scale
            // prepare to draw

            GdiFlush();

            OnDrawCorrelations(pDC, rWnd, rClip, pView);

            m_HelperWnd.SetMode(MODE_HIDDEN);
        }
    }
    return TRUE;
}


void CPlot3dPitch::OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView) {
    CSaApp * pApp = (CSaApp *)AfxGetApp();
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaDoc * pDoc = pView->GetDocument();

    if (!CreateSpectroPalette(pDC, pDoc)) {
        // error creating color palette
        pApp->ErrorMessage(IDS_ERROR_SPECTROPALETTE);
        pGraph->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
        return;
    }

    // create a temporary DC for the reading the screen
    CDC * pMemDC = new CDC;

    // create a bitmap to read the screen into and select it into the temporary DC
    BITMAPINFO * pInfo = (BITMAPINFO *) malloc(sizeof(BITMAPINFO) + 256*sizeof(RGBQUAD));

    if (pMemDC && pInfo && pMemDC->CreateCompatibleDC(NULL)) {
        pInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        pInfo->bmiHeader.biWidth = rWnd.Width();
        pInfo->bmiHeader.biHeight = rWnd.Height(); // Bit map is top down (hence negative sign)
        pInfo->bmiHeader.biPlanes = 1;
        pInfo->bmiHeader.biBitCount = 8;
        pInfo->bmiHeader.biCompression = BI_RGB;
        pInfo->bmiHeader.biSizeImage = 0; // set to zero for uncompressed (BI_RGB)
        pInfo->bmiHeader.biXPelsPerMeter = pInfo->bmiHeader.biYPelsPerMeter = 2835; // 72 DPI
        pInfo->bmiHeader.biClrUsed = 0; // fully populated bmiColors
        pInfo->bmiHeader.biClrImportant = 0;
        populateBmiColors(pInfo->bmiColors, pView);

        void * pBits;
        HBITMAP hBitmap = CreateDIBSection(pMemDC->m_hDC,pInfo,DIB_RGB_COLORS, &pBits,NULL,0);

        if (hBitmap) {
            HBITMAP hOldBitmap = (HBITMAP) ::SelectObject(pMemDC->m_hDC,hBitmap);

            //erase background
            CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
            pMemDC->FillSolidRect(rClip, pMain->GetColors()->cPlotBkg);

            // paint the data into the bitmap
            OnDraw2(pMemDC, rWnd, rClip, pView);

            GdiFlush();  // finish all drawing to pMemDC

            // copy to destination
            if (!pDC->BitBlt(rWnd.left,rWnd.top,rWnd.Width(),rWnd.Height(),pMemDC,0,0, SRCCOPY)) {
                CSaString szError;
                szError.Format(_T("BitBLT Failed in ")_T(__FILE__)_T(" line %d"),__LINE__);
                ((CSaApp *)AfxGetApp())->ErrorMessage(szError);
            }

            GdiFlush();  // finish BitBlt before destroying DIBSection

            pMemDC->SelectObject(hOldBitmap);
            DeleteObject(hBitmap);
        } else {
            if (pInfo->bmiHeader.biWidth && pInfo->bmiHeader.biHeight) {
                CSaString szError;
                szError.Format(_T("CreateDIBSection Failed in ")_T(__FILE__)_T(" line %d"),__LINE__);
                ((CSaApp *)AfxGetApp())->ErrorMessage(szError);
            }
        }
    } else {
        if (!pMemDC || !pInfo) {
            CSaString szError;
            szError.Format(_T("memory allocation error in ")_T(__FILE__)_T(" line %d"),__LINE__);
            ((CSaApp *)AfxGetApp())->ErrorMessage(szError);
        } else {
            CSaString szError;
            szError.Format(_T("CreateCompatibleDC Failed in ")_T(__FILE__)_T(" line %d"),__LINE__);
            ((CSaApp *)AfxGetApp())->ErrorMessage(szError);
        }
    }
    if (pMemDC) {
        delete pMemDC;
    }
    if (pInfo) {
        delete pInfo;
    }

}
