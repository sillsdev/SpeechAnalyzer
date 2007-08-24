/////////////////////////////////////////////////////////////////////////////
// advancedParameters.cpp:
// Implementation of the CDlgAdvancedParsePage (property page)
//                       CDlgAdvancedSegmentPage (property page)
//                       CDlgAdvanced (property sheet)
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
#include "advancedParameters.h"
#include "mainfrm.h"

//###########################################################################
// CDlgAdvancedParsePage property page
// Displays all controls to change advanced parsing parameters.

/////////////////////////////////////////////////////////////////////////////
// CDlgAdvancedParsePage message map

BEGIN_MESSAGE_MAP(CDlgAdvancedParsePage, CPropertyPage)
    //{{AFX_MSG_MAP(CDlgAdvancedParsePage)
    ON_COMMAND(IDC_BREAKWIDTHSCROLL, OnBreakWidthScroll)
    ON_EN_KILLFOCUS(IDC_BREAKWIDTHEDIT, OnKillfocusBreakWidth)
    ON_COMMAND(IDC_PARSEMAXTHRESHOLDSCROLL, OnMaxThresholdScroll)
    ON_EN_KILLFOCUS(IDC_PARSEMAXTHRESHOLDEDIT, OnKillfocusMaxThreshold)
    ON_COMMAND(IDC_PARSEMINTHRESHOLDSCROLL, OnMinThresholdScroll)
    ON_EN_KILLFOCUS(IDC_PARSEMINTHRESHOLDEDIT, OnKillfocusMinThreshold)
    ON_EN_CHANGE(IDC_BREAKWIDTHEDIT, OnChange)
    ON_EN_CHANGE(IDC_PARSEMAXTHRESHOLDEDIT, OnChange)
    ON_EN_CHANGE(IDC_PARSEMINTHRESHOLDEDIT, OnChange)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgAdvancedParsePage construction/destruction/creation

/***************************************************************************/
// CDlgAdvancedParsePage::CDlgAdvancedParsePage Constructor
/***************************************************************************/
CDlgAdvancedParsePage::CDlgAdvancedParsePage() : CPropertyPage(CDlgAdvancedParsePage::IDD)
{
  //{{AFX_DATA_INIT(CDlgAdvancedParsePage)
  //}}AFX_DATA_INIT

  // Set parsing parameters to default values.
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
  ParseParm* pParseParm     = pMainWnd->GetParseParm();
  m_nBreakWidth   = (int) (1000.0 * pParseParm->fBreakWidth);
  m_nMaxThreshold = pParseParm->nMaxThreshold;
  m_nMinThreshold = pParseParm->nMinThreshold;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgAdvancedParsePage helper functions

/***************************************************************************/
// CDlgAdvancedParsePage::DoDataExchange Data exchange
/***************************************************************************/
void CDlgAdvancedParsePage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CDlgAdvancedParsePage)
  DDX_Text(pDX, IDC_BREAKWIDTHEDIT, m_nBreakWidth);
  DDV_MinMaxInt(pDX, m_nBreakWidth, 1, 999);
  DDX_Text(pDX, IDC_PARSEMAXTHRESHOLDEDIT, m_nMaxThreshold);
  DDV_MinMaxInt(pDX, m_nMaxThreshold, 1, 100);
  DDX_Text(pDX, IDC_PARSEMINTHRESHOLDEDIT, m_nMinThreshold);
  DDV_MinMaxInt(pDX, m_nMinThreshold, 0, 99);
  //}}AFX_DATA_MAP
}

/***************************************************************************/
// CDlgAdvancedParsePage::Apply Apply button hit
/***************************************************************************/
void CDlgAdvancedParsePage::Apply()
{
  if (m_bModified) // retrieve data
  {
    CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
    ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));

    // get parse parameters document member data
    ParseParm* pParseParm = pMainFrame->GetParseParm();
    // store new data
    pParseParm->fBreakWidth = (float)m_nBreakWidth / (float)1000;
    pParseParm->nMaxThreshold = m_nMaxThreshold;
    pParseParm->nMinThreshold = m_nMinThreshold;
    m_bModified = FALSE;
    SetModified(FALSE); // RLJ 1.5Test10.9B
  }
}

/////////////////////////////////////////////////////////////////////////////
// CDlgAdvancedParsePage message handlers

/***************************************************************************/
// CDlgAdvancedParsePage::OnInitDialog Dialog initialization
/***************************************************************************/
BOOL CDlgAdvancedParsePage::OnInitDialog()
{
  CPropertyPage::OnInitDialog();
  // build and place the break width spin control
  m_SpinBreak.Init(IDC_BREAKWIDTHSCROLL, this);
  // build and place the min threshold spin control
  m_SpinMinThreshold.Init(IDC_PARSEMINTHRESHOLDSCROLL, this);
  // build and place the max threshold spin control
  m_SpinMaxThreshold.Init(IDC_PARSEMAXTHRESHOLDSCROLL, this);

  CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
  ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));

  // get parse parameters document member data
  ParseParm* pParseParm = pMainFrame->GetParseParm();

  // initialize member data
  m_nBreakWidth = (int)(pParseParm->fBreakWidth * (float)1000 + 0.5);
  m_nMaxThreshold = pParseParm->nMaxThreshold;
  m_nMinThreshold = pParseParm->nMinThreshold;
  m_bModified = FALSE;
  UpdateData(FALSE);
  return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************/
// CDlgAdvancedParsePage::OnBreakWidthScroll Break width spin control hit
/***************************************************************************/
void CDlgAdvancedParsePage::OnBreakWidthScroll()
{
  int nData = GetDlgItemInt(IDC_BREAKWIDTHEDIT, NULL, TRUE);
  if (m_SpinBreak.UpperButtonClicked()) nData++;
  else nData--;
  if (nData > 999) nData = 999;
  if (nData < 1) nData = 1;
  if (nData != m_nBreakWidth)
  {
    m_nBreakWidth = nData;
    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
  }
  SetDlgItemInt(IDC_BREAKWIDTHEDIT, m_nBreakWidth, TRUE);
}

/***************************************************************************/
// CDlgAdvancedParsePage::OnChange page changed
/***************************************************************************/
void CDlgAdvancedParsePage::OnChange()
{
  m_bModified = TRUE;
  SetModified(TRUE); // data modified, enable apply button
}

/***************************************************************************/
// CDlgAdvancedParsePage::OnKillfocusBreakWidth Break width changed
/***************************************************************************/
void CDlgAdvancedParsePage::OnKillfocusBreakWidth()
{
  int nData = GetDlgItemInt(IDC_BREAKWIDTHEDIT, NULL, TRUE);
  if (nData > 999) nData = 999;
  if (nData < 1) nData = 1;
  if (nData != m_nBreakWidth)
  {
    m_nBreakWidth = nData;
    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
  }
  SetDlgItemInt(IDC_BREAKWIDTHEDIT, m_nBreakWidth, TRUE);
}

/***************************************************************************/
// CDlgAdvancedParsePage::OnMaxThresholdScroll Max threshold spin cntrl hit
/***************************************************************************/
void CDlgAdvancedParsePage::OnMaxThresholdScroll()
{
  int nData = GetDlgItemInt(IDC_PARSEMAXTHRESHOLDEDIT, NULL, TRUE);
  if (m_SpinMaxThreshold.UpperButtonClicked()) nData++;
  else nData--;
  if (nData > 100) nData = 100;
  if (nData < 1) nData = 1;
  if (nData != m_nMaxThreshold)
  {
    m_nMaxThreshold = nData;
    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
    if ((m_nMaxThreshold - 1) < m_nMinThreshold)
    {
      m_nMinThreshold = m_nMaxThreshold - 1;
      SetDlgItemInt(IDC_PARSEMINTHRESHOLDEDIT, m_nMinThreshold, TRUE);
    }
  }
  SetDlgItemInt(IDC_PARSEMAXTHRESHOLDEDIT, m_nMaxThreshold, TRUE);
}

/***************************************************************************/
// CDlgAdvancedParsePage::OnKillfocusMaxThreshold Max threshold changed
/***************************************************************************/
void CDlgAdvancedParsePage::OnKillfocusMaxThreshold()
{
  int nData = GetDlgItemInt(IDC_PARSEMAXTHRESHOLDEDIT, NULL, TRUE);
  if (nData > 100) nData = 100;
  if (nData < 1) nData = 1;
  if (nData != m_nMaxThreshold)
  {
    m_nMaxThreshold = nData;
    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
    if ((m_nMaxThreshold - 1) < m_nMinThreshold)
    {
      m_nMinThreshold = m_nMaxThreshold - 1;
      SetDlgItemInt(IDC_PARSEMINTHRESHOLDEDIT, m_nMinThreshold, TRUE);
    }
  }
  SetDlgItemInt(IDC_PARSEMAXTHRESHOLDEDIT, m_nMaxThreshold, TRUE);
}

/***************************************************************************/
// CDlgAdvancedParsePage::OnMinThresholdScroll Min threshold spin cntrl hit
/***************************************************************************/
void CDlgAdvancedParsePage::OnMinThresholdScroll()
{
  int nData = GetDlgItemInt(IDC_PARSEMINTHRESHOLDEDIT, NULL, TRUE);
  if (m_SpinMinThreshold.UpperButtonClicked()) nData++;
  else nData--;
  if (nData > 99) nData = 99;
  if (nData < 0) nData = 0;
  if (nData != m_nMinThreshold)
  {
    m_nMinThreshold = nData;
    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
    if (m_nMaxThreshold < (m_nMinThreshold + 1))
    {
      m_nMaxThreshold = m_nMinThreshold + 1;
      SetDlgItemInt(IDC_PARSEMAXTHRESHOLDEDIT, m_nMaxThreshold, TRUE);
    }
  }
  SetDlgItemInt(IDC_PARSEMINTHRESHOLDEDIT, m_nMinThreshold, TRUE);
}

/***************************************************************************/
// CDlgAdvancedParsePage::OnKillfocusMinThreshold Min threshold changed
/***************************************************************************/
void CDlgAdvancedParsePage::OnKillfocusMinThreshold()
{
  int nData = GetDlgItemInt(IDC_PARSEMINTHRESHOLDEDIT, NULL, TRUE);
  if (nData > 99) nData = 99;
  if (nData < 0) nData = 0;
  if (nData != m_nMinThreshold)
  {
    m_nMinThreshold = nData;
    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
    if (m_nMaxThreshold < (m_nMinThreshold + 1))
    {
      m_nMaxThreshold = m_nMinThreshold + 1;
      SetDlgItemInt(IDC_PARSEMAXTHRESHOLDEDIT, m_nMaxThreshold, TRUE);
    }
  }
  SetDlgItemInt(IDC_PARSEMINTHRESHOLDEDIT, m_nMinThreshold, TRUE);
}

// CLW 1.07a start large section pasted
//###########################################################################
// CDlgAdvancedSegmentPage property page
// Displays all controls to change advanced segment parameters.

/////////////////////////////////////////////////////////////////////////////
// CDlgAdvancedSegmentPage message map

BEGIN_MESSAGE_MAP(CDlgAdvancedSegmentPage, CPropertyPage)
    //{{AFX_MSG_MAP(CDlgAdvancedSegmentPage)
    ON_COMMAND(IDC_SEGMENTWIDTHSCROLL, OnSegmentWidthScroll)
    ON_EN_KILLFOCUS(IDC_SEGMENTWIDTHEDIT, OnKillfocusSegmentWidth)
    ON_COMMAND(IDC_CHANGEMINSCROLL, OnChangeMinScroll)
    ON_EN_KILLFOCUS(IDC_CHANGEMINEDIT, OnKillfocusChangeMin)
    ON_COMMAND(IDC_ZEROCROSSINGMINSCROLL, OnZeroCrossMinScroll)
    ON_EN_KILLFOCUS(IDC_ZEROCROSSINGMINEDIT, OnKillfocusZeroCrossMin)
    ON_EN_CHANGE(IDC_SEGMENTWIDTHEDIT, OnChange)
    ON_EN_CHANGE(IDC_CHANGEMINEDIT, OnChange)
    ON_EN_CHANGE(IDC_ZEROCROSSINGMINEDIT, OnChange)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgAdvancedSegmentPage construction/destruction/creation

/***************************************************************************/
// CDlgAdvancedSegmentPage::CDlgAdvancedSegmentPage Constructor
/***************************************************************************/
CDlgAdvancedSegmentPage::CDlgAdvancedSegmentPage() :
CPropertyPage(CDlgAdvancedSegmentPage::IDD)
{
  //{{AFX_DATA_INIT(CDlgAdvancedSegmentPage)
  m_nSegmentWidth = 20;  // CLW 9/25/98: changed 10 to 20 as per ACR
  m_nChMinThreshold = 17; // CLW 10/15/98: changed 10 to 17 as per ACR
  m_nZCMinThreshold = 50; // CLW 10/12/98
  //}}AFX_DATA_INIT
}

/////////////////////////////////////////////////////////////////////////////
// CDlgAdvancedSegmentPage helper functions

/***************************************************************************/
// CDlgAdvancedSegmentPage::DoDataExchange Data exchange
/***************************************************************************/
void CDlgAdvancedSegmentPage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CDlgAdvancedSegmentPage)
  DDX_Text(pDX, IDC_SEGMENTWIDTHEDIT, m_nSegmentWidth);
  DDV_MinMaxInt(pDX, m_nSegmentWidth, 1, 999);
  DDX_Text(pDX, IDC_CHANGEMINEDIT, m_nChMinThreshold);
  DDV_MinMaxInt(pDX, m_nChMinThreshold, 0, 99);
  DDX_Text(pDX, IDC_ZEROCROSSINGMINEDIT, m_nZCMinThreshold); // CLW 10/12/98
  DDV_MinMaxInt(pDX, m_nZCMinThreshold, 0, 99); // CLW 10/12/98
  //}}AFX_DATA_MAP
}

/***************************************************************************/
// CDlgAdvancedSegmentPage::Apply Apply button hit
/***************************************************************************/
void CDlgAdvancedSegmentPage::Apply()
{
  if (m_bModified) // retrieve data
  {
    CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
    ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));

    // get segment parameters data
    SegmentParm* pSegmentParm = pMainFrame->GetSegmentParm();
    // save new data
    pSegmentParm->fSegmentWidth = (float)m_nSegmentWidth / (float)1000;
    pSegmentParm->nChThreshold = m_nChMinThreshold;
    pSegmentParm->nZCThreshold = m_nZCMinThreshold; // CLW 10/12/98
    m_bModified = FALSE;
    SetModified(FALSE); // RLJ 1.5Test10.9B
  }
}

/////////////////////////////////////////////////////////////////////////////
// CDlgAdvancedSegmentPage message handlers

/***************************************************************************/
// CDlgAdvancedSegmentPage::OnInitDialog Dialog initialization
/***************************************************************************/
BOOL CDlgAdvancedSegmentPage::OnInitDialog()
{
  CPropertyPage::OnInitDialog();
  // build and place the peak width spin control
  // m_SpinPeak.Init(IDC_PEAKWIDTHSCROLL, this);
  m_SpinSegment.Init(IDC_SEGMENTWIDTHSCROLL, this); // CLW 10/12/98 replaces above
  // build and place the min threshold spin controls
  // m_SpinMinThreshold.Init(IDC_SEGMENTMINTHRESHOLDSCROLL, this);
  m_SpinChangeMin.Init(IDC_CHANGEMINSCROLL, this); // CLW 10/12/98 replaces above
  m_SpinZeroCrossingMin.Init(IDC_ZEROCROSSINGMINSCROLL, this); // CLW 10/12/98 new
  CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
  ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));

  // get segment parameters data
  SegmentParm* pSegmentParm = pMainFrame->GetSegmentParm();
  // initialize member data
  /***********************************************************************/
  // CLW 9/25/98: added '+ 0.5' as per ACR
  // Before: SegmentWidth always displayed as 1 less than number entered
  /***********************************************************************/
  // m_nSegmentWidth = (int)(pSegmentParm->fPeakWidth * (float)1000 + 0.5);
  m_nSegmentWidth = (int)(pSegmentParm->fSegmentWidth * (float)1000 + 0.5); // CLW 10/12/98 replaces above
  m_nChMinThreshold = pSegmentParm->nChThreshold;
  m_nZCMinThreshold = pSegmentParm->nZCThreshold; // CLW 10/12/98 new
  m_bModified = FALSE;
  UpdateData(FALSE);
  return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************/
// CDlgAdvancedSegmentPage::OnSegmentWidthScroll Segment width spin control hit
// CLW 10/12/98 was OnSegmentWidthScroll. Changed to reflect change in
// segment parameters page.
/***************************************************************************/
void CDlgAdvancedSegmentPage::OnSegmentWidthScroll()
{
  int nData = GetDlgItemInt(IDC_SEGMENTWIDTHEDIT, NULL, TRUE);
  if (m_SpinSegment.UpperButtonClicked()) nData++;
  else nData--;
  if (nData > 999) nData = 999;
  if (nData < 1) nData = 1;
  if (nData != m_nSegmentWidth) // CLW 10/12/98 was m_nPeakWidth
  {
    m_nSegmentWidth = nData; // CLW 10/12/98 was m_nPeakWidth
    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
  }
  SetDlgItemInt(IDC_SEGMENTWIDTHEDIT, m_nSegmentWidth, TRUE); // CLW 10/12/98 was m_nPeakWidth
}

/***************************************************************************/
// CDlgAdvancedSegmentPage::OnChange page changed
/***************************************************************************/
void CDlgAdvancedSegmentPage::OnChange()
{
  m_bModified = TRUE;
  SetModified(TRUE); // data modified, enable apply button
}

/***************************************************************************/
// CDlgAdvancedSegmentPage::OnKillfocusSegmentWidth Segment width changed
// CLW 10/12/98 was OnKillfocusSegmentWidth. Changed to reflect change in
// segment parameters page.
/***************************************************************************/
void CDlgAdvancedSegmentPage::OnKillfocusSegmentWidth()
{
  int nData = GetDlgItemInt(IDC_SEGMENTWIDTHEDIT, NULL, TRUE);
  if (nData > 999) nData = 999;
  if (nData < 1) nData = 1;
  if (nData != m_nSegmentWidth) // CLW 10/12/98 was m_nPeakWidth
  {
    m_nSegmentWidth = nData; // CLW 10/12/98 was m_nPeakWidth
    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
  }
  SetDlgItemInt(IDC_SEGMENTWIDTHEDIT, m_nSegmentWidth, TRUE); // CLW 10/12/98 was m_nPeakWidth
}

/***************************************************************************/
// CDlgAdvancedSegmentPage::OnChangeMinScroll Change Min spin cntrl hit
// CLW 10/12/98 was OnMinThresholdScroll. Changed to reflect change in
// segment parameters page.
/***************************************************************************/
void CDlgAdvancedSegmentPage::OnChangeMinScroll()
{
  // int nData = GetDlgItemInt(IDC_SEGMENTMINTHRESHOLDEDIT, NULL, TRUE);
  int nData = GetDlgItemInt(IDC_CHANGEMINEDIT, NULL, TRUE); // CLW 10/12/98 changed above
  if (m_SpinChangeMin.UpperButtonClicked()) nData++; // CLW 10/12/98 replaced m_SpinMinThreshold
  else nData--;
  if (nData > 99) nData = 99;
  if (nData < 0) nData = 0;
  // CLW 10/12/98 replaced m_nSegmentMinThreshold
  if (nData != m_nChMinThreshold)
  {
    m_nChMinThreshold = nData; // CLW 10/12/98 replaced m_nSegmentMinThreshold
    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
  }
  // CLW 10/12/98 replaced m_nSegmentMinThreshold
  // SetDlgItemInt(IDC_SEGMENTMINTHRESHOLDEDIT, m_nChMinThreshold, TRUE);
  SetDlgItemInt(IDC_CHANGEMINEDIT, m_nChMinThreshold, TRUE); // CLW 10/12/98 changed above
}

/***************************************************************************/
// CDlgAdvancedSegmentPage::OnKillfocusChangeMin Change Min changed
// CLW 10/12/98 was OnKillfocusMinThreshold. Changed to reflect change in
// segment parameters page.
/***************************************************************************/
void CDlgAdvancedSegmentPage::OnKillfocusChangeMin()
{
  // int nData = GetDlgItemInt(IDC_SEGMENTMINTHRESHOLDEDIT, NULL, TRUE);
  int nData = GetDlgItemInt(IDC_CHANGEMINEDIT, NULL, TRUE); // CLW 10/12/98 changed above
  if (nData > 99) nData = 99;
  if (nData < 0) nData = 0;
  if (nData != m_nChMinThreshold)
  {
    m_nChMinThreshold = nData;
    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
  }
  // SetDlgItemInt(IDC_SEGMENTMINTHRESHOLDEDIT, m_nChMinThreshold, TRUE);
  SetDlgItemInt(IDC_CHANGEMINEDIT, m_nChMinThreshold, TRUE); // CLW 10/12/98 changed above
}

/***************************************************************************/
// CDlgAdvancedSegmentPage::OnZeroCrossMinScroll Zero Crossing Min spin cntrl hit
// Added CLW 10/12/98
/***************************************************************************/
void CDlgAdvancedSegmentPage::OnZeroCrossMinScroll()
{
  int nData = GetDlgItemInt(IDC_ZEROCROSSINGMINEDIT, NULL, TRUE);
  if (m_SpinZeroCrossingMin.UpperButtonClicked()) nData++;
  else nData--;
  if (nData > 99) nData = 99;
  if (nData < 0) nData = 0;
  if (nData != m_nZCMinThreshold)
  {
    m_nZCMinThreshold = nData;
    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
  }
  SetDlgItemInt(IDC_ZEROCROSSINGMINEDIT, m_nZCMinThreshold, TRUE);
}

/***************************************************************************/
// CDlgAdvancedSegmentPage::OnKillfocusZeroCrossMin Zero Crossing Min changed
// Added CLW 10/12/98
/***************************************************************************/
void CDlgAdvancedSegmentPage::OnKillfocusZeroCrossMin()
{
  int nData = GetDlgItemInt(IDC_ZEROCROSSINGMINEDIT, NULL, TRUE);
  if (nData > 99) nData = 99;
  if (nData < 0) nData = 0;
  if (nData != m_nZCMinThreshold)
  {
    m_nZCMinThreshold = nData;
    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
  }
  SetDlgItemInt(IDC_ZEROCROSSINGMINEDIT, m_nZCMinThreshold, TRUE);
}

// CLW 1.07a end large section pasted

//###########################################################################
// CDlgAdvanced property sheet
// Displays all controls to change advanced parameters.

IMPLEMENT_DYNAMIC(CDlgAdvanced, CPropertySheet)

/////////////////////////////////////////////////////////////////////////////
// CDlgAdvanced message map

BEGIN_MESSAGE_MAP(CDlgAdvanced, CPropertySheet)
    //{{AFX_MSG_MAP(CDlgAdvanced)
    ON_WM_CREATE()
    ON_COMMAND(ID_APPLY_NOW, OnApply)
    ON_COMMAND(IDOK, OnOK)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgAdvanced construction/destruction/creation

/***************************************************************************/
// CDlgAdvanced::CDlgAdvanced Constructor
/***************************************************************************/
CDlgAdvanced::CDlgAdvanced(LPCTSTR pszCaption, CWnd* pParent, UINT iSelectPage)
: CPropertySheet(pszCaption, pParent, iSelectPage)
{
  //{{AFX_DATA_INIT(CDlgAdvanced)
  // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
  AddPage(&m_dlgSegmentPage);
  AddPage(&m_dlgParsePage);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgAdvanced helper functions

/***************************************************************************/
// CDlgAdvanced::DoDataExchange Data exchange
/***************************************************************************/
void CDlgAdvanced::DoDataExchange(CDataExchange* pDX)
{
  CPropertySheet::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CDlgAdvanced)
  // NOTE: the ClassWizard will add DDX and DDV calls here
  //}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CDlgAdvanced message handlers

/***************************************************************************/
// CDlgAdvanced::OnCreate Dialog creation
/***************************************************************************/
int CDlgAdvanced::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CPropertySheet::OnCreate(lpCreateStruct) == -1)
    return -1;

  // TODO: Add your specialized creation code here

  return 0;
}

/***************************************************************************/
// CDlgAdvanced::OnApply Apply button hit
/***************************************************************************/
void CDlgAdvanced::OnApply()
{
  m_dlgSegmentPage.Apply();
  m_dlgParsePage.Apply();
  SendMessage(PSM_CANCELTOCLOSE, 0, 0L);
}

/***************************************************************************/
// CDlgAdvanced::OnOK OK button hit
/***************************************************************************/
void CDlgAdvanced::OnOK()
{
  OnApply();
  EndDialog(IDOK);
}


