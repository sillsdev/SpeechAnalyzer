#include "stdafx.h"
#include "sa.h"
#include "DlgSaveAsOptions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSaveAsOptions dialog

int CDlgSaveAsOptions::m_nSaveArea = saveEntire;
int CDlgSaveAsOptions::m_nShowFiles = showNew;

CDlgSaveAsOptions::CDlgSaveAsOptions(LPCTSTR lpszDefExt, LPCTSTR lpszFileName, DWORD dwFlags, LPCTSTR lpszFilter, CWnd * pParentWnd)
    : CFileDialog(FALSE, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd, 0, FALSE) {
    SetTemplate(IDD, IDD);
    m_ofn.hInstance = AfxFindResourceHandle(MAKEINTRESOURCE(IDD),RT_DIALOG);
}


void CDlgSaveAsOptions::DoDataExchange(CDataExchange * pDX) {
    CFileDialog::DoDataExchange(pDX);
    DDX_Radio(pDX, IDC_SAVEAS_ENTIRE, m_nSaveArea);
    DDX_Radio(pDX, IDC_SAVEAS_OPEN, m_nShowFiles);
}


BEGIN_MESSAGE_MAP(CDlgSaveAsOptions, CFileDialog)
    ON_BN_CLICKED(IDC_SAVEAS_BOTH, OnClicked)
    ON_BN_CLICKED(IDC_SAVEAS_CURSORS, OnClicked)
    ON_BN_CLICKED(IDC_SAVEAS_ENTIRE, OnClicked)
    ON_BN_CLICKED(IDC_SAVEAS_VIEW, OnClicked)
    ON_BN_CLICKED(IDC_SAVEAS_OPEN, OnClicked)
    ON_BN_CLICKED(IDC_SAVEAS_ORIGINAL, OnClicked)
END_MESSAGE_MAP()

void CDlgSaveAsOptions::OnClicked() {
    UpdateData(); // retrieve modified data
}

BOOL CDlgSaveAsOptions::OnInitDialog() {
    CFileDialog::OnInitDialog();

    return TRUE;
}
