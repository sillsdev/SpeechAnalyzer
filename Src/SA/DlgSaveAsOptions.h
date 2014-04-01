#ifndef DLGSAVEASOPTIONS_H
#define DLGSAVEASOPTIONS_H

#include "mixer.h"
#include "AppDefs.h"

class CDlgSaveAsOptions : public CFileDialog
{
// Construction
public:
    CDlgSaveAsOptions(LPCTSTR lpszDefExt,
                      LPCTSTR lpszFileName,
                      DWORD dwFlags,
                      LPCTSTR lpszFilter,
                      CWnd * pParentWnd,
					  bool saveAs,
					  bool stereo);

    ESaveArea m_eSaveArea;
    EShowFiles m_eShowFiles;
	EFileFormat m_eFileFormat;
    enum { IDD = IDD_SAVEAS };

protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support
    afx_msg void OnClicked();
    virtual BOOL OnInitDialog();

	bool stereo;
	bool saveAs;

	DECLARE_MESSAGE_MAP()
};

#endif
