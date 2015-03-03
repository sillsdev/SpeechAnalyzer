#ifndef DLGIMPORTSFMREF_H
#define DLGIMPORTSFMREF_H

class CDlgImportSFMRef : public CDialog {
    // Construction
public:
    CDlgImportSFMRef(BOOL bPhonetic, BOOL bPhonemic, BOOL bOrtho, BOOL bGloss, BOOL bGlossNat, CWnd * pParent = NULL);   // standard constructor

    // Dialog Data
    enum { IDD = IDD_ANNOTATION_IMPORT_SFM_WITH_REF };
    BOOL m_bGloss;
    BOOL m_bGlossNat;
    BOOL m_bPhonemic;
    BOOL m_bPhonetic;
    BOOL m_bOrthographic;
    CSaString m_szReference;
    CSaString m_szPhonemic;
    CSaString m_szGloss;
    CSaString m_szGlossNat;
    CSaString m_szPhonetic;
    CSaString m_szOrthographic;

    // Implementation
protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support
    void SetEnable(int nItem, BOOL bEnable);
    afx_msg void OnImportPlainText();
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()
};
#endif
