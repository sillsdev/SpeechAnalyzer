/////////////////////////////////////////////////////////////////////////////
// DlgSynthesis.cpp:
// Implementation of the CDlgSynthesis (property sheet)
//
// Author: Steve MacLean
// copyright 2001 JAARS Inc. SIL
//
// Revision History
//
// 02/15/2001
//    SDM   Replicated from advancedParameters.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DlgSynthesis.h"
#include "mainfrm.h"
#include "synthesis\sa_klatt\synth.h"
#include "DlgMbrola.h"

// The debugger can't handle symbols more than 255 characters long.
// STL often creates symbols longer than that.
// When symbols are longer than 255 characters, the warning is disabled.
#pragma warning(disable:4786)

//###########################################################################
// CDlgSynthesis property sheet
// Displays all controls to synthesize

IMPLEMENT_DYNAMIC(CDlgSynthesis, CPropertySheet)

/////////////////////////////////////////////////////////////////////////////
// CDlgSynthesis message map

BEGIN_MESSAGE_MAP(CDlgSynthesis, CPropertySheet)
    //{{AFX_MSG_MAP(CDlgSynthesis)
    //}}AFX_MSG_MAP
    ON_COMMAND(IDHELP, OnHelpSynthesis)
    ON_COMMAND(IDOK, OnClose)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSynthesis construction/destruction/creation

/***************************************************************************/
// CDlgSynthesis::CDlgSynthesis Constructor
/***************************************************************************/
CDlgSynthesis::CDlgSynthesis(LPCTSTR pszCaption, CWnd * pParent, UINT iSelectPage)
    : CPropertySheet(pszCaption, pParent, iSelectPage)
{
    //{{AFX_DATA_INIT(CDlgSynthesis)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    m_pDlgMbrolaPage = new CDlgMbrola;

    AddPage(m_pDlgMbrolaPage);
}


/***************************************************************************/
// CDlgSynthesis::~CDlgSynthesis Destructor
/***************************************************************************/
CDlgSynthesis::~CDlgSynthesis()
{
    if (m_pDlgMbrolaPage)
    {
        delete m_pDlgMbrolaPage;
    }
}


/////////////////////////////////////////////////////////////////////////////
// CDlgSynthesis helper functions

/***************************************************************************/
// CDlgSynthesis::DoDataExchange Data exchange
/***************************************************************************/
void CDlgSynthesis::DoDataExchange(CDataExchange * pDX)
{
    CPropertySheet::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgSynthesis)
    // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CDlgSynthesis message handlers


/***************************************************************************/
// CDlgSynthesis::OnInitDialog
/***************************************************************************/
BOOL CDlgSynthesis::OnInitDialog()
{
    CPropertySheet::OnInitDialog();

    CRect rect;
    GetWindowRect(rect);

    CRect newRect(0,0,75,23);
    CWnd * pWnd = GetDlgItem(IDOK);
    if (pWnd)
    {
        pWnd->GetWindowRect(newRect);
        // this button is already on the form, but MFC has hidden and disabled it
        pWnd->ShowWindow(SW_SHOW);
        pWnd->EnableWindow(TRUE);
    }
    else
    {
        m_cOK.Create(_T("OK"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, newRect, this, IDOK);
        pWnd = & m_cOK;
    }

    rect.InflateRect(0,0,0,int(newRect.Height()/4)*2 + newRect.Height());
    MoveWindow(rect);

    GetClientRect(rect);

    newRect -= newRect.TopLeft();
    newRect.OffsetRect(CPoint(rect.right - newRect.Width() - newRect.Height()/4,
                              rect.bottom - newRect.Height() - newRect.Height()/4));

    pWnd->MoveWindow(newRect);

    // newRect.OffsetRect(CPoint(-(newRect.Width() + newRect.Height()/2), 0));
    // m_cHelp.Create("Help", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, newRect, this, IDHELP);

    // m_cHelp.SetFont(pWnd->GetFont());

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

/***************************************************************************/
// CDlgSynthesis::OnHelpSynthesis Call Synthesis help
/***************************************************************************/
void CDlgSynthesis::OnHelpSynthesis()
{
    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath = szPath.Left(szPath.ReverseFind('\\'));
    szPath = szPath.Left(szPath.ReverseFind('\\')) + "\\HELP\\SYNTHESIS2.HLP";
    ::WinHelp(AfxGetMainWnd()->GetSafeHwnd(), szPath, HELP_INDEX, 0);
}
