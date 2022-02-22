/////////////////////////////////////////////////////////////////////////////
// sa_g_mel.cpp:
// Implementation of the CPlotMelogram class.
// Author: Todd Jones
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_plot.h"
#include "sa_g_mel.h"
#include "sa_minic.h"
#include "sa_graph.h"
#include "mainfrm.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "math.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CPlotMelogram
// Plot window for displaying melogram pitch data. This class only handles the
// special drawing of the data. All other work is done in the base class.

IMPLEMENT_DYNCREATE(CPlotMelogram, CPlotWnd)

/////////////////////////////////////////////////////////////////////////////
// CPlotMelogram message map

BEGIN_MESSAGE_MAP(CPlotMelogram, CPlotWnd)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotMelogram construction/destruction/creation

/***************************************************************************/
// CPlotMelogram::CPlotMelogram Constructor
/***************************************************************************/
CPlotMelogram::CPlotMelogram() {
    m_bLineDraw = FALSE;
    m_bDotDraw = TRUE;
}


CPlotWnd * CPlotMelogram::NewCopy() {
    CPlotWnd * pRet = new CPlotMelogram;

    CopyTo(pRet);

    return pRet;
}


/***************************************************************************/
// CPlotMelogram::~CPlotMelogram Destructor
/***************************************************************************/
CPlotMelogram::~CPlotMelogram() {
}

/////////////////////////////////////////////////////////////////////////////
// CPlotMelogram message handlers

int CPlotMelogram::GetPenThickness() const {
    int nLineThickness = (short)(m_fVScale  * 2.0 * (double)MEL_ACCURACY + 0.5);
    nLineThickness = nLineThickness - (nLineThickness % 2) + 1;

    return nLineThickness;
}

CGrid CPlotMelogram::GetGrid() const {
    CGrid modifiedGrid(*static_cast<CMainFrame *>(AfxGetMainWnd())->GetGrid());

    if (GetTWC()) {
        // nPenStyle = PS_DASHDOT;
        modifiedGrid.nYStyle = 4; // Dashdot
    } else {
        // nPenStyle = PS_SOLID;
        modifiedGrid.nYStyle = 1;
    }

    return modifiedGrid;
}

bool CPlotMelogram::GetScaleValues(CSaDoc * pModel, double * dMaxSemitone,double * dMinSemitone) {
    const CMusicParm * pParm = pModel->GetMusicParm();

    int nUpperBound = pParm->nUpperBound;
    int nLowerBound = pParm->nLowerBound;

    if (pParm->nRangeMode == 0) {
        CMusicParm::GetAutoRange(pModel, pModel->GetMelogram(), nUpperBound, nLowerBound);
    }

    if (dMaxSemitone) {
        *dMaxSemitone = nUpperBound;
    }

    if (dMinSemitone) {
        *dMinSemitone = nLowerBound;
    }

    CProcessMelogram * pMelogram = (CProcessMelogram *)pModel->GetMelogram(); // get pointer to melogram object
    return (pMelogram->IsDataReady() == TRUE);
}

/***************************************************************************/
// CPlotMelogram::OnDraw Drawing
// The data to draw is coming from a temporary file, created by the melogram
// processing class, which is called to do data processing if necessary
// before drawing. Call the function PlotPaintFinish at the end of the
// drawing to let the plot base class do common jobs like drawing the
// cursors.
/***************************************************************************/
void CPlotMelogram::OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView) {
    TRACE(_T("Draw Mel\n"));
    // get pointer to graph, view and document
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaDoc  *  pModel   = pView->GetDocument();
    // create melogram data
    CProcessMelogram * pMelogram = (CProcessMelogram *)pModel->GetMelogram(); // get pointer to melogram object
    short int nResult = LOWORD(pMelogram->Process(this, pModel)); // process data
    nResult = CheckResult(nResult, pMelogram); // check the process result
    if (nResult == PROCESS_ERROR) {
        return;
    } else if (pMelogram->IsStatusFlag(PROCESS_NO_PITCH)) {
        m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_NOPITCH, &rWnd);
    } else if (pMelogram->IsDataReady()) {
        m_HelperWnd.SetMode(MODE_HIDDEN);

        double dMaxSemitone;
        double dMinSemitone;

        GetScaleValues(pModel, &dMaxSemitone,&dMinSemitone);

        // always use linear display
        pGraph->SetLegendScale(SCALE | NUMBERS, dMinSemitone, dMaxSemitone, _T("Semitones")); // set legend scale

        const CMusicParm * pParm = pModel->GetMusicParm();

        if (pParm->nCalcRangeMode != 0) {
            // Load a font for tagging formants.
            CFont Font;
            LPCTSTR pszName = _T("MS Sans Serif");
            Font.CreateFont(-12, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                            CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, (VARIABLE_PITCH | FF_ROMAN), pszName);  // create font

            CFont * pOldFont = pDC->SelectObject(&Font); // select reference font for tags

            CString szTag;

            szTag.Format(_T("U: %dst L: %dst"), pParm->nCalcUpperBound, pParm->nCalcLowerBound);
            CSize sizeTag = pDC->GetTextExtent(szTag);
            CRect rInvalid(0, 4, 4 + sizeTag.cx + rClip.Width(), 4 + sizeTag.cy);
            if (rClip.bottom > rInvalid.bottom) {   // if the redraw was caused by a pan/zoom action
                InvalidateRect(&rInvalid);    // force new text to be redrawn and wipe out old text
            }
            pDC->TextOut(4,4,szTag);
            pDC->SelectObject(pOldFont); // restore old font
        }

        // do common plot paint jobs
        SetTWC(pView->GetGraphIndexForIDD(IDD_TWC) != -1);
        if (GetTWC()) {
            // We want to use TWC legend as the basis for Melograms gridlines
            {
                // Hide our grid lines and do standard processing
                // This calculates the correct scaling for the standard paint
                BOOL bGrid = ShowGrid(FALSE);
                PlotPrePaint(pDC, rWnd, rClip);
                ShowGrid(bGrid);
            }
            // Show TWC grid lines
            short nTwcGraphIndex = (short)pView->GetGraphIndexForIDD(IDD_TWC);
            CGraphWnd * pTwcGraph = pView->GetGraph(nTwcGraphIndex);
            pTwcGraph->GetPlot()->UpdateWindow(); // make sure TWC is processed and up to date
            PlotPrePaint(pDC, rWnd, rClip, pTwcGraph->GetLegendWnd());
        } else {
            PlotPrePaint(pDC, rWnd, rClip);
        }


        SetProcessMultiplier(MEL_MULT);

        PlotStandardPaint(pDC, rWnd, rClip, pMelogram, pModel, (m_bDotDraw ? PAINT_DOTS : 0) | SKIP_UNSET); // do standard data paint
    }
    // do common plot paint jobs
    PlotPaintFinish(pDC, rWnd, rClip);

}
