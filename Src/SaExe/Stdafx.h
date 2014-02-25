// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
#ifndef _STDAFX_H
#define _STDAFX_H

#define VC_EXTRALEAN        // Exclude rarely-used stuff from Windows headers

#define OEMRESOURCE         // necessary to use system bitmaps

#include <afxwin.h>         // MFC core and standard components

#include <afxext.h>         // MFC extensions (including VB)
#include <afxtempl.h>

#include <mmsystem.h>       // multimedia extensions

#include <afxdisp.h>        // MFC Automation classes
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>         // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxadv.h>
#include <afxole.h>

#include "htmlhelp.h"               // HTML Help support

#pragma warning(push, 3)  // these files generate a lot of warnings at level 4
#pragma warning(disable : 4786) // identifier was truncated to 255 chars in browser
#pragma warning(disable : 4275) // unmarked exported classes generate warning 4275
#include <algorithm>
#include <fstream>
#include <list>
#include <vector>
#include <deque>
#include <map>
#include <limits>
#include <complex>
#include <afxdlgs.h>
#include <string>
#include <vector>

using std::string;
using std::wstring;

using std::ifstream;
using std::wifstream;
using std::ofstream;
using std::wofstream;

using std::ios;
using std::ios_base;
using std::streampos;

using std::getline;
using std::find;
using std::distance;

using std::vector;

inline static int round(double value) 
{
    return (int)floor(value + 0.5);
}

#pragma warning(pop)

#ifdef _DEBUG
#define new DEBUG_NEW
#define __HIDE_REMINDER "reminder "
// Uncommenting the following #define causes MFC to check the heap for memory overwrites
// on every call to new and delete
// #define DEBUG_MEMORY_OVERWRITE
#else
#define __HIDE_REMINDER ""
#pragma warning(disable : 4711) // warning C4244: function cdcdcd() selected for inline expansion
#endif

// This macro determines access to research features
// (e.g., Synthesis, Research Parameter Tab and the Formant Tracker Parameter Tab)
// TRUE                               Enables access
// FALSE                              Completely disables access
// (GetKeyState(VK_CONTROL) < 0)      Enables access via control key only
//#define EXPERIMENTAL_ACCESS (GetKeyState(VK_CONTROL) < 0)

#define EXPERIMENTAL_ACCESS true

#define PROGRAMMER_ACCESS ((GetKeyState(VK_CONTROL) & GetKeyState(VK_SHIFT)) < 0)

#endif
