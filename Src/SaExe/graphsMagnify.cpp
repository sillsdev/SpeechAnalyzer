/////////////////////////////////////////////////////////////////////////////
// grpahsMagnify.cpp:
// Implementation of the CDlgMagnify (dialog)
//
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//
// 01/12/2000
//    SDM   Extract from sa_dlg.h
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "graphsMagnify.h"

//###########################################################################
// CDlgMagnify dialog

/////////////////////////////////////////////////////////////////////////////
// CDlgMagnify message map

BEGIN_MESSAGE_MAP(CDlgMagnify, CDialog)
//{{AFX_MSG_MAP(CDlgMagnify)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgMagnify construction/destruction/creation

/***************************************************************************/
// CDlgMagnify::CDlgMagnify Constructor
/***************************************************************************/
CDlgMagnify::CDlgMagnify(CWnd * pParent) : CDialog(CDlgMagnify::IDD, pParent)
{
    m_fMagnify = 1.0;
}

/***************************************************************************/
// CDlgMagnify::DoDataExchange Data exchange
/***************************************************************************/
void CDlgMagnify::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_MAGNIFY, m_fMagnify);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgMagnify message handlers

/***************************************************************************/
// CDlgMagnify::OnInitDialog Dialog initialization
// The dialog is centered over the main frame window.
/***************************************************************************/
BOOL CDlgMagnify::OnInitDialog()
{
    CDialog::OnInitDialog();
    CenterWindow(); // center dialog on frame window
    return TRUE;  // return TRUE  unless you set the focus to a control
}

