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

//****************************************************************************
// Added on 07/27/200 by DDO.
//****************************************************************************
CSaString CDlgExportFW::GetExportFilename(CSaString szTitle, CSaString szFilter, TCHAR * szExtension) {

    int nFind = szTitle.Find(':');
    if (nFind != -1) {
        szTitle = szTitle.Left(nFind);
    }
    nFind = szTitle.ReverseFind('.');

	// remove extension
    szTitle.Trim();
    if (nFind >= ((szTitle.GetLength() > 3) ? (szTitle.GetLength() - 4) : 0)) {
        szTitle = szTitle.Left(nFind);
    }

    CFileDialog dlg(FALSE, szExtension, szTitle, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL);

	TCHAR szBuffer[MAX_PATH];
	wmemset(szBuffer, 0, MAX_PATH);

    GetCurrentPath( szBuffer, MAX_PATH);

    dlg.m_ofn.lpstrFile = szBuffer;
    dlg.m_ofn.lpstrInitialDir = szBuffer;

    if (dlg.DoModal() != IDOK) {
        return "";
    }
    return dlg.GetPathName();
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

CDlgExportFW::CDlgExportFW(const CSaString & szDocTitle, CWnd * pParent) : CDialog(CDlgExportFW::IDD, pParent) {
    m_bAllAnnotations = TRUE;
    m_bGloss = TRUE;
    m_bOrtho = TRUE;
    m_bPhonemic = TRUE;
    m_bPhonetic = TRUE;
    m_bPOS = FALSE;
    m_bReference = FALSE;
    m_bTone = FALSE;
    m_bPhrase = FALSE;
    m_szDocTitle = szDocTitle;
}

BOOL CDlgExportFW::OnInitDialog() {

    CDialog::OnInitDialog();

    OnAllAnnotations();

    SetEnable(IDC_EX_SFM_RECORD_DATA, FALSE);
    SetEnable(IDC_EX_SFM_BANDWIDTH, FALSE);
    SetEnable(IDC_EX_SFM_BITS, FALSE);
    SetEnable(IDC_EX_SFM_QUANTIZATION, FALSE);
    SetEnable(IDC_EX_SFM_HIGHPASS, FALSE);
    SetEnable(IDC_EX_SFM_LENGTH, FALSE);
    SetEnable(IDC_EX_SFM_NUMBER_OF_SAMPLES, FALSE);
    SetEnable(IDC_EX_SFM_RATE, FALSE);

    SetEnable(IDC_EX_SFM_ALL_SOURCE, FALSE);
    SetEnable(IDC_EX_SFM_LANGUAGE, FALSE);
    SetEnable(IDC_EX_SFM_DIALECT, FALSE);
    SetEnable(IDC_EX_SFM_SPEAKER, FALSE);
    SetEnable(IDC_EX_SFM_GENDER, FALSE);
    SetEnable(IDC_EX_SFM_ETHNOLOGUE_ID, FALSE);
    SetEnable(IDC_EX_SFM_FAMILY, FALSE);
    SetEnable(IDC_EX_SFM_REGION, FALSE);
    SetEnable(IDC_EX_SFM_COUNTRY, FALSE);
    SetEnable(IDC_EX_SFM_NOTEBOOKREF, FALSE);
    SetEnable(IDC_EX_SFM_TRANSCRIBER, FALSE);
    SetEnable(IDC_EX_SFM_COMMENTS, FALSE);

    SetEnable(IDC_EX_SFM_FREE, FALSE);
    SetCheck(IDC_EX_SFM_FREE, FALSE);

    SetEnable(IDC_EX_SFM_PHONES, FALSE);
    SetCheck(IDC_EX_SFM_PHONES, FALSE);

    SetEnable(IDC_EX_SFM_WORDS, FALSE);
    SetCheck(IDC_EX_SFM_WORDS, FALSE);

    SetEnable(IDC_EX_SFM_FILENAME, FALSE);
    SetCheck(IDC_EX_SFM_FILENAME, TRUE);

    SetEnable(IDC_EX_SFM_FILE_INFO, FALSE);
    SetCheck(IDC_EX_SFM_FILE_INFO, TRUE);

    SetEnable(IDC_EX_SFM_MULTIRECORD, FALSE);
    SetEnable(IDC_EX_SFM_INTERLINEAR, FALSE);
    SetCheck(IDC_EX_SFM_MULTIRECORD, TRUE);

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


void CDlgExportFW::OnOK() {

	CSaString szFileName = GetExportFilename( m_szDocTitle, _T("Standard Format (*.sfm) |*.sfm||"), _T("sfm"));
    if (szFileName == "") {
		CDialog::OnOK();
		return;
    }

    UpdateData(TRUE);

    // process all flags
    if (m_bAllAnnotations) {
        m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bPOS = m_bPhrase = TRUE;
    }

	EWordFilenameConvention convention = WFC_GLOSS;
	if (m_bReference) {
		if (m_bGloss) {
			convention = WFC_REF_GLOSS;
		} else {
			convention = WFC_REF;
		}
	}

	bool skipEmptyGloss = true;

	CSaDoc * pDoc = (CSaDoc *)((CMainFrame *) AfxGetMainWnd())->GetCurrSaView()->GetDocument();

	TCHAR szBuffer[MAX_PATH];
	memset(szBuffer, 0, MAX_PATH);
	GetCurrentPath(szBuffer,MAX_PATH);
	if (!FolderExists(szBuffer)) {
		CreateFolder(szBuffer);
	}
	wcscat_s(szBuffer,MAX_PATH,L"LinkedFiles\\");
	if (!FolderExists(szBuffer)) {
		CreateFolder(szBuffer);
	}
	wcscat_s(szBuffer,MAX_PATH,L"AudioVisual\\");
	if (!FolderExists(szBuffer)) {
		CreateFolder(szBuffer);
	}

	if (!ValidateWordFilenames(convention,szBuffer,skipEmptyGloss)) {
		CDialog::OnOK();
		return;
	}

    CFile file( szFileName, CFile::modeCreate | CFile::modeWrite);
    CSaString szString;
	int count = 0;

    if (!TryExportSegmentsBy(REFERENCE, pDoc, file, count, skipEmptyGloss, convention, szBuffer)) {
        if (!TryExportSegmentsBy(GLOSS, pDoc, file, count, skipEmptyGloss, convention, szBuffer)) {
            if (!TryExportSegmentsBy(ORTHO, pDoc, file, count, skipEmptyGloss, convention, szBuffer)) {
                if (!TryExportSegmentsBy(PHONEMIC, pDoc, file, count, skipEmptyGloss, convention, szBuffer)) {
                    if (!TryExportSegmentsBy(TONE, pDoc, file, count, skipEmptyGloss, convention, szBuffer)) {
                        TryExportSegmentsBy(PHONETIC, pDoc, file, count, skipEmptyGloss, convention, szBuffer);
                    }
                }
            }
        }
    }

    // \date write current time
    CTime time = CTime::GetCurrentTime();
    szString = "\\dt " + time.Format("%A, %B %d, %Y, %X") + "\r\n";
    WriteFileUtf8(&file, szString);

    file.Close();

    CDialog::OnOK();
}

bool CDlgExportFW::TryExportSegmentsBy( Annotations master, CSaDoc * pDoc, CFile & file, int & count, bool skipEmptyGloss, EWordFilenameConvention convention, LPCTSTR szPath) {

    if (!GetFlag(master)) {
        return false;
    }

    CSegment * pSeg = pDoc->GetSegment(master);

    if (pSeg->GetOffsetSize() == 0) {
        return false;
    }

    CSaString szCrLf = "\r\n";
    WriteFileUtf8(&file, szCrLf);

    CSaString results[ANNOT_WND_NUMBER];
    for(int i = 0; i < ANNOT_WND_NUMBER; i++) {
        results[i] = L"";
    }
    DWORD last = pSeg->GetOffset(0) - 1;
    for(int i = 0; i < pSeg->GetOffsetSize(); i++) {
        DWORD dwStart = pSeg->GetOffset(i);
        DWORD dwStop = pSeg->GetStop(i);
        if (dwStart == last) {
            continue;
        }
        last = dwStart;
        for(int j = master; j >= 0; j--) {
            Annotations target = GetAnnotation(j);
            if (!GetFlag(target)) {
                continue;
            }
            results[target] = BuildRecord(target, dwStart, dwStop, pDoc);
        }

        if (m_bPhrase) {
            results[MUSIC_PL1] = BuildPhrase(MUSIC_PL1, dwStart, dwStop, pDoc);
            results[MUSIC_PL2] = BuildPhrase(MUSIC_PL2, dwStart, dwStop, pDoc);
            results[MUSIC_PL3] = BuildPhrase(MUSIC_PL3, dwStart, dwStop, pDoc);
            results[MUSIC_PL4] = BuildPhrase(MUSIC_PL4, dwStart, dwStop, pDoc);
        }

        if (results[REFERENCE].GetLength() > 0) {
            WriteFileUtf8(&file, results[REFERENCE]);
        }
        if (results[PHONETIC].GetLength() > 0) {
            WriteFileUtf8(&file, results[PHONETIC]);
        }
        if (results[TONE].GetLength() > 0) {
            WriteFileUtf8(&file, results[TONE]);
        }
        if (results[PHONEMIC].GetLength() > 0) {
            WriteFileUtf8(&file, results[PHONEMIC]);
        }
        if (results[ORTHO].GetLength() > 0) {
            WriteFileUtf8(&file, results[ORTHO]);
        }
        if (results[GLOSS].GetLength() > 0) {
            WriteFileUtf8(&file, results[GLOSS]);
        }
        if (results[MUSIC_PL1].GetLength() > 0) {
            WriteFileUtf8(&file, results[MUSIC_PL1]);
        }
        if (results[MUSIC_PL2].GetLength() > 0) {
            WriteFileUtf8(&file, results[MUSIC_PL2]);
        }
        if (results[MUSIC_PL3].GetLength() > 0) {
            WriteFileUtf8(&file, results[MUSIC_PL3]);
        }
        if (results[MUSIC_PL4].GetLength() > 0) {
            WriteFileUtf8(&file, results[MUSIC_PL4]);
        }

		POSITION pos = pDoc->GetFirstViewPosition();
		CSaView * pView = (CSaView *) pDoc->GetNextView(pos);  // get pointer to view
		CGlossSegment * g = (CGlossSegment*)pView->GetAnnotation(GLOSS);
		DWORD offsetSize = g->GetOffsetSize();
		bool hasGloss = (offsetSize != 0);

		if ((hasGloss) || (!skipEmptyGloss)) {
			int index = FindNearestGlossIndex(g,dwStart,dwStop);
			TCHAR szTarget[MAX_PATH];
			int result = ComposeWordSegmentFilename( g, index, convention, szPath, skipEmptyGloss, szTarget, MAX_PATH);
			if (result==0) {
				result = ExportWordSegment( count, g, index, convention, szTarget, skipEmptyGloss);
				if (result<0) {
					return false;
				}
				TCHAR szBuffer[MAX_PATH];
				wmemset(szBuffer,0,MAX_PATH);
				wcscat_s(szBuffer,MAX_PATH,L"\\pf ");
				wcscat_s(szBuffer,MAX_PATH,szTarget);
				wcscat_s(szBuffer,MAX_PATH,szCrLf);
				WriteFileUtf8( &file, szBuffer);

				wmemset(szBuffer,0,MAX_PATH);
				wcscat_s(szBuffer,MAX_PATH,L"\\tn ");
				wcscat_s(szBuffer,MAX_PATH,szTarget);
				wcscat_s(szBuffer,MAX_PATH,szCrLf);
				WriteFileUtf8( &file, szBuffer);
			}
		}

        WriteFileUtf8(&file, szCrLf);
    }

    return true;
}


CSaString CDlgExportFW::BuildRecord(Annotations target, DWORD dwStart, DWORD dwStop, CSaDoc * pDoc) {

    CSaString szTag = GetTag(target);
    CSegment * pSegment = pDoc->GetSegment(target);
    CSaString szText = pSegment->GetContainedText(dwStart, dwStop);
    szText = szText.Trim();
    if (szText.GetLength() == 0) {
        return L"";
    }
    if (target == GLOSS) {
        if (szText[0] == WORD_DELIMITER) {
            szText = szText.Right(szText.GetLength() - 1);
        }
    }
    return szTag + L" " + szText + szCrLf;
}

CSaString CDlgExportFW::BuildPhrase(Annotations target, DWORD dwStart, DWORD dwStop, CSaDoc * pDoc) {

    CSaString szTag = GetTag(target);
    CSegment * pSegment = pDoc->GetSegment(GetIndex(target));
    CSaString szText =  pSegment->GetOverlappingText(dwStart, dwStop);
    szText = szText.Trim();
    if (szText.GetLength() == 0) {
        return L"";
    }
    return szTag + L" " + szText + szCrLf;
}

BOOL CDlgExportFW::GetFlag(Annotations val) {
    switch(val) {
    case PHONETIC:
        return m_bPhonetic;
    case TONE:
        return m_bTone;
    case PHONEMIC:
        return m_bPhonemic;
    case ORTHO:
        return m_bOrtho;
    case GLOSS:
        return m_bGloss;
    case REFERENCE:
        return m_bReference;
    case MUSIC_PL1:
        return m_bPhrase;
    case MUSIC_PL2:
        return m_bPhrase;
    case MUSIC_PL3:
        return m_bPhrase;
    case MUSIC_PL4:
        return m_bPhrase;
    }
    return false;
}

int CDlgExportFW::GetIndex(Annotations val) {
    switch(val) {
    case PHONETIC:
        return 0;
    case TONE:
        return 1;
    case PHONEMIC:
        return 2;
    case ORTHO:
        return 3;
    case GLOSS:
        return 4;
    case REFERENCE:
        return 5;
    case MUSIC_PL1:
        return 6;
    case MUSIC_PL2:
        return 7;
    case MUSIC_PL3:
        return 8;
    case MUSIC_PL4:
        return 9;
    }
    return false;
}

LPCTSTR CDlgExportFW::GetTag(Annotations val) {
    switch(val) {
    case PHONETIC:
        return L"\\lx-ph";
    case TONE:
        return L"\\tn";
    case PHONEMIC:
        return L"\\lx-pm";
    case ORTHO:
        return L"\\lx-or";
    case GLOSS:
        return L"\\ge";
    case REFERENCE:
        return L"\\rf";
    case MUSIC_PL1:
        return L"\\pf";
    case MUSIC_PL2:
        return L"\\tn";
    case MUSIC_PL3:
        return L"\\pf";
    case MUSIC_PL4:
        return L"\\tn";
    }
    return L"";
}

Annotations CDlgExportFW::GetAnnotation(int val) {
    switch(val) {
    case 0:
        return PHONETIC;
    case 1:
        return TONE;
    case 2:
        return PHONEMIC;
    case 3:
        return ORTHO;
    case 4:
        return GLOSS;
    case 5:
        return REFERENCE;
    case 6:
        return MUSIC_PL1;
    case 7:
        return MUSIC_PL2;
    case 8:
        return MUSIC_PL3;
    case 9:
        return MUSIC_PL4;
    }
    return PHONETIC;
}

void CDlgExportFW::DoDataExchange(CDataExchange * pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_EXTAB_ANNOTATIONS, m_bAllAnnotations);
	DDX_Check(pDX, IDC_EXTAB_GLOSS, m_bGloss);
	DDX_Check(pDX, IDC_EXTAB_ORTHO, m_bOrtho);
	DDX_Check(pDX, IDC_EXTAB_PHONEMIC, m_bPhonemic);
	DDX_Check(pDX, IDC_EXTAB_PHONETIC, m_bPhonetic);
	DDX_Check(pDX, IDC_EXTAB_POS, m_bPOS);
	DDX_Check(pDX, IDC_EXTAB_REFERENCE, m_bReference);
	DDX_Check(pDX, IDC_EXTAB_TONE, m_bTone);
	DDX_Check(pDX, IDC_EXTAB_PHRASE, m_bPhrase);
	DDX_Control(pDX, IDC_BROWSE_OTHER, m_BrowseOther);
	DDX_Control(pDX, IDC_EDIT_FIELDWORKS_FOLDER, m_EditFieldWorksFolder);
	DDX_Control(pDX, IDC_EDIT_OTHER_FOLDER, m_EditOtherFolder);
	DDX_Control(pDX, IDC_COMBO_FIELDWORKS_PROJECT, m_ComboFieldWorksProject);
	DDX_Control(pDX, IDC_STATIC_FIELDWORKS_PROJECT, m_StaticFieldWorksProject);
	DDX_Control(pDX, IDC_RADIO_FIELDWORKS, m_RadioFieldWorks);
	DDX_Control(pDX, IDC_RADIO_OTHER, m_RadioOther);

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
	}
	DDX_Control(pDX, IDOK, m_ButtonOK);
	DDX_Control(pDX, IDC_STATIC_TAGS, m_StaticTags);
}

void CDlgExportFW::OnAllAnnotations() {
    UpdateData(TRUE);
    BOOL bEnable = !m_bAllAnnotations;
    SetEnable(IDC_EXTAB_PHONETIC, bEnable);
    SetEnable(IDC_EXTAB_TONE, bEnable);
    SetEnable(IDC_EXTAB_PHONEMIC, bEnable);
    SetEnable(IDC_EXTAB_ORTHO, bEnable);
    SetEnable(IDC_EXTAB_GLOSS, bEnable);
    SetEnable(IDC_EXTAB_REFERENCE, bEnable);
    SetEnable(IDC_EXTAB_POS, bEnable);
    SetEnable(IDC_EXTAB_PHRASE, bEnable);
    if (m_bAllAnnotations) {
        m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bPOS = m_bPhrase = TRUE;
        UpdateData(FALSE);
    } else {
        m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bPOS = m_bPhrase = FALSE;
        UpdateData(FALSE);
    }
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

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {

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

    wmemset(szBuffer, 0, size);

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