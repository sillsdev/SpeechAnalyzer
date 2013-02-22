/////////////////////////////////////////////////////////////////////////////
// sa_exprt.h:
// Interface of the      CExportTabbed class.
//
// Author: Steve MacLean
// copyright 1999 JAARS Inc. SIL
//
// Revision History
// 1.5Test8.3
//         SDM Original version
//
/////////////////////////////////////////////////////////////////////////////
#ifndef SA_EXPRT_H
#define SA_EXPRT_H

#include "exportbasicdialog.h"
#include "Settings\OBSTREAM.H"

class CSaDoc;

extern void WriteFileUtf8( CFile *pFile, const CSaString szString);
extern CSaString GetExportFilename( CSaString szTitle, CSaString szFilter, TCHAR *szExtension=_T("txt"));

/////////////////////////////////////////////////////////////////////////////
// CExportFW dialog
class CExportFW : public CExportBasicDialog
{
	// Construction
public:
	CExportFW( const CSaString & szDocTitle, CWnd* pParent = NULL);

public:
	// Implementation

protected:
	// Generated message map functions
	virtual void OnOK();
	virtual void InitializeDialog();
	DECLARE_MESSAGE_MAP()
};

#endif SA_EXPRT_H
