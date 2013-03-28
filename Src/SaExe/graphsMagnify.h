/////////////////////////////////////////////////////////////////////////////
// graphsMagnify.h:
// Interface of the CDlgMagnify (dialog)
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
#ifndef _GRAPHS_MAGNIFY_H

#include "resource.h"

#define _GRAPHS_MAGNIFY_H

//###########################################################################
// CDlgMagnify dialog

class CDlgMagnify : public CDialog
{

    // Construction/destruction/creation
public:
    CDlgMagnify(CWnd * pParent = NULL); // standard constructor

    // Attributes
private:

    // Dialog Data
public:
    //{{AFX_DATA(CDlgMagnify)
    enum { IDD = IDD_MAGNIFY };
    double   m_fMagnify;
    //}}AFX_DATA

    // Operations
protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CDlgMagnify)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif //_GRAPHS_MAGNIFY_H
