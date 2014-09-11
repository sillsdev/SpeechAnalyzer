#pragma once

class CDlgExportLiftResult : public CDialog {

    DECLARE_DYNAMIC(CDlgExportLiftResult)

public:
    CDlgExportLiftResult(CWnd * pParent = NULL);  // standard constructor
    virtual ~CDlgExportLiftResult();

    enum { IDD = IDD_EXPORTLIFTRESULT };

protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    CString m_DataCount;
    CString m_SFMCount;
    CString m_WAVCount;
};
