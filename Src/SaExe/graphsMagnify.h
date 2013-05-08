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
#define _GRAPHS_MAGNIFY_H

#include "resource.h"

class CDlgMagnify : public CDialog
{
public:
    CDlgMagnify(CWnd * pParent = NULL);					// standard constructor

    enum { IDD = IDD_MAGNIFY };
    double   m_fMagnify;

protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support

    // Generated message map functions
    virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};

#endif
