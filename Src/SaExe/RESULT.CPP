// result.cpp : implementation file
//

#include "stdafx.h"
#include "sa.h"
#include "result.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResult dialog


CResult::CResult(CString string, CWnd * pParent /*=NULL*/)
    : CDialog(CResult::IDD, pParent) {
    //{{AFX_DATA_INIT(CResult)
    m_szResult = string;
    //}}AFX_DATA_INIT
    DoModal();
}

void CResult::DoDataExchange(CDataExchange * pDX) {
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CResult)
    DDX_Text(pDX, IDC_EDIT, m_szResult);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CResult, CDialog)
//{{AFX_MSG_MAP(CResult)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CResult message handlers

void CResult::OnCancel() {
    SendMessage(WM_COMMAND, ID_EDIT_UNDO, 0);

    CDialog::OnCancel();
}
