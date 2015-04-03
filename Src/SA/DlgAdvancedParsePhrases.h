/////////////////////////////////////////////////////////////////////////////
// DlgAdvancedParse.h:
// Interface of the CDlgAdvancedParse
//
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//
// 01/12/2000
//    SDM   Extract from sa_dlg.h
//
/////////////////////////////////////////////////////////////////////////////
#ifndef DLGADVANCEDPARSEPHRASES_H
#define DLGADVANCEDPARSEPHRASES_H

#include "sa_dlwnd.h"

class CSaDoc;

class CDlgAdvancedParsePhrases : public CDialog {
    DECLARE_DYNAMIC(CDlgAdvancedParsePhrases)

    // Construction/destruction/creation
public:
    CDlgAdvancedParsePhrases(CSaDoc * pDoc);  // standard constructor
    virtual ~CDlgAdvancedParsePhrases();

    BOOL Create();
    void Show(LPCTSTR title);

    // Attributes
private:
    CSpinControl m_SpinBreak;
    CSpinControl m_SpinMaxThreshold;
    CSpinControl m_SpinMinThreshold;

    // Dialog Data
    enum { IDD = IDD_ADVANCEDPARSEPHRASE };
    int m_nBreakWidth;
    int m_nMaxThreshold;
    int m_nMinThreshold;

    // Operations
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
    void Apply();
    void Undo();

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnBreakWidthScroll();
    afx_msg void OnMaxThresholdScroll();
    afx_msg void OnMinThresholdScroll();
    afx_msg void OnBnClickedApply();
    DECLARE_MESSAGE_MAP()
    virtual void OnOK();
    virtual void OnCancel();

public:
    CButton m_ApplyButton;
    CButton m_OKButton;
    CSaDoc * m_pDoc;
};

#endif
