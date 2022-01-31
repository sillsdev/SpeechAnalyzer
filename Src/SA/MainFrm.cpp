/////////////////////////////////////////////////////////////////////////////
// mainfrm.cpp:
// Implementation of the CMainFrame class.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//   1.06.5
//        SDM Added support for CDlgEditor
//   1.06.6U2
//        SDM Added Add segment and Insert Word toolbar buttons
//   1.06.6U4
//        SDM Added Save/Copy Screen/Graph AsBMP
//        SDM Moved functionality of OnSaveAsBMP to CDib::CaptureWindow & CDib::Save
//        SDM Added GetCurrSaView
//   1.06.6U5
//        SDM Added Class CSaMDIChildWindow
//        SDM Added parameters to settings DefaultView Maximized/Height/Width
//        SDM Added CopyWindowAsBMP & SaveWindowAsBMP
//        SDM Changed GraphAsBMP to refer to client area
//   1.06.8
//        SDM Changed IsSMRunning to check ALL windows
//   1.5Test8.1
//        SDM added support for Reference annotation
//   1.5Test8.2
//        SDM added support for ToneAbove check box
//        SDM added support for saving TE position (m_wplDlgEditor)
//   1.5Test8.5
//        SDM disabled context sensitive help
//        SDM added support for dynamically changing menu, popups, and accelerators
//        SDM added support for display plot
//   1.5Test10.0
//        SDM added m_parseParm & m_segmentParm
//        SDM disabled memory page unless WIN32
//   1.5Test10.3
//        SDM set window order for tiling
//   1.5Test11.1A
//        RLJ Added m_parseParmDefaults.   (Use in addition to SaDoc->m_parseParm.)
//        RLJ Added m_segmentParmDefaults. (Use in addition to SaDoc->m_segmentParm.)
//        RLJ Added m_pitchParmDefaults, m_spectrumParmDefaults,
//                  m_spectrogramParmDefaults, m_snapshotParmDefaults
//   06/13/2000
//        RLJ Disabled Toolbar's "?" button (Help Index), since this
//            feature has not yet been implemented.
//   06/22/2000
//        RLJ Moved some "Global Help Commands" to CSaApp
//
//   08/07/00
//        DDO Completed the work on being able to save settings both
//            permanently and temporarily. Hopefully, the process has
//            been simplified on the dialog box.
//        DDO Fixed a problem with the current caption style on the Tools/
//            Options/View dialog not necessarily matching the current
//            caption style. The dialog's member var. wasn't being
//            initialized before showing it modally.
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "mainfrm.h"
#include "sa_graph.h"
#include "DlgToolsOptions.h"
#include "sa_wbch.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "doclist.h"
#include "math.h"
#include "DlgFind.h"
#include "sa_cdib.h"
#include "targview.h"
#include "sa_dplot.h"
#include "DlgAutoRecorder.h"
#include "DlgWaveformGenerator.h"
#include "SelfTest.h"
#include "AlignInfo.h"
#include "FileUtils.h"
#include "DlgPlayer.h"
#include "Process\Process.h"
#include "Process\sa_p_spu.h"
#include "Process\sa_p_fmt.h"
#include "Process\sa_p_spg.h"
#include "objectostream.h"
#include "objectistream.h"
#include "DlgSynthesis.h"
#include "DlgKlattAll.h"
#include "DlgVocalTract.h"

using std::ifstream;
using std::ofstream;
using std::ios;

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CMainFrame, CMDIFrameWnd)

//###########################################################################
// CMainFrame

/////////////////////////////////////////////////////////////////////////////
// CMainFrame static members definition

static TCHAR szFINDMSGSTRING[] = FINDMSGSTRING;
static const UINT NEAR nMsgFindReplace = ::RegisterWindowMessage(szFINDMSGSTRING);

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message map

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
    ON_WM_COPYDATA()
    ON_WM_CREATE()
    ON_WM_INITMENU()
    ON_COMMAND(ID_TOOLS_OPTIONS, OnToolsOptions)
    ON_COMMAND(ID_TOOLS_SELFTEST, OnToolsSelfTest)
    ON_COMMAND(ID_EDIT_FIND, OnEditFind)
    ON_UPDATE_COMMAND_UI(ID_EDIT_FIND, OnUpdateEditFind)
    ON_COMMAND(ID_EDIT_FIND_NEXT, OnEditFindNext)
    ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_NEXT, OnUpdateEditFindNext)
    ON_COMMAND(ID_EDIT_REPLACE, OnEditReplace)
    ON_UPDATE_COMMAND_UI(ID_EDIT_REPLACE, OnUpdateEditReplace)
    ON_WM_CLOSE()
    ON_WM_SYSCOLORCHANGE()
    ON_COMMAND(ID_EDIT_EDITOR, OnEditor)
    ON_MESSAGE(WM_USER_IDLE_UPDATE, OnIdleUpdate)
    ON_UPDATE_COMMAND_UI(ID_EDIT_EDITOR, OnUpdateEditEditor)
    ON_COMMAND(ID_GRAPHS_SCREENCAPTURE, OnSaveScreenAsBMP)
    ON_COMMAND(ID_GRAPHS_SCREENCOPY, OnCopyScreenAsBMP)
    ON_COMMAND(ID_GRAPHS_WINDOWSAVE, OnSaveWindowAsBMP)
    ON_COMMAND(ID_GRAPHS_WINDOWCOPY, OnCopyWindowAsBMP)
    ON_COMMAND(ID_GRAPHS_GRAPHSCAPTURE, OnSaveGraphsAsBMP)
    ON_COMMAND(ID_GRAPHS_GRAPHSCOPY, OnCopyGraphsAsBMP)
    ON_UPDATE_COMMAND_UI(ID_GRAPHS_GRAPHSCAPTURE, OnUpdateGraphsAsBMP)
    ON_WM_ACTIVATEAPP()
    ON_COMMAND(ID_WINDOW_TILE_HORZ, OnWindowTileHorz)
    ON_COMMAND(ID_WINDOW_TILE_VERT, OnWindowTileVert)
    ON_COMMAND(ID_WAVEFORM_GENERATOR, OnWaveformGenerator)
    ON_UPDATE_COMMAND_UI(ID_TOOLS_OPTIONS, OnUpdateToolsOptions)
    ON_UPDATE_COMMAND_UI(ID_GRAPHS_GRAPHSCOPY, OnUpdateGraphsAsBMP)
    ON_COMMAND(ID_SYNTHESIS, OnSynthesis)
    ON_UPDATE_COMMAND_UI(ID_SYNTHESIS, OnUpdateSynthesis)
    ON_COMMAND(ID_KLATT_IPA_BLEND, OnSynthesisKlattIpa)
    ON_UPDATE_COMMAND_UI(ID_KLATT_IPA_BLEND, OnUpdateSynthesis)
    ON_COMMAND(ID_KLATT_FRAGMENTS, OnSynthesisKlattFragments)
    ON_UPDATE_COMMAND_UI(ID_KLATT_FRAGMENTS, OnUpdateSynthesis)
    ON_COMMAND(ID_SYNTHESIS_VTRACT, OnSynthesisVocalTract)
    ON_UPDATE_COMMAND_UI(ID_SYNTHESIS_VTRACT, OnUpdateSynthesis)
    ON_COMMAND_EX(IDR_BAR_BASIC, OnBarCheck)
    ON_UPDATE_COMMAND_UI(IDR_BAR_BASIC, OnUpdateControlBarMenu)
    ON_COMMAND_EX(IDR_BAR_ADVANCED, OnBarCheck)
    ON_UPDATE_COMMAND_UI(IDR_BAR_ADVANCED, OnUpdateControlBarMenu)
    ON_COMMAND_EX(ID_VIEW_TASKBAR, OnBarCheck)
    ON_UPDATE_COMMAND_UI(ID_VIEW_TASKBAR, OnUpdateControlBarMenu)
    // Global help commands
    ON_COMMAND(ID_CONTEXT_HELP, CMDIFrameWnd::OnHelpIndex) // SDM 1.5Test8.5 disable context sensitive help
    // Custom messages
    ON_MESSAGE(WM_USER_APPLY_TOOLSOPTIONS, OnApplyToolsOptions)
    ON_MESSAGE(WM_USER_UPDATE_PLAYER, OnUpdatePlayer)
    ON_MESSAGE(WM_USER_PLAYER, OnPlayer)
    ON_MESSAGE(WM_USER_CHANGEVIEW, OnChangeView)
    ON_MESSAGE(WM_USER_SPEECHAPPLICATION, OnSpeechAppCall)
    ON_COMMAND(ID_GRAPHS_EQUALIZELENGTH, OnEqualizeLength)
    ON_UPDATE_COMMAND_UI(ID_GRAPHS_EQUALIZELENGTH, OnUpdateEqualizeLength)
    // status bar update
    ON_UPDATE_COMMAND_UI(ID_STATUSPANE_SAMPLES, OnUpdateDataPane)
    ON_UPDATE_COMMAND_UI(ID_STATUSPANE_FORMAT, OnUpdateDataPane)
    ON_UPDATE_COMMAND_UI(ID_STATUSPANE_CHANNELS, OnUpdateDataPane)
    ON_UPDATE_COMMAND_UI(ID_STATUSPANE_SIZE, OnUpdateDataPane)
    ON_UPDATE_COMMAND_UI(ID_STATUSPANE_TLENGTH, OnUpdateDataPane)
    ON_UPDATE_COMMAND_UI(ID_STATUSPANE_TYPE, OnUpdateDataPane)
    ON_UPDATE_COMMAND_UI(ID_STATUSPANE_BITRATE, OnUpdateDataPane)
    ON_UPDATE_COMMAND_UI(ID_STATUSPANE_EMPTY, OnUpdateDataPane)
    ON_UPDATE_COMMAND_UI(ID_STATUSPANE_1, OnUpdateDataPane)
    ON_UPDATE_COMMAND_UI(ID_STATUSPANE_2, OnUpdateDataPane)
    ON_UPDATE_COMMAND_UI(ID_STATUSPANE_3, OnUpdateDataPane)
    ON_UPDATE_COMMAND_UI(ID_STATUSPANE_4, OnUpdateDataPane)
    ON_UPDATE_COMMAND_UI(ID_PROGRESSPANE_1, OnUpdateProgressPane)
    ON_UPDATE_COMMAND_UI(ID_PROGRESSPANE_2, OnUpdateProgressPane)
    ON_UPDATE_COMMAND_UI(ID_PROGRESSPANE_3, OnUpdateProgressPane)
    ON_COMMAND(ID_POPUP_RECORD_OVERLAY, OnRecordOverlay)
    ON_UPDATE_COMMAND_UI(ID_POPUP_RECORD_OVERLAY, OnUpdateRecordOverlay)
    ON_WM_TIMER()
    ON_COMMAND(ID_AUTOSAVE_ON, OnAutoSaveOn)
    ON_COMMAND(ID_AUTOSAVE_OFF, OnAutoSaveOff)
END_MESSAGE_MAP()

static UINT BASED_CODE dataIndicators[] = {
    ID_SEPARATOR,           // data status line indicator
    ID_SEPARATOR,
    ID_SEPARATOR,
    ID_SEPARATOR,
    ID_SEPARATOR,
    ID_SEPARATOR,
    ID_SEPARATOR,
    ID_SEPARATOR,
    ID_SEPARATOR,
    ID_SEPARATOR,
    ID_SEPARATOR,
    ID_SEPARATOR,
};

static UINT BASED_CODE progressIndicators[] = {
    ID_SEPARATOR,           // progress status line indicator
    ID_SEPARATOR,
    ID_SEPARATOR,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction/creation

/***************************************************************************/
// CMainFrame::CMainFrame Constructor
/***************************************************************************/
CMainFrame::CMainFrame() {

    // options default settings
    m_bStatusBar = TRUE;               // status bar enabled at startup
    m_nStatusPosReadout = TIME;        // position readout mode is time
    m_nStatusPitchReadout = HERTZ;     // pitch readout mode is hertz
    m_bToneAbove = FALSE;              // tone below phonetic by default SDM 1.5Test8.2
    m_bScrollZoom = TRUE;              // scrollbar zooming enabled at startup
    m_nCaptionStyle = MiniWithCaption; // graph caption style is minicaption with caption text

    // init the gridlines
    m_grid.Init();

    // other settings
    m_bMenuModified = FALSE;          // menu not modified at startup
    m_nNumberOfViews = 0;             // no views at startup

    // define modeless dialog and other object pointers
    m_pWorkbenchView = NULL;          // workbench
    m_pDlgFind = NULL;                // find/replace
    m_pDlgPlayer = NULL;              // player

    //SDM 1.06.5
    m_pDlgEditor = NULL;              // editor
    m_bIsPrinting = FALSE;
    m_pAutoRecorder = NULL;

    // reset workbench processes and filter IDs
    for (int nLoop = 0; nLoop < MAX_PROCESS_NUMBER; nLoop++) {
        for (int nFilterLoop = 0; nFilterLoop < MAX_FILTER_NUMBER; nFilterLoop++) {
            m_apWbProcess[nLoop][nFilterLoop] = NULL;
            m_aWbFilterID[nLoop][nFilterLoop] = 0;
        }
    }

    m_szPermDefaultGraphs = "";  // DDO - 08/08/00
    m_szTempDefaultGraphs = "";  // DDO - 08/08/00
    m_nPermDefaultLayout = 0;    // DDO - 08/07/00
    m_nTempDefaultLayout = 0;    // DDO - 08/07/00
    m_nStartDataMode = 0;        // DDO - 08/08/00
    m_bShowStartupDlg = TRUE;    // DDO - 08/03/00
    m_bSaveOpenFiles = FALSE;    // tdg - 09/03/97
    m_bShowAdvancedAudio = FALSE;
	m_nAlgorithm = 0;

    // init the graph fonts with default
    try {
        if (!m_GraphFontFaces.GetSize()) {
            m_GraphFontFaces.Add(PHONETIC_DEFAULT_FONT);
            m_GraphFontFaces.Add(TONE_DEFAULT_FONT);
            m_GraphFontFaces.Add(PHONEMIC_DEFAULT_FONT);
            m_GraphFontFaces.Add(ORTHOGRAPHIC_DEFAULT_FONT);
            m_GraphFontFaces.Add(GLOSS_DEFAULT_FONT);
            m_GraphFontFaces.Add(GLOSS_NAT_DEFAULT_FONT);
            m_GraphFontFaces.Add(REFERENCE_DEFAULT_FONT);
            m_GraphFontFaces.Add(MUSIC_PHRASE_DEFAULT_FONT);
            m_GraphFontFaces.Add(MUSIC_PHRASE_DEFAULT_FONT);
            m_GraphFontFaces.Add(MUSIC_PHRASE_DEFAULT_FONT);
            m_GraphFontFaces.Add(MUSIC_PHRASE_DEFAULT_FONT);

            m_GraphFontSizes.Add(PHONETIC_DEFAULT_FONTSIZE);
            m_GraphFontSizes.Add(TONE_DEFAULT_FONTSIZE);
            m_GraphFontSizes.Add(PHONEMIC_DEFAULT_FONTSIZE);
            m_GraphFontSizes.Add(ORTHOGRAPHIC_DEFAULT_FONTSIZE);
            m_GraphFontSizes.Add(GLOSS_DEFAULT_FONTSIZE);
            m_GraphFontSizes.Add(GLOSS_NAT_DEFAULT_FONTSIZE);
            m_GraphFontSizes.Add(REFERENCE_DEFAULT_FONTSIZE);
            m_GraphFontSizes.Add(MUSIC_PHRASE_DEFAULT_FONTSIZE);
            m_GraphFontSizes.Add(MUSIC_PHRASE_DEFAULT_FONTSIZE);
            m_GraphFontSizes.Add(MUSIC_PHRASE_DEFAULT_FONTSIZE);
            m_GraphFontSizes.Add(MUSIC_PHRASE_DEFAULT_FONTSIZE);
        } else {
            m_GraphFontFaces.SetAt(PHONETIC, PHONETIC_DEFAULT_FONT);
            m_GraphFontFaces.SetAt(TONE, TONE_DEFAULT_FONT);
            m_GraphFontFaces.SetAt(PHONEMIC, PHONEMIC_DEFAULT_FONT);
            m_GraphFontFaces.SetAt(ORTHO, ORTHOGRAPHIC_DEFAULT_FONT);
            m_GraphFontFaces.SetAt(GLOSS, GLOSS_DEFAULT_FONT);
            m_GraphFontFaces.SetAt(REFERENCE, REFERENCE_DEFAULT_FONT);
            m_GraphFontFaces.SetAt(MUSIC_PL1, MUSIC_PHRASE_DEFAULT_FONT);
            m_GraphFontFaces.SetAt(MUSIC_PL2, MUSIC_PHRASE_DEFAULT_FONT);
            m_GraphFontFaces.SetAt(MUSIC_PL3, MUSIC_PHRASE_DEFAULT_FONT);
            m_GraphFontFaces.SetAt(MUSIC_PL4, MUSIC_PHRASE_DEFAULT_FONT);

            m_GraphFontSizes.SetAt(PHONETIC, PHONETIC_DEFAULT_FONTSIZE);
            m_GraphFontSizes.SetAt(TONE, TONE_DEFAULT_FONTSIZE);
            m_GraphFontSizes.SetAt(PHONEMIC, PHONEMIC_DEFAULT_FONTSIZE);
            m_GraphFontSizes.SetAt(ORTHO, ORTHOGRAPHIC_DEFAULT_FONTSIZE);
            m_GraphFontSizes.SetAt(GLOSS, GLOSS_DEFAULT_FONTSIZE);
            m_GraphFontSizes.SetAt(REFERENCE, REFERENCE_DEFAULT_FONTSIZE);
            m_GraphFontSizes.SetAt(MUSIC_PL1, MUSIC_PHRASE_DEFAULT_FONTSIZE);
            m_GraphFontSizes.SetAt(MUSIC_PL2, MUSIC_PHRASE_DEFAULT_FONTSIZE);
            m_GraphFontSizes.SetAt(MUSIC_PL3, MUSIC_PHRASE_DEFAULT_FONTSIZE);
            m_GraphFontSizes.SetAt(MUSIC_PL4, MUSIC_PHRASE_DEFAULT_FONTSIZE);
        }
    } catch (CMemoryException * e) {
        // handle memory fail exception
        CSaApp * pApp = (CSaApp *)AfxGetApp();
        pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
        e->Delete();
		return;
    }

    m_bPrintPreviewInProgress = FALSE;
    m_wplDlgEditor.length = 0;  // SDM 1.5Test8.2
    m_pDisplayPlot = NULL;      // SDM 1.5Test8.5
    m_pAutoRecorder = NULL;
    m_hNewMenu = NULL;          // SDM 1.5Test8.5
    m_hNewAccel = NULL;         // SDM 1.5Test8.5
    m_nPopup = 0;               // SDM 1.5Test8.5

    // Initialize parsing, segmenting, pitch, spectrum and spectrogram parameter defaults // RLJ 11.1A
    m_parseParmDefaults.Init();
    m_segmentParmDefaults.Init();
    m_pitchParmDefaults.Init();
    m_musicParmDefaults.Init();
    m_formantParmDefaults.Init();
    m_spectrumParmDefaults.Init();
    m_spectrogramParmDefaults.Init();
    m_snapshotParmDefaults.Init();
    m_pDefaultViewConfig = NULL;

    // SDM 1.06.6U5
    m_bDefaultMaximizeView = TRUE;
    m_nDefaultHeightView = 0;
    m_nDefaultWidthView = 0;
	// DDO - 08/07/00
    m_bDefaultViewExists = false;

    m_nGraphUpdateMode = STATIC_UPDATE;
    m_bAnimate = FALSE;
    m_nAnimationRate = MAX_ANIMATION_RATE;
    m_nCursorAlignment = ALIGN_AT_FRAGMENT;
    m_bAutoSave = TRUE;

	activeBreakWidth = m_parseParmDefaults.fBreakWidth;
}

/***************************************************************************/
// CMainFrame::~CMainFrame Destructor
/***************************************************************************/
CMainFrame::~CMainFrame() {
    
	// delete modeless dialog and other objects
    if (m_pWorkbenchView) {
		// workbench
        delete m_pWorkbenchView; 
        m_pWorkbenchView = NULL;
    }
    if (m_pDlgFind) {
        delete m_pDlgFind;
        m_pDlgFind = NULL;
    }
    if (m_pDlgPlayer) {
        delete m_pDlgPlayer;
        m_pDlgPlayer = NULL;
    }
    if (m_pDlgEditor!=NULL) {
		// SDM 1.06.5 editor
        delete m_pDlgEditor; 
        m_pDlgEditor = NULL;
    }
    if (m_pDefaultViewConfig!=NULL) {
        delete m_pDefaultViewConfig;
        m_pDefaultViewConfig = NULL;
    }
    if (CDlgAutoRecorder::IsLaunched()) {
        m_pAutoRecorder->SendMessage(WM_CLOSE);
        m_pAutoRecorder = NULL;
    }

    DestroySynthesizer();
    CDlgKlattAll::DestroySynthesizer();
    CDlgVocalTract::DestroySynthesizer();

    // delete workbench processes
    for (int nLoop = 0; nLoop < MAX_PROCESS_NUMBER; nLoop++) {
        for (int nFilterLoop = 0; nFilterLoop < MAX_FILTER_NUMBER; nFilterLoop++)
            if (m_apWbProcess[nLoop][nFilterLoop]) {
                delete m_apWbProcess[nLoop][nFilterLoop];
            }
    }

    if (m_pDisplayPlot!=NULL) {
        // SDM 1.5Test8.5
        delete m_pDisplayPlot;
        m_pDisplayPlot = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame helper functions

/***************************************************************************/
// CMainFrame::SwapInOverlayColors
/***************************************************************************/
void CMainFrame::SwapInOverlayColors(int index) {
    // save away for restore later on.
    m_colors.cBackupColor   = m_colors.cPlotData[0];

    // bring in the overlay colors
    m_colors.cPlotData[0]   = m_colors.cPlotData[index];
}


/***************************************************************************/
// CMainFrame::SwapOutOverlayColors
/***************************************************************************/
void CMainFrame::SwapOutOverlayColors() {
    m_colors.cPlotData[0]   = m_colors.cBackupColor;
}

/***************************************************************************/
// CMainFrame::GetVisibleMenuItemCount
/***************************************************************************/
UINT CMainFrame::GetVisibleMenuItemCount(CMenu * pMenu) {
    UINT count = 0;
    UINT maxCount = pMenu->GetMenuItemCount();
    CSaString rString;
    for (UINT i = 1; i <= maxCount; i++) {
        pMenu->GetMenuString(i, rString, MF_BYPOSITION);
        if (rString.IsEmpty()) {
            break;
        }
        count++;
    }

    return count;
}

/***************************************************************************/
// CMainFrame::ShowDataStatusBar Show or hide data status bar
/***************************************************************************/
void CMainFrame::ShowDataStatusBar(BOOL bShow) {
	// status bar is on
    if (m_bStatusBar) { 
        if (bShow) {
            // show data status bar, hide process status bar
            m_progressStatusBar.ShowWindow(SW_HIDE);
            m_dataStatusBar.ShowWindow(SW_SHOW);
        } else {
            // show process status bar, hide data status bar
            m_dataStatusBar.ShowWindow(SW_HIDE);
            // move progress status bar over data status bar
            CRect rWnd;
            m_dataStatusBar.GetWindowRect(rWnd);
            ScreenToClient(rWnd);
            m_progressStatusBar.MoveWindow(rWnd);
            m_progressStatusBar.InitProgress();
            if (!m_bIsPrinting) {
                m_progressStatusBar.ShowWindow(SW_SHOW);
                m_progressStatusBar.Invalidate();
                m_progressStatusBar.UpdateWindow();
            }
        }
    }
}

/***************************************************************************/
// CMainFrame::GetFnKeys Returns a copy of the function keys structure
// The function returns a pointer to the function key structure. If the
// parameter contains a valid pointer, it will also copy the function key
// setting to this pointer.
/***************************************************************************/
CFnKeys * CMainFrame::GetFnKeys(CFnKeys * pfnKeys) {
    if (pfnKeys) {
        *pfnKeys = m_fnKeys;
    }
    return &m_fnKeys;
}

/***************************************************************************/
// CMainFrame::SetFnKeys Sets the function keys structure
/***************************************************************************/
void CMainFrame::SetFnKeys(CFnKeys * pfnKeys) {
    m_fnKeys = *pfnKeys;
}

/***************************************************************************/
// CMainFrame::IsPlayerPlaying Returns TRUE if player is playing
/***************************************************************************/
BOOL CMainFrame::IsPlayerPlaying() {
    if (CDlgPlayer::IsLaunched()) {         // player launched
        return GetPlayer(false)->IsPlaying();   // return TRUE if player is playing
    } else {
        return FALSE;
    }
}

/***************************************************************************/
// CMainFrame::IsPlayerPlaying Returns TRUE if player is playing
/***************************************************************************/
BOOL CMainFrame::IsPlayerPaused() {
    if (CDlgPlayer::IsLaunched()) {         // player launched
        return GetPlayer(false)->IsPaused();    // return TRUE if player is paused
    } else {
        return FALSE;
    }
}

/***************************************************************************/
// CMainFrame::IsPlayerTestRun Returns TRUE if player runs function key test
/***************************************************************************/
BOOL CMainFrame::IsPlayerTestRun() {
	// player launched
    if (CDlgPlayer::IsLaunched()) {
		// return TRUE if player runs Fn test
        return GetPlayer(false)->IsTestRun();
    } else {
        return FALSE;
    }
}

/***************************************************************************/
// CMainFrame::SetPlayerTimes  sets time indicators for layer
/***************************************************************************/
void CMainFrame::SetPlayerTimes() {

	// if player dialogue launched
    if (CDlgPlayer::IsLaunched()) {                                 
        CRect rWnd;
		// set the start time
        GetPlayer(false)->SetPositionTime();
		// get start LED indicator
        CWnd * pWnd = GetPlayer(false)->GetDlgItem(IDC_POSITIONTIME); 
		// get coordinates
        pWnd->GetWindowRect(rWnd);
        pWnd->RedrawWindow(rWnd);
		// set the stop time
        GetPlayer(false)->SetTotalTime();
		// get stop LED indicator
        pWnd = GetPlayer(false)->GetDlgItem(IDC_TOTALTIME);
		// get coordinates
        pWnd->GetWindowRect(rWnd);
		// update
        pWnd->RedrawWindow(rWnd);
    }
}

/***************************************************************************/
// CMainFrame::SetupFunctionKeys Start setup Fn-keys dialog in player
/***************************************************************************/
void CMainFrame::SetupFunctionKeys() {

    if (GetPlayer(false)) {
        GetPlayer(false)->SendMessage(WM_USER_SETUP_FNKEYS, 0, 0L);
    }
}

#ifdef _DEBUG
void CMainFrame::AssertValid() const {
    CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext & dc) const {
    CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/***************************************************************************/
// CMainFrame::OnCreate Window creation
// Called by framework to initially create the one and only mainframe window.
// Creates the toolbar and statusbar. Returns FALSE on error.
/***************************************************************************/
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) {

    if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }

    // create toolbars
    if (!m_wndToolBarBasic.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
                                    | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
                                    CRect(0,0,0,0), IDR_BAR_BASIC) ||
            (!m_wndToolBarBasic.LoadToolBar(IDR_BAR_BASIC))) {
        TRACE(_T("Failed to create toolbar\n"));
		// failed to create
        return -1; 
    }

    if (!m_wndToolBarAdvanced.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
                                       | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
                                       CRect(0,0,0,0), IDR_BAR_ADVANCED) ||
            (!m_wndToolBarAdvanced.LoadToolBar(IDR_BAR_ADVANCED))) {
        TRACE(_T("Failed to create toolbar\n"));
		// failed to create
        return -1; 
    }

    // create data statusbar
    if ((!m_dataStatusBar.Create(this)) ||
        (!m_dataStatusBar.SetIndicators(dataIndicators, sizeof(dataIndicators)/sizeof(UINT)))) {
        TRACE(_T("Failed to create data status bar\n"));
		// failed to create
        return -1; 
    }

    // initialize data statusbar
    m_dataStatusBar.Init();

    // create progress statusbar
    if ((!m_progressStatusBar.Create(this)) ||
        (!m_progressStatusBar.SetIndicators(progressIndicators, sizeof(progressIndicators)/sizeof(UINT)))) {
        TRACE(_T("Failed to create progress status bar\n"));
		// failed to create
        return -1; 
    }

    // initialize progress statusbar
    m_progressStatusBar.ShowWindow(SW_HIDE);
    m_progressStatusBar.Init();

    m_wndToolBarBasic.EnableDocking(CBRS_ALIGN_ANY);
    m_wndToolBarAdvanced.EnableDocking(CBRS_ALIGN_ANY);

	// Creates CDockBar objects
    EnableDocking(CBRS_ALIGN_ANY);

    DockControlBar(&m_wndToolBarBasic, AFX_IDW_DOCKBAR_TOP);
    DockControlBar(&m_wndToolBarAdvanced, AFX_IDW_DOCKBAR_TOP);
	ShowControlBar(&m_wndToolBarBasic, FALSE, FALSE);
    ShowControlBar(&m_wndToolBarAdvanced, TRUE, FALSE);

    // Create Task Bar last this affects its position Z-Order and therefore layout behavior
    // Last in the Z-Order is preferrable for the task bar
    if (!m_wndTaskBar.Create(this, IDD_TASKBAR, WS_CHILD | WS_VISIBLE | CBRS_ALIGN_LEFT | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED, ID_VIEW_TASKBAR)) {
        TRACE(_T("Failed to create data task bar\n"));
        return -1; // failed to create
    }

    ShowControlBar( &m_wndTaskBar, TRUE, FALSE);
	m_wndTaskBar.Setup();

    // init the colors
    m_colors.SetupColors( Colors::PASTEL);
    // setup function keys default
    m_fnKeys.SetupDefaultKeys();

    return 0;
}

/***************************************************************************/
// CMainFrame::OnInitMenu Menu creation
// Called by framework to initially create the mainframe menu. Attaches
// the graph layout menu onto the mainframe menu (if not already done).
/***************************************************************************/
void CMainFrame::OnInitMenu(CMenu * pMenu) {
    CMDIFrameWnd::OnInitMenu(pMenu);
    UINT nMenuCount = GetVisibleMenuItemCount(pMenu);
    if ((nMenuCount > 6) &&         // standalone menu loaded
            (pMenu == GetMenu())) { // don't modify floating popup menu
        if (!m_bMenuModified) { // attach layout menu on standalone menu
            TCHAR szString[256];    // don't change the string
            pMenu->GetMenuString(ID_GRAPHS_LAYOUT, szString, sizeof(szString)/sizeof(TCHAR), MF_BYCOMMAND);
            if (szString[0] != 0) {
                VERIFY(pMenu->ModifyMenu(ID_GRAPHS_LAYOUT, MF_BYCOMMAND | MF_POPUP, (UINT)m_LayoutMenu.m_hMenu, szString));
                m_bMenuModified = TRUE; // modification done
            }
        }
    }

    // setup the autosave checks
    if (m_bAutoSave) {
        UINT state = pMenu->GetMenuState(ID_AUTOSAVE_ON,MF_BYCOMMAND);
        if ((m_bAutoSave)&&((state&MF_CHECKED)!=MF_CHECKED)) {
            pMenu->CheckMenuItem(ID_AUTOSAVE_ON, MF_CHECKED);
            pMenu->CheckMenuItem(ID_AUTOSAVE_OFF, MF_UNCHECKED);
        }
    } else {
        UINT state = pMenu->GetMenuState(ID_AUTOSAVE_OFF,MF_BYCOMMAND);
        if ((!m_bAutoSave)&&((state&MF_CHECKED)!=MF_CHECKED)) {
            pMenu->CheckMenuItem(ID_AUTOSAVE_ON, MF_UNCHECKED);
            pMenu->CheckMenuItem(ID_AUTOSAVE_OFF, MF_CHECKED);
        }
    }
}

/***************************************************************************/
// CMainFrame::OnToolsOptions Opens options dialog
// The user wants to see the options dialog. Creates the property sheet, sets
// up some initial values and sends a apply message, if the user hits the OK
// button.
/***************************************************************************/
void CMainFrame::OnToolsOptions() {

    CSaApp * pApp = (CSaApp *)AfxGetApp();

    if (GetCurrSaView() != NULL) {

        // set property sheet caption
        CSaString szCaption;
        szCaption.LoadString(IDS_DLGTITLE_TOOLSOPTIO); // load caption string

        // create property sheet object
        CDlgToolsOptions dlg(szCaption, NULL, true);

        // get pointer to active view and document
        CSaView * pView = (CSaView *)GetCurrSaView();

        // setup initial dialog values
        dlg.m_dlgViewPage.m_nCaptionStyle = m_nCaptionStyle; // DDO - 08/07/00
        dlg.m_dlgViewPage.m_bStatusbar = m_bStatusBar;       // setup check boxes
        dlg.m_dlgViewPage.m_nPosMode = m_nStatusPosReadout;
        dlg.m_dlgViewPage.m_nPitchMode = m_nStatusPitchReadout;
        dlg.m_dlgViewPage.m_bToolbar = AdvancedToolBarVisible();
        dlg.m_dlgViewPage.m_bTaskbar = TaskBarVisible();
        dlg.m_dlgViewPage.m_bToneAbove = m_bToneAbove;
        dlg.m_dlgViewPage.m_bScrollZoom = m_bScrollZoom;
        dlg.m_dlgViewPage.m_nCursorAlignment = pView->GetCursorAlignment();
        dlg.m_dlgViewPage.m_nGraphUpdateMode = pView->GetGraphUpdateMode();
        dlg.m_dlgViewPage.m_bAnimate = pView->IsAnimationRequested();
        dlg.m_dlgViewPage.m_nAnimationRate = pView->GetAnimationFrameRate();
        dlg.m_dlgViewPage.m_bXGrid = m_grid.bXGrid;
        dlg.m_dlgViewPage.m_bYGrid = m_grid.bYGrid;
        dlg.m_dlgViewPage.SetGridStyle(m_grid.nXStyle, m_grid.nYStyle);
        dlg.m_dlgColorPage.m_cColors = m_colors;

        try {
            for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
                dlg.m_dlgFontPage.m_GraphFonts.Add(GetFontFace(nLoop));
                dlg.m_dlgFontPage.m_GraphFontSizes.Add(GetFontSize(nLoop));
            }
        } catch (CMemoryException * e) {
            // memory allocation error
            pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
            e->Delete();
			return;
        }

        // create the modal dialog box
		// OK button pressed
        if (dlg.DoModal() == IDOK) {							
            SetToolSettings(dlg.GetSettings(true),true);
			// do apply changes
            SendMessage(WM_USER_APPLY_TOOLSOPTIONS, 0, 0);		

        }
    } else {
        // there is no view - show a limited version of the tools options dialog box
        // set property sheet caption
        CSaString szCaption;
		// load caption string
        szCaption.LoadString(IDS_DLGTITLE_TOOLSOPTIO); 
        // create property sheet object
        CDlgToolsOptions dlg(szCaption, NULL, false);

        // create the modal dialog box
		// OK button pressed
        if (dlg.DoModal() == IDOK) {							
            SetToolSettings(dlg.GetSettings(false), false);
			// do apply changes
            SendMessage(WM_USER_APPLY_TOOLSOPTIONS, 0, 0);		
        }
    }
}

void CMainFrame::OnUpdateToolsOptions(CCmdUI * pCmdUI) {
	pCmdUI->Enable(TRUE);
}

/***************************************************************************/
// CMainFrame::OnToolsSelfTest Starts CSelfTest
/***************************************************************************/
void CMainFrame::OnToolsSelfTest() {
    CSelfTestRunner test;
    test.Do();
}

/***************************************************************************/
// CMainFrame::SendMessageToMDIDescendants  Send message to all mdi children
// Just calling SendMessageToDescendants caused a crash in 32-bit version
// apparently because either the status bar or toolbar windows (descendants
// of CMainFrame) choked on a WM_USER_? message sent to them.
/***************************************************************************/
void CMainFrame::SendMessageToMDIDescendants(UINT message, WPARAM wParam, LPARAM lParam) {
    CDocList doclst; // list of currently open docs
    for (CSaDoc * pdoc = doclst.pdocFirst(); pdoc; pdoc = doclst.pdocNext()) {
        POSITION pos = pdoc->GetFirstViewPosition();
        while (pos) {
            // ramble thru all views for this document
            CSaView * pview = (CSaView *)pdoc->GetNextView(pos); // increments pos
            pview->SendMessage(message, wParam, lParam);
            // send to child windows after parent
            pview->SendMessageToDescendants(message, wParam, lParam, TRUE, TRUE);
        }
    }
}

/***************************************************************************/
// CMainFrame::OnApplyToolsOptions Applies the changes in options dialog
// The user wants to apply the changes from options dialog. Applies all the
// changes and sends the necessary messages.
/***************************************************************************/
LRESULT CMainFrame::OnApplyToolsOptions(WPARAM, LPARAM) {

    // apply to statusbar
    if (toolSettings.m_bStatusbar != m_bStatusBar) {
        SendMessage(WM_COMMAND, ID_VIEW_STATUS_BAR, 0); // change statusbar status
        m_bStatusBar = !m_bStatusBar;
    }

    if (m_nStatusPosReadout != toolSettings.m_nPosMode) {
        m_nStatusPosReadout = toolSettings.m_nPosMode;
        CSaView * pView = GetCurrSaView();
        CGraphWnd * pGraph = pView ? pView->GetFocusedGraphWnd() : NULL;
        if (pGraph!=NULL) {
            pGraph->UpdateStatusBar(pView->GetStartCursorPosition(), pView->GetStopCursorPosition(), TRUE);    // update the status bar
        }
    }

    m_nStatusPitchReadout = toolSettings.m_nPitchMode;
    // apply to toolbar
    if (toolSettings.m_bToolbar != AdvancedToolBarVisible()) {
        BOOL bAdvanced = toolSettings.m_bToolbar;
		// change toolbar status
        ShowControlBar(GetControlBar(IDR_BAR_BASIC), !bAdvanced, FALSE);
        ShowControlBar(GetControlBar(IDR_BAR_ADVANCED), bAdvanced, FALSE);	
    }
    // apply to taskbar
    if (toolSettings.m_bTaskbar != TaskBarVisible()) {
        BOOL bTaskbar = toolSettings.m_bTaskbar;
        ShowControlBar(GetControlBar(ID_VIEW_TASKBAR),bTaskbar, FALSE); // change taskbar status
    }

    // apply tone position
    if (toolSettings.m_bToneAbove != m_bToneAbove) {
        m_bToneAbove = !m_bToneAbove;
        if (m_bToneAbove) {
            CGraphWnd::m_anAnnWndOrder[1] = TONE;
            CGraphWnd::m_anAnnWndOrder[2] = PHONETIC;
        } else {
            CGraphWnd::m_anAnnWndOrder[1] = PHONETIC;
            CGraphWnd::m_anAnnWndOrder[2] = TONE;
        }
        SendMessageToMDIDescendants(WM_USER_GRAPH_ORDERCHANGED, 0, 0L);
    }
    // apply to scrolling zoom
    if (toolSettings.m_bScrollZoom != m_bScrollZoom) {
        m_bScrollZoom = !m_bScrollZoom;
        // tell about the change to all views
        SendMessageToMDIDescendants(WM_USER_VIEW_SCROLLZOOMCHANGED, m_bScrollZoom, 0L);
    }

    // update cursor alignment for the active view
    ECursorAlignment nCursorAlignmentSetting = (ECursorAlignment) toolSettings.m_nCursorAlignment;
    CSaView * pView = (CSaView *)GetCurrSaView();
    if ((pView!=NULL) && (nCursorAlignmentSetting != pView->GetCursorAlignment())) {
        pView->ChangeCursorAlignment(nCursorAlignmentSetting);
    }

    // This becomes the new default setting
    m_nCursorAlignment = nCursorAlignmentSetting;

    // update graph mode
    int nGraphUpdateModeSetting = toolSettings.m_nGraphUpdateMode;
    if (nGraphUpdateModeSetting != GetGraphUpdateMode()) {
        SetGraphUpdateMode(nGraphUpdateModeSetting);
        SendMessageToMDIDescendants(WM_USER_VIEW_GRAPHUPDATECHANGED, 0L, 0L);
    }
    // update animation
    int bAnimateSetting = toolSettings.m_bAnimate;
    int nAnimationRateSetting = toolSettings.m_nAnimationRate;
    if ((bAnimateSetting != IsAnimationRequested()) ||
            (nAnimationRateSetting != GetAnimationFrameRate())) {
        m_bAnimate = bAnimateSetting;
        m_nAnimationRate = nAnimationRateSetting;
        SendMessageToMDIDescendants(WM_USER_VIEW_ANIMATIONCHANGED, 0, 0);
    }
    // apply graph caption style
    if (toolSettings.m_nCaptionStyle != m_nCaptionStyle) {
        if (AfxMessageBox(IDS_QUESTION_RECREATEGRAPHS, MB_YESNO | MB_ICONQUESTION,0) == IDYES) {
            m_nCaptionStyle = toolSettings.m_nCaptionStyle;
            // tell about the change to all views
            SendMessageToMDIDescendants(WM_USER_GRAPH_STYLECHANGED, 0, 0L);
        }
    }

    if ((toolSettings.m_bXGrid != m_grid.bXGrid) ||
        (toolSettings.m_bYGrid != m_grid.bYGrid) ||
        (toolSettings.m_nDlgXStyle != m_grid.nXStyle) ||
        (toolSettings.m_nDlgYStyle != m_grid.nYStyle)) {
        m_grid.bXGrid = toolSettings.m_bXGrid;
        m_grid.bYGrid = toolSettings.m_bYGrid;
        m_grid.nXStyle = toolSettings.m_nDlgXStyle;
        m_grid.nYStyle = toolSettings.m_nDlgYStyle;
        // tell about the change to all views
        SendMessageToMDIDescendants(WM_USER_GRAPH_GRIDCHANGED, 0, 0L);
    }

    // apply graph colors
    if (toolSettings.m_bColorsChanged) {
        toolSettings.m_bColorsChanged = false;
        m_colors = toolSettings.m_cColors;
        // tell about the change to all views
        SendMessageToMDIDescendants(WM_USER_GRAPH_COLORCHANGED, 0, 0L);
		m_wndTaskBar.Invalidate(TRUE);
    }

    // apply graph fonts
    if (toolSettings.m_bFontChanged) {
        // get pointer to active document
        CSaDoc * pDoc = (CSaDoc *)GetCurrSaView()->GetDocument();
        toolSettings.m_bFontChanged = FALSE;
        for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
            SetFontFace(nLoop, toolSettings.m_GraphFonts.GetAt(nLoop));
            SetFontSize(nLoop, toolSettings.m_GraphFontSizes.GetAt(nLoop));
        }
        pDoc->CreateFonts();
        // tell about the change to all views
        SendMessageToMDIDescendants(WM_USER_GRAPH_FONTCHANGED, 0, 0L);
    }
    CSaApp::m_bUseUnicodeEncoding = toolSettings.m_bUseUnicodeEncoding;

    //***********************************************************
    // Save the values from the startup tab
    //***********************************************************
    m_bShowStartupDlg = toolSettings.m_showStartupDlg;
    m_bSaveOpenFiles = toolSettings.m_saveOpenFiles;
    m_bShowAdvancedAudio = toolSettings.m_bShowAdvancedAudio;
	m_nAlgorithm = toolSettings.m_nAlgorithm;
    return 0;
}

CDlgPlayer * CMainFrame::GetPlayer(bool bCreate) {

    if (!CDlgPlayer::IsLaunched()) {
        // player dialog not launched
        if (!bCreate) {
            return NULL;
        }
        if (m_pDlgPlayer!=NULL) {
			// delete old dialog object
            delete m_pDlgPlayer;
            m_pDlgPlayer = NULL;
        }
		// create new player object
        m_pDlgPlayer = new CDlgPlayer();
        if (!CDlgPlayer::IsLaunched()) { 
			// player dialog not launched
			// create player
            m_pDlgPlayer->Create();     
        }
    }
    return m_pDlgPlayer;
}

/***************************************************************************/
// CMainFrame::OnPlayer Launches the player
// The user wants to see the modeless player dialog. If not already done
// it creates the player dialog, otherwise it just sets the player active
// and on top of all other windows. lParam delivers the player mode to be
// set.
/***************************************************************************/
LRESULT CMainFrame::OnPlayer(WPARAM wParam, LPARAM lParam) {
    
	CDlgPlayer::EMode mode = (CDlgPlayer::EMode)wParam;
    TRACE("OnPlayer %s %x %x\n", CDlgPlayer::GetMode(mode), HIWORD(lParam), LOWORD(lParam));

	// retrieve pointer to active window
    CWnd * pWnd = GetActiveWindow(); 
    if (!CDlgPlayer::IsLaunched()) {
		// player dialog not launched
		// get or create player object
        GetPlayer(true); 
        BOOL bFnKey = FALSE;
        if (HIWORD(lParam) == (WORD) -1) {
            // function key call
            bFnKey = TRUE;
            lParam = MAKELONG(LOWORD(lParam), FALSE);
        }
        // if player will not have size, set the old active window
		if (!(BOOL)HIWORD(lParam)) {
			// if the window is minimized, it will be null
			// this can occur during batch mode.
			if (pWnd != NULL) {
				pWnd->SetActiveWindow();
			}
        }
		// set player mode
        GetPlayer(false)->SetPlayerMode(mode, LOWORD(lParam), (BOOL)HIWORD(lParam), bFnKey, NULL);  
    } else {
        if (!GetPlayer(false)->IsTestRun()) {
            // player not running function key test
            BOOL bFnKey = FALSE;
            // kg 32 bit cast needed
            if (HIWORD(lParam) == (WORD)-1) {
                // function key call
                bFnKey = TRUE;
                lParam = MAKELONG(LOWORD(lParam), FALSE);
            }
			// set player mode
            GetPlayer(false)->SetPlayerMode(mode, LOWORD(lParam), (BOOL)HIWORD(lParam), bFnKey, NULL);  
            if (GetPlayer(false)->IsFullSize()) { 
				// player has full size, set it the active window
				// set focus on player
                GetPlayer(false)->SetActiveWindow(); 
                GetPlayer(false)->ShowWindow(SW_SHOW);
            }
        }
    }
    return 0;
}

/***************************************************************************/
// CMainFrame::OnPlayer Launches the player
// The user wants to see the modeless player dialog. If not already done
// it creates the player dialog, otherwise it just sets the player active
// and on top of all other windows. lParam delivers the player mode to be
// set.
/***************************************************************************/
LRESULT CMainFrame::PlaySpecific( SSpecific & specific) {
    
    TRACE("PlaySpecific\n");

	CDlgPlayer::EMode mode = CDlgPlayer::PLAYING;
	UINT nSubMode = ID_PLAYBACK_SPECIFIC;
	BOOL bFullSize = FALSE;

	// retrieve pointer to active window
    CWnd * pWnd = GetActiveWindow(); 
    if (!CDlgPlayer::IsLaunched()) {
		// player dialog not launched
		// get or create player object
        GetPlayer(true); 
        BOOL bFnKey = FALSE;
        // if player will not have size, set the old active window
        pWnd->SetActiveWindow();
		// set player mode
        GetPlayer(false)->SetPlayerMode( mode, nSubMode, bFullSize, bFnKey, &specific);  
    } else {
        if (!GetPlayer(false)->IsTestRun()) {
            // player not running function key test
            BOOL bFnKey = FALSE;
            // kg 32 bit cast needed
			// set player mode
            GetPlayer(false)->SetPlayerMode(mode, nSubMode, bFullSize, bFnKey, &specific);  
            if (GetPlayer(false)->IsFullSize()) { 
				// player has full size, set it the active window
				// set focus on player
                GetPlayer(false)->SetActiveWindow(); 
                GetPlayer(false)->ShowWindow(SW_SHOW);
            }
        }
    }
    return 0;
}

// SDM 1.06.5
/**************************************************************************/
// CMainFrame::OnEditor Launches the transcription editor
// The user wants to see the modeless transcription editor. If not already done
// it creates the editor dialog, otherwise it just sets the editor active
// and on top of all other windows.
/***************************************************************************/
void CMainFrame::OnEditor() {
    
	if (!IsEditAllowed()) {
        return;
    }
    if (m_pDlgEditor==NULL) {
		// New Editor with view parent
        m_pDlgEditor = new CDlgEditor(this);  
    }
    if (m_pDlgEditor!=NULL) {
        // Create window if necessary
        m_pDlgEditor->CreateSafe(CDlgEditor::IDD, this, &m_wplDlgEditor);
    }

    if ((m_pDlgEditor!=NULL) && (!m_pDlgEditor->IsWindowVisible())) {
        m_pDlgEditor->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
    } else if (m_pDlgEditor!=NULL) {
        m_pDlgEditor->ShowWindow(SW_HIDE);
    }
}

/**************************************************************************/
/**************************************************************************/
void CMainFrame::OnUpdateEditEditor(CCmdUI * pCmdUI) {
    if ((m_pDlgEditor!=NULL) && (m_pDlgEditor->IsWindowVisible())) {
        pCmdUI->SetText(_T("Hide Transcription Editor\tF4"));
    } else {
        pCmdUI->SetText(_T("Show Transcription Editor\tF4"));
    }
}

// SDM 1.06.5
/***************************************************************************/
// CMainFrame::OnIdleUpdate handles idle time update needs
/***************************************************************************/
LRESULT CMainFrame::OnIdleUpdate(WPARAM /*wParam*/, LPARAM /*lParam*/) {
    if (m_pDlgEditor!=NULL) {
        m_pDlgEditor->UpdateDialog();
    }
    return 0;
}

// SDM 1.5Test8.3
/***************************************************************************/
// CMainFrame::IsEditAllowed
/***************************************************************************/
BOOL CMainFrame::IsEditAllowed() {
    
	if (m_pDisplayPlot!=NULL) {
        return FALSE;
    }

	return TRUE;
}

/***************************************************************************/
// CMainFrame::OnEditFind Launches the find dialog
/***************************************************************************/
void CMainFrame::OnEditFind() {
    MaybeCreateFindOrReplaceDlg(true);
}

/***************************************************************************/
// CMainFrame::OnUpdateEditFind Menu Update
/***************************************************************************/
void CMainFrame::OnUpdateEditFind(CCmdUI * pCmdUI) {
    OnUpdateEditReplace(pCmdUI);
}

// CMainFrame::OnEditFind Launches the find dialog
/***************************************************************************/
void CMainFrame::OnEditFindNext() {
    // if the dialog is available, force it to do a 'next'
    if ((m_pDlgFind!=NULL)&&(m_pDlgFind->IsWindowVisible())) {
        m_pDlgFind->OnNext();
    } else {
        MaybeCreateFindOrReplaceDlg(true);
    }
}

/***************************************************************************/
// CMainFrame::OnUpdateEditFindNext Menu Update
/***************************************************************************/
void CMainFrame::OnUpdateEditFindNext(CCmdUI * pCmdUI) {
    OnUpdateEditReplace(pCmdUI);
}

/***************************************************************************/
// CMainFrame::OnEditReplace Launches the replace dialog
/***************************************************************************/
void CMainFrame::OnEditReplace() {
    MaybeCreateFindOrReplaceDlg(false);
}

/***************************************************************************/
// CMainFrame::OnUpdateEditReplace Menu Update
/***************************************************************************/
void CMainFrame::OnUpdateEditReplace(CCmdUI * pCmdUI) {
    CSaDoc * pDoc = GetCurrDoc();
    if (pDoc!=NULL) {
        pCmdUI->Enable(pDoc->GetDataSize() != 0); // enable if data is available
    } else {
        pCmdUI->Enable(FALSE);
    }
}

/***************************************************************************/
// CMainFrame::OnChangeView Message from a view
// A view is loosing focus. If the player is actually launched, it has to be
// informed to stop playing immediatly and to change caption. The statusbar
// panes have to be cleared if there is no more view.
/***************************************************************************/
LRESULT CMainFrame::OnChangeView(WPARAM wParam, LPARAM lParam) {
    if (m_pDlgFind!=NULL) {
        if (wParam!=FALSE) {
            m_pDlgFind->ChangeView();
        } else {
            m_pDlgFind->SendMessage(WM_CLOSE);
            m_pDlgFind = NULL;
        }
    }
    if (CDlgPlayer::IsLaunched()) {                 // player dialog launched
        if (wParam!=FALSE) {
            GetPlayer(false)->ChangeView((CSaView *)lParam);    // inform player
        } else {
            GetPlayer(false)->SendMessage(WM_CLOSE);    // last view closed, close player too
        }
    }
    if (CDlgAutoRecorder::IsLaunched()) {
        if (wParam==FALSE) {
            m_pAutoRecorder->SendMessage(WM_CLOSE);
            m_pAutoRecorder = NULL;
        }
    }

    if (wParam==FALSE) { // last view closed
        // turn off symbols
        m_dataStatusBar.SetPaneSymbol(ID_STATUSPANE_SAMPLES, FALSE);
        m_dataStatusBar.SetPaneSymbol(ID_STATUSPANE_FORMAT, FALSE);
        m_dataStatusBar.SetPaneSymbol(ID_STATUSPANE_CHANNELS, FALSE);
        m_dataStatusBar.SetPaneSymbol(ID_STATUSPANE_SIZE, FALSE);
        m_dataStatusBar.SetPaneSymbol(ID_STATUSPANE_TLENGTH, FALSE);
        m_dataStatusBar.SetPaneSymbol(ID_STATUSPANE_TYPE, FALSE);
        m_dataStatusBar.SetPaneSymbol(ID_STATUSPANE_BITRATE, FALSE);
        m_dataStatusBar.SetPaneSymbol(ID_STATUSPANE_EMPTY, FALSE);
        m_dataStatusBar.SetPaneSymbol(ID_STATUSPANE_1, FALSE);
        m_dataStatusBar.SetPaneSymbol(ID_STATUSPANE_2, FALSE);
        m_dataStatusBar.SetPaneSymbol(ID_STATUSPANE_3, FALSE);
        m_dataStatusBar.SetPaneSymbol(ID_STATUSPANE_4, FALSE);
        // clear the panes
        m_dataStatusBar.SetPaneText(ID_STATUSPANE_SAMPLES, _T(""));
        m_dataStatusBar.SetPaneText(ID_STATUSPANE_FORMAT, _T(""));
        m_dataStatusBar.SetPaneText(ID_STATUSPANE_CHANNELS, _T(""));
        m_dataStatusBar.SetPaneText(ID_STATUSPANE_SIZE, _T(""));
        m_dataStatusBar.SetPaneText(ID_STATUSPANE_TLENGTH, _T(""));
        m_dataStatusBar.SetPaneText(ID_STATUSPANE_TYPE, _T(""));
        m_dataStatusBar.SetPaneText(ID_STATUSPANE_BITRATE, _T(""));
        m_dataStatusBar.SetPaneText(ID_STATUSPANE_EMPTY, _T(""));
        m_dataStatusBar.SetPaneText(ID_STATUSPANE_1, _T(""));
        m_dataStatusBar.SetPaneText(ID_STATUSPANE_2, _T(""));
        m_dataStatusBar.SetPaneText(ID_STATUSPANE_3, _T(""));
        m_dataStatusBar.SetPaneText(ID_STATUSPANE_4, _T(""));
    }

    return 0;
}

/***************************************************************************/
// CMainFrame::OnClose Mainframe closing
// Called from the framework. The mainframe window is closing. If the player
// is actually launched, it has to be informed to stop playing immediatly.
// The find/replace dialog has also to be closed and the same for the
// workbench view. If SA runs in batch mode, don't close, but call the return
// to SM menu item.
/***************************************************************************/
void CMainFrame::OnClose() {

    CSaApp * pApp = (CSaApp *)AfxGetApp();

    if (pApp->GetBatchMode() == 1) {
        // TODO: this can probably be completely removed at some point
        SendMessage(WM_COMMAND, ID_FILE_RETURN, 0L);
        return;
    }

	// user canceled close operation
    if (!AfxGetApp()->SaveAllModified()) { 
        return;
    }

    // check if workbench document is open
    CDocument * pWbDoc = ((CSaApp *)AfxGetApp())->GetWbDoc();

    //******************************************************
    // close the workbench
    //******************************************************
    if (pWbDoc) {
        POSITION pos = pWbDoc->GetFirstViewPosition();
        CView * pView = pWbDoc->GetNextView(pos);
		// close view
        pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0); 
    }

    ((CSaApp *)AfxGetApp())->SetWbOpenOnExit(pWbDoc != NULL);

    // Workbench is still there, don't close
    if (((CSaApp *)AfxGetApp())->GetWbDoc()) {
        return;
    }

    //******************************************************
    //******************************************************
    if (!m_bPrintPreviewInProgress) {
        ((CSaApp *)AfxGetApp())->WriteSettings();
    }

    //******************************************************
    // If player dialog open then close it.
    //******************************************************
    if (CDlgPlayer::IsLaunched()) {
        m_pDlgPlayer->SendMessage(WM_CLOSE);
        m_pDlgPlayer = NULL;
    }

    //******************************************************
    // If find dialog open then close it.
    //******************************************************
    if (m_pDlgFind!=NULL) {
        m_pDlgFind->SendMessage(WM_CLOSE);
        m_pDlgFind = NULL;
    }

    if (CDlgAutoRecorder::IsLaunched()) {
        m_pAutoRecorder->SendMessage(WM_CLOSE);
        m_pAutoRecorder = NULL;
    }

    CMDIFrameWnd::OnClose();
}

/***************************************************************************/
// CMainFrame::OnSysColorChange System colors changed
/***************************************************************************/
void CMainFrame::OnSysColorChange() {

    CMDIFrameWnd::OnSysColorChange();
    // update system colors
    m_colors.SetupSystemColors();
    // apply color changes to graphs
    SendMessageToMDIDescendants(WM_USER_GRAPH_STYLECHANGED, 0, 0L);
}

/***************************************************************************/
// CMainFrame::OnUpdateDataPane Data statusbar pane updating
/***************************************************************************/
void CMainFrame::OnUpdateDataPane(CCmdUI * pCmdUI) {
    if (m_bIsPrinting) {
    } else {
        pCmdUI->Enable();
    }
}

/***************************************************************************/
// CMainFrame::OnUpdateProgressPane Progress statusbar pane updating
/***************************************************************************/
void CMainFrame::OnUpdateProgressPane(CCmdUI * pCmdUI) {
    if (m_bIsPrinting) {
    } else {
        pCmdUI->Enable();
    }
}

/***************************************************************************/
// CMainFrame::OnSpeechAppCall Call from another application
// Another application wants to start up SA. Check out what to do. If SA
// is to close, be sure to first take SA out of batch mode to enable standard
// exit procedure and then send the ID_APP_EXIT.
/***************************************************************************/
LRESULT CMainFrame::OnSpeechAppCall(WPARAM wParam, LPARAM lParam) {
    // ASSERT(FALSE); // Assert to debug this function
    CSaApp * pApp = (CSaApp *)AfxGetApp();
    if (wParam == SPEECH_WPARAM_CLOSE) {
        pApp->CancelBatchMode(); // cancel batch mode for exit
        PostMessage(WM_COMMAND, ID_APP_EXIT, 0L); // close SA
    }
    CSaString szCmd;
    UNUSED_ALWAYS(lParam);
    if (wParam == SPEECH_WPARAM_SHOWSA) {
        pApp->ExamineCmdLine(szCmd, wParam);    // start up SA in batch mode with list file
    }
    if (wParam == SPEECH_WPARAM_SHOWSAREC) {
        pApp->ExamineCmdLine(szCmd, wParam);    // start up SA in batch mode in recording mode
    }
    return 0;
}

/***************************************************************************/
// CMainFrame::GetAnnotation
// Gets the annotation set index annotSetID for the current view/doc.
/***************************************************************************/
CSegment * CMainFrame::GetAnnotation(int annotSetID) {
    CSegment * pSeg = NULL;
    CSaView * pView = (CSaView *)GetCurrSaView();
    if ((pView != NULL) && pView->IsKindOf(RUNTIME_CLASS(CSaView))) {
        pSeg = pView->GetAnnotation(annotSetID);
    }
    return pSeg;
}

//SDM 1.06.6U4
/***************************************************************************/
// CMainFrame::GetCurrSaView
// returns the active SaView.
/***************************************************************************/
CSaView * CMainFrame::GetCurrSaView(void) {
    CMDIChildWnd * pMDIWnd = MDIGetActive(); // get active child window

    if (pMDIWnd) {
        CSaView * pView = (CSaView *)MDIGetActive()->GetActiveView();

        if ((pView != NULL) && pView->IsKindOf(RUNTIME_CLASS(CSaView))) {
            return pView;
        }
    }

    return NULL;
}


/***************************************************************************/
// CMainFrame::GetCurrDoc
// returns the doc for the active view.
/***************************************************************************/
CSaDoc * CMainFrame::GetCurrDoc() {
    CSaDoc * pDoc = NULL;
    CSaView * pView = GetCurrSaView();

    if (pView) {
        pDoc = pView->GetDocument();
    }

    return pDoc;
}

/***************************************************************************/
// CMainFrame::SetPrintingFlag
// Sets the "is printing" state to true.  Passes on the setting to the
// progress bar.
/***************************************************************************/
void  CMainFrame::SetPrintingFlag() {
    m_bIsPrinting = TRUE;
    m_progressStatusBar.SetIsPrintingFlag(m_bIsPrinting);
};

/***************************************************************************/
// CMainFrame::ClearPrintingFlag
/***************************************************************************/
void  CMainFrame::ClearPrintingFlag() {
    m_bIsPrinting = FALSE;
    m_progressStatusBar.SetIsPrintingFlag(m_bIsPrinting);
};

//SDM 1.06.6U4
/***************************************************************************/
// CMainFrame::OnSaveScreenAsBMP
/***************************************************************************/
void CMainFrame::OnSaveScreenAsBMP() {
    CDib dib;
    CWnd * pWnd = GetTopLevelParent();
    if (pWnd==NULL) {
        pWnd = this;
    }
    dib.CaptureWindow(pWnd);
    dib.Save();
}

//SDM 1.06.6U5
/***************************************************************************/
// CMainFrame::OnSaveWindowAsBMP
/***************************************************************************/
void CMainFrame::OnSaveWindowAsBMP() {

	CDib dib;
    CRect rectCrop(0,0,0,0);
    CRect rectToolbar, rectMainWnd;
	int tbID = IDR_BAR_BASIC;
    GetControlBar(tbID)->GetWindowRect(&rectToolbar);
    AfxGetMainWnd()->GetWindowRect(&rectMainWnd);
    int nHeight = rectToolbar.bottom - rectToolbar.top;
    int nWidth = rectToolbar.right - rectToolbar.left;
    if (!GetControlBar(tbID)->IsFloating()) {
        if ((nWidth > nHeight) && (rectToolbar.top < (rectMainWnd.top + rectMainWnd.bottom) / 2)) {
            rectCrop.top = nHeight - 2;
        }
        if ((nWidth < nHeight) && (rectToolbar.left < (rectMainWnd.left + rectMainWnd.right) / 2)) {
            rectCrop.left = nWidth - 2;
        }
    }
    dib.CaptureWindow(this, rectCrop, TRUE);
    dib.Save();
}

//SDM 1.06.6U4
/***************************************************************************/
// CMainFrame::OnSaveGraphsAsBMP
/***************************************************************************/
void CMainFrame::OnSaveGraphsAsBMP() {
    CSaView * pSaView = GetCurrSaView();
    if (pSaView==NULL) {
        return;
    }
    CDib dib;
    // SDM 1.06.6U5 capture client area
    dib.CaptureWindow(pSaView);
    dib.Save();
}

//SDM 1.06.6U4
/***************************************************************************/
// CMainFrame::OnCopyScreenAsBMP
/***************************************************************************/
void CMainFrame::OnCopyScreenAsBMP() {
    CWnd * pWnd = GetTopLevelParent();
    if (pWnd==NULL) {
        pWnd = this;
    }
    CDib dib;
    dib.CaptureWindow(pWnd);
    dib.CopyToClipboard(pWnd);
}

//SDM 1.06.6U5
/***************************************************************************/
// CMainFrame::OnCopyWindowAsBMP
/***************************************************************************/
void CMainFrame::OnCopyWindowAsBMP() {
    CDib dib;
    CRect rectCrop(0,0,0,0);
    CRect rectToolbar, rectMainWnd;
	int tbID = IDR_BAR_BASIC;
    GetControlBar(tbID)->GetWindowRect(&rectToolbar);
    GetWindowRect(&rectMainWnd);
    int nHeight = rectToolbar.bottom - rectToolbar.top;
    int nWidth = rectToolbar.right - rectToolbar.left;
    if (!GetControlBar(tbID)->IsFloating()) {
        if ((nWidth > nHeight) && (rectToolbar.top < (rectMainWnd.top + rectMainWnd.bottom) / 2)) {
            rectCrop.top = nHeight - 2;
        }
        if ((nWidth < nHeight) && (rectToolbar.left < (rectMainWnd.left + rectMainWnd.right) / 2)) {
            rectCrop.left = nWidth - 2;
        }
    }

    dib.CaptureWindow(this, rectCrop, TRUE);
    dib.CopyToClipboard(this);
}

//SDM 1.06.6U4
/***************************************************************************/
// CMainFrame::OnCopyGraphsAsBMP
/***************************************************************************/
void CMainFrame::OnCopyGraphsAsBMP() {
    CSaView * pSaView = GetCurrSaView();
    if (pSaView==NULL) {
        return;
    }
    CDib dib;
    dib.CaptureWindow(pSaView);
    dib.CopyToClipboard(pSaView);
}

//SDM 1.06.6U4
/***************************************************************************/
// CMainFrame::OnUpdateGraphsAsBMP
/***************************************************************************/
void CMainFrame::OnUpdateGraphsAsBMP(CCmdUI * pCmdUI) {
    pCmdUI->Enable((GetCurrSaView() != NULL));
}

/***************************************************************************/
// CMainFrame::OnSetDefault
/***************************************************************************/
void CMainFrame::OnSetDefaultGraphs(BOOL bPermanent) {

	// Get pointers to the active view and it's document.
    CSaView * pSrcView = (CSaView *)GetCurrSaView();

    // If a view class for the default view has exists then
    // delete it so we can create a new default from the
    // current view.  DDO - 08/07/00
    if (m_pDefaultViewConfig!=NULL) {
        delete m_pDefaultViewConfig;
    }
    m_pDefaultViewConfig = new CSaView(pSrcView);

    // Build a string of graph names used to display on the
    // dialog to tell the user the current default view
    // graphs. Also get the layout (used to show the user an
    // icon of the current layout).
    m_szTempDefaultGraphs = "";
    m_nTempDefaultLayout = (bPermanent) ? 0 : pSrcView->GetLayout();

    if (bPermanent) {
        m_szPermDefaultGraphs = "";
        m_nPermDefaultLayout = pSrcView->GetLayout();
    }

    // Build a string from graph captions.
    ((bPermanent) ? m_szPermDefaultGraphs : m_szTempDefaultGraphs) = m_pDefaultViewConfig->GetGraphsDescription();

    // SDM 1.5Test8.1
    BOOL bMaximized = m_bDefaultMaximizeView;

    if (MDIGetActive(&bMaximized)) {
        m_bDefaultMaximizeView = bMaximized;
        WINDOWPLACEMENT WP;
        WP.length = sizeof(WINDOWPLACEMENT);
        if (MDIGetActive()->GetWindowPlacement(&WP)) {
            // SDM 32 bit conversion
            m_nDefaultHeightView = WP.rcNormalPosition.bottom - WP.rcNormalPosition.top;
            m_nDefaultWidthView  = WP.rcNormalPosition.right - WP.rcNormalPosition.left;
        }
    }

    // If the user clicked on the permanent button then save
    // the default settings to the file that holds the
    // temporary default view settings. DDO - 08/08/00
    if (bPermanent) {
        m_nStartDataMode = 0;
        WriteDefaultViewToTempFile();
    }
}

/***************************************************************************/
// CMainFrame::OnSetDefaultParameters
/***************************************************************************/
void CMainFrame::OnSetDefaultParameters() {
    //**********************************************************
    // Get pointers to the active view and it's document.
    //**********************************************************
    CSaView * pSrcView = (CSaView *)GetCurrSaView();
    CSaDoc * pDoc = pSrcView->GetDocument();

    //**********************************************************
    // RLJ 1.5Test11.1A - Copy current pitch parameter values
    // to defaults.
    //**********************************************************
    const CPitchParm * pPitchParm                    = pDoc->GetPitchParm();  // Get pitch parameter values.
    m_pitchParmDefaults.nRangeMode           = pPitchParm->nRangeMode;
    m_pitchParmDefaults.nScaleMode           = pPitchParm->nScaleMode;
    m_pitchParmDefaults.nUpperBound          = pPitchParm->nUpperBound;
    m_pitchParmDefaults.nLowerBound          = pPitchParm->nLowerBound;
    m_pitchParmDefaults.bUseCepMedianFilter  = pPitchParm->bUseCepMedianFilter;
    m_pitchParmDefaults.nCepMedianFilterSize = pPitchParm->nCepMedianFilterSize;

    //**********************************************************
    // Copy current music parameter values
    // to defaults.
    //**********************************************************
    const CMusicParm * pMusicParm                    = pDoc->GetMusicParm();  // Get pitch parameter values.
    m_musicParmDefaults.nRangeMode           = pMusicParm->nRangeMode;
    m_musicParmDefaults.nUpperBound          = pMusicParm->nUpperBound;
    m_musicParmDefaults.nLowerBound          = pMusicParm->nLowerBound;

    m_intensityParmDefaults = pDoc->GetCIntensityParm();

    //**********************************************************
    // RLJ 1.5Test11.1A - Get pointer to view.
    //**********************************************************
    CSaView * pView = (CSaView *)GetCurrSaView();
    UINT * pnID = pView->GetGraphIDs();

    //**********************************************************
    // Check to see which (if any) of spectrum, Spectrogram,
    // and Snapshot graphs are displayed.
    //**********************************************************
    BOOL bFormantGraphUsed      = FALSE;
    BOOL bSpectrumGraphUsed     = FALSE;
    BOOL bSpectrogramGraphUsed = FALSE;
    BOOL bSnapshotGraphUsed = FALSE;

    for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
        if (pnID[nLoop] == IDD_3D ||
                pnID[nLoop] == IDD_F1F2 ||
                pnID[nLoop] == IDD_F2F1 ||
                pnID[nLoop] == IDD_F2F1F1 ||
                pnID[nLoop] == IDD_WAVELET)         // 7/31/01 ARH - Added for the wavelet scalogram view

        {
            bFormantGraphUsed = TRUE;
        } else if (pnID[nLoop] == IDD_SPECTRUM) {
            bSpectrumGraphUsed = TRUE;
        } else if (pnID[nLoop] == IDD_SPECTROGRAM) {
            bSpectrogramGraphUsed = TRUE;
        } else if (pnID[nLoop] == IDD_SNAPSHOT) {
            bSnapshotGraphUsed = TRUE;
        }
    }

    //**********************************************************
    // AKE 1.5Test13.1 - Copy current formant chart parameter
    // values to defaults.
    //**********************************************************
    if (bFormantGraphUsed) {
        CProcessFormants * pFormants                 = (CProcessFormants *)pDoc->GetFormants();
        CFormantParm * pFormantParm                   = pFormants->GetFormantParms();  // Get formant chart parameter values.
        m_formantParmDefaults.bFromLpcSpectrum      = pFormantParm->bFromLpcSpectrum;
        m_formantParmDefaults.bFromCepstralSpectrum = pFormantParm->bFromCepstralSpectrum;
        m_formantParmDefaults.bTrackFormants        = pFormantParm->bTrackFormants;
        m_formantParmDefaults.bSmoothFormants       = pFormantParm->bSmoothFormants;
        m_formantParmDefaults.bMelScale             = pFormantParm->bMelScale;
    }

    //**********************************************************
    // RLJ 1.5Test11.1A - Copy current spectrum parameter
    // values to defaults.
    //**********************************************************
    if (bSpectrumGraphUsed) {
        CProcessSpectrum * pSpectrum            = (CProcessSpectrum *)pDoc->GetSpectrum();
        CSpectrumParm * pSpectrumParm            = pSpectrum->GetSpectrumParms();  // Get spectrum parameter values.
        m_spectrumParmDefaults.nScaleMode      = pSpectrumParm->nScaleMode;
        m_spectrumParmDefaults.nPwrUpperBound  = pSpectrumParm->nPwrUpperBound;
        m_spectrumParmDefaults.nPwrLowerBound  = pSpectrumParm->nPwrLowerBound;
        m_spectrumParmDefaults.nFreqUpperBound = pSpectrumParm->nFreqUpperBound;
        m_spectrumParmDefaults.nFreqLowerBound = pSpectrumParm->nFreqLowerBound;
        m_spectrumParmDefaults.nFreqScaleRange = pSpectrumParm->nFreqScaleRange;
        m_spectrumParmDefaults.nSmoothLevel    = pSpectrumParm->nSmoothLevel;
        m_spectrumParmDefaults.nPeakSharpFac   = pSpectrumParm->nPeakSharpFac;
        m_spectrumParmDefaults.cWindow      = pSpectrumParm->cWindow;
        m_spectrumParmDefaults.bShowLpcSpectrum  = pSpectrumParm->bShowLpcSpectrum;
        m_spectrumParmDefaults.bShowCepSpectrum  = pSpectrumParm->bShowCepSpectrum;
        m_spectrumParmDefaults.bShowFormantFreq  = pSpectrumParm->bShowFormantFreq;
        m_spectrumParmDefaults.bShowFormantBandwidth  = pSpectrumParm->bShowFormantBandwidth;
        m_spectrumParmDefaults.bShowFormantPower = pSpectrumParm->bShowFormantPower;
    }

    //**********************************************************
    // RLJ 1.5Test11.1A - Get process index of Spectrogram
    // graph.
    //**********************************************************
    if (bSpectrogramGraphUsed) {
        CGraphWnd * pGraph = NULL;

        for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
            if (pnID[nLoop] == IDD_SPECTROGRAM) {
                pGraph = pView->GetGraph(nLoop);
            }
        }

        ASSERT(pGraph);

        //********************************************************
        // Copy current Spectrogram parameter values to object
        // containing Spectrogram defaults.
        //********************************************************
        CProcessSpectrogram * pSpectro = (CProcessSpectrogram *)pDoc->GetSpectrogram();
        m_spectrogramParmDefaults = pSpectro->GetSpectroParm();

        if (m_spectrogramParmDefaults.nFrequency >= int(pDoc->GetSamplesPerSec()*45/100))
            // This spectrogram is set to near nyquist
            // Assume the user wants all spectrograms to be display at nyquist
            // Set frequency above any reasonable sampling nyquist to force clipping to nyquist
        {
            m_spectrogramParmDefaults.nFrequency = 99999;
        }
    }

    if (bSnapshotGraphUsed) {
        CGraphWnd * pGraph = NULL;

        for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
            if (pnID[nLoop] == IDD_SNAPSHOT) {
                pGraph = pView->GetGraph(nLoop);
            }
        }

        ASSERT(pGraph);

        //********************************************************
        // Copy current Snapshot parameter values to object
        // containing Snapshot defaults.
        //********************************************************
        m_snapshotParmDefaults = pDoc->GetSnapshot()->GetSpectroParm();
        if (m_snapshotParmDefaults.nFrequency >= int(pDoc->GetSamplesPerSec()*45/100))
            // This spectrogram is set to near nyquist
            // Assume the user wants all spectrograms to be display at nyquist
            // Set frequency above any reasonable sampling nyquist to force clipping to nyquist
        {
            m_snapshotParmDefaults.nFrequency = 99999;
        }
    }

}

/***************************************************************************/
// CMainFrame::OnEqualizeLength
/***************************************************************************/
void CMainFrame::OnEqualizeLength() {
    CDlgTargView dlg(this);
    if (dlg.DoModal()==IDOK) {
        CSaView * pSrcView = (CSaView *)GetCurrSaView();
        CSaView * pTarg = dlg.Targ();
        if (pTarg) {
            CSaDoc  * pTargDoc = pTarg->GetDocument();
            if (pTargDoc) {
                CSaDoc * pSrcDoc = pSrcView->GetDocument();
                DWORD wSrcSmpSize = pSrcDoc->GetSampleSize();
                DWORD SrcLen = (pSrcView->GetStopCursorPosition() - pSrcView->GetStartCursorPosition() + wSrcSmpSize) / wSrcSmpSize;
                DWORD wTargSmpSize = pTargDoc->GetSampleSize();
                DWORD TargStop = pTarg->GetStartCursorPosition() + (DWORD)((double)SrcLen * (double)pTargDoc->GetSamplesPerSec() / (double)pSrcDoc->GetSamplesPerSec() + 0.5) * wTargSmpSize;
                if (TargStop > pTargDoc->GetDataSize()) {
                    AfxMessageBox(IDS_EQUALIZE_TOO_FAR_RIGHT);
                } else {
                    pTarg->SetStopCursorPosition(TargStop);
                }
            }
        }
    }
}

/***************************************************************************/
// CMainFrame::OnUpdateEqualizeLength
/***************************************************************************/
void CMainFrame::OnUpdateEqualizeLength(CCmdUI * pCmdUI) {

    int count = 0;
    CDocList doclst; // list of currently open docs
    for (CSaDoc * pdoc = doclst.pdocFirst(); pdoc; pdoc = doclst.pdocNext()) {
        if (pdoc != NULL) {
            count++;
        }
    }

    pCmdUI->Enable(count>1);
}


/***************************************************************************/
// CMainFrame::CreateFindReplaceDlg Launches the find/replace dialog
// The user wants to see the modeless find dialog.
/***************************************************************************/
void CMainFrame::CreateFindOrReplaceDlg() {
    // Get the string segment represented;
    CSaString sToFind;
    int annotWndIndex = 0;
    CSaView * pView = (CSaView *)GetCurrSaView();
    if (pView->IsAnyAnnotationSelected()) {
        sToFind = pView->GetSelectedAnnotationString();
        annotWndIndex = pView->FindSelectedAnnotationIndex();
    }
    CSaString fields;
    fields.LoadString(IDS_FINDFIELDS);
    m_pDlgFind = new CDlgFind((CWnd *)this,fields, sToFind, m_bFindOnly,CSaString(_T("")),annotWndIndex,this);
    if (!m_pDlgFind->Created()) {
        delete m_pDlgFind;
        m_pDlgFind = NULL;
        ASSERT_VALID(this);
    }
}



/***************************************************************************/
// CMainFrame::MaybeCreateFindOrReplaceDlg
// Brings up the find or replace dialog, creating it if neccessary.
/***************************************************************************/
void CMainFrame::MaybeCreateFindOrReplaceDlg(bool bWantFindOnly) {

    if (m_pDlgFind!=NULL) {
        delete m_pDlgFind;
        m_pDlgFind = NULL;
    }
    m_bFindOnly = bWantFindOnly;
    CreateFindOrReplaceDlg();
}

/***************************************************************************/
// CMainFrame::SetWbProcess Set new pointer to workbench process
// Sets a new pointer to a workbench process and returns the old one.
/***************************************************************************/
CProcess * CMainFrame::SetWbProcess(int nProcess, int nFilter, CProcess * pProcess) {
    CProcess * pTempProcess = m_apWbProcess[nProcess][nFilter];
    m_apWbProcess[nProcess][nFilter] = pProcess;
    return pTempProcess;
}

/***************************************************************************/
// CMainFrame::SetWbFilterID Set new filter ID to workbench
// Sets a new filter ID into the workbench and returns the old one.
/***************************************************************************/
int CMainFrame::SetWbFilterID(int nProcess, int nFilter, int nID) {
    int nTempID = m_aWbFilterID[nProcess][nFilter];
    m_aWbFilterID[nProcess][nFilter] = nID;
    return nTempID;
}

/***************************************************************************/
// CMainFrame::IsProcessUsed Checks if process used by a document
// Returns a pointer to the document that uses the process if there is one,
// else it returns NULL.
/***************************************************************************/
CDocument * CMainFrame::IsProcessUsed(int nProcess) {
    CDocList doclst; // list of currently open docs
    for (CSaDoc * pDoc = doclst.pdocFirst(); pDoc; pDoc = doclst.pdocNext()) {
        if (pDoc != NULL) {
            if (nProcess == pDoc->GetWbProcess()) {
                return (CDocument *)pDoc;
            }
        }
    }
    return NULL;
}

/***************************************************************************/
// CMainFrame::DeleteWbProcesses Deletes all workbench processes
// This function also checks, if a document had used a process and switches
// it back to plain with recalculating if the flag bSwitchBack is TRUE
// (default).
/***************************************************************************/
void CMainFrame::DeleteWbProcesses(BOOL bSwitchBack) {
    // delete workbench processes
    for (int nLoop = 0; nLoop < MAX_PROCESS_NUMBER; nLoop++) {
        for (int nFilterLoop = 0; nFilterLoop < MAX_FILTER_NUMBER; nFilterLoop++) {
            if (m_apWbProcess[nLoop][nFilterLoop]) {
                CDocument * pDoc = IsProcessUsed(nLoop);
                if ((pDoc!=NULL) && (bSwitchBack)) {
                    ((CSaDoc *)pDoc)->SetWbProcess(0);    // switch back to plain
                }
                delete m_apWbProcess[nLoop][nFilterLoop];
                m_apWbProcess[nLoop][nFilterLoop] = NULL;
            }
            m_aWbFilterID[nLoop][nFilterLoop] = 0;
        }
    }
}

//********************************************************************
//
//********************************************************************
static LPCSTR psz_mainframe          = "mainframe";
static LPCSTR psz_placementMain      = "placementMain";
static LPCSTR psz_placementEditor    = "placementEditor";
static LPCSTR psz_showstartupdlg     = "showstartupdlg";
static LPCSTR psz_showadvancedaudio  = "showadvancedaudio";
static LPCSTR psz_saveopenfiles      = "saveopenfiles";
static LPCSTR psz_statusbar          = "statusbar";
static LPCSTR psz_statusposreadout   = "statusposreadout";
static LPCSTR psz_statuspitchreadout = "statuspitchreadout";
static LPCSTR psz_toolbar            = "toolbar";
static LPCSTR psz_taskbar            = "taskbar";
static LPCSTR psz_toneAbove          = "toneAbove";
static LPCSTR psz_scrollzoom         = "scrollzoom";
static LPCSTR psz_captionstyle       = "captionstyle";
static LPCSTR psz_buffersize         = "buffersize";
static LPCSTR psz_cursoralignment    = "cursoralignment";
static LPCSTR psz_graphfontarray     = "graphfontarray";
static LPCSTR psz_graphfont          = "graphfont";
static LPCSTR psz_graphfontsize      = "graphfontsize";
static LPCSTR psz_defaultgraphlist   = "DefaultGraphList";
static LPCSTR psz_startmode          = "StartMode";
static LPCSTR psz_bAnimate           = "Animate";
static LPCSTR psz_animationRate      = "AnimationRate";
static LPCSTR psz_graphUpdateMode    = "GraphUpdateMode";
static LPCSTR psz_cursorMode         = "CursorMode";
static LPCSTR psz_importalgorithm	 = "importalgorithm";

// SDM 1.06.6U5 save maximized state of MDIChild (SaView)
static LPCSTR psz_bMaxView           = "DefaultMaximizeView";
static LPCSTR psz_WidthView          = "DefaultWidthView";
static LPCSTR psz_HeightView         = "DefaultHeightView";
static const char * psz_autosave     = "AutoSave";

//********************************************************************
//
//********************************************************************
void CMainFrame::WriteProperties(CObjectOStream & obs) {

    //*****************************************************
    // Before beginning to write the properties, make sure that if there has been a permanent default view
    // defined, it is read from the temp file. 
	// This is to make sure any temp. view stored in memory gets replaced with the permanent view the user wants to
    // keep in the settings file. i.e. This will make sure the temp. default view isn't the one written to the
    // settings file.  DDO - 08/07/00
    //*****************************************************
    if (m_bDefaultViewExists) {
        ReadDefaultViewFromTempFile();
    }

    obs.WriteNewline();
    obs.WriteBeginMarker(psz_mainframe);

    // write out properties
    WINDOWPLACEMENT wpl;
    this->GetWindowPlacement(&wpl);
    obs.WriteWindowPlacement(psz_placementMain, wpl);

    if (m_pDlgEditor!=NULL) {
        m_pDlgEditor->GetWindowPlacement(&wpl);
        obs.WriteWindowPlacement(psz_placementEditor, wpl);
    } else if (m_wplDlgEditor.length) {
        obs.WriteWindowPlacement(psz_placementEditor, m_wplDlgEditor);
    }

    //*****************************************************
    // DDO - 08/03/00 Always save on exit so don't need
    // to save a setting that tells future sessions of
    // SA to save settings.
    // obs.WriteBool(psz_saveonexit, m_bSaveOnExit);
    //*****************************************************
    obs.WriteInteger(psz_startmode, m_nStartDataMode);
    obs.WriteBool(psz_showstartupdlg, m_bShowStartupDlg);
    obs.WriteBool(psz_saveopenfiles, m_bSaveOpenFiles);
    obs.WriteBool(psz_showadvancedaudio, m_bShowAdvancedAudio);
    obs.WriteBool(psz_statusbar , m_bStatusBar);
    obs.WriteInteger(psz_statusposreadout, m_nStatusPosReadout);
    obs.WriteInteger(psz_statuspitchreadout, m_nStatusPitchReadout);
    obs.WriteBool(psz_toolbar, AdvancedToolBarVisible());
    obs.WriteBool(psz_taskbar, TaskBarVisible());
    obs.WriteBool(psz_toneAbove, m_bToneAbove);
    obs.WriteBool(psz_scrollzoom, m_bScrollZoom);
    obs.WriteInteger(psz_graphUpdateMode, m_nGraphUpdateMode);
    obs.WriteBool(psz_bAnimate, m_bAnimate);
    obs.WriteInteger(psz_animationRate, m_nAnimationRate);
    obs.WriteInteger(psz_cursorMode, m_nCursorAlignment);
	obs.WriteInteger(psz_importalgorithm, m_nAlgorithm);

    m_colors.WriteProperties(obs);
    m_fnKeys.WriteProperties(obs);
    m_grid.WriteProperties(obs);
    m_waveformGeneratorSettings.WriteProperties(obs);

    // Write parsing, segmenting, pitch, spectrum, and spectrogram parameter defaults to *.PSA file.      // RLJ 11.1A
    m_parseParmDefaults.WriteProperties(obs);
    m_segmentParmDefaults.WriteProperties(obs);
    m_pitchParmDefaults.WriteProperties(obs);
    m_musicParmDefaults.WriteProperties(obs);
    m_intensityParmDefaults.WriteProperties(obs);
    m_spectrumParmDefaults.WriteProperties(obs);
    m_spectrogramParmDefaults.WritePropertiesA(obs);
    m_snapshotParmDefaults.WritePropertiesB(obs);

    obs.WriteInteger(psz_captionstyle, m_nCaptionStyle);

    obs.WriteBeginMarker(psz_graphfontarray);
    for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
        CSaString szFont = m_GraphFontFaces.GetAt(nLoop);
        obs.WriteString(psz_graphfont, szFont.utf8().c_str());
        obs.WriteUInt(psz_graphfontsize, m_GraphFontSizes.GetAt(nLoop));
    }
    obs.WriteEndMarker(psz_graphfontarray);

    BOOL bMaximized = FALSE;

    //SDM 1.06.6U5 if there is an active child save its maximized state and normal size as the defaults
    if (MDIGetActive(&bMaximized)) {
        m_bDefaultMaximizeView = bMaximized;
        WINDOWPLACEMENT WP;
        WP.length = sizeof(WINDOWPLACEMENT);
		// SDM 32bit conversion
        if (MDIGetActive()->GetWindowPlacement(&WP)) { 
            m_nDefaultHeightView = WP.rcNormalPosition.bottom - WP.rcNormalPosition.top;
            m_nDefaultWidthView = WP.rcNormalPosition.right - WP.rcNormalPosition.left;
        }
    }

    obs.WriteBool(psz_bMaxView,  m_bDefaultMaximizeView);
    obs.WriteInteger(psz_HeightView, m_nDefaultHeightView);
    obs.WriteInteger(psz_WidthView, m_nDefaultWidthView);
    obs.WriteBool(psz_autosave, m_bAutoSave);

    obs.WriteEndMarker(psz_mainframe);
    obs.WriteNewline();
}

//********************************************************************
// Read the open databases and windows
//********************************************************************
BOOL CMainFrame::ReadProperties(CObjectIStream & obs, bool batchMode) {

    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_mainframe)) {
		return FALSE;
    }

    BOOL b = FALSE;
    int nValue;

    while (!obs.bAtEnd()) {
        WINDOWPLACEMENT wpl;
        if (obs.bReadInteger(psz_startmode, m_nStartDataMode));
        else if (obs.bReadBool(psz_showstartupdlg, m_bShowStartupDlg));
        else if (obs.bReadBool(psz_saveopenfiles, m_bSaveOpenFiles));
        else if (obs.bReadBool(psz_showadvancedaudio, m_bShowAdvancedAudio));
		else if (obs.bReadInteger(psz_importalgorithm, m_nAlgorithm));
        else if (obs.bReadWindowPlacement(psz_placementMain, wpl)) {
            if (wpl.showCmd == SW_SHOWMINIMIZED) {
                wpl.showCmd = SW_SHOWNORMAL;
            }
            wpl.ptMinPosition.x = -1;
            wpl.ptMinPosition.y = -1;

			if (!batchMode) {
				SetWindowPlacement(&wpl);
			}
            if (wpl.showCmd == SW_SHOWMAXIMIZED) {
				if (!batchMode) {
					ShowWindow(wpl.showCmd);
				}
            }
        }

        else if (obs.bReadWindowPlacement(psz_placementEditor, m_wplDlgEditor));    // SDM - 1.5Test8.2
        else if (obs.bReadBool(psz_statusbar, b)) {
            if (b != m_bStatusBar) {
                SendMessage(WM_COMMAND, ID_VIEW_STATUS_BAR, 0); // change status bar status
                m_bStatusBar = !m_bStatusBar;
            }
        }

        else if (obs.bReadInteger(psz_statusposreadout, m_nStatusPosReadout));
        else if (obs.bReadInteger(psz_statuspitchreadout, m_nStatusPitchReadout));
        else if (obs.bReadBool(psz_toolbar, b)) {
            if (b != AdvancedToolBarVisible()) {
				// change toolbar status
                BOOL bAdvanced = b;
                ShowControlBar(GetControlBar(IDR_BAR_BASIC),!bAdvanced, TRUE); 
                ShowControlBar(GetControlBar(IDR_BAR_ADVANCED), bAdvanced, TRUE); 
            }
        } else if (obs.bReadBool(psz_taskbar, b)) {
            if (b != TaskBarVisible()) {
				// change taskbar status
                ShowControlBar(GetControlBar(ID_VIEW_TASKBAR),b, TRUE); 
            }
        } else if (obs.bReadBool(psz_toneAbove, b)) {
            //SDM 1.5Test8.2
            if (b != m_bToneAbove) {
                m_bToneAbove = !m_bToneAbove;

                if (m_bToneAbove) {
                    CGraphWnd::m_anAnnWndOrder[1] = TONE;
                    CGraphWnd::m_anAnnWndOrder[2] = PHONETIC;
                } else {
                    CGraphWnd::m_anAnnWndOrder[1] = PHONETIC;
                    CGraphWnd::m_anAnnWndOrder[2] = TONE;
                }
                SendMessageToMDIDescendants(WM_USER_GRAPH_ORDERCHANGED, 0, 0L);
            }
        }

        else if (obs.bReadBool(psz_scrollzoom, m_bScrollZoom));
        else if (obs.bReadInteger(psz_graphUpdateMode, m_nGraphUpdateMode));
        else if (obs.bReadBool(psz_bAnimate, m_bAnimate));
        else if (obs.bReadInteger(psz_animationRate, m_nAnimationRate));
        else if (obs.bReadInteger(psz_cursorMode, nValue)) {
            m_nCursorAlignment = (ECursorAlignment) nValue;
        } else if (m_colors.ReadProperties(obs));
        else if (m_fnKeys.ReadProperties(obs));
        else if (m_grid.ReadProperties(obs));
        else if (m_waveformGeneratorSettings.ReadProperties(obs));
        else if (obs.bReadInteger(psz_captionstyle, m_nCaptionStyle));
        else if (m_parseParmDefaults.ReadProperties(obs));
        else if (m_segmentParmDefaults.ReadProperties(obs));
        else if (m_pitchParmDefaults.ReadProperties(obs));
        else if (m_musicParmDefaults.ReadProperties(obs));
        else if (m_intensityParmDefaults.ReadProperties(obs));
        else if (m_spectrumParmDefaults.ReadProperties(obs));
        else if (m_spectrogramParmDefaults.ReadPropertiesA(obs));
        else if (m_snapshotParmDefaults.ReadPropertiesB(obs));
        else if (obs.bReadBeginMarker(psz_graphfontarray)) {
            for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
                UINT u = 0;
                CSaString str;
                if (ReadStreamString(obs,psz_graphfont,str)) {
                    m_GraphFontFaces.SetAtGrow(nLoop, str);
                }
                if (obs.bReadUInt(psz_graphfontsize, u)) {
                    m_GraphFontSizes.SetAtGrow(nLoop, u);
                }
            }
            obs.SkipToEndMarker(psz_graphfontarray);
        }

        //SDM 1.06.6U5 read if there is an active child save its maximized, height, and width state as the default
        else if (obs.bReadBool(psz_bMaxView, m_bDefaultMaximizeView));
        else if (obs.bReadInteger(psz_HeightView, m_nDefaultHeightView));
        else if (obs.bReadInteger(psz_WidthView, m_nDefaultWidthView));
        else if (obs.bReadBool(psz_autosave, m_bAutoSave));
        else if (obs.bEnd(psz_mainframe)) {
            break;
        }
    }
    return TRUE;
}

static LPCSTR psz_defaultviewconfig = "defaultviewconfig";

//********************************************************************
//
//********************************************************************
BOOL CMainFrame::ReadDefaultView(CObjectIStream & obs) {

    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_defaultviewconfig)) {
        return FALSE;
    }

    if (m_pDefaultViewConfig!=NULL) {
		// always start from scratch
        delete m_pDefaultViewConfig;    
    }
    m_pDefaultViewConfig = new CSaView();
    m_nPermDefaultLayout = 0;
    m_szPermDefaultGraphs  = "";

    while (!obs.bAtEnd()) {
        if ((m_pDefaultViewConfig!=NULL) ? m_pDefaultViewConfig->ReadProperties(obs, FALSE) : FALSE) {
            if ((m_pDefaultViewConfig->GetLayout() < ID_LAYOUT_FIRST) ||
                (m_pDefaultViewConfig->GetLayout() > ID_LAYOUT_LAST) ||
                (m_pDefaultViewConfig->GetGraphIDs()[0] == 0)) {
                // This is a corrupted default view
                delete m_pDefaultViewConfig;
                m_pDefaultViewConfig = NULL;
            } else {
                m_nPermDefaultLayout = m_pDefaultViewConfig->GetLayout();
                m_szPermDefaultGraphs = m_pDefaultViewConfig->GetGraphsDescription();
            }
        } else if (obs.bEnd(psz_defaultviewconfig)) {
            break;
        }
    }

    // Now that documents and views have been opened, activate the top window
    // and maximize it if it had been when SA was closed.
    CSaView * pviewT = ((CSaApp *)AfxGetApp())->GetViewTop();
    if (pviewT) {
        pviewT->ShowInitialTopState();
    }

    WriteDefaultViewToTempFile();

    return TRUE;
}

//********************************************************************
//
//********************************************************************
void CMainFrame::WriteDefaultView(CObjectOStream & obs) {

	if ((m_bDefaultViewExists) && (m_pDefaultViewConfig!=NULL)) {
        obs.WriteBeginMarker(psz_defaultviewconfig);
		// We don't use this anymore but the old code expected/required it
        obs.WriteString(psz_defaultgraphlist, "");   
        m_pDefaultViewConfig->WriteProperties(obs);
        obs.WriteEndMarker(psz_defaultviewconfig);
    }
}

//********************************************************************
// WriteDefaultViewToTempFile()
// writes defaultView properties to predefined temp file
// WARNING ....read destroys temp file
//********************************************************************
void CMainFrame::WriteDefaultViewToTempFile() {
    
	CSaString szPath;
    FileUtils::GetTempDir(_MAX_PATH,szPath.GetBuffer(_MAX_PATH));
    szPath.ReleaseBuffer();

	// Use thread id to keep different threads from interfering with this file
	CSaString szTmpDfltSettingsFile;
    szTmpDfltSettingsFile.Format(_T("~!SA!~ %08X.tmp"), AfxGetThread()->m_nThreadID); 
    szPath += szTmpDfltSettingsFile;

    try {
        if (m_pDefaultViewConfig!=NULL) {
			CObjectOStream obs(szPath.utf8().c_str());
			obs.WriteBool(psz_bMaxView,  m_bDefaultMaximizeView);
			obs.WriteInteger(psz_HeightView, m_nDefaultHeightView);
			obs.WriteInteger(psz_WidthView, m_nDefaultWidthView);
			m_pDefaultViewConfig->WriteProperties(obs);
			obs.getIos().close();
			m_bDefaultViewExists = true;
		}
    } catch (...) {
    }
}

//********************************************************************
// ReadDefaultViewFromTempFile()
// writes defaultView properties to predefined temp file
// WARNING ....read destroys temp file
//********************************************************************
void CMainFrame::ReadDefaultViewFromTempFile() {

	CSaString szPath;
    FileUtils::GetTempDir(_MAX_PATH, szPath.GetBuffer(_MAX_PATH));
    szPath.ReleaseBuffer();

	// Use thread id to keep different threads from interfering with this file
    CSaString szTmpDfltSettingsFile;
    szTmpDfltSettingsFile.Format(_T("~!SA!~ %08X.tmp"), AfxGetThread()->m_nThreadID); 

    szPath += szTmpDfltSettingsFile;

    try {
        CObjectIStream obs(szPath.utf8().c_str());

        obs.bReadBool(psz_bMaxView, m_bDefaultMaximizeView);
        obs.bReadInteger(psz_HeightView, m_nDefaultHeightView);
        obs.bReadInteger(psz_WidthView, m_nDefaultWidthView);
        if (m_pDefaultViewConfig!=NULL) {
            delete m_pDefaultViewConfig;
        }
        m_pDefaultViewConfig = new CSaView();
        m_pDefaultViewConfig->ReadProperties( obs, FALSE);
        obs.Close();
        std::string szUtf8 = szPath.utf8();
        remove(szUtf8.c_str());
    } catch (...) {
    }
}

// SDM 1.5Test8.5
/***************************************************************************/
// CMainFrame::OnActivateApp delete display plot on loss of focus
/***************************************************************************/
void CMainFrame::OnActivateApp(BOOL bActive, DWORD dwThreadID) {
    CMDIFrameWnd::OnActivateApp(bActive, dwThreadID);
    if ((!bActive) && (m_pDisplayPlot!=NULL)) {
        delete m_pDisplayPlot;
        m_pDisplayPlot = NULL;
    }
}

// SDM 1.5Test10.3
/***************************************************************************/
// CMainFrame::OnWindowTileHorz set window order then tile
/***************************************************************************/
void CMainFrame::OnWindowTileHorz() {
    CDocList doclst; // list of currently open docs
    int nLoop;
    int maxID = -1;

    for (CSaDoc * pdoc = doclst.pdocFirst(); pdoc; pdoc = doclst.pdocNext()) {
        if (pdoc->GetID() > maxID) {
            maxID = pdoc->GetID();
        }
    }
    for (nLoop = maxID; nLoop >= 0; nLoop--) {
        for (CSaDoc * pdoc = doclst.pdocFirst(); pdoc; pdoc = doclst.pdocNext()) {
            if (pdoc->GetID() == nLoop) {
                POSITION pos = pdoc->GetFirstViewPosition();
                CView * pFirstView = pdoc->GetNextView(pos);
                CWnd * pWnd = pFirstView->GetParent(); // MDIChildWnd
                pWnd->SetWindowPos(&wndTop,0,0,0,0,SWP_NOMOVE+SWP_NOSIZE+SWP_NOREDRAW);
                break;
            }
        }
    }
    MDITile(MDITILE_HORIZONTAL);
}

// SDM 1.5Test10.3
/***************************************************************************/
// CMainFrame::OnWindowTileVert set window order then tile
/***************************************************************************/
void CMainFrame::OnWindowTileVert() {
    CDocList doclst; // list of currently open docs
    int nLoop;
    int maxID = -1;

    for (CSaDoc * pdoc = doclst.pdocFirst(); pdoc; pdoc = doclst.pdocNext()) {
        if (pdoc->GetID() > maxID) {
            maxID = pdoc->GetID();
        }
    }
    for (nLoop = maxID; nLoop >= 0; nLoop--) {
        for (CSaDoc * pdoc = doclst.pdocFirst(); pdoc; pdoc = doclst.pdocNext()) {
            if (pdoc->GetID() == nLoop) {
                POSITION pos = pdoc->GetFirstViewPosition();
                CView * pFirstView = pdoc->GetNextView(pos);
                CWnd * pWnd = pFirstView->GetParent(); // MDIChildWnd
                pWnd->SetWindowPos(&wndTop,0,0,0,0,SWP_NOMOVE+SWP_NOSIZE+SWP_NOREDRAW);
                break;
            }
        }
    }
    MDITile(MDITILE_VERTICAL);
}

/***************************************************************************/
// CMainFrame::NotifyFragmentDone  respond to fragment processing done
/***************************************************************************/
void CMainFrame::NotifyFragmentDone(void * /*pCaller*/) {
    // kg - prevent exception
    if ((CDlgPlayer::IsLaunched()) && (GetPlayer(false)->IsWindowVisible())) {
        GetPlayer(false)->EnableSpeedSlider();
    }
}

/***************************************************************************/
// CMainFrame::DestroyPlayer
/***************************************************************************/
void CMainFrame::DestroyPlayer() {
    if (m_pDlgPlayer!=NULL) {
        delete m_pDlgPlayer;
        m_pDlgPlayer = NULL;
    }
}

void CMainFrame::OnWaveformGenerator() {
    CDlgWaveformGenerator dlg;
    dlg.working = m_waveformGeneratorSettings;

    if (dlg.DoModal()!=IDOK) {
        return;
    }

    dlg.working.pcm.wf.nSamplesPerSec = _tcstol(dlg.m_szSamplingRate,NULL,10);
    ASSERT(dlg.working.pcm.wf.nSamplesPerSec > 0);
    dlg.working.pcm.wBitsPerSample = (unsigned short) _tcstol(dlg.m_szBits,NULL,10);
    ASSERT(dlg.working.pcm.wBitsPerSample == 8 || dlg.working.pcm.wBitsPerSample == 16);

    m_waveformGeneratorSettings = dlg.working;

    TCHAR szFile[_MAX_PATH];
    // create temp filename for synthesized waveform
    FileUtils::GetTempFileName(_T("WAV"), szFile, _MAX_PATH);
    if (m_waveformGeneratorSettings.Synthesize(szFile)) {
        CSaApp * pApp = (CSaApp *)(AfxGetApp());
        pApp->OpenWavFileAsNew(szFile);
    }
}

static CDlgSynthesis * s_pDlgSynthesis = NULL;
/***************************************************************************/
// CMainFrame::OnSynthesis Open synthesis dialog
/***************************************************************************/
void CMainFrame::OnSynthesis() {
    DestroySynthesizer();
    ASSERT(s_pDlgSynthesis == NULL);
    s_pDlgSynthesis = new CDlgSynthesis(_T("Synthesis"));
    if (s_pDlgSynthesis) {
        s_pDlgSynthesis->Create(this);
    }
}

/***************************************************************************/
// CMainFrame::OnUpdateSynthesis Menu update
/***************************************************************************/
void CMainFrame::OnUpdateSynthesis(CCmdUI * /*pCmdUI*/) {
    // pCmdUI->Enable(EXPERIMENTAL_ACCESS); // enable if control key pressed
}

/***************************************************************************/
// CMainFrame::DestroySynthesizer
/***************************************************************************/
void CMainFrame::DestroySynthesizer() {
    if (s_pDlgSynthesis) {
        s_pDlgSynthesis->DestroyWindow();
        delete s_pDlgSynthesis;
        s_pDlgSynthesis = NULL;
    }
}

/***************************************************************************/
// CMainFrame::OnSynthesisKlattIpa Open synthesis dialog
/***************************************************************************/
void CMainFrame::OnSynthesisKlattIpa() {
    CDlgKlattAll::CreateSynthesizer(this, CDlgKlattAll::kSegment);
}

/***************************************************************************/
// CMainFrame::OnSynthesisKlattFragments Open synthesis dialog
/***************************************************************************/
void CMainFrame::OnSynthesisKlattFragments() {
    CDlgKlattAll::CreateSynthesizer(this, CDlgKlattAll::kFragment);
}

/***************************************************************************/
// CMainFrame::OnSynthesis Open synthesis dialog
/***************************************************************************/
void CMainFrame::OnSynthesisVocalTract() {
    CDlgVocalTract::CreateSynthesizer(this);
}

/******************************************************************************
* used to create a new document, record and overlay in it and load it
* in another document.
******************************************************************************/
void CMainFrame::OnRecordOverlay() {
    // find the view that launched this message
    CSaView * pSourceView = GetCurrSaView();
    if (!pSourceView) {
        return;
    }

    // if the focused graph is mergeable, bring up a list of all other
    // graphs that can be merged with it, (m_pPickOverlay) then
    // call AssignGraph to merge them in.
    if ((!pSourceView->GetFocusedGraphWnd()) ||
            (!CGraphWnd::IsMergeableGraph(pSourceView->GetFocusedGraphWnd()))) {
        return;
    }

    CSaApp * pApp = (CSaApp *)AfxGetApp();
    CSaDoc * pDoc = (CSaDoc *)pApp->OpenBlankView(true);

    if (pDoc!=NULL) {
        //hide the temporary overlay document
        MDIActivate(pSourceView->pwndChildFrame());

        // launch recorder in this new view
        pDoc->SetTempOverlay(); //mark this for reuse.
        POSITION pos = pDoc->GetFirstViewPosition();
        CSaDoc * pSourceDoc = pSourceView->GetDocument();

        CAlignInfo alignInfo;
        pSourceDoc->GetAlignInfo(alignInfo);

        if (pos) {
            CSaView * pView = (CSaView *)pDoc->GetNextView(pos);
            if (pView->IsKindOf(RUNTIME_CLASS(CSaView))) {
                if (!CDlgAutoRecorder::IsLaunched()) {
                    int wholeFile = (AfxGetApp()->GetProfileInt(L"AutoRecorder",L"WholeFile",0)!=0);
                    m_pAutoRecorder = new CDlgAutoRecorder(pDoc, pView, pSourceView, alignInfo, wholeFile);
                    if (m_pDisplayPlot!=NULL) {
                        m_pDisplayPlot->m_pModal = m_pAutoRecorder;
                    }
                    m_pAutoRecorder->Create(this);
                }
            }
        } else {
            pDoc->OnCloseDocument();
        }
    }
}

void CMainFrame::OnUpdateRecordOverlay(CCmdUI * pCmdUI) {
    // if the recorder is up, we can't restart
    BOOL bEnable = FALSE;

    {
        CSaView * pSourceView = GetCurrSaView();
        if (pSourceView) {
            // if the focused graph is mergeable, bring up a list of all other
            // graphs that can be merged with it, (m_pPickOverlay) then
            // call AssignGraph to merge them in.
            if ((pSourceView->GetFocusedGraphWnd()) &&
                    (CGraphWnd::IsMergeableGraph(pSourceView->GetFocusedGraphWnd()))) {
                bEnable = TRUE;
            }
        }
    }

    pCmdUI->Enable(bEnable);
}

BOOL CMainFrame::OnCopyData(CWnd * /*pWnd*/, COPYDATASTRUCT * pCopyDataStruct) {

    // length is in bytes
    int len = pCopyDataStruct->cbData;
    wchar_t * buffer = new wchar_t[(len/2)+1];
    memset(buffer,0,len+2);
    memcpy(buffer,pCopyDataStruct->lpData,len);

    CSaApp * pApp = (CSaApp *)AfxGetApp();
    pApp->OpenDocumentFile(buffer);

    return TRUE;
}

// toolbar on/off
BOOL CMainFrame::AdvancedToolBarVisible() {
    return m_wndToolBarAdvanced.IsVisible();
}

// taskbar on/off
BOOL CMainFrame::TaskBarVisible() {
    return m_wndTaskBar.IsVisible();
}

const CSaString CMainFrame::GetPermGraphNames(void) {
    return m_szPermDefaultGraphs;
}

const CSaString CMainFrame::GetTempGraphNames(void) {
    return m_szTempDefaultGraphs;
}
const UINT CMainFrame::GetPermLayout(void) {
    return m_nPermDefaultLayout;
}
const UINT CMainFrame::GetTempLayout(void) {
    return m_nTempDefaultLayout;
}
BOOL CMainFrame::GetShowStartupDlg(void) {
    return m_bShowStartupDlg;
}
int CMainFrame::GetStartDataMode(void) {
    return m_nStartDataMode;
}
BOOL CMainFrame::GetSaveOpenFiles(void) {
    return m_bSaveOpenFiles;
}
BOOL CMainFrame::GetShowAdvancedAudio(void) {
    return m_bShowAdvancedAudio;
}
int CMainFrame::GetAudioSyncAlgorithm() {
	return m_nAlgorithm;
}
void CMainFrame::SetShowStartupDlg(BOOL bShow) {
    m_bShowStartupDlg = bShow;
}
void CMainFrame::SetStartDataMode(int nMode) {
    m_nStartDataMode = nMode;
}

float CMainFrame::GetActiveBreakWidth() {
	return activeBreakWidth;
}

void CMainFrame::SetWordBreakWidth(float value) {
	m_parseParmDefaults.fBreakWidth = value;
	activeBreakWidth = value;
}
void CMainFrame::SetPhraseBreakWidth(float value) {
	m_parseParmDefaults.fPhraseBreakWidth = value;
	activeBreakWidth = value;
}
void CMainFrame::SetMaxThreshold(int value) {
	m_parseParmDefaults.nMaxThreshold = value;
}
void CMainFrame::SetMinThreshold(int value) {
	m_parseParmDefaults.nMinThreshold = value;
}

float CMainFrame::GetWordBreakWidth() {
	return m_parseParmDefaults.fBreakWidth;
}
float CMainFrame::GetPhraseBreakWidth() {
	return m_parseParmDefaults.fPhraseBreakWidth;
}
int CMainFrame::GetMaxThreshold() {
	return m_parseParmDefaults.nMaxThreshold;
}
int CMainFrame::GetMinThreshold() {
	return m_parseParmDefaults.nMinThreshold;
}

CSegmentParm * CMainFrame::GetSegmentParm() {
    return &m_segmentParmDefaults;
}
void CMainFrame::SetSegmentParams( float width, int change, int zc) {
	m_segmentParmDefaults.fSegmentWidth = width;
	m_segmentParmDefaults.nChThreshold = change;
	m_segmentParmDefaults.nZCThreshold = zc;
}

const CPitchParm * CMainFrame::GetPitchParmDefaults() const {
    return &m_pitchParmDefaults;
}
void CMainFrame::SetPitchParmDefaults(const CPitchParm & cParm) {
    m_pitchParmDefaults = cParm;
}
const CMusicParm * CMainFrame::GetMusicParmDefaults() const {
    return &m_musicParmDefaults;
}
void CMainFrame::SetMusicParmDefaults(const CMusicParm & cParm) {
    m_musicParmDefaults = cParm;
}
void CMainFrame::SetMusicParmCalcBounds( int lowerBound, int upperBound) {
	m_musicParmDefaults.nCalcLowerBound = lowerBound;
	m_musicParmDefaults.nCalcUpperBound = upperBound;
}
const CIntensityParm & CMainFrame::GetCIntensityParmDefaults() const {
    return m_intensityParmDefaults;
}
void CMainFrame::SetCIntensityParmDefaults(const CIntensityParm & cParm) {
    m_intensityParmDefaults = cParm;
}
CFormantParm * CMainFrame::GetFormantParmDefaults() {
    return &m_formantParmDefaults;
}
CSpectrumParm * CMainFrame::GetSpectrumParmDefaults() {
    return &m_spectrumParmDefaults;
}
const CSpectroParm * CMainFrame::GetSpectrogramParmDefaults() const {
    return &m_spectrogramParmDefaults;
}
const CSpectroParm * CMainFrame::GetSnapshotParmDefaults() const {
    return &m_snapshotParmDefaults;
}
const CSaView * CMainFrame::pDefaultViewConfig() {
    return m_pDefaultViewConfig;
}
BOOL CMainFrame::DefaultIsValid() {
    return m_pDefaultViewConfig != NULL;
}
BOOL CMainFrame::IsDefaultViewMaximized() {
    return m_bDefaultMaximizeView;
}
CPoint CMainFrame::GetDefaultViewSize() {
    return CPoint(m_nDefaultWidthView, m_nDefaultHeightView);
}
void CMainFrame::DisplayPlot(CDisplayPlot * pPlot) {
    m_pDisplayPlot = pPlot;
}
CSaString CMainFrame::GetFontFace(int nIndex) {
    // return font face string
    return m_GraphFontFaces.GetAt(nIndex);
}
int CMainFrame::GetFontSize(int nIndex) {
    // return font size
    return (int)m_GraphFontSizes.GetAt(nIndex);
}
void CMainFrame::SetFontFace(int nIndex, LPCTSTR pString) {
    // set font face string
    if (_tcslen(pString)) {
        m_GraphFontFaces.SetAtGrow(nIndex, pString);
    }
}
void CMainFrame::SetFontSize(int nIndex, int nSize) {
    // set font size
    if (nSize) {
        m_GraphFontSizes.SetAtGrow(nIndex, (UINT)nSize);
    }
}

CDataStatusBar * CMainFrame::GetDataStatusBar() {
	// return pointer to status bar object
    return &m_dataStatusBar;   
}

CProgressStatusBar * CMainFrame::GetProgressStatusBar() {
	// return pointer to progress status bar object
	return &m_progressStatusBar;   
}

int CMainFrame::ComputeNumberOfViews(int nNum) {
    m_nNumberOfViews += nNum;
    return m_nNumberOfViews;
}

int CMainFrame::GetCaptionStyle() {
    // return graph caption style
    return m_nCaptionStyle;
}

Colors * CMainFrame::GetColors() {
    return &m_colors;
}

void CMainFrame::SetColors( Colors value) {
    m_colors = value;
}

CGrid * CMainFrame::GetGrid() {
    return &m_grid;
}

BOOL CMainFrame::IsStatusBar() {
    return m_bStatusBar;
}

BOOL CMainFrame::IsScrollZoom() {
    return m_bScrollZoom;
}

int  CMainFrame::GetStatusPosReadout() {
    return m_nStatusPosReadout;
}

int  CMainFrame::GetStatusPitchReadout() {
    return m_nStatusPitchReadout;
}

int  CMainFrame::GetGraphUpdateMode() {
    return m_nGraphUpdateMode;
}

void CMainFrame::SetGraphUpdateMode(int nMode) {
    m_nGraphUpdateMode = nMode;
}

BOOL CMainFrame::IsAnimationRequested() {
    return m_bAnimate;
}

int CMainFrame::GetAnimationFrameRate() {
    return m_nAnimationRate;
}

ECursorAlignment CMainFrame::GetCursorAlignment() {
    // get cursor snap mode
    return m_nCursorAlignment;
}

void CMainFrame::AppMessage(WPARAM wParam, LPARAM lParam) {
    this->SendMessage(WM_USER_APP_MESSAGE,wParam, lParam);
    SendMessageToMDIDescendants(WM_USER_APP_MESSAGE, wParam, lParam);
};

void CMainFrame::SetPreviewFlag() {
    m_bPrintPreviewInProgress = TRUE;
};

void CMainFrame::ClearPreviewFlag() {
    m_bPrintPreviewInProgress = FALSE;
};

CProcess * CMainFrame::GetWbProcess(int nProcess, int nFilter) {
    return m_apWbProcess[nProcess][nFilter];   // return pointer to workbench process
}

int CMainFrame::GetWbFilterID(int nProcess, int nFilter) {
    return m_aWbFilterID[nProcess][nFilter];   // return filter ID
}

void CMainFrame::SetNewMenu(const HMENU hMenu) {
    m_hNewMenu = hMenu;
}

HMENU CMainFrame::GetNewMenu() const {
    return m_hNewMenu;
}

void CMainFrame::SetNewAccel(const HACCEL hAccel) {
    m_hNewAccel = hAccel;
}

HACCEL CMainFrame::GetNewAccel() const {
    return m_hNewAccel;
}

void CMainFrame::SetPopup(int nPopup) {
    m_nPopup = nPopup;
};

int CMainFrame::GetPopup() const {
	int nID = IDR_SPEECHANALYZER_POPUP;
    return (m_nPopup!=0) ? m_nPopup : nID;
};

void CMainFrame::SetToolSettings(CToolSettings settings, bool fullView) {
    if (fullView) {
        toolSettings = settings;
    } else {
        toolSettings.m_bShowAdvancedAudio = settings.m_bShowAdvancedAudio;
		toolSettings.m_nAlgorithm = settings.m_nAlgorithm;
    }
}

/**
* callback for EnumChildWindows in OnTimer function
*/
BOOL CALLBACK AutosaveTimerChildProc(HWND hwnd,  LPARAM) {
    if (DYNAMIC_DOWNCAST(CSaView,CWnd::FromHandle(hwnd)) != NULL) {
        ::PostMessage(hwnd,WM_USER_AUTOSAVE,0,0);
    }
    return(TRUE);
}

void CMainFrame::OnTimer(UINT nIDEvent) {
    //TRACE("OnTimer %d\n",nIDEvent);
    if (nIDEvent == ID_TIMER_AUTOSAVE) {
        // if we are playing something, defer autosave for performance reasons
        if (IsPlayerPlaying()) {
            return;
        }
        ::EnumChildWindows(m_hWnd, AutosaveTimerChildProc, NULL);
    } else {
        CWnd::OnTimer(nIDEvent);
    }
}

void CMainFrame::InitializeAutoSave() {
    if (m_bAutoSave) {
        TRACE("enabling autosave\n");
        SetTimer(ID_TIMER_AUTOSAVE, AUTOSAVE_TIMER, NULL);  // start the timer
    } else {
        TRACE("disabling autosave\n");
        KillTimer(ID_TIMER_AUTOSAVE);
    }
}

void CMainFrame::OnAutoSaveOn() {
    CMenu * pMenu = GetMenu();
    UINT state = pMenu->GetMenuState(ID_AUTOSAVE_ON,MF_BYCOMMAND);
    if ((m_bAutoSave)&&((state&MF_CHECKED)==MF_CHECKED)) {
        return;
    }
    TRACE("enabling autosave\n");
    pMenu->CheckMenuItem(ID_AUTOSAVE_ON, MF_CHECKED);
    pMenu->CheckMenuItem(ID_AUTOSAVE_OFF, MF_UNCHECKED);
    SetTimer(ID_TIMER_AUTOSAVE, AUTOSAVE_TIMER, NULL);  // start the timer
    m_bAutoSave = TRUE;
}

void CMainFrame::OnAutoSaveOff() {
    CMenu * pMenu = GetMenu();
    UINT state = pMenu->GetMenuState(ID_AUTOSAVE_OFF,MF_BYCOMMAND);
    if ((!m_bAutoSave)&&((state&MF_CHECKED)==MF_CHECKED)) {
        return;
    }
    TRACE("disabling autosave\n");
    pMenu->CheckMenuItem(ID_AUTOSAVE_ON, MF_UNCHECKED);
    pMenu->CheckMenuItem(ID_AUTOSAVE_OFF, MF_CHECKED);
    KillTimer(ID_TIMER_AUTOSAVE);
    CAutoSave::CleanAll();
    m_bAutoSave = FALSE;
}

void CMainFrame::SetShowFormants(bool value) {
    m_spectrogramParmDefaults.SetShowFormants(value);
    m_snapshotParmDefaults.SetShowFormants(value);
}

LRESULT CMainFrame::OnUpdatePlayer(WPARAM wParam, LPARAM /*lParam*/) {
    if (m_pDlgEditor!=NULL) {
        m_pDlgEditor->UpdatePlayer();
    }
    if (CDlgAutoRecorder::IsLaunched()) {
        if (wParam ==2) {
            m_pAutoRecorder->EndPlayback();
        }
    }
    return 0;
}
