// DlgAlignTranscriptionDataTextPage.cpp : implementation file
//

#include "stdafx.h"
#include "DlgAlignTranscriptionDataTextPage.h"
#include "DlgAlignTranscriptionDataSheet.h"
#include "objectostream.h"
#include "Sa_Doc.h"
#include "SaString.h"
#include "Segment.h"
#include "DlgImportSFM.h"
#include "TextHelper.h"
#include "StringStream.h"
#include "FileEncodingHelper.h"

static LPCTSTR psz_ImportEnd = L"import";

CDlgAlignTranscriptionDataTextPage::CDlgAlignTranscriptionDataTextPage(CSaDoc * pSaDoc, EAnnotation type, int aIDD) :
    CPropertyPage(aIDD),
    m_pSaDoc(pSaDoc),
    m_Type(type),
    m_bModified(false),
    m_szText(""),
    IDD(aIDD) {
}

CDlgAlignTranscriptionDataTextPage::~CDlgAlignTranscriptionDataTextPage() {
}


BEGIN_MESSAGE_MAP(CDlgAlignTranscriptionDataTextPage, CPropertyPage)
    ON_BN_CLICKED(IDC_IMPORT, &CDlgAlignTranscriptionDataTextPage::OnClickedImport)
    ON_BN_CLICKED(IDC_REVERT, &CDlgAlignTranscriptionDataTextPage::OnClickedRevert)
    ON_EN_UPDATE(IDC_ANNOTATION, OnUpdateAnnotation)
END_MESSAGE_MAP()

// CDlgAlignTranscriptionDataTextPage message handlers

BOOL CDlgAlignTranscriptionDataTextPage::OnSetActive() {
    m_szText = m_pSaDoc->BuildString(m_Type);
    SetAnnotation();
    SetEnable(IDC_REVERT,m_bModified);
    CPropertySheet * pSheet = reinterpret_cast<CPropertySheet *>(GetParent());
    pSheet->SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
    return CPropertyPage::OnSetActive();
}


void CDlgAlignTranscriptionDataTextPage::OnClickedImport() {

    CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("Standard Format (*.sfm)(*.txt)|*.sfm;*.txt| All Files (*.*) |*.*||"), this);
    if (dlg.DoModal() != IDOK) {
        return;
    }

    CString temp = dlg.GetPathName();
    CFileEncodingHelper feh(temp);

    if (!feh.CheckEncoding(true)) {
        return;
    }

    wistringstream input;
    if (!feh.ConvertFileToUTF16(input)) {
        // display a message here
        return;
    }


    CStringStream stream(input.str().c_str());
    if (stream.bFail()) {
        return;
    }

    bool SFM = false;

    SaveAnnotation();

    if (stream.bAtBackslash()) {
        // assume SFM
        CDlgAlignTranscriptionDataSheet * pSheet = reinterpret_cast<CDlgAlignTranscriptionDataSheet *>(GetParent());

        CDlgImportSFM dlg2(pSheet->init.m_bPhonetic,pSheet->init.m_bPhonemic,pSheet->init.m_bOrthographic);
        if (dlg2.DoModal()==IDCANCEL) {
            return;
        }

        SFM = true;

        while ((SFM) && (!stream.bAtEnd())) {
            if ((dlg2.m_bPhonetic) && (stream.ReadStreamString(dlg2.m_szPhonetic,pSheet->phonetic.m_szText)));
            else if ((dlg2.m_bPhonemic) && (stream.ReadStreamString(dlg2.m_szPhonemic,pSheet->phonemic.m_szText)));
            else if ((dlg2.m_bOrthographic) && (stream.ReadStreamString(dlg2.m_szOrthographic,pSheet->ortho.m_szText)));
            else if ((dlg2.m_bGloss)  && (stream.ReadStreamString(dlg2.m_szGloss,pSheet->gloss.m_szText)));
            else if ((dlg2.m_bGlossNat)  && (stream.ReadStreamString(dlg2.m_szGlossNat,pSheet->glossNat.m_szText)));
            else if (stream.bEnd(psz_ImportEnd)) {
                break;
            }
        }

        if (SFM) {
            SetAnnotation();
        }
    }

    if (!SFM) {
        // for use with plain text
        CSaString buffer = input.str().c_str();
        SetText(IDC_ANNOTATION, buffer);
        OnUpdateAnnotation(); // Set Modified
    }
}


void CDlgAlignTranscriptionDataTextPage::OnClickedRevert() {

    m_bModified = false;
    m_szText = m_pSaDoc->BuildString(m_Type);
    CWnd * pWnd = GetDlgItem(IDC_REVERT);
    if (pWnd!=NULL) {
        pWnd->EnableWindow(m_bModified);
    }
    pWnd = GetDlgItem(IDC_ANNOTATION);
    if (pWnd!=NULL) {
        pWnd->SetWindowText(m_szText);
        CFont * pFont = (CFont *)m_pSaDoc->GetFont(m_Type);
        if (pFont!=NULL) {
            pWnd->SetFont(pFont);
        }
    }
}

void CDlgAlignTranscriptionDataTextPage::SaveAnnotation(void) {
    CWnd * pWnd = GetDlgItem(IDC_ANNOTATION);
    if (pWnd == NULL) {
        return;
    }
    pWnd->GetWindowText(m_szText);
}

void CDlgAlignTranscriptionDataTextPage::OnUpdateAnnotation() {
    m_bModified = true;
    SetEnable(IDC_REVERT,m_bModified);
}

void CDlgAlignTranscriptionDataTextPage::SetEnable(int nItem, BOOL bEnable) {
    CWnd * pWnd = GetDlgItem(nItem);
    if (pWnd==NULL) {
        return;
    }

    pWnd->EnableWindow(bEnable);
}

void CDlgAlignTranscriptionDataTextPage::SetAnnotation() {

    SetText(IDC_ANNOTATION, m_szText);
    CFont * pFont = (CFont *)m_pSaDoc->GetFont(m_Type);
    if (pFont==NULL) {
        return;
    }

    CEdit * pEdit = (CEdit *)GetDlgItem(IDC_ANNOTATION);
    if (pEdit==NULL) {
        return;
    }

    pEdit->SetFont(pFont);

    // disable selection after the control is drawn...
    pEdit->PostMessageW(EM_SETSEL,-1,0);
}

void CDlgAlignTranscriptionDataTextPage::SetText(int nItem, CSaString szText) {
    CWnd * pWnd = GetDlgItem(nItem);
    if (pWnd==NULL) {
        return;
    }

    pWnd->SetWindowText(szText);
}


LRESULT CDlgAlignTranscriptionDataTextPage::OnWizardNext() {
    CDlgAlignTranscriptionDataSheet * pSheet = reinterpret_cast<CDlgAlignTranscriptionDataSheet *>(GetParent());
    return pSheet->CalculateNext(IDD);
}


LRESULT CDlgAlignTranscriptionDataTextPage::OnWizardBack() {
    CDlgAlignTranscriptionDataSheet * pSheet = reinterpret_cast<CDlgAlignTranscriptionDataSheet *>(GetParent());
    return pSheet->CalculateBack(IDD);
}
