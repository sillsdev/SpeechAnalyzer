/////////////////////////////////////////////////////////////////////////////
// DlgExportFW.cpp:
// Implementation of the CDlgExportFW class.
// Author: Steve MacLean
// copyright 1999 JAARS Inc. SIL
//
// Revision History
// 1.5Test8.3
//         SDM Original version
// 1.5Test8.5
//         SDM Changed Table headings to use Initial Uppercase ("Emic" & "Etic")
// 1.5Test10.7
//         SDM Changed Import to not pad extra phonetic with spaces
// 1.5Test11.0
//         SDM replaced GetOffset() + GetDuration() with CSegment::GetStop()
//         SDM fixed bug in export which crashed if phonetic segment empty
//         SDM changed export to export AutoPitch (Grappl)
// 1.5Test11.3
//         SDM changed CSegment::AdjustPositionAll to CSegment::Adjust
//
//    07/27/2000
//         DDO Changed so these dialogs display before the user is asked
//             what file to export. Therefore, I had to move the get filename
//             dialog call into this module instead of doing it in the view
//             class when the user picks one of the export menu items.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DlgExportFW.h"
#include "Process\sa_proc.h"
#include "sa_segm.h"
#include "Process\sa_p_lou.h"
#include "Process\sa_p_gra.h"
#include "Process\sa_p_pit.h"
#include "Process\sa_p_cpi.h"
#include "Process\sa_p_spi.h"
#include "Process\sa_p_mel.h"
#include "Process\sa_p_zcr.h"
#include "Process\sa_p_fra.h"
#include "Process\sa_p_spu.h"
#include "Process\sa_p_spg.h"
#include "dsp\formants.h"
#include "dsp\ztransform.h"
#include "Process\formanttracker.h"
#include "dsp\mathx.h"

#include "sa_wbch.h"
#include "sa.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "sa_graph.h"
#include "mainfrm.h"
#include "settings\obstream.h"
#include "doclist.h"
using std::ifstream;
using std::ios;
using std::streampos;

#include "exportbasicdialog.h"
#include "result.h"
#include <math.h>
#include "TranscriptionDataSettings.h"
#include "DlgImport.h"
#include "GlossSegment.h"
#include "PhoneticSegment.h"
#include "FileUtils.h"
#include "SplitFileUtils.h"
#include "resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CSaString szCrLf = "\r\n";

void WriteFileUtf8(CFile * pFile, const CSaString szString) {

    std::string szUtf8 = szString.utf8();
    pFile->Write(szUtf8.c_str(), szUtf8.size());
}

CExportFWData::CExportFWData( LPCTSTR szDocTitle) {

	bAllAnnotations = TRUE;
	bGloss = TRUE;
	bOrtho = TRUE;
	bPhonemic = TRUE;
	bPhonetic = TRUE;
	bPOS = FALSE;
	bReference = TRUE;
	bPhrase = FALSE;
	CExportFWData::szDocTitle = szDocTitle;
}

/**
* returns true if IPAHelp seems to be installed
*/
CSaString GetFieldWorksProjectDirectory() {

    // retrieve IPA Help location from registry
    TCHAR szPathBuf[_MAX_PATH + 1];
    HKEY hKey = NULL;
    DWORD dwBufLen = MAX_PATH + 1;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\SIL\\FieldWorks\\7.0"), 0, KEY_QUERY_VALUE, &hKey)) {
        return L"";
    }
    long nError = RegQueryValueEx(hKey, _T("ProjectsDir"), NULL, NULL, (LPBYTE) szPathBuf, &dwBufLen);
    RegCloseKey(hKey);
    if ((nError) || (!wcslen(szPathBuf))) {
        return L"";
    }

    return CSaString(szPathBuf);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgExportFW dialog

BEGIN_MESSAGE_MAP(CDlgExportFW, CDialog)
    ON_BN_CLICKED(IDC_EXTAB_ANNOTATIONS, OnAllAnnotations)
    ON_BN_CLICKED(IDC_EX_SFM_INTERLINEAR, OnClickedExSfmInterlinear)
    ON_BN_CLICKED(IDC_EX_SFM_MULTIRECORD, OnClickedExSfmMultirecord)
    ON_COMMAND(IDHELP, OnHelpExportBasic)
    ON_BN_CLICKED(IDC_BROWSE_OTHER, OnClickedBrowseOther)
    ON_COMMAND(IDC_RADIO_FIELDWORKS, OnRadioFieldworks)
    ON_COMMAND(IDC_RADIO_OTHER, OnRadioOther)
    ON_CBN_SELCHANGE(IDC_COMBO_FIELDWORKS_PROJECT, &CDlgExportFW::OnSelchangeComboFieldworksProject)
    ON_CBN_KILLFOCUS(IDC_COMBO_FIELDWORKS_PROJECT, &CDlgExportFW::OnKillfocusComboFieldworksProject)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportSFM message handlers

CDlgExportFW::CDlgExportFW(const CSaString & szDocTitle, CWnd * pParent) : 
CDialog(CDlgExportFW::IDD, pParent),
data(szDocTitle) {
}

BOOL CDlgExportFW::OnInitDialog() {

    CDialog::OnInitDialog();

    OnAllAnnotations();

    m_EditFieldWorksFolder.SetWindowTextW(GetFieldWorksProjectDirectory());

    SetCheck(IDC_RADIO_FIELDWORKS, TRUE);

    OnRadioFieldworks();

    CenterWindow();

    UpdateData(FALSE);

	UpdateButtonState();

	CString tags;
	tags.LoadString(IDS_SFM_TAGS);
	m_StaticTags.SetWindowTextW(tags);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDlgExportFW::DoDataExchange(CDataExchange * pDX) {

	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_EXTAB_ANNOTATIONS, data.bAllAnnotations);
	DDX_Check(pDX, IDC_EXTAB_GLOSS, data.bGloss);
	DDX_Check(pDX, IDC_EXTAB_ORTHO, data.bOrtho);
	DDX_Check(pDX, IDC_EXTAB_PHONEMIC, data.bPhonemic);
	DDX_Check(pDX, IDC_EXTAB_PHONETIC, data.bPhonetic);
	DDX_Check(pDX, IDC_EXTAB_POS, data.bPOS);
	DDX_Check(pDX, IDC_EXTAB_REFERENCE, data.bReference);
	DDX_Check(pDX, IDC_EXTAB_PHRASE, data.bPhrase);
	DDX_Control(pDX, IDC_BROWSE_OTHER, m_BrowseOther);
	DDX_Control(pDX, IDC_EDIT_FIELDWORKS_FOLDER, m_EditFieldWorksFolder);
	DDX_Control(pDX, IDC_EDIT_OTHER_FOLDER, m_EditOtherFolder);
	DDX_Control(pDX, IDC_COMBO_FIELDWORKS_PROJECT, m_ComboFieldWorksProject);
	DDX_Control(pDX, IDC_STATIC_FIELDWORKS_PROJECT, m_StaticFieldWorksProject);
	DDX_Control(pDX, IDC_RADIO_FIELDWORKS, m_RadioFieldWorks);
	DDX_Control(pDX, IDC_RADIO_OTHER, m_RadioOther);
	DDX_Control(pDX, IDOK, m_ButtonOK);
	DDX_Control(pDX, IDC_STATIC_TAGS, m_StaticTags);

	if (!pDX->m_bSaveAndValidate) {

		m_ComboFieldWorksProject.ResetContent();

		TCHAR szPath[MAX_PATH];
		memset(szPath, 0, sizeof(szPath));
		m_EditFieldWorksFolder.GetWindowTextW(szPath, MAX_PATH);

		CString path = szPath;
		path.Append(L"\\");
		path.Append(L"*");

		CFileFind finder;
		BOOL bWorking = finder.FindFile(path);
		while(bWorking) {
			bWorking = finder.FindNextFile();

			if (finder.IsDots()) {
				continue;
			}
			if (!finder.IsDirectory()) {
				continue;
			}
			TRACE(L"Found %s\n", finder.GetFileName());
			m_ComboFieldWorksProject.AddString((LPCTSTR) finder.GetFileName());
		}
	} else {
		TCHAR szBuffer[MAX_PATH];
		GetCurrentPath(szBuffer,MAX_PATH);
		data.szPath = szBuffer;
	}
}

void CDlgExportFW::OnAllAnnotations() {

    UpdateData(TRUE);
    BOOL bEnable = !data.bAllAnnotations;
    SetEnable(IDC_EXTAB_PHONETIC, bEnable);
    SetEnable(IDC_EXTAB_PHONEMIC, bEnable);
    SetEnable(IDC_EXTAB_ORTHO, bEnable);
    SetEnable(IDC_EXTAB_GLOSS, bEnable);
    SetEnable(IDC_EXTAB_REFERENCE, bEnable);
    SetEnable(IDC_EXTAB_POS, bEnable);
    SetEnable(IDC_EXTAB_PHRASE, bEnable);
	UpdateData(FALSE);
}

void CDlgExportFW::SetEnable(int nItem, BOOL bEnable) {

    CWnd * pWnd = GetDlgItem(nItem);
    if (pWnd) {
        pWnd->EnableWindow(bEnable);
    }
}

void CDlgExportFW::SetCheck(int nItem, BOOL bChecked) {
    
	CButton * pWnd = (CButton *) GetDlgItem(nItem);
    if (pWnd) {
        pWnd->SetCheck(bChecked);
    }
}

void CDlgExportFW::OnHelpExportBasic() {

    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath = szPath + "::/User_Interface/Menus/File/Export/Standard_format_or_extensible_markup.htm";
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

void CDlgExportFW::OnClickedExSfmInterlinear() {

    CButton * pWnd = (CButton *) GetDlgItem(IDC_EX_SFM_MULTIRECORD);
    if (pWnd) {
        pWnd->SetCheck(FALSE);
    }
}

void CDlgExportFW::OnClickedExSfmMultirecord() {

    CButton * pWnd = (CButton *) GetDlgItem(IDC_EX_SFM_INTERLINEAR);
    if (pWnd) {
        pWnd->SetCheck(FALSE);
    }
}

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM /*lParam*/, LPARAM lpData) {

    // If the BFFM_INITIALIZED message is received
    // set the path to the start path.
    switch(uMsg) {
    case BFFM_INITIALIZED: {
        if (NULL != lpData) {
            SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
        }
    }
    }
    return 0;
}

void CDlgExportFW::OnClickedBrowseOther() {

    // szCurrent is an optional start folder. Can be NULL.
    // szPath receives the selected path on success. Must be MAX_PATH characters in length.

    CoInitialize(NULL);

    TCHAR szDisplay[MAX_PATH];
    memset(szDisplay, 0, sizeof(szDisplay));

    TCHAR szFolderLocation[MAX_PATH];
    memset(szFolderLocation, 0, sizeof(szFolderLocation));

    m_EditOtherFolder.GetWindowTextW(szFolderLocation, MAX_PATH);

    BROWSEINFO bi = { 0 };
    bi.hwndOwner = this->m_hWnd;
    bi.pszDisplayName = szDisplay;
    bi.lpszTitle = TEXT("Please choose a folder.");
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.lpfn = BrowseCallbackProc;
    bi.lParam = (LPARAM)(LPCTSTR) szFolderLocation;
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl == NULL) {
        // they canceled...
        CoUninitialize();
        return;
    }

    TCHAR szPath[MAX_PATH];
    memset(szPath, 0, sizeof(szPath));

    BOOL retval = SHGetPathFromIDList(pidl, szPath);
    CoTaskMemFree(pidl);
    if (!retval) {
        szPath[0] = TEXT('\0');
    } else {
        wstring temp(szPath);
        if (temp[temp.length() - 1] != '\\') {
            temp.append(L"\\");
        }
        m_EditOtherFolder.SetWindowText(temp.c_str());
    }

    CoUninitialize();
	UpdateButtonState();
}

void CDlgExportFW::OnRadioFieldworks() {
    
	m_EditFieldWorksFolder.EnableWindow(FALSE);
    m_ComboFieldWorksProject.EnableWindow(TRUE);
    m_StaticFieldWorksProject.EnableWindow(TRUE);
    m_EditOtherFolder.EnableWindow(FALSE);
	m_BrowseOther.EnableWindow(FALSE);
	UpdateButtonState();
}


void CDlgExportFW::OnRadioOther() {

    m_EditFieldWorksFolder.EnableWindow(FALSE);
    m_ComboFieldWorksProject.EnableWindow(FALSE);
    m_StaticFieldWorksProject.EnableWindow(FALSE);
    m_EditOtherFolder.EnableWindow(TRUE);
	m_BrowseOther.EnableWindow(TRUE);
	UpdateButtonState();
}

void CDlgExportFW::GetCurrentPath( LPTSTR szBuffer, size_t size) {

	wmemset(szBuffer,0,MAX_PATH);

    if (m_RadioFieldWorks.GetCheck() == BST_CHECKED) {
		TCHAR szTemp[MAX_PATH];
		wmemset(szTemp,0,MAX_PATH);
        m_EditFieldWorksFolder.GetWindowTextW(szTemp, MAX_PATH);
		wcscat_s(szBuffer,size,szTemp);
		AppendDirSep(szBuffer,size);
        int sel = m_ComboFieldWorksProject.GetCurSel();
        m_ComboFieldWorksProject.GetLBText(sel, szTemp);
		wcscat_s(szBuffer,size,szTemp);
		AppendDirSep(szBuffer,MAX_PATH);
		return;
    }

	// at this point we are assuming they selected 'other'
    m_EditOtherFolder.GetWindowTextW(szBuffer, MAX_PATH);
	AppendDirSep(szBuffer,MAX_PATH);
}


void CDlgExportFW::OnSelchangeComboFieldworksProject() {
	UpdateButtonState();
}


void CDlgExportFW::OnKillfocusComboFieldworksProject() {
	UpdateButtonState();
}

void CDlgExportFW::UpdateButtonState() {


    TCHAR szBuffer[MAX_PATH];
	GetCurrentPath(szBuffer,MAX_PATH);

	bool valid = FolderExists(szBuffer);
	bool selected = ((m_RadioOther.GetCheck()==BST_CHECKED)||(m_ComboFieldWorksProject.GetCurSel()!=-1));
    
	m_ButtonOK.EnableWindow(((valid)&&(selected))?TRUE:FALSE);
}