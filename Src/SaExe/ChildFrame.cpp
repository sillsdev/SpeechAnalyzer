#include "stdafx.h"
#include "ChildFrame.h"
#include "MainFrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
END_MESSAGE_MAP()

CChildFrame::CChildFrame() {
}

CChildFrame::~CChildFrame() {
}

void CChildFrame::ActivateFrame(int nCmdShow) {

    CMainFrame * pFrameWnd = (CMainFrame *) GetMDIFrame();

    if (!pFrameWnd) {
        return;
    }

    if (pFrameWnd->MDIGetActive()) {
        CMDIChildWnd::ActivateFrame(nCmdShow);  // maintain current state
    } else if (pFrameWnd->IsDefaultViewMaximized()) {
        CMDIChildWnd::ActivateFrame(SW_SHOWMAXIMIZED);
    } else {
        CMDIChildWnd::ActivateFrame(nCmdShow);  // default behavior
    }

    // Adjust size of child
    CPoint Size = pFrameWnd->GetDefaultViewSize();
    WINDOWPLACEMENT WP;
    WP.length = sizeof(WINDOWPLACEMENT);
    CRect rParent;

    pFrameWnd->GetClientRect(&rParent);

    GetWindowPlacement(&WP);
    if ((Size.x < rParent.Width())&&(Size.x > (rParent.Width()/10))) {
        WP.rcNormalPosition.right = WP.rcNormalPosition.left+Size.x;
    } else if (Size.x >= rParent.Width()) {
        WP.rcNormalPosition.right = rParent.Width();
    }

    if ((Size.y < rParent.Height())&&(Size.y > (rParent.Height()/10))) {
        WP.rcNormalPosition.bottom = WP.rcNormalPosition.top+Size.y;
    } else if (Size.y >= rParent.Height()) {
        WP.rcNormalPosition.bottom = rParent.Height();
    }

    SetWindowPlacement(&WP);
};

#ifdef _DEBUG
void CChildFrame::AssertValid() const {
    CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext & dc) const {
    CMDIChildWnd::Dump(dc);
}

#endif

