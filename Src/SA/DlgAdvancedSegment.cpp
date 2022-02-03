/////////////////////////////////////////////////////////////////////////////
// advancedParameters.cpp:
// Implementation of the CDlgAdvancedSegment
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
#include "dlgadvancedsegment.h"
#include "mainfrm.h"
#include "sa_view.h"
#include "sa_doc.h"

IMPLEMENT_DYNAMIC(CDlgAdvancedSegment, CDialog)

/////////////////////////////////////////////////////////////////////////////
// CDlgAdvancedSegment construction/destruction/creation

/***************************************************************************/
// CDlgAdvancedSegment::CDlgAdvancedSegment Constructor
/***************************************************************************/
CDlgAdvancedSegment::CDlgAdvancedSegment(CSaDoc * pModel) :
    CDialog(CDlgAdvancedSegment::IDD,NULL) {
    m_nSegmentWidth = 20;
    m_nChMinThreshold = 17;
    m_nZCMinThreshold = 50;
    m_pDoc = pModel;
}

BOOL CDlgAdvancedSegment::Create() {
    return CDialog::Create(CDlgAdvancedSegment::IDD);
}

CDlgAdvancedSegment::~CDlgAdvancedSegment() {
    DestroyWindow();
}

void CDlgAdvancedSegment::Show(LPCTSTR title) {
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

/////////////////////////////////////////////////////////////////////////////
// CDlgAdvancedSegment helper functions

/***************************************************************************/
// CDlgAdvancedSegment::DoDataExchange Data exchange
/***************************************************************************/
void CDlgAdvancedSegment::DoDataExchange(CDataExchange * pDX) {
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_SEGMENTWIDTHEDIT, m_nSegmentWidth);
    DDV_MinMaxInt(pDX, m_nSegmentWidth, 1, 999);
    DDX_Text(pDX, IDC_CHANGEMINEDIT, m_nChMinThreshold);
    DDV_MinMaxInt(pDX, m_nChMinThreshold, 0, 99);
    DDX_Text(pDX, IDC_ZEROCROSSINGMINEDIT, m_nZCMinThreshold);
    DDV_MinMaxInt(pDX, m_nZCMinThreshold, 0, 99);
    DDX_Control(pDX, IDOK, m_OKButton);
    DDX_Control(pDX, IDC_APPLY, m_ApplyButton);
}

BOOL CDlgAdvancedSegment::OnInitDialog() {
    CDialog::OnInitDialog();

    // build and place the peak width spin control
    m_SpinSegment.Init(IDC_SEGMENTWIDTHSCROLL, this);
    // build and place the min threshold spin controls
    m_SpinChangeMin.Init(IDC_CHANGEMINSCROLL, this);
    m_SpinZeroCrossingMin.Init(IDC_ZEROCROSSINGMINSCROLL, this);

    // get segment parameters data
    CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));
    CSegmentParm * pSegmentParm = pMainFrame->GetSegmentParm();
    m_nSegmentWidth = (int)(pSegmentParm->fSegmentWidth * (float)1000 + 0.5);
    m_nChMinThreshold = pSegmentParm->nChThreshold;
    m_nZCMinThreshold = pSegmentParm->nZCThreshold;

    UpdateData(FALSE);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDlgAdvancedSegment::Apply() {
  
	UpdateData(TRUE);

    // get segment parameters data
    // save new data
    CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
	pMainFrame->SetSegmentParams( (float)m_nSegmentWidth/1000.0f, m_nChMinThreshold, m_nZCMinThreshold);

    if (!m_pDoc->AdvancedSegment()) {
        Undo();
    }
}

void CDlgAdvancedSegment::Undo() {
    POSITION pos = m_pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)m_pDoc->GetNextView(pos);
    pView->SendMessage(WM_COMMAND,ID_EDIT_UNDO,0);
    pView->RedrawGraphs();
}

/***************************************************************************/
// CDlgAdvancedSegment::OnSegmentWidthScroll Segment width spin control hit
// CLW 10/12/98 was OnSegmentWidthScroll. Changed to reflect change in
// segment parameters page.
/***************************************************************************/
void CDlgAdvancedSegment::OnSegmentWidthScroll() {
    int nData = GetDlgItemInt(IDC_SEGMENTWIDTHEDIT, NULL, TRUE);
    if (m_SpinSegment.UpperButtonClicked()) {
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
    if (nData != m_nSegmentWidth) {
        m_nSegmentWidth = nData;
    }
    SetDlgItemInt(IDC_SEGMENTWIDTHEDIT, m_nSegmentWidth, TRUE);
}

/***************************************************************************/
// CDlgAdvancedSegment::OnChangeMinScroll Change Min spin cntrl hit
// CLW 10/12/98 was OnMinThresholdScroll. Changed to reflect change in
// segment parameters page.
/***************************************************************************/
void CDlgAdvancedSegment::OnChangeMinScroll() {
    int nData = GetDlgItemInt(IDC_CHANGEMINEDIT, NULL, TRUE);
    if (m_SpinChangeMin.UpperButtonClicked()) {
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
    if (nData != m_nChMinThreshold) {
        m_nChMinThreshold = nData;
    }
    SetDlgItemInt(IDC_CHANGEMINEDIT, m_nChMinThreshold, TRUE);
}

/***************************************************************************/
// CDlgAdvancedSegment::OnZeroCrossMinScroll Zero Crossing Min spin cntrl hit
// Added CLW 10/12/98
/***************************************************************************/
void CDlgAdvancedSegment::OnZeroCrossMinScroll() {
    int nData = GetDlgItemInt(IDC_ZEROCROSSINGMINEDIT, NULL, TRUE);
    if (m_SpinZeroCrossingMin.UpperButtonClicked()) {
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
    if (nData != m_nZCMinThreshold) {
        m_nZCMinThreshold = nData;
    }
    SetDlgItemInt(IDC_ZEROCROSSINGMINEDIT, m_nZCMinThreshold, TRUE);
}

BEGIN_MESSAGE_MAP(CDlgAdvancedSegment, CDialog)
    ON_COMMAND(IDC_SEGMENTWIDTHSCROLL, OnSegmentWidthScroll)
    ON_COMMAND(IDC_CHANGEMINSCROLL, OnChangeMinScroll)
    ON_COMMAND(IDC_ZEROCROSSINGMINSCROLL, OnZeroCrossMinScroll)
    ON_BN_CLICKED(IDC_APPLY, &CDlgAdvancedSegment::OnApply)
END_MESSAGE_MAP()

void CDlgAdvancedSegment::OnOK() {
    Apply();
    CDialog::OnOK();
}

void CDlgAdvancedSegment::OnCancel() {
    Undo();
    CDialog::OnCancel();
}

void CDlgAdvancedSegment::OnApply() {
    Apply();
}
