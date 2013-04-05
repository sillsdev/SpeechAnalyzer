/////////////////////////////////////////////////////////////////////////////
// sa_g_dur.cpp:
// Implementation of the CPlotDurations class.
// Author: Alec Epting
// copyright 1998 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_plot.h"
#include "sa_g_dur.h"
#include "Process\Process.h"
#include "Process\sa_p_dur.h"
#include "sa_minic.h"
#include "sa_graph.h"
#include "Segment.h"
#include "dsp\mathx.h"

#include "sa_doc.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CPlotDurations
// Plot window for displaying duration data. This class only handles the
// special drawing of the data. All other work is done in the base class.

IMPLEMENT_DYNCREATE(CPlotDurations, CPlotWnd)

/////////////////////////////////////////////////////////////////////////////
// CPlotDuration message map

BEGIN_MESSAGE_MAP(CPlotDurations, CPlotWnd)
    //{{AFX_MSG_MAP(CPlotDurations)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotDuration construction/destruction/creation

/***************************************************************************/
// CPlotDuration::CPlotDuration Constructor
/***************************************************************************/
CPlotDurations::CPlotDurations()
{
}


void  CPlotDurations::CopyTo(CPlotWnd * pT)
{
    CPlotWnd::CopyTo(pT);
}



CPlotWnd * CPlotDurations::NewCopy(void)
{
    CPlotWnd * pRet = new CPlotDurations();

    CopyTo(pRet);

    return pRet;
}


/***************************************************************************/
// CPlotDurations::~CPlotDurations Destructor
/***************************************************************************/
CPlotDurations::~CPlotDurations()
{
}


/////////////////////////////////////////////////////////////////////////////
// CPlotDurations message handlers

/***************************************************************************/
// CPlotDurations::OnDraw drawing
// The data to draw comes from the document's offset and duration chunk. Call the
// function PlotPaintFinish at the end of the drawing to let the plot base
// class do common jobs like drawing the cursors.
/***************************************************************************/
void CPlotDurations::OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView)
{
    // get pointer to main frame, graph, and document
    CSaDoc * pDoc = pView->GetDocument();
    // create loudness data
    CProcessDurations * pDurations = (CProcessDurations *)pDoc->GetDurations(); // get pointer to loudness object
    short int nResult = LOWORD(pDurations->Process(this, pDoc)); // process data
    nResult = CheckResult(nResult, pDurations); // check the process result
    if (nResult == PROCESS_ERROR)
    {
        return;
    }
    if (nResult != PROCESS_CANCELED)
    {
        SetBold(FALSE);
        SetProcessMultiplier(PRECISION_MULTIPLIER);
        DurationsPaint(0, pDurations->GetMaxDuration(), pDC, rWnd, rClip, pDurations); // do standard data paint
    }
    else
    {
        PlotPaintFinish(pDC, rWnd, rClip);
    }
}


/***************************************************************************/
// CPlotWnd::DurationsPaint
//**************************************************************************/
void CPlotDurations::DurationsPaint(DWORD /*dwMinDuration*/, DWORD dwMaxDuration, CDC * pDC,
                                    CRect rWnd, CRect rClip,
                                    CProcess * pProcess, int nFlags)
{
    UNUSED_ALWAYS(nFlags);
    // get pointer to graph, view and document
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaView * pView = (CSaView *)pGraph->GetParent();
    CSaDoc * pDoc = pView->GetDocument();
    UINT nSmpSize = pDoc->GetSampleSize();      // number of bytes per sample

    // get pointer to phonetic string
    CSegment * pSegment = pDoc->GetSegment(PHONETIC);
    if (!pSegment->IsEmpty())   // string is not empty
    {
        m_HelperWnd.SetMode(MODE_HIDDEN);

        // get pointer to phonetic offset and duration arrays
        CRect rDraw(rClip);

        // get necessary data from document and from view
        DWORD dwDataFrame = pView->AdjustDataFrame(rWnd.Width()); // number of data points to display
        if (dwDataFrame == 0)
        {
            return;    // nothing to draw
        }

        // Conditionally inflate drawing area
        if (rDraw.left > 0)
        {
            rDraw.left--;
        }
        if (rDraw.right < rWnd.Width())
        {
            rDraw.right++;
        }

        double fHPixPerByte = (double)rWnd.Width() / (double)dwDataFrame;
        double fDataPos   = pView->GetDataPosition(rWnd.Width()); // data index of first sample to display
        DWORD dwDataStart = DWORD(fDataPos + (rDraw.left / fHPixPerByte));

        DWORD dwDataEnd = (DWORD)(fDataPos + rDraw.right / fHPixPerByte + 0.5);
        if (dwDataEnd >= fDataPos + dwDataFrame)
        {
            dwDataEnd = DWORD(fDataPos + int(dwDataFrame) - nSmpSize);
        }

        // find first segment in clip region
        int nSymbol = 0;
        DWORD dwSegOffset = 0;
        for (int nSegStart = 0; nSegStart != -1; nSegStart = pSegment->GetNext(nSegStart))
        {
            if ((DWORD)pSegment->GetStop(nSegStart) >= dwDataStart)
            {
                break;    // quit at segment just beyond start of clip region
            }
            nSymbol = nSegStart;  // index to first symbol of segment
            dwSegOffset = (DWORD)pSegment->GetOffset(nSegStart);
        }

        dwMaxDuration /= (DWORD)nSmpSize;   //convert from bytes to samples
        double fMaxDuration = (double)dwMaxDuration / (double)pDoc->GetSamplesPerSec();  //in seconds

        double fMaxBarTime = (double)ceil(fMaxDuration*100.)/100.;  // time in seconds, set to
        // nearest 10 msec interval at or above maximum duration

        // do common plot paint jobs
        //!!scale should adjust if max bar stretched
        pGraph->SetLegendScale(SCALE | NUMBERS, 0, (int)(fMaxBarTime * 1000.),
                               _T("Duration (ms)")); // set legend scale     //!!allow for scaling up
        PlotPrePaint(pDC, rWnd, rClip);

        // get pointer to color structure from main frame
        CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
        Colors * pColor = pMainWnd->GetColors();

        m_fVScale = (double)rWnd.Height() / ((double)fMaxBarTime * (double)pDoc->GetSamplesPerSec() * (double)nSmpSize);

        int nLineThickness = GetPenThickness();
        CPen penData(PS_SOLID, nLineThickness, pColor->cPlotData[0]);
        CPen * pOldPen = pDC->SelectObject(&penData);

        RECT rBar;
        rBar.right = -1;
        rBar.bottom = rWnd.bottom;   //!!-3 like other plots?
        rBar.top = rBar.bottom;

        if (nSymbol > 0)
        {
            DWORD dwPrevDuration = pProcess->GetProcessedData(nSymbol-1);
            rBar.top = rBar.bottom - round(dwPrevDuration * m_fVScale);
        }

        while (pSegment->GetOffset(nSymbol) <= dwDataEnd)
        {
            DWORD dwDuration = pProcess->GetProcessedData(nSymbol);

            rBar.left = round((pSegment->GetOffset(nSymbol) - fDataPos) *
                              fHPixPerByte);
            pDC->MoveTo(rBar.left, rBar.bottom);  // space before segment

            rBar.top = rBar.bottom - round(dwDuration * m_fVScale);
            pDC->LineTo(rBar.left, rBar.top);        // left side of bar

            rBar.right = round((pSegment->GetStop(nSymbol) - fDataPos) * fHPixPerByte);
            pDC->LineTo(rBar.right, rBar.top);  // bar top (duration level)

            pDC->LineTo(rBar.right, rBar.bottom);  // right side of bar

            nSymbol = pSegment->GetNext(nSymbol);
            if (nSymbol == -1)
            {
                break;
            }
        }

        // restore old pen
        pDC->SelectObject(pOldPen);


        // do common plot paint jobs
        PlotPaintFinish(pDC, rWnd, rClip);

    }
    else
    {
        CRect rClient;
        GetClientRect(rClient);
        CDC * pDC = GetDC();
        CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
        Colors * pColor = pMainWnd->GetColors(); // get application colors
        CBrush Eraser(pColor->cPlotBkg);
        m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_NO_DURATIONS, &rClient);

        ReleaseDC(pDC);
    }
}


