// DlgInsertSilence.cpp : implementation file
//

#include "stdafx.h"
#include "DlgInsertSilence.h"
#include "Sa_Doc.h"
#include "SA_View.h"

// CDlgInsertSilence dialog

IMPLEMENT_DYNAMIC(CDlgInsertSilence, CDialog)

CDlgInsertSilence::CDlgInsertSilence(CSaDoc * pDoc, CWnd* pParent /*=NULL*/) : 
mpDoc(pDoc),
CDialog(CDlgInsertSilence::IDD, pParent),
MIN_NUM_VALUE(0.01),
MAX_NUM_VALUE(2.00)
{
	mSilence = 0.05;
}

CDlgInsertSilence::~CDlgInsertSilence()
{
}

void CDlgInsertSilence::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SPIN_SILENCE, mSpinSilence);
	DDX_Text(pDX, IDC_EDIT_SILENCE, mSilence);
	DDV_MinMaxDouble(pDX, mSilence, 0.01, 2.00);
}

BEGIN_MESSAGE_MAP(CDlgInsertSilence, CDialog)
	ON_BN_CLICKED(IDC_INSERT, &CDlgInsertSilence::OnClickedInsert)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SILENCE, &CDlgInsertSilence::OnDeltaposTimeSpin)
END_MESSAGE_MAP()

void CDlgInsertSilence::OnClickedInsert()
{
	UpdateData(TRUE);

    // get pointer to view
    POSITION pos = mpDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)mpDoc->GetNextView(pos);
	CURSORPOS start = pView->GetStartCursorPosition();
	WAVETIME startt = mpDoc->toTime( start, true);
    if (mpDoc->InsertSilenceIntoWave( mSilence, startt)) 
	{
		// get wave from the clipboard
		mpDoc->InvalidateAllProcesses();
		pView->RefreshGraphs();
	}
}

BOOL CDlgInsertSilence::OnInitDialog()
{
	CDialog::OnInitDialog();

    mSpinSilence.SetRange(1,200);

	return TRUE;
}

void CDlgInsertSilence::OnDeltaposTimeSpin(NMHDR * pNMHDR, LRESULT * pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	wchar_t buffer[64];
	wmemset( buffer, 0, _countof(buffer));

    GetDlgItemText( IDC_EDIT_SILENCE, buffer, _countof(buffer));

	mSilence = _wtof( buffer);

	float delta = (float)pNMUpDown->iDelta;
	delta /= 100.0;

	mSilence += delta;

    if (mSilence>MAX_NUM_VALUE)
    {
        mSilence = MAX_NUM_VALUE;
    }
    if (mSilence<MIN_NUM_VALUE)
    {
        mSilence = MIN_NUM_VALUE;
    }

	swprintf( buffer, _countof(buffer), L"%0.2f", mSilence);

	SetDlgItemText(IDC_EDIT_SILENCE, buffer);

    *pResult = 0;
}
