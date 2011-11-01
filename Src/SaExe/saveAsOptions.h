#if !defined(AFX_SAVEASOPTIONS_H__A157C2A8_149A_11D5_9FE4_00E098784E13__INCLUDED_)
#define AFX_SAVEASOPTIONS_H__A157C2A8_149A_11D5_9FE4_00E098784E13__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// saveAsOptions.h : header file
//

#include "mixer.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgSaveAsOptions dialog

class CDlgSaveAsOptions : public CFileDialog
{
// Construction
public:
	CDlgSaveAsOptions( LPCTSTR lpszDefExt = NULL, LPCTSTR lpszFileName = NULL, DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, LPCTSTR lpszFilter = NULL, CWnd* pParentWnd = NULL );

// Dialog Data
	static int m_nSaveArea;
	static int m_nShowFiles;
	//{{AFX_DATA(CDlgSaveAsOptions)
	enum { IDD = IDD_SAVEAS };
	//}}AFX_DATA

    enum { saveEntire = 0 , saveView = 1, saveCursors = 2 };
    enum { showNew = 0, showOriginal = 1, showBoth = 2};


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSaveAsOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgSaveAsOptions)
	afx_msg void OnClicked();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAVEASOPTIONS_H__A157C2A8_149A_11D5_9FE4_00E098784E13__INCLUDED_)
