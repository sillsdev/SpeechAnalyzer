/////////////////////////////////////////////////////////////////////////////
// DlgMultiChannel.cpp:
// Implementation of the CDlgMultiChannel
//
// Author: Kent Gorham
// copyright 2012 JAARS Inc. SIL
//
// Revision History
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "DlgMultiChannel.h"

IMPLEMENT_DYNAMIC(CDlgMultiChannel, CDialog)

CDlgMultiChannel::CDlgMultiChannel( int nChannel, bool allowCombine) :
	CDialog(CDlgMultiChannel::IDD,NULL), 
	m_nChannels(nChannel),
	m_bAllowCombine(allowCombine),
	m_nChannel(0) 
	{
	//{{AFX_DATA_INIT(CDlgMultiChannel)
	//}}AFX_DATA_INIT
}

CDlgMultiChannel::~CDlgMultiChannel(void)
{
}

void CDlgMultiChannel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgMultiChannel)
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_MULTICHANNEL_COMBO, m_MultiChannelCombo);
	DDX_CBIndex(pDX, IDC_MULTICHANNEL_COMBO, m_nChannel);
}

BEGIN_MESSAGE_MAP(CDlgMultiChannel, CDialog)
	//{{AFX_MSG_MAP(CDlgMultiChannel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CDlgMultiChannel::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString szText;
	CString szNum;
	if (m_nChannels>=1)
	{
		szNum.Format(L"%d",1);
		AfxFormatString1(szText,IDS_MC_CHANNEL_LEFT, szNum);
		m_MultiChannelCombo.AddString(szText);
		if (m_nChannels>=2) 
		{
			szNum.Format(L"%d",2);
			AfxFormatString1(szText,IDS_MC_CHANNEL_RIGHT, szNum);
			m_MultiChannelCombo.AddString(szText);
			if (m_nChannels>=3) 
			{
				for (int i=2;i<m_nChannels;i++)
				{
					szNum.Format(L"%d",i+1);
					AfxFormatString1(szText,IDS_MC_CHANNEL, szNum);
					m_MultiChannelCombo.AddString(szText);
				}
			}
		}
	}
	if (m_bAllowCombine)
	{
		szText.LoadString(IDS_MC_COMBINE);
		m_MultiChannelCombo.AddString(szText);
	}

	m_MultiChannelCombo.SetCurSel(0);

	UpdateData(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}
