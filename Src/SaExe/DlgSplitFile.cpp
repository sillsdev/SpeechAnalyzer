// CDlgSplitFile.cpp : implementation file
//

#include "stdafx.h"
#include "sa.h"
#include "DlgSplitFile.h"
#include "resource.h"
#include "FileUtils.h"

using std::wstring;

// SA_Split dialog

IMPLEMENT_DYNAMIC(CDlgSplitFile, CDialog)

CDlgSplitFile::CDlgSplitFile(CWnd * pParent /*=NULL*/) :
    CDialog(CDlgSplitFile::IDD, pParent),
    m_bSkipGlossEmpty(TRUE),
    m_bOverwriteData(FALSE),
    m_nWordConvention(2),
    m_nPhraseConvention(2),
    m_szFolderLocation(_T("")),
    m_szFolderName(_T("")),
    m_szPhraseFolderName(_T("")),
    m_szGlossFolderName(_T("")),
	m_szFilenamePrefix(_T("")),
	m_szFilenameSuffix(_T(""))
{
}

CDlgSplitFile::~CDlgSplitFile()
{
}

BOOL CDlgSplitFile::OnInitDialog()
{
    CDialog::OnInitDialog();
    GetDlgItem(IDC_SPLIT_WORD_SUBFOLDER_NAME)->EnableWindow(FALSE);
    GetDlgItem(IDC_SPLIT_PHRASE_SUBFOLDER_NAME)->EnableWindow(FALSE);

    UpdateData(TRUE);
    return TRUE;
}

void CDlgSplitFile::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_CBIndex(pDX, IDC_SPLIT_WORD_CONVENTION, m_nWordConvention);
    DDX_CBIndex(pDX, IDC_SPLIT_PHRASE_CONVENTION, m_nPhraseConvention);
    DDX_Text(pDX, IDC_SPLIT_FOLDER_LOCATION, m_szFolderLocation);
    DDX_Filename(pDX, IDC_SPLIT_FOLDER_NAME, m_szFolderName);
    DDX_Filename(pDX, IDC_SPLIT_PHRASE_SUBFOLDER_NAME, m_szPhraseFolderName);
    DDX_Filename(pDX, IDC_SPLIT_WORD_SUBFOLDER_NAME, m_szGlossFolderName);
    DDX_Check(pDX, IDC_CHECK_GLOSS_EMPTY, m_bSkipGlossEmpty);
    DDX_Check(pDX, IDC_CHECK_OVERWRITE, m_bOverwriteData);
	DDX_Filename(pDX, IDC_FILENAME_PREFIX, m_szFilenamePrefix);
    DDX_Filename(pDX, IDC_FILENAME_SUFFIX, m_szFilenameSuffix);
}

BEGIN_MESSAGE_MAP(CDlgSplitFile, CDialog)
    ON_BN_CLICKED(IDC_BROWSE_FOLDER, &CDlgSplitFile::OnBnClickedBrowseFolder)
    ON_BN_CLICKED(IDC_EDIT_PHRASE_FOLDER, &CDlgSplitFile::OnBnClickedEditPhraseFolder)
    ON_BN_CLICKED(IDC_EDIT_GLOSS_FOLDER, &CDlgSplitFile::OnBnClickedEditGlossFolder)
END_MESSAGE_MAP()

static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg, LPARAM /*lParam*/, LPARAM lpData)
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

void CDlgSplitFile::OnBnClickedBrowseFolder()
{
    // szCurrent is an optional start folder. Can be NULL.
    // szPath receives the selected path on success. Must be MAX_PATH characters in length.

    CoInitialize(NULL);

    TCHAR szDisplay[MAX_PATH];
    memset(szDisplay,0,sizeof(szDisplay));

    TCHAR szPath[MAX_PATH];
    memset(szPath,0,sizeof(szPath));

	CString msg;
	msg.LoadStringW(IDS_CHOOSE_FOLDER);

    BROWSEINFO bi = { 0 };
    bi.hwndOwner = this->m_hWnd;
    bi.pszDisplayName = szDisplay;
	bi.lpszTitle = msg.GetBuffer(msg.GetLength());
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.lpfn = BrowseCallbackProc;
    bi.lParam = (LPARAM)(LPCTSTR)m_szFolderLocation;
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl==NULL)
    {
        // they cancelled...
		msg.ReleaseBuffer();
        CoUninitialize();
        return;
    }
	msg.ReleaseBuffer();

    BOOL retval = SHGetPathFromIDList(pidl, szPath);
    CoTaskMemFree(pidl);
    if (!retval)
    {
        szPath[0] = TEXT('\0');
    }
    else
    {
        wstring temp(szPath);
        if (temp[temp.length()-1]!='\\')
        {
            temp.append(L"\\");
        }
        GetDlgItem(IDC_SPLIT_FOLDER_LOCATION)->SetWindowText(temp.c_str());
    }

    CoUninitialize();
}

enum EWordFilenameConvention CDlgSplitFile::GetWordFilenameConvention()
{
    switch (m_nWordConvention)
    {
    case 0:
        return WFC_REF;
    case 1:
        return WFC_GLOSS;
    default:
    case 2:
        return WFC_REF_GLOSS;
    }
    return WFC_REF_GLOSS;
}

enum EPhraseFilenameConvention CDlgSplitFile::GetPhraseFilenameConvention()
{
    switch (m_nPhraseConvention)
    {
    case 0:
        return PFC_REF;
    case 1:
        return PFC_GLOSS;
    case 2:
        return PFC_REF_GLOSS;
    default:
    case 3:
        return PFC_PHRASE;
    }
}

void CDlgSplitFile::OnBnClickedEditPhraseFolder()
{
    GetDlgItem(IDC_SPLIT_PHRASE_SUBFOLDER_NAME)->EnableWindow(TRUE);
}

void CDlgSplitFile::OnBnClickedEditGlossFolder()
{
    GetDlgItem(IDC_SPLIT_WORD_SUBFOLDER_NAME)->EnableWindow(TRUE);
}

void CDlgSplitFile::SetWordFilenameConvention(int value)
{
    switch (value)
    {
    case 0:
    case 1:
    case 2:
        m_nWordConvention = value;
        break;
    default:
        m_nWordConvention = 2;
        break;
    }
}

void CDlgSplitFile::SetPhraseFilenameConvention(int value)
{
    switch (value)
    {
    case 0:
    case 1:
    case 2:
    case 3:
        m_nPhraseConvention = value;
        break;
    default:
        m_nPhraseConvention = 3;
        break;
    }
}

