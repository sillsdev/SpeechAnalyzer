/////////////////////////////////////////////////////////////////////////////
// sa_cursr.h:
// Interface of the CCursorWnd classes.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
//
// Revisions
//  1.06.2
//       SDM  Added message handlers OnKey... to Start&Stop cursors
//  1.06.6U4
//       SDM  Added function ChangePosition
//  1.5Test10.4
//       SDM Made cursor windows dependent on CCursorWnd (new class)
//       SDM added bDrawn flag
/////////////////////////////////////////////////////////////////////////////
#ifndef SA_CURSR_DEF
#define SA_CURSR_DEF

class CCursorWnd : public CWnd {
public:
    CCursorWnd();
    BOOL bDrawn;
    BOOL IsDrawn();
    void SetDrawn(BOOL bNewDrawn);
};

#endif

