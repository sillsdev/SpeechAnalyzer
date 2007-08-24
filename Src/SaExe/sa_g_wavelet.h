//***************************************************************************
//* sa_g_wavelet.h (v1.0)                                                   *
//* ----------------                                                        *
//* Project          : Speech Analyzer                                      *
//* Author           : Andy Heitke                                          *
//* Date Started     : 6/11/01                                              *
//* Customer Name    : JAARS / SIL                                          *
//* Description      : This is the main header file for the Wavelet         *
//*                    graph in SA																					*
//* Revision History : 7/30/01 ARH - Imported file into Speech Analyzer     *
//*																	 v2.0 project                           *
//***************************************************************************



#if !defined(AFX_SA_G_WAVELET_H__DEDA9BC3_3A24_11D5_9FE4_00E098784E13__INCLUDED_)
#define AFX_SA_G_WAVELET_H__DEDA9BC3_3A24_11D5_9FE4_00E098784E13__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000




// D E F I N E S
//**************************************************************************
#define sparsePaletteSize 30l					// SDM 1.5Test10.3

#define IDM_BUTTON1		300							// used for the little arrow buttons on the side of the graph
#define IDM_BUTTON2		301							// these could be used for implementing a time/frequency zoom
																			// but currently control the "red line" showing the different
																			// energy bands

// T Y P E D E F S
//**************************************************************************
typedef unsigned char UBYTE;

struct RGB
{
  long r;
  long g;
  long b;
};




//************************************************************************** 
// CPlotWavelet Class
//************************************************************************** 

class CPlotWavelet : public CPlotWnd
{
  DECLARE_DYNCREATE(CPlotWavelet)

  // Attributes
private:
  enum PaletteMode { SYSTEMCOLOR, HALFCOLOR, FULLCOLOR};

  static BOOL bPaletteInit;									// TRUE, if palette initialized
  static int nPaletteMode;									// mode of created palette
  static CPalette SpectroPalette;						// color palette

	long drawing_level;												// This variable gets updated by the arrow buttons on the legend
																						// and tells the red line which level to draw
	long max_drawing_level;


private:	


	// Color Palette functions
  BOOL CreateSpectroPalette(CDC* pDC, CDocument* pDoc); // creates the palette
  void populateBmiColors(RGBQUAD *Quadcolors,CSaView *pView);


public:
	CPlotWavelet();
	
	void IncreaseDrawingLevel() { if (drawing_level < max_drawing_level) drawing_level++; }
	void DecreaseDrawingLevel() { if (drawing_level > 1) drawing_level--; }


  virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);
	virtual ~CPlotWavelet();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPlotWavelet)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SA_G_3DPITCH_H__DEDA9BC3_3A24_11D5_9FE4_00E098784E13__INCLUDED_)
