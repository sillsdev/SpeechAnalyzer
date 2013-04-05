/////////////////////////////////////////////////////////////////////////////
// sa_graph.cpp:
// Implementation of the CGraphWnd class.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//    1.06.6U4
//         SDM changed ScrollGraph to scroll exact number of pixels
//    1.06.6U5
//         SDM Change ScrollGraph to call RedrawGraph on page scroll
//         SDM Added SetCursor calls to RedrawGragh
//    1.06.6U6
//         SDM Added function SetPlaybackPosition
//    1.5Test8.1
//         SDM Added m_anAnnWndOrder[]
//         SDM Added support for Reference annotation
//    1.5Test11.1A
//         RLJ Add SetSpectroAB call so we know which set (A or B) of
//             parameters to save.
//    06/06/2000
//         RLJ Added bSetProperties(int nNewID)
//    06/17/2000
//         RLJ Extend FileOpenAs to support not only Phonetic/Music Analysis,
//               but also OpenScreenF, OpenScreenG, OpenScreenI, OpenScreenK,
//               OpenScreenM, etc.
//    07/25/2000
//         RLJ Add check for non-null pMain to destructor
//
//    08/16/2000
//         DDO Fixed a problem in the update status bar function where
//             the auto pitch process object was being pointed to for
//             melograms. I changed it so the melogram process object
//             is being pointed to. Since the code, otherwise, is the
//             same for pitch as it is for melogram, I have
//
//    08/30/2000
//         DDO Made the recording graph an area graph.
//
//    09/22/2000
//         DDO Added a recording class derived from CGraphWnd. This allows
//             the PreCreateWindow to be overridden for more control over
//             the recording window's creation.
//
//    09/26/2000
//         DDO Added code to redraw the TWC graph when the melogram is
//             redrawn.
//
//    09/13/2000
//         DDO Added code in ShowLegend to make sure the melogram and
//             magnitude legend states are the same.
//
//    10/23/2000
//         DDO Added a fix so the legend on the melogram and magnitude
//             is turned on by default when there is no TWC graph.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_minic.h"
#include "sa_graph.h"
#include "sa_plot.h"
#include "dsp\signal.h"
#include "dsp\formants.h"
#include "Process\Process.h"

#include "math.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "sa_g_raw.h"
#include "sa_g_glo.h"
#include "sa_g_lou.h"
#include "sa_g_dur.h"
#include "sa_g_zcr.h"
#include "sa_g_pit.h"
#include "sa_g_cpi.h"
#include "sa_g_spi.h"
#include "sa_g_cha.h"
#include "sa_g_spg.h"
#include "sa_g_3dPitch.h"
#include "sa_g_wavelet.h"                   // ARH 8/2/01 Added for wavelet graph
#include "sa_g_spu.h"
#include "sa_g_poa.h"
#include "sa_g_gra.h"
#include "sa_g_rec.h"
#include "sa_g_mel.h"
#include "sa_g_twc.h"
#include "sa_g_stf.h"
#include "sa_g_mag.h"
#include "sa_gZ3D.h"
#include "sa_g_rat.h"
#include "sa_gZpol.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "printdim.h"
#include "sa_mplot.h"
#include "settings\obstream.h"
#include "time.h"
#include <afxpriv.h>
#include "PrivateCursorWnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/***************************************************************************/
// CGraphWnd::SetMagnify Set new magnify factor, redraw plot and legend
/***************************************************************************/
void CGraphWnd::SetMagnify(double bFactor, BOOL bRedraw)
{
    // set magnify in plot
    m_pPlot->SetMagnify(bFactor, bRedraw);
    if (m_bLegend && bRedraw)
    {
        RedrawGraph(TRUE, TRUE);    // redraw whole graph
    }
}

/***************************************************************************/
// CGraphWnd::SetStartCursor Position the start cursor
/***************************************************************************/
void CGraphWnd::SetStartCursor(CSaView * pView)
{
    //if (HaveCursors())    //!! commented out by AKE to hide cursors for graph editing
    m_pPlot->SetStartCursor(pView);
    if (!m_pPlot->HaveCursors())
    {
        m_pPlot->Invalidate();
    }
    /*
    if (HavePrivateCursor())
    {
    // if spectrum, only update plot area to allow animation while retaining legend, x-scale, and annotations
    //!!should refresh annotations as segment boundaries are crossed  AKE
    if (m_nPlotID == IDD_CEPSPECTRUM)
    m_pPlot->Invalidate();
    //otherwise redraw x-scale and annotation because they may change based on cursor position in waveform
    else
    RedrawGraph(TRUE, FALSE, TRUE);
    }
    if (m_nPlotID == IDD_POA) RedrawGraph(TRUE, FALSE);
    if (m_nPlotID == IDD_F1F2 || m_nPlotID == IDD_F2F1 || m_nPlotID == IDD_F2F1F1 || m_nPlotID == IDD_3D)
    m_pPlot->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    */
    UpdateStatusBar(pView->GetStartCursorPosition(), pView->GetStopCursorPosition()); // update the status bar
}

/***************************************************************************/
// CGraphWnd::SetStopCursor Position the stop cursor
/***************************************************************************/
void CGraphWnd::SetStopCursor(CSaView * pView)
{
    //if (HaveCursors())    //!! commented out by AKE to hide cursors for graph editing
    m_pPlot->SetStopCursor(pView);
    if (!m_pPlot->HaveCursors())
    {
        m_pPlot->Invalidate();
    }
    /*
    if (HavePrivateCursor())
    {
    // if spectrum, only update plot area to allow animation while retaining legend, x-scale, and annotations
    //!!should refresh annotations as segment boundaries are crossed  AKE
    if (m_nPlotID == IDD_CEPSPECTRUM)
    m_pPlot->Invalidate();
    // otherwise redraw x-scale and annotation because they may change based on cursor position in waveform
    else
    RedrawGraph(TRUE, FALSE, TRUE);
    }
    if (m_nPlotID == IDD_POA)
    RedrawGraph(TRUE, FALSE);
    if (m_nPlotID == IDD_F1F2 || m_nPlotID == IDD_F2F1 || m_nPlotID == IDD_F2F1F1 || m_nPlotID == IDD_3D)
    m_pPlot->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    */
    UpdateStatusBar(pView->GetStartCursorPosition(), pView->GetStopCursorPosition()); // update the status bar
}

// SDM 1.06.6U6
/***************************************************************************/
// CGraphWnd::SetPlaybackPosition
/***************************************************************************/
void CGraphWnd::SetPlaybackPosition(CSaView * pSaView, DWORD dwPos)
{
    if (HaveCursors())
    {
        m_pPlot->SetPlaybackCursor(pSaView, dwPos);
    }
}

/***************************************************************************/
// CGraphWnd::CreateAnnotationWindows Creates the annotation windows
//**************************************************************************/
void CGraphWnd::CreateAnnotationWindows()
{
    m_apAnnWnd[PHONETIC] = new CPhoneticWnd(PHONETIC);
    m_apAnnWnd[TONE] = new CToneWnd(TONE);
    m_apAnnWnd[PHONEMIC] = new CPhonemicWnd(PHONEMIC);
    m_apAnnWnd[ORTHO] = new COrthographicWnd(ORTHO);
    m_apAnnWnd[GLOSS] = new CGlossWnd(GLOSS);
    m_apAnnWnd[REFERENCE] = new CReferenceWnd(REFERENCE);
    m_apAnnWnd[MUSIC_PL1] = new CMusicPhraseWnd(MUSIC_PL1);
    m_apAnnWnd[MUSIC_PL2] = new CMusicPhraseWnd(MUSIC_PL2);
    m_apAnnWnd[MUSIC_PL3] = new CMusicPhraseWnd(MUSIC_PL3);
    m_apAnnWnd[MUSIC_PL4] = new CMusicPhraseWnd(MUSIC_PL4);
    CRect rWnd(0, 0, 0, 0);
    for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
    {
        if (!m_apAnnWnd[nLoop]->Create(NULL, _T("Transcription"), WS_CHILD | WS_VISIBLE, rWnd, this, 0))
        {
            delete m_apAnnWnd[nLoop];
            m_apAnnWnd[nLoop] = NULL;
        }
    }
}

/***************************************************************************/
// CGraphWnd::SetLegendScale Set the scale of the legend window
//**************************************************************************/
BOOL CGraphWnd::SetLegendScale(int nMode, double dMinValue, double dMaxValue, TCHAR * pszDimension, int nDivisions, double d3dOffset)
{
    // set legend scale
    BOOL bNewScale = m_pLegend->SetScale(nMode, dMinValue, dMaxValue, pszDimension, nDivisions, d3dOffset);
    if (bNewScale && m_nPlotID != ID_GRAPHS_OVERLAY)
    {
        RedrawGraph(TRUE, m_bLegend, FALSE);    // redraw whole graph
    }

    return bNewScale && m_nPlotID != ID_GRAPHS_OVERLAY;
}

/***************************************************************************/
// CGraphWnd::SetXScale Set the scale of the x-scale window
//**************************************************************************/
void CGraphWnd::SetXScale(int nMode, int nMinValue, int nMaxValue, TCHAR * pszDimension, int nDivisions, double d3dOffset)
{
    if (m_bXScale)
    {
        // set x-scale scale
        if (m_pXScale->SetScale(nMode, nMinValue, nMaxValue, pszDimension, nDivisions, d3dOffset) && m_nPlotID != ID_GRAPHS_OVERLAY)   // set x-scale scale
        {
            RedrawGraph(TRUE, TRUE);    // redraw whole graph
        }
    }
}

/***************************************************************************/
// CGraphWnd::ChangeAnnotationSelection Change the annotation selection
//**************************************************************************/
void CGraphWnd::ChangeAnnotationSelection(int nIndex)
{
    if (m_abAnnWnd[nIndex])
    {
        m_apAnnWnd[nIndex]->Invalidate(TRUE);    // redraw
    }
}

/////////////////////////////////////////////////////////////////////////////
// CGraphWnd message handlers

/***************************************************************************/
// CGraphWnd::OnCreate Window creation
/***************************************************************************/
int CGraphWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMiniCaptionWnd::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    ModifyStyle(0, WS_CLIPCHILDREN);
    // create scale and annotation window objects
    m_pLegend = new CLegendWnd();
    m_pXScale = new CXScaleWnd();
    CRect rWnd(0, 0, 0, 0);
    // create the plot window
    if (!m_pPlot->Create(NULL, _T("Plot"), WS_CHILD | WS_VISIBLE, rWnd, this, 0))
    {
        delete m_pPlot;
        m_pPlot = NULL;
    }
    m_pPlot->SetPlotName(lpCreateStruct->lpszName);
    // create the legend window
    if (!m_pLegend->Create(NULL, _T("Legend"), WS_CHILD | WS_VISIBLE, rWnd, this, 0))
    {
        delete m_pLegend;
        m_pLegend = NULL;
    }
    // create the x-scale window
    if (!m_pXScale->Create(NULL, _T("XScale"), WS_CHILD | WS_VISIBLE, rWnd, this, 0))
    {
        delete m_pXScale;
        m_pXScale = NULL;
    }
    // create the annotation windows
    CreateAnnotationWindows();
    return 0;
}

/***************************************************************************/
// CGraphWnd::OnSize Window sizing
/***************************************************************************/
void CGraphWnd::OnSize(UINT nType, int cx, int cy)
{
    CMiniCaptionWnd::OnSize(nType, cx, cy);
    ResizeGraph(TRUE, TRUE); // repaint whole graph
}

/***************************************************************************/
// CGraphWnd::OnDraw - draw the graph window.  Currently
// only used for printing.
/***************************************************************************/
void CGraphWnd::OnDraw(CDC * pDC, const CRect * printRect,
                       int originX, int originY)
{
    if (pDC->IsPrinting())
    {
        if (m_pPlot != NULL)
        {
            PrintHiResGraph(pDC, printRect, originX, originY);
        }
    }
}

void ScaleRect(CRect & rct, double scaleX, double scaleY)
{
    rct.left   = (int)(scaleX * rct.left);
    rct.right  = (int)(scaleX * rct.right);
    rct.top    = (int)(scaleY * rct.top);
    rct.bottom = (int)(scaleY * rct.bottom);
}

/***************************************************************************/
// CGraphWnd::PrintHiResGraph - print the hi res graph by calling OnDraw()
// for every window object contained within this window.
/***************************************************************************/
void CGraphWnd::PrintHiResGraph(CDC * pDC, const CRect * printRect,
                                int originX, int originY)
{
    int nLoop = 0;
    CRect graphRect, plotRect;
    CRect xScaleRect(0,0,0,0);
    CRect scaledXscaleRect(0,0,0,0);
    CRect legendRect(0,0,0,0);
    CRect scaledLegendRect(0,0,0,0);
    CRect annotRect[ANNOT_WND_NUMBER];
    CRect scaledAnnotRect[ANNOT_WND_NUMBER];
    int height = m_iNCHeight * 2;

    for (; nLoop < ANNOT_WND_NUMBER; nLoop++)
    {
        annotRect[nLoop] = CRect(0,0,0,0);
        scaledAnnotRect[nLoop] = CRect(0,0,0,0);
    }
    GetWindowRect(&graphRect);
    double scaleX = (double)((1.0 * printRect->Width())  / graphRect.Width());
    double scaleY = (double)((1.0 * printRect->Height()) / graphRect.Height());

    // draw the window border
    OnNcDraw(pDC, *printRect, TRUE);

    // phase 1 - get the CRect's
    m_pPlot->GetWindowRect(plotRect);
    CRect scaledPlotRect(0,0,(int)(scaleX * plotRect.Width()),
                         (int)(scaleY * plotRect.Height()));

    for (nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
    {
        if (m_abAnnWnd[nLoop])
        {
            m_apAnnWnd[nLoop]->GetWindowRect(&(annotRect[nLoop]));
            scaledAnnotRect[nLoop].right  = (int)(scaleX * annotRect[nLoop].Width());
            scaledAnnotRect[nLoop].bottom = (int)(scaleY * annotRect[nLoop].Height());
        }
    }
    if (m_pXScale)
    {
        m_pXScale->GetWindowRect(&xScaleRect);
        scaledXscaleRect.right  = (int)(scaleX * xScaleRect.Width());
        scaledXscaleRect.bottom = (int)(scaleY * xScaleRect.Height());
    }
    if (m_pLegend)
    {
        m_pLegend->GetWindowRect(&legendRect);
        scaledLegendRect.right  = (int)(scaleX * legendRect.Width());
        scaledLegendRect.bottom = (int)(scaleY * legendRect.Height());
    }

    // phase 2 - adjust it
    CRect T(printRect);

    T.bottom = T.top + height;

    CRect L(scaledLegendRect);

    L.top  = T.bottom;
    L.left = printRect->left + Graph_Inset;
    L.right = LEGEND_PRINT_WIDTH;
    L.bottom = printRect->bottom - Graph_Inset;

    CRect B(scaledXscaleRect);

    B.left = L.right;
    B.right = printRect->right - Graph_Inset;
    B.bottom = printRect->bottom - Graph_Inset;
    B.top = B.bottom - XSCALE_PRINT_HEIGHT;

    CRect A[ANNOT_WND_NUMBER];

    for (nLoop=0; nLoop<ANNOT_WND_NUMBER; nLoop++)
    {
        if (m_abAnnWnd[nLoop])
        {
            break;
        }
    }

    int Abottom=T.bottom;

    if (nLoop < ANNOT_WND_NUMBER)
    {
        A[nLoop].top = T.bottom + Graph_Inset;
        A[nLoop].left = L.right;
        A[nLoop].right = printRect->right - Graph_Inset;
        A[nLoop].bottom = A[nLoop].top +  ANNOT_PRINT_HEIGHT;
        Abottom =A[nLoop].bottom;

        int prev = nLoop;

        for (int nLoop2=nLoop+1; nLoop2<ANNOT_WND_NUMBER; nLoop2++)
        {
            if (m_abAnnWnd[nLoop2])
            {
                A[nLoop2].top = A[prev].bottom;
                A[nLoop2].left = L.right;
                A[nLoop2].right = printRect->right - Graph_Inset;
                A[nLoop2].bottom = A[nLoop2].top + ANNOT_PRINT_HEIGHT;
                Abottom = A[nLoop2].bottom;
                prev = nLoop2;
            }
        }
    }

    CRect P(L.right,Abottom,printRect->right-Graph_Inset,B.top);

    scaledLegendRect.right = L.Width();
    scaledLegendRect.bottom = L.Height();
    scaledXscaleRect.right = B.Width();
    scaledXscaleRect.bottom = B.Height();

    for (nLoop=0; nLoop<ANNOT_WND_NUMBER; nLoop++)
    {
        if (m_abAnnWnd[nLoop])
        {
            scaledAnnotRect[nLoop].right = A[nLoop].Width();
            scaledAnnotRect[nLoop].bottom = A[nLoop].Height();
        }
    }

    scaledPlotRect.right = P.Width();
    scaledPlotRect.bottom = P.Height();


    // phase 3 - draw it

    for (nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
    {
        if (m_abAnnWnd[nLoop])
        {
            pDC->SetWindowOrg(CPoint(originX - A[nLoop].left,
                                     originY - A[nLoop].top));
            m_apAnnWnd[nLoop]->OnDraw(pDC,scaledAnnotRect[nLoop]);
        }
    }
    if (m_pXScale)
    {
        pDC->SetWindowOrg(CPoint(originX - B.left,originY - B.top));
        m_pXScale->OnDraw(pDC,scaledXscaleRect,scaledPlotRect);
    }
    if (m_pLegend)
    {
        pDC->SetWindowOrg(CPoint(originX - L.left,originY - L.top));

        m_pLegend->OnDraw(pDC,scaledLegendRect,scaledPlotRect,scaledXscaleRect,
                          scaledAnnotRect);
    }

    CPoint newOrg(originX - P.left, originY - P.top);
    pDC->SetWindowOrg(newOrg);
    CSaView * pView = (CSaView *)GetParent();

    CRect clipRect(scaledPlotRect);
    pDC->LPtoDP(clipRect);
    //clipRect.OffsetRect(-newOrg.x, -newOrg.y);
    if (pView->PrintPreviewInProgress())
    {
        clipRect.OffsetRect(2000, 198);
        // transform to preview DP
        CPreviewDC * pPrevDC = static_cast<CPreviewDC *>(pDC);
        pPrevDC->PrinterDPtoScreenDP(&clipRect.TopLeft());
        pPrevDC->PrinterDPtoScreenDP(&clipRect.BottomRight());
    }

    m_pPlot->OnDraw(pDC, scaledPlotRect, clipRect, pView);

    CRect rctCur;
    double scaleCurX = (double)((scaledPlotRect.Width() * 1.0) / plotRect.Width());
    double scaleCurY = (double)((scaledPlotRect.Height() * 1.0) / plotRect.Height());

    // if there are cursors to print, print cursors
    if (m_pPlot->HaveCursors())
    {
        // print the start cursor
        CStartCursorWnd * pStartCur = m_pPlot->GetStartCursorWnd();
        pStartCur->GetWindowRect(&rctCur);
        rctCur.left += CURSOR_WINDOW_HALFWIDTH;
        rctCur.OffsetRect(-plotRect.left,-plotRect.top);
        ScaleRect(rctCur, scaleCurX, scaleCurY);
        rctCur.right = rctCur.left + 1;
        pDC->SetWindowOrg(CPoint(originX - P.left - rctCur.left,
                                 originY - P.top  - rctCur.top));
        rctCur.OffsetRect(-rctCur.left,-rctCur.top);
        pStartCur->OnDraw(pDC, rctCur);

        // print the stop cursor
        CStopCursorWnd * pStopCur = m_pPlot->GetStopCursorWnd();
        pStopCur->GetWindowRect(&rctCur);
        rctCur.left += CURSOR_WINDOW_HALFWIDTH;
        rctCur.OffsetRect(-plotRect.left,-plotRect.top);
        ScaleRect(rctCur, scaleCurX, scaleCurY);
        rctCur.right = rctCur.left + 1;
        pDC->SetWindowOrg(CPoint(originX - P.left - rctCur.left,
                                 originY - P.top  - rctCur.top));
        rctCur.OffsetRect(-rctCur.left,-rctCur.top);
        pStopCur->OnDraw(pDC, rctCur);
    } // end of if there are cursors to print, print cursors


    // if there is a private cursor to print, print it.
    if (m_pPlot->HavePrivateCursor())
    {
        // print the private cursor
        CPrivateCursorWnd * pPrivCur = m_pPlot->GetPrivateCursorWnd();
        pPrivCur->GetWindowRect(&rctCur);
        rctCur.left += CURSOR_WINDOW_HALFWIDTH;
        rctCur.OffsetRect(-plotRect.left, -plotRect.top);
        ScaleRect(rctCur, scaleCurX, scaleCurY);
        rctCur.right = rctCur.left + 1;
        pDC->SetWindowOrg(CPoint(originX - P.left - rctCur.left,
                                 originY - P.top  - rctCur.top));
        rctCur.OffsetRect(-rctCur.left, -rctCur.top);
        pPrivCur->OnDraw(pDC, rctCur);
    } // end of if there is a private cursor to print, print it.
}

BOOL CGraphWnd::IsIDincluded(UINT id)
{
    return (m_nPlotID == ID_GRAPHS_OVERLAY) ?
           ((CMultiPlotWnd *)m_pPlot)->IsIDincluded(id)
           : (m_nPlotID == id);
}

CPlotWnd  * CGraphWnd::NewPlotFromID(UINT plotID)

{
    CPlotWnd * pPlot = NULL;

    // create plot window object depending on which type of graph it is
    switch (plotID)
    {
    case IDD_RAWDATA:
        pPlot = new CPlotRawData();
        break;
    case IDD_LOUDNESS:
        pPlot = new CPlotLoudness();
        break;
    case IDD_DURATION:
        pPlot = new CPlotDurations();
        break;
    case IDD_CHANGE:
        pPlot = new CPlotChange();
        break;
    case IDD_GLOTWAVE:
        pPlot = new CPlotGlottis();
        break;
    case IDD_3D_PITCH:
        pPlot = new CPlot3dPitch();
        break;
    case IDD_WAVELET:
        pPlot = new CPlotWavelet();
        break;      // ARH 8/2/01 Added for the wavelet graph



    case IDD_SPECTROGRAM:
        pPlot = new CPlotSpectrogram();
        ((CPlotSpectrogram *)pPlot)->SetSpectroAB('A'); // RLJ 1.5Test11.1A
        break; //  automatic graph
    case IDD_SNAPSHOT:
        pPlot = new CPlotSpectrogram();
        ((CPlotSpectrogram *)pPlot)->SetSpectroAB('B'); // RLJ 1.5Test11.1A
        SetAreaGraph(TRUE);
        break; //  is an area processed graph

    case IDD_SPECTRUM:
        pPlot = new CPlotSpectrum();
        break;
    case IDD_TWC:
        pPlot = new CPlotTonalWeightChart();
        break;
    case IDD_PITCH:
        pPlot = new CPlotPitch();
        break;
    case IDD_CHPITCH:
        pPlot = new CPlotCustomPitch();
        break;
    case IDD_SMPITCH:
        pPlot = new CPlotSmoothedPitch();
        break;
    case IDD_GRAPITCH:
        pPlot = new CPlotGrappl();
        break;
    case IDD_RECORDING:
        pPlot = new CPlotRecording();
        SetAreaGraph(TRUE);
        break;
    case IDD_MELOGRAM:
        pPlot = new CPlotMelogram();
        break;
    case IDD_STAFF:
        pPlot = new CPlotStaff();
        break;
    case IDD_MAGNITUDE:
        pPlot = new CPlotMagnitude();
        if (pPlot)
        {
            pPlot->SetLineDraw(FALSE);
        }
        break;     // 09/29/2000 - DDO
        /* !!not currently in use  AKE 8/25/00
        case IDD_CEPPITCH:    pPlot = new CPlotCepstralPitch();
        SetAreaGraph(TRUE); break; //  is an area processed graph
        */
    case IDD_ZCROSS:
        pPlot = new CPlotZCross();
        break;
    case IDD_POA:
        pPlot = new CPlotPOA();
        break;
        // Z-Graph graphs
    case IDD_3D:
        pPlot = new CPlot3D();
        /* SetAreaGraph(TRUE); */
        break; //  is an area processed graph
    case IDD_F1F2:
        pPlot = new CPlotF1F2();
        /* SetAreaGraph(TRUE); */
        break; //  is an area processed graph
    case IDD_F2F1:
        pPlot = new CPlotF2F1();
        /* SetAreaGraph(TRUE); */
        break; //  is an area processed graph
    case IDD_F2F1F1:
        pPlot = new CPlotF2F1F1();
        /* SetAreaGraph(TRUE); */
        break; //  is an area processed graph
    case IDD_SDP_A:
    case IDD_SDP_B:
        pPlot = new CZPlotPolar(plotID==IDD_SDP_A ? 0:1);
        SetAreaGraph(TRUE);
        break; //  is an area processed graph
    case IDD_INVSDP:
        pPlot = new CPlotInvSDP();
        SetAreaGraph(TRUE);
        break; // this is an area processed graph
    case ID_GRAPHS_OVERLAY:
        pPlot = new CMultiPlotWnd();
        break;
    }

    if (pPlot)
    {
        pPlot->SetParent(this);
    }

    return pPlot;
}

void CGraphWnd::MergeInGraph(CGraphWnd * pGraphToMerge, CSaView * pView, bool bRtOverlay)
{
    ASSERT(pGraphToMerge);
    TRACE(_T("Merge %lp\n"),pGraphToMerge);

    if ((IsMergeableGraph(pGraphToMerge)) &&
            (IsMergeableGraph(this, TRUE)))
    {
        if (m_nPlotID != ID_GRAPHS_OVERLAY)
        {
            //the very first time, we convert the target graph from something (raw,etc..)
            //into an overlay graph and add the graph back into it.
            CPlotWnd * pPlot = m_pPlot->NewCopy();
            pPlot->SetParent(this);
            m_pPlot->DestroyWindow();
            m_pPlot = NULL;

            m_pPlot = NewPlotFromID(ID_GRAPHS_OVERLAY);
            ASSERT(m_nPlotID != IDD_BLANK);
            if (m_nPlotID != IDD_BLANK)
            {
                ((CMultiPlotWnd *)m_pPlot)->AddPlot(pPlot,m_nPlotID, (CSaView *)GetParent());
            }
            m_nPlotID = ID_GRAPHS_OVERLAY;

            // create the plot window
            CRect rWnd(0, 0, 0, 0);
            if (!m_pPlot->Create(NULL, _T("Plot"), WS_CHILD | WS_VISIBLE, rWnd, this, 0))
            {
                delete m_pPlot;
                m_pPlot = NULL;
            }
            m_pPlot->SetPlotName("Overlay");
        }

        //now that we have an overlay graph, we add the new graph into it,
        //but first, if we are in "realtime autopitch mode, we need to remove previous overlay graphs.
        if (m_pPlot)
        {
            CMultiPlotWnd * pMPlot = (CMultiPlotWnd *)m_pPlot;

            pMPlot->RemoveRtPlots();

            if (pGraphToMerge->m_nPlotID == ID_GRAPHS_OVERLAY)
            {
                CMultiPlotWnd * pMPlot2 = (CMultiPlotWnd *)pGraphToMerge->m_pPlot;
                pMPlot->MergeInOverlayPlot(pMPlot2);
            }
            else
            {
                CPlotWnd * pPlot2 = pGraphToMerge->m_pPlot->NewCopy();
                pPlot2->SetParent(this);
                if (bRtOverlay)
                {
                    pPlot2->SetRtOverlay();
                }
                pMPlot->AddPlot(pPlot2,pGraphToMerge->m_nPlotID, pView);
            }
        }
    }
}

void CGraphWnd::RemoveOverlayItem(const CPlotWnd * pPlot)
{
    if (m_nPlotID == ID_GRAPHS_OVERLAY)
    {
        CMultiPlotWnd * pMPlot = (CMultiPlotWnd *)m_pPlot;
        pMPlot->RemovePlot(pPlot);
    }
}

BOOL CGraphWnd::IsMergeableGraph(CGraphWnd * pGraphToMerge, BOOL bBaseGraph)
{
    CSaView * pView = (CSaView *)pGraphToMerge->GetParent(); // get pointer to parent view
    BOOL ret = FALSE;

    switch (pGraphToMerge->m_nPlotID)
    {
    case IDD_SPECTROGRAM:
        ret = bBaseGraph;  // Should be true only if graph is the base graph
        break;

    case IDD_LOUDNESS:   // magnitude
    case IDD_GRAPITCH:  // cecil auto pitch
    case ID_GRAPHS_OVERLAY:
    case IDD_RAWDATA:    // waveform
    case IDD_ZCROSS:     // zero crossing
    case IDD_CHANGE:     // cecil change
    case IDD_GLOTWAVE:   // glottal waveform
    case IDD_PITCH:
    case IDD_CHPITCH:
    case IDD_SMPITCH:
        ret = TRUE;
        break;

    case IDD_MELOGRAM:
        ret = !(pView->IDDSelected(pView->GetGraphIDs(),IDD_TWC) || pView->IDDSelected(pView->GetGraphIDs(),IDD_MAGNITUDE));
        break;

        // if you add a new one to the list, you at least
        // need to write a NewCopy() method for the corresponding class.
    case IDD_CEPPITCH:  // supposed to enable this one. (but its an area graph)

        // could enable these easily
    case IDD_DURATION:
    case IDD_POA:

        //////////////////  pja 5/8/00
    case IDD_MAGNITUDE:
        /////////////////

    default:
        ret = FALSE;
    }


    return ret;
}

static LPCSTR psz_sagraph      = "sagraph";
//static const char* psz_placement  = "placement";
//static const char* psz_z          = "z";
static LPCSTR psz_plotid       = "plotid";
static LPCSTR psz_showlegend   = "showlegend";
static LPCSTR psz_showxscale   = "showxscale";
static LPCSTR psz_annotwnd     = "annotwnd";
static LPCSTR psz_showwnd      = "showwnd";

void CGraphWnd::WriteProperties(CObjectOStream & obs)
{
    obs.WriteBeginMarker(psz_sagraph);

    // save the window placement.
    //    obs.WriteWindowPlacement(psz_placement, pwndFrame);
    //    obs.WriteInteger(psz_z, m_z);

    obs.WriteBool(psz_showlegend, m_bLegend);
    obs.WriteBool(psz_showxscale, m_bXScale);
    obs.WriteBeginMarker(psz_annotwnd);
    for (int i=0; i< ANNOT_WND_NUMBER; i++)
    {
        obs.WriteBool(psz_showwnd, m_abAnnWnd[i]);
    }
    obs.WriteEndMarker(psz_annotwnd);

    obs.WriteEndMarker(psz_sagraph);
}

BOOL CGraphWnd::ReadProperties(CObjectIStream & obs)
{
    if (!obs.bReadBeginMarker(psz_sagraph))
    {
        return FALSE;
    }

    while (!obs.bAtEnd())
    {
        if (obs.bReadBool(psz_showlegend, m_bLegend));
        else if (obs.bReadBool(psz_showxscale, m_bXScale));
        else if (obs.bReadBeginMarker(psz_annotwnd))
        {
            for (int i = 0; i < ANNOT_WND_NUMBER; i++)
            {
                if (!obs.bReadBool(psz_showwnd, m_abAnnWnd[i]))
                {
                    break;
                }
            }
            obs.SkipToEndMarker(psz_annotwnd);
        }
        else if (obs.bReadEndMarker(psz_sagraph))
        {
            break;
        }
        else
        {
            obs.ReadMarkedString();  // Skip unexpected field
        }
    }

    return TRUE;
}

/***************************************************************************/
// CGraphWnd::bSetProperties Set graph's default properties
//
// Set default properties for specified graph
// (needed for "FileOpenAs->Phonetic/Music Analysis")
/***************************************************************************/
BOOL CGraphWnd::bSetProperties(int nID)
{
    m_bXScale = !(nID == IDD_MAGNITUDE || nID == IDD_STAFF || nID == IDD_POA);
    m_bLegend = !(nID == IDD_POA);

    //**********************************************************************
    // 10/23/2000 - DDO  This considers the case when the melogram is
    // turned on but neither the TWC or magnitude are. When that's the case
    // then the melogram's legend is on by default.
    //**********************************************************************
    CSaView * pView = ((CMainFrame *)AfxGetMainWnd())->GetCurrSaView();
    if (pView)
    {
        if (nID == IDD_MELOGRAM && pView->GetGraphIndexForIDD(IDD_TWC) == -1)
        {
            m_bLegend = TRUE;
        }
        else if (nID == IDD_MAGNITUDE && pView->GetGraphIndexForIDD(IDD_TWC) == -1)
        {
            CGraphWnd * pGraph = (CGraphWnd *)pView->GraphIDtoPtr(IDD_MELOGRAM);
            if (pGraph)
            {
                m_bLegend = pGraph->HaveLegend();
            }
        }
    }

    if (nID == IDD_TWC || nID == IDD_STAFF)
    {
        ShowCursors(FALSE, FALSE);
    }
    SetLineDraw(nID != IDD_MAGNITUDE);

    for (int i = 0; i < ANNOT_WND_NUMBER; i++)
    {
        m_abAnnWnd[i] = (nID == IDD_RAWDATA && i == 0);
    }

    return TRUE;
}

/***************************************************************************/
// CGraphWnd::SetGraphFocus Set/reset focus for the graph and propagate
// to the plot window.
/***************************************************************************/
void CGraphWnd::SetGraphFocus(BOOL bFocus)
{
    CMiniCaptionWnd::SetGraphFocus(bFocus);
    m_pPlot->GraphHasFocus(bFocus);
}

/***************************************************************************/
// CGraphWnd::AnimateFrame  Call on plot to animate one frame if graph
// contains a plot that can be animated.
/***************************************************************************/
void CGraphWnd::AnimateFrame(DWORD dwFrameIndex)
{
    if (m_pPlot->IsAnimationPlot())
    {
        m_pPlot->AnimateFrame(dwFrameIndex);
        //!! update annotation here
    }
}

/***************************************************************************/
// CGraphWnd::EndAnimation  Call on plot to terminate animation if graph
// contains a plot that can be animated.
/***************************************************************************/
void CGraphWnd::EndAnimation()
{
    if (m_pPlot->IsAnimationPlot())
    {
        m_pPlot->EndAnimation();
    }
}

IMPLEMENT_DYNCREATE(CRecGraphWnd, CGraphWnd)

/***************************************************************************/
/***************************************************************************/
CRecGraphWnd::CRecGraphWnd() : CGraphWnd(IDD_RECORDING)
{
}

/***************************************************************************/
// CRecGraphWnd::PreCreateWindow Creation
// Called from the framework before the creation of the window. Registers
// the new window class and changes the style as desired.
//
// 09/22/2000 - DDO Created
/***************************************************************************/
BOOL CRecGraphWnd::PreCreateWindow(CREATESTRUCT & cs)
{
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
                                       AfxGetApp()->LoadStandardCursor(IDC_ARROW), 0,
                                       LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_SA_ANNTYPE)));

    BOOL bRet = CWnd::PreCreateWindow(cs);
    cs.style = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_BORDER;
    return bRet;
}
