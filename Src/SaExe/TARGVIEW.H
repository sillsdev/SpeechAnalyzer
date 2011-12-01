// targview.h : header file
//
#ifndef _TARGVIEW_H
#define _TARGVIEW_H

/////////////////////////////////////////////////////////////////////////////
// CTargViewDlg dialog

class CMainFrame;
class CSaView;

#define MAX_TARGETS 256

class CTargViewDlg : public CDialog
{
	// Construction
public:
	CTargViewDlg(CMainFrame * pMainFrame);  // standard constructor

	// Dialog Data
	//{{AFX_DATA(CTargViewDlg)
	enum { IDD = IDD_EQUALIZE };
	CListBox m_TargView;
	CString   m_SrcView;
	//}}AFX_DATA

	CString OnlyTarg()  { return m_onlyTarg; };
	int    NumTargs()  { return m_numTargs; };
	CSaView  * Targ();

	// Implementation
public:
	void OnHelpEqualize();

protected:
	CMainFrame * m_pMainFrame;
	CString m_onlyTarg;
	int m_numTargs;
	CSaView * m_targets[MAX_TARGETS];
	CFont m_Font;
	int m_selectionIndex;

	virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV support
	void SetupTargViewList();
	virtual BOOL OnInitDialog();
	void TrimPath(CString * pCStr);

	// Generated message map functions
	//{{AFX_MSG(CTargViewDlg)
	// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual void OnOK();
};

#endif //_TARGVIEW_H
