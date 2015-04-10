/////////////////////////////////////////////////////////////////////////////
// DlgASStart.h
// Interface of the CDlgASStart (dialog) class.
//
// Author: Kent Gorham
// copyright 2015 JAARS Inc. SIL
//
/////////////////////////////////////////////////////////////////////////////
#ifndef DLGASSTART_H
#define DLGASSTART_H

class CSaApp;

class CDlgASStart : public CDialog {
	DECLARE_DYNAMIC(CDlgASStart)
public:
    CDlgASStart(CWnd * pParent = NULL);

    enum { IDD = IDD_AUDIOSYNC_STARTUP };

	CString audioFilename;
	CString phraseFilename;
	BOOL segmentAudio;
	BOOL loadData;

protected:
    virtual void DoDataExchange(CDataExchange * pDX);
    virtual BOOL OnInitDialog();
	afx_msg void OnClickedBrowseAudio();
	afx_msg void OnClickedBrowsePhrases();
	afx_msg void OnChange();

	CButton browse2;
	CButton checkbox1;
	CButton checkbox2;
	CEdit edit2;
	CButton ok;

	DECLARE_MESSAGE_MAP()
};

#endif
