/////////////////////////////////////////////////////////////////////////////
// DlgAnnotation.h:
// Interface of the CDlgAnnontation class.
//
// Author: Steve MacLean
// copyright 2012 JAARS Inc. SIL
//
// Revision History
// 1.5Test8.3
//         SDM Original version
//
/////////////////////////////////////////////////////////////////////////////
#ifndef DLGANNOTATION_H
#define DLGANNOTATION_H

/////////////////////////////////////////////////////////////////////////////
// CDlgAnnotation dialog

class CSaDoc;

class CDlgAnnotation : public CDialog
{
	// Construction
public:
	enum MODE { IMPORT, ALIGN };
	CDlgAnnotation(CWnd* pParent, MODE mode, CSaDoc* pSaDoc);
	~CDlgAnnotation();

	// Attributes
protected:
	bool    m_bPhoneticModified;
	bool    m_bPhonemicModified;
	bool    m_bOrthographicModified;
	bool    m_bGlossModified;

	MODE    m_mode;
	int     m_nState;
	int     m_nAlignBy;
	int     m_nSegmentBy;
	CSaString m_szPhonetic;
	CSaString m_szPhonemic;
	CSaString m_szOrthographic;
	CSaString m_szGloss;
	CFont*  m_pFont;
	CSaDoc* m_pSaDoc;

	const char* m_szPhonemicNullSegment;
	const char* m_szOrthographicNullSegment;

	enum STATE { BACK,NEXT,INIT,PHONETICTEXT,PHONEMICTEXT,ORTHOGRAPHICTEXT,GLOSSTEXT,ALIGNBY,SEGMENTBY,FINISH};

public:
	// Dialog Data
	//{{AFX_DATA(CDlgAnnotation)
	enum { IDD = IDD_ANNOTATIONWIZARD };
	BOOL m_bOrthographic;
	BOOL m_bPhonemic;
	BOOL m_bPhonetic;
	BOOL m_bGloss;
	//}}AFX_DATA

public:
	void AutoAlign(const CSaString& Phonetic, const CSaString& Phonemic,
		const CSaString& Ortho, const CSaString& Gloss,
		CSaString& Alignment, CSaString& Segmentation);

	void AutoAlign(const CSaString* Phonetic, const CSaString* Phonemic, const CSaString* Ortho, const CSaString* Gloss);
	// Operations
protected:
	void SetVisible(int nItem, BOOL bVisible);
	void SetEnable(int nItem, BOOL bEnable);
	void SetText(int nItem, CSaString szText);
	void SetText(int nItem, UINT nIDS);
	void SetChecked(int nItem);
	void SetState(int nState);
	void SetAnnotation(void);
	void SaveAnnotation(void);
	const CSaString BuildString(int nSegment);
	void OK();
	const CSaString ReadFile(const CSaString& Pathname);


	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CDlgAnnotation)
	virtual BOOL OnInitDialog();
	afx_msg void OnBack();
	afx_msg void OnNext();
	afx_msg void OnRadio();
	afx_msg void OnImport();
	afx_msg void OnRevert();
	afx_msg void OnUpdateAnnotation();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
#endif
