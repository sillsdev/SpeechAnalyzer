#include "stdafx.h"
#include "ContextProvider.h"
#include "sa.h"
#include "MainFrm.h"
#include "Context.h"

Context & ContextProvider::GetContext() {
    if (context == nullptr) {
        GetApp();
        GetFrame();
        CSaDoc * doc = frame->GetCurrDoc();
        context = std::make_unique<Context>((App&)*app,(Model&)*doc,(MainFrame&)*frame,(CmdTarget&)*doc);
    }
    return *context;
}

CSaApp& ContextProvider::GetApp() {
    if (!app) {
        app = static_cast<CSaApp*>(AfxGetApp());
    }
    return *app;
}

CMainFrame& ContextProvider::GetFrame() {
    if (!frame) {
        frame = static_cast<CMainFrame*>(AfxGetMainWnd());
    }
    return *frame;
}
