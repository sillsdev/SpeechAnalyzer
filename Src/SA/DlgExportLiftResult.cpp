// DlgExportLiftResult.cpp : implementation file
//

#include "stdafx.h"
#include "DlgExportLiftResult.h"

// CDlgExportLiftResult dialog

IMPLEMENT_DYNAMIC(CDlgExportLiftResult, CDialog)

CDlgExportLiftResult::CDlgExportLiftResult(CWnd * pParent /*=NULL*/)
    : CDialog(CDlgExportLiftResult::IDD, pParent) {

    m_DataCount = _T("");
}

CDlgExportLiftResult::~CDlgExportLiftResult() {
}

void CDlgExportLiftResult::DoDataExchange(CDataExchange * pDX) {
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_STATIC_DATA_COUNT, m_DataCount);
    DDX_Text(pDX, IDC_STATIC_SFM_COUNT, m_SFMCount);
    DDX_Text(pDX, IDC_STATIC_WAV_COUNT, m_WAVCount);
}

BEGIN_MESSAGE_MAP(CDlgExportLiftResult, CDialog)
END_MESSAGE_MAP()

