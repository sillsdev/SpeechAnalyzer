// CDlgSplit.cpp : implementation file
//

#include "stdafx.h"
#include "sa.h"
#include "dlgsplit.h"

// SA_Split dialog

IMPLEMENT_DYNAMIC(CDlgSplit, CDialog)

CDlgSplit::CDlgSplit(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSplit::IDD, pParent)
	, m_iConvention(0)
	, m_iLocation(0)
	, m_FolderName(_T(""))
{
}

CDlgSplit::~CDlgSplit()
{
}

void CDlgSplit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_SPLIT_CONVENTION, m_iConvention);
	DDX_CBIndex(pDX, IDC_SPLIT_LOCATION, m_iLocation);
	DDX_Text(pDX, IDC_SPLIT_FOLDER, m_FolderName);
}

BEGIN_MESSAGE_MAP(CDlgSplit, CDialog)
END_MESSAGE_MAP()
