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
#include "Process\sa_p_grappl.h"
#include "Process\sa_p_custompitch.h"
#include "Process\sa_p_smoothedpitch.h"
#include "Process\sa_p_melogram.h"
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
    ON_CBN_SELCHANGE(IDC_COMBO_FIELDWORKS_PROJECT, OnSelchangeComboFieldworksProject)
    ON_CBN_KILLFOCUS(IDC_COMBO_FIELDWORKS_PROJECT, OnKillfocusComboFieldworksProject)
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

    CSaString fieldworksdir = GetFieldWorksProjectDirectory();
    bool found = true;
    if (fieldworksdir.GetLength()==0) {
        AfxMessageBox(IDS_NO_FIELDWORKS_PROJECT,MB_OK|MB_ICONEXCLAMATION);
        found = false;
    }
    fieldworksdir.Append(L"\\");
    fieldworksdir.Append(L"*");

    CFileFind finder;
    if (finder.FindFile(fieldworksdir)) {
        BOOL more = TRUE;
        do {
            more = finder.FindNextFile();
            if (finder.IsDots()) {
                continue;
            }
            if (!finder.IsDirectory()) {
                continue;
            }
            TRACE(L"Found %s\n", finder.GetFileName());
            ctlComboFieldWorksProject.AddString((LPCTSTR) finder.GetFileName());
        } while (more);
    }
    int count = ctlComboFieldWorksProject.GetCount();
	if (count==1) {
		ctlComboFieldWorksProject.SetCurSel(0);
	}
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

    // Phonemic, Phonetic, and English Gloss already hard coded

    ctlReferenceList.SetCurSel(0);
    ctlOrthoList.SetCurSel(0);
    ctlGlossNatList.SetCurSel(0);
	  ctlPhraseList1List.SetCurSel(0);
	  ctlPhraseList2List.SetCurSel(0);

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
    DDX_Control(pDX, IDC_COMBO_FIELDWORKS_PROJECT, ctlComboFieldWorksProject);

    if (pDX->m_bSaveAndValidate) {

		CString buffer;
        ctlEditFolder.GetWindowTextW(buffer);
        settings.szPath = buffer;

        // Gloss English is hardcoded to English
        settings.gloss = L"en";

        ctlGlossNatList.GetWindowTextW(buffer);
        settings.glossNat = lookupCountryCode(buffer);

        ctlReferenceList.GetWindowTextW(buffer);
        settings.reference = lookupCountryCode(buffer);

        ctlOrthoList.GetWindowTextW(buffer);
        settings.ortho = lookupCountryCode(buffer);

        // Hardcode Phonemic and Phonetic list
        settings.phonemic = L"-fonipa-x-emic";

        settings.phonetic = L"-fonipa-x-etic";

        ctlPhraseList1List.GetWindowTextW(buffer);
        settings.phrase1 = lookupCountryCode(buffer);

        ctlPhraseList2List.GetWindowTextW(buffer);
        settings.phrase2 = lookupCountryCode(buffer);

        ctlComboFieldWorksProject.ResetContent();

    }
}

CString CDlgExportLift::lookupCountryCode(LPCTSTR value) {
	map<wstring,wstring>::iterator it = countryCodes.find(value);
	if (it!=countryCodes.end()) return it->second.c_str();
	return L"";
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
void CDlgExportLift::OnSelchangeComboFieldworksProject() {
    UpdateButtonState();
}


void CDlgExportLift::OnKillfocusComboFieldworksProject() {
    UpdateButtonState();
}

void CDlgExportLift::UpdateButtonState() {
    bool selected = (ctlComboFieldWorksProject.GetCurSel()!=-1);
    ctlButtonOK.EnableWindow((selected)?TRUE:FALSE);
}

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

bool CDlgExportLift::SearchForValue(HKEY hRootKey, DWORD sam, wstring keyName, LPCTSTR valueName, wstring & value) {
    TRACE(L"Trying %s\n", keyName.c_str());

    HKEY hKey = NULL;
    DWORD retCode = RegOpenKeyEx(hRootKey, keyName.c_str(), 0, sam, &hKey);
    if (retCode!=ERROR_SUCCESS) {
        TRACE(L"Unable to open %s because of %d\n", keyName.c_str(), retCode);
        return false;
    }

    TCHAR achKey[MAX_KEY_LENGTH];       // buffer for subkey name
    DWORD cbName;                       // size of name string
    TCHAR achClass[MAX_PATH] = TEXT("");  // buffer for class name
    DWORD cchClassName = MAX_PATH;      // size of class string
    DWORD cSubKeys=0;                   // number of subkeys
    DWORD cbMaxSubKey;                  // longest subkey size
    DWORD cchMaxClass;                  // longest class string
    DWORD cValues;                      // number of values for key
    DWORD cchMaxValue;                  // longest value name
    DWORD cbMaxValueData;               // longest value data
    DWORD cbSecurityDescriptor;         // size of security descriptor
    FILETIME ftLastWriteTime;           // last write time
    TCHAR achValue[MAX_VALUE_NAME];
    DWORD cchValue = MAX_VALUE_NAME;

    // Get the class name and the value count.
    retCode = RegQueryInfoKey(hKey, achClass, &cchClassName, NULL, &cSubKeys, &cbMaxSubKey, &cchMaxClass, &cValues, &cchMaxValue, &cbMaxValueData, &cbSecurityDescriptor, &ftLastWriteTime);
    if (retCode!=ERROR_SUCCESS) {
        RegCloseKey(hKey);
        TRACE(L"unable to query key info %s because of %d\n", keyName.c_str(), retCode);
        return false;
    }

    // Enumerate the subkeys, until RegEnumKeyEx fails.
    if (cSubKeys>0) {
        TRACE(L"Number of subkeys: %d\n", cSubKeys);
        for (DWORD i=0; i<cSubKeys; i++) {
            cbName = MAX_KEY_LENGTH;
            memset(achKey,0,sizeof(achKey));
            retCode = RegEnumKeyEx(hKey, i, achKey, &cbName, NULL, NULL, NULL, &ftLastWriteTime);
            if (retCode == ERROR_SUCCESS) {
                wstring childKey;
                childKey = keyName;
                childKey.append(L"\\");
                childKey.append(achKey);
                if (SearchForValue(hRootKey, sam, childKey, valueName, value)) {
                    RegCloseKey(hKey);
                    return true;
                }
            }
        }
    } else {
        TRACE(L"No sub keys found\n");
    }

    // Enumerate the key values.
    if (cValues>0) {
        TRACE(L"number of values: %d\n", cValues);
        for (DWORD i=0, retCode=ERROR_SUCCESS; i<cValues; i++) {
            cchValue = MAX_VALUE_NAME;
            wmemset(achValue,0,_countof(achValue));
            retCode = RegEnumValue(hKey, i, achValue, &cchValue, NULL, NULL, NULL, NULL);
            if (retCode == ERROR_SUCCESS) {
                TRACE(L"considering value %s\n",achValue);
                // is this the correct value?
                if (_wcsicmp(achValue, valueName)==0) {
                    TCHAR szValue[1024];
                    wmemset(szValue,0,_countof(szValue));
                    DWORD dwBufLen = sizeof(szValue);
                    retCode = RegQueryValueEx(hKey, _T("ProjectsDir"), NULL, NULL, (LPBYTE)szValue, &dwBufLen);
                    if (retCode==ERROR_SUCCESS) {
                        value = szValue;
                        TRACE(L"success at %s : %s\n", keyName.c_str(), value.c_str());
                        RegCloseKey(hKey);
                        return true;
                    }
                }
            }
        }
    } else {
        TRACE(L"No values found\n");
    }
    TRACE(L"no joy\n");
    RegCloseKey(hKey);
    return false;
}

/**
* Return the registered fieldworks project directory
* The FieldWorks application stores it's information under registry keys that are named
* according to the version - so we will recursively hunt for the ProjectsDir value
* to figure out where they are storing their projects.
*
* and to make things really interesting, FieldWorks doesn't cleanup the registry very
* well when it is uninstalled - so we need to check the validity of the registry entry.
*
*/
CSaString CDlgExportLift::GetFieldWorksProjectDirectory() {
    // first try the 32-bit registry hive
    DWORD sam = KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_READ | KEY_WOW64_32KEY;
    wstring value;
    HKEY hRootKey = HKEY_LOCAL_MACHINE;
    wstring keyName = _T("Software\\SIL\\FieldWorks");
    if (SearchForValue(hRootKey, sam, keyName, L"ProjectsDir", value)) {
        if (FileUtils::FolderExists(value.c_str())) {
            return CSaString(value.c_str());
        }
    }
    hRootKey = HKEY_CURRENT_USER;
    if (SearchForValue(hRootKey, sam, keyName, L"ProjectsDir", value)) {
        if (FileUtils::FolderExists(value.c_str())) {
            return CSaString(value.c_str());
        }
    }

    // now try the 64-bit registry hive
    sam = KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_READ;
    hRootKey = HKEY_LOCAL_MACHINE;
    if (SearchForValue(hRootKey, sam, keyName, L"ProjectsDir", value)) {
        if (FileUtils::FolderExists(value.c_str())) {
            return CSaString(value.c_str());
        }
    }
    hRootKey = HKEY_CURRENT_USER;
    if (SearchForValue(hRootKey, sam, keyName, L"ProjectsDir", value)) {
        if (FileUtils::FolderExists(value.c_str())) {
            return CSaString(value.c_str());
        }
    }

    OSVERSIONINFO versionInfo;
    memset(&versionInfo,0,sizeof(OSVERSIONINFO));
    versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    BOOL result = ::GetVersionEx(&versionInfo);
    ASSERT(result);
    bool xp = versionInfo.dwMajorVersion<6;
    xp = true;
    if (xp) {
        wstring path = GetShellFolderPath(CSIDL_COMMON_APPDATA);
        if (path.length()>0) {
            wstring projPath = path;
            projPath.append(L"\\SIL\\FieldWorks\\Projects");
            if (FileUtils::FolderExists(projPath.c_str())) {
                return CSaString(projPath.c_str());
            } else {
                projPath = path;
                projPath.append(L"\\SIL\\FieldWorks 7\\Projects");
                if (FileUtils::FolderExists(projPath.c_str())) {
                    return CSaString(projPath.c_str());
                }
            }
        }
    } else {
        wstring path = _wgetenv(L"ProgramData");
        if (path.length()>0) {
            wstring projPath = path;
            projPath.append(L"\\SIL\\FieldWorks\\Projects");
            if (FileUtils::FolderExists(projPath.c_str())) {
                return CSaString(projPath.c_str());
            } else {
                projPath = path;
                projPath.append(L"\\SIL\\FieldWorks 7\\Projects");
                if (FileUtils::FolderExists(projPath.c_str())) {
                    return CSaString(projPath.c_str());
                }
            }
        }
    }

    // it's really bad!!
    return CSaString(L"");
}

