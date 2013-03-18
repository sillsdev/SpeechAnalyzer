#pragma once

#include "resource.h"

class CDlgExportFWResult : public CDialog {

    DECLARE_DYNAMIC(CDlgExportFWResult)

public:
    CDlgExportFWResult(CWnd * pParent = NULL);  // standard constructor
    virtual ~CDlgExportFWResult();

    enum { IDD = IDD_EXPORTFWRESULT };

protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    CString m_DataCount;
    CString m_SFMCount;
    CString m_WAVCount;
};
