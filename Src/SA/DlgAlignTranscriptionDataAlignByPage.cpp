// DlgAlignTranscriptionDataAlignByPage.cpp : implementation file
//

#include "stdafx.h"
#include "DlgAlignTranscriptionDataAlignByPage.h"
#include "DlgAlignTranscriptionDataSheet.h"
#include "SaString.h"

CDlgAlignTranscriptionDataAlignByPage::CDlgAlignTranscriptionDataAlignByPage() :
    CPropertyPage(CDlgAlignTranscriptionDataAlignByPage::IDD),
    m_nAlignBy(IDC_CHARACTER)
{
}

CDlgAlignTranscriptionDataAlignByPage::~CDlgAlignTranscriptionDataAlignByPage()
{
}


BEGIN_MESSAGE_MAP(CDlgAlignTranscriptionDataAlignByPage, CPropertyPage)
    ON_BN_CLICKED(IDC_NONE, &CDlgAlignTranscriptionDataAlignByPage::OnClicked)
    ON_BN_CLICKED(IDC_WORD, &CDlgAlignTranscriptionDataAlignByPage::OnClicked)
    ON_BN_CLICKED(IDC_CHARACTER, &CDlgAlignTranscriptionDataAlignByPage::OnClicked)
END_MESSAGE_MAP()

BOOL CDlgAlignTranscriptionDataAlignByPage::OnSetActive()
{
    CheckRadioButton(IDC_NONE, IDC_CHARACTER, m_nAlignBy);
    OnClicked();
    CPropertySheet * pSheet = reinterpret_cast<CPropertySheet *>(GetParent());
    pSheet->SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
    return CPropertyPage::OnSetActive();
}

void CDlgAlignTranscriptionDataAlignByPage::OnClicked()
{
    m_nAlignBy = GetCheckedRadioButton(IDC_NONE,IDC_CHARACTER);
    switch (m_nAlignBy)
    {
    case IDC_NONE:
        SetText(IDC_DETAILS, IDS_AWIZ_DTL_NONE);
        break;
    case IDC_WORD:
        SetText(IDC_DETAILS, IDS_AWIZ_DTL_WORD);
        break;
    case IDC_CHARACTER:
        SetText(IDC_DETAILS, IDS_AWIZ_DTL_CHARACTER);
        break;
    }
}

void CDlgAlignTranscriptionDataAlignByPage::SetText(int nItem, UINT nIDS)
{
    CWnd * pWnd = GetDlgItem(nItem);
    CSaString szText;
    szText.LoadString(nIDS);
    if (pWnd)
    {
        pWnd->SetWindowText(szText);
    }
}


LRESULT CDlgAlignTranscriptionDataAlignByPage::OnWizardBack()
{
    CDlgAlignTranscriptionDataSheet * pSheet = reinterpret_cast<CDlgAlignTranscriptionDataSheet *>(GetParent());
    return pSheet->CalculateBack(IDD);
}
