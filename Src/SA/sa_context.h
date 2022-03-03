#pragma once
#ifndef _SA_CONTEXT_H
#define _SA_CONTEXT_H

class CSaApp;
class CSaView;
class CSaDoc;
class CMainFrame;

struct SaContext {
    SaContext(CSaApp& app, CSaView& view, CSaDoc& model, CMainFrame& frame) : app(app), view(view), model(model), frame(frame) {}
    CSaApp& app;
    CSaDoc& model;
    CSaView& view;
    CMainFrame& frame;
};

#endif