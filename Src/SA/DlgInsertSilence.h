#pragma once

class CSaDoc;

// CDlgInsertSilence dialog

class CDlgInsertSilence : public CDialog
{
	DECLARE_DYNAMIC(CDlgInsertSilence)

public:
	CDlgInsertSilence( CSaDoc * doc, CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgInsertSilence();

	afx_msg void OnClickedInsert();
	virtual BOOL OnInitDialog();
    afx_msg void OnDeltaposTimeSpin(NMHDR * pNMHDR, LRESULT * pResult);

	enum { IDD = IDD_INSERT_SILENCE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    const double MIN_NUM_VALUE;
    const double MAX_NUM_VALUE;
	int repetitions;
    CSpinButtonCtrl spinSilence;
	double silence;
	CSaDoc * pDocument;

	DECLARE_MESSAGE_MAP()
};
