// DlgImportElanFinishPage.cpp : implementation file
//
#include "stdafx.h"
#include "DlgImportElanFinishPage.h"
#include "DlgImportElanSheet.h"
#include "Sa_Doc.h"

class CSaDoc;

CDlgImportElanFinishPage::CDlgImportElanFinishPage() :
    CPropertyPage(IDD)
{
}

CDlgImportElanFinishPage::~CDlgImportElanFinishPage()
{
}

BEGIN_MESSAGE_MAP(CDlgImportElanFinishPage, CPropertyPage)
END_MESSAGE_MAP()


BOOL CDlgImportElanFinishPage::OnWizardFinish()
{
    return CPropertyPage::OnWizardFinish();
}

BOOL CDlgImportElanFinishPage::OnSetActive()
{
    CDlgImportElanSheet * pSheet = GetParent();
    pSheet->SetWizardButtons(PSWIZB_BACK|PSWIZB_FINISH);
    return CPropertyPage::OnSetActive();
}

LRESULT CDlgImportElanFinishPage::OnWizardBack()
{
	return IDD_IMPORT_ELAN_SELECT_PAGE;
}

CDlgImportElanSheet * CDlgImportElanFinishPage::GetParent()
{
    return reinterpret_cast<CDlgImportElanSheet *>(((CPropertySheet *)this)->GetParent());
}
