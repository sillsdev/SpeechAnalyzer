#pragma once
class CSaApp;
class CMainFrame;
class CSaView;
class CSaDoc;

class CAppContext {
private:
	CSaApp* pApp;
	CMainFrame* pMainFrame;
	CSaView* pView;
	CSaDoc* pDoc;
public:
	CAppContext(CSaView* pView);
	CSaApp * getApp();
	CMainFrame * getMainFrame();
	CSaView * getView();
	CSaDoc * getDoc();
};

