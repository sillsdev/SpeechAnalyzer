/////////////////////////////////////////////////////////////////////////////
// sa_wbch.cpp:
// Implementation of the CSaWorkbenchView class.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Process\sa_proc.h"
#include "sa_wbch.h"
#include "sa_w_dlg.h"
#include "mainfrm.h"
#include "Process\sa_w_pas.h"
#include "Process\sa_w_rev.h"
#include "Process\sa_w_equ.h"
#include "sa_w_doc.h"
#include "sa.h"
#include "settings\obstream.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CSaWorkbenchView

IMPLEMENT_DYNCREATE(CSaWorkbenchView, CFormView)

/////////////////////////////////////////////////////////////////////////////
// CSaWorkbenchView message map

BEGIN_MESSAGE_MAP(CSaWorkbenchView, CFormView)
    //{{AFX_MSG_MAP(CSaWorkbenchView)
    ON_WM_CTLCOLOR()
    ON_WM_DESTROY()
    ON_BN_DOUBLECLICKED(IDC_RAW0, OnProcess0)
    ON_BN_DOUBLECLICKED(IDC_RAW1, OnProcess1)
    ON_BN_DOUBLECLICKED(IDC_RAW2, OnProcess2)
    ON_BN_DOUBLECLICKED(IDC_RAW3, OnProcess3)
    ON_BN_DOUBLECLICKED(IDC_RESULT0, OnProcess0)
    ON_BN_DOUBLECLICKED(IDC_RESULT1, OnProcess1)
    ON_BN_DOUBLECLICKED(IDC_RESULT2, OnProcess2)
    ON_BN_DOUBLECLICKED(IDC_RESULT3, OnProcess3)
    ON_BN_DOUBLECLICKED(IDC_FILTER10, OnFilter10)
    ON_BN_DOUBLECLICKED(IDC_FILTER11, OnFilter11)
    ON_BN_DOUBLECLICKED(IDC_FILTER12, OnFilter12)
    ON_BN_DOUBLECLICKED(IDC_FILTER13, OnFilter13)
    ON_BN_DOUBLECLICKED(IDC_FILTER20, OnFilter20)
    ON_BN_DOUBLECLICKED(IDC_FILTER21, OnFilter21)
    ON_BN_DOUBLECLICKED(IDC_FILTER22, OnFilter22)
    ON_BN_DOUBLECLICKED(IDC_FILTER23, OnFilter23)
    ON_BN_DOUBLECLICKED(IDC_FILTER30, OnFilter30)
    ON_BN_DOUBLECLICKED(IDC_FILTER31, OnFilter31)
    ON_BN_DOUBLECLICKED(IDC_FILTER32, OnFilter32)
    ON_BN_DOUBLECLICKED(IDC_FILTER33, OnFilter33)
    ON_COMMAND(ID_PROCESS0_PROPERTIES, OnProcess0)
    ON_COMMAND(ID_PROCESS1_PROPERTIES, OnProcess1)
    ON_COMMAND(ID_PROCESS2_PROPERTIES, OnProcess2)
    ON_COMMAND(ID_PROCESS3_PROPERTIES, OnProcess3)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSaWorkbenchView construction/destruction/creation

/***************************************************************************/
// CSaWorkbenchView::CSaWorkbenchView Constructor
/***************************************************************************/
CSaWorkbenchView::CSaWorkbenchView() : CFormView(CSaWorkbenchView::IDD)
{
  //{{AFX_DATA_INIT(CSaWorkbenchView)
  // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
  m_hWndFocus = NULL;
  m_brBkg.CreateSolidBrush(GetSysColor(COLOR_BTNFACE)); // create the background brush
}

/////////////////////////////////////////////////////////////////////////////
// CSaWorkbenchView helper functions

/***************************************************************************/
// CSaWorkbenchView::DoDataExchange Data exchange
/***************************************************************************/
void CSaWorkbenchView::DoDataExchange(CDataExchange* pDX)
{
  CFormView::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CSaWorkbenchView)
  //}}AFX_DATA_MAP
}

/***************************************************************************/
// CSaWorkbenchView::CreateWbProcess Creates a workbench process
// The function returns NULL if the process could not create.
//***************************************************************************/
CDataProcess * CSaWorkbenchView::CreateWbProcess(int nFilterNumber)
{
  switch(nFilterNumber)
  {
    case 1: // highpass
      return new CProcessWbHighpass;
    case 2: // lowpass
      return new CProcessWbLowpass;
    case 3: // bandpass
      return new CProcessWbBandpass;
    case 4: // reverb
      return new CProcessWbReverb;
    case 5: // echo
      return new CProcessWbEcho;
    case 6: // equation
      return new CProcessWbEquation;
    case 7: // generator
      return NULL; // was new CProcessWbGenerator;
    default: return NULL;
  }
}

/***************************************************************************/
// CSaWorkbenchView::GetFilterResource Returns the filter resource (bitmap)
// The function returns the empty "IDB_FILTERU" resource on default.
//***************************************************************************/
LPCTSTR CSaWorkbenchView::GetFilterResource(int nFilterNumber)
{
  switch(nFilterNumber)
  {
    case 1: // highpass
      return _T("IDB_HPU");
    case 2: // lowpass
      return _T("IDB_LPU");
    case 3: // bandpass
      return _T("IDB_BPU");
    case 4: // reverb
      return _T("IDB_REVU");
    case 5: // echo
      return _T("IDB_ECHOU");
    case 6: // equation
      return _T("IDB_EQU");
    case 7: // Generator
      return _T("IDB_GENERATOR");
    default: return _T("IDB_PLAIN");
  }
}

/***************************************************************************/
// CSaWorkbenchView::SetupFilterProcesses Sets up the workench filter processes
// The function acsesses the data of a CWbDlgProcesses dialog and creates
// and sets up the choosen filters for the given process.
/***************************************************************************/
void CSaWorkbenchView::SetupFilterProcesses(CObject* pDialog)
{
  CWbDlgProcesses* pDlg = (CWbDlgProcesses*)pDialog; // cast pointer
  CMainFrame* pMain = (CMainFrame*)AfxGetMainWnd();
  // make local copy of existing processes and filters and clear originals
  CDataProcess* apWbLocalProcess[MAX_FILTER_NUMBER];
  int anLocalID[MAX_FILTER_NUMBER];
  for (int nLoop = 0; nLoop < MAX_FILTER_NUMBER; nLoop++)
  {
    apWbLocalProcess[nLoop] = pMain->SetWbProcess(pDlg->m_nProcess, nLoop, NULL);
    anLocalID[nLoop] = pMain->SetWbFilterID(pDlg->m_nProcess, nLoop, 0);
  }
  // first delete all the processes not longer needed
  for (int nLoop = 0; nLoop < MAX_FILTER_NUMBER; nLoop++)
  {
    int nOldFilterID = anLocalID[nLoop];
    if ((nOldFilterID != pDlg->m_nFilter1) && (nOldFilterID != pDlg->m_nFilter2) && (nOldFilterID != pDlg->m_nFilter3))
    {
      // filter no longer needed, delete it
      if (apWbLocalProcess[nLoop]) delete apWbLocalProcess[nLoop];
      apWbLocalProcess[nLoop] = NULL;
      anLocalID[nLoop] = 0;
    }
  }
  // now copy or create the new filter IDs and processes
  for (int nLoop = 0; nLoop < MAX_FILTER_NUMBER; nLoop++)
  {
    if (pDlg->m_nFilter3)
    {
      // filter 3 is not plain, get it
      for (int nFilterLoop = 0; nFilterLoop < MAX_FILTER_NUMBER; nFilterLoop++)
      {
        if (pDlg->m_nFilter3 == anLocalID[nFilterLoop])
        {
          // filter already exists, copy it
          pMain->SetWbFilterID(pDlg->m_nProcess, nLoop, anLocalID[nFilterLoop]);
          pMain->SetWbProcess(pDlg->m_nProcess, nLoop, apWbLocalProcess[nFilterLoop]);
          anLocalID[nFilterLoop] = 0;
          apWbLocalProcess[nFilterLoop] = NULL;
          pDlg->m_nFilter3 = 0;
          break;
        }
      }
      if (pDlg->m_nFilter3)
      {
        if (pDlg->m_pWbProcessFilter3)
        {
          // process exists, copy it
          pMain->SetWbProcess(pDlg->m_nProcess, nLoop, pDlg->m_pWbProcessFilter3);
          pDlg->m_pWbProcessFilter3 = NULL;
        }
        else
        {
          // filter has to be created
          CDataProcess* pProcess = CreateWbProcess(pDlg->m_nFilter3);
          if (pProcess)
          {
            pMain->SetWbProcess(pDlg->m_nProcess, nLoop, pProcess);
          }
          else pDlg->m_nFilter3 = 0;
        }
        pMain->SetWbFilterID(pDlg->m_nProcess, nLoop, pDlg->m_nFilter3);
        pDlg->m_nFilter3 = 0;
      }
    }
    else
    {
      if (pDlg->m_nFilter2)
      {
        // filter 2 is not plain, get it
        for (int nFilterLoop = 0; nFilterLoop < MAX_FILTER_NUMBER; nFilterLoop++)
        {
          if (pDlg->m_nFilter2 == anLocalID[nFilterLoop])
          {
            // filter already exists, copy it
            pMain->SetWbFilterID(pDlg->m_nProcess, nLoop, anLocalID[nFilterLoop]);
            pMain->SetWbProcess(pDlg->m_nProcess, nLoop, apWbLocalProcess[nFilterLoop]);
            anLocalID[nFilterLoop] = 0;
            apWbLocalProcess[nFilterLoop] = NULL;
            pDlg->m_nFilter2 = 0;
            break;
          }
        }
        if (pDlg->m_nFilter2)
        {
          if (pDlg->m_pWbProcessFilter2)
          {
            // process exists, copy it
            pMain->SetWbProcess(pDlg->m_nProcess, nLoop, pDlg->m_pWbProcessFilter2);
            pDlg->m_pWbProcessFilter2 = NULL;
          }
          else
          {
            // filter has to be created
            CDataProcess* pProcess = CreateWbProcess(pDlg->m_nFilter2);
            if (pProcess)
            {
              pMain->SetWbProcess(pDlg->m_nProcess, nLoop, pProcess);
            }
            else pDlg->m_nFilter2 = 0;
          }
          pMain->SetWbFilterID(pDlg->m_nProcess, nLoop, pDlg->m_nFilter2);
          pDlg->m_nFilter2 = 0;
        }
      }
      else
      {
        if (pDlg->m_nFilter1)
        {
          // filter 1 is not plain, get it
          for (int nFilterLoop = 0; nFilterLoop < MAX_FILTER_NUMBER; nFilterLoop++)
          {
            if (pDlg->m_nFilter1 == anLocalID[nFilterLoop])
            {
              // filter already exists, copy it
              pMain->SetWbFilterID(pDlg->m_nProcess, nLoop, anLocalID[nFilterLoop]);
              pMain->SetWbProcess(pDlg->m_nProcess, nLoop, apWbLocalProcess[nFilterLoop]);
              anLocalID[nFilterLoop] = 0;
              apWbLocalProcess[nFilterLoop] = NULL;
              pDlg->m_nFilter1 = 0;
              break;
            }
          }
          if (pDlg->m_nFilter1)
          {
            if (pDlg->m_pWbProcessFilter1)
            {
              // process exists, copy it
              pMain->SetWbProcess(pDlg->m_nProcess, nLoop, pDlg->m_pWbProcessFilter1);
              pDlg->m_pWbProcessFilter1 = NULL;
            }
            else
            {
              // filter has to be created
              CDataProcess* pProcess = CreateWbProcess(pDlg->m_nFilter1);
              if (pProcess)
              {
                pMain->SetWbProcess(pDlg->m_nProcess, nLoop, pProcess);
              }
              else pDlg->m_nFilter1 = 0;
            }
            pMain->SetWbFilterID(pDlg->m_nProcess, nLoop, pDlg->m_nFilter1);
            pDlg->m_nFilter1 = 0;
          }
        }
      }
    }
    }
    GetDocument()->SetModifiedFlag(); // documents data has changed
}

/***************************************************************************/
// CSaWorkbenchView::CallPropertiesDialog Calls the properties dialog of a process
/***************************************************************************/
void CSaWorkbenchView::CallPropertiesDialog(int nProcess, int nFilter)
{
  CMainFrame* pMain = (CMainFrame*)AfxGetMainWnd();
  // get the process
  if (!nFilter)
  {
    if (pMain->GetWbFilterID(nProcess, nFilter + 1)) nFilter++;
    if (pMain->GetWbFilterID(nProcess, nFilter + 1)) nFilter++;
  }
  else
  {
    if (nFilter == 1)
    {
      if (!pMain->GetWbFilterID(nProcess, nFilter + 1)) nFilter--;
    }
    else nFilter = 0;
  }
  CDataProcess* pProcess = pMain->GetWbProcess(nProcess, nFilter);
  // if process ready call process properties dialog
  if (pProcess)
  {
    if (pProcess->PropertiesDialog() == IDOK)
      GetDocument()->SetModifiedFlag(); // documents data has been modified
  }
}

/***************************************************************************/
// CSaWorkbenchView::LoadAndSortFilter Loads and sorts (left align) filters
// The function loads the filters from the mainframe if the flag bLoad is
// TRUE.
/***************************************************************************/
void CSaWorkbenchView::LoadAndSortFilter(int nProcess, int* pnFilter1, int* pnFilter2,
                                         int* pnFilter3, BOOL bLoad)
{
  if (bLoad)
  {
    CMainFrame* pMain = (CMainFrame*)AfxGetMainWnd();
    *pnFilter1 = pMain->GetWbFilterID(nProcess, 2);
    *pnFilter2 = pMain->GetWbFilterID(nProcess, 1);
    *pnFilter3 = pMain->GetWbFilterID(nProcess, 0);
  }
  if (!*pnFilter1)
  {
    *pnFilter1 = *pnFilter2;
    *pnFilter2 = *pnFilter3;
    *pnFilter3 = 0;
  }
  if (!*pnFilter1)
  {
    *pnFilter1 = *pnFilter2;
    *pnFilter2 = 0;
  }
}

/////////////////////////////////////////////////////////////////////////////
// CSaWorkbenchView message handlers

/***************************************************************************/
// CSaWorkbenchView::OnInitialUpdate Initialisation
/***************************************************************************/
void CSaWorkbenchView::OnInitialUpdate()
{
  CFormView::OnInitialUpdate();
  // build and place the static text windows
  m_aProcText[0].Init(IDC_PROCTEXT0, this);
  m_aProcText[1].Init(IDC_PROCTEXT1, this);
  m_aProcText[2].Init(IDC_PROCTEXT2, this);
  m_aProcText[3].Init(IDC_PROCTEXT3, this);
  // build and place the fancy arrow windows
  m_aArrow[0][0].Init(IDC_ARROW0, this);
  m_aArrow[1][0].Init(IDC_ARROW1, this);
  m_aArrow[2][0].Init(IDC_ARROW2, this);
  m_aArrow[3][0].Init(IDC_ARROW3, this);
  m_aArrow[0][1].Init(IDC_ARROW10, this);
  m_aArrow[1][1].Init(IDC_ARROW11, this);
  m_aArrow[2][1].Init(IDC_ARROW12, this);
  m_aArrow[3][1].Init(IDC_ARROW13, this);
  m_aArrow[0][2].Init(IDC_ARROW20, this);
  m_aArrow[1][2].Init(IDC_ARROW21, this);
  m_aArrow[2][2].Init(IDC_ARROW22, this);
  m_aArrow[3][2].Init(IDC_ARROW23, this);
  m_aArrow[0][3].Init(IDC_ARROW30, this);
  m_aArrow[1][3].Init(IDC_ARROW31, this);
  m_aArrow[2][3].Init(IDC_ARROW32, this);
  m_aArrow[3][3].Init(IDC_ARROW33, this);
  // create the raw bitmapbuttons
  m_aRawButton[0].AutoLoad(IDC_RAW0, this);
  m_aRawButton[1].AutoLoad(IDC_RAW1, this);
  m_aRawButton[2].AutoLoad(IDC_RAW2, this);
  m_aRawButton[3].AutoLoad(IDC_RAW3, this);
  // create the filter bitmap buttons
  m_aFilterButton[0][0].AutoLoad(IDC_FILTER10, this);
  m_aFilterButton[0][1].AutoLoad(IDC_FILTER20, this);
  m_aFilterButton[0][2].AutoLoad(IDC_FILTER30, this);
  m_aFilterButton[1][0].AutoLoad(IDC_FILTER11, this);
  m_aFilterButton[1][1].AutoLoad(IDC_FILTER21, this);
  m_aFilterButton[1][2].AutoLoad(IDC_FILTER31, this);
  m_aFilterButton[2][0].AutoLoad(IDC_FILTER12, this);
  m_aFilterButton[2][1].AutoLoad(IDC_FILTER22, this);
  m_aFilterButton[2][2].AutoLoad(IDC_FILTER32, this);
  m_aFilterButton[3][0].AutoLoad(IDC_FILTER13, this);
  m_aFilterButton[3][1].AutoLoad(IDC_FILTER23, this);
  m_aFilterButton[3][2].AutoLoad(IDC_FILTER33, this);
  // create the result bitmapbuttons
  m_aGraphButton[0].AutoLoad(IDC_RESULT0, this);
  m_aGraphButton[1].AutoLoad(IDC_RESULT1, this);
  m_aGraphButton[2].AutoLoad(IDC_RESULT2, this);
  m_aGraphButton[3].AutoLoad(IDC_RESULT3, this);
  // now initialize the filters for process 0
  int nFilter1, nFilter2, nFilter3;
  LoadAndSortFilter(0, &nFilter1, &nFilter2, &nFilter3);
  m_aFilterButton[0][0].LoadBitmaps(GetFilterResource(nFilter1));
  if (nFilter1) GetDlgItem(IDC_FILTER10)->EnableWindow(TRUE);
  else m_aFilterButton[0][0].ShowWindow(SW_HIDE);
  m_aFilterButton[0][1].LoadBitmaps(GetFilterResource(nFilter2));
  if (nFilter2) GetDlgItem(IDC_FILTER20)->EnableWindow(TRUE);
  else m_aFilterButton[0][1].ShowWindow(SW_HIDE);
  m_aFilterButton[0][2].LoadBitmaps(GetFilterResource(nFilter3));
  if (nFilter3) GetDlgItem(IDC_FILTER30)->EnableWindow(TRUE);
  else m_aFilterButton[0][2].ShowWindow(SW_HIDE);
  // now initialize the filters for process 1
  LoadAndSortFilter(1, &nFilter1, &nFilter2, &nFilter3);
  m_aFilterButton[1][0].LoadBitmaps(GetFilterResource(nFilter1));
  if (nFilter1) GetDlgItem(IDC_FILTER11)->EnableWindow(TRUE);
  else m_aFilterButton[1][0].ShowWindow(SW_HIDE);
  m_aFilterButton[1][1].LoadBitmaps(GetFilterResource(nFilter2));
  if (nFilter2) GetDlgItem(IDC_FILTER21)->EnableWindow(TRUE);
  else m_aFilterButton[1][1].ShowWindow(SW_HIDE);
  m_aFilterButton[1][2].LoadBitmaps(GetFilterResource(nFilter3));
  if (nFilter3) GetDlgItem(IDC_FILTER31)->EnableWindow(TRUE);
  else m_aFilterButton[1][2].ShowWindow(SW_HIDE);
  // now initialize the filters for process 2
  LoadAndSortFilter(2, &nFilter1, &nFilter2, &nFilter3);
  m_aFilterButton[2][0].LoadBitmaps(GetFilterResource(nFilter1));
  if (nFilter1) GetDlgItem(IDC_FILTER12)->EnableWindow(TRUE);
  else m_aFilterButton[2][0].ShowWindow(SW_HIDE);
  m_aFilterButton[2][1].LoadBitmaps(GetFilterResource(nFilter2));
  if (nFilter2) GetDlgItem(IDC_FILTER22)->EnableWindow(TRUE);
  else m_aFilterButton[2][1].ShowWindow(SW_HIDE);
  m_aFilterButton[2][2].LoadBitmaps(GetFilterResource(nFilter3));
  if (nFilter3) GetDlgItem(IDC_FILTER32)->EnableWindow(TRUE);
  else m_aFilterButton[2][2].ShowWindow(SW_HIDE);
  // now initialize the filters for process 3
  LoadAndSortFilter(3, &nFilter1, &nFilter2, &nFilter3);
  m_aFilterButton[3][0].LoadBitmaps(GetFilterResource(nFilter1));
  if (nFilter1) GetDlgItem(IDC_FILTER13)->EnableWindow(TRUE);
  else m_aFilterButton[3][0].ShowWindow(SW_HIDE);
  m_aFilterButton[3][1].LoadBitmaps(GetFilterResource(nFilter2));
  if (nFilter2) GetDlgItem(IDC_FILTER23)->EnableWindow(TRUE);
  else m_aFilterButton[3][1].ShowWindow(SW_HIDE);
  m_aFilterButton[3][2].LoadBitmaps(GetFilterResource(nFilter3));
  if (nFilter3) GetDlgItem(IDC_FILTER33)->EnableWindow(TRUE);
  else m_aFilterButton[3][2].ShowWindow(SW_HIDE);
  // set up the documents title
  CSaString szTitle;
  szTitle.LoadString(IDR_WORKBENCH);
  szTitle = GetDocument()->GetTitle() + ": " + szTitle;
  GetDocument()->SetTitle(szTitle);
}

/***************************************************************************/
// CSaWorkbenchView::OnCtlColor Changes the background of the workbench view
/***************************************************************************/
HBRUSH CSaWorkbenchView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nWhich)
{
  return ((nWhich == CTLCOLOR_DLG)
    || (nWhich == CTLCOLOR_EDIT)
    || (nWhich == CTLCOLOR_LISTBOX)
    || (nWhich == CTLCOLOR_BTN)
    || (nWhich == CTLCOLOR_STATIC)) ? GetBkgBrush()
    : CFormView::OnCtlColor(pDC, pWnd, nWhich);
}

/***************************************************************************/
// CSaWorkbenchView::OnActivateView View activating or desactivating
// In case of disactivating, the application will be informed about the
// file name of the workbench document.
/***************************************************************************/
void CSaWorkbenchView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactivateView)
{
  CView::OnActivateView(bActivate, pActivateView, pDeactivateView);
  if (!bActivate) // disactivating
  {
    CSaString szPath;
    szPath = GetDocument()->GetPathName();
    ((CSaApp*)AfxGetApp())->SetWorkbenchPath(&szPath);
  }
}

/***************************************************************************/
// CSaWorkbenchView::OnDestroy Destroying the workbench view
// The application will be informed about the closing workbench.
/***************************************************************************/
void CSaWorkbenchView::OnDestroy()
{
  CFormView::OnDestroy();
  ((CSaApp*)AfxGetApp())->WorkbenchClosed();
}

/***************************************************************************/
// CSaWorkbenchView::OnProcess0 Configure process 0
/***************************************************************************/
void CSaWorkbenchView::OnProcess0()
{
  CWbDlgProcesses dlgProcesses;
  dlgProcesses.m_nProcess = 0; // set process number
  if (dlgProcesses.DoModal() == IDOK)
  {
    SetupFilterProcesses(&dlgProcesses);
    // load appropriate bitmaps for filter bitmap buttons
    int nFilter1, nFilter2, nFilter3;
    LoadAndSortFilter(dlgProcesses.m_nProcess, &nFilter1, &nFilter2, &nFilter3);
    m_aFilterButton[0][0].LoadBitmaps(GetFilterResource(nFilter1));
    if (nFilter1)
    {
      GetDlgItem(IDC_FILTER10)->EnableWindow(TRUE);
      m_aFilterButton[0][0].ShowWindow(SW_SHOW);
    }
    else
    {
      GetDlgItem(IDC_FILTER10)->EnableWindow(FALSE);
      m_aFilterButton[0][0].ShowWindow(SW_HIDE);
    }
    m_aFilterButton[0][1].LoadBitmaps(GetFilterResource(nFilter2));
    if (nFilter2)
    {
      GetDlgItem(IDC_FILTER20)->EnableWindow(TRUE);
      m_aFilterButton[0][1].ShowWindow(SW_SHOW);
    }
    else
    {
      GetDlgItem(IDC_FILTER20)->EnableWindow(FALSE);
      m_aFilterButton[0][1].ShowWindow(SW_HIDE);
    }
    m_aFilterButton[0][2].LoadBitmaps(GetFilterResource(nFilter3));
    if (nFilter3)
    {
      GetDlgItem(IDC_FILTER30)->EnableWindow(TRUE);
      m_aFilterButton[0][2].ShowWindow(SW_SHOW);
    }
    else
    {
      GetDlgItem(IDC_FILTER30)->EnableWindow(FALSE);
      m_aFilterButton[0][2].ShowWindow(SW_HIDE);
    }
    Invalidate(TRUE); // redraw whole view
  }
}

/***************************************************************************/
// CSaWorkbenchView::OnProcess1 Configure process 1
/***************************************************************************/
void CSaWorkbenchView::OnProcess1()
{
  CWbDlgProcesses dlgProcesses;
  dlgProcesses.m_nProcess = 1; // set process number
  if (dlgProcesses.DoModal() == IDOK)
  {
    SetupFilterProcesses(&dlgProcesses);
    // load appropriate bitmaps for filter bitmap buttons
    int nFilter1, nFilter2, nFilter3;
    LoadAndSortFilter(dlgProcesses.m_nProcess, &nFilter1, &nFilter2, &nFilter3);
    m_aFilterButton[1][0].LoadBitmaps(GetFilterResource(nFilter1));
    if (nFilter1)
    {
      GetDlgItem(IDC_FILTER11)->EnableWindow(TRUE);
      m_aFilterButton[1][0].ShowWindow(SW_SHOW);
    }
    else
    {
      GetDlgItem(IDC_FILTER11)->EnableWindow(FALSE);
      m_aFilterButton[1][0].ShowWindow(SW_HIDE);
    }
    m_aFilterButton[1][1].LoadBitmaps(GetFilterResource(nFilter2));
    if (nFilter2)
    {
      GetDlgItem(IDC_FILTER21)->EnableWindow(TRUE);
      m_aFilterButton[1][1].ShowWindow(SW_SHOW);
    }
    else
    {
      GetDlgItem(IDC_FILTER21)->EnableWindow(FALSE);
      m_aFilterButton[1][1].ShowWindow(SW_HIDE);
    }
    m_aFilterButton[1][2].LoadBitmaps(GetFilterResource(nFilter3));
    if (nFilter3)
    {
      GetDlgItem(IDC_FILTER31)->EnableWindow(TRUE);
      m_aFilterButton[1][2].ShowWindow(SW_SHOW);
    }
    else
    {
      GetDlgItem(IDC_FILTER31)->EnableWindow(FALSE);
      m_aFilterButton[1][2].ShowWindow(SW_HIDE);
    }
    Invalidate(TRUE); // redraw whole view
  }
}

/***************************************************************************/
// CSaWorkbenchView::OnProcess2 Configure process 2
/***************************************************************************/
void CSaWorkbenchView::OnProcess2()
{
  CWbDlgProcesses dlgProcesses;
  dlgProcesses.m_nProcess = 2; // set process number
  if (dlgProcesses.DoModal() == IDOK)
  {
    SetupFilterProcesses(&dlgProcesses);
    // load appropriate bitmaps for filter bitmap buttons
    int nFilter1, nFilter2, nFilter3;
    LoadAndSortFilter(dlgProcesses.m_nProcess, &nFilter1, &nFilter2, &nFilter3);
    m_aFilterButton[2][0].LoadBitmaps(GetFilterResource(nFilter1));
    if (nFilter1)
    {
      GetDlgItem(IDC_FILTER12)->EnableWindow(TRUE);
      m_aFilterButton[2][0].ShowWindow(SW_SHOW);
    }
    else
    {
      GetDlgItem(IDC_FILTER12)->EnableWindow(FALSE);
      m_aFilterButton[2][0].ShowWindow(SW_HIDE);
    }
    m_aFilterButton[2][1].LoadBitmaps(GetFilterResource(nFilter2));
    if (nFilter2)
    {
      GetDlgItem(IDC_FILTER22)->EnableWindow(TRUE);
      m_aFilterButton[2][1].ShowWindow(SW_SHOW);
    }
    else
    {
      GetDlgItem(IDC_FILTER22)->EnableWindow(FALSE);
      m_aFilterButton[2][1].ShowWindow(SW_HIDE);
    }
    m_aFilterButton[2][2].LoadBitmaps(GetFilterResource(nFilter3));
    if (nFilter3)
    {
      GetDlgItem(IDC_FILTER32)->EnableWindow(TRUE);
      m_aFilterButton[2][2].ShowWindow(SW_SHOW);
    }
    else
    {
      GetDlgItem(IDC_FILTER32)->EnableWindow(FALSE);
      m_aFilterButton[2][2].ShowWindow(SW_HIDE);
    }
    Invalidate(TRUE); // redraw whole view
  }
}

/***************************************************************************/
// CSaWorkbenchView::OnProcess3 Configure process 3
/***************************************************************************/
void CSaWorkbenchView::OnProcess3()
{
  CWbDlgProcesses dlgProcesses;
  dlgProcesses.m_nProcess = 3; // set process number
  if (dlgProcesses.DoModal() == IDOK)
  {
    SetupFilterProcesses(&dlgProcesses);
    // load appropriate bitmaps for filter bitmap buttons
    int nFilter1, nFilter2, nFilter3;
    LoadAndSortFilter(dlgProcesses.m_nProcess, &nFilter1, &nFilter2, &nFilter3);
    m_aFilterButton[3][0].LoadBitmaps(GetFilterResource(nFilter1));
    if (nFilter1)
    {
      GetDlgItem(IDC_FILTER13)->EnableWindow(TRUE);
      m_aFilterButton[3][0].ShowWindow(SW_SHOW);
    }
    else
    {
      GetDlgItem(IDC_FILTER13)->EnableWindow(FALSE);
      m_aFilterButton[3][0].ShowWindow(SW_HIDE);
    }
    m_aFilterButton[3][1].LoadBitmaps(GetFilterResource(nFilter2));
    if (nFilter2)
    {
      GetDlgItem(IDC_FILTER23)->EnableWindow(TRUE);
      m_aFilterButton[3][1].ShowWindow(SW_SHOW);
    }
    else
    {
      GetDlgItem(IDC_FILTER23)->EnableWindow(FALSE);
      m_aFilterButton[3][1].ShowWindow(SW_HIDE);
    }
    m_aFilterButton[3][2].LoadBitmaps(GetFilterResource(nFilter3));
    if (nFilter3)
    {
      GetDlgItem(IDC_FILTER33)->EnableWindow(TRUE);
      m_aFilterButton[3][2].ShowWindow(SW_SHOW);
    }
    else
    {
      GetDlgItem(IDC_FILTER33)->EnableWindow(FALSE);
      m_aFilterButton[3][2].ShowWindow(SW_HIDE);
    }
    Invalidate(TRUE); // redraw whole view
  }
}

/***************************************************************************/
// CSaWorkbenchView::OnFilter10 Configure filter 10
/***************************************************************************/
void CSaWorkbenchView::OnFilter10()
{
  CallPropertiesDialog(0, 0);
}

/***************************************************************************/
// CSaWorkbenchView::OnFilter11 Configure filter 11
/***************************************************************************/
void CSaWorkbenchView::OnFilter11()
{
  CallPropertiesDialog(1, 0);
}

/***************************************************************************/
// CSaWorkbenchView::OnFilter12 Configure filter 12
/***************************************************************************/
void CSaWorkbenchView::OnFilter12()
{
  CallPropertiesDialog(2, 0);
}

/***************************************************************************/
// CSaWorkbenchView::OnFilter13 Configure filter 13
/***************************************************************************/
void CSaWorkbenchView::OnFilter13()
{
  CallPropertiesDialog(3, 0);
}

/***************************************************************************/
// CSaWorkbenchView::OnFilter20 Configure filter 20
/***************************************************************************/
void CSaWorkbenchView::OnFilter20()
{
  CallPropertiesDialog(0, 1);
}

/***************************************************************************/
// CSaWorkbenchView::OnFilter21 Configure filter 21
/***************************************************************************/
void CSaWorkbenchView::OnFilter21()
{
  CallPropertiesDialog(1, 1);
}

/***************************************************************************/
// CSaWorkbenchView::OnFilter22 Configure filter 22
/***************************************************************************/
void CSaWorkbenchView::OnFilter22()
{
  CallPropertiesDialog(2, 1);
}

/***************************************************************************/
// CSaWorkbenchView::OnFilter23 Configure filter 23
/***************************************************************************/
void CSaWorkbenchView::OnFilter23()
{
  CallPropertiesDialog(3, 1);
}

/***************************************************************************/
// CSaWorkbenchView::OnFilter30 Configure filter 30
/***************************************************************************/
void CSaWorkbenchView::OnFilter30()
{
  CallPropertiesDialog(0, 2);
}

/***************************************************************************/
// CSaWorkbenchView::OnFilter31 Configure filter 31
/***************************************************************************/
void CSaWorkbenchView::OnFilter31()
{
  CallPropertiesDialog(1, 2);
}

/***************************************************************************/
// CSaWorkbenchView::OnFilter32 Configure filter 32
/***************************************************************************/
void CSaWorkbenchView::OnFilter32()
{
  CallPropertiesDialog(2, 2);
}

/***************************************************************************/
// CSaWorkbenchView::OnFilter33 Configure filter 33
/***************************************************************************/
void CSaWorkbenchView::OnFilter33()
{
  CallPropertiesDialog(3, 2);
}

static const char* psz_wbview       = "wbview";
static const char* psz_placement    = "placement";
static const char* psz_processlist  = "processlist";
static const char* psz_filterlist   = "filterlist";
static const char* psz_filterID     = "filterID";


/***************************************************************************/
// CSaWorkbenchView::WriteProperties Write workbench properties
/***************************************************************************/
void CSaWorkbenchView::WriteProperties(Object_ostream& obs)
{
  CMainFrame* pMain = (CMainFrame*)AfxGetMainWnd();
  obs.WriteBeginMarker(psz_wbview);

  WINDOWPLACEMENT wpl;
  GetParent()->GetWindowPlacement(&wpl);
  obs.WriteWindowPlacement(psz_placement, wpl);

  obs.WriteBeginMarker(psz_processlist);
  for (int nLoop = 0; nLoop < MAX_PROCESS_NUMBER; nLoop++)
  {
    obs.WriteBeginMarker(psz_filterlist);
    for (int nInnerLoop = 0; nInnerLoop < MAX_FILTER_NUMBER; nInnerLoop++)
    {
      obs.WriteInteger(psz_filterID, pMain->GetWbFilterID(nLoop, nInnerLoop));
      if (pMain->GetWbProcess(nLoop, nInnerLoop)) pMain->GetWbProcess(nLoop, nInnerLoop)->WriteProperties(obs);
    }
    obs.WriteEndMarker(psz_filterlist);
  }
  obs.WriteEndMarker(psz_processlist);
  obs.WriteEndMarker(psz_wbview);
}

/***************************************************************************/
// CSaWorkbenchView::bReadProperties Read workbench properties
/***************************************************************************/
BOOL CSaWorkbenchView::bReadProperties(Object_istream& obs)
{
  CMainFrame* pMain = (CMainFrame*)AfxGetMainWnd();
  // clear all old processes first if you are loading a new file
  if (((CWorkbenchDoc*)GetDocument())->IsDifferentFile()) pMain->DeleteWbProcesses();
  if (!obs.bReadBeginMarker(psz_wbview)) return FALSE;

  WINDOWPLACEMENT wpl;
  if (!obs.bReadWindowPlacement(psz_placement, wpl)) return FALSE;
  int eInitialShowCmd = wpl.showCmd;
  wpl.showCmd = SW_HIDE;
  GetParent()->SetWindowPlacement(&wpl);

  int nLoop = 0;
  int nFilterID;
  CDataProcess* pProcess;
  BOOL bRet = obs.bReadBeginMarker(psz_processlist);
  while (!obs.bAtEnd() && bRet)
  {
    bRet = obs.bReadBeginMarker(psz_filterlist);
    int nInnerLoop = 0;
    while (!obs.bAtEnd() && bRet)
    {
      bRet = obs.bReadInteger(psz_filterID, nFilterID);
      if (!nFilterID)
      {
        if (pMain->GetWbProcess(nLoop, nInnerLoop))
        {
          delete pMain->GetWbProcess(nLoop, nInnerLoop);
          pMain->SetWbProcess(nLoop, nInnerLoop, NULL);
          pMain->SetWbFilterID(nLoop, nInnerLoop, 0);
        }
      }
      if (bRet && nFilterID)
      {
        // delete old and create new process if different
        if (pMain->GetWbFilterID(nLoop, nInnerLoop) != nFilterID)
        {
          if (pMain->GetWbProcess(nLoop, nInnerLoop)) delete pMain->GetWbProcess(nLoop, nInnerLoop);
          pProcess = CreateWbProcess(nFilterID);
          pMain->SetWbProcess(nLoop, nInnerLoop, pProcess);
          pMain->SetWbFilterID(nLoop, nInnerLoop, nFilterID);
        }
        else pProcess = pMain->GetWbProcess(nLoop, nInnerLoop);
        if (pProcess) bRet = pProcess->bReadProperties(obs);
        else bRet = FALSE;
      }
      nInnerLoop++;
      if (nInnerLoop >= MAX_FILTER_NUMBER) break;
    }
    if (bRet) bRet = obs.bReadEndMarker(psz_filterlist);
    nLoop++;
    if (nLoop >= MAX_PROCESS_NUMBER) break;
  }
  if (bRet) bRet = obs.bReadEndMarker(psz_processlist);
  if (bRet) bRet = obs.bReadEndMarker(psz_wbview);
  // if read fails, clear processes again and switch pack to plain
  if (!bRet) pMain->DeleteWbProcesses();
  if ((eInitialShowCmd == SW_SHOWMINIMIZED) || (eInitialShowCmd == SW_SHOWMAXIMIZED))
    GetParent()->ShowWindow(eInitialShowCmd);
  else GetParent()->ShowWindow(SW_SHOW);
  return bRet;
}
