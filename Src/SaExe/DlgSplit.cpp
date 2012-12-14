// CDlgSplit.cpp : implementation file
//

#include "stdafx.h"
#include "sa.h"
#include "dlgsplit.h"

// SA_Split dialog

IMPLEMENT_DYNAMIC(CDlgSplit, CDialog)

CDlgSplit::CDlgSplit(CWnd* pParent /*=NULL*/) : 
CDialog(CDlgSplit::IDD, pParent), 
m_iConvention(1), 
m_FolderLocation(_T("")), 
m_FolderName(_T("")), 
m_PhraseFolderName(_T("")), 
m_GlossFolderName(_T("")), 
m_ExportPhrase(FALSE),
m_ExportGloss(FALSE)
{
}

CDlgSplit::~CDlgSplit() 
{
}

BOOL CDlgSplit::OnInitDialog()
{
	CDialog::OnInitDialog();
	GetDlgItem(IDC_SPLIT_WORD_SUBFOLDER_NAME)->EnableWindow(FALSE);
	GetDlgItem(IDC_SPLIT_PHRASE_SUBFOLDER_NAME)->EnableWindow(FALSE);
	UpdateData(TRUE);
	return TRUE;
}

void CDlgSplit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_SPLIT_CONVENTION, m_iConvention);
	DDX_Text(pDX, IDC_SPLIT_FOLDER_LOCATION, m_FolderLocation);
	DDX_Text(pDX, IDC_SPLIT_FOLDER_NAME, m_FolderName);
	DDX_Text(pDX, IDC_SPLIT_PHRASE_SUBFOLDER_NAME, m_PhraseFolderName);
	DDX_Text(pDX, IDC_SPLIT_WORD_SUBFOLDER_NAME, m_GlossFolderName);
	DDX_Check(pDX, IDC_EXPORT_PHRASE, m_ExportPhrase);
	DDX_Check(pDX, IDC_EXPORT_WORD, m_ExportGloss);
}

BEGIN_MESSAGE_MAP(CDlgSplit, CDialog)
	ON_BN_CLICKED(IDC_EXPORT_WORD, &CDlgSplit::OnBnClickedExportGloss)
	ON_BN_CLICKED(IDC_EXPORT_PHRASE, &CDlgSplit::OnBnClickedExportPhrase)
	ON_BN_CLICKED(IDC_BROWSE_FOLDER, &CDlgSplit::OnBnClickedBrowseFolder)
END_MESSAGE_MAP()

void CDlgSplit::OnBnClickedExportGloss()
{
	BOOL state = IsDlgButtonChecked(IDC_EXPORT_WORD);
	GetDlgItem(IDC_SPLIT_WORD_SUBFOLDER_NAME)->EnableWindow(state);
}

void CDlgSplit::OnBnClickedExportPhrase()
{
	BOOL state = IsDlgButtonChecked(IDC_EXPORT_PHRASE);
	GetDlgItem(IDC_SPLIT_PHRASE_SUBFOLDER_NAME)->EnableWindow(state);
}

static int CALLBACK BrowseCallbackProc( HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM lpData) 
{

	// If the BFFM_INITIALIZED message is received    
	// set the path to the start path.    
	switch (uMsg)
	{        
	case BFFM_INITIALIZED: 
		{            
			if (NULL != lpData)
			{                
				SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);            
			}        
		}    
	}     
	return 0; 
} 

void CDlgSplit::OnBnClickedBrowseFolder()
{

	// szCurrent is an optional start folder. Can be NULL.
	// szPath receives the selected path on success. Must be MAX_PATH characters in length.
	
	CoInitialize(NULL);     

	TCHAR szDisplay[MAX_PATH];    
	memset(szDisplay,0,sizeof(szDisplay));

	TCHAR szPath[MAX_PATH];
	memset(szPath,0,sizeof(szPath));

	BROWSEINFO bi = { 0 };    
	bi.hwndOwner = this->m_hWnd;    
	bi.pszDisplayName = szDisplay;    
	bi.lpszTitle = TEXT("Please choose a folder.");    
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;    
	bi.lpfn = BrowseCallbackProc;    
	bi.lParam = (LPARAM)(LPCTSTR)m_FolderLocation;     
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (pidl==NULL)
	{
		// they cancelled...
		CoUninitialize();
		return;
	}

	BOOL retval = SHGetPathFromIDList(pidl, szPath);        
	CoTaskMemFree(pidl);    
	if (!retval)
	{
		szPath[0] = TEXT('\0');
	} 
	else
	{
		GetDlgItem(IDC_SPLIT_FOLDER_LOCATION)->SetWindowText(szPath);
	}

	CoUninitialize();
}

enum EFilenameConvention CDlgSplit::GetFilenameConvention()
{
	if (m_iConvention==1)
	{
		return FC_REF;
	} 
	else if (m_iConvention==0)
	{
		return FC_GLOSS;
	} 
	else
	{
		return FC_REF_GLOSS;
	}
}
