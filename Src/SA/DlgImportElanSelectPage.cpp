// DlgImportElanSelectPage.cpp : implementation file
//
#include "stdafx.h"
#include "DlgImportElanSelectPage.h"
#include "DlgImportElanSheet.h"

#define SAYMORE_TRANSCRIPTION L"Transcription"
#define SAYMORE_TRANSLATION L"Phrase Free Translation"

CDlgImportElanSelectPage::CDlgImportElanSelectPage() :
    CPropertyPage(IDD) {
}

CDlgImportElanSelectPage::~CDlgImportElanSelectPage() {
}

BEGIN_MESSAGE_MAP(CDlgImportElanSelectPage, CPropertyPage)
END_MESSAGE_MAP()

BOOL CDlgImportElanSelectPage::OnSetActive() {
    CPropertySheet * pSheet = reinterpret_cast<CPropertySheet *>(GetParent());
    pSheet->SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
    return CPropertyPage::OnSetActive();
}

LRESULT CDlgImportElanSelectPage::OnWizardNext() {
    return IDD_IMPORT_ELAN_FINISH_PAGE;
}


LRESULT CDlgImportElanSelectPage::OnWizardBack() {
    return IDD_IMPORT_ELAN_INTRO_PAGE;
}

void CDlgImportElanSelectPage::DoDataExchange(CDataExchange * pDX) {
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_REF_NUM, refnum);
    DDX_Control(pDX, IDC_COMBO_PHONETIC, phonetic);
    DDX_Control(pDX, IDC_COMBO_PHONEMIC, phonemic);
    DDX_Control(pDX, IDC_COMBO_ORTHO, ortho);
    DDX_Control(pDX, IDC_COMBO_GLOSS, gloss);
    DDX_Control(pDX, IDC_COMBO_GLOSS_NAT, glossNat);
    DDX_Control(pDX, IDC_COMBO_PL1, pl1);
    DDX_Control(pDX, IDC_COMBO_PL2, pl2);
    DDX_Control(pDX, IDC_COMBO_PL3, pl3);
    DDX_Control(pDX, IDC_COMBO_PL4, pl4);

    if (pDX->m_bSaveAndValidate) {
        CDlgImportElanSheet * pSheet = reinterpret_cast<CDlgImportElanSheet *>(GetParent());
        Elan::CAnnotationDocument & document = pSheet->document;
        ElanMap result;
        // first selection is blank line
        if (refnum.GetCurSel()!=CB_ERR) {
			int idx = refnum.GetCurSel();
			int sel = refnum.GetItemData(idx);
			if (sel!=-1) {
				result[REFERENCE] = document.tiers[sel].tierID.c_str();
			}
        }
        if (phonetic.GetCurSel()!=CB_ERR) {
			int idx = phonetic.GetCurSel();
			int sel = phonetic.GetItemData(idx);
			if (sel!=-1) {
				result[PHONETIC] = document.tiers[sel].tierID.c_str();
			}
        }
        if (phonemic.GetCurSel()!=CB_ERR) {
			int idx = phonemic.GetCurSel();
			int sel = phonemic.GetItemData(idx);
			if (sel!=-1) {
				result[PHONEMIC] = document.tiers[sel].tierID.c_str();
			}
        }
        if (ortho.GetCurSel()!=CB_ERR) {
			int idx = ortho.GetCurSel();
			int sel = ortho.GetItemData(idx);
			if (sel!=-1) {
				result[ORTHO] = document.tiers[sel].tierID.c_str();
			}
        }
        if (gloss.GetCurSel()!=CB_ERR) {
			int idx = gloss.GetCurSel();
			int sel = gloss.GetItemData(idx);
			if (sel!=-1) {
				result[GLOSS] = document.tiers[sel].tierID.c_str();
			}
        }
        if (glossNat.GetCurSel()!=CB_ERR) {
			int idx = glossNat.GetCurSel();
			int sel = glossNat.GetItemData(idx);
			if (sel!=-1) {
				result[GLOSS_NAT] = document.tiers[sel].tierID.c_str();
			}
        }
        if (pl1.GetCurSel()!=CB_ERR) {
			int idx = pl1.GetCurSel();
			int sel = pl1.GetItemData(idx);
			if (sel!=-1) {
				result[MUSIC_PL1] = document.tiers[sel].tierID.c_str();
			}
        }
        if (pl2.GetCurSel()!=CB_ERR) {
			int idx = pl2.GetCurSel();
			int sel = pl2.GetItemData(idx);
			if (sel!=-1) {
			    result[MUSIC_PL2] = document.tiers[sel].tierID.c_str();
			}
        }
        if (pl3.GetCurSel()!=CB_ERR) {
			int idx = pl3.GetCurSel();
			int sel = pl3.GetItemData(idx);
			if (sel!=-1) {
		        result[MUSIC_PL3] = document.tiers[sel].tierID.c_str();
			}
        }
        if (pl4.GetCurSel()!=CB_ERR) {
			int idx = pl4.GetCurSel();
			int sel = pl4.GetItemData(idx);
			if (sel!=-1) {
	            result[MUSIC_PL4] = document.tiers[sel].tierID.c_str();
			}
        }
        pSheet->assignments = result;
    }
}

BOOL CDlgImportElanSelectPage::OnInitDialog() {
    CPropertyPage::OnInitDialog();

    CDlgImportElanSheet * pSheet = reinterpret_cast<CDlgImportElanSheet *>(GetParent());
    Elan::CAnnotationDocument & document = pSheet->document;
    int selection = pSheet->selection;

    int idx = refnum.AddString(L"");
	refnum.SetItemData(idx,-1);
    idx = phonetic.AddString(L"");
	phonetic.SetItemData(idx,-1);
    idx = phonemic.AddString(L"");
	phonemic.SetItemData(idx,-1);
    idx = ortho.AddString(L"");
	ortho.SetItemData(idx,-1);
    idx = gloss.AddString(L"");
	gloss.SetItemData(idx,-1);
    idx = glossNat.AddString(L"");
	glossNat.SetItemData(idx,-1);
    idx = pl1.AddString(L"");
	pl1.SetItemData(idx,-1);
    idx = pl2.AddString(L"");
	pl2.SetItemData(idx,-1);
    idx = pl3.AddString(L"");
	pl3.SetItemData(idx,-1);
    idx = pl4.AddString(L"");
	pl4.SetItemData(idx,-1);

    for (size_t i=0; i<document.tiers.size(); i++) {
        idx = refnum.AddString(document.tiers[i].tierID.c_str());
		refnum.SetItemData(idx,i);
        idx = phonetic.AddString(document.tiers[i].tierID.c_str());
		phonetic.SetItemData(idx,i);
        idx = phonemic.AddString(document.tiers[i].tierID.c_str());
		phonemic.SetItemData(idx,i);
        idx = ortho.AddString(document.tiers[i].tierID.c_str());
		ortho.SetItemData(idx,i);
        idx = gloss.AddString(document.tiers[i].tierID.c_str());
		gloss.SetItemData(idx,i);
        idx = glossNat.AddString(document.tiers[i].tierID.c_str());
		glossNat.SetItemData(idx,i);
        idx = pl1.AddString(document.tiers[i].tierID.c_str());
		pl1.SetItemData(idx,i);
        idx = pl2.AddString(document.tiers[i].tierID.c_str());
		pl2.SetItemData(idx,i);
        idx = pl3.AddString(document.tiers[i].tierID.c_str());
		pl3.SetItemData(idx,i);
        idx = pl4.AddString(document.tiers[i].tierID.c_str());
		pl4.SetItemData(idx,i);
    }

    refnum.SetCurSel(0);
    phonetic.SetCurSel(0);
    phonemic.SetCurSel(0);
    ortho.SetCurSel(0);
    gloss.SetCurSel(0);
    glossNat.SetCurSel(0);
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
    glossNat.SetWindowPos(NULL,0,0,cbSize.right,height,SWP_NOMOVE|SWP_NOZORDER);
    pl1.SetWindowPos(NULL,0,0,cbSize.right,height,SWP_NOMOVE|SWP_NOZORDER);
    pl2.SetWindowPos(NULL,0,0,cbSize.right,height,SWP_NOMOVE|SWP_NOZORDER);
    pl3.SetWindowPos(NULL,0,0,cbSize.right,height,SWP_NOMOVE|SWP_NOZORDER);
    pl4.SetWindowPos(NULL,0,0,cbSize.right,height,SWP_NOMOVE|SWP_NOZORDER);

    // if the user started with a SayMore file, then we should look for the
    // 'Translation' and 'Transcription' tags and preassign those
    if (selection==0) {
        int found = 0;
        for (size_t i=0; i<document.tiers.size(); i++) {
            if (_wcsicmp(document.tiers[i].tierID.c_str(),SAYMORE_TRANSCRIPTION)==0) {
                found++;
            } else if (_wcsicmp(document.tiers[i].tierID.c_str(),SAYMORE_TRANSLATION)==0) {
                found++;
            }
        }
        if (found==2) {
            phonetic.SelectString(0,SAYMORE_TRANSCRIPTION);
            pl1.SelectString(0,SAYMORE_TRANSLATION);
        }
    }

    return TRUE;
}
