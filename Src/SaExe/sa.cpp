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
#include "playerRecorder.h"
#include "Process\sa_proc.h"
#include "Process\sa_p_gra.h"
#include "Process\sa_p_fra.h"
#include "sa_segm.h"

#include "sa_doc.h"
#include "sa_w_doc.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "settings\obstream.h"
#include "doclist.h"
using std::ifstream;
using std::ofstream;
using std::ios;

#include "sa_exprt.h"
#include "sa_dplot.h"
#include "sa_start.h"
#include <shellapi.h>
#include "fileOpen.h"
#include <windows.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef HMODULE  (__stdcall *SHGETFOLDERPATH)(HWND, int, HANDLE, DWORD, LPTSTR);
#define CSIDL_PERSONAL                  0x0005        // My Documents
#define CSIDL_FLAG_CREATE               0x8000        // combine with CSIDL_ value to force folder creation in SHGetFolderPath()

struct VS_VERSIONINFO
{
    WORD                wLength;
    WORD                wValueLength;
    WORD                wType;
    WCHAR               szKey[1];
    WORD                wPadding1[1];
    VS_FIXEDFILEINFO    Value;
    WORD                wPadding2[1];
    WORD                wChildren[1];
};

//###########################################################################
// CSaApp

/////////////////////////////////////////////////////////////////////////////
// CSaApp message map

BEGIN_MESSAGE_MAP(CSaApp, CSaAppBase)
    //{{AFX_MSG_MAP(CSaApp)
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
	//}}AFX_MSG_MAP
    // Standard file based document commands
    // Standard print setup command
    ON_COMMAND(ID_FILE_PRINT_SETUP, CSaAppBase::OnFilePrintSetup)
    ON_UPDATE_COMMAND_UI_RANGE(ID_FILE_MRU_FIRST, ID_FILE_MRU_LAST, OnUpdateRecentFileMenu)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSaApp construction/destruction/creation

/***************************************************************************/
// CSaApp::CSaApp Constructor
/***************************************************************************/
CSaApp::CSaApp()
{
  m_nBatchMode = FALSE; // no batch mode
  m_bModified = FALSE;
  m_nEntry = 0;         // reset number of entries in batch list file
  m_nCommand =-1; // SDM 1.5Test8.5 (not processing batch commands)
  m_bNewDocument = FALSE;
  m_pWbDoc = NULL;
  m_szWbPath.Empty();
  m_bWbOpenOnExit = FALSE;
	m_bNewUser = FALSE;

  m_OpenAsID = ID_FILE_OPEN;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSaApp object
CSaApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CSaApp helper functions

HMODULE LoadCompatibleLibrary(LPCTSTR szCName)
{
  // Is Library Compatible
  CSaString szApp;
  
  GetModuleFileName(AfxGetInstanceHandle(), szApp.GetBuffer(MAX_PATH*4), MAX_PATH*4);
  szApp.ReleaseBuffer();

  CSaString szName = szCName;

  int nFind = szApp.ReverseFind('\\');
  if(nFind != -1)
  {
    szName = szApp.Left(nFind+1) + szCName;
  }

  DWORD dwHandle;

  DWORD dwSize = GetFileVersionInfoSize(szName.GetBuffer(0), &dwHandle);

  if(!dwSize)
    return 0;
  
  void* pLibVersion = (void*) new char[dwSize];
  
  if(!GetFileVersionInfo(szName.GetBuffer(0), dwHandle, dwSize, pLibVersion))
  {
    delete [] pLibVersion;
    return 0;
  }

  VS_FIXEDFILEINFO *pLibVS = NULL;
  unsigned int nLen;
  if(!VerQueryValue(pLibVersion, _T("\\"), (void**) &pLibVS, &nLen))
  {
    delete [] pLibVersion;
    return 0;
  }

  dwSize = GetFileVersionInfoSize(szApp.GetBuffer(0), &dwHandle);

  if(!dwSize)
    return 0;
  
  void* pAppVersion = (void*) new char[dwSize];
  
  if(!GetFileVersionInfo(szApp.GetBuffer(0), dwHandle, dwSize, pAppVersion))
  {
    delete [] pLibVersion;
    delete [] pAppVersion;
    return 0;
  }

  VS_FIXEDFILEINFO *pAppVS = NULL;
  if(!VerQueryValue(pAppVersion, _T("\\"), (void**) &pAppVS, &nLen))
  {
    delete [] pLibVersion;
    delete [] pAppVersion;
    return 0;
  }

  if(pAppVS->dwFileVersionLS != pLibVS->dwFileVersionLS ||
    pAppVS->dwFileVersionMS != pLibVS->dwFileVersionMS || 
    pAppVS->dwProductVersionLS != pLibVS->dwProductVersionLS || 
    pAppVS->dwProductVersionMS != pLibVS->dwProductVersionMS || 
    pAppVS->dwFileFlags != pLibVS->dwFileFlags)
  {
    CSaString szMessage;
    
    szMessage.FormatMessage(_T("%1 contains resources from incompatible version, unloading"), szCName);
    AfxMessageBox(szMessage);
    delete [] pLibVersion;
    delete [] pAppVersion;
    return 0;
  }

  delete [] pLibVersion;
  delete [] pAppVersion;

  return LoadLibrary(szName);
}

/***************************************************************************/
// CSaApp::InitInstance Initialisation of the application
// Called by framework to initially create the application.
/***************************************************************************/
BOOL CSaApp::InitInstance()
{
#ifdef DEBUG_MEMORY_OVERWRITE
  afxMemDF |= checkAlwaysMemDF;  // check for memory overwrites in debug version (see MSDN)
#endif

#ifdef _DEBUG
  m_hEnglishResources = LoadCompatibleLibrary(_T("SA_ENUd.DLL"));
  m_hLocalizedResources = LoadCompatibleLibrary(_T("SA_LOCd.DLL"));
#else
  m_hEnglishResources = LoadCompatibleLibrary(_T("SA_ENU.DLL"));
  m_hLocalizedResources = LoadCompatibleLibrary(_T("SA_LOCAL.DLL"));
#endif

  if(!m_hEnglishResources && !m_hLocalizedResources)
  {
    AfxMessageBox(_T("No resources found, exiting"));
    return FALSE;
  }

  BOOL bInit = CSaAppBase::InitInstance();

  if (bInit)
  {

    AfxEnableControlContainer();
#ifdef _AFXDLL
    Enable3dControls();			// Call this when using MFC in a shared DLL
#else
    Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
    
    SetRegistryKey(_T("SIL"));
    
    m_pszErrors = new CStringArray; // create the error message string array
    // Standard initialization
    // If you are not using these features and wish to reduce the size
    // of your final executable, you should remove from the following
    // the specific initialization routines you do not need.
    
#ifdef _DEBUG
    //    afxTraceEnabled = 1;
    //    afxTraceFlags = 1;
#endif
    
    // Set dialog background color
    SetDialogBkColor(GetSysColor(COLOR_BTNFACE));
    // Load standard INI file options (including MRU)
    LoadStdProfileSettings(8);
    
    AfxOleInit();

    
    // Register the application's document templates. Document templates
    // serve as the connection between documents, frame windows and views.
    m_pDocTemplate = new CMultiDocTemplate(IDR_SA_ANNTYPE,
      RUNTIME_CLASS(CSaDoc),
      RUNTIME_CLASS(CChildFrame),
      RUNTIME_CLASS(CSaView));
    AddDocTemplate(m_pDocTemplate);
    // add workbench template
    AddDocTemplate(new CMultiDocTemplate(IDR_SA_WBTYPE,
      RUNTIME_CLASS(CWorkbenchDoc),
      RUNTIME_CLASS(CMDIChildWnd),
      RUNTIME_CLASS(CSaWorkbenchView)));
    
    // underlying functionality not implemented we can't add this template SDM
    // add MIDI template
    //AddDocTemplate(new CMultiDocTemplate(IDR_SA_MIDTYPE,
    //                                     RUNTIME_CLASS(CMidiDoc),
    //                                     RUNTIME_CLASS(CMDIChildWnd),
    //                                     RUNTIME_CLASS(CMidiView)));
    
    // create main MDI Frame window
    CMainFrame* pMainFrame = new CMainFrame;
    if(CSaString(m_pszExeName).Find(_T("SAS")) == -1)
    {
      if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
      {
        return FALSE;
      }
    }
    else if (!pMainFrame->LoadFrame(IDR_MAINFRAME_SAS))
    {
      return FALSE;
    }
    
    m_pMainWnd = pMainFrame;
    // check if SA runs in batchmode
    m_nBatchMode = CheckForBatchMode(m_lpCmdLine);

		// update help file path
		CSaString szNewPath = m_pszHelpFilePath;
		szNewPath = szNewPath.Left(szNewPath.ReverseFind('\\')) + _T("\\Speech_Analyzer_Help.chm");
		free((void*)m_pszHelpFilePath);
		m_pszHelpFilePath = _tcsdup(szNewPath);
    
    // check if SM (Speech Manager) is up
	/* Check is no longer necessary, taken care of through file ownership  -ALB
    if (pMainFrame->IsSMrunning())
    {
      // SM is up, if no batch mode inform user and cancel startup
      if (!GetBatchMode())
      {
        AfxMessageBox(IDS_ERROR_SMRUNNING,MB_OK,0);
        // Clean up these do not automatically get deleted in this abnormal termination SDM 1.06.8
        delete m_pMainWnd;
        m_pMainWnd = 0;
        return FALSE;
      }
    }
	*/

    if(CSaString(m_pszExeName).Find(_T("SAS")) != -1)
    {
      if (!GetBatchMode())
      {
        AfxMessageBox(IDS_ERROR_SAS,MB_OK,0);
        delete m_pMainWnd;
        m_pMainWnd = 0;
        return FALSE;
      }
    }
    
		CTime splashStartTime; // keep track of when we first showed the splash screen
		ISplashScreenPtr splash;
		CSaString szSplashText;

		// create splash window only if SA not in batchmode
    if (!GetBatchMode())
    {
			// display splash screen
			CoInitialize(NULL);
			HRESULT createResult = splash.CreateInstance(__uuidof(SplashScreen));
			if (createResult)
			{
				CSaString szCreateResult;
				szCreateResult.Format(_T("%x"), createResult);
				ErrorMessage(IDS_ERROR_CREATE_INSTANCE, _T("SplashScreen.CreateInstance()"), szCreateResult);
				return FALSE;
			}

			splash->Show();
			szSplashText.LoadString(IDS_SPLASH_LOADING);
			splash->Message = (_bstr_t)szSplashText;
			szSplashText.LoadString(IDR_MAINFRAME);
			splash->ProdName = (_bstr_t)szSplashText;
			// load version info
			CSaString szVersion((LPCTSTR)VS_VERSION);
			szVersion = szVersion.Right(szVersion.GetLength() - szVersion.Find(' ') - 1);
			// Beta version display
			int nBuildIndex = szVersion.Find(_T("Build"));
			if (nBuildIndex > 0)
				szVersion = szVersion.Left(nBuildIndex - 2);
			// RC version display
			int nRCIndex = szVersion.Find(_T("RC"));
			if (nRCIndex > 0)
				szVersion = szVersion.Left(nRCIndex - 1);
			splash->ProdVersion = (_bstr_t)szVersion;
			// load version info
			CSaString szCopyright((LPCTSTR)VS_COPYRIGHT);
			splash->Copyright = (_bstr_t)szCopyright;
			splash->Activate();
			splashStartTime = CTime::GetCurrentTime();
    }
    // enable file manager drag/drop
    m_pMainWnd->DragAcceptFiles();
    
    m_szLastVersion = _T("2.7");
	if (!GetBatchMode())
    {
      // RegisterShellFileTypes(FALSE); 
      EnableShellOpen();
      // Parse command line for standard shell commands, DDE, file open
      CCommandLineInfo cmdInfo;
      ParseCommandLine(cmdInfo);

      BOOL bSettingSuccess = ReadSettings();
      
      // Dispatch commands specified on the command line
      if (cmdInfo.m_nShellCommand != CCommandLineInfo::FileNew && !ProcessShellCommand(cmdInfo))
        return FALSE;

      if(!bSettingSuccess)
        pMainFrame->ShowWindow(m_nCmdShow);
      pMainFrame->UpdateWindow();

			// Perform setup new user, if needed
			if (m_bNewUser)
			{
				szSplashText.LoadString(IDS_SPLASH_NEW_USER_SETUP);
				splash->Message = (_bstr_t)szSplashText;
				SetupNewUser();
			}

			splash->Close();
			splash->Release();
			splash = NULL;
			CoUninitialize();

			CSaString msg = GetStartupMessage(m_szLastVersion);
			if (msg.GetLength())
				AfxMessageBox(msg);

			// Show startup dialog
			CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
			if (pMainWnd->GetShowStartupDlg() && !pMainWnd->GetCurrSaView())
				ShowStartupDialog(TRUE);
    }
    else
    {
      if (!ReadSettings())
      {
        // settings read failed.  at least show the window correctly.
        pMainFrame->ShowWindow(m_nCmdShow + 3);
      }
    
      // SDM 1.06.8 window size in this function needs to take precedence over save settings
      // examine command line
      ExamineCmdLine(m_lpCmdLine);
      pMainFrame->UpdateWindow();
    }
  }

	return bInit;
}

/***************************************************************************/
// CSaApp::ExitInstance Application exit
// Called by framework to exit the application.
/***************************************************************************/
int CSaApp::ExitInstance()
{
  // delete the temp transcription DB
	CoInitialize(NULL);
	ISaAudioDocumentWriterPtr saAudioDocWriter;
	saAudioDocWriter.CreateInstance(__uuidof(SaAudioDocumentWriter));
	saAudioDocWriter->DeleteTempDB();
	saAudioDocWriter->Close();
	saAudioDocWriter->Release();
	saAudioDocWriter = NULL;
	CoUninitialize();

	if(m_hEnglishResources)
    FreeLibrary(m_hEnglishResources);
  if(m_hLocalizedResources)
    FreeLibrary(m_hLocalizedResources);

  BOOL bOK = FALSE;
  try
  {
    if (m_pszErrors)
    {
      delete m_pszErrors;
    }
    bOK = CSaAppBase::ExitInstance();
    
    // standard implementation: save initialisation
    SaveStdProfileSettings();
  }
  catch(...)
  {
  }
  // standard implementation: save initialisation
  if (bOK)
    return m_msgCur.wParam; // Returns the value from \QuitMessage
  else
    return bOK;
}

/***************************************************************************/
// CSaApp::CheckForBatchMode Check if SA runs in batch mode
// The function returns TRUE, if SA should run in batch mode.
/***************************************************************************/
BOOL CSaApp::CheckForBatchMode(LPTSTR pCmdLine)
{
  if ((pCmdLine[0] == '-') && ((pCmdLine[1] == 'L') || (pCmdLine[1] == 'l')
      || (pCmdLine[1] == 'R') || (pCmdLine[1] == 'r')))
  {
    return TRUE;
  }
  else return FALSE;
}

CSaString CSaApp::GetBatchString(LPCTSTR lpSection, LPCTSTR lpKey, LPCTSTR lpDefault)
{
  TCHAR lpBuffer[_MAX_PATH];

  lpBuffer[0] = 0;

  GetPrivateProfileString(lpSection, lpKey, lpDefault, lpBuffer, _MAX_PATH, m_szCmdFileName);
  
  return CSaString(lpBuffer);
}

BOOL CSaApp::WriteBatchString(LPCTSTR lpSection, LPCTSTR lpKey, LPCTSTR lpValue)
{
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
void CSaApp::ExamineCmdLine(LPCTSTR pCmdLine, WPARAM wParam)
{
  if (pCmdLine[0] != '\0')
  {
    // AfxMessageBox(pCmdLine);
    // ASSERT(FALSE);  // Assert to allow debuging of the interface
    CSaString szCmdLine = pCmdLine; // copy string into CSaString object
    szCmdLine.MakeUpper(); // convert the whole string to upper case letters
    DWORD dwStart = 0;
    DWORD dwStop = 0;
    if (GetBatchMode())
    {
      // cut out the intro
      if (szCmdLine.Left(2) == "-L")
      {
        wParam = SPEECH_WPARAM_SHOWSA;
        szCmdLine = szCmdLine.Right(szCmdLine.GetLength() - 2); // SDM 1.5Test8.3
      }
      else if (szCmdLine.Left(2) == "-R") // SDM 1.5Test10.4
      {
        wParam = SPEECH_WPARAM_SHOWSAREC;
        szCmdLine = szCmdLine.Right(szCmdLine.GetLength() - 2); // SDM 1.5Test8.3
      }
      else // SDM 1.5Test10.4
      {
        szCmdLine = " " + szCmdLine; // prepend space to allow _stscanf to suceed
      }
      
      // sa has to read list file and open documents // SDM 1.5Test8.3
      _stscanf(szCmdLine, _T("%*[ 0123456789]%[^\n]"), m_szCmdFileName.GetBuffer(szCmdLine.GetLength()));
      m_szCmdFileName.ReleaseBuffer();
      
      
      CFileStatus TheStatus;
      if ((m_szCmdFileName.GetLength()==0) || !CFile::GetStatus(m_szCmdFileName,TheStatus))
      {
        // The file does not exist use original profile
        ASSERT(wParam == SPEECH_WPARAM_SHOWSAREC);
      }
      
      m_szCallingApp = GetBatchString(_T("Settings"), _T("CallingApp"), SM_CAPTION); // get the entry // SDM 1.5Test8.5
      
      // SDM 1.5Test8.3
      CSaString szString = GetBatchString(_T("Settings"), _T("ShowWindow"), _T("")); // get the entry // SDM 1.5Test8.5
      CSaString szReturn = szString;// SDM 1.5Test10.0
      CSaString szParam;
      _stscanf(szString,_T("%16[^(]%(%20[^)]"),szReturn.GetBuffer(szReturn.GetLength()),szParam.GetBuffer(szReturn.GetLength()));// SDM 1.5Test10.0
      szReturn.ReleaseBuffer();
      szParam.ReleaseBuffer();
      szReturn.MakeUpper(); // convert the whole string to upper case letters
      szReturn = szReturn.Mid(szReturn.SpanIncluding(_T(" ")).GetLength());  // Remove leading spaces
      szReturn += "        ";  // Pad with spaces
      szReturn = szReturn.Left(8);  //Take exactly 8 characters
      
      TRACE(_T("Settings:ShowWindow=%s\n"),szReturn);
      
      if (szReturn == "HIDE    ")
      {
        m_pMainWnd->ShowWindow(SW_HIDE);
        m_nCmdShow = SW_HIDE; // to prevent MFC to restore on startup
      }
      else if (szReturn == "SHOW    ")
      {
        m_pMainWnd->ShowWindow(SW_SHOW);
      }
      else if (szReturn == "RESTORE ")
      {
        m_pMainWnd->ShowWindow(SW_RESTORE);
      }
      else if (szReturn == "MAXIMIZE")
      {
        m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED); // maximize the mainframe
        m_nCmdShow = SW_HIDE; // to prevent MFC to restore on startup
      }
      else if (szReturn == "MINIMIZE")
      {
        m_pMainWnd->ShowWindow(SW_MINIMIZE);
        m_nCmdShow = SW_MINIMIZE; // to prevent MFC to restore on startup
      }
      else if (szReturn == "NONE    ")
      {
        ; // do nothing
      }
      else if (szReturn == "SIZE    ") // SDM 1.5Test8.3
      {
        // set mainframe position and show it
        int left,top,width,height;
        if (_stscanf(szParam,_T("%d%,%d%,%d%,%d"),&left,&top, &width, &height) == 4)
        {
          m_pMainWnd->ShowWindow(SW_RESTORE);// SDM 1.5Test10.0
          m_pMainWnd->SetWindowPos(NULL,left, top, width, height,SWP_SHOWWINDOW|SWP_NOZORDER);// SDM 1.5Test10.0
        }
        else
        {
          TRACE(_T("Incorrect Size Parameters\n"));// SDM 1.5Test10.0
          m_pMainWnd->ShowWindow(SW_SHOW);
        }
      }
#ifdef FallBackToCmdLineSize // SDM 1.5Test10.0
      else // Use cmd line size information // SDM 1.5Test8.3
      {
        CSaString szPosition;
        if (_stscanf(szCmdLine," %16[0123456789]", szPosition.GetBuffer(17)))
        {
          szPosition.ReleaseBuffer();
          TRACE(_T("Using Position %s\n"), szPosition);
          if (szPosition.GetLength() == 16)
          {
            // position the mainframe window
            if (szPosition.Left(16) == "0000000000000000")
            {
              TRACE(_T("Maximizing\n"));
              m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED); // maximize the mainframe
              m_nCmdShow = SW_SHOWMAXIMIZED - 3; // to prevent MFC to restore on startup
            }
            else
            {
              // set mainframe position and show it
              TRACE(_T("SettingSize\n"));
              m_pMainWnd->MoveWindow(atoi(szPosition.Left(4)), atoi(szPosition.Mid(4, 4)),
                atoi(szPosition.Mid(8, 4)), atoi(szPosition.Mid(12, 4)));
              m_pMainWnd->ShowWindow(SW_SHOW);
            }
          }
          else // SDM 1.5Test8.5
          {
            m_pMainWnd->ShowWindow(SW_SHOW);
            TRACE(_T("Invalid Position\n"));
          }
        }
        else  // SDM 1.5Test8.5
        {
          szPosition.ReleaseBuffer();
          m_pMainWnd->ShowWindow(SW_SHOW);
          TRACE(_T("Invalid Position\n"));
        }
      }
#else
      else
      {
        m_pMainWnd->ShowWindow(SW_SHOW);
        TRACE(_T("Position Missing\n"));
      }
#endif
      if (wParam == SPEECH_WPARAM_SHOWSA)
      {
        // read WAV files in list file and open them
        CSaString szEntry = "File0"; // set profile entry name
        m_nEntry = 0;
        TCHAR szTemp[3] = _T("0");
        while (TRUE)
        {
          szReturn = GetBatchString(_T("AudioFiles"), szEntry); // get the entry
          if (szReturn.GetLength() <= 0) break; // no entry, finish loop
          WriteBatchString(_T("AudioFiles"), szEntry, _T(":")); // set entry to unchanged
          // open the document
          CSaDoc* pDoc = (CSaDoc*)OpenDocumentFile(szReturn);
          if (pDoc)
          {
            // set document ID
            pDoc->SetID(m_nEntry);
            // get start and stop cursor positions
            szEntry = "Offset";
            szEntry += szTemp; // set entry
            // get start cursor position
            szReturn = GetBatchString(_T("BeginningWAVOffsets"), szEntry); // get the entry
            if (szReturn.GetLength() > 0) // string found
            {
              dwStart = _ttol(szReturn);
              // set stop cursor position
              szReturn = GetBatchString(_T("EndingWAVOffsets"), szEntry); // get the entry
              dwStop = _ttol(szReturn);
              // get pointer to view
              POSITION pos = pDoc->GetFirstViewPosition();
              CSaView* pView = (CSaView*)pDoc->GetNextView(pos); // get pointer to view
              if(dwStop > dwStart)
              {
                // set start and stop cursors and view frame
                pView->SetStartCursorPosition(dwStart);
                pView->SetStopCursorPosition(dwStop);
                pView->GraphsZoomCursors(dwStart, dwStop);
              }
            }
          }
          szEntry = "File";
          _stprintf(szTemp, _T("%i"), ++m_nEntry); // create new number
          szEntry += szTemp; // add new file number
        }
        // SDM 1.06.8 Process Commands in Batch File
        m_nCommand = 0;// SDM 1.5Test8.5
        m_pMainWnd->PostMessage(WM_COMMAND, ID_PROCESS_BATCH_COMMANDS, 0L);
      }
      if (wParam == SPEECH_WPARAM_SHOWSAREC)
      {
        // create new file and launch recorder
        CMainFrame* pMDIFrameWnd = (CMainFrame*)AfxGetMainWnd();
        pMDIFrameWnd->PostMessage(WM_COMMAND, ID_FILE_RECORD, 0L);
      }
      m_bModified = FALSE; // batch file not yet modified
    }
    else // normal mode
    {
#ifdef WeShouldNotGetHere // We are now using MFC default command line processing for this mode
      CSaString szCursors;

      if(szCmdLine[0] == '\"')
      {
        // Filename in Quotes
        int nIndex = szCmdLine.Find("\"",1);
        
        if(nIndex != -1)
        {
          m_szCmdFileName = szCmdLine.Mid(1, nIndex-1);
          if((nIndex = szCmdLine.Find(' ', nIndex)) != -1)
            szCursors = szCmdLine.Mid(nIndex+1);
        }
        else
          m_szCmdFileName = szCmdLine.Mid(1);
      }
      else if (szCmdLine.Find(' ') != -1) // space found -> start/stop cursors given
      {
        // space found filename terminator
        m_szCmdFileName = szCmdLine.Left(szCmdLine.Find(' ')); // separate file path and name
        szCursors = szCmdLine.Right(szCmdLine.GetLength() - szCmdLine.Find(' ') - 1);
      }
      else m_szCmdFileName = szCmdLine;

        // open the document
      CSaDoc* pDoc = (CSaDoc*)OpenDocumentFile(m_szCmdFileName);

      // set start and stop cursors
      if (pDoc)
      {
        if(!szCursors.IsEmpty())
        {
          if (szCursors.Find(' ') != -1) // space found -> start and stop cursor given
          {
            dwStop = _ttol(szCursors.Right(szCursors.GetLength() - szCursors.Find(' ') - 1));
            dwStart = _ttol(szCursors.Left(szCursors.Find(' ')));
          }
          else dwStart = _ttol(szCursors); // copy file path and name
        }

        POSITION pos = pDoc->GetFirstViewPosition();
        CSaView* pView = (CSaView*)pDoc->GetNextView(pos); // get pointer to view
        pView->SetStartCursorPosition(dwStart);
        if (dwStop > dwStart) pView->SetStopCursorPosition(dwStop);
      }
#endif
    }
  }
}

// SDM 1.5Test8.3
/***************************************************************************/
// extractCommaField local helper function to get field from comma delimited string
/***************************************************************************/
static const CSaString extractCommaField(const CSaString& szLine, const int nField)
{
  int nCount = 0;
  int nLoop = 0;

  while ((nLoop < szLine.GetLength()) && (nCount < nField))
  {
    if (szLine[nLoop] == ',')
    {
      nCount++;
    }
    nLoop++;
  }
  int nBegin = nLoop;
  while ((nLoop < szLine.GetLength()) && (szLine[nLoop] != ','))
  {
    nLoop++;
  }
  return szLine.Mid(nBegin, nLoop-nBegin);
}

// SDM 1.06.8 New
// SDM 1.5Test8.3 Changed to function notation for parameter passing
// SDM 1.5Test8.5 Changed to single command per call added plat & displayplot
/***************************************************************************/
// CSaApp::OnProcessBatchCommands
// Process batch commands from listFile
/***************************************************************************/
void CSaApp::OnProcessBatchCommands()
{
  if (m_nCommand == -1)
    return;

  // Process Commands
  CSaString szEntry;
  CSaString szReturn;
  CSaString szParameterList;


  _stprintf(szEntry.GetBuffer(12), _T("command%i"), m_nCommand);
  szEntry.ReleaseBuffer();
  szReturn = GetBatchString(_T("Commands"), szEntry, _T("")); // get the entry
  szReturn.MakeUpper(); // convert the whole string to upper case letters
  szParameterList = "";
  szEntry = szReturn;
  _stscanf(szEntry,_T("%[^(]%(%[^)]"),szReturn.GetBuffer(szReturn.GetLength()), szParameterList.GetBuffer(szReturn.GetLength()));
  szReturn.ReleaseBuffer();
  szParameterList.ReleaseBuffer();

  if (szReturn == "")
  {
    // reset cursor positions after batch commands
    POSITION position = m_pDocTemplate->GetFirstDocPosition();
    while (position != NULL)
    {
      CDocument* pDoc = m_pDocTemplate->GetNextDoc(position); // get pointer to document
      if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CSaDoc)) && ((CSaDoc*)pDoc)->GetID() != -1)
      {
        _stprintf(szEntry.GetBuffer(12), _T("Offset%i"), ((CSaDoc*)pDoc)->GetID()); // create new number
        szEntry.ReleaseBuffer();
        // get start cursor position
        szReturn = GetBatchString(_T("BeginningWAVOffsets"), szEntry); // get the entry
        if (szReturn.GetLength() > 0) // string found
        {
          DWORD dwStart = _ttol(szReturn);
          // set stop cursor position
          szReturn = GetBatchString(_T("EndingWAVOffsets"), szEntry); // get the entry
          DWORD dwStop = _ttol(szReturn);
          // get pointer to view
          POSITION pos = pDoc->GetFirstViewPosition();
          CSaView* pView = (CSaView*)pDoc->GetNextView(pos); // get pointer to view
          if(dwStop > dwStart)
          {
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

  TRACE(_T("Batch Command:%s(%s)\n"), szReturn, szParameterList);
  if (szReturn.Left(6) == "IMPORT")
  {
    CSaString szPath = extractCommaField(szParameterList, 0);

    int nMode = CImport::KEEP;
    CSaString szMode = extractCommaField(szParameterList, 1);
    szMode.MakeUpper();
    if (szMode.Find(_T("AUTO")) != -1)
      nMode = CImport::AUTO;
    else if (szMode.Find(_T("MAN")) != -1)
      nMode = CImport::MANUAL;

    CFileStatus status;
    if (szPath.GetLength() && CFile::GetStatus(szPath, status))// SDM 1.5Test10.0
    {
      CImport* pImport = new CImport(szPath, TRUE);
      pImport->Import(nMode);
      if (pImport) delete pImport;

      try // SDM 1.5Test10.0
      {
        // delete the list file
        CFile::Remove(szPath);
      }
      catch (CFileException e)
      {
        // error removing file
        ErrorMessage(IDS_ERROR_DELLISTFILE, szPath);
      }
      
    }
    else
    {
      CSaView* pView = (CSaView*)((CMainFrame*)AfxGetMainWnd())->GetCurrSaView();
      CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();
      if ((pDoc->GetSegment(GLOSS)->IsEmpty())&& (nMode!=CImport::KEEP))
      {
        // auto parse
        if (!pDoc->AdvancedParse())
        {
          // process canceled by user
          pDoc->Undo(FALSE);
          return;
        }
      }
      if (nMode == CImport::AUTO)
      {
        if (!pDoc->AdvancedSegment())
        {
          // process canceled by user
          pDoc->Undo(FALSE);
          return;
        }
      }
    }
    m_pMainWnd->SendMessage(WM_COMMAND, ID_FILE_SAVE, 0l); // Save the file
  }
  else if (szReturn.Left(10) == "SELECTFILE")
  {
    int nID = 1;
    _stscanf(szParameterList, _T("%d"), &nID);
    POSITION position = m_pDocTemplate->GetFirstDocPosition();
    while (position != NULL)
    {
      CDocument* pDoc = m_pDocTemplate->GetNextDoc(position); // get pointer to document
      if (pDoc->IsKindOf(RUNTIME_CLASS(CSaDoc)) && ((CSaDoc*)pDoc)->GetID() == nID)
      {
        position = pDoc->GetFirstViewPosition();
        CView* pView = pDoc->GetNextView(position);
        pView->GetParent()->BringWindowToTop();
      }
    }
  }
  else if (szReturn.Left(8) == "SAVEFILE")
  {
    m_pMainWnd->SendMessage(WM_COMMAND, ID_FILE_SAVE, 0l);
  }
  else if (szReturn.Left(4) == "PLAY")
  {
    CSaView* pView = (CSaView*)((CMainFrame*)AfxGetMainWnd())->GetCurrSaView();
    CSaDoc* pDoc = 0;
    if (pView)
    {
      pDoc = pView->GetDocument();
    }

    enum
    {
      Player_Batch_Settings = 24
    };

    ASSERT(pView && pDoc);

    FnKeys* pKeys = ((CMainFrame*)m_pMainWnd)->GetFnKeys(0);
    pKeys->bRepeat[Player_Batch_Settings] = FALSE;     // TRUE, if playback repeat enabled
    pKeys->nDelay[Player_Batch_Settings] = 100;        // repeat delay time in ms
    pKeys->nMode[Player_Batch_Settings] = ID_PLAYBACK_CURSORS;       // replay mode

    //Get speed
    pKeys->nSpeed[Player_Batch_Settings] = 50;        // default replay speed in %
    CSaString szField = extractCommaField(szParameterList, 0);
    _stscanf(szField, _T("%u"), &(pKeys->nSpeed[Player_Batch_Settings]));
    if (pKeys->nSpeed[Player_Batch_Settings] < 10)
      pKeys->nSpeed[Player_Batch_Settings] = 10;
    if (pKeys->nSpeed[Player_Batch_Settings] > 333)
      pKeys->nSpeed[Player_Batch_Settings] = 333;

    //Get volume
    pKeys->nVolume[Player_Batch_Settings] = 50;       // default play volume in %
    szField = extractCommaField(szParameterList, 1);
    _stscanf(szField, _T("%u"), &(pKeys->nVolume[Player_Batch_Settings]));
    if (pKeys->nVolume[Player_Batch_Settings] < 0)
      pKeys->nVolume[Player_Batch_Settings] = 0;
    if (pKeys->nVolume[Player_Batch_Settings] > 100)
      pKeys->nVolume[Player_Batch_Settings] = 100;

    //Get Start Position
    DWORD dwStart = 0;
    szField = extractCommaField(szParameterList, 2);
    _stscanf(szField, _T("%lu"), &dwStart);

    //Get Stop Position
    DWORD dwStop = pDoc->GetUnprocessedDataSize();
    szField = extractCommaField(szParameterList, 3);
    _stscanf(szField, _T("%lu"), &dwStop);

    // set start and stop cursors
    pView->SetStartCursorPosition(dwStart);
    pView->SetStopCursorPosition(dwStop);

    TRACE(_T("PLAY(%u,%u,%lu,%lu)\n"),pKeys->nSpeed[Player_Batch_Settings],pKeys->nVolume[Player_Batch_Settings],dwStart,dwStop);

    m_pMainWnd->PostMessage(WM_USER_PLAYER, IDC_PLAY, MAKELONG(Player_Batch_Settings, -1));
    m_nCommand++;
    // Play Stop will resume batch processing
    // m_pMainWnd->PostMessage(WM_COMMAND, ID_PROCESS_BATCH_COMMANDS, 0L);
    return;
  }
  else if (szReturn.Left(11) == "DISPLAYPLOT")
  {
    CSaView* pView = (CSaView*)((CMainFrame*)AfxGetMainWnd())->GetCurrSaView();
    CSaDoc* pDoc = 0;
    if (pView)
    {
      pDoc = pView->GetDocument();
    }

    ASSERT(pView && pDoc);
    // Get Type
    CSaString szType = extractCommaField(szParameterList, 0);

    TRACE(_T("DisplayPlot(%s)\n"), szType);
    ((CMainFrame*)AfxGetMainWnd())->DisplayPlot(new CDisplayPlot(szType));

    m_nCommand++;
    // DisplayPlot End will resume batch processing
    // m_pMainWnd->PostMessage(WM_COMMAND, ID_PROCESS_BATCH_COMMANDS, 0L);
    return;
  }
  else if (szReturn.Left(6) == "RETURN")
  {
    int nHide = 1;
    _stscanf(szParameterList, _T("%d"), &nHide);
    m_nCommand = -1;
    FileReturn(nHide > 0);
    return;
  }
  else
  {
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
void CSaApp::ErrorMessage(UINT nTextID, LPCTSTR pszText1, LPCTSTR pszText2)
{
#ifdef _DEBUG
  ASSERT(FALSE);
#endif
  CSaString szText;
  try
  {
    // create the text
    if (pszText1)
    {
      if (pszText2)
      {
        AfxFormatString2(szText, nTextID, pszText1, pszText2);
      }
      else
      {
        AfxFormatString1(szText, nTextID, pszText1);
      }
    }
    else
    {
      szText.LoadString(nTextID);
    }
    m_pszErrors->Add(szText); // add the string to the array
  }
  catch (CMemoryException e)
  {
    // memory allocation error
    ErrorMessage(IDS_ERROR_MEMALLOC);
  }
  
}

/***************************************************************************/
// CSaApp::ErrorMessage Set error message
// Set an error message in the queue to be displayed as soon as possible.
/***************************************************************************/
void CSaApp::ErrorMessage(CSaString& szText)
{
#ifdef _DEBUG
  ASSERT(FALSE);
#endif
  try
  {
    m_pszErrors->Add(szText); // add the string to the array
  }
  catch (CMemoryException e)
  {
    // memory allocation error
    ErrorMessage(IDS_ERROR_MEMALLOC);
  }
  
}

/***************************************************************************/
// CSaApp::SetBatchFileChanged Sets a entry in the list(batch) file to changed
// Called by the documents this function sets a entry (file name) in the list
// file (application in batch mode) to changed state by writing the name of
// the changed file into the list file.
/***************************************************************************/
void CSaApp::SetBatchFileChanged(CSaString szFileName, int nID, CDocument* pDoc)
{
  CSaDoc* pSaDoc = (CSaDoc*)pDoc;               // cast document pointer
  TCHAR szTemp[3];
  CSaString szEntry = "File";
  if (nID == -1)
  {
    // no valid entry, new file
    _stprintf(szTemp, _T("%i"), m_nEntry);
  }
  else                                          // create entry number from ID
  {
    _stprintf(szTemp, _T("%i"), nID);                 // create number for entry
  }
  szEntry += szTemp;                                     // create entry
  TCHAR szShortName[MAX_PATH];
  GetShortPathName(szFileName, szShortName, MAX_PATH);
  WriteBatchString(_T("AudioFiles"), szEntry, szShortName); // set entry to changed or new file
  if (nID == -1)                                // no valid entry, new file
  {
    pSaDoc->SetID(m_nEntry++);                  // set the new ID
    szEntry = "DocID";
    szEntry += szTemp;                          // create entry
    WriteBatchString(_T("DocIDs"), szEntry, _T("0")); // set Doc ID to 0
  }
  m_bModified = TRUE;                           // file has been modified
}

/***************************************************************************/
// CSaApp::IsFileOpened Check if file already opened
// This function looks through the document list and compares all the
// documents file names with the one given as parameter (full path). If there
// is a match it returns a pointer to the document that matches, else NULL.
/***************************************************************************/
CDocument* CSaApp::IsFileOpened(const TCHAR* pszFileName)
{
  CSaString szFileName = pszFileName;
  szFileName.MakeUpper();
  POSITION position = m_pDocTemplate->GetFirstDocPosition();
  while (position != NULL)
  {
    CDocument* pDoc = m_pDocTemplate->GetNextDoc(position); // get pointer to document
    CSaString szComparisonFile = pDoc->GetPathName();
    szComparisonFile.MakeUpper();
    TRACE(_T(":IsFileOpened %s %s\n"),szComparisonFile,szFileName);
	if (szComparisonFile == szFileName)
    {
      return pDoc; // match
    }
  }
  return NULL;
}

/***************************************************************************/
// CSaApp::IsFileOpened Check if file already opened
// This function looks through the document list and compares all the
// documents file names with the one given as parameter (full path). If there
// is a match it returns a pointer to the document that matches, else NULL.
/***************************************************************************/
bool CSaApp::IsDocumentOpened( const CSaDoc * pDoc)
{
  POSITION position = m_pDocTemplate->GetFirstDocPosition();
  while (position != NULL)
  {
    CDocument * pLoopDoc = m_pDocTemplate->GetNextDoc(position); // get pointer to document
    if (pLoopDoc->IsKindOf(RUNTIME_CLASS(CSaDoc)))
    {
      CSaDoc * pSaDoc = (CSaDoc *)pLoopDoc;
      if (pSaDoc == pDoc)
      {
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
void CSaApp::SetWorkbenchPath(CSaString* pszPath)
{
  if (pszPath)
  {
    m_szWbPath = *pszPath;
  }
  else
  {
    m_szWbPath.Empty();
  }
}

/***************************************************************************/
// CSaApp::CloseWorkbench Close the workbench
// Returns TRUE if workbench closed, FALSE if not.
/***************************************************************************/
BOOL CSaApp::CloseWorkbench(CDocument* pDoc)
{
  if (m_pWbDoc)
  {
    // close the workbench view
    POSITION pos = m_pWbDoc->GetFirstViewPosition();
    CView* pView = m_pWbDoc->GetNextView(pos);
    pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0); // close view
  }
  BOOL bRet = (m_pWbDoc == NULL);
  m_pWbDoc = pDoc;
  return bRet;
}

/***************************************************************************/
// CSaApp::PasteClipboardToNewFile Create a new file and paste wave data into it
// This function creates a new document.
/***************************************************************************/
void CSaApp::PasteClipboardToNewFile(HGLOBAL hData)
{
  //because we now use true CF_WAVE we can save as temp then open
  // temporary target file has to be created
  TCHAR szTempPath[_MAX_PATH];
  TCHAR lpszTempPath[_MAX_PATH];
  GetTempPath(_MAX_PATH, lpszTempPath);
  GetTempFileName(lpszTempPath, _T("WAV"), 0, szTempPath);
  
  if ((::GlobalFlags(hData)&~GMEM_LOCKCOUNT)==GMEM_DISCARDED)
    return;
  HPSTR lpData = (HPSTR)::GlobalLock(hData); // lock memory
  DWORD dwSize = ::GlobalSize(hData);
  CFile* pFile=NULL;
  CFileStatus status;
  
  try
  {
    // create and open the file
    pFile = new CFile(szTempPath, CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive);
    pFile->WriteHuge(lpData, dwSize);
    delete pFile;
  }
  catch(CException pException)
  {
    if (pFile)
    {
      delete pFile;
      // File may exist should be removed
      if (CFile::GetStatus(szTempPath, status))
        CFile::Remove(szTempPath);
    }
    return;
  }
  ::GlobalUnlock(hData);
  
	CopyClipboardTranscription(szTempPath);

  // open the new file
  CSaDoc* pResult = OpenWavFileAsNew(szTempPath);
  
  if(!pResult)    // Error opening file, destroy temp
    CFile::Remove(szTempPath);
}

CSaDoc* CSaApp::OpenWavFileAsNew(const TCHAR* szTempPath)
{
  // create new MDI child, sa type
  POSITION posTemplate = GetFirstDocTemplatePosition();
  if (!GetNextDocTemplate(posTemplate))
  {
    TRACE0("Error : no document templates registered with CWinApp\n");
    AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC,MB_OK,0);
    return NULL;
  }
  m_bNewDocument = TRUE; // this is a file new operation
  posTemplate = GetFirstDocTemplatePosition();
  CDocTemplate* pTemplate = GetNextDocTemplate(posTemplate);
  ASSERT(pTemplate != NULL);
  ASSERT(pTemplate->IsKindOf(RUNTIME_CLASS(CDocTemplate)));
  CSaDoc* pDoc = (CSaDoc*)pTemplate->OpenDocumentFile(NULL);
  m_bNewDocument = FALSE;

  if ((!pDoc)||(!pDoc->IsKindOf(RUNTIME_CLASS(CSaDoc))))
  {
    if (pDoc)
      pDoc->OnCloseDocument();
    // Error opening file, destroy temp
    return NULL;
  }
  else
  {
    // Load temporarary file into document
    if (!pDoc->LoadDataFiles(szTempPath, TRUE))
    {
      if (pDoc)
        pDoc->OnCloseDocument();
      return NULL;
    }
    return pDoc;
  }
}

/***************************************************************************/
// CSaApp::CopyClipboardTranscription Copies the transcription
// associated with the last clipboard file from SA.
// If the wave data on the clipboard doesn't match the last data copied
// from SA, nothing is copied since it's from another app.
/***************************************************************************/
void CSaApp::CopyClipboardTranscription(const TCHAR* szTempPath)
{
	short bIsClipboardFile = (short)TRUE;
  _bstr_t szMD5HashCode = (wchar_t *)0;
	short bMD5MustMatch = (short)TRUE;

	CoInitialize(NULL);
	ISaAudioDocumentWriterPtr saAudioDocWriter;
	HRESULT createResult = saAudioDocWriter.CreateInstance(__uuidof(SaAudioDocumentWriter));
  if (createResult)
  {
		CSaString szCreateResult;
		szCreateResult.Format(_T("%x"), createResult);
		ErrorMessage(IDS_ERROR_CREATE_INSTANCE, _T("SaAudioDocumentWriter.CreateInstance()"), szCreateResult);
    return;
  }


	if (!saAudioDocWriter->Initialize((_bstr_t)m_szLastClipboardPath, szMD5HashCode, (short)bIsClipboardFile))
	{
		// TODO: Display a meaningful error.
		ErrorMessage(IDS_ERROR_WRITEPHONETIC, m_szLastClipboardPath);
		EndWaitCursor();
		saAudioDocWriter->Close();
		saAudioDocWriter->Release();
		saAudioDocWriter = NULL;
		CoUninitialize();
		return;
	}
	_bstr_t szDest = szTempPath;
	saAudioDocWriter->Copy(szDest, (short)bMD5MustMatch);
}

/***************************************************************************/
// CSaApp::OpenBlankView Opens a blank view to do something on.
//   RLJ 05/31/2000
// This functionality was moved here from OnFileRecord, since it is needed
// not only there, but also in FileOpen and OnFileCreate (I THINK).
/***************************************************************************/
CDocument* CSaApp::OpenBlankView( bool bWithGraphs)
{
	// create new MDI child, sa type
	POSITION posTemplate = GetFirstDocTemplatePosition();
	if (!GetNextDocTemplate(posTemplate))
	{
		TRACE0("Error : no document templates registered with CWinApp\n");
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC,MB_OK,0);
		return NULL;
	}
	
	CDocument * pDoc = NULL;
	if (bWithGraphs)
	{
		SetOpenAsID(ID_FILE_OPENAS_NEW);
		posTemplate = GetFirstDocTemplatePosition();
		CDocTemplate* pTemplate = GetNextDocTemplate(posTemplate);
		ASSERT(pTemplate != NULL);
		ASSERT(pTemplate->IsKindOf(RUNTIME_CLASS(CDocTemplate)));
		m_bNewDocument = TRUE;
		pDoc = pTemplate->OpenDocumentFile(NULL);
		m_bNewDocument = FALSE;
	}
	else
	{
		posTemplate = GetFirstDocTemplatePosition();
		CDocTemplate* pTemplate = GetNextDocTemplate(posTemplate);
		ASSERT(pTemplate != NULL);
		ASSERT(pTemplate->IsKindOf(RUNTIME_CLASS(CDocTemplate)));
		pDoc = pTemplate->OpenDocumentFile(NULL);
	}
	return pDoc;
}

/////////////////////////////////////////////////////////////////////////////
// CSaApp message handlers

/***************************************************************************/
// CSaApp::OnAppAbout Creates about dialog
// The user wants to see the modal about dialog.
/***************************************************************************/
void CSaApp::OnAppAbout()
{
	// initialize the about box object
	CoInitialize(NULL);
	IAboutDlgPtr aboutDlg;
	HRESULT createResult = aboutDlg.CreateInstance(__uuidof(AboutDlg));
	if (createResult)
	{
		CSaString szCreateResult;
		szCreateResult.Format(_T("%x"), createResult);
		ErrorMessage(IDS_ERROR_CREATE_INSTANCE, _T("AboutDlg.CreateInstance()"), szCreateResult);
		return;
	}

	// set the text values
	aboutDlg->ProdName = _T("Speech Analyzer");
	CSaString szVersion((LPCTSTR)VS_VERSION);;
	CSaString szBuild;
	szVersion = szVersion.Right(szVersion.GetLength() - szVersion.Find(' ') - 1);
	int nBuildIndex = szVersion.Find(_T("Build"));
	if (nBuildIndex > 0)
	{
		szBuild = szVersion.Mid(nBuildIndex, szVersion.GetLength() - nBuildIndex - 1);
		szVersion = szVersion.Left(nBuildIndex - 2);
	}
	
	// Remove RC number
	int nRCIndex = szVersion.Find(_T("RC"));
	if (nRCIndex > 0)
		szVersion = szVersion.Left(nRCIndex - 1);

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
void CSaApp::OnFileCreate()
{
	OpenBlankView(false); // RLJ 05/15/2000
}

/***************************************************************************/
// CSaApp::OnUpdateFileCreate Menu update
/***************************************************************************/
void CSaApp::OnUpdateFileCreate(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(TRUE);
}

/***************************************************************************/
// CSaApp::OnFileRecord Records new wave file
// The user wants to record a new wave file. This function creates a new
// view and launches the recorder. It contains the overwritten MFC OnFileNew,
// because MFC lets the user choose a view type.
/***************************************************************************/
void CSaApp::OnFileRecord()
{
	OpenBlankView(true); // use auto naming

    CMainFrame* pMDIFrameWnd = (CMainFrame*)AfxGetMainWnd();
	ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
	
	pMDIFrameWnd->SendMessage(WM_USER_IDLE_UPDATE, 0, 0); // give editor a chance to close
	
	// launch recorder in this new view
	pMDIFrameWnd->MDIGetActive()->GetActiveView()->SendMessage(WM_USER_RECORDER, 0, 0); // send message to start recorder
}

/***************************************************************************/
// CSaApp::FileOpen Opens an existing wave file
//   RLJ 05/31/2000
//   MOST OF THIS CODE WAS COPIED FROM OnFileRecord
/***************************************************************************/
void CSaApp::FileOpen()
{
  int id = GetOpenAsID();
  CDocument* pDoc = OpenBlankView(true); // uses auto naming

  CMainFrame* pMDIFrameWnd = (CMainFrame*)AfxGetMainWnd();
  ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
  pMDIFrameWnd->SendMessage(WM_USER_IDLE_UPDATE, 0, 0); // give editor a chance to close

  SetOpenAsID(id);

	CDlgFileOpen dlgFile(_T("wav"), _T("*.wav"), OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
		_T("WAV Files (*.wav)|*.wav|Other Audio Files (*.mp3;*.wma )|*.mp3;*.wma|Speech Analyzer Workbench Files (*.wb) |*.wb||"));

  CSaString szDefault = DefaultDir(); // need to save copy (return value is destroyed)
  dlgFile.m_ofn.lpstrInitialDir = szDefault;

  if (dlgFile.DoModal() == IDOK)
    OpenDocumentFile(dlgFile.GetPathName());

  if(pDoc)
  {
    pDoc->OnCloseDocument();
  }
}

CSaString CSaApp::DefaultDir(CSaString *pFilename) const
{
  if(pFilename)
  {
    CSaString szPath = *pFilename;
    int nFind = szPath.ReverseFind('\\');
    
    if (nFind != -1)
    {
      szPath = szPath.Left(nFind);
      
      CFileStatus cStatus;
      
      if(CFile::GetStatus(szPath, cStatus) && cStatus.m_attribute & CFile::directory)
        return szPath + "\\";
    }
  }

  // prompt the user (with all document templates)
  CSaString workingDir;
  for(int i=0;i < _AFX_MRU_MAX_COUNT;i++)
  {
    GetMRUFilePath(i,workingDir);
    if (workingDir.GetLength() > 0)
    {
      CSaString szPath = workingDir;
      int nFind = szPath.ReverseFind('\\');
      if (nFind != -1)
      {
        CFileStatus cStatus;
        
        if(CFile::GetStatus(szPath.Left(nFind), cStatus) && cStatus.m_attribute & CFile::directory)
        {
          workingDir = szPath.Left(nFind + 1);
          break;
        }
        else
        {
          workingDir.Empty(); // directory does not exist
        }
      }
    }
  }

  if(workingDir.IsEmpty())
  {
		// check data location in registry
    workingDir = ((CWinApp*)this)->GetProfileString(_T(""), _T("DataLocation"));
  }

  if(workingDir.IsEmpty())
  {
    // fall back to the current directory
		TCHAR Buffer[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, Buffer);
    workingDir = Buffer;
  }

  return workingDir;
}

/***************************************************************************/
// CSaApp::OnFileOpen Opens an existing wave file
/***************************************************************************/
void CSaApp::OnFileOpen()
{
  SetOpenAsID(ID_FILE_OPEN);
  FileOpen();
}

/***************************************************************************/
// Added on 09/01/2000 by DDO
/***************************************************************************/
void CSaApp::OnFileOpenSpecial()
{
  ShowStartupDialog(FALSE);
}

// CSaApp::OnFileOpenPA Opens an existing wave file for Phonetic Analysis
/***************************************************************************/
void CSaApp::OnFileOpenPA()
{
  SetOpenAsID(ID_FILE_OPENAS_PHONETICANALYSIS);
  FileOpen();
}

/***************************************************************************/
// CSaApp::OnFileOpenMA Opens an existing wave file for Music Analysis
/***************************************************************************/
void CSaApp::OnFileOpenMA()
{
  SetOpenAsID(ID_FILE_OPENAS_MUSICANALYSIS);
  FileOpen();
}

/***************************************************************************/
// Added on 09/01/2000 by DDO
/***************************************************************************/
void CSaApp::ShowStartupDialog(BOOL bAppIsStartingUp = TRUE)
{
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
  CStartModeDlg StartDlg;
  StartDlg.m_nDataMode = pMainWnd->GetStartDataMode();
  StartDlg.m_bShowDontShowAgainOption = bAppIsStartingUp;
  StartDlg.DoModal();
}

/***************************************************************************/
// CSaApp::OnUpdateFileRecord Menu update
/***************************************************************************/
void CSaApp::OnUpdateFileRecord(CCmdUI* pCmdUI)
{
  CMainFrame* pMDIFrameWnd = (CMainFrame*)AfxGetMainWnd();
  BOOL bEnable = TRUE;
  if (pMDIFrameWnd)
  {
    CView* pView = pMDIFrameWnd->GetCurrSaView();
    if (pView)
    {
      bEnable = pView->IsKindOf(RUNTIME_CLASS(CSaView));
    }
  }
  pCmdUI->Enable(bEnable);
}

/***************************************************************************/
// CSaApp::OnUpdateFileOpen Menu update
/***************************************************************************/
void CSaApp::OnUpdateFileOpen(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(!GetBatchMode()); // enabled only if no batch mode
}

// SDM 1.06.8 New
/***************************************************************************/
// CSaApp::OnUpdateRecentFileMenu Menu update
/***************************************************************************/
void CSaApp::OnUpdateRecentFileMenu(CCmdUI* pCmdUI)
{
  int nIndex = pCmdUI->m_nIndex;
  // MFC only calls for first MRU entry
  if (pCmdUI->m_nID == ID_FILE_MRU_FILE1)
    CSaAppBase::OnUpdateRecentFileMenu(pCmdUI);
  // MFC skips other entries force full update
  pCmdUI->m_nIndex= nIndex;
  pCmdUI->Enable(!GetBatchMode()); // enabled only if no batch mode
}

/***************************************************************************/
// CSaApp::OnFileReturn Hides the application and activates PA
//
// 8/4/06 This is now mostly deprecated, since PA doesn't call SA 
// this way anymore.
/***************************************************************************/
void CSaApp::OnFileReturn()
{
  FileReturn();
}

void CSaApp::FileReturn(BOOL bHide)
{
  if (SaveAllModified())
  {
    CloseAllDocuments(FALSE);
    if (!m_bModified) // modified batch files are available
    {
      // there are no changes in the list file, so delete the list file
      try
      {
        // delete the list file
        CFileStatus status;
        if (CFile::GetStatus(m_szCmdFileName, status)) // SDM 1.5Test8.5 (if present)
          CFile::Remove(m_szCmdFileName);
      }
      catch (CFileException e)
      {
        // error removing file
        ErrorMessage(IDS_ERROR_DELLISTFILE, m_szCmdFileName);
      }
      
    }
    CWnd* pWnd = IsAppRunning();// SDM 1.5Test8.5
    if (pWnd)
    {
      if (bHide)  // Optional Hide
      {
        m_pMainWnd->ShowWindow(SW_HIDE);
        m_nCmdShow = SW_HIDE; // to prevent MFC to restore on startup
      }
      pWnd->SendMessage(WM_USER_SPEECHAPPLICATION, SPEECH_WPARAM_SHOWSM, 0);
      if(bHide)
      {
        CancelBatchMode();
        OnAppExit();
      }
    }
    else // SDM 1.5Test8.5 allow option to exit app
    {
      TRACE(_T("CallingApp not found\n"));
      if (AfxMessageBox(IDS_QUESTION_APPNOTFOUND, MB_YESNO | MB_ICONQUESTION, 0) == IDYES)
      {
        CancelBatchMode();
        OnAppExit();
      }
    }
  }
}

// SDM 1.5Test8.5
/***************************************************************************/
// CSaApp::IsAppRunning Checks, if the calling application is running
// The function examines the top level windows after their caption. If it finds
// the CallingApp caption text, returns a pointer to its window, else a NULL pointer.
/***************************************************************************/
CWnd* CSaApp::IsAppRunning()
{
  CWnd* pTopWnd = AfxGetMainWnd()->GetWindow(GW_HWNDFIRST); // get pointer to first toplevel window
  while (pTopWnd != NULL)
  {
    CSaString szCaption;
    pTopWnd->GetWindowText(szCaption); // get caption text
    if (szCaption.Left(m_szCallingApp.GetLength()) == m_szCallingApp)
    {
      return pTopWnd; //found
    }
    pTopWnd = pTopWnd->GetWindow(GW_HWNDNEXT); // get pointer to next toplevel window
  }
  return NULL;
}

/***************************************************************************/
// CSaApp::OnUpdateFileReturn Menu update
/***************************************************************************/
void CSaApp::OnUpdateFileReturn(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(GetBatchMode()); // enabled only if batch mode
}

/***************************************************************************/
// CSaApp::OnUpdateAppExit Menu update
/***************************************************************************/
void CSaApp::OnUpdateAppExit(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(!GetBatchMode()); // enabled only if no batch mode
}

/***************************************************************************/
// CSaApp::OnIdle Idle processing
/***************************************************************************/
BOOL CSaApp::OnIdle(LONG lCount)
{
  // SDM 1.06.5 Send mainframe idle update message
  if (lCount <= 0)
  {
    CWnd* pMainWnd = AfxGetMainWnd();
    if (pMainWnd != NULL && pMainWnd->m_hWnd != NULL &&
      pMainWnd->IsWindowVisible())
    {
      pMainWnd->SendMessage(WM_USER_IDLE_UPDATE, 0, 0L); // SDM 32 bit conversion
    }

  }
  BOOL bMore = CSaAppBase::OnIdle(lCount);
  // display error message if present
  DisplayErrorMessage();

  if(bMore)
    return TRUE; // more idle processing necessary

  // get active document
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
  CSaDoc *pSaDoc = pMainWnd->GetCurrDoc();

  // perform pitch processing if not already done
  if ((pSaDoc) && 
			(pSaDoc->IsBackgroundProcessing()) && 
			(pSaDoc->GetDataSize()))
  {
    CProcessGrappl * pAutoPitch = pSaDoc->GetGrappl();
    if (!pAutoPitch->IsDataReady())
    {
      if (pAutoPitch->IsCanceled()) pSaDoc->EnableBackgroundProcessing(FALSE);
	  else
      {
	    short int nResult = LOWORD(pAutoPitch->Process(this, (CSaDoc *)pSaDoc)); // process data
        if (nResult == PROCESS_ERROR || nResult == PROCESS_NO_DATA || nResult == PROCESS_CANCELED)
		{
          pAutoPitch->SetDataInvalid();
          pSaDoc->EnableBackgroundProcessing(FALSE);
          //pAutoPitch->RestartProcess(this);   
		}
        else if (pAutoPitch->IsDataReady()) pSaDoc->NotifyAutoPitchDone(this);
      }
      return TRUE;
    }
    // run fragmenter if not already done
    CProcessFragments* pFragmenter = pSaDoc->GetFragments();
    if (!pFragmenter->IsDataReady())
    {
      short int nResult = LOWORD(pFragmenter->Process(this, (CSaDoc *)pSaDoc)); // process data
      if (nResult == PROCESS_ERROR || nResult == PROCESS_NO_DATA || nResult == PROCESS_CANCELED)
      {
        pFragmenter->SetDataInvalid();
        pSaDoc->EnableBackgroundProcessing(FALSE);
        pFragmenter->RestartProcess();
      }
      else return TRUE;
    }
  }

  return bMore;
}

/***************************************************************************/
// CSaApp::DisplayErrorMessage Displays a stored error message
/***************************************************************************/
void CSaApp::DisplayErrorMessage()
{
  if (m_pszErrors->GetSize() > 0)
  {
    CSaString error = m_pszErrors->GetAt(0);
    m_pszErrors->RemoveAt(0, 1); // remove message (before we lose process thread)
    AfxMessageBox(error, MB_OK | MB_ICONEXCLAMATION, 0);
  }
}

/***************************************************************************/
// CSaApp::OnHelpContents Call Help Index (Table of Contents)
/***************************************************************************/
void CSaApp::OnHelpContents()
{
	::HtmlHelp(NULL, m_pszHelpFilePath, HH_DISPLAY_TOC, NULL);
}

/***************************************************************************/
// CSaApp::OnHelpTrouble Call Troubleshooting help topic
/***************************************************************************/
void CSaApp::OnHelpTrouble()
{
	CSaString szPath = m_pszHelpFilePath;
	szPath += "::/Troubleshooting/Troubleshooting_overview.htm";
  ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

/***************************************************************************/
// CSaApp::OnHelpGraphs Call Graphs overview help topic
/***************************************************************************/
void CSaApp::OnHelpGraphs()
{
  // create the pathname
	CSaString szPath = m_pszHelpFilePath;
	szPath = szPath + "::/User_Interface/Menus/Graphs/Graphs_overview.htm";
	::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

/***************************************************************************/
// CSaApp::OnHelpMusic Call Musical Analysis help topic
/***************************************************************************/
void CSaApp::OnHelpMusic()
{
  // create the pathname
	CSaString szPath = m_pszHelpFilePath;
	szPath = szPath + "::/User_Interface/Menus/Graphs/Graph_Types/Music_Graph_Types/Music_analysis.htm";
	::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

/***************************************************************************/
// CSaApp::OnHelpTrainingStudent Open Student Manual
/***************************************************************************/
void CSaApp::OnHelpTrainingStudent()
{
	CSaString szAppPath = m_pszHelpFilePath;
	szAppPath = szAppPath.Left(szAppPath.ReverseFind('\\'));
	CSaString szCommandLine = "\"" + szAppPath + _T("\\Training\\SA Student Manual.doc\"");
	ShellExecute(NULL, _T("open"), szCommandLine.GetBuffer(1), NULL, NULL, SW_SHOWNORMAL);
}

/***************************************************************************/
// CSaApp::OnHelpTrainingInstructor Open Instructor Guide
/***************************************************************************/
void CSaApp::OnHelpTrainingInstructor()
{
	CSaString szAppPath = m_pszHelpFilePath;
	szAppPath = szAppPath.Left(szAppPath.ReverseFind('\\'));
	CSaString szCommandLine = "\"" + szAppPath + _T("\\Training\\SA Instructor Guide.doc\"");
	ShellExecute(NULL, _T("open"), szCommandLine.GetBuffer(1), NULL, NULL, SW_SHOWNORMAL);
}

/***************************************************************************/
// CSaApp::OnHelpOnHelp Call Windows Help on Help
/***************************************************************************/
void CSaApp::OnHelpOnHelp()
{
  // create the pathname
	CSaString szPath = m_pszHelpFilePath;
	szPath = szPath + "::/Redirect.htm#its:Using_Help.chm::/Using_Help/Using_Help_overview.htm";
  ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

/***************************************************************************/
// CWbDlgProcesses::OnHelpWorkbench Call Workbench help
/***************************************************************************/
void CSaApp::OnHelpWorkbench()
{
	// create the pathname
	CString szPath = m_pszHelpFilePath;
	szPath += "::/User_Interface/Menus/Tools/Workbench.htm";
	::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

/***************************************************************************/
// CSaApp::OnAudioCon Execute winacon.exe
/***************************************************************************/
void CSaApp::OnAudioCon()
{
  // look for AudioCon in a sub-folder
  CSaString szAppPath = m_pszHelpFilePath; // C:\Program Files\SIL Software\Speech Analyzer\SA.exe
  CSaString szAudioConFolder = szAppPath.Left(szAppPath.ReverseFind('\\')) + "\\AudioCon";
  CSaString szAudioConPath = szAudioConFolder + "\\winacon.exe";
	
	try
	{
		CFile file(szAudioConPath, CFile::modeRead);
	}
	catch (CFileException* e)
	{
		if (e->m_cause == CFileException::fileNotFound)
		{
			ErrorMessage(IDS_ERROR_AUDIOCON_MISSING);
			return;
		}
		else if (e->m_cause == CFileException::sharingViolation)
		{
			// just ignore this exception and open another AudioCon process
		}
		else
		{
			ErrorMessage(IDS_ERROR_AUDIOCON_UNKNOWN);
			return;
		}
	}

	CSaString szMessage;
	szMessage.Format(CSaString((LPCTSTR)IDS_AUDIOCON_SAXML_NOTICE), (LPCTSTR)szAudioConFolder);
	AfxMessageBox(szMessage);

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
void CSaApp::OnWorkbenchOpen()
{
  // get pointer to workbench template
  POSITION posTemplate = GetFirstDocTemplatePosition();
  if (!GetNextDocTemplate(posTemplate))
  {
    TRACE0("Error : no document templates registered with CWinApp\n");
    AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC,MB_OK,0);
    return;
  }
  posTemplate = GetFirstDocTemplatePosition();
  GetNextDocTemplate(posTemplate);
  CDocTemplate* pTemplate = GetNextDocTemplate(posTemplate);
  ASSERT(pTemplate != NULL);
  ASSERT(pTemplate->IsKindOf(RUNTIME_CLASS(CDocTemplate)));
  if (!m_pWbDoc)
  {
    // create new MDI child, workbench type
    if (m_szWbPath.IsEmpty())
    {
      m_pWbDoc = pTemplate->OpenDocumentFile(NULL);
    }
    else
    {
      m_pWbDoc = pTemplate->OpenDocumentFile(m_szWbPath);
    }
    if (!m_pWbDoc)
    {
      m_szWbPath.Empty(); // was not able to open file
    }
  }
  else
  {
    if (!m_szWbPath.IsEmpty())
    {
      // close actually opened workbench and open new one
      POSITION pos = m_pWbDoc->GetFirstViewPosition();
      CView* pView = m_pWbDoc->GetNextView(pos);
      CSaString szSavePath = m_szWbPath;
      pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0); // close view
      m_szWbPath = szSavePath;
      m_pWbDoc = pTemplate->OpenDocumentFile(m_szWbPath);
      if (!m_pWbDoc)
      {
        m_szWbPath.Empty(); // was not able to open file
      }
    }
  }
}

/***************************************************************************/
// CSaApp::OnUpdateWorkbenchOpen Menu update
/***************************************************************************/
void CSaApp::OnUpdateWorkbenchOpen(CCmdUI* pCmdUI)
{
  pCmdUI->SetCheck(m_pWbDoc != NULL);
}

#define PDERR_PRINTERNOTFOUND  0x100B
#define PDERR_DNDMMISMATCH     0x1009


/***************************************************************************/
// CSaApp::SaGetPrinterDeviceDefaults
// This is a special version of the standard MFC GetPrinterDeviceDefaults
// which allows us to set the print orientation (portrait/landscape)
/***************************************************************************/
BOOL CSaApp::SaGetPrinterDeviceDefaults(PRINTDLG * pPrintDlg, BOOL landscape)
{
  UpdatePrinterSelection(m_hDevNames == NULL); //force default if no current
  if (m_hDevNames == NULL)
  {
    return FALSE;               // no printer defaults
  }

  pPrintDlg->hDevNames = m_hDevNames;
  pPrintDlg->hDevMode = m_hDevMode;

  DEVMODE * theDM = (DEVMODE *)GlobalLock(pPrintDlg->hDevMode);

  theDM->dmFields |= DM_ORIENTATION;
  if (landscape)
  {
    theDM->dmOrientation = DMORIENT_LANDSCAPE;
  }
  else
  {
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
int CSaApp::SaDoPrintDialog(CPrintDialog* pPD, BOOL landscape)
{
  UpdatePrinterSelection(FALSE);

  pPD->m_pd.hDevMode = m_hDevMode;
  pPD->m_pd.hDevNames = m_hDevNames;

  DEVMODE * theDM = pPD->GetDevMode();

  theDM->dmFields |= DM_ORIENTATION;
  if (landscape)
  {
    theDM->dmOrientation = DMORIENT_LANDSCAPE;
  }
  else
  {
    theDM->dmOrientation = DMORIENT_PORTRAIT;
  }

  pPD->m_pd.Flags |= PD_NOSELECTION; // change the PRINTDLG

  int nResponse = pPD->DoModal();

  // if OK or Cancel is selected we need to update cached devMode/Names
  while (nResponse != IDOK && nResponse != IDCANCEL)
  {
    switch (::CommDlgExtendedError())
    {
      // CommDlg cannot give these errors after NULLing these handles
    case PDERR_PRINTERNOTFOUND:
    case PDERR_DNDMMISMATCH:
      if (pPD->m_pd.hDevNames != NULL)
      {
        ASSERT(m_hDevNames == pPD->m_pd.hDevNames);

        ::GlobalFree(pPD->m_pd.hDevNames);
        pPD->m_pd.hDevNames = NULL;
        m_hDevNames = NULL;
      }

      if (pPD->m_pd.hDevMode)
      {
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
CSaView* CSaApp::pviewActive()
{
  CMainFrame* pwndMainFrame = (CMainFrame*)m_pMainWnd;
  ASSERT( pwndMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)) );

  CSaView* pview = ( pwndMainFrame ?
                     pwndMainFrame->GetCurrSaView() :
                     NULL);

  return pview;
}

/***************************************************************************/
/***************************************************************************/
CSaView* CSaApp::pviewTop()
{
  return pviewEnd(GW_HWNDPREV);
}

/***************************************************************************/
/***************************************************************************/
CSaView* CSaApp::pviewBottom()
{
  return pviewEnd(GW_HWNDNEXT);
}

/***************************************************************************/
/***************************************************************************/
CSaView* CSaApp::pviewAbove(CSaView* pviewCur)
{
  return pviewNeighbor(pviewCur, GW_HWNDPREV);
}

/***************************************************************************/
/***************************************************************************/
CSaView* CSaApp::pviewBelow(CSaView* pviewCur)
{
  return pviewNeighbor(pviewCur, GW_HWNDNEXT);
}

/***************************************************************************/
/***************************************************************************/
void CSaApp::SetZ()
{
  int z = 0;
  CSaView* pview = pviewBottom();
  for ( ; pview; pview = pviewAbove(pview) )
    pview->SetZ(z++);
}

/***************************************************************************/
/***************************************************************************/
CSaView* CSaApp::pviewEnd(UINT uNextOrPrev)
{
  CSaView* pview = pviewActive();
  CSaView* pviewN = ( pview ? pviewNeighbor(pview, uNextOrPrev) : NULL);
  for ( ; pviewN; pviewN = pviewNeighbor(pviewN, uNextOrPrev) )
    pview = pviewN;

  return pview;
}

/***************************************************************************/
/***************************************************************************/
CSaView* CSaApp::pviewNeighbor(CSaView* pviewCur, UINT uNextOrPrev)
{
  ASSERT( pviewCur );
  CMDIChildWnd* pwndChildFrame = pviewCur->pwndChildFrame();

  ASSERT( uNextOrPrev == GW_HWNDNEXT || uNextOrPrev == GW_HWNDPREV );
  CWnd* pwndNext = pwndChildFrame->GetNextWindow(uNextOrPrev);
  for ( ; pwndNext; pwndNext = pwndNext->GetNextWindow(uNextOrPrev) )
    if ( pwndNext->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)) )
      // NOTE: This test is to skip minimized icon windows,
      // because they are *in addition to* the corresponding
      // MDI child frame window which contains the view object.
    {
      break;
    }

    CSaView* pview = ( pwndNext ?
                       CSaView::s_pviewActiveChild((CMDIChildWnd*)pwndNext) :
                       NULL);

    return pview;
}

/***************************************************************************/
/***************************************************************************/
static const char* psz_mrulst       = "mrulst";
static const char* psz_mru          = "mru";
static const char* psz_SaApp        = "SaApp";
static const char* psz_settingsfile = "sa3.psa";
static const char* psz_batchsettingsfile = "sa batch.psa";
static const char* psz_workbench    = "workbench";
static const char* psz_wbonexit     = "wbonexit";

/***************************************************************************/
/***************************************************************************/
void CSaApp::WriteProperties(Object_ostream& obs)
{
	// get the version
	CSaString szVersion((LPCTSTR)VS_VERSION);
	CSaString szBuildNum;
	szVersion = szVersion.Right(szVersion.GetLength() - szVersion.Find(' ') - 1);
	int nBuildIndex = szVersion.Find(_T("Build"));
	if (nBuildIndex > 0)
		szBuildNum = _T(".") + szVersion.Mid(nBuildIndex + 6, szVersion.GetLength() - nBuildIndex - 7);
	szVersion = szVersion.Left(szVersion.Find(' ')) + szBuildNum;

	// The open databases and windows
	SetZ();  // Set the current z-order of all views
	obs.WriteBeginMarker(psz_SaApp, szVersion);
	obs.WriteNewline();

  ASSERT(m_pMainWnd);

  ((CMainFrame *)m_pMainWnd)->WriteProperties(obs);

  if (!GetBatchMode() && ((CMainFrame *)m_pMainWnd)->GetSaveOpenFiles())
  {
    //tdg 09/03/97
    CDocList doclst; // get list of currently open documents
    for ( CSaDoc* pdoc = doclst.pdocFirst(); pdoc; pdoc = doclst.pdocNext() )
    {
      if(pdoc->GetPathName().GetLength()) // only write properties for files with paths
        pdoc->WriteProperties(obs);
    }
  }

  ((CMainFrame *)m_pMainWnd)->WriteDefaultView(obs);
  obs.WriteString(psz_workbench, m_szWbPath);
  obs.WriteBool(psz_wbonexit, m_bWbOpenOnExit);
  CDlgRecorder::GetStaticSourceInfo().WriteProperties(obs);
  obs.WriteEndMarker(psz_SaApp);
}

/***************************************************************************/
// read the open databases and windows
/***************************************************************************/
BOOL CSaApp::bReadProperties(Object_istream& obs)
{
	CSaString szLastVersion;
	
	if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_SaApp, &szLastVersion))
	{
		return FALSE;
	}

  ASSERT(m_pMainWnd);

  m_szLastVersion = szLastVersion.IsEmpty() ? _T("2.7") : szLastVersion;
	
	while (!obs.bAtEnd())
  {
    if (((CMainFrame *)m_pMainWnd)->bReadProperties(obs));
    else if (!GetBatchMode() && CSaDoc::s_bReadProperties(obs));
    else if (((CMainFrame *)m_pMainWnd)->bReadDefaultView(obs));
    else if (obs.bReadString(psz_workbench, &m_szWbPath));
    else if (CDlgRecorder::GetStaticSourceInfo().bReadProperties(obs));
    else if (obs.bEnd(psz_SaApp))
      break;
  }

  // open the workbench
  if (m_bWbOpenOnExit && !m_szWbPath.IsEmpty())
  {
    OnWorkbenchOpen();
  }

  // activate top window (not last document)
  if(pviewTop())  // only if there is a top window
    pviewTop()->ShowInitialTopState();

  return TRUE;
}

/***************************************************************************/
// Read in Most Recently Used file list on the File menu
/***************************************************************************/
BOOL CSaApp::bReadMRUList(Object_istream& obs)
{
  if (!obs.bReadBeginMarker(psz_mrulst))
  {
    return FALSE;
  }

  while (!obs.bAtEnd())
  {
    CSaString sFile;
    if (obs.bReadString(psz_mru, &sFile))
#ifdef BJY_5_23_96
      AddToRecentFileList(sFile);
#else
    ;
#endif
    else if (obs.bEnd(psz_mrulst))
      break;
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
BOOL CSaApp::WriteSettings()
{
  BOOL ret = FALSE;

  CSaString szPath = ((CWinApp*)this)->GetProfileString(_T(""), _T("DataLocation"));
  szPath += "\\";

  if(GetBatchMode())
    szPath += psz_batchsettingsfile;
  else    
    szPath += psz_settingsfile;

  Object_ostream obs(szPath);

  if (!obs.getIos().fail())
  {
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
*/
/***************************************************************************/
BOOL CSaApp::ReadSettings()
{
  BOOL ret = FALSE;

  // get the data location and create it if it doesn't exist
  CSaString szPath = ((CWinApp*)this)->GetProfileString(_T(""), _T("DataLocation"), _T("*Missing*"));

	if (szPath == _T("*Missing*"))
	{
		m_bNewUser = TRUE;
		szPath.Empty();
	}

	if (szPath.IsEmpty())
	{
		// Set the DataLocation path and write it to the registry
		TCHAR buf[MAX_PATH];
		HMODULE hModule = LoadLibrary(_T("SHFOLDER.DLL"));
		if (hModule != NULL)
		{
			SHGETFOLDERPATH fnShGetFolderPath = (SHGETFOLDERPATH)GetProcAddress(hModule, "SHGetFolderPathW");

			if (fnShGetFolderPath != NULL)
			{
				fnShGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, buf);
				szPath = (LPTSTR)buf;
			}
			FreeLibrary(hModule);
		}

		if (szPath.Right(1) != "\\")
			szPath += _T("\\");
		szPath += _T("Speech Analyzer");
		WriteProfileString(_T(""), _T("DataLocation"), szPath);
	}
	if (szPath.Right(1) == "\\")
		szPath = szPath.Left(szPath.GetLength() - 1);

  CFileStatus cStatus;
  if(!(CFile::GetStatus(szPath, cStatus) && cStatus.m_attribute & CFile::directory))
    CreateDirectory(szPath, NULL);

  szPath += "\\";

  if(GetBatchMode())
    szPath += psz_batchsettingsfile;
  else    
    szPath += psz_settingsfile;

  Object_istream obs(szPath);

  if (!obs.getIos().fail())
  {
    ret = bReadProperties(obs);
  }

  return ret;
}

/***************************************************************************/
/***************************************************************************/
CSaString CSaApp::GetStartupMessage(CSaString szLastVersion)
{
	if ((szLastVersion[0] < '2') || (szLastVersion[0] > '9'))
		szLastVersion = "2.7";

	CSaString szRelease = szLastVersion.Left(3);
	CSaString szBuild = szLastVersion.Right(szLastVersion.GetLength() - 4);
	CSaString msg;
	
	if (szRelease == "2.7")
	{
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
void CSaApp::SetupNewUser()
{
  // get the data and app locations
  CSaString szDataLocation = ((CWinApp*)this)->GetProfileString(_T(""), _T("DataLocation"), _T("Missing"));
	CSaString szAppLocation = m_pszHelpFilePath;
	if (szDataLocation.IsEmpty() || szAppLocation.IsEmpty())
		return;
	szAppLocation = szAppLocation.Left(szAppLocation.ReverseFind(_T('\\')));

	// create new user's samples folders
	CSaString szSamplesLocation;
	CSaString szMusicLocation;
	szSamplesLocation = szDataLocation + _T("\\Samples");
	szMusicLocation = szSamplesLocation + _T("\\Music");
	CreateDirectory(szSamplesLocation, NULL);
	CreateDirectory(szMusicLocation, NULL);

	// copy the samples into the samples folders
	CSaString szSearchPattern;
	CSaString szSourcePath;
	CSaString szDestPath;
	CSaString subFolders[2] = {_T("\\Samples\\"), _T("\\Samples\\Music\\")};

	for (int i = 0; i < 2; i++)
	{
		// initialize the search
		WIN32_FIND_DATA FileData; 
		HANDLE hSearch;
		BOOL bFinished = FALSE;
		szSearchPattern = szAppLocation + subFolders[i] + _T("*.*");
		hSearch = FindFirstFile(szSearchPattern, &FileData);
		if (hSearch == INVALID_HANDLE_VALUE)
			return;

		// copy each file to the new directory 
		while (!bFinished)
		{
			szSourcePath = szAppLocation + subFolders[i] + FileData.cFileName;
			szDestPath = szDataLocation + subFolders[i] + FileData.cFileName;
			CopyFile(szSourcePath, szDestPath, FALSE);

			if (!FindNextFile(hSearch, &FileData) && (GetLastError() == ERROR_NO_MORE_FILES))
				bFinished = TRUE;
		}
		FindClose(hSearch);
	}
}

/***************************************************************************/
/***************************************************************************/

CDocument* CSaApp::OpenDocumentFile(LPCTSTR lpszFileName) 
{
  CSaString szPrettyName = lpszFileName;
  LPTSTR pszPretty = szPrettyName.GetBuffer(_MAX_PATH);

  // Complicated Run-Time Dynamic Linking because GetLongPathName is not supported in Win95
  HMODULE hKernel32 = GetModuleHandle(_T("KERNEL32"));
  if(hKernel32)
  {
    typedef DWORD (WINAPI *PGetLongPathName)( LPCTSTR lpszShortPath, LPTSTR  lpszLongPath, DWORD cchBuffer);

#ifdef UNICODE
    PGetLongPathName pGetLongPathName = (PGetLongPathName) GetProcAddress(hKernel32, "GetLongPathNameW"); 
#else
    PGetLongPathName pGetLongPathName = (PGetLongPathName) GetProcAddress(hKernel32, "GetLongPathNameA"); 
#endif // !UNICODE

    if(pGetLongPathName)
      // Convert path name to proper case (fixes asthetic problems)
      (pGetLongPathName)(pszPretty, pszPretty, _MAX_PATH);
  }

  return CSaAppBase::OpenDocumentFile(pszPretty);
}

BOOL CSaApp::m_bUseUnicodeEncoding = FALSE;
