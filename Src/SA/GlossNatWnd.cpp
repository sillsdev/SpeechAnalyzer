/////////////////////////////////////////////////////////////////////////////
// ReferenceWnd.cpp:
// Implementation of the CReferenceWnd classes.
// Author: Urs Ruchti
// copyright 2014-2018 JAARS Inc. SIL
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GlossNatWnd.h"
#include "sa_annot.h"
#include "sa_plot.h"
#include "sa_graph.h"
#include "Segment.h"
#include "sa_doc.h"
#include "sa.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "math.h"
#include "sa_g_stf.h"
#include "sa_g_twc.h"
#include "Partiture.hpp"
#include "sa_g_wavelet.h"               // ARH 8/3/01  Added to use the arrow buttons on the wavelet graph
#include "GlossSegment.h"
#include "GlossNatSegment.h"
#include "ReferenceSegment.h"
#include "Process\Process.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//SDM 1.5Test8.1
//###########################################################################
// CGlossNatWnd

CGlossNatWnd::CGlossNatWnd(int nIndex) : CAnnotationWnd(nIndex) {
}

/***************************************************************************/
// CGlossNatWnd::OnDraw Drawing
/***************************************************************************/
void CGlossNatWnd::OnDraw(CDC * pDC, const CRect & printRect) {

    CRect rWnd;
    CRect rClip; // get invalid region

    if (pDC->IsPrinting()) {
        rWnd = printRect;
        rClip = printRect;
    } else {
        GetClientRect(rWnd);
        pDC->GetClipBox(&rClip);
        pDC->LPtoDP(&rClip);
    }

    // select gloss font
    CFont * pOldFont = pDC->SelectObject(GetFont());
    // get text metrics
    TEXTMETRIC tm;
    pDC->GetTextMetrics(&tm);
    // get window coordinates
    if (rWnd.Width() == 0) {
		// nothing to draw
        return;    
    }
    // set font colors
	// get pointer to colors from main frame
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    Colors * pColors = pMainWnd->GetColors();

	// set font color
    pDC->SetTextColor(pColors->cAnnotationFont[m_nIndex]);
	// letters may overlap, so they must be transparent
    pDC->SetBkMode(TRANSPARENT);
    // draw 3D window border
    CPen penDkgray(PS_SOLID, 1, pColors->cSysBtnShadow);
    CPen penLtgray(PS_SOLID, 1, pColors->cSysBtnHilite);

    {
        CPen * pOldPen = pDC->SelectObject(&penDkgray);
        // SDM 1.06.6 make annotation windows symettrical
        pDC->MoveTo(rClip.left, rWnd.bottom - 1);
        pDC->LineTo(rClip.right, rWnd.bottom - 1);
        pDC->SelectObject(&penLtgray);
        pDC->MoveTo(rClip.left, rWnd.top);
        pDC->LineTo(rClip.right, rWnd.top);
        pDC->SelectObject(pOldPen);
    }

    // get pointer to graph, view and document
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaView * pView = (CSaView *)pGraph->GetParent();
    CSaDoc * pModel = (CSaDoc *)pView->GetDocument();

    //*******************************************************
    // 09/27/2000 - DDO If the graph is the TWC graph
    // then we don't want to draw the annotation text.
    //*******************************************************
    if (pGraph->IsPlotID(IDD_TWC)) {
		// set back old font
        pDC->SelectObject(pOldFont);
        return;
    }

    // SDM 1.06.6
    // Exclude Boundaries
    rWnd.SetRect(rWnd.left, rWnd.top + 1, rWnd.right, rWnd.bottom - 1);

    // check if area graph type
    double fDataStart;
    DWORD dwDataFrame;
    if (pGraph->IsAreaGraph()) {
        // get necessary data from area plot
        fDataStart = pGraph->GetPlot()->GetAreaPosition();
        dwDataFrame = pGraph->GetPlot()->GetAreaLength();
    } else {
        // check if graph has private cursor
        if (pGraph->HasPrivateCursor()) {
            // get necessary data from between public cursors
            WORD wSmpSize = WORD(pModel->GetSampleSize());
			// data index of first sample to display
            fDataStart = pView->GetStartCursorPosition();
			// number of data points to display
            dwDataFrame = pView->GetStopCursorPosition() - (DWORD) fDataStart + wSmpSize;
        } else {
            // get necessary data from document and from view
			// data index of first sample to display
            fDataStart = pView->GetDataPosition(rWnd.Width());
			// number of data points to display
            dwDataFrame = pView->CalcDataFrame(rWnd.Width());
        }
    }
    if (dwDataFrame == 0) {
        return;    // nothing to draw
    }

    // calculate the number of data samples per pixel
    double fBytesPerPix = (double)dwDataFrame / (double)rWnd.Width();

    // get pointer to gloss strings
    CGlossNatSegment * pGlossNat = (CGlossNatSegment *)pModel->GetSegment(m_nIndex);
    if (pGlossNat->GetOffsetSize()>0) {
        // array is not empty
        // get pointer to gloss offset and duration arrays
        // position prepare loop. Find first string to display in clipping rect
        int nLoop = 0;
        if ((fDataStart > 0) && (pGlossNat->GetOffsetSize() > 1)) {
            double fStart = fDataStart + (double)(rClip.left - tm.tmAveCharWidth) * fBytesPerPix;
            for (nLoop = 1; nLoop < pGlossNat->GetOffsetSize(); nLoop++) {
                if ((double)(pGlossNat->GetStop(nLoop)) > fStart) {
					// first string must be at lower position
					// this is it
                    nLoop--;
                    break;
                }
            }
        }
        if (nLoop < pGlossNat->GetOffsetSize()) { // there is something to display
            // display loop
            int nDisplayPos = 0;
            int nDisplayStop = 0;
            CString string;
            do {
                string.Empty();
                // get the string to display
                string = pGlossNat->GetText(nLoop);
                nDisplayPos = round2Int((pGlossNat->GetOffset(nLoop) - fDataStart) / fBytesPerPix);
                // check if the character is selected
                BOOL bSelect = FALSE;
                if (pGlossNat->GetSelection() == nLoop) {
                    bSelect = TRUE;
                }
                // calculate duration
                nDisplayStop = round2Int((pGlossNat->GetStop(nLoop) - fDataStart)/ fBytesPerPix);
                //SDM 1.06.2
				// Show New Boundaries
                if (m_bHintUpdateBoundaries) {
                    if (bSelect) {
                        nDisplayPos = round2Int((m_dwHintStart - fDataStart)/ fBytesPerPix);
                        nDisplayStop = round2Int((m_dwHintStop - fDataStart)/ fBytesPerPix);
                    } else if (pGlossNat->GetSelection() == (nLoop+1)) {
						// Segment prior to selected segment
                        nDisplayStop = round2Int((m_dwHintStart - fDataStart)/ fBytesPerPix);
                    } else if (pGlossNat->GetSelection() == (nLoop-1)) {
						// Segment after selected segment
                        nDisplayPos = round2Int((m_dwHintStop - fDataStart)/ fBytesPerPix);
                    }
                }
                if ((nDisplayStop - nDisplayPos) < 2) {
					// must be at least 2 to display a point
                    nDisplayStop++;
                }
                if ((nDisplayStop - nDisplayPos) < 2) {
					// must be at least 2 to display a point
                    nDisplayPos--;
                }
                // set rectangle to display string centered within
                rWnd.SetRect(nDisplayPos, rWnd.top, nDisplayStop, rWnd.bottom);
                // highlight background if selected character
                COLORREF normalTextColor = pDC->GetTextColor();
                if (bSelect) {
					// set highlighted text
                    normalTextColor = pDC->SetTextColor(pColors->cSysColorHiText);
                    CBrush brushHigh(pColors->cSysColorHilite);
                    CPen penHigh(PS_SOLID, 1, pColors->cSysColorHilite);
                    CBrush * pOldBrush = (CBrush *)pDC->SelectObject(&brushHigh);
                    CPen * pOldPen = pDC->SelectObject(&penHigh);
                    pDC->Rectangle(rWnd.left, rWnd.top - 1, rWnd.right, rWnd.bottom);
                    pDC->SelectObject(pOldBrush);
                    pDC->SelectObject(pOldPen);
                }

                DrawTranscriptionBorders(pDC,rWnd,pColors);

                BOOL bNotEnough = (nDisplayStop - nDisplayPos) <= tm.tmAveCharWidth;
                if (!bNotEnough) {
                    bNotEnough = ((nDisplayStop - nDisplayPos) <= (string.GetLength() * tm.tmAveCharWidth));
                }

                if (bNotEnough) {
                    if ((nDisplayStop-nDisplayPos) <= 4 * tm.tmAveCharWidth) {
						// even not enough space for at least two characters with dots
						// print first character
                        pDC->DrawText(_T("*"), 1, rWnd, DT_VCENTER | DT_SINGLELINE | DT_LEFT | DT_NOCLIP);
                    } else {
                        // draw as many characters as possible and 3 dots
                        string = string.Left((nDisplayStop-nDisplayPos) / tm.tmAveCharWidth - 2) + "...";
                        pDC->DrawText((LPCTSTR)string, string.GetLength(), rWnd, DT_VCENTER | DT_SINGLELINE | DT_LEFT | DT_NOCLIP);
                    }
                } else { // enough space to display string
                    pDC->DrawText((LPCTSTR)string, string.GetLength(), rWnd, DT_VCENTER | DT_SINGLELINE | DT_CENTER | DT_NOCLIP);
                }
                if (bSelect) {
                    pDC->SetTextColor(normalTextColor);    // set back old text color
                }
            } while ((nDisplayPos < rClip.right) && (++nLoop < pGlossNat->GetOffsetSize()));
        }
    }
    pDC->SelectObject(pOldFont);  // set back old font

    //SDM 1.06.5
    //keep up to date of changes force redraw of deselected virtual selections
    // get current selection information
    pView->UpdateSelection();

    // Show virtual selection (No text just highlight)
    if ((pView->IsSelectionVirtual()) &&
            (pView->GetSelectionIndex() == m_nIndex)) {
        int nStart = int(((double)pView->GetSelectionStart() - fDataStart)/ fBytesPerPix);
        int nStop = int(((double)pView->GetSelectionStop() - fDataStart)/ fBytesPerPix + 1);
        rWnd.SetRect(nStart, rWnd.top, nStop, rWnd.bottom);
        CBrush brushBk(pColors->cSysColorHilite);
        CPen penHigh(PS_INSIDEFRAME, 1, pColors->cSysColorHilite);
        CBrush * pOldBrush = (CBrush *)pDC->SelectObject(&brushBk);
        CPen * pOldPen = pDC->SelectObject(&penHigh);
        pDC->Rectangle(rWnd.left, rWnd.top + 1, rWnd.right, rWnd.bottom - 1);
        pDC->SelectObject(pOldBrush);
        pDC->SelectObject(pOldPen);
    }
	// set back old font
    pDC->SelectObject(pOldFont);
}

