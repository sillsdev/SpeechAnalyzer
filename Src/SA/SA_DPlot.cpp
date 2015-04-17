// sa_dplot.cpp : implementation file
// Implementation of the CDisplayPlot class
//
// Author: Steve MacLean
// copyright 1999 JAARS Inc. SIL
//
// Revision History
// 1.5Test8.5
//        SDM Original Version
// 1.5Test10.0
//        SDM changed default graph layout
/////////////////////////////////////////////////////////////////////////////
//

#include "stdafx.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "sa_doc.h"
#include "sa.h"
#include "sa_dplot.h"
#include "doclist.h"
#include "DlgAutoRecorder.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDisplayPlot

CDisplayPlot::CDisplayPlot(CString & szPlot) {

    m_pModal = NULL;
    m_pMainFrame = (CMainFrame *) AfxGetMainWnd();

    // adjust app
    // disable status
    m_bStatusBar = m_pMainFrame->m_bStatusBar;
    if (FALSE != m_bStatusBar) {
		// change statusbar status
        m_pMainFrame->SendMessage(WM_COMMAND, ID_VIEW_STATUS_BAR, 0); 
        m_pMainFrame->m_bStatusBar = !m_pMainFrame->m_bStatusBar;
    }
    // disable toolbar
	// change toolbar status
    m_bToolBar = m_pMainFrame->AdvancedToolBarVisible();
    m_pMainFrame->ShowControlBar(m_pMainFrame->GetControlBar(IDR_BAR_BASIC), FALSE, TRUE); 
    m_pMainFrame->ShowControlBar(m_pMainFrame->GetControlBar(IDR_BAR_AUDIOSYNC), FALSE, TRUE); 
    m_pMainFrame->ShowControlBar(m_pMainFrame->GetControlBar(IDR_BAR_ADVANCED), FALSE, TRUE); 

    // disable toolbar
    m_bTaskBar = m_pMainFrame->TaskBarVisible();
	// change toolbar status
    m_pMainFrame->ShowControlBar(m_pMainFrame->GetControlBar(ID_VIEW_TASKBAR), FALSE, TRUE); 

    // disable scrolling zoom
    m_bScrollZoom = m_pMainFrame->m_bScrollZoom;
    if (FALSE != m_bScrollZoom) {
        m_pMainFrame->m_bScrollZoom = !m_pMainFrame->m_bScrollZoom;
        // tell about the change to all views
        m_pMainFrame->SendMessageToMDIDescendants(WM_USER_VIEW_SCROLLZOOMCHANGED, m_bScrollZoom, 0L);
    }
    // set graph caption style
    m_nCaptionStyle = m_pMainFrame->m_nCaptionStyle;
    if (0 != m_nCaptionStyle) {
        m_pMainFrame->m_nCaptionStyle = 0;
        // tell about the change to all views
        m_pMainFrame->SendMessageToMDIDescendants(WM_USER_GRAPH_STYLECHANGED, 0, 0L);
    }
    // set menu
    m_hNewMenu = m_pMainFrame->GetNewMenu();

    CMenu Menu;
    Menu.LoadMenu(IDR_SA_DISPLAYPLOT);
    m_pMainFrame->SetNewMenu(Menu.Detach());
    m_pMainFrame->OnUpdateFrameMenu(NULL);
    m_pMainFrame->DrawMenuBar();

    // disable accelerators
    m_hNewAccel = m_pMainFrame->GetNewAccel();

    m_pMainFrame->SetNewAccel(::LoadAccelerators(AfxFindResourceHandle(MAKEINTRESOURCE(IDR_SA_DISPLAYPLOT),RT_ACCELERATOR), MAKEINTRESOURCE(IDR_SA_DISPLAYPLOT)));

    // set popup
    m_nPopup = m_pMainFrame->GetPopup();
    m_pMainFrame->SetPopup(IDR_SA_DISPLAYPLOT_POPUP);

    // Tile or maximize
    CDocList List;
    if (!List.pdocFirst()) {
        m_pMainFrame->PostMessage(WM_COMMAND, ID_PROCESS_BATCH_COMMANDS, 0L);
        return;
    }
    if (List.pdocNext()) {
        m_pMainFrame->SendMessage(WM_COMMAND, ID_WINDOW_TILE_HORZ, 0); // tile documents
    } else {
        // Maximize
        WINDOWPLACEMENT WP;
        WP.length = sizeof(WINDOWPLACEMENT);
        m_pMainFrame->MDIGetActive()->GetWindowPlacement(&WP);
        WP.showCmd = SW_SHOWMAXIMIZED;
        m_pMainFrame->MDIGetActive()->SetWindowPlacement(&WP);
    }

    // adjust docs
    // layout 1
    m_pMainFrame->SendMessageToMDIDescendants(WM_COMMAND, ID_LAYOUT_1, 0L);
    // plot type
    szPlot.MakeUpper();
    if (szPlot.Find(_T("PITCH"))!=-1) {
        m_pMainFrame->SendMessageToMDIDescendants(WM_COMMAND, IDD_GRAPITCH, 0L);
    } else if (szPlot.Find(_T("WAVE"))!=-1) {
        m_pMainFrame->SendMessageToMDIDescendants(WM_COMMAND, IDD_RAWDATA, 0L);
    } else if (szPlot.Find(_T("SPECTRO"))!=-1) {
        m_pMainFrame->SendMessageToMDIDescendants(WM_COMMAND, IDD_SPECTROGRAM, 0L);
        m_pMainFrame->SendMessageToMDIDescendants(WM_COMMAND, ID_RESTART_PROCESS, 0L);
    }
    m_pMainFrame->SendMessageToMDIDescendants(WM_COMMAND, ID_REFERENCE_NONE, 0L);
    m_pMainFrame->SendMessageToMDIDescendants(WM_COMMAND, ID_PHONETIC_NONE, 0L);//SDM 1.5Test10.0
    m_pMainFrame->SendMessageToMDIDescendants(WM_COMMAND, ID_PHONEMIC_NONE, 0L);
    m_pMainFrame->SendMessageToMDIDescendants(WM_COMMAND, ID_TONE_NONE, 0L);
    m_pMainFrame->SendMessageToMDIDescendants(WM_COMMAND, ID_ORTHO_NONE, 0L);
    m_pMainFrame->SendMessageToMDIDescendants(WM_COMMAND, ID_GLOSS_NONE, 0L);
    m_pMainFrame->SendMessageToMDIDescendants(WM_COMMAND, ID_LEGEND_NONE, 0L);//SDM 1.5Test10.0
    m_pMainFrame->SendMessageToMDIDescendants(WM_COMMAND, ID_XSCALE_NONE, 0L);//SDM 1.5Test10.0
}

CDisplayPlot::~CDisplayPlot() {
    if (CDlgAutoRecorder::IsLaunched()) {
        m_pModal->SendMessage(WM_CLOSE, 0, 0);
        m_pModal = NULL;
    }
    // restore app
    // status
    if (FALSE != m_bStatusBar) {
        m_pMainFrame->SendMessage(WM_COMMAND, ID_VIEW_STATUS_BAR, 0); // change statusbar status
        m_pMainFrame->m_bStatusBar = !m_pMainFrame->m_bStatusBar;
    }

	// restore toolbar
    BOOL bAdvanced = m_bToolBar;
	CSaApp * pApp = (CSaApp*)AfxGetApp();
	int tbID = (pApp->IsAudioSync())?IDR_BAR_AUDIOSYNC:IDR_BAR_BASIC;
    m_pMainFrame->ShowControlBar(m_pMainFrame->GetControlBar(tbID),!bAdvanced, TRUE); 
    m_pMainFrame->ShowControlBar(m_pMainFrame->GetControlBar(IDR_BAR_ADVANCED), bAdvanced, TRUE);

    // restore taskbar
    m_pMainFrame->ShowControlBar(m_pMainFrame->GetControlBar(ID_VIEW_TASKBAR),!m_bTaskBar, TRUE); // change toolbar status

    // restore scrolling zoom
    if (FALSE != m_bScrollZoom) {
        m_pMainFrame->m_bScrollZoom = !m_pMainFrame->m_bScrollZoom;
        // tell about the change to all views
        m_pMainFrame->SendMessageToMDIDescendants(WM_USER_VIEW_SCROLLZOOMCHANGED, m_bScrollZoom, 0L);
    }
    // set graph caption style
    if (0 != m_nCaptionStyle) {
        m_pMainFrame->m_nCaptionStyle = m_nCaptionStyle;
        // tell about the change to all views
        m_pMainFrame->SendMessageToMDIDescendants(WM_USER_GRAPH_STYLECHANGED, 0, 0L);
    }
    // restore menu
    HMENU hMenu = m_pMainFrame->GetNewMenu();
    m_pMainFrame->SetNewMenu(m_hNewMenu);
    m_pMainFrame->OnUpdateFrameMenu(NULL);
    m_pMainFrame->DrawMenuBar();

    if (hMenu) {
        ::DestroyMenu(hMenu);
    }

    // restore Accel
    HACCEL hAccel = m_pMainFrame->GetNewAccel();
    m_pMainFrame->SetNewAccel(m_hNewAccel);

    if (hAccel) {
        ::FreeResource((HGLOBAL)hAccel);
    }

    // restore popup
    m_pMainFrame->SetPopup(m_nPopup);

    // Continue Batch Processing
    m_pMainFrame->PostMessage(WM_COMMAND, ID_PROCESS_BATCH_COMMANDS, 0L);
}


