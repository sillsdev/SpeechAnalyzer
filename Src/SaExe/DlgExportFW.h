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

/////////////////////////////////////////////////////////////////////////////
// CDlgExportFW dialog
class CDlgExportFW : public CDialog
{
public:
	CDlgExportFW( LPCTSTR szDocTitle,
			      BOOL gloss,
				  BOOL ortho,
				  BOOL phonemic,
				  BOOL phonetic,
				  BOOL pos,
				  BOOL reference,
				  BOOL phrase,
				  CWnd* pParent = NULL);

	BOOL bGloss;
	BOOL bOrtho;
	BOOL bPhonemic;
	BOOL bPhonetic;
	BOOL bPOS;
	BOOL bReference;
	BOOL bPhrase;
	CString szDocTitle;
	CString szPath;

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV support
	void SetEnable(int nItem, BOOL bEnable);
	void SetCheck(int nItem, BOOL bCheck);
	void GetCurrentPath( LPTSTR szBuffer, size_t size);
	void UpdateButtonState();
	void WriteFileUtf8( CFile *pFile, const CSaString szString);
	CSaString GetFieldWorksProjectDirectory();

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

	enum { IDD = IDD_EXPORT_FW };
	CButton ctlButtonBrowseOther;
	CEdit ctlEditFieldWorksFolder;
	CEdit ctlEditOtherFolder;
	CComboBox ctlComboFieldWorksProject;
	CStatic ctlStaticFieldWorksProject;
	CButton ctlRadioFieldWorks;
	CButton ctlRadioOther;
	CButton ctlButtonOK;
	CStatic ctlStaticTags;

	BOOL bGlossDflt;
	BOOL bOrthoDflt;
	BOOL bPhonemicDflt;
	BOOL bPhoneticDflt;
	BOOL bPOSDflt;
	BOOL bReferenceDflt;
	BOOL bPhraseDflt;

	DECLARE_MESSAGE_MAP()
};

#endif
