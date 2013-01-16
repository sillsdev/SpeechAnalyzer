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
	int m_nWordConvention;
	int m_nPhraseConvention;

public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedBrowseFolder();
	afx_msg void OnBnClickedEditPhraseFolder();
	afx_msg void OnBnClickedEditGlossFolder();

public:
	CString m_szFolderLocation;
	CString m_szFolderName;
	CString m_szPhraseFolderName;
	CString m_szGlossFolderName;

	void SetWordFilenameConvention( int value);
	void SetPhraseFilenameConvention( int value);
	enum EWordFilenameConvention GetWordFilenameConvention();
	enum EPhraseFilenameConvention GetPhraseFilenameConvention();

	BOOL m_bSkipGlossEmpty;
	BOOL m_bOverwriteData;
};
