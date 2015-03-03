#ifndef FILEOPEN_H
#define FILEOPEN_H

#include "mixer.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgFileOpen dialog

class CDlgFileOpen : public CFileDialog {
    DECLARE_DYNAMIC(CDlgFileOpen)

public:
    CDlgFileOpen(LPCTSTR lpszDefExt = NULL,
                 LPCTSTR lpszFileName = NULL,
                 DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                 LPCTSTR lpszFilter = NULL,
                 CWnd * pParentWnd = NULL);

    enum { IDD = IDD_FILEOPEN };

protected:
    afx_msg void OnPlay();
    DECLARE_MESSAGE_MAP()
};

#endif
