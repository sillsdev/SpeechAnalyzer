/////////////////////////////////////////////////////////////////////////////
// DlgAnnotationEdit.h:
// Definition of the CDlgAnnotationEdit classes.
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
#ifndef DLGANNOTATIONEDIT_H
#define DLGANNOTATIONEDIT_H

//###########################################################################
// CDlgEditor dialog
// Forward declarations

class CSegment;
class CMainFrame;
class CSaDoc;
class CSaView;

class CDlgAnnotationEdit : public CDialog {
public:
    CDlgAnnotationEdit(CWnd * pParent = NULL);
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
    CMainFrame * GetMainFrame() const;
    CSaView * GetView() const;
    CSaDoc * GetDoc() const;
};
#endif
