/////////////////////////////////////////////////////////////////////////////
// DlgASSTart.cpp:
// Implementation of the CDlgASSTart (dialog) class.
//
// Author: Kent Gorham
// copyright 2015 JAARS Inc. SIL
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DlgASStart.h"
#include "ch_dlg.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "sa.h"
#include "FileUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CDlgASStart, CDialog)

BEGIN_MESSAGE_MAP(CDlgASStart, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE1, &CDlgASStart::OnClickedBrowseAudio)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE2, &CDlgASStart::OnClickedBrowsePhrases)
	ON_EN_CHANGE(IDC_EDIT_AUDIO_FILENAME, &CDlgASStart::OnChange)
	ON_EN_CHANGE(IDC_EDIT_PHRASE_FILENAME, &CDlgASStart::OnChange)
	ON_BN_CLICKED(IDC_CHECK_SEGMENT_AUDIO, &CDlgASStart::OnChange)
	ON_BN_CLICKED(IDC_CHECK_LOAD_DATA, &CDlgASStart::OnChange)
END_MESSAGE_MAP()

CDlgASStart::CDlgASStart(CWnd * pParent) : CDialog(CDlgASStart::IDD, pParent) {
	segmentAudio = TRUE;
	loadData = TRUE;
}

void CDlgASStart::DoDataExchange(CDataExchange * pDX) {

    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_AUDIO_FILENAME, audioFilename);
    DDX_Text(pDX, IDC_EDIT_PHRASE_FILENAME, phraseFilename);
	DDX_Text(pDX, IDC_COMBO_SKIP, skipCount);
    DDX_Check(pDX, IDC_CHECK_SEGMENT_AUDIO, segmentAudio);
    DDX_Check(pDX, IDC_CHECK_LOAD_DATA, loadData);

    DDX_Control(pDX, IDC_BUTTON_BROWSE2, browse2);
    DDX_Control(pDX, IDC_EDIT_AUDIO_FILENAME, edit1);
    DDX_Control(pDX, IDC_EDIT_PHRASE_FILENAME, edit2);
    DDX_Control(pDX, IDC_CHECK_SEGMENT_AUDIO, checkbox1);
    DDX_Control(pDX, IDC_CHECK_LOAD_DATA, checkbox2);
	DDX_Control(pDX, IDOK, ok);
	DDX_Control(pDX, IDC_COMBO_SKIP, combo1);
	DDX_Control(pDX, IDC_STATIC_TITLE, text1);
}

BOOL CDlgASStart::OnInitDialog() {

	CSaApp * pApp = (CSaApp*)AfxGetApp();
	CDialog::OnInitDialog();
	CenterWindow();
	// find the first entry on the MRU and use that as an initializer
    // prompt the user (with all document templates)
    CString path;
    for (int i=0; i < _AFX_MRU_MAX_COUNT; i++) {
        path = pApp->GetMRUFilePath(i);
        if (path.GetLength() == 0) continue;
        CFileStatus status;
		if (!CFile::GetStatus(path, status)) continue;
		if (status.m_attribute & CFile::directory) continue;
        break;
    }
	audioFilename = path;
	UpdateData(FALSE);
	OnChange();
	// setting selection to force caret to end of edit
	// field for really long filenames.
	edit1.SetFocus();
	edit1.SetSel(-1);
	edit2.SetSel(-1);

	combo1.SetCurSel(3);

	// return TRUE  unless you set the focus to a control
    return FALSE;
}

void CDlgASStart::OnClickedBrowseAudio() {

	CSaApp * pApp = (CSaApp*)AfxGetApp();
    CSaString szFilter = "MP3 Files (*.mp3) |*.mp3|WAV Files (*.wav) |*.wav|All Files (*.*) |*.*||";
    CFileDialog dlg(TRUE,_T("mp3"),audioFilename,OFN_HIDEREADONLY,szFilter,NULL);
	CString defaultDir = pApp->GetDefaultDir();
    dlg.m_ofn.lpstrInitialDir = defaultDir;
    if (dlg.DoModal()!=IDOK) {
        return;
    }
    audioFilename = dlg.GetPathName();
	UpdateData(FALSE);
	OnChange();
	edit1.SetSel(-1);
}

void CDlgASStart::OnClickedBrowsePhrases() {

	if ((audioFilename.GetLength()!=0) &&
	    (FileUtils::FileExists(audioFilename))) {
		// audio is OK
		// update phrase if need be
		CString datapath = FileUtils::ReplaceExtension( audioFilename, L".saxml").c_str();
		if (FileUtils::FileExists(datapath)) {
			int result = AfxMessageBox(IDS_RESTART_CONFIRM,MB_YESNO|MB_ICONQUESTION);
			if (result!=IDYES) {
				return;
			}
			::DeleteFile(datapath);
			edit2.SetWindowTextW(L"");
			phraseFilename = L"";
			OnChange();
		}
	}

	CSaApp * pApp = (CSaApp*)AfxGetApp();
	CSaString szFilter = "Phrase Files (*.phrases) |*.phrases|Text Files (*.txt) |*.txt|All Files (*.*) |*.*||";
	CFileDialog dlg(TRUE,_T("phrases"),phraseFilename,OFN_HIDEREADONLY,szFilter,NULL);
	CString defaultDir = pApp->GetDefaultDir();
	dlg.m_ofn.lpstrInitialDir = defaultDir;
	if (dlg.DoModal()!=IDOK) {
		return;
	}
	phraseFilename = dlg.GetPathName();
	// if they just selected something, and it's valid, enable segmentation and loading
	if ((phraseFilename.GetLength()>0)&&(FileUtils::FileExists(phraseFilename))) {
		segmentAudio = TRUE;
		loadData = TRUE;
	}
	UpdateData(FALSE);
	OnChange();
	edit2.SetSel(-1);
}

void CDlgASStart::OnChange() {

	UpdateData(TRUE);

	if (audioFilename.GetLength()==0) {
		// let them edit but not get out
		edit2.EnableWindow(FALSE);
		CString title;
		title.LoadStringW(IDS_BROWSE);
		browse2.SetWindowTextW(title);
		browse2.EnableWindow(FALSE);
		checkbox1.EnableWindow(FALSE);
		text1.EnableWindow(FALSE);
		combo1.EnableWindow(FALSE);
		checkbox2.EnableWindow(FALSE);
		ok.EnableWindow(FALSE);
	} else if (FileUtils::FileExists(audioFilename)) {
		// audio is OK
		// update phrase if need be
		CString datapath = FileUtils::ReplaceExtension( audioFilename, L".saxml").c_str();
		if (FileUtils::FileExists(datapath)) {
			// a data file already exists. 
			// disable the controls
			phraseFilename = datapath;
			segmentAudio = FALSE;
			loadData = FALSE;
			edit2.EnableWindow(FALSE);
			CString title;
			title.LoadStringW(IDS_RESTART_PROJECT);
			browse2.SetWindowTextW(title);
			browse2.EnableWindow(TRUE);
			checkbox1.EnableWindow(FALSE);
			text1.EnableWindow(FALSE);
			combo1.EnableWindow(FALSE);
			checkbox2.EnableWindow(FALSE);
			ok.EnableWindow(TRUE);
		} else {
			// we don't know state of data file
			if (phraseFilename.GetLength()==0) {
				// it's ok to just load the audio
				segmentAudio = FALSE;
				loadData = FALSE;
				edit2.EnableWindow(TRUE);
				CString title;
				title.LoadStringW(IDS_BROWSE);
				browse2.SetWindowTextW(title);
				browse2.EnableWindow(TRUE);
				checkbox1.EnableWindow(FALSE);
				text1.EnableWindow(FALSE);
				combo1.EnableWindow(FALSE);
				checkbox2.EnableWindow(FALSE);
				ok.EnableWindow(TRUE);
			} else if (FileUtils::FileExists(phraseFilename)) {
				// valid phrase file
				edit2.EnableWindow(TRUE);
				CString title;
				title.LoadStringW(IDS_BROWSE);
				browse2.SetWindowTextW(title);
				browse2.EnableWindow(TRUE);
				// enable segmentation if need be
				// this can happen with or without phrase file
				checkbox1.EnableWindow(TRUE);
				text1.EnableWindow(TRUE);
				combo1.EnableWindow(TRUE);
				bool segmenting = (checkbox1.GetCheck()==1);
				// enable OK button
				if (segmenting) {
					// enable locate if need be - dependent on phrase
					checkbox2.EnableWindow(TRUE);
					ok.EnableWindow(TRUE);
				} else {
					// not segmenting, no loading
					// we will now be in a valid state
					loadData=FALSE;
					checkbox2.EnableWindow(FALSE);
					ok.EnableWindow(TRUE);
				}
			} else {
				// invalid phrase data
				edit2.EnableWindow(TRUE);
				CString title;
				title.LoadStringW(IDS_BROWSE);
				browse2.SetWindowTextW(title);
				browse2.EnableWindow(TRUE);
				segmentAudio = FALSE;
				loadData = FALSE;
				checkbox1.EnableWindow(FALSE);
				text1.EnableWindow(FALSE);
				combo1.EnableWindow(FALSE);
				checkbox2.EnableWindow(FALSE);
				ok.EnableWindow(FALSE);
			}
		}
	} else {
		// audio file doesn't exist
		// let them edit, but not exit
		edit2.EnableWindow(FALSE);
		CString title;
		title.LoadStringW(IDS_BROWSE);
		browse2.SetWindowTextW(title);
		browse2.EnableWindow(FALSE);
		checkbox1.EnableWindow(FALSE);
		text1.EnableWindow(FALSE);
		combo1.EnableWindow(FALSE);
		checkbox2.EnableWindow(FALSE);
		ok.EnableWindow(FALSE);
	}
	UpdateData(FALSE);
}

int CDlgASStart::GetSkipCount() {
	return ::_wtoi(skipCount);
}

