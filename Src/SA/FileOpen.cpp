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

BOOL CDlgFileOpen::OnInitDialog() {
	GetDlgItem(IDC_PLAY)->EnableWindow(TRUE);
	GetDlgItem(IDC_STOP)->EnableWindow(FALSE);
	return TRUE;
}

BEGIN_MESSAGE_MAP(CDlgFileOpen, CFileDialog)
	ON_BN_CLICKED(IDC_PLAY, OnPlay)
	ON_BN_CLICKED(IDC_STOP, OnStop)
	ON_MESSAGE(MM_MCINOTIFY, OnMCINotify)
END_MESSAGE_MAP()

LRESULT CDlgFileOpen::OnMCINotify(WPARAM wParam, LPARAM ) {

	wchar_t response[1024];
	wmemset(response, 0, 1024);
	MCIERROR me = 0;

	switch (wParam) {
	case MCI_NOTIFY_ABORTED:	// The device received a command that prevented the current conditions 
								// for initiating the callback function from being met.
								// If a new command interrupts the current command and it also 
								// requests notification, the device sends this message only and not MCI_NOTIFY_SUPERSEDED
		TRACE("aborted\n");
		break;
	case MCI_NOTIFY_FAILURE: 	//A device error occurred while the device was executing the command.
		TRACE("aborted\n");
		me = mciSendString(L"close audio", response, 1024, NULL);
		if (me != 0) {
			TRACE(L"failed:%s\n", response);
			BOOL bResult = mciGetErrorString(me, response, 1024);
			if (bResult) {
				// failure
				TRACE(L"failed:%s\n", response);
			}
		}
		GetDlgItem(IDC_PLAY)->EnableWindow(TRUE);
		GetDlgItem(IDC_STOP)->EnableWindow(FALSE);
		break;
	case MCI_NOTIFY_SUCCESSFUL:	//The conditions initiating the callback function have been met.
		TRACE("successful\n");
		me = mciSendString(L"close audio", response, 1024, NULL);
		if (me != 0) {
			TRACE(L"failed:%s\n", response);
			BOOL bResult = mciGetErrorString(me, response, 1024);
			if (bResult) {
				// failure
				TRACE(L"failed:%s\n", response);
			}
		}
		GetDlgItem(IDC_PLAY)->EnableWindow(TRUE);
		GetDlgItem(IDC_STOP)->EnableWindow(FALSE);
		break;
	case MCI_NOTIFY_SUPERSEDED:
		TRACE("superseded\n");
		break;
	}

	return 0;
}

void CDlgFileOpen::OnPlay() {

	wchar_t command[1024];
	wchar_t response[1024];
	wmemset(command, 0, 1024);
	wmemset(response, 0, 1024);
	CString path = GetPathName().Trim();
	if (path.IsEmpty()) {
		return;
	}

	wsprintf(command, L"open \"%s\" type mpegvideo alias audio", GetPathName());
	TRACE(L"command: %s\n", command);
	MCIERROR me = mciSendString(command, response, 1024, 0);
	if (me != 0) {
		TRACE(L"failed:%s\n", response);
		BOOL bResult = mciGetErrorString(me, response, 1024);
		if (bResult) {
			// failure
			TRACE(L"reason:%s\n", response);
		}
		return;
	}
	me = mciSendString(L"play audio notify", response, 1024, GetSafeHwnd());
	if (me != 0) {
		TRACE(L"failed:%s\n", response);
		BOOL bResult = mciGetErrorString(me, response, 1024);
		if (bResult) {
			// failure
			TRACE(L"reason:%s\n", response);
		}
		return;
	}
	GetDlgItem(IDC_PLAY)->EnableWindow(FALSE);
	GetDlgItem(IDC_STOP)->EnableWindow(TRUE);
}

void CDlgFileOpen::OnStop() {

	wchar_t response[1024];
	wmemset(response, 0, 1024);
	MCIERROR me = mciSendString(L"close audio", response, 1024, NULL);
	if (me != 0) {
		TRACE(L"failed:%s\n", response);
		BOOL bResult = mciGetErrorString(me, response, 1024);
		if (bResult) {
			TRACE(L"reason:%s\n", response);
		}
		return;
	}
	GetDlgItem(IDC_PLAY)->EnableWindow(TRUE);
	GetDlgItem(IDC_STOP)->EnableWindow(FALSE);
}
