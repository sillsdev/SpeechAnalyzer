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
m_WordFolderName(_T("")), 
m_ExportPhrase(FALSE),
m_ExportWord(FALSE)
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
	DDX_Text(pDX, IDC_SPLIT_WORD_SUBFOLDER_NAME, m_WordFolderName);
	DDX_Check(pDX, IDC_EXPORT_PHRASE, m_ExportPhrase);
	DDX_Check(pDX, IDC_EXPORT_WORD, m_ExportWord);
}

BEGIN_MESSAGE_MAP(CDlgSplit, CDialog)
	ON_BN_CLICKED(IDC_EXPORT_WORD, &CDlgSplit::OnBnClickedExportWord)
	ON_BN_CLICKED(IDC_EXPORT_PHRASE, &CDlgSplit::OnBnClickedExportPhrase)
END_MESSAGE_MAP()

void CDlgSplit::OnBnClickedExportWord()
{
	BOOL state = IsDlgButtonChecked(IDC_EXPORT_WORD);
	GetDlgItem(IDC_SPLIT_WORD_SUBFOLDER_NAME)->EnableWindow(state);
}

void CDlgSplit::OnBnClickedExportPhrase()
{
	BOOL state = IsDlgButtonChecked(IDC_EXPORT_PHRASE);
	GetDlgItem(IDC_SPLIT_PHRASE_SUBFOLDER_NAME)->EnableWindow(state);
}
