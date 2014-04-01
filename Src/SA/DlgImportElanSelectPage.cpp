// DlgImportElanSelectPage.cpp : implementation file
//
#include "stdafx.h"
#include "DlgImportElanSelectPage.h"
#include "DlgImportElanSheet.h"

#define SAYMORE_TRANSCRIPTION L"Transcription"
#define SAYMORE_TRANSLATION L"Phrase Free Translation"

CDlgImportElanSelectPage::CDlgImportElanSelectPage() :
    CPropertyPage(IDD)
{
}

CDlgImportElanSelectPage::~CDlgImportElanSelectPage()
{
}

BEGIN_MESSAGE_MAP(CDlgImportElanSelectPage, CPropertyPage)
END_MESSAGE_MAP()

BOOL CDlgImportElanSelectPage::OnSetActive()
{
    CPropertySheet * pSheet = reinterpret_cast<CPropertySheet *>(GetParent());
    pSheet->SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
    return CPropertyPage::OnSetActive();
}

LRESULT CDlgImportElanSelectPage::OnWizardNext()
{
	return IDD_IMPORT_ELAN_FINISH_PAGE;
}


LRESULT CDlgImportElanSelectPage::OnWizardBack()
{
	return IDD_IMPORT_ELAN_INTRO_PAGE;
}

void CDlgImportElanSelectPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
    DDX_Control( pDX, IDC_COMBO_REF_NUM, refnum);
    DDX_Control( pDX, IDC_COMBO_PHONETIC, phonetic);
    DDX_Control( pDX, IDC_COMBO_PHONEMIC, phonemic);
    DDX_Control( pDX, IDC_COMBO_ORTHO, ortho);
    DDX_Control( pDX, IDC_COMBO_GLOSS, gloss);
    DDX_Control( pDX, IDC_COMBO_POS, pos);
    DDX_Control( pDX, IDC_COMBO_PL1, pl1);
    DDX_Control( pDX, IDC_COMBO_PL2, pl2);
    DDX_Control( pDX, IDC_COMBO_PL3, pl3);
    DDX_Control( pDX, IDC_COMBO_PL4, pl4);

	if (pDX->m_bSaveAndValidate) {
	    CDlgImportElanSheet * pSheet = reinterpret_cast<CDlgImportElanSheet *>(GetParent());
		Elan::CAnnotationDocument & document = pSheet->document;
		ElanMap result;
		// first selection is blank line
		if (refnum.GetCurSel()>0) {
			result[REFERENCE] = document.tiers[refnum.GetCurSel()-1].tierID.c_str();
		}
		if (phonetic.GetCurSel()>0) {
			result[PHONETIC] = document.tiers[phonetic.GetCurSel()-1].tierID.c_str();
		}
		if (phonemic.GetCurSel()>0) {
			result[PHONEMIC] = document.tiers[phonemic.GetCurSel()-1].tierID.c_str();
		}
		if (ortho.GetCurSel()>0) {
			result[ORTHO] = document.tiers[ortho.GetCurSel()-1].tierID.c_str();
		}
		if (gloss.GetCurSel()>0) {
			result[GLOSS] = document.tiers[gloss.GetCurSel()-1].tierID.c_str();
		}
		if (pos.GetCurSel()>0) {
			result[TONE] = document.tiers[pos.GetCurSel()-1].tierID.c_str();
		}
		if (pl1.GetCurSel()>0) {
			result[MUSIC_PL1] = document.tiers[pl1.GetCurSel()-1].tierID.c_str();
		}
		if (pl2.GetCurSel()>0) {
			result[MUSIC_PL2] = document.tiers[pl2.GetCurSel()-1].tierID.c_str();
		}
		if (pl3.GetCurSel()>0) {
			result[MUSIC_PL3] = document.tiers[pl3.GetCurSel()-1].tierID.c_str();
		}
		if (pl4.GetCurSel()>0) {
			result[MUSIC_PL4] = document.tiers[pl4.GetCurSel()-1].tierID.c_str();
		}
		pSheet->assignments = result;
	}
}

BOOL CDlgImportElanSelectPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

    CDlgImportElanSheet * pSheet = reinterpret_cast<CDlgImportElanSheet *>(GetParent());
	Elan::CAnnotationDocument & document = pSheet->document;
	int selection = pSheet->selection;

	refnum.AddString(L"");
	phonetic.AddString(L"");
	phonemic.AddString(L"");
	ortho.AddString(L"");
	gloss.AddString(L"");
	pos.AddString(L"");
	pl1.AddString(L"");
	pl2.AddString(L"");
	pl3.AddString(L"");
	pl4.AddString(L"");

	for (int i=0;i<document.tiers.size();i++) {
		refnum.AddString(document.tiers[i].tierID.c_str());
		phonetic.AddString(document.tiers[i].tierID.c_str());
		phonemic.AddString(document.tiers[i].tierID.c_str());
		ortho.AddString(document.tiers[i].tierID.c_str());
		gloss.AddString(document.tiers[i].tierID.c_str());
		pos.AddString(document.tiers[i].tierID.c_str());
		pl1.AddString(document.tiers[i].tierID.c_str());
		pl2.AddString(document.tiers[i].tierID.c_str());
		pl3.AddString(document.tiers[i].tierID.c_str());
		pl4.AddString(document.tiers[i].tierID.c_str());
	}

	refnum.SetCurSel(0);
	phonetic.SetCurSel(0);
	phonemic.SetCurSel(0);
	ortho.SetCurSel(0);
	gloss.SetCurSel(0);
	pos.SetCurSel(0);
	pl1.SetCurSel(0);
	pl2.SetCurSel(0);
	pl3.SetCurSel(0);
	pl4.SetCurSel(0);

	// resize the boxes
    CRect cbSize;           // current size of combo box
    refnum.GetClientRect(cbSize);
    int height = refnum.GetItemHeight(-1);
    height += refnum.GetItemHeight(0) * 20;
    height += GetSystemMetrics(SM_CYEDGE) * 2;
    height += GetSystemMetrics(SM_CYEDGE) * 2;
    refnum.SetWindowPos(NULL,0,0,cbSize.right,height,SWP_NOMOVE|SWP_NOZORDER);
    phonetic.SetWindowPos(NULL,0,0,cbSize.right,height,SWP_NOMOVE|SWP_NOZORDER);
    phonemic.SetWindowPos(NULL,0,0,cbSize.right,height,SWP_NOMOVE|SWP_NOZORDER);
    ortho.SetWindowPos(NULL,0,0,cbSize.right,height,SWP_NOMOVE|SWP_NOZORDER);
    gloss.SetWindowPos(NULL,0,0,cbSize.right,height,SWP_NOMOVE|SWP_NOZORDER);
    pos.SetWindowPos(NULL,0,0,cbSize.right,height,SWP_NOMOVE|SWP_NOZORDER);
    pl1.SetWindowPos(NULL,0,0,cbSize.right,height,SWP_NOMOVE|SWP_NOZORDER);
    pl2.SetWindowPos(NULL,0,0,cbSize.right,height,SWP_NOMOVE|SWP_NOZORDER);
    pl3.SetWindowPos(NULL,0,0,cbSize.right,height,SWP_NOMOVE|SWP_NOZORDER);
    pl4.SetWindowPos(NULL,0,0,cbSize.right,height,SWP_NOMOVE|SWP_NOZORDER);

	// if the user started with a SayMore file, then we should look for the
	// 'Translation' and 'Transcription' tags and preassign those
	if (selection==0) {
		int found = 0;
		for (int i=0;i<document.tiers.size();i++) {
			if (_wcsicmp(document.tiers[i].tierID.c_str(),SAYMORE_TRANSCRIPTION)==0) found++;
			else if (_wcsicmp(document.tiers[i].tierID.c_str(),SAYMORE_TRANSLATION)==0) found++;
		}
		if (found==2) {
			phonetic.SelectString(0,SAYMORE_TRANSCRIPTION);
			pl1.SelectString(0,SAYMORE_TRANSLATION);
		}
	}

	return TRUE;
}
