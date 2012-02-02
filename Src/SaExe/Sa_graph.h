/////////////////////////////////////////////////////////////////////////////
// sa_graph.h:
// Interface of the CGraphWnd classes.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//    1.06.6U4
//         SDM changed ScrollGraph to scroll exact number of pixels
//         SDM removed scroll error members
//    1.06.6U5
//         SDM changed float to double
//    1.06.6U6
//         SDM Added function SetPlaybackPosition
//    1.5Test8.1
//         SDM Added m_anAnnWndOrder[]
//    06/06/2000
//         RLJ Added bSetProperties(int nNewID)
//    09/27/2000
//         DDO Added the function IsAnnotationVisible() so outsiders can
//             get the visible state of a particular annotation for a graph.
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_GRAPH_H

#include "sa_plot.h"
#include "sa_minic.h"
#include "appdefs.h"
#include "resource.h"
#include "csastring.h"

#define _SA_GRAPH_H

//###########################################################################
// CGraphWnd

class CPlotWnd;
class CLegendWnd;
class CXScaleWnd;
class CAnnotationWnd;
class Object_ostream;
class Object_istream;

class CGraphWnd : public CMiniCaptionWnd
{   
	DECLARE_DYNCREATE(CGraphWnd)

	// Construction/destruction/creation
public:
	CGraphWnd() {;} // default constructor
	CGraphWnd(UINT nID);
	CGraphWnd(const CGraphWnd & toBeCopied);  // copy constructor
	CGraphWnd & operator=(const CGraphWnd&); // assignment operator
	virtual ~CGraphWnd();
	CPlotWnd * NewPlotFromID(UINT plotID);
	BOOL IsIDincluded(UINT id);
	void WriteProperties(Object_ostream& obs); // Save (Project) Settings
	BOOL bReadProperties(Object_istream& obs); // Autoloading
	BOOL bSetProperties(int nNewID); // Set graph's default properties (needed for "FileOpenAs->Phonetic/Music Analysis")

	// Attributes
protected:
	CPlotWnd*     m_pPlot;                // plot window
	UINT          m_nPlotID;              // ID of plot window
	CLegendWnd*   m_pLegend;              // legend window embedded object
	CXScaleWnd*   m_pXScale;              // x-scale window embedded object
	BOOL          m_bLegend;              // TRUE = legend window shown
	BOOL          m_bXScale;              // TRUE = x-scale window shown
	CAnnotationWnd* m_apAnnWnd[ANNOT_WND_NUMBER]; // array of pointers to the annotation window objects
	BOOL m_abAnnWnd[ANNOT_WND_NUMBER];    // array of boolean, TRUE if annotation window shown
	BOOL m_bAreaGraph;                    // TRUE = this is a area processed graph
	static DWORD m_dwLastStartCursor;     // last updated status bar start cursor position
	static DWORD m_dwLastStopCursor;      // last updated status bar stop cursor position
public:
	static int m_anAnnWndOrder[ANNOT_WND_NUMBER]; // order to display annotation windows

	// Operations
public:
	void RemoveRtPlots();
	void SetGraphFocus(BOOL bFocus);
	void PartialCopy( const CGraphWnd & toBeCopied);
	void RemoveOverlayItem( const CPlotWnd * pPlot);
	void SetCaptionStyle(int, BOOL bRedraw = FALSE); // set graph caption style
	void ScrollGraph(CSaView* pView, DWORD, DWORD);    // scroll graph
	void RedrawGraph(BOOL bEntire = TRUE, BOOL bLegend = FALSE, BOOL bGraph = TRUE); // redraw entire or parts of the graph
	void ResizeGraph(BOOL bEntire = TRUE, BOOL bLegend = FALSE, BOOL bGraph = TRUE); // resize entire or parts of the graph
	static double GetSemitone(double fFreq); // calculates semitones from a frequency
	static double SemitoneToFrequency(double fSemitone);
	static CSaString Semitone2Name(double fSemitone);
	void UpdateStatusBar(DWORD dwStartCursor, DWORD dwStopCursor, BOOL bForceUpdate = FALSE); // update the status bar
	// interface to plot window
	UINT GetPlotID() const {return m_nPlotID;} // return plot ID
	UINT IsPlotID(UINT test) const {return test == m_nPlotID;} // return plot ID
	void ShowBoundaries(BOOL bShow, BOOL bRedraw = FALSE) {m_pPlot->ShowBoundaries(bShow, bRedraw);}
	void SetMagnify(double bFactor, BOOL bRedraw = FALSE); // set magnify factor
	double GetMagnify() {return m_pPlot->GetMagnify();} // return magnify factor
	BOOL HaveBoundaries() {return m_pPlot->HaveBoundaries();} // boundaries visible?
	BOOL HaveDrawingStyleLine() {return m_pPlot->HaveDrawingStyleLine();} // return drawing style
	BOOL HaveCursors() {return m_pPlot->HaveCursors();} // cursors visible?
	BOOL HavePrivateCursor() {return m_pPlot->HavePrivateCursor();} // private cursor visible?
	BOOL HaveGrid() {return m_pPlot->HaveGrid();} // gridlines visible?
	void SetLineDraw(BOOL bLine) {if (m_pPlot) m_pPlot->SetLineDraw(bLine);}
	void ShowGrid(BOOL bShow, BOOL bRedraw = FALSE) {m_pPlot->ShowGrid(bShow, bRedraw);}
	void SetStartCursor(CSaView* pView); // set start cursor position
	void SetStopCursor(CSaView* pView);  // set stop cursor position
	void SetPlaybackPosition(CSaView* pView, DWORD dwPos); // set start cursor position
	void MoveStartCursor(CSaView* pView, DWORD dwNewPositon) {m_pPlot->MoveStartCursor(pView, dwNewPositon);}
	void MoveStopCursor(CSaView* pView, DWORD dwNewPositon) {m_pPlot->MoveStopCursor(pView, dwNewPositon);}
	void RestartProcess() {m_pPlot->RestartProcess();}
	// interface to annotation and legend/scale windows
	void CreateAnnotationWindows(); // creates the annotation windows
	BOOL SetLegendScale(int nMode, double dMinValue, double dMaxValue, TCHAR* pszDimension = NULL, int nDivisions = -1, double d3dOffset = 0.); // set legend scale
	void SetXScale(int nMode, int nMinValue, int nMaxValue, TCHAR* pszDimension = NULL, int nDivisions = -1, double d3dOffset = 0.); // set x-scale
	CAnnotationWnd* GetAnnotationWnd(int nIndex) {return m_apAnnWnd[nIndex];} // return pointer to indexed annotation window
	CLegendWnd* GetLegendWnd() {return m_pLegend;}       // return pointer to legend window
	CXScaleWnd* GetXScaleWnd() {return m_pXScale;}       // return pointer to x-scale window
	void ShowLegend(BOOL bShow, BOOL bRedraw = FALSE);   // show or hide legend window
	void ShowXScale(BOOL bShow, BOOL bRedraw = FALSE);   // show or hide x-scale window
	void ShowAnnotation(int nIndex, BOOL bShow, BOOL bRedraw = FALSE); // show or hide indexed annotation window
	void ShowCursors(BOOL bPrivate, BOOL bShow) {m_pPlot->ShowCursors(bPrivate, bShow);}  // set cursors visible/hidden
	BOOL HaveLegend() {return m_bLegend;}            // legend window visible?
	BOOL HaveXScale() {return m_bXScale;}            // x-scale window visible?
	BOOL HaveAnnotation(int nIndex) {return m_abAnnWnd[nIndex];} // indexed annotation window visible?
	BOOL DisableLegend();
	BOOL DisableXScale();
	BOOL DisableAnnotation(int nIndex);
	void SetAreaGraph(BOOL bArea) {m_bAreaGraph = bArea;} // set graph to area processed graph type
	void ChangeAnnotationSelection(int nIndex);      // change the selection in indexed annotation window
	void OnDraw(CDC * pDC, const CRect * printRect, int originX, int originY);
	void PrintHiResGraph(CDC * pDC, const CRect * printRect, int originX, int originY);
	void MergeInGraph(CGraphWnd * pGraphToMerge, CSaView * pView, bool bRtOverlay);
	BOOL static IsMergeableGraph(CGraphWnd * pGraphToMerge, BOOL bBaseGraph=FALSE);
	CPlotWnd* GetPlot() const             {return m_pPlot;}
	BOOL IsAnnotationVisible(int nAnnot)  {return m_abAnnWnd[nAnnot];}
	BOOL IsAreaGraph()                    {return m_bAreaGraph;}        // is this an area processed graph?
	BOOL IsAnimationGraph()               {return (m_pPlot->IsAnimationPlot());}
	BOOL IsCanceled()                     {return  m_pPlot->IsCanceled();}
	void AnimateFrame(DWORD dwFrameIndex);
	void EndAnimation();

	// Generated message map functions
protected:
	//{{AFX_MSG(CGraphWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* /*cwp*/) { if(IsPlotID(IDD_STAFF))  ::SetFocus(m_pPlot->m_hWnd);}
	afx_msg LRESULT OnGenderInfoChanged(WPARAM nGender, LPARAM lParam = 0L);
	afx_msg LRESULT OnCursorClicked(WPARAM Cursor, LPARAM ButtonState);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void Copy(const CGraphWnd & toBeCopied);
	void Clear(void);
};

//###########################################################################
// CGraphWnd
class CRecGraphWnd : public CGraphWnd
{   DECLARE_DYNCREATE(CRecGraphWnd)

public:
	CRecGraphWnd();
	virtual ~CRecGraphWnd()
	{
	};

protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
};

#endif _SA_GRAPH_H
