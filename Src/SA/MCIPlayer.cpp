#include "stdafx.h"
#include "MCIPlayer.h"

void CMCIPlayer::Play( CString path, HWND hWnd) {

	wchar_t command[1024];
	wchar_t response[1024];
	wmemset(command, 0, 1024);
	wmemset(response, 0, 1024);
	if (path.IsEmpty()) {
		return;
	}

	wsprintf(command, L"open \"%s\" type mpegvideo alias audio", path.GetString());
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
	me = mciSendString(L"play audio notify", response, 1024, hWnd);
	if (me != 0) {
		TRACE(L"failed:%s\n", response);
		BOOL bResult = mciGetErrorString(me, response, 1024);
		if (bResult) {
			// failure
			TRACE(L"reason:%s\n", response);
		}
		return;
	}
}

void CMCIPlayer::Stop() {
	// always attempt to close the device, whether we are playing or not.
	wchar_t response[1024];
	wmemset(response, 0, 1024);
	MCIERROR me = mciSendString(L"close audio", response, 1024, NULL);
	if (me != 0) {
		TRACE(L"failed:%s\n", response);
		BOOL bResult = mciGetErrorString(me, response, 1024);
		if (bResult) {
			TRACE(L"reason:%s\n", response);
		}
	}
}

void CMCIPlayer::Close() {
	// close device on cancel or OK.
	mciSendString(L"close audio", NULL, 0, NULL);
}