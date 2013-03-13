// DlgExportFWResult.cpp : implementation file
//

#include "stdafx.h"
#include "DlgExportFWResult.h"

// CDlgExportFWResult dialog

IMPLEMENT_DYNAMIC(CDlgExportFWResult, CDialog)

CDlgExportFWResult::CDlgExportFWResult(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgExportFWResult::IDD, pParent)
{

	m_DataCount = _T("");
}

CDlgExportFWResult::~CDlgExportFWResult()
{
}

void CDlgExportFWResult::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_DATA_COUNT, m_DataCount);
	DDX_Text(pDX, IDC_STATIC_SFM_COUNT, m_SFMCount);
	DDX_Text(pDX, IDC_STATIC_WAV_COUNT, m_WAVCount);
}


BEGIN_MESSAGE_MAP(CDlgExportFWResult, CDialog)
END_MESSAGE_MAP()

