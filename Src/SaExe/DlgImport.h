#ifndef DLGIMPORT_H
#define DLGIMPORT_H

class CDlgImport : public CDialog
{
    // Construction
public:
    CDlgImport(CWnd * pParent = NULL); // standard constructor

    // Dialog Data
    enum { IDD = IDD_IMPORT_DIALOG };
    int   m_nMode;

    // Implementation
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support

    // Generated message map functions
    virtual void OnOK();
    virtual BOOL OnInitDialog();
    DECLARE_MESSAGE_MAP()
};

#endif DLGIMPORT_H
