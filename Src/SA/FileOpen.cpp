// FileOpen.cpp : implementation file
//

#include "stdafx.h"
#include "sa.h"
#include "FileOpen.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgFileOpen

IMPLEMENT_DYNAMIC(CDlgFileOpen, CFileDialog)

CDlgFileOpen::CDlgFileOpen(LPCTSTR lpszDefExt,
	LPCTSTR lpszFileName,
	DWORD dwFlags,
	LPCTSTR lpszFilter,
	CWnd * pParentWnd) :
	CFileDialog(TRUE, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd, 0, FALSE) {
	SetTemplate(IDD, IDD);
	m_ofn.hInstance = AfxFindResourceHandle(MAKEINTRESOURCE(IDD), RT_DIALOG);
}

CDlgFileOpen::~CDlgFileOpen() {
	player.Close();
}

BOOL CDlgFileOpen::OnInitDialog() {
	GetDlgItem(IDC_PLAY)->EnableWindow(FALSE);
	GetDlgItem(IDC_STOP)->EnableWindow(FALSE);
	return TRUE;
}

BEGIN_MESSAGE_MAP(CDlgFileOpen, CFileDialog)
	ON_BN_CLICKED(IDC_PLAY, OnPlay)
	ON_BN_CLICKED(IDC_STOP, OnStop)
	ON_MESSAGE(MM_MCINOTIFY, OnMCINotify)
END_MESSAGE_MAP()

void CDlgFileOpen::OnFileNameChange() {
	// if they change the filename, we stop.
	StopPlayback();
}

LRESULT CDlgFileOpen::OnMCINotify(WPARAM wParam, LPARAM ) {
	// just kill the device for any reason.
	StopPlayback();
	return 0;
}

void CDlgFileOpen::OnPlay() {
	player.Play(GetPathName().Trim(), GetSafeHwnd());
	GetDlgItem(IDC_PLAY)->EnableWindow(FALSE);
	GetDlgItem(IDC_STOP)->EnableWindow(TRUE);
}

bool CDlgFileOpen::HasUsableFile() {
	CString path = GetPathName();
	path = path.TrimLeft();
	if (path.IsEmpty()) {
		return false;
	}
	path.MakeLower();
	int index = path.Find(L".wav");
	if ((index != -1) && (index == path.GetLength() - 4)) {
		return true;
	}
	index = path.Find(L".mp3");
	if ((index != -1) && (index == path.GetLength() - 4)) {
		return true;
	}
	index = path.Find(L".wma");
	if ((index != -1) && (index == path.GetLength() - 4)) {
		return true;
	}
	return false;
}

void CDlgFileOpen::OnStop() {
	StopPlayback();
}

void CDlgFileOpen::StopPlayback() {
	player.Stop();
	GetDlgItem(IDC_PLAY)->EnableWindow(HasUsableFile() ? TRUE : FALSE);
	GetDlgItem(IDC_STOP)->EnableWindow(FALSE);
}
