// SelectTranscriptionBars.cpp : implementation file
//

#include "stdafx.h"
#include "DlgSelectTranscriptionBars.h"

// CDlgSelectTranscriptionBars dialog

IMPLEMENT_DYNAMIC(CDlgSelectTranscriptionBars, CDialog)

CDlgSelectTranscriptionBars::CDlgSelectTranscriptionBars(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSelectTranscriptionBars::IDD, pParent) {
}

CDlgSelectTranscriptionBars::~CDlgSelectTranscriptionBars() {
}

void CDlgSelectTranscriptionBars::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
    DDX_Check(pDX, ID_POPUPGRAPH_REFERENCE, bReference);
    DDX_Check(pDX, ID_POPUPGRAPH_PHONETIC, bPhonetic);
    DDX_Check(pDX, ID_POPUPGRAPH_TONE, bTone);
    DDX_Check(pDX, ID_POPUPGRAPH_PHONEMIC, bPhonemic);
    DDX_Check(pDX, ID_POPUPGRAPH_ORTHO, bOrthographic);
    DDX_Check(pDX, ID_POPUPGRAPH_GLOSS, bGloss);
    DDX_Check(pDX, ID_POPUPGRAPH_GLOSS_NAT, bGlossNat);
    DDX_Check(pDX, ID_POPUPGRAPH_MUSIC_PL1, bPhraseList1);
    DDX_Check(pDX, ID_POPUPGRAPH_MUSIC_PL2, bPhraseList2);
    DDX_Check(pDX, ID_POPUPGRAPH_MUSIC_PL3, bPhraseList3);
    DDX_Check(pDX, ID_POPUPGRAPH_MUSIC_PL4, bPhraseList4);
}

BEGIN_MESSAGE_MAP(CDlgSelectTranscriptionBars, CDialog)
END_MESSAGE_MAP()
