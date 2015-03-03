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
#ifndef DLGADVANCEDPARSEWORDS_H
#define DLGADVANCEDPARSEWORDS_H

#include "sa_dlwnd.h"

class CSaDoc;

class CDlgAdvancedParseWords : public CDialog {
    DECLARE_DYNAMIC(CDlgAdvancedParseWords)

public:
    CDlgAdvancedParseWords(CSaDoc * pDoc);
    virtual ~CDlgAdvancedParseWords();
    BOOL Create();
    void Show(LPCTSTR title);

    CButton m_ApplyButton;
    CButton m_OKButton;
    CSaDoc * m_pDoc;

protected:
    virtual void DoDataExchange(CDataExchange * pDX);
    void Apply();
    void Undo();
    virtual BOOL OnInitDialog();
    afx_msg void OnBreakWidthScroll();
    afx_msg void OnMaxThresholdScroll();
    afx_msg void OnMinThresholdScroll();
    afx_msg void OnBnClickedApply();
    virtual void OnOK();
    virtual void OnCancel();

    DECLARE_MESSAGE_MAP()

private:
    CSpinControl m_SpinBreak;
    CSpinControl m_SpinMaxThreshold;
    CSpinControl m_SpinMinThreshold;

    enum { IDD = IDD_ADVANCEDPARSEWORD };
    int m_nBreakWidth;
    int m_nMaxThreshold;
    int m_nMinThreshold;

};
#endif
