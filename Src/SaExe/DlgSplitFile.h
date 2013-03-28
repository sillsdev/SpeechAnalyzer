#ifndef DLGSPLITFILE_H
#define DLGSPLITFILE_H

#include "afxwin.h"
#include "appdefs.h"

// CDlgSplitFile dialog
class CDlgSplitFile : public CDialog
{

    DECLARE_DYNAMIC(CDlgSplitFile)
public:
    CDlgSplitFile(CWnd * pParent = NULL);  // standard constructor
    virtual ~CDlgSplitFile();

    virtual BOOL OnInitDialog();
    void SetWordFilenameConvention(int value);
    void SetPhraseFilenameConvention(int value);
    enum EWordFilenameConvention GetWordFilenameConvention();
    enum EPhraseFilenameConvention GetPhraseFilenameConvention();

    afx_msg void OnBnClickedBrowseFolder();
    afx_msg void OnBnClickedEditPhraseFolder();
    afx_msg void OnBnClickedEditGlossFolder();

    CString m_szFolderLocation;
    CString m_szFolderName;
    CString m_szPhraseFolderName;
    CString m_szGlossFolderName;
    BOOL m_bSkipGlossEmpty;
    BOOL m_bOverwriteData;

    DECLARE_MESSAGE_MAP()

    enum { IDD = IDD_SPLIT };

protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support

private:
    int m_nWordConvention;
    int m_nPhraseConvention;
};

#endif
