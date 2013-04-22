/////////////////////////////////////////////////////////////////////////////
// sa_w_dlg.cpp:
// Implementation of the CWbDlgProcesses (dialog)
//                     CWbDlgFilterPass (dialog)
//                     CWbDlgFilterReverb (dialog)
//                     CWbDlgFilterEquation (dialog) classes.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Process\Process.h"
#include "sa_wbch.h"
#include "sa_w_dlg.h"
#include "mainfrm.h"
#include "Process\sa_w_equ.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CWbDlgProcesses dialog
// Lets the user configure the workbench processes

/////////////////////////////////////////////////////////////////////////////
// CWbDlgProcesses message map

BEGIN_MESSAGE_MAP(CWbDlgProcesses, CDialog)
    ON_BN_CLICKED(IDC_PROPERTIES1, OnProperties1)
    ON_BN_CLICKED(IDC_PROPERTIES2, OnProperties2)
    ON_BN_CLICKED(IDC_PROPERTIES3, OnProperties3)
    ON_WM_CLOSE()
    ON_CBN_SELCHANGE(IDC_FILTERTYPE1, OnSelchangeFilter1)
    ON_CBN_SELCHANGE(IDC_FILTERTYPE2, OnSelchangeFilter2)
    ON_CBN_SELCHANGE(IDC_FILTERTYPE3, OnSelchangeFilter3)
    ON_COMMAND(IDHELP, OnHelpWorkbench)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWbDlgProcesses construction/destruction/creation

/***************************************************************************/
// CWbDlgProcesses::CWbDlgProcesses Constructor
/***************************************************************************/
CWbDlgProcesses::CWbDlgProcesses(CWnd * pParent) : CDialog(CWbDlgProcesses::IDD, pParent)
{
    m_nFilter1 = 0;
    m_nFilter2 = 0;
    m_nFilter3 = 0;
    m_nProcess = 0;
    m_pWbProcessFilter1 = NULL;
    m_pWbProcessFilter2 = NULL;
    m_pWbProcessFilter3 = NULL;
    m_nLocalFilter1 = 0;
    m_nLocalFilter2 = 0;
    m_nLocalFilter3 = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CWbDlgProcesses helper functions

/***************************************************************************/
// CWbDlgProcesses::DoDataExchange Data exchange
/***************************************************************************/
void CWbDlgProcesses::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_CBIndex(pDX, IDC_FILTERTYPE1, m_nFilter1);
    DDX_CBIndex(pDX, IDC_FILTERTYPE2, m_nFilter2);
    DDX_CBIndex(pDX, IDC_FILTERTYPE3, m_nFilter3);
}

/////////////////////////////////////////////////////////////////////////////
// CWbDlgProcesses message handlers

/***************************************************************************/
// CWbDlgProcesses::OnInitDialog Dialog initialisation
// All the necessary informations are put into the dialog controls.
/***************************************************************************/
BOOL CWbDlgProcesses::OnInitDialog()
{
    CDialog::OnInitDialog();
    // set the process number in the dialogs caption
    CString szCaption;
    GetWindowText(szCaption);
    TCHAR szNumber[2];
    swprintf_s(szNumber, _T("%u"), m_nProcess + 1);
    szCaption = szCaption + " " + szNumber;
    SetWindowText(szCaption);
    // initialize member data
    ((CSaWorkbenchView *)GetParent())->LoadAndSortFilter(m_nProcess, &m_nFilter1, &m_nFilter2, &m_nFilter3);
    // disable properties buttons for plain filters
    if (!m_nFilter1)
    {
        GetDlgItem(IDC_PROPERTIES1)->EnableWindow(FALSE);
    }

    if (!m_nFilter2)
    {
        GetDlgItem(IDC_PROPERTIES2)->EnableWindow(FALSE);
    }

    if (!m_nFilter3)
    {
        GetDlgItem(IDC_PROPERTIES3)->EnableWindow(FALSE);
    }

    UpdateData(FALSE);
    CenterWindow();
    return TRUE;
}

/***************************************************************************/
// CWbDlgProcesses::OnProperties1 Filter one properties hit
// The process for filter one first has to be created (on the heap) and then
// its dialog gets called (via the process class). If there was already a
// process created for this filter, it will be deleted first.
/***************************************************************************/
void CWbDlgProcesses::OnProperties1()
{
    UpdateData(TRUE);
    // check if filter already exists
    CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
    int nLoop;
    for (nLoop = 0; nLoop < MAX_FILTER_NUMBER; nLoop++)
    {
        if (m_nFilter1 == pMain->GetWbFilterID(m_nProcess, nLoop))
        {
            break;
        }
    }
    if (nLoop < MAX_FILTER_NUMBER)
    {
        // filter exists
        if (pMain->GetWbProcess(m_nProcess, nLoop))
        {
            pMain->GetWbProcess(m_nProcess, nLoop)->PropertiesDialog();
        }
    }
    else
    {
        if (m_nLocalFilter1 != m_nFilter1)
        {
            // delete existing process
            if (m_pWbProcessFilter1)
            {
                delete m_pWbProcessFilter1;
            }
            // create new process
            m_pWbProcessFilter1 = ((CSaWorkbenchView *)GetParent())->CreateWbProcess(m_nFilter1);
            m_nLocalFilter1 = m_nFilter1;
        }
        // if process created call process properties dialog
        if (m_pWbProcessFilter1)
        {
            m_pWbProcessFilter1->PropertiesDialog();
        }
        else
        {
            // error creating filter process
            m_nFilter1 = 0; // switch back to plain
            UpdateData(FALSE);
        }
    }
}

/***************************************************************************/
// CWbDlgProcesses::OnProperties2 Filter two properties hit
// The process for filter two first has to be created (on the heap) and then
// its dialog gets called (via the process class). If there was already a
// process created for this filter, it will be deleted first.
/***************************************************************************/
void CWbDlgProcesses::OnProperties2()
{
    UpdateData(TRUE);
    // check if filter already exists
    CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
    int nLoop;
    for (nLoop = 0; nLoop < MAX_FILTER_NUMBER; nLoop++)
    {
        if (m_nFilter2 == pMain->GetWbFilterID(m_nProcess, nLoop))
        {
            break;
        }
    }
    if (nLoop < MAX_FILTER_NUMBER)
    {
        // filter exists
        if (pMain->GetWbProcess(m_nProcess, nLoop))
        {
            pMain->GetWbProcess(m_nProcess, nLoop)->PropertiesDialog();
        }
    }
    else
    {
        if (m_nLocalFilter2 != m_nFilter2)
        {
            // delete existing process
            if (m_pWbProcessFilter2)
            {
                delete m_pWbProcessFilter2;
            }
            // create new process
            m_pWbProcessFilter2 = ((CSaWorkbenchView *)GetParent())->CreateWbProcess(m_nFilter2); // create new process
            m_nLocalFilter2 = m_nFilter2;
        }
        // if process created call process properties dialog
        if (m_pWbProcessFilter2)
        {
            m_pWbProcessFilter2->PropertiesDialog();
        }
        else
        {
            // error creating filter process
            m_nFilter2 = 0; // switch back to plain
            UpdateData(FALSE);
        }
    }
}

/***************************************************************************/
// CWbDlgProcesses::OnProperties3 Filter three properties hit
// The process for filter three first has to be created (on the heap) and then
// its dialog gets called (via the process class). If there was already a
// process created for this filter, it will be deleted first.
/***************************************************************************/
void CWbDlgProcesses::OnProperties3()
{
    UpdateData(TRUE);
    // check if filter already exists
    CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
    int nLoop;
    for (nLoop = 0; nLoop < MAX_FILTER_NUMBER; nLoop++)
    {
        if (m_nFilter3 == pMain->GetWbFilterID(m_nProcess, nLoop))
        {
            break;
        }
    }
    if (nLoop < MAX_FILTER_NUMBER)
    {
        // filter exists
        if (pMain->GetWbProcess(m_nProcess, nLoop))
        {
            pMain->GetWbProcess(m_nProcess, nLoop)->PropertiesDialog();
        }
    }
    else
    {
        if (m_nLocalFilter3 != m_nFilter3)
        {
            // delete existing process
            if (m_pWbProcessFilter3)
            {
                delete m_pWbProcessFilter3;
            }
            // create new process
            m_pWbProcessFilter3 = ((CSaWorkbenchView *)GetParent())->CreateWbProcess(m_nFilter3);
            m_nLocalFilter3 = m_nFilter3;
        }
        // if process created call process properties dialog
        if (m_pWbProcessFilter3)
        {
            m_pWbProcessFilter3->PropertiesDialog();
        }
        else
        {
            // error creating filter process
            m_nFilter3 = 0; // switch back to plain
            UpdateData(FALSE);
        }
    }
}

/***************************************************************************/
// CWbDlgProcesses::OnSelchangeFilter1 Selection for filter one changed
// The function checks, if this kind of filter already has been selected for
// another spot. If so it switches back to plain and informs the user. If
// plain is selected, it desables the properties button for this filter,
// otherwise enables it.
/***************************************************************************/
void CWbDlgProcesses::OnSelchangeFilter1()
{
    UpdateData(TRUE);
    // check if this filtertype already used
    if ((m_nFilter1) && ((m_nFilter1 == m_nFilter2) || (m_nFilter1 == m_nFilter3)))
    {
        // already used, switch back to plain
        m_nFilter1 = 0;
        AfxMessageBox(IDS_ERROR_MULTIPLEFILTER,MB_OK,0);
        UpdateData(FALSE);
    }
    // enable/disable properties button
    if (!m_nFilter1)
    {
        GetDlgItem(IDC_PROPERTIES1)->EnableWindow(FALSE);
    }
    else
    {
        GetDlgItem(IDC_PROPERTIES1)->EnableWindow(TRUE);
        UpdateData(FALSE);
    }
}

/***************************************************************************/
// CWbDlgProcesses::OnSelchangeFilter2 Selection for filter two changed
// The function checks, if this kind of filter already has been selected for
// another spot. If so it switches back to plain and informs the user. If
// plain is selected, it desables the properties button for this filter,
// otherwise enables it.
/***************************************************************************/
void CWbDlgProcesses::OnSelchangeFilter2()
{
    UpdateData(TRUE);
    // check if this filtertype already used
    if ((m_nFilter2) && ((m_nFilter2 == m_nFilter1) || (m_nFilter2 == m_nFilter3)))
    {
        // already used, switch back to plain
        m_nFilter2 = 0;
        AfxMessageBox(IDS_ERROR_MULTIPLEFILTER,MB_OK,0);
        UpdateData(FALSE);
    }
    // enable/disable properties button
    if (!m_nFilter2)
    {
        GetDlgItem(IDC_PROPERTIES2)->EnableWindow(FALSE);
    }
    else
    {
        GetDlgItem(IDC_PROPERTIES2)->EnableWindow(TRUE);
        UpdateData(FALSE);
    }
}

/***************************************************************************/
// CWbDlgProcesses::OnSelchangeFilter3 Selection for filter three changed
// The function checks, if this kind of filter already has been selected for
// another spot. If so it switches back to plain and informs the user. If
// plain is selected, it desables the properties button for this filter,
// otherwise enables it.
/***************************************************************************/
void CWbDlgProcesses::OnSelchangeFilter3()
{
    UpdateData(TRUE);
    // check if this filtertype already used
    if ((m_nFilter3) && ((m_nFilter3 == m_nFilter1) || (m_nFilter3 == m_nFilter2)))
    {
        // already used, switch back to plain
        m_nFilter3 = 0;
        AfxMessageBox(IDS_ERROR_MULTIPLEFILTER,MB_OK,0);
        UpdateData(FALSE);
    }
    // enable/disable properties button
    if (!m_nFilter3)
    {
        GetDlgItem(IDC_PROPERTIES3)->EnableWindow(FALSE);
    }
    else
    {
        GetDlgItem(IDC_PROPERTIES3)->EnableWindow(TRUE);
        UpdateData(FALSE);
    }
}

/***************************************************************************/
// CWbDlgProcesses::OnCancel Button cancel hit
/***************************************************************************/
void CWbDlgProcesses::OnCancel()
{
    CDialog::OnCancel();
    SendMessage(WM_CLOSE);
}

/***************************************************************************/
// CWbDlgProcesses::OnClose Close the dialog
/***************************************************************************/
void CWbDlgProcesses::OnClose()
{
    CDialog::OnClose();
    // delete the created processes
    if (m_pWbProcessFilter1)
    {
        delete m_pWbProcessFilter1;
        m_pWbProcessFilter1 = NULL;
    }
    if (m_pWbProcessFilter2)
    {
        delete m_pWbProcessFilter2;
        m_pWbProcessFilter2 = NULL;
    }
    if (m_pWbProcessFilter3)
    {
        delete m_pWbProcessFilter3;
        m_pWbProcessFilter3 = NULL;
    }
}

/***************************************************************************/
// CWbDlgProcesses::OnHelpWorkbench Call Workbench help
/***************************************************************************/
void CWbDlgProcesses::OnHelpWorkbench()
{
    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath += "::/User_Interface/Menus/Tools/Workbench.htm";
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

//###########################################################################
// CWbDlgFilterPass dialog
// Lets the user configure the low/high/bandpass filters.

/////////////////////////////////////////////////////////////////////////////
// CWbDlgFilterPass message map

BEGIN_MESSAGE_MAP(CWbDlgFilterPass, CDialog)
    ON_COMMAND(IDC_PASS_UPPERSCROLL, OnUpperFreqScroll)
    ON_EN_KILLFOCUS(IDC_PASS_UPPEREDIT, OnKillfocusUpperfreq)
    ON_COMMAND(IDC_PASS_LOWERSCROLL, OnLowerFreqScroll)
    ON_EN_KILLFOCUS(IDC_PASS_LOWEREDIT, OnKillfocusLowerfreq)
    ON_COMMAND(IDC_PASS_ORDERSCROLL, OnOrderScroll)
    ON_EN_KILLFOCUS(IDC_PASS_FILTER_ORDER, OnKillfocusOrder)
    ON_COMMAND(IDHELP, OnHelpWorkbench)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWbDlgFilterPass construction/destruction/creation

/***************************************************************************/
// CWbDlgFilterPass::CWbDlgFilterPass Constructor
/***************************************************************************/
CWbDlgFilterPass::CWbDlgFilterPass(CWnd * pParent) : CDialog(CWbDlgFilterPass::IDD, pParent)
{
    m_nLowerFreq = 0;
    m_nUpperFreq = 0;
    m_nOrder = 0;
    m_bFilterFilter = FALSE;
    m_bBandPass = TRUE;
    m_bLoPass = TRUE; // m_bBandPass has higher priority
}

/////////////////////////////////////////////////////////////////////////////
// CWbDlgFilterPass helper functions

/***************************************************************************/
// CWbDlgFilterPass::DoDataExchange Data exchange
/***************************************************************************/
void CWbDlgFilterPass::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_PASS_LOWEREDIT, m_nLowerFreq);
    DDX_Text(pDX, IDC_PASS_UPPEREDIT, m_nUpperFreq);
    DDX_Text(pDX, IDC_PASS_FILTER_ORDER, m_nOrder);
    DDV_MinMaxUInt(pDX, m_nOrder, 1, 99);
    DDX_Check(pDX, IDC_PASS_FILTER_FILTER, m_bFilterFilter);
}

/////////////////////////////////////////////////////////////////////////////
// CWbDlgFilterPass message handlers

/***************************************************************************/
// CWbDlgFilterPass::OnInitDialog Dialog initialisation
// All the necessary informations are put into the dialog controls.
/***************************************************************************/
BOOL CWbDlgFilterPass::OnInitDialog()
{
    CDialog::OnInitDialog();
    // build and place the upper frequency spin control
    m_SpinUpperFreq.Init(IDC_PASS_UPPERSCROLL, this);
    // build and place the lower frequency spin control
    m_SpinLowerFreq.Init(IDC_PASS_LOWERSCROLL, this);
    m_SpinOrder.Init(IDC_PASS_ORDERSCROLL, this);
    if (!m_bBandPass)
    {
        // no bandpass
        if (m_bLoPass)
        {
            // lowpass, disable lower frequency controls
            GetDlgItem(IDC_PASS_LOWERTEXT)->EnableWindow(FALSE);
            GetDlgItem(IDC_PASS_LOWEREDIT)->EnableWindow(FALSE);
            GetDlgItem(IDC_PASS_LOWERSCROLL)->EnableWindow(FALSE);
            GetDlgItem(IDC_PASS_LOWERDIM)->EnableWindow(FALSE);
        }
        else
        {
            // hipass, disable upper frequency controls
            GetDlgItem(IDC_PASS_UPPERTEXT)->EnableWindow(FALSE);
            GetDlgItem(IDC_PASS_UPPEREDIT)->EnableWindow(FALSE);
            GetDlgItem(IDC_PASS_UPPERSCROLL)->EnableWindow(FALSE);
            GetDlgItem(IDC_PASS_UPPERDIM)->EnableWindow(FALSE);
        }
    }
    // set up right caption for dialog
    CString szCaption;
    GetWindowText(szCaption);
    if (m_bBandPass)
    {
        szCaption = szCaption.Mid(szCaption.Find('$') + 1, szCaption.Find(' ') - szCaption.Find('$') - 1) + szCaption.Right(szCaption.GetLength() - szCaption.Find(' ') - 1);
    }
    else
    {
        if (m_bLoPass)
        {
            szCaption = szCaption.Mid(szCaption.Find('/') + 1, szCaption.Find('$') - szCaption.Find('/') - 1) + szCaption.Right(szCaption.GetLength() - szCaption.Find(' ') - 1);
        }
        else
        {
            szCaption = szCaption.Left(szCaption.Find('/')) + szCaption.Right(szCaption.GetLength() - szCaption.Find(' ') - 1);
        }
    }
    SetWindowText(szCaption);
    return TRUE;
}

/***************************************************************************/
// CWbDlgFilterPass::OnUpperFreqScroll Upper frequency spin control hit
/***************************************************************************/
void CWbDlgFilterPass::OnUpperFreqScroll()
{
    m_nUpperFreq = GetDlgItemInt(IDC_PASS_UPPEREDIT, NULL, TRUE);
    if (m_SpinUpperFreq.UpperButtonClicked())
    {
        m_nUpperFreq++;
    }
    else
    {
        m_nUpperFreq--;
    }
    if (m_nUpperFreq > 22049)
    {
        m_nUpperFreq = 22049;
    }
    if (m_nUpperFreq < 2)
    {
        m_nUpperFreq = 2;
    }
    SetDlgItemInt(IDC_PASS_UPPEREDIT, m_nUpperFreq, TRUE);
    if (m_nLowerFreq >= m_nUpperFreq)
    {
        m_nLowerFreq = m_nUpperFreq - 1;
        SetDlgItemInt(IDC_PASS_LOWEREDIT, m_nLowerFreq, TRUE);
    }
}

/***************************************************************************/
// CWbDlgFilterPass::OnKillfocusUpperfreq Upper frequency changed
/***************************************************************************/
void CWbDlgFilterPass::OnKillfocusUpperfreq()
{
    m_nUpperFreq = GetDlgItemInt(IDC_PASS_UPPEREDIT, NULL, TRUE);
    if (m_nUpperFreq > 22049)
    {
        m_nUpperFreq = 22049;
    }
    if (m_nUpperFreq < 2)
    {
        m_nUpperFreq = 2;
    }
    SetDlgItemInt(IDC_PASS_UPPEREDIT, m_nUpperFreq, TRUE);
    if (m_nLowerFreq >= m_nUpperFreq)
    {
        m_nLowerFreq = m_nUpperFreq - 1;
        SetDlgItemInt(IDC_PASS_LOWEREDIT, m_nLowerFreq, TRUE);
    }
}

/***************************************************************************/
// CWbDlgFilterPass::OnLowerFreqScroll Lower frequency spin control hit
/***************************************************************************/
void CWbDlgFilterPass::OnLowerFreqScroll()
{
    m_nLowerFreq = GetDlgItemInt(IDC_PASS_LOWEREDIT, NULL, TRUE);
    if (m_SpinLowerFreq.UpperButtonClicked())
    {
        m_nLowerFreq++;
    }
    else
    {
        m_nLowerFreq--;
    }
    if (m_nLowerFreq > 22048)
    {
        m_nLowerFreq = 22048;
    }
    if (m_nLowerFreq < 1)
    {
        m_nLowerFreq = 1;
    }
    SetDlgItemInt(IDC_PASS_LOWEREDIT, m_nLowerFreq, TRUE);
    if (m_nLowerFreq >= m_nUpperFreq)
    {
        m_nUpperFreq = m_nLowerFreq + 1;
        SetDlgItemInt(IDC_PASS_UPPEREDIT, m_nUpperFreq, TRUE);
    }
}

/***************************************************************************/
// CWbDlgFilterPass::OnKillfocusLowerfreq Lower frequency changed
/***************************************************************************/
void CWbDlgFilterPass::OnKillfocusLowerfreq()
{
    m_nLowerFreq = GetDlgItemInt(IDC_PASS_LOWEREDIT, NULL, TRUE);
    if (m_nLowerFreq > 22048)
    {
        m_nLowerFreq = 22048;
    }
    if (m_nLowerFreq < 1)
    {
        m_nLowerFreq = 1;
    }
    SetDlgItemInt(IDC_PASS_LOWEREDIT, m_nLowerFreq, TRUE);
    if (m_nLowerFreq >= m_nUpperFreq)
    {
        m_nUpperFreq = m_nLowerFreq + 1;
        SetDlgItemInt(IDC_PASS_UPPEREDIT, m_nUpperFreq, TRUE);
    }
}

/***************************************************************************/
// CWbDlgFilterPass::OnOrderScroll Order spin control hit
/***************************************************************************/
void CWbDlgFilterPass::OnOrderScroll()
{
    m_nOrder = GetDlgItemInt(IDC_PASS_FILTER_ORDER, NULL, TRUE);
    if (m_SpinOrder.UpperButtonClicked())
    {
        m_nOrder++;
    }
    else
    {
        m_nOrder--;
    }
    if (m_nOrder > 20)
    {
        m_nOrder = 20;
    }
    if (m_nOrder < 1)
    {
        m_nOrder = 1;
    }
    SetDlgItemInt(IDC_PASS_FILTER_ORDER, m_nOrder, TRUE);
}

/***************************************************************************/
// CWbDlgFilterPass::OnKillfocusOrder Lower frequency changed
/***************************************************************************/
void CWbDlgFilterPass::OnKillfocusOrder()
{
    m_nOrder = GetDlgItemInt(IDC_PASS_FILTER_ORDER, NULL, TRUE);
    if (m_nOrder > 20)
    {
        m_nOrder = 20;
    }
    if (m_nOrder < 1)
    {
        m_nOrder = 1;
    }
    SetDlgItemInt(IDC_PASS_FILTER_ORDER, m_nOrder, TRUE);
}

/***************************************************************************/
// CWbDlgFilterPass::OnHelpWorkbench Call Workbench help
/***************************************************************************/
void CWbDlgFilterPass::OnHelpWorkbench()
{
    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath += "::/User_Interface/Menus/Tools/Workbench.htm";
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

//###########################################################################
// CWbDlgFilterReverb dialog
// Lets the user configure echo/reverb filters.

/////////////////////////////////////////////////////////////////////////////
// CWbDlgFilterReverb message map

BEGIN_MESSAGE_MAP(CWbDlgFilterReverb, CDialog)
    ON_COMMAND(IDC_ECHO_DELAYSCROLL, OnDelayScroll)
    ON_EN_KILLFOCUS(IDC_ECHO_DELAYEDIT, OnKillfocusDelay)
    ON_COMMAND(IDC_ECHO_GAINSCROLL, OnGainScroll)
    ON_EN_KILLFOCUS(IDC_ECHO_GAINEDIT, OnKillfocusGain)
    ON_COMMAND(IDHELP, OnHelpWorkbench)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWbDlgFilterReverb construction/destruction/creation

/***************************************************************************/
// CWbDlgFilterReverb::CWbDlgFilterReverb Constructor
/***************************************************************************/
CWbDlgFilterReverb::CWbDlgFilterReverb(CWnd * pParent) : CDialog(CWbDlgFilterReverb::IDD, pParent)
{
    m_nDelay = 100;
    m_nGain = -20;
    m_bEcho = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CWbDlgFilterReverb helper functions

/***************************************************************************/
// CWbDlgFilterReverb::DoDataExchange Data exchange
/***************************************************************************/
void CWbDlgFilterReverb::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_ECHO_DELAYEDIT, m_nDelay);
    DDX_Text(pDX, IDC_ECHO_GAINEDIT, m_nGain);
}

/////////////////////////////////////////////////////////////////////////////
// CWbDlgFilterReverb message handlers

/***************************************************************************/
// CWbDlgFilterReverb::OnInitDialog Dialog initialisation
// All the necessary informations are put into the dialog controls.
/***************************************************************************/
BOOL CWbDlgFilterReverb::OnInitDialog()
{
    CDialog::OnInitDialog();
    // build and place the delay spin control
    m_SpinDelay.Init(IDC_ECHO_DELAYSCROLL, this);
    // build and place the gain spin control
    m_SpinGain.Init(IDC_ECHO_GAINSCROLL, this);
    // set up right caption for dialog
    CString szCaption;
    GetWindowText(szCaption);
    if (m_bEcho)
    {
        szCaption = szCaption.Left(szCaption.Find('/')) + szCaption.Right(szCaption.GetLength() - szCaption.Find(' '));
    }
    else
    {
        szCaption = szCaption.Right(szCaption.GetLength() - szCaption.Find('/') - 1);
    }
    SetWindowText(szCaption);
    return TRUE;
}

/***************************************************************************/
// CWbDlgFilterReverb::OnDelayScroll Delay spin control hit
/***************************************************************************/
void CWbDlgFilterReverb::OnDelayScroll()
{
    m_nDelay = GetDlgItemInt(IDC_ECHO_DELAYEDIT, NULL, TRUE);
    if (m_SpinDelay.UpperButtonClicked())
    {
        m_nDelay++;
    }
    else
    {
        m_nDelay--;
    }
    if (m_nDelay > MAX_ECHO_DELAY)
    {
        m_nDelay = MAX_ECHO_DELAY;
    }
    if (m_nDelay < 1)
    {
        m_nDelay = 1;
    }
    SetDlgItemInt(IDC_ECHO_DELAYEDIT, m_nDelay, TRUE);
}

/***************************************************************************/
// CWbDlgFilterReverb::OnKillfocusDelay Delay changed
/***************************************************************************/
void CWbDlgFilterReverb::OnKillfocusDelay()
{
    m_nDelay = GetDlgItemInt(IDC_ECHO_DELAYEDIT, NULL, TRUE);
    if (m_nDelay > MAX_ECHO_DELAY)
    {
        m_nDelay = MAX_ECHO_DELAY;
    }
    if (m_nDelay < 1)
    {
        m_nDelay = 1;
    }
    SetDlgItemInt(IDC_ECHO_DELAYEDIT, m_nDelay, TRUE);
}

/***************************************************************************/
// CWbDlgFilterReverb::OnGainScroll Gain spin control hit
/***************************************************************************/
void CWbDlgFilterReverb::OnGainScroll()
{
    m_nGain = GetDlgItemInt(IDC_ECHO_GAINEDIT, NULL, TRUE);
    if (m_SpinGain.UpperButtonClicked())
    {
        m_nGain++;
    }
    else
    {
        m_nGain--;
    }
    if (m_nGain > -1)
    {
        m_nGain = -1;
    }
    if (m_nGain < -60)
    {
        m_nGain = -60;
    }
    SetDlgItemInt(IDC_ECHO_GAINEDIT, m_nGain, TRUE);
}

/***************************************************************************/
// CWbDlgFilterReverb::OnKillfocusGain Gain changed
/***************************************************************************/
void CWbDlgFilterReverb::OnKillfocusGain()
{
    m_nGain = GetDlgItemInt(IDC_ECHO_GAINEDIT, NULL, TRUE);
    if (m_nGain > -1)
    {
        m_nGain = -1;
    }
    if (m_nGain < -60)
    {
        m_nGain = -60;
    }
    SetDlgItemInt(IDC_ECHO_GAINEDIT, m_nGain, TRUE);
}

/***************************************************************************/
// CWbDlgFilterReverb::OnHelpWorkbench Call Workbench help
/***************************************************************************/
void CWbDlgFilterReverb::OnHelpWorkbench()
{
    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath += "::/User_Interface/Menus/Tools/Workbench.htm";
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

//###########################################################################
// CWbDlgFilterEquation dialog
// Lets the user configure the equation filter.

/////////////////////////////////////////////////////////////////////////////
// CWbDlgFilterEquation message map

BEGIN_MESSAGE_MAP(CWbDlgFilterEquation, CDialog)
    ON_COMMAND(IDHELP, OnHelpWorkbench)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWbDlgFilterEquation construction/destruction/creation

/***************************************************************************/
// CWbDlgFilterEquation::CWbDlgFilterEquation Constructor
/***************************************************************************/
CWbDlgFilterEquation::CWbDlgFilterEquation(CWnd * pParent) : CDialog(CWbDlgFilterEquation::IDD, pParent)
{
    m_szEquation = "";
}

/////////////////////////////////////////////////////////////////////////////
// CWbDlgFilterEquation helper functions

/***************************************************************************/
// CWbDlgFilterEquation::DoDataExchange Data exchange
/***************************************************************************/
void CWbDlgFilterEquation::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EQUATION, m_szEquation);
    DDV_MaxChars(pDX, m_szEquation, 64);
}

/////////////////////////////////////////////////////////////////////////////
// CWbDlgFilterEquation message handlers

/***************************************************************************/
// CWbDlgFilterEquation::OnInitDialog Dialog initialisation
// All the necessary informations are put into the dialog controls.
/***************************************************************************/
BOOL CWbDlgFilterEquation::OnInitDialog()
{
    CDialog::OnInitDialog();
    // change the font for the text control
    CWnd * pWnd = GetDlgItem(IDC_EQUATION_TEXT0);
    CFont * pFont = pWnd->GetFont(); // get the standard font
    LOGFONT logFont;
    pFont->GetObject(sizeof(LOGFONT), (void *)&logFont); // fill up logFont
    // modify the logFont
    logFont.lfWeight = FW_NORMAL; // not bold
    m_Font.CreateFontIndirect(&logFont); // create the modified font
    pWnd->SetFont(&m_Font); // set the modified font
    pWnd = GetDlgItem(IDC_EQUATION_TEXT1);
    pWnd->SetFont(&m_Font); // set the modified font
    return TRUE;
}

/***************************************************************************/
// CWbDlgFilterEquation::OnOK OK button pressed
// Check first, if the equation is ok. If not, dont close the dialog.
/***************************************************************************/
void CWbDlgFilterEquation::OnOK()
{
    UpdateData(TRUE);
    if (((CProcessWbEquation *)GetParent())->CheckFunction(&m_szEquation))
    {
        EndDialog(IDOK);
    }
    else
    {
        AfxMessageBox(IDS_ERROR_EQUATION,MB_OK,0);
    }
}

/***************************************************************************/
// CWbDlgFilterEquation::OnHelpWorkbench Call Workbench help
/***************************************************************************/
void CWbDlgFilterEquation::OnHelpWorkbench()
{
    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath += "::/User_Interface/Menus/Tools/Workbench.htm";
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

