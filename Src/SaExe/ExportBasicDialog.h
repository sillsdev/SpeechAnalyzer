/////////////////////////////////////////////////////////////////////////////
// ExportBasicDialog.h:
//
// Author: Steve MacLean
// copyright 2012 JAARS Inc. SIL
//
// Revision History
// 1.5Test8.3
//         SDM Original version
//
/////////////////////////////////////////////////////////////////////////////
#ifndef EXPORTBASICDIALOG_H
#define EXPORTBASICDIALOG_H

#include "resource.h"
#include "CSaString.h"

/////////////////////////////////////////////////////////////////////////////
// CExportBasicDialog

class CExportBasicDialog : public CDialog
{
	// Construction
public:
	CExportBasicDialog( const CSaString& szDocTitle, CWnd* pParent = NULL); // standard constructor

public:
	// Dialog Data
	enum { IDD = IDD_EXPORT_SFM };
	BOOL  m_bAllSource;
	BOOL  m_bBandwidth;
	BOOL  m_bBits;
	BOOL  m_bComments;
	BOOL  m_bDialect;
	BOOL  m_bEthnologue;
	BOOL  m_bFamily;
	BOOL  m_bAllFile;
	BOOL  m_bFileSize;
	BOOL  m_bFileName;
	BOOL  m_bFree;
	BOOL  m_bGender;
	BOOL  m_bHighPass;
	BOOL  m_bInterlinear;
	BOOL  m_bMultiRecord;
	BOOL  m_bLanguage;
	BOOL  m_bLastModified;
	BOOL  m_bLength;
	BOOL  m_bNotebookRef;
	BOOL  m_bNumberSamples;
	BOOL  m_bOriginalDate;
	BOOL  m_bOriginalFormat;
	BOOL  m_bPhones;
	BOOL  m_bSampleRate;
	BOOL  m_bAllParameters;
	BOOL  m_bRegion;
	BOOL  m_bSpeaker;
	BOOL  m_bTranscriber;
	BOOL  m_bWords;
	BOOL  m_bAllAnnotations;
	BOOL  m_bGloss;
	BOOL  m_bOrtho;
	BOOL  m_bPhonemic;
	BOOL  m_bPhonetic;
	BOOL  m_bPOS;
	BOOL  m_bReference;
	BOOL  m_bTone;
	BOOL  m_bCountry;
	BOOL  m_bQuantization;
	BOOL    m_bPhrase;
protected:
	virtual void InitializeDialog() = 0;
	CSaString m_szFileName;
	CSaString m_szDocTitle;

	// Implementation
public:
	void OnHelpExportBasic();
protected:
	void SetEnable(int nItem, BOOL bEnable);
	void SetCheck(int nItem, BOOL bCheck);
	virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV support
	virtual void OnOK() = 0; // this must be implented by the derived types

	// Generated message map functions
	afx_msg void OnAllSource();
	afx_msg void OnAllFileInfo();
	afx_msg void OnAllParameters();
	virtual BOOL OnInitDialog();
	afx_msg void OnAllAnnotations();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClickedExSfmInterlinear();
	afx_msg void OnClickedExSfmMultirecord();
};

#endif
