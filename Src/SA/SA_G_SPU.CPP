/////////////////////////////////////////////////////////////////////////////
// sa_g_spu.cpp:
// Implementation of the CPlotSpectrum class.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "sa_plot.h"
#include "sa_g_spu.h"
#include "Process\Process.h"
#include "Process\sa_p_smoothedpitch.h"
#include "Process\sa_p_fra.h"
#include "sa_minic.h"
#include "sa_graph.h"

#include <windows.h>
#include <math.h>
#include "sa_doc.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "StopWatch.h"
#include "Process\sa_p_fmt.h"
#include "Process\sa_p_spu.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CPlotSpectrum
// Plot window for displaying spectrum data. This class only handles the
// special drawing of the data. All other work is done in the base class.

IMPLEMENT_DYNCREATE(CPlotSpectrum, CPlotWnd)

/////////////////////////////////////////////////////////////////////////////
// CPlotSpectrum message map

BEGIN_MESSAGE_MAP(CPlotSpectrum, CPlotWnd)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotSpectrum construction/destruction/creation

/***************************************************************************/
// CPlotSpectrum::CPlotSpectrum Constructor
/***************************************************************************/
CPlotSpectrum::CPlotSpectrum() {
    ShowCursors(true,true);
    m_bAnimationPlot = TRUE;
}

/***************************************************************************/
// CPlotSpectrum::~CPlotSpectrum Destructor
/***************************************************************************/
CPlotSpectrum::~CPlotSpectrum() {
}

/////////////////////////////////////////////////////////////////////////////
// CPlotSpectrum message handlers

/***************************************************************************/
// CPlotSpectrum::OnDraw Drawing
// The data to paint is coming from a temporary file, created by the spectrum
// processing class, which is called to do data processing if necessary
// before painting. Call the function PlotPaintFinish at the end of the
// painting to let the plot base class do common jobs like painting the
// cursors.
/***************************************************************************/
void CPlotSpectrum::OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView) {

	// Get pointer to graph, document, and main frame.
    CGraphWnd * pGraph = (CGraphWnd *)GetParent();
    CSaDoc * pModel = pView->GetDocument();
	// calculate sample size in bytes
    WORD wSmpSize = WORD(pModel->GetSampleSize());
    double SigBandwidth = (double)pModel->GetSamplesPerSec() / 2.;
    // get application colors
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    Colors * pColor = pMainWnd->GetColors();
    bool bDynamicUpdate = (pView->GetGraphUpdateMode() == DYNAMIC_UPDATE);

    // Finish fragmenting as needed.
    CProcessFragments * pFragments = (CProcessFragments *)pModel->GetFragments();
	// Force fragmenter to finish
    pModel->EnableBackgroundProcessing(FALSE);
	// process data
    short int nResult = LOWORD(pFragments->Process(this, pModel));
    if (nResult == PROCESS_ERROR || nResult == PROCESS_NO_DATA || nResult == PROCESS_CANCELED) {
        if (!bDynamicUpdate) {
			// check the process result
            nResult = CheckResult(nResult, pFragments);
        }
        return;
    }

    // Get spectrum object and spectrum plot parameters.
	// get pointer to spectrum object
    CProcessSpectrum * pSpectrum = (CProcessSpectrum *)pModel->GetSpectrum();
    CSpectrumParm * pSpectrumParms = pSpectrum->GetSpectrumParms();

    // Get pointer to raw waveform plot.
    int nWaveGraphIndex = pView->GetGraphIndexForIDD(IDD_RAWDATA);
    CGraphWnd * pWaveGraph = pView->GetGraph(nWaveGraphIndex);
    CPlotWnd * pWavePlot = NULL;
    if (pWaveGraph) {
        pWavePlot = pWaveGraph->GetPlot();
    }

    // Set frame parameters.
    DWORD dwFrameStart, dwFrameSize;
    SSpectProcSelect SpectraSelected;
    if (bDynamicUpdate) {
        // display LPC spectrum for current fragment as start cursor is dragged or plot is animating
        if (m_dwAnimationFrame == UNDEFINED_OFFSET) {
            // initialize animation frame for new graph
            DWORD dwStartCursor = pView->GetStartCursorPosition();
            m_dwAnimationFrame = pFragments->GetFragmentIndex(dwStartCursor/wSmpSize);
        }
        SFragParms FragParms = pFragments->GetFragmentParms(m_dwAnimationFrame);
        dwFrameStart = FragParms.dwOffset * wSmpSize;
        dwFrameSize = (DWORD)FragParms.wLength * (DWORD)wSmpSize;
		// turn off to reduce processing time
        SpectraSelected.bCepstralSpectrum = -1;    
        SpectraSelected.bLpcSpectrum = -1;
    } else {
        // show raw, cepstrally-smoothed, and LPC spectra for all sample data between start and stop cursors (resting state)
        dwFrameStart = pView->GetStartCursorPosition();
        dwFrameSize = pView->GetStopCursorPosition() - dwFrameStart + wSmpSize;
		// display raw and cepstrally-smoothed power spectrum
        SpectraSelected.bCepstralSpectrum = -1;
		// also LPC spectrum
        SpectraSelected.bLpcSpectrum = -1;
    }

    // Construct eraser.
    CBrush Eraser(pColor->cPlotBkg);

    // If frame is too long, exit.
    if (dwFrameSize > (DWORD)(MAX_FFT_LENGTH * wSmpSize)) {
		// clear the plot area
        pDC->FillRect(&rClip, &Eraser);
        m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_CURCLOSER, &rWnd);
        if ((bDynamicUpdate) && (pWavePlot!=NULL)) {
            // turn off highlighted area in raw data
            pWavePlot->ClearHighLightArea();
            pWavePlot->UpdateWindow();
        }
        if (pSpectrum->IsDataReady()) {
            pSpectrum->SetDataInvalid();
        }
        return;
    }

    // Process data.
    nResult = LOWORD(pSpectrum->Process(this, pModel, dwFrameStart, dwFrameSize, SpectraSelected)); // process data
    nResult = CheckResult(nResult, pSpectrum);
    if (nResult == PROCESS_ERROR) {
        return;
    }


    // Construct pens.
    CPen penRaw(PS_SOLID, 1, pColor->cPlotData[1]);
    CPen * pOldPen = pDC->SelectObject(&penRaw);
    CPen penSmoothBold(PS_SOLID, 1, pColor->cPlotData[0]);
    CPen penLpc(PS_SOLID, 1, pColor->cPlotData[2]);
    //CPen penLpcBold(PS_SOLID, 1, pColor->cPlotData[0]);

    if (nResult != PROCESS_CANCELED) {
        m_HelperWnd.SetMode(MODE_HIDDEN);
        // Annotate graph axes.
        //pGraph->SetXScale(SCALE | NUMBERS, pSpectrumParms->nFreqLowerBound, pSpectrumParms->nFreqUpperBound, "f(Hz)");
        double ScaleFactor = 1. + pSpectrumParms->nFreqScaleRange;
        int nFreqLowerBound = 0;
        int nFreqUpperBound = (int)(ceil(SigBandwidth / ScaleFactor));
        pGraph->SetXScale(SCALE | NUMBERS, nFreqLowerBound, nFreqUpperBound, _T("f(Hz)"));
        if (pSpectrumParms->nScaleMode == LINEAR) {
            pGraph->SetLegendScale(SCALE | NUMBERS, pSpectrumParms->nPwrLowerBound, pSpectrumParms->nPwrUpperBound, _T("Power"));
        } else {
            pGraph->SetLegendScale(SCALE | NUMBERS, pSpectrumParms->nPwrLowerBound, pSpectrumParms->nPwrUpperBound, _T("Power (dB)"));
        }

        // Set start and end of plot.
        int nFirstX = rClip.left;
        if (nFirstX > 0) {
			// one pixel before to restore line entering left edge of clip region
            nFirstX--;
        }
        int nLastX = rClip.right;
        if (nLastX < rWnd.right) {
			// one pixel after to restore line exiting right edge of clip region
            nLastX++;
        }

        // Clear the plot area and set up grid.
        pDC->FillRect(&rClip, &Eraser);
        PlotPrePaint(pDC, rWnd, rClip, NULL, true, true);

        // Calculate plot scale factors.
        int PlotYRange = rWnd.Height();
		// max spectral intensity to plot
        float PlotYScaleMax = (float)pSpectrumParms->nPwrUpperBound;
		// min spectral intensity to plot
        float PlotYScaleMin = (float)pSpectrumParms->nPwrLowerBound;
		// pixels per unit power
        float PlotYScaleFac = (float)PlotYRange/(PlotYScaleMax - PlotYScaleMin);
        float PlotXScaleMax = (float)nFreqUpperBound;
        float PlotXScaleMin = (float)nFreqLowerBound;
        float PlotXScale = (PlotXScaleMax - PlotXScaleMin) / (float)rWnd.Width();

        // Generate spectral plot.  Since the number of spectral values calculated by the process class is
        // fixed and larger than the width of the client area, the power must be averaged for each pixel
        // along the horizontal (frequency) axis.  The pixels are each assumed to be centered in their respective
        // spectral bands.  Powers are stored in the process buffer on a linear scale so that they may be accumulated
        // for each band.  Once averaged, they may then be converted to a decibel (dB) scale.
        short nSpectBands = (short)pSpectrum->GetSpectralCount();
        double SpectFreqScale = SigBandwidth / ((double)nSpectBands - 1.);

        short i;
        double BandPwr;
        short LBandEdge, UBandEdge;   // lower and upper band edges in Hz

        double MaxRawBandPwr = pSpectrum->GetSpectralPowerRange().Max.Raw;
		// undefined if plot type not requested
        if (pSpectrum->GetSpectralData(0).Raw != (float)UNDEFINED_DATA) { 
            pDC->SelectObject(&penRaw);
            for (int x = nFirstX; x < nLastX; x++) {
                // plot raw spectrum
                LBandEdge = (short)max(0,(short)Round((double)(x-0.5) * PlotXScale / SpectFreqScale));
                UBandEdge = (short)min(nSpectBands-1,(short)Round((double)(x+0.5) * PlotXScale / SpectFreqScale));
                for (i = LBandEdge, BandPwr = 0.; i < UBandEdge; i++) {
                    BandPwr += (double)pSpectrum->GetSpectralData(i).Raw;
                }
                if (UBandEdge != LBandEdge) {
					// average the power within the band
                    BandPwr /= (double)(UBandEdge - LBandEdge);
                }
                if (pSpectrumParms->nScaleMode == DB) {
                    BandPwr = (BandPwr == 0.)?MIN_LOG_PWR:10.*log10((double)BandPwr) - pSpectrum->GetSpectralPowerRange().fdBRef;
                }
				int y;
                if (BandPwr > PlotYScaleMax) {
                    y = PlotYRange;
                } else if (BandPwr < PlotYScaleMin) {
                    y = 0;
                } else {
                    y = (int)Round((BandPwr - PlotYScaleMin)*PlotYScaleFac);
                }
                y = rWnd.bottom - y;
                if (x == nFirstX) {
                    pDC->MoveTo(nFirstX, y);
                } else if (UBandEdge != LBandEdge) {
                    pDC->LineTo(x, y);
                }
            }

            if (pSpectrumParms->nScaleMode == DB) {
                MaxRawBandPwr = (MaxRawBandPwr == 0.)?MIN_LOG_PWR:10.*log10((double)MaxRawBandPwr) - pSpectrum->GetSpectralPowerRange().fdBRef;
            }

            if (pSpectrumParms->bShowCepSpectrum) {
                pDC->SelectObject(&penSmoothBold);
                for (int x = nFirstX; x < nLastX; x++) {
                    // plot smooth spectrum
                    LBandEdge = (short)max(0,(short)Round((double)(x-0.5) * PlotXScale / SpectFreqScale));
                    UBandEdge = (short)min(nSpectBands-1,(short)Round((double)(x+0.5) * PlotXScale / SpectFreqScale));
                    for (i = LBandEdge, BandPwr = 0.; i < UBandEdge; i++) {
                        BandPwr += (double)pSpectrum->GetSpectralData(i).Smooth;
                    }
                    if (UBandEdge != LBandEdge) {
						// average the power within the band
                        BandPwr /= (double)(UBandEdge - LBandEdge);
                    }
                    if (pSpectrumParms->nScaleMode == DB) {
                        BandPwr = (BandPwr == 0.)?MIN_LOG_PWR:10.*log10((double)BandPwr) - pSpectrum->GetSpectralPowerRange().fdBRef;
                    }
					int y;
					if (BandPwr > PlotYScaleMax) {
                        y = PlotYRange;
                    } else if (BandPwr < PlotYScaleMin) {
                        y = 0;
                    } else {
                        y = (int)Round((BandPwr - PlotYScaleMin)*PlotYScaleFac);
                    }
                    y = rWnd.bottom - y;
                    if (x == nFirstX) {
                        pDC->MoveTo(nFirstX, y);
                    } else if (UBandEdge != LBandEdge) {
                        pDC->LineTo(x, y);
                    }
                }
            }
        }

        if (((bDynamicUpdate) || (pSpectrumParms->bShowLpcSpectrum)) &&
                (pSpectrum->GetSpectralData(0).Lpc != (float)UNDEFINED_DATA)) {
            if (bDynamicUpdate) {
                pDC->SelectObject(&penLpc);
            } else {
                pDC->SelectObject(&penLpc);
            }
            // adjust the LPC plot to ride on top of the raw spectrum
            double LpcRef;
            double MaxLpcBandPwr = pSpectrum->GetSpectralPowerRange().Max.Lpc;
            if (pSpectrumParms->nScaleMode == DB) {
                MaxLpcBandPwr = (MaxLpcBandPwr == 0.)?MIN_LOG_PWR:10.*log10((double)MaxLpcBandPwr);
                if (MaxRawBandPwr != (float)UNDEFINED_DATA) {
                    LpcRef = MaxLpcBandPwr - MaxRawBandPwr;
                } else {
                    LpcRef = pSpectrum->GetSpectralPowerRange().fdBRef;
                }
            } else {
                if (MaxRawBandPwr != (float)UNDEFINED_DATA && MaxRawBandPwr != 0.F) {
                    LpcRef = MaxLpcBandPwr / MaxRawBandPwr;
                } else {
                    LpcRef = pow(10.F, pSpectrum->GetSpectralPowerRange().fdBRef / 10.F);
                }
            }

            for (int x = nFirstX; x < nLastX; x++) {
                // plot LPC spectrum
                LBandEdge = (short)max(0,(short)Round((double)(x-0.5) * PlotXScale / SpectFreqScale));
                UBandEdge = (short)min(nSpectBands-1,(short)Round((double)(x+0.5) * PlotXScale / SpectFreqScale));
                for (i = LBandEdge, BandPwr = 0; i < UBandEdge; i++) {
                    BandPwr += pSpectrum->GetSpectralData(i).Lpc;
                }
                if (UBandEdge != LBandEdge) {
                    BandPwr /= (double)(UBandEdge - LBandEdge);    // average the power within the band
                }
                if (pSpectrumParms->nScaleMode == DB) {
                    BandPwr = (BandPwr == 0.)?MIN_LOG_PWR:10.*log10((double)BandPwr) - LpcRef;
                } else {
                    BandPwr /= LpcRef;
                }
				int y;
				if (BandPwr > PlotYScaleMax) {
                    y = PlotYRange;
                } else if (BandPwr < PlotYScaleMin) {
                    y = 0;
                } else {
                    y = (int)Round((BandPwr - PlotYScaleMin)*PlotYScaleFac);
                }
                y = rWnd.bottom - y;
                if (x == nFirstX) {
                    pDC->MoveTo(nFirstX, y);
                } else if (UBandEdge != LBandEdge) {
                    pDC->LineTo(x, y);
                }
            }

            // Draw boxes around formants.
            if (pSpectrumParms->bShowFormantFreq || pSpectrumParms->bShowFormantPower) {
                for (unsigned short nFormant = 1; nFormant <= MAX_NUM_FORMANTS; nFormant++) {
                    SFormant Formant = pSpectrum->GetFormant(nFormant);

                    if (Formant.Lpc.FrequencyInHertz == (float)UNDEFINED_DATA ||
                            Formant.Lpc.BandwidthInHertz == (float)UNDEFINED_DATA ||
                            Formant.Lpc.PowerInDecibels == FLT_MAX_NEG) {
                        continue;
                    } else if (Formant.Lpc.FrequencyInHertz > nFreqUpperBound) {
                        break;
                    }
                    int y;
                    int x = (int)(Formant.Lpc.FrequencyInHertz / PlotXScale  + 0.5);
                    BandPwr = Formant.Lpc.PowerInDecibels;
                    if (pSpectrumParms->nScaleMode == DB) {
                        BandPwr = ((BandPwr == 0.)?MIN_LOG_PWR:BandPwr) - LpcRef;
                    } else {
                        BandPwr = (BandPwr == 0.)?MIN_LOG_PWR:BandPwr;
                        BandPwr = pow(10.,BandPwr/20.);
                    }
                    if (BandPwr > PlotYScaleMax) {
                        y = PlotYRange;
                    } else if (BandPwr < PlotYScaleMin) {
                        y = 0;
                    } else {
                        y = (int)Round((BandPwr - PlotYScaleMin)*PlotYScaleFac);
                    }
                    y = rWnd.bottom - y;

                    int rx1 = x - 2;
                    int rx2 = x + 2;
                    int ry1 = y - 2;
                    int ry2 = y + 2;
                    pDC->Rectangle(rx1,ry1,rx2,ry2);
                }

                // Load a font for tagging formants.
                CFont Font;
                int nHeight = min(32, max(14, (int)(min(.06 * rWnd.Height(),(0.5/11/10) * rWnd.Width()) + 0.5)));
                LPCTSTR pszName = _T("MS Sans Serif");
                Font.CreateFont(nHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                                CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, (VARIABLE_PITCH | FF_ROMAN), pszName);  // create font

                CFont * pOldFont = pDC->SelectObject(&Font); // select reference font for tags

                // Write formant info.
                for (unsigned short nFormant = 1; nFormant <= MAX_NUM_FORMANTS; nFormant++) {
                    SFormant Formant = pSpectrum->GetFormant(nFormant);

                    if (Formant.Lpc.FrequencyInHertz == (float)UNDEFINED_DATA ||
                            Formant.Lpc.BandwidthInHertz == (float)UNDEFINED_DATA ||
                            Formant.Lpc.PowerInDecibels == FLT_MAX_NEG) {
                        continue;
                    } else if (Formant.Lpc.FrequencyInHertz > nFreqUpperBound) {
                        break;
                    }

                    int y;
                    int x = (int)(Formant.Lpc.FrequencyInHertz / PlotXScale  + 0.5);
                    BandPwr = Formant.Lpc.PowerInDecibels;
                    if (pSpectrumParms->nScaleMode == DB) {
                        BandPwr = ((BandPwr == 0.)?MIN_LOG_PWR:BandPwr) - LpcRef;
                    } else {
                        BandPwr = (BandPwr == 0.)?MIN_LOG_PWR:BandPwr;
                        BandPwr = pow(10.,BandPwr/20.);
                    }
                    if (BandPwr > PlotYScaleMax) {
                        y = PlotYRange;
                    } else if (BandPwr < PlotYScaleMin) {
                        y = 0;
                    } else {
                        y = (int)Round((BandPwr - PlotYScaleMin)*PlotYScaleFac);
                    }
                    y = rWnd.bottom - y;

                    int tx = x + 10;
                    int ty = y - 10;
                    TCHAR szTag[64];
                    szTag[0] = 0;
                    if (pSpectrumParms->bShowFormantFreq) {
                        int nTagLen = swprintf_s(szTag, _T("%.0f"), (double)Formant.Lpc.FrequencyInHertz);
                        if (pSpectrumParms->bShowFormantBandwidth) {
                            swprintf_s(&szTag[nTagLen], _countof(szTag)-nTagLen, _T("\xB1%.0f"), (double)Formant.Lpc.BandwidthInHertz/2.);
                        }
                        wcscat_s(szTag, _countof(szTag), _T("Hz"));
                    }
                    if (pSpectrumParms->bShowFormantPower) {
                        swprintf_s((LPTSTR)szTag+_tcslen(szTag), _countof(szTag)-_tcslen(szTag), _T(" %.1fdB"), (double)Formant.Lpc.PowerInDecibels-LpcRef);
                    }

                    CSize sizeTag = pDC->GetTextExtent(szTag, _tcslen(szTag));
                    CRect rTag(tx, ty, tx+sizeTag.cx, ty+sizeTag.cy);
                    CRect rOldTag;
                    if (nFormant != 1) {
                        CRect rOverlap;
                        if (rOverlap.IntersectRect(rTag, rOldTag)) {
                            // move tag to avoid overlapping with previous tag
                            int yShift;
                            if (rTag.bottom - rOldTag.top < rOldTag.bottom - rTag.top) {
                                yShift = rOldTag.top - rTag.bottom;
                            } else {
                                yShift = rOldTag.bottom - rTag.top;
                            }
                            rTag.top += yShift;
                            rTag.bottom += yShift;
                        }
                    }
                    pDC->DrawText(szTag, -1, rTag, DT_SINGLELINE | DT_TOP | DT_LEFT | DT_NOCLIP);
                    rOldTag = rTag;
                }
                pDC->SelectObject(pOldFont); // restore old font
            }
        }
    }
    // do common plot paint jobs
    PlotPaintFinish(pDC, rWnd, rClip);

    // restore pen
    pDC->SelectObject(pOldPen);
}

/***************************************************************************/
// CPlotSpectrum::AnimateFrame  Animate a single frame
/***************************************************************************/
void CPlotSpectrum::AnimateFrame(DWORD dwFrameIndex) {
    CPlotWnd::StandardAnimateFrame(dwFrameIndex);
}

/***************************************************************************/
// CPlotSpectrum::EndAnimation  Terminate animation
/***************************************************************************/
void CPlotSpectrum::EndAnimation(void) {
    CPlotWnd::StandardEndAnimation();
}
