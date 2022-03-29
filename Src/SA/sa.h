/////////////////////////////////////////////////////////////////////////////
// sa.h:
// main header file for the SA application
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//   1.06.8
//       SDM Added ProcessBatch Command
//       SDM Added OnUpdateRecentFileMenu
//       SDM added FileReturn
//   1.5Test8.5
//       SDM added m_nCommand
//       SDM added IsAppRunning, m_szCallingApp
//   1.5Test10.0
//       SDM added support for registering WAV files to database
//   1.5Test10.8
//       SDM Added ErrorMessage(CSaString&)
//   04/13/2000
//       RLJ Added OnHelpAp(), OnHelpMn();
//   05/15/2000
//       RLJ Added OpenBlankView() to open a blank view to do something on.
//   05/31/2000
//       RLJ Added m_OpenMode, GetOpenMode(), SetOpenMode(int OpenMode),
//                 FileOpen(), OnFileOpenPA(), and OnFileOpenMA();
//   06/09/2000
//       RLJ Updated links to help files by deleting OnHelpMn(); and
//           adding OnHelpIndex(); OnHelpSATutor(); OnHelpMiscInfo();
//   06/12/00
//       RLJ Added OnIPAHelp()
//   06/17/2000
//        RLJ Extend FileOpenAs to support not only Phonetic/Music Analysis,
//            but also OpenScreenF, OpenScreenG, OpenScreenI, OpenScreenK,
//            OpenScreenM, etc.
//   06/22/2000
//       RLJ Changed OnHelpIndex to OnHelpContents to allow bypass
//           Windows assuming SA.HLP is in same folder as SA.EXE
//           Added OnHelpOnHelp
//   07/24/2000
//       RLJ Added OnHelpTrouble, OnHelpMusic
/////////////////////////////////////////////////////////////////////////////
#ifndef __SA_H__
#define __SA_H__

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include "SaString.h"
#include "sa_dlwnd.h"

#import "SAUtils.tlb" no_namespace named_guids

class CSaDoc;
class CSaView;
class CObjectIStream;
class CObjectOStream;

using std::wstring;

//###########################################################################
// FindMemoryLeaks class

//class FindMemoryLeaks
//{
//  _CrtMemState m_checkpoint;
//
//public:
//  FindMemoryLeaks()
//  {
//      _CrtMemCheckpoint(&m_checkpoint);
//  };
//
//  ~FindMemoryLeaks()
//  {
//      _CrtMemState checkpoint;
//      _CrtMemCheckpoint(&checkpoint);
//      _CrtMemState diff;
//      _CrtMemDifference(&diff, &m_checkpoint, &checkpoint);
//      _CrtMemDumpStatistics(&diff);
//      _CrtMemDumpAllObjectsSince(&diff);
//  };
//};

class CSaApp : public CWinApp, public App {

public:
    CSaApp();
    ~CSaApp();

    // helper functions
    BOOL WriteSettings();
    BOOL ReadSettings(bool batchMode);
	// create a new file and paste the clipboard into it
    void PasteClipboardToNewFile(HGLOBAL hData);		
    CSaDoc * OpenWavFileAsNew(LPCTSTR szTempPath);
	// open a blank view to do something on.
    CDocument * OpenBlankView(bool bWithGraphs);
    CSaString GetDefaultDir() const;
    CSaString GetSamplesDir() const;

    static BOOL CSaApp::m_bUseUnicodeEncoding;

	// examine the command line
    void ExamineCmdLine(LPCTSTR, WPARAM wParam = 0);    
    void ErrorMessage(UINT nTextID, LPCTSTR pszText1 = NULL, LPCTSTR pszText2 = NULL);
    void ErrorMessage(CSaString & szText);
    void Message(UINT nTextID, LPCTSTR pszText1 = NULL, LPCTSTR pszText2 = NULL);
	// displays a stored error message
    void DisplayMessages();
	// set file changed in batch mode list file
    void SetBatchFileChanged(CSaString, int, CDocument *); 
	// return application mode (batch or not, exit allowed)
    int GetBatchMode();
	// allow SA to exit
    void CancelBatchMode();
    // check is this file already opened
    CSaDoc * IsFileOpened(LPCTSTR pszFileName);
    // check is this file already opened
    bool IsDocumentOpened(const CSaDoc * pModel);
    BOOL CloseWorkbench(CDocument *);                   // close an already opened workbench document
    void WorkbenchClosed();                             // signal, that workbench has been closed
    void SetWorkbenchPath(CSaString * pszPath);         // set the workbenchs document pathname
    CSaString * GetWorkbenchPath();                             // returns a pointer to the workbench pathname
    int SaDoPrintDialog(CPrintDialog * pPD, BOOL landscape);
    BOOL SaGetPrinterDeviceDefaults(PRINTDLG * pPrintDlg, BOOL landscape);
	// return TRUE if file new operation running
    BOOL IsCreatingNewFile();
	// return pointer to workbench document
    CDocument * GetWbDoc();
    void SetWbOpenOnExit(BOOL bOpen);
	// return to calling application
    void FileReturn(BOOL bHide=TRUE);
    CWnd * IsAppRunning();
	// return m_OpenAsID
    UINT GetOpenAsID();
	// set m_OpenAsID
    void SetOpenAsID(UINT OpenAsID);
	bool GetOpenMore();
	void SetOpenMore(bool val);
    void SetLastClipboardPath(LPCTSTR szPath);
    LPCTSTR GetLastClipboardPath();
    void FileOpen( UINT openAsID);
    CString GetMRUFilePath(int i) const;

    // methods for saving the settings and window state.
    CSaView * GetViewActive();
    // Return the active MDI child record window;
    // otherwise NULL, if there are no windows open.
    CSaView * GetViewTop();
    CSaView * GetViewBottom();
    CSaView * GetViewBelow(CSaView * pviewCur);
    CSaView * GetViewAbove(CSaView * pviewCur);
    // Operations on the MDI child view [window] list, ordered by z-order.
    void SetZ();
    // Set the current z-order of all MDI child views.
	// read the open databases and windows
    BOOL ReadProperties(CObjectIStream & obs, bool batchMode);  
	// write the open databases and windows
    void WriteProperties(CObjectOStream & obs); 
    CSaString GetStartupMessage(CSaString szLastVersion);
    void SetupNewUser();

    virtual BOOL InitInstance();
    virtual int ExitInstance();
    virtual CDocument * OpenDocumentFile(LPCTSTR lpszFileName);

    bool IsSAServer() const;

	CString GetVersionString();

    // App implementations
    CVowelFormantSets& GetVowelSets();
    const CVowelFormantSet& GetDefaultVowelSet();
    const CVowelFormantsVector& GetVowelVector(int nGender);

    SResearchSettings GetResearchSettings() { return researchSettings; };
    void SetResearchSettings(SResearchSettings value) { researchSettings = value;};
    // return CPlot3D::GetChartRange(nFormant, nGender);
    SRange Get3DChartRange(int nFormant, int nGender);
    // uses IDS_ERROR_GRAPPLSPACE
    void GrapplErrorMessage(LPCTSTR pszText1 = NULL, LPCTSTR pszText2 = NULL);
    int AfxMessageBox(UINT nIDPrompt, UINT nType = MB_OK, UINT nIDHelp = (UINT)-1);

protected:
    void ShowStartupDialog(BOOL bAppIsStartingUp);
    afx_msg void OnAppAbout();
    afx_msg void OnFileCreate();
    afx_msg void OnUpdateFileCreate(CCmdUI * pCmdUI);
    afx_msg void OnFileRecord();
    afx_msg void OnFileOpen();
    afx_msg void OnFileOpenSpecial();
    afx_msg void OnFileOpenPA();
    afx_msg void OnFileOpenMA();
    afx_msg void OnUpdateFileRecord(CCmdUI * pCmdUI);
    afx_msg void OnUpdateFileOpen(CCmdUI * pCmdUI);
    afx_msg void OnUpdateRecentFileMenu(CCmdUI * pCmdUI);
    afx_msg void OnUpdateAppExit(CCmdUI * pCmdUI);
    afx_msg void OnFileReturn();
    afx_msg void OnUpdateFileReturn(CCmdUI * pCmdUI);
    afx_msg BOOL OnIdle(LONG lCount);
    afx_msg void OnHelp();
    afx_msg void OnHelpContents();
	afx_msg void OnHelpShortCuts();
    afx_msg void OnHelpMiscInfo();
    afx_msg void OnHelpTrouble();
    afx_msg void OnHelpWhatsNew();
    afx_msg void OnHelpSFMMarkers();
    afx_msg void OnHelpGraphs();
    afx_msg void OnHelpMusic();
    afx_msg void OnHelpTrainingStudent();
    afx_msg void OnHelpTrainingExercises();
    afx_msg void OnHelpTrainingInstructor();
    afx_msg void OnAudioCon();
    afx_msg void OnHelpOnHelp();
    afx_msg void OnHelpWorkbench();
    afx_msg void OnWorkbenchOpen();
    afx_msg void OnUpdateWorkbenchOpen(CCmdUI * pCmdUI);
    afx_msg void OnProcessBatchCommands();

    // more methods for saving the settings and window state.
    CSaView * GetViewEnd(UINT uNextOrPrev);
    CSaView * GetViewNeighbor(CSaView * pviewCur, UINT uNextOrPrev);

    // methods for handling single instance
    void InitSingleton();
    void DestroySingleton();
    // Creates the instance handler
    BOOL CreateAsSingleton(LPCTSTR aName);
    // Callback when the instance is woken up by another
    virtual void WakeUp(LPCTSTR aCommandLine);
    // sleeping thread that waits for activation
    static UINT ActivationThread(CSaApp * aObject);

    DECLARE_MESSAGE_MAP()

    // Events to signal new instance, and kill thread
    CEvent * mEvent;
    CEvent * mSignal;
    class CSingleInstanceData * mData;
    SResearchSettings researchSettings;

private:
    int CheckForBatchMode(LPTSTR);      // check if SA runs in batch mode
    void CopyClipboardTranscription(LPCTSTR szTempPath);
    CSaString GetBatchString(LPCTSTR lpSection, LPCTSTR lpKey, LPCTSTR lpDefault = NULL);
    BOOL WriteBatchString(LPCTSTR lpSection, LPCTSTR lpKey, LPCTSTR lpValue);

    HINSTANCE m_hEnglishResources;
    HINSTANCE m_hGermanResources;
    HINSTANCE m_hLocalizedResources;
    CMultiDocTemplate * m_pDocTemplate;		// document template
    CMultiDocTemplate * m_pWbTemplate;		// workbench document template
    CSaString m_szCmdFileName;				// file path and name of CMD-Line file
    int m_nBatchMode;						// 0 = not in batch mode, 1 = in batch mode do not exit, 3 = in batch mode ok to exit
    BOOL m_bModified;						// TRUE = at least one of the batch files has been modified
    BOOL m_bNewDocument;					// TRUE = file new operation
    int m_nEntry;							// number of entries in list file
    int m_nCommand;							// command sequence to execute
    CStringArray m_pszErrors;				// error message string array
    CStringArray m_pszMessages;				// message string array
    CDocument * m_pWbDoc;					// pointer to workbench document
    CSaString m_szWbPath;					// path and filename of workbench file
    BOOL m_bWbOpenOnExit;					// TRUE, if workbench was open on exit
    CSaString m_szCallingApp;				// title bar prefix of calling app
    UINT m_OpenAsID;						// ID (ID_FILE_OPEN, ID_FILE_OPENAS_PHONETICANALYSIS, etc.) selected from File-Open or File-OpenAs menus.
	bool m_bOpenMore;						// true if user selected 'more files' from start menu.  false indicates they selected 'sample files'
    CSaString m_szLastClipboardPath;		// Path to last file sent to the clipboard
    CSaString m_szLastVersion;				// Last version SA was opened in
    BOOL m_bNewUser;						// Last version SA was opened in
};

CSaString GetShellFolderPath(DWORD csidl);

#endif
