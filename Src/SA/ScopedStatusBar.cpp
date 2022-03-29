#include "StdAfx.h"
#include "ScopedStatusBar.h"
#include "MainFrm.h"
#include "ProgressStatusBar.h"

CScopedStatusBar::CScopedStatusBar( UINT resourceID) {
	CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
	CProgressStatusBar * pStatusBar = (CProgressStatusBar *)pMainFrame->GetProgressStatusBar();
	// display status bar message
    if (!pStatusBar->GetProgressOwner()) {
		// show the progress status bar
        pMainFrame->ShowDataStatusBar(false); 
    }

    CString szText;
    szText.LoadString(resourceID);
    pStatusBar->SetPaneText(ID_PROGRESSPANE_1, szText);
    pStatusBar->SetPaneText(ID_PROGRESSPANE_3, _T(""));
}

CScopedStatusBar::~CScopedStatusBar() {
	// restore data status bar
	CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
	pMainFrame->ShowDataStatusBar(true); 
}

void CScopedStatusBar::SetProgress( int progress) {
	CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
	CProgressStatusBar * pStatusBar = (CProgressStatusBar *)pMainFrame->GetProgressStatusBar();
    pStatusBar->SetProgress(progress);
}

int CScopedStatusBar::GetProgress() {
	CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
	CProgressStatusBar * pStatusBar = (CProgressStatusBar *)pMainFrame->GetProgressStatusBar();
    return pStatusBar->GetProgress();
}
