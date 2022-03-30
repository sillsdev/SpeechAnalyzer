/////////////////////////////////////////////////////////////////////////////
// sa.cpp:
// Defines the class behaviors for the application.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//   1.06.5
//       SDM modified OnIdle() to send message to mainframe
//   1.06.6U2
//       SDM changed copy/paste waveform to support annotations
//   1.06.6U5
//       SDM Added code to handle invalid m_pszHelpFilePath
//       SDM Changed CSaDoc Template to use CSaMDIChildWnd
//   1.06.8
//       SDM Changed InitInstance to clean-up on failure
//       SDM Changed InitInstance to execute ExamineCmdLine after ReadSettings
//       SDM Changed ExamineCmdLine
//       SDM Added ProcessBatch Command
//       SDM Added OnUpdateRecentFileMenu
//       SDM Split OnFileReturn (added FileReturn)
//       SDM modified FileReturn to make Hiding Optional
//   1.5Test8.3
//       SDM modified ProcessBatch to use function notation for parameters
//       SDM modified examineCmdLine to make the position specifier optional
//   1.5Test8.5
//       SDM disabled context sensitive help
//       SDM added support for dynamically changing menu, popups, and accelerators
//       SDM added support for display plot
//       SDM added IsAppRunning
//   1.5Test10.0
//       SDM added support for registering WAV files to database
//       SDM fixed bug in batch mode ShowWindow=Size()
//       SDM changed batch Command=Import(file) to remove file
//   1.5Test10.4
//       SDM Added ability to accept batch cmd messages w/o option prefix
//   1.5Test10.8
//       SDM Added ErrorMessage(CSaString&)
//   05/12/2000
//       RLJ Added in fix from 1.5Test11.5 which fixed the bug which allowed
//           batch commands to change cursor position in SA/SM interface.
//   05/30/2000
//       AE  Disabled TRACE since it sometimes causes a GPF
//   05/31/2000
//       RLJ Added OpenBlankView() to open a blank view to do something on.
//       RLJ Added FileOpen(), OnFileOpenPA(), and OnFileOpenMA();
//   06/06/2000
//       RLJ Added OnHelpIndex(); OnHelpSATutor(); OnHelpMiscInfo();
//       RLJ Added ON_COMMAND(ID_HELP_USING, OnHelpUsing) to link to Windows
//             default Help file [Using Help]
//   06/12/00
//       RLJ Added OnIPAHelp()
//   06/17/2000
//       RLJ Extend FileOpenAs to support not only Phonetic/Music Analysis,
//             but also OpenScreenF, OpenScreenG, OpenScreenI, OpenScreenK,
//             OpenScreenM, etc.
//   06/22/2000
//       RLJ Changed ID_HELP_INDEX    and OnHelpIndex to
//                   ID_HELP_CONTENTS and OnHelpContents to prevent
//                   Windows from assuming SA.HLP is in same folder as SA.EXE
//           Added OnHelpOnHelp
//   07/24/2000
//       RLJ Added OnHelpTrouble, OnHelpMusic
//       RLJ Added Mainframe window handle to command line of IPA_Help.exe call
//
//   07/26/2000
//       DDO - Commented out the message map entries (and their respective
//             functions) for the various layout with type menu options.
//             those menu options were moved to the Graphs/Layout with Types
//             menu.
//   08/01/00
//       DDO - Moved the display of the start mode dialog box from the
//             splash screen class to here. Also added a check to see if
//             there's already a current view built (i.e. a wave file open)
//             before displaying the startmode dialog. If a view is built
//             then the start mode dialog isn't shown.
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "sa.h"
#include "Segment.h"
#include "sa_doc.h"
#include "sa_w_doc.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "doclist.h"
#include "DlgExportFW.h"
#include "sa_dplot.h"
#include "sa_start.h"
#include <shellapi.h>
#include "fileOpen.h"
#include <windows.h>
#include "ImportSFM.h"
#include "ChildFrame.h"
#include "ClipboardHelper.h"
#include "FileUtils.h"
#include "AutoSave.h"
#include "DlgRecorder.h"
#include "objectostream.h"
#include "objectistream.h"
#include "DlgPlayer.h"
#include <wchar.h>
#include "array_ptr.h"
#include <direct.h>
#include "DlgHelpSFMMarkers.h"
#include "Utils.h"
#include "ScopedCursor.h"
#include "Sa_gz3d.h"

#pragma comment(linker, "/SECTION:.shr,RWS")
#pragma data_seg(".shr")
HWND hGlobal = NULL;
#pragma data_seg()

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Name:        CSingleInstanceData
// Parent:       N/A
// Description: Manages shared data between applications, could be anything,
//              but for this case it is the "command line arguments"
class CSingleInstanceData {
public:
	// Constructor/Destructor
	CSingleInstanceData(LPCTSTR aName);
	virtual ~CSingleInstanceData();

	// Getter/Setter
	void SetValue(LPCTSTR aData);
	CString GetValue() const;

private:
	enum { MAX_DATA = 512 };

	// Data pointer
	LPTSTR  mData;
	// File handle
	HANDLE  mMap;
	// Acces mutex
	CMutex * mMutex;
};


typedef HMODULE(__stdcall * SHGETFOLDERPATH)(HWND, int, HANDLE, DWORD, LPTSTR);
#define CSIDL_PERSONAL                  0x0005        // My Documents
#define CSIDL_FLAG_CREATE               0x8000        // combine with CSIDL_ value to force folder creation in SHGetFolderPath()

BEGIN_MESSAGE_MAP(CSaApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_NEW, OnFileCreate)
	ON_UPDATE_COMMAND_UI(ID_FILE_NEW, OnUpdateFileCreate)
	ON_COMMAND(ID_FILE_RECORD, OnFileRecord)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_OPEN_SPECIAL, OnFileOpenSpecial)
	ON_COMMAND(ID_FILE_OPENAS_PHONETICANALYSIS, OnFileOpenPA)
	ON_COMMAND(ID_FILE_OPENAS_MUSICANALYSIS, OnFileOpenMA)
	ON_UPDATE_COMMAND_UI(ID_FILE_RECORD, OnUpdateFileRecord)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, OnUpdateFileOpen)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN_SPECIAL, OnUpdateFileOpen)
	ON_UPDATE_COMMAND_UI(ID_APP_EXIT, OnUpdateAppExit)
	// TODO: remove OnFileReturn() (no longer used by PA?) 8/4/06
	ON_COMMAND(ID_FILE_RETURN, OnFileReturn)
	ON_UPDATE_COMMAND_UI(ID_FILE_RETURN, OnUpdateFileReturn)
	ON_COMMAND(ID_HELP_CONTENTS, OnHelpContents)
	ON_COMMAND(ID_HELP_KEYBOARD_SHORTCUTS, OnHelpShortCuts)
	ON_COMMAND(ID_HELP_WHATS_NEW, OnHelpWhatsNew)
	ON_COMMAND(ID_HELP_SFMMARKERS, OnHelpSFMMarkers)
	ON_COMMAND(ID_HELP_TROUBLE, OnHelpTrouble)
	ON_COMMAND(ID_HELP_GRAPHS, OnHelpGraphs)
	ON_COMMAND(ID_HELP_MUSIC, OnHelpMusic)
	ON_COMMAND(ID_HELP_TRAINING_STUDENT, OnHelpTrainingStudent)
	ON_COMMAND(ID_HELP_TRAINING_INSTRUCTOR, OnHelpTrainingInstructor)
	ON_COMMAND(ID_AUDIO_CON, OnAudioCon)
	ON_COMMAND(ID_HELP_USINGHELP, OnHelpOnHelp)
	ON_COMMAND(ID_HELP_WORKBENCH, OnHelpWorkbench)
	ON_COMMAND(ID_WORKBENCH, OnWorkbenchOpen)
	ON_UPDATE_COMMAND_UI(ID_WORKBENCH, OnUpdateWorkbenchOpen)
	ON_COMMAND(ID_PROCESS_BATCH_COMMANDS, OnProcessBatchCommands)
	// Standard file based document commands
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	ON_UPDATE_COMMAND_UI_RANGE(ID_FILE_MRU_FIRST, ID_FILE_MRU_LAST, OnUpdateRecentFileMenu)
END_MESSAGE_MAP()

CSaApp::CSaApp() :
	m_hEnglishResources(NULL),
	m_hGermanResources(NULL),
	m_hLocalizedResources(NULL) {

	researchSettings.init();
	// no batch mode
	m_nBatchMode = 0;
	m_bModified = FALSE;
	// reset number of entries in batch list file
	m_nEntry = 0;
	m_nCommand = -1;         // SDM 1.5Test8.5 (not processing batch commands)
	m_bNewDocument = FALSE;
	m_pWbDoc = NULL;
	m_szWbPath.Empty();
	m_bWbOpenOnExit = FALSE;
	m_bNewUser = FALSE;
	m_OpenAsID = ID_FILE_OPEN;
	m_bOpenMore = true;

	InitSingleton();
}

CSaApp::~CSaApp() {
	TRACE("destroy CSaApp\n");
	DestroySingleton();
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSaApp object
CSaApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CSaApp helper functions

HMODULE LoadCompatibleLibrary(LPCTSTR szTarget) {

	// Is Library Compatible
	wchar_t szApp[MAX_PATH * 4];
	wmemset(szApp, 0, _countof(szApp));
	GetModuleFileName(AfxGetInstanceHandle(), szApp, _countof(szApp));

	wchar_t szName[MAX_PATH * 4];
	wcscpy_s(szName, _countof(szName), szTarget);

	wchar_t * loc = wcsrchr(szApp, L'\\');
	if (loc != NULL) {
		wmemset(szName, 0, _countof(szName));
		wcsncat_s(szName, _countof(szName), szApp, (loc - szApp) + 1);
		wcscat_s(szName, _countof(szName), szTarget);
	}

	DWORD dwHandle = 0;
	DWORD dwSize = GetFileVersionInfoSize(szName, &dwHandle);
	if (!dwSize) {
		return 0;
	}

	array_ptr<BYTE> pLibVersion(dwSize);
	if (!GetFileVersionInfo(szName, dwHandle, dwSize, pLibVersion.get())) {
		return 0;
	}

	unsigned int nLen = 0;
	VS_FIXEDFILEINFO * pLibVS = NULL;
	if (!VerQueryValue(pLibVersion.get(), _T("\\"), (LPVOID *)&pLibVS, &nLen)) {
		return 0;
	}

	// now read the local application version.
	dwSize = GetFileVersionInfoSize(szApp, &dwHandle);
	if (!dwSize) {
		return 0;
	}

	array_ptr<BYTE> pAppVersion(dwSize);
	if (!GetFileVersionInfo(szApp, dwHandle, dwSize, pAppVersion.get())) {
		return 0;
	}

	VS_FIXEDFILEINFO * pAppVS = NULL;
	if (!VerQueryValue(pAppVersion.get(), _T("\\"), (void **)&pAppVS, &nLen)) {
		return 0;
	}

	// we will only use the product version to track the relationship between
	// SA, AS and SA_ENU
	if ((pAppVS->dwProductVersionLS != pLibVS->dwProductVersionLS) ||
		(pAppVS->dwProductVersionMS != pLibVS->dwProductVersionMS)) {
		CSaString szMessage;
		szMessage.FormatMessage(_T("%1 was found, but contains resources from incompatible version.\nIt will not be used."), szTarget);
		AfxMessageBox(szMessage);
		return 0;
	}

	return LoadLibrary(szName);
}

/***************************************************************************/
// CSaApp::InitInstance Initialisation of the application
// Called by framework to initially create the application.
/***************************************************************************/
BOOL CSaApp::InitInstance() {

	// handle single instance
	if (CreateAsSingleton(_T("418486C0-7EEE-448d-AD39-2522F5D553A7")) == FALSE) {
		return FALSE;
	}

#ifdef DEBUG_MEMORY_OVERWRITE
	// check for memory overwrites in debug version (see MSDN)
	afxMemDF |= checkAlwaysMemDF;
#endif

	LANGID langid = GetUserDefaultLangID();

	m_hLocalizedResources = LoadCompatibleLibrary(_T("SA_LOC.DLL"));

	switch (langid) {
	case 0x0407:
#ifdef _DEBUG
		// this allows us to use wchar for trace statements
		// you should set the country code of yours
		_tsetlocale(LC_ALL, _T("German"));
#endif
		m_hGermanResources = LoadCompatibleLibrary(_T("SA_DEU.DLL"));
		break;
	case 0x0409:
	default:
#ifdef _DEBUG
		// this allows us to use wchar for trace statements
		// you should set the country code of yours
		_tsetlocale(LC_ALL, _T("English"));
#endif
		m_hEnglishResources = LoadCompatibleLibrary(_T("SA_ENU.DLL"));
		break;
	}

	if ((m_hEnglishResources == NULL) &&
		(m_hGermanResources == NULL) &&
		(m_hLocalizedResources == NULL)) {
		::AfxMessageBox(L"Speech Analyzer was unable to locate or use SA_ENU.DLL, SA_DEU.DLL or SA_LOC.DLL.\nIt will now exit.");
		return FALSE;
	}

	switch (langid) {
	case 0x0407:
		AfxSetResourceHandle(m_hGermanResources);
		break;
	case 0x0409:
	default:
		AfxSetResourceHandle(m_hEnglishResources);
		break;
	}


	if (!CWinApp::InitInstance()) {
		return FALSE;
	}

	AfxEnableControlContainer();

	SetRegistryKey(_T("SIL"));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need.

#ifdef _DEBUG
	//    afxTraceEnabled = 1;
	//    afxTraceFlags = 1;
#endif

	// Load standard INI file options (including MRU)
	LoadStdProfileSettings(8);

	AfxOleInit();

	// Register the application's document templates.
	// Document templates serve as the connection between documents, frame windows and views.
	m_pDocTemplate = new CMultiDocTemplate(IDR_SPEECHANALYZER,
		RUNTIME_CLASS(CSaDoc),
		RUNTIME_CLASS(CChildFrame),
		RUNTIME_CLASS(CSaView));
	AddDocTemplate(m_pDocTemplate);

	// add workbench template
	AddDocTemplate(new CMultiDocTemplate(IDR_WORKBENCH,
		RUNTIME_CLASS(CWorkbenchDoc),
		RUNTIME_CLASS(CMDIChildWnd),
		RUNTIME_CLASS(CSaWorkbenchView)));

	// check if SA runs in batchmode
	m_nBatchMode = CheckForBatchMode(m_lpCmdLine);
	if (GetBatchMode() == 0) {

		ISplashScreenPtr splash(NULL);
		CSaString szSplashText;

		// display splash screen
		//CoInitialize(NULL);
		HRESULT createResult = splash.CreateInstance(__uuidof(SplashScreen));
		if (createResult) {
			CSaString szCreateResult;
			szCreateResult.Format(_T("%x"), createResult);
			CSaString szText;
			AfxFormatString2(szText, IDS_ERROR_CREATE_INSTANCE, _T("SplashScreen.CreateInstance()"), szCreateResult);
			::AfxMessageBox(szText, MB_OK | MB_ICONEXCLAMATION, 0);
		}
		else {
			splash->ShowWithoutFade();
			//splash->Show();
			szSplashText.LoadString(IDS_SPLASH_LOADING);
			splash->Message = (_bstr_t)szSplashText;
			szSplashText.LoadString(IDR_MAINFRAME_SA);
			splash->ProdName = (_bstr_t)szSplashText;
			// load version info
			CSaApp * pApp = (CSaApp*)AfxGetApp();
			CSaString szVersion = pApp->GetVersionString();
			szVersion = szVersion.Right(szVersion.GetLength() - szVersion.Find(' ') - 1);
			// Beta version display
			int nBuildIndex = szVersion.Find(_T("Build"));
			if (nBuildIndex > 0) {
				szVersion = szVersion.Left(nBuildIndex - 2);
			}
			// RC version display
			int nRCIndex = szVersion.Find(_T("RC"));
			if (nRCIndex > 0) {
				szVersion = szVersion.Left(nRCIndex - 1);
			}
			splash->ProdVersion = (_bstr_t)szVersion;
			// load version info
			CSaString szCopyright((LPCTSTR)VS_COPYRIGHT);
			splash->Copyright = (_bstr_t)szCopyright;

			// Perform setup new user, if needed
			if (m_bNewUser) {
				szSplashText.LoadString(IDS_SPLASH_NEW_USER_SETUP);
				splash->Message = (_bstr_t)szSplashText;
			}

			splash->Activate();

			::Sleep(1000);

			splash->Close();
			splash->Release();
			splash = NULL;
		}
	}

	// create main MDI Frame window
	CMainFrame * pMainFrame = new CMainFrame();
	if (!IsSAServer()) {
		if (!pMainFrame->LoadFrame(IDR_MAINFRAME_SA)) {
			return FALSE;
		}
	}
	else if (!pMainFrame->LoadFrame(IDR_MAINFRAME_SAS)) {
		return FALSE;
	}

	m_pMainWnd = pMainFrame;
	if (!GetBatchMode()) {
		// defer display until we process the command line
		m_pMainWnd->ShowWindow(SW_SHOW);
	}

	// update help file path
	//      m_pszHelpFilePath;
	char buffer[MAX_PATH];
	memset(buffer, 0, _countof(buffer));
	_getcwd(buffer, _countof(buffer));
	CSaString szNewPath = buffer;
	szNewPath = szNewPath + "\\";
	szNewPath = szNewPath + "Speech_Analyzer_Help.chm";
	free((void *)m_pszHelpFilePath);
	m_pszHelpFilePath = _tcsdup(szNewPath);

	if (IsSAServer()) {
		if (GetBatchMode() == 0) {
			AfxMessageBox(IDS_ERROR_SAS, MB_OK, 0);
			delete m_pMainWnd;
			m_pMainWnd = 0;
			return FALSE;
		}
	}

	// enable file manager drag/drop
	m_pMainWnd->DragAcceptFiles();

	m_szLastVersion = _T("2.7");
	if (GetBatchMode() == 0) {

		// RegisterShellFileTypes(FALSE);
		EnableShellOpen();
		// Parse command line for standard shell commands, DDE, file open
		CCommandLineInfo cmdInfo;
		ParseCommandLine(cmdInfo);

		BOOL bSettingSuccess = ReadSettings(false);

		// Dispatch commands specified on the command line
		if ((cmdInfo.m_nShellCommand != CCommandLineInfo::FileNew) &&
			(!ProcessShellCommand(cmdInfo))) {
			return FALSE;
		}

		if (!bSettingSuccess) {
			pMainFrame->ShowWindow(m_nCmdShow);
		}

		pMainFrame->UpdateWindow();

		// Perform setup new user, if needed
		if (m_bNewUser) {
			SetupNewUser();
		}

		CSaString msg = GetStartupMessage(m_szLastVersion);
		if (msg.GetLength()) {
			::AfxMessageBox(msg);
		}

		CAutoSave::Check(this);

		pMainFrame->InitializeAutoSave();

		// Show startup dialog
		CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
		if ((pMainWnd->GetShowStartupDlg()) && (!pMainWnd->GetCurrSaView())) {
			ShowStartupDialog(TRUE);
		}
		return TRUE;
	}

	// we are in batch mode
	// in batch mode, messages will be dislayed, but we don't want
	// to show the window yet.
	ReadSettings(true);

	// SDM 1.06.8 window size in this function needs to take precedence over save settings
	// examine command line
	ExamineCmdLine(m_lpCmdLine);
	pMainFrame->UpdateWindow();
	return TRUE;
}

/***************************************************************************/
// CSaApp::ExitInstance Application exit
// Called by framework to exit the application.
/***************************************************************************/
int CSaApp::ExitInstance() {
	// delete the temp transcription DB
	CoInitialize(NULL);
	ISaAudioDocumentWriterPtr saAudioDocWriter;
	saAudioDocWriter.CreateInstance(__uuidof(SaAudioDocumentWriter));
	if (saAudioDocWriter != NULL) {
		saAudioDocWriter->DeleteTempDB();
		saAudioDocWriter->Close();
		saAudioDocWriter->Release();
		saAudioDocWriter = NULL;
	}
	CoUninitialize();

	if (m_hEnglishResources) {
		FreeLibrary(m_hEnglishResources);
	}
	if (m_hGermanResources) {
		FreeLibrary(m_hGermanResources);
	}
	if (m_hLocalizedResources) {
		FreeLibrary(m_hLocalizedResources);
	}

	BOOL bOK = FALSE;
	try {
		bOK = CWinApp::ExitInstance();
		// standard implementation: save initialisation
		SaveStdProfileSettings();
	}
	catch (...) {
	}

	CAutoSave::CleanAll();

	// standard implementation: save initialisation
	if (bOK) {
		return AfxGetCurrentMessage()->wParam;    // Returns the value from \QuitMessage
	}
	else {
		return bOK;
	}
}

/***************************************************************************/
// CSaApp::CheckForBatchMode Check if SA runs in batch mode
// The function returns TRUE, if SA should run in batch mode.
/***************************************************************************/
int CSaApp::CheckForBatchMode(LPTSTR pCmdLine) {
	if ((pCmdLine[0] == '-') &&
		((pCmdLine[1] == 'L') ||
			(pCmdLine[1] == 'l') ||
			(pCmdLine[1] == 'R') ||
			(pCmdLine[1] == 'r'))) {
		return 1;
	}
	return 0;
}

CSaString CSaApp::GetBatchString(LPCTSTR lpSection, LPCTSTR lpKey, LPCTSTR lpDefault) {
	TCHAR lpBuffer[_MAX_PATH] = {};
	lpBuffer[0] = 0;
	GetPrivateProfileString(lpSection, lpKey, lpDefault, lpBuffer, _MAX_PATH, m_szCmdFileName);
	return CSaString(lpBuffer);
}

BOOL CSaApp::WriteBatchString(LPCTSTR lpSection, LPCTSTR lpKey, LPCTSTR lpValue) {
	BOOL bResult = WritePrivateProfileString(lpSection, lpKey, lpValue, m_szCmdFileName);
	WritePrivateProfileString(NULL, NULL, NULL, m_szCmdFileName); // Flush cache to file
	return bResult;
}

/***************************************************************************/
// CSaApp::ExamineCmdLine Examine a command line
// This function examines a command line, appropriate wether SA is in batch
// mode or not. In batch mode it reads the list file (delivered from the
// caller) and sets up the views appropriate to its contents. The second
// parameter contains the wParam of the calling message (or 0 if called from
// the command line).
/***************************************************************************/
void CSaApp::ExamineCmdLine(LPCTSTR pCmdLine, WPARAM wParam) {

	if (pCmdLine[0] == '\0') {
		// no command line, go home
		return;
	}

	CSaString szCmdLine = pCmdLine; // copy string into CSaString object
	szCmdLine.MakeUpper(); // convert the whole string to upper case letters
	DWORD dwStart = 0;
	DWORD dwStop = 0;
	if (GetBatchMode() == 0) {
		// not in batch mode - we are done.
		return;
	}
	// cut out the intro
	if (szCmdLine.Left(2) == "-L") {
		wParam = SPEECH_WPARAM_SHOWSA;
		szCmdLine = szCmdLine.Right(szCmdLine.GetLength() - 2); // SDM 1.5Test8.3
	}
	else if (szCmdLine.Left(2) == "-R") { // SDM 1.5Test10.4
		wParam = SPEECH_WPARAM_SHOWSAREC;
		szCmdLine = szCmdLine.Right(szCmdLine.GetLength() - 2); // SDM 1.5Test8.3
	}
	else { // SDM 1.5Test10.4
		szCmdLine = " " + szCmdLine; // prepend space to allow swscanf_s to suceed
	}

	// sa has to read list file and open documents // SDM 1.5Test8.3
	TCHAR in[512];
	wmemset(in, 0, 512);
	wcscpy_s(in, 512, szCmdLine);

	TCHAR buffer[512];
	wmemset(buffer, 0, 512);
	swscanf_s(in, _T("%*[ 0123456789]%[^\n]"), buffer, 512);
	m_szCmdFileName = buffer;

	CFileStatus status;
	if (m_szCmdFileName.GetLength() == 0) {
		// The file does not exist. use original profile
		// i think only SPEECH_WPARAM_SHOWSAREC does not use a command line file
		ASSERT(wParam == SPEECH_WPARAM_SHOWSAREC);
	}
	else if (!CFile::GetStatus(m_szCmdFileName, status)) {
		// if IPA Help is used to launch SA, it must be run using compatibility mode for Windows XP SP3.
		CString msg;
		msg.FormatMessage(L"The file:\n'%1'\nwas specified on the command line, but it does not exist.\nIf you are using IPA Help on Windows 7 or later, please use the compatibility mode for Windows XP.\nSpeech Analyzer will now exit.", (LPCTSTR)m_szCmdFileName);
		::AfxMessageBox(msg, MB_ICONEXCLAMATION);
		ExitProcess(0);
		return;
	}

	m_szCallingApp = GetBatchString(_T("Settings"), _T("CallingApp"), SM_CAPTION); // get the entry // SDM 1.5Test8.5

	// SDM 1.5Test8.3
	CSaString szString = GetBatchString(_T("Settings"), _T("ShowWindow"), _T("")); // get the entry // SDM 1.5Test8.5

	wchar_t param1[512];
	wchar_t param2[512];
	wmemset(param1, 0, _countof(param1));
	wmemset(param2, 0, _countof(param2));
	int iresult = swscanf_s(szString, _T("%16[^(](%20[^)]"), param1, _countof(param1), param2, _countof(param2));
	TRACE(L"scanned %d parameters", iresult);

	CString szReturn = (iresult == 0) ? szString : param1;
	CString szParam = param2;

	szReturn.MakeUpper(); // convert the whole string to upper case letters
	szReturn = szReturn.Mid(szReturn.SpanIncluding(_T(" ")).GetLength());  // Remove leading spaces
	szReturn += "        ";  // Pad with spaces
	szReturn = szReturn.Left(8);  //Take exactly 8 characters

	TRACE(_T("Settings:ShowWindow=%s\n"), szReturn.GetString());
	if (szReturn == "HIDE    ") {
		m_pMainWnd->ShowWindow(SW_HIDE);
		m_nCmdShow = SW_HIDE; // to prevent MFC to restore on startup
	}
	else if (szReturn == "SHOW    ") {
		m_pMainWnd->ShowWindow(SW_SHOW);
	}
	else if (szReturn == "RESTORE ") {
		m_pMainWnd->ShowWindow(SW_RESTORE);
	}
	else if (szReturn == "MAXIMIZE") {
		m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED); // maximize the mainframe
		m_nCmdShow = SW_HIDE; // to prevent MFC to restore on startup
	}
	else if (szReturn == "MINIMIZE") {
		m_pMainWnd->ShowWindow(SW_MINIMIZE);
		m_nCmdShow = SW_MINIMIZE; // to prevent MFC to restore on startup
	}
	else if (szReturn == "NONE    ") {
		; // do nothing
	}
	else if (szReturn == "SIZE    ") { // SDM 1.5Test8.3
		// set mainframe position and show it
		int left, top, width, height;
		if (swscanf_s(szParam, _T("%d,%d,%d,%d"), &left, &top, &width, &height) == 4) {
			m_pMainWnd->ShowWindow(SW_RESTORE);// SDM 1.5Test10.0
			m_pMainWnd->SetWindowPos(NULL, left, top, width, height, SWP_SHOWWINDOW | SWP_NOZORDER);// SDM 1.5Test10.0
		}
		else {
			TRACE(_T("Incorrect Size Parameters\n"));// SDM 1.5Test10.0
			m_pMainWnd->ShowWindow(SW_SHOW);
		}
	}
	else {
		m_pMainWnd->ShowWindow(SW_SHOW);
		TRACE(_T("Position Missing\n"));
	}
	if (wParam == SPEECH_WPARAM_SHOWSA) {
		// read WAV files in list file and open them
		CSaString szEntry = "File0"; // set profile entry name
		m_nEntry = 0;
		TCHAR szTemp[3] = _T("0");
		while (TRUE) {
			szReturn = GetBatchString(_T("AudioFiles"), szEntry); // get the entry
			if (szReturn.GetLength() <= 0) {
				break;    // no entry, finish loop
			}
			WriteBatchString(_T("AudioFiles"), szEntry, _T(":")); // set entry to unchanged
			// open the document
			CSaDoc * pModel = (CSaDoc *)OpenDocumentFile(szReturn);
			if (pModel) {
				// set document ID
				pModel->SetID(m_nEntry);
				// get start and stop cursor positions
				szEntry = "Offset";
				szEntry += szTemp; // set entry
				// get start cursor position
				szReturn = GetBatchString(_T("BeginningWAVOffsets"), szEntry); // get the entry
				if (szReturn.GetLength() > 0) { // string found
					dwStart = _ttol(szReturn);
					// set stop cursor position
					szReturn = GetBatchString(_T("EndingWAVOffsets"), szEntry); // get the entry
					dwStop = _ttol(szReturn);
					// get pointer to view
					POSITION pos = pModel->GetFirstViewPosition();
					CSaView * pView = (CSaView *)pModel->GetNextView(pos); // get pointer to view
					if (dwStop > dwStart) {
						// set start and stop cursors and view frame
						pView->SetStartCursorPosition(dwStart);
						pView->SetStopCursorPosition(dwStop);
						pView->GraphsZoomCursors(dwStart, dwStop);
					}
				}
			}
			szEntry = "File";
			swprintf_s(szTemp, _T("%i"), ++m_nEntry); // create new number
			szEntry += szTemp; // add new file number
		}
		// SDM 1.06.8 Process Commands in Batch File
		m_nCommand = 0;// SDM 1.5Test8.5
		m_pMainWnd->PostMessage(WM_COMMAND, ID_PROCESS_BATCH_COMMANDS, 0L);
	}
	if (wParam == SPEECH_WPARAM_SHOWSAREC) {
		// create new file and launch recorder
		CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
		pMDIFrameWnd->PostMessage(WM_COMMAND, ID_FILE_RECORD, 0L);
	}
	m_bModified = FALSE; // batch file not yet modified
}

// SDM 1.5Test8.3
/***************************************************************************/
// extractCommaField local helper function to get field from comma delimited string
/***************************************************************************/
static const CSaString extractCommaField(const CSaString & szLine, const int nField) {
	int nCount = 0;
	int nLoop = 0;

	while ((nLoop < szLine.GetLength()) && (nCount < nField)) {
		if (szLine[nLoop] == ',') {
			nCount++;
		}
		nLoop++;
	}
	int nBegin = nLoop;
	while ((nLoop < szLine.GetLength()) && (szLine[nLoop] != ',')) {
		nLoop++;
	}
	return szLine.Mid(nBegin, nLoop - nBegin);
}

// SDM 1.06.8 New
// SDM 1.5Test8.3 Changed to function notation for parameter passing
// SDM 1.5Test8.5 Changed to single command per call added plat & displayplot
/***************************************************************************/
// CSaApp::OnProcessBatchCommands
// Process batch commands from listFile
/***************************************************************************/
void CSaApp::OnProcessBatchCommands() {

	if (m_nCommand == -1) {
		return;
	}

	// Process Commands
	CSaString szEntry;
	CSaString szReturn;
	CSaString szParameterList;

	swprintf_s(szEntry.GetBuffer(12), 12, _T("command%i"), m_nCommand);
	szEntry.ReleaseBuffer();
	szReturn = GetBatchString(_T("Commands"), szEntry, _T("")); // get the entry
	szReturn.MakeUpper(); // convert the whole string to upper case letters
	szEntry = szReturn;
	swscanf_s(szEntry, _T("%[^(](%[^)]"), szReturn.GetBuffer(szReturn.GetLength()), szReturn.GetLength(), szParameterList.GetBuffer(szReturn.GetLength()), szReturn.GetLength());
	szReturn.ReleaseBuffer();
	szParameterList.ReleaseBuffer();

	if (szReturn == "") {
		// reset cursor positions after batch commands
		POSITION position = m_pDocTemplate->GetFirstDocPosition();
		while (position != NULL) {
			CDocument * pModel = m_pDocTemplate->GetNextDoc(position); // get pointer to document
			if (pModel && pModel->IsKindOf(RUNTIME_CLASS(CSaDoc)) && ((CSaDoc *)pModel)->GetID() != -1) {
				swprintf_s(szEntry.GetBuffer(12), 12, _T("Offset%i"), ((CSaDoc *)pModel)->GetID()); // create new number
				szEntry.ReleaseBuffer();
				// get start cursor position
				szReturn = GetBatchString(_T("BeginningWAVOffsets"), szEntry); // get the entry
				if (szReturn.GetLength() > 0) { // string found
					DWORD dwStart = _ttol(szReturn);
					// set stop cursor position
					szReturn = GetBatchString(_T("EndingWAVOffsets"), szEntry); // get the entry
					DWORD dwStop = _ttol(szReturn);
					// get pointer to view
					POSITION pos = pModel->GetFirstViewPosition();
					CSaView * pView = (CSaView *)pModel->GetNextView(pos); // get pointer to view
					if (dwStop > dwStart) {
						// set start and stop cursors and view frame
						pView->SetStartCursorPosition(dwStart);
						pView->SetStopCursorPosition(dwStop);
						pView->GraphsZoomCursors(dwStart, dwStop);
					}
				}
			}
		}
		m_nCommand = -1;
		TRACE(_T("End Batch File\n"));
		return;
	}

	TRACE(_T("Batch Command:%s(%s)\n"), szReturn.GetString(), szParameterList.GetString());
	if (szReturn.Left(6) == "IMPORT") {
		CSaString szPath = extractCommaField(szParameterList, 0);

		CImportSFM::EImportMode nMode = CImportSFM::KEEP;
		CSaString szMode = extractCommaField(szParameterList, 1);
		szMode.MakeUpper();
		if (szMode.Find(_T("AUTO")) != -1) {
			nMode = CImportSFM::AUTO;
		}
		else if (szMode.Find(_T("MAN")) != -1) {
			nMode = CImportSFM::MANUAL;
		}

		CFileStatus status;
		if (szPath.GetLength() && CFile::GetStatus(szPath, status)) { // SDM 1.5Test10.0
			CImportSFM helper(szPath, TRUE);
			helper.Import(nMode);
			try { // SDM 1.5Test10.0
				// delete the list file
				FileUtils::Remove(szPath);
			}
			catch (CFileException * e) {
				// error removing file
				ErrorMessage(IDS_ERROR_DELLISTFILE, szPath);
				e->Delete();
			}
		}
		else {
			CSaView * pView = (CSaView *)((CMainFrame *)AfxGetMainWnd())->GetCurrSaView();
			CSaDoc * pModel = (CSaDoc *)pView->GetDocument();
			if ((pModel->GetSegment(GLOSS)->IsEmpty()) && (nMode != CImportSFM::KEEP)) {
				// auto parse
				if (!pModel->AdvancedParseAuto()) {
					// process canceled by user
					pModel->Undo(FALSE);
					return;
				}
			}
			if (nMode == CImportSFM::AUTO) {
				if (!pModel->AdvancedSegment()) {
					// process canceled by user
					pModel->Undo(FALSE);
					return;
				}
			}
		}
		m_pMainWnd->SendMessage(WM_COMMAND, ID_FILE_SAVE, 0l); // Save the file
	}
	else if (szReturn.Left(10) == "SELECTFILE") {
		int nID = 1;
		swscanf_s(szParameterList, _T("%d"), &nID);
		POSITION position = m_pDocTemplate->GetFirstDocPosition();
		while (position != NULL) {
			CDocument * pModel = m_pDocTemplate->GetNextDoc(position); // get pointer to document
			if (pModel->IsKindOf(RUNTIME_CLASS(CSaDoc)) && ((CSaDoc *)pModel)->GetID() == nID) {
				position = pModel->GetFirstViewPosition();
				CView * pView = pModel->GetNextView(position);
				pView->GetParent()->BringWindowToTop();
			}
		}
	}
	else if (szReturn.Left(8) == "SAVEFILE") {
		m_pMainWnd->SendMessage(WM_COMMAND, ID_FILE_SAVE, 0l);
	}
	else if (szReturn.Left(4) == "PLAY") {
		CSaView * pView = (CSaView *)((CMainFrame *)AfxGetMainWnd())->GetCurrSaView();
		CSaDoc * pModel = 0;
		if (pView) {
			pModel = pView->GetDocument();
		}

		enum {
			Player_Batch_Settings = 24
		};

		ASSERT(pView && pModel);

		CFnKeys * pKeys = ((CMainFrame *)m_pMainWnd)->GetFnKeys(0);
		pKeys->bRepeat[Player_Batch_Settings] = FALSE;     // TRUE, if playback repeat enabled
		pKeys->nDelay[Player_Batch_Settings] = 100;        // repeat delay time in ms
		pKeys->nMode[Player_Batch_Settings] = ID_PLAYBACK_CURSORS;       // replay mode

		//Get speed
		pKeys->nSpeed[Player_Batch_Settings] = 50;        // default replay speed in %
		CSaString szField = extractCommaField(szParameterList, 0);
		swscanf_s(szField, _T("%u"), &(pKeys->nSpeed[Player_Batch_Settings]));
		if (pKeys->nSpeed[Player_Batch_Settings] < 10) {
			pKeys->nSpeed[Player_Batch_Settings] = 10;
		}
		if (pKeys->nSpeed[Player_Batch_Settings] > 333) {
			pKeys->nSpeed[Player_Batch_Settings] = 333;
		}

		//Get volume
		pKeys->nVolume[Player_Batch_Settings] = 50;       // default play volume in %
		szField = extractCommaField(szParameterList, 1);
		swscanf_s(szField, _T("%u"), &(pKeys->nVolume[Player_Batch_Settings]));
		if (pKeys->nVolume[Player_Batch_Settings] < 0) {
			pKeys->nVolume[Player_Batch_Settings] = 0;
		}
		if (pKeys->nVolume[Player_Batch_Settings] > 100) {
			pKeys->nVolume[Player_Batch_Settings] = 100;
		}

		//Get Start Position
		DWORD dwStart = 0;
		szField = extractCommaField(szParameterList, 2);
		swscanf_s(szField, _T("%lu"), &dwStart);

		//Get Stop Position
		DWORD dwStop = pModel->GetDataSize();
		szField = extractCommaField(szParameterList, 3);
		swscanf_s(szField, _T("%lu"), &dwStop);

		// set start and stop cursors
		pView->SetStartStopCursorPosition(dwStart, dwStop);

		TRACE(_T("PLAY(%u,%u,%lu,%lu)\n"), pKeys->nSpeed[Player_Batch_Settings], pKeys->nVolume[Player_Batch_Settings], dwStart, dwStop);

		m_pMainWnd->PostMessage(WM_USER_PLAYER, CDlgPlayer::PLAYING, MAKELONG(Player_Batch_Settings, -1));
		m_nCommand++;
		// Play Stop will resume batch processing
		return;
	}
	else if (szReturn.Left(11) == "DISPLAYPLOT") {
		CSaView * pView = (CSaView *)((CMainFrame *)AfxGetMainWnd())->GetCurrSaView();
		CSaDoc * pModel = 0;
		if (pView) {
			pModel = pView->GetDocument();
		}

		ASSERT(pView && pModel);
		// Get Type
		CSaString szType = extractCommaField(szParameterList, 0);

		TRACE(_T("DisplayPlot(%s)\n"), szType.GetString());
		((CMainFrame *)AfxGetMainWnd())->DisplayPlot(new CDisplayPlot(szType));

		m_nCommand++;
		// DisplayPlot End will resume batch processing
		// m_pMainWnd->PostMessage(WM_COMMAND, ID_PROCESS_BATCH_COMMANDS, 0L);
		return;
	}
	else if (szReturn.Left(6) == "RETURN") {
		int nHide = 1;
		swscanf_s(szParameterList, _T("%d"), &nHide);
		m_nCommand = -1;
		FileReturn(nHide > 0);
		return;
	}
	else {
		m_nCommand = -1;
		TRACE(_T("Error End Batch File\n"));
		return;
	}
	m_nCommand++;
	m_pMainWnd->PostMessage(WM_COMMAND, ID_PROCESS_BATCH_COMMANDS, 0L);
}

/***************************************************************************/
// CSaApp::ErrorMessage Set error message
// Set an error message in the queue to be displayed as soon as possible.
/***************************************************************************/
void CSaApp::ErrorMessage(UINT nTextID, LPCTSTR pszText1, LPCTSTR pszText2) {
	CSaString szText;
	try {
		// create the text
		if (pszText1) {
			if (pszText2) {
				AfxFormatString2(szText, nTextID, pszText1, pszText2);
			}
			else {
				AfxFormatString1(szText, nTextID, pszText1);
			}
		}
		else {
			szText.LoadString(nTextID);
		}
		// add the string to the array
		m_pszErrors.Add(szText);
	}
	catch (CMemoryException * e) {
		// memory allocation error
		ErrorMessage(IDS_ERROR_MEMALLOC);
		e->Delete();
	}
}

/***************************************************************************/
// CSaApp::Message Set error message
// Set an error message in the queue to be displayed as soon as possible.
/***************************************************************************/
void CSaApp::Message(UINT nTextID, LPCTSTR pszText1, LPCTSTR pszText2) {
	CSaString szText;
	try {
		// create the text
		if (pszText1) {
			if (pszText2) {
				AfxFormatString2(szText, nTextID, pszText1, pszText2);
			}
			else {
				AfxFormatString1(szText, nTextID, pszText1);
			}
		}
		else {
			szText.LoadString(nTextID);
		}
		m_pszMessages.Add(szText); // add the string to the array
	}
	catch (CMemoryException * e) {
		// memory allocation error
		ErrorMessage(IDS_ERROR_MEMALLOC);
		e->Delete();
	}
}

/***************************************************************************/
// CSaApp::ErrorMessage Set error message
// Set an error message in the queue to be displayed as soon as possible.
/***************************************************************************/
void CSaApp::ErrorMessage(CSaString & szText) {
#ifdef _DEBUG
	ASSERT(FALSE);
#endif
	// add the string to the array
	m_pszErrors.Add(szText);
}

/***************************************************************************/
// CSaApp::SetBatchFileChanged Sets a entry in the list(batch) file to changed
// Called by the documents this function sets a entry (file name) in the list
// file (application in batch mode) to changed state by writing the name of
// the changed file into the list file.
/***************************************************************************/
void CSaApp::SetBatchFileChanged(CSaString szFileName, int nID, CDocument * pModel) {

	CSaDoc * pSaDoc = (CSaDoc *)pModel;
	TCHAR szTemp[3];
	CSaString szEntry = "File";
	if (nID == -1) {
		// no valid entry, new file
		swprintf_s(szTemp, _T("%i"), m_nEntry);
	}
	else {
		// create entry number from ID
		// create number for entry
		swprintf_s(szTemp, _T("%i"), nID);
	}
	// create entry
	szEntry += szTemp;
	TCHAR szShortName[MAX_PATH];
	GetShortPathName(szFileName, szShortName, MAX_PATH);
	// set entry to changed or new file
	WriteBatchString(_T("AudioFiles"), szEntry, szShortName);
	// no valid entry, new file
	if (nID == -1) {
		// set the new ID
		pSaDoc->SetID(m_nEntry++);
		szEntry = "DocID";
		// create entry
		szEntry += szTemp;
		// set Doc ID to 0
		WriteBatchString(_T("DocIDs"), szEntry, _T("0"));
	}
	// file has been modified
	m_bModified = TRUE;
}

/***************************************************************************/
// CSaApp::IsFileOpened Check if file already opened
// This function looks through the document list and compares all the
// documents file names with the one given as parameter (full path). If there
// is a match it returns a pointer to the document that matches, else NULL.
/***************************************************************************/
CSaDoc * CSaApp::IsFileOpened(LPCTSTR pszFileName) {
	wstring szFileName = pszFileName;
	POSITION position = m_pDocTemplate->GetFirstDocPosition();
	while (position != NULL) {
		// get pointer to document
		CDocument * pModel = m_pDocTemplate->GetNextDoc(position);
		CString szComparisonFile = pModel->GetPathName();
		TRACE(_T(":IsFileOpened %s %s\n"), (LPCTSTR)szComparisonFile, (LPCTSTR)szFileName.c_str());
		if (compare_no_case(szComparisonFile,szFileName.c_str())) {
			return (CSaDoc*)pModel;
		}
	}
	return nullptr;
}

/***************************************************************************/
// CSaApp::IsFileOpened Check if file already opened
// This function looks through the document list and compares all the
// documents file names with the one given as parameter (full path). If there
// is a match it returns a pointer to the document that matches, else NULL.
/***************************************************************************/
bool CSaApp::IsDocumentOpened(const CSaDoc * pModel) {
	POSITION position = m_pDocTemplate->GetFirstDocPosition();
	while (position != NULL) {
		CDocument * pLoopDoc = m_pDocTemplate->GetNextDoc(position); // get pointer to document
		if (pLoopDoc->IsKindOf(RUNTIME_CLASS(CSaDoc))) {
			CSaDoc * pSaDoc = (CSaDoc *)pLoopDoc;
			if (pSaDoc == pModel) {
				return true; // match
			}
		}
	}
	return false;
}

/***************************************************************************/
// CSaApp::SetWorkbenchPath Set the workbenchs document pathname
// If the pointer pszPath contains NULL, the path is cleared.
/***************************************************************************/
void CSaApp::SetWorkbenchPath(CSaString * pszPath) {
	if (pszPath) {
		m_szWbPath = *pszPath;
	}
	else {
		m_szWbPath.Empty();
	}
}

/***************************************************************************/
// CSaApp::CloseWorkbench Close the workbench
// Returns TRUE if workbench closed, FALSE if not.
/***************************************************************************/
BOOL CSaApp::CloseWorkbench(CDocument * pModel) {
	if (m_pWbDoc) {
		// close the workbench view
		POSITION pos = m_pWbDoc->GetFirstViewPosition();
		CView * pView = m_pWbDoc->GetNextView(pos);
		pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0); // close view
	}
	BOOL bRet = (m_pWbDoc == NULL);
	m_pWbDoc = pModel;
	return bRet;
}

/***************************************************************************/
// CSaApp::PasteClipboardToNewFile Create a new file and paste wave data into it
// This function creates a new document.
/***************************************************************************/
void CSaApp::PasteClipboardToNewFile(HGLOBAL hData) {

	//because we now use true CF_WAVE we can save as temp then open
	// temporary target file has to be created
	TCHAR szTempPath[_MAX_PATH];
	if (!CClipboardHelper::LoadFileFromData(hData, szTempPath, _MAX_PATH)) {
		TRACE("unable to load clipboard data into file");
		return;
	}

	CopyClipboardTranscription(szTempPath);

	// open the new file
	CSaDoc * pResult = OpenWavFileAsNew(szTempPath);
	if (!pResult) {
		// Error opening file, destroy temp
		FileUtils::Remove(szTempPath);
	}
}

CSaDoc * CSaApp::OpenWavFileAsNew(LPCTSTR szTempPath) {

	// create new MDI child, sa type
	POSITION posTemplate = GetFirstDocTemplatePosition();
	if (!GetNextDocTemplate(posTemplate)) {
		TRACE0("Error : no document templates registered with CWinApp\n");
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC, MB_OK, 0);
		return NULL;
	}
	// this is a file new operation
	m_bNewDocument = TRUE;
	posTemplate = GetFirstDocTemplatePosition();
	CDocTemplate * pTemplate = GetNextDocTemplate(posTemplate);
	ASSERT(pTemplate != NULL);
	ASSERT(pTemplate->IsKindOf(RUNTIME_CLASS(CDocTemplate)));
	CSaDoc * pModel = (CSaDoc *)pTemplate->OpenDocumentFile(NULL);
	m_bNewDocument = FALSE;

	if ((!pModel) || (!pModel->IsKindOf(RUNTIME_CLASS(CSaDoc)))) {
		if (pModel) {
			pModel->OnCloseDocument();
		}
		// Error opening file, destroy temp
		return NULL;
	}
	else {
		// Load temporarary file into document
		if (!pModel->LoadDataFiles(szTempPath, true)) {
			if (pModel) {
				pModel->OnCloseDocument();
			}
			return NULL;
		}
		return pModel;
	}
}

/***************************************************************************/
// CSaApp::CopyClipboardTranscription
// Copies the transcription associated with the last clipboard file from SA.
// If the wave data on the clipboard doesn't match the last data copied
// from SA, nothing is copied since it's from another app.
/***************************************************************************/
void CSaApp::CopyClipboardTranscription(LPCTSTR szTempPath) {

	_bstr_t szMD5HashCode = (wchar_t *)0;

	CoInitialize(NULL);
	ISaAudioDocumentWriterPtr saAudioDocWriter;
	HRESULT hr = saAudioDocWriter.CreateInstance(__uuidof(SaAudioDocumentWriter));
	if (hr) {
		CSaString msg;
		msg.Format(_T("%x"), hr);
		ErrorMessage(IDS_ERROR_CREATE_INSTANCE, _T("SaAudioDocumentWriter.CreateInstance()"), msg);
		return;
	}

	if (!saAudioDocWriter->Initialize((_bstr_t)m_szLastClipboardPath, szMD5HashCode, VARIANT_TRUE)) {
		// TODO: Display a meaningful error.
		ErrorMessage(IDS_ERROR_WRITEPHONETIC, m_szLastClipboardPath);
		saAudioDocWriter->Close();
		saAudioDocWriter->Release();
		saAudioDocWriter = NULL;
		CoUninitialize();
		return;
	}

	try {
		_bstr_t szDest = szTempPath;
		hr = saAudioDocWriter->Copy(szDest, VARIANT_TRUE);
		if (hr) {
			CSaString msg;
			msg.Format(_T("%x"), hr);
			ErrorMessage(IDS_ERROR_CREATE_INSTANCE, _T("SaAudioDocumentWriter.Copy()"), msg);
		}
	}
	catch (_com_error & ) {
		CString msg;
		msg.Format(_T("%x"), hr);
		ErrorMessage(IDS_ERROR_CREATE_INSTANCE, _T("SaAudioDocumentWriter.Copy()"), msg);
	}
}

/***************************************************************************/
// CSaApp::OpenBlankView Opens a blank view to do something on.
//   RLJ 05/31/2000
// This functionality was moved here from OnFileRecord, since it is needed
// not only there, but also in FileOpen and OnFileCreate (I THINK).
/***************************************************************************/
CDocument * CSaApp::OpenBlankView(bool bWithGraphs) {

	// create new MDI child, sa type
	POSITION posTemplate = GetFirstDocTemplatePosition();
	if (!GetNextDocTemplate(posTemplate)) {
		TRACE0("Error : no document templates registered with CWinApp\n");
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC, MB_OK, 0);
		return NULL;
	}

	CDocument * pModel = NULL;
	if (bWithGraphs) {
		SetOpenMore(false);
		SetOpenAsID(ID_FILE_OPENAS_NEW);
		posTemplate = GetFirstDocTemplatePosition();
		CDocTemplate * pTemplate = GetNextDocTemplate(posTemplate);
		ASSERT(pTemplate != NULL);
		ASSERT(pTemplate->IsKindOf(RUNTIME_CLASS(CDocTemplate)));
		m_bNewDocument = TRUE;
		pModel = pTemplate->OpenDocumentFile(NULL);
		m_bNewDocument = FALSE;
	}
	else {
		posTemplate = GetFirstDocTemplatePosition();
		CDocTemplate * pTemplate = GetNextDocTemplate(posTemplate);
		ASSERT(pTemplate != NULL);
		ASSERT(pTemplate->IsKindOf(RUNTIME_CLASS(CDocTemplate)));
		pModel = pTemplate->OpenDocumentFile(NULL);
	}
	return pModel;
}

/////////////////////////////////////////////////////////////////////////////
// CSaApp message handlers

/***************************************************************************/
// CSaApp::OnAppAbout Creates about dialog
// The user wants to see the modal about dialog.
/***************************************************************************/
void CSaApp::OnAppAbout() {

	// initialize the about box object
	CoInitialize(NULL);
	IAboutDlgPtr aboutDlg;

	HRESULT createResult = aboutDlg.CreateInstance(__uuidof(SAAboutDlg));
	if (createResult) {
		CSaString szCreateResult;
		szCreateResult.Format(_T("%x"), createResult);
		ErrorMessage(IDS_ERROR_CREATE_INSTANCE, _T("AboutDlg.CreateInstance()"), szCreateResult);
		return;
	}

	// set the text values
	aboutDlg->ProdName = _T("Speech Analyzer");
	CSaString szVersion = GetVersionString();
	CSaString szBuild;
	szVersion = szVersion.Right(szVersion.GetLength() - szVersion.Find(' ') - 1);
	int nBuildIndex = szVersion.Find(_T("Build"));
	if (nBuildIndex > 0) {
		szBuild = szVersion.Mid(nBuildIndex, szVersion.GetLength() - nBuildIndex - 1);
		szVersion = szVersion.Left(nBuildIndex - 2);
	}

	// Remove RC number
	int nRCIndex = szVersion.Find(_T("RC"));
	if (nRCIndex > 0) {
		szVersion = szVersion.Left(nRCIndex - 1);
	}

	CSaString szCopyright((LPCTSTR)VS_COPYRIGHT);;
	aboutDlg->ProdVersion = (_bstr_t)szVersion;
	aboutDlg->Copyright = (_bstr_t)szCopyright;
	aboutDlg->DriveLetter = _T("C");
	aboutDlg->Build = (_bstr_t)szBuild;

	// now show it
	aboutDlg->ShowDialog();

	aboutDlg->Release();
	aboutDlg = NULL;

	CoUninitialize();
}

/***************************************************************************/
// CSaApp::OnFileCreate Creates new view
// The user wants to create a new view (and document) sa type. Overwritten
// MFC OnFileNew, because MFC lets the user choose a view type.
/***************************************************************************/
void CSaApp::OnFileCreate() {
	OpenBlankView(false);
}

/***************************************************************************/
// CSaApp::OnUpdateFileCreate Menu update
/***************************************************************************/
void CSaApp::OnUpdateFileCreate(CCmdUI * pCmdUI) {
	pCmdUI->Enable(TRUE);
}

/***************************************************************************/
// CSaApp::OnFileRecord Records new wave file
// The user wants to record a new wave file. This function creates a new
// view and launches the recorder. It contains the overwritten MFC OnFileNew,
// because MFC lets the user choose a view type.
/***************************************************************************/
void CSaApp::OnFileRecord() {

	// use auto naming
	OpenBlankView(true);

	CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
	ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));

	// give editor a chance to close
	pMainFrame->SendMessage(WM_USER_IDLE_UPDATE, 0, 0);

	// launch recorder in this new view
	// send message to start recorder
	pMainFrame->MDIGetActive()->GetActiveView()->SendMessage(WM_USER_RECORDER, 0, 0);
}

/***************************************************************************/
// CSaApp::FileOpen Opens an existing wave file
//   RLJ 05/31/2000
//   MOST OF THIS CODE WAS COPIED FROM OnFileRecord
/***************************************************************************/
void CSaApp::FileOpen(UINT openAsID) {

	// cache values, initial update will overwrite them...
	m_OpenAsID = openAsID;
	bool openMore = m_bOpenMore;

	// uses auto naming
	CDocument * pModel = OpenBlankView(true);
	// this allows screen to be drawn
	CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
	ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));
	// give editor a chance to close
	pMainFrame->SendMessage(WM_USER_IDLE_UPDATE, 0, 0);

	// restore
	SetOpenMore(openMore);
	SetOpenAsID(openAsID);

	CString extension = _T("wav");
	CString filename = _T("*.wav");
	CString filter = _T("WAV Files (*.wav)|*.wav|Other Audio Files (*.mp3;*.wma )|*.mp3;*.wma|Speech Analyzer Files (*.saxml) |*.saxml|Speech Analyzer Workbench Files (*.wb) |*.wb|ELAN Files (*.eaf)|*.eaf||");

	CDlgFileOpen dlg(extension, filename, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER, filter);
	CString defaultDir;
	if (m_bOpenMore) {
		defaultDir = GetDefaultDir();
	}
	else {
		defaultDir = GetSamplesDir();
	}
	dlg.m_ofn.lpstrInitialDir = defaultDir;
	if (dlg.DoModal() == IDOK) {
		OpenDocumentFile(dlg.GetPathName());
	}

	if (pModel != NULL) {
		pModel->OnCloseDocument();
	}
}

CSaString CSaApp::GetDefaultDir() const {

	// prompt the user (with all document templates)
	CSaString workingDir;
	for (int i = 0; i < _AFX_MRU_MAX_COUNT; i++) {
		workingDir = GetMRUFilePath(i);
		if (workingDir.GetLength() > 0) {
			CSaString szPath = workingDir;
			int nFind = szPath.ReverseFind('\\');
			if (nFind != -1) {
				CFileStatus status;
				if ((CFile::GetStatus(szPath.Left(nFind), status)) && (status.m_attribute & CFile::directory)) {
					workingDir = szPath.Left(nFind + 1);
					break;
				}
				else {
					// directory does not exist
					workingDir.Empty();
				}
			}
		}
	}

	if (workingDir.IsEmpty()) {
		// check data location in registry
		workingDir = ((CWinApp *)this)->GetProfileString(_T(""), _T("DataLocation"));
	}

	if (workingDir.IsEmpty()) {
		// fall back to the current directory
		TCHAR Buffer[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, Buffer);
		workingDir = Buffer;
	}

	return workingDir;
}

// return the samples directory location, or if it doesn't exist, fall back to the 
// default location
CSaString CSaApp::GetSamplesDir() const {

	// get the default sample directory
	// get the data location and create it if it doesn't exist
	// Set the DataLocation path and write it to the registry
	CString szPath = GetShellFolderPath(CSIDL_PERSONAL);
	if (szPath.Right(1) != "\\") {
		szPath += _T("\\");
	}
	szPath += _T("Speech Analyzer\\Samples");
	if (!FileUtils::FolderExists(szPath)) {
		szPath = GetDefaultDir();
	}
	return szPath;
}

/***************************************************************************/
// CSaApp::OnFileOpen Opens an existing wave file
/***************************************************************************/
void CSaApp::OnFileOpen() {
	FileOpen(ID_FILE_OPEN);
}

/***************************************************************************/
// Added on 09/01/2000 by DDO
/***************************************************************************/
void CSaApp::OnFileOpenSpecial() {
	ShowStartupDialog(FALSE);
}

// CSaApp::OnFileOpenPA Opens an existing wave file for Phonetic Analysis
/***************************************************************************/
void CSaApp::OnFileOpenPA() {
	FileOpen(ID_FILE_OPENAS_PHONETICANALYSIS);
}

/***************************************************************************/
// CSaApp::OnFileOpenMA Opens an existing wave file for Music Analysis
/***************************************************************************/
void CSaApp::OnFileOpenMA() {
	FileOpen(ID_FILE_OPENAS_MUSICANALYSIS);
}

/***************************************************************************/
// Added on 09/01/2000 by DDO
/***************************************************************************/
void CSaApp::ShowStartupDialog(BOOL bAppIsStartingUp = TRUE) {

	CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
	CDlgStartMode dlg;
	dlg.m_nDataMode = pMainFrame->GetStartDataMode();
	dlg.m_bShowDontShowAgainOption = bAppIsStartingUp;
	dlg.DoModal();
}

/***************************************************************************/
// CSaApp::OnUpdateFileRecord Menu update
/***************************************************************************/
void CSaApp::OnUpdateFileRecord(CCmdUI * pCmdUI) {

	CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
	BOOL bEnable = TRUE;
	if (pMDIFrameWnd) {
		CView * pView = pMDIFrameWnd->GetCurrSaView();
		if (pView) {
			bEnable = pView->IsKindOf(RUNTIME_CLASS(CSaView));
		}
	}
	pCmdUI->Enable(bEnable);
}

/***************************************************************************/
// CSaApp::OnUpdateFileOpen Menu update
/***************************************************************************/
void CSaApp::OnUpdateFileOpen(CCmdUI * pCmdUI) {

	pCmdUI->Enable(GetBatchMode() == 0); // enabled only if no batch mode
}

// SDM 1.06.8 New
/***************************************************************************/
// CSaApp::OnUpdateRecentFileMenu Menu update
/***************************************************************************/
void CSaApp::OnUpdateRecentFileMenu(CCmdUI * pCmdUI) {

	int nIndex = pCmdUI->m_nIndex;
	// MFC only calls for first MRU entry
	if (pCmdUI->m_nID == ID_FILE_MRU_FILE1) {
		CWinApp::OnUpdateRecentFileMenu(pCmdUI);
	}
	// MFC skips other entries force full update
	pCmdUI->m_nIndex = nIndex;
	pCmdUI->Enable(GetBatchMode() == 0); // enabled only if no batch mode
}

/***************************************************************************/
// CSaApp::OnFileReturn Hides the application and activates PA
//
// 8/4/06 This is now mostly deprecated, since PA doesn't call SA
// this way anymore.
/***************************************************************************/
void CSaApp::OnFileReturn() {

	FileReturn();
}

void CSaApp::FileReturn(BOOL bHide) {
	if (SaveAllModified()) {
		CloseAllDocuments(FALSE);
		if (!m_bModified) { // modified batch files are available
			// there are no changes in the list file, so delete the list file
			// delete the list file
			FileUtils::Remove(m_szCmdFileName);
		}
		CWnd * pWnd = IsAppRunning(); // SDM 1.5Test8.5
		if (pWnd != NULL) {
			if (bHide) { // Optional Hide
				m_pMainWnd->ShowWindow(SW_HIDE);
				m_nCmdShow = SW_HIDE; // to prevent MFC to restore on startup
			}
			pWnd->SendMessage(WM_USER_SPEECHAPPLICATION, SPEECH_WPARAM_SHOWSM, 0);
			if (bHide) {
				CancelBatchMode();
				OnAppExit();
			}
		}
		else { // SDM 1.5Test8.5 allow option to exit app
			TRACE(_T("CallingApp not found\n"));
			if (AfxMessageBox(IDS_QUESTION_APPNOTFOUND, MB_YESNO | MB_ICONQUESTION, 0) == IDYES) {
				CancelBatchMode();
				OnAppExit();
			}
		}
	}
}

// SDM 1.5Test8.5
/***************************************************************************/
// CSaApp::IsAppRunning Checks if the calling application is running.
// The function examines the top level windows after their caption. 
// If it finds the CallingApp caption text, it returns a pointer to its window, 
// else a NULL pointer.
/***************************************************************************/
CWnd * CSaApp::IsAppRunning() {

	// why bother continuing?
	if (m_szCallingApp.IsEmpty()) {
		return NULL;
	}

	// get pointer to first toplevel window
	CWnd * pTopWnd = AfxGetMainWnd()->GetWindow(GW_HWNDFIRST);
	while (pTopWnd != NULL) {
		CSaString szCaption;
		// get caption text
		pTopWnd->GetWindowText(szCaption);
		if (szCaption.Left(m_szCallingApp.GetLength()).CompareNoCase(m_szCallingApp) == 0) {
			//found
			return pTopWnd;
		}
		// get pointer to next toplevel window
		pTopWnd = pTopWnd->GetWindow(GW_HWNDNEXT);
	}
	return NULL;
}

/***************************************************************************/
// CSaApp::OnUpdateFileReturn Menu update
/***************************************************************************/
void CSaApp::OnUpdateFileReturn(CCmdUI * pCmdUI) {
	// enabled only if batch mode
	pCmdUI->Enable(GetBatchMode() != 0);
}

/***************************************************************************/
// CSaApp::OnUpdateAppExit Menu update
/***************************************************************************/
void CSaApp::OnUpdateAppExit(CCmdUI * pCmdUI) {
	// enabled only if no batch mode
	pCmdUI->Enable(GetBatchMode() == 0);
}

/***************************************************************************/
// CSaApp::OnIdle Idle processing
/***************************************************************************/
BOOL CSaApp::OnIdle(LONG lCount) {

	// during termination, the main window may no longer be valid
	CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
	if (pMainWnd == NULL) {
		return FALSE;
	}

	// SDM 1.06.5 Send mainframe idle update message
	if (lCount <= 0) {
		if ((pMainWnd->m_hWnd != NULL) &&
			(pMainWnd->IsWindowVisible())) {
			// SDM 32 bit conversion
			pMainWnd->SendMessage(WM_USER_IDLE_UPDATE, 0, 0L);
		}

	}
	BOOL bMore = CWinApp::OnIdle(lCount);
	// display error message if present
	DisplayMessages();

	if (bMore) {
		// more idle processing necessary
		return TRUE;
	}

	// get active document
	CSaDoc * pSaDoc = pMainWnd->GetCurrDoc();

	// perform pitch processing if not already done
	if ((pSaDoc) &&
		(pSaDoc->IsBackgroundProcessing()) &&
		(pSaDoc->GetDataSize())) {
		CProcessGrappl * pAutoPitch = pSaDoc->GetGrappl();
		if (!pAutoPitch->IsDataReady()) {
			if (pAutoPitch->IsCanceled()) {
				pSaDoc->EnableBackgroundProcessing(FALSE);
			}
			else {
				// process data
				short int nResult = LOWORD(pAutoPitch->Process(this));
				if (nResult == PROCESS_ERROR || nResult == PROCESS_NO_DATA || nResult == PROCESS_CANCELED) {
					pAutoPitch->SetDataInvalid();
					pSaDoc->EnableBackgroundProcessing(FALSE);
					//pAutoPitch->RestartProcess(this);
				}
				else if (pAutoPitch->IsDataReady()) {
					pSaDoc->NotifyAutoPitchDone(this);
				}
			}
			return TRUE;
		}
		// run fragmenter if not already done
		CProcessFragments * pFragmenter = pSaDoc->GetFragments();
		if (!pFragmenter->IsDataReady()) {
			// process data
			short int nResult = LOWORD(pFragmenter->Process(this));
			if ((nResult == PROCESS_ERROR) || (nResult == PROCESS_NO_DATA) || (nResult == PROCESS_CANCELED)) {
				pFragmenter->SetDataInvalid();
				pSaDoc->EnableBackgroundProcessing(FALSE);
				pFragmenter->RestartProcess();
			}
			else {
				return TRUE;
			}
		}
	}

	return bMore;
}

/***************************************************************************/
// CSaApp::DisplayMessages Displays a stored error message
/***************************************************************************/
void CSaApp::DisplayMessages() {

	if (m_pszErrors.GetSize() > 0) {
		CSaString error = m_pszErrors.GetAt(0);
		// remove message (before we lose process thread)
		m_pszErrors.RemoveAt(0, 1);
		::AfxMessageBox(error, MB_OK | MB_ICONEXCLAMATION, 0);
	}
	if (m_pszMessages.GetSize() > 0) {
		CSaString error = m_pszMessages.GetAt(0);
		// remove message (before we lose process thread)
		m_pszMessages.RemoveAt(0, 1);
		::AfxMessageBox(error, MB_OK | MB_ICONINFORMATION, 0);
	}
}

/***************************************************************************/
// CSaApp::OnHelpContents Call Help Index (Table of Contents)
/***************************************************************************/
void CSaApp::OnHelpContents() {
	::HtmlHelp(NULL, m_pszHelpFilePath, HH_DISPLAY_TOC, NULL);
}

void CSaApp::OnHelpShortCuts() {
	CSaString szAppPath = m_pszHelpFilePath;
	szAppPath = szAppPath.Left(szAppPath.ReverseFind('\\'));
	CSaString szCommandLine = "\"" + szAppPath + _T("\\Keyboard Short-Cut Keys.pdf\"");
	ShellExecute(NULL, _T("open"), szCommandLine.GetBuffer(1), NULL, NULL, SW_SHOWNORMAL);
}

/***************************************************************************/
// CSaApp::OnHelpTrouble Call Troubleshooting help topic
/***************************************************************************/
void CSaApp::OnHelpTrouble() {

	CSaString szPath = m_pszHelpFilePath;
	szPath += "::/troubleshooting/overview.htm";
	::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

/***************************************************************************/
// CSaApp::OnHelpWhatsNew Call new feature documentation
/***************************************************************************/
void CSaApp::OnHelpWhatsNew() {

	CSaString szAppPath = m_pszHelpFilePath;
	szAppPath = szAppPath.Left(szAppPath.ReverseFind('\\'));
	CSaString szCommandLine = "\"" + szAppPath + _T("\\Release Notes.txt\"");
	ShellExecute(NULL, _T("open"), szCommandLine.GetBuffer(1), NULL, NULL, SW_SHOWNORMAL);
}

/***************************************************************************/
// CSaApp::OnHelpSFMMarkers Display SFM Marker table
/***************************************************************************/
void CSaApp::OnHelpSFMMarkers() {

	CDlgHelpSFMMarkers dlg;
	dlg.DoModal();
}
/***************************************************************************/
// CSaApp::OnHelpGraphs Call Graphs overview help topic
/***************************************************************************/
void CSaApp::OnHelpGraphs() {

	// create the pathname
	CSaString szPath = m_pszHelpFilePath;
	szPath = szPath + "::/user-interface/menus/graphs/overview.htm";
	::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

/***************************************************************************/
// CSaApp::OnHelpMusic Call Musical Analysis help topic
/***************************************************************************/
void CSaApp::OnHelpMusic() {

	// create the pathname
	CSaString szPath = m_pszHelpFilePath;
	szPath = szPath + "::/user-interface/menus/graphs/types/music/analysis.htm";
	::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

/***************************************************************************/
// CSaApp::OnHelpTrainingStudent Open Student Manual
/***************************************************************************/
void CSaApp::OnHelpTrainingStudent() {

	CSaString szAppPath = m_pszHelpFilePath;
	szAppPath = szAppPath.Left(szAppPath.ReverseFind('\\'));
	CSaString szCommandLine = "\"" + szAppPath + _T("\\Training\\SA Student Manual.pdf\"");
	ShellExecute(NULL, _T("open"), szCommandLine.GetBuffer(1), NULL, NULL, SW_SHOWNORMAL);
}

/***************************************************************************/
// CSaApp::OnHelpTrainingInstructor Open Instructor Guide
/***************************************************************************/
void CSaApp::OnHelpTrainingInstructor() {

	CSaString szAppPath = m_pszHelpFilePath;
	szAppPath = szAppPath.Left(szAppPath.ReverseFind('\\'));
	CSaString szCommandLine = "\"" + szAppPath + _T("\\Training\\SA Instructor Guide.pdf\"");
	ShellExecute(NULL, _T("open"), szCommandLine.GetBuffer(1), NULL, NULL, SW_SHOWNORMAL);
}

/***************************************************************************/
// CSaApp::OnHelpOnHelp Call Windows Help on Help
/***************************************************************************/
void CSaApp::OnHelpOnHelp() {

	// create the pathname
	CSaString szPath = m_pszHelpFilePath;
	szPath = szPath.Left(szPath.ReverseFind('\\'));
	szPath = szPath + "\\" + "Using_Help.chm" + "::/Using_Help/Using_Help_overview.htm";
	::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

/***************************************************************************/
// CWbDlgProcesses::OnHelpWorkbench Call Workbench help
/***************************************************************************/
void CSaApp::OnHelpWorkbench() {

	// create the pathname
	CString szPath = m_pszHelpFilePath;
	szPath += "::/user-interface/menus/tools/workbench.htm";
	::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

/***************************************************************************/
// CSaApp::OnAudioCon Execute winacon.exe
/***************************************************************************/
void CSaApp::OnAudioCon() {

	// look for AudioCon in a sub-folder
	CSaString szAppPath = m_pszHelpFilePath; // C:\Program Files\SIL Software\Speech Analyzer\SA.exe
	CSaString szAudioConFolder = szAppPath.Left(szAppPath.ReverseFind('\\')) + "\\AudioCon";
	CSaString szAudioConPath = szAudioConFolder + "\\winacon.exe";

	try {
		CFile file(szAudioConPath, CFile::modeRead);
	}
	catch (CFileException * e) {
		if (e->m_cause == CFileException::fileNotFound) {
			ErrorMessage(IDS_ERROR_AUDIOCON_MISSING);
			return;
		}
		else if (e->m_cause == CFileException::sharingViolation) {
			// just ignore this exception and open another AudioCon process
		}
		else {
			ErrorMessage(IDS_ERROR_AUDIOCON_UNKNOWN);
			return;
		}
	}

	CSaString szMessage;
	szMessage.Format(CSaString((LPCTSTR)IDS_AUDIOCON_SAXML_NOTICE), (LPCTSTR)szAudioConFolder);
	::AfxMessageBox(szMessage);

	PROCESS_INFORMATION piAudioCon;
	STARTUPINFO siAudioCon;
	memset(&siAudioCon, 0, sizeof(siAudioCon));
	siAudioCon.cb = sizeof(siAudioCon);
	CreateProcess(szAudioConPath, NULL, NULL, NULL, FALSE, DETACHED_PROCESS, NULL, szAudioConFolder, &siAudioCon, &piAudioCon);

	ShellExecute(NULL, _T("explore"), szAudioConFolder, NULL, NULL, SW_SHOWNORMAL);
}

/***************************************************************************/
// CSaApp::OnWorkbenchOpen Open the workbench
// Contains the MFCs overwritten OnFileNew function to create a workbench
// view.
/***************************************************************************/
void CSaApp::OnWorkbenchOpen() {

	// get pointer to workbench template
	POSITION posTemplate = GetFirstDocTemplatePosition();
	if (!GetNextDocTemplate(posTemplate)) {
		TRACE0("Error : no document templates registered with CWinApp\n");
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC, MB_OK, 0);
		return;
	}
	posTemplate = GetFirstDocTemplatePosition();
	GetNextDocTemplate(posTemplate);
	CDocTemplate * pTemplate = GetNextDocTemplate(posTemplate);
	ASSERT(pTemplate != NULL);
	ASSERT(pTemplate->IsKindOf(RUNTIME_CLASS(CDocTemplate)));
	if (!m_pWbDoc) {
		// create new MDI child, workbench type
		if (m_szWbPath.IsEmpty()) {
			m_pWbDoc = pTemplate->OpenDocumentFile(NULL);
		}
		else {
			m_pWbDoc = pTemplate->OpenDocumentFile(m_szWbPath);
		}
		if (!m_pWbDoc) {
			// was not able to open file
			m_szWbPath.Empty();
		}
	}
	else {
		if (!m_szWbPath.IsEmpty()) {
			// close actually opened workbench and open new one
			POSITION pos = m_pWbDoc->GetFirstViewPosition();
			CView * pView = m_pWbDoc->GetNextView(pos);
			CSaString szSavePath = m_szWbPath;
			// close view
			pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0);
			m_szWbPath = szSavePath;
			m_pWbDoc = pTemplate->OpenDocumentFile(m_szWbPath);
			if (!m_pWbDoc) {
				// was not able to open file
				m_szWbPath.Empty();
			}
		}
	}
}

/***************************************************************************/
// CSaApp::OnUpdateWorkbenchOpen Menu update
/***************************************************************************/
void CSaApp::OnUpdateWorkbenchOpen(CCmdUI * pCmdUI) {

	pCmdUI->SetCheck(m_pWbDoc != NULL);
}

#define PDERR_PRINTERNOTFOUND  0x100B
#define PDERR_DNDMMISMATCH     0x1009


/***************************************************************************/
// CSaApp::SaGetPrinterDeviceDefaults
// This is a special version of the standard MFC GetPrinterDeviceDefaults
// which allows us to set the print orientation (portrait/landscape)
/***************************************************************************/
BOOL CSaApp::SaGetPrinterDeviceDefaults(PRINTDLG * pPrintDlg, BOOL landscape) {

	UpdatePrinterSelection(m_hDevNames == NULL); //force default if no current
	if (m_hDevNames == NULL) {
		return FALSE;               // no printer defaults
	}

	pPrintDlg->hDevNames = m_hDevNames;
	pPrintDlg->hDevMode = m_hDevMode;

	DEVMODE * theDM = (DEVMODE *)GlobalLock(pPrintDlg->hDevMode);

	theDM->dmFields |= DM_ORIENTATION;
	if (landscape) {
		theDM->dmOrientation = DMORIENT_LANDSCAPE;
	}
	else {
		theDM->dmOrientation = DMORIENT_PORTRAIT;
	}

	::GlobalUnlock(m_hDevNames);
	::GlobalUnlock(m_hDevMode);
	return TRUE;
}

/***************************************************************************/
// CSaApp::SaDoPrintDialog
// This is a special version of the standard MFC DoPrintDialog
// which allows us to set the print orientation (portrait/landscape)
/***************************************************************************/
int CSaApp::SaDoPrintDialog(CPrintDialog * pPD, BOOL landscape) {

	UpdatePrinterSelection(FALSE);

	pPD->m_pd.hDevMode = m_hDevMode;
	pPD->m_pd.hDevNames = m_hDevNames;

	DEVMODE * theDM = pPD->GetDevMode();

	theDM->dmFields |= DM_ORIENTATION;
	if (landscape) {
		theDM->dmOrientation = DMORIENT_LANDSCAPE;
	}
	else {
		theDM->dmOrientation = DMORIENT_PORTRAIT;
	}

	pPD->m_pd.Flags |= PD_NOSELECTION; // change the PRINTDLG

	int nResponse = pPD->DoModal();

	// if OK or Cancel is selected we need to update cached devMode/Names
	while (nResponse != IDOK && nResponse != IDCANCEL) {
		switch (::CommDlgExtendedError()) {
			// CommDlg cannot give these errors after NULLing these handles
		case PDERR_PRINTERNOTFOUND:
		case PDERR_DNDMMISMATCH:
			if (pPD->m_pd.hDevNames != NULL) {
				ASSERT(m_hDevNames == pPD->m_pd.hDevNames);

				::GlobalFree(pPD->m_pd.hDevNames);
				pPD->m_pd.hDevNames = NULL;
				m_hDevNames = NULL;
			}

			if (pPD->m_pd.hDevMode) {
				ASSERT(m_hDevMode == pPD->m_pd.hDevMode);

				::GlobalFree(pPD->m_pd.hDevMode);
				pPD->m_pd.hDevMode = NULL;
				m_hDevMode = NULL;
			}
			break;

		default:
			return nResponse;       // do not update cached devMode/Names
		}
		nResponse = pPD->DoModal();
	}

	//FreeResource(hTempDlg);

	m_hDevMode = pPD->m_pd.hDevMode;
	m_hDevNames = pPD->m_pd.hDevNames;

	return nResponse;
}

/***************************************************************************/
/***************************************************************************/
CSaView * CSaApp::GetViewActive() {

	CMainFrame * pwndMainFrame = (CMainFrame *)m_pMainWnd;
	ASSERT(pwndMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));

	CSaView * pview = (pwndMainFrame ?
		pwndMainFrame->GetCurrSaView() :
		NULL);

	return pview;
}

/***************************************************************************/
/***************************************************************************/
CSaView * CSaApp::GetViewTop() {
	return GetViewEnd(GW_HWNDPREV);
}

/***************************************************************************/
/***************************************************************************/
CSaView * CSaApp::GetViewBottom() {
	return GetViewEnd(GW_HWNDNEXT);
}

/***************************************************************************/
/***************************************************************************/
CSaView * CSaApp::GetViewAbove(CSaView * pviewCur) {

	return GetViewNeighbor(pviewCur, GW_HWNDPREV);
}

/***************************************************************************/
/***************************************************************************/
CSaView * CSaApp::GetViewBelow(CSaView * pviewCur) {

	return GetViewNeighbor(pviewCur, GW_HWNDNEXT);
}

/***************************************************************************/
/***************************************************************************/
void CSaApp::SetZ() {

	int z = 0;
	CSaView * pview = GetViewBottom();
	for (; pview; pview = GetViewAbove(pview)) {
		pview->SetZ(z++);
	}
}

/***************************************************************************/
/***************************************************************************/
CSaView * CSaApp::GetViewEnd(UINT uNextOrPrev) {

	CSaView * pview = GetViewActive();
	CSaView * pviewN = (pview ? GetViewNeighbor(pview, uNextOrPrev) : NULL);
	for (; pviewN; pviewN = GetViewNeighbor(pviewN, uNextOrPrev)) {
		pview = pviewN;
	}

	return pview;
}

/***************************************************************************/
/***************************************************************************/
CSaView * CSaApp::GetViewNeighbor(CSaView * pviewCur, UINT uNextOrPrev) {

	ASSERT(pviewCur);
	CMDIChildWnd * pwndChildFrame = pviewCur->pwndChildFrame();

	ASSERT(uNextOrPrev == GW_HWNDNEXT || uNextOrPrev == GW_HWNDPREV);
	CWnd * pwndNext = pwndChildFrame->GetNextWindow(uNextOrPrev);
	for (; pwndNext; pwndNext = pwndNext->GetNextWindow(uNextOrPrev))
		if (pwndNext->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)))
			// NOTE: This test is to skip minimized icon windows,
			// because they are *in addition to* the corresponding
			// MDI child frame window which contains the view object.
		{
			break;
		}

	CSaView * pview = (pwndNext ? CSaView::GetViewActiveChild((CMDIChildWnd *)pwndNext) : NULL);
	return pview;
}

/***************************************************************************/
/***************************************************************************/
static LPCSTR psz_SaApp = "SaApp";
static LPCSTR psz_settingsfile = "sa3.psa";
static LPCSTR psz_batchsettingsfile = "sa batch.psa";
static LPCSTR psz_workbench = "workbench";
static LPCSTR psz_wbonexit = "wbonexit";

/***************************************************************************/
/***************************************************************************/
void CSaApp::WriteProperties(CObjectOStream & obs) {

	// get the version
	CSaString szVersion = GetVersionString();
	CSaString szBuildNum;
	szVersion = szVersion.Right(szVersion.GetLength() - szVersion.Find(' ') - 1);
	int nBuildIndex = szVersion.Find(_T("Build"));
	if (nBuildIndex > 0) {
		szBuildNum = _T(".") + szVersion.Mid(nBuildIndex + 6, szVersion.GetLength() - nBuildIndex - 7);
		szVersion = szVersion.Left(szVersion.Find(' ')) + szBuildNum;
	}

	// The open databases and windows
	SetZ();  // Set the current z-order of all views
	obs.WriteBeginMarker(psz_SaApp, szVersion.utf8().c_str());
	ASSERT(m_pMainWnd);
	((CMainFrame *)m_pMainWnd)->WriteProperties(obs);

	if ((GetBatchMode() == 0) && ((CMainFrame *)m_pMainWnd)->GetSaveOpenFiles()) {
		//tdg 09/03/97
		CDocList doclst; // get list of currently open documents
		for (CSaDoc * pdoc = doclst.pdocFirst(); pdoc; pdoc = doclst.pdocNext()) {
			if (pdoc->GetPathName().GetLength() == 0) {
				continue;
			}
			if (pdoc->IsTempFile()) {
				continue;
			}
			// only write properties for files with paths
			pdoc->WriteProperties(obs);
		}
	}

	((CMainFrame *)m_pMainWnd)->WriteDefaultView(obs);
	obs.WriteString(psz_workbench, m_szWbPath.utf8().c_str());
	obs.WriteBool(psz_wbonexit, m_bWbOpenOnExit);
	CDlgRecorder::GetStaticSourceInfo().WriteProperties(obs);
	obs.WriteEndMarker(psz_SaApp);
}

/***************************************************************************/
// read the open databases and windows
/***************************************************************************/
BOOL CSaApp::ReadProperties(CObjectIStream & obs, bool batchMode) {

	char buffer[1024];
	if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_SaApp, buffer, _countof(buffer))) {
		return FALSE;
	}
	CSaString szLastVersion;
	szLastVersion.setUtf8(buffer);

	ASSERT(m_pMainWnd);

	m_szLastVersion = szLastVersion.IsEmpty() ? _T("2.7") : szLastVersion;

	while (!obs.bAtEnd()) {
		if (((CMainFrame *)m_pMainWnd)->ReadProperties(obs, batchMode));
		else if ((GetBatchMode() == 0) && (CSaDoc::ReadProperties(obs)));
		else if (((CMainFrame *)m_pMainWnd)->ReadDefaultView(obs));
		else if (ReadStreamString(obs, psz_workbench, m_szWbPath));
		else if (obs.bReadBool(psz_wbonexit, m_bWbOpenOnExit));
		else if (CDlgRecorder::GetStaticSourceInfo().ReadProperties(obs));
		else if (obs.bEnd(psz_SaApp)) {
			break;
		}
	}

	// open the workbench
	if ((m_bWbOpenOnExit) && (!m_szWbPath.IsEmpty())) {
		OnWorkbenchOpen();
	}

	// activate top window (not last document)
	if (GetViewTop()) { // only if there is a top window
		GetViewTop()->ShowInitialTopState();
	}

	return TRUE;
}

/***************************************************************************/
/* CSaApp::WriteSettings
*
* The top level writer for SA.  This writes all of the
* window settings for the entire app, including which
* documents are loaded and the state of the views.
*
* returns TRUE only if it succeeds.
*/
/***************************************************************************/
BOOL CSaApp::WriteSettings() {

	BOOL ret = FALSE;

	CSaString szPath = ((CWinApp *)this)->GetProfileString(_T(""), _T("DataLocation"));
	szPath += "\\";

	if (GetBatchMode() != 0) {
		szPath += psz_batchsettingsfile;
	}
	else {
		szPath += psz_settingsfile;
	}

	CObjectOStream obs(szPath.utf8().c_str());

	if (!obs.getIos().fail()) {
		WriteProperties(obs);
		ret = TRUE;
	}

	return ret;
}

/***************************************************************************/
/* CSaApp::ReadSettings
*
* The top level reader for SA.  This reads in all of the
* window settings for the entire app, including which
* documents are loaded and the state of the views.
*
* returns TRUE only if it succeeds.
*
* These settings are set in HKEY_CURRENT_USER/Software/SIL/Speech Analyzer
/***************************************************************************/
BOOL CSaApp::ReadSettings(bool batchMode) {

	BOOL ret = FALSE;

	// get the data location and create it if it doesn't exist
	CSaString szPath = ((CWinApp *)this)->GetProfileString(_T(""), _T("DataLocation"), _T("*Missing*"));
	if (szPath == _T("*Missing*")) {
		m_bNewUser = TRUE;
		szPath.Empty();
	}

	if (szPath.IsEmpty()) {
		// Set the DataLocation path and write it to the registry
		szPath = GetShellFolderPath(CSIDL_PERSONAL);
		if (szPath.Right(1) != "\\") {
			szPath += _T("\\");
		}
		szPath += _T("Speech Analyzer");
		WriteProfileString(_T(""), _T("DataLocation"), szPath);
	}
	if (szPath.Right(1) == "\\") {
		szPath = szPath.Left(szPath.GetLength() - 1);
	}

	if (!FileUtils::FolderExists(szPath)) {
		// it doesn't exist...
		if (!FileUtils::CreateFolder(szPath)) {
			// and we can't create it.
			CString error = FormatGetLastError(GetLastError());
			CString msg;
			AfxFormatString2(msg, IDS_ERROR_NO_SETTING_DIR, szPath, error);
			::AfxMessageBox(msg, MB_OK | MB_ICONEXCLAMATION, 0);
			return FALSE;
		}
	}

	szPath += "\\";

	if (GetBatchMode() != 0) {
		szPath += psz_batchsettingsfile;
	}
	else {
		szPath += psz_settingsfile;
	}

	CObjectIStream obs(szPath.utf8().c_str());
	if (!obs.bFail()) {
		ret = ReadProperties(obs,batchMode);
	}
	return ret;
}

/***************************************************************************/
/***************************************************************************/
CSaString CSaApp::GetStartupMessage(CSaString szLastVersion) {
	CSaString msg;
	if ((szLastVersion[0] < '2') || (szLastVersion[0] > '9')) {
		szLastVersion = "2.7";
	}
	CSaString szRelease = szLastVersion.Left(3);
	CSaString szBuild = szLastVersion.Right(szLastVersion.GetLength() - 4);
	if (szRelease == "2.7") {
		msg.LoadString(IDS_STARTUP_3_0);
	}
	return msg;
}

/////////////////////////////////////////////////////////////////////////////
// CSaApp::PrepareForNewUser
//
// Perform actions required to prepare for a new user, such as copying
// samples.
/////////////////////////////////////////////////////////////////////////////
void CSaApp::SetupNewUser() {

	// get the data and app locations
	CSaString szDataLocation = ((CWinApp *)this)->GetProfileString(_T(""), _T("DataLocation"), _T("Missing"));
	CSaString szAppLocation = m_pszHelpFilePath;
	if (szDataLocation.IsEmpty() || szAppLocation.IsEmpty()) {
		return;
	}
	szAppLocation = szAppLocation.Left(szAppLocation.ReverseFind(_T('\\')));

	// create new user's samples folders
	CSaString szSamplesLocation;
	CSaString szMusicLocation;
	szSamplesLocation = szDataLocation + _T("\\Samples");
	szMusicLocation = szSamplesLocation + _T("\\Music");
	FileUtils::CreateFolder(szSamplesLocation);
	FileUtils::CreateFolder(szMusicLocation);

	// copy the samples into the samples folders
	CSaString szSearchPattern;
	CSaString szSourcePath;
	CSaString szDestPath;
	CSaString subFolders[2] = { _T("\\Samples\\"), _T("\\Samples\\Music\\") };

	for (int i = 0; i < 2; i++) {
		// initialize the search
		WIN32_FIND_DATA FileData;
		HANDLE hSearch;
		BOOL bFinished = FALSE;
		szSearchPattern = szAppLocation + subFolders[i] + _T("*.*");
		hSearch = FindFirstFile(szSearchPattern, &FileData);
		if (hSearch == INVALID_HANDLE_VALUE) {
			return;
		}

		// copy each file to the new directory
		while (!bFinished) {
			szSourcePath = szAppLocation + subFolders[i] + FileData.cFileName;
			szDestPath = szDataLocation + subFolders[i] + FileData.cFileName;
			CopyFile(szSourcePath, szDestPath, FALSE);

			if (!FindNextFile(hSearch, &FileData) && (GetLastError() == ERROR_NO_MORE_FILES)) {
				bFinished = TRUE;
			}
		}
		FindClose(hSearch);
	}
}

/***************************************************************************/
/***************************************************************************/

CDocument * CSaApp::OpenDocumentFile(LPCTSTR lpszFileName) {
	CSaString szPrettyName = lpszFileName;
	LPTSTR pszPretty = szPrettyName.GetBuffer(_MAX_PATH);
	GetLongPathNameW(pszPretty, pszPretty, _MAX_PATH);
	return CWinApp::OpenDocumentFile(pszPretty);
}

BOOL CSaApp::m_bUseUnicodeEncoding = FALSE;

// return application mode (batch or not, exit allowed)
int CSaApp::GetBatchMode() {
	return m_nBatchMode;
}

// allow SA to exit
void CSaApp::CancelBatchMode() {
	if (m_nBatchMode != 0) {
		m_nBatchMode = 3;
	}
}

CString CSaApp::GetMRUFilePath(int i) const {
	CString result;
	// no entries, need to load from registry, this is the MRU list
	if (!m_pRecentFileList->GetSize()) {
		m_pRecentFileList->ReadList();
	}
	if (i < m_pRecentFileList->GetSize()) {
		result = (*m_pRecentFileList)[i];
	}
	return result;
}

bool CSaApp::IsSAServer() const {
	return (CSaString(m_pszExeName).CompareNoCase(L"saserver") == 0);
}

// Name:        CSingleInstanceData
// Type:        Constructor
// Description: Create shared memory mapped file or create view of it
CSingleInstanceData::CSingleInstanceData(LPCTSTR aName) {
	// Build names
	CString lFileName = aName;
	lFileName += _T("-Data-Mapping-File");

	CString lMutexName = aName;
	lMutexName += _T("-Data-Mapping-Mutex");

	// Create mutex, global scope
	mMutex = new CMutex(FALSE, lMutexName);

	// Create file mapping
	mMap = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, sizeof(TCHAR) * MAX_DATA, lFileName);
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		// Close handle
		CloseHandle(mMap);
		// Open existing file mapping
		mMap = OpenFileMapping(FILE_MAP_WRITE, FALSE, lFileName);
	}

	// Set up data mapping
	mData = (LPTSTR)MapViewOfFile(mMap, FILE_MAP_WRITE, 0, 0, sizeof(TCHAR) * MAX_DATA);

	// Lock file
	CSingleLock lock(mMutex, TRUE);
	if (lock.IsLocked()) {
		// Clear data
		ZeroMemory(mData, sizeof(TCHAR) * MAX_DATA);
	}
}

// Name:        ~CSingleInstanceData
// Type:        Destructor
// Description: Close memory mapped file
CSingleInstanceData::~CSingleInstanceData() {

	if (mMap != NULL) {
		// Unmap data from file
		UnmapViewOfFile(mData);
		// Close file
		CloseHandle(mMap);
	}

	// Clean up mutex
	if (mMutex != NULL) {
		delete mMutex;
		mMutex = NULL;
	}
}

// Name:        SetValue
// Type:        Function
// Description: Set value in memory mapped file
void CSingleInstanceData::SetValue(LPCTSTR aData) {

	// Lock file
	CSingleLock lock(mMutex, TRUE);
	if (lock.IsLocked()) {
		// Check data length, prevent buffer over run
		if (_tcslen(aData) < MAX_DATA) {
			// Copy data
			wcscpy_s(mData, MAX_DATA, aData);
		}
	}
}

// Name:        GetValue
// Type:        Function
// Description: Get value from memory mapped file
CString CSingleInstanceData::GetValue() const {

	// Lock file
	CSingleLock lock(mMutex, TRUE);
	if (lock.IsLocked()) {
		// Return the data
		return mData;
	}
	// Not locked to return empty data
	return _T("");
}

// Name:        Create
// Type:        Function
// Description: Pass message to implementor
// return TRUE if this app instance is the singleton
// return FALSE if another app instance is the singleton
// return TRUE on failure
BOOL CSaApp::CreateAsSingleton(LPCTSTR aName) {

	// evaluate the command line to see if we are running 'normally'
	// we don't support single instance in batch mode
	if (CheckForBatchMode(m_lpCmdLine) != 0) {
		TRACE("SingleInstance not supported in batch mode\n");
		return TRUE;
	}
	if (IsSAServer()) {
		TRACE("SingleInstance not supported in server mode\n");
		return TRUE;
	}

	// Create shared data
	mData = new CSingleInstanceData(aName);
	if (mData == NULL) {
		TRACE("Unable to create shared data object\n");
		AfxMessageBox(IDS_ERROR_SINGLETON, MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}

	// Create event name
	CString lEventName = aName;
	lEventName += _T("-Event");

	// Create named event, global scope
	mEvent = new CEvent(FALSE, FALSE, lEventName);
	if (mEvent == NULL) {
		delete mData;
		mData = NULL;
		TRACE("Unable to create global event\n");
		AfxMessageBox(IDS_ERROR_SINGLETON, MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}

	// Check we have a handle to a valid event
	// Check last error status
	DWORD lLastError = GetLastError();
	if (lLastError == ERROR_ALREADY_EXISTS) {
		// Set command line data
		mData->SetValue(GetCommandLine());
		// Not our event, so an instance is already running, signal thread in other instance to wake up
		if (!mEvent->SetEvent()) {
			// we couldn't notify the other application
			delete mData;
			mData = NULL;
			delete mEvent;
			mEvent = NULL;
			TRACE("Unable to signal singleton instance\n");
			AfxMessageBox(IDS_ERROR_SINGLETON, MB_OK | MB_ICONEXCLAMATION);
			return TRUE;
		}

		delete mData;
		mData = NULL;
		delete mEvent;
		mEvent = NULL;
		// another application is the singleton
		return FALSE;
	}

	// we are the first process - setup a mechanism to wait for other processes
	// Create event of thread syncronization, nameless local scope
	mSignal = new CEvent();
	if (mSignal == NULL) {
		delete mData;
		mData = NULL;
		delete mEvent;
		mEvent = NULL;
		TRACE("Unable to create Signal event\n");
		AfxMessageBox(IDS_ERROR_SINGLETON, MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}

	// Create thread
	AfxBeginThread((AFX_THREADPROC)ActivationThread, (LPVOID)this);
	return true;
}

// Name:        WakeUp
// Type:        Callback function
// Description: Default action, find main application window and make foreground.
// We send a message because we are on a thread and we need to get on the
// message loops thread.  If we don't we will found some resources are not valid.
void CSaApp::WakeUp(LPCTSTR aCommandLine) {

	// Find application and main window
	CWinApp * pApp = AfxGetApp();
	if (pApp == NULL) {
		TRACE("Unable to retrieve application pointer\n");
		return;
	}
	if (pApp->m_pMainWnd == NULL) {
		TRACE("Unable to retrieve application main window\n");
		return;
	}

	// Make main window foreground, flashy, flashy time
	pApp->m_pMainWnd->SetForegroundWindow();
	// display the window if it's minimized
	pApp->m_pMainWnd->ShowWindow(SW_MAXIMIZE);

	// we are expecting only one filename at this point
	// parse the command line
	LPWSTR * szArglist = NULL;
	int nArgs = 0;
	szArglist = CommandLineToArgvW(aCommandLine, &nArgs);
	if (NULL == szArglist) {
		TRACE(L"Unable to parse command line\n");
		return;
	}
	if (nArgs != 2) {
		TRACE("Unexpected number of arguments in commandline\n");
		return;
	}

	wchar_t buffer[512];
	memset(buffer, 0, _countof(buffer) * 2);
	wcscpy_s(buffer, _countof(buffer), szArglist[1]);
	LocalFree(szArglist);

	GetLongPathNameW(buffer, buffer, _MAX_PATH);

	// structure holding data information.
	COPYDATASTRUCT stCopyData = { 0 };
	stCopyData.lpData = (void *)buffer;
	stCopyData.cbData = wcslen(buffer) * 2;

	// Send the data.
	HWND me = pApp->m_pMainWnd->GetSafeHwnd();
	::SendMessage(me, WM_COPYDATA, (UINT)me, (ULONG)&stCopyData);
}

// Name:        Sleeper
// Type:        Thread function
// Description: Sleep on events, wake and activate application or wake and quit.
UINT CSaApp::ActivationThread(CSaApp * pOwner) {
	TRACE("Starting ActivationThread\n");
	// Build event handle array
	CSyncObject * lEvents[] = {
		pOwner->mEvent,
		pOwner->mSignal
	};

	// Forever
	BOOL lForever = TRUE;
	while (lForever) {
		CMultiLock lock(lEvents, 2);
		// Goto sleep until one of the events signals, zero CPU overhead
		DWORD lResult = lock.Lock(INFINITE, FALSE);
		// What signaled, 0 = event, another instance started
		if (lResult == WAIT_OBJECT_0 + 0) {
			if (pOwner != NULL) {
				// Wake up the owner with the data (last command line)
				pOwner->WakeUp(pOwner->mData->GetValue());
			}
		}
		// 1 = signal, time to exit the thread
		else if (lResult == WAIT_OBJECT_0 + 1) {
			// Break the forever loop
			lForever = FALSE;
		}
		lock.Unlock();
	}

	// Set event to say thread is exiting
	pOwner->mEvent->SetEvent();
	return 0;
}

void CSaApp::InitSingleton() {
	// single instance handling members
	mEvent = NULL;
	mSignal = NULL;
	mData = NULL;
}

void CSaApp::DestroySingleton() {

	// If event and signal exist
	if ((mEvent != NULL) && (mSignal != NULL)) {
		// Set signal event to allow thread to exit
		if (mSignal->SetEvent()) {
			// Wait for thread to start exiting
			CSingleLock lock(mEvent, FALSE);
			lock.Lock();
		}

		// Close all open handles
		delete mEvent;
		mEvent = NULL;
		delete mSignal;
		mSignal = NULL;
	}

	if (mData != NULL) {
		delete mData;
		mData = NULL;
	}
}

void CSaApp::WorkbenchClosed() {
	// signal, that workbench has been closed
	m_pWbDoc = NULL;
}

CSaString * CSaApp::GetWorkbenchPath() {
	// returns a pointer to the workbench pathname
	return &m_szWbPath;
}
BOOL CSaApp::IsCreatingNewFile() {
	// return TRUE if file new operation running
	return m_bNewDocument;
}
CDocument * CSaApp::GetWbDoc() {
	// return pointer to workbench document
	return m_pWbDoc;
}
void CSaApp::SetWbOpenOnExit(BOOL value) {
	m_bWbOpenOnExit = value;
}
UINT CSaApp::GetOpenAsID() {
	// return m_OpenAsID
	return m_OpenAsID;
}
void CSaApp::SetOpenAsID(UINT value) {
	// set m_OpenAsID
	m_OpenAsID = value;
}
bool CSaApp::GetOpenMore() {
	return m_bOpenMore;
}
void CSaApp::SetOpenMore(bool val) {
	m_bOpenMore = val;
}
void CSaApp::SetLastClipboardPath(LPCTSTR value) {
	m_szLastClipboardPath = value;
}

LPCTSTR CSaApp::GetLastClipboardPath() {
	return m_szLastClipboardPath;
}

typedef HMODULE(__stdcall * SHGETFOLDERPATH)(HWND, int, HANDLE, DWORD, LPTSTR);

CSaString GetShellFolderPath(DWORD csidl) {

	// select a directory to restore the file to.
	TCHAR documentPath[_MAX_PATH];
	wmemset(documentPath, 0, _countof(documentPath));
	HMODULE hModule = LoadLibrary(_T("SHFOLDER.DLL"));
	if (hModule != NULL) {
		SHGETFOLDERPATH fnShGetFolderPath = (SHGETFOLDERPATH)GetProcAddress(hModule, "SHGetFolderPathW");
		if (fnShGetFolderPath != NULL) {
			fnShGetFolderPath(NULL, csidl, NULL, 0, documentPath);
		}
		FreeLibrary(hModule);
	}
	return CSaString(documentPath);
}

CString CSaApp::GetVersionString() {

	CString result;

	// Is Library Compatible
	wchar_t szApp[MAX_PATH * 4];
	wmemset(szApp, 0, _countof(szApp));
	GetModuleFileName(AfxGetInstanceHandle(), szApp, _countof(szApp));

	// now read the local application version.
	DWORD dwHandle = 0;
	DWORD dwSize = GetFileVersionInfoSize(szApp, &dwHandle);
	if (!dwSize) {
		return result;
	}

	array_ptr<BYTE> pAppVersion(dwSize);
	if (!GetFileVersionInfo(szApp, dwHandle, dwSize, pAppVersion.get())) {
		return result;
	}

	UINT nLen = 0;
	VS_FIXEDFILEINFO * pAppVS = NULL;
	if (!VerQueryValue(pAppVersion.get(), _T("\\"), (void **)&pAppVS, &nLen)) {
		return result;
	}

	DWORD major1 = (pAppVS->dwFileVersionMS & 0xffff0000) >> 16;
	DWORD major2 = pAppVS->dwFileVersionMS & 0xffff;
	DWORD minor3 = (pAppVS->dwFileVersionLS & 0xffff0000) >> 16;
	DWORD minor4 = pAppVS->dwFileVersionLS & 0xffff;

	result.Format(L"Version %d.%d.%d.%d", major1, major2, minor3, minor4);
	return result;
}

CVowelFormantSets& CSaApp::GetVowelSets() {
	CString szPath(AfxGetApp()->GetProfileString(_T(""), _T("DataLocation")));
	if (szPath.Right(1) != _T("\\")) {
		szPath += "\\";
	}
	szPath = szPath + "vowelsUtf8.psa";
	static CVowelFormantSets theVowelSets(*this, _to_utf8(szPath.GetString()));
	return theVowelSets;
}

const CVowelFormantSet& CSaApp::GetDefaultVowelSet() {
	CVowelFormantSets& cSets = GetVowelSets();
	return cSets[cSets.GetDefaultSet()];
}

const CVowelFormantsVector& CSaApp::GetVowelVector(int nGender) {
	return GetDefaultVowelSet().GetVowelFormants(nGender);
}

SRange CSaApp::Get3DChartRange(int nFormant, int nGender) {
	return CPlot3D::GetChartRange(nFormant, nGender);
}

// uses IDS_ERROR_GRAPPLSPACE
void CSaApp::GrapplErrorMessage(LPCTSTR pszText1, LPCTSTR pszText2) {
	ErrorMessage(IDS_ERROR_GRAPPLSPACE, pszText1, pszText2);
}

int CSaApp::AfxMessageBox(UINT nIDPrompt, UINT nType, UINT nIDHelp) {
	return ::AfxMessageBox(nIDPrompt, nType, nIDHelp);
}
