// DlgAlignTranscriptionDataFinishPage.cpp : implementation file
//

#include "stdafx.h"
#include "DlgAlignTranscriptionDataFinishPage.h"
#include "DlgAlignTranscriptionDataSheet.h"
#include "TranscriptionDataSettings.h"

class CSaDoc;

// CDlgAlignTranscriptionDataFinishPage

CDlgAlignTranscriptionDataFinishPage::CDlgAlignTranscriptionDataFinishPage(CSaDoc * pSaDoc) :
    CPropertyPage(CDlgAlignTranscriptionDataFinishPage::IDD),
    m_pSaDoc(pSaDoc)
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

    CDlgAlignTranscriptionDataSheet * pSheet = GetParent();
    pSheet->SetWizardButtons(PSWIZB_BACK|PSWIZB_FINISH);

    CTranscriptionDataSettings tds = pSheet->GetSettings();
    m_pSaDoc->ApplyTranscriptionChanges(tds);

    return CPropertyPage::OnSetActive();
}


LRESULT CDlgAlignTranscriptionDataFinishPage::OnWizardBack()
{
    CDlgAlignTranscriptionDataSheet * pSheet = GetParent();

    m_pSaDoc->RevertTranscriptionChanges();

    return pSheet->CalculateBack(IDD);
}

CDlgAlignTranscriptionDataSheet * CDlgAlignTranscriptionDataFinishPage::GetParent()
{
    return reinterpret_cast<CDlgAlignTranscriptionDataSheet *>(((CPropertySheet *)this)->GetParent());
}
