/////////////////////////////////////////////////////////////////////////////
// sa_edit.h:
// Definition of the CDlgEditor (dialog)
//                   CAnnotationEdit                   classes.
//
// Author: Steve MacLean
// copyright 1999 JAARS Inc. SIL
//
// Revision History
// 1.06.5
//      SDM Original
// 1.06.6
//      SDM Added class CAnnotationEdit class
// 1.06.6U4
//      SDM Added OnCancel to undo changes on ESC (Alt+F4)
// 1.5Test8.2
//      SDM moved CreateSafe to .CPP
// 1.5Test8.5
//      SDM CDlgEditor removed autohide restore
// 1.5Test10.0
//      SDM fix bug on close of TE which cancel previous change & ESC would not cancel see 1.06.6U4 & 1.5Test8.2
/////////////////////////////////////////////////////////////////////////////


#include "resource.h"

//###########################################################################
// CDlgEditor dialog
// Forward declarations

class CSegment;
class CMainFrame;
class CSaDoc;
class CSaView;


class CDlgEditor : public CDialog
{
public:
    CDlgEditor(CWnd * pParent=NULL);

    BOOL CreateSafe(UINT nIDTemplate, CWnd * pParentWnd=NULL, WINDOWPLACEMENT * pWPL=NULL);
    CMainFrame * MainFrame() const;
    CSaView * SaView() const;
    CSaDoc * SaDoc() const;
    CSegment * GetSelectedSegment() const;
    void UpdateDialog();
    virtual BOOL PreTranslateMessage(MSG * pMsg);
    void OnHelpEdit();

    enum { IDD = IDD_EDITOR };
    CButton m_cNextButton;
    CButton m_cPreviousButton;
    CButton m_cUpButton;
    CButton m_cDownButton;

protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void * pExtra,AFX_CMDHANDLERINFO * pHandlerInfo);

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnCharacterChart();
    afx_msg void OnPlaybackSegment();
    afx_msg void OnPlaybackWord();
    afx_msg void OnPlaybackPhraseL1();
    afx_msg void OnUpdateInputstring();
    afx_msg void OnDestroy();
    afx_msg void OnActivate(UINT nState, CWnd * pWndOther, BOOL bMinimized);
    afx_msg void OnPaint();
    virtual void OnCancel();
    afx_msg void OnClose();
    DECLARE_MESSAGE_MAP()

private:
    BOOL IsDifferent(BOOL bUpdate);
    //Annotation Navigation Buttons
    //
    // IsDifferentData
    CSaView * m_pPreviousSaView;
    int m_nPreviousAnnotationIndex;
    DWORD m_dwPreviousStart;
    DWORD m_dwPreviousStop;
    CString m_szPreviousString;
    //OnUpdateInputStringData
    BOOL m_bCheck;
    //Aid to help in setting focus
    BOOL m_bActivated;
    // Cursor scrooling data
    BOOL m_bScroll;
    DWORD m_dwOriginalCursorPosition;
    DWORD m_lScrollOriginalTime;
    BOOL bEditor;
};

class CAnnotationEdit : public CDialog
{
public:
    CAnnotationEdit(CWnd * pParent = NULL);
    BOOL Create(UINT nIDTemplate, CWnd * pParentWnd=NULL);
    enum { IDD = IDD_ANNOTATION_EDIT };

    CString SetText(const CString & szString);

private:
    BOOL m_bClosing;
    BOOL m_bChanged;

    //Operations
public:
    virtual BOOL PreTranslateMessage(MSG * pMsg);

protected:
    // for custom cleanup after WM_NCDESTROY
    virtual void PostNcDestroy();
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support

    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnUpdateInputstring();
    afx_msg void OnActivate(UINT nState, CWnd * pWndOther, BOOL bMinimized);
    virtual BOOL OnInitDialog();
    virtual void OnCancel();
    virtual void OnOK();
    afx_msg void OnClose();

	DECLARE_MESSAGE_MAP()

private:
    CMainFrame * MainFrame() const;
    CSaView * SaView() const;
    CSaDoc * SaDoc() const;
};
