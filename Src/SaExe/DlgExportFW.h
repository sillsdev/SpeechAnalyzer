/////////////////////////////////////////////////////////////////////////////
// DlgExportFW.h:
// Interface of the DlgExportFW class.
//
// Author: Steve MacLean
// copyright 1999 JAARS Inc. SIL
//
// Revision History
// 1.5Test8.3
//         SDM Original version
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _DGLEXPORTFW_H
#define _DGLEXPORTFW_H

#include "Settings\OBSTREAM.H"
#include "AppDefs.h"
#include "CSaString.h"
#include "resource.h"

class CSaDoc;

extern CSaString szCrLf;
extern void WriteFileUtf8( CFile *pFile, const CSaString szString);

/////////////////////////////////////////////////////////////////////////////
// CDlgExportFW dialog
class CDlgExportFW : public CDialog
{
	// Construction
public:
	CDlgExportFW( const CSaString & szDocTitle, CWnd* pParent = NULL);

public:
	// Implementation

protected:
	// Generated message map functions
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()

private:
	bool TryExportSegmentsBy( Annotations master, CSaDoc * pDoc, CFile & file, int & count, bool skipEmptyGloss, EWordFilenameConvention convention, LPCTSTR szPath);
	CSaString BuildRecord( Annotations target, DWORD dwStart, DWORD dwStop, CSaDoc * pDoc);
	CSaString BuildPhrase( Annotations target, DWORD dwStart, DWORD dwStop, CSaDoc * pDoc);
	Annotations GetAnnotation( int val);
	BOOL GetFlag( Annotations val);
	int GetIndex( Annotations val);
	LPCTSTR GetTag( Annotations val);
	void UpdateButtonState();

public:
	// Dialog Data
	enum { IDD = IDD_EXPORT_FW };
	BOOL  m_bAllAnnotations;
	BOOL  m_bGloss;
	BOOL  m_bOrtho;
	BOOL  m_bPhonemic;
	BOOL  m_bPhonetic;
	BOOL  m_bPOS;
	BOOL  m_bReference;
	BOOL  m_bTone;
	BOOL  m_bPhrase;

protected:
	CSaString m_szDocTitle;

public:
	void OnHelpExportBasic();

protected:
	void SetEnable(int nItem, BOOL bEnable);
	void SetCheck(int nItem, BOOL bCheck);
	virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV support
	void GetCurrentPath( LPTSTR szBuffer, size_t size);
	CSaString GetExportFilename( CSaString szTitle, CSaString szFilter, TCHAR *szExtension=_T("txt"));

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnAllAnnotations();
	afx_msg void OnClickedExSfmInterlinear();
	afx_msg void OnClickedExSfmMultirecord();
	afx_msg void OnClickedBrowseFieldworks();
	afx_msg void OnClickedBrowseOther();
	afx_msg void OnRadioFieldworks();
	afx_msg void OnRadioOther();

public:
	CButton m_BrowseOther;
	CEdit m_EditFieldWorksFolder;
	CEdit m_EditOtherFolder;
	CComboBox m_ComboFieldWorksProject;
	CStatic m_StaticFieldWorksProject;
	CButton m_RadioFieldWorks;
	CButton m_RadioOther;
	CButton m_ButtonOK;
	afx_msg void OnSelchangeComboFieldworksProject();
	afx_msg void OnKillfocusComboFieldworksProject();
	CStatic m_StaticTags;
};

#endif
