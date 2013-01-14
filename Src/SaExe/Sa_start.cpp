/////////////////////////////////////////////////////////////////////////////
// sa_start.cpp:
// Implementation of the CStartModeDlg (dialog) class.
//
// Author: Russ Johnson
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//   05/16/2000 - Original
//       RLJ Created so that CStartModeDlg could be moved here, to reduce
//           Sa_dlg.OBJ size enough to allow LINK/DEBUG
///  05/31/2000
//       RLJ Add SetOpenMode() call;
//   06/17/2000
//       RLJ Extend FileOpenAs to support not only Phonetic/Music Analysis,
//             but also OpenScreenF, OpenScreenG, OpenScreenI, OpenScreenK,
//             OpenScreenM, etc.
//   08/08/2000
//       DDO Completely reworked this dialog box. Added radio buttons,
//           MRU list, don't show check box, etc.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_start.h"
#include "chrchart\ch_dlg.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "dsp\hpfltr70.h"
#include "sa.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CStartModeDlg dialog
//
// RLJ - April 14, 2000

BEGIN_MESSAGE_MAP(CStartModeDlg, CDialog)
  //{{AFX_MSG_MAP(CStartModeDlg)
  ON_BN_CLICKED(ID_START_MODE_RECORD, OnStartModeRecord)
  ON_BN_CLICKED(ID_CLOSE, OnCloseButton)
  ON_LBN_DBLCLK(IDC_RECENTLIST, OnDblclkRecentlist)
  ON_BN_CLICKED(ID_STARTMODE_OK, OnOk)
	ON_COMMAND(IDC_PLAY, OnPlay)
	ON_LBN_SELCHANGE(IDC_RECENTLIST, OnSelchangeRecentlist)
	ON_BN_CLICKED(IDC_STOP, OnStop)
	ON_COMMAND(IDHELP, OnHelpStartMode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//CStartModeDlg::CStartModeDlg(CWnd* pParent /*=NULL*/)
CStartModeDlg::CStartModeDlg(CWnd* pParent) : CDialog(CStartModeDlg::IDD, pParent)
{
  //{{AFX_DATA_INIT(CStartModeDlg)
  m_nDontShowAgain = FALSE;
  m_bShowDontShowAgainOption = TRUE;
  m_nDataMode = -1;
  //}}AFX_DATA_INIT
}

void CStartModeDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CStartModeDlg)
  DDX_Control(pDX, IDC_RECENTLIST, m_lbRecentFiles);
  DDX_Check(pDX, IDC_STARTMODE_DONTSHOW, m_nDontShowAgain);
  DDX_Radio(pDX, IDC_STARTMODE_TEMPLATE, m_nDataMode);
  //}}AFX_DATA_MAP
}


/////////////////////////////////////////////////////////////////////////////
// CStartModeDlg message handlers

//****************************************************************
//****************************************************************
BOOL CStartModeDlg::OnInitDialog()
{
  CDialog::OnInitDialog();
  // center dialog on framewindow
  CenterWindow();

  //**************************************************
  // Add the more item to the MRU list and initialize
  // the list box so the first item is current.
  //**************************************************
  m_lbRecentFiles.AddString(_T("More Files..."));
  m_lbRecentFiles.SetCurSel(0);

  //**************************************************
  // Fill the MRU list.
  //**************************************************
  CSaApp* pApp = (CSaApp*)AfxGetApp();

  CSaString workDir;
  int     dx=0;
  CDC*    pDC = m_lbRecentFiles.GetDC();
  int nOldMapMode = pDC->SetMapMode(MM_TEXT);
  CFont *pOldFont = pDC->SelectObject(m_lbRecentFiles.GetFont());
  TEXTMETRIC tm;
  pDC->GetTextMetrics(&tm);
  // get coordinates of static text control
  CRect rWnd;
  m_lbRecentFiles.GetClientRect(rWnd);
  // calculate number of characters possible to display and limit the string
  int nChars = (rWnd.right / tm.tmAveCharWidth * 8 / 10); // experience values
  for (int i = 0; i < _AFX_MRU_MAX_COUNT; i++)
  {
    CFileStatus status;

    pApp->GetMRUFilePath(i,workDir);
    if ((workDir.GetLength() > 0)  && CFile::GetStatus(workDir, status))
    {
			if (workDir.GetLength() > nChars) // file path is too long
			{
				CSaString szRightPath = workDir.Right(nChars - 6);
				szRightPath = szRightPath.Right(szRightPath.GetLength() - szRightPath.Find(_T("\\")));
				workDir = workDir.Left(3) + "..." + szRightPath; // drive...rest
			}
      m_lbRecentFiles.AddString(workDir);
      CSize sz = pDC->GetTextExtent(workDir);

      if (sz.cx > dx)
        dx = sz.cx;
    }

  }
  pDC->SelectObject(pOldFont);
  pDC->SetMapMode(nOldMapMode);
  m_lbRecentFiles.ReleaseDC(pDC);

  // Set the horizontal extent so every character of all strings 
  // can be scrolled to.
  m_lbRecentFiles.SetHorizontalExtent(dx + 10);

  //**************************************************
  // Remove the ugly default Win 3.1 bold attribute
  // on several of the static controls.
  //**************************************************
  LOGFONT logFont;
  CFont* pFont = GetDlgItem(IDC_STATIC1)->GetFont();  // get the standard font
  pFont->GetObject(sizeof(LOGFONT), (void*)&logFont); // fill up logFont
  logFont.lfWeight = FW_NORMAL;                       // make font not bold
  m_Font.CreateFontIndirect(&logFont);                // create the modified font

  GetDlgItem(IDC_STATIC1)->SetFont(&m_Font);
  GetDlgItem(IDC_STATIC2)->SetFont(&m_Font);
  GetDlgItem(IDC_STATIC3)->SetFont(&m_Font);
  GetDlgItem(IDC_STATIC4)->SetFont(&m_Font);

  if (!m_bShowDontShowAgainOption)
    GetDlgItem(IDC_STARTMODE_DONTSHOW)->ShowWindow(SW_HIDE);

  //**************************************************
  // Get whether or not the user has specified a
  // permanent default view template.
  //**************************************************
  BOOL bEnableUserSpec = ((CMainFrame*)AfxGetMainWnd())->DefaultIsValid();

  //**************************************************
  // If they have not and the current data mode is
  // User Specified, then change the data mode to
  // Phonetic.
  //**************************************************
  if (m_nDataMode == 0 && !bEnableUserSpec)
  {
    m_nDataMode = 1;
    UpdateData(FALSE);
  }

  //**************************************************
  // Determine whether or not the User Specified
  // radio button should be enabled. Don't enable it
  // if the user hasn't previously specified a
  // permanent default layout and graph list (i.e.
  // under Tools/Start Options).
  //**************************************************
  GetDlgItem(IDC_STARTMODE_TEMPLATE)->EnableWindow(bEnableUserSpec);
  GetDlgItem(IDC_STATIC1)->EnableWindow(bEnableUserSpec);
  GetDlgItem(IDC_PLAY)->EnableWindow(FALSE);

  return TRUE;  // return TRUE  unless you set the focus to a control
}

//****************************************************************
//****************************************************************
void CStartModeDlg::OnStartModeRecord()
{
  if (!Cleanup()) return;
  // create new file and launch recorder
  (CMainFrame*)AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_FILE_RECORD, 0L);
  CDialog::OnOK();
}

//****************************************************************
//****************************************************************
void CStartModeDlg::OnCloseButton()
{
  if (!Cleanup()) return;
  CDialog::OnOK();
}

//****************************************************************
//****************************************************************
bool CStartModeDlg::Cleanup()
{
  //**************************************************
  // Enable this radio button first or UpdateData
  // will bomb.
  //**************************************************
  //kg why is this needed?
  //  GetDlgItem(IDC_STARTMODE_TEMPLATE)->EnableWindow(TRUE);

  if (!UpdateData(TRUE)) return false;
  ((CMainFrame*)AfxGetMainWnd())->SetShowStartupDlg(!m_nDontShowAgain);
  ((CMainFrame*)AfxGetMainWnd())->SetStartDataMode(m_nDataMode);
  return true;
}

//****************************************************************
// DDO - 08/08/00
//****************************************************************
void CStartModeDlg::OnDblclkRecentlist()
{
  UINT nOpenID = ID_FILE_OPEN;
  int nIndex = m_lbRecentFiles.GetCurSel();
  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();

  if (!Cleanup()) return;

  switch(m_nDataMode)
  {
    case 0: nOpenID = ID_FILE_OPEN;                    break;
    case 1: nOpenID = ID_FILE_OPENAS_PHONETICANALYSIS; break;
    case 2: nOpenID = ID_FILE_OPENAS_MUSICANALYSIS;    break;
  }

  if (nIndex == 0)
  {
    pMainWnd->PostMessage(WM_COMMAND, nOpenID, 0L);
  }
  else
  {
    CSaApp* pApp = ((CSaApp *)AfxGetApp());
      
    pApp->SetOpenAsID(nOpenID);

		CSaString szFile;
		CFileStatus status;
		for (int i = 0; i < nIndex; i++)
		{
      pApp->GetMRUFilePath(i,szFile);
			if (!CFile::GetStatus(szFile, status))
				nIndex++;
		}

		pMainWnd->PostMessage(WM_COMMAND, ID_FILE_MRU_FIRST + nIndex - 1, 0L);
  }

  CDialog::OnOK();
}

//****************************************************************
// DDO - 08/08/2000
//****************************************************************
void CStartModeDlg::OnOk()
{
  OnDblclkRecentlist();
}

//****************************************************************
// OnPlay  Play selected file
//****************************************************************
void CStartModeDlg::OnPlay() 
{
  int nIndex = m_lbRecentFiles.GetCurSel();

  if (nIndex > 0)
  {
		CSaApp* pApp = (CSaApp*)AfxGetApp();
    CSaString file;
 
		pApp->GetMRUFilePath(nIndex - 1,file);

    PlaySound(LPCTSTR(file), 0, SND_ASYNC | SND_NODEFAULT | SND_FILENAME );
  }
}

//****************************************************************
// OnStop  Stop playback file
//****************************************************************
void CStartModeDlg::OnStop() 
{
  PlaySound(NULL, 0, SND_PURGE | SND_NODEFAULT );	
}

//****************************************************************
// OnSelchangeRecentlist  Update play selected file button
//****************************************************************
void CStartModeDlg::OnSelchangeRecentlist() 
{
  int nIndex = m_lbRecentFiles.GetCurSel();

  OnStop();

  CWnd *pWnd = GetDlgItem(IDC_PLAY);
  if (!pWnd)
    return;

	CSaApp* pApp = (CSaApp*)AfxGetApp();
  CSaString file;
	if (nIndex > 0)
		pApp->GetMRUFilePath(nIndex - 1,file);

  CFileStatus status;

  if ((nIndex > 0) && CFile::GetStatus(LPCTSTR(file), status))
    pWnd->EnableWindow();
  else
    pWnd->EnableWindow(FALSE);

}

/***************************************************************************/
// CStartModeDlg::OnHelpStartMode Call Start Mode help
/***************************************************************************/
void CStartModeDlg::OnHelpStartMode()
{
	// create the pathname
	CString szPath = AfxGetApp()->m_pszHelpFilePath;
	szPath += "::/User_Interface/Start_Mode.htm";
	::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}
