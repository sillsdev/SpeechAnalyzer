// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#define VC_EXTRALEAN        // Exclude rarely-used stuff from Windows headers
#define OEMRESOURCE         // necessary to use system bitmaps

#include <windows.h>
#include <algorithm>
#include <fstream>
#include <list>
#include <deque>
#include <map>
#include <limits>
#include <complex>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <locale.h>
#include <assert.h>
#include <fstream>
#include <sys/stat.h>

#include <sa_dsp.h>
#include <fileutils.h>
#include <waveutils.h>
#include <zgraph.h>

using std::unique_ptr;
using std::char_traits;
using std::distance;
using std::find;
using std::getline;
using std::ifstream;
using std::ios;
using std::ios_base;
using std::istringstream;
using std::list;
using std::logic_error;
using std::map;
using std::ofstream;
using std::streampos;
using std::string;
using std::stringstream;
using std::vector;
using std::wifstream;
using std::wistringstream;
using std::wofstream;
using std::wstring;
using std::wstringstream;
using std::fstream;
using std::ios_base;

typedef char* BPTR;

#include <codecvt>
#include <string>

extern void trace(LPCSTR,...);

extern int round2Int(double);

#include "sap_defs.h"
#include "resource.h"
#include "context.h"

#endif
