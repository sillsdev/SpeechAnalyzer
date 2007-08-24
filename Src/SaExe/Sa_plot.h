/////////////////////////////////////////////////////////////////////////////
// sa_plot.h:
// Interface of the CPlotHelperWnd
//                  CPlotWnd       classes.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//   1.06.6U6
//       SDM Added function SetPlaybackCursor
//   1.5Test8.3
//       SDM Added SetProcessIndex
//   07/11/2000
//       RLJ Incremented IDM_CONVERT, etc. by 1 to prevent conflict with
//           IDM_VOICE (105) in SA_ANNOT.H
//   08/30/2000
//       DDO Made the following function virtual so they can be overridden:
//           GetDataPosition(), AdjustDataFrame(), GetAreaPosition() and
//           GetAreaLength().
//   4/2002
//       SDM Revised PlotStandardPaint to be more object oriented and more 
//           maintainable.  Included the drawing object declarations here
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _SA_PLOT_H

#include "sa_cursr.h"
#include "sa_annot.h"
#include "grid.h"

#define _SA_PLOT_H

class CGraphWnd;
class CSaDoc;
class CSaView;
class CDataProcess;
class CAreaDataProcess;

#define IDM_CONVERT          106 // was 105
#define IDM_CONVERT_DYNAMIC  107 // was 106
#define IDM_CONVERT_WEIGHTED 108 // was 107

//###########################################################################
// CPlotHelperWnd helper child window

// CPlotHelperWnd defines

// display mode, lowest 5 bits (32 modes), default is 0
#define MODE_MASK       0x001F  // masks the 5 lowest mode bits
#define MODE_HIDDEN     0       // window is not visible
#define MODE_TEXT       1       // window displays text
// frame mode next 5 bits (6 modes), default is no frame
#define FRAME_NONE      0x0000  // no window frame
#define FRAME_POPOUT    0x0020  // window frame pops out
#define FRAME_POPIN     0x0040  // window frame pops in
// window positioning code next 6 bits (8 modes), default is left top
#define POS_HLEFT       0x0000  // window horizontally left aligned
#define POS_HCENTER     0x0800  // window horizontally centered
#define POS_HRIGHT      0x1000  // window horizontally right aligned
#define POS_VTOP        0x0000  // window vertically top aligned
#define POS_VCENTER     0x2000  // window vertically centered
#define POS_VBOTTOM     0x4000  // window vertically bottom aligned

class CPlotHelperWnd : public CWnd
{ DECLARE_DYNCREATE(CPlotHelperWnd)

// Construction/destruction/creation
public:
  CPlotHelperWnd();
  ~CPlotHelperWnd();

  // Attributes
private:
  CFont   m_font;     // helper window font
  int     m_nMode;    // display mode
  CRect   m_rParent;  // parent windows client area
  int     m_nCharWidth; // helper windows font average character width
  int     m_nHeight;  // helper windows font height
  CString m_szText;   // window text

  // Operations
protected:
  CRect SetPosition(int nWidth, int nHeight, CRect* prParent); // set the helper windows position
public:
  int SetMode(int nMode, int nID = -1, CRect* prParent = NULL); // set the display mode

  // Generated message map functions
protected:
  //{{AFX_MSG(CPlotHelperWnd)
  afx_msg void OnPaint();
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CPlotWnd

// CPlotWnd defines

#define SKIP_UNSET      1  // don't draw unset (-1) data (skip)
#define SKIP_MISSING    2  // don't draw missing (-2) data (skip)
#define PAINT_CROSSES   4  // paint small crosses for each data point
#define PAINT_DOTS      8  // paint dot only for each data point
#define PAINT_LOG10     16 // paint logarithmic scale (base 10)
#define PAINT_SEMITONES 32 // take linear Hz data and plot as semitones
#define PAINT_DB        64 // take linear amplitude data and plot as dB
#define PAINT_MIN       128



class CPlotWnd : public CWnd
{   
DECLARE_DYNCREATE(CPlotWnd)

friend CStartCursorWnd; // they need to access protected members
friend CStopCursorWnd;

// Construction/destruction/creation
public:
  CPlotWnd();
  virtual ~CPlotWnd();
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  void SetParent(CGraphWnd * setParent) { m_pParent = setParent; };
  CGraphWnd * GetParent() { return m_pParent; };
  CString GetPlotName() const;
  void SetPlotName(const CString & plotName);
  virtual void GraphHasFocus(BOOL bFocus);
  CGraphWnd * GetGraph(void) { return m_pParent; };
  CPoint GetMousePointerPosition() {return m_MousePointerPosn;}
  UINT GetMouseButtonState() {return m_MouseButtonState;}
  void SetMousePointerPosition(CPoint point) {m_MousePointerPosn = point;}
  void SetMouseButtonState(UINT state) {m_MouseButtonState = state;}

protected:
	virtual void PostNcDestroy()
	{
		delete this;
	};

  // Attributes
protected:
  BOOL         m_bInitialPlot;          // TRUE = initial plot
  CString      m_szPlotName;
  CGraphWnd  * m_pParent;
  CPlotHelperWnd m_HelperWnd;           // helper window embedded object
  CStartCursorWnd* m_pStartCursor;      // start cursor window embedded object
  CStopCursorWnd* m_pStopCursor;        // stop cursor window embedded object
  CPrivateCursorWnd* m_pPrivateCursor;  // private cursor window embedded object
  CPrivateCursorWnd* m_pPlaybackCursor; // private cursor window embedded object
  CDataProcess* m_pLastProcess;     // pointer to MRU process
  CAreaDataProcess* m_pAreaProcess;     // pointer to area process (needs deleting)
  BOOL  m_bBoundaries;                  // TRUE = boundaries shown
  BOOL  m_bLineDraw;                    // TRUE = drawing style is line
  BOOL  m_bDotDraw;                     // TRUE = drawing style is dots
  BOOL  m_bCursors;                     // TRUE = cursors visible
  BOOL  m_bPrivateCursor;               // TRUE = private cursor visible
  BOOL  m_bGrid;                        // TRUE = gridlines visible
  double m_fMagnify;                    // magnify factor
  double m_fVScale;                     // vertical scale
  double m_dProcessMultiplier;
  DWORD m_dwHighLightPosition;          // highlighted area position
  DWORD m_dwHighLightLength;            // highlighted area length
  BOOL	m_bAnimationPlot;               // TRUE = plot can be animated (defaults to FALSE)
  DWORD m_dwAnimationFrame;             // animation frame index
  CPoint m_MousePointerPosn;            // mouse pointer position 
  UINT m_MouseButtonState;              // mouse button flags
  CBitmap* m_pBitmapSave;               // pointer to original bitmap
  HWND DynamicCB;                       // 'Dynamic' check box TWC control
  HWND WeightedButton;                  // 'Weighted' radio button melogram control

  // Operations
private:
  BOOL PlotPrePaintDots(CDC* pDC, int nTop, CRect rClip,
    CLegendWnd* pLegend, CXScaleWnd* pXScale,
    COLORREF cColor, int nStyle);

protected:
  void ChangeCursorPosition(CSaView* pView, DWORD dwNewPosition, CCursorWnd*, BOOL bMove = FALSE); // change the current cursor position
public:
	void SetRtOverlay();
	bool bIsRtOverlay();
	virtual void RemoveRtPlots();
  virtual CPlotWnd * NewCopy(void);
  virtual void CopyTo(CPlotWnd * pTarg);
  void ShowBoundaries(BOOL bShow, BOOL bRedraw = FALSE); // show or hide boundaries
  virtual void SetMagnify(double, BOOL bRedraw = FALSE);      // set magnify factor
  double GetMagnify() {return m_fMagnify;}            // return magnify factor
  BOOL HaveBoundaries() {return m_bBoundaries;}       // boundaries visible?
  BOOL HaveDrawingStyleLine() {return m_bLineDraw;}   // return drawing style
  BOOL HaveDrawingStyleDots() {return m_bDotDraw;}    // return drawing style
  BOOL HaveCursors() {return m_bCursors;}             // cursors visible?
  BOOL HavePrivateCursor() {return m_bPrivateCursor;} // private cursor visible?
  BOOL HaveGrid() {return m_bGrid;}                   // gridlines visible?
  void SetLineDraw(BOOL bLine);                       // set line or solid drawing style
  void SetDotsDraw(BOOL);              // set dots drawing style
  void ShowCursors(BOOL bPrivate, BOOL bShow);        // set cursors visible/hidden
  BOOL ShowGrid(BOOL bShow, BOOL bRedraw = FALSE);    // show or hide gridlines
  void ScrollPlot(CSaView* pView, int nAmount, DWORD dwOldPos, DWORD dwFrame); // scroll plot and cursors
  void RedrawPlot(BOOL bEntire = TRUE);               // repaint plot (entire or partial)
  virtual void SetStartCursor(CSaView* pView);          // position the start cursor window
  virtual void SetStopCursor(CSaView* pView);           // position the stop cursor window
  void SetPlaybackCursor(CSaView* pView, DWORD dwPos);
  void MoveStartCursor(CSaView* pView, DWORD dwNewPositon); // move the start cursor window
  void MoveStopCursor(CSaView* pView, DWORD dwNewPositon);  // move the stop cursor window
  CStartCursorWnd* GetStartCursorWnd() {return m_pStartCursor;} // return pointer to start cursor window
  CStopCursorWnd* GetStopCursorWnd() {return m_pStopCursor;} // return pointer to start cursor window
  CPrivateCursorWnd* GetPrivateCursorWnd() {return m_pPrivateCursor;} // return pointer to private cursor window
  int  GetStartCursorPosition();                   // return the pos. in pixel coord. of the start cursor
  int  GetStopCursorPosition();                    // return the pos. in pixel coord. of the stop cursor
  void SetInitialPrivateCursor();                  // set the initial private cursor
  int  GetPrivateCursorPosition();                 // return the pos. in pixel coord. of the private cursor
  BOOL IsCanceled();                               // return canceled state
  virtual void RestartProcess();                   // restart canceled process
  virtual DWORD GetAreaPosition();                         // return area position
  virtual DWORD GetAreaLength(CRect * pRwnd = NULL);       // return area length
  virtual void OnDraw(CDC * /*pDC*/, CRect /*rWnd*/, CRect /*rClip*/, CSaView * /*pView*/) {};
  DWORD GetHighLightPosition() {return m_dwHighLightPosition;} // return highlight area position
  DWORD GetHighLightLength() {return m_dwHighLightLength;} // return highlight area length
  virtual void  SetHighLightArea(DWORD dwStart, DWORD dwStop, BOOL bRedraw = TRUE, BOOL bSecondSelection = FALSE); // set a highlighted area
  DWORD CalcWaveOffsetAtPixel(CPoint pixel);                       // calculate waveform sample byte offset at horizontal pixel position

  virtual void GenderInfoChanged(int /*nGender*/) {}
  BOOL IsAnimationPlot() {return m_bAnimationPlot;}   // TRUE = plot can be animated (defaults to FALSE in constructor)
  void SetAnimationFrame(DWORD dwFrameIndex) {m_dwAnimationFrame = dwFrameIndex;}   // set animation frame to fragment index
  virtual void AnimateFrame(DWORD /*dwFrameIndex*/){}         // animate a single frame (fragment)
  virtual void EndAnimation() {}                          // terminate animation and return to resting state
  virtual BOOL IsAreaGraph() const {return m_pAreaProcess != NULL;}

protected:
  short int CheckResult(short int nResult, CDataProcess* pProcess); // check the process result
  void PlotPrePaint(CDC* pDC, CRect rWnd, CRect rClip, CLegendWnd* pLegend = NULL, 
	                BOOL bCursors = TRUE, BOOL bPrivateCursor = FALSE);  // do the common plot painting before data has been drawn
  virtual void PlotStandardPaint(CDC* pDC,CRect rWnd, CRect rClip,
    CDataProcess* pProcess, CSaDoc* pProcessDoc, int nFlags = 0);  // standard plot painting
  void PlotPaintFinish(CDC* pDC, CRect rWnd, CRect rClip);         // do the common plot paint jobs after data has been drawn
  virtual void StandardAnimateFrame(DWORD dwFrameIndex);           // animate the plot for the specified frame
  virtual void StandardEndAnimation();                             // complete animation
  virtual Grid GetGrid() const;
  void SetHorizontalCursors(BOOL bValue=TRUE) {m_bHorizontalCursors = bValue;}
  virtual int GetPenThickness() const {return m_bBold ? 2 : 1;}
  void SetBold(BOOL bValue=TRUE) {m_bBold = bValue;}
  BOOL GetBold() const {return m_bBold;}

  //**************************************************************************
  // 08/30/2000 - DDO Added so these could be overridden.
  //**************************************************************************
  virtual double GetDataPosition(int iWidth);
  virtual DWORD AdjustDataFrame(int iWidth);
  virtual BOOL  EraseBkgnd(CDC *pDC);              // 10/24/2000 - DDO
  //**************************************************************************

private:
	bool m_bRtPlot;
  BOOL m_bHorizontalCursors;
  BOOL m_bBold;

  // Generated message map functions
protected:
  // Process Multiplier is value 
  virtual double GetProcessMultiplier() const {return m_dProcessMultiplier;}
  virtual double SetProcessMultiplier(double dScale) {double dResult = m_dProcessMultiplier; m_dProcessMultiplier = dScale; return dResult;}
  //{{AFX_MSG(CPlotWnd)
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnMouseMove(UINT nFlags, CPoint Point);
  afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	//}}AFX_MSG
  DECLARE_MESSAGE_MAP()

  virtual void OnPaint();
};


// Classes designed to support standard drawing

class CDataSource
{
public:
  virtual ~CDataSource() {;}

  struct CValues
  {
    int nFirst;
    int nMax;
    int nMin;
    int nLast;
  };

  virtual void GetValues(int &nFirstSample, int nLastSample, CValues &values, BOOL &bValid)=0;
};

class CDataSourceSimple : public CDataSource
{
public:
  CDataSourceSimple(CDataProcess &cProcess);
  virtual ~CDataSourceSimple() {;}

  virtual void GetValues(int &nFirstSample, int nLastSample, CValues &values, BOOL &bValid);

private:
  CDataProcess &m_cProcess;
  int m_nSamples;
};

class CDataSourceValidate : public CDataSource
{
public:
  CDataSourceValidate(CDataProcess &cProcess, BOOL bUnset, BOOL bMissing);
  virtual ~CDataSourceValidate() {;}

  virtual void GetValues(int &nFirstSample, int nLastSample, CValues &values, BOOL &bValid);

private:
  CDataProcess &m_cProcess;
  int m_nSamples;
  BOOL m_bSkipUnset;   // Values of -1 are "Unset"
  BOOL m_bSkipMissing; // Values of -2 are "Missing"
};

class CXScale
{
public:
  virtual ~CXScale() {;}

  virtual int GetX(double fSample) const { return round(fSample); } 
  virtual double GetSample(int x) const {return x; } 
};

class CXScaleLinear : public CXScale
{
public:
  CXScaleLinear(double fSamplesPerX, double fSampleAtZero) 
    : m_fSamplesPerX(fSamplesPerX),m_fSampleAtZero(fSampleAtZero),m_fXPerSample(fSamplesPerX ? 1/fSamplesPerX : 0)
  {;}
  virtual ~CXScaleLinear() {;}

  virtual int GetX(double fSamples) const { return round((fSamples - m_fSampleAtZero)*m_fXPerSample); } 
  virtual double GetSample(int x) const {return x*m_fSamplesPerX + m_fSampleAtZero; } 

private:
  const double m_fSamplesPerX;
  const double m_fSampleAtZero;
  const double m_fXPerSample;
};

class CYScale
{
public:
  virtual ~CYScale() {;}

  virtual int GetY(double fPos) const { return round(fPos); } 
  virtual double GetValue(int y) const {return y; } 
};

class CYScaleLinear : public CYScale
{
public:
  CYScaleLinear(double fYPerValue, double fYAtZero) 
    : m_fYPerValue(fYPerValue),m_fYAtZero(fYAtZero),m_fValuePerY(fYPerValue ? 1/fYPerValue : 0)
  {;}
  virtual ~CYScaleLinear() {;}

  virtual int GetY(double fValue) const { return round(fValue*m_fYPerValue + m_fYAtZero); } 
  virtual double GetValue(int y) const {return (y-m_fYAtZero)*m_fValuePerY; } 

private:
  const double m_fValuePerY;
  const double m_fYAtZero;
  const double m_fYPerValue;
};

class CYScaleLog : public CYScale
{
public:
  CYScaleLog(double fLogScale, double fLogOffset)
    : m_fLogScale(fLogScale),m_fLogOffset(fLogOffset),m_fLogInvScale(fLogScale ? 1/fLogScale : 0)
  {;}
  virtual ~CYScaleLog() {;}

  virtual int GetY(double fValue) const { return round(fValue > 0 ? log10(fValue)*m_fLogScale + m_fLogOffset : m_fLogScale*100.); } 
  virtual double GetValue(int y) const {return pow(10,(y-m_fLogOffset)*m_fLogInvScale); } 

private:
  const double m_fLogScale;
  const double m_fLogOffset;
  const double m_fLogInvScale;
};

class CYScaleDB : public CYScaleLog
{
public:
  CYScaleDB(double fYPerValue, double fYAtZero, double fdBReference, double fdBScale=20.) 
    : CYScaleLog(fdBScale*fYPerValue, fYAtZero + fdBReference*fYPerValue) 
  {;}
  virtual ~CYScaleDB() {;}
};

class CYScaleSemitones : public CYScaleLog
{
public:
  CYScaleSemitones(double fYPerValue, double fYAtZero, double fPitchScale) 
    : CYScaleLog(dSemitoneScale*fYPerValue, fYAtZero + dSemitoneReference*fYPerValue - log10(fPitchScale)*dSemitoneScale*fYPerValue)
  {;}
  virtual ~CYScaleSemitones() {;}

private:
  static const double dSemitoneScale;
  static const double dSemitoneReference;
};

class CDrawSegment
{
public:
  CDrawSegment(CDC &cDC) : m_cDC(cDC) {;}
  virtual ~CDrawSegment() {;}

  // draw scaling y values
  virtual void DrawTo(int x, CDataSource::CValues &value, const CYScale &cYScale, BOOL bValid = TRUE);
  virtual void DrawTo(int x, int value, const CYScale &cYScale, BOOL bValid = TRUE) {UNUSED_ALWAYS(x);UNUSED_ALWAYS(value);UNUSED_ALWAYS(cYScale);UNUSED_ALWAYS(bValid);}

  // draw scaling x values
  virtual void DrawTo(CDataSource::CValues &xValues, const CXScale &cXScale, int y, BOOL bValid = TRUE);
  virtual void DrawTo(int x, const CXScale &cXScale, int y, BOOL bValid = TRUE)  {UNUSED_ALWAYS(x);UNUSED_ALWAYS(cXScale);UNUSED_ALWAYS(y);UNUSED_ALWAYS(bValid);}

  void SetColor(COLORREF clr) {m_clr = clr;}

protected:
  CDC &m_cDC;
  COLORREF m_clr;
};

class CDrawSegmentSolid : public CDrawSegment
{
public:
  CDrawSegmentSolid(CDC &cDC, int nReferenceY) : 
      CDrawSegment(cDC), m_nReferenceY(nReferenceY)
      { m_nOldX = m_nCurrentX = 0; m_nMaxValue = INT_MIN; m_nMinValue = INT_MAX;}

  virtual ~CDrawSegmentSolid() {;}

  virtual void DrawTo(int x, int value, const CYScale &cYScale, BOOL bValid = TRUE);
protected:
  const int m_nReferenceY;
  int m_nOldX;
  int m_nCurrentX;
  int m_nMaxValue;
  int m_nMinValue;
};

class CDrawSegmentLine : public CDrawSegment
{
public:
  CDrawSegmentLine(CDC &cDC) : CDrawSegment(cDC) {m_pPoints = new POINT[kSpace+1]; m_nSize = 0;Flush();}
  virtual ~CDrawSegmentLine() {delete [] m_pPoints;}

  virtual void DrawTo(int x, int value, const CYScale &cYScale, BOOL bValid = TRUE);
  // Alternate drawing orientation used for vertical histograms
  virtual void DrawTo(int x, const CXScale &cXScale, int y, BOOL bValid = TRUE);

  enum { kSpace = 4095};

private:
  int m_nSize;
  POINT *m_pPoints;

  void Flush();
};

class CDrawSegmentSample : public CDrawSegmentLine
{
public:
  CDrawSegmentSample(CDC &cDC) : CDrawSegmentLine(cDC) {;}
  virtual ~CDrawSegmentSample() {;}

  virtual void DrawTo(int x, CDataSource::CValues &value, const CYScale &cYScale, BOOL bValid = TRUE);
  virtual void DrawTo(int x, int value, const CYScale &cYScale, BOOL bValid = TRUE);
};

class CDrawSegmentDotOnly : public CDrawSegmentLine
{
public:
  CDrawSegmentDotOnly(CDC &cDC) : CDrawSegmentLine(cDC) {;}
  virtual ~CDrawSegmentDotOnly() {;}

  virtual void DrawTo(int x, CDataSource::CValues &value, const CYScale &cYScale, BOOL bValid = TRUE);
  virtual void DrawTo(int x, int value, const CYScale &cYScale, BOOL bValid = TRUE);
};

#endif //_SA_PLOT_H
