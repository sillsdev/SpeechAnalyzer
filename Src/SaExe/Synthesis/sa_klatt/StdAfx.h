// Author steve MacLean
// Copyright JAARS 2001
// 
// Modification History
//   2/14/2001 SDM Created
//                 Copied some type definitions from Sensimetrics synth.h
//                 Changed definition of PI/ TWO_PI to be more precise
//                 Changed type Float to be double (was float)
//                 Changed definitionof round to handle negative properly
//


// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__F4A12CE3_0107_11D5_9FE4_B7BCE0035028__INCLUDED_)
#define AFX_STDAFX_H__F4A12CE3_0107_11D5_9FE4_B7BCE0035028__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers


#pragma warning(disable : 4514) // unreferenced inline function

#define PI (3.14159265358979323846264338327950288419716939937511)
#define TWO_PI (2.0 * PI)
#define dB2amp(x) (Float) pow(10.0, (double) (x) / 20.0)

typedef double Float;
typedef short INT16;
enum BOOL { FALSE = 0, TRUE = 1};

inline int round(Float f) { return (int) ((f >= 0) ? f + 0.5 : f - 0.5);}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__F4A12CE3_0107_11D5_9FE4_B7BCE0035028__INCLUDED_)
