/////////////////////////////////////////////////////////////////////////////
// DlgExportFW.cpp:
// Implementation of the CDlgExportLift class.
// Author: Kent Gorham
// copyright 2014-2018 JAARS Inc. SIL
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "DlgExportLift.h"
#include "Segment.h"
#include "sa_wbch.h"
#include "sa.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "sa_graph.h"
#include "mainfrm.h"
#include "doclist.h"
#include "DlgResult.h"
#include <math.h>
#include "TranscriptionDataSettings.h"
#include "DlgImport.h"
#include "GlossSegment.h"
#include "PhoneticSegment.h"
#include "FileUtils.h"
#include "SplitFileUtils.h"
#include "Process\Process.h"
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
#include "Process\FormantTracker.h"
#include "objectostream.h"

using std::ifstream;
using std::ios;
using std::streampos;

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgExportLift dialog

BEGIN_MESSAGE_MAP(CDlgExportLift, CDialog)
    ON_COMMAND(IDHELP, OnHelpExportBasic)
    ON_BN_CLICKED(IDC_BROWSE, OnClickedBrowse)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportSFM message handlers

CDlgExportLift::CDlgExportLift(LPCTSTR docTitle,
							   LPCTSTR szPath,
                               BOOL gloss,
                               BOOL glossNat,
                               BOOL ortho,
                               BOOL phonemic,
                               BOOL phonetic,
                               BOOL reference,
                               map<wstring,wstring> codes,
                               CWnd * pParent) :
    CDialog(CDlgExportLift::IDD, pParent) {

    settings.bGloss = bGlossDflt = gloss;
    settings.bGlossNat = bGlossNatDflt = glossNat;
    settings.bOrtho = bOrthoDflt = ortho;
    settings.bPhonemic = bPhonemicDflt = phonemic;
    settings.bPhonetic = bPhoneticDflt = phonetic;
    settings.bReference = bReferenceDflt = reference;
    settings.bPhrase1 = false;
    settings.bPhrase2 = false;
    settings.bPhrase3 = false;
    settings.bPhrase4 = false;
    settings.szDocTitle = docTitle;
	settings.szPath = szPath;
	countryCodes = codes;
}

BOOL CDlgExportLift::OnInitDialog() {
    CDialog::OnInitDialog();
    UpdateButtonState();

    ctlEditFolder.SetWindowTextW(settings.szPath);

    SetCheck(IDC_EXTAB_ANNOTATIONS, TRUE);

    CenterWindow();

    map<wstring,wstring>::iterator it = countryCodes.begin();
    while (it!=countryCodes.end()) {
        // the key is the language name
        // the value is the language code
        wstring name = it->first;
    
        ctlReferenceList.AddString(name.c_str());
        ctlOrthoList.AddString(name.c_str());
        ctlGlossNatList.AddString(name.c_str());
        ctlPhraseList1List.AddString(name.c_str());
        ctlPhraseList2List.AddString(name.c_str());
        it++;
    }

    // Phonemic, Phonetic, and English Gloss already hard coded. Retrieve rest of preferences from last export
    GetLastExport(settings.LAST_EXPORT_REFERENCE, &ctlReferenceList);
    GetLastExport(settings.LAST_EXPORT_ORTHO, &ctlOrthoList);
    GetLastExport(settings.LAST_EXPORT_GLOSS_NAT, &ctlGlossNatList);
    GetLastExport(settings.LAST_EXPORT_PHRASE1, &ctlPhraseList1List);
    GetLastExport(settings.LAST_EXPORT_PHRASE2, &ctlPhraseList2List);

    CSaApp* pApp = (CSaApp*)AfxGetApp();
    CString lastOptionalLanguageTag = pApp->GetProfileString(L"Lift", settings.LAST_EXPORT_OPTIONAL_LANGUAGE_TAG, L"");
    if (!lastOptionalLanguageTag.IsEmpty()) {
      settings.optionalLanguageTag = lastOptionalLanguageTag;
    }

    UpdateData(FALSE);

	// return TRUE  unless you set the focus to a control
    return TRUE;  
}

void CDlgExportLift::DoDataExchange(CDataExchange * pDX) {

    CDialog::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_EXTAB_GLOSS, settings.bGloss);
    DDX_Check(pDX, IDC_EXTAB_GLOSS_NAT, settings.bGlossNat);
    DDX_Check(pDX, IDC_EXTAB_ORTHO, settings.bOrtho);
    DDX_Check(pDX, IDC_EXTAB_PHONEMIC, settings.bPhonemic);
    DDX_Check(pDX, IDC_EXTAB_PHONETIC,settings. bPhonetic);
    DDX_Check(pDX, IDC_EXTAB_REFERENCE, settings.bReference);
    DDX_Check(pDX, IDC_EXTAB_MUSIC_PL1, settings.bPhrase1);
    DDX_Check(pDX, IDC_EXTAB_MUSIC_PL2, settings.bPhrase2);
    DDX_Control(pDX, IDOK, ctlButtonOK);
    DDX_Control(pDX, IDC_LIST_REFERENCE, ctlReferenceList);
    DDX_Control(pDX, IDC_LIST_ORTHO, ctlOrthoList);
    DDX_Control(pDX, IDC_LIST_GLOSS_NAT, ctlGlossNatList);
    DDX_Control(pDX, IDC_LIST_MUSIC_PL1, ctlPhraseList1List);
    DDX_Control(pDX, IDC_LIST_MUSIC_PL2, ctlPhraseList2List);
    DDX_Control(pDX, IDC_EDIT_FOLDER, ctlEditFolder);
    DDX_CBString(pDX, IDC_OPTIONAL_LANGUAGE_TAG, settings.optionalLanguageTag);

    if (pDX->m_bSaveAndValidate) {

        CString buffer;
        ctlEditFolder.GetWindowTextW(buffer);
        settings.szPath = buffer;

        // Gloss English is hardcoded to English
        settings.gloss = L"en";

        ctlGlossNatList.GetWindowTextW(buffer);
        settings.glossNat = lookupLanguageID(buffer);

        ctlReferenceList.GetWindowTextW(buffer);
        settings.reference = lookupLanguageID(buffer);

        ctlOrthoList.GetWindowTextW(buffer);
        settings.ortho = lookupLanguageID(buffer);

        // Hardcode Phonemic and Phonetic list. Note: Flex just uses "-fonipa" for phonetic
        settings.phonemic = L"-fonipa-x-emic";

        settings.phonetic = L"-fonipa";

        ctlPhraseList1List.GetWindowTextW(buffer);
        settings.phrase1 = lookupLanguageID(buffer);

        ctlPhraseList2List.GetWindowTextW(buffer);
        settings.phrase2 = lookupLanguageID(buffer);
    }
}

// Previously named lookupCountryCode
// Given a language name, find the matching language ID.
// If not found, return empty string
CString CDlgExportLift::lookupLanguageID(LPCTSTR value) {
    map<wstring,wstring>::iterator it = countryCodes.find(value);
    if (it!=countryCodes.end()) return it->second.c_str();
    return L"";
}

// Given a language ID (previously countryCode), find the matching language name
// If not found, return empty string
CString CDlgExportLift::lookupLanguageName(LPCTSTR value) {
    for (map<wstring, wstring>::iterator it = countryCodes.begin(); it != countryCodes.end(); it++) {
      if (it->second == value) {
        return CString(it->first.c_str());
        }
    }

    return CString(L"");
}

// Search a ComboBox for a matching preference string. 
// If found, set the selection. Otherwise, defaults to first selection
void CDlgExportLift::GetLastExport(LPCTSTR key, CComboBox *comboBox) {
    CSaApp* pApp = (CSaApp*)AfxGetApp();
    CString lastPreference = pApp->GetProfileString(L"Lift", key, L"");

    // Cast the last preference and find the matching language name
    LPCTSTR lastPreferencePtr = (LPCTSTR)lastPreference;
    CString languageName = lookupLanguageName(lastPreferencePtr);
    int index = comboBox->FindString(-1, (LPCTSTR)languageName);
    if (index != CB_ERR) {
        comboBox->SetCurSel(index);
    } else {
        comboBox->SetCurSel(0);
    }
}

void CDlgExportLift::SetEnable(int nItem, BOOL bEnable) {
    CWnd * pWnd = GetDlgItem(nItem);
    if (pWnd) {
        pWnd->EnableWindow(bEnable);
    }
}

void CDlgExportLift::SetCheck(int nItem, BOOL bChecked) {
    CButton * pWnd = (CButton *) GetDlgItem(nItem);
    if (pWnd) {
        pWnd->SetCheck(bChecked);
    }
}

void CDlgExportLift::OnHelpExportBasic() {

    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath = szPath + "::/User_Interface/Menus/File/Export/Standard_format_or_extensible_markup.htm";
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM /*lParam*/, LPARAM lpData) {

    // If the BFFM_INITIALIZED message is received
    // set the path to the start path.
    switch (uMsg) {
    case BFFM_INITIALIZED: {
        if (NULL != lpData) {
		    SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
        }
    }
    }
    return 0;
}

void CDlgExportLift::OnClickedBrowse() {

    // szCurrent is an optional start folder. Can be NULL.
    // szPath receives the selected path on success. Must be MAX_PATH characters in length.

    CoInitialize(NULL);

    TCHAR szDisplay[MAX_PATH];
    memset(szDisplay, 0, sizeof(szDisplay));

    TCHAR szFolderLocation[MAX_PATH];
    memset(szFolderLocation, 0, sizeof(szFolderLocation));

    CString msg;
    msg.LoadStringW(IDS_CHOOSE_FOLDER);

    ctlEditFolder.GetWindowTextW(szFolderLocation, MAX_PATH);

    BROWSEINFO bi = { 0 };
    bi.hwndOwner = this->m_hWnd;
    bi.pszDisplayName = szDisplay;
    bi.lpszTitle = msg.GetBuffer(msg.GetLength());
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.lpfn = BrowseCallbackProc;
    bi.lParam = (LPARAM)(LPCTSTR)szFolderLocation;
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl == NULL) {
        // they canceled...
        msg.ReleaseBuffer();
        CoUninitialize();
        return;
    }

    msg.ReleaseBuffer();

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
        ctlEditFolder.SetWindowText(temp.c_str());
    }

    CoUninitialize();
}

void CDlgExportLift::UpdateButtonState() {
    ctlButtonOK.EnableWindow(TRUE);
}
