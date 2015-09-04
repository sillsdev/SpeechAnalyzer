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

CDlgSaveAsOptions::CDlgSaveAsOptions( LPCTSTR lpszDefExt, 
									  LPCTSTR lpszFileName, 
									  LPCTSTR lpszDefaultDir,
									  DWORD dwFlags, 
									  LPCTSTR lpszFilter, 
									  CWnd * pParentWnd, 
									  bool saveAs, 
									  bool stereo,
									  DWORD samplesPerSec) :
    CFileDialog(FALSE, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd, 0, FALSE),
    mSaveArea(saveEntire),
    mShowFiles(showNew),
    mFileFormat(((stereo)?formatStereo:formatMono)),
	mOriginalPath(lpszFileName),
	mStereo(stereo),
	mSaveAs(saveAs),
	mSamplingRate(samplesPerSec),
	mOriginalSamplingRate(samplesPerSec),
	mSamplingChoice(0) {
    SetTemplate(IDD, IDD);
    m_ofn.hInstance = AfxFindResourceHandle(MAKEINTRESOURCE(IDD),RT_DIALOG);
	m_ofn.lpstrInitialDir = lpszDefaultDir;
}

BOOL CDlgSaveAsOptions::OnInitDialog() {
    CFileDialog::OnInitDialog();
    if (mSaveAs) {
        if (mStereo) {
            GetDlgItem(IDC_SAVEAS_MONO)->EnableWindow(TRUE);
            GetDlgItem(IDC_SAVEAS_STEREO)->EnableWindow(TRUE);
            GetDlgItem(IDC_SAVEAS_RIGHT)->EnableWindow(TRUE);
        } else {
            GetDlgItem(IDC_SAVEAS_MONO)->EnableWindow(TRUE);
            GetDlgItem(IDC_SAVEAS_STEREO)->EnableWindow(FALSE);
            GetDlgItem(IDC_SAVEAS_RIGHT)->EnableWindow(FALSE);
        }
    } else {
        GetDlgItem(IDC_SAVEAS_MONO)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_SAVEAS_STEREO)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_SAVEAS_RIGHT)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_GROUP_FILE_STATIC)->ShowWindow(SW_HIDE);
    }
	// setup sampling rate choices
	GetDlgItem(IDC_SAVEAS_SAMPLE1)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_SAVEAS_SAMPLE2)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_SAVEAS_SAMPLE3)->ShowWindow(SW_SHOW);
	wchar_t buffer[20];
	wsprintf(buffer,L"%d Hz",mOriginalSamplingRate);
	if (mOriginalSamplingRate<22050) {
		// upsampling not allowed
		// sample rate is below 22.050 khz
		GetDlgItem(IDC_SAVEAS_SAMPLE1)->EnableWindow(TRUE);
		GetDlgItem(IDC_SAVEAS_SAMPLE2)->EnableWindow(FALSE);
		GetDlgItem(IDC_SAVEAS_SAMPLE3)->EnableWindow(FALSE);
		GetDlgItem(IDC_SAVEAS_SAMPLE1)->SetWindowTextW(buffer);
		GetDlgItem(IDC_SAVEAS_SAMPLE2)->SetWindowTextW(L"22050 Hz");
		GetDlgItem(IDC_SAVEAS_SAMPLE3)->SetWindowTextW(L"44100 Hz");
		mSamplingChoice = 0;
	} else if (mOriginalSamplingRate==22050) {
		GetDlgItem(IDC_SAVEAS_SAMPLE1)->EnableWindow(TRUE);
		GetDlgItem(IDC_SAVEAS_SAMPLE2)->EnableWindow(FALSE);
		GetDlgItem(IDC_SAVEAS_SAMPLE3)->EnableWindow(FALSE);
		GetDlgItem(IDC_SAVEAS_SAMPLE1)->SetWindowTextW(L"22050 Hz");
		GetDlgItem(IDC_SAVEAS_SAMPLE2)->SetWindowTextW(L"44100 Hz");
		GetDlgItem(IDC_SAVEAS_SAMPLE3)->SetWindowTextW(L"96000 Hz");
		mSamplingChoice = 0;
	} else if (mOriginalSamplingRate<44100) {
		// choice between current rate, 22 and x
		GetDlgItem(IDC_SAVEAS_SAMPLE1)->EnableWindow(TRUE);
		GetDlgItem(IDC_SAVEAS_SAMPLE2)->EnableWindow(TRUE);
		GetDlgItem(IDC_SAVEAS_SAMPLE3)->EnableWindow(FALSE);
		GetDlgItem(IDC_SAVEAS_SAMPLE1)->SetWindowTextW(L"22050 Hz");
		GetDlgItem(IDC_SAVEAS_SAMPLE2)->SetWindowTextW(buffer);
		GetDlgItem(IDC_SAVEAS_SAMPLE2)->SetWindowTextW(L"44100 Hz");
		mSamplingChoice = 1;
	} else if (mOriginalSamplingRate==44100) {
		// choice between current rate, 22 and 44
		GetDlgItem(IDC_SAVEAS_SAMPLE1)->EnableWindow(TRUE);
		GetDlgItem(IDC_SAVEAS_SAMPLE2)->EnableWindow(TRUE);
		GetDlgItem(IDC_SAVEAS_SAMPLE3)->EnableWindow(FALSE);
		GetDlgItem(IDC_SAVEAS_SAMPLE1)->SetWindowTextW(L"22050 Hz");
		GetDlgItem(IDC_SAVEAS_SAMPLE2)->SetWindowTextW(L"44100 Hz");
		GetDlgItem(IDC_SAVEAS_SAMPLE3)->SetWindowTextW(L"96000 Hz");
		mSamplingChoice = 1;
	} else {
		// greater than 44100 khz
		GetDlgItem(IDC_SAVEAS_SAMPLE1)->EnableWindow(TRUE);
		GetDlgItem(IDC_SAVEAS_SAMPLE2)->EnableWindow(TRUE);
		GetDlgItem(IDC_SAVEAS_SAMPLE3)->EnableWindow(TRUE);
		GetDlgItem(IDC_SAVEAS_SAMPLE1)->SetWindowTextW(L"22050 Hz");
		GetDlgItem(IDC_SAVEAS_SAMPLE2)->SetWindowTextW(L"44100 Hz");
		GetDlgItem(IDC_SAVEAS_SAMPLE3)->SetWindowTextW(buffer);
		mSamplingChoice = 2;
	}
	UpdateData(FALSE);
    return TRUE;
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
    DDX_Radio(pDX, IDC_SAVEAS_MONO, (int &)mFileFormat);
	DDX_Radio(pDX, IDC_SAVEAS_SAMPLE1, (int&)mSamplingChoice);
	if (pDX->m_bSaveAndValidate) {
		if (mOriginalSamplingRate<22050) {
			// only choice is current rate
			mSamplingRate = mOriginalSamplingRate;
		} else if (mOriginalSamplingRate==22050) {
			// only choice is 22050
			mSamplingRate = mOriginalSamplingRate;
		} else if (mOriginalSamplingRate<44100) {
			if (mSamplingChoice==0) {
				mSamplingRate = 22050;
			} else {
				mSamplingRate = mOriginalSamplingRate;
			}
		} else if (mOriginalSamplingRate==44100) {
			if (mSamplingChoice==0) {
				mSamplingRate = 22050;
			} else {
				mSamplingRate = 44100;
			}
		} else {
			if (mSamplingChoice==0) {
				mSamplingRate = 22050;
			} else if (mSamplingChoice==1) {
				mSamplingRate = 44100;
			} else {
				mSamplingRate = mOriginalSamplingRate;
			}
		}
	}
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
    ON_BN_CLICKED(IDC_SAVEAS_SAMPLE1, OnClicked)
    ON_BN_CLICKED(IDC_SAVEAS_SAMPLE2, OnClicked)
    ON_BN_CLICKED(IDC_SAVEAS_SAMPLE3, OnClicked)
END_MESSAGE_MAP()

void CDlgSaveAsOptions::OnClicked() {
	// retrieve modified data
    UpdateData();
}
