/////////////////////////////////////////////////////////////////////////////
// sa_cursr.cpp:
// Implementation of the CStartCursorWnd classes.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
// 1.06.1.2
//       SDM Added code to detect CONTROL+SHIFT on cursor move and cause Update Boundaries
//       SDM Restricted select to control key only
//       SDM Added message handlers OnKey... to Start&Stop cursors
// 1.06.6U4
//       SDM Changed CPrivateCursorWnd to use CPrivateCursorWnd::ChangePosition()
// 1.06.6U5
//       SDM Modified Calculate position to adjust for new plot alignment
// 1.5Test8.1
//       SDM Added support for no overlap drag UpdateBoundaries
// 1.5Test10.2
//       SDM Added annotation deselection on MouseUp if cursors do not include selected
// 1.5Test10.4
//       SDM Made cursor windows dependent on CCursorWnd (new class)
//       SDM added bDrawn flag
// 1.5Test11.0
//       SDM replaced GetOffset() + GetDuration() with CSegment::GetStop()
// 1.5Test11.4
//       SDM added support for editing PHONEMIC/TONE/ORTHO to span multiple segments
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_cursr.h"
#include "sa_plot.h"
#include "sa_graph.h"
#include "Process\Process.h"
#include "Segment.h"
#include "Process\sa_p_fra.h"

#include "math.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CCursorWnd::CCursorWnd() {
    bDrawn = FALSE;
}

BOOL CCursorWnd::IsDrawn() {
    return bDrawn;
}

void CCursorWnd::SetDrawn(BOOL bNewDrawn) {
    bDrawn = bNewDrawn;
}
