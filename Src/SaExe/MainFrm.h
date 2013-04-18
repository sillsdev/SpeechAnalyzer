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
#ifndef MAINFRM_H__
#define MAINFRM_H__

#include "sa_menu.h"
#include "sa_sbar.h"
#include "taskbar.h"
#include "fnkeys.h"
#include "colors.h"
#include "appdefs.h"
#include "saParm.h"
#include "sourceParm.h"
#include "SaString.h"
#include "WaveformGeneratorSettings.h"
#include "ToolSettings.h"
#include "SpectroParm.h"

class CDlgFind;
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
class CProcess;
struct SSpecific;
class CFontTable;

typedef UINT(CALLBACK * DLGHOOKPROC)(HWND, UINT, WPARAM, LPARAM);

typedef struct SDefaultViewInfo
{
    CSaView    *   pView;
    BOOL           bMaximize;
    int            nHeight;
    int            nWidth;
    CParseParm      ParsePrm;
    CSegmentParm    SegmentPrm;
    CPitchParm      PitchPrm;
    CFormantParm    FormantPrm;
    CSpectrumParm   SpectrumPrm;
    CSpectroParm    SpectrogramPrm;
    CSpectroParm    SnapshotPrm;
} DefaultViewInfo;

//###########################################################################
// CMainFrame window

class CMainFrame : public CMDIFrameWnd
{

    DECLARE_DYNCREATE(CMainFrame)

public:
    CMainFrame();
    virtual ~CMainFrame();

protected:

    //***********************************************************
    // DDO - 08/07/00
    //***********************************************************
    CSaString m_szPermDefaultGraphs;
    CSaString m_szTempDefaultGraphs;
    UINT m_nPermDefaultLayout;
    UINT m_nTempDefaultLayout;

    //***********************************************************
    // DDO - 08/03/00 Don't need this setting anymore.
    //BOOL              m_bSaveOnExit;            // if TRUE, save all SA settings on Exit.
    //***********************************************************
    BOOL                m_bShowStartupDlg;        // True if start dialog box gets shown after splash screen.
    BOOL                m_nStartDataMode;         // 0 = Phonetic, 1 = Music  DDO - 08/08/00
    BOOL                m_bSaveOpenFiles;         // if True, save current open files to re-load them next time   //tdg 09/03/97
    BOOL                m_bStatusBar;             // status bar on/off
    BOOL                m_bShowAdvancedAudio;
    int                 m_nStatusPosReadout;      // status bar position readout mode
    int                 m_nStatusPitchReadout;    // status bar pitch readout mode
    BOOL                m_bToneAbove;             // tone Above phonetic
    BOOL                m_bScrollZoom;            // scrollbar zoom on/off
    int                 m_nCaptionStyle;          // graph caption style
    Colors              m_colors;                 // color settings
    CFnKeys             m_fnKeys;                 // function keys setup
    CGrid                m_grid;                   // gridline settings
    int                 m_nGraphUpdateMode;       // 0 = static (when cursor set), 1 = dynamic (while cursor is moving)
    BOOL                m_bAnimate;               // TRUE = animation requested
    int                 m_nAnimationRate;         // frame rate for animations (in frames/sec)
    ECursorAlignment    m_nCursorAlignment;       // cursor snap mode: align to sample, zero crossing, or fragment
    BOOL                m_bDefaultViewExists;    // True if a default view configuration was read from the .psa file.
    CWaveformGeneratorSettings m_waveformGeneratorSettings;
    void WriteReadDefaultViewToTempFile(BOOL bWrite);

    // Default values for various parameters                     // RLJ 11.1A
    CSaView      *      m_pDefaultViewConfig;   // holds the default view for when a doc is newly opened.
    BOOL                m_bDefaultMaximizeView;  // maximize view if no other view open
    int                 m_nDefaultHeightView;    // default view height
    int                 m_nDefaultWidthView;     // default view width
    CParseParm           m_parseParmDefaults;    // parsing
    CSegmentParm         m_segmentParmDefaults;  // segmenting
    CPitchParm           m_pitchParmDefaults;    // pitch
    CMusicParm           m_musicParmDefaults;    // music
    CIntensityParm       m_intensityParmDefaults;// intensity
    CFormantParm         m_formantParmDefaults;  // formant
    CSpectrumParm        m_spectrumParmDefaults; // spectrum
    CSpectroParm         m_spectrogramParmDefaults; // spectrogram
    CSpectroParm         m_snapshotParmDefaults; // snapshot

    // other members
    BOOL                m_bMenuModified;       // menu already modified
    int                 m_nNumberOfViews;      // number of MDI child windows (views)
    CLayoutMenu         m_LayoutMenu;          // layout menu embedded object
    CDataStatusBar      m_dataStatusBar;       // status control bar embedded object for data
    CProgressStatusBar  m_progressStatusBar;   // status control bar embedded object for progress
    CToolBar            m_wndToolBarBasic;
    CToolBar            m_wndToolBarAdvanced;
    CTaskBar            m_wndTaskBar;
    CSaWorkbenchView  * m_pWorkbenchView;      // workbench view
    BOOL                m_bFindOnly;           // find/replace is only find

    // workbench processes and filter IDs
    CProcess    *   m_apWbProcess[MAX_PROCESS_NUMBER][MAX_FILTER_NUMBER];
    int                 m_aWbFilterID[MAX_PROCESS_NUMBER][MAX_FILTER_NUMBER];

    // dialogs
    CDlgPlayer * m_pDlgPlayer;          // pointer to player dialog object
    CDlgFind * m_pDlgFind;            // pointer to find/replace dialog object
    CDlgEditor * m_pDlgEditor;
    WINDOWPLACEMENT m_wplDlgEditor;
    BOOL m_bIsPrinting;
    BOOL m_bPrintPreviewInProgress;
    CDisplayPlot * m_pDisplayPlot;
    HMENU m_hNewMenu;
    HACCEL m_hNewAccel;
    int m_nPopup;
    CToolSettings toolSettings;

public:
    BOOL bToolBar();               // toolbar on/off
    BOOL bTaskBar();               // taskbar on/off
    const CSaString GetPermGraphNames(void);
    const CSaString GetTempGraphNames(void);
    const UINT GetPermLayout(void);
    const UINT GetTempLayout(void);
    BOOL GetShowStartupDlg(void);
    int GetStartDataMode(void);
    BOOL GetSaveOpenFiles(void);
    BOOL GetShowAdvancedAudio(void);
    void SetShowStartupDlg(BOOL bShow);
    void SetStartDataMode(int nMode);
    CDlgPlayer * GetPlayer(bool bCreate = false);

    // kg 32-bit conversion
    void DestroyPlayer();
    void DestroySynthesizer();

    CSegment * GetAnnotation(int annotSetIndex);
    CSaDoc * GetCurrDoc(void);
    CSaView * GetCurrSaView(void);

    // Reference to objects containing parsing, segmenting, pitch, spectrum, and spectrogram parameter defaults // RLJ 11.1A
    CParseParm * GetCParseParm();
    CSegmentParm * GetSegmentParm();
    const CPitchParm * GetPitchParmDefaults() const;
    void SetPitchParmDefaults(const CPitchParm & cParm);
    const CMusicParm * GetMusicParmDefaults() const;
    void SetMusicParmDefaults(const CMusicParm & cParm);
    const CIntensityParm & GetCIntensityParmDefaults() const;
    void SetCIntensityParmDefaults(const CIntensityParm & cParm);
    CFormantParm * GetFormantParmDefaults();
    CSpectrumParm * GetSpectrumParmDefaults();
    const CSpectroParm * GetSpectrogramParmDefaults() const;
    void SetSpectrogramParmDefaults(const CSpectroParm & cParm);
    const CSpectroParm * GetSnapshotParmDefaults() const;
    void SetSnapshotParmDefaults(const CSpectroParm & cParm);
    const CSaView * pDefaultViewConfig();
    BOOL DefaultIsValid();
    BOOL IsDefaultViewMaximized();
    CPoint GetDefaultViewSize();
    void OnSetDefaultGraphs(BOOL bToDisk);
    void OnSetDefaultParameters();

    void WriteProperties(CObjectOStream & obs);
    BOOL ReadProperties(CObjectIStream & obs);
    BOOL ReadDefaultView(CObjectIStream & obs);
    void WriteDefaultView(CObjectOStream & obs);
    void DisplayPlot(CDisplayPlot * pPlot);
    void NotifyFragmentDone(void * pCaller);

    // transcription fonts
    CSaStringArray m_GraphFontFaces;        // array of graph font face strings
    CUIntArray m_GraphFontSizes;            // array of graph font sizes

    CSaString GetFontFace(int nIndex);
    int GetFontSize(int nIndex);
    void SetFontFace(int nIndex, LPCTSTR pString);
    void SetFontSize(int nIndex, int nSize);

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext & dc) const;
#endif
    // helper functions
    void ShowDataStatusBar(BOOL bShow); // show or hide data status bar
    CDataStatusBar * GetDataStatusBar();
    CProgressStatusBar * GetProgressStatusBar();
    int ComputeNumberOfViews(int nNum);
    int GetCaptionStyle();
    Colors * GetColors();
    CFnKeys * GetFnKeys(CFnKeys *); // return a copy of the function keys structure
    void SetFnKeys(CFnKeys *); // set the function keys structure
    LRESULT OnPlayer(WPARAM, LPARAM, SSpecific *);
    CGrid * GetGrid();
    BOOL IsStatusBar();
    BOOL IsPlayerPlaying();		// return TRUE if player is playing
    BOOL IsPlayerPaused();		// return TRUE if player is paused
    BOOL IsPlayerTestRun();		// return TRUE if player is running function key test run
    void SetPlayerTimes();		// set player dialogue time in LED indicators
    BOOL IsScrollZoom();
    int  GetStatusPosReadout();
    int  GetStatusPitchReadout();
    int  GetGraphUpdateMode();
    void SetGraphUpdateMode(int nMode);
    BOOL IsAnimationRequested();
    int GetAnimationFrameRate();
    ECursorAlignment GetCursorAlignment();
    void SetupFunctionKeys();
    void SendMessageToMDIDescendants(UINT message, WPARAM wParam, LPARAM lParam); // send message to all mdi children
    void AppMessage(WPARAM wParam, LPARAM lParam);
    void  SetPreviewFlag();
    void  ClearPreviewFlag();
    void  SetPrintingFlag();
    void  ClearPrintingFlag();
    void  CreateFindOrReplaceDlg();
    void  MaybeCreateFindOrReplaceDlg(BOOL bWantFindOnly);
    // workbench helper functions
    CProcess * GetWbProcess(int nProcess, int nFilter);
    CProcess * SetWbProcess(int nProcess, int nFilter, CProcess * pProcess); // set new workbench process pointer and return old one
    int GetWbFilterID(int nProcess, int nFilter);
    int SetWbFilterID(int nProcess, int nFilter, int nID); // set new workbench filter ID
    CDocument * IsProcessUsed(int nProcess); // return pointer to document, if process used by it, else NULL
    void DeleteWbProcesses(BOOL bSwitchBack = TRUE); // deletes all workbench processes and switches all document processes back to plain
    // overlay graph helper functions
    void SwapInOverlayColors(int index);
    void SwapOutOverlayColors();

    void SetNewMenu(const HMENU hMenu);
    HMENU GetNewMenu() const;
    UINT GetVisibleMenuItemCount(CMenu * pMenu);
    void SetNewAccel(const HACCEL hAccel);
    HACCEL GetNewAccel() const;
    void SetPopup(int nPopup);
    int GetPopup() const;
    BOOL IsEditAllowed();
    void SetToolSettings(CToolSettings settings);

    friend CDisplayPlot;

    // Generated message map functions
protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnInitMenu(CMenu * pMenu);
    afx_msg void OnToolsOptions();
    afx_msg void OnToolsSelfTest();
    afx_msg void OnEditFind();
    afx_msg void OnUpdateEditFind(CCmdUI * pCmdUI);
    afx_msg void OnEditReplace();
    afx_msg void OnUpdateEditReplace(CCmdUI * pCmdUI);
    afx_msg void OnClose();
    afx_msg void OnSysColorChange();
    afx_msg void OnEditor();
    afx_msg LRESULT OnIdleUpdate(WPARAM, LPARAM);
    afx_msg void OnUpdateEditEditor(CCmdUI * pCmdUI);
    afx_msg void OnSaveScreenAsBMP();
    afx_msg void OnCopyScreenAsBMP();
    afx_msg void OnSaveWindowAsBMP();
    afx_msg void OnCopyWindowAsBMP();
    afx_msg void OnSaveGraphsAsBMP();
    afx_msg void OnCopyGraphsAsBMP();
    afx_msg void OnUpdateGraphsAsBMP(CCmdUI * pCmdUI);
    afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
    afx_msg void OnWindowTileHorz();
    afx_msg void OnWindowTileVert();
    afx_msg void OnWaveformGenerator();
    afx_msg void OnUpdateToolsOptions(CCmdUI * pCmdUI);
    afx_msg void OnSynthesis();
    afx_msg void OnUpdateSynthesis(CCmdUI * pCmdUI);
    afx_msg void OnSynthesisKlattIpa();
    afx_msg void OnSynthesisKlattFragments();
    afx_msg void OnSynthesisVocalTract();
    afx_msg void OnRecordOverlay();
    afx_msg void OnUpdateRecordOverlay(CCmdUI * pCmdUI);
    afx_msg LRESULT OnApplyToolsOptions(WPARAM, LPARAM);
    afx_msg LRESULT OnPlayer(WPARAM, LPARAM);
    afx_msg LRESULT OnChangeView(WPARAM, LPARAM);
    afx_msg LRESULT OnSpeechAppCall(WPARAM, LPARAM);
    afx_msg void OnUpdateDataPane(CCmdUI * pCmdUI);
    afx_msg void OnUpdateProgressPane(CCmdUI * pCmdUI);
    afx_msg void OnEqualizeLength();
    afx_msg void OnUpdateEqualizeLength(CCmdUI * pCmdUI);
    afx_msg BOOL OnCopyData(CWnd * pWnd, COPYDATASTRUCT * pCopyDataStruct);
    afx_msg void OnTimer(UINT nIDEvent);

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnAutoSaveOn();
    afx_msg void OnAutoSaveOff();

private:
	BOOL m_bAutoSave;

};

#endif
