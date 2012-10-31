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

// CDlgAutoReferenceData dialog

class CDlgAutoReferenceData : public CDialog
{
	DECLARE_DYNAMIC(CDlgAutoReferenceData)

public:
	CDlgAutoReferenceData(CWnd* pParent, int begin, int end);   // standard constructor
	virtual ~CDlgAutoReferenceData();

// Dialog Data
	enum { IDD = IDD_AUTO_REFERENCE_DATA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	//{{AFX_MSG(CDlgAutoReferenceData)
	virtual BOOL OnInitDialog();
	afx_msg void OnDataBeginScroll();
	afx_msg void OnDataEndScroll();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
public:
	int mAutoReferenceDataBegin;
	int mAutoReferenceDataEnd;

private:
	int mAutoReferenceDataSize;
	CSpinControl m_SpinDataBegin;
	CSpinControl m_SpinDataEnd;
};
