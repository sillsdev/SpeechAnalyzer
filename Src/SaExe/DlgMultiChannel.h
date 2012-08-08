/////////////////////////////////////////////////////////////////////////////
// DlgMultiChannel.h:
// Implementation of the CDlgMultiChannel
//
// Author: Kent Gorham
// copyright 2012 JAARS Inc. SIL
//
// Revision History
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "resource.h"
#include "afxwin.h"

class CSaDoc;

class CDlgMultiChannel : public CDialog
{
	DECLARE_DYNAMIC( CDlgMultiChannel)
public:
	CDlgMultiChannel( int nChannels, bool allowCombine);
	~CDlgMultiChannel();

	//{{AFX_DATA(CDlgMultiChannel)
	enum { IDD = IDD_MULTICHANNEL };
	//}}AFX_DATA

protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
	//{{AFX_MSG(CDlgMultiChannel)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	CComboBox m_MultiChannelCombo;
	int m_nChannels;
	bool m_bAllowCombine;

public:
	int m_nChannel;
};
