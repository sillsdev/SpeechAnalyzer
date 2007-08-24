// SA Builder.h : main header file for the SA BUILDER application
//

#if !defined(AFX_SABUILDER_H__6AC8E62F_9584_4A20_80C5_C34034E5C40D__INCLUDED_)
#define AFX_SABUILDER_H__6AC8E62F_9584_4A20_80C5_C34034E5C40D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSABuilderApp:
// See SA Builder.cpp for the implementation of this class
//

class CSABuilderApp : public CWinApp
{
public:
	CSABuilderApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSABuilderApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSABuilderApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SABUILDER_H__6AC8E62F_9584_4A20_80C5_C34034E5C40D__INCLUDED_)
