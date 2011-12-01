/////////////////////////////////////////////////////////////////////////////
// sa_g_cep.cpp:
// Implementation of the CPlotCepstralPitch class. 
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "sa_g_cep.h"
#include "math.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CPlotCepstralPitch
// Plot window for displaying cepstral pitch data. This class only handles
// the special drawing of the data. All other work is done in the base class.

IMPLEMENT_DYNCREATE(CPlotCepstralPitch, CPlotWnd)

/////////////////////////////////////////////////////////////////////////////
// CPlotCepstralPitch message map

BEGIN_MESSAGE_MAP(CPlotCepstralPitch, CPlotWnd)
	//{{AFX_MSG_MAP(CPlotCepstralPitch)
	ON_WM_CREATE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotCepstralPitch construction/destruction/creation

/***************************************************************************/
// CPlotCepstralPitch::CPlotCepstralPitch Constructor
/***************************************************************************/
CPlotCepstralPitch::CPlotCepstralPitch()
{
}

/***************************************************************************/
// CPlotCepstralPitch::~CPlotCepstralPitch Destructor
/***************************************************************************/
CPlotCepstralPitch::~CPlotCepstralPitch()
{
}

/////////////////////////////////////////////////////////////////////////////
// CPlotCepstralPitch message handlers

/***************************************************************************/
// CPlotCepstralPitch::OnCreate Window creation
// The plot has to get the area process index, because it is a area process
// plot.
/***************************************************************************/
int CPlotCepstralPitch::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPlotWnd::OnCreate(lpCreateStruct) == -1) return -1;
	// get pointer to document  
	CView* pView = (CView*)GetParent()->GetParent();
	CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();
	// get pointer to process class
	CProcessFormants* pFormants = (CProcessFormants*)pDoc->GetFormants(); // get pointer to formants object
	// get area process index
	m_nProcessIndex = pFormants->GetNextIndex();
	// add the median filter      

	PitchParm * parms = pDoc->GetPitchParm();      
	if (parms->bUseCepMedianFilter) 
	{
		pFormants->AddFilter(MEDIAN_FILTER, parms->nCepMedianFilterSize, 0);
	} 
	else 
	{
		pFormants->RemoveFilter(MEDIAN_FILTER);
	}
	// save process
	m_pAreaProcess = pFormants;
	return 0;
}

/***************************************************************************/
// CPlotCepstralPitch::OnDraw Drawing
// The data to draw is coming from a temporary file, created by the
// formants processing class, which is called to do data processing if
// necessary before drawing. Call the function PlotPaintFinish at the end
// of the painting to let the plot base class do common jobs like painting
// the cursors.
/***************************************************************************/
void CPlotCepstralPitch::OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView)
{
	if (IsIconic()) return; // nothing to draw
	// get pointer to graph, view, document and legend window   
	CGraphWnd* pGraph = (CGraphWnd*)GetParent();
	CLegendWnd* pLegend = pGraph->GetLegendWnd();
	CSaDoc    * pDoc    = pView->GetDocument();
	PitchParm * parms = pDoc->GetPitchParm();      

	// create formants data (cepstral pitch is formant 0 from formants)
	CProcessFormants* pFormants = (CProcessFormants*)pDoc->GetFormants(); // get pointer to spectrogram object
	// check if process is idle
	if (pFormants->IsIdle(this))
		m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_POSCUR, &rWnd);
	else
	{ short int nResult = LOWORD(pFormants->Process(this, pView, rWnd.Width(), TRUE)); // process data
	nResult = CheckResult(nResult, pFormants); // check the process result
	if (nResult == PROCESS_ERROR) return; 
	if (nResult != PROCESS_CANCELED)
	{ 
		if (nResult)
		{ // new data processed, all has to be displayed
			Invalidate();
			return;
		}
		m_HelperWnd.SetMode(MODE_HIDDEN); // hide helper window
		CRect rClip; // get invalid region

		if (pDC->IsPrinting()) 
		{
			rClip = rWnd;
		} 
		else 
		{
			pDC->GetClipBox(&rClip);
			pDC->LPtoDP(&rClip);
		}
		// calculate size factor between actual window size and formants data
		DWORD dwPitchSize = pFormants->GetFormantsSize(this); 
		double fSizeFactor = (double)dwPitchSize / (double)rWnd.Width();
		// get pointer to pitch parameters
		PitchParm* pPitchParm = pDoc->GetPitchParm();
		// set data range
		int nMinData, nMaxData;
		if (pPitchParm->nRangeMode)
		{ // manual range mode
			nMinData = pPitchParm->nLowerBound;
			nMaxData = pPitchParm->nUpperBound;
		}
		else
		{ // auto range mode
			nMinData = pFormants->GetMinValue(this);
			nMaxData = pFormants->GetMaxValue(this);
			nMinData /= PRECISION_MULTIPLIER;
			nMaxData /= PRECISION_MULTIPLIER;
		}     
		BOOL bLog10 = (BOOL)(pPitchParm->nScaleMode == 0); // logarithmic/linear scale
		if (bLog10) // logarithmic display
			pGraph->SetLegendScale(SCALE | NUMBERS | LOG10, nMinData, nMaxData, "f(Hz)"); // set legend scale
		else // linear display
			pGraph->SetLegendScale(SCALE | NUMBERS, nMinData, nMaxData, "f(Hz)"); // set legend scale
		// do common plot paint jobs
		PlotPrePaint(pDC, rWnd, rClip);
		// get pointer to color structure from main frame
		CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
		Colors* pColor = pMainWnd->GetColors();
		CPen* pOldPen;
		double fVScale;
		int nCenterPos;
		double fBase;
		if (bLog10)
		{ // logarithmic scale
			nCenterPos = pLegend->GetFirstGridPosition();
			fBase = (double)pLegend->GetScaleBase() + (double)log10((double)PRECISION_MULTIPLIER);
			fVScale = pLegend->GetGridDistance();
		}
		else // linear scale
		{ // calculate vertical scaling factor
			fVScale = (double)(rWnd.Height() - 3) / (double)(nMaxData - nMinData); // vertical scale
			fVScale *= m_fMagnify;
			// draw bottom line
			CPen penAxes(PS_SOLID, 1, pColor->cPlotAxes); 
			pOldPen = pDC->SelectObject(&penAxes);
			nCenterPos = rWnd.bottom - 1 + (int)((double)nMinData * fVScale); // x-axis vertical position
			pDC->MoveTo(rClip.left, nCenterPos); // draw the line
			pDC->LineTo(rClip.right, nCenterPos);
			pDC->SelectObject(pOldPen);
		}
		// drawing loop
		int nData; // process data
		BOOL bRes = TRUE; // calculation result
		int nOldY = nCenterPos; // old drawing y-position
		// prepare old y-position
		DWORD dwFormant = 0; // index to formants
		if (m_bLineDraw && (rClip.left > 0))
		{ 
			if (rClip.left > 0) dwFormant = (int)((double)(rClip.left - 1) * fSizeFactor); // first formant in this pixel
			// get data for this pixel           
			if (parms->bUseCepMedianFilter) 
			{
				nData = pFormants->GetFormantItemAsInt(this, dwFormant, &bRes, MEDIAN_FILTER);
			} 
			else 
			{
				nData = pFormants->GetProcessedData(this, dwFormant, &bRes);
			}

			// get highest value for this pixel
			DWORD dwLoopEnd = dwFormant + (DWORD)fSizeFactor;
			for (DWORD dwLoop = dwFormant + 1; dwLoop < dwLoopEnd; dwLoop++)
			{ 
				int nTemp;
				if (parms->bUseCepMedianFilter) 
				{
					nTemp = pFormants->GetFormantItemAsInt(this, dwLoop, &bRes, MEDIAN_FILTER);
				} 
				else 
				{
					nTemp = pFormants->GetProcessedData(this, dwLoop, &bRes);
				}

				if (nTemp > nData) nData = nTemp;
			}
			if (nData <= 0) nOldY = -1; // skip this data
			else // valid data
			{ 
				if (bLog10) 
					nOldY = nCenterPos - (int)((log10((double)nData) - fBase) * fVScale); // logarithmic scale
				else 
					nOldY = nCenterPos - (int)((double)nData * fVScale); // linear scale
			}
		}
		CPen penData(PS_SOLID, 1, pColor->cPlotData[0]); 
		pOldPen = pDC->SelectObject(&penData);
		for (int nLoop = rClip.left; nLoop <= rClip.right; nLoop++)
		{ 
			dwFormant = (DWORD)((double)nLoop * fSizeFactor);
			if (dwFormant >= dwPitchSize) break; // end of processed data reached
			// get data for this pixel
			if (parms->bUseCepMedianFilter) 
			{
				nData = pFormants->GetFormantItemAsInt(this, dwFormant, &bRes, MEDIAN_FILTER);
			} 
			else 
			{
				nData = pFormants->GetProcessedData(this, dwFormant, &bRes);
			}

			// get highest value for this pixel
			DWORD dwLoopEnd = dwFormant + (DWORD)fSizeFactor;
			for (DWORD dwInnerLoop = dwFormant + 1; dwInnerLoop < dwLoopEnd; dwInnerLoop++)
			{ 
				int nTemp;
				if (parms->bUseCepMedianFilter) 
				{
					nTemp = pFormants->GetFormantItemAsInt(this, dwInnerLoop, &bRes, MEDIAN_FILTER);
				} 
				else 
				{
					nTemp = pFormants->GetProcessedData(this, dwInnerLoop, &bRes);
				}
				if (nTemp > nData) nData = nTemp;
			}
			if (nData <= 0)
			{ if (nOldY == -1) continue;
			pDC->SetPixel(nLoop - 1, nOldY, RGB(0, 0, 0)); // set the pixel to the left
			nOldY = -1; // skip this data
			continue;
			}
			// draw this pixel
			if (m_bLineDraw) // draw one line from old to new position
			{ 
				if (nOldY == -1) // last data has been skipped, don't draw
				{ 
					if (bLog10) 
						nOldY = nCenterPos - (int)((log10((double)nData) - fBase) * fVScale); // logarithmic scale
					else 
						nOldY = nCenterPos - (int)((double)nData * fVScale); // linear scale
				}
				else
				{ // draw a line between old and new position
					pDC->MoveTo(nLoop - 1, nOldY);
					if (bLog10) 
						nOldY = nCenterPos - (int)((log10((double)nData) - fBase) * fVScale); // logarithmic scale
					else 
						nOldY = nCenterPos - (int)((double)nData * fVScale); // linear scale
					pDC->LineTo(nLoop, nOldY);
				}
			}
			else  // draw one line from center line to value
			{ 
				pDC->MoveTo(nLoop, nCenterPos);
				if (bLog10) 
					pDC->LineTo(nLoop, nCenterPos - (int)((log10((double)nData) - fBase) * fVScale)); // logarithmic scale
				else 
					pDC->LineTo(nLoop, nCenterPos - (int)((double)nData * fVScale)); // linear scale
			}
		}
		pDC->SelectObject(pOldPen);
	}
	}
	// do common plot paint jobs
	PlotPaintFinish(pDC, rWnd, rClip);
}
