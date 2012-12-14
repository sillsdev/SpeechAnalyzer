/////////////////////////////////////////////////////////////////////////////
// DlgAutoReferenceData.h:
// Interface of the CDlgAutoReferenceData
//
// Author: Kent Gorham
// Copyright 2012 JAARS Inc. SIL
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "sa_dlwnd.h"
#include "resource.h"
#include "CSaString.h"
#include "TranscriptionData.h"

class CDlgAutoReferenceData : public CDialog
{
	DECLARE_DYNAMIC(CDlgAutoReferenceData)

public:
	CDlgAutoReferenceData();
	virtual ~CDlgAutoReferenceData();

	enum { IDD = IDD_AUTO_REFERENCE_DATA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	afx_msg void OnRadio();
	afx_msg void OnClickedBrowseButton();
	afx_msg void OnDeltaposBeginSpin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposEndSpin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnKillfocusFilename();

private:
	CSpinButtonCtrl mSpinBegin;
	CSpinButtonCtrl mSpinEnd;
	CButton mButtonBrowse;
	CEdit mEditBegin;
	CEdit mEditEnd;
	CEdit mEditFilename;
	CComboBox mComboBegin;
	CComboBox mComboEnd;
	CButton mRadioFirstGloss;
	CButton mRadioSelectedGloss;
	CButton mRadioNumbers;
	CButton mRadioFile;

	bool mGlossSelected;
	int mSize;
	class CSaDoc * mSaDoc;

public:
	void Init( CSaDoc * pSaDoc, int begin, int end, bool glossSelected);

	// radio selection
	bool mUsingFirstGloss;
	bool mUsingNumbers;
	CString mLastImport;

	// for word list selections
	CString mBeginRef;
	CString mEndRef;

	// for numbering
	int mBegin;
	int mEnd;

};
