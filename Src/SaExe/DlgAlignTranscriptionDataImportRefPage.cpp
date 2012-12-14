// DlgAlignTranscriptionDataTextPage.cpp : implementation file
//

#include "stdafx.h"
#include "DlgAlignTranscriptionDataImportRefPage.h"
#include "DlgAlignTranscriptionDataSheet.h"
#include "settings\obstream.h"
#include "Sa_Doc.h"
#include "CSaString.h"
#include "Sa_segm.h"
#include "TranscriptionData.h"

CDlgAlignTranscriptionDataImportRefPage::CDlgAlignTranscriptionDataImportRefPage(  CSaDoc * pSaDoc) :
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

// CDlgAlignTranscriptionDataImportRefPage message handlers

BOOL CDlgAlignTranscriptionDataImportRefPage::OnSetActive() 
{
	CDlgAlignTranscriptionDataSheet * pParent = GetParent();

	CSaString path = m_pSaDoc->GetLastTranscriptionImport();
	if (path.GetLength()!=0)
	{
		m_TranscriptionData = m_pSaDoc->ImportTranscription(path,
															pParent->init.m_bGloss,
															pParent->init.m_bPhonetic,
									 						pParent->init.m_bPhonemic,
															pParent->init.m_bOrthographic);
		m_szText = CTranscriptionHelper::Render(m_TranscriptionData);
	}
	else
	{
		m_TranscriptionData = CTranscriptionData();
		m_szText = "";
	}

	SetAnnotation();
	SetEnable(IDC_REVERT,m_bModified);
	pParent->SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
	return CPropertyPage::OnSetActive();
}


void CDlgAlignTranscriptionDataImportRefPage::OnClickedImport()
{

	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("Text Files (*.txt)|*.txt|Standard Format (*.sfm)|*.sfm| All Files (*.*) |*.*||"), this );
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	CSaString path = dlg.GetPathName();

	CDlgAlignTranscriptionDataSheet * pParent = GetParent();

	m_bModified = true;
	m_TranscriptionData = m_pSaDoc->ImportTranscription(path,
													    pParent->init.m_bGloss,
													    pParent->init.m_bPhonetic,
									 				    pParent->init.m_bPhonemic,
													    pParent->init.m_bOrthographic);

	m_szText = CTranscriptionHelper::Render(m_TranscriptionData);
	CWnd* pWnd = GetDlgItem(IDC_REVERT);
	if (pWnd!=NULL) 
	{
		pWnd->EnableWindow(m_bModified);
	}
	pWnd = GetDlgItem(IDC_ANNOTATION);
	if (pWnd!=NULL) 
	{
		pWnd->SetWindowText(m_szText);
		CFont * pFont = (CFont*)m_pSaDoc->GetFont(GLOSS);
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

	m_szText = m_pSaDoc->BuildImportString( pParent->init.m_bGloss,
											pParent->init.m_bPhonetic,
											pParent->init.m_bPhonemic,
											pParent->init.m_bOrthographic);
	CWnd* pWnd = GetDlgItem(IDC_REVERT);
	if (pWnd!=NULL) 
	{
		pWnd->EnableWindow(m_bModified);
	}
	pWnd = GetDlgItem(IDC_ANNOTATION);
	if (pWnd!=NULL) 
	{
		pWnd->SetWindowText(m_szText);
		CFont * pFont = (CFont*)m_pSaDoc->GetFont(GLOSS);
		if (pFont!=NULL) 
		{
			pWnd->SetFont(pFont);
		}
	}
}

void CDlgAlignTranscriptionDataImportRefPage::SaveAnnotation(void) 
{
	CWnd* pWnd = GetDlgItem( IDC_ANNOTATION);
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
	CWnd* pWnd = GetDlgItem(nItem);
	if (pWnd==NULL) return;

	pWnd->EnableWindow(bEnable);
}

void CDlgAlignTranscriptionDataImportRefPage::SetAnnotation() 
{
	SetText(IDC_ANNOTATION, m_szText);
	CFont * pFont = (CFont*)m_pSaDoc->GetFont(GLOSS);
	if (pFont==NULL) return;

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_ANNOTATION);
	if (pEdit==NULL) return;

	pEdit->SetFont(pFont);

	// disable selection after the control is drawn...
	pEdit->PostMessageW(EM_SETSEL,-1,0);
}

void CDlgAlignTranscriptionDataImportRefPage::SetText(int nItem, CSaString szText) 
{
	CWnd * pWnd = GetDlgItem(nItem);
	if (pWnd==NULL) return;
	pWnd->SetWindowText(szText);
}


LRESULT CDlgAlignTranscriptionDataImportRefPage::OnWizardNext() 
{
	CDlgAlignTranscriptionDataSheet * pSheet = GetParent();
	return pSheet->CalculateNext( IDD);
}


LRESULT CDlgAlignTranscriptionDataImportRefPage::OnWizardBack() 
{
	CDlgAlignTranscriptionDataSheet * pSheet = GetParent();
	return pSheet->CalculateBack( IDD);
}

CDlgAlignTranscriptionDataSheet * CDlgAlignTranscriptionDataImportRefPage::GetParent() 
{
	return reinterpret_cast<CDlgAlignTranscriptionDataSheet *>(((CPropertySheet*)this)->GetParent());
}
