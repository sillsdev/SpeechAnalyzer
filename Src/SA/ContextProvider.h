#pragma once
#ifndef _CONTEXTPROVIDER_H
#define _CONTEXTPROVIDER_H

class CSaApp;
class CMainFrame;
struct Context;

struct ContextProvider {
public:
    Context& GetContext();
    CSaApp& GetApp();
    CMainFrame& GetFrame();
private:
    CSaApp * app;
    CMainFrame * frame;
    unique_ptr<Context> context;
};

#endif
