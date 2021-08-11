/////////////////////////////////////////////////////////////////////////////
// sa_sbar.h:
// Interface of the CSaDataStatusBar
//           CSaProgressStatusBar classes.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef PROGRESSSTATUSBAR_H
#define PROGRESSSTATUSBAR_H

#include "sa_dlWnd.h"

//###########################################################################
// CProgressStatusBar window

/////////////////////////////////////////////////////////////////////////////
// CProgressStatusBar defines

class CProgressStatusBar : public CStatusBar {

public:
    CProgressStatusBar();
    virtual ~CProgressStatusBar();

    void Init(); // initialisation
    void SetProcessOwner(void * pProcess, void * pCaller, int nProcessID = -1); // save the process owner
    CProcess * GetProcessOwner();   // return the process owner
    void * GetProcessCaller();      // return the process caller
    void InitProgress();            // initialisation of progress bar
    void SetProgress(int nVal);     // set progress bar
    int  GetProgress();             // get progress
    BOOL SetPaneText(int nPaneID, LPCTSTR lpszText, BOOL bUpdate = TRUE);
    void GetItemRect(int nPaneID, LPRECT lpRect);
    void SetIsPrintingFlag(BOOL isPrinting);
    virtual void DelayShow();
    BOOL InProcessMessageLoop() const;

protected:
    void MessageLoop(); // do windows message loop

    DECLARE_MESSAGE_MAP()

private:
    CFont * m_pFont;                // status bar font
    CProcess * m_pProcessOwner;     // process owner
    void * m_pProcessCaller;        // process caller
    CProgressBar m_ProgressBar;     // progress bar object
    BOOL m_bIsPrinting;
    int m_nInMessageLoop;
};

#endif
