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

	enum { IDD = IDD_SPLIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

private:
	int m_WordConvention;
	int m_PhraseConvention;

public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedBrowseFolder();
	afx_msg void OnBnClickedEditPhraseFolder();
	afx_msg void OnBnClickedEditGlossFolder();

public:
	CString m_FolderLocation;
	CString m_FolderName;
	CString m_PhraseFolderName;
	CString m_GlossFolderName;

	enum EWordFilenameConvention GetWordFilenameConvention();
	enum EPhraseFilenameConvention GetPhraseFilenameConvention();
	CButton m_CheckGlossEmpty;
	BOOL m_SkipGlossEmpty;
};
