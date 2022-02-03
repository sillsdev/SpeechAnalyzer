//***************************************************************************
//* sa_g_wavelet.cpp (v2.0)
//* ----------------
//* Project          : Speech Analyzer
//* Author           : Andy Heitke heit0050@umn.edu
//* Date Started     : 6/11/01
//* Customer Name    : JAARS / SIL
//* Description      : This is the main implementation file for the Wavelet
//*                    graph in SA
//* Revision History : 6/11/01 ARH Created this file
//*                    6/14/01 ARH Pounded head against wall until raw data
//*                                came out
//*                    6/22/01 ARH Completed Tree data structure
//*                    6/26/01 ARH Fixed convolution to center around the
//*                              data point
//*                    6/29/01 ARH Created "data filters"
//*                    7/30/01 ARH - Imported file into Speech Analyzer
//* Todo         : - Fix onDraw to allow scrolling
//*                - Fix undersampling
//*                - Scale data relatively
//*
//* Notes        : - There are three terms used in this code: tree,
//*                  node, and leaf.  A tree contains multiple nodes
//*                  and a node that terminates with nothing below is
//*                  a leaf.
//***************************************************************************
#include "stdafx.h"
#include "sa.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "mainfrm.h"
#include "sa_plot.h"
#include "sa_graph.h"
#include "sa_g_wavelet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(push, 3)
#include <stack>
#pragma warning(pop)

BEGIN_MESSAGE_MAP(CPlotWavelet, CPlotWnd)
    ON_WM_CREATE()
END_MESSAGE_MAP()

using namespace std;

static struct RGB sparsePalette[sparsePaletteSize+1] = {    // SDM 1.5Test10.3
    {0,0,0}, {70,0,90}, {87,0,114}, {74,0,138}, {60,0,160}, {0,0,170},
    {0,0,200}, {0,0,220}, {0,0,255}, {0,70,255}, {0,150,255}, {0,200,255},
    {0,220,255}, {0,255,255}, {0,255,180}, {0,255,150}, {0,255,130},
    {0,255,0}, {135,255,0}, {173,255,0}, {210,255,0}, {255,255,0}, {255,234,0},
    {255,210,0}, {255,181,0}, {255,140,0}, {255,90,0}, {255,0,0}, {255,0,100},
    {255,0,155}, {255,0,255}
};

static unsigned char sparseBlack[] = {
    255, 250, 245, 240, 238, 236, 233, 230, 227, 224, 218, 212,
    206, 199, 192, 184, 176, 168, 160, 152, 144, 136, 128, 122,
    116, 109, 102, 89, 76 , 63, 50 , 0
};

BOOL     CPlotWavelet::bPaletteInit = FALSE;                // palette not initialized yet
int      CPlotWavelet::nPaletteMode = SYSTEMCOLOR;  // use system colors only
CPalette CPlotWavelet::SpectroPalette;                          // color palette

extern CSaApp NEAR theApp;

CPlotWavelet::CPlotWavelet() {
    drawing_level = 1;
}

CPlotWavelet::~CPlotWavelet() {
}

IMPLEMENT_DYNCREATE(CPlotWavelet, CPlotWnd)

// ScatterPlotTree
//*  Description   : Does not scatter plot the entire graph, but rather does
//*                                  a scatter plot of the xth leaf determined by "which_leaf"
//*  Parameters    : CDC *pDC - pointer to the device context to draw on
//*                                  CRect *rWnd - the rectangle to draw into
//*                                  COLORREF crColor - What color to draw with
//*                                  long which_leaf - how many leafs to skip before drawing
//*                                  double start - where to start in the data
//*                                  double end - where to end in the data
//*  Preconditions : Data should be allocated and populated to all leaves
//*  Postcondtions : ScatterPlotDataNode is called on the correct node
//*  Returns       : TRUE - on success
//*                                  FALSE - otherwise
//**************************************************************************
BOOL CPlotWavelet::ScatterPlotDataTree(CWaveletNode* root,CDC* pDC, CRect* rWnd, COLORREF crColor, long which_leaf, double start, double end) {
    CWaveletNode* node_ptr = root->GetNode(which_leaf);
    if (node_ptr == NULL) {
        return FALSE;
    }
    return node_ptr->ScatterPlotDataNode(root, pDC, rWnd, crColor, start, end);
}

// ScatterPlotDataNode
//*  Description   : Draw a simple scatter plot of node data
//*  Parameters    : CDC *pDC - where to draw the data
//*                                CRect *rWnd - The rectangle to draw within
//*                                COLORREF crColor - color to use in plotting
//*                                  double start - where to start in the data
//*                                  double end - where to end in the data
//*  Preconditions : Data should be allocated and populated
//*  Postcondtions : Data is plotted to the DC
//*  Returns       : TRUE
//**************************************************************************
BOOL CPlotWavelet::ScatterPlotDataNode(CWaveletNode* root, CDC* pDC, CRect* rWnd, COLORREF crColor, double start, double end) {

    // Scaling variables
    double fHScale;
    double data_index;
    long datapoint;
    // temp
    struct tagPOINT point_coords;
    // Get the color
    CPen penPlot(PS_SOLID, 1, crColor);
    pDC->SelectObject(&penPlot);

    // Scale the data to fit in our window
    fHScale = (double)((double)(end - start) / (double)rWnd->Width());

    data_index = start;
    pDC->MoveTo(0, root->data[0]);

    // Do the drawing
    for (double x = 0; x < rWnd->Width(); x++) {

        datapoint = root->data[(long)floor(data_index)];                                // Get the data
        data_index += fHScale;                                                          // Advance the data_index
        //data_index++;
        point_coords.x = long(x);
        point_coords.y = datapoint;
        point_coords.y = rWnd->Height() - point_coords.y;                               // Need to print it upside down, because bitmaps are funny look'n
        pDC->LineTo(point_coords);                                                      // for connecting the dots
    }
    return TRUE;
}

// OnDraw
//*  Description   : This is the main drawing method which windows calls
//*                  automatically when the graph needs redrawing
//*  Parameters    : Standard
//*  Preconditions : None
//*  Postcondtions : None
//*  Returns       : void
//**************************************************************************
void CPlotWavelet::OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView) {
    if (IsIconic()) {
        return;    // nothing to draw
    }

    // Wavelet transformation data
    CWaveletNode * root = NULL;

    // Get a handle on the windows
    CSaApp * pApp = (CSaApp *)AfxGetApp();
    CSaDoc * pModel = pView->GetDocument();
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();

    CProcessWavelet * pWavelet = (CProcessWavelet *)pModel->GetWavelet(); // get pointer to wavelet process object

    // Set legend scale
    pGraph->SetLegendScale(SCALE | NUMBERS, 0, 100, _T("testing"));

    // Create the drawing area
    CDC pTempDC;                                                            // Temp area for drawing
    pTempDC.CreateCompatibleDC(pDC);
    BITMAPINFO * pInfo = (BITMAPINFO *) malloc(sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD));
    if (pInfo == NULL) {
        pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
        pGraph->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L);
        return;
    }

    int width = rWnd.Width() + (4 - (rWnd.Width() % 4));  // The width of a bitmap must be divisible by 4
    int height = rWnd.Height();

    // Set up the color Palette
    if (!CreateSpectroPalette(&pTempDC, pModel)) {
        // error creating color palette
        pApp->ErrorMessage(IDS_ERROR_SPECTROPALETTE);
        pGraph->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
        return;
    }

    // Set up the bitmap info
    pInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pInfo->bmiHeader.biWidth = width;
    pInfo->bmiHeader.biHeight = height;
    pInfo->bmiHeader.biPlanes = 1;
    pInfo->bmiHeader.biBitCount = 8;
    pInfo->bmiHeader.biCompression = BI_RGB;
    pInfo->bmiHeader.biSizeImage = 0; // set to zero for uncompressed (BI_RGB)
    pInfo->bmiHeader.biXPelsPerMeter = pInfo->bmiHeader.biYPelsPerMeter = 2835; // 72 DPI
    pInfo->bmiHeader.biClrUsed = 0; // fully populated bmiColors
    pInfo->bmiHeader.biClrImportant = 0;
    populateBmiColors(pInfo->bmiColors, pView);

    // Create the bitmap
    unsigned char * pBits = NULL;
    HBITMAP pTempBitmap = CreateDIBSection(pTempDC.m_hDC,pInfo,DIB_RGB_COLORS, (void **)&pBits,NULL,0);

    if (!pTempBitmap) {
        pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
        pGraph->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L);
        return;
    }

    HBITMAP pOldBitmap = (HBITMAP) ::SelectObject(pTempDC.m_hDC,pTempBitmap);

    //erase background
    pTempDC.FillSolidRect(rClip, pMainWnd->GetColors()->cPlotBkg);


    // do common plot paint jobs
    CRect rSaveClip(rClip);
    PlotPrePaint(&pTempDC, rWnd, rSaveClip);

    //**************************************************************************
    // A C T U A L  C O D E  G O E S  H E R E
    //**************************************************************************

    // Get the raw data from SA
    long * pData = NULL;
    DWORD dwDataSize = 0;
    if (!pWavelet->Get_Raw_Data(&pData, &dwDataSize, pModel)) {
        pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
        pGraph->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L);
        return;
    }

    // Build the tree
    CreateTree("lls", &root);
    max_drawing_level = root->GetNumLeaves();                       // Keep track of how many leaves we have


    root->SetDataNode(pData, dwDataSize, 0, 22000);                 // Load up the raw data
    root->DoMRAAnalysisTree();                                      // Generate all the wavelet data
    root->TransformEnergyTree();                                    // Square everything
    root->TransformSmoothingTree();                                 // Apply the smoothing filter
    root->TransformLogScalingTree(root->GetMaxTree());              // Apply the log filter

    long x = (int) floor(GetDataPosition(rWnd.Width()) / 2);
    long y = pView->GetDataFrame() / 2;

    // USE FOR DRAWING THE SCALOGRAM
    root->DrawColorBandTree(pBits, &rWnd, root->GetMaxTreeBounds(x, x+y), x, x + y);

    // USE FOR DRAWING THE RED LINE
    CWaveletNode * tempNode = root->GetNode(drawing_level);

    tempNode->TransformFitWindowNode(&rWnd);
    ScatterPlotDataTree(root,&pTempDC, &rWnd, RGB(255,0,0), drawing_level, x, x+y);

    // do common plot paint jobs
    PlotPaintFinish(&pTempDC, rWnd, rClip);
    GdiFlush();  // finish all drawing to pTempDC

    // Blit the final image
    if (!pDC->BitBlt(rClip.left,0, rClip.Width(), rClip.Height(), &pTempDC, 0, 0, SRCCOPY)) {
        CSaString szError;
        szError.Format(_T("BitBLT Failed in ")_T(__FILE__)_T(" line %d"),__LINE__);
        ((CSaApp *)AfxGetApp())->ErrorMessage(szError);
    }

    GdiFlush();  // finish BitBlt before destroying DIBSection

    // Clean up!
    pTempDC.SelectObject(pOldBitmap);
    DeleteObject(pTempBitmap);

    if (pData) {
        delete pData;
    }
}

/***************************************************************************/
// CPlotWavelet::CreateSpectroPalette Creates the palette for spectrogram
// This function creates the colors needed by the spectrogram. If the system
// supports only 16 colors, it won't do anything and the spectrogram will
// work with this 16 colors. If it supports 256 colors, this function creates
// a 117 grayscale and above a 117 color palette (total 236 colors available
// in system palette). If there are more than 256 colors supported, this
// function creates the full 233 grayscale and above the 233 color palette.
// The function returns FALSE in case on error, else TRUE.
/***************************************************************************/
BOOL CPlotWavelet::CreateSpectroPalette(CDC * pDC, CDocument * /*pSaDoc*/) {
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
// CPlotWavelet message handlers

// populateBmiColors
//*  Description   :
//*
//*  Parameters    :
//*  Preconditions :
//*  Postcondtions :
//*  Returns       :
//**************************************************************************
void CPlotWavelet::populateBmiColors(RGBQUAD * QuadColors, CSaView * /*pView*/) {
    CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
    // prepare to draw
    COLORREF Color[256];
    // get pointer to plot colors
    Colors * pColors = pMain->GetColors();
    // create palette
    Color[235] = pColors->cPlotBkg;  // background

    int bGrayScale = FALSE;             // COLOR OR GRAYSCALE YOU Decide, here
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













