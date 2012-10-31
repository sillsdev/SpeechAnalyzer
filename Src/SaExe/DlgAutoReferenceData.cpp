/////////////////////////////////////////////////////////////////////////////
// DlgAutoReferenceData.cpp:
// Implementation of the CDlgAutoReferenceData
//
// Author: Kent Gorham
// Copyright 2012 JAARS Inc. SIL
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DlgAutoReferenceData.h"


// CDlgAutoReferenceData dialog

IMPLEMENT_DYNAMIC(CDlgAutoReferenceData, CDialog)

CDlgAutoReferenceData::CDlgAutoReferenceData( CWnd* pParent, int begin, int end)
	: CDialog(CDlgAutoReferenceData::IDD, pParent)
{
	mAutoReferenceDataBegin = begin;
	mAutoReferenceDataEnd = end;
	mAutoReferenceDataSize = end-begin+1;
}

CDlgAutoReferenceData::~CDlgAutoReferenceData()
{
}

BOOL CDlgAutoReferenceData::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_SpinDataBegin.Init(IDC_AUTO_REF_DATA_BEGIN_SPIN, this);
	m_SpinDataEnd.Init(IDC_AUTO_REF_DATA_END_SPIN, this);
	
	UpdateData(FALSE);
	return TRUE;
}


void CDlgAutoReferenceData::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_AUTO_REF_DATA_BEGIN_EDIT, mAutoReferenceDataBegin);
	DDV_MinMaxInt(pDX, mAutoReferenceDataBegin, 0, 10000);
	DDX_Text(pDX, IDC_AUTO_REF_DATA_END_EDIT, mAutoReferenceDataEnd);
	DDV_MinMaxInt(pDX, mAutoReferenceDataEnd, mAutoReferenceDataBegin, mAutoReferenceDataBegin+mAutoReferenceDataSize-1);
}

void CDlgAutoReferenceData::OnDataBeginScroll()
{
	int nData = GetDlgItemInt(IDC_AUTO_REF_DATA_BEGIN_EDIT, NULL, TRUE);
	if (m_SpinDataBegin.UpperButtonClicked()) nData++;
	else nData--;
	if (nData > 10000) nData = 10000;
	if (nData < 0) nData = 0;
	if (nData != mAutoReferenceDataBegin) mAutoReferenceDataBegin = nData;
	SetDlgItemInt(IDC_AUTO_REF_DATA_BEGIN_EDIT, mAutoReferenceDataBegin, TRUE);
}

void CDlgAutoReferenceData::OnDataEndScroll()
{
	int nData = GetDlgItemInt(IDC_AUTO_REF_DATA_END_EDIT, NULL, TRUE);
	if (m_SpinDataEnd.UpperButtonClicked()) nData++;
	else nData--;
	if (nData > 10000) nData = 10000;
	if (nData < 1) nData = 1;
	if (nData != mAutoReferenceDataEnd) mAutoReferenceDataEnd = nData;
	SetDlgItemInt(IDC_AUTO_REF_DATA_END_EDIT, mAutoReferenceDataEnd, TRUE);
}


BEGIN_MESSAGE_MAP(CDlgAutoReferenceData, CDialog)
	//{{AFX_MSG_MAP(CDlgAutoReferenceData)
	ON_COMMAND(IDC_AUTO_REF_DATA_BEGIN_SPIN, OnDataBeginScroll)
	ON_COMMAND(IDC_AUTO_REF_DATA_END_SPIN, OnDataEndScroll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


