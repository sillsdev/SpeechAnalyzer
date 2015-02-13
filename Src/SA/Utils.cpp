#include "stdafx.h"

#include "Utils.h"

/**
* function to format GetLastError codes
*/
CString FormatGetLastError( DWORD err) {

    LPTSTR s = NULL;
    if (::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, (LPTSTR)&s, 0, NULL) == 0) { 
		// failure
		CString fmt;
		fmt.LoadString(IDS_ERROR_UNKNOWN);
		CString text;
		text.Format( fmt, err, LOWORD(err));
		return text;
	}
	// success
	LPTSTR p = _tcschr(s, _T('\r'));
	if (p != NULL) {
		*p = _T('\0');
	}
	CString text = s;
	::LocalFree(s);
	return text;
}
