// saveAsOptions.cpp : implementation file
//

#include "stdafx.h"
#include "sa.h"
#include "saveAsOptions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSaveAsOptions dialog


int CDlgSaveAsOptions::m_nSaveArea = saveEntire;
int CDlgSaveAsOptions::m_nShowFiles = showNew;

CDlgSaveAsOptions::CDlgSaveAsOptions(LPCTSTR lpszDefExt, LPCTSTR lpszFileName, DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd )
: CFileDialog(FALSE, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
	if (::GetWindowsVersion()<6)
		SetTemplate(IDD, IDD);
	m_ofn.hInstance = AfxFindResourceHandle(MAKEINTRESOURCE(IDD),RT_DIALOG);
	//{{AFX_DATA_INIT(CDlgSaveAsOptions)
	//}}AFX_DATA_INIT
}


void CDlgSaveAsOptions::DoDataExchange(CDataExchange* pDX)
{
	CFileDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSaveAsOptions)
	DDX_Radio(pDX, IDC_SAVEAS_ENTIRE, m_nSaveArea);
	DDX_Radio(pDX, IDC_SAVEAS_OPEN, m_nShowFiles);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgSaveAsOptions, CFileDialog)
	//{{AFX_MSG_MAP(CDlgSaveAsOptions)
	ON_BN_CLICKED(IDC_SAVEAS_BOTH, OnClicked)
	ON_BN_CLICKED(IDC_SAVEAS_CURSORS, OnClicked)
	ON_BN_CLICKED(IDC_SAVEAS_ENTIRE, OnClicked)
	ON_BN_CLICKED(IDC_SAVEAS_VIEW, OnClicked)
	ON_BN_CLICKED(IDC_SAVEAS_OPEN, OnClicked)
	ON_BN_CLICKED(IDC_SAVEAS_ORIGINAL, OnClicked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSaveAsOptions message handlers

void CDlgSaveAsOptions::OnClicked()
{
	UpdateData(); // retrieve modified data
}

BOOL CDlgSaveAsOptions::OnInitDialog()
{
	CFileDialog::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
