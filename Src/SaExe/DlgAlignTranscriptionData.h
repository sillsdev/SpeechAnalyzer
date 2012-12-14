/////////////////////////////////////////////////////////////////////////////
// DlgAlignAnnotation.h:
// Interface of the CDlgAlignTranscriptionData class.
//
// Author: Steve MacLean
// copyright 2012 JAARS Inc. SIL
//
// Revision History
// 1.5Test8.3
//         SDM Original version
//
/////////////////////////////////////////////////////////////////////////////
#ifndef DLGALIGNTRANSCRIPTIONDATA_H
#define DLGALIGNTRANSCRIPTIONDATA_H

/////////////////////////////////////////////////////////////////////////////
// CDlgAlignTranscriptionData dialog

class CSaDoc;

class CDlgAlignTranscriptionData : public CDialog
{
	// Construction
public:
	CDlgAlignTranscriptionData(CWnd* pParent, CSaDoc* pSaDoc);
	~CDlgAlignTranscriptionData();

	// Attributes
protected:
	int     m_nState;
	CFont*  m_pFont;
	CSaDoc* m_pSaDoc;

	bool m_bPhoneticModified;
	bool m_bPhonemicModified;
	bool m_bOrthographicModified;
	bool m_bGlossModified;
	int m_nAlignBy;
	int m_nSegmentBy;
	CSaString m_szPhonetic;
	CSaString m_szPhonemic;
	CSaString m_szOrthographic;
	CSaString m_szGloss;

	enum STATE { BACK,NEXT,INIT,PHONETICTEXT,PHONEMICTEXT,ORTHOGRAPHICTEXT,GLOSSTEXT,ALIGNBY,SEGMENTBY,FINISH};

public:
	// Dialog Data
	//{{AFX_DATA(CDlgAlignTranscriptionData)
	enum { IDD = IDD_ANNOTATION_WIZARD };
	BOOL m_bOrthographic;
	BOOL m_bPhonemic;
	BOOL m_bPhonetic;
	BOOL m_bGloss;
	//}}AFX_DATA

	// Operations
protected:
	void SetVisible(int nItem, BOOL bVisible);
	void SetEnable(int nItem, BOOL bEnable);
	void SetText(int nItem, CSaString szText);
	void SetText(int nItem, UINT nIDS);
	void SetState(int nState);
	void SetAnnotation(void);
	void SaveAnnotation(void);
	const CSaString BuildString(int nSegment);
	const CSaString ReadFile(const CSaString& Pathname);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CDlgAlignTranscriptionData)
	virtual BOOL OnInitDialog();
	afx_msg void OnBack();
	afx_msg void OnNext();
	afx_msg void OnRadio();
	afx_msg void OnCheck();
	afx_msg void OnImport();
	afx_msg void OnRevert();
	afx_msg void OnUpdateAnnotation();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void GetSettings( CTranscriptionDataSettings & settings);
};
#endif
