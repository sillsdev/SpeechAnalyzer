#ifndef DLGSAVEASOPTIONS_H
#define DLGSAVEASOPTIONS_H

#include "mixer.h"

class CDlgSaveAsOptions : public CFileDialog
{
// Construction
public:
    CDlgSaveAsOptions(LPCTSTR lpszDefExt = NULL,
                      LPCTSTR lpszFileName = NULL,
                      DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                      LPCTSTR lpszFilter = NULL,
                      CWnd * pParentWnd = NULL);

    static int m_nSaveArea;
    static int m_nShowFiles;
    enum { IDD = IDD_SAVEAS };

    enum { saveEntire = 0 , saveView = 1, saveCursors = 2 };
    enum { showNew = 0, showOriginal = 1, showBoth = 2};

protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support

protected:

    afx_msg void OnClicked();
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()
};

#endif
