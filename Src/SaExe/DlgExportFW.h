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

class CExportFWData {
public:
	CExportFWData( LPCTSTR szDocTitle);
	BOOL bAllAnnotations;
	BOOL bGloss;
	BOOL bOrtho;
	BOOL bPhonemic;
	BOOL bPhonetic;
	BOOL bPOS;
	BOOL bReference;
	BOOL bPhrase;
	CString szDocTitle;
	CString szPath;
};

/////////////////////////////////////////////////////////////////////////////
// CDlgExportFW dialog
class CDlgExportFW : public CDialog
{
	// Construction
public:
	CDlgExportFW( const CSaString & szDocTitle, CWnd* pParent = NULL);

	// Dialog Data
	enum { IDD = IDD_EXPORT_FW };

	CExportFWData data;
	CButton m_BrowseOther;
	CEdit m_EditFieldWorksFolder;
	CEdit m_EditOtherFolder;
	CComboBox m_ComboFieldWorksProject;
	CStatic m_StaticFieldWorksProject;
	CButton m_RadioFieldWorks;
	CButton m_RadioOther;
	CButton m_ButtonOK;
	CStatic m_StaticTags;

protected:
	// Generated message map functions
	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV support

	void SetEnable(int nItem, BOOL bEnable);
	void SetCheck(int nItem, BOOL bCheck);
	void GetCurrentPath( LPTSTR szBuffer, size_t size);
	void UpdateButtonState();

	afx_msg void OnAllAnnotations();
	afx_msg void OnClickedExSfmInterlinear();
	afx_msg void OnClickedExSfmMultirecord();
	afx_msg void OnClickedBrowseFieldworks();
	afx_msg void OnClickedBrowseOther();
	afx_msg void OnRadioFieldworks();
	afx_msg void OnRadioOther();
	afx_msg void OnHelpExportBasic();
	afx_msg void OnSelchangeComboFieldworksProject();
	afx_msg void OnKillfocusComboFieldworksProject();
};

#endif
