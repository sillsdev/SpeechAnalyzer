/////////////////////////////////////////////////////////////////////////////
// sa_g_spg.cpp:
// Implementation of the CPlotSpectrogram class.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//
//   1.5Test10.3
//      SDM changed color mapping
//   1.5Test10.7
//      SDM rebuild palette on DeviceCaps() change
//   1.5Test10.9B
//      RLJ Comment out some unused code.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_plot.h"
#include "sa_g_spg.h"
#include "sa_minic.h"
#include "sa_graph.h"
#include "sa_doc.h"
#include "sa.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "sa_cdib.h"
#include "SpectroParm.h"
#include "Process\Process.h"
#include "Process\sa_p_fra.h"
#include "Process\sa_p_spg.h"
#include "Process\sa_p_sfmt.h"
#include "Process\sa_p_gra.h"
#include "Process\FormantTracker.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
using namespace std;

//###########################################################################
// CPlotSpectrogram
// Plot window for displaying spectrogram data. This class handles the
// special drawing of the data. It also has to store the graphs range index
// (delivered and needed by the process class).

IMPLEMENT_DYNCREATE(CPlotSpectrogram, CPlotWnd)

/////////////////////////////////////////////////////////////////////////////
// CPlotSpectrogram static member definition

BOOL CPlotSpectrogram::bPaletteInit = FALSE;  // palette not initialized yet
int CPlotSpectrogram::nPaletteMode = SYSTEMCOLOR; // use system colors only
CPalette CPlotSpectrogram::SpectroPalette; // color palette

BEGIN_MESSAGE_MAP(CPlotSpectrogram, CPlotWnd)
	ON_WM_CREATE()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

CPlotSpectrogram::CPlotSpectrogram() {
}

CPlotSpectrogram::~CPlotSpectrogram() {
}

void  CPlotSpectrogram::CopyTo(CPlotWnd * pT) {
	CPlotSpectrogram * pTarg = (CPlotSpectrogram *)pT;

	CPlotWnd::CopyTo(pT);

	pTarg->SpectroAB = SpectroAB;

	// Take possession of process data
	pTarg->m_pAreaProcess = m_pAreaProcess;
	m_pAreaProcess = NULL;
}

CPlotWnd * CPlotSpectrogram::NewCopy(void) {
	CPlotWnd * pRet = new CPlotSpectrogram();

	CopyTo(pRet);

	return pRet;
}

// SDM 1.5Test10.3
#define sparsePaletteSize 30l

static struct RGB sparsePalette[sparsePaletteSize + 1] = {
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

/***************************************************************************/
// CPlotSpectrogram::SetSpectroAB Access method for SpectroAB attribute.
/***************************************************************************/
void CPlotSpectrogram::SetSpectroAB(char MySpectroAB) {
	SpectroAB = MySpectroAB;
}

/***************************************************************************/
// CPlotSpectrogram::GetSpectroAB Access method for SpectroAB attribute.
/***************************************************************************/
char CPlotSpectrogram::GetSpectroAB() const {
	return SpectroAB;
}

/***************************************************************************/
// CPlotSpectrogram::CreateSpectroPalette Creates the palette for spectrogram
// This function creates the colors needed by the spectrogram. If the system
// supports only 16 colors, it won't do anything and the spectrogram will
// work with this 16 colors. If it supports 256 colors, this function creates
// a 117 grayscale and above a 117 color palette (total 236 colors available
// in system palette). If there are more than 256 colors supported, this
// function creates the full 233 grayscale and above the 233 color palette.
// The function returns FALSE in case on error, else TRUE.
/***************************************************************************/
BOOL CPlotSpectrogram::CreateSpectroPalette(CDC * pDC, CDocument * /*pSaDoc*/) {
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
	}
	else {
		nRasterCaps = FALSE;
	}
	int nNumColors;
	if (nRasterCaps) {
		nNumColors = pDC->GetDeviceCaps(SIZEPALETTE);
	}
	else {
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
		}
		else {
			if (!SpectroPalette.ResizePalette(2 * nPaletteSize)) {
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
			lRed = (sparsePalette[lowSparseIndex + 1].r * remainderSparseIndex
				+ sparsePalette[lowSparseIndex].r * (nPaletteSize - remainderSparseIndex)) / nPaletteSize;

			lGreen = (sparsePalette[lowSparseIndex + 1].g * remainderSparseIndex
				+ sparsePalette[lowSparseIndex].g * (nPaletteSize - remainderSparseIndex)) / nPaletteSize;

			lBlue = (sparsePalette[lowSparseIndex + 1].b * remainderSparseIndex
				+ sparsePalette[lowSparseIndex].b * (nPaletteSize - remainderSparseIndex)) / nPaletteSize;

			*(unsigned long *)&lpLogPalette->palPalEntry[wColorIndex + nPaletteSize] = SET_RGB(lRed, lGreen, lBlue);
		}
		SpectroPalette.SetPaletteEntries(0, 2 * nPaletteSize, lpLogPalette->palPalEntry);
		delete[] lpLogPalette;
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

/***************************************************************************/
// CPlotSpectrogram::OnCreate Window creation
// The plot has to get the area process index, because it is a area process
// plot.
/***************************************************************************/
int CPlotSpectrogram::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CPlotWnd::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}
	// get pointer to document
	CView * pView = (CView *)GetParent()->GetParent();
	CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();

	// save process
	GetSpectrogram(pDoc);

	return 0;
}

void CPlotSpectrogram::populateBmiColors(RGBQUAD * QuadColors, CSaView * pView) {

	CSaDoc * pDoc = pView->GetDocument();
	CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
	// prepare to draw
	COLORREF Color[256];
	// get pointer to plot colors
	Colors * pColors = pMain->GetColors();
	// create palette
	Color[235] = pColors->cPlotBkg;  // background
	Color[236] = pColors->cPlotData[0];;  // formant tracks
	Color[237] = pColors->cPlotData[1];;  // formant tracks
	Color[238] = pColors->cPlotData[2];;  // formant tracks
	Color[239] = pColors->cPlotData[3];;  // formant tracks
	Color[240] = pColors->cPlotData[4];;  // formant tracks
	Color[241] = pColors->cPlotData[5];;  // formant tracks

	const CSpectroParm * pSpectroParm = &GetSpectrogram(pDoc)->GetSpectroParm();
	if (pSpectroParm->nColor) {
		// create grayscale palette
		Color[0] = RGB(255, 255, 255);   // white
		Color[234] = RGB(0, 0, 0);     // black
		unsigned int nPaletteSize = 234;
		// use closest system gray
		for (WORD wColorIndex = 1; wColorIndex < 234; wColorIndex++) {
			if (wColorIndex <= (WORD)pSpectroParm->nMinThreshold) {
				Color[wColorIndex] = RGB(255, 255, 255);
			}
			else if (wColorIndex >= (WORD)pSpectroParm->nMaxThreshold) {
				Color[wColorIndex] = RGB(0, 0, 0);
			}
			else {
				long intermediate = (wColorIndex - pSpectroParm->nMinThreshold) * (sizeof(sparseBlack) - 1) * 234
					/ (pSpectroParm->nMaxThreshold - pSpectroParm->nMinThreshold);
				long lowSparseIndex = intermediate / nPaletteSize;
				long remainderSparseIndex = intermediate % nPaletteSize;

				// interpolate colors
				long lGray = sparseBlack[lowSparseIndex + 1] * remainderSparseIndex / nPaletteSize
					+ sparseBlack[lowSparseIndex] * (nPaletteSize - remainderSparseIndex) / nPaletteSize;

				Color[wColorIndex] = RGB(lGray, lGray, lGray);
			}
		}
	}
	else {
		// create color map
		// SDM 1.5Test10.3
		Color[0] = RGB(0, 0, 0);       // black
		Color[234] = RGB(255, 0, 255);     // magenta
		long lRed, lGreen, lBlue;
		unsigned int nPaletteSize = 234;

		// use closest system color
		for (WORD wColorIndex = 0; wColorIndex < nPaletteSize; wColorIndex++) {
			if (wColorIndex <= (WORD)pSpectroParm->nMinThreshold) {
				Color[wColorIndex] = RGB(0, 0, 0);
			}
			else if (wColorIndex >= (WORD)pSpectroParm->nMaxThreshold) {
				Color[wColorIndex] = RGB(255, 0, 255);
			}
			else {
				long intermediate = (wColorIndex - pSpectroParm->nMinThreshold) * sparsePaletteSize * 234
					/ (pSpectroParm->nMaxThreshold - pSpectroParm->nMinThreshold);
				long lowSparseIndex = intermediate / nPaletteSize;
				long remainderSparseIndex = intermediate % nPaletteSize;


				// interpolate colors
				lRed = sparsePalette[lowSparseIndex + 1].r * remainderSparseIndex / nPaletteSize
					+ sparsePalette[lowSparseIndex].r * (nPaletteSize - remainderSparseIndex) / nPaletteSize;

				lGreen = sparsePalette[lowSparseIndex + 1].g * remainderSparseIndex / nPaletteSize
					+ sparsePalette[lowSparseIndex].g * (nPaletteSize - remainderSparseIndex) / nPaletteSize;

				lBlue = sparsePalette[lowSparseIndex + 1].b * remainderSparseIndex / nPaletteSize
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
// CPlotSpectrogram::OnDraw Drawing
// The data to draw is coming from a temporary file, created by the
// spectrogram processing class, which is called to do data processing if
// necessary before drawing. Call the function PlotPaintFinish at the end of
// the drawing to let the plot base class do common jobs like drawing the
// cursors.
/***************************************************************************/
void CPlotSpectrogram::OnDrawSpectrogram(std::unique_ptr<CDC>& memDC, CRect rWnd, CRect rClip, CSaView * pView, BOOL bSmooth, BOOL * bAliased) {

	// get pointer to graph, view, document, application and mainframe
	CSaDoc * pDoc = pView->GetDocument();
	DWORD nSmpSize = pDoc->GetSampleSize();
	BOOL bEnhanceFormants = ResearchSettings.m_bSpectrogramContrastEnhance;

	// get pointer to process class
	CProcessSpectrogram * pSpectrogram = GetSpectrogram(pDoc); // get pointer to spectrogram object

	// get pointer to spectrogram parameters
	const CSpectroParm * pSpectroParm = &GetSpectrogram(pDoc)->GetSpectroParm();

	BOOL bOverlay = (pSpectroParm->nOverlay == 0) || (!pSpectroParm->bShowFormants);
	if (!bOverlay) {
		CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
		memDC->FillSolidRect(rWnd, pMain->GetColors()->cPlotBkg);
		return;
	}

	BITMAP * pBitmap = new BITMAP;
	HBITMAP hBitmap = (HBITMAP)GetCurrentObject(memDC->GetSafeHdc(), OBJ_BITMAP);

	GetObject(hBitmap, sizeof(BITMAP), pBitmap);

	// get spectrogram parameters
	int nSpectraCnt = pSpectrogram->GetWindowWidth();
	DWORD dwSpgmStart = pSpectrogram->GetAreaPosition();
	DWORD dwSpgmLength = pSpectrogram->GetAreaLength();
	// This is the distance between these points therefore we need to subtract one.
	// We only have 0.5 a point at DC and nyquist
	double dSpectrumPoints = pSpectrogram->GetWindowHeight() - 1;
	double dVisibleSpectrumPoints = dSpectrumPoints * pSpectroParm->nFrequency * 2. / pDoc->GetSamplesPerSec();
	double fSpectrumPointsPerPix = dVisibleSpectrumPoints / (double)rWnd.Height();

	// get sample data frame parameters
	double fDataStart = pView->GetDataPosition(rWnd.Width()); // byte offset of first sample to displayed in raw waveform plot
	DWORD dwDataLength = pView->GetDataFrame(); // number of bytes displayed in raw waveform plot

	if (!IsRealTime()) {
		// Snapshot(Spectrogram B) special case
		fDataStart = dwSpgmStart;
		dwDataLength = dwSpgmLength;
	}

	// calculate scale factors and starting positions
	double fSamplesPerPix = double(dwDataLength / nSmpSize) / (double)(rWnd.Width() + 1);
	double fSample = (double)(fDataStart / nSmpSize);
	double fSpectraPerSample = (double)nSpectraCnt / (double)(dwSpgmLength / nSmpSize);
	double fSpectraPerPix = fSpectraPerSample * fSamplesPerPix;
	double fSpectrum = (fSample - double(dwSpgmStart / nSmpSize)) * fSpectraPerSample;

	if (bAliased && fSpectraPerPix > 1.0) {
		*bAliased = TRUE;
	}

	fSpectrum += rClip.left*fSpectraPerPix;

	// offset for initial pass to preload left
	DWORD dwSpectrum = (bSmooth)?(DWORD)floor(fSpectrum): (DWORD)(fSpectrum + 0.5);

	DWORD dwPrevSpectrum = (DWORD)UNDEFINED_OFFSET;

	int nSpectrumIndex = int(dSpectrumPoints);
	double * dlog = NULL;
	if (bEnhanceFormants) {
		dlog = new double[nSpectrumIndex + 1];
		for (; nSpectrumIndex > 0; nSpectrumIndex--) {
			dlog[nSpectrumIndex] = log(double(nSpectrumIndex));
		}
		dlog[0] = dlog[1];
	}

	// draw the spectrogram: right now there is no intelligent stretch algorithm
	// built in. If the graph is larger than it was when the spectrogram was generated,
	// then the missing pixels will just be copied from existing data
	// points. If it is smaller, data points will be skipped.
	if (bSmooth) {
		short * pPowerLeft = new short[rWnd.bottom];
		short * pPowerRight = new short[rWnd.bottom];
		short * pIndex = new short[rWnd.bottom];
		unsigned char * pBottom = new unsigned char[rWnd.bottom];

		double fSpectrumIndex = dSpectrumPoints - dVisibleSpectrumPoints;

		// Vertical Linear Interpolation
		for (int y = 0; y < rWnd.bottom; y++) {
			short nSpectrumIndex2 = (short)floor(fSpectrumIndex);
			double dBottom = fSpectrumIndex - nSpectrumIndex2;
			pBottom[y] = (unsigned char)round2Int(dBottom * 128);
			pIndex[y] = nSpectrumIndex2;

			fSpectrumIndex += fSpectrumPointsPerPix;
		}

		int nLeftRightStatus = 0;  // will be 0x3 when we have two valid samples
		for (int x = rClip.left; x <= rClip.right; x++) {
			if (dwSpectrum != dwPrevSpectrum) {

				// swap data sets
				short * pSwap = pPowerLeft;
				pPowerLeft = pPowerRight;
				pPowerRight = pSwap;

				dwSpectrum = dwSpectrum < dwSpgmLength ? dwSpectrum : dwSpgmLength - 1;

				// Get new data to form right edge
				uint8 * pPower = (uint8 *)pSpectrogram->GetSpectroSlice((DWORD)dwSpectrum); // return spectrogram slice data

				BOOL bWhite = (pPower[0] == BLANK);

				nLeftRightStatus = (nLeftRightStatus * 2) & 0x3;

				if (!bWhite) {
					if (bEnhanceFormants) {
						double dOffset = 0;
						double localSlope = 0;
						int n = 0;
						double sumX = 0;
						double sumY = 0;
						double sumXX = 0;
						double sumYY = 0;
						double sumXY = 0;
						for (int nSpectrumIndex3 = int(dSpectrumPoints); nSpectrumIndex3 > 0; nSpectrumIndex3--) {
							double Y = pPower[nSpectrumIndex3];
							double X = dlog[nSpectrumIndex3];

							sumX += X;
							sumXX += X*X;
							sumY += Y;
							sumXY += X*Y;
							sumYY += Y*Y;

							n++;
						}
						if (sumX) {
							localSlope = (n*sumXY - sumX*sumY) / (n*sumXX - sumX*sumX);
							dOffset = sumY / n - localSlope*sumX / n;
						}

						double dMaxPower = 0;
						double dMinPower = 1000;
						for (int nSpectrumIndex4 = int(dSpectrumPoints); nSpectrumIndex4 >= 0; nSpectrumIndex4--) {
							dMaxPower = max(dMaxPower, pPower[nSpectrumIndex4] - (dlog[nSpectrumIndex4] * localSlope + dOffset));
							dMinPower = min(dMinPower, pPower[nSpectrumIndex4] - (dlog[nSpectrumIndex4] * localSlope + dOffset));
						}

						if (dMaxPower < dMinPower + 12) {
							dMaxPower = dMinPower + 12;
						}

						// Vertical Linear Interpolation
						for (int y = 0; y < rWnd.bottom; y++) {
							int nSpectrumIndex5 = pIndex[y];
							int nBottom = pBottom[y];
							int nTop = 128 - nBottom;

							short value = short((pPower[nSpectrumIndex5] * nTop + pPower[nSpectrumIndex5 + 1] * nBottom - (dlog[nSpectrumIndex5] * localSlope + dOffset + dMinPower) * 128)*220. / (dMaxPower - dMinPower));
							pPowerRight[y] = value;
						}
					}
					else {

						// Vertical Linear Interpolation
						for (int y = 0; y < rWnd.bottom; y++) {
							int nSpectrumIndex6 = pIndex[y];
							int nBottom = pBottom[y];
							int nTop = 128 - nBottom;
							short value = short(pPower[nSpectrumIndex6] * nTop + pPower[nSpectrumIndex6 + 1] * nBottom);
							pPowerRight[y] = value;
						}
					}
					nLeftRightStatus += 1;
				}

				if (dwPrevSpectrum == (DWORD)UNDEFINED_OFFSET) {
					// initial pass we need to preload left (just finished)
					// then load right
					dwPrevSpectrum = dwSpectrum;
					dwSpectrum = (DWORD)ceil(fSpectrum);
					x--;
					continue;
				}
			}

			uint8 * pPixel = ((uint8 *)pBitmap->bmBits) + x;

			if (nLeftRightStatus == 0x3) {
				// Horizontal linear interpolation
				double dLeft = dwSpectrum - fSpectrum;
				int nLeft = int(dLeft * 128 + 0.5);
				int nRight = 128 - nLeft;
				for (int y = 0; y < rWnd.bottom; y++) {
					*pPixel = (uint8)((pPowerLeft[y] * nLeft + pPowerRight[y] * nRight + 8192) >> 14);
					pPixel += pBitmap->bmWidthBytes;
				}
			}
			else {
				// Draw White (Don't interpolate beyond first/last valid spectrum pairs)
				for (int y = 0; y < rWnd.bottom; y++) {
					*pPixel = (uint8)BLANK;
					pPixel += pBitmap->bmWidthBytes;
				}
			}

			fSpectrum += fSpectraPerPix;
			dwPrevSpectrum = dwSpectrum;

			dwSpectrum = (DWORD)ceil(fSpectrum);
		}
		delete[] pPowerRight;
		pPowerRight = NULL;
		delete[] pPowerLeft;
		pPowerLeft = NULL;
		delete[] pIndex;
		delete[] pBottom;
	}
	else {
		uint8 * pPowerRight = new uint8[rWnd.bottom];
		short * pIndex = new short[rWnd.bottom];

		double fSpectrumIndex = dSpectrumPoints - dVisibleSpectrumPoints;

		// Vertical nearest neighbor
		for (int y = 0; y < rWnd.bottom; y++) {
			pIndex[y] = (short)(fSpectrumIndex + 0.5);
			fSpectrumIndex += fSpectrumPointsPerPix;
		}

		for (int x = rClip.left; x <= rClip.right; x++) {
			if (dwSpectrum != dwPrevSpectrum) {
				dwSpectrum = dwSpectrum < dwSpgmLength ? dwSpectrum : dwSpgmLength - 1;
				// Get new data to form right edge
				uint8 * pPower = (uint8 *)pSpectrogram->GetSpectroSlice((DWORD)dwSpectrum); // return spectrogram slice data
				// Vertical nearest neighbor
				for (int y = 0; y < rWnd.bottom; y++) {
					uint8 value = pPower[pIndex[y]];
					pPowerRight[y] = value;
				}
			}

			uint8 * pPixel = ((uint8 *)pBitmap->bmBits) + x;

			// Horizontal nearest neighbor
			for (int y = 0; y < rWnd.bottom; y++) {
				*pPixel = pPowerRight[y];
				pPixel += pBitmap->bmWidthBytes;
			}

			fSpectrum += fSpectraPerPix;
			dwPrevSpectrum = dwSpectrum;

			dwSpectrum = (DWORD)(fSpectrum + 0.5);
		}
		delete[] pPowerRight;
		delete[] pIndex;
	}


	delete[] dlog;
	if (pBitmap) {
		delete pBitmap;
	}

	// do common plot paint jobs
	PlotPaintFinish(memDC.get(), rWnd, rClip);
}

/***************************************************************************/
// CPlotSpectrogram::OnDraw Drawing
// The data to draw is coming from a temporary file, created by the
// spectrogram processing class, which is called to do data processing if
// necessary before drawing. Call the function PlotPaintFinish at the end of
// the drawing to let the plot base class do common jobs like drawing the
// cursors.
/***************************************************************************/
void CPlotSpectrogram::OnDrawFormantTracksFragment(unique_ptr<CDC>& memDC, CRect rWnd, CRect rClip, CSaView * pView) {
	// get pointer to graph, view, document, application and mainframe
	CSaDoc * pDoc = pView->GetDocument();
	DWORD nSmpSize = pDoc->GetSampleSize();
	CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();

	CProcessFragments * pFragments = (CProcessFragments *)pDoc->GetFragments();
	short int nResult = LOWORD(pFragments->Process(this, pDoc));
	// check the process result. dont clear the graph because we've already drawn the spectrogram
	nResult = CheckResult(nResult, pFragments,false);
	if ((nResult == PROCESS_ERROR) || (nResult == PROCESS_CANCELED) || (!pFragments->IsDataReady())) {
		return;
	}

	// get pointer to process class and spectrogram parameters
	CProcessSpectrogram* pSpectrogram = GetSpectrogram(pDoc); // get pointer to spectrogram object
	CProcessSpectroFormants * pSpectroFormants = pSpectrogram->GetFormantProcess(); // get pointer to spectrogram object
	const CSpectroParm * pSpectroParm = &GetSpectrogram(pDoc)->GetSpectroParm();

	// draw the formants tracks if requested
	nResult = LOWORD(pSpectroFormants->ExtractFormants(pDoc, 0, pDoc->GetDataSize(), pSpectroParm->bSmoothFormantTracks));
	// check the process result. dont clear the graph because we've already drawn the spectrogram
	nResult = CheckResult(nResult, pSpectroFormants,false); 
	if ((nResult == PROCESS_ERROR) || (nResult == PROCESS_CANCELED) || (!pFragments->IsDataReady())) {
		return;
	}

	// get pointer to plot colors
	Colors * pColors = pMain->GetColors();

	// get sample data frame parameters
	double fDataStart = pView->GetDataPosition(rWnd.Width()) / nSmpSize; // index of first sample displayed in raw waveform plot
	DWORD dwDataLength = pView->GetDataFrame() / nSmpSize;   // number of samples displayed in raw waveform plot

	if (!IsRealTime()) {
		// Spectrogram B special case
		fDataStart = pSpectrogram->GetAreaPosition() / nSmpSize;
		dwDataLength = pSpectrogram->GetAreaLength() / nSmpSize;
	}

	// calculate scale factors and starting positions
	if (rClip.left > 0) {
		rClip.left--;
	}
	if (rClip.right < rWnd.right) {
		rClip.right++;
	}
	double fSamplesPerPix = (double)dwDataLength / (double)(rWnd.Width() + 1);
	DWORD dwFirstSample = (DWORD)((double)fDataStart + rClip.left*fSamplesPerPix + 0.5);
	DWORD dwLastSample = (DWORD)((double)fDataStart + rClip.right*fSamplesPerPix + 0.5);

	DWORD dwFirstFragment = pFragments->GetFragmentIndex(dwFirstSample);
	DWORD dwLastFragment = pFragments->GetFragmentIndex(dwLastSample);
	if (dwFirstFragment > 0) {
		dwFirstFragment--;
	}
	if (dwLastFragment < pFragments->GetFragmentIndex(pDoc->GetDataSize() / nSmpSize - 1)) {
		dwLastFragment++;
	}

	double fFormantFactor = (double)pSpectroParm->nFrequency / (double)rWnd.Height();

	COLORREF cFormantColor = pColors->cPlotData[pSpectroParm->nColor && pSpectroParm->bFormantColor ? 4 : 0];
	CPen penFormantTrack(PS_SOLID, 2, cFormantColor);
	CPen * pOldPen = memDC->SelectObject(&penFormantTrack);
	int nDrawMode = 0;
	if (pSpectroParm->nColor && !pSpectroParm->bFormantColor) {
		nDrawMode = memDC->SetROP2(R2_NOTMASKPEN);
	}
	for (int nFormantIndex = 1; nFormantIndex <= MAX_NUM_FORMANTS; nFormantIndex++) {
		switch (nFormantIndex) {
		case 1:
			if ((pSpectroParm->bShowFormants) && (!pSpectroParm->bShowF1)) {
				continue;
			}
			break;
		case 2:
			if ((pSpectroParm->bShowFormants) && (!pSpectroParm->bShowF2)) {
				continue;
			}
			break;
		case 3:
			if ((pSpectroParm->bShowFormants) && (!pSpectroParm->bShowF3)) {
				continue;
			}
			break;
		case 4:
			if ((pSpectroParm->bShowFormants) && (!pSpectroParm->bShowF4)) {
				continue;
			}
			break;
		default:
			if ((pSpectroParm->bShowFormants) && (!pSpectroParm->bShowF5andUp)) {
				continue;
			}
		}
		BOOL bPlot = FALSE;
		BOOL bOnScreen = FALSE;
		BOOL bSkip = TRUE;

		for (DWORD dwFragment = dwFirstFragment; dwFragment <= dwLastFragment; dwFragment++) {
			SFormantFreq * pFormants = pSpectroFormants->GetFormant(dwFragment); // return formant slice data
			ASSERT(pFormants);
			if ((pFormants->F[0] != (double)UNVOICED) &&
				(pFormants->F[0] != (double)NA) &&
				(pFormants->F[nFormantIndex] != (double)NA)) {
				bPlot = TRUE;
				int y = rWnd.bottom - (int)Round((double)pFormants->F[nFormantIndex] / (double)fFormantFactor);
				if (y >= rWnd.top) {
					bOnScreen = TRUE;
				}
				bSkip = bSkip || !ResearchSettings.m_bSpectrogramConnectFormants;  // Research Setting
				SFragParms FragmentParm = pFragments->GetFragmentParms(dwFragment);
				int x = max((int)(((double)FragmentParm.dwOffset - (double)fDataStart) / fSamplesPerPix + 0.5), rClip.left);
				if (bSkip) {
					memDC->MoveTo(x, y);
					x = min((int)(((double)FragmentParm.dwOffset + (double)FragmentParm.wLength - 1. - (double)fDataStart) / fSamplesPerPix + 0.5), rClip.right);
					bSkip = FALSE;
				}
				else {
					memDC->LineTo(x, y);
					x = min((int)(((double)FragmentParm.dwOffset + (double)FragmentParm.wLength - 1. - (double)fDataStart) / fSamplesPerPix + 0.5), rClip.right);
				}
				memDC->LineTo(x, y);
			} else {
				bSkip = TRUE;
			}
		}

		if (bPlot && !bOnScreen) {
			break;
		}
	}
	memDC->SelectObject(pOldPen);
	if (pSpectroParm->nColor && !pSpectroParm->bFormantColor) {
		memDC->SetROP2(nDrawMode);
	}
	return;
}


/***************************************************************************/
// CPlotSpectrogram::OnDraw Drawing
// The data to draw is coming from a temporary file, created by the
// spectrogram processing class, which is called to do data processing if
// necessary before drawing. Call the function PlotPaintFinish at the end of
// the drawing to let the plot base class do common jobs like drawing the
// cursors.
/***************************************************************************/
void CPlotSpectrogram::OnDrawFormantTracksTime(std::unique_ptr<CDC> & memDC, CRect rWnd, CRect rClip, CSaView * pView) {
	// get pointer to graph, view, document, application and mainframe
	CSaDoc  *  pDoc = pView->GetDocument();
	DWORD nSmpSize = pDoc->GetSampleSize();
	CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();

	// get pointer to process class and spectrogram parameters
	CProcessFormantTracker * pSpectroFormants = pDoc->GetFormantTracker(); // get pointer to spectrogram object
	CProcessSpectrogram* pSpectrogram = GetSpectrogram(pDoc); // get pointer to spectrogram object
	const CSpectroParm * pSpectroParm = &pSpectrogram->GetSpectroParm();

	// draw the formants tracks if requested
	short int nResult = LOWORD(pSpectroFormants->Process(pView, pDoc));
	// not the process result. dont clear the plot because we've already drawn the spectrogram
	nResult = CheckResult(nResult, pSpectroFormants,false);
	if ((nResult == PROCESS_ERROR) || (nResult == PROCESS_CANCELED)) {
		return;
	}

	// OK so far
	double dSamplesPerSlice = pDoc->GetDataSize() / nSmpSize / double(pSpectroFormants->GetDataSize(sizeof(SFormantFreq)));
	// get pointer to plot colors
	Colors * pColors = pMain->GetColors();

	// get sample data frame parameters
	double fDataStart = pView->GetDataPosition(rWnd.Width()) / nSmpSize; // index of first sample displayed in raw waveform plot
	DWORD dwDataLength = pView->GetDataFrame() / nSmpSize;   // number of samples displayed in raw waveform plot

	if (!IsRealTime()) {
		// Spectrogram B special case
		fDataStart = pSpectrogram->GetAreaPosition() / nSmpSize;
		dwDataLength = pSpectrogram->GetAreaLength() / nSmpSize;
	}

	// calculate scale factors and starting positions
	if (rClip.left > 0) {
		rClip.left--;
	}
	if (rClip.right < rWnd.right) {
		rClip.right++;
	}
	double fSamplesPerPix = (double)dwDataLength / (double)(rWnd.Width() + 1);

	DWORD dwFirstSlice = (DWORD)floor((rClip.left*fSamplesPerPix + fDataStart) / dSamplesPerSlice);
		DWORD dwLastSlice = (DWORD)ceil((rClip.right*fSamplesPerPix + fDataStart) / dSamplesPerSlice);


		double fFormantFactor = (double)pSpectroParm->nFrequency / (double)rWnd.Height();

	COLORREF cFormantColor = pColors->cPlotData[pSpectroParm->nColor && pSpectroParm->bFormantColor ? 4 : 0];
	CPen penFormantTrack(PS_SOLID, 2, cFormantColor);
	CPen * pOldPen = memDC->SelectObject(&penFormantTrack);
	int nDrawMode = 0;
	if (pSpectroParm->nColor && !pSpectroParm->bFormantColor) {
		nDrawMode = memDC->SetROP2(R2_NOTMASKPEN);
	}
	for (int nFormantIndex = 1; nFormantIndex <= MAX_NUM_FORMANTS; nFormantIndex++) {
		switch (nFormantIndex) {
		case 1:
			if ((pSpectroParm->bShowFormants) && (!pSpectroParm->bShowF1)) {
				continue;
			}
			break;
		case 2:
			if ((pSpectroParm->bShowFormants) && (!pSpectroParm->bShowF2)) {
				continue;
			}
			break;
		case 3:
			if ((pSpectroParm->bShowFormants) && (!pSpectroParm->bShowF3)) {
				continue;
			}
			break;
		case 4:
			if ((pSpectroParm->bShowFormants) && (!pSpectroParm->bShowF4)) {
				continue;
			}
			break;
		default:
			if ((pSpectroParm->bShowFormants) && (!pSpectroParm->bShowF5andUp)) {
				continue;
			}
		}
		BOOL bPlot = FALSE;
		BOOL bOnScreen = FALSE;
		BOOL bSkip = TRUE;
		CProcessIterator<SFormantFreq> iterFormants(*pSpectroFormants, dwFirstSlice);

		for (DWORD dwSlice = dwFirstSlice; dwSlice <= dwLastSlice; dwSlice++) {
			if ((*iterFormants).F[nFormantIndex] != (double)NA) {
				bPlot = TRUE;
				int y = rWnd.bottom - (int)Round((double)(*iterFormants).F[nFormantIndex] / (double)fFormantFactor);
				if (y >= rWnd.top) {
					bOnScreen = TRUE;
				}
				bSkip = bSkip || !ResearchSettings.m_bSpectrogramConnectFormants;  // Research Setting
				int x = max((int)((dSamplesPerSlice*dwSlice - (double)fDataStart) / fSamplesPerPix + 0.5), rClip.left);
				if (bSkip) {
					memDC->MoveTo(x, y);
					x = min((int)((dSamplesPerSlice*(dwSlice + 1) - 1. - (double)fDataStart) / fSamplesPerPix + 0.5), rClip.right);
					bSkip = FALSE;
				}
				else {
					memDC->LineTo(x, y);
					x = min((int)((dSamplesPerSlice*(dwSlice + 1) - 1. - (double)fDataStart) / fSamplesPerPix + 0.5), rClip.right);
				}
				memDC->LineTo(x, y);
				}
				else {
					bSkip = TRUE;
			}
			++iterFormants; // move to next slice
		}

		if (bPlot && !bOnScreen) {
			break;
		}
	}

	memDC->SelectObject(pOldPen);
	if (pSpectroParm->nColor && !pSpectroParm->bFormantColor) {
		memDC->SetROP2(nDrawMode);
	}
}

bool CPlotSpectrogram::OnDraw2(unique_ptr<CDC> & memDC, CRect rWnd, CRect rClip, CSaView * pView) {

	if (GetSafeHwnd() && IsIconic()) {
		return false;    // nothing to draw
	}
	// get pointer to graph, view, document, application and mainframe
	CGraphWnd * pGraph = (CGraphWnd *)GetParent();
	CSaDoc * pDoc = pView->GetDocument();
	// get pointer to process class
	CProcessSpectrogram * pSpectrogram = GetSpectrogram(pDoc); // get pointer to spectrogram object
	CProcessSpectroFormants* pSpectroFormants = pSpectrogram->GetFormantProcess(); // get pointer to spectrogram formants object
	// get pointer to spectrogram parameters
	const CSpectroParm * pSpectroParm = &pSpectrogram->GetSpectroParm();

	if (rWnd.Height() <= 0) {
		return false;    // nothing to draw
	}

	// check if process is idle
	if (pSpectrogram->IsIdle() && !IsRealTime()) {
		m_HelperWnd.SetMode(MODE_TEXT | FRAME_POPOUT | POS_HCENTER | POS_VCENTER, IDS_HELPERWND_SNAPSHOT, &rWnd);
		return false;
	}

	// create spectrogram data
	short int nResult = LOWORD(pSpectrogram->Process(this, pDoc, pView, rWnd.Width(), rWnd.Height())); // process data
	nResult = CheckResult(nResult, pSpectrogram); // check the process result
	if (nResult == PROCESS_ERROR) {
		return false;
	}

	if (nResult == PROCESS_CANCELED) {
		return true;
	}

	ShowCursors(false, true);

	// set legend scale
	pGraph->SetLegendScale(SCALE | NUMBERS, 0, (int)pSpectroParm->nFrequency, _T("f(Hz)")); // set legend scale

	// prepare to draw
	BOOL bSmoothSpectra = pSpectroParm->bSmoothSpectra;
	BOOL bAliased = pSpectrogram->IsAliased();
	OnDrawSpectrogram(memDC, rWnd, rClip, pView, bSmoothSpectra, &bAliased);

	if (nResult) {
		// new data processed, all has to be displayed
		// This gets the mplot wnd in overlay situation
		GetParent()->GetPlot()->Invalidate(); 
		return true;
	}

	if (pSpectroParm->bShowFormants) {
		if (pSpectroParm->bShowPitch) {
			BOOL bPitch = TRUE;
			CProcessGrappl * pGrappl = (CProcessGrappl *)pDoc->GetGrappl(); // get pointer to grappl object
			pGrappl->Process(this, pDoc);
			if (bPitch && pGrappl->IsDataReady() && !pGrappl->IsStatusFlag(PROCESS_NO_PITCH)) {
				SetProcessMultiplier(PRECISION_MULTIPLIER);
				SetBold(TRUE);
				CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
				int j = pSpectroParm->nColor && pSpectroParm->bFormantColor ? 2 : 0;
				pMainWnd->SwapInOverlayColors(j);
				PlotPrePaint(memDC.get(), rWnd, rClip); // Calculate Legend before drawing
				PlotStandardPaint(memDC.get(), rWnd, rWnd, pGrappl, pDoc, SKIP_UNSET); // do standard data paint */
				pMainWnd->SwapOutOverlayColors();
			}
		}

		if (pSpectroFormants->IsCanceled()) {
			m_pLastProcess = pDoc->GetSpectrogram()->GetFormantProcess();
			return true;
		}

		if (!formantTrackerOptions.m_bShowOriginalFormantTracks) {
			OnDrawFormantTracksTime(memDC, rWnd, rClip, pView);
		}
		if (formantTrackerOptions.m_bShowOriginalFormantTracks) {
			OnDrawFormantTracksFragment(memDC, rWnd, rClip, pView);
		}
		if (pSpectroFormants->IsCanceled()) {
			return true;
		}
		return true;
	}

	// if the user toggled to state of the display of formants while the process was cancelled, we can take down the 'cancelled' display.
	if ((pSpectrogram->IsIdle() || pSpectrogram->IsDataReady()) &&
		(pSpectroFormants->IsIdle() || pSpectroFormants->IsDataReady()) &&
		m_HelperWnd.IsCanceled()) {
		m_HelperWnd.SetMode(MODE_HIDDEN);
	}
	return true;
}

void CPlotSpectrogram::OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView) {
	CSaApp * pApp = (CSaApp *)AfxGetApp();
	CGraphWnd * pGraph = (CGraphWnd *)GetParent();
	CSaDoc * pDoc = pView->GetDocument();

	if (!CreateSpectroPalette(pDC, pDoc)) {
		// error creating color palette
		pApp->ErrorMessage(IDS_ERROR_SPECTROPALETTE);
		pGraph->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L); // close the graph
		return;
	}

	CSpectroParm cSpectroParm = GetSpectrogram(pDoc)->GetSpectroParm();
	CSpectroParm * pSpectroParm = &cSpectroParm;

	BOOL isColor = (pDC->GetDeviceCaps(BITSPIXEL) > 1);
	int  backupNcolor = pSpectroParm->nColor;

	if (!isColor) {
		pSpectroParm->nColor = 1; // use greyscale to match b/w printer.
		GetSpectrogram(pDoc)->SetSpectroParm(*pSpectroParm);
	}
	// create a temporary DC for the reading the screen
	std::unique_ptr<CDC> memDC(new CDC());
	if (!memDC.get()) {
		CSaString szError;
		szError.Format(_T("memory allocation error in ")_T(__FILE__)_T(" line %d"), __LINE__);
		((CSaApp*)AfxGetApp())->ErrorMessage(szError);
		return;
	}

	// create a bitmap to read the screen into and select it into the temporary DC
	unique_ptr<BITMAPINFO> info((BITMAPINFO *)malloc(sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD)));
	if (!info.get()) {
		CSaString szError;
		szError.Format(_T("memory allocation error in ")_T(__FILE__)_T(" line %d"), __LINE__);
		((CSaApp*)AfxGetApp())->ErrorMessage(szError);
		return;
	}

	if (!memDC->CreateCompatibleDC(NULL)) {
		CSaString szError;
		szError.Format(_T("CreateCompatibleDC Failed in ")_T(__FILE__)_T(" line %d"), __LINE__);
		((CSaApp*)AfxGetApp())->ErrorMessage(szError);
		return;
	}

	info->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info->bmiHeader.biWidth = (rWnd.Width() + 3) & ~0x3;
	info->bmiHeader.biHeight = -rWnd.Height(); // Bit map is top down (hence negative sign)
	info->bmiHeader.biPlanes = 1;
	info->bmiHeader.biBitCount = 8;
	info->bmiHeader.biCompression = BI_RGB;
	info->bmiHeader.biSizeImage = 0; // set to zero for uncompressed (BI_RGB)
	info->bmiHeader.biXPelsPerMeter = info->bmiHeader.biYPelsPerMeter = 2835; // 72 DPI
	info->bmiHeader.biClrUsed = 0; // fully populated bmiColors
	info->bmiHeader.biClrImportant = 0;
	populateBmiColors(info->bmiColors, pView);

	void * pBits;
	HBITMAP hBitmap = CreateDIBSection(memDC->m_hDC, info.get(), DIB_RGB_COLORS, &pBits, NULL, 0);
	if (!hBitmap) {
		if (info->bmiHeader.biWidth && info->bmiHeader.biHeight) {
			CSaString szError;
			szError.Format(_T("CreateDIBSection Failed in ")_T(__FILE__)_T(" line %d"), __LINE__);
			((CSaApp*)AfxGetApp())->ErrorMessage(szError);
		}
		return;
	}

	HBITMAP hOldBitmap = (HBITMAP)::SelectObject(memDC->m_hDC, hBitmap);
	// paint the data into the bitmap
	if (OnDraw2(memDC, rWnd, rClip, pView)) {
		GdiFlush();  // finish all drawing to memDC
		// copy to destination
		if (!pDC->BitBlt(rWnd.left, rWnd.top, rWnd.Width(), rWnd.Height(), memDC.get(), 0, 0, SRCCOPY)) {
			CSaString szError;
			szError.Format(_T("BitBLT Failed in ")_T(__FILE__)_T(" line %d"), __LINE__);
			((CSaApp *)AfxGetApp())->ErrorMessage(szError);
		}
		GdiFlush();  // finish BitBlt before destroying DIBSection
	} else {
		//erase background
		CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
		pDC->FillSolidRect(rClip, pMain->GetColors()->cPlotBkg);
		m_HelperWnd.Invalidate();
		m_HelperWnd.UpdateWindow();
	}

	memDC->SelectObject(hOldBitmap);
	DeleteObject(hBitmap);

	pSpectroParm->nColor = backupNcolor;
	GetSpectrogram(pDoc)->SetSpectroParm(*pSpectroParm);
}

CProcessSpectrogram * CPlotSpectrogram::GetSpectrogram(CSaDoc * pDoc) {
	m_pAreaProcess = (IsRealTime())?pDoc->GetSpectrogram():pDoc->GetSnapshot();
	return static_cast<CProcessSpectrogram *>(m_pAreaProcess);
}

BOOL CPlotSpectrogram::OnSetCursor(CWnd * /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/) {

	CGraphWnd * pGraph = (CGraphWnd *)GetParent();
	CSaView * pView = static_cast<CSaView *>(pGraph->GetParent());
	CSaDoc * pDoc = pView->GetDocument();
	// get pointer to process class
	CProcessSpectrogram * pSpectrogram = GetSpectrogram(pDoc); // get pointer to spectrogram object
	CProcessSpectroFormants* pSpectroFormants = pSpectrogram->GetFormantProcess(); // get pointer to spectrogram formants object
	// get pointer to spectrogram parameters
	const CSpectroParm * pSpectroParm = &pSpectrogram->GetSpectroParm();
	BOOL bShowFormantTracks = ((pSpectroParm->bShowFormants) && (pSpectroFormants->AreFormantTracksReady()));
	BOOL bHasFocus = (pGraph == pView->GetFocusedGraphWnd());
	if (bShowFormantTracks || !bHasFocus) {
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
	}
	else {
		::SetCursor(AfxGetApp()->LoadCursor(IDC_MFINGERN));
	}
	return TRUE;
}

bool CPlotSpectrogram::IsRealTime() const {
	return GetSpectroAB() == 'A';
}

BOOL CPlotSpectrogram::IsAreaGraph() const {
	return CPlotWnd::IsAreaGraph() && !IsRealTime();
}

BOOL CPlotSpectrogram::EraseBkgnd(CDC * /*pDC*/) {
	// we don't need to erase this plot
	return TRUE;
}
