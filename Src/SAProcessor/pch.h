// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#define VC_EXTRALEAN        // Exclude rarely-used stuff from Windows headers
#define OEMRESOURCE         // necessary to use system bitmaps

#include <afx.h>
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

typedef char* HPSTR;

inline static int round2Int(double value) {
    return (int)floor(value + 0.5);
}

#include <codecvt>
#include <string>

// convert UTF-8 string to wstring
static std::wstring _to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.from_bytes(str);
}

// convert wstring to UTF-8 string
static std::string _to_utf8(const std::wstring& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.to_bytes(str);
}

#define EXPERIMENTAL_ACCESS true

#define PROGRAMMER_ACCESS ((GetKeyState(VK_CONTROL) & GetKeyState(VK_SHIFT)) < 0)

#define UNDEFINED_DATA                  -1          // data not yet defined
#define UNDEFINED_OFFSET                0xFFFFFFFF  // offset not yet defined
#define PRECISION_MULTIPLIER            10          // multiplier to amount precision
#define MAX_CALCULATION_FREQUENCY       500         // permissible parameter range

#define CALCULATION_INTERVAL(samplingRate)      (100*22050/samplingRate)         // number of raw data samples taken to build a processed data result
#define CALCULATION_DATALENGTH(samplingRate)    (300*22050/samplingRate)         // length of data used for calculation

#define ZCROSS_SAMPLEWINDOWWIDTH(samplingRate)  CALCULATION_INTERVAL(samplingRate)  // number of samples in the calculation window to calculate zero crossing

#define MAX_FILTER_NUMBER               3           // number of filters in workbench processes

#define MEL_MULT                        100         // multiplier for melogram data

// CECIL pitch default settings
#define CECIL_PITCH_MAXFREQ             500
#define CECIL_PITCH_MINFREQ             40
#define CECIL_PITCH_VOICING             32
#define CECIL_PITCH_CHANGE              10
#define CECIL_PITCH_GROUP               6
#define CECIL_PITCH_INTERPOL            7

#include "resource.h"

#include "context.h"

#endif
