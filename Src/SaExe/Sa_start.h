/////////////////////////////////////////////////////////////////////////////
// sa_start.h:
// Interface of the CStartModeDlg (dialog) class.
//
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//   05/16/2000 - Original
//       RLJ Created so that CStartModeDlg could be moved here, to reduce
//           Sa_dlg.OBJ size enough to allow LINK/DEBUG
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_START_H

#include "resource.h"

#define _SA_START_H

/////////////////////////////////////////////////////////////////////////////
// CStartModeDlg dialog
// RLJ - April 18, 2000

class CSaApp;

class CStartModeDlg : public CDialog {
    // Construction
public:
    CStartModeDlg(CWnd * pParent = NULL); // standard constructor

    // Dialog Data
    //{{AFX_DATA(CStartModeDlg)
    enum { IDD = IDD_START_MODE };
    CListBox  m_lbRecentFiles;
    BOOL  m_nDontShowAgain;
    BOOL  m_bShowDontShowAgainOption;
    int   m_nDataMode;
    //}}AFX_DATA

    // Attributes
private:
    CSaApp * pSaApp;

    CFont m_Font;

    // Implementation
public:
    void OnHelpStartMode();
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support

    bool Cleanup();

    // Generated message map functions
    //{{AFX_MSG(CStartModeDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnStartModeRecord();
    afx_msg void OnCloseButton();
    afx_msg void OnDblclkRecentlist();
    afx_msg void OnOk();
    afx_msg void OnPlay();
    afx_msg void OnSelchangeRecentlist();
    afx_msg void OnStop();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif //_SA_START_H
