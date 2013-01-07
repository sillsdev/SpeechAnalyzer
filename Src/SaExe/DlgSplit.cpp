// CDlgSplit.cpp : implementation file
//

#include "stdafx.h"
#include "sa.h"
#include "dlgsplit.h"

// SA_Split dialog

IMPLEMENT_DYNAMIC(CDlgSplit, CDialog)

CDlgSplit::CDlgSplit(CWnd* pParent /*=NULL*/) : 
CDialog(CDlgSplit::IDD, pParent), 
m_WordConvention(2), 
m_PhraseConvention(3), 
m_FolderLocation(_T("")), 
m_FolderName(_T("")), 
m_PhraseFolderName(_T("")), 
m_GlossFolderName(_T("")) 
{
}

CDlgSplit::~CDlgSplit() {
}

BOOL CDlgSplit::OnInitDialog() 
{

	CDialog::OnInitDialog();
	GetDlgItem(IDC_SPLIT_WORD_SUBFOLDER_NAME)->EnableWindow(FALSE);
	GetDlgItem(IDC_SPLIT_PHRASE_SUBFOLDER_NAME)->EnableWindow(FALSE);

	m_CheckGlossEmpty.SetCheck(TRUE);

	UpdateData(TRUE);
	return TRUE;
}

void CDlgSplit::DoDataExchange(CDataExchange* pDX) 
{
	CDialog::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_SPLIT_WORD_CONVENTION, m_WordConvention);
	DDX_CBIndex(pDX, IDC_SPLIT_PHRASE_CONVENTION, m_PhraseConvention);
	DDX_Text(pDX, IDC_SPLIT_FOLDER_LOCATION, m_FolderLocation);
	DDX_Text(pDX, IDC_SPLIT_FOLDER_NAME, m_FolderName);
	DDX_Text(pDX, IDC_SPLIT_PHRASE_SUBFOLDER_NAME, m_PhraseFolderName);
	DDX_Text(pDX, IDC_SPLIT_WORD_SUBFOLDER_NAME, m_GlossFolderName);
	DDX_Control(pDX, IDC_CHECK_GLOSS_EMPTY, m_CheckGlossEmpty);
	DDX_Check(pDX, IDC_CHECK_GLOSS_EMPTY, m_SkipGlossEmpty);
}

BEGIN_MESSAGE_MAP(CDlgSplit, CDialog) 
	ON_BN_CLICKED(IDC_BROWSE_FOLDER, &CDlgSplit::OnBnClickedBrowseFolder)
	ON_BN_CLICKED(IDC_EDIT_PHRASE_FOLDER, &CDlgSplit::OnBnClickedEditPhraseFolder)
	ON_BN_CLICKED(IDC_EDIT_GLOSS_FOLDER, &CDlgSplit::OnBnClickedEditGlossFolder)
END_MESSAGE_MAP()

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
	if (pidl==NULL) {
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

enum EWordFilenameConvention CDlgSplit::GetWordFilenameConvention() 
{
	switch (m_WordConvention) {
	case 0: return WFC_REF;
	case 1: return WFC_GLOSS;
	default:case 2: return WFC_REF_GLOSS;
	}
	return WFC_REF_GLOSS;
}

enum EPhraseFilenameConvention CDlgSplit::GetPhraseFilenameConvention() 
{
	switch (m_PhraseConvention) {
	case 0: return PFC_REF;
	case 1: return PFC_GLOSS;
	case 2: return PFC_REF_GLOSS;
	default:case 3: return PFC_PHRASE;
	}
}

void CDlgSplit::OnBnClickedEditPhraseFolder() 
{
	GetDlgItem(IDC_SPLIT_PHRASE_SUBFOLDER_NAME)->EnableWindow(TRUE);
}

void CDlgSplit::OnBnClickedEditGlossFolder() 
{
	GetDlgItem(IDC_SPLIT_WORD_SUBFOLDER_NAME)->EnableWindow(TRUE);
}
