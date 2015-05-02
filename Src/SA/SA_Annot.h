/////////////////////////////////////////////////////////////////////////////
// sa_annot.h:
// Interface of the CXScaleWnd
//                  CAnnotationWnd
//                  CPhoneticWnd
//                  CToneWnd
//                  CPhonemicWnd
//                  COrthographicWnd
//                  CGlossWnd classes.
//
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//   1.06.2
//         SDM removed parameterless constructors from CAnnotationWnd and sub classes
//         SDM added SetHintUpdateBoundaries and members m_bHintUpdateBoundaries...
//   1.06.4
//         SDM changed return type for CAnnotationWnd::GetFont
//         SDM added include for CFontTable
//   1.06.6
//         SDM added m_nSelectTickCount

//   1.06.6U4
//         SDM changed floats to double to fix precision errors in scrolling
//   1.5Test8.1
//         SDM added CReferenceWnd class
//         SDM changed parameters for ClegendWnd::OnDraw
//         SDM added member CAnnotationWnd::m_bOverlap
//         SDM changed parameters to CAnnotationWnd::SetHintUpdateBoundaries
//   07/20/2000
//         RLJ Added support for adjusting y-scale (semitone #) in Melogram
//   09/27/2000
//         DDO Added the function GetTWCXScaleWindowHeight() to the x-scale
//             window class.
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_ANNOT_H
#define _SA_ANNOT_H

class Colors;

//###########################################################################
// CXScaleWnd window

// CXScaleWnd defines

// scale mode
#define NO_SCALE        0x0000  // no scale displayed
#define SCALE           0x0001  // scale with vertical lines per division
#define NUMBERS         0x0002  // numbers near vertical lines
#define TIME_FROM_VIEW  0x0004  // take the time scale from view

class CXScaleWnd : public CWnd {

    // Construction/destruction/creation
public:
    CXScaleWnd();
    virtual ~CXScaleWnd();

    // Attributes
private:
	// scale mode
    int m_nScaleMode;
    int m_nGridDivisions;
	// scale min value
    double m_fScaleMinValue;
	// scale max value
    double m_fScaleMaxValue;
	// scale offset when used with 3d zgraph
    double m_d3dOffset;
	// scale dimension text
    CString m_szScaleDimension;
	// legend character font
    CFont m_font;
	// TRUE, if scale has to be recalculated
    BOOL m_bRecalculate;
	// window width, used for last calculation
    int m_nWidthUsed;
	// divisions used for last calculation
    int m_nDivisionsUsed;
	// scale min value, used for last calculation
    double m_fMinValueUsed;
	// scale max value, used for last calculation
    double m_fMaxValueUsed;
    // calculation results
	// position of first large gridline
    double m_fFirstGridPos;
	// gridline distance in pixels
    double m_fGridDistance;
	// base scale distance in numbers
    double m_fBase;
	// numbers per pixel
    double m_fNumbPerPix;
	// position of first gridline (large or small)
    double m_fPosition;
	// TRUE, if first gridline is large
    BOOL m_bLargeLine;

    // Operations
public:
	// return height of x-scale window
    int GetWindowHeight();
    BOOL SetScale(int nMode, double dMinValue, double nMaxValue, TCHAR * pszDimension = NULL, int nDivisions = -1, double d3dOffset = 0.);
	// calculate new scale
    void CalculateScale(CDC *, int nWidth);
    CFont * GetFont();
    int GetScaleMode();
    double GetFirstGridPosition();
    double GetGridDistance();
    CString * GetDimensionText();
    void OnDraw(CDC * pDC, const CRect & printRect, const CRect & printPlotWnd);

private:
    int GetTWCXScaleWindowHeight();
    virtual BOOL PreCreateWindow(CREATESTRUCT & cs);

    // Generated message map functions
protected:
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC * pDC);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CAnnotationWnd window

class CDlgAnnotationEdit;

class CAnnotationWnd : public CWnd {

    // Construction/destruction/creation
public:
    CAnnotationWnd(int nIndex);
    virtual BOOL PreCreateWindow(CREATESTRUCT & cs);
    virtual void OnDraw(CDC * pDC, const CRect & printRect);
	// return height of selected font
    int GetWindowHeight();
    CFont * GetFont();
    // SDM 1.06.2
    void SetHintUpdateBoundaries(bool bHint, DWORD dwStart, DWORD dwStop, bool bOverlap);
    void SetHintUpdateBoundaries(bool bHint, bool bOverlap);
    void OnCreateEdit(const CString * szString = NULL);
    static void CreateAnnotationFont(CFont * pFont, int nPoint, LPCTSTR szFace);
    void ShowTranscriptionBoundaries(BOOL val);

protected:
    void DrawTranscriptionBorders(CDC * pDC, CRect rWnd, Colors * pColors);

    afx_msg BOOL OnEraseBkgnd(CDC * pDC);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnPaint();
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	// index of annotation window
    int m_nIndex;
	// request for draw hint of updated boundaries
    bool m_bHintUpdateBoundaries;
    DWORD m_dwHintStart;
	DWORD m_dwHintStop;
    bool m_bOverlap;
    bool m_bTranscriptionBoundary;

    DECLARE_MESSAGE_MAP()

private:
	bool m_bHoverOver;
    DWORD m_nSelectTickCount;
};

//###########################################################################
// CPhoneticWnd window

class CPhoneticWnd : public CAnnotationWnd {
public:
    CPhoneticWnd(int nIndex);
};

//###########################################################################
// CToneWnd window

class CToneWnd : public CAnnotationWnd {
public:
    CToneWnd(int nIndex);
};

//###########################################################################
// CPhonemicWnd window

class CPhonemicWnd : public CAnnotationWnd {
public:
    CPhonemicWnd(int nIndex);
};

//###########################################################################
// COrthographicWnd window

class COrthographicWnd : public CAnnotationWnd {
public:
    COrthographicWnd(int nIndex);
};

//###########################################################################
// CGlossWnd window

class CGlossWnd : public CAnnotationWnd {
public:
    CGlossWnd(int nIndex);
    virtual void OnDraw(CDC * pDC, const CRect & printRect);
};

class CMusicPhraseWnd : public CAnnotationWnd {
public:
    CMusicPhraseWnd(int nIndex);
    virtual void OnDraw(CDC * pDC, const CRect & printRect);
};

#endif
