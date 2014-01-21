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

CDlgSaveAsOptions::CDlgSaveAsOptions( LPCTSTR lpszDefExt, LPCTSTR lpszFileName, DWORD dwFlags, LPCTSTR lpszFilter, CWnd * pParentWnd, bool saveAs, bool stereo) : 
CFileDialog(FALSE, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd, 0, FALSE),
m_eSaveArea(saveEntire),
m_eShowFiles(showNew),
m_eFileFormat(formatMono)
{
    SetTemplate(IDD, IDD);
    m_ofn.hInstance = AfxFindResourceHandle(MAKEINTRESOURCE(IDD),RT_DIALOG);
	CDlgSaveAsOptions::stereo = stereo;
	CDlgSaveAsOptions::saveAs = saveAs;
	m_eFileFormat = ((stereo)?formatStereo:formatMono);
}

void CDlgSaveAsOptions::DoDataExchange(CDataExchange * pDX)
{
    CFileDialog::DoDataExchange(pDX);
    DDX_Radio(pDX, IDC_SAVEAS_ENTIRE, (int&)m_eSaveArea);
    DDX_Radio(pDX, IDC_SAVEAS_OPEN, (int&)m_eShowFiles);
    DDX_Radio(pDX, IDC_SAVEAS_STEREO, (int&)m_eFileFormat);
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

void CDlgSaveAsOptions::OnClicked()
{
    UpdateData(); // retrieve modified data
}

BOOL CDlgSaveAsOptions::OnInitDialog()
{
    CFileDialog::OnInitDialog();
	if (saveAs) {
		if (stereo) {
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
