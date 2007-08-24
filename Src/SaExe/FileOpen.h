#if !defined(AFX_FILEOPEN_H__D649F703_85A4_11D5_9FE4_00E0988A4887__INCLUDED_)
#define AFX_FILEOPEN_H__D649F703_85A4_11D5_9FE4_00E0988A4887__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileOpen.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgFileOpen dialog

class CDlgFileOpen : public CFileDialog
{
	DECLARE_DYNAMIC(CDlgFileOpen)

public:
	CDlgFileOpen(LPCTSTR lpszDefExt = NULL, LPCTSTR lpszFileName = NULL, DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, LPCTSTR lpszFilter = NULL, CWnd* pParentWnd = NULL);

// Dialog Data
	//{{AFX_DATA(CDlgSaveAsOptions)
	enum { IDD = IDD_FILEOPEN };
	//}}AFX_DATA

protected:
	//{{AFX_MSG(CDlgFileOpen)
	afx_msg void OnPlay();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILEOPEN_H__D649F703_85A4_11D5_9FE4_00E0988A4887__INCLUDED_)
