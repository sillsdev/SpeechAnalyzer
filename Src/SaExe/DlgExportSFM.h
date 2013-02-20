#ifndef DLGEXPORTSFM_H
#define DLGEXPORTSFM_H

#include "CSaString.h"
#include "SA_Exprt.h"
#include "AppDefs.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgExportSFM dialog

class CDlgExportSFM : public CExportBasicDialog
{
	// Construction
public:
	CDlgExportSFM( const CSaString& szDocTitle, CWnd* pParent = NULL);

protected:
	// Generated message map functions
	virtual void OnOK();
	virtual void InitializeDialog();
	DECLARE_MESSAGE_MAP()

	void ExportStandard();
	void ExportMultiRec();

	bool TryExportSegmentsBy( Annotations master, CSaDoc * pDoc, CFile & file);
	void ExportFile( CSaDoc * pDoc, CFile & file);
	void ExportCounts( CSaDoc * pDoc, CFile & file);
	void ExportAllFileInformation( CSaDoc * pDoc, CFile & file);
	void ExportAllParameters( CSaDoc * pDoc, CFile & file);
	void ExportAllSource( CSaDoc * pDoc, CFile & file);

private:
	Annotations GetAnnotation( int val);
	bool GetFlag( Annotations val);
	int GetIndex( Annotations val);
	LPCTSTR GetTag( Annotations val);
};

#endif DLGEXPORTSFM_H
