#pragma once
class CSaApp;
class CMainFrame;
class CSaView;
class CSaDoc;

class CAppContext : public App {
private:
	CSaApp* pApp;
	CMainFrame* pMainFrame;
	CSaView* pView;
	CSaDoc* pModel;
public:
	CAppContext(CSaView* pView);
	CSaApp * getApp();
	CMainFrame * getMainFrame();
	CSaView * getView();
	CSaDoc * getDoc();
};

