/////////////////////////////////////////////////////////////////////////////
// sa_find.h
// Interface of the CSaFindDlg class
// This class manages the users page printing options using a dialog.
// Author: Doug Eberman
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_FIND_H

#include "resource.h"

#define _SA_FIND_H

class CMainFrame;

/////////////////////////////////////////////////////////////////////////////
// CSaFindDlg dialog

class CSaFindDlg : public CDialog
{
    // Construction
public:
    CSaFindDlg(CWnd * pParent,
               LPCTSTR pszFieldsToSearch,
               const CString & strToFind,
               BOOL bFindOnly,
               const CString & strToReplace,
               int idxDefaultField,
               CMainFrame * pMainFrame);

    BOOL Created() const
    {
        return m_bCreated;
    }
    int AnnotationSetID();
    void ChangeView()
    {
        OnSelchangeCbofield();
    };

    // Operations
    // Helpers for parsing information after successful return
    CString GetReplaceString();
    CString GetFindString();
    void OnHelpFind();

    // Dialog Data
    enum { IDD = IDD_FINDREPLACE };
    CString  m_strToFind;
    CComboBox m_annotSetId;
    int    m_breakOrBookMark;
    CString  m_replaceStr;
    CComboBox m_annotSetID2;

    // Implementation
protected:

    // Generated message map functions
    afx_msg void OnNext();
    afx_msg void OnPrevious();
    afx_msg void OnCharacterChart();
    afx_msg void OnSelchangeCbofield();
    afx_msg void OnChangeInputstring();
    afx_msg void OnSetFocusInputString();
    afx_msg void OnSetFocusReplaceString();
    afx_msg void OnReplace();
    afx_msg void OnReplaceAll();
    afx_msg void OnSelchangeCbofield2();
    DECLARE_MESSAGE_MAP()

    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
    virtual BOOL OnInitDialog();
    void Replace();
    void ResetCompletionCheck();
    void ScrollIfNeeded();
    void EnableDisable();
    void SetEditFont();
    BOOL Completed(BOOL isForward, int newPos);
    void SetupDialogForFindOnly();
    void SetupDialogForFindAndReplace();
    void SaveDialogLayout();
    void EnableSearch(BOOL enable);

    // internal data
    BOOL    m_bCreated;
    CString m_sFieldsToSearch;
    int     m_annotWndIndex;
    int     m_beginFind;
    int     m_curPos;
    BOOL    m_bFindOnly;
    CMainFrame * m_pMainFrame;
    BOOL    m_wraped;
    CRect   m_rctSearchInText;
    CRect   m_rctSearchInCombo;
    CRect   m_rctGlossGroup;
    CRect   m_rctWordbreakRadio;
    CRect   m_rctBookmarkRadio;
    CRect   m_rctCancelButton;
    CRect   m_rctHelpButton;
    CRect   m_rctChartButton;
    CRect   m_rctDialog;
    CRect   m_rctReplaceString;
    int     m_nTextBoxInFocus;
};

#endif //_SA_FIND_H
