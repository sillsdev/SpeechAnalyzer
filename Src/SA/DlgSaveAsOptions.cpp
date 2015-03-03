#include "stdafx.h"
#include "sa.h"
#include "DlgSaveAsOptions.h"
#include "AppDefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSaveAsOptions dialog

CDlgSaveAsOptions::CDlgSaveAsOptions( LPCTSTR lpszDefExt, 
									  LPCTSTR lpszFileName, 
									  LPCTSTR lpszDefaultDir,
									  DWORD dwFlags, 
									  LPCTSTR lpszFilter, 
									  CWnd * pParentWnd, 
									  bool saveAs, 
									  bool stereo) :
    CFileDialog(FALSE, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd, 0, FALSE),
    mSaveArea(saveEntire),
    mShowFiles(showNew),
    mFileFormat(((stereo)?formatStereo:formatMono)),
	mOriginalPath( lpszFileName),
	mStereo(stereo),
	mSaveAs(saveAs) {
    SetTemplate(IDD, IDD);
    m_ofn.hInstance = AfxFindResourceHandle(MAKEINTRESOURCE(IDD),RT_DIALOG);
	m_ofn.lpstrInitialDir = lpszDefaultDir;
}

INT_PTR CDlgSaveAsOptions::DoModal() {
	return CFileDialog::DoModal();
}

/**
* returns true if the current file is the same
* as the file that was used to initialize the 
* dialog box
*/
bool CDlgSaveAsOptions::IsSameFile() {
	CString selected = GetPathName();
    return (selected.CompareNoCase(mOriginalPath)==0);
}

CString CDlgSaveAsOptions::GetSelectedPath() {
	return GetPathName();
}

void CDlgSaveAsOptions::DoDataExchange(CDataExchange * pDX) {
    CFileDialog::DoDataExchange(pDX);
    DDX_Radio(pDX, IDC_SAVEAS_ENTIRE, (int &)mSaveArea);
    DDX_Radio(pDX, IDC_SAVEAS_OPEN, (int &)mShowFiles);
    DDX_Radio(pDX, IDC_SAVEAS_STEREO, (int &)mFileFormat);
}

BEGIN_MESSAGE_MAP(CDlgSaveAsOptions, CFileDialog)
    ON_BN_CLICKED(IDC_SAVEAS_BOTH, OnClicked)
    ON_BN_CLICKED(IDC_SAVEAS_CURSORS, OnClicked)
    ON_BN_CLICKED(IDC_SAVEAS_ENTIRE, OnClicked)
    ON_BN_CLICKED(IDC_SAVEAS_VIEW, OnClicked)
    ON_BN_CLICKED(IDC_SAVEAS_OPEN, OnClicked)
    ON_BN_CLICKED(IDC_SAVEAS_ORIGINAL, OnClicked)
    ON_BN_CLICKED(IDC_SAVEAS_STEREO, OnClicked)
    ON_BN_CLICKED(IDC_SAVEAS_MONO, OnClicked)
    ON_BN_CLICKED(IDC_SAVEAS_RIGHT, OnClicked)
END_MESSAGE_MAP()

void CDlgSaveAsOptions::OnClicked() {
    UpdateData(); // retrieve modified data
}

BOOL CDlgSaveAsOptions::OnInitDialog() {
    CFileDialog::OnInitDialog();
    if (mSaveAs) {
        if (mStereo) {
            GetDlgItem(IDC_SAVEAS_STEREO)->EnableWindow(TRUE);
            GetDlgItem(IDC_SAVEAS_MONO)->EnableWindow(TRUE);
            GetDlgItem(IDC_SAVEAS_RIGHT)->EnableWindow(TRUE);
        } else {
            GetDlgItem(IDC_SAVEAS_STEREO)->EnableWindow(FALSE);
            GetDlgItem(IDC_SAVEAS_MONO)->EnableWindow(FALSE);
            GetDlgItem(IDC_SAVEAS_RIGHT)->EnableWindow(FALSE);
        }
    } else {
        GetDlgItem(IDC_SAVEAS_STEREO)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_SAVEAS_MONO)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_SAVEAS_RIGHT)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_GROUP_FILE_STATIC)->ShowWindow(SW_HIDE);
    }
    return TRUE;
}
