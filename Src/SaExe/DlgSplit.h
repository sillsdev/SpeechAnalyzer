#pragma once
#include "afxwin.h"

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
	DECLARE_MESSAGE_MAP()

public:
	int m_iConvention;
	CString m_FolderLocation;
	CString m_FolderName;
	CString m_PhraseFolderName;
	CString m_WordFolderName;
	BOOL m_ExportPhrase;
	BOOL m_ExportWord;
	afx_msg void OnBnClickedExportWord();
	afx_msg void OnBnClickedExportPhrase();
};
