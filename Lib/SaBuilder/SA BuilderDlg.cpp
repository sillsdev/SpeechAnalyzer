// SA BuilderDlg.cpp : implementation file
//

#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <direct.h>
#include "stdafx.h"
#include <setupapi.h>
#include <windows.h>
#include "SA Builder.h"
#include "SA BuilderDlg.h"
#include "Table.h"
#include <fstream.h>
#include <afxinet.h>
#include <math.h>
#define MAX_LEN 100

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSABuilderDlg dialog

CSABuilderDlg::CSABuilderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSABuilderDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSABuilderDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// note command line switch(es)
	m_bBatchMode = FALSE;
	m_bLabelVersion = FALSE;
  m_bDeleteFilesFirst = TRUE;
	m_bSACodeChanges = FALSE;
	m_bPACodeChanges = FALSE;
	m_bCSharpCodeChanges = FALSE;
	LPTSTR lpCmdLine = AfxGetApp()->m_lpCmdLine;
	for(register i=0;lpCmdLine[i] == _T('/');)
	{
		++i;
		if(AfxGetApp()->m_lpCmdLine[i] == _T('B')) // batch mode
		{
			m_bBatchMode = TRUE;
			while(lpCmdLine[++i]==_T(' '));
			continue;
		}
		if(AfxGetApp()->m_lpCmdLine[i] == _T('L')) // label version
		{
			m_bLabelVersion = TRUE;
			while(lpCmdLine[++i]==_T(' '));
			continue;
		}
	}

	// Find SABuilder's path
	GetModuleFileName(AfxGetApp()->m_hInstance, m_szSABuilderPath.GetBuffer(_MAX_PATH), _MAX_PATH);
	m_szSABuilderPath.ReleaseBuffer();
	int nSlashPos = m_szSABuilderPath.ReverseFind('\\');
	m_szSABuilderPath.Delete(nSlashPos + 1, m_szSABuilderPath.GetLength() - nSlashPos - 1);
  m_nErrors = 0;
  m_nWarnings = 0;

	// set path environment variable
	CString szPathOld = getenv("path");
	_putenv("Path=" + szPathOld + ";" + m_szSvnExePath + ";" + m_szMSDevPath);

}

void CSABuilderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSABuilderDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSABuilderDlg, CDialog)
	//{{AFX_MSG_MAP(CSABuilderDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_BRANCH_EDIT, OnChangeBranchEdit)
	ON_EN_CHANGE(IDC_VERSION_MAJOR_EDIT, OnChangeVersionMajorEdit)
	ON_EN_CHANGE(IDC_VERSION_MINOR_EDIT, OnChangeVersionMinorEdit)
	ON_EN_CHANGE(IDC_RC_EDIT, OnChangeRCEdit)
	ON_EN_CHANGE(IDC_DEST_PATH_EDIT, OnChangeDestPathEdit)
	ON_BN_CLICKED(IDC_STORE_BUTTON, OnStoreButton)
	ON_EN_CHANGE(IDC_MSDEVPATH, OnChangeDevPath)
	ON_EN_CHANGE(IDC_MSBUILDPATH, OnChangeBuildPath)
	ON_EN_CHANGE(IDC_TESTPATH, OnChangeTestPath)
	ON_EN_CHANGE(IDC_NOTEPATH, OnChangeNotePath)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSABuilderDlg message handlers

void CSABuilderDlg::OnChangeBranchEdit() 
{
}

void CSABuilderDlg::OnChangeDestPathEdit() 
{
}

void CSABuilderDlg::OnChangeDevPath() 
{	GetDlgItemText(IDC_MSDEVPATH,m_szMSDevPath);
}

void CSABuilderDlg::OnChangeBuildPath() 
{	GetDlgItemText(IDC_MSBUILDPATH,m_szMSBuildPath);
}

void CSABuilderDlg::OnChangeNotePath() 
{	GetDlgItemText(IDC_NOTEPATH,m_szNotepadPath);
}

void CSABuilderDlg::OnChangeRCEdit() 
{
}

void CSABuilderDlg::OnChangeTestPath() 
{	GetDlgItemText(IDC_TESTPATH,m_szTestPath);
}

void CSABuilderDlg::OnChangeVersionMajorEdit() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
}

void CSABuilderDlg::OnChangeVersionMinorEdit() 
{
}

BOOL CSABuilderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// Read registry & fill boxes
	char szString[_MAX_PATH]="";
  char szBranchKey[_MAX_PATH]="SA Builder\\";
	sdfGetRegistryString("SA Builder","SvnBranch","trunk",szString,_MAX_PATH);
	SetDlgItemText(IDC_BRANCH_EDIT,szString);
  strcat(szBranchKey, szString);
	sdfGetRegistryString(szBranchKey,"VersionMajor","3",szString,_MAX_PATH);
	SetDlgItemText(IDC_VERSION_MAJOR_EDIT,szString);
	sdfGetRegistryString(szBranchKey,"VersionMinor","0",szString,_MAX_PATH);
	SetDlgItemText(IDC_VERSION_MINOR_EDIT,szString);
	sdfGetRegistryString(szBranchKey,"VersionBuild", "0", szString,_MAX_PATH);
	m_szVersionBuild = szString;
	sdfGetRegistryString(szBranchKey,"RC","",szString,_MAX_PATH);
	SetDlgItemText(IDC_RC_EDIT,szString);
	sdfGetRegistryString(szBranchKey,"Beta","",szString,_MAX_PATH);
	m_szBeta = szString;
	sdfGetRegistryString("SA Builder","DestinationPath","",szString,_MAX_PATH);
	SetDlgItemText(IDC_DEST_PATH_EDIT,szString);
	sdfGetRegistryString("SA Builder","BuildPath","",szString,_MAX_PATH);
	SetDlgItemText(IDC_SA_PATH_EDIT,szString);
	sdfGetRegistryString("SA Builder","SvnExePath","C:\\Program Files\\Subversion\\bin\\",szString,_MAX_PATH);
	SetDlgItemText(IDC_SVN_EXE_PATH_EDIT,szString);
	sdfGetRegistryString("SA Builder","SvnUser","",szString,_MAX_PATH);
	SetDlgItemText(IDC_SVN_USER_EDIT,szString);
	sdfGetRegistryString("SA Builder","SvnPass","",szString,_MAX_PATH);
	SetDlgItemText(IDC_SVN_PASS_EDIT,szString);
	sdfGetRegistryString("SA Builder","SvnRepoURL","https://svn.sil.org/langsw/speechtools/",szString,_MAX_PATH);
	SetDlgItemText(IDC_SVN_REPO_URL_EDIT,szString);
	sdfGetRegistryString("SA Builder","MSDevPath","C:\\Program Files\\Microsoft Visual Studio\\Common\\MSDev98\\Bin\\",szString,_MAX_PATH);
	SetDlgItemText(IDC_MSDEVPATH,szString);
	sdfGetRegistryString("SA Builder","MSBuildPath","C:\\WINDOWS\\Microsoft.NET\\Framework\\v2.0.50727\\",szString,_MAX_PATH);
	SetDlgItemText(IDC_MSBUILDPATH,szString);
	sdfGetRegistryString("SA Builder","TestPath","",szString,_MAX_PATH);
	SetDlgItemText(IDC_TESTPATH,szString);
	sdfGetRegistryString("SA Builder","NotepadPath","",szString,_MAX_PATH);
	SetDlgItemText(IDC_NOTEPATH,szString);
	sdfGetRegistryString("SA Builder","WebPagePath","",szString,_MAX_PATH);
	SetDlgItemText(IDC_WEB_PATH_EDIT,szString);
	sdfGetRegistryString("SA Builder","FTPUserName","",szString,_MAX_PATH);
	SetDlgItemText(IDC_FTP_USER_EDIT,szString);
	sdfGetRegistryString("SA Builder","FTPPassword","",szString,_MAX_PATH);
	SetDlgItemText(IDC_FTP_PASS_EDIT,DecryptString(CString(szString)));
  sdfGetRegistryString("SA Builder", "SvnLastRev", "816", szString, _MAX_PATH);
  m_szSvnLastRev = szString;
  sdfGetRegistryString("SA Builder", "SvnLastLabelRev", "816", szString, _MAX_PATH);
  m_szSvnLastLabelRev = szString;

	if(m_bBatchMode)
	{
		OnOK();
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSABuilderDlg::OnOK() 
{
	int nResult = 0;
  CString szResult;

  GetDialogStrings();
  WriteRegistryStrings();

	CheckForCodeChanges();
	if (!m_bSACodeChanges && !m_bCSharpCodeChanges && m_bBatchMode)
	{
		CDialog::OnOK();
		PostQuitMessage(0);
		return;
	}

  RemoveOldBuildFolders();
  InitializeLog();

	// verify and change to build folder
	if (!SetCurrentDirectory(m_szBuildPath))
	{
		if(!m_bBatchMode)
			MessageBox(m_szBuildPath + " is unavailable.", "Error", MB_OK|MB_ICONHAND);
		CDialog::OnOK();
		PostQuitMessage(0);
		return;
	}

  SetVersionStrings();
	CheckoutCode();
  SetCurrentDirectory(m_szBuildPath + (m_szSvnBranch == "Trunk" ? "Trunk" : "Branches/" + m_szSvnBranch));
  UpdateRC(m_szBuildPath + m_szSvnBranch + "\\Src\\SaExe\\SA.rc");
	LookupCommitterAddress();

	// Build
	if (m_szMSDevPath.GetLength() && (m_bCSharpCodeChanges || m_bSACodeChanges))
	{
		m_nErrors += BuildSaCSharpProjects();
    m_nErrors += BuildSA();
	}
	//if (m_szMSBuildPath.GetLength() && (m_bCSharpCodeChanges || m_bPACodeChanges))
  //	m_nErrors += BuildPA();

	// Set up for testing
	if(!m_nErrors && m_szTestPath.GetLength())
    m_nErrors += SetUpTest();

	// Build installer
	if(!m_nErrors && m_szWisePath.GetLength() && m_szSetupPath.GetLength())
    m_nErrors += BuildInstaller();

	// Distribute files
	if(!m_nErrors && m_szDestinationPath.GetLength())
    DistributeFiles();

  //CommitInstallerChanges();

	// Commit version change (label)
	if(!m_nErrors && m_bLabelVersion)
	{
    LabelVersion();
		m_szSvnLastLabelRev = m_szSvnHeadRev;
	}

  // Write strings to registry
  m_szSvnLastRev = m_szSvnHeadRev;
  WriteRegistryStrings();

	// Notify user
  CompleteLog();
  EmailResults();
  if (!m_bBatchMode)
    MessageBox("Build Complete!", "SA Builder", MB_OK|MB_ICONINFORMATION);

  CDialog::OnOK();
	PostQuitMessage(0);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSABuilderDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSABuilderDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSABuilderDlg::OnStoreButton() 
{
  GetDialogStrings();

  WriteRegistryStrings();

	if(!m_bBatchMode)
		PostQuitMessage(0);
}

void CSABuilderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

void CSABuilderDlg::Browse(CString &szDocumentName)
{
	HKEY hExt = NULL;
	int nExtLen;
	nExtLen = szDocumentName.GetLength() - szDocumentName.ReverseFind('.');
	CString szExt = szDocumentName.Right(nExtLen);
	if(RegCreateKey(HKEY_CLASSES_ROOT,szExt,&hExt)==ERROR_SUCCESS)
	{
		char szExtClassName[256] = "";
		long Length = 256;
		if(RegQueryValue(hExt,NULL,szExtClassName,&Length)==ERROR_SUCCESS)
		{
			HKEY hExtClass = NULL;
			if(RegCreateKey(HKEY_CLASSES_ROOT,szExtClassName,&hExtClass)==ERROR_SUCCESS)
			{
				HKEY hExtShellOpenCommand = NULL;
				if(RegCreateKey(hExtClass,"shell\\Open\\command",&hExtShellOpenCommand)==ERROR_SUCCESS)
				{
					char szExtCommand[_MAX_PATH];
					Length = _MAX_PATH;
					if(RegQueryValue(hExtShellOpenCommand,NULL,szExtCommand,&Length)==ERROR_SUCCESS)
					{
						CString szCommand = szExtCommand;
						CString szRestOfCommand = " ";
						if(*szExtCommand=='\"')
						{
							szCommand = szCommand.Right(szCommand.GetLength()-1);
							int nEnd = szCommand.FindOneOf("\"");
							if(nEnd)
							{
								szRestOfCommand = szCommand.Right(szCommand.GetLength()-nEnd-1);
								szRestOfCommand.TrimLeft();
								szCommand = szCommand.Left(nEnd);
							}
							
						}
						else
						{
							int nEnd = szCommand.FindOneOf(" /");
							if(nEnd)
							{
								szRestOfCommand = szCommand.Right(szCommand.GetLength()-nEnd-1);
								szCommand = szCommand.Left(nEnd);
							}
						}
						_spawnl(_P_NOWAIT, szCommand, szRestOfCommand, szDocumentName, NULL);
					}
				}
			}
		}
	}
}

int CSABuilderDlg::RemoveOldBuildFolders()
{
  int nErrors = 0;

  if (m_bDeleteFilesFirst)
  {
    nErrors += system("rd /s /q \"" + m_szBuildPath + m_szSvnBranch + "\\Src\"");
    nErrors += system("rd /s /q \"" + m_szBuildPath + m_szSvnBranch + "\\Output\"");
	  nErrors += CreateDirectory(m_szBuildPath + m_szSvnBranch + "\\Src", NULL);
	  nErrors += CreateDirectory(m_szBuildPath + m_szSvnBranch + "\\Output", NULL);

  }
  return (nErrors == 0);
}

int CSABuilderDlg::InitializeLog()
{
  m_szLogFilePath = m_szBuildPath + m_szSvnBranch + "\\SaBuilder.log";
	DeleteFile(m_szLogFilePath);

	CTime time = CTime::GetCurrentTime();

  return Log(time.Format("%a %x"), "***** Build started *****");
}

int CSABuilderDlg::CompleteLog()
{
	CTime time = CTime::GetCurrentTime();

  return Log(time.Format("%a %x"), "***** Build complete *****\r\n");
}

int CSABuilderDlg::CheckoutCode()
{
  // checkout Src folder
	CString szSvnBranchURL = m_szSvnRepoURL;
  szSvnBranchURL += (m_szSvnBranch == "Trunk" ? "Trunk/Src" : "Branches/" + m_szSvnBranch + "/Src");
  CString szCommandString = "svn checkout " + szSvnBranchURL + " " + m_szSvnBranch + "\\Src --non-interactive --username " + m_szSvnUser + " --password "+ m_szSvnPass;
  char* szCommand = szCommandString.GetBuffer(szCommandString.GetLength());
  char* szConsoleOutput = new char[65536];
  int nResult = RunConsoleCommand(szCommand, szConsoleOutput);

	// format results
  CString szResult;
  szResult.Format("ERR%d", nResult);
	Log((nResult==0) ? "OK  " : szResult, "Checkout Program Source");
	if(nResult != 0)
		m_nErrors++;

	delete [] szConsoleOutput;

  return (nResult == 0);
}

int CSABuilderDlg::BuildSaCSharpProjects()
{
	int nResult = 0;
  CString szResult = "";
	int nErrors = 0;
  int nWarnings = 0;
  
	CString szCommandString = m_szMSBuildPath;
	szCommandString += "MSBuild.exe ";
	szCommandString += "Src\\SaC#Projects.sln ";
	szCommandString += "/property:Configuration=Release";
  char* szCommand = szCommandString.GetBuffer(szCommandString.GetLength());
  char* szConsoleOutput = new char[65536];
  nResult = RunConsoleCommand(szCommand, szConsoleOutput, "SaCSharp-MSBuild.log");
  nErrors += nResult;
  szResult.Format("ERR%d", nResult);
	Log((nResult==0) ? "OK  " : szResult, "Build SA C# Projects");

	// Create SpeechToolsUtils.reg
	LPTSTR szWindowsPath = new char[MAX_PATH + 1];
	GetWindowsDirectory(szWindowsPath, MAX_PATH + 1);
  szCommandString = szWindowsPath;
	szCommandString += "\\Microsoft.NET\\Framework\\v2.0.50727\\RegAsm.exe";
	szCommandString += " /regfile Output\\Release\\SpeechToolsUtils.dll";
  szCommand = szCommandString.GetBuffer(szCommandString.GetLength());
  nResult = RunConsoleCommand(szCommand, szConsoleOutput);
  szResult.Format("ERR%d", nResult);
	Log((nResult==0) ? "OK  " : szResult, "Create SpeechToolsUtils.reg");

	// e-mail log
	if (nResult != 0)
		EmailFile(m_szBuildPath + m_szSvnBranch + "\\SaCSharp-MSBuild.log", m_szCommitterAddress, "C# Projects Build Log");

	delete [] szConsoleOutput;
	delete [] szWindowsPath;
  
  return nErrors;
}

int CSABuilderDlg::BuildSA()
{
	int nResult = 0;
  CString szResult = "";
	int nErrors = 0;
  
	if(nResult != 0)
		nErrors++;
  nResult = _spawnl(_P_WAIT, m_szMSDevPath + "MSDev.exe", "MSDev.exe", "Src\\SaExe\\SAUnicode.dsw", "/MAKE \"SaUnicode - Win32 URelease\"", "/REBUILD", "/OUT Sa-MSDEV.log", NULL);
  szResult.Format("ERR%d", nResult);
	Log((nResult==0) ? "OK  " : szResult, "Build SA URelease");

	// if Release build fails, build for Debug
	if (nResult != 0)
	{
		nErrors++;

		// e-mail release log file
		EmailFile(m_szBuildPath + m_szSvnBranch + "\\Sa-MSDEV.log", m_szCommitterAddress, "SA Build Log");

		// build debug version
		nResult = _spawnl(_P_WAIT, m_szMSDevPath + "MSDev.exe", "MSDev.exe", "Src\\SaExe\\SAUnicode.dsw", "/MAKE \"SaUnicode - Win32 UDebug\"", "/REBUILD", "/OUT SaDebug-MSDEV.log", NULL);
		szResult.Format("ERR%d", nResult);
		Log((nResult==0) ? "OK  " : szResult, "Build SA UDebug");
		if(nResult != 0)
		{
			nErrors++;
			EmailFile(m_szBuildPath + m_szSvnBranch + "\\SaDebug-MSDEV.log", m_szCommitterAddress, "SA Debug Build Log");
		}
	}

  //////////////////////////////////////////////////// Phil's error checking code:
	char tline[MAX_LEN], lline[MAX_LEN];
	ifstream infile;
	infile.open("Sa-MSDEV.log", ios::in,filebuf::openprot);
	if(infile.is_open())
	{	while(infile.good())
		{	strcpy(lline,tline);
			infile.getline(tline,MAX_LEN);
		}
	}
	if(tline[0] != NULL)
		strcpy(lline,tline);
	else if(lline[0] != NULL)
		strcpy(tline,lline);

	tline[10] = NULL;
	if( (strcmp(tline, "SA.exe - 0")) != 0 )
		OnBuildError(lline, m_szBuildPath);

  ////////////////////////////////////////////////////

  return nErrors;
}

int CSABuilderDlg::BuildPA()
{
  CString szResult = "";
	int nErrors = 0;
  int nWarnings = 0;
  int nResult;

  // get current directory
	LPTSTR szCurrentDir = new TCHAR[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szCurrentDir);
	SetCurrentDirectory("src\\pa");

	// build for release
	CString szCommandString = m_szMSBuildPath;
	szCommandString += "MSBuild.exe ";
	szCommandString += "Pa.sln ";
	szCommandString += "/property:Configuration=Release";
  char* szCommand = szCommandString.GetBuffer(szCommandString.GetLength());
  char* szConsoleOutput = new char[65536];
	CString szOutFile = szCurrentDir;
	szOutFile += "\\Pa-MSBuild.log";
  nResult = RunConsoleCommand(szCommand, szConsoleOutput, szOutFile.GetBuffer(szOutFile.GetLength()));

  nErrors += nResult;
  szResult.Format("ERR%d", nResult);
	Log((nResult==0) ? "OK  " : szResult, "Build PA");

  if (nErrors)
	{
		// e-mail build log
		EmailFile(m_szBuildPath + m_szSvnBranch + "\\Pa-MSBuild.log", m_szCommitterAddress, "PA Build Log");

		// try building for debug
		szCommandString = szCommandString.Left(szCommandString.ReverseFind('=')) + "=Debug";
		szCommand = szCommandString.GetBuffer(szCommandString.GetLength());
		szConsoleOutput = new char[65536];
		CString szOutFile = szCurrentDir;
		szOutFile += "\\PaDebug-MSBuild.log";
	  nResult = RunConsoleCommand(szCommand, szConsoleOutput, szOutFile.GetBuffer(szOutFile.GetLength()));

		nErrors += nResult;
		szResult.Format("ERR%d", nResult);
		Log((nResult==0) ? "OK  " : szResult, "Build PA (Debug)");
		if (nResult)
			EmailFile(m_szBuildPath + m_szSvnBranch + "\\PaDebug-MSBuild.log", m_szCommitterAddress, "PA Debug Build Log");
	}

	m_nErrors += nErrors;
	SetCurrentDirectory(szCurrentDir);
	delete szCurrentDir;

  return nErrors;
}

int CSABuilderDlg::BuildInstaller()
{
  int nErrors = 0;

/*
  if (m_szVersionComment != szCurrentSSVersion)
  {
    nResult = CopyFile(m_szBuildPath + "Release\\SA2.exe", m_szSetupPath + "SA\\SA2.exe", FALSE);
		Log("Copy SA for install\t\t\t" + ((nResult!=0) ? "OK  " : "Error "+ CString(itoa(nResult, szResult, 10))));
		if(nResult == 0)
			nErrors++;
		nResult = CopyFile(m_szBuildPath + "ActiveX_Release\\Speech Analyzer.ocx", m_szSetupPath + "SA\\Speech Analyzer.ocx", FALSE);
		Log("Copy OCX for install\t\t\t" + ((nResult!=0) ? "OK  " : "Error "+ CString(itoa(nResult, szResult, 10))));
		if(nResult == 0)
			nErrors++;
		nResult = CopyFile(m_szBuildPath + "SaShell\\SaShell___Win32_ActiveX_Release\\SaShell.exe", m_szSetupPath + "SA\\SaShell.exe", FALSE);
		Log("Copy SaShell for install\t\t" + ((nResult!=0) ? "OK  " : "Error "+ CString(itoa(nResult, szResult, 10))));
		if(nResult == 0)
			nErrors++;
		//nResult = CopyFile(m_szBuildPath + "Release\\SA_ENU.dll", m_szSetupPath + "SA\\SA_ENU.dll", FALSE);
		//Log("Copy SA_ENU.dll for install\t\t" + ((nResult!=0) ? "OK  " : "Error "+ CString(itoa(nResult, szResult, 10))));
		nResult = _spawnl(_P_WAIT, m_szWisePath + "INSTALLBUILDER.EXE", "INSTALLBUILDER.EXE", "/c", m_szSetupPath + "Setups\\Setup-ST.wse", NULL);
		Log("Build Setup-ST\t\t\t\t" + ((nResult==0) ? "OK  " : "Error "+ CString(itoa(nResult, szResult, 10))));
		if(nResult != 0)
			nErrors++;
		MessageLoop(120000);
  }
*/

  return nErrors;
}

/***************************************************************************/
// CSABuilderDlg::CheckForCodeChanges Check for code changes
// since the last build.
// 
// Return value:	 >0		changes exist (returns # of changes)
//					        0		no changes
/***************************************************************************/
int CSABuilderDlg::CheckForCodeChanges()
{
  // Check most recent source-safe item date/time
  CString szSvnBranchURL = m_szSvnRepoURL;
  szSvnBranchURL += (m_szSvnBranch == "Trunk" ? "Trunk/Src" : "Branches/" + m_szSvnBranch + "/Src");
  CString szCommandString = "svn log " + szSvnBranchURL + " -v -r HEAD:" + m_szSvnLastRev + " --username " + m_szSvnUser + " --password "+ m_szSvnPass;
  char* szCommand = szCommandString.GetBuffer(szCommandString.GetLength());
  char* szConsoleOutput = new char[65536];
	strcpy(szConsoleOutput, "");
	char* szLogFilePath = new char[256];
	strcpy(szLogFilePath, m_szBuildPath.GetBuffer(m_szBuildPath.GetLength() + 1));
	strcat(szLogFilePath, "SvnLog.txt");
  int nReturn = RunConsoleCommand(szCommand, szConsoleOutput, szLogFilePath);
  CString szLog(szConsoleOutput);
	// Check if log request succeeded
	if (szLog.GetLength() == 0)
	{
		//EmailFile("", "corey_wenger@sil.org", "SVN unavailable");
		delete [] szConsoleOutput;
		return 0;
	}
  // Compare head rev with last built rev
  int nRevStart = szLog.Find("r") + 1;
  int nRevLength = szLog.Find(" ", nRevStart) - nRevStart;
  m_szSvnHeadRev = szLog.Mid(nRevStart, nRevLength);
  int nCodeChanges = atoi((LPCSTR)m_szSvnHeadRev) - atoi((LPCSTR)m_szSvnLastRev);
	if (m_bLabelVersion)
		nCodeChanges += atoi((LPCSTR)m_szSvnLastRev) - atoi((LPCSTR)m_szSvnLastLabelRev);
	int nLastRevIndex = szLog.Find("r" + m_szSvnLastRev +" ");
	nLastRevIndex = nLastRevIndex > -1 ? nLastRevIndex : szLog.GetLength();
	CString szLogAfterLastRev = szLog.Left(nLastRevIndex); // cut off log entry of last rev
	if (nCodeChanges > 0)
	{
		m_bSACodeChanges = TRUE;
		//m_bCSharpCodeChanges = szLogAfterLastRev.Find("src/common/SpeechToolsUtils") >= 0;
		//m_bSACodeChanges = szLogAfterLastRev.Find("src/sa") >= 0;
		//m_bSACodeChanges = m_bSACodeChanges || (szLogAfterLastRev.Find("src/common/ST_Audio") >= 0);
		//m_bSACodeChanges = (m_bSACodeChanges || (szLogAfterLastRev.Find("src/common/yeti") >= 0));
		//m_bPACodeChanges = szLogAfterLastRev.Find("src/pa") >= 0;
		//m_bPACodeChanges = (m_bPACodeChanges || (szLogAfterLastRev.Find("src/common") >= 0)
		//										&& !m_bSACodeChanges && !m_bCSharpCodeChanges);
	}

	// if this is a label version build and there have been incremental builds, assume code changes
	if ((m_bLabelVersion) && (m_szSvnLastLabelRev < m_szSvnLastRev))
	{
		m_bCSharpCodeChanges = TRUE;
		m_bSACodeChanges = TRUE;
	}
  
	delete [] szConsoleOutput;

  return nCodeChanges;
}

int CSABuilderDlg::CommitChanges()
{
  CString szCommandString = "svn commit -m \"SA" + m_szVersion + "\" --non-interactive --username " + m_szSvnUser + " --password "+ m_szSvnPass;
  char* szCommand = szCommandString.GetBuffer(szCommandString.GetLength());
  char* szConsoleOutput = new char[65536];
  int nResult = RunConsoleCommand(szCommand, szConsoleOutput);
  CString szResult;
  szResult.Format("ERR%d", nResult);
	Log((nResult==0) ? "OK  " : szResult, "Commit code changes");

  // test for errors
  int nSvnLastRev = atoi((LPCSTR)m_szSvnLastRev);
  if (nResult == 0)
  {
    m_szSvnLastRev.Format("%d", nSvnLastRev + CheckForCodeChanges()); // update last built rev level
  }

  delete [] szConsoleOutput;

  return nResult;
}

CString CSABuilderDlg::DecryptString(CString &szEncryptedText)
{
	// decrypt password
  CString szDecryptedText = szEncryptedText;
	for(int n=0; n<szEncryptedText.GetLength(); n++)
	{
		szDecryptedText.SetAt(n, (char)((int)szEncryptedText[n] - 4));
	}

  return szDecryptedText;
}

int CSABuilderDlg::DistributeFiles()
{
	int nResult = 0;
	int nErrors = 0;
	CString szSource = m_szBuildPath + m_szSvnBranch + "\\Output\\Release\\";

	CheckOutputFileCount();
	
  if (m_bSACodeChanges || m_bCSharpCodeChanges)
	{
		// Create folder
		m_szDestinationFolderName = "SA" + m_szVersion;
		if (!m_bLabelVersion)
			m_szDestinationFolderName += " (Rev " + m_szSvnHeadRev + ")";
		if(m_szRC.GetLength())
			m_szDestinationFolderName += "RC" + m_szRC;
		if(m_szDestinationPath.GetLength())
			nResult = CreateDirectory(m_szDestinationPath + m_szDestinationFolderName, NULL);
		//CString szEXEName = "SA" + m_szVersion + ".exe";

		// copy to Speech Tools network share
		CString szDest = m_szDestinationPath + m_szDestinationFolderName + "\\";
		Log("", "Copying to " + szDest);
		nErrors += Copy(szSource, szDest, "SA.exe");
		nErrors += Copy(szSource, szDest, "SA_ENU.dll");
		nErrors += Copy(szSource, szDest, "mbrola.dll");
		nErrors += Copy(szSource, szDest, "SA_DSP.dll");
		nErrors += Copy(szSource, szDest, "sfSettings.dll");
		nErrors += Copy(szSource, szDest, "zGraph.dll");
		nErrors += Copy(szSource, szDest, "SpeechToolsUtils.dll");
		nErrors += Copy(szSource, szDest, "SpeechToolsUtils.reg");
		nErrors += Copy(szSource, szDest, "ST_Audio.dll");
		nErrors += Copy(szSource, szDest, "yeti.mmedia.dll");
		nErrors += Copy(szSource, szDest, "yeti.wmfsdk.dll");
	}

	// TODO: Distribute SVN log

  // CopyToWebSite();

  m_nErrors += nErrors;

  return (nErrors == 0);
}

///////////////////////////////////////////////////////////////////////////////
// UpdateInstallerFiles Updates installer files from installer SVN repository 
///////////////////////////////////////////////////////////////////////////////
int CSABuilderDlg::UpdateInstallerFilesFromSVN(CString szInstallerFilesPath)
{
  CString szCommandString = "svn update \"" + szInstallerFilesPath + "\\Speech Analyzer\" --non-interactive";
  char* szCommand = szCommandString.GetBuffer(szCommandString.GetLength());
  char* szConsoleOutput = new char[65536];
  int nResult = RunConsoleCommand(szCommand, szConsoleOutput);

  CString szResult;
  szResult.Format("ERR%d", nResult);
	Log((nResult==0) ? "OK  " : szResult, "Update installer files");
	delete [] szConsoleOutput;
	
	return nResult;
}

///////////////////////////////////////////////////////////////////////////////
// CommitInstallerChanges Commits build outputs to installer SVN repository 
///////////////////////////////////////////////////////////////////////////////
int CSABuilderDlg::CommitInstallerChanges()
{
	CString szInstallerFilesPath = "C:\\speechtools\\Installer Files";
	int nResult = 0;

	UpdateInstallerFilesFromSVN(szInstallerFilesPath);

	// copy files to installer folder
	CString szSource = m_szBuildPath + m_szSvnBranch + "\\Output\\Release\\";
	CString szDest = szInstallerFilesPath + "\\Speech Analyzer\\";
	Log("", "Copying to " + szDest);
	nResult += Copy(szSource, szDest, "SA.exe");
	nResult += Copy(szSource, szDest, "SA_ENU.dll");
	nResult += Copy(szSource, szDest, "mbrola.dll");
	nResult += Copy(szSource, szDest, "SA_DSP.dll");
	nResult += Copy(szSource, szDest, "sfSettings.dll");
	nResult += Copy(szSource, szDest, "zGraph.dll");
	nResult += Copy(szSource, szDest, "SpeechToolsUtils.dll");
	nResult += Copy(szSource, szDest, "ST_Audio.dll");
	nResult += Copy(szSource, szDest, "yeti.mmedia.dll");
	nResult += Copy(szSource, szDest, "yeti.wmfsdk.dll");

  // Commit changes to installer repository
	CString szCommandString = "svn commit \"" + szInstallerFilesPath + "\" -m \"svn.sil.org r" + m_szSvnHeadRev + "\" --non-interactive --username wenger --password svnwenger";
  char* szCommand = szCommandString.GetBuffer(szCommandString.GetLength());
  char* szConsoleOutput = new char[65536];
  nResult += RunConsoleCommand(szCommand, szConsoleOutput);
  CString szResult;
  szResult.Format("ERR%d", nResult);
	Log((nResult==0) ? "OK  " : szResult, "Commit installer files");
	delete [] szConsoleOutput;
	
	return nResult;
}

///////////////////////////////////////////////////////////////////////////////
// CheckOutputFileCount Checks the count of installer files to see if
// anything was added or deleted.
///////////////////////////////////////////////////////////////////////////////
int CSABuilderDlg::CheckOutputFileCount()
{
	WIN32_FIND_DATA FileData;
	HANDLE hSearch;
	CString szSearchPath = m_szBuildPath + m_szSvnBranch + "\\Output\\Release\\*.*";
	int nResult = 0;
	BOOL bSearching = TRUE;
	CString szFileNameNoPeriods;

	hSearch = FindFirstFile(szSearchPath, &FileData);
	if (hSearch == INVALID_HANDLE_VALUE)
		return 0;
	
	while (bSearching)
	{
		// dont count . and ..
		szFileNameNoPeriods = FileData.cFileName;
		szFileNameNoPeriods.Remove(_T('.'));
		if (szFileNameNoPeriods.GetLength())
			nResult++;

		bSearching = FindNextFile(hSearch, &FileData);
	}

	int nExpectedCount = 7; // SpeechToolsUtils
	nExpectedCount += 26; // SA files

	CString szResult = (nResult == nExpectedCount ? "OK" : "WARN");
	if (nResult != nExpectedCount)
		m_nWarnings++;
	CString szMessage;
	szMessage.Format("Checking output folder: %d files found (%d expected)", nResult, nExpectedCount);
	Log(szResult, szMessage);

	return nResult;
}

int CSABuilderDlg::Copy(CString szSourcePath, CString szDestPath, CString szFileName)
{
  // copy the file
	int nResult = CopyFile(szSourcePath + szFileName, szDestPath + szFileName, FALSE);
  
	// log the results
	CString szErrorResult;
  szErrorResult.Format("ERR%d", nResult);
	CString szAction = "Copy ";
	szAction += szFileName;
	Log((nResult != 0) ? "OK  " : szErrorResult, szAction);

	return (nResult == 0);
}

int CSABuilderDlg::CopyToWebSite()
{
	/*
  if(m_szWebPagePath.GetLength() && m_szFTPUser.GetLength() && m_szFTPPass.GetLength())
	{
    // Hard-coded parameters (should have edit boxes)
	  CString szWinZipPath = "F:\\Program Files\\WinZip\\";
	  CString szDocumentName = "http://www.sil.org/computing/speechtools/Terry/terry.htm";

  	// open ftp connection
		int i = 0;
		CInternetSession isWWWSILORG("SA Builder");
		CFtpConnection* pFTP = isWWWSILORG.GetFtpConnection("www.sil.org", m_szFTPUser, m_szFTPPass);
		while((!pFTP) && (i<13))
		{
			CFtpConnection* pFTP = isWWWSILORG.GetFtpConnection("www.sil.org", m_szFTPUser, m_szFTPPass);
			MessageLoop((ULONG)(pow(2,i))*500);
			++i;
		}

		// Upload to web server
		if(pFTP && szVersionComment != szCurrentSSVersion)
		{
			// Build strings
	    m_szZIPFILEVERSION = m_szVersionMajor + "-" + m_szVersionMinor + "-" + m_szVersionBuild;
			CString szHTMLPath = "\"" + m_szSABuilderPath + "Terry.htm\"";
			CString szEXEZIPPath = "\"" + m_szBuildPath + "Release\\SA2.zip";
			CString szOCXZIPPath = "\"" + m_szBuildPath + "ActiveX_Release\\speech analyzer.zip\"";
			CString szShellZIPPath = "\"" + m_szBuildPath + "SaShell\\SaShell___Win32_ActiveX_Release\\SaShell.zip\"";
			CString szSetupEXEPath = "\"" + m_szSetupPath + "Setups\\Setup-SA.exe\"";
			CString SetupSAmin2xEXE = "\"" + m_szSetupPath + "Setups\\Setup-SAmin.exe\"";
			CString szSetupSTEXEPath = "\"" + m_szSetupPath + "Setups\\Setup-ST.exe\"";
			// Prepare distributables for uploading
			Log(CString("Zip..."));
			nResult = _spawnl(_P_WAIT, szWinZipPath + "wzzip.exe", "wzzip", "/a", szEXEZIPPath, "\"" + m_szBuildPath + "Release\\SA2.exe", NULL);
			Log("...SA\t\t\t\t\t" + ((nResult==0) ? "OK  " : (CString(itoa(nResult, szResult, 10)) + " errors")));
			if(nResult != 0)
				nErrors++;
			MessageLoop(100);
			nResult = _spawnl(_P_WAIT, szWinZipPath + "wzzip.exe", "wzzip", "/a", "\"" + szOCXZIPPath + "\"", "\"" + m_szBuildPath + "ActiveX_Release\\speech analyzer.ocx" + "\"", NULL);
			Log("...OCX\t\t\t\t\t" + ((nResult==0) ? "OK  " : (CString(itoa(nResult, szResult, 10)) + " errors")));
			if(nResult != 0)
				nErrors++;
			MessageLoop(100);
			nResult = _spawnl(_P_WAIT, szWinZipPath + "wzzip.exe", "wzzip", "/a", szShellZIPPath, "\"" + m_szBuildPath + "SaShell\\SaShell___Win32_ActiveX_Release\\SaShell.exe\"", NULL);
			Log("...SaShell\t\t\t\t" + ((nResult==0) ? "OK  " : (CString(itoa(nResult, szResult, 10)) + " errors")));
			if(nResult != 0)
				nErrors++;
			MessageLoop(100);
			// Make backup copy on network
			nResult = CopyFile(szEXEZIPPath, m_szDestinationPath + m_szDestinationFolderName + "\\" + "SA" + m_szZIPFILEVERSION + ".zip", FALSE);
			Log("Backup SA.zip\t\t\t\t\t" + ((nResult!=0) ? "OK  " : "Error "+ CString(itoa(nResult, szResult, 10))));
			if(nResult == 0)
				nErrors++;
			nResult = CopyFile(szOCXZIPPath, m_szDestinationPath + m_szDestinationFolderName + "\\" + "OCX" + m_szZIPFILEVERSION + ".zip", FALSE);
			Log("Backup OCX.zip\t\t\t\t\t" + ((nResult!=0) ? "OK  " : "Error "+ CString(itoa(nResult, szResult, 10))));
			if(nResult == 0)
				nErrors++;
			nResult = CopyFile(szShellZIPPath, m_szDestinationPath + m_szDestinationFolderName + "\\" + "SaShell" + m_szZIPFILEVERSION + ".zip", FALSE);
			Log("Backup SaShell.zip\t\t\t\t\t" + ((nResult!=0) ? "OK  " : "Error "+ CString(itoa(nResult, szResult, 10))));
			if(nResult == 0)
				nErrors++;
			// Download current version of web page
			nResult = pFTP->SetCurrentDirectory(m_szWebPagePath);
			Log("Open web folder\t\t\t\t" + ((nResult!=0) ? "OK  " : "Error "+ CString(itoa(nResult, szResult, 10))));
			if(nResult == 0)
				nErrors++;
			// nResult = pFTP->GetFile("terry.htm", m_szSABuilderPath + "terry.htm", FALSE, FILE_ATTRIBUTE_NORMAL, FTP_TRANSFER_TYPE_ASCII);
			// Log("Download web page\t\t\t" + ((nResult!=0) ? "OK  " : "Error "+ CString(itoa(nResult, szResult, 10))));
			// Update Web page
			// tTime = CTime::GetCurrentTime();
			// CString szTimeDate = tTime.Format("%a %#m/%#d/%y %#I:%M%p EST");
			// UpdateHTML(szHTMLPath, szEXEZIPPath, szOCXZIPPath, szShellZIPPath, szSetupEXEPath, SetupSAmin2xEXE, szSetupSTEXEPath, szTimeDate);
			
			// Delete old files
			CString szFileName = "";
			CFtpFileFind finder(pFTP);
			if(finder.FindFile())
			{
				while(finder.FindNextFile())
				{
					szFileName = finder.GetFileName();
					szFileName.MakeUpper();
					if((szFileName=="SA" + szCurrentSSVersion + ".ZIP" 
						|| szFileName=="OCX" + szCurrentSSVersion + ".ZIP"
						|| szFileName=="SASHELL" + szCurrentSSVersion + ".ZIP"
						|| szFileName=="SETUP-SAMIN" + szCurrentSSVersion + ".EXE"))
					{
							nResult = pFTP->Remove(szFileName);
							Log("Remove " + szFileName + "\t\t\t" + ((nResult!=0) ? "OK  " : ("Error "+ CString(itoa(nResult, szResult, 10)))));
							if(nResult == 0)
								nErrors++;
					}
				}
			}
			// Upload web page and distributables to web server
			// nResult = pFTP->PutFile(szHTMLPath, "terry.htm", FTP_TRANSFER_TYPE_ASCII);
			// Log("Upload web page\t\t\t\t" + ((nResult!=0) ? "OK  " : "Error "+ CString(itoa(nResult, szResult, 10))));
			// MessageLoop(100);
			nResult = pFTP->PutFile(szEXEZIPPath, CString("SA" + m_szZIPFILEVERSION + ".zip"), FTP_TRANSFER_TYPE_BINARY);
			Log("Upload SA\t\t\t\t" + ((nResult!=0) ? "OK  " : "Error "+ CString(itoa(nResult, szResult, 10))));
			if(nResult == 0)
				nErrors++;
			MessageLoop(100);
			nResult = pFTP->PutFile(szOCXZIPPath, CString("OCX" + m_szZIPFILEVERSION + ".zip"), FTP_TRANSFER_TYPE_BINARY);
			Log("Upload OCX\t\t\t\t" + ((nResult!=0) ? "OK  " : "Error "+ CString(itoa(nResult, szResult, 10))));
			if(nResult == 0)
				nErrors++;
			MessageLoop(100);
			nResult = pFTP->PutFile(szShellZIPPath, CString("SaShell" + m_szZIPFILEVERSION + ".zip"), FTP_TRANSFER_TYPE_BINARY);
			Log("Upload SaShell\t\t\t\t" + ((nResult!=0) ? "OK  " : "Error "+ CString(itoa(nResult, szResult, 10))));
			if(nResult == 0)
				nErrors++;
			MessageLoop(100);
			//nResult = pFTP->PutFile(szSetupEXEPath, CString("Setup-SA" + m_szZIPFILEVERSION + ".exe"), FTP_TRANSFER_TYPE_BINARY);
			//Log("Upload Setup-SA\t\t\t" + ((nResult!=0) ? "OK  " : "Error "+ CString(itoa(nResult, szResult, 10))));
			//MessageLoop(100);
			nResult = pFTP->PutFile(SetupSAmin2xEXE, CString("Setup-SAmin" +m_ szZIPFILEVERSION + ".exe"), FTP_TRANSFER_TYPE_BINARY);
			Log("Upload Setup-SAmin\t\t\t" + ((nResult!=0) ? "OK  " : "Error "+ CString(itoa(nResult, szResult, 10))));
			if(nResult == 0)
				nErrors++;
			MessageLoop(100);
			//nResult = pFTP->PutFile(szSetupSTEXEPath + "Web Setups\\", CString("Setup-ST" + m_szZIPFILEVERSION + ".exe"), FTP_TRANSFER_TYPE_BINARY);
			//Log("Upload Setup-ST\t\t\t" + ((nResult!=0) ? "OK  " : "Error "+ CString(itoa(nResult, szResult, 10))));
			//MessageLoop(100);
			// Display web page
			Browse(szDocumentName);
		}

		// Close ftp connection
		if (pFTP != NULL)
		{
			pFTP->Close();
			delete pFTP;
			pFTP = NULL;
		}
		else
		{
			Log(CString("FTP connection failed -- no files uploaded"));
			nErrors++;
		}
	}
	*/

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// LookupCommitterAddress 
///////////////////////////////////////////////////////////////////////////////
void CSABuilderDlg::LookupCommitterAddress()
{
	char pszAddressLookup[3][2][30] = {{"david_olson","david_olson.org"},
						{"steve_maclean", "steve_maclean@kastanet.net"},
						{"todd_jones","todd_jones@sil.org"}};
	CFile svnLog;
	char pbuf[100];
	CString szSvnLogPath = m_szBuildPath;
	szSvnLogPath += "SvnLog.txt";

	// Get SVN username
	svnLog.Open(szSvnLogPath, CFile::modeRead);
	UINT nBytes = svnLog.Read(pbuf, 100);
	svnLog.Close();

	CString szLog = pbuf;
  int nUserStart = szLog.Find(" | ") + 3;
  int nUserLength = szLog.Find(" ", nUserStart) - nUserStart;
  CString szUser = szLog.Mid(nUserStart, nUserLength);

	// Lookup user's address
	m_szCommitterAddress = "";
	for (int i = 0; i < 3; i++)
	{
		if (pszAddressLookup[i][0] == szUser)
		{
			m_szCommitterAddress = pszAddressLookup[i][1];
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// EmailFile 
///////////////////////////////////////////////////////////////////////////////
int CSABuilderDlg::EmailFile(CString file, CString recipient, CString subject)
{
  // find mail server
	CString szCommandString = "ping /n 1 ma2.jaars.org";
  char* szCommand = szCommandString.GetBuffer(szCommandString.GetLength());
  char* szConsoleOutput = new char[65536];
  int nResult = RunConsoleCommand(szCommand, szConsoleOutput);
  CString szPingResults(szConsoleOutput, 500);
  int nStart = szPingResults.Find("[") + 1;
  int nEnd = szPingResults.Find("]");
  CString szSvrAddr = szPingResults.Mid(nStart, nEnd - nStart);
	if(nResult != 0)
		m_nErrors++;

  // send the file
	szCommandString = m_szSABuilderPath + "bmail.exe" + 
		                " -s " + szSvrAddr +									// mail server
		                " -t " + recipient +									// recipients, separated by commas
                    " -f corey_wenger@sil.org" +					// from address
                    " -a \"" + subject + "\"";						// subject
	if (file.GetLength())
			szCommandString += " -m \"" + file + "\"";					// file to use as body
	szCommandString += " -c";																// add CR/LF to separate header from body

  szCommand = szCommandString.GetBuffer(szCommandString.GetLength());

  nResult = RunConsoleCommand(szCommand, szConsoleOutput, "Sa-bmail.log");
	delete [] szConsoleOutput;

	return nResult;
}

///////////////////////////////////////////////////////////////////////////////
// EmailResults 
///////////////////////////////////////////////////////////////////////////////
int CSABuilderDlg::EmailResults()
{
	// Check destination folder contents
	int nResult;
  // nResult = SetCurrentDirectory(m_szBuildPath);
	nResult = system("dir \"" + m_szDestinationPath + m_szDestinationFolderName + "\" > dir.txt");

	// build subject line
	CString szSuccess = "";
  CString szErrors = "";
	if(!m_nErrors) // build successful
  {
    if (m_nWarnings)
      szErrors.Format(" (%d Warnings)", m_nWarnings);
    szSuccess = "Succeeded";
  }
	else // errors reported
  {
    szErrors.Format(" (%d Errors, %d Warnings)", m_nErrors, m_nWarnings);
		szSuccess = "Failed";
  }
	//CString szSubject = "SA " + m_szVersion;
	//if (!m_bLabelVersion)
	//	szSubject += " (Rev " + m_szSvnHeadRev + ")";
	CString szSubject = "r" + m_szSvnHeadRev;
	szSubject += " - Build " + szSuccess + szErrors;

	// e-mail committer
	nResult = EmailFile(m_szLogFilePath, m_szCommitterAddress, szSubject);
	
  return nResult;
}

CString CSABuilderDlg::EncryptString(CString &szDecryptedText)
{
	// encrypt for registry
	CString szEncryptedText = szDecryptedText;
	for(int n=0; n<szDecryptedText.GetLength(); n++)
	{
		szEncryptedText.SetAt(n, (char)((int)szDecryptedText[n] + 4));
	}
  return szEncryptedText;
}

void CSABuilderDlg::GetDialogStrings()
{
	GetDlgItemText(IDC_BRANCH_EDIT,m_szSvnBranch);
	GetDlgItemText(IDC_VERSION_MAJOR_EDIT,m_szVersionMajor);
	GetDlgItemText(IDC_VERSION_MINOR_EDIT,m_szVersionMinor);
  GetDlgItemText(IDC_RC_EDIT,m_szRC);
	GetDlgItemText(IDC_DEST_PATH_EDIT,m_szDestinationPath);
	if(m_szDestinationPath.GetLength() && m_szDestinationPath.Right(1) != "\\")
		m_szDestinationPath += "\\";
	GetDlgItemText(IDC_SA_PATH_EDIT,m_szBuildPath);
	if(m_szBuildPath.GetLength() && m_szBuildPath.Right(1) != "\\")
		m_szBuildPath += "\\";
	GetDlgItemText(IDC_SVN_EXE_PATH_EDIT,m_szSvnExePath);
	if(m_szSvnExePath.GetLength() && m_szSvnExePath.Right(1) != "\\")
		m_szSvnExePath += "\\";
	GetDlgItemText(IDC_SVN_USER_EDIT,m_szSvnUser);
	GetDlgItemText(IDC_SVN_PASS_EDIT,m_szSvnPass);
	GetDlgItemText(IDC_SVN_REPO_URL_EDIT,m_szSvnRepoURL);
	GetDlgItemText(IDC_MSDEVPATH,m_szMSDevPath);
	if(m_szMSDevPath.GetLength() && m_szMSDevPath.Right(1) != "\\")
		m_szMSDevPath += "\\";
	GetDlgItemText(IDC_MSBUILDPATH,m_szMSBuildPath);
	if(m_szMSBuildPath.GetLength() && m_szMSBuildPath.Right(1) != "\\")
		m_szMSBuildPath += "\\";
	GetDlgItemText(IDC_NOTEPATH,m_szNotepadPath);
	if(m_szNotepadPath.GetLength() && m_szNotepadPath.Right(1) != "\\")
		m_szNotepadPath += "\\";
	GetDlgItemText(IDC_TESTPATH,m_szTestPath);
	if(m_szTestPath.GetLength() && m_szTestPath.Right(1) != "\\")
		m_szTestPath += "\\";
	GetDlgItemText(IDC_WEB_PATH_EDIT,m_szWebPagePath);
	if(m_szWebPagePath.GetLength() && m_szWebPagePath.Right(1) != "\\")
		m_szWebPagePath += "\\";
	GetDlgItemText(IDC_FTP_USER_EDIT,m_szFTPUser);
	GetDlgItemText(IDC_FTP_PASS_EDIT,m_szFTPPass);
}

void CSABuilderDlg::SetVersionStrings()
{
	CTime time = CTime::GetCurrentTime();
  // m_szVersionYear = time.Format("%Y");
  // m_szVersionDate = time.Format("%m%d");
  
  // Compose strings for SA.RC, SA.EXE filename and SourceSafe comments
	int nBuild = atoi(m_szVersionBuild);
	m_szVersionBuild.Format("%d", nBuild + 1); // update build number
  m_szVersion = m_szVersionMajor + "." + m_szVersionMinor + "." + m_szVersionBuild;
  CString szShowVersion = m_szVersion;
	if (m_szBeta != "")
		szShowVersion = m_szVersionMajor + "." + m_szVersionMinor + " Beta " + m_szBeta 
										+ " (Build " + m_szVersionBuild + ")";
	m_szFILEVERSION = m_szVersionMajor + "," + m_szVersionMinor + "," + m_szVersionBuild + ", 0";
  if (m_szRC != "")
  {
    szShowVersion = m_szVersionMajor + "." + m_szVersionMinor + " RC" + m_szRC 
										+ " (Build " + m_szVersionBuild + ")";
  }
  m_szVersionComment = "v" + szShowVersion;
	m_szLegalCopyright = "© 1996-" + time.Format("%Y");
	m_szSplashVersion  = "v" + szShowVersion;
  m_szVS_VERSION = "Version " + szShowVersion;
}

void CSABuilderDlg::LabelVersion()
{
  CString szRCPath = "\"" + m_szBuildPath + m_szSvnBranch + "\\Src\\SaExe\\sa.rc\"";
  CString szCommandString = "svn commit " + szRCPath + " --username " + m_szSvnUser + " --password "+ m_szSvnPass + " -m \"" + m_szVersionComment + "\"";
  char* szCommand = szCommandString.GetBuffer(szCommandString.GetLength());
  char* szConsoleOutput = new char[65536];
  int nResult = RunConsoleCommand(szCommand, szConsoleOutput);
  CString szResult;
  szResult.Format("ERR%d", nResult);
	Log((nResult==0) ? "OK  " : szResult, "Label Version");
	if(nResult != 0)
		m_nErrors++;
	
	m_szSvnHeadRev.Format("%d", atoi((LPCSTR)m_szSvnHeadRev) + 1); // add 1 for the label

	delete [] szConsoleOutput;
}

int CSABuilderDlg::Log(CString szErrorCode, CString szMessageText)
{
	// Open log file
	CFile buildLog;
  if(!buildLog.Open(m_szLogFilePath, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite))
    return FALSE;
	// Prepend timestamp
	CTime tTime = CTime::GetCurrentTime();
  if (szMessageText.GetLength())
    szMessageText = tTime.Format("%X") + "  " + szErrorCode + "\t" + szMessageText + "\r\n";
  else
    szMessageText = "\r\n";

	// Write log
	char* szMessageTextBuffer = szMessageText.GetBuffer(szMessageText.GetLength());
	DWORD dwBuildLogLen = szMessageText.GetLength();
	buildLog.SeekToEnd();
	buildLog.Write(szMessageTextBuffer,dwBuildLogLen);
	szMessageText.ReleaseBuffer(); szMessageTextBuffer = NULL;
	buildLog.Flush();
	buildLog.Close();

  return TRUE;
}

/***************************************************************************/
// CSABuilderDlg::MessageLoop Do windows message loop
// This function enables key down message to come through while
// processing.
/***************************************************************************/
void CSABuilderDlg::MessageLoop(DWORD dwMilliSeconds)
{
	DWORD dwStartTime=::GetTickCount();
	BOOL bDoingBackgroundProcessing = TRUE;

	while ( bDoingBackgroundProcessing && (dwStartTime+dwMilliSeconds > ::GetTickCount()) )
	{ 
		MSG msg;
		while ( ::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ) 
		{ 
			if ( !AfxGetApp()->PumpMessage() ) 
			{ 
				bDoingBackgroundProcessing = FALSE; 
				//::PostQuitMessage( ); 
				break; 
			} 
		} 
		// let MFC do its idle processing
		LONG lIdle = 0;
		while ( AfxGetApp()->OnIdle(lIdle++ ) )
			;  
		// Perform some background processing here 
		// using another call to OnIdle
	}
}

void CSABuilderDlg::OnBuildError(char* data, CString m_szBuildPath)
{	int nResult;
	CString dat;
	dat = (LPCSTR)data;

	GetDlgItemText(IDC_NOTEPATH,m_szNotepadPath);
	// therefore we have an error!!!
  Log("", "");
  Log("", "Build failed: " + dat);
  if (!m_bBatchMode)
  {
	  nResult = _spawnl(_P_NOWAIT, m_szNotepadPath, "Notepad", "\"" + m_szBuildPath + "\\MakeRelease.log\"", NULL);
	  MessageBox("Build failed! \n" + dat, "SA Builder", MB_OK|MB_ICONSTOP);
  }

	exit(-1);
}

int CSABuilderDlg::RunConsoleCommand(char* szCommand, char* szOutputBuffer, char* szOutputFileName)
{
  char* szBatchFileName = "Command.bat";
	bool bRemoveOutputFile = false;
	if (szOutputFileName == "")
	{
		szOutputFileName = "Output.txt";
		bRemoveOutputFile = true;
	}
  CFile fileBatch(szBatchFileName, CFile::modeCreate | CFile::modeReadWrite);
  CString szBatchCommand(szCommand);
  szBatchCommand += " > ";
  szBatchCommand += CString(szOutputFileName);
  fileBatch.Write(szBatchCommand, lstrlen(szBatchCommand)); // Send output to a text file
  fileBatch.Close();
  char* szBatchFilePath = fileBatch.GetFilePath().GetBuffer(fileBatch.GetFilePath().GetLength());
  strcat(szBatchFilePath, szBatchFileName);
  int nReturn = _spawnl(P_WAIT, szBatchFilePath, szBatchFileName, NULL); // Run the command
  fileBatch.Remove(szBatchFileName);

  // Load the output into the buffer
  CFile fileOutput(szOutputFileName, CFile::modeRead);
  DWORD dwLength = fileOutput.GetLength();
  if (dwLength > 65536)
    dwLength = 65536;
  fileOutput.Read(szOutputBuffer, dwLength);
  fileOutput.Close();
  if (bRemoveOutputFile)
		fileOutput.Remove(szOutputFileName);
  
  return nReturn;
}

int CSABuilderDlg::SetUpTest()
{
	int nResult = 0;
	int nErrors = 0;

  return nErrors;
  
  CFileFind finder;
	Log("", "Test...");
	nResult = CopyFile(m_szBuildPath + "Debug\\SA2.exe", m_szTestPath + "Debug\\SA2.exe", FALSE);
  CString szResult;
  szResult.Format("ERR%d", nResult);
	Log((nResult!=0) ? "OK  " : "Error "+ szResult, "Update Debug\\SA2");
	if(nResult == 0)
		nErrors++;
	nResult = CopyFile(m_szBuildPath + "Debug\\SA_ENUd.dll", m_szTestPath + "Debug\\SA_ENUd.dll", FALSE);
  szResult.Format("ERR%d", nResult);
	Log((nResult!=0) ? "OK  " : "Error "+ szResult, "Update Debug\\SA_ENUd.dll");
	if(nResult == 0)
		nErrors++;
	nResult = CopyFile(m_szBuildPath + "Release\\SA2.exe", m_szTestPath + "Release\\SA2.exe", FALSE);
  szResult.Format("ERR%d", nResult);
	Log((nResult!=0) ? "OK  " : "Error "+ szResult, "Update Release\\SA2");
	if(nResult == 0)
		nErrors++;
	nResult = CopyFile(m_szBuildPath + "Release\\SA_ENU.dll", m_szTestPath + "Release\\SA_ENU.dll", FALSE);
  szResult.Format("ERR%d", nResult);
	Log((nResult!=0) ? "OK  " : "Error "+ szResult, "Update Release\\SA_ENU.dll");
	if(nResult == 0)
		nErrors++;
	// ensure existence of test folders
	if(!finder.FindFile(m_szTestPath + "*.*"))
		CreateDirectory(m_szTestPath, NULL);
	if(!finder.FindFile(m_szTestPath + "Debug\\SelfTest\\*.*"))
		CreateDirectory(m_szTestPath + "Debug\\SelfTest", NULL);
	if(!finder.FindFile(m_szTestPath + "Release\\SelfTest\\*.*"))
		CreateDirectory(m_szTestPath + "Release\\SelfTest", NULL);
	nResult = SetCurrentDirectory(m_szTestPath + "Debug\\SelfTest");
  szResult.Format("ERR%d", nResult);
	Log((nResult!=0) ? "OK  " : "Error "+ szResult, "cd Debug\\SelfTest");
	if(nResult == 0)
		nErrors++;
	//nResult = _spawnl(_P_WAIT, m_szSvnExePath + "SS.exe", "ss", "Get", "\"" + m_szSAProjectName + "/Program Source/Debug/SelfTest\"", "-R", "-I-", "-Y" + m_szSvnUser+","+szSSPass, NULL);
	MessageLoop(100);
  szResult.Format("ERR%d", nResult);
	Log((nResult!=0) ? "OK  " : "Error "+ szResult, "Get to Debug\\SelfTest");
	if(nResult != 0)
		nErrors++;
	nResult = SetCurrentDirectory(m_szTestPath + "Release\\SelfTest");
  szResult.Format("ERR%d", nResult);
	Log((nResult!=0) ? "OK  " : "Error "+ szResult, "cd Release\\SelfTest");
	if(nResult == 0)
		nErrors++;
	//nResult = _spawnl(_P_WAIT, m_szSvnExePath + "SS.exe", "ss", "Get", "\"" + m_szSAProjectName + "/Program Source/Release/SelfTest\"", "-R", "-I-", "-Y" + m_szSvnUser+","+szSSPass, NULL);
	MessageLoop(100);
  szResult.Format("ERR%d", nResult);
	Log((nResult!=0) ? "OK  " : "Error "+ szResult, "Get to Release\\SelfTest");
	if(nResult != 0)
		nErrors++;
	//nResult = _spawnl(_P_WAIT, m_szBuildPath + "Debug\\SA.exe", m_szBuildPath + "Debug\\SA.exe", "/T", NULL);
	//nResult = _spawnl(_P_WAIT, m_szBuildPath + "Release\\SA.exe", m_szBuildPath + "Release\\SA.exe", "/T", NULL);

}

void CSABuilderDlg::UpdateHTML(CString &szHTMLPath,CString &szEXEZIPPath,CString &szOCXZIPPath,CString &szShellZIPPath,
							   CString &szSetupEXEPath,CString &SetupSAmin2xEXE,CString &szSetupSTEXEPath,CString &szTimeDate)
{
	// Open html file for editing
	CFile fileHTML(szHTMLPath,CFile::modeReadWrite);
	DWORD dwHTMLlen = fileHTML.GetLength();
	CString szBuffer('\0',dwHTMLlen);
	char * szBufferBuffer = szBuffer.GetBuffer(dwHTMLlen);
	fileHTML.Read(szBufferBuffer, dwHTMLlen);
	szBuffer.ReleaseBuffer(); szBufferBuffer = NULL;
	int nIndex = 0; // set buffer index to beginning of file

	// Update EXE version
	if((nIndex = szBuffer.Find("href=\"SA",nIndex)) > 0)
	{
		CFile fileEXEZIP(szEXEZIPPath,CFile::modeRead);
		DWORD dwEXEZIPlen = fileEXEZIP.GetLength();
	
		nIndex += 8; // now we should have xx-yy
		int nEndString = szBuffer.Find("</P>",nIndex);
		if (nEndString == -1)
			nEndString = szBuffer.Find("</p>",nIndex);

		// create string
		CString szString;
		szString.Format("%s.zip\">SA%s.zip</A> (%iK) Uploaded %s",m_szZIPFILEVERSION,m_szZIPFILEVERSION,(dwEXEZIPlen+512)/1024,szTimeDate);

		szBuffer.Delete(nIndex,nEndString-nIndex);
		szBuffer.Insert(nIndex,szString);
	}

	// Update OCX version
	if((nIndex = szBuffer.Find("href=\"OCX",nIndex)) > 0)
	{
		CFile fileOCXZIP(szOCXZIPPath,CFile::modeRead);
		DWORD dwOCXZIPlen = fileOCXZIP.GetLength();
	
		nIndex += 9; // now we should have xx-yy
		int nEndString = szBuffer.Find("</P>",nIndex);
		if (nEndString == -1)
			nEndString = szBuffer.Find("</p>",nIndex);

		// create string
		CString szString;
		szString.Format("%s.zip\">OCX%s.zip</A> (%iK) Uploaded %s",m_szZIPFILEVERSION,m_szZIPFILEVERSION,dwOCXZIPlen/1024,szTimeDate);

		szBuffer.Delete(nIndex,nEndString-nIndex);
		szBuffer.Insert(nIndex,szString);
	}

	// Update SHELL version
	if((nIndex = szBuffer.Find("href=\"SaShell",nIndex)) > 0)
	{
		CFile fileShellZIP(szShellZIPPath,CFile::modeRead);
		DWORD dwShellZIPlen = fileShellZIP.GetLength();
	
		nIndex += 13; // now we should have xx-yy
		int nEndString = szBuffer.Find("</P>",nIndex);
		if (nEndString == -1)
			nEndString = szBuffer.Find("</p>",nIndex);

		// create string
		CString szString;
		szString.Format("%s.zip\">SaShell%s.zip</A> (%iK) Uploaded %s",m_szZIPFILEVERSION,m_szZIPFILEVERSION,dwShellZIPlen/1024,szTimeDate);

		szBuffer.Delete(nIndex,nEndString-nIndex);
		szBuffer.Insert(nIndex,szString);
	}

	// Update Setup-SA version
	if((nIndex = szBuffer.Find("href=\"Setup-SA",nIndex)) > 0
		&& (szBuffer[nIndex+14]>='0' && szBuffer[nIndex+14]<='9'))
	{
		CFile fileSetupEXE(szSetupEXEPath,CFile::modeRead);
		DWORD dwSetupEXElen = fileSetupEXE.GetLength();
	
		nIndex += 14; // now we should have xx-yy
		int nEndString = szBuffer.Find("</P>",nIndex);
		if (nEndString == -1)
			nEndString = szBuffer.Find("</p>",nIndex);

		// create string
		CString szString;
		szString.Format("%s.exe\">Setup-SA%s.exe</A> (%iK) Uploaded %s",m_szZIPFILEVERSION,m_szZIPFILEVERSION,dwSetupEXElen/1024,szTimeDate);

		szBuffer.Delete(nIndex,nEndString-nIndex);
		szBuffer.Insert(nIndex,szString);
	}

	// Update Setup-SAmin2x version
	if((nIndex = szBuffer.Find("href=\"Setup-SAmin",nIndex)) > 0
		&& (szBuffer[nIndex+17]=='2'))
	{
		CFile fileSetupSAmin2xEXE(SetupSAmin2xEXE,CFile::modeRead);
		DWORD dwSetupSAmin2xEXElen = fileSetupSAmin2xEXE.GetLength();
	
		nIndex += 17; // now we should have xx-yy
		int nEndString = szBuffer.Find("</P>",nIndex);
		if (nEndString == -1)
			nEndString = szBuffer.Find("</p>",nIndex);

		// create string
		CString szString;
		szString.Format("%s.exe\">Setup-SAmin%s.exe</A> (%iK) Uploaded %s",m_szZIPFILEVERSION,m_szZIPFILEVERSION,dwSetupSAmin2xEXElen/1024,szTimeDate);

		szBuffer.Delete(nIndex,nEndString-nIndex);
		szBuffer.Insert(nIndex,szString);
	}

	// Update Speech Tools Setup version
	if((nIndex = szBuffer.Find("href=\"Setup-ST",nIndex)) > 0)
	{
		CFile fileSetupSTEXE(szSetupSTEXEPath,CFile::modeRead);
		DWORD dwSetupSTEXElen = fileSetupSTEXE.GetLength();
	
		nIndex += 14; // now we should have xx-yy
		int nEndString = szBuffer.Find("</P>",nIndex);
		if (nEndString == -1)
			nEndString = szBuffer.Find("</p>",nIndex);

		// create string
		CString szString;
		szString.Format("%s.exe\">Setup-ST%s.exe</A> (%iK) Uploaded %s",m_szZIPFILEVERSION,m_szZIPFILEVERSION,dwSetupSTEXElen/1024,szTimeDate);

		szBuffer.Delete(nIndex,nEndString-nIndex);
		szBuffer.Insert(nIndex,szString);
	}

	// Write HTML
	fileHTML.SetLength(0);
	szBufferBuffer = szBuffer.GetBuffer(dwHTMLlen);
	dwHTMLlen = szBuffer.GetLength();
	fileHTML.Write(szBufferBuffer,dwHTMLlen);
	szBuffer.ReleaseBuffer(); szBufferBuffer = NULL;
	fileHTML.Flush();
	fileHTML.Close();
}

void CSABuilderDlg::UpdateRC(CString &szPath)
{
	// Open RC file for editing
	CFile fileRC(szPath,CFile::modeReadWrite);
	DWORD dwRClen = fileRC.GetLength();
	CString szBuffer('\0',dwRClen);
	char * szBufferBuffer = szBuffer.GetBuffer(dwRClen);
	fileRC.Read(szBufferBuffer, dwRClen);
	szBuffer.ReleaseBuffer(); szBufferBuffer = NULL;

	// Update all copyright statements
	int nIndex = 0;
	while((nIndex = szBuffer.Find("© 1996",nIndex)) > 0)
	{
		szBuffer.Delete(nIndex,m_szLegalCopyright.GetLength());
		szBuffer.Insert(nIndex,m_szLegalCopyright);
		++nIndex;
	}
	
	// Modify SaShell Version
	nIndex = szBuffer.Find("SaShell Version ");
	if(nIndex != -1)
	{
		nIndex += 8;
		for(register i=nIndex;szBuffer[i]!='\"';++i);
		szBuffer.Delete(nIndex,i-nIndex);
		szBuffer.Insert(nIndex,m_szVS_VERSION);
	}

	// Modify file version info
	nIndex = szBuffer.Find("FILEVERSION ");
	if(nIndex != -1)
	{
		nIndex += 12;
		for(register i=nIndex;szBuffer[i]!='\n';++i);
		szBuffer.Delete(nIndex,i-nIndex);
		szBuffer.Insert(nIndex,m_szFILEVERSION);
	}

	// Modify fileversion
	nIndex = szBuffer.Find("\"FileVersion\"");
	if(nIndex != -1)
	{
		nIndex += 14;
		nIndex = szBuffer.Find('\"',nIndex);
		++nIndex;
		for(register i=nIndex;szBuffer[i]!='\\';++i);
		szBuffer.Delete(nIndex,i-nIndex);
		szBuffer.Insert(nIndex,m_szSplashVersion);
	}
	
	nIndex = 0;

	// Modify splash screen version
	while((nIndex = szBuffer.Find("VS_VERSION",nIndex)) > 0)
	if(nIndex != -1)
	{
		if(szBuffer[nIndex+10]=='_')
		{
			++nIndex;
			continue;
		}

		nIndex = szBuffer.Find('\"',nIndex);
		if(nIndex<0) break;
		++nIndex;
		for(register i=nIndex;szBuffer[i]!='\"';++i);
		szBuffer.Delete(nIndex,i-nIndex);
		szBuffer.Insert(nIndex,m_szVS_VERSION);
	}

	// Write RC file
	fileRC.SetLength(0);
	szBufferBuffer = szBuffer.GetBuffer(dwRClen);
	dwRClen = szBuffer.GetLength();
	fileRC.Write(szBufferBuffer,dwRClen);
	szBuffer.ReleaseBuffer(); szBufferBuffer = NULL;
	fileRC.Flush();
	fileRC.Close();
}

void CSABuilderDlg::WriteRegistryStrings()
{
  char szBranchKey[_MAX_PATH] = "SA Builder\\";
  strcat(szBranchKey, (LPCTSTR)m_szSvnBranch);
	sdfWriteRegistryString("SA Builder","SvnBranch",m_szSvnBranch);
	sdfWriteRegistryString(szBranchKey,"VersionMajor",m_szVersionMajor);
	sdfWriteRegistryString(szBranchKey,"VersionMinor",m_szVersionMinor);
	sdfWriteRegistryString(szBranchKey,"VersionBuild",m_szVersionBuild);
	sdfWriteRegistryString(szBranchKey,"RC",m_szRC);
	sdfWriteRegistryString("SA Builder","DestinationPath",m_szDestinationPath);
	sdfWriteRegistryString("SA Builder","BuildPath",m_szBuildPath);
	sdfWriteRegistryString("SA Builder","SvnExePath",m_szSvnExePath);
	sdfWriteRegistryString("SA Builder","SvnUser",m_szSvnUser);
	sdfWriteRegistryString("SA Builder","SvnPass",m_szSvnPass);
	sdfWriteRegistryString("SA Builder","SvnRepoURL",m_szSvnRepoURL);
	sdfWriteRegistryString("SA Builder","MSDevPath",m_szMSDevPath);
	sdfWriteRegistryString("SA Builder","MSBuildPath",m_szMSBuildPath);
	sdfWriteRegistryString("SA Builder","NotepadPath",m_szNotepadPath);
	sdfWriteRegistryString("SA Builder","TestPath",m_szTestPath);
	sdfWriteRegistryString("SA Builder","WebPagePath",m_szWebPagePath);
	sdfWriteRegistryString("SA Builder","FTPUserName",m_szFTPUser);
  CString szFTPPassEncrypted = "";
	sdfWriteRegistryString("SA Builder","FTPPassword", EncryptString(m_szFTPPass));
  sdfWriteRegistryString("SA Builder", "SvnLastRev", m_szSvnLastRev);
  sdfWriteRegistryString("SA Builder", "SvnLastLabelRev", m_szSvnLastLabelRev);
}