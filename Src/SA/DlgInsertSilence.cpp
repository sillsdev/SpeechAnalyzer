// DlgInsertSilence.cpp : implementation file
//

#include "stdafx.h"
#include "DlgInsertSilence.h"
#include "Sa_Doc.h"
#include "SA_View.h"

// CDlgInsertSilence dialog

IMPLEMENT_DYNAMIC(CDlgInsertSilence, CDialog)

CDlgInsertSilence::CDlgInsertSilence(CSaDoc * pModel, CWnd * pParent /*=NULL*/) :
    pDocument(pModel),
    CDialog(CDlgInsertSilence::IDD, pParent),
    MIN_NUM_VALUE(0.01),
    MAX_NUM_VALUE(2.00),
    repetitions(0),
    silence(0.05) {
}

CDlgInsertSilence::~CDlgInsertSilence() {
}

void CDlgInsertSilence::DoDataExchange(CDataExchange * pDX) {
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SPIN_SILENCE, spinSilence);
    DDX_Text(pDX, IDC_EDIT_SILENCE, silence);
    DDV_MinMaxDouble(pDX, silence, 0.01, 2.00);
}

BEGIN_MESSAGE_MAP(CDlgInsertSilence, CDialog)
    ON_BN_CLICKED(IDC_INSERT, &CDlgInsertSilence::OnClickedInsert)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SILENCE, &CDlgInsertSilence::OnDeltaposTimeSpin)
END_MESSAGE_MAP()

void CDlgInsertSilence::OnClickedInsert() {
    UpdateData(TRUE);

    // get pointer to view
    POSITION pos = pDocument->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pDocument->GetNextView(pos);
    CURSORPOS start = pView->GetStartCursorPosition();
    WAVETIME startt = pDocument->toTime(start, true);
    if (pDocument->InsertSilenceIntoWave(silence, startt, ++repetitions)) {
        // get wave from the clipboard
        pDocument->InvalidateAllProcesses();
        pView->RedrawGraphs();
    }
}

BOOL CDlgInsertSilence::OnInitDialog() {
    CDialog::OnInitDialog();

    spinSilence.SetRange(1,200);

    return TRUE;
}

void CDlgInsertSilence::OnDeltaposTimeSpin(NMHDR * pNMHDR, LRESULT * pResult) {
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

    wchar_t buffer[64];
    wmemset(buffer, 0, _countof(buffer));

    GetDlgItemText(IDC_EDIT_SILENCE, buffer, _countof(buffer));

    silence = _wtof(buffer);

    float delta = (float)pNMUpDown->iDelta;
    delta /= 100.0;

    silence += delta;

    if (silence>MAX_NUM_VALUE) {
        silence = MAX_NUM_VALUE;
    }
    if (silence<MIN_NUM_VALUE) {
        silence = MIN_NUM_VALUE;
    }

    swprintf(buffer, _countof(buffer), L"%0.2f", silence);

    SetDlgItemText(IDC_EDIT_SILENCE, buffer);

    *pResult = 0;
}
