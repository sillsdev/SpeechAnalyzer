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
//         RLJ Added SetProperties(int nNewID)
//    09/27/2000
//         DDO Added the function IsAnnotationVisible() so outsiders can
//             get the visible state of a particular annotation for a graph.
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_GRAPH_H

#include "sa_plot.h"
#include "sa_minic.h"
#include "SaString.h"

#define _SA_GRAPH_H

//###########################################################################
// CGraphWnd

class CPlotWnd;
class CLegendWnd;
class CXScaleWnd;
class CAnnotationWnd;
class CObjectOStream;
class CObjectIStream;

class CGraphWnd : public CMiniCaptionWnd {
    DECLARE_DYNCREATE(CGraphWnd)

public:
    CGraphWnd();
    CGraphWnd(UINT nID);
	// copy constructor
    CGraphWnd(const CGraphWnd & toBeCopied);
	// assignment operator
    CGraphWnd & operator=(const CGraphWnd &);
    virtual ~CGraphWnd();
    CPlotWnd * NewPlotFromID(UINT plotID);
    BOOL IsIDincluded(UINT id);
	// Save (Project) Settings
    void WriteProperties(CObjectOStream & obs);
	// Autoloading
    BOOL ReadProperties(CObjectIStream & obs);
	// Set graph's default properties (needed for "FileOpenAs->Phonetic/Music Analysis")
    BOOL SetProperties(int nNewID);
    void HideCursors();
    void ShowCursors();
	CPoint GetPopupMenuPosition();

	static EAnnotation m_anAnnWndOrder[ANNOT_WND_NUMBER]; // order to display annotation windows
    void RemoveRtPlots();
    void SetGraphFocus(BOOL bFocus);
    void PartialCopy(const CGraphWnd & toBeCopied);
    void RemoveOverlayItem(const CPlotWnd * pPlot);
    void SetCaptionStyle(int, BOOL bRedraw = FALSE); // set graph caption style
    void ScrollGraph(CSaView * pView, DWORD, DWORD);   // scroll graph
    void RedrawGraph(BOOL bEntire = TRUE, BOOL bLegend = FALSE, BOOL bGraph = TRUE); // redraw entire or parts of the graph
    void ResizeGraph(BOOL bEntire = TRUE, BOOL bLegend = FALSE, BOOL bGraph = TRUE); // resize entire or parts of the graph
    static double GetSemitone(double fFreq); // calculates semitones from a frequency
    static double SemitoneToFrequency(double fSemitone);
    static CSaString Semitone2Name(double fSemitone);
    void UpdateStatusBar(DWORD dwStartCursor, DWORD dwStopCursor, BOOL bForceUpdate = FALSE); // update the status bar
    // interface to plot window
    UINT GetPlotID() const;
    UINT IsPlotID(UINT test) const;
    void ShowSegmentBoundaries(BOOL bShow, BOOL bRedraw = FALSE);
    void ShowTranscriptionBoundaries(BOOL bShow);
    void SetMagnify(double bFactor, BOOL bRedraw = FALSE); // set magnify factor
    double GetMagnify();
    BOOL HaveBoundaries();
    BOOL HaveDrawingStyleLine();
    bool HaveCursors();
    bool HavePrivateCursor();
    BOOL HaveGrid();
    void SetLineDraw(BOOL bLine);
    void ShowGrid(BOOL bShow, BOOL bRedraw = FALSE);
	// set start cursor position
    void SetStartCursor(CSaView * pView);
	// set stop cursor position
    void SetStopCursor(CSaView * pView);
	// set stop cursor position
    void SetPlaybackCursor(CSaView * pView);
	// set start cursor position
    void SetPlaybackPosition(CSaView * pView, bool scroll); 
    void SetPlaybackFlash(bool on);
    void MoveStartCursor(CSaView * pView, DWORD dwNewPositon);
    void MoveStopCursor(CSaView * pView, DWORD dwNewPositon);
    void RestartProcess();
    // interface to annotation and legend/scale windows
    void CreateAnnotationWindows(); // creates the annotation windows
    BOOL SetLegendScale(int nMode, double dMinValue, double dMaxValue, TCHAR * pszDimension = NULL, int nDivisions = -1, double d3dOffset = 0.); // set legend scale
    void SetXScale(int nMode, int nMinValue, int nMaxValue, TCHAR * pszDimension = NULL, int nDivisions = -1, double d3dOffset = 0.); // set x-scale
    CAnnotationWnd * GetAnnotationWnd(int nIndex);
    CLegendWnd * GetLegendWnd();
    CXScaleWnd * GetXScaleWnd();
    void ShowLegend(BOOL bShow, BOOL bRedraw = FALSE);   // show or hide legend window
    void ShowXScale(BOOL bShow, BOOL bRedraw = FALSE);   // show or hide x-scale window
    void ShowAnnotation(EAnnotation nIndex, BOOL bShow, BOOL bRedraw = FALSE); // show or hide indexed annotation window
    void ShowCursors(bool bPrivate, bool bShow);
    BOOL HaveLegend();
    BOOL HaveXScale();
    BOOL HaveAnnotation(int nIndex);
    BOOL DisableLegend();
    BOOL DisableXScale();
    BOOL DisableAnnotation(int nIndex);
    void SetAreaGraph(BOOL bArea);
    void ChangeAnnotationSelection(int nIndex);      // change the selection in indexed annotation window
    void OnDraw(CDC * pDC, const CRect * printRect, int originX, int originY);
    void PrintHiResGraph(CDC * pDC, const CRect * printRect, int originX, int originY);
    void MergeInGraph(CGraphWnd * pGraphToMerge, CSaView * pView, bool bRtOverlay);
    BOOL static IsMergeableGraph(CGraphWnd * pGraphToMerge, BOOL bBaseGraph=FALSE);
    CPlotWnd * GetPlot() const;
    BOOL IsAnnotationVisible(int nAnnot);
    BOOL IsAreaGraph();
    BOOL IsAnimationGraph();
    BOOL IsCanceled();
    void AnimateFrame(DWORD dwFrameIndex);
    void EndAnimation();
	void SetPopupMenuLocation( CPoint point);

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnSetFocus(CWnd * /*cwp*/);
    afx_msg LRESULT OnGenderInfoChanged(WPARAM nGender, LPARAM lParam = 0L);
    afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
    afx_msg void OnDestroy();
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    DECLARE_MESSAGE_MAP()

    void Copy(const CGraphWnd & toBeCopied);
    void Clear(void);

	// plot window
    CPlotWnd * m_pPlot;
	// ID of plot window
    UINT m_nPlotID;
	// legend window embedded object
    CLegendWnd * m_pLegend;
	// x-scale window embedded object
    CXScaleWnd * m_pXScale;
	// TRUE = legend window shown
    BOOL m_bLegend;
	// TRUE = x-scale window shown
    BOOL m_bXScale;
	// array of pointers to the annotation window objects
    CAnnotationWnd * m_apAnnWnd[ANNOT_WND_NUMBER];
	// array of boolean, TRUE if annotation window shown
    BOOL m_abAnnWnd[ANNOT_WND_NUMBER];
	// TRUE = this is a area processed graph
    BOOL m_bAreaGraph;
	// last updated status bar start cursor position
    static DWORD m_dwLastStartCursor;
	// last updated status bar stop cursor position
    static DWORD m_dwLastStopCursor;
	// popup menu location
	CPoint m_PopupMenuPos;
};

//###########################################################################
// CGraphWnd
class CRecGraphWnd : public CGraphWnd {
    DECLARE_DYNCREATE(CRecGraphWnd)

public:
    CRecGraphWnd();
    virtual ~CRecGraphWnd() {
    };

protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT & cs);
};

#endif _SA_GRAPH_H
