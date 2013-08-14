#pragma once

#include "resource.h"
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
    CSpinButtonCtrl mSpinSilence;
	double mSilence;
    const double MIN_NUM_VALUE;
    const double MAX_NUM_VALUE;
	CSaDoc * mpDoc;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
