/////////////////////////////////////////////////////////////////////////////
// DlgEditor.h:
// Definition of the CDlgEditor (dialog) classes.
//
// Author: Steve MacLean
// copyright 1999 JAARS Inc. SIL
//
// Revision History
// 1.06.5
//      SDM Original
// 1.06.6
//      SDM Added class CDlgAnnotationEdit class
// 1.06.6U4
//      SDM Added OnCancel to undo changes on ESC (Alt+F4)
// 1.5Test8.2
//      SDM moved CreateSafe to .CPP
// 1.5Test8.5
//      SDM CDlgEditor removed autohide restore
// 1.5Test10.0
//      SDM fix bug on close of TE which cancel previous change & ESC would not cancel see 1.06.6U4 & 1.5Test8.2
/////////////////////////////////////////////////////////////////////////////
#ifndef DLGEDITOR_H
#define DLGEDITOR_H

//###########################################################################
// CDlgEditor dialog
// Forward declarations

class CSegment;
class CMainFrame;
class CSaDoc;
class CSaView;

class CDlgEditor : public CDialog {
public:
    CDlgEditor(CWnd * pParent=NULL);

    BOOL CreateSafe(UINT nIDTemplate, CWnd * pParentWnd=NULL, WINDOWPLACEMENT * pWPL=NULL);
    CMainFrame * GetMainFrame() const;
    CSaView * GetView() const;
    CSaDoc * GetDoc() const;
    CSegment * GetSelectedSegment() const;
    void UpdateDialog();
    virtual BOOL PreTranslateMessage(MSG * pMsg);
    void OnHelpEdit();
    void UpdatePlayer();

    enum { IDD = IDD_EDITOR };
    CButton m_NextButton;
    CButton m_PreviousButton;
    CButton m_UpButton;
    CButton m_DownButton;

protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void * pExtra,AFX_CMDHANDLERINFO * pHandlerInfo);

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnCharacterChart();
    afx_msg void OnPlaybackSegment();
    afx_msg void OnPlaybackWord();
    afx_msg void OnPlaybackPhraseL1();
    afx_msg void OnStopWord();
    afx_msg void OnStopPhraseL1();
    afx_msg void OnUpdateInputstring();
    afx_msg void OnDestroy();
    afx_msg void OnActivate(UINT nState, CWnd * pWndOther, BOOL bMinimized);
    afx_msg void OnPaint();
    virtual void OnCancel();
    afx_msg void OnClose();
    DECLARE_MESSAGE_MAP()

private:
    BOOL IsDifferent(BOOL bUpdate);
    void SetButtonState(int button1, int button2, bool playing);

    //Annotation Navigation Buttons
    //
    // IsDifferentData
    CSaView * m_pPreviousSaView;
    int m_nPreviousAnnotationIndex;
    DWORD m_dwPreviousStart;
    DWORD m_dwPreviousStop;
    CString m_szPreviousString;
    // OnUpdateInputStringData
    bool m_bCheck;
    // Aid to help in setting focus
    BOOL m_bActivated;
    // Cursor scrolling data
    DWORD m_dwOriginalCursorPosition;
    DWORD m_lScrollOriginalTime;
    BOOL bEditor;
};

#endif
