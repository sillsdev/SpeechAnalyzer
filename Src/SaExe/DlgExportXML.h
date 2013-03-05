#ifndef DLGEXPORTXML_H
#define DLGEXPORTXML_H

#include "CSaString.h"
#include "ExportBasicDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgExportXML dialog

class CDlgExportXML : public CExportBasicDialog
{
	// Construction
public:
	CDlgExportXML( const CSaString& szDocTitle, CWnd* pParent = NULL);

public:
	// Implementation
	static const char XML_HEADER1[];
	static const char XML_HEADER2[];
	static const char XML_FOOTER[];
	static void CDlgExportXML::OutputXMLField( CFile* pFile, const TCHAR *szFieldName, const CSaString &szContents);

protected:
	// Generated message map functions
	virtual void OnOK();
	virtual void InitializeDialog();

	DECLARE_MESSAGE_MAP()
};

#endif DLGEXPORTXML_H
