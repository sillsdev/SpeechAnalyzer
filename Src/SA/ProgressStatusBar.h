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

class CProgressStatusBar : public CStatusBar  {

public:
    CProgressStatusBar();
    virtual ~CProgressStatusBar();

    // initialisation
    void Init(); 
    // save the process owner
    virtual void SetProgressOwner(CProcess * pProcess, void * pCaller, ProcessorType processorType); 
    virtual void ClearProgressOwner(CProcess * pProcess);
    virtual CProcess * GetProgressOwner();   // return the process owner
    void * GetProgressCaller();              // return the process caller
    void InitProgress();                    // initialisation of progress bar
    virtual void SetProgress( int nVal);     // set progress bar
    int  GetProgress();                     // get progress
    BOOL SetPaneText(int nPaneID, LPCTSTR lpszText, BOOL bUpdate = TRUE);
    void GetItemRect(int nPaneID, LPRECT lpRect);
    void SetIsPrintingFlag(bool isPrinting);
    virtual void DelayShow();
    BOOL InProcessMessageLoop() const;

protected:
    void MessageLoop(); // do windows message loop

    DECLARE_MESSAGE_MAP()

private:
    CFont * m_pFont;                // status bar font
    map<CProcess*, void*> callers;
    list<CProcess*> owners;
    CProgressBar m_ProgressBar;     // progress bar object
    bool isPrinting;
    int m_nInMessageLoop;
};

#endif
