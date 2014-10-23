#pragma once

class CDlgHelpSFMMarkers : public CDialog {

    DECLARE_DYNAMIC(CDlgHelpSFMMarkers)

public:
    CDlgHelpSFMMarkers(CWnd * pParent = NULL);  // standard constructor
    virtual ~CDlgHelpSFMMarkers();

    enum { IDD = IDD_HELP_SFM_MARKERS};

protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support
    DECLARE_MESSAGE_MAP()
};
