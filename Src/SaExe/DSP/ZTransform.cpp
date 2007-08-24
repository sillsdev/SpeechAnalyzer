// ZTransform.cpp: implementation of the CZTransformGeneric class.
//
// Author: Steve MacLean
// copyright 2001-2003 JAARS Inc. SIL
//
// This class is designed to implement generic Z-Transforms
// The class currently implements a transform a cascade of low order
// transform stages.  Each stage could actually be of any arbitrary 
// order, but there is a tendency for high order transforms to overflow
//
//
//  The current implementation model each transform stage is:
//
//          a[n]z^-n + a[n - 1]z^-(n -1) + ... + a[0]
//          ---------------------------------------
//          b[n]z^-n + b[n - 1]z^-(n -1) + ... + b[0]
//
// Using the model (b[0] must == 1)
//
//          x[n] ----> + ------> + -----> +.......+ ----  
//                     ^         ^                      |
//                     |         |                      |
//                   -b[n]    -b[n-1]                   |
//                     |         |                      |
//                      -- z^-1 <-- z^-1 <-........ ----|
//                     |         |                      |
//                   a[n]      a[n-1]                 a[0]
//                     |         |                      |
//                     v         v                      v
//                     + ------> + -----> +.......+ --> + ----> y[n]
//
//////////////////////////////////////////////////////////////////////

#include "stddsp.h"
#include "ZTransform.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

