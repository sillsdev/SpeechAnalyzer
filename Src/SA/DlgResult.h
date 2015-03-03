#ifndef RESULT_H
#define RESULT_H

class CResult : public CDialog {
public:
    CResult(LPCTSTR result, CWnd * pParent = NULL);  // standard constructor

    enum { IDD = IDD_RESULT };
    CString m_szResult;

protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support
    virtual void OnCancel();

    DECLARE_MESSAGE_MAP()
};

#endif
