#include "stdafx.h"
#include "clip.hpp"

char * RTFprolog =
    "{\\rtf1\\ansi\\ftnbj{\\fonttbl{\\f0 \\fnil Musique;}}"
    "{\\colortbl ;\\red255\\green255\\blue255 ;\\red0\\green0\\blue128 ;\\red0\\green0\\blue0 ;}"
    "{\\stylesheet{\\cs2\\f0\\fs50\\cf0\\cb1 Musique;}}\\cs2\\f0\\fs50 ";
char * RTFepilog = "\\par}";


CClipboard::CClipboard(HWND hwnd) {
    hWindow = hwnd;
    OpenClipboard(hwnd);
    clipboard_open = TRUE;
    string = 0;
    RTFFormat = RegisterClipboardFormat(_T("Rich Text Format"));
}

CClipboard::~CClipboard() {
    close();
}

CClipboard & CClipboard::operator<<(LPCSTR val) {
    // clear out old contents of clipboard
    EmptyClipboard();

    // allocate memory for the copy operation
    HANDLE hMemory = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, strlen(val) + 1);

    if (!hMemory) {
        reportError();
        return *this;
    }

    // copy the data into the clipboard
    LPSTR lpMemory = (LPSTR) GlobalLock(hMemory);
    while (*val) {
        *lpMemory++ = *val++;
    }
    *lpMemory = '\0';
    GlobalUnlock(hMemory);

    SetClipboardData(CF_OEMTEXT, hMemory);
    return *this;
}

CClipboard & CClipboard::operator>>(char *& string) {
    HANDLE hClipboard = GetClipboardData(CF_OEMTEXT);
    if (!hClipboard) {
        // no data available in the requested format
        string = NULL;
        return *this;
    }

    // allocate memory for the copy operation
    DWORD size = GlobalSize(hClipboard) + 1;
    string = new char [size];
    if (!string) {
        reportError();
        return *this;
    }

    // get a pointer to the clipboard data
    LPSTR lpClipboard = (LPSTR) GlobalLock(hClipboard);

    // copy the data from the clipboard
    char * cp = string;
    while (*lpClipboard) {
        *cp++ = *lpClipboard++;
    }
    *cp = '\0';

    // unlock the clipboard memory
    GlobalUnlock(hClipboard);
    return *this;
}

void CClipboard::SetTextRTF(LPCSTR RTFstring, LPCSTR TEXTstring) {
    // clear out old contents of clipboard
    EmptyClipboard();

    // allocate memory for the copy operation
    HANDLE hRTFMemory =
        GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                    strlen(RTFprolog) + strlen(RTFstring) + strlen(RTFepilog)+ 1);
    if (!hRTFMemory) {
        reportError();
        return;
    }

    HANDLE hTEXTMemory =
        GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                    strlen(TEXTstring) + 1);

    if (!hTEXTMemory) {
        GlobalFree(hRTFMemory);
        reportError();
        return;
    }

    register int i=0;

    // copy the data into the clipboard
    LPSTR lpMemory = (LPSTR) GlobalLock(hRTFMemory);
    for (i=0; RTFprolog[i]; ++i) {
        *lpMemory++ = RTFprolog[i];
    }
    while (*RTFstring) {
        *lpMemory++ = *RTFstring++;
    }
    for (i=0; RTFepilog[i]; ++i) {
        *lpMemory++ = RTFepilog[i];
    }
    *lpMemory = '\0';
    GlobalUnlock(hRTFMemory);
    SetClipboardData(RTFFormat, hRTFMemory);

    lpMemory = (LPSTR) GlobalLock(hTEXTMemory);
    while (*TEXTstring) {
        *lpMemory++ = *TEXTstring++;
    }
    *lpMemory = '\0';
    GlobalUnlock(hTEXTMemory);
    SetClipboardData(CF_OEMTEXT, hTEXTMemory);
}

void CClipboard::reportError() {
    // Close clipboard and report the error
    close();
    AfxMessageBox(IDS_MEMORY_FAILURE, MB_OK | MB_ICONEXCLAMATION);
    return;
}

int CClipboard::hasText() {
    // see if the clipboard contains text
    return IsClipboardFormatAvailable(CF_TEXT);
}

void CClipboard::close() {
    if (!clipboard_open)
        // clipboard has already been closed
    {
        return;
    }

    clipboard_open = FALSE;
    CloseClipboard();

    if (string)
        // delete any dynamic memory used in text pasting
    {
        delete string;
    }
}
