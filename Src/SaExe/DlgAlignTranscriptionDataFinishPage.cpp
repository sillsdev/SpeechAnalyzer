// DlgAlignTranscriptionDataFinishPage.cpp : implementation file
//

#include "stdafx.h"
#include "DlgAlignTranscriptionDataFinishPage.h"
#include "DlgAlignTranscriptionDataSheet.h"

// CDlgAlignTranscriptionDataFinishPage

CDlgAlignTranscriptionDataFinishPage::CDlgAlignTranscriptionDataFinishPage()
:CPropertyPage(CDlgAlignTranscriptionDataFinishPage::IDD)
{
}

CDlgAlignTranscriptionDataFinishPage::~CDlgAlignTranscriptionDataFinishPage()
{
}


BEGIN_MESSAGE_MAP(CDlgAlignTranscriptionDataFinishPage, CPropertyPage)
END_MESSAGE_MAP()
// CDlgAlignTranscriptionDataFinishPage message handlers


BOOL CDlgAlignTranscriptionDataFinishPage::OnWizardFinish()
{
	return CPropertyPage::OnWizardFinish();
}


BOOL CDlgAlignTranscriptionDataFinishPage::OnSetActive()
{
	CPropertySheet * pSheet = reinterpret_cast<CPropertySheet *>(GetParent());
	pSheet->SetWizardButtons(PSWIZB_BACK|PSWIZB_FINISH);
	return CPropertyPage::OnSetActive();
}


LRESULT CDlgAlignTranscriptionDataFinishPage::OnWizardBack()
{
	CDlgAlignTranscriptionDataSheet * pSheet = reinterpret_cast<CDlgAlignTranscriptionDataSheet *>(GetParent());
	return pSheet->CalculateBack( IDD);
}
