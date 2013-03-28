// DlgAlignTranscriptionDataInitPage.cpp : implementation file
//

#include "stdafx.h"
#include "DlgAlignTranscriptionDataInitPage.h"
#include "DlgAlignTranscriptionDataSheet.h"

// CDlgAlignTranscriptionDataInitPage

CDlgAlignTranscriptionDataInitPage::CDlgAlignTranscriptionDataInitPage() :
    CPropertyPage(CDlgAlignTranscriptionDataInitPage::IDD),
    m_bReference(true),
    m_bPhonetic(true)
{
}

CDlgAlignTranscriptionDataInitPage::~CDlgAlignTranscriptionDataInitPage()
{
}

BEGIN_MESSAGE_MAP(CDlgAlignTranscriptionDataInitPage, CWnd)
    ON_BN_CLICKED(IDC_USE_REFERENCE, &CDlgAlignTranscriptionDataInitPage::OnBnClicked)
    ON_BN_CLICKED(IDC_REFERENCE, &CDlgAlignTranscriptionDataInitPage::OnBnClicked)
    ON_BN_CLICKED(IDC_PHONETIC, &CDlgAlignTranscriptionDataInitPage::OnBnClicked)
    ON_BN_CLICKED(IDC_PHONEMIC, &CDlgAlignTranscriptionDataInitPage::OnBnClicked)
    ON_BN_CLICKED(IDC_ORTHOGRAPHIC, &CDlgAlignTranscriptionDataInitPage::OnBnClicked)
    ON_BN_CLICKED(IDC_GLOSS, &CDlgAlignTranscriptionDataInitPage::OnBnClicked)
END_MESSAGE_MAP()

void CDlgAlignTranscriptionDataInitPage::OnBnClicked()
{
    UpdateNext();
}


BOOL CDlgAlignTranscriptionDataInitPage::OnSetActive()
{
    UpdateNext();
    return CPropertyPage::OnSetActive();
}


void CDlgAlignTranscriptionDataInitPage::DoDataExchange(CDataExchange * pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_GLOSS, m_bGloss);
    DDX_Check(pDX, IDC_ORTHOGRAPHIC, m_bOrthographic);
    DDX_Check(pDX, IDC_PHONEMIC, m_bPhonemic);
    DDX_Check(pDX, IDC_PHONETIC, m_bPhonetic);
    DDX_Check(pDX, IDC_REFERENCE, m_bReference);
    DDX_Check(pDX, IDC_USE_REFERENCE, m_bUseReference);
}

void CDlgAlignTranscriptionDataInitPage::UpdateNext()
{
    bool enable = ((IsDlgButtonChecked(IDC_PHONETIC)!=0)||
                   (IsDlgButtonChecked(IDC_PHONEMIC)!=0)||
                   (IsDlgButtonChecked(IDC_GLOSS)!=0)||
                   (IsDlgButtonChecked(IDC_ORTHOGRAPHIC)!=0));
    CPropertySheet * pSheet = reinterpret_cast<CPropertySheet *>(GetParent());
    if (enable)
    {
        pSheet->SetWizardButtons(PSWIZB_NEXT);
    }
    else
    {
        pSheet->SetWizardButtons(0);
    }

    enable = (IsDlgButtonChecked(IDC_USE_REFERENCE)!=0);
    if (enable)
    {
        CButton * pButton = (CButton *)GetDlgItem(IDC_REFERENCE);
        if (pButton)
        {
            pButton->EnableWindow(!enable);
            pButton->SetCheck(enable);
        }
    }
    else
    {
        CButton * pButton = (CButton *)GetDlgItem(IDC_REFERENCE);
        if (pButton)
        {
            pButton->EnableWindow(!enable);
        }
    }
}

LRESULT CDlgAlignTranscriptionDataInitPage::OnWizardNext()
{
    UpdateData(TRUE);
    CDlgAlignTranscriptionDataSheet * pSheet = reinterpret_cast<CDlgAlignTranscriptionDataSheet *>(GetParent());
    return pSheet->CalculateNext(IDD);
}
