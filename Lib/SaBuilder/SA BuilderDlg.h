// SA BuilderDlg.h : header file
//

#if !defined(AFX_SABUILDERDLG_H__47F27B4A_04C2_4AA9_A127_66690AFC70BA__INCLUDED_)
#define AFX_SABUILDERDLG_H__47F27B4A_04C2_4AA9_A127_66690AFC70BA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CSABuilderDlg dialog

class CSABuilderDlg : public CDialog
{
// Construction
public:
	void OnBuildError(char* data, CString szSAPath);
	CSABuilderDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSABuilderDlg)
	enum { IDD = IDD_SABUILDER_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSABuilderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void Browse(CString &szDocumentName);
	int RemoveOldBuildFolders();
  int InitializeLog();
  int CompleteLog();
  int CheckoutCode();
  int BuildSaCSharpProjects();
  int BuildSA();
	int BuildPA();
	int BuildInstaller();
  int CheckForCodeChanges();
	int CheckOutputFileCount();
	int UpdateInstallerFilesFromSVN(CString szInstallerFilesPath);
  int CommitChanges();
	int CommitInstallerChanges();
  CString DecryptString(CString &szEncryptedText);
  int DistributeFiles();
	int Copy(CString szSourcePath, CString szDestPath, CString szFileName);
	int CopyToWebSite();
	void LookupCommitterAddress();
	int EmailFile(CString file, CString recipient, CString subject);
  int EmailResults();
	CString EncryptString(CString &szDecryptedText);
  void GetDialogStrings();
  void SetVersionStrings();
	void LabelVersion();
  int Log(CString szErrorCode, CString szMessageText);
	void MessageLoop(DWORD dwMilliSeconds);
  //void ReadRegistryStrings();
  int RunConsoleCommand(char* szCommand, char* szOutputBuffer, char* szOutputFileName = "");
  int SetUpTest();
	void UpdateRC(CString &szFilePath);
	void UpdateHTML(CString &szHTMLPath,CString &szEXEZIPPath,CString &szOCXZIPPath,
		              CString &szShellZIPPath,CString &szSetupEXEPath,CString &szSetupMinEXEPath,
		              CString &szSetupSTEXEPath,CString &szTimeDate);
  void WriteRegistryStrings();

  // Dialog strings	
	CString m_szSvnBranch;
  CString m_szVersionMajor;
  CString m_szVersionMinor;
  CString m_szVersionYear;
  CString m_szVersionDate;
  CString m_szVersionBuild;
  CString m_szRC;
  CString m_szBeta;
	CString m_szDestinationPath;
	CString m_szBuildPath;
	CString m_szSvnExePath;
	CString m_szSvnUser;
  CString m_szSvnPass;
  CString m_szSvnRepoURL;
	CString m_szMSDevPath;
	CString m_szMSBuildPath;
	CString m_szWisePath;
	CString m_szSetupPath;
	CString m_szTestPath;
	CString m_szNotepadPath;
	CString m_szWebPagePath;
	CString m_szFTPUser;
	CString m_szFTPPass;


  // Version strings
  CString m_szFILEVERSION;
	CString m_szZIPFILEVERSION;
	CString m_szLegalCopyright;
	CString m_szVersion;
	CString m_szSplashVersion;
	CString m_szVersionComment;
	CString m_szVS_VERSION;

  // Other member variables
  CString m_szSvnLastRev;
  CString m_szSvnLastLabelRev;
  CString m_szSvnHeadRev;
	BOOL m_bSACodeChanges;
	BOOL m_bPACodeChanges;
	BOOL m_bCSharpCodeChanges;
	CString m_szCommitterAddress;
  CString m_szSABuilderPath;
  int m_nErrors;
  int m_nWarnings;

	HICON m_hIcon;
	BOOL m_bBatchMode;
	BOOL m_bLabelVersion;
	BOOL m_bReboot;
	BOOL m_bDeleteFilesFirst;
  CString m_szLogFilePath;
  CString m_szDestinationFolderName;

	// Generated message map functions
	//{{AFX_MSG(CSABuilderDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnChangeBranchEdit();
  afx_msg void OnChangeVersionMajorEdit();
	afx_msg void OnChangeVersionMinorEdit();
  afx_msg void OnChangeRCEdit();
	afx_msg void OnChangeDestPathEdit();
	virtual void OnOK();
	afx_msg void OnStoreButton();
	afx_msg void OnChangeDevPath();
	afx_msg void OnChangeBuildPath();
	afx_msg void OnChangeTestPath();
	afx_msg void OnChangeNotePath();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SABUILDERDLG_H__47F27B4A_04C2_4AA9_A127_66690AFC70BA__INCLUDED_)
