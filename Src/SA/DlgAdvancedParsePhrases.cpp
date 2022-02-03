/////////////////////////////////////////////////////////////////////////////
// advancedParameters.cpp:
// Implementation of the CDlgAdvancedParse
//
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//
// 01/12/2000
//    SDM   Extract from sa_dlg.cpp
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "dlgadvancedparsephrases.h"
#include "mainfrm.h"
#include "sa_view.h"
#include "sa_doc.h"
#include "Segment.h"

IMPLEMENT_DYNAMIC(CDlgAdvancedParsePhrases, CDialog)

CDlgAdvancedParsePhrases::CDlgAdvancedParsePhrases(CSaDoc * pModel) :
    CDialog(CDlgAdvancedParsePhrases::IDD, NULL) {
    // Set parsing parameters to default values.
    CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
    m_nBreakWidth = (int)(1000.0 * pMainFrame->GetPhraseBreakWidth());
    m_nMaxThreshold = pMainFrame->GetMaxThreshold();
    m_nMinThreshold = pMainFrame->GetMinThreshold();
    m_pDoc = pModel;
}

BOOL CDlgAdvancedParsePhrases::Create() {
    return CDialog::Create(CDlgAdvancedParsePhrases::IDD);
}

CDlgAdvancedParsePhrases::~CDlgAdvancedParsePhrases() {
    DestroyWindow();
}

void CDlgAdvancedParsePhrases::Show(LPCTSTR title) {
    CString text;
    GetWindowTextW(text);
    text.Append(L" - ");
    CString a(title);
    int mark = a.Find(L":");
    if (mark!=-1) {
        a.Truncate(mark);
    }
    text.Append(a);
    SetWindowTextW(text);
    ShowWindow(SW_SHOW);
}

void CDlgAdvancedParsePhrases::DoDataExchange(CDataExchange * pDX) {
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_BREAKWIDTHEDIT, m_nBreakWidth);
    DDV_MinMaxInt(pDX, m_nBreakWidth, 1, 999);
    DDX_Text(pDX, IDC_PARSEMAXTHRESHOLDEDIT, m_nMaxThreshold);
    DDV_MinMaxInt(pDX, m_nMaxThreshold, 1, 100);
    DDX_Text(pDX, IDC_PARSEMINTHRESHOLDEDIT, m_nMinThreshold);
    DDV_MinMaxInt(pDX, m_nMinThreshold, 0, 99);
    DDX_Control(pDX, IDOK, m_OKButton);
    DDX_Control(pDX, IDC_APPLY, m_ApplyButton);
}

BOOL CDlgAdvancedParsePhrases::OnInitDialog() {
    CDialog::OnInitDialog();

    // build and place the break width spin control
    m_SpinBreak.Init(IDC_BREAKWIDTHSCROLL, this);
    // build and place the min threshold spin control
    m_SpinMinThreshold.Init(IDC_PARSEMINTHRESHOLDSCROLL, this);
    // build and place the max threshold spin control
    m_SpinMaxThreshold.Init(IDC_PARSEMAXTHRESHOLDSCROLL, this);

    // get parse parameters document member data
    // initialize member data
    CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));
    m_nBreakWidth = (int)(pMainFrame->GetPhraseBreakWidth() * (float)1000 + 0.5);
	m_nMaxThreshold = pMainFrame->GetMaxThreshold();
	m_nMinThreshold = pMainFrame->GetMinThreshold();

    UpdateData(FALSE);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDlgAdvancedParsePhrases::Apply() {
    UpdateData(TRUE);

    // get parse parameters document member data
    // store new data
    CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));
    pMainFrame->SetPhraseBreakWidth((float)m_nBreakWidth / (float)1000);
    pMainFrame->SetMaxThreshold(m_nMaxThreshold);
    pMainFrame->SetMinThreshold(m_nMinThreshold);

    if (!m_pDoc->AdvancedParsePhrase()) {
        Undo();
    }
}

void CDlgAdvancedParsePhrases::Undo() {
    POSITION pos = m_pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)m_pDoc->GetNextView(pos);
    pView->SendMessage(WM_COMMAND,ID_EDIT_UNDO,0);
    pView->RedrawGraphs();
}

/***************************************************************************/
// CDlgAdvancedParse::OnBreakWidthScroll Break width spin control hit
/***************************************************************************/
void CDlgAdvancedParsePhrases::OnBreakWidthScroll() {
    int nData = GetDlgItemInt(IDC_BREAKWIDTHEDIT, NULL, TRUE);
    if (m_SpinBreak.UpperButtonClicked()) {
        nData++;
    } else {
        nData--;
    }
    if (nData > 999) {
        nData = 999;
    }
    if (nData < 1) {
        nData = 1;
    }
    if (nData != m_nBreakWidth) {
        m_nBreakWidth = nData;
    }
    SetDlgItemInt(IDC_BREAKWIDTHEDIT, m_nBreakWidth, TRUE);
}

/***************************************************************************/
// CDlgAdvancedParse::OnMaxThresholdScroll Max threshold spin cntrl hit
/***************************************************************************/
void CDlgAdvancedParsePhrases::OnMaxThresholdScroll() {
    int nData = GetDlgItemInt(IDC_PARSEMAXTHRESHOLDEDIT, NULL, TRUE);
    if (m_SpinMaxThreshold.UpperButtonClicked()) {
        nData++;
    } else {
        nData--;
    }
    if (nData > 100) {
        nData = 100;
    }
    if (nData < 1) {
        nData = 1;
    }
    if (nData != m_nMaxThreshold) {
        m_nMaxThreshold = nData;
        if ((m_nMaxThreshold - 1) < m_nMinThreshold) {
            m_nMinThreshold = m_nMaxThreshold - 1;
            SetDlgItemInt(IDC_PARSEMINTHRESHOLDEDIT, m_nMinThreshold, TRUE);
        }
    }
    SetDlgItemInt(IDC_PARSEMAXTHRESHOLDEDIT, m_nMaxThreshold, TRUE);
}

/***************************************************************************/
// CDlgAdvancedParsePhrase::OnMinThresholdScroll Min threshold spin cntrl hit
/***************************************************************************/
void CDlgAdvancedParsePhrases::OnMinThresholdScroll() {
    int nData = GetDlgItemInt(IDC_PARSEMINTHRESHOLDEDIT, NULL, TRUE);
    if (m_SpinMinThreshold.UpperButtonClicked()) {
        nData++;
    } else {
        nData--;
    }
    if (nData > 99) {
        nData = 99;
    }
    if (nData < 0) {
        nData = 0;
    }
    if (nData != m_nMinThreshold) {
        m_nMinThreshold = nData;
        if (m_nMaxThreshold < (m_nMinThreshold + 1)) {
            m_nMaxThreshold = m_nMinThreshold + 1;
            SetDlgItemInt(IDC_PARSEMAXTHRESHOLDEDIT, m_nMaxThreshold, TRUE);
        }
    }
    SetDlgItemInt(IDC_PARSEMINTHRESHOLDEDIT, m_nMinThreshold, TRUE);
}

BEGIN_MESSAGE_MAP(CDlgAdvancedParsePhrases, CDialog)
    ON_COMMAND(IDC_BREAKWIDTHSCROLL, OnBreakWidthScroll)
    ON_COMMAND(IDC_PARSEMAXTHRESHOLDSCROLL, OnMaxThresholdScroll)
    ON_COMMAND(IDC_PARSEMINTHRESHOLDSCROLL, OnMinThresholdScroll)
    ON_BN_CLICKED(IDC_APPLY, &CDlgAdvancedParsePhrases::OnApply)
END_MESSAGE_MAP()

void CDlgAdvancedParsePhrases::OnOK() {
    Apply();
    CDialog::OnOK();
}

void CDlgAdvancedParsePhrases::OnCancel() {
    Undo();
    CDialog::OnCancel();
}

void CDlgAdvancedParsePhrases::OnApply() {
    Apply();
}
