#if !defined(AFX_SA_G_3DPITCH_H__DEDA9BC3_3A24_11D5_9FE4_00E098784E13__INCLUDED_)
#define AFX_SA_G_3DPITCH_H__DEDA9BC3_3A24_11D5_9FE4_00E098784E13__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// sa_g_3dPitch.h : header file
//



/////////////////////////////////////////////////////////////////////////////
// CPlot3dPitch command target

class CProcess3dPitch;

class CPlot3dPitch : public CPlotWnd
{
  DECLARE_DYNCREATE(CPlot3dPitch)
// Construction
public:
	CPlot3dPitch();

  // Attributes
private:
  enum PaletteMode                          // mode of palette
  { 
    SYSTEMCOLOR,
    HALFCOLOR,
    FULLCOLOR,
  };
  static BOOL bPaletteInit;              // TRUE, if palette initialized
  static int nPaletteMode;        // mode of created palette
  static CPalette SpectroPalette;     // color palette

  CProcess3dPitch * m_p3dPitch;

  // Operations
private:
  BOOL CreateSpectroPalette(CDC* pDC, CDocument* pDoc); // creates the palette
  void populateBmiColors(RGBQUAD *Quadcolors,CSaView *pView);
  BOOL OnDraw2(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);
  BOOL OnDrawCorrelations(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

public:
  virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlot3dPitch)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPlot3dPitch();
  virtual BOOL  EraseBkgnd(CDC * /*pDC*/) { return TRUE;} // we dont need to erase this plot


	// Generated message map functions
protected:
	//{{AFX_MSG(CPlot3dPitch)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SA_G_3DPITCH_H__DEDA9BC3_3A24_11D5_9FE4_00E098784E13__INCLUDED_)
