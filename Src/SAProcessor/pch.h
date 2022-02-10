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

#include "resource.h"

#include "context.h"

#endif
