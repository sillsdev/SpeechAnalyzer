// DlgAlignTranscriptionDataSegmentByPage.cpp : implementation file
//

#include "stdafx.h"
#include "DlgAlignTranscriptionDataSegmentByPage.h"


// CDlgAlignTranscriptionDataSegmentByPage

CDlgAlignTranscriptionDataSegmentByPage::CDlgAlignTranscriptionDataSegmentByPage() :
    CPropertyPage(CDlgAlignTranscriptionDataSegmentByPage::IDD),
    m_nSegmentBy(IDC_KEEP) {
}

CDlgAlignTranscriptionDataSegmentByPage::~CDlgAlignTranscriptionDataSegmentByPage() {
}


BEGIN_MESSAGE_MAP(CDlgAlignTranscriptionDataSegmentByPage, CPropertyPage)
    ON_BN_CLICKED(IDC_KEEP, &CDlgAlignTranscriptionDataSegmentByPage::OnClicked)
    ON_BN_CLICKED(IDC_AUTOMATIC, &CDlgAlignTranscriptionDataSegmentByPage::OnClicked)
    ON_BN_CLICKED(IDC_MANUAL, &CDlgAlignTranscriptionDataSegmentByPage::OnClicked)
END_MESSAGE_MAP()

BOOL CDlgAlignTranscriptionDataSegmentByPage::OnSetActive() {
    CheckRadioButton(IDC_KEEP, IDC_AUTOMATIC, m_nSegmentBy);
    OnClicked();
    CPropertySheet * pSheet = reinterpret_cast<CPropertySheet *>(GetParent());
    pSheet->SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
    return CPropertyPage::OnSetActive();
}

void CDlgAlignTranscriptionDataSegmentByPage::OnClicked() {
    m_nSegmentBy = GetCheckedRadioButton(IDC_KEEP,IDC_MANUAL);
    switch (m_nSegmentBy) {
    case IDC_KEEP:
        SetText(IDC_DETAILS, IDS_AWIZ_DTL_KEEP);
        break;
    case IDC_MANUAL:
        SetText(IDC_DETAILS, IDS_AWIZ_DTL_MANUAL);
        break;
    case IDC_AUTOMATIC:
        SetText(IDC_DETAILS, IDS_AWIZ_DTL_AUTOMATIC);
        break;
    }
}

void CDlgAlignTranscriptionDataSegmentByPage::SetText(int nItem, UINT nIDS) {
    CWnd * pWnd = GetDlgItem(nItem);
    CSaString szText;
    szText.LoadString(nIDS);
    if (pWnd) {
        pWnd->SetWindowText(szText);
    }
}
