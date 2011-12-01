/////////////////////////////////////////////////////////////////////////////
// advancedParameters.cpp:
// Implementation of the CDlgAdvancedParse
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
#include "dlgadvancedparse.h"
#include "mainfrm.h"
#include "sa_view.h"
#include "sa_doc.h"

IMPLEMENT_DYNAMIC(CDlgAdvancedParse, CDialog)

CDlgAdvancedParse::CDlgAdvancedParse( CWnd* pParent /*=NULL*/) : 
	CDialog(CDlgAdvancedParse::IDD, pParent)
{
	// Set parsing parameters to default values.
	CMainFrame * pMainWnd = (CMainFrame*)AfxGetMainWnd();
	ParseParm * pParseParm = pMainWnd->GetParseParm();
	m_nBreakWidth = (int)(1000.0 * pParseParm->fBreakWidth);
	m_nMaxThreshold = pParseParm->nMaxThreshold;
	m_nMinThreshold = pParseParm->nMinThreshold;
}

void CDlgAdvancedParse::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgAdvancedParse)
	DDX_Text(pDX, IDC_BREAKWIDTHEDIT, m_nBreakWidth);
	DDV_MinMaxInt(pDX, m_nBreakWidth, 1, 999);
	DDX_Text(pDX, IDC_PARSEMAXTHRESHOLDEDIT, m_nMaxThreshold);
	DDV_MinMaxInt(pDX, m_nMaxThreshold, 1, 100);
	DDX_Text(pDX, IDC_PARSEMINTHRESHOLDEDIT, m_nMinThreshold);
	DDV_MinMaxInt(pDX, m_nMinThreshold, 0, 99);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDOK, m_OKButton);
	DDX_Control(pDX, IDC_APPLY, m_ApplyButton);
}

BOOL CDlgAdvancedParse::OnInitDialog()
{
	CDialog::OnInitDialog();

	// build and place the break width spin control
	m_SpinBreak.Init(IDC_BREAKWIDTHSCROLL, this);
	// build and place the min threshold spin control
	m_SpinMinThreshold.Init(IDC_PARSEMINTHRESHOLDSCROLL, this);
	// build and place the max threshold spin control
	m_SpinMaxThreshold.Init(IDC_PARSEMAXTHRESHOLDSCROLL, this);

	// get parse parameters document member data
	// initialize member data
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));
	ParseParm* pParseParm = pMainFrame->GetParseParm();
	m_nBreakWidth = (int)(pParseParm->fBreakWidth * (float)1000 + 0.5);
	m_nMaxThreshold = pParseParm->nMaxThreshold;
	m_nMinThreshold = pParseParm->nMinThreshold;

	UpdateData(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDlgAdvancedParse::Apply()
{
	UpdateData(TRUE);

	// get parse parameters document member data
	// store new data
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));
	ParseParm* pParseParm = pMainFrame->GetParseParm();
	pParseParm->fBreakWidth = (float)m_nBreakWidth / (float)1000;
	pParseParm->nMaxThreshold = m_nMaxThreshold;
	pParseParm->nMinThreshold = m_nMinThreshold;

	CSaView* pView = pMainFrame->GetCurrSaView();
	CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();
	if (!pDoc->AdvancedParse()) 
	{
		Undo();
	}
}

void CDlgAdvancedParse::Undo() 
{

	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));
	CSaView* pView = pMainFrame->GetCurrSaView();
	pView->SendMessage(WM_COMMAND,ID_EDIT_UNDO,0);
	pView->RefreshGraphs();
}

/***************************************************************************/
// CDlgAdvancedParse::OnBreakWidthScroll Break width spin control hit
/***************************************************************************/
void CDlgAdvancedParse::OnBreakWidthScroll()
{
	int nData = GetDlgItemInt(IDC_BREAKWIDTHEDIT, NULL, TRUE);
	if (m_SpinBreak.UpperButtonClicked()) nData++;
	else nData--;
	if (nData > 999) nData = 999;
	if (nData < 1) nData = 1;
	if (nData != m_nBreakWidth) m_nBreakWidth = nData;
	SetDlgItemInt(IDC_BREAKWIDTHEDIT, m_nBreakWidth, TRUE);
}

/***************************************************************************/
// CDlgAdvancedParse::OnMaxThresholdScroll Max threshold spin cntrl hit
/***************************************************************************/
void CDlgAdvancedParse::OnMaxThresholdScroll()
{
	int nData = GetDlgItemInt(IDC_PARSEMAXTHRESHOLDEDIT, NULL, TRUE);
	if (m_SpinMaxThreshold.UpperButtonClicked()) nData++;
	else nData--;
	if (nData > 100) nData = 100;
	if (nData < 1) nData = 1;
	if (nData != m_nMaxThreshold)
	{
		m_nMaxThreshold = nData;
		if ((m_nMaxThreshold - 1) < m_nMinThreshold)
		{
			m_nMinThreshold = m_nMaxThreshold - 1;
			SetDlgItemInt(IDC_PARSEMINTHRESHOLDEDIT, m_nMinThreshold, TRUE);
		}
	}
	SetDlgItemInt(IDC_PARSEMAXTHRESHOLDEDIT, m_nMaxThreshold, TRUE);
}

/***************************************************************************/
// CDlgAdvancedParse::OnMinThresholdScroll Min threshold spin cntrl hit
/***************************************************************************/
void CDlgAdvancedParse::OnMinThresholdScroll()
{
	int nData = GetDlgItemInt(IDC_PARSEMINTHRESHOLDEDIT, NULL, TRUE);
	if (m_SpinMinThreshold.UpperButtonClicked()) nData++;
	else nData--;
	if (nData > 99) nData = 99;
	if (nData < 0) nData = 0;
	if (nData != m_nMinThreshold)
	{
		m_nMinThreshold = nData;
		if (m_nMaxThreshold < (m_nMinThreshold + 1))
		{
			m_nMaxThreshold = m_nMinThreshold + 1;
			SetDlgItemInt(IDC_PARSEMAXTHRESHOLDEDIT, m_nMaxThreshold, TRUE);
		}
	}
	SetDlgItemInt(IDC_PARSEMINTHRESHOLDEDIT, m_nMinThreshold, TRUE);
}

BEGIN_MESSAGE_MAP(CDlgAdvancedParse, CDialog)
	//{{AFX_MSG_MAP(CDlgAdvancedParse)
	ON_COMMAND(IDC_BREAKWIDTHSCROLL, OnBreakWidthScroll)
	ON_COMMAND(IDC_PARSEMAXTHRESHOLDSCROLL, OnMaxThresholdScroll)
	ON_COMMAND(IDC_PARSEMINTHRESHOLDSCROLL, OnMinThresholdScroll)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_APPLY, &CDlgAdvancedParse::OnBnClickedApply)
END_MESSAGE_MAP()

void CDlgAdvancedParse::OnOK()
{
	Apply();
	CDialog::OnOK();
}

void CDlgAdvancedParse::OnCancel()
{
	Undo();
	CDialog::OnCancel();
}

void CDlgAdvancedParse::OnBnClickedApply()
{
	Apply();
}
