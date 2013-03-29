// result.cpp : implementation file
//

#include "stdafx.h"
#include "sa.h"
#include "DlgResult.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CResult::CResult( LPCTSTR string, CWnd * pParent /*=NULL*/) : CDialog(CResult::IDD, pParent)
{
    m_szResult = string;
}

void CResult::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT, m_szResult);
}

BEGIN_MESSAGE_MAP(CResult, CDialog)
END_MESSAGE_MAP()

void CResult::OnCancel()
{
    SendMessage(WM_COMMAND, ID_EDIT_UNDO, 0);

    CDialog::OnCancel();
}
