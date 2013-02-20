#include "stdafx.h"
#include "dlgimport.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgImport dialog

CDlgImport::CDlgImport(CWnd* pParent /*=NULL*/) : 
CDialog(CDlgImport::IDD, pParent)
{
	m_nMode = 0;
}

void CDlgImport::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_KEEP, m_nMode);
}

BEGIN_MESSAGE_MAP(CDlgImport, CDialog)
END_MESSAGE_MAP()

void CDlgImport::OnOK()
{
	UpdateData(TRUE); // retrieve data
	CDialog::OnOK();
}

BOOL CDlgImport::OnInitDialog()
{
	CDialog::OnInitDialog();
	CenterWindow();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

