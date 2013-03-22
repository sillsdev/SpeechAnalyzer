/////////////////////////////////////////////////////////////////////////////
// sa_annot.h:
// Interface of the CLegendWnd
//                  CXScaleWnd
//                  CAnnotationWnd
//                  CPhoneticWnd
//                  CToneWnd
//                  CPhonemicWnd
//                  COrthographicWnd
//                  CGlossWnd
//                  CReferenceWnd classes.
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



//###########################################################################
// CLegendWnd window

// CLegendWnd defines

// scale mode
#define NO_SCALE        0x0000  // no scale displayed
#define SCALE           0x0001  // scale with horizontal lines per division
#define NUMBERS         0x0002  // numbers near horizontal lines
#define LOG10           0x0004  // logarithmic (10 base) scale
#define ARBITRARY       0x0008  // do not align numbers to zero.
#define SCALE_INFO      0x0010  // show waveform info


#define IDM_PLAY 101
#define IDM_PAUSE 102
#define IDM_STOP 103
#define IDM_LOOP 104
#define IDM_VOICE 105
// 106-108 are used by Sa_Plot
#define IDM_UP 109    // For TWC legend buttons
#define IDM_DOWN 110  // For TWC legend buttons
#define IDM_TEMPO 111 // For staff control
#define IDM_PLAY_BOTH 112
#define IDM_PLAY_WAVE 113

class CLegendWnd : public CWnd {

    // Construction/destruction/creation
public:
    CLegendWnd();
    virtual ~CLegendWnd();

    // Attributes
private:
    int     m_nScaleMode;       // scale mode
    int     m_nGridDivisions;
    double  m_dScaleMinValue;   // scale min value
    double  m_dScaleMaxValue;   // scale max value
    double   m_d3dOffset;        // scale offset when used with 3d zgraph
    CString m_szScaleDimension; // scale dimension text
    CFont   m_font;             // legend character font
    HFONT hButtonFont;    // font for play/stop/pause/loop buttons
    HWND  hPlayButton;
    HWND  hPlayBothButton;
    HWND  hPlayWaveButton;
    HWND  hPauseButton;
    HWND  hStopButton;
    HWND  hLoopButton;
    HWND  hVoiceButton;
    HWND  hTempoButton;
    HWND  hConvertButton;
    HWND  hImportButton;
    HWND  hExportButton;
    HWND    hUpButton;          // For TWC legend buttons
    HWND    hDownButton;        // For TWC legend buttons
    BOOL    m_bRecalculate;     // TRUE, if scale has to be recalculated
    int     m_nHeightUsed;      // window height, used for last calculation
    int     m_nDivisionsUsed;      // window height, used for last calculation
    double  m_fMagnifyUsed;     // magnify factor, used for last calculation
    double  m_dFirstGridPos;    // position of first gridline
    double  m_fGridDistance;    // gridline distance in pixels
    double   m_fBase;            // base scale distance in numbers
    double  m_fNumbPerPix;      // numbers per pixel
    CRect   m_rRecalc;          // rectangle of recalculate button
    BOOL    m_bRecalcUp;        // status of recalculation button
    HWND  m_hUpButton;    // Up   button window for TWC legend (to adjust Semitone # offset)
    HWND  m_hDownButton;    // Down button window for TWC legend (to adjust Semitone # offset)

    // Operations
public:
    int GetWindowWidth(); // return width of legend window
    BOOL SetScale(int nMode, double dMinValue, double dMaxValue, TCHAR * pszDimension = NULL, int nDivisions = -1, double d3dOffset = 0.); // set legend scale
    void CalculateScale(CDC *, CRect *); // calculate new scale
    CFont * GetFont() {
        return &m_font;   // return selected font
    }
    int GetScaleMode() {
        return m_nScaleMode;   // return scale mode
    }
    double GetFirstGridPosition() {
        return m_dFirstGridPos;   // return position of first gridline
    }
    double GetGridDistance() {
        return m_fGridDistance;   // return distance between gridlines
    }
    CString * GetDimensionText() {
        return &m_szScaleDimension;   // return pointer to dimension text
    }
    double GetScaleBase() {
        return m_fBase;   // return base of scale
    }
    double GetScaleMaxValue() {
        return m_dScaleMaxValue;   // return max scale value
    }
    double GetScaleMinValue() {
        return m_dScaleMinValue;   // return min scale value
    }
    void OnDraw(CDC * pDC,
                const CRect & printRect,
                const CRect & printPlotWnd,
                const CRect & printXscaleRect,
                const CRect * printAnnotation);

private:
    virtual BOOL PreCreateWindow(CREATESTRUCT & cs);


    // Generated message map functions
protected:
    //{{AFX_MSG(CLegendWnd)
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC * pDC);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    afx_msg void OnSetFocus(CWnd * pOldWnd);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

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
    int     m_nScaleMode;       // scale mode
    int     m_nGridDivisions;
    double   m_fScaleMinValue;   // scale min value
    double   m_fScaleMaxValue;   // scale max value
    double   m_d3dOffset;        // scale offset when used with 3d zgraph
    CString m_szScaleDimension; // scale dimension text
    CFont   m_font;             // legend character font
    BOOL    m_bRecalculate;     // TRUE, if scale has to be recalculated
    int     m_nWidthUsed;       // window width, used for last calculation
    int     m_nDivisionsUsed;    // divisions used for last calculation
    double   m_fMinValueUsed;    // scale min value, used for last calculation
    double   m_fMaxValueUsed;    // scale max value, used for last calculation
    // calculation results
    double     m_fFirstGridPos;    // position of first large gridline
    double   m_fGridDistance;    // gridline distance in pixels
    double   m_fBase;            // base scale distance in numbers
    double   m_fNumbPerPix;      // numbers per pixel
    double   m_fPosition;        // position of first gridline (large or small)
    BOOL    m_bLargeLine;       // TRUE, if first gridline is large

    // Operations
public:
    int GetWindowHeight(); // return height of x-scale window
    BOOL SetScale(int nMode, double dMinValue, double nMaxValue, TCHAR * pszDimension = NULL, int nDivisions = -1, double d3dOffset = 0.);
    void CalculateScale(CDC *, int nWidth); // calculate new scale
    CFont * GetFont() {
        return &m_font;   // return selected font
    }
    int GetScaleMode() {
        return m_nScaleMode;   // return scale mode
    }
    double GetFirstGridPosition() {
        return m_fFirstGridPos;   // return position of first gridline
    }
    double GetGridDistance() {
        return m_fGridDistance;   // return distance between gridlines
    }
    CString * GetDimensionText() {
        return &m_szScaleDimension;   // return pointer to dimension text
    }
    void OnDraw(CDC * pDC, const CRect & printRect, const CRect & printPlotWnd);

private:
    int GetTWCXScaleWindowHeight();     // 09/27/2000 - DDO
    virtual BOOL PreCreateWindow(CREATESTRUCT & cs);

    // Generated message map functions
protected:
    //{{AFX_MSG(CXScaleWnd)
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC * pDC);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CAnnotationWnd window

class CAnnotationEdit;

class CAnnotationWnd : public CWnd {

    // Construction/destruction/creation
public:
    CAnnotationWnd(int nIndex);
    virtual BOOL PreCreateWindow(CREATESTRUCT & cs);
    virtual void OnDraw(CDC * pDC, const CRect & printRect);

    // Attributes
protected:
    int       m_nIndex;         // index of annotation window
    // SDM 1.06.2
    BOOL      m_bHintUpdateBoundaries;  // request for draw hint of updated boundaries
    DWORD     m_dwHintStart,m_dwHintStop;
    BOOL      m_bOverlap;
private:
    DWORD     m_nSelectTickCount;

    // Operations
public:
    int GetWindowHeight(); // return height of selected font
    CFont * GetFont();
    // SDM 1.06.2
    void SetHintUpdateBoundaries(BOOL bHint, DWORD dwStart, DWORD dwStop, BOOL bOverlap) ;
    void SetHintUpdateBoundaries(BOOL bHint, BOOL bOverlap) {
        SetHintUpdateBoundaries(bHint,m_dwHintStart,m_dwHintStop, bOverlap);
    };
    void OnCreateEdit(const CString * szString = NULL);
    static void CreateAnnotationFont(CFont * pFont, int nPoint, LPCTSTR szFace);

protected:  // implementation
    //    void CreateAnnotFont();

    // Generated message map functions
protected:
    //{{AFX_MSG(CAnnotationWnd)
    afx_msg BOOL OnEraseBkgnd(CDC * pDC);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnPaint();
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CPhoneticWnd window

class CPhoneticWnd : public CAnnotationWnd {

    // Construction/destruction/creation
public:
    CPhoneticWnd(int nIndex) : CAnnotationWnd(nIndex) {};
};

//###########################################################################
// CToneWnd window

class CToneWnd : public CAnnotationWnd {

    // Construction/destruction/creation
public:
    CToneWnd(int nIndex) : CAnnotationWnd(nIndex) {};
};

//###########################################################################
// CPhonemicWnd window

class CPhonemicWnd : public CAnnotationWnd {

    // Construction/destruction/creation
public:
    CPhonemicWnd(int nIndex) : CAnnotationWnd(nIndex) {};
};

//###########################################################################
// COrthographicWnd window

class COrthographicWnd : public CAnnotationWnd {

    // Construction/destruction/creation
public:
    COrthographicWnd(int nIndex) : CAnnotationWnd(nIndex) {};
};

//###########################################################################
// CGlossWnd window

class CGlossWnd : public CAnnotationWnd {
public:
    CGlossWnd(int nIndex) : CAnnotationWnd(nIndex) {};
    virtual void OnDraw(CDC * pDC, const CRect & printRect);
};

class CReferenceWnd : public CAnnotationWnd {
public:
    CReferenceWnd(int nIndex) : CAnnotationWnd(nIndex) {};
    virtual void OnDraw(CDC * pDC, const CRect & printRect);
};

class CMusicPhraseWnd : public CAnnotationWnd {
public:
	CMusicPhraseWnd(int nIndex) : CAnnotationWnd(nIndex) {};
};

#endif //_SA_ANNOT_H


