/////////////////////////////////////////////////////////////////////////////
// DlgExportFW.cpp:
// Implementation of the CDlgExportFW class.
// Author: Kent Gorham
// copyright 2013 JAARS Inc. SIL
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "DlgExportFW.h"
#include "Segment.h"
#include "sa_wbch.h"
#include "sa.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "sa_graph.h"
#include "mainfrm.h"
#include "doclist.h"
#include "result.h"
#include <math.h>
#include "TranscriptionDataSettings.h"
#include "DlgImport.h"
#include "GlossSegment.h"
#include "PhoneticSegment.h"
#include "FileUtils.h"
#include "SplitFileUtils.h"
#include "resource.h"
#include "Process\sa_proc.h"
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
#include "Process\formanttracker.h"
#include "dsp\formants.h"
#include "dsp\ztransform.h"
#include "dsp\mathx.h"
#include "settings\obstream.h"

using std::ifstream;
using std::ios;
using std::streampos;

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgExportFW dialog

BEGIN_MESSAGE_MAP(CDlgExportFW, CDialog)
    ON_BN_CLICKED(IDC_EXTAB_ANNOTATIONS, OnAllAnnotations)
    ON_BN_CLICKED(IDC_EX_SFM_INTERLINEAR, OnClickedExSfmInterlinear)
    ON_BN_CLICKED(IDC_EX_SFM_MULTIRECORD, OnClickedExSfmMultirecord)
    ON_BN_CLICKED(IDC_BROWSE_OTHER, OnClickedBrowseOther)
    ON_CBN_SELCHANGE(IDC_COMBO_FIELDWORKS_PROJECT, OnSelchangeComboFieldworksProject)
    ON_CBN_KILLFOCUS(IDC_COMBO_FIELDWORKS_PROJECT, OnKillfocusComboFieldworksProject)
	ON_COMMAND(IDHELP, OnHelpExportBasic)
	ON_COMMAND(IDC_RADIO_FIELDWORKS, OnRadioFieldworks)
	ON_COMMAND(IDC_RADIO_OTHER, OnRadioOther)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportSFM message handlers

CDlgExportFW::CDlgExportFW( LPCTSTR docTitle,
							BOOL gloss,
							BOOL ortho,
							BOOL phonemic,
							BOOL phonetic,
							BOOL pos,
							BOOL reference,
							BOOL phrase,
							CWnd* pParent) : 
CDialog(CDlgExportFW::IDD, pParent) {

	settings.bGloss = bGlossDflt = gloss;
	settings.bOrtho = bOrthoDflt = ortho;
	settings.bPhonemic = bPhonemicDflt = phonemic;
	settings.bPhonetic = bPhoneticDflt = phonetic;
	settings.bPOS = bPOSDflt = pos;
	settings.bReference = bReferenceDflt = reference;
	settings.bPhrase = bPhraseDflt = phrase;
	settings.szDocTitle = docTitle;
}

BOOL CDlgExportFW::OnInitDialog() {

    CDialog::OnInitDialog();

    ctlEditFieldWorksFolder.SetWindowTextW(GetFieldWorksProjectDirectory());
    SetCheck(IDC_RADIO_FIELDWORKS, TRUE);
	SetCheck(IDC_EXTAB_ANNOTATIONS, TRUE);
	CString tags;
	tags.LoadString(IDS_SFM_TAGS);
	ctlStaticTags.SetWindowTextW(tags);

    OnRadioFieldworks();
	OnAllAnnotations();
    CenterWindow();
	UpdateButtonState();

	UpdateData(FALSE);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDlgExportFW::DoDataExchange(CDataExchange * pDX) {

	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_EXTAB_GLOSS, settings.bGloss);
	DDX_Check(pDX, IDC_EXTAB_ORTHO, settings.bOrtho);
	DDX_Check(pDX, IDC_EXTAB_PHONEMIC, settings.bPhonemic);
	DDX_Check(pDX, IDC_EXTAB_PHONETIC,settings. bPhonetic);
	DDX_Check(pDX, IDC_EXTAB_POS, settings.bPOS);
	DDX_Check(pDX, IDC_EXTAB_REFERENCE, settings.bReference);
	DDX_Check(pDX, IDC_EXTAB_PHRASE, settings.bPhrase);
	DDX_Control(pDX, IDC_BROWSE_OTHER, ctlButtonBrowseOther);
	DDX_Control(pDX, IDC_EDIT_FIELDWORKS_FOLDER, ctlEditFieldWorksFolder);
	DDX_Control(pDX, IDC_EDIT_OTHER_FOLDER, ctlEditOtherFolder);
	DDX_Control(pDX, IDC_COMBO_FIELDWORKS_PROJECT, ctlComboFieldWorksProject);
	DDX_Control(pDX, IDC_STATIC_FIELDWORKS_PROJECT, ctlStaticFieldWorksProject);
	DDX_Control(pDX, IDC_RADIO_FIELDWORKS, ctlRadioFieldWorks);
	DDX_Control(pDX, IDC_RADIO_OTHER, ctlRadioOther);
	DDX_Control(pDX, IDOK, ctlButtonOK);
	DDX_Control(pDX, IDC_STATIC_TAGS, ctlStaticTags);

	if (!pDX->m_bSaveAndValidate) {

		ctlComboFieldWorksProject.ResetContent();

		TCHAR szPath[MAX_PATH];
		memset(szPath, 0, sizeof(szPath));
		ctlEditFieldWorksFolder.GetWindowTextW(szPath, MAX_PATH);

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
			ctlComboFieldWorksProject.AddString((LPCTSTR) finder.GetFileName());
		}
	} else {
		TCHAR szBuffer[MAX_PATH];
		GetCurrentPath(szBuffer,MAX_PATH);
		settings.szPath = szBuffer;
	}
}

void CDlgExportFW::OnAllAnnotations() {

    UpdateData(TRUE);
	bool checked = ::IsDlgButtonChecked(m_hWnd,IDC_EXTAB_ANNOTATIONS)?true:false;
    BOOL bEnable = !checked;
    SetEnable(IDC_EXTAB_PHONETIC, bEnable);
    SetEnable(IDC_EXTAB_PHONEMIC, bEnable);
    SetEnable(IDC_EXTAB_ORTHO, bEnable);
    SetEnable(IDC_EXTAB_GLOSS, bEnable);
    SetEnable(IDC_EXTAB_REFERENCE, bEnable);
    SetEnable(IDC_EXTAB_POS, bEnable);
    SetEnable(IDC_EXTAB_PHRASE, bEnable);
	if (checked) {
		settings.bGloss = bGlossDflt;
		settings.bOrtho = bOrthoDflt;
		settings.bPhonemic = bPhonemicDflt;
		settings.bPhonetic = bPhoneticDflt;
		settings.bPOS = bPOSDflt;
		settings.bReference = bReferenceDflt;
		settings.bPhrase = bPhraseDflt;
	} else {
		settings.bReference = settings.bPhonetic = settings.bPhonemic = settings.bOrtho = settings.bGloss = settings.bPOS = settings.bPhrase = FALSE;
	}
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

    ctlEditOtherFolder.GetWindowTextW(szFolderLocation, MAX_PATH);

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
        ctlEditOtherFolder.SetWindowText(temp.c_str());
    }

    CoUninitialize();
	UpdateButtonState();
}

void CDlgExportFW::OnRadioFieldworks() {
    
	ctlEditFieldWorksFolder.EnableWindow(FALSE);
    ctlComboFieldWorksProject.EnableWindow(TRUE);
    ctlStaticFieldWorksProject.EnableWindow(TRUE);
    ctlEditOtherFolder.EnableWindow(FALSE);
	ctlButtonBrowseOther.EnableWindow(FALSE);
	UpdateButtonState();
}


void CDlgExportFW::OnRadioOther() {

    ctlEditFieldWorksFolder.EnableWindow(FALSE);
    ctlComboFieldWorksProject.EnableWindow(FALSE);
    ctlStaticFieldWorksProject.EnableWindow(FALSE);
    ctlEditOtherFolder.EnableWindow(TRUE);
	ctlButtonBrowseOther.EnableWindow(TRUE);
	UpdateButtonState();
}

void CDlgExportFW::GetCurrentPath( LPTSTR szBuffer, size_t size) {

	wmemset(szBuffer,0,MAX_PATH);

    if (ctlRadioFieldWorks.GetCheck() == BST_CHECKED) {
		TCHAR szTemp[MAX_PATH];
		wmemset(szTemp,0,MAX_PATH);
        ctlEditFieldWorksFolder.GetWindowTextW(szTemp, MAX_PATH);
		wcscat_s(szBuffer,size,szTemp);
		AppendDirSep(szBuffer,size);
        int sel = ctlComboFieldWorksProject.GetCurSel();
        ctlComboFieldWorksProject.GetLBText(sel, szTemp);
		wcscat_s(szBuffer,size,szTemp);
		AppendDirSep(szBuffer,MAX_PATH);
		return;
    }

	// at this point we are assuming they selected 'other'
    ctlEditOtherFolder.GetWindowTextW(szBuffer, MAX_PATH);
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
	bool selected = ((ctlRadioOther.GetCheck()==BST_CHECKED)||(ctlComboFieldWorksProject.GetCurSel()!=-1));
    
	ctlButtonOK.EnableWindow(((valid)&&(selected))?TRUE:FALSE);
}

void CDlgExportFW::WriteFileUtf8(CFile * pFile, const CSaString szString) {

	std::string szUtf8 = szString.utf8();
	pFile->Write(szUtf8.c_str(), szUtf8.size());
}

/**
* return the registered fieldworks project directory
*/
CSaString CDlgExportFW::GetFieldWorksProjectDirectory() {

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

