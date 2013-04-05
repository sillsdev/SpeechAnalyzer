/////////////////////////////////////////////////////////////////////////////
// sa_gZpol.cpp:
// Implementation of the CZPlotPolar
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_plot.h"
#include "Process\Process.h"
#include "sa_gZpol.h"

#include "sa_doc.h"
#include "sa.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "math.h"
#include "zgraph\toolkit.h"
#include "sa_graph.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CZPlotPolar
// Plot window for displaying formants data. This class only handles
// the special drawing of the data. All other work is done in the base class.
// For the drawing, this class holds two buffers (each 1kByte) for the polar
// data to deliver to the Z-graph functions.

/////////////////////////////////////////////////////////////////////////////
// CZPlotPolar message map

BEGIN_MESSAGE_MAP(CZPlotPolar, CPlotWnd)
    //{{AFX_MSG_MAP(CZPlotPolar)
    ON_WM_CREATE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CZPlotPolar construction/destruction/creation

/***************************************************************************/
// CZPlotPolar::CZPlotPolar Constructor
/***************************************************************************/
CZPlotPolar::CZPlotPolar(int nIndexSDP)  : m_nIndexSDP(nIndexSDP)
{
    m_hDataX = NULL;
    m_hDataY = NULL;
    m_hDataSym = NULL;
}

/***************************************************************************/
// CZPlotPolar::~CZPlotPolar Destructor
/***************************************************************************/
CZPlotPolar::~CZPlotPolar()
{
    // free the buffer memory
    if (m_hDataX)
    {
        ::GlobalUnlock(m_hDataX);
        ::GlobalFree(m_hDataX);
    }
    if (m_hDataY)
    {
        ::GlobalUnlock(m_hDataY);
        ::GlobalFree(m_hDataY);
    }
    if (m_hDataSym)
    {
        ::GlobalUnlock(m_hDataSym);
        ::GlobalFree(m_hDataSym);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CZPlotPolar message handlers

/***************************************************************************/
// CZPlotPolar::OnCreate Window creation
// The plot has to get the area process index, because it is a area process
// plot.
/***************************************************************************/
int CZPlotPolar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CPlotWnd::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }
    // get pointer to document
    CView * pView = (CView *)GetParent()->GetParent();
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
    // get pointer to process class
    CProcessSDP * pSDP = GetSDP(pDoc); // get pointer to SDP object
    // save process
    m_pAreaProcess = pSDP;
    return 0;
}

CProcessSDP * CZPlotPolar::GetSDP(CSaDoc * pDoc)
{
    return pDoc->GetSDP(m_nIndexSDP);
}

/***************************************************************************/
// CZPlotPolar::OnDraw Drawing
// The data to paint is raw data, only lightly processed directly before
// painting. The process called, will do anything than just save the area
// data for the plot and set the right flags in the process status. The plot
// itself is painted by the ZGraph functions. Because the regular polar graph
// does not support to draw just a dot, and we need that, the zgraph drawing
// function has been changed to be able to draw just a dot.
/***************************************************************************/
void CZPlotPolar::OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView)
{
#define zTWOPI         6.28318530718
#define zNUM_SAMPLES   512
#define zMAX_NUMPANES  8
#define zMAX_NUMPOINTS zNUM_SAMPLES * 2 * zMAX_NUMPANES

    CSaDoc * pDoc = pView->GetDocument();

    if (IsIconic())
    {
        return;    // nothing to draw
    }
    // get pointer to view and document
    // process workbench if necessary
    pDoc->WorkbenchProcess();
    // get the SDP parameters
    SDPParm * pSDPParm = pDoc->GetSDPParm();
    int nPanes = pSDPParm->nPanes;
    long lSteps = (long)pSDPParm->nSteps;
    BOOL bAuto = (pSDPParm->nStepMode == 0);
    BOOL bAverage = pSDPParm->bAverage;
    // calculate the normfactor
    double dfNormRadFactor = zTWOPI / 360.0 * (double)pSDPParm->nUpperBound;
    // allocate the three data buffers
    CSaApp * pApp = (CSaApp *)AfxGetApp();
    if (!m_hDataX)   // not yet allocated
    {
        m_hDataX = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (DWORD)zMAX_NUMPOINTS * sizeof(double)); // allocate memory
        if (!m_hDataX)
        {
            // memory allocation error
            pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
            GetParent()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
            return;
        }
        m_lpDataX = (HPSTR)::GlobalLock(m_hDataX); // lock memory
        if (!m_lpDataX)
        {
            // memory lock error
            pApp->ErrorMessage(IDS_ERROR_MEMLOCK);
            ::GlobalFree(m_hDataX);
            m_hDataX = NULL;
            GetParent()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
            return;
        }
    }
    if (!m_hDataY)   // not yet allocated
    {
        m_hDataY = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (DWORD)zMAX_NUMPOINTS * sizeof(double)); // allocate memory
        if (!m_hDataY)
        {
            // memory allocation error
            pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
            GetParent()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
            return;
        }
        m_lpDataY = (HPSTR)::GlobalLock(m_hDataY); // lock memory
        if (!m_lpDataY)
        {
            // memory lock error
            pApp->ErrorMessage(IDS_ERROR_MEMLOCK);
            ::GlobalFree(m_hDataY);
            m_hDataY = NULL;
            GetParent()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
            return;
        }
    }
    if (!m_hDataSym)   // not yet allocated
    {
        m_hDataSym = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (DWORD)zMAX_NUMPOINTS * sizeof(int)); // allocate memory
        if (!m_hDataSym)
        {
            // memory allocation error
            pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
            GetParent()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
            return;
        }
        m_lpDataSym = (HPSTR)::GlobalLock(m_hDataSym); // lock memory
        if (!m_lpDataSym)
        {
            // memory lock error
            pApp->ErrorMessage(IDS_ERROR_MEMLOCK);
            ::GlobalFree(m_hDataSym);
            m_hDataSym = NULL;
            GetParent()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
            return;
        }
    }

    // get pointer to process class
    CProcessSDP * pSDP = GetSDP(pDoc); // get pointer to SDP object
    // check if process is idle
    if (pSDP->IsIdle())
    {
        m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_POSCUR, &rWnd);
    }
    else
    {
        m_HelperWnd.SetMode(MODE_HIDDEN); // hide helper window
        int nMaxValue = 1;
        int nMinValue = 0x7fff;
        BOOL bRadiusValid = TRUE;
        DWORD dwStart = pSDP->GetAreaPosition();
        DWORD dwNumber = pSDP->GetAreaLength();
        if ((dwNumber - 1) > zNUM_SAMPLES)
        {
            dwNumber = zNUM_SAMPLES + 1;    // limit number to buffer sizes
        }
        // get a handle to windows for Z-graph
        HDC hdc = pDC->GetSafeHdc();
        // set up an array of colors to be used in the Z-graph
        zRGB PlotColorArray[zMAX_DATA_SETS + 1];
        // get pointer to color structure from main frame
        CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
        Colors * pColor = pMainWnd->GetColors();
        for (int nLoop = 0; nLoop <= zMAX_DATA_SETS; nLoop++)
        {
            PlotColorArray[nLoop] = pColor->cPlotData[0];
        }
        // cast the pointers to the data buffers
        double * pDataX = (double *)m_lpDataX; // pointer to X-axis data
        double * pDataY = (double *)m_lpDataY; // pointer to X-axis data
        int * pDataSym = (int *)m_lpDataSym; // pointer to symbol data

        pSDP->SetArea(pView);
        if (TRUE)
        {
            // new data has to be processed
            dwStart = pSDP->GetAreaPosition();
            UINT nBlockAlign = pDoc->GetBlockAlign();
            if (bAuto)
            {
                // auto step mode, determine steps
                lSteps = (long)(pSDP->GetAreaLength() / (zNUM_SAMPLES * (DWORD)nBlockAlign));
                if (!lSteps)
                {
                    lSteps = 1;
                }
            }
            dwNumber = (pSDP->GetAreaLength() - 1) / (DWORD)lSteps + 1;
            if ((dwNumber - 1) > zNUM_SAMPLES)
            {
                dwNumber = zNUM_SAMPLES + 1;    // limit number to buffer sizes
            }
            if (dwNumber <= 2)
            {
                return;
            }
            DWORD dwSmplSteps = (DWORD)lSteps * (DWORD)nBlockAlign;
            DWORD dwRawBufferSize = pDoc->GetWaveDataBufferSize();
            // fill up data buffers
            HPSTR pData = pDoc->GetWaveData(dwStart, TRUE); // get pointer to wave data block
            if (pData == NULL)
            {
                return;    // error occured while reading data block
            }
            DWORD dwDataPos = dwStart;
            dwStart += dwRawBufferSize;
            int nLastData = 0, nData;
            nMaxValue = 0;
            nMinValue = 0x7FFF;
            bRadiusValid = FALSE;
            // first find min and max values
            for (DWORD dwLoop = 0; dwLoop < dwNumber; dwLoop++)
            {
                // read data
                if (bAverage)
                {
                    // build average inside step
                    long lAverageData = 0;
                    for (DWORD dwLoop = 0; dwLoop < (DWORD)lSteps; dwLoop++)
                    {
                        if (dwDataPos >= dwStart)
                        {
                            // reload data
                            pData = pDoc->GetWaveData(dwDataPos, TRUE); // get new data block
                            if (pData == NULL)
                            {
                                return;    // error occured while reading data block
                            }
                            dwStart = dwDataPos + dwRawBufferSize;
                        }
                        if (nBlockAlign == 1)   // 8 bit data
                        {
                            BYTE bData = *pData++; // data range is 0...255 (128 is center)
                            nData = bData - 128;
                        }
                        else                  // 16 bit data
                        {
                            nData = *((short int *)pData++);
                            pData++;
                            dwDataPos++;
                        }
                        dwDataPos++;
                        lAverageData += (long)nData; // build data sum
                    }
                    nData = (int)(lAverageData / lSteps);
                }
                else
                {
                    // take just one sample
                    if (dwDataPos >= dwStart)
                    {
                        // reload data
                        pData = pDoc->GetWaveData(dwDataPos, TRUE); // get new data block
                        if (pData == NULL)
                        {
                            return;    // error occured while reading data block
                        }
                        dwStart = dwDataPos + dwRawBufferSize;
                    }
                    if (nBlockAlign == 1)   // 8 bit data
                    {
                        BYTE bData = *pData; // data range is 0...255 (128 is center)
                        nData = bData - 128;
                    }
                    else
                    {
                        nData = *((short int *)pData);    // 16 bit data
                    }
                    pData += dwSmplSteps;
                    dwDataPos += dwSmplSteps;
                }
                if (!dwLoop)
                {
                    nLastData = nData;    // save first data
                }
                // find min and max values
                if (nData > nMaxValue)
                {
                    nMaxValue = nData;    // store higher value
                }
                if (nData < nMinValue)
                {
                    nMinValue = nData;    // store lower value
                }
            }
            // reset data pointers
            dwStart = pSDP->GetAreaPosition();
            pData = pDoc->GetWaveData(dwStart, TRUE); // get first wave data block
            if (pData == NULL)
            {
                return;    // error occured while reading data block
            }
            dwDataPos = dwStart;
            dwStart += dwRawBufferSize;
            // now process the data and fill up the buffers
            double dfHminL = 0.0001;
            if (nMaxValue != nMinValue)
            {
                dfHminL = (double)nMaxValue - (double)nMinValue;
            }
            pData += dwSmplSteps;
            dwDataPos += dwSmplSteps;
            for (DWORD dwLoop = 1; dwLoop < dwNumber; dwLoop++)
            {
                // read data
                if (bAverage)
                {
                    // build average inside step
                    long lAverageData = 0;
                    for (DWORD dwLoop = 0; dwLoop < (DWORD)lSteps; dwLoop++)
                    {
                        if (dwDataPos >= dwStart)
                        {
                            // reload data
                            pData = pDoc->GetWaveData(dwDataPos, TRUE); // get new data block
                            if (pData == NULL)
                            {
                                return;    // error occured while reading data block
                            }
                            dwStart = dwDataPos + dwRawBufferSize;
                        }
                        if (nBlockAlign == 1)   // 8 bit data
                        {
                            BYTE bData = *pData++; // data range is 0...255 (128 is center)
                            nData = bData - 128;
                        }
                        else                  // 16 bit data
                        {
                            nData = *((short int *)pData++);
                            pData++;
                            dwDataPos++;
                        }
                        dwDataPos++;
                        lAverageData += (long)nData; // build data sum
                    }
                    nData = (int)(lAverageData / lSteps);
                }
                else
                {
                    // take just one sample
                    if (dwDataPos >= dwStart)
                    {
                        // reload data
                        pData = pDoc->GetWaveData(dwDataPos, TRUE); // get new data block
                        if (pData == NULL)
                        {
                            return;    // error occured while reading data block
                        }
                        dwStart = dwDataPos + dwRawBufferSize;
                    }
                    if (nBlockAlign == 1)   // 8 bit data
                    {
                        BYTE bData = *pData; // data range is 0...255 (128 is center)
                        nData = bData - 128;
                    }
                    else
                    {
                        nData = *((short int *)pData);    // 16 bit data
                    }
                    pData += dwSmplSteps;
                    dwDataPos += dwSmplSteps;
                }
                // calculate radius
                double dfRadius = ((double)nLastData - (double)nMinValue) / dfHminL * (double)pSDPParm->nUpperBound; // radius
                if (dfRadius)
                {
                    bRadiusValid = TRUE;
                }
                // calculate angle
                double dfTheta = ((double)nData - (double)nMinValue) / dfHminL * dfNormRadFactor; // radius
                for (int nLoop = 1; nLoop <= nPanes; nLoop++)
                {
                    double dfBase = (double)(nLoop * zTWOPI / nPanes);
                    *pDataY++ = dfBase + dfTheta; // angle in radian
                    *pDataX++ = dfRadius; // radius
                    *pDataSym++ = zDOT;
                    *pDataY++ = dfBase - dfTheta; // angle in radian
                    *pDataX++ = dfRadius; // radius
                    *pDataSym++ = zDOT;
                }
                nLastData = nData;
            }
        }
        // set up Z-graph structure
        SGraph zG;
        // fill In the [SGraph] structure elements
        zG.hdc            = hdc            ; // windows HDC
        zG.RWindow        = &rWnd          ; // RECT with window dimensions
        zG.xdata          = (double *)m_lpDataX; // x-data array, radius
        zG.ydata          = (double *)m_lpDataY; // y-data array, angle (radian)
        zG.zdata          = NULL           ; // z-data array
        zG.symbols        = (int *)m_lpDataSym; // symbols array
        if (((nMinValue == 0) && (nMaxValue == 0)) || !bRadiusValid)
        {
            zG.n = 0;    // nothing to display
        }
        else
        {
            zG.n = (int)((dwNumber - 1) * 2 * nPanes);    // total number of data points
        }
        zG.axis_color     = pColor->cPlotGrid; // graph axis color
        zG.bkg_color      = pColor->cPlotBkg; // graph bkg. color
        zG.plot_color     = PlotColorArray ; // array of plot colors
        zG.fill_style     = 0              ;
        zG.title          = _T("\0")           ;
        zG.H_Title        = _T("")             ; // horizontal axis title
        zG.V_Title        = _T("")             ; // vertical axis title
        zG.legend_position = zNO_LEGEND    ;
        zG.item_title_format = zNUMBERS    ; // show numbers along axes
        zG.line_connect   = OFF            ;
        zG.symbols_flag   = ON             ;
        zG.grid_flag      = HaveGrid()     ; // grid on/off for graph
        zG.auto_scale     = OFF            ; // use manuel scaling
        zG.draw_forward   = TRUE           ;
        zG._3D_flag       = FALSE          ;
        zG.bkg_gradient_mode = zNO_GRADIENT;
        zG.x_axis_style   = zPOLAR         ;
        zG.y_axis_style   = zPOLAR         ;
        zG.x_axis_divisions = pSDPParm->nPanes / 10; // x-axis divisions
        zG.y_axis_divisions = pSDPParm->nPanes / 10; // y-axis divisions
        zG.digit_format   = zFIXED         ; // round axis numbers to nearest whole integer
        zG.digit_precision = 1             ;
        zG.grid_pen_style = zDOTTED        ; // use dotted lines for grid
        zG.shading_style  = zNO_SHADING    ;
        zG.quality        = zG3D_QUALITY_HI;
        zG.function       = NULL           ;
        // draw the graph
        PolarGraph(&zG);
    }
    // finish the plot
    PlotPaintFinish(pDC, rWnd, rClip);
}
