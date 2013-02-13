/////////////////////////////////////////////////////////////////////////////
// sa_view.h:
// Interface of the CSaView class
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//  1.06.5
//      SDM Added m_AdvancedSelection & ASelection() to support virtual selection
//      SDM Added command handlers for segment add, edit, remove, & navigation
//  1.06.6U2
//      SDM Added OnEditAddSyllable
//  1.06.6U5
//      SDM Added InitialUpdate()
//      SDM Changed DeleteGraphs() to return WINDOWPLACEMENT
//  1.06.6U6
//      SDM Added function SetPlaybackPosition
//  1.5Test10.5
//        SDM added variables to support SetPlaybackPosition improved accuracy
//        SDM added OnTimer
//  06/01/2000
//        RLJ Added ChangeDisplayedGraphs(int OpenMode) so that
//            "File-->Open As" can selectively bring up graphs for
//            Phonetic Analysis or Music Analysis (rather than
//            using waveform default or *.PSA settings).
//        RLJ Added pSaApp and pViewMainFrame in order to reduce
//              Sa_view.obj size enough for LINK/DEBUG to work.
//  06/07/2000
//        RLJ For use with "FileOpenAs->Phonetic/Music Analysis"
//              Added CREATE_FROMSCRATCH option to ::CreateGraphs,
//              Deleted ChangeDisplayedGraphs(int OpenMode)
//              Added CreateOpenAsGraphs(int OpenMode)
//              Changed from private to public:
//                UINT  m_anGraphID[MAX_GRAPHS_NUMBER];
// 06/17/2000
//        RLJ Extend FileOpenAs to support not only Phonetic/Music Analysis,
//              but also OpenScreenF, OpenScreenG, OpenScreenI, OpenScreenK,
//              OpenScreenM, etc.
// 06/22/200
//        RLJ Added SendPlayMessage(UINT Int1, UINT Int2)
//              to make LINK/DEBUG work
//
// 07/26/2000
//        DDO - Added messages and functions for the Layout with Types
//              menu options on Graphs menu.
//
// 09/26/2000
//        DDO - Added another parameter to DeleteGraphs()
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_VIEW_H

#include "colors.h"
#include "sa_ansel.h"
#include "CSaString.h"

#define _SA_VIEW_H

typedef enum ECREATE_HOW { CREATE_STANDARD=0, CREATE_FROMSTREAM=1, CREATE_FROMGRAPH=2, CREATE_FROMSCRATCH=3 } CREATE_HOW;

//###########################################################################
// CSaView view

class CPickOverlayDlg;
class CASegmentSelection;
class CDib;
class CPrintOptionsDlg;
class Object_istream;  
class CSaApp;
class CGraphWnd;
class CRecGraphWnd;
class CStopwatch;
class CMainFrame;
struct CPrintInfo;
class CMusicPhraseSegment;
class CSegment;
class CSaDoc;
class CDlgAutoRecorder;

class CSaView : public CView
{
	//  friend class CMiniFrame;

	DECLARE_DYNCREATE(CSaView)

public:
	CSaView(const CSaView * pToBeCopied = NULL);  // copy constructor
	CSaView & operator=(const CSaView&); // assignment operator

	// Construction/destruction/creation
protected:
	BOOL PreCreateWindow(CREATESTRUCT& cs);
	void Copy(const CSaView & toBeCopied);
	void PartialCopy(const CSaView & fromThis);
	void Clear(void);


	// Attributes
public:
	CGraphWnd * m_apGraphs[MAX_GRAPHS_NUMBER]; // array of pointers to the graph objects

private:
	UINT  m_anGraphID[MAX_GRAPHS_NUMBER]; // array of graph IDs
	UINT  m_nLayout;                     // actual Layout number

private:
	CASegmentSelection m_advancedSelection;
	CGraphWnd* m_pFocusedGraph;          // pointer to focused graph
	CPrintOptionsDlg * m_pPageLayout;
	CPrintOptionsDlg * m_pPgLayoutBackup;
	CPickOverlayDlg * m_pPickOverlay;

	UINT  m_nFocusedID;                  // ID of focused graph
	BOOL  m_bLegendAll;                  // legend window show/hide all
	BOOL  m_bLegendNone;                 // legend window hide all
	BOOL  m_bXScaleAll;                  // x-scale window show/hide all
	BOOL  m_bXScaleNone;                 // x-scale window hide all
	BOOL  m_abAnnAll[ANNOT_WND_NUMBER];  // array of boolean, annotation window show/hide all
	BOOL  m_abAnnNone[ANNOT_WND_NUMBER]; // array of boolean, annotation window hide all
	CURSOR_ALIGNMENT m_nCursorAlignment;            // cursor snap mode: align to sample, zero crossing, or fragment
	BOOL  m_bBoundariesAll;              // boundaries show/hide all
	BOOL  m_bBoundariesNone;             // boundaries hide all
	BOOL  m_bUpdateBoundaries;           // boundaries updated or not in transcription editor
	bool  m_bEditBoundaries;			 // TRUE = INS pressed
	bool  m_bEditSegmentSize;			 // TRUE = CTRL_SHIFT pressed
	BOOL  m_bDrawStyleLine;              // graph drawing style line or solid
	DWORD m_dwDataPosition;              // current start position of displayed data
	double m_fMagnify;                   // magnify factor
	double m_fZoom;                      // current zoom factor
	double m_fMaxZoom;                   // max. zoom factor
	DWORD m_dwHScrollFactor;             // factor to represent scroll position on horizontal scroll bar
	double m_fVScrollSteps;              // number of vertical scroll steps
	DWORD m_dwScrollLine;                // number of samples to scroll one line
	DWORD m_dwStartCursor;               // start cursor position
	DWORD m_dwStopCursor;                // stop cursor position
	double m_dPlaybackPosition;          // playback cursor position
	DWORD m_dwPlaybackTime;              // TickCount of last playback update
	double m_dPlaybackPositionLimit;     // playback cursor position
	int   m_nPlaybackSpeed;
	BOOL  m_bViewIsActive;               // TRUE = view is activated
	BOOL  m_bAnimating;                  // TRUE = animation in progress
	BOOL  m_bStaticTWC;                  // TRUE if the 'Dynamic' TWC check box control is NOT checked
	BOOL  m_bNormalMelogram;             // TRUE if the 'Weighted' Melogram check box control is NOT checked
	CStopwatch* m_pStopwatch;            // pointer to stopwatch for timing events


	// internal data for printing
	POINT     m_printArea;               // the size of the part of the printed page that we will use.
	CPoint    m_printerDPI;              // the actual dpi of the printer
	int       m_newPrinterDPI;           // the dpi we set up for printing
	double    m_printScaleX;             // scaling factor for scaling along x axis
	double    m_printScaleY;             // scaling factor for scaling along y axis
	Colors    m_saveColors;              // save the colors before optimizing them for print, restore later.
	BOOL      m_bPrintPreviewInProgress; // true if a print preview is in progress.

	// internal data for screen shot printing
	CRect     m_memRect;                 // the frame rectangle
	CDib    * m_pCDibForPrint;           // we convert from a bitmap to a CDib.
	CPoint    m_printOrigin;             // upper left corner where printing starts
	//     - only needed for screen shot
	BOOL      m_restartPageOptions;

	// internal data for saving the window state
	int m_z; // The z-order of the MDI child frame corresponding to this view.
	int m_eInitialShowCmd; // The state of the window when SA was closed.
	static Object_istream* s_pobsAutoload;
	BOOL m_WeJustReadTheProperties;
	BOOL m_bViewCreated;

	CSaApp* pSaApp;
	CMainFrame* pViewMainFrame;

	// Operations
private:
	static int  GetNumberOfGraphs(UINT *pGraphIDs);        // return number of graphs from layout ID                // change graph type
	WINDOWPLACEMENT DeleteGraphs(int nPosition = -1, BOOL bClearID = TRUE); // delete existing graph objects
	void CreateGraph(int nPosition, int nNewID,
		CREATE_HOW ch = CREATE_STANDARD,
		Object_istream * pFromStream = NULL,
		CGraphWnd * pFromGraph= NULL);

private:
	void ChangeLayout(UINT nNewLayout);               // change graph layout
	void ChangeGraph( int nID);               // change graph type
	void ChangeGraph( int idx, int nID);               // change graph type

	BOOL IsFocusGraph(UINT id);
	BOOL GraphIDincluded(UINT id);
	void EditAddGloss( bool bDelimiter);
	//    void SendPlayMessage(UINT Int1, UINT Int2);
	void SendPlayMessage(WORD Int1, WORD Int2);
	BOOL StartAnimation(DWORD dwStartWaveIndex, DWORD dwStopWaveIndex);
	void EndAnimation();
	void ShowAnnotation(int nAnnot);
	UINT GraphPtrtoID(CGraphWnd* pGraph);
	int  GraphPtrToOffset(CGraphWnd * pGraph);

	//********************************************
	// 09/2000 - DDO
	//********************************************
	void    ToggleAnnotation(int nAnnot, BOOL bShow, BOOL bRawDataOnly = FALSE);
	void    ToggleDpGraph(UINT nID);                      // 09/27/2000 - DDO
	void    UpdateDpGraphsMenu(CCmdUI* pCmdUI, int nID);  // 09/27/2000 - DDO
	void    MakeGraphArraysContiguous();                  // 09/24/2000 - DDO
	BOOL    GetGraphSubRect(const CRect *pWndRect, CRect *pSubRect, int nPos, const UINT *anGraphID = NULL) const { return GetGraphSubRect(m_nLayout, pWndRect, pSubRect, nPos, anGraphID); }
	void    ArrangeMelogramGraphs(const CRect *pMeloRect, UINT *pGraphIDs);
	BOOL    GraphTypeEnabled(int nID, BOOL bIncludeCtrlKeyCheck = FALSE);
	void    CreateOneGraph(UINT *pID, CGraphWnd **pGraph);
	void    CreateOneGraphStepOne(UINT nID, CGraphWnd **pGraph, CREATE_HOW how, CGraphWnd *pFromGraph, Object_istream * pObs);
	UINT    PreCreateOverlayGraph(int nIndex);
	//********************************************
public:
	static UINT SetLayout(UINT*);                            // return the corresponding layout to selected graphs
	static CSaString GetGraphTitle(UINT nID);
	static int GetNumberOfGraphsInLayout(UINT nID);         // return number of graphs from layout ID
	static BOOL GetGraphSubRect(int nLayout, const CRect *pWndRect, CRect *pSubRect, int nPos, const UINT *anGraphID = NULL);
	static BOOL IDDSelected(const UINT*, UINT);

public:
	UINT GetLayout(void) {return m_nLayout;}               // DDO - 08/07/00
	//********************************************

	CGraphWnd* GraphIDtoPtr(UINT id);
	int     GetGraphIndexForIDD(UINT);   // get index for a given IDD resource
	void    RefreshGraphs(BOOL bEntire = TRUE, BOOL bLegend = FALSE, BOOL bLayout = FALSE); // refresh (redraw) the graphs (entire or partial)
	void    BroadcastMessage(UINT Message , WPARAM wParam = 0, LPARAM lParam = 0);
	CSaDoc* GetDocument();               // get a pointer to the document
	UINT*   GetGraphIDs()            {return &m_anGraphID[0];}  // get the graph IDs
	CSaString GetGraphsDescription() const;
	static CSaString GetGraphsDescription(const UINT*);
	CGraphWnd* GetGraph(int nIndex)  {if (nIndex < 0 || nIndex > MAX_GRAPHS_NUMBER) return NULL; else return m_apGraphs[nIndex];} // get the pointers to a graph
	int     GetGraphUpdateMode();
	UINT    GetDynamicGraphCount();
	UINT    GetAnimationGraphCount();
	BOOL    IsAnimationRequested();
	BOOL    IsAnimating() {return m_bAnimating;} 
	int     GetAnimationFrameRate();
	CURSOR_ALIGNMENT GetCursorAlignment();   // get cursor snap mode
	void    ChangeCursorAlignment(CURSOR_ALIGNMENT nCursorSetting) {m_nCursorAlignment = nCursorSetting; OnCursorAlignmentChanged();}
	double   GetDataPosition(int nWndWidth); // get the actual data position
	DWORD   GetDataFrame();              // return current data frame (width) of displayed data
	DWORD AdjustDataFrame(int nWndWidth); // return adjusted data frame width for particular window
	DWORD   GetStartCursorPosition() {return m_dwStartCursor;} // get the start cursor position
	DWORD   GetStopCursorPosition()  {return m_dwStopCursor;}  // get the stop cursor position
	void    SetCursorPosition(int nCursorSelect, DWORD dwNewPos, SNAP_DIRECTION nSnapDirection = SNAP_BOTH,
				CURSOR_ALIGNMENT nCursorAlignment = ALIGN_USER_SETTING);
	void    SetStartCursorPosition(DWORD dwNewPos, SNAP_DIRECTION nSnapDirection = SNAP_BOTH,
				CURSOR_ALIGNMENT nCursorAlignment = ALIGN_USER_SETTING); // set new start cursor position
	void    SetStopCursorPosition(DWORD dwNewPos, SNAP_DIRECTION nSnapDirection = SNAP_BOTH,
				CURSOR_ALIGNMENT nCursorAlignment = ALIGN_USER_SETTING);  // set new stop cursor
	void    SetStartStopCursorPosition( DWORD dwNewStartPos, DWORD dwNewStopPos, SNAP_DIRECTION nSnapDirection = SNAP_BOTH,
				CURSOR_ALIGNMENT nCursorAlignment = ALIGN_USER_SETTING); // set new start/stop cursor position
	void    SetPlaybackPosition(double dwPos = ~0,int nSpeed = 0, BOOL bEstimate = FALSE);  // default hide playback position indicators
	int     iGetStartCursorSegment(int iSegment);
	int     iGetStopCursorSegment(int iSegment);
	void    MoveStartCursor(DWORD);        // move start cursor
	void    MoveStopCursor(DWORD);         // move stop cursor
	void    GraphsZoomCursors(DWORD dwStart, DWORD dwStop, DWORD percent = 80);
	void    GetDataFrame(DWORD &dwStart, DWORD &dwFrame); // set view frame start and width of data to display
	void    SetDataFrame(DWORD dwStart, DWORD dwFrame); // set view frame start and width of data to display
	void    ChangeAnnotationSelection(CSegment*, int nIndex, DWORD dwStart, DWORD dwStop); // change the annotation selection
	void    ChangeAnnotationSelection(CSegment*, int nIndex); // change the annotation selection
	void    DeselectAnnotations(void);
	void    SetFocusedGraph(CGraphWnd*); // sets the focused graph pointer
	CGraphWnd * GetFocusedGraphWnd() {return m_pFocusedGraph;} // gets the focused graph window pointer
	UINT    GetFocusedGraphID() {return m_nFocusedID;} // gets the focused graph ID
	void    ResetFocusedGraph();         // resets the focused graph pointer
	BOOL    ViewIsActive() {return m_bViewIsActive;}; // returns TRUE, if view is active
	BOOL    IsUpdateBoundaries() {return m_bUpdateBoundaries;} // return TRUE, if boundaries updated
	void    SetUpdateBoundaries(BOOL bUpdate) {m_bUpdateBoundaries = bUpdate;}
	void    ZoomIn(double fZoomAmount, BOOL bZoom = TRUE); // zoom in
	void    ZoomOut(double fZoomAmount);     // zoom out
	void    SetScrolling();                 // set scrolling parameters
	void    SetInitialCursors();            // set initial cursor positions
	void    AdjustCursors(DWORD dwSectionStart, DWORD dwSectionLength, BOOL bShrink); // adjust cursor positions to new file size
	BOOL    IsAnyAnnotationSelected(void);
	BOOL    IsCutAllowed();
	void    OnUpdateHasSel(CCmdUI* pCmdUI);
	void    OnEditCopy();
	CSaString GetSelectedAnnotationString(void);
	void    ChangeSelectedAnnotationData(const CSaString & str);
	int     FindSelectedAnnotationIndex();
	CSegment *FindSelectedAnnotation();
	CSegment *GetAnnotation(int annotSetID);
	DWORD OnPlaybackSegment();
	DWORD OnPlaybackWord();
	DWORD OnPlaybackPhraseL2();

	//SDM 1.06.5
	CASegmentSelection& ASelection() {return m_advancedSelection;};
	void RemoveSelectedAnnotation();
	void OnEditPaste();
	void OnEditPasteNew();
	void OnEditCut();
	void OnEditCopyMeasurements();

	void Scroll(DWORD desiredPosition)
	{
		UINT nPos = (UINT)(desiredPosition / m_dwHScrollFactor);
		SendMessage( WM_HSCROLL, SB_THUMBPOSITION, nPos);
	};

	void   Scroll(UINT nSBCode, UINT nPos = 0) {SendMessage(WM_HSCROLL, nSBCode, nPos);}
	BOOL   PrintPreviewInProgress() { return m_bPrintPreviewInProgress; };
	int    PrinterDPI() { return m_newPrinterDPI; };
	CPoint RealPrinterDPI() { return m_printerDPI; };

	void NotifyFragmentDone(void *pCaller);
	// methods for saving the windows settings
	static CSaView* s_pviewActiveChild(CMDIChildWnd* pwnd);
	int z() const { return m_z; } // The bottom window's z is zero.
	void SetZ(int z) { m_z = z; } // Greater z means above; lesser z means below.
	CMDIChildWnd* pwndChildFrame() const;
	void ShowInitialStateAndZ();
	void ShowInitialTopState();
	void WriteProperties(Object_ostream& obs); // Save (Project) Settings
	BOOL ReadProperties(Object_istream& obs, BOOL createGraphs); // Autoloading
	BOOL ReadGraphListProperties(Object_istream& obs, BOOL createGraphs); // Autoloading
	static void s_SetObjectStream(Object_istream& obs);
	static void s_ClearObjectStream();
	BOOL ReadGraphListProperties(const CSaView & fromThis);
	void InitialUpdate(BOOL bTemp=FALSE); // called first time after construct and when applying new wave
	void CreateOpenAsGraphs(UINT OpenAsID);
	BOOL GetStaticTWC()                   {return m_bStaticTWC;}          // TCJ 6/23/00
	void SetStaticTWC(BOOL bChecked)      {m_bStaticTWC = bChecked;}      // TCJ 6/23/00
	BOOL GetNormalMelogram()              {return m_bNormalMelogram;}     // TCJ 6/23/00
	void SetNormalMelogram(BOOL bChecked) {m_bNormalMelogram = bChecked;} // TCJ 6/23/00

	void MoveStartCursorRight();                                              // TCJ 7/6/00
	void MoveStartCursorLeft();                                               // TCJ 7/6/00

	int GetEditBoundaries(int nFlags, BOOL checkKeys = TRUE);
	BOOL AssignOverlay( CGraphWnd * pTarget, CSaView * pSourceView);		// change graph type

protected:
	virtual void OnInitialUpdate(); // called first time after construct

public:
	void RemoveRtPlots();
	virtual ~CSaView();
	CMainFrame* MainFrame() {return pViewMainFrame;}
	LRESULT OnFrameAnimationDone(WPARAM wParam = 0, LPARAM lParam = 0L);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSaView)
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

public:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	void SetScrollRange(int nBar, int nMinPos, int nMaxPos, BOOL bRedraw = TRUE);

	// printing methods
	void CalcPrintScaling(const CRect * srcRect);
	void CalcPrintRect(CRect * pPrintRect, const CRect * srcRect);
	void CalcPrintPoint(CPoint * pPrintPoint, const CPoint * srcPoint);
	void OnPrint(CDC * pDC, CPrintInfo * pInfo);
	void OnPrepareDC(CDC * pDC, CPrintInfo * pInfo);
	BOOL SaDoPreparePrinting(CPrintInfo * pInfo, BOOL bIsPortrait);
	void OnPrintPageSetup(void);
	int  CalcGraphIndexForCurrentPage(int currPage);
	void PrintPageTitle(CDC * pDC, int titleAreaHeight);
	void PrintGraph(CDC * pDC, const CRect * viewRect, int graphNum,
		const CRect * graphRect, int originX, int originY);
	void CalcCustomPage(CRect * customPage, const CRect * viewRect, int row, int col);
	void DoHiResPrint(CDC * pDC, CPrintInfo * pInfo);
	void PrepareDCforHiResPrint(CDC * pDC, CPrintInfo * pInfo);
	void DoScreenShot(CDC * pDC, CPrintInfo * pInfo);
	void CalculatePrintArea(CDC * pDC, CPrintInfo * pInfo);
	void CalculatePrintOrigin(CDC * pDC);
	int  CalculateHiResPrintPages(void);
	void PreparePrintingForScreenShot(void);

protected:
	BOOL DestroyGraph(CGraphWnd **pGraph, BOOL bResetFocus = TRUE);

	CRecGraphWnd * CreateRecGraph(CRecGraphWnd *pFromGraph = NULL, Object_istream *pObs = NULL);
public:
	void OnGraphsTypesPostProcess(const UINT * GraphIDs, int nLayout = -1);

	// Generated message map functions
protected:

	void OnUpdateEditPaste(CCmdUI* pCmdUI);
	void OnUpdateEditPasteNew(CCmdUI* pCmdUI);
	void OnEditUndo();
	void OnUpdateEditUndo(CCmdUI* pCmdUI);
	void OnEditRedo();
	void OnUpdateEditRedo(CCmdUI* pCmdUI);
	void OnEditAddPhrase(CMusicPhraseSegment *pSeg);
	void OnUpdateEditAddPhrase(CCmdUI* pCmdUI, CMusicPhraseSegment* pSeg);


	//{{AFX_MSG(CSaView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdatePrintPreview(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFilenew(CCmdUI* pCmdUI);
	afx_msg void OnGraphsTypes();
	afx_msg void OnGraphsParameters();
	afx_msg void OnEditBoundaries();
	afx_msg void OnUpdateEditBoundaries(CCmdUI *pCmdUI);
	afx_msg void OnEditSegmentSize();
	afx_msg void OnUpdateEditSegmentSize(CCmdUI *pCmdUI);
	afx_msg void OnExportStaff();
	afx_msg void OnUpdateExportStaff(CCmdUI *pCmdUI);
	afx_msg void OnImportStaff();
	afx_msg void OnUpdateImportStaff(CCmdUI *pCmdUI);
	afx_msg void OnPlaybackCursors();
	afx_msg void OnPlaybackFile();
	afx_msg void OnPlaybackWindow();
	afx_msg void OnPlaybackLtostart();
	afx_msg void OnPlaybackStarttor();
	afx_msg void OnPlaybackLtoStop();
	afx_msg void OnPlaybackStopToR();
	afx_msg void OnPlayerStop();
	afx_msg void OnUpdatePlayerStop(CCmdUI* pCmdUI);
	afx_msg void OnPlayer();
	afx_msg void OnExportXML();
	afx_msg void OnExportFW();
	afx_msg void OnExportSFM();
	afx_msg void OnExportTimeTable();
	afx_msg void OnExportMIDI();
	afx_msg void OnImportSFM();
	afx_msg void OnImportSFT();
	afx_msg void OnFilePhonologyAssistant();
	afx_msg void OnFileInformation();
	afx_msg void OnGraphsRetile();
	afx_msg void OnNextGraph();
	afx_msg void OnLegendAll();
	afx_msg void OnUpdateLegendAll(CCmdUI* pCmdUI);
	afx_msg void OnLegendRawdata();
	afx_msg void OnUpdateLegendRawdata(CCmdUI* pCmdUI);
	afx_msg void OnLegendNone();
	afx_msg void OnUpdateLegendNone(CCmdUI* pCmdUI);
	afx_msg void OnXScaleAll();
	afx_msg void OnUpdateXScaleAll(CCmdUI* pCmdUI);
	afx_msg void OnXScaleRawdata();
	afx_msg void OnUpdateXScaleRawdata(CCmdUI* pCmdUI);
	afx_msg void OnXScaleNone();
	afx_msg void OnUpdateXScaleNone(CCmdUI* pCmdUI);
	afx_msg void OnBoundariesAll();
	afx_msg void OnUpdateBoundariesAll(CCmdUI* pCmdUI);
	afx_msg void OnBoundariesRawdata();
	afx_msg void OnUpdateBoundariesRawdata(CCmdUI* pCmdUI);
	afx_msg void OnBoundariesNone();
	afx_msg void OnUpdateBoundariesNone(CCmdUI* pCmdUI);
	afx_msg void OnGraphsZoomCursors();
	afx_msg void OnUpdateGraphsZoomCursors(CCmdUI* pCmdUI);
	afx_msg void OnGraphsZoomAll();
	afx_msg void OnUpdateGraphsZoomAll(CCmdUI* pCmdUI);
	afx_msg void OnGraphsZoomIn();
	afx_msg void OnUpdateGraphsZoomIn(CCmdUI* pCmdUI);
	afx_msg void OnGraphsZoomOut();
	afx_msg void OnUpdateGraphsZoomOut(CCmdUI* pCmdUI);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGraphsStyleLine();
	afx_msg void OnUpdateGraphsStyleLine(CCmdUI* pCmdUI);
	afx_msg void OnGraphsStyleSolid();
	afx_msg void OnUpdateGraphsStyleSolid(CCmdUI* pCmdUI);
	afx_msg void OnPopupgraphStyleLine();
	afx_msg void OnUpdatePopupgraphStyleLine(CCmdUI* pCmdUI);
	afx_msg void OnPopupgraphStyleSolid();
	afx_msg void OnUpdatePopupgraphStyleSolid(CCmdUI* pCmdUI);
	afx_msg void OnPopupgraphStyleDots();
	afx_msg void OnUpdatePopupgraphStyleDots(CCmdUI* pCmdUI);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnActivateView(BOOL, CView*, CView*);
	afx_msg void OnDestroy();
	afx_msg void OnUpdateGraphsTypes(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGraphsParameters(CCmdUI* pCmdUI);
	afx_msg void OnPopupgraphGridlines();
	afx_msg void OnUpdatePopupgraphGridlines(CCmdUI* pCmdUI);
	afx_msg void OnPopupgraphLegend();
	afx_msg void OnUpdatePopupgraphLegend(CCmdUI* pCmdUI);
	afx_msg void OnPopupgraphXScale();
	afx_msg void OnUpdatePopupgraphXScale(CCmdUI* pCmdUI);
	afx_msg void OnDrawingBoundaries();
	afx_msg void OnUpdateDrawingBoundaries(CCmdUI* pCmdUI);
	afx_msg void OnGraphsMagnify1();
	afx_msg void OnUpdateGraphsMagnify1(CCmdUI* pCmdUI);
	afx_msg void OnGraphsMagnify2();
	afx_msg void OnUpdateGraphsMagnify2(CCmdUI* pCmdUI);
	afx_msg void OnGraphsMagnify4();
	afx_msg void OnUpdateGraphsMagnify4(CCmdUI* pCmdUI);
	afx_msg void OnGraphsMagnifycustom();
	afx_msg void OnUpdateGraphsMagnifycustom(CCmdUI* pCmdUI);
	afx_msg void OnPopupgraphMagnify1();
	afx_msg void OnUpdatePopupgraphMagnify1(CCmdUI* pCmdUI);
	afx_msg void OnPopupgraphMagnify2();
	afx_msg void OnUpdatePopupgraphMagnify2(CCmdUI* pCmdUI);
	afx_msg void OnPopupgraphMagnify4();
	afx_msg void OnUpdatePopupgraphMagnify4(CCmdUI* pCmdUI);
	afx_msg void OnPopupgraphMagnifycustom();
	afx_msg void OnUpdatePopupgraphMagnifycustom(CCmdUI* pCmdUI);
	afx_msg void OnRestartProcess();
	afx_msg void OnUpdateRestartProcess(CCmdUI* pCmdUI);
	afx_msg void OnEditSelectWaveform();
	afx_msg void OnUpdateEditSelectWaveform(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePlayback(CCmdUI* pCmdUI);
	afx_msg void OnSetupFnkeys();
	afx_msg void OnUpdateSetupFnkeys(CCmdUI* pCmdUI);
	afx_msg void OnEditRemove();
	afx_msg void OnUpdateEditRemove(CCmdUI* pCmdUI);
	afx_msg void OnEditAdd();
	afx_msg void OnUpdateEditAdd(CCmdUI* pCmdUI);
	afx_msg void OnEditAddPhraseL1();
	afx_msg void OnUpdateEditAddPhraseL1(CCmdUI* pCmdUI);
	afx_msg void OnEditAddPhraseL2();
	afx_msg void OnUpdateEditAddPhraseL2(CCmdUI* pCmdUI);
	afx_msg void OnEditAddPhraseL3();
	afx_msg void OnUpdateEditAddPhraseL3(CCmdUI* pCmdUI);
	afx_msg void OnEditAddPhraseL4();
	afx_msg void OnUpdateEditAddPhraseL4(CCmdUI* pCmdUI);
	afx_msg void OnEditUp();
	afx_msg void OnUpdateEditUp(CCmdUI* pCmdUI);
	afx_msg void OnEditDown();
	afx_msg void OnUpdateEditDown(CCmdUI* pCmdUI);
	afx_msg void OnEditPrevious();
	afx_msg void OnUpdateEditPrevious(CCmdUI* pCmdUI);
	afx_msg void OnEditNext();
	afx_msg void OnUpdateEditNext(CCmdUI* pCmdUI);
	afx_msg void OnEditAddSyllable();
	afx_msg void OnUpdateEditAddSyllable(CCmdUI* pCmdUI);
	afx_msg void OnEditAddWord();
	afx_msg void OnUpdateEditAddWord(CCmdUI* pCmdUI);
	afx_msg void OnEditAddBookmark();
	afx_msg void OnUpdateEditAddBookmark(CCmdUI* pCmdUI);
	afx_msg void OnEditCursorStartLeft();
	afx_msg void OnEditCursorStopLeft();
	afx_msg void OnEditCursorStartRight();
	afx_msg void OnEditCursorStopRight();
	afx_msg void OnMoveStopCursorHere();
	afx_msg void OnPlaybackSlow();
	afx_msg void OnDpGrapitch();
	afx_msg void OnUpdateDpGrapitch(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDpRawdata(CCmdUI* pCmdUI);
	afx_msg void OnDpRawdata();
	afx_msg void OnDpSpectrogram();
	afx_msg void OnUpdateDpSpectrogram(CCmdUI* pCmdUI);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnEditInplace();
	afx_msg void OnVerifyOverlays();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSpectroFormants();
	afx_msg void OnUpdateSpectroFormants(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg LRESULT OnUniChar(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnImeComposition(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLayout(UINT nID);
	afx_msg void OnUpdateLayout(CCmdUI* pCmdUI);
	afx_msg void OnGraphTypesSelect(UINT nID);
	afx_msg void OnUpdateGraphTypesSelect(CCmdUI* pCmdUI);
	afx_msg void OnChangeGraph(UINT nID);
	afx_msg void OnUpdateChangeGraph(CCmdUI* pCmdUI);
	afx_msg void OnAnnotationAll(UINT nID);
	afx_msg void OnUpdateAnnotationAll(CCmdUI* pCmdUI);
	afx_msg void OnAnnotationRawdata(UINT nID);
	afx_msg void OnUpdateAnnotationRawdata(CCmdUI* pCmdUI);
	afx_msg void OnAnnotationNone(UINT nID);
	afx_msg void OnUpdateAnnotationNone(CCmdUI* pCmdUI);
	afx_msg void OnPopupgraphAnnotation(UINT nID);
	afx_msg void OnUpdatePopupgraphAnnotation(CCmdUI* pCmdUI);
	afx_msg void OnPlayFKey(UINT nID);
	afx_msg LRESULT OnGraphDestroyed(WPARAM, LPARAM);
	afx_msg LRESULT OnGraphStyleChanged(WPARAM, LPARAM);
	afx_msg LRESULT OnGraphGridChanged(WPARAM, LPARAM);
	afx_msg LRESULT OnGraphColorChanged(WPARAM, LPARAM);
	afx_msg LRESULT OnGraphOrderChanged(WPARAM, LPARAM);
	afx_msg LRESULT OnGraphFontChanged(WPARAM, LPARAM);
	afx_msg LRESULT OnGraphUpdateModeChanged(WPARAM = 0, LPARAM = 0L);
	afx_msg LRESULT OnScrollZoomChanged(WPARAM, LPARAM);
	afx_msg LRESULT OnCursorAlignmentChanged(WPARAM = 0, LPARAM = 0L);
	afx_msg LRESULT OnCursorInFragment(WPARAM CursorSelect, LPARAM FragmentIndex);
	afx_msg LRESULT OnAnimationChanged(WPARAM = 0, LPARAM = 0L);
	afx_msg LRESULT OnRecorder(WPARAM, LPARAM);
	afx_msg LRESULT OnAppMessage(WPARAM hint, LPARAM hintObject);
	afx_msg void OnFilePrint();
	afx_msg void OnUpdateFilePrint(CCmdUI* pCmdUI);
	afx_msg void OnFilePrintPreview();
	afx_msg void OnAddOverlay();
	afx_msg void OnUpdateAddOverlay(CCmdUI* pCmdUI);
	afx_msg void OnRemoveOverlay();
	afx_msg void OnUpdateRemoveOverlay(CCmdUI* pCmdUI);
	afx_msg void OnRemoveOverlays();
	afx_msg void OnEditCopyPhoneticToPhonemic();
	afx_msg void OnUpdateEditCopyPhoneticToPhonemic(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

};


#ifndef _DEBUG  // debug version in saView.cpp
inline CSaDoc* CSaView::GetDocument()
{ return (CSaDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif //_SA_VIEW_H

