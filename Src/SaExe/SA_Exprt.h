/////////////////////////////////////////////////////////////////////////////
// sa_exprt.h:
// Interface of the      CExportSFM class.
//                       CExportTabbed class.
//                       CImport class.
//
// Author: Steve MacLean
// copyright 1999 JAARS Inc. SIL
//
// Revision History
// 1.5Test8.3
//         SDM Original version
//
/////////////////////////////////////////////////////////////////////////////

#include "exportbasicdialog.h"

/////////////////////////////////////////////////////////////////////////////
// CExportSFM dialog

class CExportSFM : public CExportBasicDialog
{
	// Construction
public:
	CExportSFM( const CSaString& szDocTitle, CWnd* pParent = NULL) : CExportBasicDialog( szDocTitle, pParent) {} // standard constructor

public:
	// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CExportSFM)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CExportXML dialog

class CExportXML : public CExportBasicDialog
{
	// Construction
public:
	CExportXML( const CSaString& szDocTitle, CWnd* pParent = NULL) : CExportBasicDialog(szDocTitle, pParent) {} // standard constructor

public:
	// Implementation
	static const char XML_HEADER1[];
	static const char XML_HEADER2[];
	static const char XML_FOOTER[];
	static void CExportXML::OutputXMLField(CFile* pFile,const TCHAR *szFieldName,const CSaString &szContents);

protected:
	// Generated message map functions
	//{{AFX_MSG(CExportXML)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CExportTable dialog

class CExportTable : public CDialog
{
	// Construction
public:
	CExportTable(const CSaString& szDocTitle, CWnd* pParent = NULL); // standard constructor

	// Dialog Data
	//{{AFX_DATA(CExportTable)
	enum { IDD = IDD_EXPORT_TABLE };
	BOOL  m_bAnnotations;
	BOOL  m_bCalculations;
	BOOL  m_bF1;
	BOOL  m_bF2;
	BOOL  m_bF3;
	BOOL  m_bF4;
	BOOL  m_bFormants;
	BOOL  m_bGloss;
	CSaString m_szIntervalTime;
	BOOL  m_bSegmentLength;
	BOOL  m_bMagnitude;
	BOOL  m_bOrtho;
	BOOL  m_bPhonemic;
	BOOL  m_bPhonetic;
	BOOL  m_bPitch;
	BOOL  m_bPOS;
	BOOL  m_bReference;
	BOOL  m_bSegmentStart;
	BOOL  m_bSampleTime;
	BOOL  m_bTone;
	BOOL  m_bZeroCrossings;
	int   m_nSampleRate;
	int   m_nCalculationMethod;
	BOOL  m_bPhonetic2;
	int     m_nRegion;
	BOOL	m_bMelogram;
	//}}AFX_DATA
private:
	CSaString m_szFileName;
	CSaString m_szDocTitle;

	// Implementation
public:
	void OnHelpExportTable();
protected:
	void SetEnable(int nItem, BOOL bEnable);
	void SetVisible(int nItem, BOOL bVisible, BOOL bEnable = TRUE);
	virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CExportTable)
	afx_msg void OnAllAnnotations();
	afx_msg void OnAllFormants();
	afx_msg void OnAllCalculations();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnUpdateIntervalTime();
	afx_msg void OnSample();
	afx_msg void OnPhonetic();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CImport{
public:
	CImport(const CSaString& szFileName, BOOL batch=FALSE) {m_szPath = szFileName;m_bBatch = batch;}; // standard constructor

	CSaString m_szPath;
	BOOL m_bBatch;

	enum { KEEP = 0, AUTO = 1, MANUAL = 2, QUERY = 3 };

	BOOL Import(int nMode = QUERY);
	void AutoAlign( CSaDoc * pSaDoc, const CSaString * Phonetic, const CSaString * Phonemic, const CSaString * Ortho, const CSaString * Gloss);

private:
	BOOL ReadTable(Object_istream &ios, int nMode = QUERY);
};

/////////////////////////////////////////////////////////////////////////////
// CImportDlg dialog

class CImportDlg : public CDialog
{
	// Construction
public:
	CImportDlg(CWnd* pParent = NULL); // standard constructor

	// Dialog Data
	//{{AFX_DATA(CImportDlg)
	enum { IDD = IDD_IMPORT_DIALOG };
	int   m_nMode;
	//}}AFX_DATA

	// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CImportDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
