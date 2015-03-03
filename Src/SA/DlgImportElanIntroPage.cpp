// DlgAlignTranscriptionDataTextPage.cpp : implementation file
//

#include "stdafx.h"
#include "DlgImportElanIntroPage.h"
#include "DlgImportElanSheet.h"
#include "objectostream.h"
#include "Sa_Doc.h"
#include "SaString.h"
#include "Segment.h"
#include "DlgImportSFM.h"
#include "TextHelper.h"
#include "StringStream.h"
#include "FileEncodingHelper.h"

static LPCTSTR psz_ImportEnd = L"import";

CDlgImportElanIntroPage::CDlgImportElanIntroPage() :
    CPropertyPage(IDD),
    selection(1) {
}

CDlgImportElanIntroPage::~CDlgImportElanIntroPage() {
}

BEGIN_MESSAGE_MAP(CDlgImportElanIntroPage, CPropertyPage)
    ON_BN_CLICKED(IDC_RADIO_ELAN, &CDlgImportElanIntroPage::OnClickedElan)
    ON_BN_CLICKED(IDC_RADIO_SAYMORE, &CDlgImportElanIntroPage::OnClickedSayMore)
END_MESSAGE_MAP()

BOOL CDlgImportElanIntroPage::OnSetActive() {
    CPropertySheet * pSheet = reinterpret_cast<CPropertySheet *>(GetParent());
    pSheet->SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
    return CPropertyPage::OnSetActive();
}

LRESULT CDlgImportElanIntroPage::OnWizardNext() {
    return IDD_IMPORT_ELAN_SELECT_PAGE;
}

LRESULT CDlgImportElanIntroPage::OnWizardBack() {
    return -1;
}

void CDlgImportElanIntroPage::DoDataExchange(CDataExchange * pDX) {
    // TODO: Add your specialized code here and/or call the base class
    CPropertyPage::DoDataExchange(pDX);
    DDX_Radio(pDX, IDC_RADIO_SAYMORE, selection);

    if (pDX->m_bSaveAndValidate) {
        CDlgImportElanSheet * pSheet = reinterpret_cast<CDlgImportElanSheet *>(GetParent());
        pSheet->selection = selection;
    }
}

BOOL CDlgImportElanIntroPage::OnInitDialog() {
    CPropertyPage::OnInitDialog();
    CString str;
    str.LoadStringW(IDS_IMPORT_ELAN_METHOD_ELAN);
    GetDlgItem(IDC_METHOD_TEXT)->SetWindowTextW(str);
    return TRUE;
}

void CDlgImportElanIntroPage::OnClickedElan() {
    CString str;
    str.LoadStringW(IDS_IMPORT_ELAN_METHOD_ELAN);
    GetDlgItem(IDC_METHOD_TEXT)->SetWindowTextW(str);
}

void CDlgImportElanIntroPage::OnClickedSayMore() {
    CString str;
    str.LoadStringW(IDS_IMPORT_ELAN_METHOD_SAYMORE);
    GetDlgItem(IDC_METHOD_TEXT)->SetWindowTextW(str);
}
