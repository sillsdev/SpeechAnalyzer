#include "stdafx.h"
#include "AppContext.h"
#include "SA_View.h"

CAppContext::CAppContext(CSaView * aView) {
    pApp = (CSaApp*)AfxGetApp();
    pMainFrame = (CMainFrame*)AfxGetMainWnd();
    pView = aView;
    pModel = pView->GetDocument();
}

CSaApp * CAppContext::getApp() {
    return pApp;
}

CMainFrame * CAppContext::getMainFrame() {
    return pMainFrame;
}

CSaView * CAppContext::getView() {
    return pView;
}

CSaDoc * CAppContext::getDoc() {
    return pModel;
}
