/////////////////////////////////////////////////////////////////////////////
// sa_g_poa:
// Implementation of the CPlotPOA class.
// Author: Alec Epting
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_plot.h"
#include "sa_g_poa.h"
#include "Process\Process.h"
#include "Process\sa_p_poa.h"
#include "Process\sa_p_fra.h"
#include "Process\sa_p_gra.h"
#include "sa_minic.h"
#include "sa_graph.h"
#include <math.h>
#include "mainfrm.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "stpwatch.h"
#include "FaceDiag.h"
#include "Process\sa_p_poa.h"
#include "Lpc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CPlotPOA
// Plot window for displaying point-of-articulation half-face diagram. This
// class only handles the special drawing of the data. All other work is done
// in the base class.

IMPLEMENT_DYNCREATE(CPlotPOA, CPlotWnd)

/////////////////////////////////////////////////////////////////////////////
// CPlotPOA message map

BEGIN_MESSAGE_MAP(CPlotPOA, CPlotWnd)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotPOA construction/destruction/creation

/***************************************************************************/
// CPlotPOA::CPlotPOA Constructor
/***************************************************************************/
CPlotPOA::CPlotPOA() {
    m_bAnimationPlot = TRUE;
    m_dwAnimationFrame = UNDEFINED_OFFSET;
    m_dwFrameStart = UNDEFINED_OFFSET;  // set to invalid value to force change
    m_bShowModelData = TRUE;
}

/***************************************************************************/
// CPlotPOA::~CPlotPOA Destructor
/***************************************************************************/
CPlotPOA::~CPlotPOA() {
    GraphHasFocus(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CPlotPOA message handlers


/***************************************************************************/
// CPlotPOA::OnDraw drawing
// The data to draw is generated from a CLinPredCoding object constructed
// in this function.
/***************************************************************************/
void CPlotPOA::OnDraw(CDC * pDC, CRect rWnd, CRect /*rClip*/, CSaView * pView) {
    // Get dimensions of client window.
    int nWndWidth = rWnd.Width();
    int nWndHeight = rWnd.Height();

    // Get waveform and buffer parameters.
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
    WORD wSmpSize = (WORD)(pDoc->GetSampleSize());      //compute sample size in bytes

    // Get pointer to raw waveform plot.
    int nWaveGraphIndex = pView->GetGraphIndexForIDD(IDD_RAWDATA);
    CGraphWnd * pWaveGraph = pView->GetGraph(nWaveGraphIndex);
    CPlotWnd * pWavePlot = NULL;
    if (pWaveGraph) {
        pWavePlot = pWaveGraph->GetPlot();
    }

    // Get frame parameters.
    bool bDynamicUpdate = (pView->GetGraphUpdateMode() == DYNAMIC_UPDATE);
    CProcessGrappl * pAutoPitch = pDoc->GetGrappl();   // get pitch process object
    CProcessFragments * pFragments = pDoc->GetFragments(); // get fragmenter object
    short int nResult = LOWORD(pFragments->Process(this, pDoc)); // process data
    if (nResult == PROCESS_ERROR || nResult == PROCESS_NO_DATA || nResult == PROCESS_CANCELED) {
        if (!bDynamicUpdate) {
            nResult = CheckResult(nResult, pFragments);    // check the process result
        }
        return;
    }

    DWORD dwStartCursorPosn = pView->GetStartCursorPosition();
    DWORD dwFrameIndex;
    if (bDynamicUpdate) {
        if (m_dwAnimationFrame == UNDEFINED_OFFSET) {
            // initialize animation frame for new graph
            DWORD dwStartCursor = pView->GetStartCursorPosition();
            m_dwAnimationFrame = pFragments->GetFragmentIndex(dwStartCursor/wSmpSize);
        }
        dwFrameIndex = m_dwAnimationFrame;
    } else {
        dwFrameIndex = pFragments->GetFragmentIndex(dwStartCursorPosn / wSmpSize);
    }
    SFragParms FragParms = pFragments->GetFragmentParms(dwFrameIndex);
    DWORD dwFrameStart = FragParms.dwOffset * wSmpSize;
    DWORD dwFrameSize = (DWORD)FragParms.wLength * (DWORD)wSmpSize;
    DWORD dwFrameEnd = dwFrameStart + dwFrameSize - wSmpSize;

    CGraphWnd * pGraph = GetGraph();
    BOOL bFocus = (pGraph == pView->GetFocusedGraphWnd());
    if ((pWavePlot) && ((bDynamicUpdate) || ((!bDynamicUpdate) && bFocus))) {
        // highlight fragment in raw data
        pWavePlot->SetHighLightArea(dwFrameStart, dwFrameStart + dwFrameSize, TRUE, TRUE);
        pWavePlot->UpdateWindow();
    }

    // Check for valid frame.
    CProcessPOA * pPOA = (CProcessPOA *)pDoc->GetPOA(); // get pointer to POA object
    if (!pAutoPitch->IsVoiced(pDoc, dwFrameStart) || !pAutoPitch->IsVoiced(pDoc, dwFrameStart + dwFrameSize - wSmpSize)) {
        // frame is not entirely voiced
		// get application colors
        Colors * pColor = pMainWnd->GetColors(); 
        CBrush Eraser(pColor->cPlotBkg);
		// clear the plot area
        pDC->FillRect(&rWnd, &Eraser);  
        if (!bDynamicUpdate) {
            m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_SELECTVOICED, &rWnd);
        } else {
            m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_UNVOICED, &rWnd);
        }
        if (pPOA->IsDataReady()) {
            pPOA->SetDataInvalid();
        }
        return;
    }
    if ((!bDynamicUpdate) && (dwStartCursorPosn != dwFrameStart)) {
        // start cursor not aligned to start of a pitch period
        m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_START2PITCH, &rWnd);
        if (pPOA->IsDataReady()) {
            pPOA->SetDataInvalid();
        }
        return;
    }
    // Determine if processing needs to be done.
    if (dwFrameStart != m_dwFrameStart) {
        // cursor has been moved, so request processing
        m_dwFrameStart = dwFrameStart;
        pPOA->SetDataInvalid();   // force recalculation
    }

    nResult = LOWORD(pPOA->Process(this, pDoc, dwFrameStart, dwFrameEnd));  // process data
    nResult = CheckResult(nResult, pPOA); // check the process result
    if (nResult == PROCESS_ERROR) {
        MessageBox(_T("Error occurred in vocal tract analysis"), _T("Vocal Tract"), MB_OK | MB_ICONEXCLAMATION);
        OnSysCommand(SC_CLOSE, 0L); // close the graph
        return;
    }

    if (nResult != PROCESS_CANCELED) {
        m_HelperWnd.SetMode(MODE_HIDDEN);
        SVocalTractModel * pVocalTract = (SVocalTractModel *)pPOA->GetProcessedData();

        // Alter the face diagram.
        CBitmap bmFace;
        bmFace.LoadBitmap(IDB_FACE);      //load the face diagram
        CBitmap bmJaw;
        bmJaw.LoadBitmap(IDB_JAW);        //load the jaw bitmap
        CBitmap bmTongue;
        bmTongue.LoadBitmap(IDB_TONGUE);  //load the tongue bitmap

        BITMAP FaceMetrics;
        bmFace.GetObject(sizeof(FaceMetrics), &FaceMetrics);       //get dimensions of face

        BITMAP JawMetrics;
        bmJaw.GetObject(sizeof(JawMetrics), &JawMetrics);          //get dimensions of jaw

        BITMAP TongueMetrics;
        bmTongue.GetObject(sizeof(TongueMetrics), &TongueMetrics); //get dimensions of tongue

        CDC  dcFace, dcJaw, dcTongue;
        if (!dcFace.CreateCompatibleDC(pDC) ||
                !dcJaw.CreateCompatibleDC(pDC) ||
                !dcTongue.CreateCompatibleDC(pDC)) { //create memory space with a compatible device context
            MessageBox(_T("Error occurred in vocal tract plot"), _T("Vocal Tract"), MB_OK | MB_ICONEXCLAMATION);
            OnSysCommand(SC_CLOSE, 0L); //close the graph
            return;
        }

        CBitmap * pOldFace = dcFace.SelectObject(&bmFace);      //load face bitmap into memory device
        CBitmap * pOldTongue = dcTongue.SelectObject(&bmTongue); //load tongue bitmap into memory device
        CBitmap * pOldJaw = dcJaw.SelectObject(&bmJaw);         //load jaw bitmap into memory device

		//get application colors
        Colors * pColor = pMainWnd->GetColors();
		//get pen
        CPen Pen(PS_SOLID, 1, pColor->cPlotData[0]);
		//select pen for plotting
        CPen * pOldPen = dcFace.SelectObject(&Pen);

        static SVocalTractCoord VocalTractPlotSect[] = {
#include "voctract.h"    //locate the vocal tract sections in the plot
        };

        dcFace.MoveTo(VocalTractPlotSect[0].X - GLOTTIS_DIAMETER, VocalTractPlotSect[0].Y); //start plotting
        //from front of
        //glottis
        short x = 0, y = 0;
        for (unsigned short i = 1; i <= pVocalTract->nNormCrossSectAreas - 3; i++) { //continue plotting front
            //of vocal tract wall
            double NormDiam = pow(pVocalTract->dNormCrossSectArea[pVocalTract->nNormCrossSectAreas-i-1], 0.5)*
                              GLOTTIS_DIAMETER; //normalize to glottis
            x = (short)Round((double)VocalTractPlotSect[i].X +
                             NormDiam*cos(VocalTractPlotSect[i].RadAng));
            y = (short)Round((double)VocalTractPlotSect[i].Y -
                             NormDiam*sin(VocalTractPlotSect[i].RadAng));
            dcFace.LineTo(x, y);
            dcFace.LineTo(VocalTractPlotSect[i].X, VocalTractPlotSect[i].Y);
            dcFace.MoveTo(x, y);
        }

        CRect rTongue(TONGUE_ULX, TONGUE_ULY, TONGUE_LRX, TONGUE_LRY);
        dcFace.BitBlt(x - rTongue.Width(), y, rTongue.Width(), rTongue.Height(),
                      &dcTongue, TONGUE_ULX, TONGUE_ULY, SRCAND);  //attach tongue tip

        //!! teeth

        CRect rJaw(JAW_ULX, JAW_ULY, JAW_LRX, JAW_LRY);
        dcFace.BitBlt(VocalTractPlotSect[pVocalTract->nNormCrossSectAreas-1].X - 13,
                      VocalTractPlotSect[pVocalTract->nNormCrossSectAreas-1].Y +
                      (short)Round(pow(pVocalTract->dNormCrossSectArea[0], 0.5)*GLOTTIS_DIAMETER),
                      rJaw.Width(), rJaw.Height(),
                      &dcJaw, JAW_ULX, JAW_ULY, SRCAND);    //attach jaw

        // Copy the face diagram to the client window, sizing it to fit without changing
        // the aspect ratio.
        float fScaleFac;
        int nTopMargin, nLeftMargin;
        int nImageWidth, nImageHeight;

        if (nWndHeight < nWndWidth) { //scale along shorter dimension
            fScaleFac = (float)nWndHeight/(float)FaceMetrics.bmHeight;
            nImageWidth = (int)((float)FaceMetrics.bmWidth*fScaleFac + 0.5F);    //scale width
            nImageHeight = (int)((float)FaceMetrics.bmHeight*fScaleFac + 0.5F);  //scale height
            nLeftMargin = (nWndWidth - nImageWidth)/2;
            nTopMargin = 0;
        } else {
            fScaleFac = (float)nWndWidth/(float)FaceMetrics.bmWidth;
            nImageWidth = (int)((float)FaceMetrics.bmWidth*fScaleFac + 0.5F);    //scale width
            nImageHeight = (int)((float)FaceMetrics.bmHeight*fScaleFac + 0.5F);  //scale height
            nLeftMargin = 0;
            nTopMargin = (nWndHeight - nImageHeight)/2;
        }

        pDC->StretchBlt(nLeftMargin, nTopMargin, //center the image
                        nImageWidth, nImageHeight, &dcFace, 0, 0,
                        FaceMetrics.bmWidth, FaceMetrics.bmHeight, SRCCOPY);

        if (m_bShowModelData) {
            // write model data
            CFont TextFont;
            int nRows = pVocalTract->nNormCrossSectAreas / 2 + 1;  //split across 2 columns and include heading
            int nRowSpacing = 2;
            int nCharHeight = (nImageHeight - (nRows - 1) * nRowSpacing) / nRows;
            TCHAR * pszName = _T("MS Sans Serif");
            TextFont.CreateFont(nCharHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                                ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                DEFAULT_QUALITY, (VARIABLE_PITCH | FF_ROMAN), pszName);
            CFont * pOldFont = pDC->SelectObject(&TextFont); //select font
            // get text metrics
            //TEXTMETRIC TextMetric;
            //pDC->GetTextMetrics(&TextMetric);
            CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd(); // get pointer to colors from main frame
            Colors * pColors = pMainWnd->GetColors();
            pDC->SetTextColor(pColors->cScaleFont); // set font color
            pDC->SetBkMode(TRANSPARENT); // letters may overlap, so they must be transparent

            TCHAR szText[128];
            swprintf_s(szText, _countof(szText), _T("%% Error: %6.2f"), pVocalTract->dErrorRatio * 100.);
            CRect rText(0,0, nLeftMargin - 40, 10);
            pDC->DrawText(szText, -1, rText, DT_SINGLELINE | DT_TOP | DT_RIGHT | DT_NOCLIP);

            int nRowPitch = nCharHeight + nRowSpacing;
            int nRowX = nLeftMargin + nImageWidth;
            int nRowY = nRowPitch;
            for (size_t i = 0; i < (UINT)nRows - 1; i++, nRowY += nRowPitch) {  //left column
                if (i == 0) {
                    swprintf_s(szText,_countof(szText), _T("  Lips:  %6.2f"), pVocalTract->dNormCrossSectArea[i]);
                } else {
                    swprintf_s(szText, _countof(szText), _T("         %6.2f"), pVocalTract->dNormCrossSectArea[i]);
                }
                rText.SetRect(0, nRowY, nLeftMargin - 40, 10);
                pDC->DrawText(szText, -1, rText, DT_SINGLELINE | DT_TOP | DT_RIGHT | DT_NOCLIP);
            }

            nRowX = nLeftMargin + nImageWidth;
            nRowY = nRowPitch*(nRows - 1);
            rText.SetRect(nRowX, nRowY, nWndWidth, nRowY + 10);
            swprintf_s(szText,_countof(szText), _T("Glottis: %6.2f"), pVocalTract->dNormCrossSectArea[pVocalTract->nNormCrossSectAreas-1]);
            pDC->DrawText(szText, -1, rText, DT_SINGLELINE | DT_TOP | DT_RIGHT | DT_NOCLIP | DT_CALCRECT);
            int nRowXEnd = nRowX + rText.Width();
            pDC->DrawText(szText, -1, rText, DT_SINGLELINE | DT_TOP | DT_RIGHT | DT_NOCLIP);
            nRowY = 0;
            for (size_t i = 0; i < (UINT)nRows - 1; i++, nRowY += nRowPitch) {  //right column
                swprintf_s(szText, _countof(szText), _T("         %6.2f"), pVocalTract->dNormCrossSectArea[i + nRows - 1]);
                //rText.SetRect(nWndWidth - strlen(szText)*TextMetric.tmMaxCharWidth, (nCharHeight+nRowSpacing)*i, 100, 10);
                rText.SetRect(nRowX, nRowY, nRowXEnd, nRowY + 10);
                pDC->DrawText(szText, -1, rText, DT_SINGLELINE | DT_TOP | DT_RIGHT | DT_NOCLIP);
            }
            pDC->SelectObject(pOldFont);   //restore old font
        }

        dcFace.SelectObject(pOldPen);       //restore old pen
        dcFace.SelectObject(pOldFace);      //restore bitmap for face object
        dcTongue.SelectObject(pOldTongue);  //restore bitmap for tongue object
        dcJaw.SelectObject(pOldJaw);        //restore bitmap for jaw object
        bmFace.DeleteObject();              //delete bitmap from face object
        bmTongue.DeleteObject();            //delete bitmap from tongue object
        bmJaw.DeleteObject();               //delete bitmap from jaw object
    }

}


/***************************************************************************/
// CPlotPOA::GraphHasFocus  If graph for this plot has focus and the plot is
// not animating, highlights frame in the raw waveform plot; otherwise, turns
// highlighting off.  This will indicate that the vocal tract model is
// computed only for a single fragment and not for the entire region between
// the start and stop cursors.
/***************************************************************************/
void CPlotPOA::GraphHasFocus(BOOL bFocus) {
    // Get pointer to raw waveform plot.
    // the following method for getting to the View doesn't work properly when called from the destructor
    //CGraphWnd* pGraphWnd = (CGraphWnd *)GetParent();
    //CSaView* pView = (CSaView *)pGraphWnd->GetParent();
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    CSaView * pView = (pMainWnd)?(CSaView *)pMainWnd->GetCurrSaView():NULL;
    if (!pView) {
        return;
    }
    int nWaveGraphIndex = pView->GetGraphIndexForIDD(IDD_RAWDATA);
    CGraphWnd * pWaveGraph = pView->GetGraph(nWaveGraphIndex);
    if (pWaveGraph) {
        CPlotWnd * pWavePlot = pWaveGraph->GetPlot();
        if (bFocus) {
            CSaDoc * pDoc = pView->GetDocument();
            WORD wSmpSize = (WORD)(pDoc->GetSampleSize());          //compute sample size in bytes
            CProcessFragments * pFragments = pDoc->GetFragments();  // get fragmenter object
            pFragments->Process(this, pDoc);                        // process data
            if (pFragments->IsDataReady()) {
                DWORD dwFrameIndex;
                bool bDynamicUpdate = (pView->GetGraphUpdateMode() == DYNAMIC_UPDATE);
                if ((bDynamicUpdate) && (m_dwAnimationFrame != UNDEFINED_OFFSET)) {
                    dwFrameIndex = m_dwAnimationFrame;
                } else {
					// get start cursor position in bytes
                    DWORD dwStartCursorPosn = pView->GetStartCursorPosition();  
                    dwFrameIndex = pFragments->GetFragmentIndex(dwStartCursorPosn / wSmpSize);
                }
                SFragParms FragParms = pFragments->GetFragmentParms(dwFrameIndex);
				// frame start in bytes
                DWORD dwFrameStart = FragParms.dwOffset * wSmpSize;
				// frame size in bytes
                DWORD dwFrameSize = (DWORD)FragParms.wLength * (DWORD)wSmpSize;
				// highlight the fragment in the raw waveform
                pWavePlot->SetHighLightArea(dwFrameStart, dwFrameStart + dwFrameSize, TRUE, TRUE);  
                pWavePlot->UpdateWindow();
            }
        } else if (!pView->IsAnimating()) {
            // not in focus and not animating
			// turn off highlighting in raw waveform
            pWavePlot->ClearHighLightArea();
            pWavePlot->UpdateWindow();
        }
    }
}

/***************************************************************************/
// CPlotPOA::AnimateFrame  Animate a single frame using the base plot class
// function with additional options for this plot.
//!!this will change when property sheet is created for the vocal tract
//!!graph
/***************************************************************************/
void CPlotPOA::AnimateFrame(DWORD dwFrameIndex) {
    m_bShowModelData = FALSE;
    CPlotWnd::StandardAnimateFrame(dwFrameIndex);
}

/***************************************************************************/
// CPlotPOA::EndAnimation  Terminate the animation with additional options
// for this plot, returning the plot to its resting state.
//!!this will change when property sheet is created for the vocal tract
//!!graph
/***************************************************************************/
void CPlotPOA::EndAnimation() {
    m_bShowModelData = TRUE;
    CPlotWnd::StandardEndAnimation();
}
