/////////////////////////////////////////////////////////////////////////////
// DlhSynthesis.h:
// Interface of the CDlgSynthesis (property sheet)
//
// Author: Steve MacLean
// copyright 2001 JAARS Inc. SIL
//
// Revision History
//
// 02/15/2001
//    SDM   Replicated from advancedParameters.cpp
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _DLG_SYNTHESIS_H
#define _DLG_SYNTHESIS_H

//###########################################################################
// CDlgSynthesis property sheet

class CDlgMbrola;

class CDlgSynthesis : public CPropertySheet
{
    DECLARE_DYNAMIC(CDlgSynthesis)

// Construction/destruction/creation
public:
    CDlgSynthesis(LPCTSTR pszCaption, CWnd * pParent = NULL, UINT iSelectPage = 0);
    ~CDlgSynthesis();

public:
    void OnHelpSynthesis();
    void OnHelp();
    CDlgMbrola * m_pDlgMbrolaPage;

    CButton m_cOK;
    // CButton m_cHelp;

    // Operations
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support

    // Generated message map functions
protected:
    //{{AFX_MSG(CDlgSynthesis)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif //_DLG_SYNTHESIS_H
