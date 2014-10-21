// DlgAlignTranscriptionDataTextPage.cpp : implementation file
//

#include "stdafx.h"
#include "DlgAlignTranscriptionDataImportRefPage.h"
#include "DlgAlignTranscriptionDataSheet.h"
#include "objectostream.h"
#include "Sa_Doc.h"
#include "SaString.h"
#include "Segment.h"
#include "TranscriptionData.h"
#include "sa.h"
#include "TextHelper.h"
#include "FileEncodingHelper.h"

CDlgAlignTranscriptionDataImportRefPage::CDlgAlignTranscriptionDataImportRefPage(CSaDoc * pSaDoc) :
    CPropertyPage(IDD),
    m_pSaDoc(pSaDoc),
    m_bModified(false),
    m_szText("")
{
}

CDlgAlignTranscriptionDataImportRefPage::~CDlgAlignTranscriptionDataImportRefPage()
{
}

BEGIN_MESSAGE_MAP(CDlgAlignTranscriptionDataImportRefPage, CPropertyPage)
    ON_BN_CLICKED(IDC_IMPORT, &CDlgAlignTranscriptionDataImportRefPage::OnClickedImport)
    ON_BN_CLICKED(IDC_REVERT, &CDlgAlignTranscriptionDataImportRefPage::OnClickedRevert)
    ON_EN_UPDATE(IDC_ANNOTATION, OnUpdateAnnotation)
END_MESSAGE_MAP()

BOOL CDlgAlignTranscriptionDataImportRefPage::OnSetActive()
{
    CDlgAlignTranscriptionDataSheet * pParent = GetParent();

    CSaApp * pApp = (CSaApp *)AfxGetApp();
    CSaString path = pApp->GetProfileString(L"AutoRef",L"LastImport",L"");

    m_TranscriptionData = CTranscriptionData();

	if (path.GetLength()==0) 
	{
        m_TranscriptionData = CTranscriptionData();
        m_szText = "";
		SetAnnotation();
		SetEnable(IDC_REVERT,m_bModified);
		pParent->SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
		return CPropertyPage::OnSetActive();
	}

	CFileEncodingHelper feh(path);
	if (!feh.CheckEncoding(false)) 
	{
        m_TranscriptionData = CTranscriptionData();
        m_szText = "";
		SetAnnotation();
		SetEnable(IDC_REVERT,m_bModified);
		pParent->SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
		return CPropertyPage::OnSetActive();
	}

	wistringstream stream;
	if (!feh.ConvertFileToUTF16(stream)) 
	{
        m_TranscriptionData = CTranscriptionData();
        m_szText = "";
		SetAnnotation();
		SetEnable(IDC_REVERT,m_bModified);
		pParent->SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
		return CPropertyPage::OnSetActive();
	}

    if (m_pSaDoc->ImportTranscription( stream,
                                       pParent->init.m_bGloss,
									   pParent->init.m_bGlossNat,
                                       pParent->init.m_bPhonetic,
                                       pParent->init.m_bPhonemic,
                                       pParent->init.m_bOrthographic,
                                       m_TranscriptionData,
                                       false,
									   true))
    {
        m_szText = CTranscriptionHelper::Render(m_TranscriptionData);
    }
    else
    {
        m_szText = L"";
    }

    SetAnnotation();
    SetEnable(IDC_REVERT,m_bModified);
    pParent->SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
    return CPropertyPage::OnSetActive();
}


void CDlgAlignTranscriptionDataImportRefPage::OnClickedImport()
{

    CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("Standard Format (*.sfm)(*.txt)|*.sfm;*.txt| All Files (*.*) |*.*||"), this);
    if (dlg.DoModal() != IDOK)
    {
        return;
    }

    CSaString path = dlg.GetPathName();

	CFileEncodingHelper feh(path);
	if (!feh.CheckEncoding(true)) 
	{
		return;
	}

    CSaApp * pApp = (CSaApp *)AfxGetApp();
    pApp->WriteProfileString(L"AutoRef",L"LastImport",(LPCTSTR)path);

    CDlgAlignTranscriptionDataSheet * pParent = GetParent();

    m_TranscriptionData = CTranscriptionData();

	wistringstream stream;
	if (!feh.ConvertFileToUTF16(stream)) 
	{
		return;
	}

    m_bModified = true;
    if (m_pSaDoc->ImportTranscription( stream,
                                       pParent->init.m_bGloss,
                                       pParent->init.m_bGlossNat,
                                       pParent->init.m_bPhonetic,
                                       pParent->init.m_bPhonemic,
                                       pParent->init.m_bOrthographic,
                                       m_TranscriptionData,
                                       false,
									   true))
    {
        m_szText = CTranscriptionHelper::Render(m_TranscriptionData);
        CSaApp * pApp = (CSaApp *)AfxGetApp();
        pApp->WriteProfileString(L"AutoRef",L"LastImport",(LPCTSTR)path);

    }
    else
    {
        m_szText = L"";
    }

    CWnd * pWnd = GetDlgItem(IDC_REVERT);
    if (pWnd!=NULL)
    {
        pWnd->EnableWindow(m_bModified);
    }
    pWnd = GetDlgItem(IDC_ANNOTATION);
    if (pWnd!=NULL)
    {
        pWnd->SetWindowText(m_szText);
        CFont * pFont = (CFont *)m_pSaDoc->GetFont(GLOSS);
        if (pFont!=NULL)
        {
            pWnd->SetFont(pFont);
        }
    }
}


void CDlgAlignTranscriptionDataImportRefPage::OnClickedRevert()
{
    m_bModified = false;
    CDlgAlignTranscriptionDataSheet * pParent = GetParent();

    m_szText = m_pSaDoc->BuildImportString(pParent->init.m_bGloss,
                                           pParent->init.m_bGlossNat,
                                           pParent->init.m_bPhonetic,
                                           pParent->init.m_bPhonemic,
                                           pParent->init.m_bOrthographic);
    CWnd * pWnd = GetDlgItem(IDC_REVERT);
    if (pWnd!=NULL)
    {
        pWnd->EnableWindow(m_bModified);
    }
    pWnd = GetDlgItem(IDC_ANNOTATION);
    if (pWnd!=NULL)
    {
        pWnd->SetWindowText(m_szText);
        CFont * pFont = (CFont *)m_pSaDoc->GetFont(GLOSS);
        if (pFont!=NULL)
        {
            pWnd->SetFont(pFont);
        }
    }
}

void CDlgAlignTranscriptionDataImportRefPage::SaveAnnotation(void)
{

    CWnd * pWnd = GetDlgItem(IDC_ANNOTATION);
    if (pWnd == NULL)
    {
        return;
    }
    pWnd->GetWindowText(m_szText);
}

void CDlgAlignTranscriptionDataImportRefPage::OnUpdateAnnotation()
{
    m_bModified = true;
    SetEnable(IDC_REVERT,m_bModified);
}

void CDlgAlignTranscriptionDataImportRefPage::SetEnable(int nItem, BOOL bEnable)
{
    CWnd * pWnd = GetDlgItem(nItem);
    if (pWnd==NULL)
    {
        return;
    }

    pWnd->EnableWindow(bEnable);
}

void CDlgAlignTranscriptionDataImportRefPage::SetAnnotation()
{
    SetText(IDC_ANNOTATION, m_szText);
    CFont * pFont = (CFont *)m_pSaDoc->GetFont(GLOSS);
    if (pFont==NULL)
    {
        return;
    }

    CEdit * pEdit = (CEdit *)GetDlgItem(IDC_ANNOTATION);
    if (pEdit==NULL)
    {
        return;
    }

    pEdit->SetFont(pFont);

    // disable selection after the control is drawn...
    pEdit->PostMessageW(EM_SETSEL,-1,0);
}

void CDlgAlignTranscriptionDataImportRefPage::SetText(int nItem, CSaString szText)
{
    CWnd * pWnd = GetDlgItem(nItem);
    if (pWnd==NULL)
    {
        return;
    }
    pWnd->SetWindowText(szText);
}


LRESULT CDlgAlignTranscriptionDataImportRefPage::OnWizardNext()
{
    CDlgAlignTranscriptionDataSheet * pSheet = GetParent();
    return pSheet->CalculateNext(IDD);
}


LRESULT CDlgAlignTranscriptionDataImportRefPage::OnWizardBack()
{
    CDlgAlignTranscriptionDataSheet * pSheet = GetParent();
    return pSheet->CalculateBack(IDD);
}

CDlgAlignTranscriptionDataSheet * CDlgAlignTranscriptionDataImportRefPage::GetParent()
{
    return reinterpret_cast<CDlgAlignTranscriptionDataSheet *>(((CPropertySheet *)this)->GetParent());
}
