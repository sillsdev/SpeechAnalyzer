/////////////////////////////////////////////////////////////////////////////
// mainfrm.h:
// Interface of the CMainFrame class
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//   1.06.5
//       SDM Added support for CDlgEditor including:
//             member m_pDlgEditor
//             OnIdleUpdate()
//   1.06.6U4
//        SDM Added Save/Copy Screen/Graph AsBMP
//        SDM Moved functionality of OnSaveAsBMP to CDib::CaptureWindow & CDib::Save
//        SDM Added GetCurrSaView
//   1.06.6U5
//        SDM Added class CSaMDIChildWnd
//        SDM Added members m_bDefaultMaximizeView, m_nDefaultHeightView, m_nDefaultWidthView
//        SDM Added BOOL IsDefaultViewMaximized()
//        SDM Added CPoint GetDefaultViewSize()
//        SDM Added CopyWindowAsBMP & SaveWindowAsBMP
//   1.5Test8.2
//        SDM added m_wplDlgEditor, m_bToneAbove
//   1.5Test8.5
//        SDM added support for dynamically changing menu, popups, and accelerators
//        SDM added support for display plot
//   1.5Test10.0
//        SDM added m_parseParm & m_segmentParm
//   1.5Test11.1A
//        RLJ Changed m_parseParm to m_parseParmDefaults
//        RLJ Changed m_segmentParm to m_segmentParmDefaults
//        RLJ Added m_pitchParmDefaults, m_spectrumParmDefaults
//        RLJ Added m_spectroAParmDefaults, m_spectroBParmDefaults
/////////////////////////////////////////////////////////////////////////////
#ifndef __MAINFRM_H__
#define __MAINFRM_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "sa_menu.h"
#include "sa_sbar.h"
#include "taskbar.h"
#include "fnkeys.h"
#include "colors.h"
#include "appdefs.h"
#include "saParm.h"
#include "sourceParm.h"
#include "CSaString.h"

class CSaFindDlg;
class CSegment;
class CSaDoc;
class CSaView;
class CDlgEditor;
class CDisplayPlot;
class CDlgToolsOptions;
class CDlgPlayer;
class CSaWorkbenchView;
class CGraphWnd;
class CDlgAutoRecorder;
class CDataProcess;
struct SSpecific;
class CFontTable;

typedef UINT (CALLBACK *DLGHOOKPROC)(HWND, UINT, WPARAM, LPARAM);

typedef struct {
	CSaView       *pView;
	BOOL           bMaximize;
	int            nHeight;
	int            nWidth;
	ParseParm      ParsePrm;
	SegmentParm    SegmentPrm;
	PitchParm      PitchPrm;
	FormantParm    FormantPrm;
	SpectrumParm   SpectrumPrm;
	SpectroParm    SpectrogramPrm;
	SpectroParm    SnapshotPrm;
} DefaultViewInfo;

//###########################################################################
// CMainFrame window

#define CMainFrameBase CMDIFrameWnd

class CMainFrame : public CMainFrameBase
{
	DECLARE_DYNCREATE(CMainFrame)

	// Construction/destruction/creation
public:
	CMainFrame();
	virtual ~CMainFrame();

	// Attributes

protected:
	// options settings

	//***********************************************************
	// DDO - 08/07/00
	//***********************************************************
	CSaString m_szPermDefaultGraphs;
	CSaString m_szTempDefaultGraphs;
	UINT m_nPermDefaultLayout;
	UINT m_nTempDefaultLayout;

	//***********************************************************
	// DDO - 08/03/00 Don't need this setting anymore.
	//BOOL              m_bSaveOnExit;         // if TRUE, save all SA settings on Exit.
	//***********************************************************
	BOOL				m_bShowStartupDlg;        // True if start dialog box gets shown after splash screen.
	BOOL				m_nStartDataMode;         // 0 = Phonetic, 1 = Music  DDO - 08/08/00
	BOOL				m_bSaveOpenFiles;         // if True, save current open files to re-load them next time   //tdg 09/03/97
	BOOL				m_bStatusBar;             // status bar on/off
	BOOL				m_bShowAdvancedAudio;
	int					m_nStatusPosReadout;      // status bar position readout mode
	int					m_nStatusPitchReadout;    // status bar pitch readout mode  
	BOOL				m_bToneAbove;             // tone Above phonetic
	BOOL				m_bScrollZoom;            // scrollbar zoom on/off
	int					m_nCaptionStyle;          // graph caption style
	Colors				m_colors;                 // color settings
	FnKeys				m_fnKeys;                 // function keys setup
	Grid				m_grid;                   // gridline settings
	int					m_nGraphUpdateMode;       // 0 = static (when cursor set), 1 = dynamic (while cursor is moving)
	BOOL				m_bAnimate;               // TRUE = animation requested
	int					m_nAnimationRate;         // frame rate for animations (in frames/sec)
	CURSOR_ALIGNMENT	m_nCursorAlignment;       // cursor snap mode: align to sample, zero crossing, or fragment

	BOOL				m_bDefaultViewExists;    // True if a default view configuration was read from the .psa file.
	void WriteReadDefaultViewToTempFile(BOOL bWrite);

	// Default values for various parameters                     // RLJ 11.1A
	CSaView *			m_pDefaultViewConfig;   // holds the default view for when a doc is newly opened.
	BOOL				m_bDefaultMaximizeView;  // maximize view if no other view open
	int					m_nDefaultHeightView;    // default view height
	int					m_nDefaultWidthView;     // default view width
	ParseParm			m_parseParmDefaults;    // parsing
	SegmentParm			m_segmentParmDefaults;  // segmenting
	PitchParm			m_pitchParmDefaults;    // pitch
	MusicParm			m_musicParmDefaults;    // music
	IntensityParm		m_intensityParmDefaults;// intensity
	FormantParm			m_formantParmDefaults;  // formant
	SpectrumParm		m_spectrumParmDefaults; // spectrum
	SpectroParm			m_spectrogramParmDefaults; // spectrogram
	SpectroParm			m_snapshotParmDefaults; // snapshot

	// other members
	BOOL				m_bMenuModified;       // menu already modified
	int					m_nNumberOfViews;      // number of MDI child windows (views)
	CLayoutMenu			m_LayoutMenu;          // layout menu embedded object
	CDataStatusBar		m_dataStatusBar;       // status control bar embedded object for data
	CProgressStatusBar	m_progressStatusBar;   // status control bar embedded object for progress
	CToolBar			m_wndToolBarBasic;
	CToolBar			m_wndToolBarAdvanced;
	CTaskBar			m_wndTaskBar;
	CSaWorkbenchView*	m_pWorkbenchView;      // workbench view
	BOOL				m_bFindOnly;           // find/replace is only find

	// workbench processes and filter IDs
	CDataProcess*		m_apWbProcess[MAX_PROCESS_NUMBER][MAX_FILTER_NUMBER];
	int					m_aWbFilterID[MAX_PROCESS_NUMBER][MAX_FILTER_NUMBER];

	// dialogs

	CDlgPlayer*			m_pDlgPlayer;          // pointer to player dialog object
	CSaFindDlg*			m_pDlgFind;            // pointer to find/replace dialog object
	CDlgEditor*			m_pDlgEditor;
	WINDOWPLACEMENT		m_wplDlgEditor;
	CDlgToolsOptions*	m_pDlgToolsOptions;    // pointer to options property sheet object
	BOOL				m_bIsPrinting;
	BOOL				m_bPrintPreviewInProgress;
	CDisplayPlot*		m_pDisplayPlot;
	HMENU				m_hNewMenu;
	HACCEL				m_hNewAccel;
	int					m_nPopup;
	HMENU				m_hExpMenu;
	CString				m_szExpMenuTitle;

	// Operations
public:
	BOOL					bToolBar() { return m_wndToolBarAdvanced.IsVisible() ;};               // toolbar on/off
	BOOL					bTaskBar() { return m_wndTaskBar.IsVisible() ;};               // taskbar on/off
	const CSaString			GetPermGraphNames(void) {return m_szPermDefaultGraphs;}   // DDO - 08/07/00
	const CSaString			GetTempGraphNames(void) {return m_szTempDefaultGraphs;}   // DDO - 08/07/00
	const UINT				GetPermLayout(void) {return m_nPermDefaultLayout;}    // DDO - 08/07/00
	const UINT				GetTempLayout(void) {return m_nTempDefaultLayout;}    // DDO - 08/07/00
	BOOL					GetShowStartupDlg(void) {return m_bShowStartupDlg;}       // DDO - 08/03/00
	int						GetStartDataMode(void) {return m_nStartDataMode;}        // DDO - 08/08/00
	BOOL					GetSaveOpenFiles(void) {return m_bSaveOpenFiles; }       // tdg - 09/03/97
	BOOL					GetShowAdvancedAudio(void) {return m_bShowAdvancedAudio;}
	void					SetShowStartupDlg(BOOL bShow) {m_bShowStartupDlg = bShow;}      // DDO - 08/03/00
	void					SetStartDataMode(int nMode) {m_nStartDataMode = nMode;}       // DDO - 08/08/00
	CDlgPlayer *			GetPlayer(BOOL bCreate = FALSE);

	// kg 32-bit conversion
	void DestroyPlayer();
	void DestroySynthesizer();

	CSegment * GetAnnotation(int annotSetIndex);
	CSaDoc * GetCurrDoc(void);
	CSaView * GetCurrSaView(void);

	// Reference to objects containing parsing, segmenting, pitch, spectrum, and spectrogram parameter defaults // RLJ 11.1A
	ParseParm*     GetParseParm()       {return &m_parseParmDefaults;}
	SegmentParm*   GetSegmentParm()     {return &m_segmentParmDefaults;}
	const PitchParm* GetPitchParmDefaults() const {return &m_pitchParmDefaults;}
	void SetPitchParmDefaults(const PitchParm& cParm) {m_pitchParmDefaults = cParm;}
	const MusicParm* GetMusicParmDefaults() const {return &m_musicParmDefaults;}
	void SetMusicParmDefaults(const MusicParm& cParm) {m_musicParmDefaults = cParm;}
	const IntensityParm& GetIntensityParmDefaults() const {return m_intensityParmDefaults;}
	void SetIntensityParmDefaults(const IntensityParm& cParm) {m_intensityParmDefaults = cParm;}
	FormantParm*   GetFormantParmDefaults()     {return &m_formantParmDefaults;}
	SpectrumParm*  GetSpectrumParmDefaults()    {return &m_spectrumParmDefaults;}
	const SpectroParm*   GetSpectrogramParmDefaults() const {return &m_spectrogramParmDefaults;}
	void SetSpectrogramParmDefaults(const SpectroParm& cParm)    {m_spectrogramParmDefaults = cParm;}
	const SpectroParm*   GetSnapshotParmDefaults() const {return &m_snapshotParmDefaults;}
	void SetSnapshotParmDefaults(const SpectroParm& cParm)    {m_snapshotParmDefaults = cParm;}
	const CSaView *pDefaultViewConfig()         {return m_pDefaultViewConfig;}
	BOOL           DefaultIsValid()             {return m_pDefaultViewConfig != NULL;}
	BOOL           IsDefaultViewMaximized()     {return m_bDefaultMaximizeView;}
	CPoint         GetDefaultViewSize()         {return CPoint(m_nDefaultWidthView, m_nDefaultHeightView);}
	void           OnSetDefaultGraphs(BOOL bToDisk);
	void           OnSetDefaultParameters();

	void WriteProperties(Object_ostream& obs);
	BOOL ReadProperties(Object_istream& obs);
	BOOL bReadDefaultView(Object_istream& obs);
	void WriteDefaultView(Object_ostream& obs);
	void DisplayPlot(CDisplayPlot* pPlot) {m_pDisplayPlot = pPlot;}
	void NotifyFragmentDone(void *pCaller);

	// transcription fonts
	CSaStringArray* m_pGraphFontFaces;  // array of graph font face strings
	CUIntArray* m_pGraphFontSizes;// array of graph font sizes

	CSaString GetFontFace(int nIndex) {return m_pGraphFontFaces->GetAt(nIndex);} // return font face string
	int	GetFontSize(int nIndex) {return (int)m_pGraphFontSizes->GetAt(nIndex);} // return font size
	void SetFontFace(int nIndex, LPCTSTR pString) {if (_tcslen(pString)) m_pGraphFontFaces->SetAtGrow(nIndex, pString);} // set font face string
	void SetFontSize(int nIndex, int nSize)      {if (nSize) m_pGraphFontSizes->SetAtGrow(nIndex, (UINT)nSize);} // set font size

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	// helper functions
	//CWnd* IsSMrunning();    // checks, if SM is running  //Check no longer necessary, taken care of through file ownership  -ALB
	void ShowDataStatusBar(BOOL bShow); // show or hide data status bar
	CDataStatusBar* GetDataStatusBar() {return &m_dataStatusBar;} // return pointer to status bar object
	CProgressStatusBar* GetProgressStatusBar() {return &m_progressStatusBar;} // return pointer to progress status bar object
	int ComputeNumberOfViews(int nNum) {m_nNumberOfViews += nNum; return m_nNumberOfViews;}
	int GetCaptionStyle() {return m_nCaptionStyle;} // return graph caption style
	Colors* GetColors() {return &m_colors;} // return pointer to color structure
	FnKeys* GetFnKeys(FnKeys*); // return a copy of the function keys structure
	void SetFnKeys(FnKeys*); // set the function keys structure
	LRESULT OnPlayer(WPARAM, LPARAM, SSpecific*);
	Grid* GetGrid() {return &m_grid;} // return pointer to grid structure
	BOOL IsStatusBar() {return m_bStatusBar;} // return visibility of statusbar
	BOOL IsPlayerPlaying(); // return TRUE if player is playing
	BOOL IsPlayerTestRun(); // return TRUE if player is running function key test run
	void SetPlayerTimes();  // set player dialogue time in LED indicators
	BOOL IsScrollZoom() {return m_bScrollZoom;} // return TRUE if scroll zoom enabled
	int  GetStatusPosReadout() {return m_nStatusPosReadout;} // return status bar position readout mode
	int  GetStatusPitchReadout() {return m_nStatusPitchReadout;} // return status bar pitch readout mode
	int  GetGraphUpdateMode() {return m_nGraphUpdateMode;}
	void SetGraphUpdateMode(int nMode) {m_nGraphUpdateMode = nMode;}
	BOOL IsAnimationRequested() {return m_bAnimate;}
	int GetAnimationFrameRate() {return m_nAnimationRate;}
	CURSOR_ALIGNMENT GetCursorAlignment() {return m_nCursorAlignment;}   // get cursor snap mode
	void SetupFunctionKeys();
	void SendMessageToMDIDescendants(UINT message, WPARAM wParam, LPARAM lParam); // send message to all mdi children
	void AppMessage(WPARAM wParam, LPARAM lParam)
	{
		this->SendMessage(WM_USER_APP_MESSAGE,wParam, lParam);
		SendMessageToMDIDescendants(WM_USER_APP_MESSAGE, wParam, lParam);
	};
	void  SetPreviewFlag()    { m_bPrintPreviewInProgress = TRUE; };
	void  ClearPreviewFlag()  { m_bPrintPreviewInProgress = FALSE; };
	void  SetPrintingFlag();
	void  ClearPrintingFlag();
	void  CreateFindOrReplaceDlg();
	void  MaybeCreateFindOrReplaceDlg(BOOL bWantFindOnly);
	// workbench helper functions
	CDataProcess* GetWbProcess(int nProcess, int nFilter) {return m_apWbProcess[nProcess][nFilter];} // return pointer to workbench process
	CDataProcess* SetWbProcess(int nProcess, int nFilter, CDataProcess* pProcess); // set new workbench process pointer and return old one
	int GetWbFilterID(int nProcess, int nFilter) {return m_aWbFilterID[nProcess][nFilter];} // return filter ID
	int SetWbFilterID(int nProcess, int nFilter, int nID); // set new workbench filter ID
	CDocument* IsProcessUsed(int nProcess); // return pointer to document, if process used by it, else NULL
	void DeleteWbProcesses(BOOL bSwitchBack = TRUE); // deletes all workbench processes and switches all document processes back to plain
	// overlay graph helper functions
	void SwapInOverlayColors(int index);
	void SwapOutOverlayColors();

	void SetNewMenu(const HMENU hMenu) { m_hNewMenu = hMenu;}
	HMENU GetNewMenu() const { return m_hNewMenu; }
	UINT GetVisibleMenuItemCount(CMenu* pMenu);
	void SetNewAccel(const HACCEL hAccel) { m_hNewAccel = hAccel;}
	HACCEL GetNewAccel() const { return m_hNewAccel; }
	void SetPopup(int nPopup) { m_nPopup = nPopup; };
	int GetPopup() const { return m_nPopup ? m_nPopup : IDR_SA_FLOATINGPOPUP; };
	BOOL IsEditAllowed();

	friend CDisplayPlot;

	// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnToolsOptions();
	afx_msg void OnToolsSelfTest();
	afx_msg void OnEditFind();
	afx_msg void OnUpdateEditFind(CCmdUI* pCmdUI);
	afx_msg void OnEditReplace();
	afx_msg void OnUpdateEditReplace(CCmdUI* pCmdUI);
	afx_msg void OnClose();
	afx_msg void OnSysColorChange();
	afx_msg void OnEditor();
	afx_msg LRESULT OnIdleUpdate(WPARAM, LPARAM);
	afx_msg void OnUpdateEditEditor(CCmdUI* pCmdUI);
	afx_msg void OnSaveScreenAsBMP();
	afx_msg void OnCopyScreenAsBMP();
	afx_msg void OnSaveWindowAsBMP();
	afx_msg void OnCopyWindowAsBMP();
	afx_msg void OnSaveGraphsAsBMP();
	afx_msg void OnCopyGraphsAsBMP();
	afx_msg void OnUpdateGraphsAsBMP(CCmdUI* pCmdUI);
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	afx_msg void OnWindowTileHorz();
	afx_msg void OnWindowTileVert();
	afx_msg void OnWaveformGenerator();
	afx_msg void OnUpdateToolsOptions(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnSynthesis();
	afx_msg void OnUpdateSynthesis(CCmdUI* pCmdUI);
	afx_msg void OnSynthesisKlattIpa();
	afx_msg void OnSynthesisKlattFragments();
	afx_msg void OnSynthesisVocalTract();
	afx_msg void OnRecordOverlay();
	afx_msg void OnUpdateRecordOverlay(CCmdUI* pCmdUI);
	afx_msg LRESULT OnApplyToolsOptions(WPARAM, LPARAM);
	afx_msg LRESULT OnPlayer(WPARAM, LPARAM);
	afx_msg LRESULT OnChangeView(WPARAM, LPARAM);
	afx_msg LRESULT OnSpeechAppCall(WPARAM, LPARAM);
	// status bar update
	afx_msg void OnUpdateDataPane(CCmdUI *pCmdUI);
	afx_msg void OnUpdateProgressPane(CCmdUI *pCmdUI);
	afx_msg void OnEqualizeLength();
	afx_msg void OnUpdateEqualizeLength(CCmdUI *pCmdUI);
	DECLARE_MESSAGE_MAP()

};

// ChildFrm.h : interface of the CChildFrame class
//
/////////////////////////////////////////////////////////////////////////////

class CChildFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CChildFrame)
public:
	CChildFrame();

	// Attributes
public:

	// Operations
public:

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildFrame)
	virtual void ActivateFrame(int nCmdShow = -1) ;
	//}}AFX_VIRTUAL

	// Implementation
public:
	virtual ~CChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CChildFrame)
	// NOTE - the ClassWizard will add and remove member functions here.
	//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // __MAINFRM_H__
