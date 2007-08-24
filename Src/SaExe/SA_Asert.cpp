// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1993 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include "sa_asert.h"

#ifdef AFX_DBG1_SEG
#pragma code_seg(AFX_DBG1_SEG)
#endif

// NOTE: in separate module so it can replaced if needed

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
int NEAR saIgnoreAssertCount = 0;               // for testing diagnostics
LONG NEAR saAssertBusy = -1;
#endif

#pragma optimize("qgel", off) // assembler cannot be globally optimized

extern "C"
void AFXAPI SaAssertFailedLine(LPCSTR lpszFileName, int nLine)
{
#ifdef _DEBUG
  if (saIgnoreAssertCount > 0)
  {
    saIgnoreAssertCount--;
    return;
  }

#ifdef _WINDOWS
  TCHAR sz[255];
  static TCHAR szTitle[] = _T("Assertion Failed!");
  static TCHAR szMessage[] = _T("%s: File %s, Line %d");
  static TCHAR szUnknown[] = _T("<unknown application>");

  // In case _AfxGetAppDataFails.
  if (++saAssertBusy > 0)
  {
    // assume the debugger or auxiliary port
    _stprintf(sz, szMessage,szUnknown, lpszFileName, nLine);
#ifdef _AFXCTL
    // Checking afxTraceEnabled may cause infinite loop
    // on faild AfxGetAppData.

    AfxOutputDebugString(sz);
#else
    if (afxTraceEnabled)
      ::OutputDebugString(sz);
#endif
    saAssertBusy--;

    // break into the debugger (or Dr Watson log)
#ifndef _PORTABLE
    _asm
    {
      int 3
    };
#endif
    return;
  }

  // get app name or NULL if unknown (don't call assert)
  const TCHAR* pszAppName = AfxGetAppName();
  _stprintf(sz, szMessage,
    (pszAppName == NULL) ? szUnknown : pszAppName,
    lpszFileName,
    nLine);

  if (afxTraceEnabled)
  {
    // assume the debugger or auxiliary port
    ::OutputDebugString(sz);
    ::OutputDebugString(_T(", "));
    ::OutputDebugString(szTitle);
    ::OutputDebugString(_T("\n\r"));
  }

  int nCode = ::MessageBox(NULL, sz, szTitle,
    MB_SYSTEMMODAL | MB_ICONHAND | MB_ABORTRETRYIGNORE);
  saAssertBusy--;

  // break into the debugger (or Dr Watson log)
#ifndef _PORTABLE
  _asm
  {
    int 3
  };
#endif

  if (nCode == IDIGNORE)
  {
    return;     // ignore
  }
  else if (nCode == IDRETRY)
  {
    return; // ignore and continue in debugger to diagnose problem
  }
  // else fall through and call AfxAbort

#else
  static char szMessage[] = "Assertion Failed: file %Fs, line %d\r\n";
  fprintf(stderr, szMessage, lpszFileName, nLine);
#endif // _WINDOWS

#else
  // parameters not used if non-debug
  (void)lpszFileName;
  (void)nLine;
#endif // _DEBUG

  AfxAbort();
}

#pragma optimize("", on)
