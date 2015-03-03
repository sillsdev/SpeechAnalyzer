// targview.h : header file
//
#ifndef _TARGVIEW_H
#define _TARGVIEW_H

/////////////////////////////////////////////////////////////////////////////
// CTargViewDlg dialog

class CMainFrame;
class CSaView;

#define MAX_TARGETS 256

class CDlgTargView : public CDialog {

public:
    CDlgTargView(CMainFrame * pMainFrame);  // standard constructor

    enum { IDD = IDD_EQUALIZE };
    CListBox m_TargView;
    CString   m_SrcView;

    CString OnlyTarg();
    int NumTargs();
    CSaView * Targ();

    void OnHelpEqualize();

protected:
    CMainFrame * m_pMainFrame;
    CString m_onlyTarg;
    int m_numTargs;
    CSaView * m_targets[MAX_TARGETS];
    CFont m_Font;
    int m_selectionIndex;

    virtual void DoDataExchange(CDataExchange * pDX);
    void SetupTargViewList();
    virtual BOOL OnInitDialog();
    void TrimPath(CString * pCStr);

    DECLARE_MESSAGE_MAP()

    virtual void OnOK();
};

#endif
