#pragma once
#include "afxwin.h"
#include "appdefs.h"

// CDlgSplit dialog
class CDlgSplit : public CDialog
{
	DECLARE_DYNAMIC(CDlgSplit)
public:
	CDlgSplit(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSplit();

	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_SPLIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

private:
	int m_iConvention;

public:
	CString m_FolderLocation;
	CString m_FolderName;
	CString m_PhraseFolderName;
	CString m_GlossFolderName;
	BOOL m_ExportPhrase;
	BOOL m_ExportGloss;

	//{{AFX_MSG(CDlgParametersPitchPage)
	afx_msg void OnBnClickedExportGloss();
	afx_msg void OnBnClickedExportPhrase();
	afx_msg void OnBnClickedBrowseFolder();
	//}}AFX_MSG

	enum EFilenameConvention GetFilenameConvention();

	DECLARE_MESSAGE_MAP()
};
