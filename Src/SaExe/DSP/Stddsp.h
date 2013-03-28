// stddsp.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
#ifndef _STDDSP_H

#include "dspTypes.h"
//#include "dspError.h"

#define _STDDSP_H

#define ASSERT(a)


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

#define UNDEFINED_DATA                  -1          // data not yet defined

inline static int32 round(double value)
{
    return (int32)floor(value + 0.5);
}

#pragma warning(pop)

#ifdef _DEBUG
#define __HIDE_REMINDER "reminder "
// Uncommenting the following #define causes MFC to check the heap for memory overwrites
// on every call to new and delete
// #define DEBUG_MEMORY_OVERWRITE
#pragma warning(disable : 4711) // warning C4244: function cdcdcd() selected for inline expansion
#pragma warning(disable : 4514) // unreferenced inline function has been removed
#else
#define __HIDE_REMINDER ""
#pragma warning(disable : 4711) // warning C4244: function cdcdcd() selected for inline expansion
#pragma warning(disable : 4514) // unreferenced inline function has been removed
#endif


#endif //_STDDSP_H