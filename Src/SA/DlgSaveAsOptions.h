#ifndef DLGSAVEASOPTIONS_H
#define DLGSAVEASOPTIONS_H

#include "mixer.h"

class CDlgSaveAsOptions : protected CFileDialog {

public:
	// Construction
    CDlgSaveAsOptions(LPCTSTR lpszDefExt,
                      LPCTSTR lpszFileName,
                      LPCTSTR lpszDefaultDir,
					  DWORD dwFlags,
                      LPCTSTR lpszFilter,
                      CWnd * pParentWnd,
                      bool saveAs,
                      bool stereo,
					  DWORD samplesPerSec);
	virtual INT_PTR DoModal();
	bool IsSameFile();
	CString GetSelectedPath();

    ESaveArea mSaveArea;
    EShowFiles mShowFiles;
    EFileFormat mFileFormat;
	DWORD mSamplingRate;

    bool mStereo;
    bool mSaveAs;

    enum { IDD = IDD_SAVEAS };

protected:
	// DDX/DDV support
    virtual void DoDataExchange(CDataExchange * pDX);   
    afx_msg void OnClicked();
    virtual BOOL OnInitDialog();

	// the original saveas filename
	CString mOriginalPath;
	int mSamplingChoice;
	DWORD mOriginalSamplingRate;

    DECLARE_MESSAGE_MAP()
};

#endif
